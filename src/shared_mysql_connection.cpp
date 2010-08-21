#include <upgm/shared_mysql_connection.hpp>

#include <mysql++/mysql++.h>
#include <mysql++/connection.h>
#include <upgm/semaphore_lock.hpp>

namespace PG
{

SharedMysqlConnection::SharedMysqlConnection()
{
	::sem_init(&_lock, 0, 1);
}

SharedMysqlConnection::~SharedMysqlConnection() throw()
{
	SemaphoreLock guard(_lock );
	try
	{
		checkConnection();
		_connection->disconnect();
	}
	catch (...) { ;; }
}

Db::RequestResult SharedMysqlConnection::performRequest(const std::string & request)
{
	SemaphoreLock guard(_lock );
	checkConnection();
	fprintf(stderr, "execute [%s]\n",request.c_str());

	Db::RequestResult result;
	mysqlpp::Query query = _connection->query(request);
	mysqlpp::StoreQueryResult dbResult = query.store();
	const size_t fields = dbResult.num_fields();
	const size_t rows = dbResult.num_rows();
	for (size_t r = 0; r < rows; ++r)
	{
		Db::Row row;
		for (size_t f = 0; f < fields; ++f)
		{
			const mysqlpp::Field & field = dbResult.field(f);
			const mysqlpp::String & value = dbResult[r][f];
			std::string str(value.begin(), value.end());
			row[ field.name() ] = str;
		}
		result.push_back(row);
	}
	return result;
}

template<typename T>
class ArrayScopeGuard
{
public:
	ArrayScopeGuard(T * ptr): _ptr(ptr) { ;; }
	~ArrayScopeGuard() throw() { delete [] _ptr; }
private:
	ArrayScopeGuard(const ArrayScopeGuard &);
	void operator=(const ArrayScopeGuard &);
	T * _ptr;
};

std::string SharedMysqlConnection::escape(const std::string & str)
{
	SemaphoreLock guard(_lock );
	checkConnection();

	const size_t strSize = str.size();
	char * buffer = new char[strSize*2+1];
	ArrayScopeGuard<char> bufferGuard(buffer);

	mysqlpp::Query query = _connection->query();
	const size_t l = query.escape_string(buffer, str.data(), strSize);
	return std::string(buffer, l);
}

void SharedMysqlConnection::checkConnection()
{
	// Note: this one is not thread-safe
	if (_connection.get() == 0)
	{
		throw std::runtime_error("Not connected");
	}
}

void SharedMysqlConnection::connect(const std::string & host, unsigned int port, const std::string & dbname, const std::string & username, const std::string & password)
{
	SemaphoreLock guard(_lock );
	if (0 == _connection.get())
	{
		_connection = std::auto_ptr<mysqlpp::Connection>(new mysqlpp::Connection());
	}

	if (_connection->ping() == false)
	{
		_connection->connect(dbname.c_str(), host.c_str(), username.c_str(), password.c_str(), port);
	}
}

void SharedMysqlConnection::disconnect()
{

}

}
