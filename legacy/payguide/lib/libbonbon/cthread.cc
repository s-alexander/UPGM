#include "cthread.h"
#include <stdlib.h>

bonbon::CThread::CThread()
{
    module=NULL;
    params=NULL;
    thread_id=0;
    finished=false;
    sem_init(&self_lock, 0, 1);
    pthread_create(&thread, NULL, &bonbon::ThreadExec, this);
}

bonbon::CThread::CThread(bonbon::CModule &m, bonbon::CThreadParam &p)
{
    module=&m;
    params=&p;
    thread_id=0;
    sem_init(&self_lock, 0, 1);
    pthread_create(&thread, NULL, &bonbon::ThreadExec, this);
}

bonbon::CThread::~CThread()
{
    pthread_join(thread, NULL);
}

void bonbon::CThread::SetThreadId(pid_t t_id)
{
    sem_wait(&self_lock);
    thread_id=t_id;
    sem_post(&self_lock);
}


void *bonbon::ThreadExec(void *data)
{
    void *result=NULL;
    if (data!=NULL)
    {
        bonbon::CThread *thread=(bonbon::CThread *) data;
        bonbon::CModule *module=thread->VGetModule();
        thread->thread_id=0;
        bonbon::CThreadParam *params=thread->VGetParams();
        if (module!=NULL && params!=NULL)
        {
            result=module->Execute(*params);
            thread->Finish();
            return result;
        }
        else
        {
            thread->Finish();
            return result;
        }
    }
    return result;
}

const bonbon::CModule *bonbon::CThread::GetModule()
{
    return module;
}

const bonbon::CThreadParam *bonbon::CThread::GetParams()
{
    return params;
}

bonbon::CModule *bonbon::CThread::VGetModule()
{
    return module;
}

bonbon::CThreadParam *bonbon::CThread::VGetParams()
{
    return params;
}

pid_t bonbon::CThread::GetID()
{
    return thread_id;
}

void bonbon::CThread::Finish()
{
    sem_wait(&self_lock);
    finished=true;
    sem_post(&self_lock);
}

bool bonbon::CThread::Finished()
{
    bool result;
    sem_wait(&self_lock);
    result=finished;
    sem_post(&self_lock);
    return result;
}
