/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "helpers/assert.h"
#include "helpers/demand.h"
#include "vcos.h"
#include "helpers/gfx/gfx_options.h"
#include <stdarg.h>
#include <ctype.h>
#ifdef _MSC_VER
#define strtoull _strtoui64
#endif

static VCOS_LOG_CAT_T gfx_options_log = VCOS_LOG_INIT("gfx_options", VCOS_LOG_INFO);
#define VCOS_LOG_CATEGORY (&gfx_options_log)

static VCOS_TLS_KEY_T tl_cat;

static void init(void)
{
   VCOS_STATUS_T status;

   status = vcos_tls_create(&tl_cat, NULL);
   demand(status == VCOS_SUCCESS);
}

static VCOS_ONCE_T once = VCOS_ONCE_INIT;

void gfx_options_begin(const char *cat)
{
   demand(vcos_once(&once, init) == VCOS_SUCCESS);

   demand(vcos_tls_set(tl_cat, (void *)cat) == VCOS_SUCCESS);
}

void gfx_options_end(void)
{
   demand(vcos_tls_set(tl_cat, NULL) == VCOS_SUCCESS);
}

static const char *cat(void)
{
   const char *c = (const char *)vcos_tls_get(tl_cat);
   assert(c);
   return c;
}

#define log(LEVEL, STR, ...) vcos_log_##LEVEL("[%s options] " STR, cat(), __VA_ARGS__)
#define log_warn(...) log(warn, __VA_ARGS__)
#define log_info(...) log(info, __VA_ARGS__)
#define log_trace(...) log(trace, __VA_ARGS__)

static int read_option(const char *name, char * value, size_t value_size,
      const char *default_value)
{
#ifdef DISABLE_OPTION_PARSING
   return 0;
#else
   return vcos_property_get(name, value, value_size, default_value);
#endif
}

bool gfx_options_bool(const char *name, bool def)
{
   char value[VCOS_PROPERTY_VALUE_MAX];

   if (read_option(name, value, sizeof(value), NULL) < 1)
   {
      log_trace("%s=%s (default)", name, def ? "true" : "false");
      return def;
   }

   if (value[0] == 't' || value[0] == 'T' || value[0] == 'y' || value[0] == 'Y' || value[0] == '1')
   {
      log_info("%s=true", name);
      return true;
   }

   if (value[0] == 'f' || value[0] == 'F' || value[0] == 'n' || value[0] == 'N' || value[0] == '0')
   {
      log_info("%s=false", name);
      return false;
   }

   log_warn(
      "The GFX option %s is set to '%s'.\n"
      "Expected some kind of boolean value (t/T/y/Y/1 = true or f/F/n/N/0 = false).\n"
      "Using default of %s.",
      name, value, def ? "true" : "false");

   return def;
}

uint32_t gfx_options_uint32(const char *name, uint32_t def)
{
   char value[VCOS_PROPERTY_VALUE_MAX];
   uint32_t i;

   if (read_option(name, value, sizeof(value), NULL) < 1)
   {
      log_trace("%s=%"PRIu32" (0x%"PRIx32") (default)", name, def, def);
      return def;
   }

   /* TODO Check opt is sane? */
   i = (uint32_t)strtoul(value, NULL, 0);
   //log_info("%s=%"PRIu32" (0x%"PRIx32")", name, i, i); TODO This does not work on VisualStudio - why?
   vcos_log_info("[%s options] " "%s=%"PRIu32" (0x%"PRIx32")", cat(), name, i, i);
   return i;
}

uint64_t gfx_options_uint64(const char *name, uint64_t def)
{
   char value[VCOS_PROPERTY_VALUE_MAX];
   uint64_t i;

   if (read_option(name, value, sizeof(value), NULL) < 1)
   {
      log_trace("%s=%"PRIu64" (0x%"PRIx64") (default)", name, def, def);
      return def;
   }

   /* TODO Check opt is sane? */
   i = (uint64_t)strtoull(value, NULL, 0);
   log_info("%s=%"PRIu64" (0x%"PRIx64")", name, i, i);
   return i;
}

int32_t gfx_options_int32(const char *name, int32_t def)
{
   char value[VCOS_PROPERTY_VALUE_MAX];
   int32_t i;

   if (read_option(name, value, sizeof(value), NULL) < 1)
   {
      log_trace("%s=%"PRId32" (default)", name, def);
      return def;
   }

   /* TODO Check opt is sane? */
   i = (int32_t)strtol(value, NULL, 0);
   //log_info("%s=%"PRId32, name, i); TODO This does not work on VisualStudio - why?
   vcos_log_info("[%s options] " "%s=%"PRId32" (0x%"PRIx32")", cat(), name, i, i);
   return i;
}

int gfx_options_str(const char *name, const char *def, char * value, size_t value_size)
{
   int size = read_option(name, value, value_size, def);

   if (size < 1)
   {
      log_trace("%s=%s%s%s (default)", name,
         def ? "'" : "", def ? def : "NULL", def ? "'" : "");
   }
   else if (size > (int)value_size)
   {
      log_warn(
         "The GFX option %s was returned truncated to '%s'.\n"
         "The provided buffer of %lu bytes was too short for the %lu bytes long value.",
         name, value, (unsigned long)value_size, (unsigned long)size);
   }
   else
   {
      log_info("%s='%s'", name, value);
   }
   return size;
}

int gfx_options_enum(const char *name, int def,
   /* List of (enum_name, enum_value) pairs ending with enum_name == NULL */
   ...)
{
   va_list args;
   const char *def_name;
   char value[VCOS_PROPERTY_VALUE_MAX];

   va_start(args, def);
   for (;;)
   {
      const char *enum_name = va_arg(args, const char *);
      assert(enum_name); /* Or we didn't find def in list */
      if (def == va_arg(args, int))
      {
         def_name = enum_name;
         break;
      }
   }
   va_end(args);

   if (read_option(name, value, sizeof(value), NULL) < 1)
   {
      log_trace("%s=%s (default)", name, def_name);
      return def;
   }

   va_start(args, def);
   for (;;)
   {
      const char *enum_name = va_arg(args, const char *);
      int enum_value;
      if (!enum_name)
      {
         break;
      }
      enum_value = va_arg(args, int);
      if (!strcmp(value, enum_name))
      {
         va_end(args);
         log_info("%s=%s", name, enum_name);
         return enum_value;
      }
   }
   va_end(args);

   /* Didn't find opt in list... */

   {
      char message[256];
      size_t offset = 0;
      bool first;

      offset = VCOS_SAFE_SPRINTF(message, offset,
         "The GFX option %s is set to '%s'.\n"
         "Expected one of ",
         name, value);

      va_start(args, def);
      first = true;
      for (;;)
      {
         const char *enum_name = va_arg(args, const char *);
         if (!enum_name)
         {
            break;
         }
         va_arg(args, int);
         offset = VCOS_SAFE_SPRINTF(message, offset, "%s%s",
            first ? "" : ", ",
            enum_name);
         first = false;
      }
      va_end(args);

      offset = VCOS_SAFE_SPRINTF(message, offset,
         ".\n"
         "Using default of %s.",
         def_name);

      log_warn("%s", message);
   }

   return def;
}
