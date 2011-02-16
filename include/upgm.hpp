#ifndef PG_UPGM
#define PG_UPGM

#include <stdexcept>
#include <string>
#include <fstream>
#include <memory>

#include <upgm/data_tree.hpp>
#include <upgm/config.hpp>
#include <upgm/transport.hpp>
#include <upgm/db.hpp>
#include <upgm/parser.hpp>
#include <upgm/path.hpp>
#include <upgm/payment_sequence.hpp>
#include <upgm/hooks_map.hpp>
#include <upgm/log.hpp>

namespace PG
{

class RequestTemplate;
class Payment;
class Hook;
class DbConnection;

class NotAVariableException: public std::runtime_error
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

class HookUndefException: public std::runtime_error
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
class VariableUndefException: public std::runtime_error
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
	void performStage(DbConnection * dbConnection, Payment & payment);

	void setScheme(const Config & requestScheme);
	void setConfig(const Config & config);
	void setCodes(const Config & codes);
	void setLog(const std::string & path);
protected:
	void registerHook(HookPtr hook);
	virtual void registerUserHooks(Transport & transport, Parser & parser, Payment & payment) { ;; }
private:
	virtual std::auto_ptr<Transport> getTransport(const std::string & name);
	virtual std::auto_ptr<Parser> getParser(const std::string & name);
	virtual std::auto_ptr<Db> getDb(const std::string & name, DbConnection * dbConnection);

	std::string evaluateConfigValue(const ConfigValue & cvalue);
	void evaluateConfigParam(const std::string & param, const std::string & value);
	void evalParams(const std::string & sectionName);

	Hooks _hooks;

	Config   _scheme;
	Config   _config;
	Config   _codes;
	PaymentSequence _sequence;
	std::string _logPath;
	Log _log;
};

}

#endif
