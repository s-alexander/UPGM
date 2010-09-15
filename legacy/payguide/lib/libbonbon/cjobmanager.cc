#include <algorithm>
#include <cstdio>

#include "cjobmanager.h"


bonbon::CJobManager::CJobManager()
{
    jobs_tot=0;
    threads_tot=0;
    sem_init(&self_lock, 0, 1);
}

bonbon::CJobManager::~CJobManager()
{

}

int bonbon::CJobManager::GetJobsCount()
{
    int result;
    sem_wait(&self_lock);
    result=jobs_tot;
    sem_post(&self_lock);
    return result;
}
int bonbon::CJobManager::LockQueue()
{
    return queue_lock.Lock();
}

int bonbon::CJobManager::UnLockQueue()
{
    return queue_lock.UnLock();
}
int bonbon::CJobManager::PushJob(bonbon::CJob &job)
{
    return PushJob(job, false);
}

int bonbon::CJobManager::PushUniqueJob(bonbon::CJob &job)
{
    return PushJob(job, true);
}

int bonbon::CJobManager::PushJob(bonbon::CJob &job, bool control_unique)
{
    sem_wait(&self_lock);
    if (jobs_tot<MAX_CJOBS_COUNT_IN_CJOBMANAGER)
    {
        jobs_tot++;

/*        if (control_unique)
        {
            std::vector<bonbon::CJob *>::iterator it=std::find(jobs.begin(), jobs.end(), &job);
            if (jobs.end()==it)
                jobs.push_back(&job);
            else
            {
                sem_post(&self_lock);
                main_break.Run();
                return 0;

            }
        }
        else*/
            jobs.push(&job);

        sem_post(&self_lock);
        main_break.Run();
        printf("Job pushed\n");
        return 0;
    }
    sem_post(&self_lock);
    return -1;
}

/*int bonbon::CJobManager::RemoveJob(bonbon::CThread &thread, bonbon::CJob &job)
{
    bonbon::CThreadJobs new_thread(thread);
    std::vector<bonbon::CThreadJobs>::iterator it=std::find(threads_and_jobs.begin(), threads_and_jobs.end(), new_thread);
    int result=it->RemoveJob(job);
    if (result==0 && jobs_tot!=0)
        jobs_tot--;
    return result;
}*/

bonbon::CJob *bonbon::CJobManager::GetJob()
{
    CJob *result=NULL;
    sem_wait(&self_lock);
//    if (jobs.end()!=jobs.begin())
    if (jobs_tot>0)
    {
//        std::vector<bonbon::CJob *>::iterator it=jobs.end()-1;
        result=jobs.front();
        jobs.pop();
        //jobs.erase(it);
        jobs_tot--;
    }
    sem_post(&self_lock);
    return result;
}


int bonbon::CJobManager::WaitForANewJob()
{
    main_break.Wait();
    return 0;
}

void bonbon::CJobManager::Destroy()
{
    main_break.Destroy();
}
