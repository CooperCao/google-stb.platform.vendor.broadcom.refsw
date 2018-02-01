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
 **************************************************************************/
#ifndef NEXUS_VIDEO_ENCODER_MODULE_H__
#define NEXUS_VIDEO_ENCODER_MODULE_H__

#include "nexus_base.h"
#include "nexus_video_encoder_thunks.h"
#include "nexus_video_encoder_types.h"
#include "nexus_video_encoder.h"
#include "nexus_video_encoder_output.h"
#include "nexus_video_encoder_init.h"
#include "nexus_platform_features.h"

#include "priv/nexus_core_img.h"
#include "priv/nexus_rave_priv.h"
#include "priv/nexus_video_encoder_priv.h"
#include "priv/nexus_video_encoder_standby_priv.h"
#if NEXUS_DISPLAY_VIP_SUPPORT
#include "priv/nexus_stc_channel_priv.h"
#endif

#include "bvce.h"
#include "bdbg_fifo.h"

#ifdef __cplusplus
extern "C" {
#endif

struct NEXUS_VideoEncoder_P_Device;

struct NEXUS_VideoEncoder {
    NEXUS_OBJECT(NEXUS_VideoEncoder);
    unsigned index;
    BVCE_Channel_Handle enc;
    NEXUS_VideoEncoderSettings settings;
    NEXUS_VideoEncoderOpenSettings openSettings;
    NEXUS_VideoEncoderStartSettings startSettings;
    NEXUS_TaskCallbackHandle watchdogCallbackHandler;
#if NEXUS_DISPLAY_VIP_SUPPORT
    NEXUS_StcChannelSnapshotHandle snapshot;
#endif
    struct NEXUS_VideoEncoder_P_Device *device;
    struct { /* mapping between MMA and nexus memory blocks, there is an assumption that BVCE encoder will return the _same_ memory blocks for as long as it opened */
        BMMA_Block_Handle mma;
        NEXUS_MemoryBlockHandle nexus;
    } frame,meta, debugLog;
    NEXUS_IsrCallbackHandle dataReadyCallback;
    bool started;
};

typedef struct NEXUS_VideoEncoder_P_Device {
    BVCE_Handle vce;
    BMMA_Heap_Handle mma;
    struct NEXUS_VideoEncoder channels[NEXUS_NUM_VCE_CHANNELS];
    /* image interface */
    void * img_context;
    BIMG_Interface img_interface;
    BKNI_EventHandle watchdogEvent;
    NEXUS_EventCallbackHandle watchdogEventHandler;
    struct {
        BVCE_Debug_FifoInfo fifoInfo;
        void *fifo;
        void *entry;
        BDBG_FifoReader_Handle reader;
        NEXUS_TimerHandle timer;
    } debugLog;
    bool watchdogOccured;
} NEXUS_VideoEncoder_P_Device;

typedef struct NEXUS_VideoEncoder_P_State {
    NEXUS_ModuleHandle module;
    NEXUS_VideoEncoder_P_Device vceDevices[NEXUS_NUM_VCE_DEVICES];
    NEXUS_VideoEncoderModuleInternalSettings config;
    NEXUS_VideoEncoderModuleSettings settings;
    NEXUS_TimerHandle logTimer;
} NEXUS_VideoEncoder_P_State;

extern NEXUS_VideoEncoder_P_State g_NEXUS_VideoEncoder_P_State;
#define NEXUS_MODULE_NAME video_encoder
#define NEXUS_MODULE_SELF g_NEXUS_VideoEncoder_P_State.module
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_VideoEncoder);


#ifdef __cplusplus
}
#endif


#endif /* NEXUS_VIDEO_ENCODER_MODULE_H__ */
