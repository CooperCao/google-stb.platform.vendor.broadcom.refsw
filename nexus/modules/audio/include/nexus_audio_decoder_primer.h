/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#ifndef NEXUS_AUDIO_DECODER_PRIMER_H__
#define NEXUS_AUDIO_DECODER_PRIMER_H__

#include "nexus_audio_decoder.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Handle for audio decoder primer returned by NEXUS_AudioDecoderPrimer_Open
**/
typedef struct NEXUS_AudioDecoderPrimer *NEXUS_AudioDecoderPrimerHandle;

/**
Summary:
Start processing a pid channel for fast audio change using NEXUS_AudioDecoderPrimer_StopPrimerAndStartDecode
**/
NEXUS_AudioDecoderPrimerHandle NEXUS_AudioDecoderPrimer_Open( /* attr{destructor=NEXUS_AudioDecoderPrimer_Close} */
    NEXUS_AudioDecoderHandle audioDecoder
    );

/**
Summary:
Shutdown the primer.
**/
void NEXUS_AudioDecoderPrimer_Close(
    NEXUS_AudioDecoderPrimerHandle primer
    );

NEXUS_AudioDecoderPrimerHandle NEXUS_AudioDecoderPrimer_Create( /* attr{destructor=NEXUS_AudioDecoderPrimer_Close} */
    const NEXUS_AudioDecoderOpenSettings *pSettings /* attr{null_allowed=yes} */
    );
#define NEXUS_AudioDecoderPrimer_Destroy(X) NEXUS_AudioDecoderPrimer_Close(X)

/**
Summary:
Tell primer to start processing data.
**/
NEXUS_Error NEXUS_AudioDecoderPrimer_Start(
    NEXUS_AudioDecoderPrimerHandle primer,
    const NEXUS_AudioDecoderStartSettings *pStartSettings
    );

/**
Summary:
Tell primer to stop processing data.

Description:
Primers are automatically stopped if you call NEXUS_AudioDecoderPrimer_StopPrimerAndStartDecode.
**/
void NEXUS_AudioDecoderPrimer_Stop(
    NEXUS_AudioDecoderPrimerHandle primer
    );

/**
Summary:
Tell primer to Flush ( reset it's pointers ).

Description:
**/
void NEXUS_AudioDecoderPrimer_Flush( 
    NEXUS_AudioDecoderPrimerHandle primer 
    );

/**
Summary:
Start decode using a primed pid channel.

Description:
This is mutually exclusive with NEXUS_AudioDecoder_Start.
Call NEXUS_AudioDecoder_Stop, just like a non-primed channel.
After stopping decode, you must call NEXUS_AudioDecoderPrimer_Start to reactivate the primer.
If the decode start part of this call fails, both the primer and the decoder
will remain in the stopped state.  Call NEXUS_AudioDecoder_Start or
NEXUS_AudioDecoderPrimer_Start again to resume decoding or priming, respectively, as desired.
**/
NEXUS_Error NEXUS_AudioDecoderPrimer_StopPrimerAndStartDecode(
    NEXUS_AudioDecoderPrimerHandle primer,
    NEXUS_AudioDecoderHandle audioDecoder
    );

/**
Summary:
Stop Audio decoder, start primer
Description:
This function enables a seamless change (no flush of audio data) from audio decoder to audio primer.
If the primer start part of this call fails, both the primer and the decoder
will remain in the stopped state.  Call NEXUS_AudioDecoder_Start or
NEXUS_AudioDecoderPrimer_Start again to resume decoding or priming, respectively, as desired.
**/
NEXUS_Error NEXUS_AudioDecoderPrimer_StopDecodeAndStartPrimer(
    NEXUS_AudioDecoderPrimerHandle primer,
    NEXUS_AudioDecoderHandle audioDecoder
    );

/**
Summary:
TBD
**/
typedef struct NEXUS_AudioDecoderPrimerSettings
{
    unsigned ptsOffset; /* Add an offset to the primer's TSM equation. Measured in 45Hz PTS units.
                           A separate ptsOffset must be applied to AudioDecoder if desired. */
} NEXUS_AudioDecoderPrimerSettings;


/**
Summary:
Get the current NEXUS_AudioDecoderPrimerSettings from the primer.
**/
void NEXUS_AudioDecoder_GetPrimerSettings(
    NEXUS_AudioDecoderPrimerHandle primer,
    NEXUS_AudioDecoderPrimerSettings *pSettings /* [out] */
    );

/**
Summary:
Set the current NEXUS_AudioDecoderPrimerSettings from the primer.
**/
NEXUS_Error NEXUS_AudioDecoder_SetPrimerSettings(
    NEXUS_AudioDecoderPrimerHandle primer,
    const NEXUS_AudioDecoderPrimerSettings *pSettings
    );

#ifdef __cplusplus
}
#endif

#endif
