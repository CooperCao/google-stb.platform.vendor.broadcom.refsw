/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
APIs for capturing a buffer to file
=============================================================================*/
#include "vcos.h"
#include "vcos_logging.h"
#include "tools/v3d/txtfmt/txtfmt_gfx_buffer.h"
#include "khrn_capture_buffer.h"
#include "interface/khronos/common/khrn_options.h"
#include "v3d_platform/gmem.h"

#define VCOS_LOG_CATEGORY (&khrn_capture_buffer_log)

static VCOS_LOG_CAT_T khrn_capture_buffer_log =
   VCOS_LOG_INIT("khrn_capture_buffer_log", VCOS_LOG_WARN);

static void set_log_level(void)
{
   if (khrn_options.checksum_log_verbose)
   {
      vcos_log_set_level( VCOS_LOG_CATEGORY, VCOS_LOG_INFO );
   }
}

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
   set_log_level();
   vcos_log_info(" >>> capturing image buffer %s", filename);
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
   set_log_level();
   header.type = TXTFMT_TYPE_UNFORMATTED;
   memcpy_from_v3d(buf, begin_addr, buffer_size);
   vcos_log_info(" >>> capturing unformatted buffer %s", filename);
   txtfmt_store(&header, buffer_size, buf, filename, txtfmt_on_error_assert, (void *)filename);
   free(buf);
}
