/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#include "log.h"

#include "libs/util/demand.h"
#include "libs/util/gfx_util/gfx_util_term_col.h"
#include "libs/util/gfx_util/gfx_util_wildcard.h"
#include "vcos.h"
#include <ctype.h>
#include <string.h>
#ifdef ANDROID
#include <android/log.h>
#endif
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#define DEFAULT_LEVEL LOG_INFO

/* Fixed number of level specs of fixed size as I want to avoid long-term
 * mallocs -- either we would leak the memory on exit or we'd have to install
 * some kind of atexit hook to free it, neither of which sounds great. */

struct log_level_spec
{
   char pattern[64]; /* Wildcard pattern */
   log_level_t level;
};

static struct
{
   VCOS_ONCE_T once;
   VCOS_MUTEX_T mutex;
   size_t num_level_specs;
   struct log_level_spec level_specs[64];
   bool colorise;
} log_state = {VCOS_ONCE_INIT};

static char *strchr_or_end(char *s, char c)
{
   char *first = strchr(s, c);
   if (!first)
      first = s + strlen(s);
   return first;
}

static char *first_nonspace(char *s)
{
   while (isspace(*s))
      ++s;
   return s;
}

static void strip_spaces_at_end(char *s)
{
   char *end = s + strlen(s);
   while ((end > s) && isspace(end[-1]))
      --end;
   *end = '\0';
}

static bool is_prefix_of(const char *a, const char *b)
{
   for (; *a; ++a, ++b)
      if (*a != *b)
         return false;
   return true;
}

static void parse_log_level(void)
{
   const char *log_level = getenv("LOG_LEVEL");
   if (!log_level)
      return;

   /* malloc a copy so we can safely fiddle with it */
   char *malloced = malloc(strlen(log_level) + 1);
   demand(malloced);
   char *s = malloced;
   strcpy(s, log_level);

   for (;;)
   {
      s = first_nonspace(s);

      char *comma = strchr_or_end(s, ',');
      bool last = !*comma;
      *comma = '\0';

      strip_spaces_at_end(s);

      if (*s)
      {
         demand(log_state.num_level_specs < countof(log_state.level_specs));
         struct log_level_spec *spec = &log_state.level_specs[log_state.num_level_specs++];

         char *colon = strrchr(s, ':');
         demand_msg(colon, "LOG_LEVEL ('%s') malformed -- spec '%s' has no colon!", log_level, s);
         *colon = '\0';

         strip_spaces_at_end(s);
         demand_msg(*s, "LOG_LEVEL ('%s') malformed -- spec has nothing before colon!", log_level);

         demand(strlen(s) < sizeof(spec->pattern));
         strcpy(spec->pattern, s);

         char *level = first_nonspace(colon + 1);
         demand_msg(*level, "LOG_LEVEL ('%s') malformed -- spec has nothing after colon!", log_level);

         if (is_prefix_of(level, "none"))             spec->level = LOG_NONE;
         else if (is_prefix_of(level, "error"))       spec->level = LOG_ERROR;
         else if (is_prefix_of(level, "warning"))     spec->level = LOG_WARN;
         else if (is_prefix_of(level, "information")) spec->level = LOG_INFO;
         else if (is_prefix_of(level, "trace"))       spec->level = LOG_TRACE;
         else
            demand_msg(false, "LOG_LEVEL ('%s') malformed -- unrecognised log level '%s'!", log_level, level);
      }

      if (last)
         break;

      s = comma + 1;
   }

   free(malloced);
}

static void init(void)
{
   demand(vcos_mutex_create(&log_state.mutex, "log_state.mutex") == VCOS_SUCCESS);
   log_state.num_level_specs = 0;
   parse_log_level();
   log_state.colorise = gfx_term_col_ok(stderr);
}

static log_level_t get_cat_level(const char *name)
{
   for (size_t i = log_state.num_level_specs - 1; i != -1; --i)
   {
      const struct log_level_spec *spec = &log_state.level_specs[i];
      if (gfx_wildcard_pattern_matches(spec->pattern, name))
         return spec->level;
   }
   return DEFAULT_LEVEL;
}

bool log_cat_enabled_extern(struct log_cat *cat, log_level_t level)
{
   /* Should early-out in inline functions if WANT_LOGGING is off */
   assert(WANT_LOGGING);

   log_level_t cat_level = log_cat_load_level(cat);
   if (cat_level == LOG_UNKNOWN)
   {
      demand(vcos_once(&log_state.once, init) == VCOS_SUCCESS);
      vcos_mutex_lock(&log_state.mutex);

      cat_level = log_cat_load_level(cat);
      if (cat_level == LOG_UNKNOWN)
      {
         cat_level = get_cat_level(cat->name);
         vcos_atomic_store_uint32(&cat->level, (uint32_t)cat_level, VCOS_MEMORY_ORDER_RELAXED);
      }

      vcos_mutex_unlock(&log_state.mutex);
   }

   return cat_level >= level;
}

#ifdef ANDROID
static int level_to_android_prio(log_level_t level)
{
   switch (level)
   {
   case LOG_ERROR:   return ANDROID_LOG_ERROR;
   case LOG_WARN:    return ANDROID_LOG_WARN;
   case LOG_INFO:    return ANDROID_LOG_INFO;
   case LOG_TRACE:   return ANDROID_LOG_DEBUG;
   default:          unreachable(); return 0;
   }
}
#else
static gfx_term_col_t get_level_col(log_level_t level)
{
   switch (level)
   {
   case LOG_ERROR:   return GFX_TERM_COL_BR_RED;
   case LOG_WARN:    return GFX_TERM_COL_YELLOW;
   case LOG_INFO:    return GFX_TERM_COL_INVALID;
   case LOG_TRACE:   return GFX_TERM_COL_BR_BLACK;
   default:          unreachable(); return (gfx_term_col_t)0;
   }
}
#endif

void log_cat_msg_v_extern(struct log_cat *cat, log_level_t level, const char *fmt, va_list args)
{
   if (!log_cat_enabled_extern(cat, level))
      return;

#ifdef ANDROID
   __android_log_vprint(level_to_android_prio(level), cat->name, fmt, args);
#else
   char buf[512];
   size_t offset = 0;

   demand(vcos_once(&log_state.once, init) == VCOS_SUCCESS); /* Ensure log_state.colorise is safe to read */
   gfx_term_col_t col = get_level_col(level);
   if (log_state.colorise && (col != GFX_TERM_COL_INVALID))
      offset = gfx_term_col_sprint_set_fg(buf, sizeof(buf), offset, col);

   char msg[512];
   VCOS_SAFE_VSPRINTF(msg, 0, fmt, args);
   char *m = msg;
   for (;;)
   {
      char *nl = strchr(m, '\n');
      if (nl)
         *nl = '\0';
      offset = VCOS_SAFE_SPRINTF(buf, offset, "[%s] %s\n", cat->name, m);
      if (nl)
         m = nl + 1;
      else
         break;
   }

   if (log_state.colorise && (col != GFX_TERM_COL_INVALID))
      offset = gfx_term_col_sprint_reset(buf, sizeof(buf), offset);

   fputs(buf, stderr);
#ifdef _WIN32
   OutputDebugStringA(buf);
#endif
#endif
}

void log_cat_msg_extern(struct log_cat *cat, log_level_t level, const char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);
   log_cat_msg_v_extern(cat, level, fmt, args);
   va_end(args);
}
