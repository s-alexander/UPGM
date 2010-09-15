// 
// File:   cmodule.h
// Author: alex
//
// Created on 15 Февраль 2008 г., 23:06
//

#ifndef _cmodule_H
#define	_cmodule_H

#include <vector>
//#include "cjob.h"
#include "csemaphore.h"
//#include "cthread.h"
#include "ctparam.h"

namespace bonbon
{
    class CJob;
    class CModule
    {
        public:
            CModule();
            virtual ~CModule();
            virtual void *Execute(bonbon::CThreadParam &params);
        private:
        protected:
    };
}

#endif	/* _cmodule_H */
