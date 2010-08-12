#ifndef PG_XML_PARSER
#define PG_XML_PARSER

#include <upgm/parser.hpp>

namespace PG
{

class XmlParser: public Parser
{
public:
	XmlParser();
	virtual ~XmlParser() throw();
	virtual DataTree parse(const std::string & data);
};

}

#endif
