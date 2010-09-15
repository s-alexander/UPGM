#include "sem.h"
#define PARALLEL_READERS 10
CSemaphore::CSemaphore()
{
	sem_init(&main_lock, 0,PARALLEL_READERS);
	sem_init(&self_lock, 0,1);
}

CSemaphore::~CSemaphore()
{

}

int CSemaphore::LockRead()
{
	sem_wait(&main_lock);
	return 0;
}

int CSemaphore::UnLockRead()
{
	sem_post(&main_lock);
	return 0;
}

int CSemaphore::LockWrite()
{
	sem_wait(&self_lock);
	for (int i=0; i<PARALLEL_READERS;i++)
	{
		sem_wait(&main_lock);
	}
	sem_post(&self_lock);
	return 0;
}

int CSemaphore::UnLockWrite()
{
	for (int i=0; i<PARALLEL_READERS;i++)
	{
		sem_post(&main_lock);
	}
	return 0;
}


