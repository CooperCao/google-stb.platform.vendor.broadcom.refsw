/*=============================================================================
Broadcom Proprietary and Confidential. (c)2012 Broadcom.
All rights reserved.
=============================================================================*/

#include "platform.h"

#ifndef WIN32

#include <pthread.h>
#include "nexus_graphicsv3d.h"

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

unsigned int plGetTimeNowMs(void)
{
   static unsigned int sTimeBase = 0;
   uint64_t now;
   unsigned int nowMs;
   NEXUS_Graphicsv3d_GetTime(&now);
   nowMs = now / 1000;

   if (sTimeBase == 0)
      sTimeBase = nowMs;

   return nowMs - sTimeBase;
}

void plGetAccurateTime(unsigned int *secs, unsigned int *nanosecs)
{
   unsigned int s, ns;
   uint64_t t;
   NEXUS_Graphicsv3d_GetTime(&t);

   s = t / 1000000;
   ns = (t - ((uint64_t)s * 1000000)) * 1000;

   *secs = s;
   *nanosecs = ns;
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

void plGetAccurateTime(unsigned int *secs, unsigned int *nanosecs)
{
   unsigned int msec = timeGetTime();

   *secs = msec / 1000;
   *nanosecs = 1000 * (msec - (*secs * 1000));
}


#endif
