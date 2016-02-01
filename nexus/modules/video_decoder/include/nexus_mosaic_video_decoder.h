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
 ***************************************************************************/
#ifndef NEXUS_MOSAIC_VIDEO_DECODER_H__
#define NEXUS_MOSAIC_VIDEO_DECODER_H__

#include "nexus_video_decoder.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=****************
In mosaic mode, the video decoder timeslices its main decoder in order to decoder several lower resolution streams
(e.g. CIF or QCIF sized). Each mosaic stream is routed to a special mosaic video window.

See nexus/modules/display/CHIP/include/nexus_mosaic_display.h for the mosaic mode video window API.

See nexus/examples/video/mosaic_decode.c and nexus/examples/dvr/mosaic_playback.c for an example of how to use the
mosaic decode and display API's.
*/

/**
Summary:
Wrapper around normal video decoder settings to provide additional mosaic settings.

Description:
See Also:
NEXUS_VideoDecoder_GetDefaultOpenMosaicSettings
NEXUS_VideoDecoder_OpenMosaic
**/
typedef struct NEXUS_VideoDecoderOpenMosaicSettings
{
    NEXUS_VideoDecoderOpenSettings openSettings; /* normal video decoder settings */
    unsigned maxWidth, maxHeight;     /* Deprecated. Use NEXUS_VideoDecoderSettings.maxWidth and .maxHeight instead. These will be applied
                                         if different from default values. */
    struct {
        bool enabled;
        unsigned avdIndex;
    } linkedDevice; /* allow mosaics on HVD1 to flow through HVD0 feeder */
} NEXUS_VideoDecoderOpenMosaicSettings;

/**
Summary:
Get default settings for use in NEXUS_VideoDecoder_OpenMosaic.

Description:
See Also:
NEXUS_VideoDecoder_OpenMosaic
**/
void NEXUS_VideoDecoder_GetDefaultOpenMosaicSettings(
    NEXUS_VideoDecoderOpenMosaicSettings *pOpenSettings   /* [out] */
    );

/**
Summary:
Open a mosaic video decoder within the parent video decoder.

Description:
If you open a mosaic video decoder, so can no longer use the parent video decoder.
Once you close the last mosaic video decoder, you can use the parent video decoder for a non-mosaic decode.

See Also:
NEXUS_VideoDecoder_OpenMosaic
NEXUS_VideoDecoder_Close - use the normal close function for mosaic video decoders
**/
NEXUS_VideoDecoderHandle NEXUS_VideoDecoder_OpenMosaic( /* attr{destructor=NEXUS_VideoDecoder_Close}  */
    unsigned parentIndex,       /* The index of the main decoder which will be used for mosaic decode. You cannot
                                   open a main decoder with this index (using NEXUS_VideoDecoder_Open()) at the same time.
                                   Bounded by NEXUS_MAX_VIDEO_DECODERS. */
    unsigned index,             /* The index of the mosaic within the main decoder. These must be sequential.
                                   The index of the mosaic must correspond to the index of the mosaic VideoWindow. This is no sharing of handles
                                   to make this association.
                                   For now, bounded by NEXUS_NUM_MOSAIC_DECODES. */
    const NEXUS_VideoDecoderOpenMosaicSettings *pOpenSettings /* attr{null_allowed=y} Settings for the video decoder which include normal VideoDecoder settings as well
                                   as mosaic-specific settings. */
    );
    
/**
Summary:
Run-time mosaic decoder settings
**/
typedef struct NEXUS_VideoDecoderMosaicSettings
{
    unsigned maxWidth, maxHeight; /* Deprecated. Use NEXUS_VideoDecoderSettings.maxWidth and .maxHeight instead. These will be applied
                                         if different from default values. */
} NEXUS_VideoDecoderMosaicSettings;

/**
Summary:
If not a mosaic, it will return HD settings.
**/
void NEXUS_VideoDecoder_GetMosaicSettings(
    NEXUS_VideoDecoderHandle handle,
    NEXUS_VideoDecoderMosaicSettings *pSettings /* [out] */
    );
    
/**
Summary:
**/
NEXUS_Error NEXUS_VideoDecoder_SetMosaicSettings(
    NEXUS_VideoDecoderHandle handle,
    const NEXUS_VideoDecoderMosaicSettings *pSettings   
    );
    
#ifdef __cplusplus
}
#endif

#endif
