#ifndef PG_MYSQL_CONNECTION
#define PG_MYSQL_CONNECTION

#include <string>
#include <memory>
#include <semaphore.h>

#include <upgm/data_tree.hpp>
#include <upgm/db.hpp>
#include <upgm/db_connection.hpp>

namespace mysqlpp
{
	class Connection;
}

namespace PG
{

class MysqlConnection: public DbConnection
{
public:
	MysqlConnection() { ;; }
	virtual ~MysqlConnection() throw() { ;; }
};

}

#endif
