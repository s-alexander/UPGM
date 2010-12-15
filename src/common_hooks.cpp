#include <upgm/as_string.hpp>
#include <upgm/common_hooks.hpp>
#include <stdexcept>
#include <algorithm>
#include <cstdio>
#include <fstream>

namespace
{

void pathMustBeLesserThan(size_t size, const PG::Path & path)
{
	if (path.size() >= size) {
		throw PG::InvalidArgumentException("path is too big");
	}
}
void pathMustBeGreaterThan(size_t size, const PG::Path & path)
{
	if (path.size() <= size) {
		throw PG::InvalidArgumentException("path is too short");
	}
}

void pathMustBeEqualTo(size_t size, const PG::Path & path)
{
	if (path.size() != size) {
		throw PG::InvalidArgumentException(std::string("path must be equal to ") + PG::asString(size));
	}
}

void pathMustBeEmpty(const PG::Path & path)
{
	if (!path.empty()) {
		throw PG::InvalidArgumentException("path must be empty");
	}
}

}

namespace PG
{


VarHook::VarHook() { ;; }
VarHook::~VarHook() throw() { ;; }

std::string VarHook::read(const Path & path)
{
	return _data(path);
}

void VarHook::write(const Path & path, const std::string & value)
{
	_data.set(path, value);
}

MainConfigHook::MainConfigHook() { ;; }
MainConfigHook::~MainConfigHook() throw() { ;; }

std::string MainConfigHook::read(const Path & path)
{
	pathMustBeEqualTo(1, path);
//	fprintf(stderr, "REQUEST [%s]\n", path[0].c_str());
	return param(path[0]);
}

void MainConfigHook::write(const Path & path, const std::string & value)
{
}

ConfigHook::ConfigHook(Hooks * hooks): _hooks(hooks) { ;; }
ConfigHook::~ConfigHook() throw() { ;; }

std::string ConfigHook::read(const Path & path)
{
	pathMustBeEqualTo(2, path);

	const std::string & hookName (path[0]);
	Hooks::const_iterator it = _hooks->find(hookName);
	if (it != _hooks->end())
	{
		Hook * hook = it->second;
		const std::string & paramName(path[1]);
		return hook->param(paramName);
	}
	else
	{
		throw std::runtime_error(std::string("Can't configure hook ") + hookName + ": invalid hook name");
	}

}

void ConfigHook::write(const Path & path, const std::string & value)
{
	pathMustBeEqualTo(2, path);

	const std::string & hookName (path[0]);
	Hooks::const_iterator it = _hooks->find(hookName);
	if (it != _hooks->end())
	{
		Hook * hook = it->second;
		const std::string & paramName(path[1]);
		hook->setParam(paramName, value);
	}
	else
	{
		throw std::runtime_error(std::string("Can't configure hook ") + hookName + ": invalid hook name");
	}
}

PrintHook::PrintHook() { ;; }
PrintHook::~PrintHook() throw() { ;; }

std::string PrintHook::read(const Path & path) {
	throw InvalidArgumentException("print hook invalid usage");
}

void PrintHook::write(const Path & path, const std::string & value) {
	pathMustBeEmpty(path);
	fprintf(stderr, " * [%s]\n", value.c_str());
}

DbHook::DbHook(Db * db): _db(db) { ;; }
DbHook::~DbHook() throw() { ;; }

std::string DbHook::read(const Path & path) {
	if (path.size() == 2 && path[0] == "field") {
		const std::string & name = path[1];
//		fprintf(stderr, "field %s is [%s]\n", name.c_str(), _data.at(0)[name].c_str());
		return _data.at(0)[name];
	}
	else if (path.size() == 1 && path[0] == "sql") {
		return _requestTemplate.evaluate(_requestArg);
	}

	throw InvalidArgumentException("db hook invalid read");
}
void DbHook::write(const Path & path, const std::string & value) {
	if (path.size() == 1 && path[0] == "execute")
	{
		connect();
		_data = _db->performRequest(value);
	}
	else if ( path.size() == 1 && path[0] == "sql" )
	{
		_requestArg.clear();
		_requestTemplate = value;
	} else if (path.size() == 2 && path[0] == "sql" ) {
		connect();
		_requestArg[ path[1] ] = _db->escape(value);
	} else if (path.size() == 3 && path[0] == "sql" && path[2] == "raw") {
		_requestArg[ path[1] ] = value;
	}
	else {
		throw InvalidArgumentException("db hook invalid write");
	}
}
void DbHook::connect()
{
	_db->connect(param("host"), atoi(param("port").c_str()), param("database"), param("username"), param("password"));
}


TransportHook::TransportHook(Transport * transport): _transport(transport) { ;; }
TransportHook::~TransportHook() throw() { ;; }

std::string TransportHook::read(const Path & path) {
	pathMustBeEqualTo(1, path);

	if (path[0] == "read") {
		return _answer;
	}

	throw InvalidArgumentException("transport hook invalid read");
}
void TransportHook::write(const Path & path, const std::string & value) {
	pathMustBeEqualTo(1, path);
	if (path[0] == "write") {
		Params::const_iterator it  = params().begin();
		const Params::const_iterator end  = params().end();
		DataTree config;
		for (; it != end; ++it)
		{
			config.set(it->first, it->second);
		}

		_transport->configure( config );

		*_transport << value;

		_net = *_transport >> _answer;

		// Parsing answer
		//_answer = _parser->parse(answer);
	}
	else {
		throw InvalidArgumentException(std::string("transport hook invalid operation - ") + path[0]);
	}
}

ParserHook::ParserHook(Parser * parser): _parser(parser) { ;; }
ParserHook::~ParserHook() throw() { ;; }

std::string ParserHook::read(const Path & path) {
	pathMustBeGreaterThan(1, path);

	if (path[0] == "get") {
		return _data(Path(path.begin()+1, path.end()));
	}

	throw InvalidArgumentException("parser hook invalid read");
}

void ParserHook::write(const Path & path, const std::string & value) {
	pathMustBeEqualTo(1, path);
	if (path[0] == "parse") {
		_data = _parser->parse(value);
	}
	else {
		throw InvalidArgumentException("parser hook invalid ");
	}
}

PaymentHook::PaymentHook(Payment * pay):_pay(pay), _dataParsed(false)
{
	_data = pay->generateDataTree();
}
PaymentHook::~PaymentHook() throw() { ;; }

static std::vector<std::string> arrayFromStringWithSeparators(const std::string & data, const std::string & separator)
{
	std::vector<std::string> result;
	typedef std::string::const_iterator iter;
	size_t pos(0);
	const size_t len = data.size();
	const size_t sepLen = separator.size();
	while (pos != std::string::npos && pos < len)
	{
		const size_t next = data.find(separator, pos);
		if (next != std::string::npos) {
			result.push_back(std::string(data.begin()+pos, data.begin()+next));
			pos = next+sepLen;
		} else {
			result.push_back(std::string(data.begin()+pos, data.end()));
			break;
		}
	}
	return result;
}

std::string PaymentHook::read(const Path & path)
{
	if (path.size() == 1)
	{
		return _data(path);
	}
	else if (path.size() == 2 && "data" == path[0])
	{
		if (!_dataParsed)
		{
//			fprintf(stderr, "data.data = [%s]\n", _data("data").c_str());
			_payData = arrayFromStringWithSeparators(_data("data"), param("data_separator"));
			_dataParsed = true;
		}
		try
		{
			return _payData.at(atoi(path[1].c_str()));
		}
		catch( std::out_of_range & e)
		{
			throw std::runtime_error(std::string("Can't access payment data index ") + asString(atoi(path[1].c_str())));
		}
	}
	throw InvalidArgumentException("payment data hook invalid read");
}
void PaymentHook::write(const Path & path, const std::string & value)
{
//	fprintf(stderr, "value = %s path[0]=%s\n",value.c_str(), path[0].c_str());
	pathMustBeEqualTo(1, path);
	if (path[0] == "result")
	{
		if (value == "completed") { _pay->completed(); }
		else if (value == "sleep") { _pay->sleep(60*5); }
		else if (value == "failed") { _pay->failed(); }
		else { throw InvalidArgumentException(std::string("invalid pay status - ") + value); }
		return;
	}
	else if (path[0] == "sleep")
	{
		_pay->sleep(atoi(value.c_str()));
		return;
	}
	else if (path[0] == "error")
	{
		_pay->error(value);
		return;
	}
	throw InvalidArgumentException(std::string("invalid pay operation - ") + path[0]);
}

void CodeHook::populate(DataTree & tree, const Config::Section & sec)
{
	for (Config::Section::const_iterator it = sec.begin();
	     it != sec.end();
	     ++it) {
		tree.set(it->first, it->second);
	}
}

CodeHook::CodeHook(const Config & codes):_codes(codes) { ;; }
CodeHook::~CodeHook() throw() { ;; }

std::string CodeHook::read(const Path & path) {
	return _data(path);
}

void CodeHook::write(const Path & path, const std::string & value)
{
	pathMustBeEmpty(path);
	try {
		const std::string & code = value;
//			fprintf(stderr, "code = %s\n", code.c_str());

		DataTree codeData;
		try {
			populate(codeData,_codes.section(code));
		}
		catch ( ... ) {
			populate(codeData,_codes.section("default"));
			const std::string messageStr(std::string("Unknown payment error (code ") + code + ")");
			codeData.set("message", messageStr);
		}

		_data = codeData;
	}
	catch (const Config::NoSuchValue & e) {
		throw std::runtime_error(std::string("Unknow error code - ") +
					 asString(atoi(value.c_str())) +
					 "; Define it in codes.cfg");
	}
}

RequestHook::RequestHook() { ;; }
RequestHook::~RequestHook() throw() { ;; }

std::string RequestHook::read(const Path & path)
{
	pathMustBeEmpty(path);
	return _requestTemplate.evaluate(_requestArg);
}
void RequestHook::write(const Path & path, const std::string & value)
{
	pathMustBeLesserThan(2, path);
	if ( path.empty() )
	{
		_requestArg.clear();
		_requestTemplate = value;
	} else if (path.size() == 1) {
		_requestArg[ path[0] ] = value;
	}
}


size_t StageHook::asInt(const std::string & str)
{
	return atoi(str.c_str());
}

StageHook::StageHook(PaymentSequence * process): _sequence(process) { ;; }
StageHook::~StageHook() throw() { ;; }

std::string StageHook::read(const Path & path)
{
	if (path.empty())
	{
		return _sequence->stageName();
	} else if (path[0] == "next") {
		return _sequence->stageName( _sequence->currentStage() + 1 );
	}
	throw InvalidArgumentException(std::string("invalid stage read operation - ") + path[0]);
}
void StageHook::write(const Path & path, const std::string & value)
{
	if (path.empty()) {
		_sequence->setStage(value);
	}
	else if (path.size() == 2 && path[0] == "id") {
		_sequence->addStage(asInt(path[1]), value);
	}
	else if (path.size() == 1 && path[0] == "id") {
		_sequence->setStage(asInt(value));
	}
}

}
