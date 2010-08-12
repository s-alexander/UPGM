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
	typedef const std::string & (UPGM::*HookRead)(const Path & );
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

	const std::string & evaluateConfigValue(const std::string & value);
	void evaluateConfigParam(const std::string & param, const std::string & value);
	virtual std::string customVariableWithName(const std::string & name);
	void populate(DataTree & tree, const Config::Section & sec);

	const std::string & paymentHookRead(const Path & path);
	const std::string & codeHookRead(const Path & path);
	const std::string & answerHookRead(const Path & path);
	const std::string & transportHookRead(const Path & path);

	const std::string & dbHookRead(const Path & path);
	void dbHookWrite(const Path & path, const std::string & value);

	DataTree _payment;
	DataTree _code;
	DataTree _answer;
	DataTree _transport;
	DataTree _database;

	Config   _scheme;
	Config   _codes;
};

}

#endif
