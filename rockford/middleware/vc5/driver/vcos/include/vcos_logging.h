/*
 * Copyright (c) 2010-2011 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom Corporation and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use
 *    all reasonable efforts to protect the confidentiality thereof, and to
 *    use this information only in connection with your use of Broadcom
 *    integrated circuit products.
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
 *    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
 *    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
 *    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 */

/*=============================================================================
VideoCore OS Abstraction Layer - logging support
=============================================================================*/

#ifndef VCOS_LOGGING_H
#define VCOS_LOGGING_H

#include <stdarg.h>

#include "vcos_types.h"
#include "vcos_platform.h"
#include "vcos_logging_control.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \file
 *
 * Logging support
 *
 * This provides categorised logging. Clients register
 * a category, and then get a number of logging levels for
 * that category.
 *
 * The logging level flag is tested using a flag *before* the
 * function call, which makes logging very fast when disabled - there
 * is no function call overhead just to find out that this log
 * message is disabled.
 *
 * \section VCOS_LOG_CATEGORY
 *
 * As a convenience, clients define VCOS_LOG_CATEGORY to point to
 * their category; the various vcos_log_xxx() macros then expand to
 * use this.
 *
 * e.g.
 *
 *     #define VCOS_LOG_CATEGORY (&my_category)
 *
 *     #include <vcos.h>
 *
 *     VCOS_LOG_CAT_T my_category;
 *
 *     ....
 *
 *     vcos_log_trace("Stuff happened: %d", n_stuff);
 *
 */

/** Logging levels */
typedef enum VCOS_LOG_LEVEL_T
{
   VCOS_LOG_UNINITIALIZED   = 0,
   VCOS_LOG_NEVER,
   VCOS_LOG_ERROR,
   VCOS_LOG_WARN,
   VCOS_LOG_INFO,
   VCOS_LOG_TRACE,
} VCOS_LOG_LEVEL_T;

#ifdef VCOS_LOG_AUTO_REGISTER
#define VCOS_LOG_CAT_SECTION_ATTR VCOS_ATTR_SECTION(".init.vcos_categories")
#else
#define VCOS_LOG_CAT_SECTION_ATTR
#endif

/** Initialize a logging category without going through vcos_log_register().
 *
 * This is useful for the case where there is no obvious point to do the
 * registration (no initialization function for the module). However, it
 * means that your logging category is not registered, so cannot be easily
 * changed at run-time.
 */
#define VCOS_LOG_INIT(n,l) { l, n, 0, {0}, 0, 0 }

/** Create a logging category statically, registered at link-time.
 *
 * @param s    symbol to declare
 * @param n    name of category
 * @param l    level
 */
#define VCOS_LOG_CATEGORY_REGISTER(s,n,l) \
   VCOS_AUTO_LOG_CAT_T s VCOS_LOG_CAT_SECTION_ATTR = VCOS_LOG_INIT(n,l)


/** A registered logging category.
  */
typedef struct VCOS_LOG_CAT_T
{
   VCOS_LOG_LEVEL_T level;      /**< Which levels are enabled for this category */
   const char *name;            /**< Name for this category. */
   struct VCOS_LOG_CAT_T *next;
   struct {
      unsigned int want_prefix:1;
   } flags;
   unsigned int refcount;
   void *platform_data;         /**< platform specific data */
} VCOS_LOG_CAT_T;

typedef VCOS_ATTR_SECTION_QUALIFIER struct VCOS_LOG_CAT_T VCOS_AUTO_LOG_CAT_T;

typedef void (*VCOS_VLOG_IMPL_FUNC_T)(const VCOS_LOG_CAT_T *cat, VCOS_LOG_LEVEL_T _level, const char *fmt, va_list args);

/** Convert a VCOS_LOG_LEVEL_T into a printable string.
  * The platform needs to implement this function.
  */
const char * vcos_log_level_to_string( VCOS_LOG_LEVEL_T level );

/** Convert a string into a VCOS_LOG_LEVEL_T
  * The platform needs to implement this function.
  */
VCOS_STATUS_T vcos_string_to_log_level( const char *str, VCOS_LOG_LEVEL_T *level );

/** Log a message. Basic API. Normal code should not use this.
  * The platform needs to implement this function.
  */
void vcos_log_impl(const VCOS_LOG_CAT_T *cat, VCOS_LOG_LEVEL_T _level, const char *fmt, ...) VCOS_FORMAT_ATTR_(printf, 3, 4);

/** Log a message using a varargs parameter list. Normal code should
  * not use this.
  */
void vcos_vlog_impl(const VCOS_LOG_CAT_T *cat, VCOS_LOG_LEVEL_T _level, const char *fmt, va_list args) VCOS_FORMAT_ATTR_(printf, 3, 0);

/** Set the function which does the actual logging output.
 *  Passing in NULL causes the default logging function to be
 *  used.
  */
void vcos_set_vlog_impl( VCOS_VLOG_IMPL_FUNC_T vlog_impl_func );

/** The default logging function, which is provided by each
  * platform.
  */

void vcos_vlog_default_impl(const VCOS_LOG_CAT_T *cat, VCOS_LOG_LEVEL_T _level, const char *fmt, va_list args) VCOS_FORMAT_ATTR_(printf, 3, 0);

/*
 * Initialise the logging subsystem. This is called from
 * vcos_init() so you don't normally need to call it.
 */
void vcos_logging_init(void);

/** Register a logging category.
  *
  * @param name the name of this category.
  * @param category the category to register.
  */
void vcos_log_register(const char *name, VCOS_LOG_CAT_T *category);

/** Unregister a logging category.
  */
void vcos_log_unregister(VCOS_LOG_CAT_T *category);

/** Return a default logging category, for people too lazy to create their own.
  *
  * Using the default category will be slow (there's an extra function
  * call overhead). Don't do this in normal code.
  */
const VCOS_LOG_CAT_T * vcos_log_get_default_category(void);

void vcos_set_log_options(const char *opt);

/** Set vcos logging level using category name.
  */
VCOS_STATUS_T vcos_log_set_category_level(const char *cat_name, VCOS_LOG_LEVEL_T level);

/** Set the logging level for a category at run time. Without this, the level
  * will be that set by vcos_log_register from a platform-specific source.
  *
  * @param category the category to modify.
  * @param level the new logging level for this category.
  */
static inline void vcos_log_set_level(VCOS_LOG_CAT_T *category, VCOS_LOG_LEVEL_T level)
{
   category->level = level;
}

#define vcos_log_dump_mem(cat,label,addr,voidMem,numBytes)  do { if (vcos_is_log_enabled(cat,VCOS_LOG_TRACE)) vcos_log_dump_mem_impl(cat,label,addr,voidMem,numBytes); } while (0)

void vcos_log_dump_mem_impl( const VCOS_LOG_CAT_T *cat,
                             const char           *label,
                             uintptr_t             addr,
                             const void           *voidMem,
                             size_t                numBytes );

/*
 * Platform specific hooks (optional).
 */
#ifndef vcos_log_platform_init
#define vcos_log_platform_init()                (void)0
#endif

#ifndef vcos_log_platform_register
#define vcos_log_platform_register(category)    (void)0
#endif

#ifndef vcos_log_platform_unregister
#define vcos_log_platform_unregister(category)  (void)0
#endif

/* VCOS_TRACE() - deprecated macro which just outputs in a debug build and
 * is a no-op in a release build.
 *
 * _VCOS_LOG_X() - internal macro which outputs if the current level for the
 * particular category is higher than the supplied message level.
 */

#define VCOS_LOG_DFLT_CATEGORY vcos_log_get_default_category()

#define _VCOS_LEVEL(x) (x)

#define vcos_is_log_enabled(cat,_level)  (_VCOS_LEVEL((cat)->level) >= _VCOS_LEVEL(_level))

#define VCOS_LOG_ERROR_ENABLED (vcos_is_log_enabled(VCOS_LOG_CATEGORY, VCOS_LOG_ERROR))
#define VCOS_LOG_WARN_ENABLED (vcos_is_log_enabled(VCOS_LOG_CATEGORY, VCOS_LOG_WARN))
#define VCOS_LOG_INFO_ENABLED (vcos_is_log_enabled(VCOS_LOG_CATEGORY, VCOS_LOG_INFO))
#define VCOS_LOG_TRACE_ENABLED (vcos_is_log_enabled(VCOS_LOG_CATEGORY, VCOS_LOG_TRACE))

#if defined(_VCOS_METAWARE) || defined(__GNUC__)

# if !defined(AMPUTATE_ALL_VCOS_LOGGING) && (!defined(NDEBUG) || defined(VCOS_ALWAYS_WANT_LOGGING))
#  define VCOS_LOGGING_ENABLED
#  define _VCOS_LOG_X(cat, _level, fmt...)   do { if (vcos_is_log_enabled(cat,_level)) vcos_log_impl(cat,_level,fmt); } while (0)
#  define _VCOS_VLOG_X(cat, _level, fmt, ap) do { if (vcos_is_log_enabled(cat,_level)) vcos_vlog_impl(cat,_level,fmt,ap); } while (0)
#  define _VCOS_LOG_X_ALWAYS(cat, _level, fmt...)   do { vcos_log_impl(cat,_level,fmt); } while (0)
# else
#  define _VCOS_LOG_X(cat, _level, fmt...) (void)0
#  define _VCOS_VLOG_X(cat, _level, fmt, ap) (void)0
#  define _VCOS_LOG_X_ALWAYS(cat, _level, fmt...) (void)0
# endif



# define vcos_log_error(...)   _VCOS_LOG_X(VCOS_LOG_CATEGORY, VCOS_LOG_ERROR, __VA_ARGS__)
# define vcos_log_warn(...)    _VCOS_LOG_X(VCOS_LOG_CATEGORY, VCOS_LOG_WARN, __VA_ARGS__)
# define vcos_log_info(...)    _VCOS_LOG_X(VCOS_LOG_CATEGORY, VCOS_LOG_INFO, __VA_ARGS__)
# define vcos_log_trace(...)   _VCOS_LOG_X(VCOS_LOG_CATEGORY, VCOS_LOG_TRACE, __VA_ARGS__)

# define vcos_logc_error(cat,...)   _VCOS_LOG_X(cat, VCOS_LOG_ERROR, __VA_ARGS__)
# define vcos_logc_warn(cat,...)    _VCOS_LOG_X(cat, VCOS_LOG_WARN, __VA_ARGS__)
# define vcos_logc_info(cat,...)    _VCOS_LOG_X(cat, VCOS_LOG_INFO, __VA_ARGS__)
# define vcos_logc_trace(cat,...)   _VCOS_LOG_X(cat, VCOS_LOG_TRACE, __VA_ARGS__)

# define vcos_vlogc_error(cat,fmt,ap)  _VCOS_VLOG_X(cat, VCOS_LOG_ERROR, fmt, ap)
# define vcos_vlogc_warn(cat,fmt,ap)   _VCOS_VLOG_X(cat, VCOS_LOG_WARN, fmt, ap)
# define vcos_vlogc_info(cat,fmt,ap)   _VCOS_VLOG_X(cat, VCOS_LOG_INFO, fmt, ap)
# define vcos_vlogc_trace(cat,fmt,ap)  _VCOS_VLOG_X(cat, VCOS_LOG_TRACE, fmt, ap)

# define vcos_log(...)   _VCOS_LOG_X(VCOS_LOG_DFLT_CATEGORY, VCOS_LOG_INFO, __VA_ARGS__)
# define vcos_vlog(fmt,ap)  _VCOS_VLOG_X(VCOS_LOG_DFLT_CATEGORY, VCOS_LOG_INFO, fmt, ap)
# define VCOS_ALERT(...) _VCOS_LOG_X(VCOS_LOG_DFLT_CATEGORY, VCOS_LOG_ERROR, __VA_ARGS__)
# define VCOS_TRACE(...) _VCOS_LOG_X(VCOS_LOG_DFLT_CATEGORY, VCOS_LOG_INFO, __VA_ARGS__)
# define VCOS_IF_LOG(cond,...) do { if (cond) _VCOS_LOG_X_ALWAYS(VCOS_LOG_DFLT_CATEGORY, VCOS_LOG_INFO, __VA_ARGS__); } while (0)

/*
 * MS Visual Studio - pre 2005 does not grok variadic macros
 */
#elif defined(_MSC_VER)

# if _MSC_VER >= 1400

#  if !defined(AMPUTATE_ALL_VCOS_LOGGING) && (!defined(NDEBUG) || defined(VCOS_ALWAYS_WANT_LOGGING))
#   define VCOS_LOGGING_ENABLED
#   define _VCOS_LOG_X(cat, _level, fmt,...) do { if (vcos_is_log_enabled(cat,_level)) vcos_log_impl(cat, _level, fmt, __VA_ARGS__); } while (0)
#   define _VCOS_VLOG_X(cat, _level, fmt, ap) do { if (vcos_is_log_enabled(cat,_level)) vcos_vlog_impl(cat, _level, fmt, ap); } while (0)
#   define _VCOS_LOG_X_ALWAYS(cat, _level, fmt, ...) do { vcos_log_impl(cat, _level, fmt, __VA_ARGS__); } while (0)
#  else
#   define _VCOS_LOG_X(cat, _level, fmt,...) (void)0
#   define _VCOS_VLOG_X(cat, _level, fmt, ap) (void)0
#   define _VCOS_LOG_X_ALWAYS(cat, _level, fmt,...) (void)0
#  endif

# define vcos_log_error(fmt,...)   _VCOS_LOG_X(VCOS_LOG_CATEGORY, VCOS_LOG_ERROR, fmt, __VA_ARGS__)
# define vcos_log_warn(fmt,...)    _VCOS_LOG_X(VCOS_LOG_CATEGORY, VCOS_LOG_WARN, fmt, __VA_ARGS__)
# define vcos_log_info(fmt,...)    _VCOS_LOG_X(VCOS_LOG_CATEGORY, VCOS_LOG_INFO, fmt, __VA_ARGS__)
# define vcos_log_trace(fmt,...)   _VCOS_LOG_X(VCOS_LOG_CATEGORY, VCOS_LOG_TRACE, fmt, __VA_ARGS__)

# define vcos_logc_error(cat,fmt,...)   _VCOS_LOG_X(cat, VCOS_LOG_ERROR, fmt, __VA_ARGS__)
# define vcos_logc_warn(cat,fmt,...)    _VCOS_LOG_X(cat, VCOS_LOG_WARN, fmt, __VA_ARGS__)
# define vcos_logc_info(cat,fmt,...)    _VCOS_LOG_X(cat, VCOS_LOG_INFO, fmt, __VA_ARGS__)
# define vcos_logc_trace(cat,fmt,...)   _VCOS_LOG_X(cat, VCOS_LOG_TRACE, fmt, __VA_ARGS__)

# define vcos_log(fmt,...)   _VCOS_LOG_X(VCOS_LOG_DFLT_CATEGORY, VCOS_LOG_INFO, fmt, __VA_ARGS__)
# define VCOS_ALERT(fmt,...) _VCOS_LOG_X(VCOS_LOG_DFLT_CATEGORY, VCOS_LOG_ERROR, fmt, __VA_ARGS__)
# define VCOS_TRACE(fmt,...) _VCOS_LOG_X(VCOS_LOG_DFLT_CATEGORY, VCOS_LOG_INFO, fmt, __VA_ARGS__)
# define VCOS_IF_LOG(cond,fmt,...) do { if (cond) _VCOS_LOG_X_ALWAYS(VCOS_LOG_DFLT_CATEGORY, VCOS_LOG_INFO, fmt, __VA_ARGS__); } while (0)

# else /* _MSC_VER >= 1400 */

/* do not define these */

# endif /* _MSC_VER >= 1400 */

#endif

#define vcos_vlog_error(fmt,ap)  _VCOS_VLOG_X(VCOS_LOG_CATEGORY, VCOS_LOG_ERROR, fmt, ap)
#define vcos_vlog_warn(fmt,ap)   _VCOS_VLOG_X(VCOS_LOG_CATEGORY, VCOS_LOG_WARN, fmt, ap)
#define vcos_vlog_info(fmt,ap)   _VCOS_VLOG_X(VCOS_LOG_CATEGORY, VCOS_LOG_INFO, fmt, ap)
#define vcos_vlog_trace(fmt,ap)  _VCOS_VLOG_X(VCOS_LOG_CATEGORY, VCOS_LOG_TRACE, fmt, ap)

/** Register all the logging categories that have been setup via a linker
 * section with VCOS_LOG_CATEGORY_REGISTER. Must be called once.
 */
void vcos_register_auto_categories(void);

#ifdef __cplusplus
}
#endif

#endif /* VCOS_LOGGING_H */
