/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  SpyHook
Module   :  Platform layer

FILE DESCRIPTION
Platform specific abstractions
=============================================================================*/

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

unsigned int plGetTimeNowMs()
{
   static unsigned int sTimeBase = 0;

   unsigned int nowMs;

   struct timespec now;

   clock_gettime(CLOCK_REALTIME, &now);

   nowMs = now.tv_sec * 1000;
   nowMs += now.tv_nsec / 1000000;

   if (sTimeBase == 0)
      sTimeBase = nowMs;

   return nowMs - sTimeBase;
}

uint64_t plGetTimeNowUs()
{
   static uint64_t sTimeBase = 0;

   unsigned int nowUs;

   struct timespec now;

   clock_gettime(CLOCK_REALTIME, &now);

   nowUs = now.tv_sec * 1000000;
   nowUs += now.tv_nsec / 1000;

   if (sTimeBase == 0)
      sTimeBase = nowUs;

   return nowUs - sTimeBase;
}

void plGetAccurateTime(unsigned int *secs, unsigned int *nanosecs)
{
   struct timespec   now;
   clock_gettime(CLOCK_REALTIME, &now);

   *secs = now.tv_sec;
   *nanosecs = now.tv_nsec;
}

void plGetTime(TIMESTAMP *now)
{
   clock_gettime(CLOCK_REALTIME, now);
}

unsigned int plTimeDiffNano(TIMESTAMP *start, TIMESTAMP *end)
{
   return ((end->tv_sec - start->tv_sec) * 1000000000) + (end->tv_nsec - start->tv_nsec);
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

unsigned int plGetTimeNowMs()
{
   return timeGetTime();
}

uint64_t plGetTimeNowUs()
{
   unsigned int msec = timeGetTime();

   return (uint64_t)msec * 1000;
}

void plGetAccurateTime(unsigned int *secs, unsigned int *nanosecs)
{
   unsigned int msec = timeGetTime();

   *secs = msec / 1000;
   *nanosecs = 1000 * (msec - (*secs * 1000));
}

void plGetTime(TIMESTAMP *now)
{
   *now = 0;   /* Not needed on Windows */
}

unsigned int plTimeDiffNano(TIMESTAMP *start, TIMESTAMP *end)
{
   return *end - *start;
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
