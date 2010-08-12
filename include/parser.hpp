#ifndef PG_PARSER
#define PG_PARSER

#include <string>
#include <upgm/data_tree.hpp>

namespace PG
{

class Parser
{
public:
	Parser();
	virtual ~Parser() throw() = 0;
	virtual DataTree parse(const std::string & data) = 0;
};

}

#endif
