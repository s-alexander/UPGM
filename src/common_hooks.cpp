#include <upgm/common_hooks.hpp>
#include <stdexcept>
#include <algorithm>
#include <cstdio>
#include <fstream>

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
	if (path.empty())
	{
		throw InvalidArgumentException();
	}
//	fprintf(stderr, "REQUEST [%s]\n", path[0].c_str());
	return param(path[0]);
}

void MainConfigHook::write(const Path & path, const std::string & value)
{
}

PrintHook::PrintHook() { ;; }
PrintHook::~PrintHook() throw() { ;; }

std::string PrintHook::read(const Path & path) {
	throw InvalidArgumentException();
}

void PrintHook::write(const Path & path, const std::string & value) {
	if (path.empty()) {
		fprintf(stderr, " * [%s]\n", value.c_str());
	}
	else {
		throw InvalidArgumentException();
	}
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

	throw InvalidArgumentException();
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
		throw InvalidArgumentException();
	}
}
void DbHook::connect()
{
	_db->connect(param("host"), atoi(param("port").c_str()), param("database"), param("username"), param("password"));
}


TransportHook::TransportHook(Transport * transport): _transport(transport) { ;; }
TransportHook::~TransportHook() throw() { ;; }

std::string TransportHook::read(const Path & path) {
	if (path.size() == 1)
	{
		if (path[0] == "read") {
			return _answer;
		}
	}
	throw InvalidArgumentException();
}
void TransportHook::write(const Path & path, const std::string & value) {
	if (path.empty()) {
		throw InvalidArgumentException();
	} else if (path.size() == 1 && path[0] == "write") {
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
		throw InvalidArgumentException();
	}
}

ParserHook::ParserHook(Parser * parser): _parser(parser) { ;; }
ParserHook::~ParserHook() throw() { ;; }

std::string ParserHook::read(const Path & path) {
	if (path.size() > 1)
	{
		if (path[0] == "get") {
			return _data(Path(path.begin()+1, path.end()));
		}
	}
	throw InvalidArgumentException();
}

void ParserHook::write(const Path & path, const std::string & value) {
	if (path.empty()) {
		throw InvalidArgumentException();
	} else if (path.size() == 1 && path[0] == "parse") {
		_data = _parser->parse(value);
	}
	else {
		throw InvalidArgumentException();
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
		return _payData.at(atoi(path[1].c_str()));
	}
	throw InvalidArgumentException();
}
void PaymentHook::write(const Path & path, const std::string & value)
{
//	fprintf(stderr, "value = %s path[0]=%s\n",value.c_str(), path[0].c_str());
	if (path.size() == 1)
	{
		if (path[0] == "result")
		{
			if (value == "completed") { _pay->completed(); }
			else if (value == "sleep") { _pay->sleep(60*5); }
			else if (value == "failed") { _pay->failed(); }
			else { throw InvalidArgumentException(); }
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
	}
	throw InvalidArgumentException();
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
	if (path.empty())
	{
		try {
			const std::string & code = value;
//			fprintf(stderr, "code = %s\n", code.c_str());

			DataTree codeData;
			try {
				populate(codeData,_codes.section(code));
			}
			catch ( ... ) {
				populate(codeData,_codes.section("default"));
			}

			_data = codeData;
		}
		catch (const Config::NoSuchValue & e) { fprintf(stderr, "code undef (%s)\n", e.what()); }
	}
	else { throw InvalidArgumentException(); }
}

RequestHook::RequestHook() { ;; }
RequestHook::~RequestHook() throw() { ;; }

std::string RequestHook::read(const Path & path)
{
	if (path.empty())
	{
		return _requestTemplate.evaluate(_requestArg);
	}
	throw InvalidArgumentException();
}
void RequestHook::write(const Path & path, const std::string & value)
{
	if ( path.empty() )
	{
		_requestArg.clear();
		_requestTemplate = value;
	} else if (path.size() == 1) {
		_requestArg[ path[0] ] = value;
	} else { throw InvalidArgumentException(); }
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
	throw InvalidArgumentException();
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
