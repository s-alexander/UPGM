#ifndef PG_DB
#define PG_DB

#include <string>
#include <upgm/data_tree.hpp>

namespace PG
{

class Db
{
public:
	Db();
	virtual ~Db() throw() = 0;
	virtual const std::string & operator[](const std::string & field) = 0;
	virtual std::string & operator[](const std::string & field) = 0;
	virtual void read(const std::vector<std::string> fields) = 0;
	virtual void write() = 0;
	void configure(const DataTree & config);
protected:
	DataTree _config;
};

}

#endif
