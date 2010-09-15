#include <cstdio>
#include <fstream>
#include <sstream>
#include <upgm/config.hpp>
#include <upgm/path.hpp>
#include <upgm/data_tree.hpp>
#include <upgm/payment.hpp>
#include <upgm/request_template.hpp>
#include <test/test_transport.hpp>
#include <upgm/http_transport.hpp>
#include <upgm/db_mysql.hpp>
#include <upgm/xml_parser.hpp>
#include <upgm/shared_mysql_connection.hpp>

#include <upgm/upgm.hpp>

#include "../modules/webmoney/include/webmoney.hpp"

int main(int argc, char ** argv)
{
	try
	{
	PG::WebMoney upgm;

	{
		if (argc < 4)
		{
			fprintf(stderr, "Usage: %s scheme.cfg config.cfg codes.cfg\n", argv[0]);
			return -1;
		}

		PG::Config scheme;
		scheme.parseFile(argv[1]);

		PG::Config config;
		config.parseFile(argv[2]);

		PG::Config codes;
		codes.parseFile(argv[3]);

		upgm.setScheme(scheme);
		upgm.setCodes(codes);
		upgm.setConfig(config);
	}

	SPay pay;
	strncpy(pay.data, "TESTER$R477366332869$79111128076$000000$19700101", SIZE_DATA);
	strncpy(pay.bill_num, "1234567890", SIZE_BILL_NUM);
	strncpy(pay.stamp, "2010-09-16 00:19:12", SIZE_STAMP);
	pay.summ=10.0;

	PG::Payment payment(pay);

	std::queue<std::string> answers;
	answers.push("<?xml version=\"1.0\">\n<code>0</code><errmsg>all ok</errmsg>");
	answers.push("<?xml version=\"1.0\">\n<code>1</code><errmsg>unsupported sum</errmsg>");
	//TEST::TestTransport transport(answers);
	PG::SharedMysqlConnection mysqlConnection;
	upgm.performStage(&mysqlConnection, payment);
	}
	catch (std::exception & e)
	{
		fprintf(stderr, "Error: %s\n", e.what());
	}
	catch (...)
	{
		fprintf(stderr, "Unknown error occured\n");
	}

	exit(0);
}
