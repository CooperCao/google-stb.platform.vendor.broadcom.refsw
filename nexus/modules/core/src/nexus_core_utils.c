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
* API Description:
*   API name: Base
*   It also initializes the magnum base modules. And provides system level
*   interrupt routing
*
***************************************************************************/
#include "nexus_core_module.h"
#include "bkni.h"
#include "bfmt.h"

BDBG_MODULE(nexus_core_utils);

NEXUS_VideoFormat g_NEXUS_customVideoFormat;
NEXUS_VideoFormatInfo g_NEXUS_customVideoFormatInfo;

void NEXUS_P_VideoFormat_SetInfo( NEXUS_VideoFormat videoFormat, const NEXUS_VideoFormatInfo *pInfo )
{
    g_NEXUS_customVideoFormat = videoFormat;
    g_NEXUS_customVideoFormatInfo = *pInfo;
}

void NEXUS_VideoFormat_GetInfo_isrsafe(NEXUS_VideoFormat videoFormat, NEXUS_VideoFormatInfo *pInfo)
{
    const BFMT_VideoInfo *fmtInfo;
    BERR_Code rc = BERR_SUCCESS;
    BFMT_VideoFmt fmt;

    BKNI_Memset(pInfo, 0, sizeof(*pInfo));

    /* allow for one custom video format at a time. TODO: consider extending this if needed. */
    if (videoFormat == g_NEXUS_customVideoFormat) {
        *pInfo = g_NEXUS_customVideoFormatInfo;
        return;
    }

    rc = NEXUS_P_VideoFormat_ToMagnum_isrsafe(videoFormat, &fmt);
    if (rc) {BERR_TRACE(rc); return;}

    fmtInfo = BFMT_GetVideoFormatInfoPtr_isrsafe(fmt);
    if (!fmtInfo) {BERR_TRACE(NEXUS_INVALID_PARAMETER); return;}

    BDBG_MSG(("NEXUS_VideoFormat_GetInfo %d->%d: %dx%d", videoFormat, fmt, fmtInfo->ulWidth, fmtInfo->ulHeight));

    pInfo->width = fmtInfo->ulWidth;
    pInfo->height = fmtInfo->ulHeight;
    pInfo->digitalWidth = fmtInfo->ulDigitalWidth;
    pInfo->digitalHeight = fmtInfo->ulDigitalHeight;
    pInfo->scanWidth = fmtInfo->ulScanWidth;
    pInfo->scanHeight = fmtInfo->ulScanHeight;
    pInfo->topActive = fmtInfo->ulTopActive;
    pInfo->topMaxVbiPassThru = fmtInfo->ulTopMaxVbiPassThru;
    pInfo->bottomMaxVbiPassThru = fmtInfo->ulBotMaxVbiPassThru;
    pInfo->verticalFreq = fmtInfo->ulVertFreq;
    pInfo->interlaced = fmtInfo->bInterlaced;
    pInfo->aspectRatio = fmtInfo->eAspectRatio;
    pInfo->pixelFreq = fmtInfo->ulPxlFreq;

    pInfo->isFullRes3d = BFMT_IS_3D_MODE(fmt);
    return;
}

void NEXUS_VideoFormat_GetInfo(NEXUS_VideoFormat videoFormat, NEXUS_VideoFormatInfo *pInfo)
{
    NEXUS_VideoFormat_GetInfo_isrsafe(videoFormat, pInfo);
}

void NEXUS_PixelFormat_GetInfo( NEXUS_PixelFormat pixelFormat, NEXUS_PixelFormatInfo *pInfo )
{
    const NEXUS_PixelFormatConvertInfo *info = NEXUS_P_PixelFormat_GetConvertInfo_isrsafe(pixelFormat);
    if (info) {
        *pInfo = info->info;
    }
    else {
        BKNI_Memset(pInfo, 0, sizeof(*pInfo));
    }
}

void
NEXUS_LookupFrameRate(unsigned frameRateInteger, NEXUS_VideoFrameRate *pNexusFrameRate)
{
    NEXUS_P_FrameRate_FromRefreshRate_isrsafe(frameRateInteger, pNexusFrameRate);
    return;
}

NEXUS_Error NEXUS_VideoFrameRate_GetRefreshRate( NEXUS_VideoFrameRate frameRate, unsigned *pRefreshRate )
{
    *pRefreshRate = NEXUS_P_RefreshRate_FromFrameRate_isrsafe(frameRate);
    return (!frameRate || *pRefreshRate) ? NEXUS_SUCCESS : NEXUS_INVALID_PARAMETER;
}

struct NEXUS_FrontendConnector
{
    NEXUS_OBJECT(NEXUS_FrontendConnector);
};

NEXUS_FrontendConnectorHandle NEXUS_FrontendConnector_Create(void)
{
    NEXUS_FrontendConnectorHandle handle;
    handle = BKNI_Malloc(sizeof(*handle));
    if (!handle) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_FrontendConnector, handle);
    NEXUS_OBJECT_REGISTER(NEXUS_FrontendConnector, handle, Create);
    return handle;
}

NEXUS_FrontendConnectorHandle NEXUS_FrontendConnector_Acquire(unsigned index)
{
    BSTD_UNUSED(index);
    return NULL;
}

void NEXUS_FrontendConnector_Release(NEXUS_FrontendConnectorHandle handle)
{
    BSTD_UNUSED(handle);
}  

static void NEXUS_FrontendConnector_P_Release( NEXUS_FrontendConnectorHandle handle )
{
    NEXUS_OBJECT_UNREGISTER(NEXUS_FrontendConnector, handle, Destroy);
}

static void NEXUS_FrontendConnector_P_Finalizer( NEXUS_FrontendConnectorHandle handle )
{
    NEXUS_OBJECT_ASSERT(NEXUS_FrontendConnector, handle);
    NEXUS_OBJECT_DESTROY(NEXUS_FrontendConnector, handle);
    BKNI_Free(handle);
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_FrontendConnector, NEXUS_FrontendConnector_Destroy);
