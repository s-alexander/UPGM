#ifndef PAYGUIDE_CURL
#define PAYGUIDE_CURL 1

#include <string>
#include <curl/curl.h>
#include "semaphore_lock.hpp"

class Curl
{
public:
	enum METHOD
	{
		POST,
		GET
	};

	enum KEY_TYPE
	{
		PEM,
		DER
	};

	Curl();
	~Curl() throw();
	void SetHeader(const std::string & headers);

	void SetServerCertificate(const std::string & fileName);

	void SetClientPrivateKey(const std::string & fileName,
	                         const std::string & password,
	                         KEY_TYPE type = PEM);

	void SetClientPrivateKey(const std::string & fileName,
	                         KEY_TYPE type = PEM);

	void SetClientCertificate(const std::string & fileName,
	                          const std::string & password,
	                          KEY_TYPE type = PEM);

	void SetClientCertificate(const std::string & fileName,
	                          KEY_TYPE type = PEM);

	void SetPrivateKey(const std::string & fileName,
	                   KEY_TYPE type = PEM);

	void SetPrivateKey(const std::string & fileName,
	                   const std::string & password,
	                   KEY_TYPE type = PEM);

	bool SendRequest(const std::string & url,
	                 int port,
	                 const std::string & data,
	                 METHOD method,
	                 int timeout = 10);

	void WaitAnswer();
	const std::string & Answer() const;
private:
	CURL * curl_;
	std::string answer_;
	sem_t answ_lock;
	curl_slist * slist;

	const char * ToString(KEY_TYPE keyType);
};

#endif
