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
Categorized logging for VCOS - a generic implementation.
=============================================================================*/

#include "vcos.h"
#include <ctype.h>
#include "vcos_string.h"
#include <inttypes.h>
#include "helpers/snprintf.h"

/* Macro to get the width of a pointer for use with printf style functions */
#define PRIxPTR_WIDTH ((int)(sizeof(uintptr_t)*2))

static VCOS_MUTEX_T lock;
static int warned_loglevel;             /* only warn about invalid log level once */
static VCOS_VLOG_IMPL_FUNC_T vcos_vlog_impl_func = vcos_vlog_default_impl;

#define  VCOS_LOG_CATEGORY (&dflt_log_category)
static VCOS_LOG_CAT_T dflt_log_category;
VCOS_LOG_CAT_T *vcos_logging_categories = NULL;
static int inited;

/* For now, do not link logging categories into linked lists
 * as it breaks when people unload shared libraries without
 * getting the counts right.
 */
#ifdef __VIDEOCORE__
#define REGISTER_CATEGORIES 1
#else
#define REGISTER_CATEGORIES 0
#endif

void vcos_logging_init(void)
{
   if (inited)
   {
      /* FIXME: should print a warning or something here */
      return;
   }
   vcos_mutex_create(&lock, "vcos_log");

   vcos_log_platform_init();

   vcos_log_register("default", &dflt_log_category);

   assert(!inited);
   inited = 1;
}

/** Read an alphanumeric token, returning True if we succeeded.
  */

static int read_tok(char *tok, size_t toklen, const char **pstr, char sep)
{
   const char *str = *pstr;
   size_t n = 0;
   char ch;

   /* skip past any whitespace */
   while (str[0] && isspace((int)(str[0])))
      str++;

   while ((ch = *str) != '\0' &&
          ch != sep &&
          (isalnum((int)ch) || (ch == '_')) &&
          n != toklen-1)
   {
      tok[n++] = ch;
      str++;
   }

   /* did it work out? */
   if ( ( ch == '\0' || ch == sep ) && ( n > 0 ) )
   {
      if (ch) str++; /* move to next token if not at end */
      /* yes */
      tok[n] = '\0';
      *pstr = str;
      return 1;
   }
   else
   {
      /* no */
      return 0;
   }
}

const char *vcos_log_level_to_string( VCOS_LOG_LEVEL_T level )
{
   switch (level)
   {
      case VCOS_LOG_UNINITIALIZED:  return "uninit";
      case VCOS_LOG_NEVER:          return "never";
      case VCOS_LOG_ERROR:          return "error";
      case VCOS_LOG_WARN:           return "warn";
      case VCOS_LOG_INFO:           return "info";
      case VCOS_LOG_TRACE:          return "trace";
   }
   return "???";
}

VCOS_STATUS_T vcos_string_to_log_level( const char *str, VCOS_LOG_LEVEL_T *level )
{
   if (strcmp(str,"error") == 0)
      *level = VCOS_LOG_ERROR;
   else if (strcmp(str,"never") == 0)
      *level = VCOS_LOG_NEVER;
   else if (strcmp(str,"warn") == 0)
      *level = VCOS_LOG_WARN;
   else if (strcmp(str,"warning") == 0)
      *level = VCOS_LOG_WARN;
   else if (strcmp(str,"info") == 0)
      *level = VCOS_LOG_INFO;
   else if (strcmp(str,"trace") == 0)
      *level = VCOS_LOG_TRACE;
   else
      return VCOS_EINVAL;

   return VCOS_SUCCESS;
}

static int read_level(VCOS_LOG_LEVEL_T *level, const char **pstr, char sep)
{
   char buf[16];
   int ret = 1;
   if (read_tok(buf,sizeof(buf),pstr,sep))
   {
      if (vcos_string_to_log_level(buf,level) != VCOS_SUCCESS)
      {
         vcos_log("Invalid trace level '%s'\n", buf);
         ret = 0;
      }
   }
   else
   {
      ret = 0;
   }
   return ret;
}

void vcos_log_register(const char *name, VCOS_LOG_CAT_T *category)
{
   const char *env;
   VCOS_LOG_CAT_T *i;

   category->name  = name;
   if ( category->level == VCOS_LOG_UNINITIALIZED )
   {
      category->level = VCOS_LOG_ERROR;
   }
   category->flags.want_prefix = (category != &dflt_log_category );

   if (!REGISTER_CATEGORIES)
      return;

   vcos_mutex_lock(&lock);

   /* is it already registered? */
   for (i = vcos_logging_categories; i ; i = i->next )
   {
      if (i == category)
      {
         i->refcount++;
         break;
      }
   }

   if (!i)
   {
      /* not yet registered */
      category->next = vcos_logging_categories;
      vcos_logging_categories = category;
      category->refcount++;

      vcos_log_platform_register(category);
   }

   vcos_mutex_unlock(&lock);

   /* Check to see if this log level has been enabled. Look for
    * (<category:level>,)*
    *
    * VC_LOGLEVEL=ilcs:info,vchiq:warn
    */

   env = _VCOS_LOG_LEVEL();
   if (env)
   {
      do
      {
         char env_name[64];
         VCOS_LOG_LEVEL_T level;
         if (read_tok(env_name, sizeof(env_name), &env, ':') &&
             read_level(&level, &env, ','))
         {
            if (strcmp(env_name, name) == 0)
            {
               category->level = level;
               break;
            }
         }
         else
         {
            if (!warned_loglevel)
            {
                vcos_log("VC_LOGLEVEL format invalid at %s\n", env);
                warned_loglevel = 1;
            }
            return;
         }
      } while (env[0] != '\0');
   }

   vcos_log_info( "Registered log category '%s' with level %s",
                  category->name,
                  vcos_log_level_to_string( category->level ));
}

void vcos_log_unregister(VCOS_LOG_CAT_T *category)
{
   VCOS_LOG_CAT_T **pcat;

   if (!REGISTER_CATEGORIES)
      return;

   vcos_mutex_lock(&lock);
   category->refcount--;
   if (category->refcount == 0)
   {
      pcat = &vcos_logging_categories;
      while (*pcat != category)
      {
         if (!*pcat)
            break;   /* possibly deregistered twice? */
         if ((*pcat)->next == NULL)
         {
            assert(0); /* already removed! */
            vcos_mutex_unlock(&lock);
            return;
         }
         pcat = &(*pcat)->next;
      }
      if (*pcat)
         *pcat = category->next;

      vcos_log_platform_unregister(category);
   }
   vcos_mutex_unlock(&lock);
}

const VCOS_LOG_CAT_T * vcos_log_get_default_category(void)
{
   return &dflt_log_category;
}

void vcos_set_log_options(const char *opt)
{
   (void)opt;
}

void vcos_log_dump_mem_impl( const VCOS_LOG_CAT_T *cat,
                             const char           *label,
                             uintptr_t             addr,
                             const void           *voidMem,
                             size_t                numBytes )
{
   const uint8_t  *mem = (const uint8_t *)voidMem;
   size_t          offset;
   char            lineBuf[ 100 ];
   char           *s;

   while ( numBytes > 0 )
   {
       s = lineBuf;

       for ( offset = 0; offset < 16; offset++ )
       {
           if ( offset < numBytes )
           {
               s += snprintf( s, 4, "%02x ", mem[ offset ]);
           }
           else
           {
               s += snprintf( s, 4, "   " );
           }
       }

       for ( offset = 0; offset < 16; offset++ )
       {
           if ( offset < numBytes )
           {
               uint8_t ch = mem[ offset ];

               if (( ch < ' ' ) || ( ch > '~' ))
               {
                   ch = '.';
               }
               *s++ = (char)ch;
           }
       }
       *s++ = '\0';

       if (( label != NULL ) && ( *label != '\0' ))
       {
          vcos_log_impl( cat, VCOS_LOG_INFO, "%s: %0*" PRIxPTR ": %s", label, PRIxPTR_WIDTH, addr, lineBuf );
       }
       else
       {
          vcos_log_impl( cat, VCOS_LOG_INFO, "%0*" PRIxPTR ": %s", PRIxPTR_WIDTH, addr, lineBuf );
       }

       addr += 16;
       mem += 16;
       if ( numBytes > 16 )
       {
           numBytes -= 16;
       }
       else
       {
           numBytes = 0;
       }
   }

}

void vcos_log_impl(const VCOS_LOG_CAT_T *cat, VCOS_LOG_LEVEL_T _level, const char *fmt, ...)
{
   va_list ap;
   va_start(ap,fmt);
   vcos_vlog_impl( cat, _level, fmt, ap );
   va_end(ap);
}

void vcos_vlog_impl(const VCOS_LOG_CAT_T *cat, VCOS_LOG_LEVEL_T _level, const char *fmt, va_list args)
{
   vcos_vlog_impl_func( cat, _level, fmt, args );
}

void vcos_set_vlog_impl( VCOS_VLOG_IMPL_FUNC_T vlog_impl_func )
{
   if ( vlog_impl_func == NULL )
   {
      vcos_vlog_impl_func = vcos_vlog_default_impl;
   }
   else
   {
      vcos_vlog_impl_func = vlog_impl_func;
   }
}

VCOS_STATUS_T vcos_log_set_category_level(const char *cat_name, VCOS_LOG_LEVEL_T level)
{
   VCOS_LOG_CAT_T   *cat;
   VCOS_STATUS_T     status;

   if( level > VCOS_LOG_TRACE )
   {
      vcos_log_error( "Unrecognized level: '%d'", level );
      return VCOS_EINVAL;
   }

   vcos_mutex_lock(&lock);

   status = VCOS_SUCCESS;
   for ( cat = vcos_logging_categories; cat != NULL; cat = cat->next )
   {
      if ( strcmp( cat_name, cat->name ) == 0 )
      {
         cat->level = level;
         break;
      }
   }
   if ( cat == NULL )
   {
      vcos_log_error( "Unrecognized category: '%s'", cat_name );
      status = VCOS_ENOENT;
   }

   vcos_mutex_unlock(&lock);

   return status;
}
