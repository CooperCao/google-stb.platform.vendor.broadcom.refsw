/*=============================================================================
Copyright (c) 2012 Broadcom Europe Limited.
All rights reserved.

Project  :  SpyHook
Module   :  Platform layer

FILE DESCRIPTION
Platform specific abstractions
=============================================================================*/

#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#ifndef WIN32

// Unix
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>

#if !defined(ANDROID)
// Back-tracing isn't available in the bionic C library
#include <execinfo.h>
#include <cxxabi.h>
#endif

typedef pthread_mutex_t MutexHandle;

#define SPY_UNUSED __attribute__((unused))

#else

// Windows
#include <windows.h>
#include <process.h>

typedef HANDLE MutexHandle;

#define getpid _getpid

#define SPY_UNUSED

#define snprintf sprintf_s

#endif

extern void plCreateMutex(MutexHandle *handle);
extern void plDestroyMutex(MutexHandle *handle);
extern void plLockMutex(MutexHandle *handle);
extern void plUnlockMutex(MutexHandle *handle);

extern unsigned int plGetTimeNowMs(void);
extern void plGetAccurateTime(unsigned int *secs, unsigned int *nanosecs);

#endif /* __PLATFORM_H__ */
