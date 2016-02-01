/*==============================================================================
 Copyright (c) 2013 Broadcom Europe Limited.
 All rights reserved.

 Module   : gfx_buffer

 FILE DESCRIPTION
 Calculates the TFU checksum for an expected.txt file
==============================================================================*/

#include "vcos.h"
#include "gfx_tfu_checksum.h"
#include "tools/v3d/txtfmt/txtfmt.h"
#include "helpers/demand.h"

#define VCOS_LOG_CATEGORY (&gfx_tfu_checksum_log)

void gfx_tfu_checksum_check_file(uint32_t checksum_result[4], const char *filename)
{
   unsigned int checksum = 0;
   TXTFMT_HEADER_T header = {0};
   void *raw_buffer = NULL;
   size_t raw_buffer_size = 0;
   size_t aligned_buffer_size = 0;

   // Up to 3 words of trailing unaligned data
   void *trailing_data_src = NULL;
   size_t trailing_size = 0;
   uint32_t trailing_words [3] = {0};

   // Load buffer
   raw_buffer = txtfmt_load(&header, &raw_buffer_size, filename, NULL, NULL,
                            txtfmt_on_error_print_and_exit, (void *)filename);
   demand(raw_buffer);

   // Compute checksum

   // TFU operates at a granularity of 16 byte blocks
   // Cannot guarantee zero padding for buffers, so TFU checksum performed on 16 byte aligned data
   // and up to 3 words of trailing unaligned data is copied for direct comparison
   trailing_size = raw_buffer_size % 16;
   aligned_buffer_size = raw_buffer_size - trailing_size; // Round down to nearest 16 bytes
   checksum = gfx_tfu_checksum_check_buffer(raw_buffer, aligned_buffer_size);

   // Copy up to 3 words worth of trailing unaligned data for direct comparison
   trailing_data_src = (char *)raw_buffer + aligned_buffer_size;
   memcpy(trailing_words, trailing_data_src, trailing_size);

   free(raw_buffer);
   checksum_result[0] = checksum;
   checksum_result[1] = trailing_words[0];
   checksum_result[2] = trailing_words[1];
   checksum_result[3] = trailing_words[2];
   vcos_log_trace("tfu checksum for %s, of %zu bytes (%zu trailing bytes)", filename, raw_buffer_size, trailing_size);
   vcos_log_trace("checksum = 0x%08x, trailing bytes = 0x%08x, 0x%08x, 0x%08x", checksum_result[0], checksum_result[1], checksum_result[2], checksum_result[3]);
}
