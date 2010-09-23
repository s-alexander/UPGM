#ifndef PG_HOOKS_MAP
#define PG_HOOKS_MAP
#include <map>
#include <string>


namespace PG
{

class Hook;
typedef std::map<std::string, Hook *> Hooks;

}

#endif
