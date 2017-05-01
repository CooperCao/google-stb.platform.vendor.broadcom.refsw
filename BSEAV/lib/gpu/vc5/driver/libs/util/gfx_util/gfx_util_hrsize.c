/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "gfx_util_hrsize.h"
#include "vcos_string.h"

void gfx_util_hrsize_internal(char *buf, size_t buf_size, size_t size)
{
   size_t offset = 0;

   if (size < 1024)
      offset = vcos_safe_sprintf(buf, buf_size, offset, "%"PRIuMAX" byte%s", (uintmax_t)size, (size == 1) ? "" : "s");
   else if (size < (1024 * 1024))
      offset = vcos_safe_sprintf(buf, buf_size, offset, "%g kB", size / 1024.0f);
   else
      offset = vcos_safe_sprintf(buf, buf_size, offset, "%g MB", size / (1024.0f * 1024.0f));

   assert(offset < buf_size); /* Or we truncated it */
}
