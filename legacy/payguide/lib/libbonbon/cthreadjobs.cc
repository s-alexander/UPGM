#include <algorithm>

#include "cthreadjobs.h"

bonbon::CThreadJobs::CThreadJobs(bonbon::CThread &t)
{
    thread=&t;
    thread_id=t.GetID();
}

bonbon::CThreadJobs::~CThreadJobs()
{

}

int bonbon::CThreadJobs::PushJob(bonbon::CJob &job)
{
    jobs.push_back(&job);
    return 0;
}

int bonbon::CThreadJobs::RemoveJob(bonbon::CJob &job)
{
    std::vector<CJob *>::iterator it;
    CJob *job_ptr=&job;
    it=std::find(jobs.begin(), jobs.end(), job_ptr); 
    if (it!=jobs.end())
        jobs.erase(it);
    else
        return -1;
    return 0;
}

int bonbon::CThreadJobs::GetJobsNum()
{
    return jobs.size();
}

std::vector<bonbon::CJob *> *bonbon::CThreadJobs::GetJobs()
{
    return &jobs;
}

bool bonbon::CThreadJobs::operator<(const CThreadJobs &cmp)
{
    bool result=false;
    if (cmp.jobs.size()>jobs.size())
        result=true;
    return result;
}

bool bonbon::CThreadJobs::operator==(const bonbon::CThreadJobs &cmp)
{
    if (cmp.thread_id==thread_id)
        return true;
    return false;
}

pid_t bonbon::CThreadJobs::GetID()
{
    return thread_id;
}

bool bonbon::CThreadJobs::operator==(bonbon::CThread &th)
{
    if (th.GetID()==thread_id)
        return true;
    return false;
}
