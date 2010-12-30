#include <upgm/payment.hpp>
#include <upgm/data_tree.hpp>
#include <upgm/request_template.hpp>
#include <upgm/common_hooks.hpp>
#include <upgm/upgm.hpp>
#include <upgm/http_transport.hpp>
#include <upgm/db_mysql.hpp>
#include <upgm/xml_parser.hpp>
#include <upgm/mysql_connection.hpp>

#include <fstream>
#include <sstream>
#include <limits>
#include <cstdlib>

namespace PG
{


UPGM::UPGM()
{
}

UPGM::~UPGM() throw()
{
	Hooks::const_iterator it = _hooks.begin();
	const Hooks::const_iterator end = _hooks.end();
	for (; end != it; ++it) { delete it->second; }

}

void UPGM::registerHook(HookPtr  hook)
{
	const char * hookName = hook->name();
	try
	{
		const Config::Section & section = _config.section(hookName);
		hook->configure(section);
	}
	catch (...) { /* unconfigured*/ }
	const Hooks::iterator it = _hooks.find(hookName);
	const Hooks::const_iterator end = _hooks.end();
	if (end != it)
	{
		delete it->second;
		it->second = hook.release();
	}
	else
	{
		_hooks.insert(std::make_pair(hookName, hook.release()));
	}
}

std::string UPGM::evaluateConfigValue(const std::string & value)
{
	const char variableMark = '$';
	if (value.length() > 0)
	{
		if (value[0] == variableMark)
		{
			const std::string pathStr(value.begin() + 1, value.end());
			const Path path ( pathFromString( pathStr ));
			if (path.size() > 0)
			{
				const std::string & hookName = path[0];
				Hooks::const_iterator it = _hooks.find(hookName);
				if (it != _hooks.end())
				{
					return it->second->read(Path(path.begin()+1, path.end()));
				}
				throw HookUndefException(hookName);
			}
		}
	}
	return value;
}

void UPGM::evaluateConfigParam(const std::string & param, const std::string & value)
{
	const char variableMark = '$';
	if (param.length() > 0)
	{
		std::string::const_iterator begin = param.begin();
		const std::string::const_iterator end = param.end();
		if (param[0] == variableMark) {
			++begin;
		}
		const std::string pathStr(begin, end);
		const Path path ( pathFromString( pathStr ));
		if (path.size() > 0)
		{
			const std::string & hookName = path[0];
			Hooks::const_iterator it = _hooks.find(hookName);
			if (it != _hooks.end())
			{
				it->second->write(Path(path.begin()+1, path.end()), value);
				return;
			}
			throw HookUndefException(hookName);
		}
	}
	throw NotAVariableException(value);
}

void UPGM::evalParams(const std::string & sectionName)
{
	const Config::Section & section = _scheme.section(sectionName);
	for (Config::Section::const_iterator it = section.begin();
	     it != section.end();
	     ++it) {
		try {
			evaluateConfigParam(it->first, evaluateConfigValue(it->second));
		} catch (const NotAVariableException & e) { ;; }
	}
}

void UPGM::performStage(DbConnection * dbConnection,
                        Payment & payment)
{
	try
	{
		const std::string payid(payment.generateDataTree()("id"));
		_log.setup(_logPath, payid);
		_log << "*** Got payment " << payid << "\n";
		DataTree mainConfig;
		CodeHook::populate( mainConfig, _config.section("main") );
		std::auto_ptr<Transport> transport = this->getTransport( mainConfig("transport") );
		std::auto_ptr<Parser> parser = this->getParser( mainConfig("parser") );
		std::auto_ptr<Db> db = this->getDb( mainConfig("db"), dbConnection );

		registerHook( HookPtr( new VarHook() ) );
		registerHook( HookPtr( new MainConfigHook() ) );
		registerHook( HookPtr( new StageHook(&_sequence) ) );
		registerHook( HookPtr( new PrintHook() ) );
		registerHook( HookPtr( new CodeHook(_codes) ) );

		registerHook( HookPtr( new TransportHook( transport.get() ) ) );
		registerHook( HookPtr( new ParserHook( parser.get() ) ) );
		registerHook( HookPtr( new RequestHook () ) );
		registerHook( HookPtr( new PaymentHook( &payment ) ) );
		registerHook( HookPtr( new DbHook( db.get() ) ) );
		registerHook( HookPtr( new ConfigHook( &_hooks ) ) );
		registerHook( HookPtr( new LogHook( ) ) );
		registerHook( HookPtr( new TemplateHook( ) ) );

		registerUserHooks(*(transport.get()), *(parser.get()), payment);

		do
		{
			_log << "Performing stage " << _sequence.stageName() << "\n";
			evalParams(_sequence.stageName());
		}
		while ( payment.stateUndef() );
	}
	catch (std::exception & e)
	{
		_log << "*** Error: " << e.what() << "\n";
		const char * errStr = e.what();
		fprintf(stderr, "Error %s\n", errStr);
		payment.error(errStr ? errStr : "(null)");
		payment.sleep(60*10);
	}
	catch (...)
	{
		payment.error("Unknown error occured");
		payment.failed();
	}

}

void UPGM::setLog(const std::string & path) {
	_logPath = path;
}

void UPGM::setScheme(const Config & requestScheme)
{
	_scheme = requestScheme;
}

void UPGM::setConfig(const Config & config)
{
	_config = config;
}

void UPGM::setCodes(const Config & codes)
{
	_codes = codes;
}

std::auto_ptr<Transport> UPGM::getTransport(const std::string & name)
{
	if (name == "http")
	{
		return std::auto_ptr<Transport>(new PG::HTTPTransport( _log ));
	}
	throw std::runtime_error("Unsupported transport type - " + name);
}

std::auto_ptr<Parser> UPGM::getParser(const std::string & name)
{
	if (name == "xml")
	{
		return std::auto_ptr<Parser>(new PG::XmlParser());
	}
	throw std::runtime_error("Unsupported parser type - " + name);
}

std::auto_ptr<Db> UPGM::getDb(const std::string & name, DbConnection * dbConnection)
{
	if (name == "mysql")
	{
		return std::auto_ptr<Db>(new PG::DbMysql(dynamic_cast<MysqlConnection*>(dbConnection)));
	}
	throw std::runtime_error("Unsupported db type - " + name);
}

}
