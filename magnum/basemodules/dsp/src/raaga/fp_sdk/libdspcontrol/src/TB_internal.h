/****************************************************************************
 *                Copyright (c) 2013 Broadcom Corporation                   *
 *                                                                          *
 *      This material is the confidential trade secret and proprietary      *
 *      information of Broadcom Corporation. It may not be reproduced,      *
 *      used, sold or transferred to any third party without the prior      *
 *      written consent of Broadcom Corporation. All rights reserved.       *
 *                                                                          *
 ****************************************************************************/

/**
 * Internal structures and functions of the TB module,
 * declared in this header for use by libdspcontrol tests.
 */

#ifndef _TB_INTERNAL_H_
#define _TB_INTERNAL_H_

/* NOTE: this file gets exported into the Raaga Magnum host library and so it
 *       must abide by a specific strict set of rules. Please use only ANSI C
 *       constructs and include only FPSDK headers which do as well. After any
 *       edit, always make sure that the Raaga Magnum build succeeds without
 *       any warning.
 */

#include "libdspcontrol/CHIP.h"

#if !FEATURE_IS(SW_HOST, RAAGA_MAGNUM)
#  include <stddef.h>
#  include <stdint.h>
#else
#  include "bstd_defs.h"
#endif

#include "libdspcontrol/DSP.h"
#include "libdspcontrol/TB.h"

#include "libsyschip/tbuf.h"
#include "libsyschip/tbuf_chips.h"



#if FEATURE_IS(TB_VARIANT, CIRCULAR)

/**
 * Gets the TB_shared control structure content from the system/DSP/shared memory.
 * Access constraints are respected and endianness is translated as required.
 *
 * @param dsp           the DSP instance
 * @param tb_addr       address of the source TB_shared control structure in shared memory
 * @param tb_addr_space the address space tbAddr refers to
 * @param dest          address of the destination TB_shared structure in local memory
 */
__attribute__((nonnull))
void TB_retrieveShared(DSP *dsp, uint32_t tb_addr, DSP_ADDRESS_SPACE tb_addr_space, TB_shared *dest);


/**
 * Stores the local TB_shared content to system/DSP/shared memory.
 * Access constraints are respected and endianness is translated as required.
 *
 * @param dsp           the DSP instance
 * @param tb_addr       address of the destination TB_shared control structure in shared memory
 * @param tb_addr_space the address space tb_addr refers to
 * @param src           address of the source TB_shared structure in local memory
 */
__attribute__((nonnull))
void TB_storeShared(DSP *dsp, uint32_t tb_addr, DSP_ADDRESS_SPACE tb_addr_space, TB_shared *src);


/**
 * Returns the available number of free bytes in the TB circular buffer.
 *
 * @param tb    the TB structure to access
 * @return      maximum number of bytes that can currently be written
 *              to the TB buffer without overflowing it
 */
__attribute__((nonnull))
size_t TB_freeSpace(TB *tb);


/**
 * Writes data to the given Target Buffer.
 * All or only a portion of the src buffer data can be written,
 * depending on the TB current free space.
 *
 * @param tb     the TB structure to access
 * @param src    buffer containing data to be written
 * @param size   number of bytes to write
 * @return       number of bytes actually written
 */
__attribute__((nonnull))
size_t TB_writeCircular(TB *tb, void *src, size_t size);

#endif  /* FEATURE_IS(TB_VARIANT, CIRCULAR) */


#if !FEATURE_IS(TB_VARIANT, NONE)

/*
 * Macros used by TB_seek, TB_peek, TB_attachToBuffers,
 * shared between 'circular' and 'linear' source code.
 */
/* Moves the read cursor to the first byte of the first buffer. */
#define TB_SEEK_TO_BEGIN(desc)                  \
{                                               \
    desc->cur_offset = 0;                       \
    desc->cur_buffer = 0;                       \
    desc->cur_pointer = desc->buffers[0].data;  \
}


/* Moves the read cursor beyond the last buffer last byte. */
#define TB_SEEK_TO_END(desc)                                                                            \
{                                                                                                       \
    desc->cur_offset = desc->total_size;                                                                \
    desc->cur_buffer = desc->buffers_count - 1;                                                         \
    /* size_t last_buf_size = desc->buffers[desc->cur_buffer].size; */                                  \
    desc->cur_pointer = desc->buffers[desc->cur_buffer].data + desc->buffers[desc->cur_buffer].size;    \
}


#if TARGET_BUFFER_VERSION == 2


/**
 * Fills in a TB_frame_info structure reading from the passed TB_header
 * and frame trailer. If @p descriptor_clone_at_payload is not NULL, a clone
 * of @p descriptor whose read cursor points at payload data is stored in it.
 * The descriptor read cursor should be just after the header when calling
 * this function. If @p move_cursor is true, on return the read cursor will be
 * located after the frame end.
 */
__attribute__((nonnull(1, 2, 3)))
void TB_readFrameInfo(TB_data_descriptor *descriptor,
                      TB_header *frame_header,
                      TB_frame_info *ret_value,
                      TB_data_descriptor *descriptor_clone_at_payload,
                      bool move_cursor);



#if FEATURE_IS(TB_VARIANT, CIRCULAR) && FEATURE_IS(FILE_IO, AVAILABLE)

/* TODO: this function is just a quick hack to make dspcontrol build and function
 * as a test tool for Target Buffer data transfers, this is why its definition has
 * been relegated to the internal-only header. A more evolved "payload dispatcher"
 * function should be designed. */
/**
 * Utility function to extract frames payload data from a Target Buffer
 * and dump it to a file. It tries to read as many (complete) frames as
 * there are available in the TB.
 *
 * @param tb                      the TB structure to access
 * @param services                bitmask of which services data to read
 * @param fd                      file descriptor to append data to
 * @param discard_up_to_read_data after reading, discard up to the last read frame; note that if some
 *                                frames where ignored (wrong service) and not read, they will be discarded
 *                                as well
 * @return                        number of bytes transferred, or -1 if write fails
 */
__attribute__((nonnull))
ssize_t TB_copyFramesPayloadToFile(TB *tb, TB_service_flag services, int fd, bool discard_up_to_read_data);

#endif  /* FEATURE_IS(TB_VARIANT, CIRCULAR) && FEATURE_IS(FILE_IO, AVAILABLE) */
#endif  /* TARGET_BUFFER_VERSION == 2 */
#endif  /* !FEATURE_IS(TB_VARIANT, NONE) */


#endif  /* _TB_INTERNAL_H_ */
