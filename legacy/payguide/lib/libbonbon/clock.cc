#include <vector>
#include <algorithm>

#include <string>
#include <iostream>
#include <cstdio>

#include "clock.h"
#include "slockgraph.h"

#define CLOCK_MODE_SOFT 1
#define CLOCK_MODE_HARD 2

static int CheckLockGraph();
static std::vector<bonbon::SLockGraph> locks_graph;
static sem_t locks_graph_lock;
static int RequireLock(pid_t transaction, bonbon::CLock *lock, int lock_mode);
static int CatchLock(pid_t transaction, bonbon::CLock *lock);
static int ReleaseLock(pid_t transaction, bonbon::CLock *lock);
static int CheckLockGraph();

int bonbon::CLockInit()
{
    sem_init(&locks_graph_lock, 0, 1);
}

bonbon::CLock::CLock()
{
    
}

bonbon::CLock::~CLock()
{
    
}

int bonbon::CLock::HardLockRequire(pid_t locker)
{
    RequireLock(locker, this, CLOCK_MODE_HARD);
}

int bonbon::CLock::LockRegister(pid_t locker)
{
    CatchLock(locker, this);
}

int bonbon::CLock::LockUnRegister(pid_t locker)
{
    ReleaseLock(locker, this);
}

int bonbon::CLock::SoftLockRequire(pid_t locker)
{
    RequireLock(locker, this, CLOCK_MODE_SOFT);
}

int RequireLock(pid_t transaction, bonbon::CLock *lock, int lock_mode)
{
    int result=0;
    sem_wait(&locks_graph_lock);
    bonbon::SLockGraph new_point(transaction, lock, LOCKGRAPH_TYPE_REQ, lock_mode);
    locks_graph.push_back(new_point);
//    printf("RequireLock %p thread %i type %i\n",lock, transaction, lock_mode);
    result=CheckLockGraph();
    sem_post(&locks_graph_lock);
    return result;
}

int CatchLock(pid_t transaction, bonbon::CLock *lock)
{
//    printf("CatchLock %p by %i\n", lock, transaction);
    int result=0;
    sem_wait(&locks_graph_lock);
    std::vector<bonbon::SLockGraph>::iterator it;
    bonbon::SLockGraph new_point(transaction, lock, LOCKGRAPH_TYPE_CAT, 0);
    it=std::find(locks_graph.begin(), locks_graph.end(), new_point);
//  printf("CatchLock %p thread %i\n", lock, transaction);
    if (it!=locks_graph.end())
        it->type=LOCKGRAPH_TYPE_CAT;
    else
    {
//      printf("NOT FOUND???\n");
        exit(-2);
    }
    result=CheckLockGraph();
    sem_post(&locks_graph_lock);
    return result;
}

int ReleaseLock(pid_t transaction, bonbon::CLock *lock)
{
//  printf("ReleaseLock %p by %i\n", lock, transaction);
    sem_wait(&locks_graph_lock);
    std::vector<bonbon::SLockGraph>::iterator it;
    bonbon::SLockGraph new_point(transaction, lock,LOCKGRAPH_TYPE_CAT, 0);
    it=std::find(locks_graph.begin(), locks_graph.end(), new_point);
//  printf("ReleaseLock %p thread %i\n", lock, transaction);
    if (it!=locks_graph.end())
        locks_graph.erase(it);
    else
    {
//      printf("NOT FOUND???\n");
        exit(-2);
    }
    sem_post(&locks_graph_lock);
    return 0;
}

int CheckLockGraph()
{
    std::vector<bonbon::SLockGraph> edited_locks_graph=locks_graph;
    std::vector<bonbon::SLockGraph>::iterator it;
    bool some_erased=true;
    bool any_hard=false;
    while (some_erased)
    {
        some_erased=false;
        any_hard=false;
        for (it=edited_locks_graph.begin(); it<edited_locks_graph.end(); it++)
        {
            if (it->mode==CLOCK_MODE_HARD)
                any_hard=true;
            if (it->type==LOCKGRAPH_TYPE_CAT)
            {
                bool not_req=true;
                std::vector<bonbon::SLockGraph>::iterator it2;
                for (it2=edited_locks_graph.begin(); it2<edited_locks_graph.end(); it2++)
                {
                    if (it2->type==LOCKGRAPH_TYPE_REQ && it2!=it)
                    {
                        if (it2->transaction==it->transaction)
                        {
//                            if (it->mode==CLOCK_MODE_HARD || it2->mode==CLOCK_MODE_HARD)
                            {
                                not_req=false;
                            }
                        }
                    }

                }
                if (not_req)
                {
                    edited_locks_graph.erase(it);
                    some_erased=true;
                }
            }
            else if (it->type==LOCKGRAPH_TYPE_REQ)
            {
                bool can_swap=true;
                std::vector<bonbon::SLockGraph>::iterator it2;
                for (it2=edited_locks_graph.begin(); it2<edited_locks_graph.end(); it2++)
                {
                    if (it2->type==LOCKGRAPH_TYPE_CAT && it2!=it)
                    {
                        if (it2->lock==it->lock)
                        {
                            can_swap=false;
                        }

                    }
                }
                if (can_swap)
                {
                    it->type=LOCKGRAPH_TYPE_CAT;
                    some_erased=true;
                }
            }

        }
        
    }
    if (edited_locks_graph.begin()!=edited_locks_graph.end() && any_hard)
    {
        printf("Deadlock detected. Links list:\n");
        std::vector<bonbon::SLockGraph>::iterator it;
        for (it=edited_locks_graph.begin(); it<edited_locks_graph.end(); it++)
        {
            if (it->type==LOCKGRAPH_TYPE_REQ)
                printf("lock %p - thread %i (WAITING FOR A LOCK)\n", it->lock, it->transaction);
            else if (it->type==LOCKGRAPH_TYPE_CAT)
                printf("lock %p - thread %i (LOCKED)\n", it->lock, it->transaction);
        }
        std::string in;
        std::cin >> in;
        return 1;
    }
    return 0;
}
