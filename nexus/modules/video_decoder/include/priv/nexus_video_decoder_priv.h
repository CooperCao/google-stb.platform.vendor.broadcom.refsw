/***************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#ifndef NEXUS_VIDEO_DECODER_PRIV_H__
#define NEXUS_VIDEO_DECODER_PRIV_H__

#include "nexus_video_decoder.h"
#include "priv/nexus_core_video.h"
#include "bint.h"
#include "bavc.h"
#include "bxvd.h"
#include "bxdm_pp.h"
#include "bbox.h"

/*-**************
Private API for VideoDecoder.
****************/

#ifdef __cplusplus
extern "C" {
#endif

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_VideoDecoder);

/**
Summary:
**/
typedef void (*NEXUS_VideoDecoderDataReadyCallback)(void *context, const BAVC_MFD_Picture *picture);

/**
Summary:
**/
typedef void (*NEXUS_VideoDecoderUserDataCallback)(void *context, const BAVC_USERDATA_info *userDataInfo);


/**
Summary:
VBI isr callback from VideoDecoder to Display.
**/
typedef void (*NEXUS_DigitalVbiDataCallback)(
    NEXUS_VideoInput input,
    bool is608data, /* true if EIA-608 */
    const NEXUS_ClosedCaptionData *pData /* one item of EIA-608 or EIA-708 data. */
    );

struct NEXUS_VideoDecoderStcSnapshot
{
    bool set;
    unsigned trigger;
};
void NEXUS_VideoDecoder_SetStcSnapshot_priv(
    NEXUS_VideoDecoderHandle handle,
    const struct NEXUS_VideoDecoderStcSnapshot *pStcSnapshot
    );

/**
Summary:
Data needed to connect VideoDecoder to a Display.
**/
typedef struct NEXUS_VideoDecoderDisplayConnection
{
    struct
    {
        BINT_Id intId; /* These L2 interrupt ID's allow the VideoDecoder to receive VEC interrupts to drive the display manager. */
    } top, bottom, frame;

    NEXUS_VideoDecoderDataReadyCallback dataReadyCallback_isr; /* Allows VideoDecoder to notify Display when a new picture is available. */

    NEXUS_VideoDecoderUserDataCallback userDataCallback_isr; /* Allows VideoDecoder to pass user data to display for VEC encoding */

    NEXUS_DigitalVbiDataCallback vbiDataCallback_isr; /* Allows VideoInput to pass VBI data to Display for user capture or VEC encoding. */

    void *callbackContext;

    /* status */
    unsigned parentIndex; /* parentIndex corresponds to the MFD index.
                             For mosaic, it's the parent index. For non-mosaic, it's the decoder index. */
    NEXUS_VideoDecoderSecureType secureVideo;
} NEXUS_VideoDecoderDisplayConnection;

#if NEXUS_HAS_ASTM
/**
Summary:
Used by Astm
**/
typedef struct NEXUS_VideoDecoderAstmSettings
{
    bool enableAstm; /* Astm will enable/disable astm mode */
    unsigned int syncLimit; /* limit outside of which decoder will not apply astm mode */
    bool enableTsm; /* Astm will enable/disable tsm. */
    bool enablePlayback; /* Astm will enable/disable playback. */
    int32_t ptsOffset; /* Astm will use this while in TSM-disabled mode */
    NEXUS_Callback firstPts_isr; /* Notify Astm when First PTS occurs. */
    NEXUS_Callback tsmPass_isr; /* Notify Astm when TSM passes. */
    NEXUS_Callback tsmFail_isr; /* Notify Astm when TSM fails. */
    NEXUS_Callback tsmLog_isr; /* Notify Astm when TSM log buffer fills */
    NEXUS_Callback lifecycle_isr; /* Notify Astm when decoder is started/stopped */
    NEXUS_Callback watchdog_isr; /* Notify Astm when a watchdog occurs */
    void *callbackContext; /* user context passed callback_isr */
} NEXUS_VideoDecoderAstmSettings;

/**
Summary:
Used by Astm
**/
typedef struct NEXUS_VideoDecoderAstmStatus
{
    bool started;
    uint32_t pts;
    int32_t ptsStcDiff;
    unsigned decodedCount;
    struct
    {
        unsigned int address;
        unsigned int size;
    } tsmLog;
} NEXUS_VideoDecoderAstmStatus;

#endif /* NEXUS_HAS_ASTM */

/**
Summary:
Get current connection to Display
**/
void NEXUS_VideoDecoder_GetDisplayConnection_priv(
    NEXUS_VideoDecoderHandle handle,
    NEXUS_VideoDecoderDisplayConnection *pConnection /* [out] */
    );

/**
Summary:
Update connection to Display
**/
NEXUS_Error NEXUS_VideoDecoder_SetDisplayConnection_priv(
    NEXUS_VideoDecoderHandle handle,
    const NEXUS_VideoDecoderDisplayConnection *pConnection
    );

/**
Summary:
BAVC_SourceId indicates the MPEG feeder (MFD) where the picture will be available
**/
void NEXUS_VideoDecoder_GetSourceId_priv(
    NEXUS_VideoDecoderHandle handle,
    BAVC_SourceId *pSource /* [out] */
    );

/**
Summary:
Heap used by the video decoder
**/
void NEXUS_VideoDecoder_GetHeap_priv(
    NEXUS_VideoDecoderHandle handle,
    NEXUS_HeapHandle *pHeap /* [out] */
    );

/**
Summary:
SyncChannel calls VideoInput which calls this.
**/
void NEXUS_VideoDecoder_GetSyncSettings_priv(
    NEXUS_VideoDecoderHandle videoDecoder,
    NEXUS_VideoInputSyncSettings *pSyncSettings /* [out] */
    );

/**
Summary:
SyncChannel calls VideoInput which calls this.
**/
NEXUS_Error NEXUS_VideoDecoder_SetSyncSettings_priv(
    NEXUS_VideoDecoderHandle videoDecoder,
    const NEXUS_VideoInputSyncSettings *pSyncSettings
    );

/**
Summary:
SyncChannel calls VideoInput which calls this.
**/
NEXUS_Error NEXUS_VideoDecoder_GetSyncStatus_isr(
    NEXUS_VideoDecoderHandle videoDecoder,
    NEXUS_VideoInputSyncStatus *pSyncStatus /* [out] */
    );

typedef struct NEXUS_VideoDecoder_DisplayInformation {
    unsigned refreshRate; /* refresh rate in units of 1/100th Hz.  For example 60.00Hz would be 6000, and 59.94Hz would be 5994
                             this is the frequency at which the display will consume the decoded pictures */
    unsigned stgIndex; /* if display type is simple time genenrator, this would be index (number) of STG hardware, otherwise undefined */
} NEXUS_VideoDecoder_DisplayInformation;

void NEXUS_VideoDecoder_UpdateDisplayInformation_priv(
    NEXUS_VideoDecoderHandle videoDecoder,
    const NEXUS_VideoDecoder_DisplayInformation *displayInformation
    );



#if NEXUS_HAS_ASTM
/**
Summary:
Astm calls this.
**/
void NEXUS_VideoDecoder_GetAstmSettings_priv(
    NEXUS_VideoDecoderHandle videoDecoder,
    NEXUS_VideoDecoderAstmSettings * pAstmSettings /* [out] */
    );

/**
Summary:
Astm calls this.
**/
NEXUS_Error NEXUS_VideoDecoder_SetAstmSettings_priv(
    NEXUS_VideoDecoderHandle videoDecoder,
    const NEXUS_VideoDecoderAstmSettings * pAstmSettings
    );

/**
Summary:
Astm calls this.
**/
NEXUS_Error NEXUS_VideoDecoder_GetAstmStatus_isr(
    NEXUS_VideoDecoderHandle videoDecoder,
    NEXUS_VideoDecoderAstmStatus * pAstmStatus /* [out] */
    );

#endif /* NEXUS_HAS_ASTM */

void NEXUS_VideoDecoder_SetVideoCmprStdList_priv(
    const bool *supportedCodecs,
    BXVD_ChannelSettings *channelSettings,
    unsigned videoCmprStdListSize
    );

BXVD_DecodeResolution NEXUS_VideoDecoder_GetDecodeResolution_priv(
    unsigned maxWidth,
    unsigned maxHeight
    );

/* video as graphics interrupt connection */
void NEXUS_VideoDecoder_GetDefaultDisplayConnection_priv(NEXUS_VideoDecoderDisplayConnection *connection);
NEXUS_Error NEXUS_VideoDecoder_SetDefaultDisplayConnection_priv(const NEXUS_VideoDecoderDisplayConnection *connection);
void NEXUS_VideoDecoder_UpdateDefaultDisplayInformation_priv(const NEXUS_VideoDecoder_DisplayInformation *pDisplayInformation);

/* Hooks to allow an upper layer to install/remove XDM providers using AVD's display interrupt handler */
NEXUS_Error NEXUS_VideoDecoderModule_InstallXdmProvider_priv(
    unsigned mfdIndex,
    BXDM_PictureProvider_Handle provider,
    BXDM_DisplayInterruptHandler_PictureDataReady_isr callback_isr,
    void *pContext,
    BINT_Id topField,
    BINT_Id bottomField,
    BINT_Id frame,
    BXDM_DisplayInterruptHandler_Handle *dih
    );

void NEXUS_VideoDecoderModule_RemoveXdmProvider_priv(
    BXDM_DisplayInterruptHandler_Handle dih,
    BXDM_PictureProvider_Handle provider);

/* Video As Graphics support is not universal */
NEXUS_Error NEXUS_VideoDecoder_VideoAsGraphicsSupported_priv(
    NEXUS_VideoDecoderHandle handle
    );

unsigned NEXUS_VideoDecoder_GetMosaicIndex_isrsafe(NEXUS_VideoDecoderHandle videoDecoder);
void NEXUS_VideoDecoder_P_GetVsyncRate_isrsafe(unsigned refreshRate, unsigned *vsyncRate, bool *b1001Slip);
BXDM_PictureProvider_MonitorRefreshRate NEXUS_VideoDecoder_P_GetXdmMonitorRefreshRate_isrsafe(unsigned nexusRefreshRate);

NEXUS_Timebase NEXUS_VideoDecoder_P_GetTimebase_isrsafe(NEXUS_VideoDecoderHandle handle);
void NEXUS_VideoDecoder_P_GetDefaultSettings_isrsafe(NEXUS_VideoDecoderSettings *pSettings);
void NEXUS_VideoDecoder_P_GetDefaultPlaybackSettings_isrsafe(NEXUS_VideoDecoderPlaybackSettings *pSettings);
void NEXUS_VideoDecoder_P_GetDefaultExtendedSettings_isrsafe(NEXUS_VideoDecoderExtendedSettings *pSettings);

bool NEXUS_VideoDecoderModule_DecoderOpenInSecureHeaps_priv(void);
void NEXUS_VideoDecoder_Clear_priv(NEXUS_VideoDecoderHandle handle);

NEXUS_VideoDecoderExclusiveMode NEXUS_P_VideoDecoderExclusiveMode_isrsafe(const BBOX_Config *boxConfig, unsigned avdIndex);

NEXUS_Error NEXUS_VideoDecoderModule_GetStatus_priv(NEXUS_VideoDecoderModuleStatus *pStatus);
#ifdef __cplusplus
}
#endif

#endif
