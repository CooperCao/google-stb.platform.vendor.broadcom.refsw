/***************************************************************************
 *     (c)2010-2014 Broadcom Corporation
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
#ifndef NEXUS_SIMPLE_VIDEO_DECODER_SERVER_H__
#define NEXUS_SIMPLE_VIDEO_DECODER_SERVER_H__

#include "nexus_types.h"
#include "nexus_simple_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
This server-side is semi-private. In multi-process systems, only server apps like nxserver will call it.
Client apps will not call it. Therefore, this API is subject to non-backward compatible change.
**/

/**
Summary:

Simple decoder assumes videoDecoder is already connected to the windows
**/
typedef struct NEXUS_SimpleVideoDecoderServerSettings
{
    bool enabled;
    NEXUS_SimpleDecoderDisableMode disableMode;
    NEXUS_VideoDecoderHandle videoDecoder; /* by giving the VideoDecoder handle, the caller gives up all ability to any VideoDecoder API directly */
    NEXUS_VideoWindowHandle window[NEXUS_MAX_DISPLAYS]; /* by giving the VideoWindow handle, the caller gives up the ability to call 
        NEXUS_VideoWindow_AddInput, RemoveInput and RemoveAllInputs. the caller can still use other VideoWindow API's. for instance, 
        it can call NEXUS_VideoWindow_SetSettings to move the video window. */
    int stcIndex; /* used for allocating stc channel once connected to simple stc channel */
    NEXUS_DisplayHandle display[NEXUS_MAX_DISPLAYS]; /* used for closedCaptionRouting */
    bool mosaic;
} NEXUS_SimpleVideoDecoderServerSettings;

/**
Summary:
**/
void NEXUS_SimpleVideoDecoder_GetDefaultServerSettings(
    NEXUS_SimpleVideoDecoderServerSettings *pSettings /* [out] */
    );

/**
Summary:
Server could set up multiple SimpleVideoDecoders for one VideoDecoder.
This would allow it to create multiple ways of using the same decoder.
**/
NEXUS_SimpleVideoDecoderHandle NEXUS_SimpleVideoDecoder_Create( /* attr{destructor=NEXUS_SimpleVideoDecoder_Destroy}  */
    unsigned index,
    const NEXUS_SimpleVideoDecoderServerSettings *pSettings
    );

/**
Summary:
Server is responsible for ensuring that all clients have called NEXUS_SimpleVideoDecoder_Release.
If any handle is not released beforehand, there will likely be system failure.
If a client will not release its handle, the client process should be forcibly terminated before calling Destroy.
**/
void NEXUS_SimpleVideoDecoder_Destroy(
    NEXUS_SimpleVideoDecoderHandle handle
    );

/**
Summary:
**/
void NEXUS_SimpleVideoDecoder_GetServerSettings(
    NEXUS_SimpleVideoDecoderHandle handle,
    NEXUS_SimpleVideoDecoderServerSettings *pSettings /* [out] */
    );

/**
Summary:
This might cause a stop/start, unbeknownst to the client
Even the VideoDecoder could be taken away while Simple is acquired and even started
**/
NEXUS_Error NEXUS_SimpleVideoDecoder_SetServerSettings(
    NEXUS_SimpleVideoDecoderHandle handle,
    const NEXUS_SimpleVideoDecoderServerSettings *pSettings
    );

/**
Allow simple decoder module to cache handles passed in with NEXUS_SimpleVideoDecoderServerSettings.
Must set to false before destroying those handles.
**/
void NEXUS_SimpleVideoDecoderModule_SetCacheEnabled(
    bool enabled
    );

/**
To speed up channel changes, simple decoder defers tear down of decoder/window connections.
However, if a decoder is destroyed, or if a window will be repurposed, simple decoder must be informed
so it can clear its cache.

If window and videoDecoder are set, then any connection with that window or videoDecoder which is not same must be broken.
By implication, if neither are set then all connections must be broken.
**/
void NEXUS_SimpleVideoDecoderModule_CheckCache(
    NEXUS_VideoWindowHandle window, /* attr{null_allowed=y} */
    NEXUS_VideoDecoderHandle videoDecoder /* attr{null_allowed=y} */
    );

/**
Summary:
Allow simple decoder to load default settings from a freshly opened handle
**/
void NEXUS_SimpleVideoDecoderModule_LoadDefaultSettings(
    NEXUS_VideoDecoderHandle video
    );

/**
Summary:
Return StcIndex being used by started video decoder or primer.
**/
void NEXUS_SimpleVideoDecoder_GetStcIndex(
    NEXUS_SimpleVideoDecoderHandle handle,
    int *pStcIndex /* returns -1 if StcChannel is unused */
    );

NEXUS_Error NEXUS_SimpleVideoDecoder_SwapWindows(
    NEXUS_SimpleVideoDecoderHandle decoder1,
    NEXUS_SimpleVideoDecoderHandle decoder2
    );

#ifdef __cplusplus
}
#endif

#endif
