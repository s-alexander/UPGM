#ifndef __Statistic__
#define __Statistic__
#include "pay.h"
int StatisticInit();
int StatisticAddPay(SPay *pay, int result, int sleep, const char *msg, const char *sender_name);
int StatisticTick(float secdelay);
#endif

