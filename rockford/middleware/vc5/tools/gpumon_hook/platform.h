/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
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
#include <stdint.h>

#if defined(ANDROID)
#include <cutils/properties.h>
#include <cutils/log.h>
#else
// Back-tracing isn't available in the bionic C library
#include <execinfo.h>
#include <cxxabi.h>
#define ALOGD printf
#define ALOGE printf
#endif

typedef pthread_mutex_t MutexHandle;

#define DLLEXPORT __attribute__((visibility("default")))

typedef struct timespec TIMESTAMP;

#else

// Windows
#include <windows.h>
#include <process.h>
#include <stdint.h>

typedef HANDLE MutexHandle;

#define getpid _getpid

#define DLLEXPORT __declspec(dllexport)
typedef unsigned int TIMESTAMP;

#endif

// It seems that __FUNCTION__ isn't standard
#if !defined(__STDC__) || (__STDC_VERSION__ < 199901L)
# if (defined(__GNUC__) && (__GNUC__ >= 2)) || defined(__VIDEOCORE__) || defined(_MSC_VER)
#  define PL_FUNCTION __FUNCTION__
# else
#  define PL_FUNCTION "<unknown>"
# endif
#else
# define PL_FUNCTION __func__
#endif

extern void plCreateMutex(MutexHandle *handle);
extern void plDestroyMutex(MutexHandle *handle);
extern void plLockMutex(MutexHandle *handle);
extern void plUnlockMutex(MutexHandle *handle);

extern unsigned int plGetTimeNowMs();
extern uint64_t plGetTimeNowUs();
extern void plGetAccurateTime(unsigned int *secs, unsigned int *nanosecs);
extern void plGetTime(TIMESTAMP *now);
extern unsigned int plTimeDiffNano(TIMESTAMP *start, TIMESTAMP *end);

extern bool plGlobalLock(const char *funcName, bool *threadChanged);
extern void plGlobalUnlock();
extern unsigned int plGetThreadID();

#endif /* __PLATFORM_H__ */
