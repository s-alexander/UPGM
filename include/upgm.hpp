#ifndef PG_UPGM
#define PG_UPGM

#include <stdexcept>
#include <string>
#include <memory>

#include <upgm/data_tree.hpp>
#include <upgm/config.hpp>
#include <upgm/transport.hpp>
#include <upgm/parser.hpp>
#include <upgm/path.hpp>
#include <upgm/payment_sequence.hpp>

namespace PG
{

class RequestTemplate;
class Payment;
class Hook;

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
	typedef std::auto_ptr<Hook> HookPtr;
	UPGM();
	virtual ~UPGM() throw();
	void     performStage(int stage,
	                      Transport & transport,
	                      Parser & parser,
	                      Payment & payment);

	void setScheme(const Config & requestScheme);
protected:
	void registerHook(HookPtr hook);
	virtual void registerUserHooks(Transport & transport, Parser & parser, Payment & payment) { ;; }
private:

	std::string evaluateConfigValue(const std::string & value);
	void evaluateConfigParam(const std::string & param, const std::string & value);
	void evalParams(const std::string & sectionName);

	typedef std::map<std::string, Hook *> Hooks;
	Hooks _hooks;

	Config   _scheme;
	PaymentSequence _sequence;
};

}

#endif
