/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <stdio.h>
#include <stdarg.h>

#include "debuglog.h"

extern "C" bool process_match(void);

void debug_log(int level, const char *f, ...)
{
   static bool alreadyInited = false;
   static bool found = false;

   if (!alreadyInited)
      found = process_match();

   // DEBUG_ERROR will print reguardless of process id
   if (found || (level == DEBUG_ERROR))
   {
      va_list args;
      va_start(args, f);
      vfprintf((level == DEBUG_WARN ? stdout : stderr), f, args);
      va_end(args);
   }
}
