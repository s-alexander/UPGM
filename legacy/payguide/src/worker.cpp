#include <iostream>
#include <mysql/mysql.h>
#include <mysql/mysql_com.h>
#include <mysql/mysql_version.h>
#include "worker.h"
#include "operator.h"
#include "db.h"
#include "ctl.h"
#include "log.h"
#include "perlm.h"
#include "format.h"
#include "namespace.h"

#define EVER ;;

void * WorkEx(void *param);

static void PayKill(void *param);
static void ThreadExit (CPaySys *paysys, void *thread_init);

struct CleanUpData
{
	CPaySys *paysys;
	void *init_values;
	SPay *pay;
};

static int workers_ids=1;
CWorker::~CWorker()
{
	if (pay!=NULL) delete pay;
	pay=NULL;
}

CWorker::CWorker()
{
	pay=NULL;
	paysys=NULL;
	operat=NULL;
	detached=0;
	lifetime=0;	
	engine_name=NULL;
	id=workers_ids;
	if (workers_ids<9999) workers_ids++; else workers_ids=1;
	sem_init(&sem, 0,0);
	killed=0;
	
	int d=Detach();
	if (d!=0) 
	{
		KillSelf();
		LogWrite(LOGMSG_ERROR, "Can't create a new thread - pthread_create failed. To much threads?");
		
	}

}

int CWorker::GetId()
{
	return id;
}

void CWorker::KillSelf()
{
	killed=1;
}


int CWorker::Killed()
{
	return killed;
}

/*void CWorker::Cancel()
{
	if (this->Busy()==1)
	{
		working_workers--;
		this->CancelPay();
	}

	this->KillSelf();
	this->Run();
	pthread_join(thread, NULL);
	return;
}*/

void CWorker::Kill()
{
	if (this->Busy()==1)
	{
		payguide::working_workers--;
		pthread_cancel(thread);
		pthread_join(thread, NULL);
	}
	else
	{
		this->KillSelf();
		this->Run();
		pthread_join(thread, NULL);
	}
	return;
}


int CWorker::SetPaySys(CPaySys *new_paysys)
{
	paysys=new_paysys;
	if (paysys!=NULL)
		paysys->Use();
	
	return 0;
}

int CWorker::SetOperator(COperator *new_operator)
{
	operat=new_operator;
	if (operat!=NULL)
		operat->Use();
	return 0;
}

CUlist<CWorker> *CreateWorkers(int num)
{
	CUlist<CWorker> *result=new CUlist<CWorker>();
	if (result==NULL) return NULL;
	for (int i=0;i<num;i++)
	{
		CWorker *tmp=new CWorker();
		
		/* If Killed()==1, pthread_create fails or smth goes wrong.
		Killed()==0 indicates that worker is OK */
		
		if (tmp->Killed()==0)
			result->AddItem(tmp);
		else
			delete tmp;
	}
	return result;
}

int CWorker::Detach()
{
	int result=1;
	if (0==detached && killed==0)
	{
		detached=1;
		killed=0;

		result=pthread_create(&thread, NULL, &WorkEx, this);
		
	}
	else
	{
		/* Already detached, or killed */
		return 1;
	}
return result;
}

pthread_t *CWorker::GetThread()
{
	return &thread;
}

CPaySys *CWorker::GetPaySys()
{
	return paysys;
}

COperator *CWorker::GetOperator()
{
	return operat;
}

int CWorker::AddPay(SPay *new_pay)
{

	if (new_pay!=NULL) 
	{
		lifetime=0;
		killed=0;
	}
	
	if (pay==NULL)
	{
	    pay=new_pay;
	    return 0;
	}
	else return 1;
}

int CWorker::Busy()
{
	if (pay==NULL) return 0;
	return 1;
}

int CWorker::CancelPay()
{
	if (pay!=NULL)
	{
		sem_wait(&pay->pay_canceled_sem);
		pay->pay_canceled=1;
		sem_post(&pay->pay_canceled_sem);
		return 0;
	}
	return 1;
}

int CWorker::FinishPay()
{
	if (pay!=NULL)
	{
		lifetime=0;
		delete pay;pay=NULL;
		
		return 0;
	}
	return 1;
}


SPay *CWorker::GetPay()
{
	return pay;
}

void CWorker::Run()
{
    sem_post(&sem);
}

void CWorker::Wait()
{
    sem_wait(&sem);
}

void CWorker::Live()
{
	/*604800 means 604800 sec = 168 hours = 7 days */
	if (lifetime<604800) lifetime++;
	return;
}

int CWorker::GetLifeTime()
{
	return lifetime;
}

int SetFreeWorker(CWorker *worker)
{
	if (worker!=NULL)
	{
		payguide::working_workers--;
		payguide::free_worker=worker;
		return 0;
	}
	else
	{
		unsigned int l=payguide::workers_list->GetLen();

		payguide::workers_list->ResetCursor();
		for (unsigned int i=0; i<l; i++)
		{
			CWorker *w=payguide::workers_list->GetNext();
			if (w!=NULL)
			{
				if (w->Busy()==0 && w->Killed()==0)
				{
					payguide::free_worker=w;
					payguide::workers_list->ResetCursor();
					return 0;
				}
			}
		}
		
		/* All threads are working - add new thread */
		if (l<payguide::thread_max)
		{
//			std::cout << "Adding new thread, " << l+1 << " workers now." << std::endl;
			char lm[50]; snprintf(lm,49, "Adding new thread, %i workers now.", l+1);
			LogWrite(LOGMSG_NORMAL,lm);

			CWorker *tmp=new CWorker();
			if (tmp==NULL) return 2;
			if (tmp->Killed()!=0)
			{
				delete tmp;
				payguide::free_worker=NULL;
				return -1;
			}
			payguide::workers_list->AddItem(tmp);
			payguide::free_worker=tmp;
			return 0;
		}
		else
		{
		/* Threads limit hit */
		payguide::free_worker=NULL;
		return -1;
		}
	}
	return -1;
}


CWorker *InitFreeWorker(SPay *pay)
{
	if (payguide::free_worker!=NULL)
	{
		COperator *operat=NULL;
		payguide::operators_list->ResetCursor();
		for (unsigned int i=0; i<payguide::operators_list->GetLen(); i++)
		{
			COperator *op=payguide::operators_list->GetNext();
			if (op!=NULL)
			{
				if (op->GetOperatorId()==pay->provider_id) operat=op;
			}
		}
		if (operat==NULL)
		{
//			std::cout << "No operator with id=" << pay->provider_id << ", using libnull.so" << std::endl;
//			if (payguide::null_pay_sys==NULL) {std::cout << "NULL pay sys is invalid!\n";}
			char lm[50]; snprintf(lm,49, "No operator with id=%i, using libnull.so", pay->provider_id);
			LogWrite(LOGMSG_WARNING,lm);

			payguide::free_worker->SetPaySys(payguide::null_pay_sys);
			payguide::free_worker->SetOperator(NULL);
			return payguide::free_worker;
		}
		else
		{
			CPaySys *paysys=operat->GetValidPaySys(pay);
			payguide::free_worker->SetPaySys(paysys);
			payguide::free_worker->SetOperator(operat);
			return payguide::free_worker;
		}
	}
	return NULL;
}

int KillWorker(long long pay_id)
{
	int result=1;
	sem_wait(&payguide::free_workers_lock);

	unsigned int l=payguide::workers_list->GetLen();

	payguide::workers_list->ResetCursor();
	for (unsigned int i=0; i<l; i++)
	{
		CWorker *w=payguide::workers_list->GetNext();
		if (w!=NULL)
		{
			if (1==w->Busy())
			{
				if (w->GetPay()->id==pay_id)
				{
					w->Kill();
					if (payguide::free_worker==w) payguide::free_worker=NULL;
					payguide::workers_list->RemoveThis();
					result=0;
					i=l;
				}
			}
		}
	}
	sem_post(&payguide::free_workers_lock);
	return result;
}

int CancelWorker(long long pay_id)
{
	int result=1;
	sem_wait(&payguide::free_workers_lock);
	
	unsigned int l=payguide::workers_list->GetLen();
	
	payguide::workers_list->ResetCursor();
	for (unsigned int i=0; i<l; i++)
	{
		CWorker *w=payguide::workers_list->GetNext();
		if (w!=NULL)
		{
			if (1==w->Busy())
			{
				if (w->GetPay()->id==pay_id)
				{
					w->CancelPay();
					if (w->Busy()==1) result=0;
					else result=2;
					i=l;
				}
			}
		}
	}
	sem_post(&payguide::free_workers_lock);
	return result;
}

int CancelAllWorkers()
{
	sem_wait(&payguide::free_workers_lock);
	
	unsigned int l=payguide::workers_list->GetLen();
	
	payguide::workers_list->ResetCursor();
	for (unsigned int i=0; i<l; i++)
	{
		CWorker *w=payguide::workers_list->GetNext();
		if (w!=NULL)
			w->CancelPay();
	}
	payguide::free_worker=NULL;
	sem_post(&payguide::free_workers_lock);
	return 0;
}

void *WorkEx(void *param)
{
	SPayResult work_result;
	CleanUpData cud;
	int db_err=0;
	int worker_id=-1;
	CWorker *worker;
	CPaySys *paysys=NULL;
	COperator *operat=NULL;
	void *thread_init=NULL;
	CPaySys *paysys_new=NULL;
	COperator *operat_new=NULL;
//	char *perl_conv_sub=NULL;
	
	mysql_thread_init();
	if (param==NULL)
		ThreadExit(paysys, thread_init);
	
	worker=(CWorker *)param;
	
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
	
	for (EVER)
	{
//		std::cout << "Thread " << worker->GetId() << " ready and waiting for a pay" << std::endl;

		
		worker->Wait();
		//printf("thread started\n");
		if (1==worker->Killed()) /* We are killed :( */
			ThreadExit(paysys,thread_init);
		
		sem_wait(&payguide::free_workers_lock);
		SPay *pay=worker->GetPay();
		
		CPaySys *validator=GetPaySysBySoName("validator.so");
		bool validator_off=false;
		bool validator_ignore_warnings=true;
//		CPaySys *validator=NULL;

		if (pay==NULL)
			ThreadExit(paysys, thread_init);\
	
		
			
		paysys_new=worker->GetPaySys();
		operat_new=worker->GetOperator();
		
		if (paysys!=paysys_new)
		{


			if (paysys!=NULL)
			{
				paysys->ThreadEnd(thread_init);
				paysys->UnUse();
			}
			if (paysys_new!=NULL)
			{
				thread_init=paysys_new->ThreadStart();
				paysys_new->Use();
			}
			

			paysys=paysys_new;
		}
		
		operat=operat_new;
		
		if (paysys==NULL)
		{
			/* I hope that will never happens, but whatever... */
//			std::cout << "Bad pay system or operator for pay id=" << pay->id << " (no *.so file?)" << std::endl;
			worker->FinishPay();
			sem_post(&payguide::free_workers_lock);
			DBSetState(pay, RESULT_NO_PAY_SYS,0, NULL, "null_d");
		}
		else
		{
//			paysys->Use();
//			operat->Use();
			worker_id=worker->GetId();
			
			//long long pay_id=pay->id;
			SPayResult general_pay_validate;
			general_pay_validate.code=RESULT_SUCESS;
			
			paysys->lock.LockRead();
			if (operat!=NULL)
				operat->lock.LockRead();
			
			if (validator!=NULL)
				validator->Use();
			sem_post(&payguide::free_workers_lock);
			
			/* Turn validator off */
			if (operat!=NULL && operat->GetConfig()!=NULL)
			{
				const char *tmp=operat->GetConfig()->GetValue("validator");
				if (tmp!=NULL)
				{
					if (strcmp(tmp,"off")==0)
						validator_off=true;
				}
			}
			
			/* Validate */
			if (validator!=NULL && validator_off==false)
			{
				if (operat!=NULL && operat->GetConfig()!=NULL)
				{
					const char *tmp=operat->GetConfig()->GetValue("validator_ignore_warnings");
					if (tmp!=NULL)
					{
						if (strcmp(tmp, "off")==0)
							validator_ignore_warnings=false;
					}
				}
				validator->lock.LockRead();
				pay->lock.LockWrite();
				void *validator_init_values=validator->InitPay(validator, operat, pay, worker_id);
				general_pay_validate=validator->SendPay(pay, validator_init_values, NULL);
				validator->CleanUp(validator_init_values);
				validator->lock.UnLockRead();
				pay->lock.UnLockWrite();
			}
			
			/* "Test" pay */
			if (general_pay_validate.code==RESULT_TEST)
			{
				if (operat!=NULL)
					operat->lock.UnLockRead();

				paysys->lock.UnLockRead();

				DBSetState(pay, &general_pay_validate);
			}
			/* Continue checking */
			else if (general_pay_validate.code==RESULT_SUCESS || general_pay_validate.code==RESULT_SAVE_STATE)
			{
			
				int white=2;
				int black=2;
			
				if (operat!=NULL)
				{
					white=ValidData(operat->GetWhiteFormat(), pay->data);
					black=ValidData(operat->GetBlackFormat(), pay->data);
				}
				
				/* OK */
				if (pay->test==OK_TEST)
				{
					if (operat!=NULL)
						operat->lock.UnLockRead();
					paysys->lock.UnLockRead();
					if (general_pay_validate.code==RESULT_SUCESS)
						db_err=DBSetState(pay, &general_pay_validate);
					else
						db_err=DBSetState(pay, &general_pay_validate);
				}
				/* NOK */
				else if (pay->test==NOK_TEST)
				{
					if (operat!=NULL)
						operat->lock.UnLockRead();
					paysys->lock.UnLockRead();
					db_err=DBSetState(pay, RESULT_FAILED, 0,  "Failed", "checker_d");
				}
				/* B/W lists only */
				else if (pay->test==FORMAT_TEST)
				{
					if (operat!=NULL)
						operat->lock.UnLockRead();
					paysys->lock.UnLockRead();
					if (white!=0 && black!=1)
					{
						/* Format test passed */
						if (general_pay_validate.code==RESULT_SUCESS)
							db_err=DBSetState(pay, RESULT_SUCESS, 0,  "Данные платежа соответствуют формату данных оператора", "checker_d");
						else
							db_err=DBSetState(pay, &general_pay_validate);
					}
					else
					{
					        /* Format test failed */
						db_err=DBSetState(pay, RESULT_FAILED, 0,  "Данные платежа не соответствуют формату данных оператора", "checker_d");
					}
				
				}
				/* Paycheck unsupported  */
				else if (pay->test==FULL_TEST && paysys->CheckSupport()!=1)
				{
					if (operat!=NULL)
						operat->lock.UnLockRead();
					paysys->lock.UnLockRead();
					db_err=DBSetState(pay, RESULT_ONLINECHECK_NOTSUPP, 0,  "Module doesn't support online-check", "checker_d");
				}
				/* Lets do our dirty job... */
				else 
				{
//							printf("white=%i\nblack=%i\n",white,black);
		
					if (white!=0 && black!=1)
					{
						if (validator_ignore_warnings==false && general_pay_validate.code==RESULT_SAVE_STATE)
						{
							db_err=DBSetState(pay,&general_pay_validate);
						}
						else
						{
						
//							std::cout << "Thread " << worker_id << " working" << std::endl;
		
							/* Convert data by perl script */
							if (pay->conv_sub!=NULL)
							{
								pay->lock.LockWrite();
								char new_data[1024];
								int conv_res=payguide::data_conventor.Exec(new_data,1024,pay->data,pay->conv_sub);
						
								if (conv_res==0)
								{
		//								printf("data converted sucessfully! New data=[%s]\n", new_data);
									strncpy(pay->data, new_data, SIZE_DATA);
								}
								else
								{
		//								printf("Data convertation failed!\n", new_data);
								}
								pay->lock.UnLockWrite();
							}
/*						
							if (validator!=NULL && pay->test!=NO_TEST)
							{
								general_pay_validate=validator->SendPay(pay, NULL, NULL);
							}
*/		
							/* Call pay module init */
							pay->lock.LockRead();
							void *init_values=paysys->InitPay(paysys, operat, pay, worker_id);
					
							if (operat!=NULL)
								operat->lock.UnLockRead();
	
								paysys->lock.UnLockRead();

							/* Register thread cleanup function */
							cud.paysys=paysys; cud.init_values=init_values; cud.pay=pay;
							
//							pthread_cleanup_push (PayKill, &cud);

			
							/* -!!!- Proceed our pay - call function from *.so module -!!!- */
							work_result=paysys->SendPay(pay, init_values, thread_init);
								
							/* Call CleanUp fucntion from *.so module */
							paysys->CleanUp(init_values);
							pay->lock.UnLockRead();
							/* Remove PayKill from stack without call it  */
//							pthread_cleanup_pop (0);
							
							if (work_result.code==RESULT_SUCESS && general_pay_validate.code==RESULT_SAVE_STATE && pay->test!=NO_TEST)
								work_result=general_pay_validate;

							/* Update local MySQL DB */
							db_err=DBSetState(pay, &work_result);


						}
					/* B/W lists test failed */
					}
					else
					{
						
						if (operat!=NULL)
							operat->lock.UnLockRead();
						paysys->lock.UnLockRead();
	
						db_err=DBSetState(pay, RESULT_FAILED, 0,  "Данные платежа не соответствуют формату данных оператора", "format_d");

					}
					
				}

			}
			/* Failed at validate stage */
			else
			{
				if (operat!=NULL)
					operat->lock.UnLockRead();
				paysys->lock.UnLockRead();
					
				//work_result=general_pay_validate;
				db_err=DBSetState(pay, &general_pay_validate);
			
			}

//			if (work_result.code==RESULT_CANCELED)
//				std::cout << "Thread " << worker->GetId() << " canceled." << std::endl;
			
			/* Update worker status */
			sem_wait(&payguide::free_workers_lock);
			if (validator!=NULL)
				validator->UnUse();

//			std::cout << "Thread " << worker->GetId() << " finished." << std::endl;
			if (paysys!=NULL) paysys->UnUse();
			if (operat!=NULL)operat->UnUse(pay->paysys_codename);
			worker->FinishPay();
			SetFreeWorker(worker);
			sem_post(&payguide::free_workers_lock);
			
			/* OK, it's done. Return and wait for new pay */
		}

	}
	ThreadExit(paysys, thread_init);
	return NULL;
}

static void PayKill(void *param)
{
	CleanUpData *cud=(CleanUpData *)param;
	cud->paysys->CleanUp(cud->init_values);
	DBSetState(cud->pay, RESULT_KILLED,0, NULL,"payguide");
	mysql_thread_end();
}

void ThreadExit(CPaySys *paysys, void *thread_init)
{
	if (paysys!=NULL)
	{
		paysys->ThreadEnd(thread_init);
		paysys->UnUse();
	}

	mysql_thread_end();
	pthread_exit(NULL);
}
