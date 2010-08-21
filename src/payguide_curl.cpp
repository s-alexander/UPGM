#include <upgm/payguide_curl.hpp>


Curl::Curl(): curl_(NULL), slist(NULL)
{
	sem_init(& answ_lock, 0, 1);
	curl_ = ::curl_easy_init();
	curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, false);
}

Curl::~Curl() throw()
{
	if (slist != NULL) { ::curl_slist_free_all(slist); }
	if (curl_ != NULL) { ::curl_easy_cleanup(curl_);   }
}

void Curl::SetServerCertificate(const std::string & fileName)
{
	curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, true);
	curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, true);
	curl_easy_setopt(curl_, CURLOPT_CAINFO, fileName.c_str());
}

void Curl::SetClientPrivateKey(const std::string & keyFilename,
                               const std::string & keyPassword,
                               KEY_TYPE type)
{
	curl_easy_setopt(curl_, CURLOPT_SSLKEY, keyFilename.c_str());
	curl_easy_setopt(curl_, CURLOPT_SSLKEYTYPE, ToString(type));
	curl_easy_setopt(curl_, CURLOPT_SSLKEYPASSWD, keyPassword.c_str());
}

void Curl::SetClientPrivateKey(const std::string & keyFilename,
                               KEY_TYPE type)
{
	curl_easy_setopt(curl_, CURLOPT_SSLKEY, keyFilename.c_str());
	curl_easy_setopt(curl_, CURLOPT_SSLKEYTYPE, ToString(type));
}

void Curl::SetClientCertificate(const std::string & certFilename,
                                const std::string & certPassword,
                                KEY_TYPE type)
{
	curl_easy_setopt(curl_, CURLOPT_SSLCERT, certFilename.c_str());
	curl_easy_setopt(curl_, CURLOPT_SSLCERTTYPE, ToString(type));
	curl_easy_setopt(curl_, CURLOPT_SSLCERTPASSWD, certPassword.c_str());
}

void Curl::SetClientCertificate(const std::string & certFilename,
                                KEY_TYPE type)
{
	curl_easy_setopt(curl_, CURLOPT_SSLCERT, certFilename.c_str());
	curl_easy_setopt(curl_, CURLOPT_SSLCERTTYPE, ToString(type));
}

void Curl::SetHeader(const std::string & header)
{
	slist = curl_slist_append(slist, header.c_str());
}

static size_t GetCurlData(void *buffer, size_t size, size_t nmemb, void *userp)
{
	try
	{
		if (userp==NULL) return 0;
		std::string * & buff=*((std::string **)userp);

		if (0 == buff) {
			buff = new std::string((const char *)buffer, nmemb);
		}
		else {
			buff->append((const char *)buffer, nmemb);
		}
	}
	catch ( ... ) { return 0; }
	return nmemb;
}

bool Curl::SendRequest(const std::string & url,
                       int port,
                       const std::string & data,
                       METHOD method,
                       int timeout)
{
	SemaphoreLock lock(answ_lock);
	answer_.clear();

	std::string * dataBuff = 0;

	curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, GetCurlData);
	curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &dataBuff);

	curl_easy_setopt(curl_, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl_, CURLOPT_PORT, port);

	curl_easy_setopt(curl_, CURLOPT_VERBOSE, true);

	std::string fullUrl(url);

	if (NULL != slist) { curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, slist); }


	switch (method)
	{
		case POST:
		{
			curl_easy_setopt(curl_, CURLOPT_POST, true);
			curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, data.c_str());
		}
		break;
		case GET:
		{
			curl_easy_setopt(curl_, CURLOPT_POST, false);
			fullUrl += data;
		}
		break;
	}
	fprintf(stderr, "curl %p perform %s\n", curl_, fullUrl.c_str());

	curl_easy_setopt(curl_, CURLOPT_URL, fullUrl.c_str());


	if (!(timeout<0)) { curl_easy_setopt(curl_, CURLOPT_TIMEOUT, timeout); }


	curl_easy_perform(curl_);

	if (dataBuff != NULL)
	{
		answer_.assign(*dataBuff);
		delete dataBuff;
		return true;
	}
	else {
		fprintf(stderr, "empty data\n");
	}

	return false;
}

void Curl::WaitAnswer()
{
	SemaphoreLock lock(answ_lock);
}


const std::string & Curl::Answer() const
{
	return answer_;
}

void Curl::SetPrivateKey(const std::string & fileName,
                         KEY_TYPE type)
{
	curl_easy_setopt(curl_, CURLOPT_SSLKEY, fileName.c_str());
	curl_easy_setopt(curl_, CURLOPT_SSLKEYTYPE, ToString(type));
}

void Curl::SetPrivateKey(const std::string & fileName,
                         const std::string & password,
                         KEY_TYPE type)
{
	curl_easy_setopt(curl_, CURLOPT_SSLKEY, fileName.c_str());
	curl_easy_setopt(curl_, CURLOPT_SSLKEYTYPE, ToString(type));
	curl_easy_setopt(curl_, CURLOPT_SSLKEYPASSWD, password.c_str());
}

const char * Curl::ToString(KEY_TYPE keyType)
{
	switch (keyType)
	{
		case PEM:
			return "PEM";
		break;
		case DER:
			return "DER";
		break;
	}
	return "UNKNOWN_KEY_TYPE";
}

