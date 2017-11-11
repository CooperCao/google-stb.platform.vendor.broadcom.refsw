/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#define LOG_TAG "V3D"
#include <cutils/log.h>
#include <cutils/properties.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "interface/vcos/vcos_log.h"

static FILE *fp = NULL;

static void vcos_create_directory(const char *dir)
{
   struct stat s;
   int res = stat(dir, &s);
   if (res == 0 && s.st_mode & S_IFDIR)
      __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "********** %s ********** exists", dir);
   else
      // create the directory
      mkdir(dir, 0770);
}

static void vcos_open_file(const char *process_name)
{
   char name[256];
   snprintf(name, sizeof(name), "/data/data/%s", process_name);
   vcos_create_directory(name);

   snprintf(name, sizeof(name), "/data/data/%s/logs", process_name);
   vcos_create_directory(name);

   snprintf(name, sizeof(name), "/data/data/%s/logs/log_%d.txt", process_name, getpid());

   __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "********** %s ********** opened", name);

   fp = fopen(name, "w");
}

static bool vcos_of_interest(void)
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
         {
            match = true;
            vcos_open_file(filename);
         }
         else
         {
            property_get( "debug.egl.hw.logd1", value, "" );
            if (strcmp(filename, value) == 0)
            {
               match = true;
               vcos_open_file(filename);
            }
         }
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
   if (vcos_of_interest())
   {
      if (fp)
      {
         va_list args;
         va_start(args, fmt);
         vfprintf(fp, fmt, args);
         fputc('\n', fp);
         va_end(args);
      }
      else
      {
         /* android can loose logd, so this is not really very reliable for debug */
         va_list args;
         va_start(args, fmt);
         __android_log_vprint(level_to_android_prio(log_level), LOG_TAG, fmt, args);
         va_end(args);
      }
   }
}
