/*==============================================================================
 Copyright (c) 2012 Broadcom Europe Limited.
 All rights reserved.
==============================================================================*/

#ifndef GFX_UTIL_STR_H
#define GFX_UTIL_STR_H

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include "vcos.h"
#include <stdbool.h>
#include "helpers/demand.h"

/*
 * Variants of the stdlib.h strto*() functions that return an explicit error
 * flag (gfx_try_*) or abort (gfx_x*; akin to xmalloc) on failure. Being unable
 * to convert anything in the input counts as failure. Leading and trailing
 * whitespace (as defined by isspace()) is permitted.
 *
 * Also includes variants for types not in ANSI C89.
 *
 * The gfx_x* functions are only intended for use in test applications and
 * debug code.
 */

static inline long gfx_xstrtol(const char *str, int base)
{
   long ret;
   char *endptr;

   errno = 0;
   ret = strtol(str, &endptr, base);
   demand(!errno);
   demand(endptr != str);
   while (isspace(*endptr)) { ++endptr; }
   demand(*endptr == '\0');
   return ret;
}

static inline bool gfx_try_strtoul(unsigned long *ul,
   const char *str, int base)
{
   unsigned long ret;
   char *endptr;

   errno = 0;
   ret = strtoul(str, &endptr, base);
   if (errno || (endptr == str))
      return false;
   while (isspace(*endptr)) { ++endptr; }
   if (*endptr != '\0')
      return false;

   if (ul)
      *ul = ret;
   return true;
}

static inline unsigned long gfx_xstrtoul(const char *str, int base)
{
   unsigned long ret = 0;
   demand(gfx_try_strtoul(&ret, str, base));
   return ret;
}

static inline unsigned int gfx_xstrtoui(const char *str, int base)
{
   unsigned long l = gfx_xstrtoul(str, base);
   demand(l <= UINT_MAX);
   return (unsigned int)l;
}

static inline double gfx_xstrtod(const char *str)
{
   double ret;
   char *endptr;

   errno = 0;
   ret = strtod(str, &endptr);
   demand(!errno);
   demand(endptr != str);
   while (isspace(*endptr)) { ++endptr; }
   demand(*endptr == '\0');
   return ret;
}

/* c89 infinities are a pain so I'm letting overflows through here */
static inline float gfx_xstrtof(const char *str)
{
   double d = gfx_xstrtod(str);
   float f = (float)d;
   if (f == 0.0f) { demand(d == 0.0f); } /* underflow in the cast */
   return f;
}

#endif
