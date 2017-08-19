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

BDBG_MODULE(nexus_playback_flow);
#define BDBG_MSG_FLOW(X)  /* BDBG_MSG(X) */
#define BDBG_MSG_TRACE(X)  /* BDBG_MSG(X) */

static void bplay_p_pre_read_complete(NEXUS_PlaybackHandle p, size_t skip, size_t amount_used);
static bool b_play_wait_for_end(NEXUS_PlaybackHandle p);

#if B_PLAYBACK_CAPTURE
void
b_play_capture_open(NEXUS_PlaybackHandle p)
{
    char filename[64];
    BKNI_Snprintf(filename, sizeof(filename),"playback_%u.mpg", p->capture.no);
    p->capture.fout = fopen(filename, "w");
    p->capture.has_data = false;
    p->capture.no++;
    return;
}

void b_play_capture_close(NEXUS_PlaybackHandle p)
{
    if(p->capture.fout) {
        fclose(p->capture.fout);
        p->capture.fout = NULL;
        p->capture.has_data = false;
    }
    return;
}

void b_play_capture_write(NEXUS_PlaybackHandle p, const void *buf, size_t len)
{
    if(p->capture.fout) {
        p->capture.has_data = true;
        fwrite(buf, 1, len, p->capture.fout);
    }
    return;
}
#endif /* B_PLAYBACK_CAPTURE */

#if DIRECT_IO_SUPPORT
static int
b_test_buffer(NEXUS_PlaybackHandle p, uint8_t **addr, size_t *size)
{
    /* The last portion at the end of the playback buffer may be unusable for
    O_DIRECT because of alignment requirements, so skip it.
    This test shall be first test, otherwise deadlock is possible. */
    BDBG_OBJECT_ASSERT(p, NEXUS_Playback);
    BDBG_ASSERT(p->params.playpump);
    if (*size > 0 && (void *)(*addr) > p->buf_limit) {
        NEXUS_Error rc;
        BDBG_MSG(("Throwing away useless buffer %p(%u), recycling", (void*)(*addr), (unsigned)*size));
        rc = NEXUS_Playpump_WriteComplete(p->params.playpump, *size, 0);
        BDBG_ASSERT(rc == NEXUS_SUCCESS);
#if NEXUS_PLAYBACK_BLOCKAUTH
        if (NEXUS_Playback_P_BlockAuthDrainSkip(p, *size)) {
            return -1;
        }
#endif
        if (NEXUS_Playpump_GetBuffer(p->params.playpump, (void**)addr, size)!=NEXUS_SUCCESS || (*size) == 0) {
          return -1;
        } else {
          return 0;
        }
    }
    /* Make sure the block is large enough to satisfy any possible alignment
    requirement. */
    if (*size < B_IO_BLOCK_LIMIT) {
        /* minimum size needed for buffer alignment */
        BDBG_MSG_FLOW(("available space is too small (%#lx). wait for more.", (unsigned long)*size));
        return -1;
    }
    return 0;
}
#endif /* DIRECT_IO_SUPPORT */

/**
Process data before going to bplaypump_read_complete.
This allows the scrambling bits to be set prior to being fed to bplaypump_read_complete
(and any other pre-processing which may be required).  At the moment, this only affects
188 bytes of valid data in encrypted playback due to a limitation in M2M decryption.
**/
static void
bplay_p_pre_read_complete(NEXUS_PlaybackHandle p, size_t skip, size_t amount_used )
{
    BSTD_UNUSED(skip);
    BDBG_OBJECT_ASSERT(p, NEXUS_Playback);
    if (p->state.encrypted && amount_used == 188) {
        uint8_t *buf = (uint8_t *)p->state.buf + p->state.io.file_behind;
        buf[3] |= 0xC0;
    }
}

/* this function shall be called from API functions *BEFORE* releasing lock */
void
b_play_check_buffer(NEXUS_PlaybackHandle p)
{
    BDBG_OBJECT_ASSERT(p, NEXUS_Playback);
    if (p->state.state == eWaitingPlayback) {
        BDBG_MSG_FLOW(("trying to get a buffer"));
        p->state.data_source(p);
    }
}



/**
This is the callback that is called either playpump, from module scheduler or from dedicated thread.
Translate the context and route it to the appropriate function.
**/
void
b_play_read_callback(void *context)
{
    NEXUS_PlaybackHandle playback = (NEXUS_PlaybackHandle)context;

    BDBG_OBJECT_ASSERT(playback, NEXUS_Playback);
    /* We have to lock before testing state so that we don't change state
    between the check and the lock. */
    if (!playback->state.start_paused && playback->state.state == eWaitingPlayback) {
            playback->state.data_source(playback);
    }
}

void
b_play_handle_read_error(NEXUS_PlaybackHandle p, ssize_t size)
{
    BSTD_UNUSED(size);
    BDBG_MSG(("read returned size %d", (int)size));

    p->state.data_error_cnt++;
    NEXUS_TaskCallback_Fire(p->errorCallback);

    if (p->params.playErrorHandling == NEXUS_PlaybackErrorHandlingMode_eAbort) {
        NEXUS_Playback_P_AbortPlayback(p);
        return;
    }
    else {
        /* read has failed, so we shall drain a decoder buffer and then handle loop */
        b_play_start_drain(p);
        return;
    }
}

/* when data file read returns less than expected, we have to decide if we start the end-of-file drain or wait for record */
static void NEXUS_Playback_P_EndOfDataFile(NEXUS_PlaybackHandle p, ssize_t size, bool allowTimeshiftingWait)
{
    /* we have not read any useful data */
    if (p->params.timeshifting && allowTimeshiftingWait && p->recordProgressCnt) { /* wait for record and try latter */
#if DIRECT_IO_SUPPORT
        if (size) { /* revert partial read */
            off_t seek_result, seek_off;
            seek_off = p->state.io.last_file_offset - size;
            seek_result = p->file->file.data->seek(p->file->file.data, seek_off, SEEK_SET);
            if (seek_result != seek_off) {
                BDBG_ERR(("Unable to recover position after partial read"));
                p->state.state = eTransition;
                b_play_start_drain(p);
                return;
            }
        }
#else
        BSTD_UNUSED(size);
#endif
        BDBG_MSG(("Waiting for record to pump more data"));
        p->state.state = eWaitingRecord;
        if (!p->state.timer) {
            p->state.timer = NEXUS_ScheduleTimer(B_FRAME_DISPLAY_TIME * 5, b_play_timer, p);
        }
        return;
    }

    p->state.state = eTransition; /* Can't be in the eWaitingPlayback or we risk a deadlock from the playpump callback. */
    BDBG_MSG(("NEXUS_Playback_P_EndOfDataFile -> wait_for_drain"));
    {
        b_play_start_drain(p);

        /* do a special ReadComplete which notifies media framework to flush its remaining data */
        {
            void *buffer;
            size_t size;
            if (!NEXUS_Playpump_GetBuffer(p->params.playpump, &buffer, &size)) {
                NEXUS_Error rc;
                rc = NEXUS_Playpump_WriteComplete(p->params.playpump, 0, 0);
                if (rc) BERR_TRACE(rc); /* keep going */
            }
        }
    }
}

static void bplay_p_filter_frame(NEXUS_PlaybackHandle p, unsigned filteredPid, uint8_t *buf, size_t block_size, unsigned frame_offset)
{
    unsigned hdr_offset;
    unsigned packet_offset;
    unsigned pkt_size = p->params.playpumpSettings.timestamp.type == NEXUS_TransportTimestampType_eNone?188:192;
    unsigned packets, filtered_packets;


    packet_offset = frame_offset % pkt_size;
    /* start peeking at the packet boundary */
    if(packet_offset==0) {
        packet_offset = 0;
    } else {
        packet_offset = pkt_size - packet_offset;
    }
    hdr_offset = p->state.packet.size == 188 ? 0 : 4;
    for(filtered_packets=packets=0;packet_offset + hdr_offset + 4 < block_size;packet_offset += pkt_size, packets++) {
        unsigned pid;
        uint8_t *ts_hdr = buf+hdr_offset+packet_offset;

        if(ts_hdr[0] != 0x47) {
            BDBG_WRN(("%u", (unsigned)block_size));
            BDBG_WRN(("%u", (unsigned)packet_offset));
            BDBG_WRN(("bplay_p_filter_frame:%p out of sync at %u %#x(%u,%u)", (void*)p, packet_offset, ts_hdr[0], (unsigned)block_size, frame_offset));
            break;
        }
        pid = ((unsigned)(ts_hdr[1]&0x1F))<<8 | ts_hdr[2];
        if(pid!=filteredPid) {
            BDBG_MSG_TRACE(("bplay_p_filter_frame:%p filtering out pid %#x at %u", (void*)p, pid, packet_offset));
            ts_hdr[1] = 0x1F;
            ts_hdr[2] = 0xFF;
            filtered_packets++;
        }
    }
    BDBG_MSG(("bplay_p_filter_frame:%p pid %#x filtered %u/%u packets frame:%u/%u", (void*)p, filteredPid, packets, filtered_packets, (unsigned)block_size, frame_offset));
    return;
}

/* Callback from async io (already synced) or from sync_playback when reviving io */
void
b_play_frame_data(void *playback_, ssize_t size)
{
    NEXUS_Error rc;
    NEXUS_PlaybackHandle p = playback_;
    int last;
    size_t read_skip, read_size;

    BDBG_OBJECT_ASSERT(p, NEXUS_Playback);
    BDBG_ASSERT(p->params.playpump);

    last = p->state.frame.cur + (p->state.read_size - (p->state.io.file_behind) -
        p->state.io.next_data); /* last byte what we've just read */
    BDBG_MSG_FLOW(("finished read %d out of %d (frame %d:%d:%d, pre=%d, post=%d)",
        (int)size, (int)p->state.read_size,
        (int)p->state.frame.cur, (int)last, p->state.frame.size,
        p->state.io.file_behind, p->state.io.next_data));
    BDBG_ASSERT(size <= (int)p->state.read_size);

    if (b_play_control(p,eControlDataIO)) {
        p->state.io_size = size;
        return;
    }

    if (size<0) {
        if(size == BFILE_ERROR_NO_DATA) { /* if got no data then start I/O again, however we have gotten chance to go through the b_play_control logic */
            NEXUS_File_AsyncRead(p->file->file.data, p->state.buf, p->state.read_size, NEXUS_MODULE_SELF, b_play_frame_data, p);
        } else {
            b_play_handle_read_error(p, size);
        }
        return;
    }

#if DIRECT_IO_SUPPORT
    if (size<(ssize_t)p->state.read_size) { /* we read less than expected */
        if (B_IO_ALIGN_REST(size)) {
            off_t seek_result, seek_off;

            /* On linux 2.6, O_DIRECT will read the last byte, but leave the file pointer unaligned to 4K pages. This causes an O_DIRECT
            read failure. On linux 2.4, linux simply doesn't read the last bit of data. On files we record with the refsw, this is a moot
            point because we always record in 4K aligned pages. But for imported streams, especially stills, where getting every byte is important,
            this can be a problem. This technique gives us consistency. We may need to address non-4K aligned files with additional Settop API
            configurability. */
            BDBG_WRN(("Guarantee that we stay block aligned in O_DIRECT mode. Seek back %lu, truncating read to %lu", (long unsigned int)B_IO_ALIGN_REST(size), (long unsigned int)B_IO_ALIGN_TRUNC(size)));
            size = B_IO_ALIGN_TRUNC(size);
            seek_off = p->state.io.last_file_offset+size;
            seek_result = p->file->file.data->seek(p->file->file.data, seek_off, SEEK_SET);
            if (seek_result != seek_off) {
                NEXUS_Playback_P_EndOfDataFile(p, 0, false);
                return;
            }
            }
        p->state.io.last_file_offset += size;

        if (size<=(ssize_t)p->state.io.file_behind) {
            NEXUS_Playback_P_EndOfDataFile(p, size, true);
            return;
        }
        /* there is still some data what we could use */
        if (p->state.read_size - size < p->state.io.next_data) {
            p->state.io.next_data -= p->state.read_size - size; /* adjust number of prefetched bytes and don't adjust 'last' */
        } else {
            /* clear number of prefetched bytes and adjust last */
            last  = last - ((p->state.read_size - size) - p->state.io.next_data);
            p->state.io.next_data = 0;
        }
        BDBG_MSG(("new prefetch size is %d", (int)p->state.io.next_data));
        BDBG_ASSERT(last >= (int)p->state.frame.cur); /* at this point last shall point ahead of frame_cur */
    }
#if NEXUS_PLAYBACK_BLOCKAUTH
    else {
        NEXUS_Playback_P_BlockAuthPreFrameData(p, size);
    }
#endif

    BDBG_ASSERT(last <= (int)p->state.frame.size);

    read_skip = p->state.io.junk_data;
    read_size = size - p->state.io.next_data - p->state.io.file_behind;

    BDBG_MSG_FLOW(("bplay_frame_data read_complete skip=%u, size=%u frame:last %u size %u", read_skip, read_size, last, p->state.frame.size));
    if (read_size > BMEDIA_PLAYER_MAX_BLOCK) {
        BDBG_ERR(("%d, %u, %u, %u", (int)size, (unsigned)p->state.io.next_data,
            (unsigned)p->state.io.junk_data,(unsigned)p->state.io.file_behind));
        BDBG_ASSERT(false);
    }
#else /* DIRECT_IO_SUPPORT */
    BDBG_ASSERT(size % 188 == 0);
    if (size<(ssize_t)p->state.read_size) { /* we read less than expected */
        last -= p->state.read_size - size;
        if (size == 0) {
            NEXUS_Playback_P_EndOfDataFile(p, size, true);
            return;
        }
    }
    read_skip = 0;
    read_size = size;
#endif /* DIRECT_IO_SUPPORT */

    p->state.state = eTransition; /* Can't be in the eWaitingPlayback or we risk a deadlock from the playpump callback. */
    bplay_p_pre_read_complete(p, read_skip, read_size);
    if(p->state.media.entry.filteredPid!=0) {
        bplay_p_filter_frame(p, p->state.media.entry.filteredPid, (uint8_t *)p->state.buf + p->state.io.file_behind, read_size, p->state.frame.cur);
    }

    b_play_capture_write(p, (uint8_t *)p->state.buf + p->state.io.file_behind, read_size);
#if NEXUS_PLAYBACK_BLOCKAUTH
    rc = NEXUS_Playback_P_BlockAuthFrameData(p, size, &read_size, &last);
    switch (rc) {
    case 0:
        break;
    case NEXUS_PLAYBACK_BLOCKAUTH_READ_ERROR:
        b_play_handle_read_error(p, size);
        return;
    case NEXUS_PLAYBACK_BLOCKAUTH_EOF:
        BDBG_ERR(("seek wasn't able to find mpeg data"));
        NEXUS_Playback_P_EndOfDataFile(p, 0, false);
        return;
    case NEXUS_PLAYBACK_BLOCKAUTH_DRAIN:
        return;
    }
#endif
    rc = NEXUS_Playpump_WriteComplete(p->params.playpump, read_skip, read_size);
    if (rc!=NEXUS_SUCCESS) {
        BDBG_ERR(("bplay_frame_data bplaypump_read_complete error %d, playback aborted", (unsigned)rc));
        p->state.state = eAborted;
        NEXUS_TaskCallback_Fire(p->errorCallback);
        return;
    }

#if NEXUS_PLAYBACK_BLOCKAUTH
    if (!NEXUS_Playback_P_BlockAuthEnabled(p)) {
#endif
    /* Now that we've processed the file read, we're ready for the next read,
    so clear file_behind. If there's more to read without seeking, we'll use next_data
    to reset file_behind. If we need to seek, then the value of file_behind and
    next_data will be reset. */
    p->state.io.file_behind = 0;
#if NEXUS_PLAYBACK_BLOCKAUTH
    }
#endif

    p->state.frame.cur = (unsigned)last;
    if (p->state.frame.cur==p->state.frame.size) {
        /* got all data, read next frame */
        p->state.state = eTransition;
        b_play_next_frame(p);
    } else {
        b_play_send_frame(p); /* continue reading data */
    }
}


/* this function is called when we want to send generated packet */
void
b_play_send_packet(NEXUS_PlaybackHandle p)
{
    NEXUS_Error rc;
    size_t size;
    void *vaddr;

    BDBG_OBJECT_ASSERT(p, NEXUS_Playback);
    BDBG_ASSERT(p->params.playpump);

    for(;;) {
        if (NEXUS_Playpump_GetBuffer(p->params.playpump, &vaddr, &size)!=NEXUS_SUCCESS || size == 0) {
            goto keep_waiting;
        }

        size = b_imin(size, p->state.packet.size - p->state.packet.cur);
        BDBG_MSG_FLOW(("sending %d out of %d bytes left in packet",
            size, p->state.packet.size-p->state.packet.cur));
        BKNI_Memcpy(vaddr, p->state.packet.buf+p->state.packet.cur, size);

        /* Can't be in eWaitingPlayback or we risk a deadlock from playpump callback. */
        p->state.state = eTransition;
        b_play_capture_write(p, vaddr, size);
        rc = NEXUS_Playpump_WriteComplete(p->params.playpump, 0, size);
        if (rc!=NEXUS_SUCCESS) {
            BDBG_ERR(("bplay_send_packet bplaypump_read_complete error %#x, playback aborted", (unsigned)rc));
            p->state.state = eAborted;
            NEXUS_TaskCallback_Fire(p->errorCallback);
            return;
        }

        p->state.packet.cur+=size;
        if (p->state.packet.cur >= p->state.packet.size) {
            /* we're done sending the packet, so go get the next frame */
            b_play_next_frame(p);
            return;
        }
        /* keep writing data */
    }
    return;
keep_waiting:
    p->state.state = eWaitingPlayback;
    p->state.data_source = b_play_send_packet; /* schedule to call us when space is avaliable */
    return;
}

void
b_play_send_frame(NEXUS_PlaybackHandle p)
{
    size_t skip;
    uint8_t *addr;
    void *vd_addr;
    size_t size;
    size_t left;

    BDBG_ASSERT(p->params.playpump);

    if (NEXUS_Playpump_GetBuffer(p->params.playpump, (void**)&vd_addr, &size) || size == 0) {
        goto keep_waiting;
    }
    addr = (uint8_t *) vd_addr;
    BDBG_MSG_FLOW(("get_buffer %x %d", addr, size));
#if DIRECT_IO_SUPPORT
    if (b_test_buffer(p, &addr, &size)) {
        goto keep_waiting;
    }

    skip = B_IO_ALIGN_REST(addr);
    if (skip>0) { /* if playback buffer address is not aligned, compensate it */
        skip = B_IO_BLOCK_SIZE - skip;
        addr += skip;
        size -= skip;
    }
    size = B_IO_ALIGN_TRUNC(size);
    if (p->state.io.next_data) {
        off_t seek_result, seek_off;
        seek_off = p->state.io.last_file_offset-B_IO_BLOCK_SIZE;
        seek_result = p->file->file.data->seek(p->file->file.data, seek_off, SEEK_SET);
        if (seek_result != seek_off) {
            NEXUS_Playback_P_EndOfDataFile(p, 0, false);
            return;
        }
        p->state.io.last_file_offset = seek_off;
        p->state.io.file_behind = B_IO_BLOCK_SIZE-p->state.io.next_data;
        p->state.io.next_data = 0; /* clear flag out */
        BDBG_MSG_FLOW(("seeking back to %d bytes now behind %d bytes", B_IO_BLOCK_SIZE, p->state.io.file_behind));
    }
    left = p->state.io.file_behind + p->state.frame.size - p->state.frame.cur;
    if (size>left) {
        p->state.io.next_data = B_IO_ALIGN_ROUND(left) - left;
        size = B_IO_ALIGN_ROUND(left);
    } else {
        p->state.io.next_data = 0;
    }
    if (size == 0) {
        BDBG_MSG(("After alignment, nothing left. Waiting for more space."));
        goto keep_waiting;
    }
#else /* DIRECT_IO_SUPPORT */
    skip = 0;
    left = p->state.frame.size - p->state.frame.cur;
    size = b_imin(size, left);
    p->state.io.next_data = 0;
#endif

    p->state.io.junk_data = skip + p->state.io.file_behind;
    p->state.buf = addr;
    BDBG_MSG_FLOW(("reading into pb (%#lx), read=%d (skip=%d+%d, overrun=%d)",
        (unsigned long)p->state.buf, size, skip, p->state.io.file_behind, p->state.io.next_data));
    p->state.state = eWaitingIo;
    p->state.read_size = size;
    NEXUS_File_AsyncRead(p->file->file.data, p->state.buf, size, NEXUS_MODULE_SELF, b_play_frame_data, p);
    return;

keep_waiting:
    p->state.state = eWaitingPlayback;
    p->state.data_source = b_play_send_frame; /* call us when space is avaliable */
    return;
}

/* Retrieve some mark from the decoder to show current location.
This uses a combination to CDB depth, PTS change and DM picture queue depth.
*/

/**
playback must decide if the stream has come to the end. This is determined based on a series of tests
looking at decoder inputs and outputs, fifo depths, and changing PTS values.
The following conditions are evaluated in order. If any are not true, playback will wait longer.

1) playback fifoDepth must go to zero.
2) video decoder queueDepth (which is the post-decoder picture queue) must go to zero.
3) fifoMarker (which is a combination of current PTS and post-decoder queue depths) must be unchanged for 6 frame times

NOTE: audio decoder queuedFrames (which is the post-decoder frame queue) only contributes to the fifoMarker because bad
streams may give a bogus frame count that never drops below a certain level; whereas video is guaranteed to drop to zero.

If all of these are true, the stream has come to an end. Playback will evaluate the beginning/endOfStreamAction and
fire the beginning/endOfStreamCallback.
**/
void
bplay_get_decode_mark(NEXUS_PlaybackHandle playback, uint32_t *pFifoMarker, bool *pQueued, unsigned *pWaitTime)
{
    const NEXUS_Playback_P_PidChannel *pid;
    NEXUS_Error rc;
    NEXUS_PlaypumpStatus playpumpStatus;
    uint32_t fifoMarker=0;
    bool videoTested=false; /* at the time we support only single decoded video program, but possible two video pids (for SVC/MVC) */
    
    *pQueued = false;
    BDBG_OBJECT_ASSERT(playback, NEXUS_Playback);
    BDBG_ASSERT(playback->params.playpump);
    rc = NEXUS_Playpump_GetStatus(playback->params.playpump, &playpumpStatus);
    if(rc==BERR_SUCCESS) {
        BDBG_MSG_FLOW(("%s:%lx playpump %u",  "bplay_get_decode_mark", (unsigned long)playback, (unsigned)playpumpStatus.fifoDepth));
        fifoMarker += playpumpStatus.fifoDepth;
        if(playpumpStatus.fifoDepth>0) {
            /* if there's even 1 byte in the playback fifo, we must still wait */
            *pQueued = true;
        }
    } else { rc=BERR_TRACE(rc);}


    for(pid = BLST_S_FIRST(&playback->pid_list); pid ; pid = BLST_S_NEXT(pid, link)) {
        switch(pid->cfg.pidSettings.pidType) {
        default: break;
        case NEXUS_PidType_eVideo:
            if(!videoTested) {
                NEXUS_VideoDecoderStatus videoStatus;
                videoTested = true;
                rc = NEXUS_P_Playback_VideoDecoder_GetStatus(pid, &videoStatus);
                if (rc==NEXUS_SUCCESS && videoStatus.started) {
                    unsigned fifoDepth;
                    BDBG_MSG_FLOW(("%s:%lx video %u(%u):%u:%u",  "bplay_get_decode_mark", (unsigned long)playback, (unsigned)videoStatus.fifoDepth, (unsigned)videoStatus.enhancementFifoDepth, (unsigned)videoStatus.queueDepth, (unsigned)videoStatus.pts));
                    fifoDepth = videoStatus.fifoDepth;
                    if(videoStatus.enhancementFifoSize) {
                        /* for SVC/MVC use min of base/enhancement layer FIFO's */
                        if(fifoDepth>videoStatus.enhancementFifoDepth) {
                            fifoDepth = videoStatus.enhancementFifoDepth;
                        }
                    }
                    fifoMarker += videoStatus.fifoDepth;
                    if(videoStatus.queueDepth>0) {
                        *pQueued = true;
                    }
                    fifoMarker += videoStatus.pts;
                    fifoMarker += videoStatus.queueDepth;
                }
            }
            break;
        case NEXUS_PidType_eAudio:
            {
                NEXUS_AudioDecoderStatus audioStatus, audioStatus2;
                rc = NEXUS_P_Playback_AudioDecoder_GetStatus(pid, &audioStatus, &audioStatus2);
                if (!rc) {
                    if(audioStatus.started) {
                        BDBG_MSG_FLOW(("%s:%lx audio[PRI] %u:%u:%u",  "bplay_get_decode_mark", (unsigned long)playback, (unsigned)audioStatus.fifoDepth, (unsigned)audioStatus.queuedFrames, (unsigned)audioStatus.pts));
                        fifoMarker += audioStatus.fifoDepth;
                        fifoMarker += audioStatus.pts;
                        fifoMarker += audioStatus.queuedFrames;
                        if(audioStatus.queuedFrames>8) {
                            /* if we have a few audio frames, we need to wait longer. */
                            *pWaitTime = 500; /* msec */
                        }
                    }
                    if(audioStatus2.started) {
                        BDBG_MSG_FLOW(("%s:%lx audio[SEC] %u:%u:%u",  "bplay_get_decode_mark", (unsigned long)playback, (unsigned)audioStatus2.fifoDepth, (unsigned)audioStatus2.queuedFrames, (unsigned)audioStatus2.pts));
                        fifoMarker += audioStatus2.pts;
                        fifoMarker += audioStatus2.queuedFrames;
                        if(audioStatus2.queuedFrames>8) {
                            /* if we have a few audio frames, we need to wait longer. */
                            *pWaitTime = 500; /* msec */
                        }
                    }
                }
            }
            break;
        }
    }
    *pFifoMarker = fifoMarker;
    BDBG_MSG_FLOW(("%s:%p queued %u, fifoMarker %u",  "bplay_get_decode_mark", (void*)playback, (unsigned)*pQueued, (unsigned)fifoMarker));
    if(fifoMarker==0) {
        BDBG_WRN(("unable to get decode mark"));
    }
    return;
}


/* Wait for the decoder to see if we've decoded to the end.
Returns true if end of stream was reached
*/
static bool
b_play_wait_for_end(NEXUS_PlaybackHandle p)
{
    uint32_t fifoMarker;
    bool queued;
    NEXUS_Time now;
    long diff;
    unsigned waitTime = B_FRAME_DISPLAY_TIME * 6;

    BDBG_OBJECT_ASSERT(p, NEXUS_Playback);
    /* in the case of forward frame advance, we have to use different logic */
    if (p->state.mode == NEXUS_PlaybackState_ePaused ) { /* skip fifo check */
        if(p->state.fifoMarkerCounter==0) {
            return false;
        }
        p->state.fifoMarkerCounter--;
        if (p->state.fifoMarkerCounter) {
            return false;
        }
        /* when p->state.fifoMarkerCounter hits 0, then it will fall through and perform the test once */
    }

    bplay_get_decode_mark(p, &fifoMarker, &queued, &waitTime);
    if (fifoMarker != p->state.fifoMarker) {
        p->state.fifoMarker = fifoMarker;
        NEXUS_Time_Get(&p->state.fifoLast);
        return false;
    }
    NEXUS_Time_Get(&now);
    diff = NEXUS_Time_Diff(&now, &p->state.fifoLast);

    BDBG_MSG_FLOW(("b_play_wait_for_end:%#lx %d %u", (unsigned long)p, diff, queued));

    if (queued || diff < (int)waitTime) {
        /* need to wait longer */
        return false;
    }

    /* else fall through and jump right now.
    reinit depth and timer for the next time. */
    p->state.fifoMarker = 0;
    /* reset */
    NEXUS_Time_Get(&p->state.fifoLast);
    return true;
}

/**
* If ts/BNAV_Player_getNextEntry fails, we must assume we've come to the beginning or end.
*
* Returns true if playback should continue without reading more data
*    because we're waiting for decode to finish
* Returns false if the player was reset and another readindex should be performed
*/
static NEXUS_Error
b_play_handle_loop_condition(NEXUS_PlaybackHandle p)
{
    NEXUS_PlaybackLoopMode loopmode;
    bool is_beginning;
    bmedia_player_pos pos;
    bmedia_player_status status;
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(p, NEXUS_Playback);
    BDBG_ASSERT(p->media_player);

    if (p->state.mode == NEXUS_PlaybackState_ePaused) {
        if (p->state.loopedDuringPause) {
            return NEXUS_SUCCESS;
        }
    }
    else {
        p->state.loopedDuringPause = false;
    }

    b_play_update_location(p);
    bmedia_player_get_status(p->media_player, &status);
    pos = b_play_correct_position(p, &status);

    /* Now we have a verified end of stream, so notity the app and take action. */
    if (p->state.direction < 0 || (pos && (pos < status.bounds.first))) {
        /* BOF can happen when going backwards or if you fall off the beginning during continuous record.
        We careful that pos can be zero if the decoder has no PTS on a transition. */
        is_beginning = true;
        NEXUS_TaskCallback_Fire(p->beginningOfStreamCallback);
        loopmode = p->params.beginningOfStreamAction;
        BDBG_MSG(("Beginning of stream reached (pos %d in %u...%u), loopmode %d", (int)pos, (unsigned)status.bounds.first, (unsigned)status.bounds.last, loopmode));
    } else {
        is_beginning = false;
        NEXUS_TaskCallback_Fire(p->endOfStreamCallback);
        loopmode = p->params.endOfStreamAction;
        BDBG_MSG(("End of stream reached (pos %d in %u...%u), loopmode %d", (int)pos, (unsigned)status.bounds.first, (unsigned)status.bounds.last, loopmode));
    }
    rc = b_play_media_handle_loop_condition(p, is_beginning, loopmode, &status.bounds);

    if (!rc && p->state.mode == NEXUS_PlaybackState_ePaused) {
        /* we have looped (i.e. fired an endOfStream or beginningOfStream action) while paused, so don't fire it again. */
        p->state.loopedDuringPause = true;
    }

    return rc;
}

/* this function called from the timer context */
void
b_play_timer(void *playback)
{
    NEXUS_PlaybackHandle p = playback;

    BDBG_OBJECT_ASSERT(p, NEXUS_Playback);
    p->state.timer = NULL;

    /* In timeshifting mode, we may have hit the BOF. This should never invoke the drain which monitors EOF.
    Check for this special condition and invoke loop_condition directly if so.
    If we're waiting for an IO request to complete we can't reset our state. We'll soon be in eWaitingPlayback, and then it's safe to make a change. */
    if (p->params.timeshifting && p->state.state != eWaitingIo) {
        bmedia_player_pos pos;
        bmedia_player_status status;
        b_play_update_location(p);
        bmedia_player_get_status(p->media_player, &status);
        pos = b_play_correct_position(p, &status);
        if (pos < status.bounds.first) {
            b_play_handle_loop_condition(p);
            p->state.drain_mode = false;
            b_play_next_frame(playback);
            goto done;
        }

        if (p->state.state == eWaitingRecord) {
            /* if we're at the end of the file, we should test if we've come to the end of decode.
            if so, the application needs an endOfStream callback so it can switch to live decode. */
            if (b_play_wait_for_end(playback)) {
                b_play_handle_loop_condition(p);
            }
            if (!p->state.timer) { /* timer could have been started by b_play_handle_loop_condition */
                p->state.timer = NEXUS_ScheduleTimer(B_FRAME_DISPLAY_TIME*5, b_play_timer, p);
            }
            goto done;
        }
    }
    
    if (p->state.state == eTimer) {
        if (p->state.drain_mode) {
            if (b_play_control(p, eControlFrame)) { goto done; }
            if (b_play_wait_for_end(playback)) {
                /* all data is decoded now, ask user what to do next */
                b_play_handle_loop_condition(p);
                p->state.drain_mode = false;
                /* schedule timer and it will restart playback */
                BDBG_MSG(("Turn off drain. Rescheduling timer event."));
                if (!p->state.timer) { /* timer could have been started by b_play_handle_loop_condition */
                    p->state.timer = NEXUS_ScheduleTimer(B_FRAME_DISPLAY_TIME, b_play_timer, p);  /* schedule another call into the same function after while */
                }
            } else {
                /* decoder FIFO not drained yet, schedule another timer */
                BDBG_MSG(("FIFO not drained. Rescheduling timer event."));
                BDBG_ASSERT(!p->state.timer);
                p->state.timer = NEXUS_ScheduleTimer(B_FRAME_DISPLAY_TIME, b_play_timer, p);  /* schedule another call into the same function after while */
            }
        } else {
            BDBG_MSG(("timer feed"));
            b_play_next_frame(playback);
        }
    } else {
        BDBG_MSG(("Timer: next frame was cancelled %d", p->state.state));
    }

    if( p->state.mode == NEXUS_PlaybackState_ePaused && !p->state.timer) {
        /* If we're in ePaused state, we cannot exit this function without the timer being rescheduled.
        If the drain logic has already rescheduled it, that's fine. But if not, we must schedule it here. */
        BDBG_MSG(("Paused. Rescheduling timer event."));
        p->state.timer = NEXUS_ScheduleTimer(B_FRAME_DISPLAY_TIME, b_play_timer, p);  /* schedule another call into the same function after while */
    }

done:
    b_play_check_buffer(p);
    return;
}


/* This function is used to initiate drain in a decoder buffer, once drain is completed, b_play_handle_loop_condition will be called */
NEXUS_Error
b_play_start_drain(NEXUS_PlaybackHandle p)
{
    BDBG_OBJECT_ASSERT(p, NEXUS_Playback);
    if (b_play_control(p, eControlFrame)) { return true; }
    /* we are waiting for some external event */
    p->state.state = eTimer;
    p->state.drain_mode = true;
    BDBG_MSG(("b_play_start_drain starting timer"));
    if (!p->state.timer) {
        p->state.timer = NEXUS_ScheduleTimer(B_FRAME_DISPLAY_TIME, b_play_timer, p);  /* schedule another call into the same function after while */
    }
    return NEXUS_SUCCESS;
}

/*
 * this function monitors any user requests, and handles them as necessary,
 * it returns true, if normal execution shall be aborted and false otherwise
 */
bool
b_play_control(NEXUS_PlaybackHandle playback, enum b_control_origin origin)
{
    BDBG_OBJECT_ASSERT(playback, NEXUS_Playback);
    if (origin != eControlDataIO &&
        (playback->state.state == eStopping || playback->state.state == eCancelIo))
    {
        BDBG_MSG(("Alive, but waiting for I/O to finish"));
        return false; /* keep going */
    }

    if (playback->state.state == eStopping) {
        BDBG_MSG(("Stop request received, sending ack"));
        playback->state.state = eStopped;
        BKNI_SetEvent(playback->ack_event);
        return true; /* stop execution */
    }
    if (playback->state.state == eCancelIo) {
        BDBG_MSG(("CancelIo request received, sending ack"));
        playback->state.state = eIoCanceled;
        BKNI_SetEvent(playback->ack_event);
        return true; /* stop execution */
    }

    if(playback->state.state == eStopped || playback->state.state == eAborted) {
        BDBG_MSG(("Playback stopped"));
        return true; /* stop execution */
    }

    return false; /* keep going */
}

