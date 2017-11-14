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
#include "nexus_core_utils.h"
#include "priv/nexus_file_priv.h"

BDBG_MODULE(nexus_playback);

/* We have a separate dbg macro for data flow. When you want to see it, uncomment
the following */
#define BDBG_MSG_FLOW(X) /* BDBG_MSG(X) */



BDBG_OBJECT_ID(NEXUS_Playback);

/*
 * the following function is used to control playback dataflow
 * and handle various transition between trickmodes
 */
static NEXUS_Error bplay_p_frameadvance(NEXUS_PlaybackHandle p, bool forward, bool *restart);
static void b_play_playpump_read_callback_locked(void *context);
static NEXUS_Error b_play_playpump_read_callback_guard(void *context);

#define HAS_INDEX(p) ((p)->file->file.index)

void
NEXUS_Playback_GetDefaultSettings( NEXUS_PlaybackSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->playpump = NULL;
    NEXUS_Playpump_GetDefaultSettings(&pSettings->playpumpSettings);
    pSettings->endOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
    pSettings->beginningOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
    NEXUS_CallbackDesc_Init(&pSettings->endOfStreamCallback);
    NEXUS_CallbackDesc_Init(&pSettings->beginningOfStreamCallback);
    NEXUS_CallbackDesc_Init(&pSettings->errorCallback);
    NEXUS_CallbackDesc_Init(&pSettings->parsingErrorCallback);
    pSettings->startPaused = false;
    pSettings->timeshifting = false;
    pSettings->stcTrick = false;
    pSettings->accurateSeek = false;
    pSettings->enableStreamProcessing = false;
    pSettings->ioTimeout = 5000; /* 5 sec */
    pSettings->playErrorHandling = NEXUS_PlaybackErrorHandlingMode_eEndOfStream;
    pSettings->seekErrorHandling = NEXUS_PlaybackErrorHandlingMode_eIgnore;
    pSettings->trickErrorHandling = NEXUS_PlaybackErrorHandlingMode_eIgnore;
    /* milliseconds to gap between end of playback and record which will not cause decode stuttering */
    pSettings->timeshiftingSettings.endOfStreamGap = 3000;
    /* milliseconds to gap between beginning of playback and truncating record which will not cause excess beginningOfStream events */
    pSettings->timeshiftingSettings.beginningOfStreamGap = 5000;

    return;
}

NEXUS_PlaybackHandle
NEXUS_Playback_Create( void )
{
    NEXUS_PlaybackHandle playback;
    NEXUS_Error rc;

    playback = BKNI_Malloc(sizeof(*playback));
    if (!playback) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto err_alloc; }

    BDBG_OBJECT_INIT(playback,NEXUS_Playback);
    BKNI_Memset(&playback->state, 0, sizeof(playback->state));
#if NEXUS_PLAYBACK_BLOCKAUTH
    BKNI_Memset(&playback->blockauthSettings, 0, sizeof(playback->blockauthSettings));
#endif

    playback->state.state = eStopped;
    BLST_S_INIT(&playback->pid_list);
    playback->file = NULL;
    playback->media_player = NULL;
    playback->thread_terminate = false;
    playback->playpump_thread_event = NULL;
    playback->playpump_thread = NULL;
    playback->playpump_event_callback = NULL;
    playback->playpump_event = NULL;
    playback->index_file_mode = NEXUS_PlaybackIndexFileIo_eCallback;
    playback->actualTransportType = NEXUS_TransportType_eTs;
    NEXUS_CallbackHandler_Init(playback->dataCallbackHandler, b_play_playpump_read_callback_locked, playback);
    NEXUS_CallbackHandler_SetGuard(playback->dataCallbackHandler, b_play_playpump_read_callback_guard);
    NEXUS_CallbackHandler_Init(playback->videoDecoderFirstPts, NEXUS_Playback_P_VideoDecoderFirstPts, playback);
    NEXUS_CallbackHandler_Init(playback->videoDecoderFirstPtsPassed, NEXUS_Playback_P_VideoDecoderFirstPtsPassed, playback);

    NEXUS_Thread_GetDefaultSettings(&playback->playpump_thread_settings);
    NEXUS_Playback_GetDefaultTrickModeSettings(&playback->state.trickmode_params);
    playback->errorCallback = NEXUS_TaskCallback_Create(playback, NULL);
    if(!playback->errorCallback) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto err_error_callback;}
    playback->endOfStreamCallback = NEXUS_TaskCallback_Create(playback, NULL);
    if(!playback->endOfStreamCallback) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto err_end_of_stream_callback;}
    playback->beginningOfStreamCallback = NEXUS_TaskCallback_Create(playback, NULL);
    if(!playback->beginningOfStreamCallback) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto err_beginning_of_stream_callback;}
    playback->parsingErrorCallback = NEXUS_TaskCallback_Create(playback, NULL);
    if(!playback->parsingErrorCallback) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto err_parsing_error_callback;}

    NEXUS_Playback_GetDefaultSettings(&playback->params);

    rc = BKNI_CreateEvent(&playback->ack_event);
    if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_ack_event;}

    playback->playpump_event = NULL;
    playback->playpump_event_callback = NULL;

    b_play_media_time_test();
#if B_PLAYBACK_CAPTURE
    playback->capture.has_data = false;
    playback->capture.no = 0;
    playback->capture.fout = NULL;
#endif
    playback->recordProgressCnt = 0;

    return playback;

err_ack_event:
    NEXUS_TaskCallback_Destroy(playback->parsingErrorCallback);
err_parsing_error_callback:
    NEXUS_TaskCallback_Destroy(playback->beginningOfStreamCallback);
err_beginning_of_stream_callback:
    NEXUS_TaskCallback_Destroy(playback->endOfStreamCallback);
err_end_of_stream_callback:
    NEXUS_TaskCallback_Destroy(playback->errorCallback);
err_error_callback:
    BKNI_Free(playback);
err_alloc:
    return NULL;
}


void
NEXUS_Playback_ClosePidChannel(NEXUS_PlaybackHandle playback, NEXUS_PidChannelHandle pidChannel)
{
    NEXUS_Error rc;
    NEXUS_Playback_P_PidChannel *play_pid;

    BDBG_OBJECT_ASSERT(playback, NEXUS_Playback);

    BLST_S_DICT_FIND(&playback->pid_list, play_pid, pidChannel, pidChn, link);
    if(play_pid==NULL) {
        BDBG_WRN(("NEXUS_Playback_ClosePidChannel: %#lx can't find pid:%#lx", (unsigned long)playback, (unsigned long)pidChannel));
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return;
    }

    if (play_pid->cfg.pidSettings.pidType == NEXUS_PidType_eVideo) {
        NEXUS_VideoDecoderPlaybackSettings playbackSettings;
        NEXUS_P_Playback_VideoDecoder_GetPlaybackSettings(play_pid, &playbackSettings);
        if (playbackSettings.firstPts.callback || playbackSettings.firstPtsPassed.callback) {
            playbackSettings.firstPts.callback = NULL;
            playbackSettings.firstPtsPassed.callback = NULL;
            (void)NEXUS_P_Playback_VideoDecoder_SetPlaybackSettings(play_pid, &playbackSettings);
            NEXUS_CallbackHandler_Stop(playback->videoDecoderFirstPts);
            NEXUS_CallbackHandler_Stop(playback->videoDecoderFirstPtsPassed);
        }
    }

    NEXUS_CallbackHandler_Stop(playback->videoDecoderFirstPts);
    NEXUS_CallbackHandler_Stop(playback->videoDecoderFirstPtsPassed);
    BLST_S_DICT_REMOVE(&playback->pid_list, play_pid, pidChannel, NEXUS_Playback_P_PidChannel, pidChn, link);
    BDBG_ASSERT(play_pid);
    BDBG_ASSERT(playback->params.playpump);
    rc = NEXUS_Playpump_ClosePidChannel(playback->params.playpump, play_pid->pidChn);
    if(rc!=NEXUS_SUCCESS) {rc = BERR_TRACE(rc);}
    BKNI_Free(play_pid);
    return;
}

void
NEXUS_Playback_CloseAllPidChannels(NEXUS_PlaybackHandle playback)
{
    NEXUS_Playback_P_PidChannel *pid;

    /* clear all pid channels */
    while(NULL!=(pid = BLST_S_FIRST(&playback->pid_list))) {
        /* use NEXUS_Playback_ClosePidChannel so that other cleanup will happen */
        NEXUS_Playback_ClosePidChannel(playback, pid->pidChn);
    }
    return;
}

void
NEXUS_Playback_GetDefaultPidChannelSettings(NEXUS_PlaybackPidChannelSettings *pSettings)
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pSettings->pidSettings);
    BDBG_ASSERT(pSettings->pidSettings.pidType == NEXUS_PidType_eUnknown);
    return;
}

NEXUS_PidChannelHandle
NEXUS_Playback_OpenPidChannel(NEXUS_PlaybackHandle playback, unsigned pidNo, const NEXUS_PlaybackPidChannelSettings *pSettings)
{
    NEXUS_PlaybackPidChannelSettings settings;
    NEXUS_Playback_P_PidChannel *play_pid;
    unsigned playpump_pidNo;

    BDBG_OBJECT_ASSERT(playback, NEXUS_Playback);
    if(pSettings==NULL) {
        NEXUS_Playback_GetDefaultPidChannelSettings(&settings);
        pSettings = &settings;
    }

    if(playback->params.playpump==NULL) {
        BDBG_WRN(("NEXUS_Playback_OpenPidChannel: %#lx pid could only be open if playpump is attached", (unsigned long)playback));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err_playpump;
    }
    play_pid = BKNI_Malloc(sizeof(*play_pid));
    if(play_pid==NULL) { BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto err_alloc;}
    playpump_pidNo = pidNo;

    switch(playback->params.playpumpSettings.transportType) {
    default:
        break;
#if 0
    case NEXUS_TransportType_eMp4:
    case NEXUS_TransportType_eMkv:
        break;
#endif
    case NEXUS_TransportType_eEs:
        if(playback->params.enableStreamProcessing) {
            playpump_pidNo = 0xC0;
        }
        break;
    }

    BDBG_MSG(("NEXUS_Playback_OpenPidChannel: %#lx pid %#lx mapped %u->%#x", (unsigned long)playback, (unsigned long)play_pid, (unsigned)pidNo, (unsigned)playpump_pidNo));
    play_pid->pidChn = NEXUS_Playpump_OpenPidChannel(playback->params.playpump, playpump_pidNo, &pSettings->pidSettings);
    if(!play_pid->pidChn) { BERR_TRACE(BERR_NOT_SUPPORTED);goto err_open_pid;}
    play_pid->pid = pidNo;
    play_pid->cfg = *pSettings;
    play_pid->playback = playback;

    BLST_S_DICT_ADD(&playback->pid_list, play_pid, NEXUS_Playback_P_PidChannel, pidChn, link, err_duplicate);

    return play_pid->pidChn;

err_duplicate:
    BERR_TRACE(BERR_INVALID_PARAMETER);
    NEXUS_Playpump_ClosePidChannel(playback->params.playpump, play_pid->pidChn);
err_open_pid:
    BKNI_Free(play_pid);
err_alloc:
err_playpump:
    return NULL;

}

NEXUS_Error
NEXUS_Playback_GetPidChannelSettings(NEXUS_PlaybackHandle playback, NEXUS_PidChannelHandle pidChannel, NEXUS_PlaybackPidChannelSettings *pSettings)
{
    const NEXUS_Playback_P_PidChannel *pid;
    NEXUS_Error rc;
    BDBG_OBJECT_ASSERT(playback, NEXUS_Playback);
    BDBG_ASSERT(pSettings);

    BLST_S_DICT_FIND(&playback->pid_list, pid, pidChannel, pidChn, link);
    if(!pid) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto err_invalid_pid;
    }
    *pSettings = pid->cfg;
    return BERR_SUCCESS;
err_invalid_pid:
    return rc;
}

NEXUS_Error
NEXUS_Playback_SetPidChannelSettings( NEXUS_PlaybackHandle playback, NEXUS_PidChannelHandle pidChannel, const NEXUS_PlaybackPidChannelSettings *pSettings)
{
    NEXUS_Playback_P_PidChannel *pid;
    NEXUS_Error rc;
    BDBG_OBJECT_ASSERT(playback, NEXUS_Playback);
    BDBG_ASSERT(pSettings);

    BLST_S_DICT_FIND(&playback->pid_list, pid, pidChannel, pidChn, link);
    if(!pid) { rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto err_invalid_pid; }

    if(BKNI_Memcmp(&pid->cfg.pidSettings, &pSettings->pidSettings, sizeof(pSettings->pidSettings))!=0) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);goto err_settings; } /* we can't change playpump pid configuration */
    pid->cfg.pidTypeSettings = pSettings->pidTypeSettings;

    if (playback->state.state != eStopped) {
        /* reapply current decoder settings */
        rc = b_play_trick_set_pid(playback, pid, NULL);
        if (rc) {rc = BERR_TRACE(rc); goto err_settings;}
    }

    return BERR_SUCCESS;

err_settings:
err_invalid_pid:
    return rc;
}

static void
NEXUS_Playback_P_KillPlaypumpThread(NEXUS_PlaybackHandle playback)
{
    playback->thread_terminate = true;
    BDBG_ASSERT(playback->playpump_thread_event);
    BKNI_SetEvent(playback->playpump_thread_event);
    NEXUS_UnlockModule();
    BDBG_MSG(("killing playpump_thread ..."));
    NEXUS_Thread_Destroy(playback->playpump_thread);
    BDBG_MSG(("killing playpump_thread ... done"));
    NEXUS_LockModule();
    playback->playpump_thread = NULL;
    return;
}


void
NEXUS_Playback_Destroy( NEXUS_PlaybackHandle playback)
{
    BDBG_OBJECT_ASSERT(playback, NEXUS_Playback);

    if(playback->state.state != eStopped) {
        BDBG_WRN(("NEXUS_Playback_Destroy: destroying playback without stopping"));
        NEXUS_Playback_Stop(playback);
    }
    NEXUS_Playback_CloseAllPidChannels(playback);

    if(playback->playpump_thread) {
        NEXUS_Playback_P_KillPlaypumpThread(playback);
    }
    if(playback->playpump_thread_event) {
        BKNI_DestroyEvent(playback->playpump_thread_event);
    }
    if(playback->playpump_event) {
        BDBG_ASSERT(playback->playpump_event_callback);
        NEXUS_UnregisterEvent(playback->playpump_event_callback);
        BKNI_DestroyEvent(playback->playpump_event);
    }
    BKNI_DestroyEvent(playback->ack_event);
    NEXUS_CallbackHandler_Shutdown(playback->dataCallbackHandler);
    NEXUS_CallbackHandler_Shutdown(playback->videoDecoderFirstPts);
    NEXUS_CallbackHandler_Shutdown(playback->videoDecoderFirstPtsPassed);
    NEXUS_TaskCallback_Destroy(playback->parsingErrorCallback);
    NEXUS_TaskCallback_Destroy(playback->errorCallback);
    NEXUS_TaskCallback_Destroy(playback->endOfStreamCallback);
    NEXUS_TaskCallback_Destroy(playback->beginningOfStreamCallback);

    BDBG_OBJECT_DESTROY(playback, NEXUS_Playback);
    BKNI_Free(playback);
    return;
}


static int
NEXUS_P_Playback_GetPriority(void *cntx)
{
    NEXUS_PlaybackHandle playback=cntx;
    NEXUS_PlaypumpStatus status;
    NEXUS_Error rc;
    int factor;

    /* no b_lock here because the state check is atomic and bplaypump_get_status
    has its own synchronization. */
    if (playback->state.mode == NEXUS_PlaybackState_ePlaying) {
        BDBG_ASSERT(playback->params.playpump);

        rc = NEXUS_Playpump_GetStatus(playback->params.playpump, &status);
        if(rc==NEXUS_SUCCESS) {
            factor  = (status.fifoSize - status.fifoDepth)/(status.fifoSize/256); /* priority is 0 .. 256 */
            /* make read priority grow linearly with space in the fifo*/
            BDBG_MSG_FLOW(("[%#x] level %d%% priority %d", playback, status.fifoDepth*100/status.fifoSize, factor));
     } else {
            factor = 0;
        }
    } else {
        factor = 32;
    }

    return factor;
}


/**
Relocate BNAV_Player to the current decode location.
Call these before changing pvr state in order to get the current pts.
**/
void
b_play_update_location(NEXUS_PlaybackHandle p)
{
    NEXUS_Error rc;
    uint32_t pts;

    rc = b_play_getpts(p, &pts);
    if(rc==NEXUS_SUCCESS) {
        p->state.validPts = true;
        bmedia_player_update_position(p->media_player, pts);
    } else {
        p->state.validPts = false;
        BDBG_MSG(("Can't reset location because no pts."));
    }
    return;
}

const NEXUS_Playback_P_PidChannel *
b_play_get_video_decoder(NEXUS_PlaybackHandle playback)
{
    const NEXUS_Playback_P_PidChannel *pid;
    for(pid = BLST_S_FIRST(&playback->pid_list); pid ; pid = BLST_S_NEXT(pid, link)) {
        if(pid->cfg.pidSettings.pidType == NEXUS_PidType_eVideo) {
            if (pid->cfg.pidTypeSettings.video.decoder || pid->cfg.pidTypeSettings.video.simpleDecoder) return pid;
        }
    }
    return NULL;
}

static bool
b_play_has_audio_decoder(NEXUS_PlaybackHandle playback)
{
    const NEXUS_Playback_P_PidChannel *pid;
    for(pid = BLST_S_FIRST(&playback->pid_list); pid ; pid = BLST_S_NEXT(pid, link)) {
        if(pid->cfg.pidSettings.pidType == NEXUS_PidType_eAudio) {
            if (pid->cfg.pidTypeSettings.audio.primary || pid->cfg.pidTypeSettings.audio.secondary || pid->cfg.pidTypeSettings.audio.simpleDecoder) {
                return true;
            }
        }
    }
    return false;
}

bool b_play_has_connected_decoders(NEXUS_PlaybackHandle p)
{
    return b_play_get_video_decoder(p)!=NULL || b_play_has_audio_decoder(p);
}

bmedia_player_pos b_play_correct_position(NEXUS_PlaybackHandle p, const bmedia_player_status *player_status)
{
    bmedia_player_pos position;
    BDBG_ASSERT(player_status);
    if(p->state.validPts || b_play_has_connected_decoders(p)) {
        bmedia_player_tell(p->media_player, &position);
    } else {
        position = player_status->position;
    }
    return position;
}


bool
b_play_decoders_in_tsm_mode(NEXUS_PlaybackHandle playback)
{
    NEXUS_Error rc;
    const NEXUS_Playback_P_PidChannel *pid;
    for(pid = BLST_S_FIRST(&playback->pid_list); pid ; pid = BLST_S_NEXT(pid, link)) {
        if(pid->cfg.pidSettings.pidType == NEXUS_PidType_eVideo) {
            NEXUS_VideoDecoderStatus status;
            rc = NEXUS_P_Playback_VideoDecoder_GetStatus(pid, &status);
            if (rc) return false;
            if (!status.tsm) return false;
        }
        else if(pid->cfg.pidSettings.pidType == NEXUS_PidType_eAudio) {
            NEXUS_AudioDecoderStatus status, status2;
            rc = NEXUS_P_Playback_AudioDecoder_GetStatus(pid, &status, &status2);
            if (rc) return false;
            if (status.started && !status.tsm) return false;
            if (status2.started && !status2.tsm) return false;
        }
    }
    return true;
}

NEXUS_Error
b_play_getpts(NEXUS_PlaybackHandle p, uint32_t *pts)
{
    NEXUS_Error rc;
    NEXUS_Error result=NEXUS_NOT_SUPPORTED;
    const NEXUS_Playback_P_PidChannel *pid;

    BDBG_ASSERT(pts);
    *pts = 0;

    /* check video before checking audio */
    pid = b_play_get_video_decoder(p);
    if (pid) {
        NEXUS_VideoDecoderStatus status;
        rc = NEXUS_P_Playback_VideoDecoder_GetStatus(pid, &status);
        if(rc==NEXUS_SUCCESS) {
            if(status.ptsType == NEXUS_PtsType_eCoded || status.ptsType == NEXUS_PtsType_eInterpolatedFromValidPTS || p->actualTransportType==NEXUS_TransportType_eEs) {
                *pts = status.pts;
                result = NEXUS_SUCCESS;
            }
        }
    } else {
        bool foundDecoder = false;

        for(pid = BLST_S_FIRST(&p->pid_list); pid ; pid = BLST_S_NEXT(pid, link)) {
            if(pid->cfg.pidSettings.pidType == NEXUS_PidType_eAudio) {
                NEXUS_AudioDecoderStatus audioStatus, audioStatus2;
                rc = NEXUS_P_Playback_AudioDecoder_GetStatus(pid, &audioStatus, &audioStatus2);
                    if(rc==BERR_SUCCESS) {
                    if(audioStatus.started && (audioStatus.ptsType == NEXUS_PtsType_eCoded || audioStatus.ptsType == NEXUS_PtsType_eInterpolatedFromValidPTS || p->actualTransportType==NEXUS_TransportType_eEs)) {
                            *pts = audioStatus.pts;
                        result = NEXUS_SUCCESS;
                        foundDecoder = true;
                        break;
                    }
                    if(audioStatus2.started && (audioStatus2.ptsType == NEXUS_PtsType_eCoded || audioStatus2.ptsType == NEXUS_PtsType_eInterpolatedFromValidPTS)) {
                        *pts = audioStatus2.pts;
                        result = NEXUS_SUCCESS;
                        foundDecoder = true;
                        break;
                    }
                    if ( audioStatus.started || audioStatus2.started ) {
                        /* Found a valid pid but no PTS available yet */
                        foundDecoder = true;
                    }
                }
            }
        }
        if(!foundDecoder && !p->state.noDecoderWarning) {
            BDBG_WRN(("Unable to look up position because no audio decoder was provided in any NEXUS_PlaybackPidChannelSettings."));
            p->state.noDecoderWarning = true; /* only print once per start */
        }
    }
    return result;
}



void
NEXUS_Playback_GetDefaultTrickModeSettings(NEXUS_PlaybackTrickModeSettings *trickmode)
{
    BKNI_Memset(trickmode, 0, sizeof(*trickmode));
    trickmode->mode = NEXUS_PlaybackHostTrickMode_eNone;
    trickmode->rate = NEXUS_NORMAL_PLAY_SPEED;
    trickmode->mode_modifier = 1;
    trickmode->maxDecoderRate = 1.2 * NEXUS_NORMAL_PLAY_SPEED;
    trickmode->rateControl = NEXUS_PlaybackRateControl_eDecoder;
    trickmode->skipControl = NEXUS_PlaybackSkipControl_eHost;
    trickmode->brcmTrickMode = true;
    return;
}

static void bplay_p_apply_accurate_seek(NEXUS_PlaybackHandle p)
{
    if(p->state.accurateSeek.state == b_accurate_seek_state_videoPtsQueued ) {
        const NEXUS_Playback_P_PidChannel *pid = b_play_get_video_decoder(p);
        if(pid)  {
            NEXUS_VideoDecoderHandle decode= pid->cfg.pidTypeSettings.video.decoder;
            if (decode) {
                /* tell decoder to discard until this pts */
                NEXUS_Error rc = NEXUS_VideoDecoder_SetStartPts(decode, p->state.accurateSeek.videoPts);
                if (rc==BERR_SUCCESS) {
                    p->state.accurateSeek.state=b_accurate_seek_state_videoPtsQueued;
                    return;
                }
            }
        }
        p->state.accurateSeek.state=b_accurate_seek_state_idle;
    }
    return;
}

static NEXUS_Error bplay_p_accurate_seek(NEXUS_PlaybackHandle p, bmedia_player_pos position)
{
    uint32_t pts = 0;
    const NEXUS_Playback_P_PidChannel *pid = b_play_get_video_decoder(p);
    NEXUS_Error rc;
    b_trick_settings settings;

    /* requires regular decoder. does not support simpleDecoder */
    if (!pid) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if(!HAS_INDEX(p)) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED); /* playback should be started with an index */
    }

    switch (p->params.playpumpSettings.transportType) {
    case NEXUS_TransportType_eTs:
        if(p->state.index_type!=NEXUS_PlaybackMpeg2TsIndexType_eNav) {
            /* not supported for self-indexed MPEG-2 TS files */
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
        break;
    case NEXUS_TransportType_eAsf:
    case NEXUS_TransportType_eAvi:
        break;
    case NEXUS_TransportType_eMp4:
    case NEXUS_TransportType_eMkv:
        break;
    default:
        /* not supported for other containers */
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    b_play_trick_get(p, &settings);
    if (settings.state != b_trick_state_normal) {
        /* only applies to normal play mode */
        return NEXUS_SUCCESS;
    }

    if(p->state.accurateSeek.state != b_accurate_seek_state_idle) {
        BDBG_WRN(("bplay_p_accurate_seek:%p unexpected state:%u", (void *)p, p->state.accurateSeek.state));
        p->state.accurateSeek.state = b_accurate_seek_state_idle;
    }

    if (!bmedia_player_lookup_pts(p->media_player, position, &pts)) {

        switch (p->params.playpumpSettings.transportType) {
        case NEXUS_TransportType_eAsf:
        case NEXUS_TransportType_eAvi:
            {
                bmedia_player_decoder_mode mode;
                bmedia_player_entry entry;
                /* jump back to the previous key frame */
                bmedia_player_set_direction(p->media_player, -33, -BMEDIA_TIME_SCALE_BASE, &mode);
                do {
                    if (bmedia_player_next(p->media_player, &entry)) {
                        entry.timestamp = position;
                        break;
                    }
                }
                while (entry.type == bmedia_player_entry_type_file && entry.start == 0);
                bmedia_player_set_direction(p->media_player, 0, BMEDIA_TIME_SCALE_BASE, &mode);
                bmedia_player_seek(p->media_player, entry.timestamp);
            }
            break;
        case NEXUS_TransportType_eMp4:
        case NEXUS_TransportType_eMkv:
            {
                unsigned try;
                bmedia_player_pos original_position = BMEDIA_PLAYER_INVALID;
                bmedia_player_pos current_target = position;

                for(try=0;;try++)
                {
                    unsigned limit = 100; /* number of steps */
                    bmedia_player_pos step = 500; /* ms */
                    bmedia_player_status status;

                    bmedia_player_get_status(p->media_player, &status);
                    if(status.position == BMEDIA_PLAYER_INVALID) {
                        BDBG_ERR(("bplay_p_accurate_seek:%p can't obtain position after seeking to %ld", (void *)p, (long)current_target));
                        /* current position is unknown, aboirt */
                        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
                    }
                    if(try==0) {
                        original_position = status.position;
                    }
                    BDBG_MSG(("bplay_p_accurate_seek:%p seek to [%u]%ld->%ld[%ld]", (void *)p, try, (long)current_target, (long)status.position, (long)original_position));
                    if(status.position<position) {
                        BDBG_MSG(("bplay_p_accurate_seek:%p completed seek to %ld-(%ld,%ld) PTS:%#x(%u) in %u steps", (void *)p, (long)position, (long)current_target, (long)status.position, (unsigned)pts, (unsigned)(pts/45),try));
                        break;
                    }
                    if(try>=limit || current_target == 0) {
                        BDBG_ERR(("bplay_p_accurate_seek:%p can't find seek point for %ld in range %ld -> %ld...%ld", (void *)p, (long)position, (long)current_target, (long)status.position, (long)original_position));
                        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
                    }
                    if(current_target>step) {
                        current_target -= step;
                    } else {
                        current_target = 0;
                    }
                    rc = bmedia_player_seek(p->media_player, current_target);
                    if( rc!=0 ) {
                        BDBG_ERR(("bplay_p_accurate_seek:%p can't seek to %ld", (void *)p, (long)current_target));
                        /* current position is unknown, aboirt */
                        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
                    }
                }
            }
        break;

        default:
            /* bcmplayer already seeks back to previous I frame */
            BDBG_WRN(("accurate seek: to %d (pts %#x)", (int)position, (unsigned)pts));
            break;
        }
        /* We can't call yet video decoder API (it should be called after Flush, so store target PTS and call when we can */
        p->state.accurateSeek.state = b_accurate_seek_state_videoPtsQueued;
        p->state.accurateSeek.videoPts = pts;
        /* start state machine to allow audio to chase video's PTS. This keeps a full audio CDB from
        hanging video as it seeks for its start PTS. the state machine is:
        1) wait for video first PTS
        2) start audio pause (which launches the rap_monitor_timer)
        3) wait for video first PTS passed, which means we've found the start pts
        4) stop audio pause
        */
    }
    else {
        BDBG_WRN(("Unable to do accurateSeek position=%d(%#x)", (int)position, pts));
    }

    return NEXUS_SUCCESS;
}
static bool bplay_p_use_stc_trick(NEXUS_PlaybackHandle p)
{
    bool streamEs;
    switch(p->params.playpumpSettings.transportType) {
    case NEXUS_TransportType_eAiff:
    case NEXUS_TransportType_eAmr:
    case NEXUS_TransportType_eApe:
    case NEXUS_TransportType_eEs:
    case NEXUS_TransportType_eFlac:
    case NEXUS_TransportType_eOgg:
    case NEXUS_TransportType_eWav:
        streamEs = true;
        break;
    default:
        streamEs = false;
        break;
    }

    return p->params.stcTrick && !streamEs;
}

/*
Resume normal playback. This should not be called immediately after bplay_start because it's
not needed.

We may come from the following states:
1) stc trick modes.
2) decoder pause from decoder trick or normal play.
3) decoder pause from host trick. requires reset_location.
4) decoder trick.
5) host trick. requires reset_location.

The way to distinguish 2 & 3 is to use the BNAV_Player state. See below.
*/
static NEXUS_Error
bplay_p_play_impl(NEXUS_PlaybackHandle p, bool *restart, bool flush, bmedia_player_pos position)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    b_trick_settings settings;
    bmedia_player_decoder_mode mode;
    bmedia_player_decoder_config config;

    *restart = false;

    if (flush) {
        b_play_flush(p);
    }

    /* XXX this should be avoided in favor of calling bmedia_player_set_direction, and then handling bmedia_player_decoder_mode */
    b_play_trick_get(p, &settings);
    settings.forward = true;
    settings.state = b_trick_state_normal;
    settings.decode_rate = NEXUS_NORMAL_PLAY_SPEED;
    settings.decode_mode = NEXUS_VideoDecoderDecodeMode_eAll;
    settings.fragmented = false;
    settings.reordering_mode = NEXUS_VideoDecoderReorderingMode_eNone;
    settings.stc_trick = bplay_p_use_stc_trick(p);
    settings.force_source_frame_rate = 0;
    settings.simulated_tsm = false;
    NEXUS_Playback_GetDefaultTrickModeSettings(&p->state.trickmode_params);
    rc = b_play_trick_set(p, &settings);
    if (rc != NEXUS_SUCCESS) { return BERR_TRACE(rc); }

    bmedia_player_get_decoder_config(p->media_player, &config);
    config.host_mode = bmedia_player_host_trick_mode_auto;
    config.mode_modifier = 1; /* unused */
    rc = bmedia_player_set_decoder_config(p->media_player, &config);
    if (rc) return BERR_TRACE(rc);

    if (p->state.decoder_flushed) {
        /* resync player */
        bmedia_player_set_direction(p->media_player, 0, BMEDIA_TIME_SCALE_BASE, &mode); /* normal decode */

        if (p->params.accurateSeek) {
            rc = bplay_p_accurate_seek(p, position);
            if(rc==NEXUS_SUCCESS) {
                /* must flush before NEXUS_VideoDecoder_SetStartPts is called to avoid a false detect
                with existing pictures in the queue */
                b_play_flush(p);
                bplay_p_apply_accurate_seek(p);
            } else {
                rc = BERR_SUCCESS; /* clear error from bplay_p_accurate_seek */
            }
        }

        *restart = true;
    }

    p->state.direction = 1;
    p->state.mode = NEXUS_PlaybackState_ePlaying;
    p->state.frame_advance = NEXUS_Playback_P_FrameAdvance_Forward;
    return rc;
}

NEXUS_Error
bplay_p_play(NEXUS_PlaybackHandle p, bool *restart, bool flush)
{
    bmedia_player_pos position;

    BDBG_MSG(("resume play"));
    b_play_update_location(p);
    bmedia_player_tell(p->media_player, &position);
    return bplay_p_play_impl(p, restart, flush, position);
}

NEXUS_Error
bplay_p_play_from(NEXUS_PlaybackHandle p, bmedia_player_pos position)
{
    bool restart;
    bmedia_player_seek(p->media_player, position);
    return bplay_p_play_impl(p, &restart, true, position);
}


NEXUS_Error
bplay_p_pause(NEXUS_PlaybackHandle p)
{
    b_trick_settings settings;
    BDBG_MSG(("going to a pause, state.state %d, state.mode %d", p->state.state, p->state.mode));
    if (p->state.mode == NEXUS_PlaybackState_ePaused) {
        return NEXUS_SUCCESS;
    }
    p->state.mode = NEXUS_PlaybackState_ePaused;
    p->state.loopedDuringPause = false;
    p->state.seekPositionValid = false;

    /* during pause, we have a timer so we can notify the caller of internal changes trim continuous record trim */
    if (!p->state.timer) {
        p->state.timer = NEXUS_ScheduleTimer(B_FRAME_DISPLAY_TIME * 5, b_play_timer, p);   /* schedule another call into the same function after while */
    }

    b_play_trick_get(p, &settings);
    settings.decode_rate = 0;
    return b_play_trick_set(p, &settings);
}

/**
Callbacks from NEXUS_Record
**/
void NEXUS_Playback_AddRecordProgress_priv(NEXUS_PlaybackHandle p)
{
    BDBG_OBJECT_ASSERT(p, NEXUS_Playback);
    NEXUS_ASSERT_MODULE();
    p->recordProgressCnt++;
}
void NEXUS_Playback_RemoveRecordProgress_priv(NEXUS_PlaybackHandle p)
{
    BDBG_OBJECT_ASSERT(p, NEXUS_Playback);
    NEXUS_ASSERT_MODULE();
    if (p->recordProgressCnt) {
        p->recordProgressCnt--;
    }
}
void
NEXUS_Playback_RecordProgress_priv(NEXUS_PlaybackHandle p)
{
    BDBG_OBJECT_ASSERT(p, NEXUS_Playback);
    NEXUS_ASSERT_MODULE();

    if (p->state.start_paused) {
        return;
    }

    if (p->state.state != eWaitingRecord) {
        BDBG_MSG_FLOW(("Got record event, but not waiting for record(%d), ignore", p->state.state));
        if (p->state.state == eWaitingPlayback) {
            goto restart; /* Kickstart playback */
        }
        return;
    }

    BDBG_MSG(("NEXUS_Playback_RecordProgress_priv %u", (unsigned)p->state.read_size));
    if (p->state.read_size==0) {
        bmedia_player_pos pos;
        bmedia_player_status status;

        bmedia_player_tell(p->media_player, &pos);
        bmedia_player_get_status(p->media_player, &status);
        if (pos > status.bounds.first) {
            /* we are waiting for an index, try to get new entry */
            b_play_next_frame(p);
        }
        else {
            /* if we've fallen off the beginning of a continuous record, we need to reposition */
            p->state.state = eWaitingPlayback;
        }
    }
    else {
       /* we are waiting for a data */
        p->state.state = eWaitingIo;
        NEXUS_File_AsyncRead(p->file->file.data, p->state.buf, p->state.read_size, NEXUS_MODULE_SELF, b_play_frame_data, p);
    }

restart:
    b_play_check_buffer(p);
    return;
}

/**
Whatever state you're in, get into a paused state that can be
used for frame advance. This means paused-from-play.
**/
static NEXUS_Error
bplay_p_get_into_paused_play_state(NEXUS_PlaybackHandle p, bool *restart)
{
    NEXUS_Error rc;
    rc = bplay_p_play(p, restart, true);
    if (rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);}
    rc = bplay_p_pause(p);
    if (rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);}
    *restart = true; /* we've already flushed, but we need to restart playback after this */
    p->state.frame_advance = NEXUS_Playback_P_FrameAdvance_Forward;
    return rc;
}

/* set b_trick_settings based on bmedia_player_decoder_mode and (optionally) a NEXUS_PlaybackTrickModeSettings override */
static void
NEXUS_Playback_P_ConvertPlayerDecodeMode(NEXUS_PlaybackHandle p, const bmedia_player_decoder_mode *mode,  b_trick_settings *trick_settings, const NEXUS_PlaybackTrickModeSettings *params)
{
    unsigned abs_time_scale;
    BDBG_CASSERT(NEXUS_VideoDecoderDecodeMode_eAll==(NEXUS_VideoDecoderDecodeMode)bmedia_player_decoder_frames_all);
    BDBG_CASSERT(NEXUS_VideoDecoderDecodeMode_eIP==(NEXUS_VideoDecoderDecodeMode)bmedia_player_decoder_frames_IP);
    BDBG_CASSERT(NEXUS_VideoDecoderDecodeMode_eI==(NEXUS_VideoDecoderDecodeMode)bmedia_player_decoder_frames_I);
    trick_settings->decode_mode = (NEXUS_VideoDecoderDecodeMode)mode->display_frames;

    BDBG_CASSERT(NEXUS_VideoDecoderReorderingMode_eNone==(NEXUS_VideoDecoderReorderingMode)bmedia_player_reordering_mode_none);
    BDBG_CASSERT(NEXUS_VideoDecoderReorderingMode_eSequential==(NEXUS_VideoDecoderReorderingMode)bmedia_player_reordering_mode_sequential);
    BDBG_CASSERT(NEXUS_VideoDecoderReorderingMode_eInterleaved==(NEXUS_VideoDecoderReorderingMode)bmedia_player_reordering_mode_interleaved);
    BDBG_CASSERT(NEXUS_VideoDecoderReorderingMode_eChunkForward==(NEXUS_VideoDecoderReorderingMode)bmedia_player_reordering_mode_forward);
    BDBG_CASSERT(NEXUS_VideoDecoderReorderingMode_eChunkBackward==(NEXUS_VideoDecoderReorderingMode)bmedia_player_reordering_mode_backward);
    trick_settings->reordering_mode = (NEXUS_VideoDecoderReorderingMode)mode->reordering_mode;
    trick_settings->simulated_tsm = false;
    abs_time_scale = mode->time_scale >= 0 ? mode->time_scale : -mode->time_scale;
    if(mode->continuous) {
        trick_settings->decode_rate = abs_time_scale * (NEXUS_NORMAL_PLAY_SPEED/BMEDIA_TIME_SCALE_BASE);
        trick_settings->state = b_trick_state_normal;
    } else {
        trick_settings->decode_rate = NEXUS_NORMAL_PLAY_SPEED;
        if(mode->brcm) {
             trick_settings->state = b_trick_state_brcm_trick_mode;
             trick_settings->decode_rate = (abs_time_scale * (NEXUS_NORMAL_PLAY_SPEED/BMEDIA_TIME_SCALE_BASE));
        } else if(mode->dqt) {
             if (params && (params->mode == NEXUS_PlaybackHostTrickMode_ePlayMultiPassDqt || params->mode == NEXUS_PlaybackHostTrickMode_ePlayMultiPassDqtIP)) {
                trick_settings->state = b_trick_state_mdqt_mode;
            }
            else {
                trick_settings->state = b_trick_state_dqt_mode;
            }
        } else if(mode->tsm) {
            trick_settings->decode_rate = mode->time_scale * (NEXUS_NORMAL_PLAY_SPEED/BMEDIA_TIME_SCALE_BASE);
            trick_settings->state = b_trick_state_host_paced;
        } else if(mode->simulated_tsm) {
            trick_settings->decode_rate = params?params->rate:NEXUS_NORMAL_PLAY_SPEED;
            trick_settings->state = b_trick_state_host_trick_mode;
            trick_settings->simulated_tsm = true;
        } else if(mode->fragmented) {
            /* use simulated tsm on top of fragmented mode */
            trick_settings->simulated_tsm = true;
            trick_settings->decode_rate = params?params->rate:NEXUS_NORMAL_PLAY_SPEED;
            trick_settings->state = b_trick_state_host_trick_mode;
            trick_settings->reordering_mode = trick_settings->forward ? NEXUS_VideoDecoderReorderingMode_eChunkForward : NEXUS_VideoDecoderReorderingMode_eChunkBackward;
        } else if(mode->reordering_mode!=bmedia_player_reordering_mode_none) {
            trick_settings->decode_rate = abs_time_scale * (NEXUS_NORMAL_PLAY_SPEED/BMEDIA_TIME_SCALE_BASE);
            trick_settings->state = b_trick_state_host_trick_mode;
        } else {
            trick_settings->decode_rate = abs_time_scale * (NEXUS_NORMAL_PLAY_SPEED/BMEDIA_TIME_SCALE_BASE);
            trick_settings->state = b_trick_state_host_trick_mode;
        }
    }
    
#define HAS_STC_CHANNEL(p) ((p)->params.stcChannel || (p)->params.simpleStcChannel)

    /* apply custom modifications */
    if (params && params->mode!=NEXUS_PlaybackHostTrickMode_eNone) {
        if (params->rateControl == NEXUS_PlaybackRateControl_eStream) {
            /* select HW STC if possible, otherwise SW STC */
            if (HAS_STC_CHANNEL(p) && params->rate >= 0 && params->rate <= (int)params->maxDecoderRate) {
                trick_settings->stc_trick = true;
                trick_settings->simulated_tsm = false;
            }
            else {
                trick_settings->stc_trick = false;
                trick_settings->simulated_tsm = true;
            }
            trick_settings->decode_rate = params->rate;
        }
        else {
            /* NEXUS_PlaybackRateControl_eDecoder only supports positive rates */
            trick_settings->decode_rate = params->rate>0?params->rate:-params->rate;
        }
        switch(params->mode) {
        case NEXUS_PlaybackHostTrickMode_ePlayI:
            trick_settings->decode_mode = NEXUS_VideoDecoderDecodeMode_eI;
            break;
        case NEXUS_PlaybackHostTrickMode_ePlayIP:
            trick_settings->decode_mode = NEXUS_VideoDecoderDecodeMode_eIP;
            break;
        default:
            break;
        }
    } else {
        trick_settings->stc_trick = bplay_p_use_stc_trick(p);
    }
    return;
}

/**
The type of frame advance depends on the type of pause used. You must always be paused
before doing frame advance/reverse.

Frame advance (i.e. forward) is can be decoder-based or stc based.
Frame reverse is host-based.
**/
static NEXUS_Error
bplay_p_frameadvance(NEXUS_PlaybackHandle p, bool forward, bool *restart)
{
    NEXUS_Error rc;

    BDBG_MSG(("frame %s", forward?"advance":"reverse"));
    *restart = false;

    /* fifoMarkerCounter tells the wait_for_end routine to not check for a certain amount of time after a frame advance.
    If your system checks for static video PTS to detect EOF, this will give it time to move. */
    p->state.loopedDuringPause = false;
    if (p->state.fifoMarkerCounter == 0) {
        bool queued;
        unsigned waitTime = 0;

        bplay_get_decode_mark(p, &p->state.fifoMarker, &queued, &waitTime);
        p->state.fifoMarkerCounter = 5;
    }

    if(forward && p->state.frame_advance != NEXUS_Playback_P_FrameAdvance_Forward) {
        rc = bplay_p_get_into_paused_play_state(p, restart);
        if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);}
    } else if(!forward && p->state.frame_advance != NEXUS_Playback_P_FrameAdvance_Reverse) {
        bmedia_player_decoder_mode mode;
        b_trick_settings settings;
        bmedia_player_decoder_config config;

        bmedia_player_get_decoder_config(p->media_player, &config);
        if (p->params.frameReverse.gopDepth) {
            config.host_mode = bmedia_player_host_trick_mode_gop;
            if (p->params.frameReverse.gopDepth > 100) {
                return BERR_TRACE(NEXUS_INVALID_PARAMETER);
            }
            config.mode_modifier = -1 * (100 + p->params.frameReverse.gopDepth);
        }
        else {
            /* -1x Brcm for MPEG, -1 I for other codecs */
            config.host_mode = bmedia_player_host_trick_mode_auto;
        }
        bmedia_player_set_decoder_config(p->media_player, &config);

        b_play_trick_get(p, &settings);
        b_play_update_location(p);
        bmedia_player_set_direction(p->media_player, -33 /* 33 ms */, -BMEDIA_TIME_SCALE_BASE, &mode);
        NEXUS_Playback_P_ConvertPlayerDecodeMode(p, &mode,  &settings, NULL);
        if(mode.discontinuity) {
            b_play_flush(p);
        }
        p->state.frame_advance = NEXUS_Playback_P_FrameAdvance_Reverse;
        *restart = true;
        settings.decode_rate = 0;
        settings.forward = false;
        rc = b_play_trick_set(p, &settings);
        if (rc) {
            p->state.frame_advance = NEXUS_Playback_P_FrameAdvance_Invalid;
            return BERR_TRACE(rc);
        }
    } else {
        BDBG_MSG(("continuing frame %s", forward?"advance":"reverse"));
    }

    p->state.direction = forward?1:-1;

    rc = b_play_trick_frameadvance(p);
    if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);}
    p->state.mode = NEXUS_PlaybackState_ePaused;
    return rc;
}

static NEXUS_Error NEXUS_Playback_P_CheckWaitingIo(NEXUS_PlaybackHandle playback, bool stopping)
{
    unsigned i = 0;
    unsigned ioTimeout = playback->params.ioTimeout; /* exposed in the NEXUS_PlaybackSettings if we support recovery from io error by killing the File i/o thread. */

/* period of 1/10th of a second */
#define IO_TIMEOUT_PERIOD 100
    if (playback->state.state == eWaitingIo) { /* WaitingIo it's non cancelable state, so we should wait for I/O to complete */
        playback->state.state = stopping ? eStopping : eCancelIo;
        if (playback->state.media.nest_timer) {
            playback->state.media.nest_count = 0;
            NEXUS_CancelTimer(playback->state.media.nest_timer);
            b_play_suspend_timer(playback); /* allow timer function to resolve it */
        }
        else {
            BDBG_MSG(("NEXUS_Playback_P_CheckWaitingIo %p waiting for I/O to complete", (void *)playback));
            do {
                NEXUS_Error rc;
                NEXUS_UnlockModule();
                rc = BKNI_WaitForEvent(playback->ack_event, IO_TIMEOUT_PERIOD);
                NEXUS_LockModule();
                if (rc == BERR_TIMEOUT) {
                    BDBG_WRN(("NEXUS_Playback_P_CheckWaitingIo %p timed out, keep waiting", (void *)playback));
                }
                else if (rc!=0) {
                    return BERR_TRACE(rc);
                }
                if (++i >= ioTimeout / IO_TIMEOUT_PERIOD) {
                    BDBG_ERR(("NEXUS_Playback_P_CheckWaitingIo %p timed out. Canceling File thread.", (void *)playback));
                    NEXUS_FilePlay_Cancel(playback->file);
                    i = 0;
                    /* we keep looping. File will spawn a new thread and will invoke the callback with an error. Playback should go to eAborted. */
                }
            } while (!(playback->state.state == eIoCanceled || playback->state.state == eStopped || playback->state.state == eAborted));
        }
    }
    if (playback->state.state == eAborted) {
        return -1;
    }
    return 0;
}


/*
* This function is used to synchronize control with
* data pump. it return b_ok if synchronization suceeded,
* false otherwise.
*/
static NEXUS_Error
bplay_sync_playback(NEXUS_PlaybackHandle p)
{
    BERR_Code rc;

    if (p->state.start_paused) {
        /* if playback wasn't started yet, just do nothing. we should be in WaitingPlayback state. */
        BDBG_ASSERT(p->state.state == eWaitingPlayback);
        return NEXUS_SUCCESS;
    }
    if (p->state.state == eStopped) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    if (p->state.state == eAborted) {
        /* something went wrong. we need the app to stop. */
        return BERR_TRACE(NEXUS_UNKNOWN);
    }

    rc = NEXUS_Playback_P_CheckWaitingIo(p, false);
    if (rc) return BERR_TRACE(rc);

    p->state.reset = false;

    /* still locked */
    return NEXUS_SUCCESS;
}

/* this function is used to jumpstart data pump if it was cancelled because of bplay_sync_playback */
static void
bplay_restart_playback(NEXUS_PlaybackHandle p, bool restart_data_flow)
{
    if (p->state.start_paused) {
        /* if playback wasn't started yet, just do nothing. we should be in WaitingPlayback state. */
        BDBG_ASSERT(p->state.state == eWaitingPlayback);
        return;
    }

    /* if restart_data_flow is false, then either the API has failed or we're in a paused state and
    don't want to restart. in all other cases, it's true. */
    if (restart_data_flow) {
        /* 3 states override this */
        restart_data_flow = (
            p->state.state != eIoCanceled &&
            p->state.state != eWaitingPlayback &&
            p->state.state != eWaitingRecord);
    }
#if NEXUS_PLAYBACK_BLOCKAUTH
	NEXUS_Playback_P_BlockAuthRestart(p);
#endif

    if (p->state.reset) {
        BDBG_MSG(("I/O reset was requested"));
        if(p->state.media.entry.atom) {
            batom_release(p->state.media.entry.atom);
            p->state.media.entry.atom = NULL;
        }
        b_play_flush(p);
        bplay_p_apply_accurate_seek(p); /* apply videoPTS after decoder flish */
        bplay_p_clear_buffer(p);
        p->state.state = eTransition;
        /* start data flowing */
        b_play_next_frame(p);
    }
    /* PR 21643 - The eWaitingRecord exception was added because if you pause immediately after starting a timeshift record,
    you could be in waiting record state. we need to restart. */
    else if (restart_data_flow)
    {
        BDBG_MSG(("Restart: start data flowing"));
        b_play_next_frame(p);
    }
    else if (p->state.state == eWaitingRecord)
    {
        BDBG_MSG(("Restart: start data flowing in eWaitingRecord"));
        if (p->state.read_size==0) {
            /* we are waiting for an index, try to get new entry */
            b_play_next_frame(p);
        }
        else {
           /* we are waiting for a data */
            p->state.state = eWaitingIo;
            NEXUS_File_AsyncRead(p->file->file.data, p->state.buf, p->state.read_size, NEXUS_MODULE_SELF, b_play_frame_data, p);
        }
    }
    else if (p->state.state == eIoCanceled)  {
        BDBG_MSG(("Restart: I/O was canceled, reviving it"));
        p->state.state = eWaitingIo;
        switch(p->state.io_size) {
        case B_MEDIA_ATOM_MAGIC:
            b_play_media_send_meta(p);
            break;
        case B_MEDIA_NEST_MAGIC:
            b_play_next_frame(p);
            break;
       default:
            b_play_frame_data(p, p->state.io_size);
            break;
        }
    }
    else if (p->state.state == eWaitingPlayback ||
             p->state.state == eTimer)
    {
        /* This section of code needs to handle all normal states that require no change. */
        BDBG_MSG(("Restart: no state change state=%d,mode=%d", p->state.state, p->state.mode));
    }
    else {
        /* If this occurs, something is broken in the state machine and must be fixed immediately. This
        is complex code and small changes can lead to disastrous lockups and reentrancy. */
        BDBG_ERR(("Restart: unknown state %d", p->state.state));
    }

    bplay_p_apply_accurate_seek(p); /* call so don't keep stale target videoPTS */
    p->state.decoder_flushed = false;
    b_play_check_buffer(p);
    return;
}

/* this function is used to abort playback if trick/play/seek functions have failed, it's should be called instead of bplay_restart_playback */
void
NEXUS_Playback_P_AbortPlayback(NEXUS_PlaybackHandle p)
{
    BDBG_WRN(("NEXUS_Playback_P_AbortPlayback: %p aborting", (void*)p));
    if (p->state.start_paused) {
        /* if playback wasn't started yet, just do nothing. we should be in WaitingPlayback state. */
        BDBG_ASSERT(p->state.state == eWaitingPlayback);
        return;
    }

    /* save playback position */
    b_play_update_location(p);
    bmedia_player_tell(p->media_player, &p->state.abortPosition);

    b_play_flush(p);
    if(p->state.media.entry.atom) {
        batom_release(p->state.media.entry.atom);
        p->state.media.entry.atom = NULL;
    }
    p->state.state = eAborted;
    p->state.decoder_flushed = false;
    return;
}


/* rest of functions is a public API to control playback */
NEXUS_Error
NEXUS_Playback_Play(NEXUS_PlaybackHandle p)
{
    NEXUS_Error rc;
    BDBG_OBJECT_ASSERT(p, NEXUS_Playback);

    if (p->state.start_paused) {
        BDBG_MSG(("NEXUS_Playback_Play: %#lx starting playback from paused state", (unsigned long)p));
        p->state.start_paused = false;
        p->state.data_source(p);
        return NEXUS_SUCCESS;
    }

    if (p->state.mode == NEXUS_PlaybackState_ePlaying) {
        BDBG_WRN(("Already in the playback mode"));
        return NEXUS_SUCCESS;
    }

    rc = bplay_sync_playback(p);
    if (rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc); goto done; }

    rc = bplay_p_play(p, &p->state.reset, false);
    if (rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);} /* fall through so bplay_restart_playback is called */

    if(rc==NEXUS_SUCCESS || p->params.seekErrorHandling==NEXUS_PlaybackErrorHandlingMode_eIgnore) {
        bplay_restart_playback(p, rc==NEXUS_SUCCESS);
    } else {
        NEXUS_Playback_P_AbortPlayback(p);
        rc = BERR_TRACE(rc);
    }
done:
    return rc;
}


NEXUS_Error
NEXUS_Playback_Pause(NEXUS_PlaybackHandle p)
{
    NEXUS_Error rc=NEXUS_SUCCESS;

    BDBG_OBJECT_ASSERT(p, NEXUS_Playback);

    if (p->state.start_paused) {
        BDBG_MSG(("NEXUS_Playback_Play: %#lx starting playback from paused state", (unsigned long)p));
        p->state.start_paused = false;
        p->state.data_source(p);
    } else if (p->state.mode == NEXUS_PlaybackState_ePaused) {
        BDBG_WRN(("Already in pause mode"));
        goto done;
    }

    rc = bplay_p_pause(p);
    if (rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);} /* fall through */

done:
    return rc;
}

NEXUS_Error
NEXUS_Playback_FrameAdvance(NEXUS_PlaybackHandle p, bool forward)
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(p, NEXUS_Playback);

    if (p->state.start_paused) {
        BDBG_MSG(("NEXUS_Playback_FrameAdvance: %#lx starting playback from paused state", (unsigned long)p));
        p->state.start_paused = false;
        rc = bplay_p_pause(p);
        if (rc) return BERR_TRACE(rc);
    }
    else if (p->state.mode != NEXUS_PlaybackState_ePaused) {
        BDBG_WRN(("Not in paused state"));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto done;
    }
    p->state.seekPositionValid = false;

    rc = bplay_sync_playback(p);
    if (rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc); goto done; }

    rc = bplay_p_frameadvance(p, forward, &p->state.reset);
    if (rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);} /* fall through so bplay_restart_playback is called */

    bplay_restart_playback(p, rc==BERR_SUCCESS && !p->state.drain_mode);

done:
    return rc;
}


static NEXUS_Error
b_play_set_media_player_config(NEXUS_PlaybackHandle p, const NEXUS_PlaybackTrickModeSettings *params)
{
    NEXUS_Error rc;
    bmedia_player_decoder_config config;

    BDBG_CASSERT( NEXUS_PlaybackHostTrickMode_eNone == (NEXUS_PlaybackHostTrickMode)bmedia_player_host_trick_mode_auto);
    BDBG_CASSERT( NEXUS_PlaybackHostTrickMode_eNormal ==  (NEXUS_PlaybackHostTrickMode)bmedia_player_host_trick_mode_normal);
    BDBG_CASSERT( NEXUS_PlaybackHostTrickMode_ePlayI == (NEXUS_PlaybackHostTrickMode)bmedia_player_host_trick_mode_I);
    BDBG_CASSERT( NEXUS_PlaybackHostTrickMode_ePlaySkipB == (NEXUS_PlaybackHostTrickMode)bmedia_player_host_trick_mode_skipB);
    BDBG_CASSERT( NEXUS_PlaybackHostTrickMode_ePlayIP == (NEXUS_PlaybackHostTrickMode)bmedia_player_host_trick_mode_IP);
    BDBG_CASSERT( NEXUS_PlaybackHostTrickMode_ePlaySkipP == (NEXUS_PlaybackHostTrickMode)bmedia_player_host_trick_mode_skipP);
    BDBG_CASSERT( NEXUS_PlaybackHostTrickMode_ePlayBrcm == (NEXUS_PlaybackHostTrickMode)bmedia_player_host_trick_mode_brcm);
    BDBG_CASSERT( NEXUS_PlaybackHostTrickMode_ePlayDqt == (NEXUS_PlaybackHostTrickMode)bmedia_player_host_trick_mode_gop);
    BDBG_CASSERT( NEXUS_PlaybackHostTrickMode_ePlayDqtIP == (NEXUS_PlaybackHostTrickMode)bmedia_player_host_trick_mode_gop_IP);
    BDBG_CASSERT( NEXUS_PlaybackHostTrickMode_ePlayMultiPassDqt == (NEXUS_PlaybackHostTrickMode)bmedia_player_host_trick_mode_mdqt);
    BDBG_CASSERT( NEXUS_PlaybackHostTrickMode_ePlayMultiPassDqtIP == (NEXUS_PlaybackHostTrickMode)bmedia_player_host_trick_mode_mdqt_IP);
    BDBG_CASSERT( NEXUS_PlaybackHostTrickMode_eTimeSkip == (NEXUS_PlaybackHostTrickMode)bmedia_player_host_trick_mode_time_skip);

    bmedia_player_get_decoder_config(p->media_player, &config);
    b_play_update_media_player_config(p, &config);

    if (params->mode != NEXUS_PlaybackHostTrickMode_eNone && params->skipControl == NEXUS_PlaybackSkipControl_eDecoder) {
        switch(params->mode) {
        case NEXUS_PlaybackHostTrickMode_eNormal:
        case NEXUS_PlaybackHostTrickMode_ePlayI:
        case NEXUS_PlaybackHostTrickMode_ePlayIP:
            config.host_mode = bmedia_player_host_trick_mode_auto;
            break;
        default:
            /* decoder doesn't support other skip modes */
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else {
        config.host_mode = params->mode;
    }

    config.mode_modifier = params->mode_modifier;
    config.max_decoder_rate = params->maxDecoderRate/(NEXUS_NORMAL_PLAY_SPEED/BMEDIA_TIME_SCALE_BASE);
    if (params->mode != NEXUS_PlaybackHostTrickMode_eNone) {
        config.stc = (params->rateControl == NEXUS_PlaybackRateControl_eStream && HAS_STC_CHANNEL(p));
    }
    else {
        /* p->params.stcTrick is only used for automatic trick modes */
        config.stc = bplay_p_use_stc_trick(p);
    }
    config.brcm = params->brcmTrickMode;
    rc = bmedia_player_set_decoder_config(p->media_player, &config);
    if (rc) return BERR_TRACE(rc);

    return rc;
}


NEXUS_Error
NEXUS_Playback_TrickMode(NEXUS_PlaybackHandle p, const NEXUS_PlaybackTrickModeSettings *params)
{
    NEXUS_Error rc;
    int new_direction;
    b_trick_settings trick_settings;
    bmedia_player_decoder_mode mode;
    NEXUS_PlaybackTrickModeSettings modified_params;

    BDBG_OBJECT_ASSERT(p, NEXUS_Playback);
    if (params->mode >= NEXUS_PlaybackHostTrickMode_eMax) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if (params->maxDecoderRate > 4 * NEXUS_NORMAL_PLAY_SPEED) {
        BDBG_WRN(("maxDecoderRate %d is greater than max of 4000.", params->maxDecoderRate));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    if (params->skipControl == NEXUS_PlaybackSkipControl_eDecoder || params->rateControl == NEXUS_PlaybackRateControl_eDecoder) {
        if (!b_play_get_video_decoder(p) && !b_play_has_audio_decoder(p)) {
            BDBG_ERR(("NEXUS_Playback_TrickMode failed because no video or audio decoder was provided in any NEXUS_PlaybackPidChannelSettings."));
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
    }

    if (params->mode == NEXUS_PlaybackHostTrickMode_eNormal
        && params->rateControl == NEXUS_PlaybackRateControl_eStream
        && params->rate > (int)params->maxDecoderRate)
    {
        BDBG_WRN(("changing host trick mode from 'None' to 'TimeSkip' because rate control is stream and rate > maxDecoderRate"));
        modified_params = *params;
        modified_params.mode = NEXUS_PlaybackHostTrickMode_eTimeSkip;
        params = &modified_params;
    }

    /* validate params based on automatic or custom mode */
    if (params->mode == NEXUS_PlaybackHostTrickMode_eNone) {
        /* in automatic mode, validate all custom trick params are left at default values */
        if (params->rateControl != NEXUS_PlaybackRateControl_eDecoder ||
            params->skipControl != NEXUS_PlaybackSkipControl_eHost ||
            params->mode_modifier != 1)
        {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        if(params->rate < 0 || (params->rate > (int)params->maxDecoderRate)) { /* if host trick play */
            if (!b_play_get_video_decoder(p)) {
                BDBG_ERR(("NEXUS_Playback_TrickMode fast-forward/rewind requires video stream, parhaps no video decoder was provided in any NEXUS_PlaybackPidChannelSettings."));
                return BERR_TRACE(NEXUS_INVALID_PARAMETER);
            }
        }
        new_direction = (params->rate >= 0)?1:-1;
    }
    else {
        /* verify custom mode params */
        if (params->skipControl == NEXUS_PlaybackSkipControl_eDecoder &&
            params->mode_modifier != 1) {
            /* decoder can't do anything with the mode modifier, so ensure its default */
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        if (params->mode_modifier >= 0 && params->rate < 0) {
            /* do not make this conversion. fix the app. */
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        if (params->mode_modifier < 0 && params->rate >= 0) {
            /* grandfather this conversion */
            BDBG_WRN(("changing trick rate from %d to %d because mode_modifier is negative", params->rate, -1*params->rate));
            modified_params = *params;
            modified_params.rate = -modified_params.rate;
            params = &modified_params;
        }
        /* we may support negative eStream rates in the future using SW STC. */
        new_direction = (params->rate >= 0)?1:-1;
    }

    rc = bplay_sync_playback(p);
    if (rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc); goto done; }

    BDBG_CASSERT( (256 * NEXUS_NORMAL_PLAY_SPEED)/BMEDIA_TIME_SCALE_BASE == 256 * (NEXUS_NORMAL_PLAY_SPEED/BMEDIA_TIME_SCALE_BASE));

    p->state.start_paused = false;

    b_play_update_location(p);

    rc = b_play_set_media_player_config(p, params);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error;}

    /* set host player rate */
    {
        bmedia_player_step direction;
        bmedia_time_scale time_scale;
        int player_rate;

        if (params->skipControl == NEXUS_PlaybackSkipControl_eDecoder) {
            /* if skipControl is eDecoder, then we aren't skipping in the host. */
            player_rate = NEXUS_NORMAL_PLAY_SPEED;
        }
        else {
            player_rate = params->rate;
        }

        direction = (33*player_rate)/NEXUS_NORMAL_PLAY_SPEED;
        if (player_rate && !direction) { /* prevent rounding down to zero */
            direction = player_rate>0?1:-1;
        }
        time_scale = player_rate/(NEXUS_NORMAL_PLAY_SPEED/BMEDIA_TIME_SCALE_BASE);
        if (player_rate && !time_scale) { /* prevent rounding down to zero */
            time_scale = player_rate>0?1:-1;
        }

        rc = bmedia_player_set_direction(p->media_player, direction, time_scale, &mode);
        if(rc!=BERR_SUCCESS) {
            if (!HAS_INDEX(p)) {
                BDBG_ERR(("trick mode not supported without index"));
            }
            else {
                rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);
            }
            goto error;
        }
    }

    /* set decoder/stc rate */
    b_play_trick_get(p, &trick_settings);

    NEXUS_Playback_P_ConvertPlayerDecodeMode(p, &mode, &trick_settings, params);
    trick_settings.forward = (new_direction==1);
    trick_settings.avoid_flush = (params->avoidFlush || !HAS_INDEX(p));
    trick_settings.force_source_frame_rate = mode.force_source_frame_rate;
    trick_settings.maxFrameRepeat = params->maxFrameRepeat;

    rc = b_play_trick_set(p, &trick_settings);
    if (rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc); } /* fall through. we need to maintain internal Playback state, even on a bad trick mode setting. */

    if(mode.discontinuity) {
        b_play_flush(p);
    }

    if (p->state.decoder_flushed) {
        p->state.reset = true;
    }

    if (rc==NEXUS_SUCCESS) {
        p->state.trickmode_params = *params;
        p->state.direction = new_direction;
        p->state.mode = NEXUS_PlaybackState_eTrickMode;
    }

error:
    if(rc==NEXUS_SUCCESS || p->params.seekErrorHandling==NEXUS_PlaybackErrorHandlingMode_eIgnore) {
        bplay_restart_playback(p, rc==NEXUS_SUCCESS);
    } else {
        NEXUS_Playback_P_AbortPlayback(p);
        rc = BERR_TRACE(rc);
    }

done:
    if(!rc && params->rate == -NEXUS_NORMAL_PLAY_SPEED) {
        p->state.frame_advance = NEXUS_Playback_P_FrameAdvance_Reverse;
    } else if(!rc && trick_settings.state == b_trick_state_normal) {
        p->state.frame_advance = NEXUS_Playback_P_FrameAdvance_Forward;
    } else {
        p->state.frame_advance = NEXUS_Playback_P_FrameAdvance_Invalid;
    }
    return rc;
}

NEXUS_Error
NEXUS_Playback_Seek(NEXUS_PlaybackHandle p, NEXUS_PlaybackPosition position)
{
    NEXUS_Error rc;
    int result;

    BDBG_OBJECT_ASSERT(p, NEXUS_Playback);
    rc = bplay_sync_playback(p);
    if(rc!=NEXUS_SUCCESS) {rc = BERR_TRACE(rc); goto done;}

    if(p->state.accurateSeek.state != b_accurate_seek_state_idle) {
        BDBG_WRN(("NEXUS_Playback_Seek:%p unexpected state:%u", (void *)p, p->state.accurateSeek.state));
        p->state.accurateSeek.state = b_accurate_seek_state_idle;
    }

    if (!b_play_get_video_decoder(p) && !b_play_has_audio_decoder(p)) {
        /* no-decoder playback is valid, so this is only a WRN */
        BDBG_WRN(("No video or audio decoder was provided with NEXUS_PlaybackPidChannelSettings; therefore NEXUS_Playback_Seek will not be precise."));
    }

    p->state.seekPositionValid = false;
    result = bmedia_player_seek(p->media_player, position);
    if(result!=-1) {
        p->state.seekPositionValid = true;
        p->state.seekPosition = position;
    }

    if (result!=-1 && p->params.accurateSeek && !p->state.start_paused) {
        p->state.seekPositionValid = false;
        bplay_p_accurate_seek(p, position);
    }

    if (result!=-1) {
        p->state.reset = true;
    }
    else {
        /* don't use BERR_TRACE. failed seek is normal. */
        rc = NEXUS_UNKNOWN;
    }
    if(rc!=NEXUS_SUCCESS) {
        if(p->params.seekErrorHandling==NEXUS_PlaybackErrorHandlingMode_eAbort) {
            NEXUS_Playback_P_AbortPlayback(p);
            rc = BERR_TRACE(rc);
            goto done;
        }
    }

    bplay_restart_playback(p, false);
done:
    return rc;
}

NEXUS_Error
NEXUS_Playback_GetStatus(NEXUS_PlaybackHandle p, NEXUS_PlaybackStatus *status)
{
    NEXUS_PlaypumpStatus pumpstatus;
    NEXUS_Error rc=NEXUS_SUCCESS;
    bmedia_player_status player_status;
    bool need_checkbuffer = false;

    BKNI_Memset(status, 0, sizeof(*status));
    NEXUS_Playback_GetDefaultTrickModeSettings(&status->trickModeSettings);
    switch(p->state.state) {
    case eStopped:
        status->state = NEXUS_PlaybackState_eStopped;
        break;
    case eAborted:
        status->state = NEXUS_PlaybackState_eAborted;
        break;

    default:
        status->state = p->state.mode;
        status->trickModeSettings = p->state.trickmode_params;
        if(!p->state.start_paused) {
            need_checkbuffer = true;
        } else {
            status->state = NEXUS_PlaybackState_ePaused;
        }
        if(status->state == NEXUS_PlaybackState_ePaused) {
            status->trickModeSettings.rate = 0;
        }
        break;
    }

    if (p->state.state != eStopped) {
        BDBG_ASSERT(p->params.playpump);
        rc = NEXUS_Playpump_GetStatus(p->params.playpump, &pumpstatus);
        if(rc==NEXUS_SUCCESS) {
            status->fifoDepth = pumpstatus.fifoDepth;
            status->fifoSize = pumpstatus.fifoSize;
            status->bytesPlayed = pumpstatus.bytesPlayed;
        }
        bmedia_player_get_status(p->media_player, &player_status);
        status->first = player_status.bounds.first;
        status->last = player_status.bounds.last;
        status->indexErrors = p->state.index_error_cnt + player_status.index_error_cnt;
        status->dataErrors = p->state.data_error_cnt + player_status.data_error_cnt;
        status->readPosition = player_status.position;

        if (p->state.state != eAborted) {
            if(!p->state.start_paused) {
                b_play_update_location(p);
            }
            if(p->state.mode == NEXUS_PlaybackState_ePaused && p->state.seekPositionValid) {
                status->position = p->state.seekPosition;
            } else {
                status->position = b_play_correct_position(p, &player_status);
            }
        }
        else {
            status->position = p->state.abortPosition;
        }
    }
    if(need_checkbuffer) {
        b_play_check_buffer(p);
    }

    return rc;
}

void
NEXUS_Playback_GetDefaultStartSettings(NEXUS_PlaybackStartSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->mode = NEXUS_PlaybackMode_eIndexed;
    pSettings->bitrate = 10 * 1000 * 1000;
    pSettings->mpeg2TsIndexType = NEXUS_PlaybackMpeg2TsIndexType_eAutoDetect;
    pSettings->indexFileIo.mode = NEXUS_PlaybackIndexFileIo_eCallback;
    NEXUS_Thread_GetDefaultSettings(&pSettings->indexFileIo.threadSettings);
    return;
}

static void
NEXUS_Playback_P_PlaypumpThread(void *context)
{
    NEXUS_PlaybackHandle playback = context;
    for(;;) {
        BERR_Code rc;
        BDBG_OBJECT_ASSERT(playback, NEXUS_Playback);
        BDBG_ASSERT(playback->playpump_thread_event);
        rc=BKNI_WaitForEvent(playback->playpump_thread_event, 100);
        if(!(rc==BERR_SUCCESS || rc==BERR_TIMEOUT)) {
            rc=BERR_TRACE(rc);
            break;
        }
        NEXUS_LockModule();
        BDBG_ASSERT(playback->playpump_thread_event);
        if(playback->thread_terminate) {
            NEXUS_UnlockModule();
            break;
        }
        if(rc==BERR_SUCCESS && playback->state.state != eStopped) {
            b_play_read_callback(playback);
        }
        NEXUS_UnlockModule();
    }
    BDBG_MSG(("NEXUS_Playback_P_PlaypumpThread: %p terminated", (void*)playback));
    return;
}

NEXUS_Error
NEXUS_Playback_Start(NEXUS_PlaybackHandle playback, NEXUS_FilePlayHandle file, const NEXUS_PlaybackStartSettings *pSettings)
{
    NEXUS_PlaypumpStatus playpump_status;
    NEXUS_Error rc=NEXUS_SUCCESS;
    NEXUS_PlaybackStartSettings defaultStartSettings;

    if (!file) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    NEXUS_Module_Lock(g_NEXUS_PlaybackModulesSettings.modules.file);
    rc = NEXUS_FilePlay_Lock_priv(file);
    NEXUS_Module_Unlock(g_NEXUS_PlaybackModulesSettings.modules.file);
    if (rc) {
        return BERR_TRACE(rc);
    }
    BDBG_OBJECT_ASSERT(playback, NEXUS_Playback);

    if(!pSettings) {
        NEXUS_Playback_GetDefaultStartSettings(&defaultStartSettings);
        pSettings = &defaultStartSettings;
    }
    b_play_capture_open(playback);

    playback->state.media.entry.atom = NULL;
    playback->state.frame.valid = false;
    playback->state.navTrailingMode = false;

    if (playback->state.state != eStopped) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error_state;}
    if (playback->params.playpump == NULL)  { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error_state;}
    if (file->file.data == NULL) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error_state;}

    BDBG_MSG(("NEXUS_Playback_Start: %p", (void *)playback));
    BKNI_Memset(&playback->state, 0, sizeof(playback->state)); /* wipe-out all temporary state */
    playback->state.validPts = false;
    playback->state.drain_mode = false;
    playback->state.frame_advance = NEXUS_Playback_P_FrameAdvance_Forward;
    playback->state.seekPositionValid = false;
    playback->state.seekPosition = 0;


    switch(pSettings->indexFileIo.mode) {
    case NEXUS_PlaybackIndexFileIo_eCallback:
        /* do nothing */
        break;
    case NEXUS_PlaybackIndexFileIo_eModule:
        if(playback->playpump_event == NULL) {
            rc = BKNI_CreateEvent(&playback->playpump_event);
            if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto error_index_mode;}
            playback->playpump_event_callback = NEXUS_RegisterEvent(playback->playpump_event, b_play_read_callback, playback);
            if(playback->playpump_event_callback==NULL) {
                rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
                BKNI_DestroyEvent(playback->playpump_event);
                playback->playpump_event=NULL;
                goto error_index_mode;
            }
        }
        break;
    case NEXUS_PlaybackIndexFileIo_eThread:
        if(playback->playpump_thread_event == NULL) {
            rc = BKNI_CreateEvent(&playback->playpump_thread_event);
            if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error_index_mode;}
        }
        if(playback->playpump_thread) {
            if(BKNI_Memcmp(&playback->playpump_thread_settings, &pSettings->indexFileIo.threadSettings, sizeof(pSettings->indexFileIo.threadSettings))==0) {
                break; /* thread configuration is the same, keep on using old thread */
            }
            NEXUS_Playback_P_KillPlaypumpThread(playback);
        }
        {
            char thread_name[32];
            BKNI_Snprintf(thread_name, sizeof(thread_name), "nx_playback%p", (void *)playback);
            playback->playpump_thread = NEXUS_Thread_Create(thread_name, NEXUS_Playback_P_PlaypumpThread, playback, &pSettings->indexFileIo.threadSettings);
            if(!playback->playpump_thread) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error_index_mode; }
        }
        break;
    default:
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto error_index_mode;
    }
    playback->index_file_mode = pSettings->indexFileIo.mode;

    b_play_trick_init(playback);
    /* trick must know about stcTrick now in case of pause */
    playback->trick.settings.stc_trick = bplay_p_use_stc_trick(playback);
    /* reset any currently attached decoders */
    rc = b_play_trick_set(playback, NULL);
    if (rc) {rc = BERR_TRACE(rc); goto error_state;}

    playback->state.state = eStopped;
    playback->file = file;
    NEXUS_Time_Get(&playback->state.fifoLast); /* get init value */

    rc = NEXUS_Playpump_GetStatus(playback->params.playpump, &playpump_status);
    if (rc) {rc = BERR_TRACE(rc); goto error_state;}

    playback->buf_limit = (uint8_t*)playpump_status.bufferBase + playpump_status.fifoSize - B_IO_BLOCK_LIMIT;
    BDBG_MSG(("playback buffer %p(%u)",
        (void *)playpump_status.bufferBase, (unsigned)playpump_status.fifoSize));

    bio_read_attach_priority(playback->file->file.data, NEXUS_P_Playback_GetPriority, playback);

    playback->state.mode = NEXUS_PlaybackState_ePlaying;
    playback->state.encrypted = false ; /* mpeg->encryption.type != bencryption_type_none; */

    playback->state.state = eTransition;
    playback->media_player = NULL;
    /* seek to the beginning of the file */
    playback->file->file.data->seek(playback->file->file.data, 0, SEEK_SET);

    rc = b_play_start_media(playback, file, &playpump_status, pSettings);
    if(rc!=NEXUS_SUCCESS) { rc = BERR_TRACE(rc); goto error_player;}

    NEXUS_StartCallbacks(playback->params.playpump);
    /* assume that playback and decode will start in normal play */
    NEXUS_Playback_GetDefaultTrickModeSettings(&playback->state.trickmode_params);

    playback->state.state = eWaitingPlayback;
    playback->state.start_paused = playback->params.startPaused;
    rc = NEXUS_Playpump_Start(playback->params.playpump);
    if (rc!=NEXUS_SUCCESS) { rc = BERR_TRACE(rc); goto error_playpump; }

    return BERR_SUCCESS;

error_playpump:
    b_play_stop_media(playback);
error_player:
error_state:
error_index_mode:
    playback->state.state = eStopped;
    NEXUS_Module_Lock(g_NEXUS_PlaybackModulesSettings.modules.file);
    NEXUS_FilePlay_Unlock_priv(file);
    NEXUS_Module_Unlock(g_NEXUS_PlaybackModulesSettings.modules.file);
    /* On any Start failure, we must end in a clean eStopped state that can be restarted. */
    return rc;
}

void
NEXUS_Playback_GetSettings(NEXUS_PlaybackHandle playback, NEXUS_PlaybackSettings *settings)
{
    BDBG_OBJECT_ASSERT(playback, NEXUS_Playback);
    BDBG_ASSERT(settings);
    *settings = playback->params;
    return;
}

static NEXUS_Error
b_play_playpump_read_callback_guard(void *context)
{
    NEXUS_PlaybackHandle playback = (NEXUS_PlaybackHandle)context;


    BDBG_OBJECT_ASSERT(playback, NEXUS_Playback);
    /* no need to acquire lock here, since this mode is only changing when starting playback */
    if(playback->state.state == eStopped) {
        return NEXUS_CALLBACK_DEFLECTED;
    }
    switch(playback->index_file_mode) {
    case NEXUS_PlaybackIndexFileIo_eCallback:
        return NEXUS_SUCCESS;


        break;
    case NEXUS_PlaybackIndexFileIo_eModule:
        BKNI_SetEvent(playback->playpump_event); /* set event and do work inside module's context */
        break;
    case NEXUS_PlaybackIndexFileIo_eThread:
        BKNI_SetEvent(playback->playpump_thread_event); /* set event and do work inside thread */
        break;
    default:
        BDBG_ASSERT(0);
        break;
    }
    return NEXUS_CALLBACK_DEFLECTED;
}

static void
b_play_playpump_read_callback_locked(void *context)
{
    b_play_read_callback(context);
}

NEXUS_Error
NEXUS_Playback_SetSettings(NEXUS_PlaybackHandle playback, const NEXUS_PlaybackSettings *settings)
{
    NEXUS_Error  rc;

    BDBG_OBJECT_ASSERT(playback, NEXUS_Playback);
    BDBG_ASSERT(settings);

    if(settings->endOfStreamAction >= NEXUS_PlaybackLoopMode_eMax) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if(settings->beginningOfStreamAction >= NEXUS_PlaybackLoopMode_eMax) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if(settings->playErrorHandling>=NEXUS_PlaybackErrorHandlingMode_eMax) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if(settings->seekErrorHandling>=NEXUS_PlaybackErrorHandlingMode_eMax) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if(settings->trickErrorHandling>=NEXUS_PlaybackErrorHandlingMode_eMax) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if(settings->playErrorHandling!=NEXUS_PlaybackErrorHandlingMode_eEndOfStream
        && settings->playErrorHandling!=NEXUS_PlaybackErrorHandlingMode_eAbort) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    if(settings->frameReverse.gopDepth) {
        if (settings->playpumpSettings.transportType != NEXUS_TransportType_eTs) {
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
    }

    if (settings->stcTrick && !settings->stcChannel && !settings->simpleStcChannel) {
        BDBG_ERR(("NEXUS_PlaybackSettings.stcTrick requires NEXUS_PlaybackSettings.stcChannel"));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if (settings->stcTrick != playback->params.stcTrick && playback->state.state != eStopped) {
        BDBG_ERR(("You must set NEXUS_PlaybackSettings.stcTrick before calling NEXUS_Playback_Start"));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if (playback->state.state != eStopped && (playback->state.mode == NEXUS_PlaybackState_eTrickMode || playback->state.trickmode_params.rate != NEXUS_NORMAL_PLAY_SPEED)) {
        if(settings->stcChannel != playback->params.stcChannel || settings->simpleStcChannel != playback->params.simpleStcChannel) {
            BDBG_ERR(("Cannot change Playback's stcChannel during a trick mode."));
            rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
            goto err_settings;
        }
    }

    /* settings that can't change if we are started or there's a pid opened */
    if(playback->state.state != eStopped || BLST_S_FIRST(&playback->pid_list)) {
        if (settings->playpump != playback->params.playpump) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);goto err_settings;}
        /* in playpumpSettings, the only setting that's currently sensitive to pidchannels being opened is transportType. this could change. */
        if (settings->playpumpSettings.transportType != playback->params.playpumpSettings.transportType) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);goto err_settings;}
    }

        if(settings->playpump) {
           NEXUS_PlaypumpSettings pumpCfg = settings->playpumpSettings;
           NEXUS_PlaypumpSettings actualCfg;

           NEXUS_Playpump_GetSettings(settings->playpump, &actualCfg);
           pumpCfg.mode = actualCfg.mode; /* modified by nexus_playback_media, but not propagated to app. */

            /* replace dataCallback, and don't leak it outside */
           NEXUS_CallbackHandler_PrepareCallback(playback->dataCallbackHandler, pumpCfg.dataCallback);
           pumpCfg.originalTransportType = NEXUS_TransportType_eUnknown;
           switch(pumpCfg.transportType) {
#if 0
           case NEXUS_TransportType_eMp4:
           case NEXUS_TransportType_eMkv:
               pumpCfg.originalTransportType = pumpCfg.transportType;
               pumpCfg.transportType = NEXUS_TransportType_eMpeg2Pes;
               break;
#endif
           case NEXUS_TransportType_eEs:
                if(settings->enableStreamProcessing) {
                    pumpCfg.originalTransportType = pumpCfg.transportType;
                    pumpCfg.transportType = NEXUS_TransportType_eMpeg2Pes;
                }
               break;
           default:
               break;
           }

           playback->actualTransportType = pumpCfg.transportType;
           rc = NEXUS_Playpump_SetSettings(settings->playpump, &pumpCfg);
           if(rc!=NEXUS_SUCCESS) { rc = BERR_TRACE(rc); goto err_playpump; }
        }

    /* cannot call b_play_trick_set here. it is init'd in Start */

    NEXUS_TaskCallback_Set(playback->errorCallback, &settings->errorCallback);
    NEXUS_TaskCallback_Set(playback->endOfStreamCallback, &settings->endOfStreamCallback);
    NEXUS_TaskCallback_Set(playback->beginningOfStreamCallback, &settings->beginningOfStreamCallback);
    NEXUS_TaskCallback_Set(playback->parsingErrorCallback, &settings->parsingErrorCallback);

    if(playback->state.state != eStopped && (!settings->timeshifting && playback->params.timeshifting)) {
        playback->params.timeshifting = false;  /* clear timeshifting flag and simulate notification from record */
        NEXUS_Playback_RecordProgress_priv(playback);
    }

    playback->params = *settings;
    return NEXUS_SUCCESS;
err_playpump:
err_settings:
    return rc;
}

void
NEXUS_Playback_Stop(NEXUS_PlaybackHandle playback)
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(playback, NEXUS_Playback);
    BDBG_MSG(("NEXUS_Playback_Stop: %p", (void *)playback));
    if(playback->state.state == eStopped) {
        goto done;
    }

    /* if aborted, we can try to stop. */
    if(playback->state.state != eAborted) {
        rc = NEXUS_Playback_P_CheckWaitingIo(playback, true);
        if (rc) {
            BDBG_ERR(("Playback unable to stop. System is now unstable."));
            rc = BERR_TRACE(rc);
        }
    }

    /* stop timer before unlocking module so that no additional calls to Playpump are made after NEXUS_Playpump_Stop */
    if( playback->state.timer ) {
        BDBG_MSG(("Timer still active, cancelling %p", (void *)playback->state.timer ));
        NEXUS_CancelTimer(playback->state.timer);
        playback->state.timer = NULL;
    }
#if NEXUS_PLAYBACK_BLOCKAUTH
	NEXUS_Playback_P_BlockAuthStop(playback);
#endif

    BDBG_ASSERT(playback->params.playpump);
    NEXUS_Playpump_Stop(playback->params.playpump);


    b_play_stop_media(playback);
    b_play_trick_shutdown(playback);
    b_play_capture_close(playback);
    playback->state.accurateSeek.state = b_accurate_seek_state_idle;
    playback->state.state = eStopped;

    NEXUS_StopCallbacks(playback->params.playpump);
    NEXUS_CallbackHandler_Stop(playback->dataCallbackHandler);
    NEXUS_CallbackHandler_Stop(playback->videoDecoderFirstPts);
    NEXUS_CallbackHandler_Stop(playback->videoDecoderFirstPtsPassed);

    NEXUS_Module_Lock(g_NEXUS_PlaybackModulesSettings.modules.file);
    NEXUS_FilePlay_Unlock_priv(playback->file);
    NEXUS_Module_Unlock(g_NEXUS_PlaybackModulesSettings.modules.file);

done:
    return ;
}

void NEXUS_Playback_IsTransportTypeSupported( NEXUS_PlaybackHandle playback, NEXUS_TransportType transportType, bool *pIsSupported )
{
    BDBG_OBJECT_ASSERT(playback, NEXUS_Playback);
    if (playback->params.playpump) {
        switch (transportType) {
        case NEXUS_TransportType_eMp4:
        case NEXUS_TransportType_eMkv:
            *pIsSupported = true;
            break;
        default:
            NEXUS_Playpump_IsTransportTypeSupported(playback->params.playpump, transportType, pIsSupported);
            break;
        }
    }
    else {
        *pIsSupported = false;
    }
}
