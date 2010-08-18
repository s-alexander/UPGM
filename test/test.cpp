#include <cstdio>
#include <fstream>
#include <sstream>
#include <upgm/config.hpp>
#include <upgm/path.hpp>
#include <upgm/data_tree.hpp>
#include <upgm/payment.hpp>
#include <upgm/request_template.hpp>
#include <upgm/xml_parser.hpp>
#include <test/test_transport.hpp>
#include <upgm/http_transport.hpp>

#include <upgm/upgm.hpp>

int main(int argc, char ** argv)
{
	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s scheme.cfg\n", argv[0]);
		return -1;
	}


	PG::Config config;
	config.parseFile(argv[1]);

	PG::UPGM upgm;
	upgm.setScheme(config);

	PG::Payment payment;
	std::queue<std::string> answers;
	answers.push("<?xml version=\"1.0\">\n<code>0</code><errmsg>all ok</errmsg>");
	answers.push("<?xml version=\"1.0\">\n<code>0</code><errmsg>unsupported sum</errmsg>");
	TEST::TestTransport transport(answers);
	//PG::HTTPTransport transport;
	PG::XmlParser parser;
	upgm.performStage(0, transport, parser, payment);

	return 0;
}
