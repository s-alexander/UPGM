// 
// File:   csemaphore.h
// Author: alex
//
// Created on 15 Февраль 2008 г., 22:34
//

#ifndef _csemaphore_H
#define	_csemaphore_H

#include <vector>
#include "clock.h"

#define CSEMAPHORE_DRLOCK 10
#define CSEMAPHORE_DRUNLOCK 11
#define CSEMAPHORE_DWLOCK 12
#define CSEMAPHORE_DWUNLOCK 13
#define CSEMAPHORE_WRITE_LOCKED 14
#define CSEMAPHORE_DEADLOCK 15

namespace bonbon
{
    class CSemaphore: public CLock
    {
        public:
            CSemaphore();
            CSemaphore(unsigned int max_readers_num);
            ~CSemaphore();
            int LockRead();
            int UnLockRead();

            int LockWrite();
            int UnLockWrite();
        private:
            sem_t lock;
            unsigned int val;
            sem_t self_lock;
            sem_t write_lock;
            std::vector<pid_t> r_lockers;
            std::vector<pid_t> w_lockers;
    };
}
#endif	/* _csemaphore_H */

