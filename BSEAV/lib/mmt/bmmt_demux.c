/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 **************************************************************************/

#include "bstd.h"
#include "bmmt_demux.h"
#include "bkni.h"
#include "blst_slist.h"
#include "blst_list.h"
#include "biobits.h"

BDBG_MODULE(bmmt_demux);

#define BDBG_MSG_TRACE(x)  BDBG_MSG(x)
#define BDBG_TRACE_ERROR(x) (BDBG_LOG(("Error: %s (%s,%u)",#x,__FILE__,__LINE__)),x)

struct b_mmt_timestamp_entry {
    BLST_S_ENTRY(b_mmt_timestamp_entry) link;
    uint32_t mpu_sequence_number;
    bool extended_timestamp_valid;
    bool timestamp_valid;
    btlv_mpu_timestamp_descriptor_entry timestamp;
    btlv_mpu_extended_timestamp_descriptor_header extended_timestamp_descriptor_header;
    btlv_mpu_extended_timestamp_descriptor_entry extended_timestamp_descriptor_entry;
};

BLST_S_HEAD(b_mmt_timestamp_queueimestamp_entries,b_mmt_timestamp_entry);

struct bmmt_timestamp_queue {
    BDBG_OBJECT(bmmt_timestamp_queue)
    struct b_mmt_timestamp_queueimestamp_entries timestamps;
};

BDBG_OBJECT_ID(bmmt_timestamp_queue);

bmmt_timestamp_queue bmmt_timestamp_queue_create(void)
{
    bmmt_timestamp_queue queue;

    queue = BKNI_Malloc(sizeof(*queue));
    if(queue==NULL) {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    BDBG_OBJECT_INIT(queue, bmmt_timestamp_queue);
    BLST_S_INIT(&queue->timestamps);
    return queue;
}

static void b_tlv_mmt_queue_recycle_head(bmmt_timestamp_queue queue)
{
    struct b_mmt_timestamp_entry *timestamp = BLST_S_FIRST(&queue->timestamps);
    BDBG_MSG(("%p:recycle timestamp for mpu_sequence_number:%u", (void *)queue, (unsigned)timestamp->mpu_sequence_number));
    BLST_S_REMOVE_HEAD(&queue->timestamps, link);
    BKNI_Free(timestamp);
    return;
}

void bmmt_timestamp_queue_destroy(bmmt_timestamp_queue queue)
{
    struct b_mmt_timestamp_entry *timestamp;
    BDBG_OBJECT_ASSERT(queue, bmmt_timestamp_queue);
    while(NULL != (timestamp = BLST_S_FIRST(&queue->timestamps))) {
        b_tlv_mmt_queue_recycle_head(queue);
    }
    BDBG_OBJECT_DESTROY(queue, bmmt_timestamp_queue);
    BKNI_Free(queue);
    return ;
}

static struct b_mmt_timestamp_entry *b_tlv_mmt_queue_find_and_create_timestamp_entry(bmmt_timestamp_queue queue, uint32_t mpu_sequence_number)
{
    struct b_mmt_timestamp_entry *timestamp_prev;
    struct b_mmt_timestamp_entry *timestamp;

    for(timestamp_prev = NULL,timestamp=BLST_S_FIRST(&queue->timestamps); timestamp!=NULL; timestamp = BLST_S_NEXT(timestamp, link)) {
        if(timestamp->mpu_sequence_number == mpu_sequence_number) {
            return timestamp;
        }
        if(timestamp->mpu_sequence_number > mpu_sequence_number) {
            break;
        }
        timestamp_prev = timestamp;
    }
    BDBG_MSG(("%p:insert timestamp for mpu_sequence_number:%u", (void *)queue, (unsigned)mpu_sequence_number));
    timestamp = BKNI_Malloc(sizeof(*timestamp));
    if(timestamp==NULL) {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    BKNI_Memset(timestamp, 0, sizeof(*timestamp));
    timestamp->mpu_sequence_number = mpu_sequence_number;
    timestamp->extended_timestamp_valid = false;
    timestamp->timestamp_valid = false;
    if(timestamp_prev) {
        BLST_S_INSERT_AFTER(&queue->timestamps, timestamp_prev, timestamp, link);
    } else {
        BLST_S_INSERT_HEAD(&queue->timestamps, timestamp, link);
    }
    return timestamp;
}

int bmmt_timestamp_queue_add_timestamp_descriptors(bmmt_timestamp_queue queue, const btlv_mpu_timestamp_descriptor_entry * timestamps, unsigned n_timestamps)
{
    unsigned i;

    BDBG_OBJECT_ASSERT(queue, bmmt_timestamp_queue);
    for(i=0;i<n_timestamps;i++) {
        struct b_mmt_timestamp_entry *timestamp = b_tlv_mmt_queue_find_and_create_timestamp_entry(queue, timestamps[i].mpu_sequence_number);
        if(timestamps==NULL) {
            return -1;
        }
        BDBG_MSG(("add_timestamp:%p->%p [%u/%u] (%u," BDBG_UINT64_FMT ")", (void *)queue, (void *)timestamp, i, n_timestamps, (unsigned)timestamps[i].mpu_sequence_number, BDBG_UINT64_ARG(timestamps[i].mpu_presentation_time)));
        timestamp->timestamp_valid = true;
        timestamp->timestamp = timestamps[i];
    }
    return 0;
}

int bmmt_timestamp_queue_add_extended_timestamp_descriptors(bmmt_timestamp_queue queue,
                                            const btlv_mpu_extended_timestamp_descriptor_header *extended_timestamp_descriptor_header,
                                            const btlv_mpu_extended_timestamp_descriptor_entry *extended_timestamp_descriptor_entry, unsigned n_extended_timestamps
                                           )
{
    unsigned i;
    BDBG_OBJECT_ASSERT(queue, bmmt_timestamp_queue);
    for(i=0;i<n_extended_timestamps;i++) {
        struct b_mmt_timestamp_entry *timestamp = b_tlv_mmt_queue_find_and_create_timestamp_entry(queue, extended_timestamp_descriptor_entry[i].mpu_sequence_number);
        if(timestamp==NULL) {
            return -1;
        }
        timestamp->extended_timestamp_valid = true;
        timestamp->extended_timestamp_descriptor_header = *extended_timestamp_descriptor_header;
        timestamp->extended_timestamp_descriptor_entry = extended_timestamp_descriptor_entry[i];
    }
    return 0;
}

int bmmt_timestamp_queue_get_presentation_time(bmmt_timestamp_queue queue, uint32_t sequence_number, uint32_t sample_number, btlv_ntp_time *presentation_time)
{
    struct b_mmt_timestamp_entry *timestamp;
    unsigned i;

    BDBG_OBJECT_ASSERT(queue, bmmt_timestamp_queue);

    BDBG_MSG(("get_presentation_timestap:%p mpu_sequence_number:%u sample_number:%u", (void *)queue, (unsigned)sequence_number, (unsigned)sample_number));
    for(;;) {
        timestamp = BLST_S_FIRST(&queue->timestamps);
        if(timestamp==NULL) {
            return BTLV_RESULT_NOT_FOUND;
        }
        if(timestamp->mpu_sequence_number == sequence_number) {
            break;
        }
        if(timestamp->mpu_sequence_number > sequence_number) {
            return BTLV_RESULT_NOT_FOUND;
        }
        b_tlv_mmt_queue_recycle_head(queue);
    }
    if(!timestamp->timestamp_valid) {
        return BTLV_RESULT_NOT_FOUND;
    }
    if(!timestamp->extended_timestamp_valid) {
        if(sample_number!=0) {
            return BTLV_RESULT_NOT_FOUND;
        }
        *presentation_time = timestamp->timestamp.mpu_presentation_time;
    } else {
        int pts_offset;

        if(sample_number>=timestamp->extended_timestamp_descriptor_entry.num_of_au) {
            return BTLV_RESULT_NOT_FOUND;
        }
        pts_offset = timestamp->extended_timestamp_descriptor_entry.au[sample_number].dts_pts_offset;
        if(timestamp->extended_timestamp_descriptor_header.pts_offset_type == 1) {
            pts_offset += timestamp->extended_timestamp_descriptor_header.default_pts_offset * sample_number;
        } else if(timestamp->extended_timestamp_descriptor_header.pts_offset_type == 2) {
            for(i=0;i<sample_number;i++) {
                pts_offset += timestamp->extended_timestamp_descriptor_entry.au[i].pts_offset;
            }
        } else if(sample_number != 0) {
            return BTLV_RESULT_NOT_FOUND;
        }
        pts_offset -= timestamp->extended_timestamp_descriptor_entry.mpu_decoding_time_offset;
        BDBG_MSG(("get_presentation_timestap:%p %u,%u: pts_offset:%d", (void *)queue,  (unsigned)sequence_number, (unsigned)sample_number, (int)pts_offset));
        if(timestamp->extended_timestamp_descriptor_header.timescale_flag &&  timestamp->extended_timestamp_descriptor_header.timescale) {
            *presentation_time = btplv_ntp_add_offset(timestamp->timestamp.mpu_presentation_time, timestamp->extended_timestamp_descriptor_header.timescale, pts_offset);
            BDBG_MSG(("get_presentation_timestap:%p mpu_presentation_time:" BDBG_UINT64_FMT "  timescale:%u pts_offset:%d mpu_presentation_time:" BDBG_UINT64_FMT "", (void *)queue, BDBG_UINT64_ARG(timestamp->timestamp.mpu_presentation_time), (unsigned)timestamp->extended_timestamp_descriptor_header.timescale, (int)pts_offset, BDBG_UINT64_ARG(*presentation_time)));
        } else {
            if(sample_number == 0) {
                *presentation_time = timestamp->timestamp.mpu_presentation_time;
            } else {
                return BTLV_RESULT_UNKNOWN;
            }
        }
    }
    return 0;
}

BDBG_OBJECT_ID(bmmt_demux_stream);
struct bmmt_demux_stream {
    BDBG_OBJECT(bmmt_demux_stream)
    BLST_D_ENTRY(bmmt_demux_stream) link;
    bmmt_demux_stream_config config;
    bmmt_timestamp_queue timestamp_queue;
    bmmt_demux_t demux;
    batom_accum_t frame;
    batom_accum_t data_unit;
    bmmt_demux_time_info time_info;
#if 0
    uint8_t pes_header[BMEDIA_PES_HEADER_MAX_SIZE];
#endif
};
BLST_D_HEAD(b_mmt_demux_streams, bmmt_demux_stream);

BDBG_OBJECT_ID(bmmt_demux);
struct bmmt_demux {
    BDBG_OBJECT(bmmt_demux)
    bmmt_demux_config config;
    batom_factory_t factory;
    struct b_mmt_demux_streams streams;
    struct {
        bool valid;
        btlv_ntp_time value;
    } initial_timestamp;
};

void bmmt_demux_config_init(bmmt_demux_config *config)
{
    BKNI_Memset(config, 0, sizeof(*config));
    return;
}

bmmt_demux_t bmmt_demux_create(batom_factory_t factory, const bmmt_demux_config *config)
{
    bmmt_demux_t    demux;
    demux = BKNI_Malloc(sizeof(*demux));
    if(demux==NULL) {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    BDBG_OBJECT_INIT(demux, bmmt_demux);
    demux->config = *config;
    demux->factory = factory;
    demux->initial_timestamp.valid = false;
    BLST_D_INIT(&demux->streams);
    return demux;
}

void bmmt_demux_destroy(bmmt_demux_t demux)
{
    bmmt_demux_stream_t stream;
    BDBG_OBJECT_ASSERT(demux, bmmt_demux);
    while(NULL!=(stream=BLST_D_FIRST(&demux->streams))) {
        bmmt_demux_stream_destroy(demux, stream);
    }
    BDBG_OBJECT_DESTROY(demux, bmmt_demux);
    BKNI_Free(demux);
}

void bmmt_demux_stream_config_init(bmmt_demux_stream_config *config)
{
    BKNI_Memset(config, 0, sizeof(*config));
    return;
}

bmmt_demux_stream_t bmmt_demux_stream_create(bmmt_demux_t demux,  bmmt_demux_stream_config *config)
{
    bmmt_demux_stream_t stream;
    BDBG_OBJECT_ASSERT(demux, bmmt_demux);
    stream = BKNI_Malloc(sizeof(*stream));
    if(stream==NULL) {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_malloc;
    }
    BDBG_OBJECT_INIT(stream, bmmt_demux_stream);
    stream->config = *config;
    stream->demux = demux;
    BMEDIA_PES_INFO_INIT(&stream->time_info.pes_info, stream->config.pes_stream_id);
    stream->frame = batom_accum_create(demux->factory);
    if(stream->frame==NULL) {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_frame;
    }
    stream->data_unit = batom_accum_create(demux->factory);
    if(stream->data_unit ==NULL) {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_data_unit;
    }
    stream->timestamp_queue = bmmt_timestamp_queue_create();
    if(stream->timestamp_queue==NULL) {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_queue;
    }
    BLST_D_INSERT_HEAD(&demux->streams, stream, link);
    return stream;
err_queue:
    batom_accum_destroy(stream->data_unit);
err_data_unit:
    batom_accum_destroy(stream->frame);
err_frame:
    BKNI_Free(stream);
err_malloc:
    return NULL;
}

void bmmt_demux_stream_destroy(bmmt_demux_t demux, bmmt_demux_stream_t stream)
{
    BDBG_OBJECT_ASSERT(stream, bmmt_demux_stream);
    BDBG_ASSERT(demux == stream->demux);
    BDBG_OBJECT_ASSERT(demux, bmmt_demux);
    BLST_D_REMOVE(&demux->streams, stream, link);
    batom_accum_destroy(stream->data_unit);
    batom_accum_destroy(stream->frame);
    bmmt_timestamp_queue_destroy(stream->timestamp_queue);
    BDBG_OBJECT_DESTROY(stream, bmmt_demux_stream);
    BKNI_Free(stream);
    return;
}

int b_mmt_demux_stream_process_payload_h265(bmmt_demux_stream_t stream, const btlv_mpu_header *mpu, btlv_timed_mfu_data *mfu)
{
    int rc;
    static const uint8_t nal_prefix[4]={0x00, 0x00, 0x00, 0x01};
    if(mpu->f_i==0x00) { /* Payload contains one or more complete data units */
        unsigned mfu_length = mfu->length;
        if(mfu_length>sizeof(nal_prefix)) {
            mfu_length -= sizeof(nal_prefix);
            batom_cursor_skip(&mfu->payload, sizeof(nal_prefix));
            batom_accum_add_range(stream->frame, nal_prefix, sizeof(nal_prefix));
        }
        rc = stream->config.copy_payload(stream->config.stream_context, stream->frame, &mfu->payload, mfu_length);
        if(rc!=0) {
            return BDBG_TRACE_ERROR(rc);
        }
        stream->config.stream_data(stream->config.stream_context, stream->frame, &stream->time_info);
        BMEDIA_PES_UNSET_PTS(&stream->time_info.pes_info);
        stream->time_info.mpu_time_valid = false;
        batom_accum_clear(stream->frame);
    } else {
        rc = stream->config.copy_payload(stream->config.stream_context, stream->data_unit, &mfu->payload, mfu->length);
        if(rc!=0) {
            return BDBG_TRACE_ERROR(rc);
        }
        if(mpu->f_i==0x3) { /* Payload contains the last fragment of data unit */
            batom_cursor cursor;
            batom_cursor cursor_end;
            batom_cursor_from_accum(&cursor, stream->data_unit);
            BATOM_CLONE(&cursor_end,&cursor);
            batom_cursor_skip(&cursor_end, batom_accum_len(stream->data_unit));
            batom_cursor_skip(&cursor, sizeof(nal_prefix));
            batom_accum_add_range(stream->frame, nal_prefix, sizeof(nal_prefix));
            if(!batom_accum_append(stream->frame, stream->data_unit, &cursor, &cursor_end)) {
                return BDBG_TRACE_ERROR(BTLV_RESULT_OUT_OF_MEMORY);
            }
            batom_accum_clear(stream->data_unit);
            stream->config.stream_data(stream->config.stream_context, stream->frame, &stream->time_info);
            BMEDIA_PES_UNSET_PTS(&stream->time_info.pes_info);
            stream->time_info.mpu_time_valid = false;
            batom_accum_clear(stream->frame);
        }
    }
    return 0;
}

int b_mmt_demux_stream_process_payload_aac(bmmt_demux_stream_t stream, const btlv_mpu_header *mpu, btlv_timed_mfu_data *mfu)
{
    uint8_t header_bytes[4];
    uint32_t header;
    int rc = 0;

    if(mpu->f_i==0x00) { /* Payload contains one or more complete data units */
        header = B_SET_BITS("syncword", 0x2B7, 23, 13) | B_SET_BITS("audioMuxLengthBytes", mfu->length, 12, 0);
        B_MEDIA_SAVE_UINT32_BE(header_bytes, header);
        batom_accum_add_range(stream->frame, header_bytes+1,sizeof(header_bytes)-1); /* save 3 bytes (24-bits) */
        rc = stream->config.copy_payload(stream->config.stream_context, stream->frame, &mfu->payload, mfu->length);
        if(rc!=0) {
            return BDBG_TRACE_ERROR(rc);
        }
        stream->config.stream_data(stream->config.stream_context, stream->frame, &stream->time_info);
        BMEDIA_PES_UNSET_PTS(&stream->time_info.pes_info);
        stream->time_info.mpu_time_valid = false;
        batom_accum_clear(stream->frame);
    } else {
        rc = stream->config.copy_payload(stream->config.stream_context, stream->data_unit, &mfu->payload, mfu->length);
        if(rc!=0) {
            return BDBG_TRACE_ERROR(rc);
        }
        if(mpu->f_i==0x3) { /* Payload contains the last fragment of data unit */
            size_t length = batom_accum_len(stream->data_unit);
            batom_cursor cursor;
            batom_cursor cursor_end;
            batom_cursor_from_accum(&cursor, stream->data_unit);
            BATOM_CLONE(&cursor_end,&cursor);
            batom_cursor_skip(&cursor_end, length);

            header = B_SET_BITS("syncword", 0x2B7, 23, 13) | B_SET_BITS("audioMuxLengthBytes", length, 12, 0);
            batom_accum_add_range(stream->frame, header_bytes+1,sizeof(header_bytes)-1); /* save 3 bytes (24-bits) */
            if(!batom_accum_append(stream->frame, stream->data_unit, &cursor, &cursor_end)) {
                return BDBG_TRACE_ERROR(BTLV_RESULT_OUT_OF_MEMORY);
            }
            batom_accum_clear(stream->data_unit);
            stream->config.stream_data(stream->config.stream_context, stream->frame, &stream->time_info);
            BMEDIA_PES_UNSET_PTS(&stream->time_info.pes_info);
            stream->time_info.mpu_time_valid = false;
            batom_accum_clear(stream->frame);
        }
    }
    return 0;
}


int bmmt_demux_stream_process_payload(bmmt_demux_t demux, bmmt_demux_stream_t stream, const btlv_mmt_packet_header *header, batom_cursor *payload)
{
    btlv_mpu_header mpu;
    int rc;

    BDBG_OBJECT_ASSERT(stream, bmmt_demux_stream);

    if(btlv_parse_mpu_payload_header(payload,&mpu)!=0) {
        return 0;
    }
    if(mpu.type==BTLV_MPU_TYPE_MFU && mpu.timed) {
        btlv_timed_mfu_data mfu[16];
        unsigned parsed_mfu;
        if(btlv_parse_mpu_payload_mfu(payload, &mpu, mfu, sizeof(mfu)/sizeof(mfu[0]), &parsed_mfu)==0) {
            unsigned i;
            for(i=0;i<parsed_mfu;i++) {
                if(mfu[i].offset==0 ) {
                    btlv_ntp_time presentation_time;

                    BMEDIA_PES_UNSET_PTS(&stream->time_info.pes_info);
                    stream->time_info.mpu_time_valid = false;
                    if(bmmt_timestamp_queue_get_presentation_time(stream->timestamp_queue, mpu.sequence_number, mfu[i].sample_number, &presentation_time)==0) {
                        uint32_t pts;
                        btlv_ntp_time mpu_time_full = btplv_ntp_merge(presentation_time, header->timestamp);

                        BDBG_MSG(("%p: presentation_time: " BDBG_UINT64_FMT " mmt.timestamp:" BDBG_UINT64_FMT "(%#x)", (void *)stream, BDBG_UINT64_ARG(presentation_time), BDBG_UINT64_ARG(mpu_time_full), (unsigned)header->timestamp));
                        if(!demux->initial_timestamp.valid) {
                            btlv_ntp_time _10_min;
                            demux->initial_timestamp.valid = true;
                            _10_min = btplv_ntp_time_init(10 * 60, 0);
                            if(presentation_time > _10_min) {
                                demux->initial_timestamp.value = presentation_time - _10_min;
                            } else {
                                demux->initial_timestamp.value = 0;
                            }
                        }
                        pts = btlv_ntp_45khz(demux->initial_timestamp.value, presentation_time);
                        BMEDIA_PES_SET_PTS(&stream->time_info.pes_info, pts);
                        stream->time_info.mpu_time = btlv_ntp_45khz(demux->initial_timestamp.value, mpu_time_full);
                        stream->time_info.mpu_time_valid = true;
                    }
                }
                switch(stream->config.stream_type) {
                case bmmt_stream_type_h265:
                    rc = b_mmt_demux_stream_process_payload_h265(stream, &mpu, &mfu[i]);
                    if(rc!=0) {
                        return BDBG_TRACE_ERROR(rc);
                    }
                    break;
                case bmmt_stream_type_aac:
                    rc = b_mmt_demux_stream_process_payload_aac(stream, &mpu, &mfu[i]);
                    if(rc!=0) {
                        return BDBG_TRACE_ERROR(rc);
                    }
                    break;
                default:
                    break;
                }
            }
        }
    }

    return 0;
}

static int b_mmt_demux_process_mp_table(bmmt_demux_t demux,batom_cursor *payload)
{
    btlv_mp_table_header mp_header;
    if(btlv_parse_mp_table_header(payload, &mp_header)==0) {
        unsigned asset_no;
        for(asset_no=0;asset_no<mp_header.number_of_assets;asset_no++) {
            btlv_mmt_package_table_asset asset;
            btlv_mmt_package_table_asset_init(&asset);
            BDBG_MSG(("asset_no:%u", asset_no));
            if(btlv_parse_mmt_package_table_asset(payload, &asset)==0) {
                unsigned i;
                bmmt_timestamp_queue timestamp_queue=NULL;
                if(0) batom_cursor_dump(&asset.asset_descriptors.asset_descriptors_bytes,"asset_descriptors");
                for(i=0;i<asset.asset_location.location_count;i++) {
                    bmmt_demux_stream_t stream;
                    for(stream=BLST_D_FIRST(&demux->streams);stream;stream=BLST_D_NEXT(stream,link)) {
                        if(
                           (asset.asset_location.locations[0].location_type==btlv_mmt_general_location_type_id &&  asset.asset_location.locations[0].data.packet_id==stream->config.packet_id) ||
                           (asset.asset_location.locations[0].location_type==btlv_mmt_general_location_type_mmt_ipv6 &&  asset.asset_location.locations[0].data.mmt_ipv6.packet_id==stream->config.packet_id)
                          ) {
                            timestamp_queue = stream->timestamp_queue;
                            break;
                        }
                    }
                    if(timestamp_queue) {
                        break;
                    }
                }
                if(asset.asset_descriptors.asset_descriptors_length) {
                    size_t asset_descriptors_end = batom_cursor_pos(&asset.asset_descriptors.asset_descriptors_bytes) + asset.asset_descriptors.asset_descriptors_length;
                    for(;;) {
                        btlv_mpu_timestamp_descriptor_header timestamp_descriptor_header;
                        btlv_mpu_extended_timestamp_descriptor_header extended_timestamp_descriptor_header;
                        btlv_mpu_extended_timestamp_descriptor_entry extended_timestamp_descriptor_entry[8];
                        unsigned extended_timestamp_descriptor_entry_enries;
                        BDBG_MSG(("asset_descriptors: %u ... %u bytes" , (unsigned)batom_cursor_pos(&asset.asset_descriptors.asset_descriptors_bytes), (unsigned)asset_descriptors_end));
                        if(batom_cursor_pos(&asset.asset_descriptors.asset_descriptors_bytes) + BMEDIA_FIELD_LEN("tag",uint16_t) + BMEDIA_FIELD_LEN("length",uint8_t) >= asset_descriptors_end) {
                            break;
                        }
                        if(btlv_parse_mpu_timestamp_descriptor_header(&asset.asset_descriptors.asset_descriptors_bytes, &timestamp_descriptor_header)==0) {
                            unsigned timestamp;
                            for(timestamp=0;timestamp<timestamp_descriptor_header.N;timestamp++) {
                                btlv_mpu_timestamp_descriptor_entry timestamp_descriptor_entry;
                                if(btlv_parse_mpu_timestamp_descriptor_entry(&asset.asset_descriptors.asset_descriptors_bytes, &timestamp_descriptor_entry)==0) {
                                    if(timestamp_queue) {
                                        bmmt_timestamp_queue_add_timestamp_descriptors(timestamp_queue, &timestamp_descriptor_entry, 1);
                                    }
                                    continue;
                                } else {
                                    break;
                                }
                            }
                        } else if(btlv_parse_mpu_extended_timestamp_descriptor(&asset.asset_descriptors.asset_descriptors_bytes, &extended_timestamp_descriptor_header, 8, extended_timestamp_descriptor_entry, &extended_timestamp_descriptor_entry_enries)==0) {
                            if(0) {
                                unsigned i;
                                for(i=0;i<extended_timestamp_descriptor_entry_enries;i++) {
                                    unsigned j;
                                    for(j=0;j<extended_timestamp_descriptor_entry[i].num_of_au;j++) {
                                        BDBG_MSG(("timestamp[%u,%u] = %u,%u", i, j, (unsigned)extended_timestamp_descriptor_entry[i].au[j].dts_pts_offset, (unsigned)extended_timestamp_descriptor_entry[i].au[j].pts_offset));
                                    }
                                }
                            }
                            if(timestamp_queue) {
                                bmmt_timestamp_queue_add_extended_timestamp_descriptors(timestamp_queue, &extended_timestamp_descriptor_header, extended_timestamp_descriptor_entry, extended_timestamp_descriptor_entry_enries);
                            }
                        } else {
                            uint16_t descriptor_tag;
                            uint8_t descriptor_length;

                            descriptor_tag = batom_cursor_uint16_be(&asset.asset_descriptors.asset_descriptors_bytes);
                            descriptor_length = batom_cursor_byte(&asset.asset_descriptors.asset_descriptors_bytes);
                            if(BATOM_IS_EOF(&asset.asset_descriptors.asset_descriptors_bytes)) {
                                break;
                            }
                            BDBG_MSG(("dropping asset descriptor:%#x %u bytes", (unsigned)descriptor_tag, (unsigned)descriptor_length));
                            if(batom_cursor_skip(&asset.asset_descriptors.asset_descriptors_bytes, descriptor_length) != descriptor_length) {
                                break;
                            }
                        }
                    }
                }
                btlv_mmt_package_table_asset_shutdown(&asset);
                continue;
            } else {
                break;
            }
        }
    }
    return 0;
}

int bmmt_demux_process_signaling_message(bmmt_demux_t demux, batom_cursor *payload, const btlv_signalling_header *signalling_header)
{
    unsigned parsed_messages;
    btlv_signalling_message_data message_data[16];
    unsigned i;
    int rc;

    rc = btlv_parse_signalling_message(payload, signalling_header, message_data, sizeof(message_data)/sizeof(message_data[0]), &parsed_messages);
    if(rc!=0) {
        return BDBG_TRACE_ERROR(rc);
    }
    for(i=0;i<parsed_messages;i++) {
        btlv_package_access_message pa_message[16];
        unsigned pa_messages;
        unsigned j;
        btlv_signalling_message_header message_header;
        rc = btlv_parse_signalling_message_header(&message_data[i].message,&message_header);
        if(rc!=0) {
            return BDBG_TRACE_ERROR(rc);
        }
        if(!BTLV_IS_PA_MESSAGE(message_header.message_id)) {
            continue;
        }
        rc = btlv_parse_package_access_message(&message_data[i].message, pa_message, sizeof(pa_message)/sizeof(pa_message[0]), &pa_messages);
        if(rc!=0) {
            return BDBG_TRACE_ERROR(rc);
        }
        BDBG_MSG(("pa %u",pa_messages));
        for(j=0;j<pa_messages;j++) {
            BDBG_MSG(("mp %#x",pa_message[j].table_id));
            if(pa_message[j].table_id==BTLV_COMPLETE_MP_TABLE) {
                rc = b_mmt_demux_process_mp_table(demux,&pa_message[j].payload);
            }
        }
    }
    return 0;
}
