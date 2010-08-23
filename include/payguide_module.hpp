#ifndef PG_PAYGUIDE_MODULE
#define PG_PAYGUIDE_MODULE


extern "C" void *ThreadStart()
{
	return 0;
}

extern "C" void ThreadEnd(void *thread_init)
{
	return;
}

extern "C" void *Load(CPaySys *paysys)
{
	return 0;
}

extern "C" void Call(void *first_init_param)
{
	/* Runs every 1.0-1.2 sec */
	return;
}

extern "C" void UnLoad(CPaySys *paysys, void *first_init_param)
{

}

extern "C" void *PayInitExt(CPaySys *paysys, COperator *operat, SPay *pay, int worker_id, void *first_init_param)
{
	return 0;
}

extern "C" SPayResult SendData(SPay *data, void *init_param, void *first_init_param, void *pthread_init)
{
	SPayResult rsl;
	strncpy(rsl.msg, "-",SIZE_REPONSE_MSG);
	strncpy(rsl.sender_name, "eport_d",SIZE_REPONSE_SENDER);
	rsl.code=RESULT_FAILED;
	rsl.sleep=0;
	return rsl;
}


extern "C" void CleanUp(void *param)
{
}

#endif
