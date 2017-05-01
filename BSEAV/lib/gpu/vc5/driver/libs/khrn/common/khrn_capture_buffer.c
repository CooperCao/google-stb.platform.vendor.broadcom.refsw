/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "vcos.h"
#include "libs/util/log/log.h"
#include "libs/tools/txtfmt/txtfmt_gfx_buffer.h"
#include "libs/sim/gfx_tfu_checksum/gfx_tfu_checksum.h"
#include "khrn_capture_buffer.h"
#include "khrn_options.h"
#include "khrn_memaccess.h"
#include "libs/platform/gmem.h"

LOG_DEFAULT_CAT("khrn_capture_buffer")

void khrn_capture_image_buffer(khrn_memaccess* ma, v3d_addr_t begin_addr,
   v3d_addr_t end_addr, const char *filename, const GFX_BUFFER_DESC_T *desc)
{
   size_t buffer_size = end_addr - begin_addr;
   void *buf = malloc(buffer_size);
   demand(buf);
   khrn_memaccess_read(ma, buf, begin_addr, buffer_size);
   log_trace("capturing image buffer %s", filename);
   txtfmt_store_gfx_buffer(desc, buf, 0, filename, txtfmt_on_error_assert, (void *)filename);
   free(buf);
}

void khrn_capture_unformatted_buffer(khrn_memaccess* ma,
   v3d_addr_t begin_addr, v3d_addr_t end_addr, const char *filename)
{
   TXTFMT_HEADER_T header = {0};
   size_t buffer_size = end_addr - begin_addr;
   void *buf = malloc(buffer_size);
   demand(buf);
   header.type = TXTFMT_TYPE_UNFORMATTED;
   khrn_memaccess_read(ma, buf, begin_addr, buffer_size);
   log_trace("capturing unformatted buffer %s", filename);
   txtfmt_store(&header, buffer_size, buf, filename, txtfmt_on_error_assert, (void *)filename);
   free(buf);
}

/*
 Performs TFU checksum on a V3D buffer

 start_address     - V3D address of start of buffer
 size_bytes        - Length of V3D buffer in bytes
 return value      - CRC of the entire buffer
 */
uint32_t khrn_tfu_checksum_buffer(khrn_memaccess* ma,
   v3d_addr_t start_address, v3d_size_t size_bytes)
{
   uint32_t buffer[4] = {0};

   GFX_TFU_CHECKSUM_STATE_T state;
   bool first = true;

   while(size_bytes > 0)
   {
      unsigned int copy_bytes = GFX_MIN(size_bytes, (unsigned int)sizeof(buffer));

      khrn_memaccess_read(ma, buffer, start_address, copy_bytes);
      if(first)
      {
         first = false;
         gfx_tfu_checksum_add_first_buffer(&state, buffer, sizeof(buffer));
      }
      else
      {
         gfx_tfu_checksum_add_buffer(&state, buffer, sizeof(buffer));
      }

      start_address += copy_bytes;
      size_bytes -= copy_bytes;
   }

   return gfx_tfu_checksum_finalise(&state);
}