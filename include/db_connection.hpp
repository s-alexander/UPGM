#ifndef PG_DB_CONNECTION
#define PG_DB_CONNECTION

#include <string>
#include <memory>
#include <semaphore.h>

#include <upgm/data_tree.hpp>
#include <upgm/db.hpp>

namespace PG
{

class DbConnection
{
public:
	DbConnection() { ;; }
	virtual ~DbConnection() throw() { ;; }
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
