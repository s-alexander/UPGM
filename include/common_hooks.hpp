#ifndef PG_COMMON_HOOKS
#define PG_COMMON_HOOKS

#include <string>
#include <vector>
#include <stdexcept>
#include <cstdlib>

#include <upgm/hook.hpp>
#include <upgm/transport.hpp>
#include <upgm/payment_sequence.hpp>
#include <upgm/data_tree.hpp>
#include <upgm/transport.hpp>
#include <upgm/parser.hpp>
#include <upgm/payment.hpp>
#include <upgm/config.hpp>

namespace PG
{

class PrintHook: public Hook
{
public:
	PrintHook() { ;; }
	virtual ~PrintHook() throw() { ;; }
	virtual const char * name() const { return "print"; }

	virtual std::string read(const Path & path) {
		throw InvalidArgumentException();
	}

	virtual void write(const Path & path, const std::string & value) {
		if (path.empty()) {
			fprintf(stderr, " * %s\n", value.c_str());
		}
		else {
			throw InvalidArgumentException();
		}
	}
};

class TransportHook: public Hook
{
public:
	TransportHook(Transport * transport): _transport(transport) { ;; }
	virtual ~TransportHook() throw() { ;; }
	virtual const char * name() const { return "transport"; }

	virtual std::string read(const Path & path) {
		if (path.size() == 1)
		{
			if (path[0] == "read") {
				return _answer;
			}
		}
		throw InvalidArgumentException();
	}
	virtual void write(const Path & path, const std::string & value) {
		if (path.empty()) {
			throw InvalidArgumentException();
		} else if (path.size() > 1 && path[0] == "config") {
			_config.set(Path(path.begin()+1, path.end()), value);
		} else if (path.size() == 1 && path[0] == "write") {

			_transport->configure( _config );

			fprintf(stderr, "performing request [%s]\n", value.c_str());
			*_transport << value;

			_net = *_transport >> _answer;

			// Parsing answer
			//_answer = _parser->parse(answer);
			fprintf(stderr, "result = [%s]\n", _answer.c_str());
		}
		else {
			throw InvalidArgumentException();
		}
	}
private:
	Transport * _transport;
	std::string _answer;
	DataTree _config;
	DataTree _net;
};

class ParserHook: public Hook
{
public:
	ParserHook(Parser * parser): _parser(parser) { ;; }
	virtual ~ParserHook() throw() { ;; }
	virtual const char * name() const { return "parser"; }

	virtual std::string read(const Path & path) {
		if (path.size() > 1)
		{
			if (path[0] == "get") {
				return _data(Path(path.begin()+1, path.end()));
			}
		}
		throw InvalidArgumentException();
	}

	virtual void write(const Path & path, const std::string & value) {
		if (path.empty()) {
			throw InvalidArgumentException();
		} else if (path.size() == 1 && path[0] == "parse") {
			_data = _parser->parse(value);
		}
		else {
			throw InvalidArgumentException();
		}
	}
private:
	Parser * _parser;
	DataTree _data;
};

class PaymentHook: public Hook
{
public:
	PaymentHook(Payment * pay):_pay(pay) { _data = pay->generateDataTree(); }
	virtual ~PaymentHook() throw() { ;; }
	virtual const char * name() const { return "pay"; }

	virtual std::string read(const Path & path)
	{
		return _data(path);
	}
	virtual void write(const Path & path, const std::string & value)
	{
		fprintf(stderr, "value = %s path[0]=%s\n",value.c_str(), path[0].c_str());
		if (path.size() == 1)
		{
			if (path[0] == "result")
			{
				if (value == "completed") { _pay->completed(); }
				else if (value == "sleep") { _pay->sleep(); }
				else if (value == "failed") { _pay->failed(); }
				else { throw InvalidArgumentException(); }
				return;
			}
		}
		throw InvalidArgumentException();
	}
private:
	DataTree _data;
	Payment * _pay;
};

class CodeHook: public Hook
{
private:
	static void populate(DataTree & tree, const Config::Section & sec)
	{

		for (Config::Section::const_iterator it = sec.begin();
		     it != sec.end();
		     ++it) {
			tree.set(it->first, it->second);
		}
	}

public:
	CodeHook() { ;; }
	virtual ~CodeHook() throw() { ;; }
	virtual const char * name() const { return "code"; }

	virtual std::string read(const Path & path) {
		return _data(path);
	}
	virtual void write(const Path & path, const std::string & value)
	{
		if (path.size()==1 && path[0] == "def")
		{
			_codes.parse(value);
		}
		else if (path.empty())
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

				_data = codeData;
			}
			catch (const Config::NoSuchValue & e) { fprintf(stderr, "code undef (%s)\n", e.what()); }
		}
		else { throw InvalidArgumentException(); }
	}
private:
	Config _codes;
	DataTree _data;
};

class RequestHook: public Hook
{
public:
	RequestHook() { ;; }
	virtual ~RequestHook() throw() { ;; }
	virtual const char * name() const { return "request"; }

	virtual std::string read(const Path & path)
	{
		if (path.empty())
		{
			return _requestTemplate.evaluate(_requestArg);
		}
		throw InvalidArgumentException();
	}
	virtual void write(const Path & path, const std::string & value)
	{
		if ( path.empty() )
		{
			_requestTemplate = value;
		} else if (path.size() == 1) {
			_requestArg[ path[0] ] = value;
		} else { throw InvalidArgumentException(); }
	}
private:
	RequestTemplate _requestTemplate;
	RequestTemplate::VariablesMap _requestArg;
};

class StageHook: public Hook
{
private:
	static size_t asInt(const std::string & str)
	{
		return atoi(str.c_str());
	}
public:
	StageHook(PaymentSequence * process): _sequence(process) { ;; }
	virtual ~StageHook() throw() { ;; }
	virtual const char * name() const { return "stage"; }

	virtual std::string read(const Path & path)
	{
		if (path.empty())
		{
			return _sequence->stageName();
		} else if (path[0] == "next") {
			return _sequence->stageName( _sequence->currentStage() + 1 );
		}
		throw InvalidArgumentException();
	}
	virtual void write(const Path & path, const std::string & value)
	{
		if (path.empty()) {
			_sequence->setStage(value);
		}
		else if (path.size() == 1) {
			_sequence->addStage(asInt(path[0]), value);
		}
	}
private:
	PaymentSequence * _sequence;
};


}

#endif
