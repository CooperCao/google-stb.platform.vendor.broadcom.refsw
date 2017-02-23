/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.

Project  :  vcos
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef VCOS_LOGCAT_H
#define VCOS_LOGCAT_H

#include "interface/vcos/vcos_types.h"
#include <stdbool.h>

typedef enum
{
   LOG_ERROR,
   LOG_WARN,
   LOG_INFO,
   LOG_TRACE
} log_level_t;

extern bool vcos_of_interest_impl(void);
extern void vcos_log_impl(log_level_t log_level, const char *fmt, ...);

#ifdef NDEBUG

#ifndef vcos_of_interest
#define vcos_of_interest(cond) 0
#endif

#ifndef vcos_log
#define vcos_log(log_level, fmt, ...) (void)0
#endif

#else

#ifndef vcos_of_interest
#define vcos_of_interest(cond) vcos_of_interest_impl(cond)
#endif

#ifndef vcos_log
#define vcos_log(log_level, ...) vcos_log_impl(log_level, __VA_ARGS__)
#endif

#endif /* NDEBUG */

#endif /* VCOS_LOGCAT_H */
