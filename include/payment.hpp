#ifndef PG_PAYMENT
#define PG_PAYMENT

#include <upgm/data_tree.hpp>
#include <pay.h>

namespace PG
{

class Payment
{
public:
	enum Result { UNDEF, COMPLETE, FAIL, SLEEP };
	Payment();
	bool stateUndef() const { return UNDEF == _result; }
	Result result() const { return _result; }
	void completed() { _result = COMPLETE; }
	void failed() { _result = FAIL; }
	void sleep(unsigned int sec) { _result = SLEEP; _sleep = sec; }
	void setErrorCode(int errorCode) { _errorCode = errorCode; }
	void error(const std::string & errorStr) {
		if (_errorStr.empty())
		{
			_errorStr = errorStr;
		}
		else {
			_errorStr.append("; (" + errorStr + ")");
		}
	}
	DataTree generateDataTree() const;
	Payment(const SPay & pay);
	const std::string & errorStr() const { return _errorStr; }
	SPayResult asSPayResult();
	SPayResult asSPayResult(const char * modname);
private:
	SPay _data;
	Result _result;
	unsigned int _sleep;
	int _errorCode;
	std::string _errorStr;
};

}

#endif
