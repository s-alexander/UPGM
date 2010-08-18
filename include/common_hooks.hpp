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

namespace PG
{

class PrintHook: public Hook
{
public:
	PrintHook() { ;; }
	virtual ~PrintHook() throw() { ;; }
	virtual const char * name() { return "print"; }

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
	virtual const char * name() { return "transport"; }

	virtual std::string read(const Path & path) {
		if (path.size() == 1)
		{
			if (path[0] == "answer") {
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
		} else if (path.size() == 1 && path[0] == "request") {

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
	virtual const char * name() { return "parser"; }

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
	PaymentHook() { ;; }
	virtual ~PaymentHook() throw() { ;; }
	virtual const char * name() { return "pay"; }

	virtual std::string read(const Path & path);
	virtual void write(const Path & path, const std::string & value);
};

class CodeHook: public Hook
{
public:
	CodeHook() { ;; }
	virtual ~CodeHook() throw() { ;; }
	virtual const char * name() { return "code"; }

	virtual std::string read(const Path & path);
	virtual void write(const Path & path, const std::string & value);
};

class AnswerHook: public Hook
{
public:
	AnswerHook() { ;; }
	virtual ~AnswerHook() throw() { ;; }
	virtual const char * name() { return "answer"; }

	virtual std::string read(const Path & path);
	virtual void write(const Path & path, const std::string & value);
};

class RequestHook: public Hook
{
public:
	RequestHook() { ;; }
	virtual ~RequestHook() throw() { ;; }
	virtual const char * name() { return "request"; }

	virtual std::string read(const Path & path);
	virtual void write(const Path & path, const std::string & value);
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
	virtual const char * name() { return "stage"; }

	virtual std::string read(const Path & path)
	{
		if (path.empty())
		{
			return _sequence->stageName();
		} else if (path[0] == "next") {
			_sequence->nextStage();
			return _sequence->stageName();
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
