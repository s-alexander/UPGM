#ifndef PG_DB_MYSQL
#define PG_DB_MYSQL

#include <string>
#include <memory>
#include <semaphore.h>

#include <upgm/data_tree.hpp>
#include <upgm/db.hpp>

namespace mysqlpp
{
	class Connection;
}

namespace PG
{

class MysqlConnection;
class DbMysql: public Db
{
public:
	DbMysql(MysqlConnection * connection);
	virtual ~DbMysql() throw();
	virtual RequestResult performRequest(const std::string & request);

	virtual std::string escape(const std::string & str);

	virtual void connect(const std::string & host,
	                     unsigned int port,
	                     const std::string & dbname,
	                     const std::string & username,
	                     const std::string & password);
private:
	DataTree _data;
	MysqlConnection * _connection;
};

}

#endif
