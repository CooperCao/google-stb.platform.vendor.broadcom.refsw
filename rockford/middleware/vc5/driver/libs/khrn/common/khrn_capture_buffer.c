/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
APIs for capturing a buffer to file
=============================================================================*/
#include "vcos.h"
#include "libs/util/log/log.h"
#include "libs/tools/txtfmt/txtfmt_gfx_buffer.h"
#include "khrn_capture_buffer.h"
#include "khrn_options.h"
#include "libs/platform/gmem.h"

LOG_DEFAULT_CAT("khrn_capture_buffer")

inline void *memcpy_from_v3d(void *dest, unsigned int v3d_addr_src, unsigned int length)
{
   void *ret = 0;
   memcpy(dest, gmem_addr_to_ptr(v3d_addr_src), length);
   return ret;
}

void khrn_capture_image_buffer(v3d_addr_t begin_addr, v3d_addr_t end_addr,
   const char *filename, const GFX_BUFFER_DESC_T *desc)
{
   size_t buffer_size = end_addr - begin_addr;
   void *buf = malloc(buffer_size);
   demand(buf);
   memcpy_from_v3d(buf, begin_addr, buffer_size);
   log_trace("capturing image buffer %s", filename);
   txtfmt_store_gfx_buffer(desc, buf, 0, filename, txtfmt_on_error_assert, (void *)filename);
   free(buf);
}

void khrn_capture_unformatted_buffer(v3d_addr_t begin_addr, v3d_addr_t end_addr,
   const char *filename)
{
   TXTFMT_HEADER_T header = {0};
   size_t buffer_size = end_addr - begin_addr;
   void *buf = malloc(buffer_size);
   demand(buf);
   header.type = TXTFMT_TYPE_UNFORMATTED;
   memcpy_from_v3d(buf, begin_addr, buffer_size);
   log_trace("capturing unformatted buffer %s", filename);
   txtfmt_store(&header, buffer_size, buf, filename, txtfmt_on_error_assert, (void *)filename);
   free(buf);
}
