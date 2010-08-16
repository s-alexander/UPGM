#ifndef PG_HOOK
#define PG_HOOK

#include <string>

namespace PG
{

class Hook
{
public:
	Hook() { ;; }
	virtual Hook() throw() { ;; }

	virtual std::string read(const Path & path) = 0;
	virtual void write(const Path & path, const std::string & value) = 0;
}

}

#endif
