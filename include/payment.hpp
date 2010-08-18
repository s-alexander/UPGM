#ifndef PG_PAYMENT
#define PG_PAYMENT

#include <upgm/data_tree.hpp>

namespace PG
{

class Payment
{
public:
	enum Result { UNDEF, COMPLETE, FAIL, SLEEP };
	Payment():_result(UNDEF) { ;; }
	bool stateUndef() const { return UNDEF == _result; }
	Result result() const { return _result; }
	void completed() { _result = COMPLETE; }
	void failed() { _result = FAIL; }
	void sleep() { _result = SLEEP; }
	DataTree generateDataTree() const;
private:
	Result _result;
};

}

#endif
