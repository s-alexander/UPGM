#ifndef PG_HOOK
#define PG_HOOK

#include <string>
#include <upgm/config.hpp>
#include <upgm/path.hpp>

namespace PG
{

class Hook
{
public:
	Hook() { ;; }
	void configure(const Config::Section & config) {
		for (Config::Section::const_iterator it = config.begin();
		     it != config.end();
		     ++it)
		{
			_params[it->first] = it->second;
		}
	}
	virtual ~Hook() throw() { ;; }
	virtual const char * name() const = 0;

	virtual std::string read(const Path & path) = 0;
	virtual void write(const Path & path, const std::string & value) = 0;
protected:
	typedef std::map<std::string, std::string> Params;
	const Params & params() const { return _params; }
	const std::string & param(const std::string & name) const {
		Params::const_iterator it = _params.find(name);
		if (it != _params.end()) {
			return it->second;
		}
		throw std::runtime_error("param " + name + " undeclared");
	}
private:
	Params _params;
};

}

#endif
