/*==============================================================================
 Copyright (c) 2015 Broadcom Europe Limited.
 All rights reserved.

 Module   :  khrn_mr_crc.c

 FILE DESCRIPTION
 Saves CRC checksums after an fmem flush.
==============================================================================*/
#include "khrn_mr_crc.h"
#include "tools/v3d/mr/mr.h"
#include "middleware/khronos/common/khrn_fmem.h"
#include "middleware/khronos/common/khrn_control_list.h"
#include "middleware/khronos/common/khrn_memaccess.h"
#include "interface/khronos/common/khrn_options.h"
#include "middleware/khronos/common/khrn_capture_buffer.h"
#include "helpers/gfx/gfx_tfu_checksum.h"
#include "helpers/demand.h"
#include "helpers/snprintf.h"
#include <algorithm>
#include <vector>

namespace
{

#define VCOS_LOG_CATEGORY (&log_cat)
VCOS_LOG_CAT_T log_cat = VCOS_LOG_INIT("khrn_mr_crc",
     VCOS_LOG_WARN);

unsigned int tfu_checksum_buffer_index = 0;
FILE *checksum_capture_file = NULL;

bool checksum_capture_enabled(void)
{
   return (khrn_options.checksum_capture_filename != NULL);
}

bool checksum_buffer_capture_enabled(void)
{
   // Dumped buffers are based off the checksum list filename.
   return checksum_capture_enabled();
}

void store_output_buffer_checksum(unsigned int buffer_index, size_t buffer_size, uint32_t checksum,
                               const char *buffer_info)
{
   demand(khrn_options.checksum_capture_filename);

   if (!checksum_capture_file)
   {
     checksum_capture_file = fopen(khrn_options.checksum_capture_filename, "w");
     demand(checksum_capture_file);
   }
   fprintf(checksum_capture_file, "%u %u %08x // %s\n",
         buffer_index, (unsigned int)buffer_size, checksum, buffer_info);
   fflush(checksum_capture_file);
}

void output_buffer_desc(char *buf, size_t buf_size, const MR_TYPE_T *type)
{
   size_t offset = 0;
   switch (type->kind)
   {
   case MR_KIND_IMAGE:
      offset = vcos_safe_sprintf(buf, buf_size, offset, "image: ");
      offset = gfx_buffer_sprint_desc(buf, buf_size, offset, &type->u.image.desc);
      break;
   case MR_KIND_RAW_DATA:
      assert(type->u.raw_data.used_for_tf);
      offset = vcos_safe_sprintf(buf, buf_size, offset, "tf");
      break;
   default:
      unreachable();
   }
   assert(offset < buf_size); /* Or we truncated it */
}

/*
 Performs TFU checksum on a V3D buffer

 start_address     - V3D address of start of buffer
 size_bytes        - Length of V3D buffer in bytes
 return value      - CRC of the entire buffer
 */
unsigned int khrn_tfu_checksum_buffer(unsigned int start_address, unsigned int size_bytes)
{
   uint32_t buffer[4] = {0};

   GFX_TFU_CHECKSUM_STATE_T state;
   bool first = true;

   while(size_bytes > 0)
   {
      unsigned int copy_bytes = GFX_MIN(size_bytes, (unsigned int)sizeof(buffer));

      memcpy_from_v3d(buffer, start_address, copy_bytes);
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

struct output_buffer
{
   const MR_RANGE_T *begin_range;
   v3d_addr_t end_addr;

   /* first_access_id order should be fairly stable, so use that rather than eg
    * ordering by address */
   bool operator<(const output_buffer &other) const
   {
      return begin_range->first_access_id < other.begin_range->first_access_id;
   }
};

void handle_output_buffer(const output_buffer &output_buf)
{
   v3d_addr_t begin_addr = output_buf.begin_range->begin_addr;
   v3d_addr_t end_addr = output_buf.end_addr;
   const MR_TYPE_T *type = &output_buf.begin_range->type;

   switch (type->kind)
   {
   case MR_KIND_RAW_DATA:
   case MR_KIND_IMAGE:
      break;
   case MR_KIND_QUERY_COUNTER:
   case MR_KIND_TILE_STATE:
   case MR_KIND_TILE_ALLOC:
      return;
   default:
      /* Shouldn't get writes of any other kind */
      unreachable();
   }

   uint32_t checksum;
   uint32_t buffer_size = end_addr - begin_addr;

   // Compute checksum/capture buffer if index is within range
   if (tfu_checksum_buffer_index >= khrn_options.checksum_start_buffer_index &&
      ((tfu_checksum_buffer_index < khrn_options.checksum_end_buffer_index) ||
      khrn_options.checksum_end_buffer_index == UINT32_MAX))
   {
     char buffer_info[512];
     output_buffer_desc(buffer_info, sizeof(buffer_info), type);

     checksum = khrn_tfu_checksum_buffer(begin_addr, buffer_size);

     vcos_log_info(" >>> @%08x:%08x id:%u size:%u checksum:%08x // %s",
               begin_addr, end_addr, tfu_checksum_buffer_index, buffer_size,
               checksum, buffer_info);

     if (checksum_capture_enabled())
     {
       store_output_buffer_checksum(tfu_checksum_buffer_index, buffer_size, checksum, buffer_info);
     }

     if (checksum_buffer_capture_enabled())
     {
        // Store output buffer to file prefixed with the name of the checksum list file.
        char filename[1024] = {0};
        demand(snprintf(filename, sizeof(filename), "%s_buffer_%.4u_clif_%.4u.txt",
                    khrn_options.checksum_capture_filename, tfu_checksum_buffer_index, khrn_fmem_frame_i) > 0);
        switch(type->kind)
        {
           case MR_KIND_IMAGE:
              assert(type->u.image.base_addr == begin_addr);
              khrn_capture_image_buffer(begin_addr, end_addr, filename, &type->u.image.desc);
              break;
           case MR_KIND_RAW_DATA:
              khrn_capture_unformatted_buffer(begin_addr, end_addr, filename);
              break;
           default:
              unreachable();
        }
     }

   }

   tfu_checksum_buffer_index++;

   // Terminate execution if end buffer index exceeded (and explicitly specified)
   if (tfu_checksum_buffer_index > khrn_options.checksum_end_buffer_index &&
      khrn_options.checksum_end_buffer_index != UINT32_MAX)
   {
     vcos_log_warn("checksums completed for buffer indices %d to %d, exiting",
               khrn_options.checksum_start_buffer_index,
               khrn_options.checksum_end_buffer_index);
     exit(0);
   }
}

void collect_output_buffers(std::vector<output_buffer> &output_buffers, const MR_T *mr)
{
   output_buffer b;
   b.begin_range = NULL;

   for (const auto &r : mr->rangeset)
   {
      /* Merge contiguous TF ranges */
      if (b.begin_range && mr_type_is_tf_output(&b.begin_range->type) &&
         (b.end_addr == r.begin_addr) && mr_type_is_tf_output(&r.type))
      {
         assert(r.type.write);
         b.end_addr = r.end_addr;
         continue;
      }

      if (b.begin_range)
      {
         output_buffers.push_back(b);
         b.begin_range = NULL;
      }

      if (r.type.write)
      {
         b.begin_range = &r;
         b.end_addr = r.end_addr;
      }
   }

   if (b.begin_range)
      output_buffers.push_back(b);
}

void handle_output_buffers(const MR_T *mr)
{
   std::vector<output_buffer> output_buffers;
   collect_output_buffers(output_buffers, mr);

   std::sort(output_buffers.begin(), output_buffers.end());

   for (const auto &b : output_buffers)
      handle_output_buffer(b);
}

void do_bin(MR_T *mr, const V3D_BIN_RENDER_INFO_T *br_info)
{
   uint32_t core;
   for (core = 0; core != br_info->num_bins; ++core)
   {
       uint32_t num_flushes;
       num_flushes = mr_bin_with_unspecified_mem(NULL, mr, br_info->bin_begins[core],
                                            br_info->bin_ends[core], MR_CLE_MODE_BIN);
       (void) num_flushes;
   }
}

void do_render(MR_T *mr, const V3D_BIN_RENDER_INFO_T *br_info)
{
   uint32_t core;
   for (core = 0; core != br_info->num_renders; ++core)
   {
       uint32_t num_flushes;
       num_flushes = mr_render_with_unspecified_tile_list_bases(mr,
                            br_info->render_begins[core], br_info->render_ends[core],
                            br_info->num_bins);
       (void) num_flushes;
   }
}

}

void khrn_save_crc_checksums(const V3D_BIN_RENDER_INFO_T *br_info)
{
   MR_T mr;
   mr_init(&mr, &khrn_memaccess_ro, khrn_get_hw_misccfg());

   do_bin(&mr, br_info);
   do_render(&mr, br_info);

   handle_output_buffers(&mr);

   mr_term(&mr);
}
