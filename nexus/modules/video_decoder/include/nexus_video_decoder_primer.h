/***************************************************************************
 *     (c)2007-2013 Broadcom Corporation
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
#ifndef NEXUS_VIDEO_DECODER_PRIMER_H__
#define NEXUS_VIDEO_DECODER_PRIMER_H__

#include "nexus_video_decoder.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Handle for video decoder primer returned by NEXUS_VideoDecoderPrimer_Open
**/
typedef struct NEXUS_VideoDecoderPrimer *NEXUS_VideoDecoderPrimerHandle;

/**
Summary:
**/
typedef struct NEXUS_VideoDecoderPrimerCreateSettings 
{
    unsigned fifoSize;
    unsigned maximumGops; /* Maximum # of GOP's in CDB. This allows us to prime without a PCR.
                              Note that decode requires vsync mode, ASTM, or eAuto TSM.
                              Default is 0, which means a PCR is required. */
} NEXUS_VideoDecoderPrimerCreateSettings;

/**
Summary:
**/
void NEXUS_VideoDecoderPrimer_GetDefaultCreateSettings(
    NEXUS_VideoDecoderPrimerCreateSettings *pSettings
    );

/**
Summary:
Create a video primer

Description:
VideoDecoder is not required for priming; jowever, functions are provided to quickly transiton to/from a primer and a VideoDecoder.
**/
NEXUS_VideoDecoderPrimerHandle NEXUS_VideoDecoderPrimer_Create( /* attr{destructor=NEXUS_VideoDecoderPrimer_Destroy} */
    const NEXUS_VideoDecoderPrimerCreateSettings *pSettings /* attr{null_allowed=y} */
    );

/**
Summary:
**/
void NEXUS_VideoDecoderPrimer_Destroy(
    NEXUS_VideoDecoderPrimerHandle primer
    );
    
/**
Summary:
Tell primer to start processing data.
**/
NEXUS_Error NEXUS_VideoDecoderPrimer_Start(
    NEXUS_VideoDecoderPrimerHandle primer,
    const NEXUS_VideoDecoderStartSettings *pStartSettings
    );

/**
Summary:
Tell primer to stop processing data.

Description:
Primers are automatically stopped if you call NEXUS_VideoDecoderPrimer_StopPrimerAndStartDecode.
**/
void NEXUS_VideoDecoderPrimer_Stop(
    NEXUS_VideoDecoderPrimerHandle primer
    );

/**
Summary:
Start decode using a primed pid channel.

Description:
This is mutually exclusive with NEXUS_VideoDecoder_Start.
Call NEXUS_VideoDecoder_Stop, just like a non-primed channel.
After stopping decode, you must call NEXUS_VideoDecoderPrimer_Start to reactivate the primer.
**/
NEXUS_Error NEXUS_VideoDecoderPrimer_StopPrimerAndStartDecode(
    NEXUS_VideoDecoderPrimerHandle primer,
    NEXUS_VideoDecoderHandle videoDecoder
    );

/**
Summary:
Start priming using a pid channel currently being decoded.

Description:
This is mutually exclusive with NEXUS_VideoDecoderPrimer_Start.
**/
NEXUS_Error NEXUS_VideoDecoderPrimer_StopDecodeAndStartPrimer(
    NEXUS_VideoDecoderPrimerHandle primer,
    NEXUS_VideoDecoderHandle videoDecoder
    );

/**
Summary:
Get the current NEXUS_VideoDecoderPrimerSettings from the primer.
**/
void NEXUS_VideoDecoderPrimer_GetSettings(
    NEXUS_VideoDecoderPrimerHandle primer,
    NEXUS_VideoDecoderPrimerSettings *pSettings /* [out] */
    );

/**
Summary:
Set the current NEXUS_VideoDecoderPrimerSettings from the primer.
**/
NEXUS_Error NEXUS_VideoDecoderPrimer_SetSettings(
    NEXUS_VideoDecoderPrimerHandle primer,
    const NEXUS_VideoDecoderPrimerSettings *pSettings
    );

/**
Summary:
Populates only those fields that are relavant for primer: pts, fifoDepth, fifoSize.
**/
NEXUS_Error NEXUS_VideoDecoderPrimer_GetStatus(
    NEXUS_VideoDecoderPrimerHandle primer,
    NEXUS_VideoDecoderStatus *pStatus
    );

/* deprecated */
#define NEXUS_VideoDecoder_OpenPrimer(VIDEODECODER) NEXUS_VideoDecoderPrimer_Create(NULL)
#define NEXUS_VideoDecoder_ClosePrimer(VIDEODECODER,PRIMER) NEXUS_VideoDecoderPrimer_Destroy(PRIMER)
#define NEXUS_VideoDecoder_StartPrimer(VIDEODECODER,PRIMER,PSETTINGS) NEXUS_VideoDecoderPrimer_Start(PRIMER,PSETTINGS)
#define NEXUS_VideoDecoder_StopPrimer(VIDEODECODER,PRIMER) NEXUS_VideoDecoderPrimer_Stop(PRIMER)
#define NEXUS_VideoDecoder_GetPrimerSettings(PRIMER,PSETTINGS) NEXUS_VideoDecoderPrimer_GetSettings(PRIMER,PSETTINGS)
#define NEXUS_VideoDecoder_SetPrimerSettings(PRIMER,PSETTINGS) NEXUS_VideoDecoderPrimer_SetSettings(PRIMER,PSETTINGS)
#define NEXUS_VideoDecoder_StartDecodeWithPrimer(VIDEODECODER,PRIMER) NEXUS_VideoDecoderPrimer_StopPrimerAndStartDecode(PRIMER,VIDEODECODER)

NEXUS_VideoDecoderPrimerHandle NEXUS_VideoDecoderPrimer_Open( /* attr{destructor=NEXUS_VideoDecoderPrimer_Destroy} */
    NEXUS_VideoDecoderHandle videoDecoder /* attr{null_allowed=y} */
    );
void NEXUS_VideoDecoderPrimer_Close( NEXUS_VideoDecoderPrimerHandle primer );

#ifdef __cplusplus
}
#endif

#endif
