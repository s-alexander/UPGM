#ifndef __Pay__
#define __Pay__
//Defenitions for filed's sizes and types:
#define SIZE_DATA 2048
#define SIZE_BILL_NUM 30
#define SIZE_STAMP 25

#define SIZE_REPONSE_MSG 1024
#define SIZE_REPONSE_SENDER 25

#include <pthread.h>
#include <semaphore.h>
#include <cstring>
#include "paysys.h"
#include "../lib/libbonbon/bonbon.h"
//Defenitions for work results:

#define RESULT_SUCESS 0
#define RESULT_SUCESS_BIN 5000
#define RESULT_ONLINECHECK_NOTSUPP 13
#define RESULT_UNKNOWN 20
#define RESULT_CANCELED 60
#define RESULT_SAVE_STATE 65
#define RESULT_KILLED 70
#define RESULT_NO_PAY_SYS 99
#define RESULT_FAILED 105
#define RESULT_31 31
#define RESULT_101 101
#define RESULT_106 106
#define RESULT_109 109
#define RESULT_TEST 200
#define RESULT_DO_NOTHING 1000
#define RESULT_QUEUE_FULL 201

#define NO_TEST 0
#define FORMAT_TEST 1
#define CHECKSYS_TEST 2
#define FORMAT_CHECKSYS_TEST 3
#define FULL_TEST 3
#define OK_TEST 4
#define NOK_TEST 5

class SPay
{
    public:
	SPay()
	{
		id=0;
		pay_engine=0;
		provider_id=0;
		memset(data, 0, SIZE_DATA+1);
		memset(stamp, 0, SIZE_STAMP+1);
		memset(bill_num, 0, SIZE_BILL_NUM+1);
		terminal_id=0;
		amount=0;
		summ=0;
		back_summ=0;
		currency=0;
		pay_canceled=false;
		sem_init(&pay_canceled_sem,0,1);
		lo_perc=0;
		op_perc=0;
		paysys_codename=NULL;
		conv_sub=NULL;
		test=NO_TEST;
		exists_int_checkreqs=0;
		type=0;
		magic='0';
		nettype='0';
		netversion='0';
		xml_output=false;
		xml_sock=NULL;
	}
	SPay &operator=(const SPay &val)
	{
		id=val.id;
		pay_engine=val.pay_engine;
		provider_id=val.provider_id;
		memcpy(data, val.data,SIZE_DATA+1);
		memcpy(stamp, val.stamp, SIZE_STAMP+1);
		memcpy(bill_num, val.bill_num, SIZE_BILL_NUM+1);
		terminal_id=val.terminal_id;
		amount=val.amount;
		summ=val.summ;
		back_summ=val.back_summ;
		currency=val.currency;
		lo_perc=val.lo_perc;
		op_perc=val.op_perc;
		paysys_codename=val.paysys_codename;
		conv_sub=val.conv_sub;
		test=val.test;
		exists_int_checkreqs=val.exists_int_checkreqs;
		type=val.type;
		magic=val.magic;
		nettype=val.nettype;
		netversion=val.netversion;
		xml_output=val.xml_output;
		xml_sock=val.xml_sock;
		return *this;
	}
	~SPay(){};
	bonbon::CSemaphore lock;
	long long id;
	int pay_engine;
	int provider_id;
	char data[SIZE_DATA+1];
	char stamp[SIZE_STAMP+1];
	char bill_num[SIZE_BILL_NUM+1];
	long long terminal_id;
	float amount;
	float summ;	//REAL SUMM
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
	unsigned char nettype;
	unsigned char netversion;
	bool xml_output;
	void *xml_sock;
};

class SPayResult
{
    public:
	SPayResult()
	{
		code=RESULT_FAILED;
		sleep=0;
		memset(msg, 0, SIZE_REPONSE_MSG+1);
		memset(sender_name, 0, SIZE_REPONSE_SENDER+1);
//		memset(bdata, 0, SIZE_REPONSE_BINARY+1);
		bdata_size=0;
	}
	~SPayResult(){};
	int code;
	int sleep;
	char msg[SIZE_REPONSE_MSG+1];
	char sender_name[SIZE_REPONSE_SENDER+1];
//	char bdata[SIZE_REPONSE_BINARY+1];
	int bdata_size;
};

#endif


