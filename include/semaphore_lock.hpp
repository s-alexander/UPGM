#ifndef PAYGUIDE_SEMAPHORE_LOCK
#define PAYGUIDE_SEMAPHORE_LOCK 1

#include <semaphore.h>


class SemaphoreLock
{
public:
	SemaphoreLock(sem_t & semaphore):sem_(semaphore) { ::sem_wait(& sem_); }
	~SemaphoreLock() throw() { ::sem_post(& sem_); }
private:
	sem_t & sem_;
};


#endif
