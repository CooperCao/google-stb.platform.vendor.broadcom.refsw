/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#ifndef __FATAL_ERROR_H__
#define __FATAL_ERROR_H__

#include <stdio.h>
#include <stdlib.h>

static void fatalError(const char *msg, const char *function, int line)
{
   printf("FATAL : %s (%s, %d)\n", msg, function, line);
   exit(0);
}
#define FATAL_ERROR(MSG) fatalError(MSG, __FUNCTION__, __LINE__)

#endif
