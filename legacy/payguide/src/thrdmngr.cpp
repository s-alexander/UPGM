#include "thrdmngr.h"
#include "paysys.h"
#include "log.h"
#include "namespace.h"

void ManagerEx()
{
	if (payguide::thread_autodelete==0) return;
	sem_wait(&payguide::free_workers_lock);
	
	/* Calling "void Call()" function from all modules */
	if (payguide::modules_list!=NULL)
	{
		payguide::modules_list->ResetCursor();
		for (unsigned int i=0; i<payguide::modules_list->GetLen(); i++)
		{
			CPaySys *tmp=payguide::modules_list->GetNext();
			if (tmp!=NULL)
				tmp->Call();
		}
	}

	
	if (payguide::need_cleanup_modules==1)
	{
		RemoveUnloadedModules();
		payguide::need_cleanup_modules=0;
	
	}

	int lifetime=payguide::thread_inactivity_time;
	if (payguide::workers_list!=NULL)
	{
		payguide::workers_list->ResetCursor();
		unsigned int l=payguide::workers_list->GetLen();
		for (unsigned int i=0; i<l; i++)
		{
			CWorker *worker=payguide::workers_list->GetNext();
			if (worker!=NULL)
			{
				/* All actions are protected by payguide::free_workers_lock mutex, so we are safe now. */
				worker->Live();
				if (worker->Busy()==0)
				{
					if (worker->GetLifeTime()>lifetime)
					{
						
						if (l>payguide::thread_min)
						{
							if (payguide::free_worker==worker) payguide::free_worker=NULL;
//							char logmsg[101]; snprintf(logmsg, 100,"Thread %i dropped cause of innactivity", worker->GetId());SendLogMessages(0, logmsg);LogWrite(LOGMSG_NORMAL, logmsg);
							worker->CancelPay();
							worker->KillSelf();
							worker->Run();
							pthread_join(*(worker->GetThread()), NULL);
							payguide::workers_list->RemoveThis();
							worker=NULL;
							l--;
							i--;
						}
					}
				}
			}
		}
	}	
	sem_post(&payguide::free_workers_lock);
	
	return;
}

void WaitForAllPaysToFinish(bool cancel_pays)
{
	unsigned int l=1;
	while (l!=0)
	{
		sem_wait(&payguide::free_workers_lock);
		l=payguide::workers_list->GetLen();
		payguide::workers_list->ResetCursor();
//		sem_post(&payguide::free_workers_lock);
//		for (unsigned int i=0; i<l; i++)
		{
//			sem_wait(&payguide::free_workers_lock);
			CWorker *w=payguide::workers_list->GetNext();
			if (w!=NULL)
			{
				if (cancel_pays)
				{
					w->CancelPay();
					w->KillSelf();
					w->Run();
					sem_post(&payguide::free_workers_lock);
					pthread_join(*(w->GetThread()), NULL);
				}
				else
				{
					w->KillSelf();
					w->Run();
					sem_post(&payguide::free_workers_lock);
					pthread_join(*(w->GetThread()), NULL);
					
				}
				
				sem_wait(&payguide::free_workers_lock);
				payguide::workers_list->ResetCursor();
				if (payguide::workers_list->GetLen()>0)
				{
					CWorker *w=payguide::workers_list->GetNext();
					payguide::workers_list->RemoveThis();
				}
				sem_post(&payguide::free_workers_lock);
//				l--; i--;

			}
			else
				sem_post(&payguide::free_workers_lock);
		}
	}
	char log[128];
	snprintf(log, 128, "All threads finished (%u threads left)\n", payguide::workers_list->GetLen());
	LogWrite(LOGMSG_SYSTEM, log);
	
	sem_wait(&payguide::free_workers_lock);
	delete payguide::workers_list;
	payguide::workers_list=NULL;
	sem_post(&payguide::free_workers_lock);
	
	return;
}
