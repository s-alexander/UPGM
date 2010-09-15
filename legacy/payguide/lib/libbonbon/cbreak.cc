#include <sys/syscall.h>
#include <unistd.h>
#include <algorithm>
#include "cbreak.h"

bonbon::CBreak::CBreak()
{
    sem_init(&lock, 0,0);
    sem_init(&self_lock, 0,1);
    destroyed=false;
}

bonbon::CBreak::~CBreak()
{
    Destroy();
}

int bonbon::CBreak::Wait()
{
    sem_wait(&self_lock);

    if (destroyed)
    {
        sem_post(&self_lock);
        return CBREAK_DESTROYED;
    }

    pid_t locker=syscall(SYS_gettid);
    waiters.push_back(locker);
    sem_post(&self_lock);

    sem_wait(&lock);

    sem_wait(&self_lock);
    std::vector<pid_t>::iterator it;
    it=std::find(waiters.begin(), waiters.end(), locker);
    if (it!=waiters.end())
        waiters.erase(it);

    if (destroyed)
    {
        sem_post(&self_lock);
        return CBREAK_DESTROYED;
    }

    sem_post(&self_lock);
    return 0;
}

int bonbon::CBreak::Run()
{
    sem_post(&lock);
    return 0;
}

int bonbon::CBreak::Destroy()
{
    sem_wait(&self_lock);
    if (destroyed)
    {
        sem_post(&self_lock);
        return CBREAK_DESTROYED;
    }
    else
    {
        destroyed=true;
        std::vector<pid_t>::iterator it=waiters.begin();
        while (it!=waiters.end())
        {
            sem_post(&lock);
            it++;
        }
        sem_post(&self_lock);

    }
    return 0;
}
