/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <stdarg.h>
#include <stdio.h>

#if defined(_MSC_VER) && (_MSC_VER < 1900)

#include "vcos_types.h" /* For inline */

/* MSVC versions older than 2015 do not have snprintf
 * Implement using _vsnprintf_s
 * See http://stackoverflow.com/questions/2915672/snprintf-and-visual-studio-2010 */

static inline int msvc_vsnprintf(char *buf, size_t buflen, const char *fmt, va_list ap)
{
   int count = -1;
   if (buflen != 0)
      count = _vsnprintf_s(buf, buflen, _TRUNCATE, fmt, ap);
   if (count == -1)
      count = _vscprintf(fmt, ap);
   return count;
}
#define vsnprintf msvc_vsnprintf // VS2013 declares a vsnprintf with different behaviour.

static inline int snprintf(char *buf, size_t buflen, const char *fmt, ...)
{
   va_list ap;
   va_start(ap, fmt);
   int count = vsnprintf(buf, buflen, fmt, ap);
   va_end(ap);
   return count;
}

#endif
