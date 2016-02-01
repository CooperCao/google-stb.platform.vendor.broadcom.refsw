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
#ifndef NEXUS_VIDEO_DECODER_TRICK_H__
#define NEXUS_VIDEO_DECODER_TRICK_H__

#include "nexus_video_decoder.h"
#include "nexus_video_decoder_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Get the current trick mode state.

Description:
See Also:
NEXUS_VideoDecoder_SetTrickState
**/
void NEXUS_VideoDecoder_GetTrickState(
    NEXUS_VideoDecoderHandle handle,
    NEXUS_VideoDecoderTrickState *pState /* [out] current trick mode state */
    );

/**
Summary:
Set a new trick mode state.

Description:
You should call NEXUS_VideoDecoder_GetTrickState, modify any settings you want, then call NEXUS_VideoDecoder_SetTrickState.

See Also:
NEXUS_VideoDecoder_GetTrickState
**/
NEXUS_Error NEXUS_VideoDecoder_SetTrickState(
    NEXUS_VideoDecoderHandle handle,
    const NEXUS_VideoDecoderTrickState *pState
    );

/*
Summary:
If decoder is paused, this will cause the decoder to push out one picture.

Description:
Pause the decoder by settings NEXUS_VideoDecoderTrickState.rate to 0.

See Also:
NEXUS_VideoDecoder_SetTrickState
**/
NEXUS_Error NEXUS_VideoDecoder_FrameAdvance(
    NEXUS_VideoDecoderHandle handle
    );

/**
Summary:
Return the PTS of the next picture in the picture delivery queue.
**/
NEXUS_Error NEXUS_VideoDecoder_GetNextPts(
    NEXUS_VideoDecoderHandle handle,
    uint32_t *pNextPts /* [out] */
    );

/**
Summary:
Dedicated settings for the Playback module
**/
void NEXUS_VideoDecoder_GetPlaybackSettings(
    NEXUS_VideoDecoderHandle handle,
    NEXUS_VideoDecoderPlaybackSettings *pSettings /* [out] */
    );

/**
Summary:
Dedicated settings for the Playback module
**/
NEXUS_Error NEXUS_VideoDecoder_SetPlaybackSettings(
    NEXUS_VideoDecoderHandle handle,
    const NEXUS_VideoDecoderPlaybackSettings *pSettings
    );

/**
Summary:
Invalidate the simulated STC.

Description:
If the STC is invalid, the first PTS will set it. After that, the STC will increment/decrement based on NEXUS_VideoDecoderTrickState.rate.
STC is always marked invalid when NEXUS_VideoDecoder_Start is called.
**/
NEXUS_Error NEXUS_VideoDecoder_InvalidateSimulatedStc(
    NEXUS_VideoDecoderHandle handle
    );
    
/**
Summary:
Get default for NEXUS_VideoDecoderTrickState which corresponds to normal play.
**/
void NEXUS_VideoDecoder_GetNormalPlay(
    NEXUS_VideoDecoderTrickState *pState /* [out] */
    );

#ifdef __cplusplus
}
#endif

#endif
