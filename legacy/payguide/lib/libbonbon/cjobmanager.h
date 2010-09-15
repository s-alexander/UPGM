// 
// File:   cjobmanager.h
// Author: alex
//
// Created on 8 Март 2008 г., 0:01
//

#ifndef _CJOBMANAGER_H
#define	_CJOBMANAGER_H

#include <queue>
#include <sys/types.h>

#include "cmutex.h"
#include "cthread.h"
#include "cbreak.h"
#include "cjob.h"

#define MAX_CJOBS_COUNT_IN_CJOBMANAGER 2147483647

namespace bonbon
{
    class CJobManager
    {
        public:
            CJobManager();
            ~CJobManager();
            int PushJob(bonbon::CJob &job);
            int PushUniqueJob(bonbon::CJob &job);
            int PushJob(bonbon::CJob &job, bool unique);
            bonbon::CJob *GetJob();
            int WaitForANewJob();
            int GetJobsCount();
            void Destroy();
            int LockQueue();
            int UnLockQueue();
        private:
            int jobs_tot;
            unsigned int threads_tot;
            sem_t self_lock;
            bonbon::CMutex queue_lock;
            std::queue<bonbon::CJob *> jobs;
            bonbon::CBreak main_break;        
    };
}

#endif	/* _CJOBMANAGER_H */

