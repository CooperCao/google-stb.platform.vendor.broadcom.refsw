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
* API Description:
*   API name: AudioDecoder Trick
*    API for audio decoder trick-play.
*
***************************************************************************/
#ifndef NEXUS_AUDIO_DECODER_TRICK_H__
#define NEXUS_AUDIO_DECODER_TRICK_H__

#include "nexus_types.h"
#include "nexus_audio_types.h"
#include "nexus_audio_decoder.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Get the current trick mode state.
**/
void NEXUS_AudioDecoder_GetTrickState(
    NEXUS_AudioDecoderHandle decoder,
    NEXUS_AudioDecoderTrickState *pState /* [out] current trick mode state */
    );


/**
Summary:
Set a new trick mode state.

Description:
You should call NEXUS_AudioDecoder_GetTrickState, modify any settings you want, then call NEXUS_AutioDecoder_SetTrickState.
**/
NEXUS_Error NEXUS_AudioDecoder_SetTrickState(
    NEXUS_AudioDecoderHandle decoder,
    const NEXUS_AudioDecoderTrickState *pTrickState
    );


/***************************************************************************
Summary:
    Advances audio decoder to the target PTS

Description:
    NEXUS_AudioDecoder_Advance causes audio decoder to drop compressed data,
    until it finds data with timestamp greater then target PTS.  The decoder
    must be in a paused state for this to work.  To pause, call
    NEXUS_AudioDecoder_SetTrickState with a rate value of 0.

See Also:
    NEXUS_AudioDecoder_SetTrickState
 ***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_Advance(
    NEXUS_AudioDecoderHandle decoder,
    uint32_t targetPts /* PTS in 45Khz (MPEG-2) or 27MHz domain */
    );

#ifdef __cplusplus
}
#endif

#endif  /* NEXUS_AUDIO_DECODER_TRICK_H__ */

