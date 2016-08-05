/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2007-2016 Broadcom. All rights reserved.
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
 **************************************************************************/
/* this file shall be included only from nexus_display_module.h */
#ifndef NEXUS_VIDEO_WINDOW_IMPL_H__
#define NEXUS_VIDEO_WINDOW_IMPL_H__

void NEXUS_VideoWindow_P_InitState(NEXUS_VideoWindowHandle window, unsigned parentIndex, unsigned index, NEXUS_DisplayHandle display);
BERR_Code NEXUS_VideoWindow_P_SetVdcSettings(NEXUS_VideoWindowHandle window, const NEXUS_VideoWindowSettings *settings, bool force);

BERR_Code NEXUS_VideoWindow_P_CreateVdcWindow(NEXUS_VideoWindowHandle window, const NEXUS_VideoWindowSettings *cfg);
void NEXUS_VideoWindow_P_DestroyVdcWindow(NEXUS_VideoWindowHandle window);

NEXUS_Error NEXUS_VideoWindow_P_AddMosaicInput(NEXUS_VideoWindowHandle window, NEXUS_VideoInput input);
void NEXUS_VideoWindow_P_RemoveMosaicInput(NEXUS_VideoWindowHandle window, NEXUS_VideoInput input);
NEXUS_Error NEXUS_VideoWindow_P_ApplyMosaic(NEXUS_VideoWindowHandle window);
const BAVC_MFD_Picture *NEXUS_VideoWindow_P_CutBackendMosaic_isr(NEXUS_VideoWindowHandle window, const BAVC_MFD_Picture *pPicture);

NEXUS_Error NEXUS_VideoWindow_P_ConfigMasterFrameRate(NEXUS_VideoWindowHandle window, const NEXUS_DisplaySettings *pDisplaySettings, const NEXUS_VideoWindowSettings *pWindowsettings);
void NEXUS_VideoWindow_P_UpdatePhaseDelay_isr(NEXUS_VideoWindowHandle window, unsigned refreshRate);

struct NEXUS_VideoWindow {
    NEXUS_OBJECT(NEXUS_VideoWindow);
    NEXUS_DisplayHandle display; /* pointer back to display */
    NEXUS_VideoWindowSettings cfg;
    NEXUS_VideoWindowSplitScreenSettings splitScreenSettings;
    NEXUS_VideoWindowStatus status;
    int phaseDelay; /* internal variable used to store phase delay as communicated by VDC, necessary for computing public phase delay later */
    bool open;
    bool layoutChanged;
    NEXUS_IsrCallbackHandle letterBoxDetectionCallback;
    NEXUS_IsrCallbackHandle outputRectChangedCallback;
    NEXUS_VideoWindowLetterBoxStatus letterBoxStatus;
    BKNI_EventHandle lb_event; /* for pcinput auto position lb return data */
    BKNI_EventHandle syncLockEvent; /* for propagating sync-locked display info to decoder */
    NEXUS_EventCallbackHandle syncLockCallback;
#if BVDC_BUF_LOG && NEXUS_BASE_OS_linuxuser
    BKNI_EventHandle bufLogEvent; /* for capturing buffer debug log to file */
    NEXUS_EventCallbackHandle bufLogCallback;
#endif
    bool bypassVideoProcessing;

    NEXUS_SurfaceHandle captureBuffer; /* current surface returned by NEXUS_VideoWindow_CaptureVideoBuffer */
    BVDC_Window_CapturedImage captureImage; /* image from  BVDC_Window_GetBuffer */

    NEXUS_VideoWindowSyncSettings syncSettings;
    NEXUS_VideoWindowSyncStatus syncStatus;

    NEXUS_PixelFormat pixelFormat;

    NEXUS_ColorMatrix colorMatrix;
    bool colorMatrixSet;
    bool colorMatrixOverride;

    NEXUS_VideoWindowAfdSettings afdSettings;
    bool afdSet;

    unsigned index; /* 0 for main, 1 for pip. windowId is redundant and deprecated. */
    BVDC_WindowId windowId;
    BVDC_Heap_Handle vdcHeap, vdcDeinterlacerHeap;

#if NEXUS_NUM_MOSAIC_DECODES
    struct {
        /* fields for mosaic parent */
        BLST_S_HEAD(NEXUS_VideoInput_P_MosaicChildren, NEXUS_VideoWindow) children;

        /* fields for mosaic children */
        NEXUS_VideoWindowHandle parent;
        BLST_S_ENTRY(NEXUS_VideoWindow) link;
        unsigned userIndex; /* external window index, only used for duplicate index checking */
        NEXUS_VideoWindowMosaicSettings mosaicSettings;
        BAVC_MFD_Picture picture; /* used for backend */
    } mosaic;
#endif

    NEXUS_VideoAdjContext adjContext;
    NEXUS_PictureCtrlContext picContext;
    NEXUS_VideoInput input;
    NEXUS_VideoInput standby;
    struct {
        BVDC_Window_Handle window; /* could be NULL */
        bool masterFrameRate;
    } vdcState;

    struct {
        NEXUS_VideoWindowCrcData *queue;
        unsigned size; /* num entries, not num bytes */
        unsigned rptr, wptr;
    } crc;
#if NEXUS_HAS_SAGE
    struct {
        BAVC_CoreList coreList;
    } sage;
#endif
};

#define NEXUS_DISPLAY_ENCODER_MAX_PICTURE_BUFFERS NEXUS_NUM_DSP_ENCODER_PICTURE_BUFFERS

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_VideoWindow);
#endif /* NEXUS_VIDEO_WINDOW_IMPL_H__ */
