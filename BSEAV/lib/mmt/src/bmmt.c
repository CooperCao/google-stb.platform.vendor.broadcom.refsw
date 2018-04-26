/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 **************************************************************************/

#include "bmmt.h"
#include "btlv_parser.h"
#include "bmmt_parser.h"
#include "bmmt_demux.h"
#include "blst_queue.h"
#include "bmedia_util.h"
#include "biobits.h"
#include "bioatom.h"
#include "bpool.h"
#include "barena.h"
#include "inttypes.h"

BDBG_MODULE(bmmt);

#define BDBG_MSG_TRACE(x)  BDBG_MSG(x)
#define BDBG_TRACE_ERROR(x) (BDBG_LOG(("Error: %s (%s,%u)",#x,__FILE__,__LINE__)),x)

/**
Summary:
 bmmt defines a context for an instance of mmt library.
**/
BDBG_OBJECT_ID(bmmt);
typedef struct bmmt {
    BDBG_OBJECT(bmmt)
    bmmt_open_settings open_settings;
    FILE *fin;
    FILE *fout;
    NEXUS_RecpumpHandle recpump;
    BKNI_EventHandle rec_event;
    NEXUS_PidChannelHandle allPassPidChannel;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaypumpStatus playpumpStatus;
    bool initial_timestamp_valid;
    batom_factory_t factory;
    btlv_ntp_time initial_timestamp;
    BLST_Q_HEAD(bmmt_streams, bmmt_stream) streams;
    BLST_Q_HEAD(bmmt_msgs, bmmt_msg) msgs;
    bmmt_msg_t tlv_msg;
    bmmt_demux_t demux;
    bmmt_demux_config demux_config;
    bmmt_demux_stream_config demux_stream_config;
    bool started;
    btlv_parser parser;
    btlv_ip_parser ip_parser;
    bmmt_buffer buffer;
    unsigned num_io_buffers;
    unsigned io_buffer_size;
    bmmt_io_data *io_data;
    pthread_t thread;
    bool ip_filter_valid;
    btlv_ip_address addr;
    BKNI_MutexHandle mutex;
}bmmt;

/**
Summary:
 bmmt_stream defines a context for an es stream extracted from
 MMT packets
**/
BDBG_OBJECT_ID(bmmt_stream);
typedef struct bmmt_stream {
    BDBG_OBJECT(bmmt_stream)
    BLST_Q_ENTRY(bmmt_stream) link;
    NEXUS_PidChannelHandle pid_channel;
    bmmt_stream_settings settings;
    bmmt_buffer buffer;
    bmmt_timestamp_queue timestamp_queue;
    bmmt_demux_stream_config demux_stream_config;
    bmmt_demux_stream_t demux_stream;
    bmmt_t mmt;
    unsigned pes_id;
}bmmt_stream;

/**
Summary:
 bmmt_stream defines a context for an es stream extracted from
 MMT packets
**/
BDBG_OBJECT_ID(bmmt_msg);
typedef struct bmmt_msg {
    BDBG_OBJECT(bmmt_msg)
    BLST_Q_ENTRY(bmmt_msg) link;
    bmmt_msg_settings settings;
    bmmt_buffer buf[BMMT_MAX_MSG_BUFFERS];
    bool valid[BMMT_MAX_MSG_BUFFERS];
    uint8_t write_index;
    uint8_t read_index;
    bmmt_t mmt;
}bmmt_msg;



static int bmmt_p_copy_payload(void *context, batom_accum_t accum, batom_cursor *cursor, unsigned bytes);
static int bmmt_p_stream_copy_payload(void *context, batom_accum_t accum, batom_cursor *cursor, unsigned bytes);
static void bmmt_p_stream_data(void *context, batom_accum_t data, const bmmt_demux_time_info *time_info);
static void bmmt_p_feed_playpump( bmmt_stream_t stream, const void *buf, unsigned len);
static void bmmt_p_atom_free(batom_t atom, void *user);
static void * bmmt_p_io_playback_thread(void *context);
static void * bmmt_p_io_live_thread(void *context);
static int bmmt_p_process_tlv_packet(bmmt_t mmt, batom_cursor *cursor);
static void bmmt_p_recpump_dataready_callback(void *context, int param);
static void bmmt_p_recpump_overflow_callback(void *context, int param);

static const batom_user b_atom_mmt = {
    bmmt_p_atom_free,
    0
};



static void bmmt_p_recpump_dataready_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
    return;
}

static void bmmt_p_recpump_overflow_callback(void *context, int param)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
    BDBG_ERR(("Recpump overflow!"));
    return;
}

static void bmmt_p_atom_free(batom_t atom, void *user)
{
    const batom_vec *vec = batom_get_vec(atom, 0);
    BDBG_ASSERT(user);
    BDBG_MSG(("free_mmt %p", vec->base));
    BSTD_UNUSED(atom);
    BKNI_Free(vec->base);
    return;
}
static int bmmt_p_process_tlv_packet(bmmt_t mmt, batom_cursor *cursor)
{
    int rc=0;
    btlv_ip_parser_result ip_result;
    batom_cursor payload;
    bmmt_stream_t stream=NULL;
    bmmt_msg_t msg=NULL;
    bmmt_packet_header mmt_header;

    BDBG_OBJECT_ASSERT(mmt,bmmt);
    rc=btlv_ip_parser_process(&mmt->ip_parser, cursor, &ip_result);
    if (rc)
    {
        BDBG_ERR(("IP parsing error"));
        goto done;
    }
    BKNI_AcquireMutex(mmt->mutex);
    if(ip_result.type == btlv_ip_parser_result_signaling)
    {
        batom_cursor tlv_si_payload;
        msg = mmt->tlv_msg;
        if (!msg)
        {
            BDBG_MSG(("TLV msg not opened"));
        }
        else
        {
            BATOM_CLONE(&tlv_si_payload, &ip_result.data);
            msg->valid[msg->write_index] = false;
            msg->buf[msg->write_index].offset = (unsigned)batom_cursor_size(&tlv_si_payload);;
            BDBG_ASSERT(msg->buf[msg->write_index].length >= msg->buf[msg->write_index].offset);
            batom_cursor_copy(&tlv_si_payload,msg->buf[msg->write_index].data,msg->buf[msg->write_index].offset);
            msg->valid[msg->write_index] = true;
            msg->write_index += 1;
            msg->write_index = (msg->write_index% BMMT_MAX_MSG_BUFFERS);
            BDBG_MSG(("TLV SI processing %u",msg->buf[msg->write_index].offset));
        }
        goto done;

    }
    else
    {
        if (mmt->ip_filter_valid)
        {
            if(!btlv_ip_demux(&ip_result, &mmt->addr, &payload))
            {
                 BDBG_MSG(("Unknown IP"));
                 goto done;
            }
        }
        else
        {
            BDBG_MSG(("no IP set"));
            goto done;
        }

    }

    rc = bmmt_parse_mmt_header(&payload,&mmt_header);
    if (rc )
    {
        BDBG_WRN(("MMTP parsing error"));
        goto done;
    }

    if (mmt_header.type == BMMT_TYPE_MPU)
    {
        for (stream = BLST_Q_FIRST(&mmt->streams);(stream && stream->settings.pid != mmt_header.packet_id);stream=BLST_Q_NEXT(stream, link));
        if (!stream)
        {
             BDBG_MSG(("Unknown packet ID"));
             goto done;
        }
        else
        {
            rc = bmmt_demux_stream_process_payload(mmt->demux, stream->demux_stream, &mmt_header, &payload);
        }
    }
    else
    {
        if(mmt_header.type  == BMMT_TYPE_SIGNALLING_MESSAGE )
        {
            for (msg = BLST_Q_FIRST(&mmt->msgs);(msg && msg->settings.pid != mmt_header.packet_id);msg=BLST_Q_NEXT(msg, link));
            if (!msg)
            {
                BDBG_MSG(("unknown message MMT packet ID"));
                goto done;
            }
            else
            {
                bmmt_signalling_header signalling_header;
                batom_cursor mmt_si_payload;
                BDBG_MSG(("MMT SI processing "));
                rc = bmmt_parse_signalling_header(&payload, &signalling_header);
                if (rc)
                {
                    BDBG_WRN(("MMTP SI header parsing error"));
                }
                else
                {
                    if(signalling_header.f_i==0)
                    {
                        BATOM_CLONE(&mmt_si_payload, &payload);
                        msg->valid[msg->write_index] = false;
                        msg->buf[msg->write_index].offset = (unsigned)batom_cursor_size(&mmt_si_payload);
                        BDBG_ASSERT(msg->buf[msg->write_index].length >= msg->buf[msg->write_index].offset);
                        batom_cursor_copy(&mmt_si_payload,msg->buf[msg->write_index].data,msg->buf[msg->write_index].offset);
                        msg->valid[msg->write_index] = true;
                        BDBG_MSG(("%s msg->write_index %u msg->buf[msg->write_index].offset %u",
                                   __extension__ __FUNCTION__,msg->write_index, msg->buf[msg->write_index].offset));
                        msg->write_index += 1;
                        msg->write_index = ( msg->write_index % BMMT_MAX_MSG_BUFFERS);
                        rc = bmmt_demux_process_signaling_message(mmt->demux, &payload, &signalling_header);

                    }
                    else
                    {
                        if (signalling_header.f_i >= 1 && signalling_header.f_i <=3)
                        {
                            size_t len = batom_cursor_size(&payload);
                            if (signalling_header.f_i == 1) { msg->buf[msg->write_index].offset=0; }
                            msg->valid[msg->write_index] = false;
                            BDBG_ASSERT(msg->buf[msg->write_index].length  >= (msg->buf[msg->write_index].offset+len));
                            batom_cursor_copy(&payload, (uint8_t *)msg->buf[msg->write_index].data + msg->buf[msg->write_index].offset,len);
                            msg->buf[msg->write_index].offset += (unsigned)len;
                        }
                        if (signalling_header.f_i == 3)
                        {
                            batom_vec signal_vec;
                            batom_cursor signal_payload;
                            msg->valid[msg->write_index] = true;
                            BATOM_VEC_INIT(&signal_vec,msg->buf[msg->write_index].data,msg->buf[msg->write_index].offset);
                            batom_cursor_from_vec(&signal_payload,&signal_vec,1);
                            rc = bmmt_demux_process_signaling_message(mmt->demux, &signal_payload, &signalling_header);
                            BDBG_MSG(("%s msg->write_index %u msg->buf[msg->write_index].offset %u",
                                   __extension__ __FUNCTION__,msg->write_index, msg->buf[msg->write_index].offset));
                            msg->write_index += 1;
                            msg->write_index = ( msg->write_index % BMMT_MAX_MSG_BUFFERS);
                        }
                    }
                }
            }
        }
        else
        {
            BDBG_MSG(("not handling mmtp type %d",mmt_header.type ));
        }
    }
done:
    BKNI_ReleaseMutex(mmt->mutex);
    return rc;
}

static void * bmmt_p_io_live_thread(void *context)
{
    bmmt_t mmt = (bmmt_t) context;
    unsigned last_io_data=0;
    unsigned recycled = 0;
    int rc;
    void *buf;
    btlv_parser_packets packets;
    const void *data_buffer[2];
    size_t data_buffer_size[2];
    unsigned offset;
    unsigned skip = 0;
    unsigned len;
    unsigned k=0;
    BDBG_OBJECT_ASSERT(mmt,bmmt);
    while (mmt->started)
    {
        rc = NEXUS_Recpump_GetDataBufferWithWrap(mmt->recpump, &data_buffer[0], &data_buffer_size[0], &data_buffer[1],&data_buffer_size[1]);
        if(rc!=NEXUS_SUCCESS)
        {
            (void)BERR_TRACE(rc);
            break;
        }
        if (data_buffer_size[0] +  data_buffer_size[1] <= skip) {
            BKNI_Sleep(10);
            continue;
        }
        recycled = 0;
        for (k=0;k<2;k++) {
            if (data_buffer_size[k]==0) {
                continue;
            }
            if (data_buffer_size[k] > skip) {
                buf = (uint8_t *)data_buffer[k] + skip;
                len = data_buffer_size[k] - skip;
                for (offset=0;offset + mmt->io_buffer_size <=len;offset+=mmt->io_buffer_size)
                {

                    if(mmt->open_settings.input_format == ebmmt_input_format_tlv)
                    {
                        rc = btlv_parser_process_tlv(&mmt->parser, (uint8_t *)buf + offset, mmt->io_buffer_size, &packets);
                    }
                    else
                    {
                        rc = btlv_parser_process_mpeg2ts(&mmt->parser, (uint8_t *)buf + offset, &packets);
                    }

                    if(rc==0)
                    {
                        unsigned i;
                        if(packets.packet_valid)
                        {
                            rc = bmmt_p_process_tlv_packet(mmt,&packets.packet);
                        }
                        for(i=0;i<packets.count;i++)
                        {
                            batom_cursor cursor;
                            batom_cursor_from_vec(&cursor, packets.packets+i, 1);
                            rc = bmmt_p_process_tlv_packet(mmt, &cursor);
                        }
                        if(packets.keep_previous_packets )
                        {
                            last_io_data++;
                        }
                        else
                        {
                             if(packets.keep_current_packet)
                             { /* current packet to the first */
                                 recycled += last_io_data*mmt->io_buffer_size;
                                 last_io_data = 1;
                              }
                              else
                              {
                                  recycled += (last_io_data+1)*mmt->io_buffer_size;
                                  last_io_data = 0; /* reset buffer */
                              }
                        }
                    }
                    else
                    {
                        if(rc==BTLV_RESULT_UNKNOWN_PID)
                        {
                            if (!last_io_data)
                            {
                                recycled += mmt->io_buffer_size;
                            }
                            else
                            {
                                last_io_data++;
                            }
                            continue;
                        }
                        else
                        {
                            btlv_parser_reset(&mmt->parser);
                            recycled += (last_io_data+1)*mmt->io_buffer_size;
                            last_io_data = 0;
                        }
                    }
                }
                skip = 0;
            }
            else
            {
                skip -= data_buffer_size[k];
            }
        }
        BDBG_ASSERT((data_buffer_size[0] + data_buffer_size[1]) >= recycled);
        NEXUS_Recpump_DataReadComplete(mmt->recpump, recycled);

        #if 0
        if ((data_buffer_size[0] + data_buffer_size[1]) < (16*4096 + skip)) {
            BKNI_Sleep(10);
        }
        #endif
        skip = data_buffer_size[0] + data_buffer_size[1] - recycled;
        BDBG_MSG(("%s skip %u size 0:%lu 1:%lu recycled %u ", __extension__ __FUNCTION__,skip,(unsigned long)data_buffer_size[0],(unsigned long)data_buffer_size[1] ,recycled));
    }
    return NULL;
}

static void * bmmt_p_io_playback_thread(void *context)
{
    bmmt_t mmt = (bmmt_t) context;
    unsigned last_io_data=0;
    BDBG_OBJECT_ASSERT(mmt,bmmt);
    while (mmt->started)
    {
        int rc;
        void *buf;
        btlv_parser_packets packets;
        if(last_io_data >= mmt->num_io_buffers)
        {
            BDBG_ERR(("last_io_data %x",last_io_data));
            BDBG_ASSERT(0);
        }
        buf  = mmt->io_data[last_io_data].io_buf;
loop:
        rc = fread(buf,1,mmt->io_buffer_size,mmt->fin);
        if(rc<=0)
        {
            BDBG_WRN(("reached the end of mmt playback!!!"));
            if (mmt->open_settings.loop) {
               fseek(mmt->fin, 0, SEEK_SET);
               btlv_parser_reset(&mmt->parser);
               NEXUS_Playpump_Flush(mmt->playpump);
               goto loop;
            }
            else {
               break;
            }
        }
        if(mmt->open_settings.input_format == ebmmt_input_format_tlv)
        {
            rc = btlv_parser_process_tlv(&mmt->parser, buf,rc, &packets);
        }
        else
        {
            rc = btlv_parser_process_mpeg2ts(&mmt->parser, buf, &packets);
        }

        if(rc==0)
        {
            unsigned i;
            if(packets.packet_valid)
            {
                rc = bmmt_p_process_tlv_packet(mmt,&packets.packet);
            }
            for(i=0;i<packets.count;i++)
            {
                batom_cursor cursor;
                batom_cursor_from_vec(&cursor, packets.packets+i, 1);
                rc = bmmt_p_process_tlv_packet(mmt, &cursor);
            }
            if(packets.keep_previous_packets )
            {
                last_io_data++;
            }
            else
            {
                 if(packets.keep_current_packet)
                 { /* current packet to the first */
                     mmt->io_data[last_io_data].io_buf = mmt->io_data[0].io_buf;
                     mmt->io_data[0].io_buf = buf;
                     last_io_data = 1;
                  }
                  else
                  {
                        last_io_data = 0; /* reset buffer */
                  }
            }
        }
        else
        {
            if(rc==BTLV_RESULT_UNKNOWN_PID)
            {
                continue;
            }
            else
            {
                btlv_parser_reset(&mmt->parser);
                last_io_data = 0;
            }
            /*fflush(state.fout);*/
        }
    }
    return NULL;
}


static int bmmt_p_copy_payload(void *context, batom_accum_t accum, batom_cursor *cursor, unsigned bytes)
{
    void *data;
    batom_t atom;
    batom_factory_t factory = context;
    size_t copied_bytes;
    BDBG_MSG(("%s >>>", __extension__ __FUNCTION__));
    data = BKNI_Malloc(bytes);
    if(data==NULL) {
        goto err_alloc;
    }
    copied_bytes = batom_cursor_copy(cursor, data, bytes);
    if(copied_bytes != bytes) {
        BDBG_LOG(("%u %u", (unsigned)copied_bytes, bytes));
        goto err_copy;
    }
    atom = batom_from_range(factory, data, bytes, &b_atom_mmt, NULL);
    if(atom==NULL) {
        goto err_atom;
    }
    batom_accum_add_atom(accum, atom);
    batom_release(atom);
    BDBG_MSG(("%s <<<", __extension__ __FUNCTION__));
    return 0;

err_atom:
err_copy:
    BKNI_Free(data);
err_alloc:
    return -1;
}

static int bmmt_p_stream_copy_payload(void *context, batom_accum_t accum, batom_cursor *cursor, unsigned bytes)
{
    bmmt_stream_t stream = context;
    bmmt_t mmt = stream->mmt;
    BDBG_OBJECT_ASSERT(mmt,bmmt);
    BDBG_OBJECT_ASSERT(stream,bmmt_stream);
    BDBG_MSG(("%s >>>", __extension__ __FUNCTION__));
    if(stream->buffer.offset + bytes < stream->buffer.length) {
        void *data = (uint8_t *)stream->buffer.data + stream->buffer.offset;
        size_t copied_bytes;
        copied_bytes = batom_cursor_copy(cursor, data, bytes);
        if(copied_bytes != bytes) {
            return -1;
        }
        batom_accum_add_range(accum, (uint8_t *)stream->buffer.data + stream->buffer.offset, bytes);
        stream->buffer.offset += bytes;
        return 0;
    }
    BDBG_MSG(("%s <<<<", __extension__ __FUNCTION__));
    return bmmt_p_copy_payload(mmt->factory, accum, cursor, bytes);
}


static void bmmt_p_feed_playpump( bmmt_stream_t stream, const void *buf, unsigned len)
{
    NEXUS_Error rc;
    unsigned offset;
    unsigned left;
    bmmt_t mmt = stream->mmt;
    BDBG_OBJECT_ASSERT(mmt,bmmt);
    BDBG_OBJECT_ASSERT(stream,bmmt_stream);
    for(offset=0,left=len;left>0;) {
        unsigned to_copy;
        if(mmt->buffer.length > mmt->buffer.offset + left) {
            to_copy = left;
        } else {
            to_copy  = (mmt->buffer.length - mmt->buffer.offset);
        }
        if(to_copy>0) {
            BKNI_Memcpy((uint8_t *)mmt->buffer.data + mmt->buffer.offset, (uint8_t *)buf+offset, to_copy);
            left -= to_copy;
            offset += to_copy;
            mmt->buffer.offset += to_copy;
        } else { /* to_copy == 0 */
            unsigned dropThreshold = 16384;
            if(mmt->buffer.offset>0) {
                BDBG_MSG(("Complete %u", mmt->buffer.offset));
                NEXUS_Playpump_WriteComplete(mmt->playpump, 0, mmt->buffer.offset);
            }
            mmt->buffer.length = 0;
            mmt->buffer.offset = 0;
            rc = NEXUS_Playpump_GetBuffer(mmt->playpump, &mmt->buffer.data, &mmt->buffer.length);
            if(rc!=NEXUS_SUCCESS) {
                (void)BERR_TRACE(rc);
                return;
            }
            if(mmt->buffer.length < dropThreshold) {
                if( (uint8_t *)mmt->playpumpStatus.bufferBase + (mmt->playpumpStatus.fifoSize - dropThreshold) <= (uint8_t *)mmt->buffer.data) {
                    BDBG_MSG(("Drop %u", (unsigned)mmt->buffer.length));
                    NEXUS_Playpump_WriteComplete(mmt->playpump, mmt->buffer.length , 0);
                } else {
                    BDBG_MSG(("Wait"));
                    BKNI_Sleep(10); /* wait 10 milliseconds */
                }
                mmt->buffer.length = 0;
            }
        }
    }
    return;
}

static void bmmt_p_stream_data(void *context, batom_accum_t data, const bmmt_demux_time_info *time_info)
{
    bmmt_stream_t stream = context;
    bmmt_t mmt = stream->mmt;
    batom_cursor cursor;
    unsigned i;
    uint8_t pes_header[BMEDIA_PES_HEADER_MAX_SIZE];
    size_t pes_header_length;
    size_t length = 0;
    BDBG_OBJECT_ASSERT(mmt,bmmt);
    BDBG_OBJECT_ASSERT(stream,bmmt_stream);
    if(stream->settings.stream_type == bmmt_stream_type_h265)
    {
        length = 0; /* unbounded PES for video*/
    }
    else
    {
        length = batom_accum_len(data); /* bounded otherwise */
    }

    BDBG_MSG(("MPU:%x PTS:%x", (unsigned)(time_info->mpu_time), (unsigned)time_info->pes_info.pts));
    pes_header_length = bmedia_pes_header_init(pes_header, length , &time_info->pes_info);
    mmt->buffer.offset = 0;
    mmt->buffer.length = 0;
    if (mmt->fout)
    {
        fwrite(pes_header, pes_header_length, 1, mmt->fout);
    }
    else
    {
        bmmt_p_feed_playpump(stream, pes_header, pes_header_length);
    }

    batom_cursor_from_accum(&cursor, data);
    for(i=0;i<cursor.count;i++)
    {
        if (mmt->fout)
        {
            fwrite(cursor.vec[i].base, cursor.vec[i].len, 1, mmt->fout);
        }
        else
        {
            bmmt_p_feed_playpump(stream, cursor.vec[i].base, cursor.vec[i].len);

        }
    }

    if(!mmt->fout && mmt->buffer.offset)
        NEXUS_Playpump_WriteComplete(mmt->playpump, 0, mmt->buffer.offset);
    stream->buffer.offset= 0;

    return;

}

void bmmt_get_default_open_settings(bmmt_open_settings *open_settings)
{
    BKNI_Memset(open_settings,0,sizeof(*open_settings));
    return;
}

bmmt_t bmmt_open(bmmt_open_settings *open_settings)
{
    bmmt_t mmt = NULL;
    NEXUS_PlaypumpSettings playpumpSettings;
    #if 0
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
    #endif
    unsigned i;
    BDBG_WRN(("%s >>>", __extension__ __FUNCTION__));
    mmt = BKNI_Malloc(sizeof(*mmt));
    BKNI_Memset(mmt,0,sizeof(*mmt));
    BDBG_ASSERT(mmt);
    BDBG_OBJECT_INIT(mmt, bmmt);
    if (open_settings->input_format == ebmmt_input_format_mpeg2ts)
    {
        if (open_settings->tlv_pid)
        {
            BDBG_WRN(("%s MPEG2TS PID %x", __extension__ __FUNCTION__,open_settings->tlv_pid));
            btlv_parser_init(&mmt->parser, open_settings->tlv_pid);
            mmt->num_io_buffers = BMMT_MAX_TS_BUFFERS;
            mmt->io_buffer_size = BMPEG2TS_PKT_LEN;
        }
        else
        {
            BDBG_ERR(("%s: invalid tlv_pid %x", __extension__ __FUNCTION__,open_settings->tlv_pid));
            BKNI_Free(mmt);
            goto error;
        }
    }
    else
    {
        if (open_settings->input_format == ebmmt_input_format_tlv)
        {
            btlv_parser_init(&mmt->parser, BTLV_DEFAULT_PID);
            mmt->num_io_buffers = BMMT_MAX_TLV_BUFFERS;
            mmt->io_buffer_size = BMMT_TLV_PKT_READ_SIZE;
        }
        else
        {
            BDBG_ERR(("%s: invalid input format %d", __extension__ __FUNCTION__,open_settings->input_format));
            BKNI_Free(mmt);
            goto error;
        }
    }
    btlv_ip_parser_init(&mmt->ip_parser);
    BDBG_WRN(("%s: input format %d", __extension__ __FUNCTION__,open_settings->input_format));
    BDBG_WRN(("%s: playback %s", __extension__ __FUNCTION__,open_settings->playback?"true":"false"));
    if (open_settings->playback)
    {
        BDBG_WRN(("%s: fileName %s", __extension__ __FUNCTION__,open_settings->fileName));
        mmt->fin = fopen(open_settings->fileName,"rb");
        BDBG_ASSERT(mmt->fin);
    }
    else
    {
         NEXUS_RecpumpSettings recpumpSettings;
         NEXUS_RecpumpOpenSettings recpumpOpenSettings;
         NEXUS_PidChannelSettings pidCfg;

         BDBG_WRN(("%s parserBand %d", __extension__ __FUNCTION__,(int)open_settings->parserBand));

         NEXUS_PidChannel_GetDefaultSettings(&pidCfg);
         NEXUS_ParserBand_GetAllPassPidChannelIndex(open_settings->parserBand, &pidCfg.pidChannelIndex);
         if (open_settings->input_format == ebmmt_input_format_tlv)
         {
            mmt->allPassPidChannel = NEXUS_PidChannel_Open(open_settings->parserBand, 0, &pidCfg);
         }
         else
         {
             mmt->allPassPidChannel = NEXUS_PidChannel_Open(open_settings->parserBand,open_settings->tlv_pid, &pidCfg);
         }
         BDBG_ASSERT(mmt->allPassPidChannel);

         BKNI_CreateEvent(&mmt->rec_event);
         NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);
         recpumpOpenSettings.data.dataReadyThreshold  = (recpumpOpenSettings.data.bufferSize * 3)/4;
         recpumpOpenSettings.data.bufferSize = recpumpOpenSettings.data.bufferSize*2;
         recpumpOpenSettings.indexType = NEXUS_RecpumpIndexType_eNone;
         mmt->recpump = NEXUS_Recpump_Open(NEXUS_ANY_ID, &recpumpOpenSettings);
         BDBG_ASSERT(mmt->recpump);

         NEXUS_Recpump_GetSettings(mmt->recpump, &recpumpSettings);
         recpumpSettings.data.dataReady.callback = bmmt_p_recpump_dataready_callback;
         recpumpSettings.data.overflow.callback = bmmt_p_recpump_overflow_callback;
         recpumpSettings.data.dataReady.context = mmt->rec_event;
         recpumpSettings.bandHold = NEXUS_RecpumpFlowControl_eDisable;
         recpumpSettings.outputTransportType = NEXUS_TransportType_eBulk;
         NEXUS_Recpump_SetSettings(mmt->recpump, &recpumpSettings);

         NEXUS_Recpump_AddPidChannel(mmt->recpump,mmt->allPassPidChannel, NULL);
    }

    mmt->factory = batom_factory_create(bkni_alloc, 256);
    BDBG_ASSERT(mmt->factory);

    bmmt_demux_config_init(&mmt->demux_config);
    mmt->demux_config.context = mmt->factory;
    mmt->demux_config.copy_payload = bmmt_p_copy_payload;
    mmt->demux = bmmt_demux_create(mmt->factory, &mmt->demux_config);
    BDBG_ASSERT(mmt->demux);
    #if 0
    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
    playpumpOpenSettings.fifoSize = 10*1024*1024;
    #endif
    mmt->playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
    BDBG_ASSERT(mmt->playpump);

    NEXUS_Playpump_GetSettings(mmt->playpump, &playpumpSettings);
    playpumpSettings.transportType = NEXUS_TransportType_eMpeg2Pes;
    NEXUS_Playpump_SetSettings(mmt->playpump, &playpumpSettings);

    mmt->io_data = BKNI_Malloc(sizeof(bmmt_io_data)*mmt->num_io_buffers);
    BDBG_ASSERT(mmt->io_data);
    for (i=0;i<mmt->num_io_buffers;i++)
    {
        mmt->io_data[i].io_buf = BKNI_Malloc(mmt->io_buffer_size);
        BDBG_ASSERT(mmt->io_data[i].io_buf);
    }

    BLST_Q_INIT(&mmt->streams);
    BLST_Q_INIT(&mmt->msgs);
    BKNI_CreateMutex(&mmt->mutex);
    mmt->ip_filter_valid = false;
    BKNI_Memcpy(&mmt->open_settings,open_settings,sizeof(*open_settings));
    if (open_settings->pesOut)
    {
       mmt->fout = fopen(open_settings->fileOut,"wb");
    }
    else
    {
        mmt->fout = NULL;
    }
    BDBG_WRN(("%s <<<", __extension__ __FUNCTION__));
    return mmt;

error:
    BDBG_WRN(("%s <<<", __extension__ __FUNCTION__));
    return NULL;
}

int bmmt_close(bmmt_t mmt)
{
    batom_factory_stats factory_stats;
    unsigned i;
    bmmt_stream_t stream,prev_stream;
    bmmt_msg_t msg,prev_msg;
    BDBG_WRN(("%s >>>", __extension__ __FUNCTION__));
    BDBG_OBJECT_ASSERT(mmt,bmmt);
    stream = BLST_Q_FIRST(&mmt->streams);
    while (stream)
    {
        BLST_Q_REMOVE(&mmt->streams, stream,link);
        NEXUS_Playpump_ClosePidChannel(mmt->playpump,stream->pid_channel);
        bmmt_timestamp_queue_destroy(stream->timestamp_queue);
        bmmt_demux_stream_destroy(mmt->demux,stream->demux_stream);
        BKNI_Free(stream->buffer.data);
        prev_stream = stream;
        stream = BLST_Q_NEXT(stream,link);
        BDBG_OBJECT_DESTROY(prev_stream, bmmt_stream);
        BKNI_Free(prev_stream);
    }

    msg = BLST_Q_FIRST(&mmt->msgs);
    while (msg)
    {
        for (i=0;i<BMMT_MAX_MSG_BUFFERS;i++)
        {
            BKNI_Free(msg->buf[i].data);
        }
        BLST_Q_REMOVE(&mmt->msgs, msg,link);
        prev_msg = msg;
        msg = BLST_Q_NEXT(msg,link);
        BDBG_OBJECT_DESTROY(prev_msg, bmmt_msg);
        BKNI_Free(prev_msg);
    }
    if (mmt->tlv_msg)
    {
        for (i=0;i<BMMT_MAX_MSG_BUFFERS;i++)
        {
            BKNI_Free(mmt->tlv_msg->buf[i].data);
        }
        BDBG_OBJECT_DESTROY(mmt->tlv_msg, bmmt_msg);
        BKNI_Free(mmt->tlv_msg);
    }
    for (i=0;i<mmt->num_io_buffers;i++)
    {
        BKNI_Free(mmt->io_data[i].io_buf);
    }
    BKNI_Free(mmt->io_data);

    NEXUS_Playpump_CloseAllPidChannels(mmt->playpump);
    NEXUS_Playpump_Close(mmt->playpump);

    if (mmt->open_settings.playback)
    {
        fclose(mmt->fin);
    }
    else
    {
        NEXUS_Recpump_RemoveAllPidChannels(mmt->recpump);
        NEXUS_Recpump_Close(mmt->recpump);
        NEXUS_PidChannel_Close(mmt->allPassPidChannel);
        BKNI_DestroyEvent(mmt->rec_event);
    }

    if (mmt->fout)
    {
        fclose(mmt->fout);
    }

    bmmt_demux_destroy(mmt->demux);
    batom_factory_get_stats(mmt->factory, &factory_stats);
    BDBG_WRN(("status: atoms[live:%u allocated:%u freed:%u] alloc[pool:%u/%u arena:%u/%u alloc:%u/%u]", factory_stats.atom_live, factory_stats.atom_allocated, factory_stats.atom_freed, factory_stats.alloc_pool, factory_stats.free_pool, factory_stats.alloc_arena, factory_stats.free_arena, factory_stats.alloc_alloc, factory_stats.free_alloc));
    batom_factory_dump(mmt->factory);
    batom_factory_destroy(mmt->factory);
    BKNI_DestroyMutex(mmt->mutex);
    BDBG_OBJECT_DESTROY(mmt, bmmt);
    BKNI_Free(mmt);
    BDBG_WRN(("%s <<<", __extension__ __FUNCTION__));
    return 0;
}

int bmmt_start(bmmt_t mmt)
{
    int rc;
    BDBG_OBJECT_ASSERT(mmt,bmmt);
    mmt->started = true;
    if (!mmt->open_settings.playback) {
        NEXUS_Recpump_Start(mmt->recpump);
    }
    NEXUS_Playpump_Start(mmt->playpump);
    NEXUS_Playpump_GetStatus(mmt->playpump, &mmt->playpumpStatus);
    if (mmt->open_settings.playback)
    {
        rc = pthread_create(&mmt->thread, NULL, bmmt_p_io_playback_thread, mmt);
    }
    else
    {
        rc = pthread_create(&mmt->thread, NULL, bmmt_p_io_live_thread, mmt);
    }
    return rc;
}
int bmmt_stop(bmmt_t mmt)
{

    BDBG_OBJECT_ASSERT(mmt,bmmt);
    mmt->started=false;
    pthread_join(mmt->thread,NULL);
    if (!mmt->open_settings.playback)
    {
        NEXUS_Recpump_Stop(mmt->recpump);
    }
    NEXUS_Playpump_Stop(mmt->playpump);
    btlv_ip_parser_shutdown(&mmt->ip_parser);
    return 0;
}

void bmmt_stream_get_default_settings(bmmt_stream_settings *settings)
{
    BKNI_Memset(settings,0,sizeof(*settings));
    return;
}

bmmt_stream_t bmmt_stream_open(bmmt_t mmt, bmmt_stream_settings *settings)
{

    bmmt_stream_t stream = NULL;
    BDBG_OBJECT_ASSERT(mmt,bmmt);
    if (!settings)
    {
        BDBG_WRN(("bmmt_stream_settings is NULL"));
    }
    else
    {

        stream = BKNI_Malloc(sizeof(*stream));
        BDBG_ASSERT(stream);
        BKNI_Memset(stream,0,sizeof(*stream));
        BDBG_OBJECT_INIT(stream, bmmt_stream);
        stream->buffer.length = 2*1024*1024;
        stream->buffer.offset = 0;
        stream->buffer.data = BKNI_Malloc(stream->buffer.length);
        BDBG_ASSERT(stream->buffer.data);
        bmmt_demux_stream_config_init(&stream->demux_stream_config);
        stream->demux_stream_config.stream_type = settings->stream_type;
        stream->demux_stream_config.packet_id = settings->pid;
        if (settings->stream_type == bmmt_stream_type_h265) {
            stream->pes_id = 0xE0;
        }
        else {
           if (settings->stream_type == bmmt_stream_type_aac) {
              stream->pes_id = 0xC0;
           }
           else
           {
              stream->pes_id = 0xBD;
           }
        }
        stream->demux_stream_config.pes_stream_id = stream->pes_id;
        stream->demux_stream_config.stream_context = stream;
        stream->demux_stream_config.stream_data = bmmt_p_stream_data;
        stream->demux_stream_config.copy_payload = bmmt_p_stream_copy_payload;
        stream->demux_stream = bmmt_demux_stream_create(mmt->demux, &stream->demux_stream_config);
        BDBG_ASSERT(stream->demux_stream);
        stream->mmt = mmt;
        stream->timestamp_queue = bmmt_timestamp_queue_create();
        BDBG_ASSERT(stream->timestamp_queue);
        stream->pid_channel = NEXUS_Playpump_OpenPidChannel(mmt->playpump, stream->pes_id, NULL);
        BDBG_ASSERT(stream->pid_channel);
        BKNI_AcquireMutex(mmt->mutex);
        BLST_Q_INSERT_TAIL(&mmt->streams,stream,link);
        BKNI_Memcpy(&stream->settings,settings,sizeof(*settings));
        BKNI_ReleaseMutex(mmt->mutex);
    }
    return stream;
}

NEXUS_PidChannelHandle bmmt_stream_get_pid_channel(bmmt_stream_t stream)
{
    BDBG_OBJECT_ASSERT(stream,bmmt_stream);
    return stream->pid_channel;
}

int bmmt_stream_close(bmmt_stream_t stream)
{
     bmmt_t mmt = stream->mmt;
     BDBG_OBJECT_ASSERT(stream,bmmt_stream);
     BDBG_OBJECT_ASSERT(mmt, bmmt);
     BKNI_AcquireMutex(mmt->mutex);
     BLST_Q_REMOVE(&mmt->streams, stream, link);
     BKNI_ReleaseMutex(mmt->mutex);
     NEXUS_Playpump_ClosePidChannel(mmt->playpump,stream->pid_channel);
     bmmt_timestamp_queue_destroy(stream->timestamp_queue);
     bmmt_demux_stream_destroy(mmt->demux,stream->demux_stream);
     BKNI_Free(stream->buffer.data);
     BDBG_OBJECT_DESTROY(stream, bmmt_stream);
     BKNI_Free(stream);
     return 0;
}

void bmmt_msg_get_default_settings(bmmt_msg_settings *settings)
{
   BKNI_Memset(settings,0,sizeof(*settings));
   return;
}
bmmt_msg_t bmmt_msg_open(bmmt_t mmt, bmmt_msg_settings *settings)
{
    bmmt_msg_t msg = NULL;
    BDBG_OBJECT_ASSERT(mmt, bmmt);
    if (!settings)
    {
        BDBG_WRN(("bmmt_msg_settings is null"));
    }
    else
    {
        unsigned i=0;
        msg = BKNI_Malloc(sizeof(*msg));
        BDBG_ASSERT(msg);
        BKNI_Memset(msg,0,sizeof(*msg));
        BDBG_OBJECT_INIT(msg, bmmt_msg);
        msg->mmt = mmt;
        msg->read_index = 0;
        msg->write_index = 0;
        for (i=0;i<BMMT_MAX_MSG_BUFFERS;i++)
        {
            msg->buf[i].length = (settings->msg_type == ebmmt_msg_type_mmt)?BMMT_MAX_MMT_SI_BUFFER_SIZE:BMMT_MAX_TLV_SI_BUFFER_SIZE;
            msg->buf[i].offset = 0;
            msg->buf[i].data = BKNI_Malloc(msg->buf[i].length );
            msg->valid[i] = false;
            BDBG_ASSERT(msg->buf[i].data);
        }
        BKNI_AcquireMutex(mmt->mutex);
        if (settings->msg_type == ebmmt_msg_type_mmt)
        {
          BLST_Q_INSERT_TAIL(&mmt->msgs,msg,link);
        }
        else
        {
           mmt->tlv_msg = msg;
        }
        BKNI_Memcpy(&msg->settings,settings,sizeof(*settings));
        BKNI_ReleaseMutex(mmt->mutex);
    }
    return msg;
}

size_t bmmt_msg_get_buffer(bmmt_msg_t msg, void *buf, size_t buf_len)
{
    bmmt_t mmt = msg->mmt;
    size_t len = 0;
    BDBG_OBJECT_ASSERT(msg, bmmt_msg);
    BDBG_OBJECT_ASSERT(mmt, bmmt);
    if (msg->settings.msg_type == ebmmt_msg_type_mmt && buf_len < BMMT_MAX_MMT_SI_BUFFER_SIZE)
    {
        BDBG_WRN(("ebmmt_msg_type_mmt buffer should be of size %u",BMMT_MAX_MMT_SI_BUFFER_SIZE));
        return len;
    }

    if (msg->settings.msg_type == ebmmt_msg_type_tlv && buf_len < BMMT_MAX_TLV_SI_BUFFER_SIZE) {
        BDBG_WRN(("ebmmt_msg_type_tlv buffer should be of size %u",BMMT_MAX_TLV_SI_BUFFER_SIZE));
        return len;
    }
    BKNI_AcquireMutex(mmt->mutex);
    if (msg->valid[msg->read_index])
    {
            BDBG_ASSERT(buf_len >= msg->buf[msg->read_index].offset);
            BKNI_Memcpy(buf,msg->buf[msg->read_index].data,msg->buf[msg->read_index].offset);
            len = msg->buf[msg->read_index].offset;
            msg->valid[msg->read_index] = false;
            msg->buf[msg->read_index].offset = 0;
            BDBG_MSG(("%s mmt->mmt_msg_r %u len %lu", __extension__ __FUNCTION__,msg->read_index,(unsigned long)len));
            msg->read_index +=1;
            msg->read_index = (msg->read_index % BMMT_MAX_MSG_BUFFERS);
    }
    BKNI_ReleaseMutex(mmt->mutex);
    return len;
}

int bmmt_msg_close(bmmt_msg_t msg)
{
    unsigned i=0;
    bmmt_t mmt = msg->mmt;
    BDBG_OBJECT_ASSERT(msg, bmmt_msg);
    BDBG_OBJECT_ASSERT(mmt, bmmt);
    if (msg->settings.msg_type == ebmmt_msg_type_mmt)
    {

       BKNI_AcquireMutex(mmt->mutex);
       BLST_Q_REMOVE(&mmt->msgs, msg, link);
       BKNI_ReleaseMutex(mmt->mutex);
    }
    else
    {
       BKNI_AcquireMutex(mmt->mutex);
       mmt->tlv_msg = NULL;
       BKNI_ReleaseMutex(mmt->mutex);
    }
    for (i=0;i<BMMT_MAX_MSG_BUFFERS;i++)
    {
        BKNI_Free(msg->buf[i].data);
    }
    BDBG_OBJECT_DESTROY(msg,bmmt_msg);
    BKNI_Free(msg);
    return 0;
}

bool bmmt_get_pl_table(void *buf, size_t len, bmmt_pl_table *pl_table)
{
    batom_cursor payload;
    batom_vec signal_vec;
    bmmt_signalling_message_header message_header;
    bmmt_package_access_message pa_message[16];
    unsigned pa_messages;
    bool table_found = false;
    unsigned j;

    BATOM_VEC_INIT(&signal_vec,buf,len);
    batom_cursor_from_vec(&payload,&signal_vec,1);
    bmmt_parse_signalling_message_header(&payload,&message_header);
    BDBG_MSG(("msg id %u len %u ver %u",message_header.message_id, message_header.length,message_header.version));
    if(!BMMT_IS_PA_MESSAGE(message_header.message_id))
        return table_found;
    bmmt_parse_package_access_message(&payload, pa_message, sizeof(pa_message)/sizeof(pa_message[0]), &pa_messages);
    BDBG_MSG(("pa %u",pa_messages));
    for(j=0;j<pa_messages;j++)
    {
        BDBG_MSG(("mp %#x",pa_message[j].table_id));
        if(pa_message[j].table_id==BMMT_TABLE_PLT)
        {
            bmmt_parse_pl_table(&pa_message[0].payload,pl_table);
            bmmt_print_pl_table(pl_table);
            table_found = true;
            break;
        }
    }
    return table_found;
}

bool bmmt_get_mp_table(void *buf, size_t len, bmmt_mp_table *mp_table)
{
    batom_cursor payload;
    batom_vec signal_vec;
    bmmt_signalling_message_header message_header;
    bmmt_package_access_message pa_message[16];
    unsigned pa_messages;
    bool table_found = false;
    unsigned j;

    BATOM_VEC_INIT(&signal_vec,buf,len);
    batom_cursor_from_vec(&payload,&signal_vec,1);
    bmmt_parse_signalling_message_header(&payload,&message_header);
    BDBG_MSG(("msg id %u len %u ver %u",message_header.message_id, message_header.length,message_header.version));
    if(!BMMT_IS_PA_MESSAGE(message_header.message_id))
        return table_found;
    bmmt_parse_package_access_message(&payload, pa_message, sizeof(pa_message)/sizeof(pa_message[0]), &pa_messages);
    BDBG_MSG(("pa %u",pa_messages));
    for(j=0;j<pa_messages;j++)
    {
        BDBG_MSG(("mp %#x",pa_message[j].table_id));
        if(pa_message[j].table_id==BMMT_TABLE_COMPLETE_MP)
        {
            bmmt_parse_mp_table(&pa_message[0].payload,mp_table);
            bmmt_print_mp_table(mp_table);
            table_found = true;
            break;
        }
    }
    return table_found;
}

bool bmmt_get_am_table(void *buf, size_t len, btlv_am_table *am_table)
{
    batom_cursor payload;
    batom_vec signal_vec;
    bool table_found = false;

    BATOM_VEC_INIT(&signal_vec,buf,len);
    batom_cursor_from_vec(&payload,&signal_vec,1);
    if(btlv_parse_am_table(&payload, am_table) == 0)
    {
        btlv_print_am_table(am_table);
        table_found = true;
    }


    return table_found;
}

void bmmt_set_ip_filter(bmmt_t mmt, btlv_ip_address *addr)
{
    BDBG_OBJECT_ASSERT(mmt, bmmt);
    BKNI_AcquireMutex(mmt->mutex);
    if (addr)
    {
        mmt->ip_filter_valid = true;
        BKNI_Memcpy(&mmt->addr,addr,sizeof(btlv_ip_address));
    }
    else
    {
        mmt->ip_filter_valid = false;
        BKNI_Memset(&mmt->addr,0,sizeof(btlv_ip_address));
    }
    BKNI_ReleaseMutex(mmt->mutex);
    return;
}

#define TLV_HDR_SIZE 4
bool bmmt_p_check_tlv_packet(uint8_t *buf, size_t len, uint32_t offset,uint8_t bitShift, uint8_t count)
{
    uint64_t bits_64 = 0,tmp=0;
    uint16_t byte0=0, byte1=0;
    uint32_t tmpOffset = offset;
    uint8_t i=0;
    uint16_t tlv_packet_length=0;
    bool found = false;
    while (count)
    {
        if ((tmpOffset + 8) > len)
        {
            break;
        }
        else
        {
            bits_64 = 0;
            for (i=0;i<8;i++)
            {
                tmp = 0;
                tmp = ((uint64_t)(buf[tmpOffset+i]) & 0xff);
                tmp = (tmp << (8*(7-i)));
                bits_64 |= tmp;
            }
            tmp = bits_64 << bitShift;
            if (((tmp >> 56) & 0xff) == 0x7f)
            {
                byte0 = ((uint16_t)(tmp>>32) & 0xff);
                byte1 = ((uint16_t)(tmp>>40) & 0xff);
                tlv_packet_length = (byte0 | byte1 << 8);
                BDBG_MSG(("tlv_pkt_len %#x",tlv_packet_length));
                BDBG_MSG(("tmpOffset %u",tmpOffset));
                tmpOffset = tmpOffset + tlv_packet_length + TLV_HDR_SIZE;
                count -= 1;
            }
            else
            {
               break;
            }
        }
    }

    found = (count==0)?true:false;
    return found;
}

uint8_t bmmt_get_tlv_sync_byte_bitshift(uint8_t *buf, size_t len)
{
    uint8_t bitShift=0;
    uint32_t offset = 0;
    uint64_t bits_64 = 0;
    uint64_t tmp;
    uint8_t i=0;
    bool found = false;
    BDBG_ASSERT((len > 8));

    for (bitShift=0; bitShift<8;bitShift++)
    {
        BDBG_WRN(("bitShift try %u",bitShift));
        for (offset=0;offset+8 < len;offset++)
        {
           bits_64 = 0x0;
           for (i=0;i<8;i++) {
               tmp = 0;
               tmp = ((uint64_t)(buf[offset+i]) & 0xff);
               tmp = (tmp << (8*(7-i)));
               bits_64 |= tmp;
           }
          /* BDBG_WRN(("++bits_64 %16"PRIx64"",bits_64));*/

           tmp = bits_64 << bitShift;
         /*  BDBG_WRN((">>bits_64 %16"PRIx64"",tmp));*/
           if (((tmp >> 56) & 0xff) == 0x7f)
           {
               found = bmmt_p_check_tlv_packet(buf,len,offset,bitShift, 8);
               if (found) {
                   break;
               }

           }
        }
        if (found) {
            break;
        }

    }
    if (!found)
    {
       bitShift = 0xff;
    }
    else
    {
       /* bitShift = bitShift==0?bitShift:8-bitShift;*/
        BDBG_WRN(("bitShift %u",bitShift));
    }
    return bitShift;
}
