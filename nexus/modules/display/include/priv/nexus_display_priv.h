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
#ifndef NEXUS_DISPLAY_PRIV_H__
#define NEXUS_DISPLAY_PRIV_H__

#include "nexus_display.h"
#include "nexus_hddvi_input.h"
#include "nexus_video_image_input.h"
#include "nexus_ccir656_input.h"
#include "nexus_ccir656_output.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#include "nexus_svideo_output.h"
#if NEXUS_HAS_VIDEO_ENCODER
#include "bxudlib.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

NEXUS_Error NEXUS_DisplayModule_SetUpdateMode_priv(
    NEXUS_DisplayUpdateMode updateMode,
    NEXUS_DisplayUpdateMode *pPrevUpdateMode /* optional */
    );

#if NEXUS_HAS_VIDEO_ENCODER
void NEXUS_DisplayModule_ClearDisplay_priv(
        NEXUS_DisplayHandle display
        );

NEXUS_Error NEXUS_DisplayModule_SetUserDataEncodeMode_priv(
    NEXUS_DisplayHandle display,
    bool encodeUserData,
    BXUDlib_Settings *userDataEncodeSettings,
    NEXUS_VideoWindowHandle udWindow
    );
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
typedef struct NEXUS_DisplayCapturedImage {
    BMMA_Block_Handle   hImage;
    unsigned            offset;
    unsigned            width;
    unsigned            height;
    unsigned            encPicId;
    unsigned            decPicId;
    BAVC_Polarity       polarity;
    uint32_t            origPts;
    NEXUS_VideoFrameRate framerate;
    uint32_t             stallStc;
    uint32_t             ignorePicture;
    uint32_t             aspectRatioX;
    uint32_t             aspectRatioY;
} NEXUS_DisplayCapturedImage;

typedef struct NEXUS_DisplayEncoderSettings {
#if NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    NEXUS_Error (*enqueueCb_isr)(void *context, BAVC_EncodePictureBuffer *picture);
    NEXUS_Error (*dequeueCb_isr)(void *context, BAVC_EncodePictureBuffer *picture);
#else
    NEXUS_Error (*enqueueCb_isr)(void *context, NEXUS_DisplayCapturedImage *image);
    NEXUS_Error (*dequeueCb_isr)(void *context, NEXUS_DisplayCapturedImage *image);
#endif
    void *context;
    NEXUS_VideoFrameRate encodeRate;
    uint32_t extIntAddress;
    uint32_t extIntBitNum;
#if NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    uint32_t stcSnapshotLoAddr; /* for new soft transcoder STC snapshot */
    uint32_t stcSnapshotHiAddr;
    struct {
        BMMA_Heap_Handle   hHeap; /* for new soft transcoder VIP picture heap. If NULL, disabled VIP */
        BVDC_VipMemConfigSettings stMemSettings;
    } vip;
#endif
} NEXUS_DisplayEncoderSettings;

#if NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
unsigned NEXUS_Display_GetStgIndex_priv(NEXUS_DisplayHandle display);
#endif
NEXUS_Error NEXUS_Display_SetEncoderCallback_priv(NEXUS_DisplayHandle display, NEXUS_VideoWindowHandle window, NEXUS_DisplayEncoderSettings *pSettings);
NEXUS_Error NEXUS_Display_EnableEncoderCallback_priv(NEXUS_DisplayHandle display);
NEXUS_Error NEXUS_Display_DisableEncoderCallback_priv(NEXUS_DisplayHandle display);
#else
NEXUS_Error NEXUS_DisplayModule_SetStgResolutionRamp_priv(
    NEXUS_DisplayHandle display,
    unsigned rampFrameCount
    );
#endif /* NEXUS_NUM_DSP_VIDEO_ENCODERS */
#endif /* NEXUS_HAS_VIDEO_ENCODER  */

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_ComponentOutput);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_SvideoOutput);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_CompositeOutput);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_Ccir656Output);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_Display);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_HdDviInput);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_VideoImageInput);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_Ccir656Input);

NEXUS_Error NEXUS_DisplayModule_GetStatus_priv(NEXUS_DisplayModuleStatus *pStatus);
unsigned NEXUS_Display_GetLastVsyncTime_isr(NEXUS_DisplayHandle display);

/* Some chips require a connected HDMI output before any window is created.
If this is done, an external NEXUS_Display_AddOutput/NEXUS_Display_RemoveOutput is a no-op.
*/
NEXUS_Error NEXUS_DisplayModule_AddRequiredOutput_priv(NEXUS_VideoOutput output);
void        NEXUS_DisplayModule_RemoveRequiredOutput_priv(NEXUS_VideoOutput output);

/* returns non-zero without BERR_TRACE is window is unused */
NEXUS_Error NEXUS_Display_P_GetWindowMemc_isrsafe(unsigned displayIndex, unsigned windowIndex, unsigned *pMemcIndex);

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_DISPLAY_PRIV_H__ */
