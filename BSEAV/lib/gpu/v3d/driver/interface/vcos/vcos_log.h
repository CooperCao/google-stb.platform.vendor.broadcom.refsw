/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "interface/vcos/vcos_types.h"
#include <stdbool.h>

typedef enum
{
   LOG_ERROR,
   LOG_WARN,
   LOG_INFO,
   LOG_TRACE
} log_level_t;

extern void vcos_log_impl(log_level_t log_level, const char *fmt, ...);

#ifdef NDEBUG

#ifndef vcos_log
#define vcos_log(log_level, fmt, ...) (void)0
#endif

#else

#ifndef vcos_log
#define vcos_log(log_level, ...) vcos_log_impl(log_level, __VA_ARGS__)
#endif

#endif /* NDEBUG */
