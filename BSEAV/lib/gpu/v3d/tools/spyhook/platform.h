/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#ifndef WIN32

// Unix
#include <sys/time.h>
#include <unistd.h>

#if !defined(ANDROID)
// Back-tracing isn't available in the bionic C library
#include <execinfo.h>
#include <cxxabi.h>
#endif

#define SPY_UNUSED __attribute__((unused))

#else

// Windows
#include <windows.h>
#include <process.h>

#define getpid _getpid

#define SPY_UNUSED

#define snprintf sprintf_s

#endif

extern unsigned int plGetTimeNowMs(void);
extern void plGetAccurateTime(unsigned int *secs, unsigned int *nanosecs);