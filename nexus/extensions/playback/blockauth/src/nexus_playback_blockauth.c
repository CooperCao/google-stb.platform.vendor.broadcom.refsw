/***************************************************************************
 *     (c)2015 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 **************************************************************************/
#include "nexus_playback_module.h"
#include "nexus_playback_impl.h"

BDBG_MODULE(nexus_playback_blockauth);

static NEXUS_Error b_play_start_fifo_drain(NEXUS_PlaybackHandle p);
static void b_play_fifo_timer(void *playback);

void NEXUS_Playback_P_BlockAuthRestart(NEXUS_PlaybackHandle p)
{
    if (p->state.state == eTransition && p->state.blockauth.fifo_drain_mode) {
        BDBG_MSG(("cancel fifo drain mode, restart requested"));
        if( p->state.blockauth.fifo_drain_timer ) {
            BDBG_MSG(("FIFO drain timer still active, cancelling %#x", p->state.blockauth.fifo_drain_timer ));
            NEXUS_CancelTimer(p->state.blockauth.fifo_drain_timer);
            p->state.blockauth.fifo_drain_timer = NULL;
        }
        p->state.blockauth.fifo_drain_mode = false;
    }
}

void NEXUS_Playback_P_BlockAuthStop(NEXUS_PlaybackHandle p)
{
    if ( p->state.blockauth.fifo_drain_timer ) {
        BDBG_MSG(("Timer still active, cancelling %#x", p->state.blockauth.fifo_drain_timer ));
        NEXUS_CancelTimer(p->state.blockauth.fifo_drain_timer);
        p->state.blockauth.fifo_drain_timer = NULL;
        p->state.blockauth.fifo_drain_mode = false;
    }
}

bool NEXUS_Playback_P_BlockAuthEnabled(NEXUS_PlaybackHandle p)
{
    return p->blockauthSettings.enabled;
}

int NEXUS_Playback_P_BlockAuthFrameData(NEXUS_PlaybackHandle p, unsigned size, unsigned read_size, int *plast)
{
    size_t authorized = 0;

    if (!p->blockauthSettings.enabled) return 0;

    if (p->blockauthSettings.authorize_block(
            p->blockauthSettings.ca_state,
            (p->state.io.last_file_offset - size + p->state.io.file_behind),
            read_size,
            &authorized) != 0) {
        return NEXUS_PLAYBACK_BLOCKAUTH_READ_ERROR;
    }
    BDBG_MSG(("%s:Auth offset = %lld, size=%d, Authorised=%d", __FUNCTION__, (p->state.io.last_file_offset - size + p->state.io.file_behind), read_size, authorized));
    if (authorized < read_size) {
        *plast = (*plast - (read_size - authorized));
        BDBG_MSG(("%s req_size=%u authorized=%u frame:last %u size %u", __FUNCTION__, read_size, authorized, *plast, p->state.frame.size));
        p->state.io.last_file_offset -= size;
        p->state.io.last_file_offset = (p->state.io.last_file_offset + p->state.io.file_behind + authorized);
        BDBG_MSG((">>>>>> modified offset %lld", p->state.io.last_file_offset));

        {
            off_t offset = p->state.io.last_file_offset;
            off_t seek_result;
            p->state.io.file_behind = offset - B_IO_ALIGN_TRUNC(offset);
            p->state.io.next_data = 0;
            BDBG_MSG((">>>>>> seek to %lld (aligned=%lld + pre=%d)",
                      (uint64_t)offset,
                      (uint64_t)B_IO_ALIGN_TRUNC(offset), p->state.io.file_behind));
            offset = B_IO_ALIGN_TRUNC(offset);
            p->state.io.last_file_offset = offset;
            seek_result = p->file->file.data->seek(p->file->file.data, offset, SEEK_SET);
            if (offset != seek_result) {
                return NEXUS_PLAYBACK_BLOCKAUTH_EOF;
            }
        }

        if (authorized == 0) {
            b_play_start_fifo_drain(p);
            return NEXUS_PLAYBACK_BLOCKAUTH_READ_ERROR;
        }
        else {
            read_size = authorized;
        }
    }
    else {
        /* Now that we've processed the file read, we're ready for the next read,
           so clear file_behind. If there's more to read without seeking, we'll use next_data
           to reset file_behind. If we need to seek, then the value of file_behind and
           next_data will be reset. */
        p->state.io.file_behind = 0;
    }
    return 0;
}

static void
b_play_fifo_timer(void *playback)
{
    bool cancel = false;
    NEXUS_PlaybackHandle p = playback;

    BDBG_OBJECT_ASSERT(p, NEXUS_Playback);
    p->state.blockauth.fifo_drain_timer = NULL;

    BDBG_MSG(("%s: entry mode=%d, marker=%d, skip=%d)", __FUNCTION__,
              p->state.blockauth.fifo_drain_mode, p->state.fifoMarker, p->state.blockauth.drainskip));

    if (p->state.blockauth.fifo_drain_mode){
        cancel = (p->state.state == eStopping
                       || p->state.state == eCancelIo
                       || p->state.state == eStopped
                       || p->state.state == eAborted
                       || p->state.state == eIoCanceled);
        if (cancel == false) {
            NEXUS_PlaypumpHandle playpump = p->params.playpump;
            NEXUS_PlaypumpStatus playpumpStatus;

            playpumpStatus.fifoDepth = 0;
            NEXUS_Playpump_GetStatus(playpump, &playpumpStatus);
            if (playpumpStatus.fifoDepth != 0) {
                int timeout = 500;

                if (p->state.blockauth.drainskip && (uint32_t)p->state.blockauth.drainskip == p->state.fifoMarker
                    && p->state.fifoMarker == playpumpStatus.fifoDepth
                    && playpumpStatus.descFifoDepth == 1){
                    NEXUS_Time now;
                    long diff;

                    NEXUS_Time_Get(&now);
                    diff = NEXUS_Time_Diff(&now, &p->state.fifoLast);
                    if (diff >= timeout){
                        BDBG_MSG(("%s: FIFO not drained (fifoDepth last=%d, cur=%d, skip=%d)", __FUNCTION__,
                                  p->state.fifoMarker, playpumpStatus.fifoDepth, p->state.blockauth.drainskip));
                        p->state.blockauth.drain_timeout = 0;
                        p->state.blockauth.fifo_drain_mode = false;
                        p->state.blockauth.drainskip = 0;

                        BDBG_MSG(("%s:%d load_key (offset=%lld)", __FUNCTION__, __LINE__, p->state.io.last_file_offset + p->state.io.file_behind));
                        if (p->blockauthSettings.load_key(
                                p->blockauthSettings.ca_state,
                                (p->state.io.last_file_offset + p->state.io.file_behind)) != 0) {
                            b_play_handle_read_error(p, 0);
                            return;
                        }
                        BDBG_MSG(("%s: load complete", __FUNCTION__));
                        goto done;
                    }
                }
                else {
                    p->state.fifoMarker = playpumpStatus.fifoDepth;
                    NEXUS_Time_Get(&p->state.fifoLast);
                }

                p->state.blockauth.drain_timeout += 50;
                BDBG_MSG(("%s: Rescheduling timer (fifoDepth=%d,marker=%d,skip=%d,desc=%d)", __FUNCTION__, playpumpStatus.fifoDepth, p->state.fifoMarker, p->state.blockauth.drainskip, playpumpStatus.descFifoDepth));
                p->state.blockauth.fifo_drain_timer = NEXUS_ScheduleTimer(50, b_play_fifo_timer, p);
            }
            else {
                BDBG_MSG(("%s: FIFO drained (fifoDepth=%d)", __FUNCTION__, playpumpStatus.fifoDepth));
                p->state.blockauth.drain_timeout = 0;
                p->state.blockauth.fifo_drain_mode = false;
                p->state.blockauth.drainskip = 0;
                BDBG_MSG(("%s: load_key (offset=%lld)", __FUNCTION__, p->state.io.last_file_offset + p->state.io.file_behind));
                if (p->blockauthSettings.load_key(
                        p->blockauthSettings.ca_state,
                        (p->state.io.last_file_offset + p->state.io.file_behind)) != 0) {
                    b_play_handle_read_error(p, 0);
                    return;
                }
                BDBG_MSG(("%s: load complete", __FUNCTION__));
            }
        }
        else {
            p->state.blockauth.drain_timeout = 0;
            p->state.blockauth.fifo_drain_mode = false;
            p->state.blockauth.drainskip = 0;
            BDBG_MSG(("%s: %#lx IO control cancelled", __FUNCTION__, p));
        }
    }
done:
    BDBG_MSG(("%s: done, mode=%d, cancel=%d", __FUNCTION__, p->state.blockauth.fifo_drain_mode, cancel));
    if (p->state.blockauth.fifo_drain_mode == false){
        if (!cancel) {
            b_play_send_frame(p); /* continue reading data */
        }
    }
}

/* This function is used to initiate drain the dma so a key can be loaded */
static NEXUS_Error
b_play_start_fifo_drain(NEXUS_PlaybackHandle p)
{
    BDBG_OBJECT_ASSERT(p, NEXUS_Playback);

    if (p->state.mode != NEXUS_PlaybackState_ePaused) { /* if we are in pause mode, just exit from the data pump loop, API would restart it when required */
        p->state.blockauth.fifo_drain_mode = true;
        p->state.blockauth.drain_timeout = 0;
        b_play_fifo_timer(p);
    } else {
        BDBG_WRN(("b_play_start_fifo_drain while in pause"));
    }
    return NEXUS_SUCCESS;
}

int NEXUS_Playback_P_BlockAuthDrainSkip(NEXUS_PlaybackHandle p, unsigned size)
{
    if (p->blockauthSettings.enabled) {
        p->state.blockauth.drainskip = size;
        return -1;
    }
    return 0;
}

void NEXUS_Playback_P_BlockAuthPreFrameData(NEXUS_PlaybackHandle p, unsigned size)
{
    if (p->blockauthSettings.enabled) {
        p->state.io.last_file_offset += size;
    }
}

void NEXUS_Playback_GetBlockAuthorizationSettings( NEXUS_PlaybackHandle p, NEXUS_PlaybackBlockAuthorizationSettings *pSettings )
{
    *pSettings = p->blockauthSettings;
}

NEXUS_Error NEXUS_Playback_SetBlockAuthorizationSettings( NEXUS_PlaybackHandle p, const NEXUS_PlaybackBlockAuthorizationSettings *pSettings )
{
    if (p->state.state != eStopped) {
        BDBG_ERR(("You must set encrypted settings before calling NEXUS_Playback_Start"));
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    if (pSettings->enabled &&
        (p->params.timeshifting ||
         (pSettings->ca_state == 0) ||
         (pSettings->authorize_block == 0) ||
         (pSettings->load_key == 0))) {
        BDBG_ERR(("Encrypted settings missing"));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    p->blockauthSettings = *pSettings;
    return 0;
}
