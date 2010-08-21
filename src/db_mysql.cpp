#include <upgm/db_mysql.hpp>
#include <upgm/mysql_connection.hpp>

namespace PG
{

DbMysql::DbMysql(MysqlConnection * connection): _connection(connection)
{
}

DbMysql::~DbMysql() throw()
{
	_connection->disconnect();
}

Db::RequestResult DbMysql::performRequest(const std::string & request)
{
	return _connection->performRequest( request );
}

std::string DbMysql::escape(const std::string & str)
{
	return _connection->escape(str);
}

void DbMysql::connect(const std::string & host, unsigned int port, const std::string & dbname, const std::string & username, const std::string & password)
{
	_connection->connect(host.c_str(), port, dbname.c_str(), username.c_str(), password.c_str());
}

}
