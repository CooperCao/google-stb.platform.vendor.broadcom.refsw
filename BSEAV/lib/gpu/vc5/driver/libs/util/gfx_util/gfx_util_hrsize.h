/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef GFX_UTIL_HRSIZE_H
#define GFX_UTIL_HRSIZE_H

#include "gfx_util.h"

EXTERN_C_BEGIN

/* Human-readable size, eg 1024 --> 1 kB */

extern void gfx_util_hrsize_internal(char *buf, size_t buf_size, size_t size);

#define GFX_UTIL_HRSIZE(BUF_NAME, SIZE) \
   char BUF_NAME[32]; \
   gfx_util_hrsize_internal(BUF_NAME, sizeof(BUF_NAME), SIZE)

EXTERN_C_END

#endif
