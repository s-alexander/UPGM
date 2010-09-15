// 
// File:   bonbon.h
// Author: alex
//
// Created on 15 февраль 2008 г., 22:54
//
// BonBon is a C++ library for thread programming.
// It supports deadlocks detection, smart job spreader and more.
//
// Supported classes:
// CThread - POSIX thread abstraction,
// CModule - Thread algorithm abstraction,
// CThreadParam - thread(s) data abstraction,
// CMutex - very simple lock,
// CSemaphore - lock for writing and lock for reading,
// CBreak - break a thread until someone run it,
// CJob - micro task for your thread,
// CJobManager - spreads yours CJobs to treads.
//
//
//
//
//
//
//
//
//
//
#ifndef _bonbon_H
#define	_bonbon_H

#include "cjob.h"
#include "cmutex.h"
#include "csemaphore.h"
#include "cthread.h"
#include "cmodule.h"
#include "cbreak.h"
#include "clock.h"
#include "cjobmanager.h"

namespace bonbon
{
    int BonbonInit();
}
#endif	/* _bonbon_H */

