/***************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "nexus_audio_module.h"
#include "bpcrlib.h"

BDBG_MODULE(nexus_audio_decoder_trick);

void NEXUS_AudioDecoder_GetTrickState(
    NEXUS_AudioDecoderHandle decoder,
    NEXUS_AudioDecoderTrickState *pState 
    )
{
    BDBG_ASSERT(decoder);
    BDBG_ASSERT(pState);
    *pState = decoder->trickState;
    return;
}

NEXUS_Error NEXUS_AudioDecoder_SetTrickState(
    NEXUS_AudioDecoderHandle decoder,             
    const NEXUS_AudioDecoderTrickState *pTrickState
    )
{
    NEXUS_Error rc;
    NEXUS_AudioDecoderTrickState oldState;
    bool dspMixerAttached = false;
    bool wasPaused = false;
    bool trick = false;
    bool wasTrick = false;
    bool forceStop = false;
    bool wasForceStopped = false;
    bool wasStoppedForTrick = false;
    bool stopForTrick = false;
    bool dsola = false;
    bool wasDsola = false;
    bool muteForTrick = false;

    /* actions to take */
    bool stop = false;
    bool flush = false;
    bool play = false;
    bool pause = false;
    bool resume = false;


    /* Valid Rate Changes: */
    /* 1 Normal --> Pause */
    /* 2 Normal --> Trick */
    /* 3 Pause --> Normal */
    /* 4 Pause --> Trick */
    /* 5 Trick --> Normal */
    /* 6 Trick --> Pause */

    BDBG_ASSERT(decoder);
    BDBG_ASSERT(pTrickState);

    if( !decoder->started ) 
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    rc = NEXUS_AudioDecoder_P_IsDspMixerAttached(decoder, &dspMixerAttached);
    if(rc!=NEXUS_SUCCESS)
    {
        rc=BERR_TRACE(rc);
        goto err_set_state;
    }

    /* Save OLD state */
    wasStoppedForTrick = decoder->trickState.stopForTrick;
    wasForceStopped = decoder->trickState.forceStopped;

    switch ( decoder->trickState.rate )
    {
        case 0:
            wasPaused = !wasStoppedForTrick && !wasForceStopped;
            wasTrick = false;
            break;
        case NEXUS_NORMAL_DECODE_RATE:
            wasTrick = false;
            break;
        default:
            wasTrick = true;
            break;
    }

    /* Set up NEW state */
    stopForTrick = pTrickState->stopForTrick;
    forceStop = pTrickState->forceStopped;

    if (!stopForTrick && !forceStop)
    {
        play = true;
    }

    switch ( pTrickState->rate )
    {
        case 0:
            pause = !forceStop && !stopForTrick;
            trick = false;
            break;
        case NEXUS_NORMAL_DECODE_RATE:
            trick = false;
            break;
        default:
            trick = true;
            break;
    }

    if (trick)
    {
        if (g_NEXUS_audioModuleData.capabilities.dsp.decodeRateControl && !dspMixerAttached)
        {
            if (pTrickState->allowDsola && pTrickState->rate <= NEXUS_AUDIO_DECODER_P_MAX_DSOLA_RATE && !pause)
            {
                dsola = true;
            }
        }
        if (!dsola)
        {
            muteForTrick = true;
        }
    }

    if (g_NEXUS_audioModuleData.capabilities.dsp.decodeRateControl &&
        decoder->settings.alwaysEnableDsola &&
        pTrickState->rate <= NEXUS_AUDIO_DECODER_P_MAX_DSOLA_RATE &&
        !dspMixerAttached) {
        dsola = true;
    }

    if (wasTrick || decoder->settings.alwaysEnableDsola)
    {
        wasDsola = decoder->apeStartSettings.decodeRateControl;
    }

    if (forceStop)
    {
        stop = true;
        flush = true;
    }

    if (!forceStop && !wasForceStopped) { /* we were playing or decoder trick mode */
        flush = false;
        if ( trick != wasTrick ) {
            if (dsola == wasDsola &&
                decoder->settings.alwaysEnableDsola) {
                stop = false;
            }
            else {
                stop = true;
            }
        }
        else if ( dsola != wasDsola ) {
            stop = true;
        }
        else {
            stop = false;
        }
    }
    else
    {
        stop = true;
        flush = true;
    }

    if (stop)
    {
        BDBG_MSG(("Stopping audio decode"));
        rc = NEXUS_AudioDecoder_P_Stop(decoder, flush);
        if(rc!=NEXUS_SUCCESS)
        {
            rc=BERR_TRACE(rc);
            goto err_set_state;
        }
    }

    decoder->apeStartSettings.decodeRateControl = dsola;
    resume = !pause && wasPaused && !forceStop && !stopForTrick;

    BDBG_MSG(("Was: muted %d,rate %d,trick %d,forceStopped %d,stopForTrick %d,wasPaused %d,dsola %d",
              decoder->trickState.muted, decoder->trickState.rate, wasTrick, wasForceStopped, wasStoppedForTrick, wasPaused, wasDsola));
    BDBG_MSG(("Going to: mute %d,rate %d,trick %d,forceStop %d,stopForTrick %d,dsola %d",
              pTrickState->muted, pTrickState->rate, trick, forceStop, stopForTrick, dsola));
    BDBG_MSG(("Actions: stop %d,flush %d,play %d,pause %d,resume %d",
              stop, flush, play, pause, resume));

    if ( play )
    {
        if ((!NEXUS_AudioDecoder_P_IsRunning(decoder)))
        {
            BDBG_MSG(("Starting audio decode"));
            rc = NEXUS_AudioDecoder_P_Start(decoder);
            if(rc!=NEXUS_SUCCESS)
            {
                rc=BERR_TRACE(rc);
                goto err_set_state;
            }
        }
    }

    if ( pause )
    {
        /* 1 Normal --> Pause */
        /* 6 Trick --> Pause */
        BDBG_MSG(("Pausing audio decode"));
        rc = BAPE_Decoder_Pause(decoder->channel);
        if(rc!=NEXUS_SUCCESS)
        {
            rc=BERR_TRACE(rc);
            goto err_set_state;
        }
    }
    else if ( resume )
    {
        /* 3 Pause --> Normal */
        /* 4 Pause --> Trick (was paused from trick [6]) */
        BDBG_MSG(("UnPausing audio decode"));
        rc = BAPE_Decoder_Resume(decoder->channel);
        if(rc!=NEXUS_SUCCESS)
        {
            rc=BERR_TRACE(rc);
            goto err_set_state;
        }
    }

    oldState = decoder->trickState;
    decoder->trickState = *pTrickState;

    /* If we are attempting trickplay and we cannot run DSOLA we need to mute */
    if (trick && muteForTrick)
    {
        decoder->trickState.muted = true;
    }

    /* If we are stopping for trick maintain the old rate until we come through again with !stopForTrick.
        This resolves issue with pause->play with rate of 0. */
    if (decoder->trickState.stopForTrick)
    {
        decoder->trickState.rate = oldState.rate;
    }

    /* If we determined we didn't have to stop (we were paused or pausing) do not set stopForTrick to true */
    if (!stop && decoder->trickState.stopForTrick)
    {
        decoder->trickState.stopForTrick = false;
    }
    rc = NEXUS_AudioDecoder_ApplySettings_priv(decoder); /* Apply trick mute and rate */
    if ( rc )
    {
        rc = BERR_TRACE(rc);
        goto err_apply_settings;
    }
    NEXUS_AudioDecoder_P_SetTsm(decoder);           /* Apply TSM setting */

    return BERR_SUCCESS;
err_apply_settings:
    decoder->trickState = oldState;
err_set_state:
    return rc;
}

/***************************************************************************
Summary:
    Advances audio decoder to the target PTS
Desctiption:
    NEXUS_AudioDecoder_Advance causes audio decoder to drop compressed data,
    until it finds data with timestamp greater then target PTS.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_Advance(
    NEXUS_AudioDecoderHandle decoder,             
    uint32_t targetPts /* PTS in 45Khz (MPEG-2) or 27MHz domain */
    )
{
    BERR_Code rc;
    uint32_t audio_pts;
    BAPE_DecoderHandle channel;
    BAPE_DecoderTsmStatus tsmStatus;

    BDBG_ASSERT(decoder);

    if( !decoder->started ) 
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( !decoder->running )
    {
        return BERR_SUCCESS;
    }

    BDBG_ASSERT(decoder->channel);
    channel = decoder->channel;

    BAPE_Decoder_GetTsmStatus(channel, &tsmStatus);
    if ( tsmStatus.ptsInfo.ePTSType != BAVC_PTSType_eInterpolatedFromInvalidPTS )
    {
        audio_pts = tsmStatus.ptsInfo.ui32CurrentPTS;
    }
    else
    {
        audio_pts = 0;
    }

    if ( targetPts == 0 || audio_pts == 0 ) 
    {
        /* If there's no video or audio, we still need to advance a bit.
        If video is 0, we can't allow audio to hold playback and lock up the state machine.
        If audio is 0, we need the decoder to consume a little bit so it can get a PTS. */
        rc = BAPE_Decoder_Advance(channel, 10);
        if ( rc ) 
        {
            return BERR_TRACE(rc);
        }
    } 
    else
    {
        int32_t diff; /* in 22.5 KHz units */
        diff = BPCRlib_StcDiff_isrsafe(decoder->isDss, targetPts, audio_pts);
        if (diff > 0) 
        {   
            diff /= 23; /* convert from 22.5Khz to milliseconds */
        
            /* don't advance unless we have a complete frame (50 milliseconds handles NTSC and PAL). this
                minimizes calls to RAP FW. */
            if (diff >= 50) 
            {
                BDBG_MSG(("frameadv[%#lx]: vpts=%#x, apts=%#x (diff %d), adv %d msec", (unsigned long)decoder, targetPts, audio_pts, targetPts-audio_pts, diff));
                rc = BAPE_Decoder_Advance(channel, diff);
                if ( rc ) 
                {
                    return BERR_TRACE(rc);
                }
            }
        } 
        else if (diff < -(16*22500))
        {
            /* if we're way behind, just consume a bit and expect we will get through it */
            rc = BAPE_Decoder_Advance(channel, 1500);
            if ( rc )
            {
                return BERR_TRACE(rc);
            }
        }
        else 
        {
            BDBG_MSG(("frameadv[%#lx]: vpts=%#x,apts=%#x (diff %d), no advance", (unsigned long)decoder, targetPts,audio_pts, diff));
        }
    }
    return BERR_SUCCESS;
}

void NEXUS_AudioDecoder_P_TrickReset( NEXUS_AudioDecoderHandle decoder)
{
    BDBG_ASSERT(decoder);
    decoder->trickState.forceStopped = false;
    decoder->trickState.rate = NEXUS_NORMAL_DECODE_RATE;
    decoder->trickState.tsmEnabled = true;
    decoder->trickState.muted = false;
    decoder->trickState.stcTrickEnabled = false;
    decoder->trickState.allowDsola = true;
    if ( decoder->channel )
    {
        NEXUS_AudioDecoder_ApplySettings_priv(decoder);
    }
}
