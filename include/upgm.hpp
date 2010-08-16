#ifndef PG_UPGM
#define PG_UPGM

#include <stdexcept>
#include <string>
#include <upgm/data_tree.hpp>
#include <upgm/config.hpp>
#include <upgm/transport.hpp>
#include <upgm/parser.hpp>
#include <upgm/path.hpp>

namespace PG
{

class RequestTemplate;
class Payment;

class NotAVariableException: std::runtime_error
{
public:
	NotAVariableException(const std::string & varName):
		std::runtime_error(varName + " is not a valid variable name"), _name(varName)
	{
	};

	~NotAVariableException() throw() { ;; }
	const std::string & name() { return _name; }
private:
	std::string _name;
};

class HookUndefException: std::runtime_error
{
public:
	HookUndefException(const std::string & varName):
		std::runtime_error("Variable " + varName + " not defined"), _name(varName)
	{
	};

	~HookUndefException() throw() { ;; }
	const std::string & name() { return _name; }
private:
	std::string _name;
};
class VariableUndefException: std::runtime_error
{
public:
	VariableUndefException(const std::string & varName):
		std::runtime_error("Variable " + varName + " not defined"), _name(varName)
	{
	};

	~VariableUndefException() throw() { ;; }
	const std::string & name() { return _name; }
private:
	std::string _name;
};

class UPGM
{
public:
	enum RequestResult { UNDEF, NEXT_STAGE, COMPLETED, FAIL, SLEEP };
	UPGM();
	virtual ~UPGM() throw();
	void     performStage(int stage,
	                      Transport & transport,
	                      Parser & parser,
	                      const Payment & payment);

	void setScheme(const Config & requestScheme);
protected:
	typedef std::string (UPGM::*HookRead)(const Path & );
	typedef void (UPGM::*HookWrite)(const Path & , const std::string &);
	void registerHook(const std::string & name, HookRead hook);
	void registerHook(const std::string & name, HookWrite hook);
private:

	void request(const std::string & actionName, Transport & transport, Parser & parser);
	void evalParams(const std::string & sectionName);
	typedef std::map<std::string, HookRead>  HooksRead;
	typedef std::map<std::string, HookWrite> HooksWrite;
	HooksRead  _hooksRead;
	HooksWrite _hooksWrite;

	std::string evaluateConfigValue(const std::string & value);
	void evaluateConfigParam(const std::string & param, const std::string & value);
	virtual std::string customVariableWithName(const std::string & name);
	void populate(DataTree & tree, const Config::Section & sec);

	std::string paymentHookRead(const Path & path);
	std::string codeHookRead(const Path & path);
	std::string answerHookRead(const Path & path);
	std::string transportHookRead(const Path & path);
	std::string nextHook(const Path & path);
	std::string requestHookRead(const Path & path);

	std::string dbHookRead(const Path & path);
	void dbHookWrite(const Path & path, const std::string & value);

	void stageHookWrite(const Path & path, const std::string & value);
	void codesHookWrite(const Path & path, const std::string & value);
	void actionHookWrite(const Path & path, const std::string & value);
	void requestHookWrite(const Path & path, const std::string & value);

	void transportHookWrite(const Path & path, const std::string & value);
	void codeHookWrite(const Path & path, const std::string & value);
	void resultHookWrite(const Path & path, const std::string & value);

	DataTree _payment;
	DataTree _code;
	DataTree _answer;
	DataTree _net;
	DataTree _database;

	Config   _scheme;
	Config   _codes;
	unsigned int _stage;
	std::map<int, std::string> _stages;


	RequestTemplate _requestTemplate;
	std::map<std::string, std::string> _requestArg;

	Transport * _transport;
	Parser * _parser;

	std::string _actionName;
};

}

#endif
