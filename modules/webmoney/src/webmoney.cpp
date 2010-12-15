#include <errno.h>
#include <cstdlib>
#include <cstdio>
#include <fstream>

#include "webmoney.hpp"
#include <upgm/hook.hpp>

#include "wmsigner/stdafx.h"
#include "wmsigner/signer.h"
#include "wmsigner/base64.h"
#include "wmsigner/cmdbase.h"

namespace PG
{

class WebMoneySignatureHook: public Hook
{
	~WebMoneySignatureHook() throw() { ;; }
	const char * name() const { return "wm_signature"; }

	virtual std::string read(const Path & path)
	{
		const std::string toSignCopy(_toSign);
		szptr toSign = toSignCopy.c_str(), szSign, login = _login.c_str(), password = _password.c_str();
		_toSign.clear();
		char pszOut[MAXBUF+1] = "";

		Signer sign(login, password, 0);
		sign.isIgnoreKeyFile = true;
		sign.isIgnoreIniFile = true;
		sign.isKWMFileFromCL = true;
		//  szKeyData[MAXBUF+1] = "";       // Buffer for Signre-s key
		bool key64 (true);
		sign.Key64Flag = key64;

		fprintf(stderr, "*** SIGN [%s] ***\n", toSignCopy.c_str());
		//if( key64 == TRUE ) {
			sign.SetKeyFromCL( true, _keydata.data() );
		//}
		int result = sign.Sign(toSign, szSign);
		short errorCode = sign.ErrorCode();

		if ( result ){
			strncpy( pszOut, szSign, MAXSTR);
			return pszOut;
		}
		sprintf(pszOut, "Error %d", errorCode );
		return pszOut;
	}
	virtual void write(const Path & path, const std::string & value)
	{
		if (path.size() == 1)
		{
			if (path[0] == "keyfile")
			{
				_keydata.clear();
				std::ifstream file(value.c_str());
				while (file.good()) {
					enum { bufferSize = 1024 };
					char buffer[bufferSize+1];

					file.read(buffer, bufferSize);
					_keydata.append(buffer, file.gcount());
				}
				file.close();
			} else if (path[0] == "login") {
				_login = value;
			} else if (path[0] == "password") {
				_password = value;
			} else if (path[0] == "append_data") {
				_toSign += value;
			}
			else {
				throw InvalidArgumentException("Supported operations: keyfile login password append_data");
			}
		}
		else {
			throw InvalidArgumentException("path size must be equal to 1");
		}
	}
private:
	std::string _login;
	std::string _password;
	std::string _keydata;
	std::string _toSign;
};

void WebMoney::registerUserHooks(Transport &transport, Parser &parser, Payment &payment)
{
	registerHook(HookPtr(new WebMoneySignatureHook()));
}
}
