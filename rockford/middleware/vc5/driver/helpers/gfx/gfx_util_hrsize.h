/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GFX_UTIL_HRSIZE_H
#define GFX_UTIL_HRSIZE_H

#include "helpers/gfx/gfx_util.h"

VCOS_EXTERN_C_BEGIN

/* Human-readable size, eg 1024 --> 1 kB */

extern void gfx_util_hrsize_internal(char *buf, size_t buf_size, size_t size);

#define GFX_UTIL_HRSIZE(BUF_NAME, SIZE) \
   char BUF_NAME[32]; \
   gfx_util_hrsize_internal(BUF_NAME, sizeof(BUF_NAME), SIZE)

VCOS_EXTERN_C_END

#endif
