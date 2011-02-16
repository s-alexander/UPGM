#include <upgm/payment.hpp>
#include <upgm/mysql_connection.hpp>
#include <upgm/db_mysql.hpp>
#include <upgm/as_string.hpp>

namespace PG
{


DataTree Payment::generateDataTree() const
{
	DataTree result;
	result.set("id", asString(_data.id));
	result.set("pay_engine", asString(_data.pay_engine));
	result.set("provider_id", asString(_data.provider_id));
	result.set("data", _data.data);
	result.set("stamp", _data.stamp);
	result.set("bill_num", _data.bill_num);
	result.set("terminal_id", asString(_data.terminal_id));
	result.set("summ", asString(_data.summ));
	result.set("back_summ", asString(_data.back_summ));
	result.set("currency", asString(_data.currency));
	result.set("lo_perc", asString(_data.lo_perc));
	result.set("op_perc", asString(_data.op_perc));
	result.set("paysys_codename", asString(_data.paysys_codename));
	result.set("test", asString(_data.test));
	result.set("exists_int_checkreqs", asString(_data.exists_int_checkreqs));
	return result;
}
const unsigned int fiveMinutes = 60 * 5;
Payment::Payment():_result(UNDEF), _sleep(fiveMinutes), _errorCode(0) {
	_data.paysys_codename = 0;
}

Payment::Payment(const SPay & pay)
{
	new(this) Payment();
	_data = pay;
}

SPayResult Payment::asSPayResult() { return this->asSPayResult("UPGM"); }

SPayResult Payment::asSPayResult(const char * modname)
{
	SPayResult payRes;
	memset(payRes.sender_name, 0, SIZE_REPONSE_SENDER);
	memset(payRes.msg, 0, SIZE_REPONSE_MSG);

	strncpy(payRes.sender_name, modname, SIZE_REPONSE_SENDER);

	if (result() == Payment::FAIL)
	{
		strncpy(payRes.msg, _errorStr.c_str(), SIZE_REPONSE_MSG-1);
		payRes.code=RESULT_FAILED;
		payRes.sleep = 0;
	}
	else if (result() == Payment::COMPLETE)
	{
		strncpy(payRes.msg, "Payment completed", SIZE_REPONSE_MSG-1);
		payRes.code=RESULT_SUCESS;
		payRes.sleep = 0;
	}
	else
	{
		strncpy(payRes.msg, _errorStr.c_str(), SIZE_REPONSE_MSG-1);
		payRes.code=RESULT_SAVE_STATE;
		payRes.sleep = _sleep;
	}

	char * ptr = payRes.msg;
	const char badChrs[] = {';', '\\', '\'', '\"'};
	while (*ptr != 0) {
		for (size_t c = 0; c < sizeof(badChrs)/sizeof(badChrs[0]); ++c) {
			if (*ptr == badChrs[c]) {
				*ptr = '?';
				break;
			}
		}
		++ptr;
	}

	fprintf(stderr, "Return result [%s] - %i\n", payRes.msg, payRes.sleep);
	return payRes;
}


}
