/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GFX_ARGS_H
#define GFX_ARGS_H

#include "helpers/v3d/v3d_common.h"
#include "helpers/gfx/gfx_lfmt.h"
#include <stdbool.h>
#include <stdio.h>

VCOS_EXTERN_C_BEGIN

struct gfx_args
{
   int c; /* argc */
   char **v; /* argv */
   int i; /* Index of next arg to process */

   void (*print_usage)(FILE *f, const char *argv0);
};

extern void gfx_args_begin(struct gfx_args *a,
   int argc, char **argv,
   void (*print_usage)(FILE *, const char *));
extern void gfx_args_end(struct gfx_args *a);

/* Prints the error message & the program usage message, then calls exit() */
extern void gfx_args_fail(const struct gfx_args *a,
   const char *fmt, ...);

/* Any args left to process? */
extern bool gfx_args_more(const struct gfx_args *a);

/* Return the next unprocessed arg, but don't step past it. If there are no
 * more args, gfx_args_fail() is called. */
extern const char *gfx_args_peek(const struct gfx_args *a);

/* Like peek(), but step past the arg */
extern const char *gfx_args_pop(struct gfx_args *a);

/* These return true iff there are more args to process and the next arg can be
 * parsed as the specified type */
extern bool gfx_args_can_pop_uint32(const struct gfx_args *a);
extern bool gfx_args_can_pop_addr(const struct gfx_args *a);

/* These call gfx_args_fail() if there are no more args to process or the next
 * arg cannot be parsed as the specified type. Otherwise they return the next
 * arg parsed as the specified type. */
extern float gfx_args_pop_float(struct gfx_args *a);
extern uint32_t gfx_args_pop_uint32(struct gfx_args *a);
extern v3d_addr_t gfx_args_pop_addr(struct gfx_args *a);
/* m+<offset> --> mem_begin_addr + <offset>
 * +<offset> --> begin_addr + <offset> (only if is_end_addr)
 * Will call gfx_args_fail() if addr is outside mem_begin_addr..mem_end_addr,
 * or if is_end_addr and addr is before begin_addr. */
extern v3d_addr_t gfx_args_pop_addr_fancy(struct gfx_args *a,
   v3d_addr_t mem_begin_addr, v3d_addr_t mem_end_addr,
   bool is_end_addr, v3d_addr_t begin_addr);
extern bool gfx_args_pop_bool(struct gfx_args *a);
extern GFX_LFMT_T gfx_args_pop_lfmt(struct gfx_args *a);

VCOS_EXTERN_C_END

#endif
