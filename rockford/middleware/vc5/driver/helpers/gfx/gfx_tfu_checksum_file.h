/*==============================================================================
 Copyright (c) 2013 Broadcom Europe Limited.
 All rights reserved.

 Module   : gfx_buffer

 FILE DESCRIPTION
 Calculates the TFU checksum for an expected.txt file
==============================================================================*/
#ifndef GFX_TFU_CHECKSUM_FILE
#define GFX_TFU_CHECKSUM_FILE

#include "vcos_types.h"
#include <stdint.h>

VCOS_EXTERN_C_BEGIN

/*
 Performs checksum on an expected.txt file.

 @param checksum_result  Array containing checksum and up to 3 words of trailing data.
                         Expected.txt files with non-16-byte-aligned data will have up to
                         3 words of trailing data.
 @param filename         Name of expected.txt file
 */
extern void gfx_tfu_checksum_check_file(uint32_t checksum_result[4], const char *filename);

VCOS_EXTERN_C_END

#endif // GFX_TFU_CHECKSUM_FILE
