/******************************************************************************
 * (c) 2002-2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/

#include "bthumbnail_extractor.h"
#include "bmedia_player.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <stdio.h>

BDBG_MODULE(bthumbnail_extractor);

BDBG_OBJECT_ID(bthumbnail_extractor);

struct bthumbnail_extractor {
    BDBG_OBJECT(bthumbnail_extractor)
    bthumbnail_extractor_create_settings create_settings;
    bthumbnail_extractor_settings settings;
    bthumbnail_extractor_status status;
    bmedia_player_t player;
    batom_factory_t factory;
    bfile_buffer_t buffer;
    bfile_buffer_cfg buffer_cfg;
/* TODO: buf[] should be smaller. right now the PlaypumpSegment logic requries that the picture be read in in one shot */
    unsigned char buf[256*1024];
};

struct bthumbnail_stream_p_endcode {
    size_t length;
    uint8_t payload[5];
};
#define MAX_PACKET_SIZE 192

static int bthumbnail_extractor_p_send(bthumbnail_extractor_t handle, unsigned offset, const void *buffer, unsigned size, bmedia_player_pos timestamp);
static void bthumbnail_extractor_p_destroy_resources(bthumbnail_extractor_t handle);
static const struct bthumbnail_stream_p_endcode *bthumbnail_get_endcode(NEXUS_VideoCodec videoCodec);
static int bthumbnail_stream_p_generate_startcode_packet(bthumbnail_extractor_t handle, uint8_t *buffer, unsigned size);
static int bthumbnail_extractor_p_send_raw(bthumbnail_extractor_t handle, const void *buffer, unsigned size);

void bthumbnail_extractor_get_default_create_settings( bthumbnail_extractor_create_settings *p_settings )
{
    BKNI_Memset(p_settings, 0, sizeof(*p_settings));
    p_settings->buffer_size = 200 * 1024;
}

bthumbnail_extractor_t bthumbnail_extractor_create( const bthumbnail_extractor_create_settings *p_settings )
{
    bthumbnail_extractor_t handle;

    handle = BKNI_Malloc(sizeof(*handle));
    if (!handle) {
        BERR_Code rc = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        BSTD_UNUSED(rc);
        return NULL;
    }
    BKNI_Memset(handle, 0, sizeof(*handle));
    BDBG_OBJECT_SET(handle, bthumbnail_extractor);

    handle->create_settings = *p_settings;

    /* default settings */
    handle->settings.videoCodec = NEXUS_VideoCodec_eMpeg2;
    handle->settings.transportType = NEXUS_TransportType_eTs;
    /* if videoCdbSize is too large, there may not be room to get the endcode in.
    if too small, we may not get an I-frame. */
    handle->settings.videoCdbSize = 1024 * 1024; /* 1 MB */

    return handle;
}

void bthumbnail_extractor_destroy( bthumbnail_extractor_t handle )
{
    BDBG_OBJECT_ASSERT(handle, bthumbnail_extractor);
    bthumbnail_extractor_p_destroy_resources(handle);
    BDBG_OBJECT_DESTROY(handle, bthumbnail_extractor);
    BKNI_Free(handle);
}

void bthumbnail_extractor_get_settings( bthumbnail_extractor_t handle, bthumbnail_extractor_settings *p_settings )
{
    BDBG_OBJECT_ASSERT(handle, bthumbnail_extractor);
    *p_settings = handle->settings;
}

static void bthumbnail_extractor_p_destroy_resources(bthumbnail_extractor_t handle)
{
    if (handle->player) {
        bmedia_player_destroy(handle->player);
        handle->player = NULL;
    }
    if (handle->buffer) {
        bfile_buffer_destroy(handle->buffer);
        handle->buffer = NULL;
    }
    if (handle->buffer_cfg.buf) {
        BKNI_Free(handle->buffer_cfg.buf);
        handle->buffer_cfg.buf = NULL;
    }
    if (handle->factory) {
        batom_factory_destroy(handle->factory);
        handle->factory = NULL;
    }
}

int bthumbnail_extractor_set_settings( bthumbnail_extractor_t handle, const bthumbnail_extractor_settings *p_settings )
{
    bmedia_player_config config;
    bmedia_player_decoder_mode decode_mode;
    bmedia_player_stream stream;
    int rc;
    bool autoindex = false;

    BDBG_OBJECT_ASSERT(handle, bthumbnail_extractor);
    if (p_settings != &handle->settings) {
        handle->settings = *p_settings;
    }

    bthumbnail_extractor_p_destroy_resources(handle);

    if (!p_settings->indexfile && !p_settings->datafile) {
        /* this just frees resources */
        return 0;
    }

    switch (p_settings->transportType) {
    case NEXUS_TransportType_eAsf:
    case NEXUS_TransportType_eAvi:
    case NEXUS_TransportType_eMkv:
    case NEXUS_TransportType_eMp4:
        autoindex = true;
        if (!handle->settings.indexfile) {
            handle->settings.indexfile = handle->settings.datafile;
        }
        break;
    case NEXUS_TransportType_eEs:
    case NEXUS_TransportType_eMpeg2Pes:
        /* will attempt no-index feed */
        return 0;
    case NEXUS_TransportType_eTs:
        if (!handle->settings.indexfile) {
            /* will attempt no-index feed */
            return 0;
        }
    default:
        break;
    }

    /* needed for MP4 and MKV */
    handle->factory = batom_factory_create(bkni_alloc, 256);
    if (!handle->factory) {rc = BERR_TRACE(NEXUS_UNKNOWN); goto err_buf;}

    bfile_buffer_default_cfg(&handle->buffer_cfg);
    handle->buffer_cfg.buf = BKNI_Malloc(handle->buffer_cfg.buf_len);
    if (!handle->buffer_cfg.buf) {rc = BERR_TRACE(NEXUS_UNKNOWN); goto err_buf;}
    handle->buffer_cfg.async = false;
    handle->buffer_cfg.fd = handle->settings.indexfile;
    handle->buffer = bfile_buffer_create(handle->factory, &handle->buffer_cfg);
    if (!handle->buffer) {rc = BERR_TRACE(NEXUS_UNKNOWN); goto err_buf;}

    bmedia_player_init_config(&config);
    config.buffer = handle->buffer;
    config.decoder_features.brcm = false;
    config.decoder_features.dqt = false;

    bmedia_player_init_stream(&stream);
    stream.stream.mpeg2ts.packet_size = 188;
    if (p_settings->timestampType != NEXUS_TransportTimestampType_eNone) {
        stream.stream.mpeg2ts.packet_size += 4;
    }
    switch (p_settings->transportType) {
    case NEXUS_TransportType_eAsf: stream.format = bstream_mpeg_type_asf; break;
    case NEXUS_TransportType_eAvi: stream.format = bstream_mpeg_type_avi; break;
    case NEXUS_TransportType_eMkv: stream.format = bstream_mpeg_type_mkv; break;
    case NEXUS_TransportType_eMp4: stream.format = bstream_mpeg_type_mp4; break;
    case NEXUS_TransportType_eTs: stream.format = bstream_mpeg_type_ts; break;
    default: rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_buf;
    }
    stream.master = p_settings->videoPid;
    stream.without_index = !handle->settings.indexfile;

    handle->player = bmedia_player_create(handle->settings.indexfile ? handle->settings.indexfile : p_settings->datafile, &config, &stream);
    if (!handle->player) { rc = BERR_TRACE(NEXUS_UNKNOWN); goto err_player; }

    /* non-dqt, non-brcm reverse will always be I-frame only.
       note, setting to forward mode will result in seek returning other non-I-frames after the I-frame.
       this trips up AVD when doing still decode */
    rc = bmedia_player_set_direction(handle->player, -1, -BMEDIA_TIME_SCALE_BASE, &decode_mode);
    if (rc) {BERR_TRACE(rc); goto err_player;}

    return 0;

err_player:
    if (handle->settings.indexfile && !autoindex) {
        /* try again without the index */
        handle->settings.indexfile = NULL;
        rc = bthumbnail_extractor_set_settings(handle, &handle->settings);
        if (!rc) return 0;
        /* else, fall through */
    }
err_buf:
    bthumbnail_extractor_p_destroy_resources(handle);
    return rc;
}

static int bthumbnail_stream_p_send_raw_endcode(bthumbnail_extractor_t handle)
{
    unsigned char buf[256];
    int rc;
    if (handle->settings.transportType != NEXUS_TransportType_eTs) {
        const struct bthumbnail_stream_p_endcode *endcode;
        endcode = bthumbnail_get_endcode(handle->settings.videoCodec);
        if (!endcode) return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        /* send two to push through RAVE startcode pipeline */
        rc = bthumbnail_extractor_p_send_raw(handle, endcode->payload, endcode->length);
        if (rc) return BERR_TRACE(rc);
        rc = bthumbnail_extractor_p_send_raw(handle, endcode->payload, endcode->length);
        if (rc) return BERR_TRACE(rc);
        /* flush packet through playback packetizer */
        BKNI_Memset(buf, 0, sizeof(buf));
        rc = bthumbnail_extractor_p_send_raw(handle, buf, sizeof(buf));
        if (rc) return BERR_TRACE(rc);
    }
    else {
        unsigned n;
        n = bthumbnail_stream_p_generate_startcode_packet(handle, buf, sizeof(buf));
        /* send two to push through RAVE startcode pipeline */
        rc = bthumbnail_extractor_p_send_raw(handle, buf, n);
        if (rc) return BERR_TRACE(rc);
        rc = bthumbnail_extractor_p_send_raw(handle, buf, n);
        if (rc) return BERR_TRACE(rc);
    }
    return 0;
}

static int bthumbnail_stream_p_send_endcode(bthumbnail_extractor_t handle)
{
    unsigned char buf[256];
    int rc;
    unsigned n;
    BDBG_ASSERT(handle->settings.transportType == NEXUS_TransportType_eTs);
    n = bthumbnail_stream_p_generate_startcode_packet(handle, buf, sizeof(buf));
    /* send two to push through RAVE startcode pipeline */
    rc = bthumbnail_extractor_p_send(handle, 0, buf, n, 0);
    if (rc) return BERR_TRACE(rc);
    rc = bthumbnail_extractor_p_send(handle, 0, buf, n, 0);
    if (rc) return BERR_TRACE(rc);
    return 0;
}

static int bthumbnail_extractor_p_feed_picture_no_index( bthumbnail_extractor_t handle )
{
    int rc;
    unsigned cdbSize = handle->settings.videoCdbSize;

    while (cdbSize >= sizeof(handle->buf) + 4096) {
        rc = handle->settings.datafile->read(handle->settings.datafile, handle->buf, sizeof(handle->buf));
        if (rc > 0) { /* some data */
            BDBG_MSG(("size %d", rc));
            if (bthumbnail_extractor_p_send_raw(handle, handle->buf, rc)) {
                break;
            }
            cdbSize -= rc;
        } else if(rc==0) { /* end of file */
            break;
        }
        else { /* error */
            return BERR_TRACE(rc);
        }
    }

    bthumbnail_stream_p_send_raw_endcode(handle);

    return 0;
}

int bthumbnail_extractor_feed_picture( bthumbnail_extractor_t handle, unsigned timestamp )
{
    off_t last_start = 0;
    int stage = 0;
    int rc;
    unsigned char pad[512];
    bmedia_player_entry entry;
    unsigned first_timestamp = 0;

    BDBG_OBJECT_ASSERT(handle, bthumbnail_extractor);
    BKNI_Memset(&handle->status, 0, sizeof(handle->status));

    if (handle->settings.datafile->seek) {
        handle->settings.datafile->seek(handle->settings.datafile, 0, SEEK_SET);
    }

    if (handle->settings.transportType == NEXUS_TransportType_eEs ||
        handle->settings.transportType == NEXUS_TransportType_eMpeg2Pes ||
        (handle->settings.transportType == NEXUS_TransportType_eTs && !handle->settings.indexfile))
    {
        /* without an index, timestamp is ignored. we try to get the first picture. */
        return bthumbnail_extractor_p_feed_picture_no_index(handle);
    }

    /* read the still */
    BDBG_MSG(("bthumbnail_extractor_get_thumbnail:%#lx time:%u", handle, timestamp));
    rc = bmedia_player_seek(handle->player, timestamp);
    if (rc) {
        bmedia_player_status status;
        bmedia_player_get_status(handle->player, &status);
        if (timestamp != status.bounds.first) {
            BDBG_MSG(("reseek from %d to %d", timestamp, status.bounds.first));
            rc = bmedia_player_seek(handle->player, status.bounds.first);
        }
        if (rc) {
            BERR_TRACE(rc);
            goto done;
        }
    }

    while (1) {
        if (bmedia_player_next(handle->player, &entry)) {
            /* may have hit BOF */
            break;
        }
        BDBG_MSG(("bmedia_player_next %u, %u, %u, %d, %u:%u", (unsigned)entry.start, (unsigned)entry.length, entry.embedded, entry.timestamp, entry.type, entry.content));
        if (!entry.embedded && entry.start < last_start) {
            /* this works before bmedia is set in reverse mode */
            break;
        }

        if (entry.embedded) {
            rc = bthumbnail_extractor_p_send(handle, entry.start, entry.embedded, entry.length, 0);
            if (rc) break;
        }
        else if (entry.atom) {
            batom_cursor cursor;
            ssize_t n;
            unsigned total_read = 0;

            batom_cursor_from_atom(&cursor, entry.atom);
            while (total_read < entry.length) {
                n = entry.length - total_read;
                if (n > (int)sizeof(handle->buf)) {
                    n = sizeof(handle->buf);
                }
                /* TODO: avoid the memcpy */
                n = batom_cursor_copy(&cursor, handle->buf, n);
                rc = bthumbnail_extractor_p_send(handle, entry.start, handle->buf, n, 0);
                if (rc) break;
                total_read += n;
            }
            batom_release(entry.atom);

            /* one atom = one picture */
            stage = 2;
        }
        else {
            ssize_t n;
            unsigned total_read = 0;

            if (handle->settings.transportType == NEXUS_TransportType_eAsf || handle->settings.transportType == NEXUS_TransportType_eAvi) {
                if (handle->settings.transportType != NEXUS_TransportType_eAvi) {
                    if (entry.content != bmedia_player_entry_content_header) {
                        stage = 2;
                    }
                }
                else { /* avi */
                    if (entry.content != bmedia_player_entry_content_header) {
                        stage++;
                        /* we need two payloads from avi player after the header.
                           the first payload will contain the first sequence header and first I-frame in the stream.
                           timestamp==-1 must be fed back into avi player in order to extract just the sequence header.
                           the next payload will contain the I-frame we seeked to */
                    }
                    if (entry.timestamp==0) {
                        /* in the corner case where the timestamp of the I-frame we want is actually at 0, we must set
                           the timestamp to some nonzero value in order to get the I-frame.
                           see nexus_playpump_media.c:b_pump_demux_descriptor */
                        entry.timestamp = 1;
                    }
                }
            }
            else {
                /* transport - the first timestamp we see is the thumbnail. the next timestamp means we've finished that picture. */
                if (!first_timestamp) {
                    first_timestamp = entry.timestamp;
                }
                else if (entry.timestamp != first_timestamp) {
                    break;
                }
            }

            handle->settings.datafile->seek(handle->settings.datafile, entry.start, SEEK_SET);

            while (total_read < entry.length) {
                n = entry.length - total_read;
                if (n > (int)sizeof(handle->buf)) {
                    n = sizeof(handle->buf);
                }
                n = handle->settings.datafile->read(handle->settings.datafile, handle->buf, n);
                if (n <= 0) { /* error or end of file */
                    if (timestamp) {
                        goto try_zero;
                    }
                    rc = BERR_TRACE(NEXUS_UNKNOWN);
                    goto done;
                }
                rc = bthumbnail_extractor_p_send(handle, entry.start, handle->buf, n, entry.timestamp);
                if (rc) break;
                total_read += n;
            }
            last_start = entry.start;
        }
        if (stage == 2) break;
    };

    handle->status.timestamp = entry.timestamp;

    if (handle->settings.transportType == NEXUS_TransportType_eMkv || handle->settings.transportType == NEXUS_TransportType_eMp4) {
        bmedia_pes_info info;
        unsigned char packet[MAX_PACKET_SIZE];
        unsigned n;
        const struct bthumbnail_stream_p_endcode *endcode;

        /* create EOS PES packet */
        bmedia_pes_info_init(&info, 0xe0);
        n = bmedia_pes_header_init(packet, MAX_PACKET_SIZE, &info);

        /* add the terminating startcode */
        endcode = bthumbnail_get_endcode(handle->settings.videoCodec);
        if(endcode) {
            BKNI_Memcpy(packet+n, endcode->payload, endcode->length);
            n+= endcode->length;
        }
        while (n < 36) {
            packet[n++] = 0;
        }
        BDBG_ASSERT(n == 36);
        bthumbnail_extractor_p_send(handle, 0, packet, n, 0);

        BKNI_Memset(pad, 0, sizeof(pad));
        bthumbnail_extractor_p_send(handle, 0, pad, sizeof(pad), 0);
    }
    else if (handle->settings.transportType == NEXUS_TransportType_eAsf || handle->settings.transportType == NEXUS_TransportType_eAvi) {
        unsigned char fill = 0;

        /* now add a one byte payload segment so that mediaframe generates an end-of-sequence */
        bthumbnail_extractor_p_send(handle, 0, &fill, sizeof(fill), 0);
    }
    else {
        bthumbnail_stream_p_send_endcode(handle);
    }

    rc = 0;

done:
    return rc;

try_zero:
    /* the stream data may not exist for index entry. try the first picture (with one level of recursion). */
    return bthumbnail_extractor_feed_picture(handle, 0);
}

void bthumbnail_extractor_get_status(bthumbnail_extractor_t handle, bthumbnail_extractor_status *status)
{
    BKNI_Memcpy(status, &handle->status, sizeof(handle->status));
}

int bthumbnail_extractor_start_playpump(bthumbnail_extractor_t handle)
{
    NEXUS_PlaypumpSettings playpumpSettings;
    int rc;

    if (!handle->settings.playpump) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    NEXUS_Playpump_GetSettings(handle->settings.playpump, &playpumpSettings);
    switch (handle->settings.transportType) {
    case NEXUS_TransportType_eMkv:
    case NEXUS_TransportType_eMp4:
        playpumpSettings.transportType = NEXUS_TransportType_eMpeg2Pes;
        break;
    default:
        playpumpSettings.transportType = handle->settings.transportType;
        break;
    }
    if (handle->player) {
        playpumpSettings.mode = NEXUS_PlaypumpMode_eSegment;
    }
    else {
        playpumpSettings.mode = NEXUS_PlaypumpMode_eFifo;
    }
    playpumpSettings.timestamp.type = handle->settings.timestampType;
    rc = NEXUS_Playpump_SetSettings(handle->settings.playpump, &playpumpSettings);
    if (rc) return BERR_TRACE(rc);

    rc = NEXUS_Playpump_Start(handle->settings.playpump);
    if (rc) return BERR_TRACE(rc);

    if (handle->player) {
        /* We must set the playRate after Start for it to be properly applied.
        A non-normal playrate is needed to force media framework into key-frame only mode. Some ASF/AVI files have indexes which
        point to more than just a single key-frame, so code to filter down to a single key-frame has to be handled inside Playpump. */
        NEXUS_Playpump_GetSettings(handle->settings.playpump, &playpumpSettings);
        playpumpSettings.playRate = NEXUS_NORMAL_PLAY_SPEED+1;
        rc = NEXUS_Playpump_SetSettings(handle->settings.playpump, &playpumpSettings);
        if (rc) return BERR_TRACE(rc);
    }

    return 0;
}

void bthumbnail_extractor_stop_playpump(bthumbnail_extractor_t handle)
{
    NEXUS_Playpump_Stop(handle->settings.playpump);
}

static int bthumbnail_extractor_p_send_raw(bthumbnail_extractor_t handle, const void *buffer, unsigned size)
{
    while (size) {
        size_t playback_size;
        void *playback_buffer;
        int rc;

        rc = NEXUS_Playpump_GetBuffer(handle->settings.playpump, &playback_buffer, &playback_size);
        if (rc) return BERR_TRACE(rc);

        if (!playback_size) {
            /* we can't send any more */
            return 0;
        }

        if (playback_size > size) {
            playback_size = size;
        }
        BKNI_Memcpy(playback_buffer, buffer, playback_size);

        rc = NEXUS_Playpump_WriteComplete(handle->settings.playpump, 0, playback_size);
        if (rc) return BERR_TRACE(rc);

        buffer = (uint8_t*)buffer + playback_size;
        size -= playback_size;
    }

    return 0;
}

static int bthumbnail_extractor_p_send(bthumbnail_extractor_t handle, unsigned offset, const void *buffer, unsigned size, bmedia_player_pos timestamp)
{
    NEXUS_PlaypumpSegment segment;
    int rc;

    BKNI_Memset(&segment, 0, sizeof(segment));
    segment.signature = NEXUS_PLAYPUMP_SEGMENT_SIGNATURE;
    segment.length = sizeof(NEXUS_PlaypumpSegment) + size;
    segment.offset = offset;

    segment.timestamp = timestamp;
    segment.timestamp_delta[0].stream_id = handle->settings.videoPid;
    segment.timestamp_delta[0].timestamp_delta = 0;

    rc = bthumbnail_extractor_p_send_raw(handle, &segment, sizeof(segment));
    if (rc) return BERR_TRACE(rc);

    rc = bthumbnail_extractor_p_send_raw(handle, buffer, size);
    if (rc) return BERR_TRACE(rc);

    BDBG_MSG(("send size %u, offset %u, timestamp %d", size, offset, timestamp));
    return 0;
}

static const struct bthumbnail_stream_p_endcode *bthumbnail_get_endcode(NEXUS_VideoCodec videoCodec)
{
    static const struct bthumbnail_stream_p_endcode
        mpeg2 = {4, {0x00,0x00,0x01, 0xB7}},
        h264 =  {4, {0x00,0x00,0x01, 0x0A}},
        h265 =  {5, {0x00,0x00,0x01, 0x4A, 0x01}},
        vc1 =   {4, {0x00,0x00,0x01, 0x0A}},
        mpeg4part2 =   {4, {0x00,0x00,0x01, 0xB1}};

    switch (videoCodec){
    case NEXUS_VideoCodec_eMpeg2:
        return &mpeg2;
    case NEXUS_VideoCodec_eH264:
        return &h264;
    case NEXUS_VideoCodec_eH265:
        return &h265;
    case NEXUS_VideoCodec_eVc1:
    case NEXUS_VideoCodec_eVc1SimpleMain:
        return &vc1;
    case NEXUS_VideoCodec_eMpeg4Part2:
    case NEXUS_VideoCodec_eDivx311:
        return &mpeg4part2;
    default:
        BDBG_ERR(("unsupported video codec %d", videoCodec));
        return &h264;
    }
}

static int bthumbnail_stream_p_generate_startcode_packet(bthumbnail_extractor_t handle, uint8_t *buffer, unsigned size)
{
    unsigned offset = 0;
    const struct bthumbnail_stream_p_endcode *endcode;

    BSTD_UNUSED(size);

    if (handle->settings.timestampType != NEXUS_TransportTimestampType_eNone) {
        offset = 4;
    }
    buffer[0+offset] = 0x47;   /* SYNC BYTE */
    buffer[1+offset] = (handle->settings.videoPid >> 8) & 0x1f;
    buffer[2+offset] = handle->settings.videoPid & 0xff;  /* PID */
    buffer[3+offset] = 0x30; /* not scrambled, adaptation_field then payload, 0 continuity counter */
    endcode = bthumbnail_get_endcode(handle->settings.videoCodec);
    if(endcode) {
        buffer[4+offset] = 188-(5+endcode->length); /* leave only 'endcode->length' bytes of payload */
        BKNI_Memset(&buffer[5+offset], 0, 188-(5+endcode->length)); /* zero out adaptation field */
        BKNI_Memcpy(&buffer[188+offset-endcode->length], endcode->payload, endcode->length);
    }

    return 188+offset;
}
