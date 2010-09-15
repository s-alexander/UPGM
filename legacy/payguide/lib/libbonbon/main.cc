//
// File:   main.cc
// Author: alex
//
// Created on 15 Февраль 2008 г., 21:06
//
// This is an example of usage of bonbon thread API.
//
//
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <cstdio>
#include <sys/syscall.h>
#include <unistd.h>


#define BONBON_MAIN_CPP
#include "bonbon.h"
#undef BONBON_MAIN_CPP

class CMyParam: public bonbon::CThreadParam
{
    public:
        CMyParam(){manager=NULL;};
        ~CMyParam(){};
        bonbon::CSemaphore lock1;
        bonbon::CSemaphore lock2;
        bonbon::CJobManager *manager;
};

class CMyModule: public bonbon::CModule
{
    public:
        CMyModule(){threadtype=0;}
        CMyModule(int tt){threadtype=tt;}
        ~CMyModule(){};
        void *Execute(bonbon::CThreadParam &params)
        {

            CMyParam &p=dynamic_cast<CMyParam &>(params);
            bool working=true;
            while (working)
            {
/*                p.manager->WaitForANewJob();
                bonbon::CJob *job=p.manager->GetJob();
                printf("I got a new job!!!\n");
                if (job->GetType()==CJOB_TYPE_KILLER)
                {
                    printf("OMG! I'm killed!\n");
                    working=false;
                }
                else
                {
                    printf("Working on job....\n");
                    sleep(rand()%10+1);
                }*/
                int type=rand()%2;
                if (type==1)
                {
                    printf("case 1\n");
                    p.lock1.LockRead();

                    //!
                    p.lock2.LockWrite();

                    printf("thread %i lock OK\n", syscall(SYS_gettid));
                    p.lock2.UnLockWrite();
                    p.lock1.UnLockRead();
                }
                else
                {
                    printf("case 2\n");
/*                    p.lock2.LockRead();
                    p.lock1.LockWrite();
                    printf("thread %i lock OK\n", syscall(SYS_gettid));
                    p.lock1.UnLockWrite();
                    p.lock2.UnLockRead();*/

                    p.lock1.LockRead();
                    p.lock2.LockWrite();
                    printf("thread %i lock OK\n", syscall(SYS_gettid));
                    p.lock2.UnLockWrite();
                    p.lock1.UnLockRead();
                }

                    /*p.lock1.LockRead();
                    p.lock2.LockWrite();
                    printf("thread %i lock OK\n", syscall(SYS_gettid));
                    p.lock2.UnLockWrite();
                    p.lock1.UnLockRead();*/

            }


            printf("Thread %i returning.\n",syscall(SYS_gettid));
            return NULL;
        }
    private:
        int threadtype;
};

bonbon::CJob *GetNewJob();

int main(int argc, char** argv)
{
    srand(time(NULL));
    {
        bonbon::BonbonInit();
        CMyModule module;
        CMyParam params;
        bool work=true;
        std::vector<bonbon::CThread *> threads;
        int threads_min=0;
        int threads_max=20;
        int max_in_queue=2;

        bonbon::CJobManager manager;
        params.manager=&manager;


        /*while (work)
        {
            bonbon::CJob *job=GetNewJob();

            if (job!=NULL)
                manager.PushUniqueJob(*job);

            if (threads.size()+max_in_queue<manager.GetJobsCount() && threads.size()<threads_max)
            {
                bonbon::CThread *new_thread=new bonbon::CThread(&module, &params);
                threads.push_back(new_thread);
                printf("thread added!\n");
            }
            else if (threads.size()>manager.GetJobsCount() && threads.size()>threads_min)
            {
                printf("Kill thread\n");
                bonbon::CJob killer;
                killer.MakeKiller();
                manager.PushUniqueJob(killer);

                bool deleted=false;
                while(!deleted)
                {
                    std::vector<bonbon::CThread *>::iterator it;
                    for (it=threads.begin(); it<threads.end(); it++)
                    {
                        if ((*it)->Finished())
                        {
                            deleted=true;
                            delete *it;
                            threads.erase(it);
                        }
                    }
                    if (!deleted)
                        sleep(1);
                }
            }
        }*/
        bonbon::CThread thread1(module, params);
        bonbon::CThread thread2(module, params);

        printf("Waiting for all threads to exit...\n");
    }
    printf("Exit\n");
    return (EXIT_SUCCESS);
}

bonbon::CJob *GetNewJob()
{
    static int c=10;
    bonbon::CJob *result=NULL;
    if (c>0)
    {
        result=new bonbon::CJob();
        c--;
    }
    else
        sleep(1);

    return result;
}
