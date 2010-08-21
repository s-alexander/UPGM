#ifndef PG_MYSQL_CONNECTION
#define PG_MYSQL_CONNECTION

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

class MysqlConnection
{
public:
	MysqlConnection() { ;; }
	virtual ~MysqlConnection() throw() { ;; }
	virtual Db::RequestResult performRequest(const std::string & request) = 0;

	virtual std::string escape(const std::string & str) = 0;
	virtual void disconnect() = 0;

	virtual void connect(const std::string & host,
	                     unsigned int port,
	                     const std::string & dbname,
	                     const std::string & username,
	                     const std::string & password) = 0;
};

}

#endif
