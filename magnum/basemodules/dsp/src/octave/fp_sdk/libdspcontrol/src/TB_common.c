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

#if !FEATURE_IS(SW_HOST, RAAGA_MAGNUM)
#  include <inttypes.h>
#  include <limits.h>
#  include <stdbool.h>
#  include <string.h>
#else
#  include "bstd_defs.h"
#  include "DSP/DSP_raaga_inttypes.h"
#endif

#include "libdspcontrol/COMMON.h"
#include "libdspcontrol/DSPLOG.h"
#include "libdspcontrol/TB.h"
#include "libdspcontrol/UTIL_c.h"

#include "libsyschip/tbuf_chips.h"

#include "libfp/src/c_utils_internal.h"

#include "ENDIANESS.h"
#include "TB_internal.h"

#if !IS_HOST(DSP_LESS)
#  include "libdspcontrol/DSP.h"
#  include "UTIL_addr.h"
#endif



#if !HAS_FEATURE(TB_VARIANT) || FEATURE_IS(TB_VARIANT, NONE)
#  error "This source file is only for not 'NONE' TB variants"
#endif


TB_BUFF_SIZE TB_read(TB_data_descriptor *descriptor, void *dest, TB_BUFF_SIZE size, bool move_cursor)
{
    TB_BUFF_SIZE available_data, remaining_size;
    uint8_t *dest_cursor;
    unsigned cur_buffer;

#if !IS_HOST(DSP_LESS)
#  define cur_read_pointer_t    TB_BUFF_PTR
#else
#  define cur_read_pointer_t    uint8_t *
#endif
    cur_read_pointer_t cur_read_pointer;

    /* cap the read amount to the available data */
    available_data = descriptor->total_size - descriptor->cur_offset;
    if(size > available_data)
        size = available_data;

    dest_cursor = dest;
    remaining_size = size;
    cur_buffer = descriptor->cur_buffer;
    cur_read_pointer = descriptor->cur_pointer;

    while(remaining_size > 0)
    {
        /* how much can we read from this buffer? */
        TB_BUFF_SIZE this_buff_size = descriptor->buffers[cur_buffer].size -
                                      (cur_read_pointer - descriptor->buffers[cur_buffer].data);
        TB_BUFF_SIZE this_read_size = remaining_size;
        if(this_read_size > this_buff_size)
            this_read_size = this_buff_size;

        /* read the data */
#if !IS_HOST(DSP_LESS)
        {
            ADDR cur_read_addr;
            ADDR_SIZE read_size;
            UTIL_addr_assign_from(cur_read_addr, cur_read_pointer_t, cur_read_pointer);
            UTIL_addr_size_assign_from(read_size, TB_BUFF_SIZE, this_read_size);
            cur_read_addr.addr_space = descriptor->buffers_address_space;
            DSP_readData(descriptor->dsp, dest_cursor, cur_read_addr, read_size);
        }
#else
        memcpy(dest_cursor, cur_read_pointer, this_read_size);
#endif

        /* update pointers */
        cur_read_pointer += this_read_size;
        dest_cursor += this_read_size;
        remaining_size -= this_read_size;
        /* if we read all from this buffer and there's more to read
           (to avoid moving beyond the last buffer), move to the next one */
        if(this_read_size == this_buff_size && remaining_size > 0)
        {
            cur_buffer++;
            cur_read_pointer = descriptor->buffers[cur_buffer].data;
        }
    }

    if(move_cursor)
        TB_seek(descriptor, size, TB_SEEK_CUR);

    return size;
}


/*
 * Always move forward. When asked to move backward, start from the beginning.
 */
TB_BUFF_SIZE TB_seek(TB_data_descriptor *descriptor, int offset, TB_seek_whence whence)
{
    unsigned i;
    bool fall_through = false;

    switch(whence)
    {
    case TB_SEEK_BEGIN: ;
        if(offset <= 0)
            TB_SEEK_TO_BEGIN(descriptor)
        else if((unsigned) offset >= descriptor->total_size)
            TB_SEEK_TO_END(descriptor)
        else if((unsigned) offset < descriptor->cur_offset)
        {
            /* save the final value */
            descriptor->cur_offset = offset;
            /* walk buffers */
            for(i = 0; i < descriptor->buffers_count; i++)
            {
                if((unsigned) offset < descriptor->buffers[i].size)
                {
                    /* the read cursor will be inside this buffer */
                    descriptor->cur_buffer = i;
                    descriptor->cur_pointer = descriptor->buffers[i].data + ((TB_BUFF_PTR) offset);
                    break;
                }
                else
                {
                    /* skip this buffer */
                    offset -= descriptor->buffers[i].size;
                }
            }
        }
        else
        {
            /* the the scenario can be better handled by the TB_SEEK_CUR case */
            offset -= descriptor->cur_offset;
            fall_through = true;
        }

        if(!fall_through)
            break;
    __fallthrough;
        /* no break, no KitKat (so Eclipse's happy) */

    case TB_SEEK_CUR:
        if(offset == 0)
            break;
        else if(offset < 0)
            TB_seek(descriptor, (int) descriptor->cur_offset + offset, TB_SEEK_BEGIN);
        else if(descriptor->cur_offset + (unsigned) offset >= descriptor->total_size)
            TB_SEEK_TO_END(descriptor)
        else
        {
            /* save the final value */
            descriptor->cur_offset += (size_t) offset;
            /* let's pretend we are starting from the beginning of the buffer, adjust the requested offset */
            offset += descriptor->cur_pointer - descriptor->buffers[descriptor->cur_buffer].data;
            for(i = descriptor->cur_buffer; i < descriptor->buffers_count; i++)
            {
                if((unsigned) offset < descriptor->buffers[i].size)
                {
                    /* the read cursor will be inside this buffer */
                    descriptor->cur_buffer = i;
                    descriptor->cur_pointer = descriptor->buffers[i].data + ((TB_BUFF_PTR) offset);
                    break;
                }
                else
                {
                    /* skip this buffer */
                    offset -= descriptor->buffers[i].size;
                }
            }
        }
        break;

    case TB_SEEK_END:
        if(offset >= 0)
            TB_SEEK_TO_END(descriptor)
        else if(offset <= - ((int) descriptor->total_size))
            TB_SEEK_TO_BEGIN(descriptor)
        else
            TB_seek(descriptor, (int) descriptor->total_size + offset, TB_SEEK_BEGIN);
        break;
    }

    return descriptor->cur_offset;
}


#if !IS_HOST(DSP_LESS) && FEATURE_IS(DSP_SYS_ADDR_DIRECT_ACCESS, SUPPORTED)

TB_BUFF_SIZE TB_map(TB_data_descriptor *descriptor, MAPPED_ADDR *addr, TB_BUFF_SIZE size, bool move_cursor)
{
    /* mapping is only supported in system address space */
    if(descriptor->buffers_address_space != ADDR_SPACE_SYSTEM)
        return 0;

    {
        DSP_RET ret;

        /* map as much as possible from the current buffer (that's the most we
         * can assure to be contiguous) */
        TB_data_region *cur_buffer = &descriptor->buffers[descriptor->cur_buffer];
        TB_BUFF_SIZE size_cap = cur_buffer->size - (descriptor->cur_pointer - cur_buffer->data);
        TB_BUFF_SIZE length_to_map = UNSAFE_MIN(size_cap, size);
        if(length_to_map == 0)
            return 0;

        ret = DSP_mapSystemAddress(descriptor->dsp,
                                   SYSTEM_ADDR_CAST(descriptor->cur_pointer),
                                   (SYSTEM_ADDR_SIZE) length_to_map, addr);
        if(DSP_FAILED(ret))
            return 0;

        if(move_cursor)
            TB_seek(descriptor, length_to_map, TB_SEEK_CUR);

        return length_to_map;
    }
}


unsigned TB_mapAll(TB_data_descriptor *descriptor, MAPPED_ADDR *addr_array, unsigned addr_count)
{
    const unsigned required_addr_count =  descriptor->buffers_count;

    /* we map everything or nothing */
    if(addr_count < required_addr_count)
        return 0;

    /* mapping is only supported in system address space */
    if(descriptor->buffers_address_space != ADDR_SPACE_SYSTEM)
        return 0;

    /* a NULL addr_array is just a query on the number of required entries in addr_array */
    if(addr_array == NULL)
        return required_addr_count;

    /* map all buffers in sequence; in case of errors, rollback */
    {
        unsigned mapped_entries;
        for(mapped_entries = 0; mapped_entries < descriptor->buffers_count; mapped_entries++)
        {
            TB_data_region *cur_buffer = &descriptor->buffers[mapped_entries];
            DSP_RET ret = DSP_mapSystemAddress(descriptor->dsp,
                                               SYSTEM_ADDR_CAST(cur_buffer->data),
                                               (SYSTEM_ADDR_SIZE) cur_buffer->size,
                                               &addr_array[mapped_entries]);

            if(DSP_SUCCEEDED(ret))
                mapped_entries++;
            else
            {
                while(mapped_entries > 0)
                {
                    mapped_entries--;
                    DSP_unmapSystemAddress(descriptor->dsp,
                                           &addr_array[mapped_entries]);
                }
                break;
            }
        }

        return mapped_entries;
    }
}

#endif /* !IS_HOST(DSP_LESS) && FEATURE_IS(DSP_SYS_ADDR_DIRECT_ACCESS, SUPPORTED) */


#if TARGET_BUFFER_VERSION == 2

/**
 * Reads a TB_header from descriptor (moving the cursor forward) and takes
 * care of required endianness swaps.
 *
 * @param[in]  descriptor   descriptor where to read the header from
 * @param[out] frame_header where to store the header data
 */
BFPSDK_STATIC_ALWAYS_INLINE
void TB_readFrameHeader(TB_data_descriptor *descriptor, TB_header *frame_header)
{
    /* read data, it will be in little endian (FP) format */
    TB_read(descriptor, frame_header, sizeof(TB_header), true);

    /* assumptions that drive our endianness swap */
    COMPILE_TIME_ASSERT(sizeof(frame_header->prologue)            == 1);
    COMPILE_TIME_ASSERT(sizeof(frame_header->pre_padding_length)  == 1);
    COMPILE_TIME_ASSERT(sizeof(frame_header->payload_length)      == 4);
    COMPILE_TIME_ASSERT(sizeof(frame_header->post_padding_length) == 1);

    /* only payload_length needs swapping */
    frame_header->payload_length = ENDIANESS_fptoh32(frame_header->payload_length);
}


void TB_readFrameInfo(TB_data_descriptor *descriptor,
                      TB_header *frame_header,
                      TB_frame_info *ret_value,
                      TB_data_descriptor *descriptor_clone_at_payload,
                      bool move_cursor)
{
    const size_t frame_size =
            sizeof(TB_header)                 +
            frame_header->pre_padding_length  +
            frame_header->payload_length      +
            frame_header->post_padding_length;

    /* where we are at any moment respect to the frame start,
     * by contract we start from after the header */
    TB_BUFF_SIZE frame_relative_off = sizeof(TB_header);
    TB_BUFF_SIZE saved_descriptor_offset = descriptor->cur_offset;

    /* extract info from the header */
    uint8_t frame_flags = TB_PROLOGUE_GET_FLAGS(frame_header->prologue);
    ret_value->tb_id = TB_unzip_id(TB_PROLOGUE_GET_ID(frame_header->prologue));
    ret_value->discardable = frame_flags & TB_HEADER_FLAG_DISCARDABLE;
    ret_value->payload_offset = descriptor->cur_offset + frame_header->pre_padding_length;
    ret_value->payload_address = NULL;
    ret_value->payload_length = frame_header->payload_length;
#if !IS_HOST(DSP_LESS)
    ret_value->indirect = frame_flags & TB_HEADER_FLAG_INDIRECT;
    ret_value->indirect_payload.address = (uint32_t)(uintptr_t) NULL;
    ret_value->indirect_payload.length = 0;
#endif

    /* do we need to clone the descriptor at payload position? */
    if(descriptor_clone_at_payload != NULL)
    {
        if(frame_header->pre_padding_length != 0)
        {
            TB_seek(descriptor, frame_header->pre_padding_length, TB_SEEK_CUR);
            frame_relative_off += frame_header->pre_padding_length;
        }
        *descriptor_clone_at_payload = *descriptor;
    }

#if !IS_HOST(DSP_LESS)
    /* if this is an indirect frame, read the TB_indirect_frame_payload content */
    if(ret_value->indirect)
    {
        /* we could be or not at the beginning of the payload */
        TB_BUFF_SIZE payload_position_offset = sizeof(TB_header) +
                                               frame_header->pre_padding_length -
                                               frame_relative_off;
        if(payload_position_offset > 0)
            TB_seek(descriptor,
                    payload_position_offset,
                    TB_SEEK_CUR);
        TB_read(descriptor, &ret_value->indirect_payload, sizeof(TB_indirect_frame_payload), true);
        ret_value->indirect_payload.address = ENDIANESS_fptoh32(ret_value->indirect_payload.address);
        ret_value->indirect_payload.length = ENDIANESS_fptoh32(ret_value->indirect_payload.length);
        frame_relative_off += payload_position_offset + sizeof(TB_indirect_frame_payload);
    }
#endif

    /* try to extract the trailer */
    if(frame_header->post_padding_length > 0)
    {
#ifdef TB_FRAMES_HAVE_TRAILER
        if(frame_flags & TB_HEADER_FLAG_HAS_TRAILER)
        {
            /* move to the trailer location */
            TB_BUFF_SIZE trailer_position_offset = frame_size -
                                                   sizeof(TB_trailer) -
                                                   frame_relative_off;
            if(trailer_position_offset > 0)
                TB_seek(descriptor,
                        trailer_position_offset,
                        TB_SEEK_CUR);

            /* read trailer */
            TB_trailer frame_trailer;
            COMPILE_TIME_ASSERT(sizeof(TB_trailer) == 1);   /* otherwise we should deal with endianness */
            TB_read(descriptor, &frame_trailer, sizeof(TB_trailer), true);
            ret_value->has_trailer_info = true;
            ret_value->trailer_info.overwritten = frame_trailer & TB_TRAILER_OVERWRITTEN_BIT;
            ret_value->trailer_info.chunks_count = frame_trailer & TB_TRAILER_CHUNKS_COUNT_MASK;

            frame_relative_off += trailer_position_offset + sizeof(TB_trailer);
        }
        else
#endif
        {
            /* no trailer */
            ret_value->has_trailer_info = false;
        }
    }
    else
        ret_value->has_trailer_info = false;

    /* fulfil the move_cursor request */
    if(!move_cursor && frame_relative_off != sizeof(TB_header))
        TB_seek(descriptor, saved_descriptor_offset, TB_SEEK_BEGIN);
    if(move_cursor)
    {
        TB_BUFF_SIZE final_position_offset = frame_size - frame_relative_off;
        if(final_position_offset > 0)
            TB_seek(descriptor,
                    final_position_offset,
                    TB_SEEK_CUR);
    }
}


bool TB_services_filter(TB_frame_info *frame_info, void *data)
{
    TB_services_filter_config *config = (TB_services_filter_config *) data;
    if(frame_info->discardable && !config->accept_overwritten_frames)
        return false;

    if(TB_SERVICE_FLAG_FROM_ID(frame_info->tb_id.service_id) & config->accepted_services)
        return true;
    else
        return false;
}


TB_BUFF_SIZE TB_countFrames(TB_data_descriptor *descriptor,
                            unsigned max_frames,
                            unsigned *frames_count,
                            TB_BUFF_SIZE *available_payload,
                            TB_BUFF_SIZE *max_payload_size,
                            TB_frame_filter filter,
                            void *filter_data)
{
    unsigned total_frames;
    TB_BUFF_SIZE total_payload_size;
    TB_BUFF_SIZE biggest_payload_size;
    TB_BUFF_SIZE remaining_data;
    TB_BUFF_SIZE cursor_after_last_frame;

    /* save current buffer position */
    TB_BUFF_SIZE saved_descriptor_offset = descriptor->cur_offset;

    /* reset values */
    *frames_count = 0;
    *available_payload = 0;
    *max_payload_size = 0;
    /* nothing to do? */
    if(max_frames == 0)
        return descriptor->cur_offset;

    /* walk the frames */
    total_frames = 0;
    total_payload_size = 0;
    biggest_payload_size = 0;
    remaining_data = descriptor->total_size - descriptor->cur_offset;
    cursor_after_last_frame = descriptor->cur_offset;
    while(remaining_data >= sizeof(TB_header) &&
          total_frames < max_frames)
    {
        TB_frame_info frame_info;
        TB_BUFF_SIZE frame_size;

        /* read the header */
        TB_header frame_header;
        TB_readFrameHeader(descriptor, &frame_header);
        DSPLOG_JUNK("TB_countFrames read header prologue=%#x, post_padding_length=%" PRIu8 ","
                    " payload_length=%" PRIu32 ", pre_padding_length=%" PRIu8,
                    frame_header.prologue, frame_header.pre_padding_length,
                    frame_header.payload_length, frame_header.post_padding_length);

        /* is this frame complete? */
        frame_size = sizeof(TB_header)               +
                     frame_header.payload_length     +
                     frame_header.pre_padding_length +
                     frame_header.post_padding_length;
        if(remaining_data < frame_size)
            break;      /* incomplete frame, exit here */

        /* extract the frame info to be able to take some decisions */
        TB_readFrameInfo(descriptor, &frame_header, &frame_info, NULL, true);
        /* update the "byte after the last considered frame" position */
        cursor_after_last_frame = descriptor->cur_offset;
        /* update the remaining available data */
        remaining_data -= frame_size;

        if(filter == NULL || filter(&frame_info, filter_data))
        {
            /* update counters */
#if !IS_HOST(DSP_LESS)
            TB_BUFF_SIZE actual_payload_size = frame_info.indirect ?
                    frame_info.indirect_payload.length : frame_header.payload_length;
#else
            TB_BUFF_SIZE actual_payload_size = frame_header.payload_length;
#endif

            total_frames++;
            total_payload_size += actual_payload_size;
            if(actual_payload_size > biggest_payload_size)
                biggest_payload_size = actual_payload_size;
        }
    }

    /* restore read cursor position */
    TB_seek(descriptor, saved_descriptor_offset, TB_SEEK_BEGIN);

    /* update out parameters */
    *frames_count = total_frames;
    *available_payload = total_payload_size;
    *max_payload_size = biggest_payload_size;

    return cursor_after_last_frame;
}


TB_BUFF_SIZE TB_peekFrames(TB_data_descriptor *descriptor,
                           TB_frame_info *frame_info,
                           unsigned num_frame_info,
                           unsigned *frames_count,
                           TB_BUFF_SIZE *available_payload,
                           TB_frame_filter filter,
                           void *filter_data)
{
    unsigned total_frames;
    TB_BUFF_SIZE total_payload_size;
    TB_BUFF_SIZE remaining_data;
    TB_BUFF_SIZE cursor_after_last_frame;

    /* save current buffer position */
    size_t saved_descriptor_offset = descriptor->cur_offset;

    /* reset values */
    *frames_count = 0;
    *available_payload = 0;
    /* nothing to do? */
    if(num_frame_info == 0)
        return descriptor->cur_offset;

    /* walk the frames */
    total_frames = 0;
    total_payload_size = 0;
    remaining_data = descriptor->total_size - descriptor->cur_offset;
    cursor_after_last_frame = descriptor->cur_offset;
    while(remaining_data >= sizeof(TB_header) &&
          total_frames < num_frame_info)
    {
        TB_frame_info *cur_frame_info;
        TB_BUFF_SIZE frame_size;

        /* read the header */
        TB_header frame_header;
        TB_readFrameHeader(descriptor, &frame_header);

        /* is this frame complete? */
        frame_size = sizeof(TB_header)               +
                     frame_header.payload_length     +
                     frame_header.pre_padding_length +
                     frame_header.post_padding_length;
        if(remaining_data < frame_size)
            break;      /* incomplete frame, exit here */

        /* extract frame info */
        cur_frame_info = &frame_info[total_frames];
        TB_readFrameInfo(descriptor, &frame_header, cur_frame_info, NULL, true);

        if(filter == NULL || filter(cur_frame_info, filter_data))
        {
            /* frame accepted */
            total_frames++;
#if !IS_HOST(DSP_LESS)
            total_payload_size += cur_frame_info->indirect ?
                    cur_frame_info->indirect_payload.length : frame_header.payload_length;
#else
            total_payload_size += frame_header.payload_length;
#endif
        }

        /* track where we read up to */
        cursor_after_last_frame = descriptor->cur_offset;
        /* update the remaining available data */
        remaining_data -= frame_size;
    }

    /* restore read cursor position */
    TB_seek(descriptor, saved_descriptor_offset, TB_SEEK_BEGIN);

    /* update out parameters */
    *frames_count = total_frames;
    *available_payload = total_payload_size;

    return cursor_after_last_frame;
}


TB_BUFF_SIZE TB_readAllFrames(TB_data_descriptor *descriptor,
                              TB_frame_info *frame_info,
                              unsigned num_frame_info,
                              unsigned *frames_count,
                              void *buf,
                              TB_BUFF_SIZE buf_size,
                              TB_BUFF_SIZE *available_payload,
                              TB_frame_filter filter,
                              void *filter_data)
{
    unsigned total_frames;
    TB_BUFF_SIZE total_payload_size;
    uint8_t *write_cursor;
    TB_BUFF_SIZE write_remaining_data;
    TB_BUFF_SIZE remaining_data;
    TB_BUFF_SIZE cursor_after_last_frame;

    /* save current buffer position */
    TB_BUFF_SIZE saved_descriptor_offset = descriptor->cur_offset;

    /* reset values */
    *frames_count = 0;
    *available_payload = 0;
    /* nothing to do? */
    if(num_frame_info == 0)
        return descriptor->cur_offset;

    /* walk the frames */
    total_frames = 0;
    total_payload_size = 0;
    write_cursor = buf;
    write_remaining_data = buf_size;
    remaining_data = descriptor->total_size - descriptor->cur_offset;
    cursor_after_last_frame = descriptor->cur_offset;
    while(remaining_data >= sizeof(TB_header) &&
          total_frames < num_frame_info)
    {
        TB_data_descriptor descriptor_at_payload;
        TB_frame_info *cur_frame_info;
        TB_BUFF_SIZE frame_size, actual_payload_length;
        bool frame_accepted;

        /* read the header */
        TB_header frame_header;
        TB_readFrameHeader(descriptor, &frame_header);

        /* is this frame complete? */
        frame_size = sizeof(TB_header)               +
                     frame_header.payload_length     +
                     frame_header.pre_padding_length +
                     frame_header.post_padding_length;
        if(remaining_data < frame_size)
            break;      /* incomplete frame, exit here */

        /* extract frame info */
        cur_frame_info = &frame_info[total_frames];
        TB_readFrameInfo(descriptor,
                         &frame_header,
                         cur_frame_info,
                         &descriptor_at_payload,
                         true);
#if !IS_HOST(DSP_LESS)
        actual_payload_length = cur_frame_info->indirect ?
                cur_frame_info->indirect_payload.length : frame_header.payload_length;
#else
        actual_payload_length = frame_header.payload_length;
#endif

        /* check the filter response */
        frame_accepted = true;
        if(filter != NULL)
           frame_accepted = filter(cur_frame_info, filter_data);

        /* lack of available space is important only if the frame has been accepted */
        if(frame_accepted && write_remaining_data < actual_payload_length)
            break;  /* we exhausted the write space, quit */

        /* how to proceed? */
        if(!frame_accepted)
        {
            /* frame rejected, update only relevant state */
            remaining_data -= frame_size;
            cursor_after_last_frame = descriptor->cur_offset;
        }
        else
        {
            /* frame accepted, read payload */
#if !IS_HOST(DSP_LESS)
            if(cur_frame_info->indirect)
                DSP_readDataAsDsp(descriptor->dsp, write_cursor,
                                  cur_frame_info->indirect_payload.address,
                                  cur_frame_info->indirect_payload.length);
            else
#endif
                TB_read(&descriptor_at_payload, write_cursor, frame_header.payload_length, false);
            cur_frame_info->payload_address = write_cursor;

            /* update state */
            total_frames++;
            total_payload_size += actual_payload_length;
            write_cursor += actual_payload_length;
            write_remaining_data -= actual_payload_length;
            cursor_after_last_frame = descriptor->cur_offset;
        }
    }

    /* restore read cursor position */
    TB_seek(descriptor, saved_descriptor_offset, TB_SEEK_BEGIN);

    /* update out parameters */
    *frames_count = total_frames;
    *available_payload = total_payload_size;

    return cursor_after_last_frame;
}


TB_BUFF_SIZE TB_readSingleFrames(TB_data_descriptor *descriptor,
                                 TB_frame_reader frames_reader,
                                 void *reader_data)
{
     /* save current buffer position */
     TB_BUFF_SIZE saved_descriptor_offset = descriptor->cur_offset;

     /* walk the frames */
     TB_BUFF_SIZE remaining_data = descriptor->total_size - descriptor->cur_offset;
     TB_BUFF_SIZE cursor_after_last_frame = descriptor->cur_offset;
     while(remaining_data >= sizeof(TB_header))
     {
         TB_frame_info frame_info;
         TB_data_descriptor descriptor_at_payload;
         TB_BUFF_SIZE frame_size;

         /* read the header */
         TB_header frame_header;
         TB_readFrameHeader(descriptor, &frame_header);

         /* is this frame complete? */
         frame_size = sizeof(TB_header)               +
                      frame_header.payload_length     +
                      frame_header.pre_padding_length +
                      frame_header.post_padding_length;
         if(remaining_data < frame_size)
             break;      /* incomplete frame, exit here */

         /* extract frame info */
         TB_readFrameInfo(descriptor,
                          &frame_header,
                          &frame_info,
                          &descriptor_at_payload,
                          true);

         /* invoke the callback to decide whether to continue */
         if(!frames_reader(&frame_info,
                           &descriptor_at_payload,
                           reader_data))
             break;

         /* update only if frames_reader returned true */
         cursor_after_last_frame = descriptor->cur_offset;

         /* update the remaining available data */
         remaining_data -= frame_size;
     }

     /* restore original read read cursor position */
     TB_seek(descriptor, saved_descriptor_offset, TB_SEEK_BEGIN);

     return cursor_after_last_frame;
}

#endif  /* TARGET_BUFFER_VERSION == 2 */
