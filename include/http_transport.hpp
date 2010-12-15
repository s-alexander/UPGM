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
	HTTPTransport(Log & log);
	virtual ~HTTPTransport() throw();
	void configure(const DataTree & config);
private:
	virtual void writeImpl(const std::string & data);
	virtual DataTree readImpl(std::string & data);
	Curl * _curl;
};

}

#endif
