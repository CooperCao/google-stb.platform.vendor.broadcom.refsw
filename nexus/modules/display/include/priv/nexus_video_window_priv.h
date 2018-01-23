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
 * Module Description:
 *
 **************************************************************************/
#ifndef NEXUS_VIDEO_WINDOW_PRIV_H__
#define NEXUS_VIDEO_WINDOW_PRIV_H__

#include "nexus_video_window.h"
#include "priv/nexus_core_video.h"
#include "bavc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NEXUS_VideoWindowSyncSettings
{
    unsigned delay;
    
    NEXUS_Callback stateChangeCallback_isr; /* fire if window state changes, except for format and delay */
    NEXUS_Callback delayCallback_isr; /* fire if BVN delay changes */
    NEXUS_Callback formatCallback_isr; /* fire if display format changes */
    NEXUS_Callback closeCallback_isr; /* fire if window is closed */
    void *callbackContext; /* user context passed callback_isr */
} NEXUS_VideoWindowSyncSettings;

void NEXUS_VideoWindow_GetSyncSettings_priv(
    NEXUS_VideoWindowHandle handle,
    NEXUS_VideoWindowSyncSettings *pSyncSettings
    );
    
NEXUS_Error NEXUS_VideoWindow_SetSyncSettings_priv(
    NEXUS_VideoWindowHandle handle,
    const NEXUS_VideoWindowSyncSettings *pSyncSettings
    );
    
typedef struct NEXUS_VideoWindowSyncStatus 
{
    /* window */
    bool forcedCaptureEnabled; /* is forced capture enabled on this window */
    bool masterFrameRateEnabled; /* is master frame rate enabled on the main window for this display */
    bool fullScreen; /* does window rect match display rect? */
    bool visible; /* is this window visible? */
    bool syncLocked; /* is this window sync-locked? */
    bool delayValid;
    int delay; /* current delay in VSYNCs at the display rate */
    int phaseDelay; /* current sub-VSYNC delay in ms */

    /* display (format is always valid) */   
    unsigned int height; /* height of display format, required to predict VDC MAD state changes */
    bool interlaced; /* whether the display format is interlaced */
    BAVC_FrameRateCode frameRate; /* the frame rate of the display format */
    bool aligned; /* this display is aligned to another display */
    unsigned refreshRate; /* the refresh rate of the display */
} NEXUS_VideoWindowSyncStatus;
        
NEXUS_Error NEXUS_VideoWindow_GetSyncStatus_isr(
    NEXUS_VideoWindowHandle handle,
    NEXUS_VideoWindowSyncStatus *pSyncStatus
    );
    
bool NEXUS_VideoWindow_HasOutput_isr(NEXUS_VideoWindowHandle window, NEXUS_VideoOutputType type);

void NEXUS_VideoWindow_GetSettings_priv(
    NEXUS_VideoWindowHandle handle,
    NEXUS_VideoWindowSettings *pSettings
    );
NEXUS_Error NEXUS_VideoWindow_SetSettings_priv(
    NEXUS_VideoWindowHandle handle,
    const NEXUS_VideoWindowSettings *pSettings
    );

NEXUS_Error NEXUS_VideoWindow_BypassVideoProcessing_priv(
    NEXUS_DisplayHandle display,
    NEXUS_VideoWindowHandle window,
    bool bypassVideoProcessing
    );

/* return true if MAD should be enabled by default for this window */
bool NEXUS_VideoAdj_P_DefaultMadEnabled_priv(NEXUS_VideoWindowHandle window);
/* return true if this window has box mode RTS for smooth scaling */
bool NEXUS_VideoWindow_IsSmoothScaling_isrsafe(NEXUS_VideoWindowHandle window);
void NEXUS_VideoWindow_GetDefaultMinDisplayFormat_isrsafe(NEXUS_VideoWindowHandle window, NEXUS_VideoFormat *pMinDisplayFormat);
void NEXUS_VideoWindow_GetParentIndex_isrsafe(NEXUS_VideoWindowHandle window, unsigned *parentIndex, bool *isMosaic);

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_VIDEO_WINDOW_PRIV_H__ */

