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
 **************************************************************************/
#include "nexus_playback_module.h"
#include "nexus_playback_impl.h"
#if NEXUS_HAS_AUDIO
#include "nexus_audio_decoder_trick.h"
#endif

BDBG_MODULE(nexus_playback_trick);

static void b_play_stc_rate(NEXUS_PlaybackHandle p, unsigned increment, unsigned prescale);
static void b_play_stc_invalidate(NEXUS_PlaybackHandle p);

static void
b_play_trick_monitor(void *p_)
{
    NEXUS_PlaybackHandle p = p_;
    BERR_Code rc;
    uint32_t video_pts;
    const NEXUS_Playback_P_PidChannel *pid;
    NEXUS_VideoDecoderStatus status;

    BDBG_OBJECT_ASSERT(p, NEXUS_Playback);
    p->trick.rap_monitor_timer = NULL;
    if (p->state.accurateSeek.state == b_accurate_seek_state_idle) {
        const NEXUS_Playback_P_PidChannel *decode = b_play_get_video_decoder(p);
        if(!decode) { goto done; }
        rc = NEXUS_P_Playback_VideoDecoder_GetStatus(decode, &status);
        if(rc!=NEXUS_SUCCESS) {goto done;}
        if(status.ptsType == NEXUS_PtsType_eCoded || status.ptsType == NEXUS_PtsType_eInterpolatedFromValidPTS || p->params.playpumpSettings.transportType==NEXUS_TransportType_eEs) {
            video_pts = status.pts;
        } else {goto done; }
    } else {
        video_pts = p->state.accurateSeek.videoPts;
    }
    BDBG_MSG(("b_play_trick_monitor: %p, video pts %#x", (void *)p, video_pts));

    for(pid = BLST_S_FIRST(&p->pid_list); pid ; pid = BLST_S_NEXT(pid, link)) {
        if(pid->cfg.pidSettings.pidType==NEXUS_PidType_eAudio) {
            rc = NEXUS_Playback_P_AudioDecoder_Advance(pid, video_pts);
            if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);}
        }
    }

done: /* keep going anyway */
    p->trick.rap_monitor_timer = NEXUS_ScheduleTimer(100, b_play_trick_monitor, p);
    return;
}

void
b_play_flush(NEXUS_PlaybackHandle playback)
{
    NEXUS_Error rc;
    const NEXUS_Playback_P_PidChannel *pid;

    BDBG_MSG(("b_play_flush:> %#lx", (unsigned long)playback));

    playback->state.frame.valid = false;
    playback->state.decoder_flushed = true;
    playback->state.navTrailingMode = false;
    BDBG_ASSERT(playback->params.playpump);
    rc = NEXUS_Playpump_Flush(playback->params.playpump);
    if(rc!=NEXUS_SUCCESS) {rc = BERR_TRACE(rc);}

    for(pid = BLST_S_FIRST(&playback->pid_list); pid ; pid = BLST_S_NEXT(pid, link)) {
        switch(pid->cfg.pidSettings.pidType) {
        default: break;
        case NEXUS_PidType_eVideo: NEXUS_P_Playback_VideoDecoder_Flush(pid); break;
        case NEXUS_PidType_eAudio: NEXUS_P_Playback_AudioDecoder_Flush(pid); break;
        }
    }

    b_play_stc_invalidate(playback);

#if B_PLAYBACK_CAPTURE
    if(playback->capture.has_data) {
        b_play_capture_close(playback);
        b_play_capture_open(playback);
    }
#endif

    BDBG_MSG(("b_play_flush:< %#lx", (unsigned long)playback));
    return;
}

void
b_play_trick_init(NEXUS_PlaybackHandle p)
{
    BKNI_Memset(&p->trick.settings, 0, sizeof(p->trick.settings));
    p->trick.settings.state = b_trick_state_normal;
    p->trick.settings.forward = true;
    p->trick.settings.decode_rate = NEXUS_NORMAL_PLAY_SPEED;
    p->trick.settings.decode_mode = NEXUS_VideoDecoderDecodeMode_eAll;
    p->trick.settings.fragmented = false;
    p->trick.settings.reordering_mode = NEXUS_VideoDecoderReorderingMode_eNone;
    p->trick.rap_monitor_timer = NULL;
    return;
}

void
b_play_trick_shutdown(NEXUS_PlaybackHandle p)
{
    if (p->trick.rap_monitor_timer) {
        NEXUS_CancelTimer(p->trick.rap_monitor_timer);
        p->trick.rap_monitor_timer = NULL;
    }

    b_play_stc_rate(p, 1, 0);
    return;
}

void
b_play_trick_get(NEXUS_PlaybackHandle p, b_trick_settings *cfg)
{
    BDBG_OBJECT_ASSERT(p, NEXUS_Playback);
    *cfg = p->trick.settings;
}

static NEXUS_Error
b_play_trick_set_each_audio(NEXUS_PlaybackHandle p, const NEXUS_Playback_P_PidChannel *pid, const b_trick_settings *settings)
{
    NEXUS_Error rc;
    NEXUS_AudioDecoderTrickState audioState;
    bool forceStopped = false;

    NEXUS_P_Playback_AudioDecoder_GetTrickState(pid, &audioState, NULL);

    if(!settings->stc_trick) {
        /* decoder trick modes */
        if(settings->decode_rate < NEXUS_NORMAL_PLAY_SPEED) {
            audioState.rate = 0;
        }
        else {
            audioState.rate = settings->decode_rate;
        }
        audioState.stcTrickEnabled = false;
    }
    else {
        /* stc trick modes */
        if (settings->decode_rate != 0) {
            /* for non-0, we pass down the rate for DSOLA trick modes. */
            audioState.rate = settings->decode_rate;
        }
        else {
            /* if 0, just do an STC pause, no RAP pause. but we also have to return to mute. */
            audioState.rate = 0;
        }
        audioState.stcTrickEnabled = true;
    }
    if (settings->audio_only_pause) {
        audioState.rate = 0;
    }

    audioState.forceStopped = (settings->state != b_trick_state_normal || forceStopped);
    if(settings->decode_mode!=NEXUS_VideoDecoderDecodeMode_eAll) {
        audioState.forceStopped = true;
    }

    /* mute for stc based pause */
    if ((settings->stc_trick && settings->decode_rate == 0)) {
        audioState.muted = true;
    }
    else {
        audioState.muted = false;
    }

    BDBG_MSG(("audio trick muted=%d, rate=%d, forceStopped=%d", audioState.muted, audioState.rate, audioState.forceStopped));

    audioState.stopForTrick = true;
    rc = NEXUS_P_Playback_AudioDecoder_SetTrickState(pid, &audioState);
    if(rc!=NEXUS_SUCCESS) { rc = BERR_TRACE(rc); goto error;}

    audioState.stopForTrick = false;
    rc = NEXUS_P_Playback_AudioDecoder_SetTrickState(pid, &audioState);
    if(rc!=NEXUS_SUCCESS) { rc = BERR_TRACE(rc); goto error;}

    if((audioState.rate==0 || settings->audio_only_pause) && !settings->stc_trick) {
        if (p->state.audioTrick.primary || p->state.audioTrick.secondary) {
            if(!p->trick.rap_monitor_timer) {
                p->trick.rap_monitor_timer = NEXUS_ScheduleTimer(100, b_play_trick_monitor, p);
            }
        }
    } else {
        if (p->trick.rap_monitor_timer) {
            NEXUS_CancelTimer(p->trick.rap_monitor_timer);
            p->trick.rap_monitor_timer = NULL;
        }
    }

    return NEXUS_SUCCESS;

error:
    return rc;
}

void NEXUS_Playback_P_VideoDecoderFirstPts(void *context)
{
    NEXUS_PlaybackHandle playback = context;

    BDBG_OBJECT_ASSERT(playback, NEXUS_Playback);

    if (playback->state.accurateSeek.state != b_accurate_seek_state_idle) {
        b_trick_settings settings;
        /* video has found its first pts, so now audio can chase */
        /* NOTE: Due to XVD callback order, FirstPtsPassed cback can occur before FirstPts */
        /*       So check if we are in audio pause mode */
        b_play_trick_get(playback, &settings);
        if( settings.audio_only_pause ) {
           /* We got FirstPtsPassed first ! */
           settings.audio_only_pause = false;
           b_play_trick_set(playback, &settings);
           playback->state.accurateSeek.state = b_accurate_seek_state_idle;
           BDBG_WRN(("accurate seek: first pts passed, audio seek done FirstPts"));
        }
        else {
            BDBG_WRN(("accurate seek: first pts, starting audio seek FirstPts"));
            settings.audio_only_pause = true;
            playback->state.accurateSeek.state = b_accurate_seek_state_videoFirstPts;
        }
        b_play_trick_set(playback, &settings);
    }
    return;
}

void NEXUS_Playback_P_VideoDecoderFirstPtsPassed(void *context)
{
    NEXUS_PlaybackHandle playback = context;

    BDBG_OBJECT_ASSERT(playback, NEXUS_Playback);

    if (playback->state.accurateSeek.state != b_accurate_seek_state_idle) {
        b_trick_settings settings;
        b_play_trick_get(playback, &settings);
        if( settings.audio_only_pause ) {
            /* we're done. video has arrived. now audio can start. */
            settings.audio_only_pause = false;
            b_play_trick_set(playback, &settings);
            playback->state.accurateSeek.state = b_accurate_seek_state_idle;
            BDBG_WRN(("accurate seek: first pts passed, audio seek done PtsPassed"));
        }
        else {
            BDBG_WRN(("accurate seek: first pts, starting audio seek PtsPassed"));
            settings.audio_only_pause = true;
            b_play_trick_set(playback, &settings);
        }
    }
    return;
}

NEXUS_Error
b_play_trick_set_pid(NEXUS_PlaybackHandle p, const NEXUS_Playback_P_PidChannel *pid, const b_trick_settings *settings)
{
    NEXUS_Error rc;
    bool slowMotion;

    if (!settings) {
        /* reapply previous settings */
        settings = &p->trick.settings;
    }

    switch (pid->cfg.pidSettings.pidType) {
    case NEXUS_PidType_eVideo:
        {
        NEXUS_VideoDecoderTrickState vdecState;
        NEXUS_VideoDecoderStatus status;
        NEXUS_VideoDecoderPlaybackSettings playbackSettings;

        if (!pid->cfg.pidTypeSettings.video.decoder && !pid->cfg.pidTypeSettings.video.simpleDecoder) {
            /* no decoder can be normal. already checked at NEXUS_Playback_TrickMode. */
            return 0;
        }

        NEXUS_P_Playback_VideoDecoder_GetPlaybackSettings(pid, &playbackSettings);
        if( playbackSettings.firstPts.callback == NULL ) {
            NEXUS_CallbackHandler_PrepareCallback(p->videoDecoderFirstPts, playbackSettings.firstPts);
            NEXUS_CallbackHandler_PrepareCallback(p->videoDecoderFirstPtsPassed, playbackSettings.firstPtsPassed);
            rc = NEXUS_P_Playback_VideoDecoder_SetPlaybackSettings(pid, &playbackSettings);
            if (rc) return BERR_TRACE(rc);
        }

        rc = NEXUS_P_Playback_VideoDecoder_GetStatus(pid, &status);
        if (rc) return BERR_TRACE(rc);
        if (status.tsm && !p->params.stcChannel && pid->cfg.pidTypeSettings.video.decoder) {
            /* This was a recent change. Apps must be fixed. Without this, certain trick mode transitions will fail
            because Playback will be unable to invalidate the STC. */
            BDBG_ERR(("Playback with TSM requires NEXUS_PlaybackSettings.stcChannel."));
            return BERR_TRACE(NEXUS_UNKNOWN);
        }
        if (!status.started) {
            BDBG_MSG(("Playback unable to set VideoDecoder trick state because it has not been started. Playback will continue, but you may not get desired results."));
            /* TODO: consider a callback from VideoDecoder to apply Playback's trick state when it is started */
            return 0;
        }

        NEXUS_P_Playback_VideoDecoder_GetTrickState(pid, &vdecState);
        if((settings->decode_rate!=0 && settings->state == b_trick_state_host_paced) || (settings->stc_trick && settings->state == b_trick_state_normal)) {
            vdecState.rate = NEXUS_NORMAL_PLAY_SPEED;
        } else {
            vdecState.rate = settings->decode_rate;
        }
        vdecState.reorderingMode = settings->reordering_mode;
        vdecState.decodeMode = settings->decode_mode;
        vdecState.brcmTrickModesEnabled = settings->state == b_trick_state_brcm_trick_mode;
        vdecState.hostTrickModesEnabled = settings->state == b_trick_state_host_paced || settings->state == b_trick_state_host_trick_mode;
        vdecState.stcTrickEnabled = settings->stc_trick && (settings->decode_rate != NEXUS_NORMAL_PLAY_SPEED);
        switch (settings->state) {
        case b_trick_state_dqt_mode:
            vdecState.dqtEnabled = NEXUS_VideoDecoderDqtMode_eSinglePass;
            break;
        case b_trick_state_mdqt_mode:
            vdecState.dqtEnabled = NEXUS_VideoDecoderDqtMode_eMultiPass;
            break;
        default:
            vdecState.dqtEnabled = NEXUS_VideoDecoderDqtMode_eDisabled;
            break;
        }
        slowMotion = (-NEXUS_NORMAL_PLAY_SPEED < (int)settings->decode_rate && settings->decode_rate < NEXUS_NORMAL_PLAY_SPEED);
        /* show only top field if decoder expected to repeat frames or do TSM trickmodes */
        vdecState.topFieldOnly = slowMotion || settings->state == b_trick_state_host_paced || settings->simulated_tsm || vdecState.stcTrickEnabled;
        vdecState.reverseFields = !settings->forward;
        if (settings->simulated_tsm) {
            vdecState.tsmEnabled = NEXUS_TsmMode_eSimulated;
            vdecState.rate = settings->decode_rate;
        }
        else {
            vdecState.tsmEnabled = (settings->decode_rate!=0 && settings->state == b_trick_state_host_paced) ||
                (settings->state == b_trick_state_normal &&
                 ((settings->decode_rate >= NEXUS_NORMAL_PLAY_SPEED && settings->decode_mode==NEXUS_VideoDecoderDecodeMode_eAll)
                  || settings->stc_trick)
                );
        }
        vdecState.forceSourceFrameRate = settings->force_source_frame_rate;
        vdecState.maxFrameRepeat = settings->maxFrameRepeat;
        BDBG_MSG(("calling NEXUS_VideoDecoder_SetTrickState: rate=%d %s %s %s %s %s %s %s", (int)vdecState.rate, B_DECODE_MODE_STR(vdecState.decodeMode),
            vdecState.brcmTrickModesEnabled?"brcm-trick":"", vdecState.hostTrickModesEnabled?"host-trick":"", vdecState.dqtEnabled?"dqt":"",
            vdecState.topFieldOnly?"top-field-only":"", vdecState.reverseFields?"reverse-field":"", vdecState.tsmEnabled?"tsm":"vsync"));
        rc = NEXUS_P_Playback_VideoDecoder_SetTrickState(pid, &vdecState);
        if (rc) return BERR_TRACE(rc);
        }
        break;

    case NEXUS_PidType_eAudio:
        rc = b_play_trick_set_each_audio(p, pid, settings);
        if (rc)  return BERR_TRACE(rc);
        break;

    default:
        break;
    }

    return 0;
}

static void remove_common_factors(unsigned val1, unsigned val2, unsigned *pResult1, unsigned *pResult2)
{
    static const unsigned factors[] = {5, 3};
    static const unsigned numFactors = sizeof(factors)/sizeof(unsigned);
    unsigned factor, index;

    /* Remove factors of 2 first to use AND/shift operations instead of div/mod */
    while ( val1 >= 2 && val2 >= 2 )
    {
        if ( (val1 & 1) || (val2 & 1) )
        {
            break;
        }

        val1 >>= 1;
        val2 >>= 1;
    }

    /* Now go through remaining factors */
    for ( index = 0; index < numFactors; index++ )
    {
        factor = factors[index];
        while ( val1 >= factor && val2 >= factor )
        {
            if ( (val1 % factor) || (val2 % factor) )
            {
                break;
            }

            val1 /= factor;
            val2 /= factor;
        }
    }

    *pResult1 = val1;
    *pResult2 = val2;
}


static NEXUS_Error
b_play_trick_stc_set_rate(NEXUS_PlaybackHandle p, unsigned decode_rate)
{
    unsigned denominator,numerator;

    BDBG_MSG(("b_play_trick_stc_set_rate %d", decode_rate));

    if (decode_rate != NEXUS_NORMAL_PLAY_SPEED && !b_play_decoders_in_tsm_mode(p)) {
        BDBG_ERR(("STC trick modes require decoder(s) to be in TSM mode"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    remove_common_factors( decode_rate, NEXUS_NORMAL_PLAY_SPEED, &numerator, &denominator);
    if ( numerator && (numerator > 255 || denominator > 255) ) {
        /* This will occur for prime number numerators between min decode rate and max decode rate, eg 1009/1000 */
        BDBG_MSG(("common Factors > 255, stcTrickmode decode rate not optimal numr=%d denr=%d!", numerator, denominator ));
    }

    b_play_stc_rate(p, numerator, denominator - 1 /* -1 because HW uses denom+1 for prescale val */ );
    return 0;
}

NEXUS_Error
b_play_trick_set(NEXUS_PlaybackHandle p, const b_trick_settings *settings)
{
    NEXUS_Error rc;
    const NEXUS_Playback_P_PidChannel *pid;

    BDBG_CASSERT(NEXUS_NORMAL_PLAY_SPEED == NEXUS_NORMAL_DECODE_RATE);

    if (!settings) {
        /* reapply previous settings */
        settings = &p->trick.settings;
    }

    BDBG_MSG(("b_play_trick_set: %#lx %s->%s %s->%s %d->%d %d->%d %s->%s", (unsigned long)p,
        B_TRICK_STATE_STR(p->trick.settings.state), B_TRICK_STATE_STR(settings->state),
        B_FORWARD_STR(p->trick.settings.forward), B_FORWARD_STR(settings->forward),
        p->trick.settings.decode_rate, settings->decode_rate,
        p->trick.settings.stc_trick, settings->stc_trick,
        B_DECODE_MODE_STR(p->trick.settings.decode_mode), B_DECODE_MODE_STR(settings->decode_mode) ));

    /* fail invalid trick modes */
    if (settings->state == b_trick_state_normal && !settings->forward) {
        rc = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err_settings;
    }

    /* if the state and direction aren't changing, we don't need to flush. just jump over. */
    if (p->trick.settings.state == settings->state &&
        p->trick.settings.forward == settings->forward &&
        (p->trick.settings.decode_mode == settings->decode_mode || p->trick.settings.avoid_flush))
    {
        goto set_decoder_state;
    }

    /* this flushes both playback and decoders. this must occur before changing decoder state.
    if not, the new decoder state might be applied to a small amount of the old data and undesired
    results may be seen. if the interface to the decoder is slow (which it is), the likelihood
    increases. */
    b_play_flush(p);

set_decoder_state:

    if(p->trick.settings.state==b_trick_state_host_paced  || settings->state == b_trick_state_host_paced) {
        NEXUS_PlaypumpSettings pumpCfg;

        NEXUS_Playpump_GetSettings(p->params.playpump, &pumpCfg);
        BDBG_CASSERT(NEXUS_NORMAL_DECODE_RATE==NEXUS_NORMAL_PLAY_SPEED);
        if(settings->decode_rate!=0 && settings->state == b_trick_state_host_paced) {
            pumpCfg.playRate = settings->decode_rate;
        } else {
            if(settings->forward) {
                pumpCfg.playRate = NEXUS_NORMAL_DECODE_RATE;
            } else {
                pumpCfg.playRate = -NEXUS_NORMAL_DECODE_RATE;
            }
        }
        rc = NEXUS_Playpump_SetSettings(p->params.playpump, &pumpCfg);
        if(rc!=NEXUS_SUCCESS) { rc = BERR_TRACE(rc); goto err_settings;}
    }

    /* SW35230-2125, invalidate STC channel to force the STC to get reloaded. This is
    required to minimize the lost of audio/video data when transitioning from
    decoder_pause-->play or decoder_pause->seek->play.  The problem shows up if the
    time period between transition is long. */
    /* SW7425-4163: when going into pause, we need to ensure that the STC is not 
    reset until after the decoders are paused. However, the above PR says that
    we need to ensure that the STC is invalidated before we unpause.  Since pause
    and unpause are done by trick_set_pid, we must have logic invalidating the 
    STC before and after, based on whether we are entering pause or leaving it. */

    /* back out SW7425-4163 changes for now.  They may introduce a future race 
    condition if the order of pause, invalidate changes */
#if 1
    if(!settings->stc_trick) {
        b_play_stc_invalidate(p);

        if (settings->decode_rate != 0 && settings->decode_rate != NEXUS_NORMAL_DECODE_RATE) {
            /* Don't allow decoder trick for audio only. It just results in mysterious silence and premature loops. */
            for (pid = BLST_S_FIRST(&p->pid_list); pid ; pid = BLST_S_NEXT(pid, link)) {
                if (pid->cfg.pidSettings.pidType == NEXUS_PidType_eVideo) break;
            }
            if (!pid ) {
                BDBG_ERR(("decoder trick not supported for audio only playback"));
                return BERR_TRACE(NEXUS_NOT_SUPPORTED);
            }
        }
    }

    /* sets pause state, amongst other things */
    for (pid = BLST_S_FIRST(&p->pid_list); pid ; pid = BLST_S_NEXT(pid, link)) {
        rc = b_play_trick_set_pid(p, pid, settings);
        if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto err_set_pid;}
    }
#else
    /* coming from pause, invalidate STC first before unpausing */
    if(!settings->stc_trick && settings->decode_rate != 0 && p->trick.settings.decode_rate == 0) {
        b_play_stc_invalidate(p);
    }

    /* sets pause state, amongst other things */
    for (pid = BLST_S_FIRST(&p->pid_list); pid ; pid = BLST_S_NEXT(pid, link)) {
        rc = b_play_trick_set_pid(p, pid, settings);
        if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto err_set_pid;}
    }

    /* going to pause, invalidate STC after pausing */
    if(!settings->stc_trick && settings->decode_rate == 0 && p->trick.settings.decode_rate != 0) {
        b_play_stc_invalidate(p);
    }
#endif

    /* decoders must already be set into their state before setting STC */
    if (settings->stc_trick && settings->state == b_trick_state_normal) {
        /* set stc trick mode rate */
        rc = b_play_trick_stc_set_rate(p, settings->decode_rate);
        if(rc!=NEXUS_SUCCESS) { rc = BERR_TRACE(rc); goto err_settings;}
    }
    else {
        /* if playback has the stcChannel, set it to a normal rate in all other cases. */
        b_play_stc_rate(p, 1, 0);
    }

    p->trick.settings = *settings;
    return NEXUS_SUCCESS;

err_set_pid:
err_settings:
    return rc;
}

NEXUS_Error
b_play_trick_frameadvance(NEXUS_PlaybackHandle p)
{
    const NEXUS_Playback_P_PidChannel *pid;
    NEXUS_Error rc;

    pid = b_play_get_video_decoder(p);
    if (!pid) {
        BDBG_WRN(("Can't do frame advance without Nexus_VideoDecoderHandle"));
        BDBG_WRN(("Please set NEXUS_PlaybackPidChannelSettings.pidTypeSettings.video.decoder before calling NEXUS_Playback_OpenPidChannel()"));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto err_advance;
    }

    if (pid->cfg.pidTypeSettings.video.decoder) {
        if (p->trick.settings.stc_trick && p->trick.settings.forward && p->params.stcChannel) {
            uint32_t next_pts, stc;
            NEXUS_StcChannelSettings stcSettings;

            rc = NEXUS_VideoDecoder_GetNextPts(pid->cfg.pidTypeSettings.video.decoder, &next_pts);
            if (rc!=NEXUS_SUCCESS) {
                BDBG_WRN(("cannot frame advance. PTS not available yet."));
                /* don't fail. this is normal on transition. app should simply call again. */
                return BERR_SUCCESS;
            }
            NEXUS_StcChannel_GetStc(p->params.stcChannel, &stc);
            NEXUS_StcChannel_GetSettings(p->params.stcChannel, &stcSettings);

            BDBG_MSG(("b_play_trick_frameadvance: STC=%08X, Next PTS=%08X", stc, next_pts));
            stc = next_pts + stcSettings.modeSettings.Auto.offsetThreshold;
            rc = NEXUS_StcChannel_SetStc(p->params.stcChannel, stc);
            if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto err_advance;}
        }
        else {
            BDBG_MSG(("b_play_trick_frameadvance: video frame advance"));
            rc = NEXUS_VideoDecoder_FrameAdvance(pid->cfg.pidTypeSettings.video.decoder);
            if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_advance;}
        }
    }
#if NEXUS_HAS_SIMPLE_DECODER
    else if (pid->cfg.pidTypeSettings.video.simpleDecoder) {
        if (p->trick.settings.stc_trick && p->trick.settings.forward && p->params.simpleStcChannel) {
            uint32_t next_pts, stc;
            NEXUS_SimpleStcChannelSettings stcSettings;

            rc = NEXUS_SimpleVideoDecoder_GetNextPts(pid->cfg.pidTypeSettings.video.simpleDecoder, &next_pts);
            if (rc!=NEXUS_SUCCESS) {
                BDBG_WRN(("cannot frame advance. PTS not available yet."));
                /* don't fail. this is normal on transition. app should simply call again. */
                return BERR_SUCCESS;
            }
            NEXUS_SimpleStcChannel_GetStc(p->params.simpleStcChannel, &stc);
            NEXUS_SimpleStcChannel_GetSettings(p->params.simpleStcChannel, &stcSettings);

            BDBG_MSG(("b_play_trick_frameadvance: STC=%08X, Next PTS=%08X", stc, next_pts));
            stc = next_pts + stcSettings.modeSettings.Auto.offsetThreshold;
            rc = NEXUS_SimpleStcChannel_SetStc(p->params.simpleStcChannel, stc);
            if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto err_advance;}
        }
        else {
            BDBG_MSG(("b_play_trick_frameadvance: video frame advance"));
            rc = NEXUS_SimpleVideoDecoder_FrameAdvance(pid->cfg.pidTypeSettings.video.simpleDecoder);
            if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_advance;}
        }
    }
#endif

    return NEXUS_SUCCESS;
err_advance:
    return rc;
}

bool NEXUS_Playback_P_CheckSimpleDecoderSuspended(const NEXUS_Playback_P_PidChannel *pid)
{
#if NEXUS_HAS_SIMPLE_DECODER
    int rc;
    NEXUS_SimpleVideoDecoderClientStatus status;
    rc = NEXUS_SimpleVideoDecoder_GetClientStatus(pid->cfg.pidTypeSettings.video.simpleDecoder, &status);
    return !rc && !status.enabled;
#else
    BSTD_UNUSED(pid);
    return false;
#endif
}

NEXUS_Error NEXUS_P_Playback_VideoDecoder_GetStatus(const NEXUS_Playback_P_PidChannel *pid, NEXUS_VideoDecoderStatus *pStatus)
{
    if (pid->cfg.pidTypeSettings.video.decoder) {
        return NEXUS_VideoDecoder_GetStatus(pid->cfg.pidTypeSettings.video.decoder, pStatus);
    }
#if NEXUS_HAS_SIMPLE_DECODER
    else if (pid->cfg.pidTypeSettings.video.simpleDecoder) {
        int rc;
        if (NEXUS_Playback_P_CheckSimpleDecoderSuspended(pid)) {
            pid->playback->state.simpleDecoderSuspend.pid = pid;
            *pStatus = pid->playback->state.simpleDecoderSuspend.lastStatus;
            return 0;
        }
        else {
            rc = NEXUS_SimpleVideoDecoder_GetStatus(pid->cfg.pidTypeSettings.video.simpleDecoder, pStatus);
            if (rc) return rc;
            
            pid->playback->state.simpleDecoderSuspend.lastStatus = *pStatus;
            return 0;
        }
    }
#endif
    return NEXUS_NOT_AVAILABLE; /* no BERR_TRACE. could be normal. */
}

void NEXUS_P_Playback_VideoDecoder_Flush(const NEXUS_Playback_P_PidChannel *pid)
{
    /* only apply to one. there will never be >1 at a time. */
    if(pid->cfg.pidTypeSettings.video.decoder) {
        NEXUS_VideoDecoder_Flush(pid->cfg.pidTypeSettings.video.decoder);
    }
#if NEXUS_HAS_SIMPLE_DECODER
    else if(pid->cfg.pidTypeSettings.video.simpleDecoder) {
        NEXUS_SimpleVideoDecoder_Flush(pid->cfg.pidTypeSettings.video.simpleDecoder);
    }
#endif
}

void NEXUS_P_Playback_VideoDecoder_GetTrickState(const NEXUS_Playback_P_PidChannel *pid, NEXUS_VideoDecoderTrickState *pState)
{
    if (pid->cfg.pidTypeSettings.video.decoder) {
        NEXUS_VideoDecoder_GetTrickState(pid->cfg.pidTypeSettings.video.decoder, pState);
        return;
    }
#if NEXUS_HAS_SIMPLE_DECODER
    else if (pid->cfg.pidTypeSettings.video.simpleDecoder) {
        NEXUS_SimpleVideoDecoder_GetTrickState(pid->cfg.pidTypeSettings.video.simpleDecoder, pState);
        return;
    }
#endif
    BKNI_Memset(pState, 0, sizeof(*pState));
    return;
}

NEXUS_Error NEXUS_P_Playback_VideoDecoder_SetTrickState(const NEXUS_Playback_P_PidChannel *pid, const NEXUS_VideoDecoderTrickState *pState)
{
    /* only apply to one. there will never be >1 at a time. */
    if (pid->cfg.pidTypeSettings.video.decoder) {
        return NEXUS_VideoDecoder_SetTrickState(pid->cfg.pidTypeSettings.video.decoder, pState);
    }
#if NEXUS_HAS_SIMPLE_DECODER
    else if (pid->cfg.pidTypeSettings.video.simpleDecoder) {
        return NEXUS_SimpleVideoDecoder_SetTrickState(pid->cfg.pidTypeSettings.video.simpleDecoder, pState);
    }
#endif
    return BERR_TRACE(NEXUS_NOT_AVAILABLE);
}

void NEXUS_P_Playback_VideoDecoder_GetPlaybackSettings(const NEXUS_Playback_P_PidChannel *pid, NEXUS_VideoDecoderPlaybackSettings *pSettings)
{
    if (pid->cfg.pidTypeSettings.video.decoder) {
        NEXUS_VideoDecoder_GetPlaybackSettings(pid->cfg.pidTypeSettings.video.decoder, pSettings);
        return;
    }
#if NEXUS_HAS_SIMPLE_DECODER
    else if (pid->cfg.pidTypeSettings.video.simpleDecoder) {
        NEXUS_SimpleVideoDecoder_GetPlaybackSettings(pid->cfg.pidTypeSettings.video.simpleDecoder, pSettings);
        return;
    }
#endif
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    return;
}

NEXUS_Error NEXUS_P_Playback_VideoDecoder_SetPlaybackSettings(const NEXUS_Playback_P_PidChannel *pid, const NEXUS_VideoDecoderPlaybackSettings *pSettings)
{
    /* only apply to one. there will never be >1 at a time. */
    if (pid->cfg.pidTypeSettings.video.decoder) {
        return NEXUS_VideoDecoder_SetPlaybackSettings(pid->cfg.pidTypeSettings.video.decoder, pSettings);
    }
#if NEXUS_HAS_SIMPLE_DECODER
    else if (pid->cfg.pidTypeSettings.video.simpleDecoder) {
        return NEXUS_SimpleVideoDecoder_SetPlaybackSettings(pid->cfg.pidTypeSettings.video.simpleDecoder, pSettings);
    }
#endif
    return BERR_TRACE(NEXUS_NOT_AVAILABLE);
}

NEXUS_Error NEXUS_P_Playback_AudioDecoder_GetStatus(const NEXUS_Playback_P_PidChannel *pid, NEXUS_AudioDecoderStatus *pStatus, NEXUS_AudioDecoderStatus *pSecondaryStatus)
{
    NEXUS_Error rc=NEXUS_SUCCESS;

#if NEXUS_HAS_AUDIO
    /* primary and simple are mutually exclusive for status */
    if (pid->cfg.pidTypeSettings.audio.primary) {
        rc = NEXUS_AudioDecoder_GetStatus(pid->cfg.pidTypeSettings.audio.primary, pStatus);
        if (rc) return BERR_TRACE(rc);
    }
    else
#endif
#if NEXUS_HAS_SIMPLE_DECODER
    if (pid->cfg.pidTypeSettings.audio.simpleDecoder) {
        rc = NEXUS_SimpleAudioDecoder_GetStatus(pid->cfg.pidTypeSettings.audio.simpleDecoder, pStatus);
        if (rc) return BERR_TRACE(rc);
    }
    else
#endif
    {
        BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    }

    if (pSecondaryStatus) {
#if NEXUS_HAS_AUDIO
        if (pid->cfg.pidTypeSettings.audio.secondary) {
            rc = NEXUS_AudioDecoder_GetStatus(pid->cfg.pidTypeSettings.audio.secondary, pSecondaryStatus);
            if (rc) return BERR_TRACE(rc);
        }
        else
#endif
        {
            BKNI_Memset(pSecondaryStatus, 0, sizeof(*pSecondaryStatus));
        }
    }

    {
        /* if forcedStopped, we are effectively not started, so modify status */
        NEXUS_AudioDecoderTrickState trickState, secondaryTrickState;

        NEXUS_P_Playback_AudioDecoder_GetTrickState(pid, &trickState, &secondaryTrickState);
        pStatus->started &= !trickState.forceStopped;
        if(pSecondaryStatus) {
            pSecondaryStatus->started &= !secondaryTrickState.forceStopped;
        }
    }

    /* if there are no decoders for this pid, status will be memset(0), which translates to status.started = false. */
    return rc;
}

void NEXUS_P_Playback_AudioDecoder_Flush(const NEXUS_Playback_P_PidChannel *pid)
{
    BSTD_UNUSED(pid);
#if NEXUS_HAS_AUDIO
    if(pid->cfg.pidTypeSettings.audio.primary) {
        (void)NEXUS_AudioDecoder_Flush(pid->cfg.pidTypeSettings.audio.primary);
    }
    if(pid->cfg.pidTypeSettings.audio.secondary) {
        (void)NEXUS_AudioDecoder_Flush(pid->cfg.pidTypeSettings.audio.secondary);
    }
    if (pid->cfg.pidTypeSettings.audio.primer) {
        NEXUS_AudioDecoderPrimer_Flush(pid->cfg.pidTypeSettings.audio.primer);
    }
    if (pid->cfg.pidTypeSettings.audio.secondaryPrimer) {
        NEXUS_AudioDecoderPrimer_Flush(pid->cfg.pidTypeSettings.audio.secondaryPrimer);
    }
#endif
#if NEXUS_HAS_SIMPLE_DECODER
    if (pid->cfg.pidTypeSettings.audio.simpleDecoder) {
        NEXUS_SimpleAudioDecoder_Flush(pid->cfg.pidTypeSettings.audio.simpleDecoder);
    }
#endif
}

NEXUS_Error NEXUS_Playback_P_AudioDecoder_Advance(const NEXUS_Playback_P_PidChannel *pid, uint32_t video_pts)
{
    NEXUS_Error rc;
    BSTD_UNUSED(video_pts);
    BSTD_UNUSED(pid);

#if NEXUS_HAS_AUDIO
    if(pid->cfg.pidTypeSettings.audio.primary) {
        if (pid->playback->state.audioTrick.primary) {
            rc = NEXUS_AudioDecoder_Advance(pid->cfg.pidTypeSettings.audio.primary, video_pts);
            if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);}
        }
    }
    if(pid->cfg.pidTypeSettings.audio.secondary) {
        if (pid->playback->state.audioTrick.secondary) {
            rc = NEXUS_AudioDecoder_Advance(pid->cfg.pidTypeSettings.audio.secondary, video_pts);
            if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);}
        }
    }
#else
    BSTD_UNUSED(rc);
#endif
#if NEXUS_HAS_SIMPLE_DECODER
    if (pid->cfg.pidTypeSettings.audio.simpleDecoder) {
        rc = NEXUS_SimpleAudioDecoder_Advance(pid->cfg.pidTypeSettings.audio.simpleDecoder, video_pts);
        if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);}
    }
#endif
    return 0;
}

void NEXUS_P_Playback_AudioDecoder_GetTrickState(const NEXUS_Playback_P_PidChannel *pid, NEXUS_AudioDecoderTrickState *pState, NEXUS_AudioDecoderTrickState *pSecondaryState)
{
    BSTD_UNUSED(pid);
#if NEXUS_HAS_AUDIO
    /* primary and simple are mutually exclusive for getting state */
    if (pid->cfg.pidTypeSettings.audio.primary) {
        NEXUS_AudioDecoder_GetTrickState(pid->cfg.pidTypeSettings.audio.primary, pState);
    }
    else 
#endif
#if NEXUS_HAS_SIMPLE_DECODER
    if (pid->cfg.pidTypeSettings.audio.simpleDecoder) {
        NEXUS_SimpleAudioDecoder_GetTrickState(pid->cfg.pidTypeSettings.audio.simpleDecoder, pState);
    }
    else 
#endif
    {
        BKNI_Memset(pState, 0, sizeof(*pState));
    }

    if (pSecondaryState) {
#if NEXUS_HAS_AUDIO
        if (pid->cfg.pidTypeSettings.audio.secondary) {
            NEXUS_AudioDecoder_GetTrickState(pid->cfg.pidTypeSettings.audio.secondary, pSecondaryState);
        }
        else 
#endif
        {
            BKNI_Memset(pSecondaryState, 0, sizeof(*pSecondaryState));
        }
    }
    return;
}

NEXUS_Error NEXUS_P_Playback_AudioDecoder_SetTrickState(const NEXUS_Playback_P_PidChannel *pid, const NEXUS_AudioDecoderTrickState *pState)
{
    NEXUS_Error rc=NEXUS_SUCCESS;
    BSTD_UNUSED(pState);
    /* set any available */
    pid->playback->state.audioTrick.primary = 
    pid->playback->state.audioTrick.secondary = false;
#if NEXUS_HAS_AUDIO
    if (pid->cfg.pidTypeSettings.audio.primary) {
        NEXUS_AudioDecoderStatus status;
        rc = NEXUS_AudioDecoder_GetStatus(pid->cfg.pidTypeSettings.audio.primary, &status);
        if (!rc && status.started) {
            rc = NEXUS_AudioDecoder_SetTrickState(pid->cfg.pidTypeSettings.audio.primary, pState);
            if (rc) return BERR_TRACE(rc);
            pid->playback->state.audioTrick.primary = true;
        }
    }
    if (pid->cfg.pidTypeSettings.audio.secondary) {
        NEXUS_AudioDecoderStatus status;
        rc = NEXUS_AudioDecoder_GetStatus(pid->cfg.pidTypeSettings.audio.secondary, &status);
        if (!rc && status.started) {
            rc = NEXUS_AudioDecoder_SetTrickState(pid->cfg.pidTypeSettings.audio.secondary, pState);
            if (rc) return BERR_TRACE(rc);
            pid->playback->state.audioTrick.secondary = true;
        }
    }
#endif
#if NEXUS_HAS_SIMPLE_DECODER
    if (pid->cfg.pidTypeSettings.audio.simpleDecoder) {
        rc = NEXUS_SimpleAudioDecoder_SetTrickState(pid->cfg.pidTypeSettings.audio.simpleDecoder, pState);
        if (rc) return BERR_TRACE(rc);
    }
#endif
    return rc;
}

/**
abstraction for StcChannel and SimpleStcChannel
**/
static void
b_play_stc_rate(NEXUS_PlaybackHandle p, unsigned increment, unsigned prescale)
{
    if (p->params.stcChannel) {
        (void)NEXUS_StcChannel_SetRate(p->params.stcChannel, increment, prescale);
    }
#if NEXUS_HAS_SIMPLE_DECODER
    else if (p->params.simpleStcChannel) {
        (void)NEXUS_SimpleStcChannel_SetRate(p->params.simpleStcChannel, increment, prescale);
    }
#endif
    return;
}

static void
b_play_stc_invalidate(NEXUS_PlaybackHandle p)
{
    if (p->params.stcChannel) {
        (void)NEXUS_StcChannel_Invalidate(p->params.stcChannel);
    }
#if NEXUS_HAS_SIMPLE_DECODER
    else if (p->params.simpleStcChannel) {
        (void)NEXUS_SimpleStcChannel_Invalidate(p->params.simpleStcChannel);
    }
#endif
    return;
}

