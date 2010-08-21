#ifndef PG_SHARED_MYSQL_CONNECTION
#define PG_SHARED_MYSQL_CONNECTION

#include <string>
#include <memory>
#include <semaphore.h>

#include <upgm/data_tree.hpp>
#include <upgm/mysql_connection.hpp>

namespace mysqlpp
{
	class Connection;
}

namespace PG
{

class SharedMysqlConnection: public MysqlConnection
{
public:
	SharedMysqlConnection();
	virtual ~SharedMysqlConnection() throw();
	virtual Db::RequestResult performRequest(const std::string & request);

	virtual std::string escape(const std::string & str);

	virtual void connect(const std::string & host,
	                     unsigned int port,
	                     const std::string & dbname,
	                     const std::string & username,
	                     const std::string & password);
	virtual void disconnect();
private:
	void checkConnection();
	std::auto_ptr<mysqlpp::Connection> _connection;
	sem_t _lock;
};

}

#endif
