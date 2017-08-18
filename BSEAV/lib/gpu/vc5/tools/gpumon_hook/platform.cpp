/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "platform.h"

#ifndef WIN32

#include <pthread.h>
#include <stdio.h>

void plCreateMutex(MutexHandle *handle)
{
   pthread_mutex_init(handle, NULL);
}

void plDestroyMutex(MutexHandle *handle)
{
   pthread_mutex_destroy(handle);
}

void plLockMutex(MutexHandle *handle)
{
   pthread_mutex_lock(handle);
}

void plUnlockMutex(MutexHandle *handle)
{
   pthread_mutex_unlock(handle);
}

static pthread_once_t  sInitLockOnce;
static pthread_mutex_t sGlobalLock;
static unsigned int    sLastThreadId = 0;

extern const char *    gLastFuncName;
extern bool            gAPIReentrancy;

static void spyhook_init_lock()
{
   pthread_mutex_init(&sGlobalLock, NULL);
}

unsigned int plGetThreadID()
{
   return ((unsigned int)pthread_self());
}

// Returns true if the lock was taken, false if not
bool plGlobalLock(const char *funcName, bool *threadChanged)
{
   pthread_once(&sInitLockOnce, spyhook_init_lock);

   bool         lockedOk = true;
   unsigned int tid = plGetThreadID();

   if (pthread_mutex_trylock(&sGlobalLock) != 0)
   {
      // Couldn't get the mutex - must be already locked, either in another thread, or this one
      if (tid == sLastThreadId)
      {
         // Already locked in this thread - so this must be a re-entrant API call
#ifndef NDEBUG
         printf("****** Module API re-entrancy detected in API call (%s - from %s) - check driver code\n",
                funcName ? funcName : "null",
                gLastFuncName ? gLastFuncName : "null");
#endif
         // Set the global flag to prevent capture/trap of the inner re-entrant APIs
         gAPIReentrancy = true;

         // Return false so we don't try to unlock the mutex that we didn't take
         lockedOk = false;
      }
      else
      {
         // Different thread - just wait for the top-level mutex
         pthread_mutex_lock(&sGlobalLock);
         gAPIReentrancy = false;
      }
   }
   else
   {
      // We got the lock
      gAPIReentrancy = false;
   }

   *threadChanged = (tid != sLastThreadId && sLastThreadId != 0);
   sLastThreadId = tid;

   return lockedOk;
}

void plGlobalUnlock()
{
   pthread_mutex_unlock(&sGlobalLock);
}

#else

#include <windows.h>

void plCreateMutex(MutexHandle *handle)
{
   *handle = CreateMutex(0, FALSE, NULL);
}

void plDestroyMutex(MutexHandle *handle)
{
   CloseHandle(*handle);
}

void plLockMutex(MutexHandle *handle)
{
   WaitForSingleObject(*handle, INFINITE);
}

void plUnlockMutex(MutexHandle *handle)
{
   ReleaseMutex(*handle);
}

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
