#include <upgm/payment.hpp>
#include <upgm/mysql_connection.hpp>
#include <upgm/db_mysql.hpp>

namespace PG
{

DataTree Payment::generateDataTree() const
{
	DataTree result;
	result.set("id", "13");
	fprintf(stderr, "data = %s\n", _data.data);
	result.set("data", _data.data);
/*struct SPay
{
        long long id;
        int pay_engine;
        int provider_id;
        char data[SIZE_DATA+1];
        char stamp[SIZE_STAMP+1];
        char bill_num[SIZE_BILL_NUM+1];
        long long terminal_id;
        float summ;
        float back_summ;
        int currency;
        bool pay_canceled;
        sem_t pay_canceled_sem;
        float lo_perc;
        float op_perc;
        const char *paysys_codename;
        const char *conv_sub;
        int test;
        int exists_int_checkreqs;
        int type;
        char magic;
};
*/
	return result;
}

Payment::Payment(const SPay & pay)
{
	new(this) Payment();
	_data = pay;
}

SPayResult Payment::asSPayResult()
{
	SPayResult payRes;
	memset(payRes.sender_name, 0, SIZE_REPONSE_SENDER);
	memset(payRes.msg, 0, SIZE_REPONSE_MSG);
	strncpy(payRes.msg, _errorStr.c_str(), SIZE_REPONSE_MSG);


	strncpy(payRes.sender_name, "UPGM", SIZE_REPONSE_SENDER);

	if (result() == Payment::FAIL)
	{
		payRes.code=RESULT_FAILED;
		payRes.sleep = 0;
	}
	if (result() == Payment::COMPLETE)
	{
		payRes.code=RESULT_SUCESS;
		payRes.sleep = 0;
	}
	else
	{
		payRes.code=RESULT_SAVE_STATE;
		payRes.sleep = _sleep;
	}
	fprintf(stderr, "Return result [%s] - %i\n", payRes.msg, payRes.sleep);
	return payRes;
}


}
