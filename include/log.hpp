#ifndef PG_LOG
#define PG_LOG

#include <string>
#include <fstream>
#include <upgm/data_tree.hpp>

namespace PG
{

class Log
{
public:
	Log();
	virtual ~Log() throw();
	void setup(const std::string & path, const std::string & payment);
	Log & operator<<(const std::string & data) throw();
private:
	std::ofstream * _log;
};

}

#endif
