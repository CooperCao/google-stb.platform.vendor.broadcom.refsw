/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.

Project  :  vcos
Module   :

FILE DESCRIPTION
=============================================================================*/

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#include "interface/vcos/vcos_log.h"

bool vcos_of_interest_impl(void)
{
   return false;
}

void vcos_log_impl(log_level_t level, const char *fmt, ...)
{
   VCOS_UNUSED(level);
   char buffer[256];
   va_list args;
   va_start(args, fmt);
   vsnprintf(buffer, sizeof(buffer), fmt, args);
   printf("%s\n", buffer);
}
