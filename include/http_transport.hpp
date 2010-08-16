#ifndef PG_HTTP_TRANSPORT
#define PG_HTTP_TRANSPORT

#include <string>
#include <upgm/transport.hpp>

class Curl;

namespace PG
{

class HTTPTransport: public Transport
{
public:
	HTTPTransport();
	virtual ~HTTPTransport() throw();
	void configure(const DataTree & config);
	virtual void operator<<(const std::string & data);
	virtual DataTree operator>>(std::string & buffer);
private:
	Curl * _curl;
};

}

#endif
