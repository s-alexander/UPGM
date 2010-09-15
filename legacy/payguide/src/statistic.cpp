#include "log.h"
#include "statistic.h"
#include "namespace.h"

static int StaticResetCounters();


//static float timer=0;

//static int normal_pays[13];
//static int check_pays[13];

int StatisticAddPay(SPay *pay, int result, int sleep, const char *msg, const char *sender_name)
{
/*	int *store=NULL;
	if (pay->type==NO_TEST)
		store=normal_pays;
	else
		store=check_pays;

	if (result==RESULT_SUCESS)
		store[0]++;
	if (result==RESULT_ONLINECHECK_NOTSUPP)
		store[1]++;
	if (result==RESULT_CANCELED)
		store[2]++;
	if (result==RESULT_SAVE_STATE)
		store[3]++;
	if (result==RESULT_KILLED)
		store[4]++;
	if (result==RESULT_NO_PAY_SYS)
		store[5]++;
	if (result==RESULT_FAILED)
		store[6]++;
	if (result==RESULT_101)
		store[7]++;
	if (result==RESULT_106)
		store[8]++;
	if (result==RESULT_109)
		store[9]++;
	if (result==RESULT_TEST)
		store[10]++;
	if (result==RESULT_DO_NOTHING)
		store[11]++;
	if (result==RESULT_QUEUE_FULL)
		store[12]++;*/

	return 0;
}

int StatisticTick(float secdelay)
{
//	timer+=secdelay;
//	if (timer>=5)
//	{
//		StaticResetCounters();
//		timer=0;
//	}
	return 0;
}

static int StaticResetCounters()
{
//	memset(normal_pays,0, sizeof(normal_pays));
//	memset(check_pays,0, sizeof(check_pays));
	return 0;
}

int StatisticInit()
{
	return 0;
}
