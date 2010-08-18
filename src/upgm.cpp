#include <upgm/payment.hpp>
#include <upgm/data_tree.hpp>
#include <upgm/request_template.hpp>
#include <upgm/common_hooks.hpp>
#include <upgm/upgm.hpp>

#include <fstream>
#include <sstream>
#include <limits>
#include <cstdlib>

namespace PG
{


UPGM::UPGM()
{
	// TODO defeat memleak
	registerHook(new StageHook(&_sequence));
	registerHook(new PrintHook());

/*	registerHook( "pay"       , &UPGM::paymentHookRead   );
	registerHook( "code"      , &UPGM::codeHookRead      );
	registerHook( "answer"    , &UPGM::answerHookRead    );
	registerHook( "transport" , &UPGM::transportHookRead );
	registerHook( "db"        , &UPGM::dbHookRead        );
	registerHook( "next"      , &UPGM::nextHook          );

	registerHook( "db"      , &UPGM::dbHookWrite    );
	registerHook( "stage"   , &UPGM::stageHookWrite );
	registerHook( "codes"   , &UPGM::codesHookWrite );
	registerHook( "action"  , &UPGM::actionHookWrite );

	registerHook( "request"  , &UPGM::requestHookWrite );
	registerHook( "request"  , &UPGM::requestHookRead );
	registerHook( "transport"  , &UPGM::transportHookWrite );

	registerHook( "code"   , &UPGM::codeHookWrite );
	registerHook( "result" , &UPGM::resultHookWrite );*/
}

UPGM::~UPGM() throw()
{
}

static UPGM::RequestResult asRequestResult(const std::string & str)
{
	if (str == "next_stage" ) { return UPGM::NEXT_STAGE; }
	if (str == "complete"   ) { return UPGM::COMPLETED;  }
	if (str == "fail"       ) { return UPGM::FAIL;       }
	if (str == "sleep"      ) { return UPGM::SLEEP;      }
	throw std::runtime_error("No such payment stage - " + str);
}

std::string evaluateStringFromTemplate(const RequestTemplate & request)
{
	//TODO implement
	return std::string("TEST");
}

void UPGM::registerHook(const std::string & name, HookRead hook)
{
	//_hooksRead[name] = hook;
}

void UPGM::registerHook(const std::string & name, HookWrite hook)
{
	//_hooksWrite[name] = hook;
}

void UPGM::registerHook(Hook * hook)
{
	_hooks[ hook->name() ] = hook;
}

static std::string asString(UPGM::RequestResult result)
{

	switch (result)
	{
		case UPGM::UNDEF      : return "UNDEF";
		case UPGM::NEXT_STAGE : return "NEXT_STAGE";
		case UPGM::COMPLETED  : return "COMPLETED";
		case UPGM::FAIL       : return "FAIL";
		case UPGM::SLEEP      : return "SLEEP";
	}
	throw std::runtime_error("RequestResult value undef");
}

static std::string asString(int i)
{
	enum { size = std::numeric_limits<int>::digits };
	char buff[size+1];
	snprintf(buff, size, "%i", i);
	return buff;
}

std::string UPGM::evaluateConfigValue(const std::string & value)
{
	fprintf(stderr, "evaluate value [%s]\n", value.c_str());
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
	fprintf(stderr, "evaluate param [%s]\n", param.c_str());
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

void UPGM::populate(DataTree & tree, const Config::Section & sec)
{

	for (Config::Section::const_iterator it = sec.begin();
	     it != sec.end();
	     ++it) {
		tree.set(it->first, evaluateConfigValue(it->second));
	}
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

void      UPGM::performStage(int stage,
                            Transport & transport,
                            Parser & parser,
                            const Payment & payment)
{

	registerHook(new TransportHook(&transport));
	registerHook(new ParserHook(&parser));


	_transport = &transport;
	_parser = &parser;
	RequestResult result(UNDEF);

	int currentStage (stage);
	std::string actionName ("main");// = _scheme.getValue("stages", asString(currentStage));

/*	DataTree transportCfg;
	const Config::Section & section = _scheme.section("transport");
	for (Config::Section::const_iterator it = section.begin();
	     it != section.end();
	     ++it)
	{
		transportCfg.set(it->first, it->second);
	}

	transport.configure( transportCfg );*/

	//_actionName = actionName;
	bool noError(true);
	_payment = payment.generateDataTree();
	evalParams(actionName);

	do
	{
		actionName = _sequence.stageName();
		evalParams(actionName);
		// Evaluate parameters
	}
	while ( actionName != _sequence.stageName() );
}


std::string UPGM::customVariableWithName(const std::string & name)
{
	throw VariableUndefException(name);
}

void UPGM::setScheme(const Config & requestScheme)
{
	_scheme = requestScheme;
}

/*std::string UPGM::paymentHookRead(const Path & path)
{
	return _payment(path);
}
std::string UPGM::codeHookRead(const Path & path)
{
	return _code(path);
}
std::string UPGM::answerHookRead(const Path & path)
{
	return _answer(path);
}

std::string UPGM::transportHookRead(const Path & path)
{
	return _net(path);
}

std::string UPGM::dbHookRead(const Path & path)
{
	return _database(path);
}

void UPGM::dbHookWrite(const Path & path, const std::string & value)
{
	_database.set(path, value);
}

void UPGM::stageHookWrite(const Path & path, const std::string & value)
{

	if (!path.empty())
	{
		_stages[ atoi(path[0].c_str()) ] = value;
	}
}

void UPGM::codesHookWrite(const Path & path, const std::string & value)
{
	fprintf(stderr, "parsing file [%s]\n", value.c_str());
	_codes.parseFile(value);
}

void UPGM::actionHookWrite(const Path & path, const std::string & value)
{
	_actionName = value;
}

std::string UPGM::nextHook(const Path & path)
{
	++_stage;
	_actionName = _stages[ _stage ];
	// _actionName = ;;
	return _actionName;
}

void UPGM::requestHookWrite(const Path & path, const std::string & value)
{
	if ( path.empty() )
	{
		_requestTemplate = value;
	} else {
		_requestArg[ path[0] ] = value;
	}
}

std::string UPGM::requestHookRead(const Path & path)
{
	return _requestTemplate.evaluate(_requestArg);
}

void UPGM::transportHookWrite(const Path & path, const std::string & value)
{
	if (!path.empty())
	{
		fprintf(stderr, "performing request [%s]\n", value.c_str());
		*_transport << value;

		std::string answer;
		_net = *_transport >> answer;

		// Parsing answer
		_answer = _parser->parse(answer);
		fprintf(stderr, "result = [%s]\n", answer.c_str());
	}
}

void UPGM::resultHookWrite(const Path & path, const std::string & value)
{
	fprintf(stderr, "result = %s\n", value.c_str());
}

void UPGM::codeHookWrite(const Path & path, const std::string & value)
{
	try {
		const std::string & code = value;
		fprintf(stderr, "code = %s\n", code.c_str());

		DataTree codeData;
		try {
			populate(codeData,_codes.section(code));
		}
		catch ( ... ) {
			populate(codeData,_codes.section("default"));
		}

		_code = codeData;
	}
	catch (const Config::NoSuchValue & e) { fprintf(stderr, "code undef (%s)\n", e.what()); }
}*/

}
