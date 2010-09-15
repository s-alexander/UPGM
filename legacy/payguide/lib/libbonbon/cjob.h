// 
// File:   cjob.h
// Author: alex
//
// Created on 15 Февраль 2008 г., 22:32
//

#ifndef _cjob_H
#define	_cjob_H

#include "csemaphore.h"
#include "cmodule.h"

#define CJOB_TYPE_KILLER 1000
#define CJOB_TYPE_VOID 1001

namespace bonbon
{
    class CJobManager;
    class CJob
    {
        public:
            friend class bonbon::CJobManager;
            CJob();
            CJob(void *user_data);
            ~CJob();
            const bonbon::CJob &operator=(const CJob &job);
            int GetType();
            int MakeKiller();
         private:
             const bonbon::CModule *GetModule();
             int job_type;
    };
}
#endif	/* _cjob_H */

