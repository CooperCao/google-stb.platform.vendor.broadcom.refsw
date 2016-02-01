/*==============================================================================
 Copyright (c) 2013 Broadcom Europe Limited.
 All rights reserved.

 Module   : gfx_buffer

 FILE DESCRIPTION
 Calculates the TFU checksum for a buffer
==============================================================================*/
#ifndef GFX_TFU_CHECKSUM
#define GFX_TFU_CHECKSUM

#include "vcos.h"
#include <stdbool.h>
#include "vcos_logging.h"
#include <stdio.h>

VCOS_EXTERN_C_BEGIN

extern VCOS_LOG_CAT_T gfx_tfu_checksum_log;

/*
 Sets vcos log level for gfx tfu checksum functions

 @param log_level    logging level
 @param want_prefix  enables/disables vcos category prefix - useful for command line tools
 */
extern void gfx_tfu_checksum_set_log_level(VCOS_LOG_LEVEL_T log_level, bool want_prefix);

typedef struct
{
   uint64_t v;
} GFX_TFU_CHECKSUM_STATE_T;

/*
 Initiates a chained TFU checksum on a buffer.

 @param state  Information used to persist state across a session of chained checksums.
 @param buffer Word aligned pointer to first buffer containing data to checksum.
 @param length Size of buffer in bytes. Buffer size is assumed to be 16 byte aligned.
 */
extern void gfx_tfu_checksum_add_first_buffer(GFX_TFU_CHECKSUM_STATE_T *state, const uint32_t *buffer, size_t length);

/*
 Continues a chained TFU checksum on a buffer.

 @param state  Information used to persist state across a session of chained checksums.
 @param buffer Word aligned pointer to a buffer containing data to checksum (which is not the first buffer).
 @param length Size of buffer in bytes. Buffer size is assumed to be 16 byte aligned.
 */
extern void gfx_tfu_checksum_add_buffer(GFX_TFU_CHECKSUM_STATE_T *state, const uint32_t *buffer, size_t length);

/*
 Computes a TFU checksum result for a chained checksums.

 @param state  Information used to persist state across a session of chained checksums.

 @return Checksum of series of supplied buffers - CRC32 polynomial, 0X04C11DB7.
 */
extern uint32_t gfx_tfu_checksum_finalise(const GFX_TFU_CHECKSUM_STATE_T *state);

/*
 Performs TFU checksum on a buffer.

 @param buffer  Word aligned pointer to buffer containing data to checksum.
 @param length  Size of buffer in bytes. Buffer size is assumed to be 16 byte aligned.

 @return Checksum of supplied buffer - CRC32 polynomial, 0X04C11DB7.
 */
extern uint32_t gfx_tfu_checksum_check_buffer(const uint32_t *buffer, size_t length);

VCOS_EXTERN_C_END

#endif // GFX_TFU_CHECKSUM
