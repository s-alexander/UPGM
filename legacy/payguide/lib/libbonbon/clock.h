// 
// File:   clock.h
// Author: alex
//
// Created on 7 Март 2008 г., 16:42
//

#ifndef _CLOCK_H
#define	_CLOCK_H

#include <sys/types.h>
#include <semaphore.h>

namespace bonbon
{
    int CLockInit();
    class CLock
    {
        public:
            CLock();
            ~CLock();
        protected:
            int HardLockRequire(pid_t locker);
            int SoftLockRequire(pid_t locker);
            int LockRegister(pid_t locker);
            int LockUnRegister(pid_t locker);
    private:
    };
}

#endif	/* _CLOCK_H */

