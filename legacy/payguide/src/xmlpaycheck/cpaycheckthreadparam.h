// 
// File:   cpaycheckthreadparam.h
// Author: alex
//
// Created on 13 Март 2008 г., 4:25
//

#ifndef _CPAYCHECKTHREADPARAM_H
#define	_CPAYCHECKTHREADPARAM_H

#include "../../lib/libbonbon/ctparam.h"
#include "../../lib/libbonbon/cmutex.h"
#include "../../lib/libbonbon/cjobmanager.h"

namespace paycheck
{
    class CSocket;
    class CPaycheckCore;
    class CPaycheckThreadParam: public bonbon::CThreadParam
    {
        public:
            //CPaycheckThreadParam();
	    CPaycheckThreadParam(bonbon::CJobManager *in_m, std::vector <paycheck::CSocket *> *connections_s, CPaycheckCore *paycheck_c);
            ~CPaycheckThreadParam();
            bonbon::CMutex jobs_lock;
//	protected:
            bonbon::CJobManager *in_manager;
	    std::vector <paycheck::CSocket *> *connections_store;
	    CPaycheckCore *paycheck_core;
    };
}

#endif	/* _CPAYCHECKTHREADPARAM_H */

