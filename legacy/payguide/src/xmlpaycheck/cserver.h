// 
// File:   cserver.h
// Author: alex
//
// Created on 13 Март 2008 г., 4:30
//

#ifndef _CSERVER_H
#define	_CSERVER_H

#include "../../lib/libbonbon/cmodule.h"
#include "csocket.h"

#define POST_MESSAGE_LIMIT 4096

namespace paycheck
{
    class CServer: public bonbon::CModule
    {
        public:
            CServer();
            ~CServer();
            void *Execute(bonbon::CThreadParam &params);
	    void Kill();
	    bool Killed();
        private:
            int threadtype;
	    bool killed;
	    bonbon::CMutex kill_lock;
    };
}

#endif	/* _CSERVER_H */

