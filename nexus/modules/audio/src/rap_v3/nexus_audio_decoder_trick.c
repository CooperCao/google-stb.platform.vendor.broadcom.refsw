/***************************************************************************
 *     (c)2007-2011 Broadcom Corporation
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 **************************************************************************/
#include "nexus_audio_module.h"
#include "nexus_audio_priv.h"
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
    bool forceMute;

    BDBG_ASSERT(decoder);
    BDBG_ASSERT(pTrickState);

    if( !decoder->started ) 
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BDBG_MSG(("muted %d,force stopped %d,rate %d",pTrickState->muted,pTrickState->forceStopped,pTrickState->rate));

    if ( (pTrickState->rate < 500 || pTrickState->rate > 2000) && pTrickState->rate != 0 )
    {
        BDBG_MSG(("Unsupported trick rate.  Muting decoder output."));
        forceMute = true;
    }
    else
    {
        forceMute = false;
    }

    rc = NEXUS_AudioDecoder_P_SetTrickMute(decoder, pTrickState->muted || forceMute);
    if(rc!=NEXUS_SUCCESS) 
    {
        rc=BERR_TRACE(rc);
        goto err_set_state;
    }

    if ( decoder->trickForceStopped != pTrickState->forceStopped ||
         decoder->trickState.rate != pTrickState->rate )
    {    
        BRAP_DSPCHN_PlayBackRate oldRate = decoder->audioParams.sDspChParams.uiPBRate;

        if ( pTrickState->rate >= (NEXUS_NORMAL_DECODE_RATE/2) && pTrickState->rate <= (NEXUS_NORMAL_DECODE_RATE*2) )
        {
            decoder->audioParams.sDspChParams.uiPBRate = (pTrickState->rate * BRAP_DSPCHN_PLAYBACKRATE_NORMAL)/NEXUS_NORMAL_DECODE_RATE;
        }

        if ( pTrickState->forceStopped ) 
        {
            BDBG_MSG(("Stopping audio decode"));
            rc = NEXUS_AudioDecoder_P_Stop(decoder, true);
        } 
#if BRAP_VER == 3  /* Function not yet defined */
        else if ( oldRate != decoder->audioParams.sDspChParams.uiPBRate && decoder->audioParams.sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eDecode )
        {
            if ( oldRate == BRAP_DSPCHN_PLAYBACKRATE_NORMAL || 
                 decoder->audioParams.sDspChParams.uiPBRate == BRAP_DSPCHN_PLAYBACKRATE_NORMAL )
            {
                /* We are entering or leaving a trick mode.  Must stop/start */
                BDBG_MSG(("Restarting audio decode"));
                (void)NEXUS_AudioDecoder_P_Stop(decoder, false);
                rc = NEXUS_AudioDecoder_P_Start(decoder);
            }
            else
            {
                /* Trick -> Trick */
                BDBG_MSG(("Updating trick rate from %u -> %u", oldRate, decoder->audioParams.sDspChParams.uiPBRate));
                rc = BRAP_SetAudioPlayRate(decoder->rapChannel, decoder->audioParams.sDspChParams.uiPBRate);
            }
        }
#else
        else if ( oldRate != decoder->audioParams.sDspChParams.uiPBRate && decoder->audioParams.sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eDecode )
        {
            BDBG_MSG(("Restarting audio decode"));
            (void)NEXUS_AudioDecoder_P_Stop(decoder, false);
            rc = NEXUS_AudioDecoder_P_Start(decoder);
        }
#endif
        else if (!pTrickState->forceStopped && decoder->trickForceStopped )
        {
            BDBG_MSG(("Starting audio decode"));
            rc = NEXUS_AudioDecoder_P_Start(decoder);
        }

        if(rc!=NEXUS_SUCCESS) 
        {
            rc=BERR_TRACE(rc);
            goto err_set_state;
        }
        decoder->trickForceStopped = pTrickState->forceStopped;
    }

    BDBG_ASSERT(decoder->rapChannel);
    if(!decoder->trickForceStopped) {
#if BCHP_CHIP == 3563
        if(0 == pTrickState->rate) {
            /* No pause functionality supported at the moment... */
            rc = BERR_TRACE(BERR_NOT_SUPPORTED);
        }
#else
        rc = BRAP_EnablePvrPause(decoder->rapChannel, pTrickState->rate==0);
        if(rc!=NEXUS_SUCCESS) 
        {
            rc=BERR_TRACE(rc);
            goto err_set_state;
        }
#endif
    }
    decoder->trickState = *pTrickState;
    NEXUS_AudioDecoder_P_SetTsm(decoder);
    return BERR_SUCCESS;
err_set_state:
    return rc;
}

/***************************************************************************
Summary:
    Controls pause state of the audio decoder
See Also:
    NEXUS_AudioDecoder_Start
    NEXUS_AudioDecoder_Advance
 ***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_Pause(
    NEXUS_AudioDecoderHandle decoder,             
    bool enable
    )
{
#if BCHP_CHIP == 3563
/* WARNING: 3563 RAP has not implemented BRAP_EnablePvrPause */
    BSTD_UNUSED(decoder);
    BSTD_UNUSED(enable);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#else
    BERR_Code rc;

    BDBG_ASSERT(decoder);

    if( !decoder->started ) 
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( !decoder->running )
    {
        return BERR_SUCCESS;
    }

    rc = BRAP_EnablePvrPause(decoder->rapChannel, enable);
    if ( rc ) 
    {
        return BERR_TRACE(rc);
    }

    return NEXUS_SUCCESS;
#endif
}

/***************************************************************************
Summary:
    Advances audio decoder to the target PTS
Desctiption:
    NEXUS_AudioDecoder_Advance causes audio decoder to drop compressed data,
    untill it finds data with timestamp greater then target PTS.

See Also:
    NEXUS_AudioDecoder_Pause
 ***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_Advance(
    NEXUS_AudioDecoderHandle decoder,             
    uint32_t targetPts /* PTS in 45Khz (MPEG-2) or 27MHz domain */
    )
{
#if BCHP_CHIP == 3563
/* WARNING: 3563 RAP has not implemented BRAP_EnablePvrPause */
    BSTD_UNUSED(decoder);
    BSTD_UNUSED(targetPts);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#else
    BERR_Code rc;
    uint32_t audio_pts;
    BRAP_ChannelHandle channel;
    BRAP_DSPCHN_PtsInfo ptsInfo;

    BDBG_ASSERT(decoder);

    if( !decoder->started ) 
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( !decoder->running )
    {
        return BERR_SUCCESS;
    }

    BDBG_ASSERT(decoder->rapChannel);
    channel = decoder->rapChannel;

    rc = BRAP_GetCurrentPTS(channel, &ptsInfo);
    if ( rc ) 
    {
        if ( rc == BRAP_ERR_BAD_DEVICE_STATE )
        {
            /* PTS not available yet.  Don't treat this as an error case. */
            return BERR_SUCCESS;
        }
        else
        {
            return BERR_TRACE(rc);
        }
    }
    if ( ptsInfo.ePtsType != BRAP_DSPCHN_PtsType_eInterpolatedFromInvalidPTS ) 
    {
        audio_pts = ptsInfo.ui32RunningPts;
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
        rc = BRAP_PvrFrameAdvance(channel, 10);
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
                rc = BRAP_PvrFrameAdvance(channel, diff);
                if ( rc ) 
                {
                    return BERR_TRACE(rc);
                }
            }
        } 
        else if (diff < -(16*22500))
        {
            /* if we're way behind, just consume a bit and expect we will get through it */
            rc = BRAP_PvrFrameAdvance(channel, 1500);
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
#endif
}

void NEXUS_AudioDecoder_P_TrickReset( NEXUS_AudioDecoderHandle decoder)
{
    BDBG_ASSERT(decoder);
    NEXUS_AudioDecoder_P_SetTrickMute(decoder, false);
    decoder->trickState.forceStopped = false;
    decoder->trickState.rate = NEXUS_NORMAL_DECODE_RATE;
    decoder->trickState.tsmEnabled = true;
    decoder->trickState.muted = false;
    decoder->trickState.stcTrickEnabled = false;
}

