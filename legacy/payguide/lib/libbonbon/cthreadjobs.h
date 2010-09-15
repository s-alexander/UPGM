// 
// File:   cthreadjobs.h
// Author: alex
//
// Created on 8 Март 2008 г., 0:08
//

#ifndef _CTHREADJOBS_H
#define	_CTHREADJOBS_H

#include <vector>

#include "cthread.h"
#include "cjob.h"

namespace bonbon
{
    class CThreadJobs
    {
        public:
            CThreadJobs(bonbon::CThread &t);
            ~CThreadJobs();
            int PushJob(bonbon::CJob &job);
            int RemoveJob(bonbon::CJob &job);
            std::vector<bonbon::CJob *> *GetJobs();
            bool operator<(const bonbon::CThreadJobs &cmp);
            bool operator==(const bonbon::CThreadJobs &cmp);
            bool operator==(bonbon::CThread &th);
            int GetJobsNum();
            pid_t GetID();
        private:
            std::vector<bonbon::CJob *> jobs;
            const CThread *thread;
            pid_t thread_id;

    };
}


#endif	/* _CTHREADJOBS_H */

