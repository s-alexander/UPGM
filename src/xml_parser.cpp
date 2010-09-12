#include <upgm/xml_parser.hpp>
#include <xml/tinyxml.h>
#include <cstdio>

namespace PG
{

XmlParser::XmlParser()
{
}

XmlParser::~XmlParser() throw()
{
}

static DataTree populateDataFrom(TiXmlElement * element)
{
	DataTree childData;

	// Attributes
	TiXmlAttribute * attr = element->FirstAttribute();
	while( attr )
	{
//		fprintf(stderr, "set %s = %s\n", attr->Name(), attr->Value());
		childData.set( attr->Name(), attr->Value() );
		attr = attr->Next();
	}

	// Subnodes
	TiXmlElement * subElement = element->FirstChildElement();
	while( subElement )
	{
//		fprintf(stderr, "got subelement %s\n",subElement->Value());
		childData.set(subElement->Value(), populateDataFrom(subElement));
		subElement = subElement->NextSiblingElement();
	}

	// Text
	const char * text = 0;
	if (( text = element->GetText() )) {
		childData.set( "", text);
	}

	return childData;
}

DataTree XmlParser::parse(const std::string & data)
{
	TiXmlDocument doc;
	doc.Parse(data.c_str());
	DataTree result;

	// Subnodes
	TiXmlElement * subElement = doc.FirstChildElement();
	while( subElement )
	{
		result.set(subElement->Value(), populateDataFrom(subElement));
		subElement = subElement->NextSiblingElement();
	}
	return result;
}

}
