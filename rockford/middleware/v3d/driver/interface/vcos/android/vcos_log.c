/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.

Project  :  vcos
Module   :

FILE DESCRIPTION
=============================================================================*/

#define LOG_TAG "V3D"
#include <cutils/log.h>
#include <cutils/properties.h>

#include "interface/vcos/vcos_log.h"

bool vcos_of_interest_impl(void)
{
   static bool tried = false;
   static bool match = false;

   if (!tried)
   {
      tried = true;
      char buffer[256];
      FILE *fp;

      snprintf(buffer, sizeof(buffer), "/proc/%d/cmdline", getpid());

      fp = fopen(buffer, "r");
      if (fp != NULL)
      {
         buffer[0] = '\0';
         fgets(buffer, sizeof(buffer), fp);
         fclose(fp);

         char value[PROPERTY_VALUE_MAX];
         property_get( "debug.egl.hw.logd", value, "" );
         char *filename = basename(buffer);

         if (strcmp(filename, value) == 0)
            match = true;
      }
   }

   return match;
}

static int level_to_android_prio(log_level_t level)
{
   switch (level)
   {
   case LOG_ERROR:   return ANDROID_LOG_ERROR;
   case LOG_WARN:    return ANDROID_LOG_WARN;
   case LOG_INFO:    return ANDROID_LOG_INFO;
   case LOG_TRACE:   return ANDROID_LOG_DEBUG;
   default:          __builtin_unreachable(); return 0;
   }
}

void vcos_log_impl(log_level_t log_level, const char *fmt, ...)
{
   if (vcos_of_interest_impl())
   {
      va_list args;
      va_start(args, fmt);
      __android_log_vprint(level_to_android_prio(log_level), LOG_TAG, fmt, args);
      va_end(args);
   }
}
