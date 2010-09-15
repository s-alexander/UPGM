// 
// File:   cgraph.h
// Author: alex
//
// Created on 7 Март 2008 г., 16:52
//

#ifndef _SGRAPH_H
#define	_SGRAPH_H

#define LOCKGRAPH_TYPE_REQ 1
#define LOCKGRAPH_TYPE_CAT 2

namespace bonbon
{
    class CLock;
    class SLockGraph
    {
        public:
            SLockGraph(pid_t trans, CLock *l, int lock_type, int lock_mode)
            {
                transaction=trans;
                lock=l;
                type=lock_type;
                mode=lock_mode;
            }
            ~SLockGraph()
            {
                
            }
            const SLockGraph &operator=(const SLockGraph &lg)
            {
                transaction=lg.transaction;
                lock=lg.lock;
                type=lg.type;
                return *this;
            }
            bool operator==(const SLockGraph &lg)
            {
                if (transaction==lg.transaction && lock==lg.lock)
                    return true;
                return false;
            }
        
            pid_t transaction;
            CLock *lock;
            int type;
            int mode;
    };
}

#endif	/* _CGRAPH_H */

