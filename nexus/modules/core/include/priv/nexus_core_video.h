/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
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
* API Description:
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_CORE_VIDEO_H_
#define NEXUS_CORE_VIDEO_H_

#include "nexus_types.h"
#include "bavc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NEXUS_VideoInputType {
    NEXUS_VideoInputType_eDecoder,
    NEXUS_VideoInputType_eCcir656,
    NEXUS_VideoInputType_eHdmi,
    NEXUS_VideoInputType_eImage,
    NEXUS_VideoInputType_eHdDvi,
    NEXUS_VideoInputType_eMax
} NEXUS_VideoInputType;


struct NEXUS_VideoInput {
    NEXUS_OBJECT(NEXUS_VideoInput);
    NEXUS_VideoInputType type; /* type of video input */
    void *source; /* polymorphic pointer to the source, must be not NULL */
    void *destination; /* polymorphic pointer to the destination (e.g. DisplayModule), must be NULL if disconnected, must be not NULL if connected */
    unsigned ref_cnt; /* reference counter is incremented if VideoInput is cached/stored in the destination */
#if NEXUS_HAS_SYNC_CHANNEL
    struct
    {
        NEXUS_Callback connectionChangedCallback_isr; /* Notify SyncChannel when an input connection changes. */
        void * callbackContext;
    } sync;
#endif
};

typedef struct NEXUS_VideoInput NEXUS_VideoInputObject;

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_VideoInput);

#define NEXUS_VIDEO_INPUT_INIT(input, inputType, src) do { NEXUS_OBJECT_SET(NEXUS_VideoInput, (input) );(input)->type = inputType;(input)->source=src;(input)->destination=NULL;(input)->ref_cnt=0;}while(0)

typedef enum NEXUS_VideoOutputType {
    NEXUS_VideoOutputType_eComposite,
    NEXUS_VideoOutputType_eSvideo,
    NEXUS_VideoOutputType_eComponent,
    NEXUS_VideoOutputType_eRfm,
    NEXUS_VideoOutputType_eHdmi,
    NEXUS_VideoOutputType_eHdmiDvo,
    NEXUS_VideoOutputType_eCcir656,
    NEXUS_VideoOutputType_eMax
} NEXUS_VideoOutputType;

struct NEXUS_VideoOutput {
    NEXUS_OBJECT(NEXUS_VideoOutput);
    NEXUS_VideoOutputType type; /* type of video output */
    void *source; /* polymorphic pointer to the source, must be not NULL */
    void *destination; /* polymorphic pointer to the destination (e.g. DisplayHandle), must be NULL if disconnected, must be not NULL if connected */
    unsigned ref_cnt; /* reference counter is incremented if VideoInput is cached/stored in the destination */
};

typedef struct NEXUS_VideoOutput NEXUS_VideoOutputObject;

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_VideoOutput);

#define NEXUS_VIDEO_OUTPUT_INIT(output, outputType, src) do { NEXUS_OBJECT_SET(NEXUS_VideoOutput, (output) );(output)->type = outputType;(output)->source=src;(output)->destination=NULL;(output)->ref_cnt=0;}while(0)

/**
Summary:
Used by SyncChannel
**/
typedef struct NEXUS_VideoInputSyncSettings
{
    unsigned startDelay;/* This is the start delay applied by SyncChannel. It is always in 45 KHz units. */
    unsigned delay;     /* This is the delay applied by SyncChannel. It is always in 45 KHz units. */
    bool mute;          /* Mute video until SyncChannel is ready. */
    unsigned delayCallbackThreshold; /* threshold (in 45 KHz units) to limit frequency of callback */

    NEXUS_Callback delayCallback_isr; /* Notify SyncChannel of phase difference between PTS and STC.
                                         Set to NULL to disable.
                                         int param contains the delay. */
    NEXUS_Callback startCallback_isr; /* Notify SyncChannel when on start/stop.
                                         Call NEXUS_VideoInput_GetSyncStatus_isr to get required info. */
    NEXUS_Callback formatCallback_isr;/* Notify SyncChannel when format changes.
                                         Call NEXUS_VideoInput_GetSyncStatus_isr to get required info. */
    NEXUS_Callback connectionChangedCallback_isr; /* Notify SyncChannel when an input connection changes. */
    void *callbackContext; /* user context passed callback_isr */
} NEXUS_VideoInputSyncSettings;

/**
Summary:
Used by SyncChannel
**/
typedef struct NEXUS_VideoInputSyncStatus
{
    bool started;
    bool digital;
    unsigned int height;
    bool interlaced;
    BAVC_FrameRateCode frameRate;
    bool lastPictureHeld;
    bool delayValid;
    int delay;
    bool nonRealTime;
    uint32_t customPtsOffset; /* milliseconds, replaces primerPtsOffsetMs, codec, and sarnoff_video_delay_workaround */
} NEXUS_VideoInputSyncStatus;

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_CORE_VIDEO_H_ */

