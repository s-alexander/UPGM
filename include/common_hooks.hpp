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
#include <upgm/db.hpp>
#include <upgm/transport.hpp>
#include <upgm/parser.hpp>
#include <upgm/payment.hpp>
#include <upgm/config.hpp>
#include <upgm/request_template.hpp>

namespace PG
{

class PrintHook: public Hook
{
public:
	PrintHook();
	virtual ~PrintHook() throw();
	virtual const char * name() const { return "print"; }

	virtual std::string read(const Path & path);

	virtual void write(const Path & path, const std::string & value);
};

class DbHook: public Hook
{
public:
	DbHook(Db * db);
	virtual ~DbHook() throw();
	virtual const char * name() const { return "db"; }

	virtual std::string read(const Path & path);
	virtual void write(const Path & path, const std::string & value);
private:
	void connect();
	Db::RequestResult _data;
	RequestTemplate _requestTemplate;
	RequestTemplate::VariablesMap _requestArg;
	Db * _db;
};

class TransportHook: public Hook
{
public:
	TransportHook(Transport * transport);
	virtual ~TransportHook() throw();
	virtual const char * name() const { return "transport"; }

	virtual std::string read(const Path & path);
	virtual void write(const Path & path, const std::string & value);
private:
	Transport * _transport;
	std::string _answer;
	DataTree _net;
};

class ParserHook: public Hook
{
public:
	ParserHook(Parser * parser);
	virtual ~ParserHook() throw();
	virtual const char * name() const { return "parser"; }

	virtual std::string read(const Path & path);

	virtual void write(const Path & path, const std::string & value);
private:
	Parser * _parser;
	DataTree _data;
};

class PaymentHook: public Hook
{
public:
	PaymentHook(Payment * pay);
	virtual ~PaymentHook() throw();
	virtual const char * name() const { return "pay"; }

	virtual std::string read(const Path & path);
	virtual void write(const Path & path, const std::string & value);
private:
	DataTree _data;
	Payment * _pay;
};

class CodeHook: public Hook
{
private:
	static void populate(DataTree & tree, const Config::Section & sec);

public:
	CodeHook();
	virtual ~CodeHook() throw();
	virtual const char * name() const { return "code"; }

	virtual std::string read(const Path & path);
	virtual void write(const Path & path, const std::string & value);
private:
	Config _codes;
	DataTree _data;
};

class RequestHook: public Hook
{
public:
	RequestHook();
	virtual ~RequestHook() throw();
	virtual const char * name() const { return "request"; }

	virtual std::string read(const Path & path);
	virtual void write(const Path & path, const std::string & value);
private:
	RequestTemplate _requestTemplate;
	RequestTemplate::VariablesMap _requestArg;
};

class StageHook: public Hook
{
private:
	static size_t asInt(const std::string & str);
public:
	StageHook(PaymentSequence * process);
	virtual ~StageHook() throw();
	virtual const char * name() const { return "stage"; }

	virtual std::string read(const Path & path);
	virtual void write(const Path & path, const std::string & value);
private:
	PaymentSequence * _sequence;
};


}

#endif
