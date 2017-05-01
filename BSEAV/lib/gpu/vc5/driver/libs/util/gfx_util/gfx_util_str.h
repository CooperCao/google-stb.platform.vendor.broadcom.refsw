/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include "vcos.h"
#include <stdbool.h>
#include "libs/util/demand.h"

/*
 * Variants of the stdlib.h strto*() functions that return an explicit error
 * flag (gfx_try_*) or abort (gfx_x*; akin to xmalloc) on failure. Being unable
 * to convert anything in the input counts as failure. Leading and trailing
 * whitespace (as defined by isspace()) is permitted.
 *
 * The gfx_x* functions are only intended for use in test applications and
 * debug code.
 */

static inline bool gfx_try_strtoll_in_range(long long *ll, const char *str, int base,
   long long min, long long max)
{
   long long ret;
   char *endptr;

   errno = 0;
   ret = strtoll(str, &endptr, base);
   if (errno || (endptr == str))
      return false;
   while (isspace(*endptr)) { ++endptr; }
   if (*endptr != '\0')
      return false;
   if ((ret < min) || (ret > max))
      return false;

   if (ll)
      *ll = ret;
   return true;
}

static inline bool gfx_try_strtoull_in_range(unsigned long long *ull, const char *str, int base,
   unsigned long long min, unsigned long long max)
{
   unsigned long long ret;
   char *endptr;

   errno = 0;
   ret = strtoull(str, &endptr, base);
   if (errno || (endptr == str))
      return false;
   while (isspace(*endptr)) { ++endptr; }
   if (*endptr != '\0')
      return false;
   if ((ret < min) || (ret > max))
      return false;

   if (ull)
      *ull = ret;
   return true;
}

#define GFX_STRTO_INT_FUNCS(TYPE, NAME, MIN, MAX, LARGE_TYPE, LARGE_NAME)           \
   static inline bool gfx_try_strto##NAME(TYPE *val, const char *str, int base)     \
   {                                                                                \
      LARGE_TYPE large_val = 0;                                                     \
      if (!gfx_try_strto##LARGE_NAME##_in_range(&large_val, str, base, MIN, MAX))   \
         return false;                                                              \
      if (val)                                                                      \
         *val = (TYPE)large_val;                                                    \
      return true;                                                                  \
   }                                                                                \
                                                                                    \
   static inline TYPE gfx_xstrto##NAME(const char *str, int base)                   \
   {                                                                                \
      TYPE val = 0;                                                                 \
      demand(gfx_try_strto##NAME(&val, str, base));                                 \
      return val;                                                                   \
   }

#define GFX_STRTO_SIGNED_INT_FUNCS(TYPE, NAME, MIN, MAX) \
   GFX_STRTO_INT_FUNCS(TYPE, NAME, MIN, MAX, long long, ll)
GFX_STRTO_SIGNED_INT_FUNCS(int, i, INT_MIN, INT_MAX)
GFX_STRTO_SIGNED_INT_FUNCS(long, l, LONG_MIN, LONG_MAX)
GFX_STRTO_SIGNED_INT_FUNCS(long long, ll, LLONG_MIN, LLONG_MAX)
GFX_STRTO_SIGNED_INT_FUNCS(int32_t, i32, INT32_MIN, INT32_MAX)
GFX_STRTO_SIGNED_INT_FUNCS(int64_t, i64, INT64_MIN, INT64_MAX)
#undef GFX_STRTO_SIGNED_INT_FUNCS

#define GFX_STRTO_UNSIGNED_INT_FUNCS_ULL(TYPE, NAME, MAX) \
   GFX_STRTO_INT_FUNCS(TYPE, NAME, 0, MAX, unsigned long long, ull)
GFX_STRTO_UNSIGNED_INT_FUNCS_ULL(unsigned int, ui, UINT_MAX)
GFX_STRTO_UNSIGNED_INT_FUNCS_ULL(unsigned long, ul, ULONG_MAX)
GFX_STRTO_UNSIGNED_INT_FUNCS_ULL(unsigned long long, ull, ULLONG_MAX)
GFX_STRTO_UNSIGNED_INT_FUNCS_ULL(uint32_t, u32, UINT32_MAX)
GFX_STRTO_UNSIGNED_INT_FUNCS_ULL(uint64_t, u64, UINT64_MAX)
#undef GFX_STRTO_UNSIGNED_INT_FUNCS_ULL

#undef GFX_STRTO_INT_FUNCS

static inline bool gfx_try_strtod(double *d, const char *str)
{
   double ret;
   char *endptr;

   errno = 0;
   ret = strtod(str, &endptr);
   if (errno || (endptr == str))
      return false;
   while (isspace(*endptr)) { ++endptr; }
   if (*endptr != '\0')
      return false;

   if (d)
      *d = ret;
   return true;
}

static inline double gfx_xstrtod(const char *str)
{
   double ret = 0.0;
   demand(gfx_try_strtod(&ret, str));
   return ret;
}

/* c89 infinities are a pain so I'm letting overflows through here */
static inline bool gfx_try_strtof(float *f_out, const char *str)
{
   double d = 0.0;
   if (!gfx_try_strtod(&d, str))
      return false;
   float f = (float)d;
   if ((f == 0.0f) && (d != 0.0))
      return false; /* underflow in the cast */
   if (f_out)
      *f_out = f;
   return true;
}

static inline float gfx_xstrtof(const char *str)
{
   float ret = 0.0f;
   demand(gfx_try_strtof(&ret, str));
   return ret;
}
