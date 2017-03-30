/***************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description:
 *
 **************************************************************************/
#include "nexus_playback_module.h"
#include "nexus_playback_impl.h"
#include "biobits.h"
#include "nexus_platform_features.h"

BDBG_MODULE(nexus_playback_media);

#define BDBG_MSG_FLOW(X) /* BDBG_MSG(X) */

static void
b_play_media_send_eos(NEXUS_PlaybackHandle p);

void
b_play_suspend_timer(void *playback)
{
    NEXUS_PlaybackHandle p = playback;
    BDBG_ASSERT(p->state.media.nest_count==0);
    p->state.media.nest_timer = NULL;
    if (p->state.simpleDecoderSuspend.pid) {
        /* recheck */
        if (!NEXUS_Playback_P_CheckSimpleDecoderSuspended(p->state.simpleDecoderSuspend.pid)) {
            p->state.simpleDecoderSuspend.pid = NULL;
        }
    }
    if (b_play_control(p, eControlDataIO)) {
        p->state.io_size = B_MEDIA_NEST_MAGIC;
        BDBG_MSG(("b_play_suspend_timer: %#lx interrupted",(unsigned long)p));
        return;
    }
    BDBG_MSG(("b_play_suspend_timer: %#lx continue",(unsigned long)p));
    b_play_next_frame(p);
    return;
}

static void
b_play_handle_player_error(NEXUS_PlaybackHandle p, bool endofstream /* true of endofstream, false if file error */)
{

    BDBG_MSG(("b_play_handle_player_error: %#lx %s", (unsigned long)p, endofstream?"End Of Stream":"File Error"));
    if(!endofstream) {
        BDBG_MSG(("b_play_handle_player_error: %#lx forcing %s", (unsigned long)p, (p->params.playErrorHandling==NEXUS_PlaybackErrorHandlingMode_eAbort)?"abort":"EOF"));

        p->state.index_error_cnt++;
        NEXUS_TaskCallback_Fire(p->errorCallback);

        if (p->params.playErrorHandling == NEXUS_PlaybackErrorHandlingMode_eAbort) {
            NEXUS_Playback_P_AbortPlayback(p);
            return;
        }
    }
    /* If we are doing normal play speed or slower, then we should wait.
    This code won't work for a decode skip mode + slow motion which is slower than normal play, but that's unlikely. */
    if (p->params.timeshifting && (p->state.trickmode_params.rate > 0 && p->state.trickmode_params.rate <= NEXUS_NORMAL_PLAY_SPEED) && p->recordProgressCnt) {
        bmedia_player_pos pos;
        bmedia_player_status status;

        b_play_update_location(p);
        bmedia_player_get_status(p->media_player, &status);
        pos = b_play_correct_position(p, &status);

        /* only go into eWaitingRecord state if we are certain about the current postion and we know
        we can reliably start waiting. */
        if ( (p->state.validPts || !b_play_has_connected_decoders(p)) && pos >= status.bounds.first) {
            BDBG_MSG(("index->wait_for_record"));
            p->state.read_size = 0; /* this is used as a flag that we are waiting for index */
            p->state.state = eWaitingRecord;
            if (!p->state.timer) {
                p->state.timer = NEXUS_ScheduleTimer(B_FRAME_DISPLAY_TIME * 5, b_play_timer, p);
            }
            return;
        }
    }
    BDBG_MSG(("b_play_handle_player_error-> wait_for_drain"));
    b_play_media_send_eos(p);
    return;
}

static void
b_play_media_next_frame(NEXUS_PlaybackHandle p)
{
    int rc;
    off_t offset;

    BDBG_ASSERT(p->media_player);
    if (b_play_control(p, eControlFrame)) {
        return ;
    }
    if(p->state.media.nest_count>8 || p->state.simpleDecoderSuspend.pid) {
        if(p->state.media.nest_timer==NULL) {
            /* schedule a call-out to either unwind a stack or wait for simpleDecoder to be resumed */
            p->state.media.nest_timer = NEXUS_ScheduleTimer(p->state.simpleDecoderSuspend.pid ? 100 : 0, b_play_suspend_timer, p);
            BDBG_MSG(("b_play_media_next_frame: %#lx nest:%u schedule timer %#lx",(unsigned long)p, p->state.media.nest_count, (unsigned long)p->state.media.nest_timer));
            if(p->state.media.nest_timer) {
                p->state.state = eWaitingIo; /* Reuse eWaitingIo state */
                return;
            }
        } else {
            BDBG_WRN(("b_play_media_next_frame: %#lx detected fork in control flow",(unsigned long)p));
            return;
        }
    }
    p->state.media.nest_count++;
    if(p->state.index_type == NEXUS_PlaybackMpeg2TsIndexType_eNav && p->state.navTrailingMode) {
        BKNI_Memset(&p->state.media.entry,0, sizeof(p->state.media.entry));
        p->state.media.entry.type = bmedia_player_entry_type_end_of_stream;
        rc = 0;
    } else {
        rc = bmedia_player_next(p->media_player, &p->state.media.entry);
    }
    if (rc!=0) {
        b_play_handle_player_error(p, false);
        goto done;
    }
    if(p->state.media.entry.type == bmedia_player_entry_type_end_of_stream) {
        if(p->state.index_type == NEXUS_PlaybackMpeg2TsIndexType_eNav && !p->state.navTrailingMode &&
           !p->params.timeshifting && p->state.frame.valid &&
           (p->state.trickmode_params.rate > 0 && p->state.trickmode_params.rate <= NEXUS_NORMAL_PLAY_SPEED)) {
            off_t first, last;

            rc = p->file->file.data->bounds(p->file->file.data, &first, &last);
            if(rc==0 && last > p->state.frame.offset + p->state.frame.size) {
                NEXUS_PlaypumpStatus playpumpStatus;
                p->state.navTrailingMode = true;
                p->state.media.entry.type=bmedia_player_entry_type_file;
                p->state.media.entry.start = p->state.frame.offset + p->state.frame.size;
                p->state.media.entry.length = last - (p->state.frame.offset + p->state.frame.size);
                if(NEXUS_Playpump_GetStatus(p->params.playpump, &playpumpStatus) == NEXUS_SUCCESS && p->state.media.entry.length > playpumpStatus.fifoSize) {
                    p->state.media.entry.length = playpumpStatus.fifoSize;
                }
                BDBG_MSG(("b_play_media_next_frame: %p sending trailing %u bytes",(void *)p, (unsigned)p->state.media.entry.length ));
            }
        }
    }
    if(p->state.media.entry.type == bmedia_player_entry_type_async) {
        p->state.state = eWaitingIo;
        goto done;
    }
    if(p->state.media.entry.type==bmedia_player_entry_type_file) {
       off_t seek_result;
       offset = p->state.media.entry.start;
       p->state.frame.valid = true;
       p->state.frame.offset = offset;
#if DIRECT_IO_SUPPORT
        p->state.io.file_behind = offset - B_IO_ALIGN_TRUNC(offset);
        p->state.io.next_data = 0;
        BDBG_MSG_FLOW((">>>>>> seek to %lld (aligned=%lld + pre=%d)",
            (uint64_t)offset,
            (uint64_t)B_IO_ALIGN_TRUNC(offset), p->state.io.file_behind));
        offset = B_IO_ALIGN_TRUNC(offset);
#endif
        p->state.io.last_file_offset = offset;
        seek_result = p->file->file.data->seek(p->file->file.data, offset, SEEK_SET);
        if (offset != seek_result) {
            BDBG_ERR(("seek wasn't able to find mpeg data"));
            BDBG_MSG(("b_play_next_media_frame -> wait_for_drain"));
            b_play_media_send_eos(p);
            goto done;
        }
    }

    p->state.frame.cur = 0;
    p->state.frame.size = p->state.media.entry.length;
    BDBG_ASSERT((int)p->state.frame.size  >=0);
    b_play_media_send_meta(p);
done:
    p->state.media.nest_count--;
    return;
}

void
bplay_p_clear_buffer(NEXUS_PlaybackHandle p)
{
    BSTD_UNUSED(p);
#if NEXUS_HAS_SECURITY
    if(p->params.playpumpSettings.securityContext &&  p->state.media.buffer) {
        /* if there is decryption [in place] and file buffer used, it should be cleared on transition to prevent double decryption */
        bfile_buffer_clear(p->state.media.buffer);
    }
#endif
    return;
}


NEXUS_Error
b_play_media_handle_loop_condition(NEXUS_PlaybackHandle p, bool is_beginning, NEXUS_PlaybackLoopMode loopmode, const bmedia_player_bounds *bounds)
{
    int rc;
    int pos;

    BDBG_ASSERT(p->media_player);
    switch(loopmode) {
    case NEXUS_PlaybackLoopMode_eLoop:
        if (is_beginning) {
            BDBG_MSG(("loop to end (index %ld)", bounds->last));
            b_play_flush(p);
            rc = bmedia_player_seek(p->media_player, bounds->last);
            if (rc!=0) {
                BDBG_WRN(("b_play_media_handle_loop_condition: bmedia_player_seek failed %d", rc));
                rc = bmedia_player_seek(p->media_player, bounds->first/2 + bounds->last/2);
            }
        } else {
            BDBG_MSG(("loop to beginning"));
            b_play_flush(p);
            rc = bmedia_player_seek(p->media_player, bounds->first);
            if (rc!=0) {
                BDBG_WRN(("b_play_media_handle_loop_condition: bmedia_player_seek failed %d", rc));
                rc = bmedia_player_seek(p->media_player, bounds->first/2 + bounds->last/2);
            }
        }
        bplay_p_clear_buffer(p);
        break;
    case NEXUS_PlaybackLoopMode_ePlay:
        if (is_beginning) {
            /* if the beginning has been trimmed, we should gap so that we avoid getting chopped again by bfile_fifo. */
            pos = (bounds->first == 0) ? bounds->first : bounds->first + p->params.timeshiftingSettings.beginningOfStreamGap;
        } else {
            /* play at EOF requires a gap for record, otherwise, there's really nothing we can do. */
            if (!p->params.timeshifting) {
                BDBG_WRN(("endOfStreamAction == NEXUS_PlaybackLoopMode_ePlay without timeshifting has been converted to ePause"));
                bplay_p_pause(p);
                break;
            } else {
                /* The recommended endOfStream action for timeshifting is to switch to live decode. This can only be done
                by the application. This code provides a gap to reduce stuttering. */
                if (bounds->last >= p->params.timeshiftingSettings.endOfStreamGap) {
                    pos = bounds->last - p->params.timeshiftingSettings.endOfStreamGap;
                }
                else {
                    pos = 0;
                }
            }
        }
        BDBG_MSG(("seek pos %u in %u...%u", (unsigned)pos, (unsigned)bounds->first, (unsigned)bounds->last));

        bplay_p_clear_buffer(p);
        bplay_p_play_from(p, pos);
        break;
    default:
    case NEXUS_PlaybackLoopMode_ePause:
        bplay_p_pause(p);
        break;
    }
    return NEXUS_SUCCESS;
}

/* this function called when it's time to feed new frame into the playback buffer,
it should be called without pending I/O */
void
b_play_next_frame(NEXUS_PlaybackHandle p)
{
    p->state.decoder_flushed = false; /* clear flushed state */
    b_play_media_next_frame(p);
    return;
}

static void
b_play_media_pts_timer(void *playback)
{
    NEXUS_PlaybackHandle p = playback;
    p->state.media.pts_timer = NEXUS_ScheduleTimer(BMEDIA_UPDATE_POSITION_INTERVAL, b_play_media_pts_timer, p);
    b_play_update_location(p);
    return;
}

void
b_play_media_time_test(void)
{
#if 0
    uint32_t time, pts;
    bmedia_dword delta;

    delta = bmedia_pts2time((uint32_t)(BMEDIA_PTS_MODULO-1), BMEDIA_TIME_SCALE_BASE);
    pts = bmedia_time2pts(delta, BMEDIA_TIME_SCALE_BASE);
    time = bmedia_pts2time(pts, BMEDIA_TIME_SCALE_BASE);
    BDBG_MSG(("modulo_test %u %u %u", delta, pts, time));
    BDBG_ASSERT(time == delta);

    pts = bmedia_time2pts(delta+100, BMEDIA_TIME_SCALE_BASE);
    time = bmedia_pts2time(pts, BMEDIA_TIME_SCALE_BASE);
    BDBG_MSG(("modulo_test %u %u %u", delta, pts, time));
    BDBG_ASSERT(time == 99);

    pts = bmedia_time2pts(delta, -BMEDIA_TIME_SCALE_BASE);
    time = bmedia_pts2time(pts, -BMEDIA_TIME_SCALE_BASE);
    BDBG_MSG(("modulo_test %u %u %u", delta, pts, time));
    BDBG_ASSERT(time == delta);

    pts = bmedia_time2pts(delta+100, -BMEDIA_TIME_SCALE_BASE);
    time = bmedia_pts2time(pts, -BMEDIA_TIME_SCALE_BASE);
    BDBG_MSG(("modulo_test %u %u %u", delta, pts, time));
    BDBG_ASSERT(time == 99);

    pts = bmedia_time2pts(1000, BMEDIA_TIME_SCALE_BASE);
    time = bmedia_pts2time(pts, BMEDIA_TIME_SCALE_BASE);
    BDBG_MSG(("time_test: %u %u", (unsigned)pts, (unsigned)time));
    BDBG_ASSERT(time == 1000);
    pts = bmedia_time2pts(1000, -BMEDIA_TIME_SCALE_BASE);
    time = bmedia_pts2time(pts, -BMEDIA_TIME_SCALE_BASE);
    BDBG_MSG(("time_test: %u %u", (unsigned)pts, (unsigned)time));
    BDBG_ASSERT(time == 1000);

    pts = bmedia_time2pts(337023, BMEDIA_TIME_SCALE_BASE);
    time = bmedia_pts2time(pts, BMEDIA_TIME_SCALE_BASE);
    BDBG_MSG(("time_test: %u %u", (unsigned)pts, (unsigned)time));
    BDBG_ASSERT(time == 337023);

    pts = bmedia_time2pts(337023, -BMEDIA_TIME_SCALE_BASE);
    time = bmedia_pts2time(pts, -BMEDIA_TIME_SCALE_BASE);
    BDBG_MSG(("time_test: %u %u", (unsigned)pts, (unsigned)time));
    BDBG_ASSERT(time == 337023);

    pts = bmedia_time2pts(2*delta+337023, BMEDIA_TIME_SCALE_BASE);
    time = bmedia_pts2time(pts, BMEDIA_TIME_SCALE_BASE);
    BDBG_MSG(("time_test: %u %u %u", (unsigned)pts, (unsigned)time, 337023+2*delta));
    BDBG_ASSERT(337023-time<10);

    pts = bmedia_time2pts(2*delta+337023, BMEDIA_TIME_SCALE_BASE);
    time = bmedia_pts2time(pts, BMEDIA_TIME_SCALE_BASE);
    BDBG_MSG(("time_test: %u %u %u", (unsigned)pts, (unsigned)time, 337023+2*delta));
    BDBG_ASSERT(337023-time<10);


    return;
#endif
}

static void
b_play_media_async_atom_ready(void *cntx, bmedia_player_entry *entry)
{
    NEXUS_PlaybackHandle p = cntx;

    p->state.frame.cur = 0;
    p->state.media.entry = *entry;
    p->state.frame.size = p->state.media.entry.length;
    BDBG_ASSERT((int)p->state.frame.size  >=0);
    if (b_play_control(p, eControlDataIO)) {
        p->state.io_size = B_MEDIA_ATOM_MAGIC;
        goto done;
    }

    if (p->state.state != eWaitingIo) {
        BDBG_WRN(("b_play_media_async_atom_ready got unexpected data"));
        goto done;
    }

    b_play_media_send_meta(p);

done:
    return;
}

/* this function is called when we want to send generated atom (scatteg/gather packet) */
static void
b_play_media_send_atom(NEXUS_PlaybackHandle p)
{
    NEXUS_Error rc;
    size_t size;
    void *vaddr;

    BDBG_ASSERT(p->state.media.entry.atom);

    BDBG_ASSERT(p->params.playpump);

    for(;;) {
        if (NEXUS_Playpump_GetBuffer(p->params.playpump, &vaddr, &size)!=NEXUS_SUCCESS || size == 0) {
            goto keep_waiting;
        }
        size = batom_cursor_copy(&p->state.media.cursor, vaddr, size);
        BDBG_MSG_FLOW(("sending %d out of %d bytes left in packet",
            size, p->state.packet.size-p->state.packet.cur));

        /* Can't be in eWaitingPlayback or we risk a deadlock from playpump callback. */
        p->state.state = eTransition;
        b_play_capture_write(p, vaddr, size);
        rc = NEXUS_Playpump_ReadComplete(p->params.playpump, 0, size);
        if (rc!=NEXUS_SUCCESS) {
            rc = BERR_TRACE(rc);
            BDBG_ERR(("b_play_media_send_atom NEXUS_Playpump_ReadComplete error %#x, playback aborted", (unsigned)rc));
            p->state.state = eAborted;
            NEXUS_TaskCallback_Fire(p->errorCallback);
            return;
        }

        p->state.packet.cur+=size;
        if (p->state.packet.cur >= p->state.packet.size) {
            batom_release(p->state.media.entry.atom);
            p->state.media.entry.atom = NULL;
            /* we're done sending the packet, so go get the next frame */
            b_play_next_frame(p);
            return;
        }
        /* keep writing data */
    }
    return;
keep_waiting:
    p->state.state = eWaitingPlayback;
    p->state.data_source = b_play_media_send_atom; /* schedule to call us when space is avaliable */
    return;
}



void
b_play_media_send_meta(NEXUS_PlaybackHandle p)
{
    NEXUS_Error rc;
    size_t size;
    void *vaddr;
    NEXUS_PlaypumpSegment segment;

    BDBG_ASSERT(p->media_player);
    BDBG_ASSERT(p->params.playpump);

    /* handle no data cases prior to sending segment */
    switch(p->state.media.entry.type) {
    case bmedia_player_entry_type_atom:
    case bmedia_player_entry_type_embedded:
    case bmedia_player_entry_type_file:
        if(p->state.media.entry.length!=0) {
            break;
        }
        /* keep going */
    case bmedia_player_entry_type_noop:
    case bmedia_player_entry_type_no_data:
        b_play_media_next_frame(p); /* keep on reading  */
        return;
    case bmedia_player_entry_type_end_of_stream:
        b_play_handle_player_error(p, true);
        return;
    case bmedia_player_entry_type_error:
        b_play_handle_player_error(p, false);
        return;
    case bmedia_player_entry_type_async:
        BDBG_ASSERT(0); /* keep going */
    }

    if(p->state.media.segmented) {
        for(;;) {
            if (NEXUS_Playpump_GetBuffer(p->params.playpump, &vaddr, &size)!=NEXUS_SUCCESS || size == 0) {
                goto keep_waiting;
            }
            if (vaddr > (void *)p->state.media.last_segment_hdr) {
                /* skip buffer */
                BDBG_MSG(("b_play_media_send_meta: skip %#lx %lu", (unsigned long)vaddr, (unsigned long)size));
                rc = NEXUS_Playpump_ReadComplete(p->params.playpump, size, 0);
                if (rc!=NEXUS_SUCCESS) {
                    BDBG_ERR(("b_play_media_send_meta: NEXUS_Playpump_ReadComplete error %#x, playback aborted", (unsigned)rc));
                    p->state.state = eAborted;
                    NEXUS_TaskCallback_Fire(p->errorCallback);
                    return;
                }
                continue;
            }
            if(size<sizeof(segment)) {
                goto keep_waiting;
            }
            BKNI_Memset(&segment, 0, sizeof(segment));
            segment.length = p->state.media.entry.length + sizeof(segment);
            segment.offset = p->state.media.entry.start;
            segment.timestamp = p->state.media.entry.timestamp;
            segment.signature = NEXUS_PLAYPUMP_SEGMENT_SIGNATURE;
            if(p->state.media.entry.entry){
                unsigned i;
                /* Video timestamp is always on slot 0*/
                segment.timestamp = p->state.media.entry.entry->resync[0].timestamp;
                segment.timestamp_delta[0].stream_id = p->state.media.entry.entry->resync[0].stream_id;
                /* extra  slots used to pass timestamp */
                for(i=1; i<sizeof(segment.timestamp_delta)/sizeof(segment.timestamp_delta[0]); i++){
                    segment.timestamp_delta[i].stream_id = p->state.media.entry.entry->resync[i].stream_id;
                    segment.timestamp_delta[i].timestamp_delta = p->state.media.entry.entry->resync[i].timestamp - segment.timestamp;
                }
            }

            BKNI_Memcpy(vaddr, &segment, sizeof(segment)); /* vaddr might be not alligned, therefore use temporary buffer */
            BDBG_MSG_FLOW(("segment at %#lx %#lx %lu(%lu)", (unsigned long)vaddr, (unsigned long)segment.signature, (unsigned long)segment.offset, (unsigned long)segment.length));
            /* Can't be in eWaitingPlayback or we risk a deadlock from playpump callback. */
            p->state.state = eTransition;
            rc = NEXUS_Playpump_ReadComplete(p->params.playpump, 0, sizeof(segment));
            if (rc!=NEXUS_SUCCESS) {
                BDBG_ERR(("b_play_media_send_meta: NEXUS_Playpump_ReadComplete error %#x, playback aborted", (unsigned)rc));
                p->state.state = eAborted;
                NEXUS_TaskCallback_Fire(p->errorCallback);
                return;
            }
            break;
        }
    }
    switch(p->state.media.entry.type) {
    case bmedia_player_entry_type_embedded:
        BDBG_ASSERT(p->state.media.entry.embedded);
        p->state.packet.size = p->state.media.entry.length;
        p->state.packet.cur = 0;
        p->state.packet.buf = p->state.media.entry.embedded;
/* Also see define in nexus/modules/transport/.. */
#if NEXUS_NUM_DMA_CHANNELS && NEXUS_HAS_SECURITY
#define NEXUS_ENCRYPTED_DVR_WITH_M2M 1
#endif
#if NEXUS_ENCRYPTED_DVR_WITH_M2M
        if( p->params.playpumpSettings.securityContext && p->params.playpumpSettings.transportType==NEXUS_TransportType_eTs && (p->state.packet.size==188 || p->state.packet.size == 192) ) {
            /* if playing back encrypted data, we need to mark inserted packets as encrypted and then playpump would see it,
             * bypass decryption, and then clear encryption marking. Life is weird. */
            uint8_t timestampoffset = p->state.packet.size==188?0:4;
            ((uint8_t *)p->state.packet.buf)[3 + timestampoffset] = B_GET_BITS(((uint8_t *)p->state.packet.buf)[3 + timestampoffset],5,0) | B_SET_BITS( transport_scrambling_control, 0x03, 7, 6); /* set adaptation field */
        }
#endif
        b_play_send_packet(p);
        break;
    case bmedia_player_entry_type_atom:
        BDBG_ASSERT(p->state.media.entry.atom);
        batom_cursor_from_atom(&p->state.media.cursor, p->state.media.entry.atom);
        p->state.packet.size = p->state.media.entry.length;
        p->state.packet.cur = 0;
        b_play_media_send_atom(p);
        break;
    case bmedia_player_entry_type_file:
        b_play_send_frame(p);
        break;

    /* should have handled previously */
    case bmedia_player_entry_type_end_of_stream:
    case bmedia_player_entry_type_no_data:
    case bmedia_player_entry_type_error:
    case bmedia_player_entry_type_async:
    case bmedia_player_entry_type_noop:
        BDBG_ASSERT(0);
        break;
    }
    return;
keep_waiting:
    p->state.state = eWaitingPlayback;
    p->state.data_source = b_play_media_send_meta; /* schedule to call us when space is avaliable */
    return;
}

static void
b_play_media_send_eos(NEXUS_PlaybackHandle p)
{
    NEXUS_Error rc;
    size_t size;
    void *vaddr;

    BDBG_ASSERT(p->media_player);
    BDBG_ASSERT(p->params.playpump);

    if(!p->state.media.segmented) {
        if (NEXUS_Playpump_GetBuffer(p->params.playpump, &vaddr, &size)!=NEXUS_SUCCESS || size == 0) {
            goto keep_waiting;
        }
        BDBG_MSG(("b_play_media_send_eos: skip %#lx %lu", (unsigned long)vaddr, (unsigned long)size));
        rc = NEXUS_Playpump_ReadComplete(p->params.playpump, 0, 0);
        if (rc!=NEXUS_SUCCESS) {
            BDBG_ERR(("b_play_media_send_eos: NEXUS_Playpump_ReadComplete error %#x, playback aborted", (unsigned)rc));
            p->state.state = eAborted;
            NEXUS_TaskCallback_Fire(p->errorCallback);
            return;
        }
    } else {
        BDBG_MSG(("b_play_media_send_eos: not supported"));
    }
    b_play_start_drain(p);
    return;
keep_waiting:
    p->state.state = eWaitingPlayback;
    p->state.data_source = b_play_media_send_eos; /* schedule to call us when space is avaliable */
    return;
}

static void
b_play_media_io_read_complete(void *playback_, ssize_t size)
{
    NEXUS_PlaybackHandle playback = playback_;
    void (*read_cont)(void *cont, ssize_t size);
    void *cntx;
    BDBG_OBJECT_ASSERT(playback, NEXUS_Playback);
    /* acquire mutex and call continuation */
    BDBG_ASSERT(playback->state.media.async_read.read_cont);
    read_cont = playback->state.media.async_read.read_cont;
    cntx = playback->state.media.async_read.cntx;
    playback->state.media.async_read.read_cont=NULL;
    playback->state.media.async_read.cntx=NULL;
    read_cont(cntx, size);
}

static void
b_play_media_io_async_read(void *sync_cnxt, bfile_io_read_t fd, void *buf, size_t length, void (*read_cont)(void *cont, ssize_t size), void *cntx)
{
    NEXUS_PlaybackHandle playback = sync_cnxt;
    BDBG_OBJECT_ASSERT(playback, NEXUS_Playback);
    BDBG_ASSERT(playback->state.media.async_read.read_cont==NULL);
    BDBG_ASSERT(playback->state.media.async_read.cntx==NULL);
    /* save context and call into async I/O */
    playback->state.media.async_read.read_cont=read_cont;
    playback->state.media.async_read.cntx=cntx;
    NEXUS_File_AsyncRead(fd, buf, length, NEXUS_MODULE_SELF, b_play_media_io_read_complete, playback);
    return;
}

void
b_play_stop_media(NEXUS_PlaybackHandle playback)
{
    if(playback->state.media.entry.atom) {
        batom_release(playback->state.media.entry.atom);
        playback->state.media.entry.atom = NULL;
    }
    if(playback->state.media.pts_timer) {
        NEXUS_CancelTimer(playback->state.media.pts_timer);
        playback->state.media.pts_timer = NULL;
    }
    if(playback->state.media.nest_timer) {
        NEXUS_CancelTimer(playback->state.media.nest_timer);
        playback->state.media.nest_timer = NULL;
    }

    bmedia_player_destroy(playback->media_player);
    playback->media_player = NULL;
    if(playback->state.media.buffer) {
        BDBG_ASSERT(playback->state.media.factory);
        bfile_buffer_destroy(playback->state.media.buffer);
        batom_factory_destroy(playback->state.media.factory);
        BDBG_ASSERT(playback->state.media.buf);
        BKNI_Free(playback->state.media.buf);
        playback->state.media.factory = NULL;
        playback->state.media.buffer = NULL;
        playback->state.media.buf = NULL;
    }
    return;
}

static void
b_play_media_player_error(void *cntx)
{
    NEXUS_PlaybackHandle playback = cntx;
    BDBG_OBJECT_ASSERT(playback, NEXUS_Playback);
    BDBG_MSG(("b_play_media_player_error: %#lx", (unsigned long)playback));
    NEXUS_TaskCallback_Fire(playback->parsingErrorCallback);
    return;
}

static int
b_play_media_get_dqt_index(void *cntx, unsigned *index, unsigned *openGopPictures)
{
    NEXUS_PlaybackHandle playback = cntx;
    const NEXUS_Playback_P_PidChannel *pid;
    BDBG_OBJECT_ASSERT(playback, NEXUS_Playback);
    for (pid = BLST_S_FIRST(&playback->pid_list); pid ; pid = BLST_S_NEXT(pid, link)) {
        if(pid->cfg.pidSettings.pidType==NEXUS_PidType_eVideo && pid->cfg.pidTypeSettings.video.decoder) {
            NEXUS_VideoDecoderMultiPassDqtData data;
            int rc = NEXUS_VideoDecoder_ReadMultiPassDqtData(pid->cfg.pidTypeSettings.video.decoder, &data);
            if (!rc) {
                *index = data.intraGopPictureIndex; /* Last intra gop picture index returned by decoder. Needed to advance MP DQT to previous GOP. */
                *openGopPictures = data.openGopPictures;
            }
            return rc;
        }
#if NEXUS_HAS_SIMPLE_DECODER
        if(pid->cfg.pidSettings.pidType==NEXUS_PidType_eVideo && pid->cfg.pidTypeSettings.video.simpleDecoder) {
            NEXUS_VideoDecoderMultiPassDqtData data;
            int rc = NEXUS_SimpleVideoDecoder_ReadMultiPassDqtData(pid->cfg.pidTypeSettings.video.simpleDecoder, &data);
            if (!rc) {
                *index = data.intraGopPictureIndex; /* Last intra gop picture index returned by decoder. Needed to advance MP DQT to previous GOP. */
                *openGopPictures = data.openGopPictures;
            }
            return rc;
        }
#endif
    }
    return -1;
}

void
b_play_update_media_player_config(NEXUS_PlaybackHandle p, bmedia_player_decoder_config *config)
{
    const NEXUS_Playback_P_PidChannel *pid;

    for (pid = BLST_S_FIRST(&p->pid_list); pid ; pid = BLST_S_NEXT(pid, link)) {
        NEXUS_VideoDecoderStatus videoStatus;
        if(pid->cfg.pidSettings.pidType==NEXUS_PidType_eVideo && pid->cfg.pidTypeSettings.video.decoder) {
            NEXUS_Error rc = NEXUS_VideoDecoder_GetStatus(pid->cfg.pidTypeSettings.video.decoder, &videoStatus);
            if(rc==BERR_SUCCESS) {
                config->video_buffer_size = videoStatus.fifoSize;
            }
        }
    }
    config->fragmented = true;
#if NEXUS_OTFPVR
    config->otf = true;
#endif
    return;
}

#include "bcmindexer.h"
#include "bcmplayer.h"
#include "../src/bcmindexerpriv.h"

static NEXUS_PlaybackMpeg2TsIndexType
b_play_mpeg2ts_probe_index(bfile_io_read_t fd, NEXUS_PlaybackHandle playback)
{
    NEXUS_TransportTimestampType timestampType = playback->params.playpumpSettings.timestamp.type;
    int rc;
    const BNAV_Entry *entry;
    size_t nav_size;
    int nav_version;
    unsigned nav_count;
    unsigned nav_total_count;
    unsigned off;
    unsigned ts_header;
    unsigned ts_count;
    unsigned ts_total_count;

    fd->seek(fd, 0, SEEK_SET);
    rc = fd->read(fd, playback->state.media.probe_buf, sizeof(playback->state.media.probe_buf));
    fd->seek(fd, 0, SEEK_SET);
    if(rc!=sizeof(playback->state.media.probe_buf)) {
        return NEXUS_PlaybackMpeg2TsIndexType_eAutoDetect;
    }
    entry = (void *)playback->state.media.probe_buf;
    nav_version = BNAV_get_version( entry );
    nav_size = BNAV_GetEntrySize(nav_version);
    for(nav_total_count=0,nav_count=0;(uint8_t *)entry+nav_size<playback->state.media.probe_buf+sizeof(playback->state.media.probe_buf);
         nav_total_count++,entry = (void *)((uint8_t *)entry + nav_size)) {
        if(nav_version == BNAV_get_version(entry)) {
            nav_count++;
        }
    }
    ts_header = timestampType==NEXUS_TransportTimestampType_eNone?0:4;
    for(ts_total_count=0,ts_count=0,off=ts_header;off<sizeof(playback->state.media.probe_buf)-1;off+=ts_header+188,ts_total_count++) {
        if(playback->state.media.probe_buf[off]==0x47) {
            ts_count++;
        }
    }
    BDBG_MSG(("b_play_mpeg2ts_probe_index: ts_count:%u/%u nav_count:%u/%u", ts_count,ts_total_count, nav_count, nav_total_count));
    if(nav_count==nav_total_count) {
        return NEXUS_PlaybackMpeg2TsIndexType_eNav;
    } else if(ts_count==ts_total_count) {
        return NEXUS_PlaybackMpeg2TsIndexType_eSelf;
    } else {
        return NEXUS_PlaybackMpeg2TsIndexType_eAutoDetect;
    }
}

static const struct {
    NEXUS_VideoCodec nexusVideoCodec;
    bvideo_codec media_video_codec;
} NEXUS_Playback_P_VideoCodecMap[]= {
    {NEXUS_VideoCodec_eMpeg1, bvideo_codec_mpeg1},
    {NEXUS_VideoCodec_eMpeg2, bvideo_codec_mpeg2},
    {NEXUS_VideoCodec_eMpeg4Part2, bvideo_codec_mpeg4_part2},
    {NEXUS_VideoCodec_eH263, bvideo_codec_h263},
    {NEXUS_VideoCodec_eH264, bvideo_codec_h264},
    {NEXUS_VideoCodec_eH264_Svc, bvideo_codec_h264_svc},
    {NEXUS_VideoCodec_eH264_Mvc, bvideo_codec_h264_mvc},
    {NEXUS_VideoCodec_eH265, bvideo_codec_h265},
    {NEXUS_VideoCodec_eVc1, bvideo_codec_vc1},
    {NEXUS_VideoCodec_eVc1SimpleMain, bvideo_codec_vc1_sm},
    {NEXUS_VideoCodec_eDivx311, bvideo_codec_divx_311},
    {NEXUS_VideoCodec_eAvs, bvideo_codec_avs}
};

static bvideo_codec
NEXUS_Playback_P_VideoCodecMap_ToMedia(NEXUS_VideoCodec videoCodec)
{
    unsigned i;
    for(i=0;i<sizeof(NEXUS_Playback_P_VideoCodecMap)/sizeof(*NEXUS_Playback_P_VideoCodecMap);i++) {
        if(NEXUS_Playback_P_VideoCodecMap[i].nexusVideoCodec == videoCodec) {
            return NEXUS_Playback_P_VideoCodecMap[i].media_video_codec;
        }
    }
    return bvideo_codec_unknown;
}

static const struct {
    NEXUS_AudioCodec nexusAudioCodec;
    baudio_format media_audio_codec;
} NEXUS_Playback_P_AudioCodecMap[]= {
    {NEXUS_AudioCodec_eMpeg, baudio_format_mpeg},
    {NEXUS_AudioCodec_eMp3, baudio_format_mp3},
    {NEXUS_AudioCodec_eAac, baudio_format_aac},
    {NEXUS_AudioCodec_eAacLoas, baudio_format_aac},
    {NEXUS_AudioCodec_eAacPlus, baudio_format_aac_plus},
    {NEXUS_AudioCodec_eAacPlusAdts, baudio_format_aac_plus_adts},
    {NEXUS_AudioCodec_eAc3,  baudio_format_ac3},
    {NEXUS_AudioCodec_eAc3Plus,  baudio_format_ac3_plus},
    {NEXUS_AudioCodec_eDts,  baudio_format_dts},
    {NEXUS_AudioCodec_eLpcmDvd,  baudio_format_lpcm_dvd},
    {NEXUS_AudioCodec_eLpcmHdDvd,  baudio_format_lpcm_hddvd},
    {NEXUS_AudioCodec_eLpcmBluRay, baudio_format_lpcm_bluray},
    {NEXUS_AudioCodec_eDtsHd,  baudio_format_dts_hd},
    {NEXUS_AudioCodec_eWmaStd, baudio_format_wma_std},
    {NEXUS_AudioCodec_eWmaPro, baudio_format_wma_pro},
    {NEXUS_AudioCodec_eAvs,  baudio_format_avs},
    {NEXUS_AudioCodec_ePcm,  baudio_format_pcm}
};

static baudio_format
NEXUS_Playback_P_AudioCodecMap_ToMedia(NEXUS_AudioCodec audioCodec)
{
    unsigned i;
    for(i=0;i<sizeof(NEXUS_Playback_P_AudioCodecMap)/sizeof(*NEXUS_Playback_P_AudioCodecMap);i++) {
        if(NEXUS_Playback_P_AudioCodecMap[i].nexusAudioCodec == audioCodec) {
            return NEXUS_Playback_P_AudioCodecMap[i].media_audio_codec;
        }
    }
    return baudio_format_unknown;
}

NEXUS_Error
b_play_start_media(NEXUS_PlaybackHandle playback, NEXUS_FilePlayHandle file, const NEXUS_PlaypumpStatus *playpump_status,const NEXUS_PlaybackStartSettings *params)
{
    bmedia_player_stream stream;
    bmedia_player_config player_config;
    unsigned i;
    const NEXUS_Playback_P_PidChannel *pid;
    const NEXUS_Playback_P_PidChannel *master_pid;
    NEXUS_Error rc=NEXUS_SUCCESS;
    NEXUS_PlaypumpSettings pumpCfg;
    bmedia_player_decoder_mode mode;

    playback->media_player = NULL;
    playback->state.index_type = NEXUS_PlaybackMpeg2TsIndexType_eAutoDetect;
    playback->state.media.last_segment_hdr = (uint8_t*)playpump_status->bufferBase + playpump_status->fifoSize - sizeof(NEXUS_PlaypumpSegment);
    playback->state.media.time_scale = BMEDIA_TIME_SCALE_BASE;
    playback->state.media.buffer = NULL;
    playback->state.media.buf = NULL;
    playback->state.media.factory = NULL;
    playback->state.media.pts_timer = NEXUS_ScheduleTimer(BMEDIA_UPDATE_POSITION_INTERVAL, b_play_media_pts_timer, playback);

    b_play_media_time_test();
    bmedia_player_init_stream(&stream);
    switch(playback->params.playpumpSettings.transportType) {
    case NEXUS_TransportType_eAsf:
        stream.format = bstream_mpeg_type_asf;
        playback->state.media.segmented = true;
        break;
    case NEXUS_TransportType_eAvi:
        stream.format = bstream_mpeg_type_avi;
        playback->state.media.segmented = true;
        break;
    case NEXUS_TransportType_eMp4:
        stream.format = bstream_mpeg_type_mp4;
        playback->state.media.segmented = true;
        break;
    case NEXUS_TransportType_eMkv:
        stream.format = bstream_mpeg_type_mkv;
        playback->state.media.segmented = true;
        break;
    case NEXUS_TransportType_eTs:
    case NEXUS_TransportType_eBulk:
        stream.format = bstream_mpeg_type_ts;
        playback->state.media.segmented = false;
        break;
    case NEXUS_TransportType_eDssPes:
    case NEXUS_TransportType_eDssEs:
        stream.format = bstream_mpeg_type_dss_es;
        playback->state.media.segmented = false;
        break;
    case NEXUS_TransportType_eMpeg2Pes:
        stream.format = bstream_mpeg_type_pes;
        playback->state.media.segmented = false;
        break;
    case NEXUS_TransportType_eMpeg1Ps:
        stream.format = bstream_mpeg_type_ts;
        playback->state.media.segmented = false;
        break;
    case NEXUS_TransportType_eVob:
        stream.format = bstream_mpeg_type_vob;
        playback->state.media.segmented = false;
        break;
    case NEXUS_TransportType_eWav:
        stream.format = bstream_mpeg_type_wav;
        playback->state.media.segmented = true;
        break;
    case NEXUS_TransportType_eMp4Fragment:
        stream.format = bstream_mpeg_type_mp4_fragment;
        playback->state.media.segmented = true;
        break;
    case NEXUS_TransportType_eRmff:
        stream.format = bstream_mpeg_type_rmff;
        playback->state.media.segmented = true;
        break;
    case NEXUS_TransportType_eFlv:
        stream.format = bstream_mpeg_type_flv;
        playback->state.media.segmented = true;
        break;
    case NEXUS_TransportType_eOgg:
        stream.format = bstream_mpeg_type_ogg;
        playback->state.media.segmented = false;
        break;
    case NEXUS_TransportType_eFlac:
        stream.format = bstream_mpeg_type_flac;
        playback->state.media.segmented = false;
        break;
    case NEXUS_TransportType_eApe:
        stream.format = bstream_mpeg_type_ape;
        playback->state.media.segmented = false;
        break;
    case NEXUS_TransportType_eAmr:
        stream.format = bstream_mpeg_type_amr;
        playback->state.media.segmented = false;
        break;
    case NEXUS_TransportType_eAiff:
        stream.format = bstream_mpeg_type_aiff;
        playback->state.media.segmented = false;
        break;
    default:
        BDBG_WRN(("Unknown transport type %u, defaulting to ES", playback->params.playpumpSettings.transportType));
        /* fallthrough */
    case NEXUS_TransportType_eEs:
        stream.format = bstream_mpeg_type_es;
        playback->state.media.segmented = false;
        break;
    }

    master_pid = NULL;

    /* choose a master track */
    for(pid = BLST_S_FIRST(&playback->pid_list); pid ; pid = BLST_S_NEXT(pid, link)) {
        switch(pid->cfg.pidSettings.pidType) {
        case NEXUS_PidType_eVideo:
            if(pid->cfg.pidTypeSettings.video.codec==NEXUS_VideoCodec_eH264) {
                /* H264 (base layer) overwrite master pid */
                master_pid = pid;
            } else {
                if(master_pid==NULL || master_pid->cfg.pidSettings.pidType!=NEXUS_PidType_eVideo) { /* Video overwrites audio */
                    master_pid = pid;
                }
            }
            break;
        case NEXUS_PidType_eAudio:
            if(master_pid==NULL) {
                master_pid = pid;
            }
            break;
        default: break;
        }
    }
    if(master_pid) {
        stream.master = master_pid->pid;
    } else {
        /* if there's no masgter pid, then we can still do playback-only work. setting to the NULL
        pid tells media framework plugs that we have not forgotten a param in their API. */
        stream.master = 0x1fff;
    }

    for(i=0,pid = BLST_S_FIRST(&playback->pid_list); pid ; pid = BLST_S_NEXT(pid, link)) {
        bool statusValid;
        NEXUS_PidChannelStatus status;
        baudio_format audio_codec = baudio_format_unknown; 
        bvideo_codec video_codec = bvideo_codec_unknown;

        switch(pid->cfg.pidSettings.pidType) {
        case NEXUS_PidType_eVideo:
            video_codec = NEXUS_Playback_P_VideoCodecMap_ToMedia(pid->cfg.pidTypeSettings.video.codec);
            break;
        case NEXUS_PidType_eAudio:
            audio_codec = NEXUS_Playback_P_AudioCodecMap_ToMedia(pid->cfg.pidSettings.pidTypeSettings.audio.codec);
            break;
        default: break;
        }
        statusValid = NEXUS_PidChannel_GetStatus(pid->pidChn, &status)==NEXUS_SUCCESS;
        if(pid==master_pid) {
            stream.stream.es.video_codec = video_codec;
            stream.stream.es.audio_codec = audio_codec;
            if(statusValid) {
                BDBG_MSG(("%s:%p mapping track %u(master) -> %#x:%#x", "b_play_start_media", (void *)playback, (unsigned)pid->pid, (unsigned)status.remappedPid, (unsigned)status.pid));
                stream.stream.id.master = status.remappedPid;
            }
        } else {
            if(i<BMEDIA_PLAYER_MAX_TRACKS) {
                stream.stream.es.other_video[i] = video_codec;
                stream.stream.es.other_audio[i] = audio_codec;
                if(statusValid) {
                    BDBG_MSG(("%s:%p mapping track %u(%u) -> %#x:%#x", "b_play_start_media", (void *)playback, (unsigned)pid->pid, (unsigned)i, (unsigned)status.remappedPid, (unsigned)status.pid));
                    stream.stream.id.other[i] = status.remappedPid;
                }
                stream.other[i] = pid->pid;
            } else {
                BDBG_WRN(("%s:%p track:%u exceeded max number of tracks %u>=%u", "b_play_start_media", (void *)playback, (unsigned)pid->pid, (unsigned)i, BMEDIA_PLAYER_MAX_TRACKS));
            }
            i++;
        }
    }

    if( (playback->params.playpumpSettings.transportType==NEXUS_TransportType_eMp4 ||
         playback->params.playpumpSettings.transportType==NEXUS_TransportType_eApe ||
         playback->params.playpumpSettings.transportType==NEXUS_TransportType_eMkv) &&
        file->file.index==NULL
       )
    {
        BDBG_ERR(("NEXUS_FilePlayHandle must be opened with an index for this transport type"));
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto error_file_index;
    }

    bmedia_player_init_config(&player_config);
    if(file->file.index && params->mode == NEXUS_PlaybackMode_eIndexed && playback->params.playpumpSettings.transportType==NEXUS_TransportType_eTs) {
        switch(params->mpeg2TsIndexType) {
        case NEXUS_PlaybackMpeg2TsIndexType_eAutoDetect:
            playback->state.index_type = b_play_mpeg2ts_probe_index(file->file.index, playback);
            break;
        case NEXUS_PlaybackMpeg2TsIndexType_eNav:
        case NEXUS_PlaybackMpeg2TsIndexType_eSelf:
            playback->state.index_type = (NEXUS_PlaybackMpeg2TsIndexType)params->mpeg2TsIndexType;
            break;
        default:
            rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
            goto error_file_index;
        }
    }
    if( playback->params.playpumpSettings.transportType==NEXUS_TransportType_eMp4 ||
        playback->params.playpumpSettings.transportType==NEXUS_TransportType_eApe ||
        playback->params.playpumpSettings.transportType==NEXUS_TransportType_eMkv ||
        (playback->params.playpumpSettings.transportType==NEXUS_TransportType_eEs && (file->file.index || params->mode == NEXUS_PlaybackMode_eAutoBitrate || playback->params.enableStreamProcessing)) ||
        ((params->mode != NEXUS_PlaybackMode_eIndexed || file->file.index==NULL) && playback->params.playpumpSettings.transportType==NEXUS_TransportType_eTs) ||
        (playback->params.playpumpSettings.transportType==NEXUS_TransportType_eTs && playback->state.index_type==NEXUS_PlaybackMpeg2TsIndexType_eSelf) ||
    ((playback->params.playpumpSettings.transportType==NEXUS_TransportType_eMpeg2Pes || playback->params.playpumpSettings.transportType==NEXUS_TransportType_eVob) && params->mode==NEXUS_PlaybackMode_eIndexed && file->file.index)
    )
      {
          bfile_buffer_cfg buffer_cfg;

          playback->state.media.factory = batom_factory_create(bkni_alloc, 16);
          if(!playback->state.media.factory) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error_factory; }
          bfile_buffer_default_cfg(&buffer_cfg);
          {
              /* detect playpump buffer size increase and create matching buffer for buffer, this allows application to affect size of the buffer */
              NEXUS_PlaypumpStatus playpumpStatus;
              NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
              NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
              if(NEXUS_Playpump_GetStatus(playback->params.playpump, &playpumpStatus) == NEXUS_SUCCESS) {
                  if(playpumpStatus.fifoSize > playpumpOpenSettings.fifoSize) {
                      size_t segmentSize = buffer_cfg.buf_len / buffer_cfg.nsegs; /* don't attempt to change size of the segment */
                      buffer_cfg.nsegs = playpumpStatus.fifoSize/ segmentSize;
                      buffer_cfg.buf_len = buffer_cfg.nsegs * segmentSize;
                      player_config.format.mp4.fragmentBufferSize = playpumpStatus.fifoSize;
                  }
              }
          }
          playback->state.media.buf = BKNI_Malloc(buffer_cfg.buf_len+BIO_BLOCK_SIZE);
          if(!playback->state.media.buf) {
              BDBG_ERR(("b_play_start_media: %#lx can't allocate %u bytes for buffer", (unsigned long)playback, (unsigned)buffer_cfg.buf_len+BIO_BLOCK_SIZE));
              rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
              goto error_media;
          }
          buffer_cfg.buf = (void*)B_IO_ALIGN_ROUND((unsigned long)playback->state.media.buf);
          buffer_cfg.fd = playback->file->file.data;
          buffer_cfg.async = true;
          buffer_cfg.sync_cnxt = playback;
          /* since async I/O is not synchronized with playback, we need to pass async I/O over custom layer that would acquire lock before calling back into the bfile_buffer */
          buffer_cfg.async_read = b_play_media_io_async_read;
          playback->state.media.buffer = bfile_buffer_create(playback->state.media.factory, &buffer_cfg);
          if(!playback->state.media.buffer) {
              rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
              goto error_buffer;
          }
      }

    player_config.buffer = playback->state.media.buffer;
    player_config.factory = playback->state.media.factory;
    player_config.cntx = playback;
    player_config.atom_ready = b_play_media_async_atom_ready;
    player_config.error_detected = b_play_media_player_error;
    player_config.get_dqt_index = b_play_media_get_dqt_index;
    player_config.timeshifting = playback->params.timeshifting;
    b_play_update_media_player_config(playback, &player_config.decoder_features);

    if(playback->params.playpumpSettings.transportType==NEXUS_TransportType_eTs) {
        stream.stream.mpeg2ts.packet_size = playback->params.playpumpSettings.timestamp.type == NEXUS_TransportTimestampType_eNone?188:192;
    }

    if(params->mode == NEXUS_PlaybackMode_eIndexed && file->file.index) {
        if(playback->state.index_type==NEXUS_PlaybackMpeg2TsIndexType_eSelf || playback->params.playpumpSettings.transportType==NEXUS_TransportType_eMpeg2Pes || playback->params.playpumpSettings.transportType==NEXUS_TransportType_eVob || playback->params.playpumpSettings.transportType==NEXUS_TransportType_eEs)  {
            playback->state.media.segmented = false;
            stream.without_index = true;
            stream.stream.noindex.auto_rate = true;
            stream.stream.noindex.bitrate = params->bitrate;
            if(playback->params.enableStreamProcessing) {
                stream.stream.noindex.auto_rate = true;
                stream.stream.id.master = 0xC0;
           }
           else if( playback->params.playpumpSettings.transportType==NEXUS_TransportType_eEs)
           {
                stream.stream.id.master = 0x0;
           }
        }
        if (!player_config.timeshifting) {
            off_t first, last;
            if (!file->file.data->bounds(file->file.data, &first, &last)) {
                player_config.data_file_size = last;
            }
        }
        playback->media_player = bmedia_player_create(file->file.index, &player_config, &stream);
        if (!playback->media_player) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error_player; }
    } else {
        playback->state.media.segmented = false;
        stream.without_index = true;
        stream.stream.noindex.auto_rate = params->mode == NEXUS_PlaybackMode_eAutoBitrate;
        if(playback->params.playpumpSettings.transportType==NEXUS_TransportType_eEs) {
            if(playback->params.enableStreamProcessing) {
                stream.stream.noindex.auto_rate = true;
                stream.stream.id.master = 0xC0;
            } else {
                if(params->mode == NEXUS_PlaybackMode_eAutoBitrate) {
                    stream.stream.id.master = 0;
                }
            }
        }
        stream.stream.noindex.bitrate = params->bitrate;
        playback->media_player = bmedia_player_create(file->file.data, &player_config, &stream);
        if (!playback->media_player) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error_player; }
    }
    bmedia_player_set_direction(playback->media_player, 0, BMEDIA_TIME_SCALE_BASE, &mode); /* normal decode */
    NEXUS_Playpump_GetSettings(playback->params.playpump, &pumpCfg);
    playback->actualTransportType = pumpCfg.transportType;
    pumpCfg.mode = playback->state.media.segmented ? NEXUS_PlaypumpMode_eSegment : NEXUS_PlaypumpMode_eFifo;
    rc = NEXUS_Playpump_SetSettings(playback->params.playpump, &pumpCfg);
    if(rc!=NEXUS_SUCCESS) { rc = BERR_TRACE(rc); goto error_playpump; }
    playback->state.direction = 1;
    playback->state.data_source = b_play_next_frame;
    return NEXUS_SUCCESS;

error_playpump:
    bmedia_player_destroy(playback->media_player);
    playback->media_player = NULL;
error_player:
    if(playback->state.media.buffer) {
        BDBG_ASSERT(playback->state.media.factory);
        bfile_buffer_destroy(playback->state.media.buffer);
        batom_factory_destroy(playback->state.media.factory);
        playback->state.media.factory = NULL;
        playback->state.media.buffer = NULL;
    }
error_buffer:
    if(playback->state.media.buf) {
        BKNI_Free(playback->state.media.buf);
        playback->state.media.buf = NULL;
    }
error_media:
    if(playback->state.media.factory) {
        batom_factory_destroy(playback->state.media.factory);
        playback->state.media.factory = NULL;
    }
error_file_index:
error_factory:
    if(playback->state.media.pts_timer) {
        NEXUS_CancelTimer(playback->state.media.pts_timer);
        playback->state.media.pts_timer = NULL;
    }
    return rc;
}

