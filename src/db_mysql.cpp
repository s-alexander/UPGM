#include <upgm/db_mysql.hpp>
#include <mysql++/mysql++.h>
#include <upgm/mysql_connection.hpp>

namespace PG
{

DbMysql::DbMysql(MysqlConnection * connection): _connection(connection)
{
}

DbMysql::~DbMysql() throw()
{
	try
	{
		_connection->disconnect();
	}
	catch (mysqlpp::Exception & e)
	{
		throw DbException(std::string("MySQL error ") + e.what());
	}
}

Db::RequestResult DbMysql::performRequest(const std::string & request)
{
	try
	{
		return _connection->performRequest( request );
	}
	catch (mysqlpp::Exception & e)
	{
		throw DbException(std::string("MySQL error ") + e.what());
	}
}

std::string DbMysql::escape(const std::string & str)
{
	try
	{
		return _connection->escape(str);
	}
	catch (mysqlpp::Exception & e)
	{
		throw DbException(std::string("MySQL error ") + e.what());
	}
}

void DbMysql::connect(const std::string & host, unsigned int port, const std::string & dbname, const std::string & username, const std::string & password)
{
	try
	{
		_connection->connect(host.c_str(), port, dbname.c_str(), username.c_str(), password.c_str());
	}
	catch (mysqlpp::Exception & e)
	{
		throw DbException(std::string("MySQL error ") + e.what());
	}
}

}
