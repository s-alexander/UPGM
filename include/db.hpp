#ifndef PG_DB
#define PG_DB

#include <string>
#include <stdexcept>
#include <upgm/data_tree.hpp>

namespace PG
{

class DbException: public std::runtime_error
{
public:
	DbException(const std::string & what): std::runtime_error(what) { ;; }
};

class Db
{
public:
	Db();
	virtual ~Db() throw() = 0;

	typedef std::map< std::string, std::string > Row;

	typedef std::vector< Row > RequestResult;
	virtual RequestResult performRequest(const std::string & request) = 0;

	virtual std::string escape(const std::string & str) = 0;

	virtual void connect(const std::string & host,
	                     unsigned int port,
	                     const std::string & dbname,
	                     const std::string & username,
	                     const std::string & password) = 0;
protected:
	DataTree _config;
};

}

#endif
