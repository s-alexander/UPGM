#ifndef __Sem__
#define __Sem__
#include <pthread.h>
#include <semaphore.h>
class CSemaphore
{
	public:
		CSemaphore();
		~CSemaphore();
		
		int LockRead();
		int LockWrite();
		
		int UnLockRead();
		int UnLockWrite();
	private:
		sem_t main_lock;
		sem_t self_lock;
};

#endif
