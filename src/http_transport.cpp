#include <upgm/http_transport.hpp>
#include <upgm/payguide_curl.hpp>
#include <cstdlib>

namespace PG
{

HTTPTransport::HTTPTransport(Log & log):Transport(log), _curl (0)
{
	_curl = new Curl();
}

HTTPTransport::~HTTPTransport() throw()
{
	delete _curl;
}

void HTTPTransport::writeImpl(const std::string & data)
{
	const std::string url = _config("url");
	const int port = atoi(_config("port").c_str());
	int timeout = 30;
	try {
		timeout = atoi(_config("timeout").c_str());
	} catch ( ... ) { ;; }


	try {
		const std::string & serverSertificate = _config("server_certificate");
		_curl->SetServerCertificate(serverSertificate);
	} catch ( ... ) { ;; }

	try {
		const bool isPem ( true );
		const std::string & privateKey = _config("client_private_key");
		try
		{
			const std::string & keyPassword = _config("client_private_key_password");
			_curl->SetClientPrivateKey(privateKey, keyPassword, isPem ? Curl::PEM : Curl::DER);
		}
		catch ( ... ) {
			_curl->SetClientPrivateKey(privateKey, isPem ? Curl::PEM : Curl::DER);
		}
	} catch ( ... ) { ;; }


	try {
		const bool isPem ( true );
		const std::string & cert = _config("client_certificate");
		try {
			const std::string & password = _config("client_certificate_password");
			_curl->SetClientCertificate(cert, password, isPem ? Curl::PEM : Curl::DER);
		}
		catch ( ... )
		{
			_curl->SetClientCertificate(cert, isPem ? Curl::PEM : Curl::DER);
		}
	} catch ( ... ) { ;; }

	try {
		const bool isPem ( true );
		const std::string & privateKey = _config("private_key");
		try {
			const std::string & password = _config("private_key_password");
			_curl->SetPrivateKey(privateKey, password, isPem ? Curl::PEM : Curl::DER);
		} catch ( ... ) {
			_curl->SetPrivateKey(privateKey, isPem ? Curl::PEM : Curl::DER);
		}
	} catch ( ... ) { ;; }

	std::string method("post");
	try {
		method=_config("method");
	} catch ( ...) { ;; }
	_curl->SendRequest(url, port, data, ((method == "get" ) ? Curl::GET: Curl::POST), timeout);
}

PG::DataTree HTTPTransport::readImpl(std::string & buffer)
{
	_curl->WaitAnswer();
	buffer = _curl->Answer();

	// TODO check error
	return DataTree();
}

}
