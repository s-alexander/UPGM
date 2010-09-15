#ifndef __Worker__
#define __Worker__
#include "settings.h"
#include "paysys.h"
#include "pay.h"
#include <pthread.h>
#include <semaphore.h>
class CWorker;

//void WaitForAllPaysToFinish(bool cancel_pays);
//void ManagerEx();

/* Create a list of num workers */
CUlist<CWorker> *CreateWorkers(int num);

/* if NULL value passed, finds a free worker or create one,
or return non-zero if threads limit hit. 
Otherwise set a free_worker as worker */
int SetFreeWorker(CWorker *worker); 

/* Init a free_worker with a pay (adjust right PaySys) */
CWorker *InitFreeWorker(SPay *pay);

/* Kill pay by worker's ID */
int KillWorker(long long pay_id);

/* Cancel pay by worker's ID */
int CancelWorker(long long pay_id);

/* Cancel all pays */
int CancelAllWorkers();

class CWorker
{
	public:
		/* Constructor */
		CWorker();
		
		/* Destructor */
		~CWorker();
		
		/* Adjust PaySys */
		int SetPaySys(CPaySys *new_paysys);
		
		/* Returns  "paysys" field */
		CPaySys *GetPaySys();
		
		COperator *GetOperator();
		int SetOperator(COperator *new_op);
		
		/* Unlock thread (post_sem(&sem)) and begin to work */
		void Run();
		
		/* Wait for thread unlock (wait_sem(&sem)) */
		void Wait();
		
		/* Get worker ID */
		int GetId();
		
		/* Get "thread" field */
		pthread_t *GetThread();
		
		/* Adjust pay (set pay filed) */
		int AddPay(SPay *new_pay);
		
		/* Returns 1 if thread works on pay (pay!=NULL)  */
		int Busy(); 
		
		/* Kill pay. NOTE: this fucntion is VERY strong and can be cause of memory leacks! */
		void Kill();
		
		/* Set cancel_pay field in worker's pay to 1 */
		int CancelPay();
		
		/* Increase lifetime */
		void Live();
		
		/* Returns lifetime value */
		int GetLifeTime();
		
		/* Return "pay" field */
		SPay *GetPay();

		void KillSelf();
		
		int Killed();


	private:
		friend void * WorkEx(void *param);
		friend int SetFreeWorker(CWorker *worker);
		int Detach();
		int FinishPay();
		int lifetime;
		bool detached;
		CPaySys *paysys;
		COperator *operat;
		const char *engine_name;
		int work_state;
		int id;
		SPay *pay;
		int killed;
		pthread_t thread;
		sem_t sem;

};


#endif

