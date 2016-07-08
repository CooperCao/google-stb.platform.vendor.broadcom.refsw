/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#include "libs/util/assert_helpers.h"
#include "libs/util/demand.h"
#include "vcos.h"
#include "gfx_options.h"
#include "libs/util/gfx_util/gfx_util_str.h"
#include <stdarg.h>
#include <ctype.h>
#ifdef _MSC_VER
#define strtoull _strtoui64
#endif

static int read_option(const char *name, char * value, size_t value_size)
{
#ifdef DISABLE_OPTION_PARSING
   return 0;
#else
   return vcos_property_get(name, value, value_size, NULL);
#endif
}

bool gfx_options_bool_log_cat(struct log_cat *cat, const char *name, bool def)
{
   char value[VCOS_PROPERTY_VALUE_MAX];
   if (read_option(name, value, sizeof(value)) < 1)
   {
      log_cat_trace(cat, "%s=%s (default)", name, def ? "true" : "false");
      return def;
   }

   if (value[0] == 't' || value[0] == 'T' || value[0] == 'y' || value[0] == 'Y' || value[0] == '1')
   {
      log_cat_info(cat, "%s=true", name);
      return true;
   }

   if (value[0] == 'f' || value[0] == 'F' || value[0] == 'n' || value[0] == 'N' || value[0] == '0')
   {
      log_cat_info(cat, "%s=false", name);
      return false;
   }

   log_cat_warn(cat,
      "The GFX option %s is set to '%s'.\n"
      "Expected some kind of boolean value (t/T/y/Y/1 = true or f/F/n/N/0 = false).\n"
      "Using default of %s.",
      name, value, def ? "true" : "false");

   return def;
}

uint32_t gfx_options_uint32_log_cat(struct log_cat *cat, const char *name, uint32_t def)
{
   char value[VCOS_PROPERTY_VALUE_MAX];
   if (read_option(name, value, sizeof(value)) < 1)
   {
      log_cat_trace(cat, "%s=%" PRIu32 " (0x%" PRIx32 ") (default)", name, def, def);
      return def;
   }

   uint32_t u = 0;
   if (!gfx_try_strtou32(&u, value, 0))
   {
      log_cat_warn(cat,
         "The GFX option %s is set to '%s'.\n"
         "Expected a 32-bit unsigned integer.\n"
         "Using default of %" PRIu32 ".",
         name, value, def);
      return def;
   }

   log_cat_info(cat, "%s=%" PRIu32 " (0x%" PRIx32 ")", name, u, u);
   return u;
}

uint64_t gfx_options_uint64_log_cat(struct log_cat *cat, const char *name, uint64_t def)
{
   char value[VCOS_PROPERTY_VALUE_MAX];
   if (read_option(name, value, sizeof(value)) < 1)
   {
      log_cat_trace(cat, "%s=%" PRIu64 " (0x%" PRIx64 ") (default)", name, def, def);
      return def;
   }

   uint64_t u = 0;
   if (!gfx_try_strtou64(&u, value, 0))
   {
      log_cat_warn(cat,
         "The GFX option %s is set to '%s'.\n"
         "Expected a 64-bit unsigned integer.\n"
         "Using default of %" PRIu64 ".",
         name, value, def);
      return def;
   }

   log_cat_info(cat, "%s=%" PRIu64 " (0x%" PRIx64 ")", name, u, u);
   return u;
}

int32_t gfx_options_int32_log_cat(struct log_cat *cat, const char *name, int32_t def)
{
   char value[VCOS_PROPERTY_VALUE_MAX];
   if (read_option(name, value, sizeof(value)) < 1)
   {
      log_cat_trace(cat, "%s=%" PRId32 " (default)", name, def);
      return def;
   }

   int32_t i;
   if (!gfx_try_strtoi32(&i, value, 0))
   {
      log_cat_warn(cat,
         "The GFX option %s is set to '%s'.\n"
         "Expected a 32-bit signed integer.\n"
         "Using default of %" PRId32 ".",
         name, value, def);
      return def;
   }

   log_cat_info(cat, "%s=%" PRId32 " (0x%" PRIx32 ")", name, i, i);
   return i;
}

double gfx_options_double_log_cat(struct log_cat *cat, const char *name, double def)
{
   char value[VCOS_PROPERTY_VALUE_MAX];
   if (read_option(name, value, sizeof(value)) < 1)
   {
      log_cat_trace(cat, "%s=%f (default)", name, def);
      return def;
   }

   double d = 0;
   if (!gfx_try_strtod(&d, value))
   {
      log_cat_warn(cat,
         "The GFX option %s is set to '%s'.\n"
         "Expected a float.\n"
         "Using default of %f.",
         name, value, def);
      return def;
   }

   log_cat_info(cat, "%s=%f", name, d);
   return d;
}

void gfx_options_str_log_cat(struct log_cat *cat, const char *name, const char *def, char * value, size_t value_size)
{
   assert(def);

   int size = read_option(name, value, value_size);

   if (size < 1)
   {
      size = vcos_safe_strcpy(value, def, value_size, 0);
      log_cat_trace(cat, "%s='%s' (default)", name, value);
   }
   else
   {
      log_cat_info(cat, "%s='%s'", name, value);
   }

   if (size >= (int)value_size)
   {
      log_cat_warn(cat,
         "The GFX option %s was returned truncated to '%s'. "
         "The provided buffer of %lu bytes was too short for the %lu bytes long value.",
         name, value, (unsigned long)value_size, (unsigned long)size + 1);
   }
}

int gfx_options_enum_log_cat(struct log_cat *cat, const char *name, int def,
   /* List of (enum_name, enum_value) pairs ending with enum_name == NULL */
   ...)
{
   va_list args;

   va_start(args, def);
   const char *def_name;
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

   char value[VCOS_PROPERTY_VALUE_MAX];
   if (read_option(name, value, sizeof(value)) < 1)
   {
      log_cat_trace(cat, "%s=%s (default)", name, def_name);
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
         log_cat_info(cat, "%s=%s", name, enum_name);
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

      log_cat_warn(cat, "%s", message);
   }

   return def;
}
