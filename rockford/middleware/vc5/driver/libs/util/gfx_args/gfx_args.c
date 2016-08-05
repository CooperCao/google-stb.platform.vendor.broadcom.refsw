/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "gfx_args.h"
#include "libs/util/gfx_util/gfx_util_str.h"
#include "libs/core/v3d/v3d_util.h"
#include <stdarg.h>
#include <stdlib.h>

void gfx_args_begin(struct gfx_args *a,
   int argc, char **argv,
   void (*print_usage)(FILE *, const char *))
{
   a->c = argc;
   a->v = argv;
   a->i = 1;

   a->print_usage = print_usage;
}

void gfx_args_end(struct gfx_args *a)
{
   if (gfx_args_more(a))
      gfx_args_fail(a, "Too many arguments!");
}

void gfx_args_fail(const struct gfx_args *a,
   const char *fmt, ...)
{
   va_list args;

   va_start(args, fmt);
   vfprintf(stderr, fmt, args);
   va_end(args);
   fprintf(stderr, "\n");
   if (a->print_usage)
   {
      fprintf(stderr, "\n");
      a->print_usage(stderr, a->v[0]);
   }

   exit(1);
}

bool gfx_args_more(const struct gfx_args *a)
{
   return a->i < a->c;
}

const char *gfx_args_peek(const struct gfx_args *a)
{
   if (!gfx_args_more(a))
      gfx_args_fail(a, "Expected more arguments!");
   return a->v[a->i];
}

const char *gfx_args_pop(struct gfx_args *a)
{
   const char *s = gfx_args_peek(a);
   ++a->i;
   return s;
}

bool gfx_args_can_pop_uint32(const struct gfx_args *a)
{
   return gfx_args_more(a) && gfx_try_strtou32(NULL, gfx_args_peek(a), 0);
}

bool gfx_args_can_pop_addr(const struct gfx_args *a)
{
   return gfx_args_can_pop_uint32(a);
}

static float parse_float(struct gfx_args *a, const char *s)
{
   float f = 0.0f;
   if (!gfx_try_strtof(&f, s))
      gfx_args_fail(a, "Couldn't parse '%s' as a float!", s);
   return f;
}

static uint32_t parse_uint32(struct gfx_args *a, const char *s)
{
   uint32_t u = 0;
   if (!gfx_try_strtou32(&u, s, 0))
      gfx_args_fail(a, "Couldn't parse '%s' as a uint32!", s);
   return u;
}

static uint64_t parse_uint64(struct gfx_args *a, const char *s)
{
   uint64_t u = 0;
   if (!gfx_try_strtou64(&u, s, 0))
      gfx_args_fail(a, "Couldn't parse '%s' as a uint64!", s);
   return u;
}

float gfx_args_pop_float(struct gfx_args *a)
{
   return parse_float(a, gfx_args_pop(a));
}

uint32_t gfx_args_pop_uint32(struct gfx_args *a)
{
   return parse_uint32(a, gfx_args_pop(a));
}

uint64_t gfx_args_pop_uint64(struct gfx_args *a)
{
   return parse_uint64(a, gfx_args_pop(a));
}

v3d_addr_t gfx_args_pop_addr(struct gfx_args *a)
{
   return gfx_args_pop_uint32(a);
}

v3d_addr_t gfx_args_pop_addr_fancy(struct gfx_args *a,
   v3d_addr_t mem_begin_addr, v3d_addr_t mem_end_addr,
   bool is_end_addr, v3d_addr_t begin_addr)
{
   const char *s = gfx_args_pop(a);

   v3d_addr_t base_addr = 0;
   const char *offset = s;
   if (s[0] == 'm')
   {
      if (s[1] != '+')
         gfx_args_fail(a, "Badly formatted address '%s'!", s);
      base_addr = mem_begin_addr;
      offset = s + 2;
   }
   else if (s[0] == '+')
   {
      if (!is_end_addr)
         gfx_args_fail(a, "+<offset> format only valid for end address! ('%s')", s);
      base_addr = begin_addr;
      offset = s + 1;
   }

   v3d_addr_t addr = v3d_addr_offset(base_addr, parse_uint32(a, offset));

   if ((addr < mem_begin_addr) || (addr > mem_end_addr))
      gfx_args_fail(a, "0x%08x ('%s') is not within usable memory range 0x%08x..0x%08x",
         addr, s, mem_begin_addr, mem_end_addr);

   if (is_end_addr && (addr < begin_addr))
      gfx_args_fail(a, "End address 0x%08x ('%s') before begin address 0x%08x!",
         addr, s, begin_addr);

   return addr;
}

bool gfx_args_pop_bool(struct gfx_args *a)
{
   uint32_t u = gfx_args_pop_uint32(a);
   if (u > 1)
      gfx_args_fail(a, "Expected bool (0 or 1), got %" PRIu32 "!", u);
   return !!u;
}

GFX_LFMT_T gfx_args_pop_lfmt(struct gfx_args *a)
{
   const char *s = gfx_args_pop(a);
   GFX_LFMT_T lfmt;
   if (!gfx_lfmt_maybe_from_desc(&lfmt, s))
      gfx_args_fail(a, "Unrecognised lfmt '%s'!", s);
   return lfmt;
}
