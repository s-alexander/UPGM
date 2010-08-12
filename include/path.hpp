#ifndef PG_PATH
#define PG_PATH

#include <vector>
#include <string>

namespace PG
{

typedef std::vector<std::string> Path;
Path pathFromString(const std::string & string);

}

#endif
