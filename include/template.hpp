#ifndef PG_TEMPLATE
#define PG_TEMPLATE

#include <string>
#include <map>
#include <vector>

namespace PG
{

class Template
{
public:
	Template(const std::string & request);
	Template();
	void setTemplate(const std::string & tmpl);
	virtual ~Template() throw() { ;; }

	typedef std::vector<std::string> Variables;
	const Variables & variables() { return _variables; }

	typedef std::map<std::string, std::string> VariablesMap;
	std::string evaluate(const VariablesMap & data);

	void set(const std::string & name, const std::string & value);
	std::string evaluate();
private:
	void parse(const std::string & request);
	void saveVar(const std::string & varName, size_t position);
	std::string _request;
	typedef std::map<size_t, std::string> ParamLocation;
	ParamLocation _location;
	Variables _variables;
	VariablesMap _data;
};

}

#endif
