#include <sys/syscall.h>
#include <unistd.h>
#include <cstdio>
#include <algorithm>
#include "cmutex.h"

bonbon::CMutex::CMutex()
{
    sem_init(&lock, 0,1);
    sem_init(&self_lock, 0,1);
}

bonbon::CMutex::~CMutex()
{
    sem_wait(&self_lock);

    std::vector<pid_t>::iterator it;
    if (lockers_threads.begin()!=lockers_threads.end())
    {
        printf("CMutex %p destroyed, but some threads still are locking it:\n", this);
        for (it=lockers_threads.begin() ; it < lockers_threads.end(); it++ )
        {
            printf(" * Thread id %i\n", *it);
        }
        printf("Looks like you forgotten to call UnLock() function somewhere.\n");
    }

    sem_post(&self_lock);

}

int bonbon::CMutex::Lock()
{
    std::vector<pid_t>::iterator it;
    pid_t locker=syscall(SYS_gettid);

    sem_wait(&self_lock);
    it=std::find(lockers_threads.begin(), lockers_threads.end(), locker);
    if (it!=lockers_threads.end())
    {
        sem_post(&self_lock);
        printf("Possible deadlock by thread %i in CMutex located at %p", locker, this);
        return CMUTEX_DEADLOCK;
    }
    else
    {
        lockers_threads.push_back(locker);
        sem_post(&self_lock);
        HardLockRequire(locker);
        sem_wait(&lock);
        LockRegister(locker);
    }
    return 0;
}

int bonbon::CMutex::UnLock()
{
    std::vector<pid_t>::iterator it;
    pid_t locker=syscall(SYS_gettid);

    sem_wait(&self_lock);
    it=std::find(lockers_threads.begin(), lockers_threads.end(), locker);

    if (it==lockers_threads.end())
    {
        sem_post(&self_lock);
        printf("Possible double unlock by thread %i in CMutex located at %p", locker, this);
        return CMUTEX_DUNLOCK;
    }
    else
    {
        lockers_threads.erase(it);
        sem_post(&self_lock);

        LockUnRegister(locker);
        sem_post(&lock);
    }
    return 0;

}
