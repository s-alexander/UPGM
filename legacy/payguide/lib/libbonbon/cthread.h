// 
// File:   cthread.h
// Author: alex
//
// Created on 15 Февраль 2008 г., 22:31
//

#ifndef _cthread_H
#define	_cthread_H

#include <pthread.h>
#include "cmodule.h"
#include "ctparam.h"
#include "cjob.h"


namespace bonbon
{
    void *ThreadExec(void *data);
    class CThread
    {
        public:
            friend void *ThreadExec(void *data);
            CThread();
            CThread(bonbon::CModule &m);
            CThread(bonbon::CModule &m, bonbon::CThreadParam &p);
            ~CThread();
            const bonbon::CModule *GetModule();
            const bonbon::CThreadParam *GetParams();
            pid_t GetID();
            bool Finished();
        private:
            bonbon::CModule *VGetModule();
            bonbon::CThreadParam *VGetParams();
            sem_t self_lock;
            void SetThreadId(pid_t t_id);
            bonbon::CModule *module;
            CThreadParam *params;
            pthread_t thread;
            pid_t thread_id;
            bool finished;
            void Finish();
    };
}
#endif	/* _cthread_H */

