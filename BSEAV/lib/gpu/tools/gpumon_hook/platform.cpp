/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "platform.h"

#ifndef WIN32

#include <pthread.h>
#include <stdio.h>
#include <mutex>

static std::recursive_mutex  sGlobalLock;
static unsigned int          sLockedThreadId  = 0;
static unsigned int          sLastThreadId    = 0;
static unsigned int          sReentrancyCount = 0;

extern const char           *gLastFuncName;
extern bool                  gAPIReentrancy;

unsigned int plGetThreadID()
{
   return ((unsigned int)pthread_self());
}

// Returns true if the lock was taken, false if not
void plGlobalLock(const char *funcName, bool *threadChanged)
{
   sGlobalLock.lock();

   unsigned int tid = plGetThreadID();

   // Are we recursing in the same thread?
   if (tid == sLockedThreadId)
   {
      sReentrancyCount++;

      // Already locked in this thread - so this must be a re-entrant API call
#ifndef NDEBUG
      printf("****** Module API re-entrancy detected in API call (%s - from %s) - check driver code\n",
               funcName ? funcName : "null",
               gLastFuncName ? gLastFuncName : "null");
#endif
      // Set the global flag to prevent capture/trap of the inner re-entrant APIs
      gAPIReentrancy = true;
   }
   else
   {
      gAPIReentrancy = false;
   }

   *threadChanged  = (tid != sLastThreadId && sLastThreadId != 0);
   sLockedThreadId = tid;
   sLastThreadId   = tid;
}

void plGlobalUnlock()
{
   if (sReentrancyCount > 0)
      sReentrancyCount--;

   if (sReentrancyCount == 0)
      sLockedThreadId = 0;

   sGlobalLock.unlock();
}

#else

#include <windows.h>

// TODO - Windows platform funcs
#error "Missing some porting functions on Windows"
bool plGlobalLock(const char *funcName, bool *threadChanged)
{
   // TODO
}

void plGlobalUnlock()
{
   // TODO
}

unsigned int plGetThreadID()
{
   return ((unsigned int)GetCurrentThreadId());
}

#endif
