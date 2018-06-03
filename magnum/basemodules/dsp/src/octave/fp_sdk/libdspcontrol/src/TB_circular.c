/****************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ****************************************************************************/

/* NOTE: this file gets exported into the Raaga Magnum host library and so it
 *       must abide by a specific strict set of rules. Please use only ANSI C
 *       constructs and include only FPSDK headers which do as well. After any
 *       edit, always make sure that the Raaga Magnum build succeeds without
 *       any warning.
 */

#include "libdspcontrol/CHIP.h"
#include "fp_sdk_config.h"

#if !FEATURE_IS(SW_HOST, RAAGA_MAGNUM)
#  include <inttypes.h>
#  include <limits.h>
#  include <stdint.h>
#  include <stdlib.h>
#else
#  include "bstd_defs.h"
#  include "DSP/DSP_raaga_inttypes.h"
#endif

#include "libdspcontrol/COMMON.h"
#include "libdspcontrol/DSP.h"
#include "libdspcontrol/DSPLOG.h"
#include "libdspcontrol/TB.h"

#if FEATURE_IS(FILE_IO, AVAILABLE) && !(IS_TARGET(RaagaFP4015_si_magnum_permissive))
#  include "libdspcontrol/OS.h"
#endif

#include "libfp/c_utils.h"

#include "libsyschip/tbuf.h"
#include "libsyschip/tbuf_chips.h"
#include "libsyschip/tbuf_services.h"

#include "libfp/src/c_utils_internal.h"

#include "ENDIANESS.h"
#include "TB_internal.h"
#include "UTIL_addr.h"



#ifndef TB_HAS_SHARED_STRUCT
#  error "This TB version is intended only for chips with TB_shared structure support"
#endif
#if !FEATURE_IS(TB_VARIANT, CIRCULAR)
#  error "This source file is only for the TB circular variant"
#endif



#define TB_NO_NAME_STRING   "<unnamed>"


/* --------------------
 *  Internal functions
 * --------------------*/
__unused
static void compile_time_assertions(void)
{
    /* Relying on this for printing and endianess swapping */
    COMPILE_TIME_ASSERT(sizeof(TB_BUFF_PTR) == 4);
    COMPILE_TIME_ASSERT(STRUCT_MEMBER_SIZE(TB_shared, dummy1) == 1);
    COMPILE_TIME_ASSERT(STRUCT_MEMBER_SIZE(TB_shared, dummy2) == 1);
    COMPILE_TIME_ASSERT(STRUCT_MEMBER_SIZE(TB_shared, storage_kbytes) == 2);
    COMPILE_TIME_ASSERT(STRUCT_MEMBER_SIZE(TB_shared, storage) == 4);
    COMPILE_TIME_ASSERT(STRUCT_MEMBER_SIZE(TB_shared, read_ptr) == 4);
    COMPILE_TIME_ASSERT(STRUCT_MEMBER_SIZE(TB_shared, write_ptr) == 4);
}


void TB_retrieveShared(DSP *dsp, ADDR tb_addr, TB_shared *dest)
{
    ADDR_SIZE tb_size;
    UTIL_addr_size_assign_from(tb_size, size_t, sizeof(TB_shared));
    DSP_readData(dsp, dest, tb_addr, tb_size);

/*    dst->dummy1 = dst->dummy1;    no byte swap possible on bytes */
/*    dst->dummy2 = dst->dummy2;                                   */
    dest->storage_kbytes = ENDIANESS_fptoh16(dest->storage_kbytes);
    dest->storage = ENDIANESS_fptoh32(dest->storage);
    dest->write_ptr = ENDIANESS_fptoh32(dest->write_ptr);
    dest->read_ptr = ENDIANESS_fptoh32(dest->read_ptr);
}


void TB_storeShared(DSP *dsp, ADDR tb_addr, TB_shared *src)
{
    ADDR_SIZE tb_size;
    TB_shared temp;
    temp.dummy1 = src->dummy1;
    temp.dummy2 = src->dummy2;
    temp.storage_kbytes = ENDIANESS_htofp16(src->storage_kbytes);
    temp.storage = ENDIANESS_htofp32(src->storage);
    temp.write_ptr = ENDIANESS_htofp32(src->write_ptr);
    temp.read_ptr = ENDIANESS_htofp32(src->read_ptr);

    UTIL_addr_size_assign_from(tb_size, size_t, sizeof(TB_shared));
    DSP_writeData(dsp, tb_addr, &temp, tb_size);
}


#if !B_REFSW_MINIMAL

TB_BUFF_SIZE TB_freeSpace(TB *tb)
{
    /* the TB buffer can hold a maximum of buffer_size - 1 bytes */
    return tb->buffer_size - TB_availableData(tb) - 1;
}


TB_BUFF_SIZE TB_writeCircular(TB *tb, void *src, TB_BUFF_SIZE size)
{
    TB_shared local_tb;
    TB_BUFF_PTR tb_end;
    uint8_t *src_cursor;
    TB_BUFF_SIZE written_amount;

    TB_retrieveShared(tb->dsp, tb->tb_addr, &local_tb);
    tb_end = local_tb.storage + tb->buffer_size;
    src_cursor = src;
    written_amount = 0;

    DSPLOG_JUNK("TB: TB_shared %s, storage=0x%08" PRIx32 ", write ptr=0x%08" PRIx32 ", read ptr=0x%08" PRIx32,
                tb->display_id, local_tb.storage, local_tb.write_ptr, local_tb.read_ptr);

    while(1)
    {
        /* How much free space is available? */
        int32_t partial = local_tb.read_ptr - local_tb.write_ptr - 1;
        if(partial < 0)
            partial += tb->buffer_size;

        /* No more space */
        if(partial == 0)
            break;

        /* Clip the length so we only write one contiguous stretch. The incidental advantage is
           that we can't end up writing outside the buffer even if the write pointer goes wild. */
        if(local_tb.write_ptr + partial > tb_end)
            partial = tb_end - local_tb.write_ptr;

        /* Clip the length according to the buffer size */
        if(partial > (int32_t) size)
            partial = size;

        /* Because of clipping, nothing else to do? */
        if(partial == 0)
            break;

        /* Do the write */
        DSPLOG_DEBUG("TB: TB_shared %s, now writing %" PRId32 " bytes to 0x%08" PRIx32,
                     tb->display_id, partial, local_tb.write_ptr);
        {
            ADDR write_ptr_addr;
            ADDR_SIZE partial_size;
            UTIL_addr_assign_from(write_ptr_addr, TB_BUFF_PTR, local_tb.write_ptr);
            UTIL_addr_size_assign_from(partial_size, uint32_t, (uint32_t) partial);
            write_ptr_addr.addr_space = tb->buffer_addr_space;
            DSP_writeData(tb->dsp, write_ptr_addr, src_cursor, partial_size);
        }

        /* Update write ptr */
        local_tb.write_ptr += partial;
        if(local_tb.write_ptr == tb_end)
            local_tb.write_ptr = local_tb.storage;

        /* Update local pointers */
        written_amount += partial;
        size -= partial;
        src_cursor += partial;
    }

    if(written_amount > 0)
    {
        /* Update remote structure */
        ADDR_SIZE tb_write_ptr_size;
        ADDR tb_write_ptr_addr = tb->tb_addr;
#define ADDR_OP_ADD_OFFSET(addr, type) \
            addr += offsetof(TB_shared, write_ptr);
        UTIL_addr_op(tb_write_ptr_addr, ADDR_OP_ADD_OFFSET);
        UTIL_addr_size_assign_from(tb_write_ptr_size, size_t, sizeof(local_tb.write_ptr));
#undef ADDR_OP_ADD_OFFSET

        local_tb.write_ptr = ENDIANESS_htofp32(local_tb.write_ptr);
        DSP_writeData(tb->dsp,
                      tb_write_ptr_addr,
                      &local_tb.write_ptr,
                      tb_write_ptr_size);

        DSPLOG_DEBUG("TB: TB_shared %s, new write ptr=0x%08" PRIx32 ", written 0x%" PRIx32 " bytes",
                     tb->display_id, local_tb.write_ptr, written_amount);
    }

    return written_amount;
}

#endif /* !B_REFSW_MINIMAL */


/* ------------------
 *  Public interface
 * ------------------*/
void TB_init(TB *ret_value,
             DSP *dsp,
             ADDR tb_addr,
             TB_BUFF_PTR buff_addr,
             ADDR_SPACE buff_addr_space,
             uint16_t buff_len_KiB,
             const char *name)
{
    /* Initialise the shared structure */
    TB_shared localTB;
    localTB.dummy1 = 0;
    localTB.dummy2 = 0;
    localTB.storage_kbytes = buff_len_KiB;
    localTB.storage = buff_addr;
    localTB.write_ptr = buff_addr;
    localTB.read_ptr = buff_addr;
    TB_storeShared(dsp, tb_addr, &localTB);

    /* Initialise the control structure */
    ret_value->dsp = dsp;
    ret_value->tb_addr = tb_addr;
    ret_value->buffer_addr_space = buff_addr_space;
    ret_value->buffer_size = buff_len_KiB * 1024;
    ret_value->display_id = name == TB_NO_NAME ? TB_NO_NAME_STRING : (char *) name;

    if(DSPLOG_getLevel() >= DSPLOG_DEBUG_LEVEL)
    {
        char tb_addr_str[UTIL_ADDR_2_HEX_MIN_SIZE];
        UTIL_addr_2_hex_func(ret_value->tb_addr, tb_addr_str);

        DSPLOG_DEBUG("TB: Initialised TB_shared %s at 0x%s, storage=0x%08" PRIx32 ", len=%d KiB",
                     ret_value->display_id, tb_addr_str, localTB.storage, localTB.storage_kbytes);
        DSPLOG_DEBUG("TB: Attached to TB_shared %s at 0x%s",
                     ret_value->display_id, tb_addr_str);
    }
}


#if !B_REFSW_MINIMAL

void TB_attach(TB *ret_value,
               DSP *dsp,
               ADDR tb_addr,
               ADDR_SPACE buff_addr_space,
               const char *name)
{
    TB_shared local_tb;
    TB_retrieveShared(dsp, tb_addr, &local_tb);

    /* Initialise the control structure */
    ret_value->dsp = dsp;
    ret_value->tb_addr = tb_addr;
    ret_value->buffer_addr_space = buff_addr_space;
    ret_value->buffer_size = local_tb.storage_kbytes * 1024;
    ret_value->display_id = name == TB_NO_NAME ? TB_NO_NAME_STRING : (char *) name;

    if(DSPLOG_getLevel() >= DSPLOG_DEBUG_LEVEL)
    {
        char tb_addr_str[UTIL_ADDR_2_HEX_MIN_SIZE];
        UTIL_addr_2_hex_func(ret_value->tb_addr, tb_addr_str);
        DSPLOG_DEBUG("TB: Attached to TB_shared %s at 0x%s", ret_value->display_id, tb_addr_str);
    }
}


void TB_close(TB *tb __unused)
{
    /* currently no-op */
}


void TB_bufferAddress(TB *tb, TB_BUFF_PTR *buff_addr, ADDR_SPACE *buff_addr_space)
{
    if(buff_addr != NULL)
    {
        TB_shared local_tb;
        TB_retrieveShared(tb->dsp, tb->tb_addr, &local_tb);
        *buff_addr = local_tb.storage;
    }

    if(buff_addr_space != NULL)
        *buff_addr_space = tb->buffer_addr_space;
}


TB_BUFF_SIZE TB_bufferSize(TB *tb)
{
    return tb->buffer_size;
}


TB_BUFF_SIZE TB_availableData(TB *tb)
{
    TB_shared local_tb;
    TB_BUFF_SIZE available_data;

    TB_retrieveShared(tb->dsp, tb->tb_addr, &local_tb);
    if(local_tb.write_ptr >= local_tb.read_ptr)
        available_data = local_tb.write_ptr - local_tb.read_ptr;
    else
        available_data = local_tb.write_ptr - local_tb.read_ptr + tb->buffer_size;

    if(available_data > tb->buffer_size)
        DSPLOG_ERROR("TB: TB_shared %s, available data > storage size, TB_shared is likely to be corrupted",
                     tb->display_id);

    return available_data;
}

#endif /* !B_REFSW_MINIMAL */


TB_BUFF_SIZE TB_discard(TB *tb, TB_BUFF_SIZE amount)
{
    TB_shared local_tb;
    TB_BUFF_SIZE available_data;

    TB_retrieveShared(tb->dsp, tb->tb_addr, &local_tb);
    if(local_tb.write_ptr >= local_tb.read_ptr)
        available_data = local_tb.write_ptr - local_tb.read_ptr;
    else
        available_data = local_tb.write_ptr - local_tb.read_ptr + tb->buffer_size;

    /* Clip the discarded amount to the available data */
    if(amount > available_data)
        amount = available_data;

    /* Update local read ptr */
    local_tb.read_ptr += amount;
    if(local_tb.read_ptr >= local_tb.storage + tb->buffer_size)
        local_tb.read_ptr -= tb->buffer_size;

    /* Update remote structure */
    {
        ADDR_SIZE tb_read_ptr_size;
        ADDR tb_read_ptr_addr = tb->tb_addr;
#define ADDR_OP_ADD_OFFSET(addr, type) \
            addr += offsetof(TB_shared, read_ptr);
        UTIL_addr_op(tb_read_ptr_addr, ADDR_OP_ADD_OFFSET);
        UTIL_addr_size_assign_from(tb_read_ptr_size, size_t, sizeof(local_tb.read_ptr));
#undef ADDR_OP_ADD_OFFSET

        local_tb.read_ptr = ENDIANESS_htofp32(local_tb.read_ptr);
        DSP_writeData(tb->dsp,
                      tb_read_ptr_addr,
                      &local_tb.read_ptr,
                      tb_read_ptr_size);
    }

    return amount;
}


#if !B_REFSW_MINIMAL

TB_BUFF_SIZE TB_readCircular(TB *tb, void *buff, TB_BUFF_SIZE size, bool discard_after_read)
{
    TB_shared local_tb;
    TB_BUFF_PTR tb_end;
    uint8_t *buffer_cursor;
    TB_BUFF_SIZE read_amount;

    TB_retrieveShared(tb->dsp, tb->tb_addr, &local_tb);
    tb_end = local_tb.storage + tb->buffer_size;
    buffer_cursor = buff;
    read_amount = 0;

    DSPLOG_JUNK("TB: TB_shared %s, storage=0x%08" PRIx32 ", write ptr=0x%08" PRIx32 ", read ptr=0x%08" PRIx32,
                tb->display_id, local_tb.storage, local_tb.write_ptr, local_tb.read_ptr);

    while(1)
    {
        /* How much data is available */
        int32_t partial = local_tb.write_ptr - local_tb.read_ptr;
        if(partial < 0)
            partial += tb->buffer_size;

        /* Nothing to do */
        if(partial == 0)
            break;

        /* Clip the length so we only read one contiguous stretch. The incidental advantage is
           that we can't end up reading outside the buffer even if the write pointer goes wild. */
        if(local_tb.read_ptr + partial > tb_end)
            partial = tb_end - local_tb.read_ptr;

        /* Clip the length according to the buffer size */
        if(partial > (int32_t) size)
            partial = size;

        /* Because of clipping, nothing else to do */
        if(partial == 0)
            break;

        /* Do the read */
        DSPLOG_DEBUG("TB: TB_shared %s, now reading %" PRId32 " bytes from 0x%08" PRIx32,
                     tb->display_id, partial, local_tb.read_ptr);
        {
            ADDR read_ptr_addr;
            ADDR_SIZE partial_size;
            UTIL_addr_assign_from(read_ptr_addr, TB_BUFF_PTR, local_tb.read_ptr);
            UTIL_addr_size_assign_from(partial_size, uint32_t, (uint32_t) partial);
            read_ptr_addr.addr_space = tb->buffer_addr_space;
            DSP_readData(tb->dsp, buffer_cursor, read_ptr_addr, partial_size);
        }

        /* Update read ptr */
        local_tb.read_ptr += partial;
        if(local_tb.read_ptr == tb_end)
            local_tb.read_ptr = local_tb.storage;

        /* Update local pointers */
        read_amount += partial;
        size -= partial;
        buffer_cursor += partial;
    }

    if(read_amount > 0 && discard_after_read)
    {
        /* Update remote structure */
        ADDR_SIZE tb_read_ptr_size;
        ADDR tb_read_ptr_addr = tb->tb_addr;
#define ADDR_OP_ADD_OFFSET(addr, type) \
            addr += offsetof(TB_shared, read_ptr);
        UTIL_addr_op(tb_read_ptr_addr, ADDR_OP_ADD_OFFSET);
        UTIL_addr_size_assign_from(tb_read_ptr_size, size_t, sizeof(local_tb.read_ptr));
#undef ADDR_OP_ADD_OFFSET

        local_tb.read_ptr = ENDIANESS_htofp32(local_tb.read_ptr);
        DSP_writeData(tb->dsp,
                      tb_read_ptr_addr,
                      &local_tb.read_ptr,
                      tb_read_ptr_size);

        DSPLOG_DEBUG("TB: TB_shared %s, new read ptr=0x%08" PRIx32 ", read 0x%" PRIx32 " bytes",
                     tb->display_id, local_tb.read_ptr, read_amount);
    }

    return read_amount;
}

#endif /* !B_REFSW_MINIMAL */


#if FEATURE_IS(FILE_IO, AVAILABLE) && !(IS_TARGET(RaagaFP4015_si_magnum_permissive))

ssize_t TB_dumpToFile(TB *tb, int fd)
{
    /* is there any data to read? */
    int size = TB_availableData(tb);
    if(size == 0)
        return 0;

    /* allocate buffer and read data */
    void *buffer;
    MALLOC_OR_FAIL(buffer, void *, size, "TB: ");
    ssize_t read_count = TB_readCircular(tb, buffer, size, false);

    /* dump to file */
    ssize_t write_ret = OS_write(fd, buffer, read_count);
    free(buffer);

    if(write_ret > 0)
    {
        if(write_ret == read_count)
            DSPLOG_DEBUG("TB: TB_shared %s, dumped %zd bytes to fd %d",
                         tb->display_id, write_ret, fd);
        else
            DSPLOG_ERROR("TB: TB_shared %s, dumped %zd bytes to fd %d (but %zd were read from the TB buffer)",
                         tb->display_id, write_ret, fd, read_count);

        TB_discard(tb, write_ret);
    }
    else if(write_ret < 0)
        DSPLOG_ERROR("TB: TB_shared %s, error while dumping %zd bytes to fd %d, OS_write returned %zd",
                     tb->display_id, read_count, fd, write_ret);

    return write_ret;
}

#endif /* FEATURE_IS(FILE_IO, AVAILABLE) && !(IS_TARGET(RaagaFP4015_si_magnum_permissive))*/


TB_BUFF_SIZE TB_peek(TB *tb, TB_data_descriptor *data_descriptor)
{
    TB_shared local_tb;
    TB_BUFF_SIZE wrap_around_available_data;

    TB_retrieveShared(tb->dsp, tb->tb_addr, &local_tb);

#if FEATURE_IS(TB_VARIANT, CIRCULAR)
    data_descriptor->dsp = tb->dsp;
#endif

    /* deduce buffers layout */
    data_descriptor->buffers_address_space = tb->buffer_addr_space;
    if(local_tb.write_ptr >= local_tb.read_ptr)
    {
        data_descriptor->total_size = local_tb.write_ptr - local_tb.read_ptr;
        wrap_around_available_data = 0;
    }
    else
    {
        data_descriptor->total_size = local_tb.write_ptr - local_tb.read_ptr + tb->buffer_size;
        wrap_around_available_data = local_tb.write_ptr - local_tb.storage;
    }

    if(wrap_around_available_data == 0)
    {
        data_descriptor->buffers_count = 1;
        data_descriptor->buffers[0].data = local_tb.read_ptr;
        data_descriptor->buffers[0].size = data_descriptor->total_size;
    }
    else
    {
        data_descriptor->buffers_count = 2;
        data_descriptor->buffers[0].data = local_tb.read_ptr;
        data_descriptor->buffers[0].size = data_descriptor->total_size - wrap_around_available_data;
        data_descriptor->buffers[1].data = local_tb.storage;
        data_descriptor->buffers[1].size = wrap_around_available_data;
    }

    /* move the read cursor to the begin */
    TB_SEEK_TO_BEGIN(data_descriptor);

    return data_descriptor->total_size;
}


#if TARGET_BUFFER_VERSION == 2

#if FEATURE_IS(FILE_IO, AVAILABLE) && !(IS_TARGET(RaagaFP4015_si_magnum_permissive))

/* TODO: this is just a dirty quick hack, see more in TB_internal.h */
ssize_t TB_copyFramesPayloadToFile(TB *tb, TB_service_flag services, int fd, bool discard_up_to_read_data)
{
    /* extract info about available data */
    TB_data_descriptor data_descriptor;
    TB_BUFF_SIZE total_data_size = TB_peek(tb, &data_descriptor);
    if(total_data_size == 0)
        return 0;

    /* extract info about available frames / payload for the specific service */
    unsigned available_frames;
    TB_BUFF_SIZE available_payload;
    TB_BUFF_SIZE max_payload_size;
    TB_services_filter_config filter_config =
    {
        .accepted_services = services,
        .accept_overwritten_frames = true
    };
    TB_countFrames(&data_descriptor,
                   UINT_MAX,
                   &available_frames,
                   &available_payload,
                   &max_payload_size,
                   TB_services_filter,
                   &filter_config);
    if(available_payload == 0)
        return 0;

    /* allocate required resources */
    TB_frame_info frames_info[available_frames];
    void *payload_buffer = NULL;
    MALLOC_OR_FAIL(payload_buffer, void *, available_payload, "TB: ");

    /* read frames payload (we read even if available_payload == 0 to evacuate empty frames) */
    unsigned read_frames;
    TB_BUFF_SIZE read_payload;
    TB_BUFF_SIZE next_read_cursor;
    next_read_cursor = TB_readAllFrames(&data_descriptor,
                                        frames_info,
                                        available_frames,
                                        &read_frames,
                                        payload_buffer,
                                        available_payload,
                                        &read_payload,
                                        TB_services_filter,
                                        &filter_config);

    /* check for consistency */
    if(read_payload != available_payload)
        DSPLOG_ERROR("TB: TB_shared %s, error while reading frames, "
                     "TB_countFrames returned %" PRIu32 " bytes were available while "
                     "TB_readFrames returned %" PRIu32 " bytes worth of payload",
                     tb->display_id, available_payload, read_payload);
    if(read_frames != available_frames)
        DSPLOG_ERROR("TB: TB_shared %s, error while reading frames, "
                     "TB_countFrames returned %" PRIu32 " frames available while "
                     "TB_readFrames read %" PRIu32 " frames",
                     tb->display_id, available_frames, read_frames);
    /* now we can return if no actual data to write */
    if(read_payload == 0)
        return 0;

    /* write data to file */
    ssize_t write_ret = OS_write(fd, payload_buffer, read_payload);
    free(payload_buffer);

    /* choose a proper message */
    if(write_ret >= 0)
    {
        if((TB_BUFF_SIZE) write_ret == read_payload)
            DSPLOG_DEBUG("TB: TB_shared %s, dumped %zd bytes of frames payload to fd %d",
                         tb->display_id, write_ret, fd);
        else
            DSPLOG_ERROR("TB: TB_shared %s, dumped %zd bytes of frames payload to fd %d "
                         "(but 0x%" PRIx32 " were read from the TB buffer)",
                         tb->display_id, write_ret, fd, read_payload);
    }
    else if(write_ret < 0)
        DSPLOG_ERROR("TB: TB_shared %s, error while dumping 0x%" PRIx32 " bytes of frames payload to "
                     "fd %d, OS_write returned %zd", tb->display_id, read_payload, fd, write_ret);

    /* free TB space */
    if(discard_up_to_read_data)
        TB_discard(tb, next_read_cursor);

    return write_ret;
}

#endif /* FEATURE_IS(FILE_IO, AVAILABLE) && !(IS_TARGET(RaagaFP4015_si_magnum_permissive))*/

#endif /* TARGET_BUFFER_VERSION == 2 */
