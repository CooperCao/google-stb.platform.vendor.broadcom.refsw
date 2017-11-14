/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#include "interface/vcos/vcos_log.h"

void vcos_log_impl(log_level_t level, const char *fmt, ...)
{
   VCOS_UNUSED(level);
   char buffer[256];
   va_list args;
   va_start(args, fmt);
   vsnprintf(buffer, sizeof(buffer), fmt, args);
   printf("%s\n", buffer);
}
