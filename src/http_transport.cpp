#include <upgm/http_transport.hpp>
#include <upgm/payguide_curl.hpp>
#include <cstdlib>

namespace PG
{

HTTPTransport::HTTPTransport(): _curl (0)
{
	_curl = new Curl();
}

HTTPTransport::~HTTPTransport() throw()
{
	delete _curl;
}

void HTTPTransport::operator<<(const std::string & data)
{
//	curl.SetClientCertificate(point.cert, point.password);
//	curl.SetServerCertificate(point.server_cert);

//	curl.SetHeader("SOAPAction: http://usmp.com.ua/" + SOAPAction);
//	curl.SetHeader("Content-Type: text/xml");

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


	_curl->SendRequest(url, port, data, Curl::POST, timeout);
}

PG::DataTree HTTPTransport::operator>>(std::string & buffer)
{
	_curl->WaitAnswer();
	buffer = _curl->Answer();
	fprintf(stderr, "GOT [%s]n", buffer.c_str());

	// TODO check error
	return DataTree();
}

}
