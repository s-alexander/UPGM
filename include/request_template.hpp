#ifndef PG_REQUEST_TEMPLATE
#define PG_REQUEST_TEMPLATE

#include <string>
#include <map>
#include <vector>

namespace PG
{

class RequestTemplate
{
public:
	RequestTemplate(const std::string & request);
	~RequestTemplate() throw() { ;; }

	typedef std::vector<std::string> Variables;
	const Variables & variables() { return _variables; }

	typedef std::map<std::string, std::string> VariablesMap;
	std::string evaluate(const VariablesMap & data);
private:
	void saveVar(const std::string & varName, size_t position);
	std::string _request;
	typedef std::map<size_t, std::string> ParamLocation;
	ParamLocation _location;
	Variables _variables;
};

}

#endif
