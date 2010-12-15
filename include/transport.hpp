#ifndef PG_TRANSPORT
#define PG_TRANSPORT

#include <string>
#include <upgm/data_tree.hpp>
#include <upgm/log.hpp>

namespace PG
{

class Transport
{
public:
	Transport(Log & log);
	virtual ~Transport() throw() = 0;
	void configure(const DataTree & config);
	void operator<<(const std::string & data);
	DataTree operator>>(std::string & buffer);
protected:
	DataTree _config;
private:
	virtual void writeImpl(const std::string & data) = 0;
	virtual DataTree readImpl(std::string & data) = 0;
	Log & _log;
};

}

#endif
