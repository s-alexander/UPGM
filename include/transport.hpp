#ifndef PG_TRANSPORT
#define PG_TRANSPORT

#include <string>
#include <upgm/data_tree.hpp>

namespace PG
{

class Transport
{
public:
	Transport();
	virtual ~Transport() throw() = 0;
	void configure(const DataTree & config);
	virtual void operator<<(const std::string & data) = 0;
	virtual DataTree operator>>(std::string & buffer) = 0;
private:
	DataTree _config;
};

}

#endif
