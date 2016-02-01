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
    NEXUS_Error rc=NEXUS_SUCCESS;
	bool bPassthru,bForceStop,bForceMute;
    BDBG_ASSERT(decoder);
    BDBG_ASSERT(pTrickState);

	bPassthru = ( decoder->outputLists[NEXUS_AudioDecoderConnectorType_eCompressed].outputs[0] != NULL &&
				  decoder->outputLists[NEXUS_AudioDecoderConnectorType_eStereo].outputs[0] == NULL && 
				  decoder->outputLists[NEXUS_AudioDecoderConnectorType_eMultichannel].outputs[0] == NULL );
	bForceStop = false;
    switch ( pTrickState->rate )
    {
    case 0:
    case 1000:
    case 800:
    case 1200:
        bForceMute = false;
        break;
    default:
        /* Force the decoder to mute at non-standard rates */
        BDBG_MSG(("Unsupported trick rate.  Muting decoder output."));
        bForceMute = true;
        break;
    }

    if( !decoder->started ) 
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BDBG_MSG(("muted %d,force stopped %d,rate %d",pTrickState->muted,pTrickState->forceStopped,pTrickState->rate));   

    if ( decoder->trickForceStopped != pTrickState->forceStopped ||
         decoder->trickState.rate != pTrickState->rate )
    {    
        BRAP_DSPCHN_PlayBackRate oldRate = decoder->audioParams.sDspChParams.ePBRate;

        switch(pTrickState->rate)
        {
        case 0:
            break;
        default:
        case 1000:
            decoder->audioParams.sDspChParams.ePBRate=BRAP_DSPCHN_PlayBackRate_e100;
            break;
        case 1200:
            BDBG_WRN(("1.2X fast playback"));
            decoder->audioParams.sDspChParams.ePBRate=BRAP_DSPCHN_PlayBackRate_e120;
			bForceStop = bPassthru;
            break;
        case 800:
            BDBG_WRN(("0.8X slow playback"));
            decoder->audioParams.sDspChParams.ePBRate = BRAP_DSPCHN_PlayBackRate_e80;
			bForceStop = bPassthru;
            break;
        }

        if ( pTrickState->forceStopped || bForceStop ) 
        {
            BDBG_MSG(("Stopping audio decode"));
            rc = NEXUS_AudioDecoder_P_Stop(decoder, true);
			/* Mark channel status invalid */
			decoder->channelStatusValid = false;
        } 
        else if ( oldRate != decoder->audioParams.sDspChParams.ePBRate  )
        {
            BDBG_MSG(("Restarting audio decode"));
			decoder->channelStatusValid = false;
            (void)NEXUS_AudioDecoder_P_Stop(decoder, false);
            rc = NEXUS_AudioDecoder_P_Start(decoder);
        }
        else if (!pTrickState->forceStopped && decoder->trickForceStopped )
        {
            BDBG_MSG(("Starting audio decode"));
			decoder->channelStatusValid = false;
            rc = NEXUS_AudioDecoder_P_Start(decoder);
        }

        if(rc!=NEXUS_SUCCESS) 
        {
            rc=BERR_TRACE(rc);
            goto err_set_state;
        }
        decoder->trickForceStopped = pTrickState->forceStopped;
    }

    rc = NEXUS_AudioDecoder_P_SetTrickMute(decoder, pTrickState->muted || bForceMute);
    if(rc!=NEXUS_SUCCESS) 
    {
        rc=BERR_TRACE(rc);
        goto err_set_state;
    }

    BDBG_ASSERT(decoder->rapChannel);
    if(!decoder->trickForceStopped) {
#if BCHP_CHIP == 3563
        if(0 == pTrickState->rate) {
            /* No pause functionality supported at the moment... */
            rc = BERR_TRACE(BERR_NOT_SUPPORTED);
        }
#else
        rc = BRAP_PVR_EnablePause(decoder->rapChannel, pTrickState->rate==0);
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
/* WARNING: 3563 RAP has not implemented BRAP_PVR_EnablePause */
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

    rc = BRAP_PVR_EnablePause(decoder->rapChannel, enable);
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
/* WARNING: 3563 RAP has not implemented BRAP_PVR_EnablePause */
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

    rc = BRAP_DSPCHN_GetCurrentPTS(channel, &ptsInfo);
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
        rc = BRAP_PVR_FrameAdvance(channel, 10);
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
                rc = BRAP_PVR_FrameAdvance(channel, diff);
                if ( rc ) 
                {
                    return BERR_TRACE(rc);
                }
            }
        } 
        else if (diff < -(16*22500))
        {
            /* if we're way behind, just consume a bit and expect we will get through it */
            rc = BRAP_PVR_FrameAdvance(channel, 1500);
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
    decoder->trickState.stcTrickEnabled = false;
    return;
}

