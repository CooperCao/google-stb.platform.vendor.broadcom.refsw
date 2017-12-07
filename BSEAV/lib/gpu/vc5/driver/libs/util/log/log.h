/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

/* Use like:
 *
 *    struct log_cat my_log_cat = LOG_CAT_INIT("my_log_cat");
 *    log_cat_error(&my_log_cat, "%uscary%ume", 3, 5);
 *
 * or:
 *
 *    LOG_DEFAULT_CAT("my_log_cat") // At top of file
 *    log_info("This is a %s message", "log"); // Uses default category (my_log_cat)
 */

#include "libs/util/common.h"
#include "vcos_atomic.h"
#include <assert.h>
#include <stdbool.h>
#include <stdarg.h>

EXTERN_C_BEGIN

typedef enum
{
   /* Order of these is important! */
   LOG_NONE,
   LOG_ERROR,
   LOG_WARN,
   LOG_INFO,
   LOG_TRACE,
   LOG_UNKNOWN
} log_level_t;

struct log_cat
{
   const char *name;

   /* This is really a log_level_t, but use uint32_t so we can use the
    * vcos_atomic_* functions.
    * LOG_UNKNOWN means not figured out level yet.
    * LOG_NONE means don't display any log messages at all. */
   uint32_t level;
};

#define LOG_CAT_INIT(NAME) {NAME, LOG_UNKNOWN}

static inline log_level_t log_cat_load_level(const struct log_cat *cat)
{
   return (log_level_t)vcos_atomic_load_uint32(&cat->level, VCOS_MEMORY_ORDER_RELAXED);
}

extern bool log_cat_enabled_extern(struct log_cat *cat, log_level_t level);

extern void log_cat_msg_v_extern(struct log_cat *cat, log_level_t level, const char *fmt, va_list args);
extern void log_cat_msg_extern(struct log_cat *cat, log_level_t level, const char *fmt, ...) ATTRIBUTE_FORMAT(printf, 3, 4);

#ifndef WANT_LOGGING
#define WANT_LOGGING 0
#endif
static inline bool log_cat_possibly_enabled(const struct log_cat *cat, log_level_t level)
{
   return WANT_LOGGING && (log_cat_load_level(cat) >= level);
}

static inline bool log_cat_enabled(struct log_cat *cat, log_level_t level)
{
   return log_cat_possibly_enabled(cat, level) && log_cat_enabled_extern(cat, level);
}

static inline bool log_cat_error_enabled(struct log_cat *cat)  { return log_cat_enabled(cat, LOG_ERROR); }
static inline bool log_cat_warn_enabled(struct log_cat *cat)   { return log_cat_enabled(cat, LOG_WARN); }
static inline bool log_cat_info_enabled(struct log_cat *cat)   { return log_cat_enabled(cat, LOG_INFO); }
static inline bool log_cat_trace_enabled(struct log_cat *cat)  { return log_cat_enabled(cat, LOG_TRACE); }

#define log_cat_msg(CAT, LEVEL, ...)                     \
   do                                                    \
   {                                                     \
      struct log_cat *cat_ = (CAT);                      \
      log_level_t level_ = (LEVEL);                      \
      if (log_cat_possibly_enabled(cat_, level_))        \
         log_cat_msg_extern(cat_, level_, __VA_ARGS__);  \
   } while (0)

static inline void log_cat_msg_v(struct log_cat *cat, log_level_t level, const char *fmt, va_list args)
{
   if (log_cat_possibly_enabled(cat, level))
      log_cat_msg_v_extern(cat, level, fmt, args);
}

#define log_cat_error(CAT, ...)  log_cat_msg(CAT, LOG_ERROR, __VA_ARGS__)
#define log_cat_warn(CAT, ...)   log_cat_msg(CAT, LOG_WARN, __VA_ARGS__)
#define log_cat_info(CAT, ...)   log_cat_msg(CAT, LOG_INFO, __VA_ARGS__)
#define log_cat_trace(CAT, ...)  log_cat_msg(CAT, LOG_TRACE, __VA_ARGS__)

#define log_cat_error_v(CAT, FMT, ARGS)   log_cat_msg_v(CAT, LOG_ERROR, FMT, ARGS)
#define log_cat_warn_v(CAT, FMT, ARGS)    log_cat_msg_v(CAT, LOG_WARN, FMT, ARGS)
#define log_cat_info_v(CAT, FMT, ARGS)    log_cat_msg_v(CAT, LOG_INFO, FMT, ARGS)
#define log_cat_trace_v(CAT, FMT, ARGS)   log_cat_msg_v(CAT, LOG_TRACE, FMT, ARGS)

/** LOG_DEFAULT_CAT() & co */

#define LOG_DEFAULT_CAT(NAME) static struct log_cat log_default_cat = LOG_CAT_INIT(NAME);

#define log_enabled(LEVEL)    log_cat_enabled(&log_default_cat, LEVEL)
#define log_error_enabled()   log_cat_error_enabled(&log_default_cat)
#define log_warn_enabled()    log_cat_warn_enabled(&log_default_cat)
#define log_info_enabled()    log_cat_info_enabled(&log_default_cat)
#define log_trace_enabled()   log_cat_trace_enabled(&log_default_cat)

#define log_msg(LEVEL, ...)         log_cat_msg(&log_default_cat, LEVEL, __VA_ARGS__)
#define log_msg_v(LEVEL, FMT, ARGS) log_cat_msg_v(&log_default_cat, LEVEL, FMT, ARGS)

#define log_error(...)  log_cat_error(&log_default_cat, __VA_ARGS__)
#define log_warn(...)   log_cat_warn(&log_default_cat, __VA_ARGS__)
#define log_info(...)   log_cat_info(&log_default_cat, __VA_ARGS__)
#define log_trace(...)  log_cat_trace(&log_default_cat, __VA_ARGS__)

#define log_error_v(FMT, ARGS)   log_cat_error_v(&log_default_cat, FMT, ARGS)
#define log_warn_v(FMT, ARGS)    log_cat_warn_v(&log_default_cat, FMT, ARGS)
#define log_info_v(FMT, ARGS)    log_cat_info_v(&log_default_cat, FMT, ARGS)
#define log_trace_v(FMT, ARGS)   log_cat_trace_v(&log_default_cat, FMT, ARGS)

EXTERN_C_END
