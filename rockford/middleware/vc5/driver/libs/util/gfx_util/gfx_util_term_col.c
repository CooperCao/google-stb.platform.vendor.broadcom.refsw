/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "gfx_util_term_col.h"
#include "libs/util/assert_helpers.h"
#include "vcos_string.h"

#ifdef WIN32

/* TODO Could use SetConsoleTextAttribute()... */

bool gfx_term_col_ok(FILE *f)
{
   return false;
}

void gfx_term_col_set_fg(FILE *f, gfx_term_col_t color)
{
   not_impl();
}

void gfx_term_col_set_bg(FILE *f, gfx_term_col_t color)
{
   not_impl();
}

void gfx_term_col_reset(FILE *f)
{
   not_impl();
}

size_t gfx_term_col_sprint_set_fg(char *buf, size_t buf_size, size_t offset,
   gfx_term_col_t color)
{
   not_impl();
}

size_t gfx_term_col_sprint_set_bg(char *buf, size_t buf_size, size_t offset,
   gfx_term_col_t color)
{
   not_impl();
}

size_t gfx_term_col_sprint_reset(char *buf, size_t buf_size, size_t offset)
{
   not_impl();
}

#else

#include <unistd.h>

/* See http://en.wikipedia.org/wiki/ANSI_escape_code */

bool gfx_term_col_ok(FILE *f)
{
   return !!isatty(fileno(f));
}

static inline bool fg_col_ok(gfx_term_col_t color)
{
   return (color >= 0) && (color < 16);
}

static inline bool bg_col_ok(gfx_term_col_t color)
{
   return (color >= 0) && (color < 8);
}

#define SET_FG_PRINTF_ARGS(COLOR) "\x1b[%d;%dm", 30 + ((COLOR) & 7), ((COLOR) & 8) ? 1 : 22
#define SET_BG_PRINTF_ARGS(COLOR) "\x1b[%dm", 40 + (COLOR)
#define RESET_PRINTF_ARGS "\x1b[m"

void gfx_term_col_set_fg(FILE *f, gfx_term_col_t color)
{
   assert(fg_col_ok(color));
   fprintf(f, SET_FG_PRINTF_ARGS(color));
}

void gfx_term_col_set_bg(FILE *f, gfx_term_col_t color)
{
   assert(bg_col_ok(color));
   fprintf(f, SET_BG_PRINTF_ARGS(color));
}

void gfx_term_col_reset(FILE *f)
{
   fprintf(f, RESET_PRINTF_ARGS);
}

size_t gfx_term_col_sprint_set_fg(char *buf, size_t buf_size, size_t offset,
   gfx_term_col_t color)
{
   assert(fg_col_ok(color));
   return vcos_safe_sprintf(buf, buf_size, offset, SET_FG_PRINTF_ARGS(color));
}

size_t gfx_term_col_sprint_set_bg(char *buf, size_t buf_size, size_t offset,
   gfx_term_col_t color)
{
   assert(bg_col_ok(color));
   return vcos_safe_sprintf(buf, buf_size, offset, SET_BG_PRINTF_ARGS(color));
}

size_t gfx_term_col_sprint_reset(char *buf, size_t buf_size, size_t offset)
{
   return vcos_safe_sprintf(buf, buf_size, offset, RESET_PRINTF_ARGS);
}

#endif
