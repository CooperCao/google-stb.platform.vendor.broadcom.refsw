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
 ***************************************************************************/
#ifndef NEXUS_MOSAIC_DISPLAY_H__
#define NEXUS_MOSAIC_DISPLAY_H__

#include "nexus_display.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=****************
In mosaic mode, the video decoder timeslices its main decoder in order to decoder several lower resolution streams
(e.g. CIF or QCIF sized). Each mosaic stream is routed to a special mosaic video window.

You must call NEXUS_VideoWindow_AddInput for one, not all, of the mosaic VideoDecoders.

See nexus/modules/video_decoder/CHIP/include/nexus_mosaic_video_decoder.h for the mosaic mode video decoder API.

See nexus/examples/video/mosaic_decode.c for an example of how to use the mosaic decode and display API's.
*/

/**
Summary:
Open a mosaic window

Description:
Only a subset of VideoWindow functionality is available for mosaics that is:
    NEXUS_VideoWindowSettings.position
    NEXUS_VideoWindowSettings.zorder
    NEXUS_VideoWindowSettings.visible
    NEXUS_VideoWindow_AddInput

You must connect a mosaic VideoDecoder to a mosaic VideoWindow.
All mosaic VideoDecoder instances must be part of the same parent VideoDecoder.
**/
NEXUS_VideoWindowHandle NEXUS_VideoWindow_OpenMosaic( /* attr{destructor=NEXUS_VideoWindow_Close}  */
    NEXUS_VideoWindowHandle parentWindow,
    unsigned mosaicIndex
    );

/**
Summary:
Mosaic-only window settings
**/
typedef struct NEXUS_VideoWindowMosaicSettings
{
    struct {
        bool enabled; /* Set to true if the decoder is not in mosaic mode and you want the display module to
                         manually cut up the picture into mosaics. This will be done using backendMosaic.clipRect and backendMosaic.baseRect.
                         This must be set before NEXUS_VideoWindow_AddInput is called for any mosaic window.
                         This is used for 3DTV decoding support. */
        NEXUS_Rect clipRect;
        NEXUS_Rect clipBase;
    } backendMosaic;
} NEXUS_VideoWindowMosaicSettings;

/**
Summary:
**/
void NEXUS_VideoWindow_GetMosaicSettings(
    NEXUS_VideoWindowHandle window,
    NEXUS_VideoWindowMosaicSettings *pSettings /* [out] */
    );

/**
Summary:
**/
NEXUS_Error NEXUS_VideoWindow_SetMosaicSettings(
    NEXUS_VideoWindowHandle window,
    const NEXUS_VideoWindowMosaicSettings *pSettings
    );

#ifdef __cplusplus
}
#endif

#endif

