/***************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#ifndef NEXUS_VIDEO_DECODER_INIT_H__
#define NEXUS_VIDEO_DECODER_INIT_H__

/*=========================================
The VideoDecoder module provides Interfaces for decode of compressed digital video.

It provides a single interface of the same name. See NEXUS_VideoDecoder_Open and related functions.
===========================================*/

#include "nexus_types.h"
#include "nexus_video_decoder_types.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Settings used to configure the VideoDecoder module.

Description:

See Also:
NEXUS_VideoDecoderModule_GetDefaultInternalSettings
NEXUS_VideoDecoderModule_Init
**/
typedef struct NEXUS_VideoDecoderModuleInternalSettings
{
    NEXUS_ModuleHandle transport; /* Handle to Transport module. See NEXUS_Transport_Init. */

    NEXUS_HeapHandle secureHeap;    /* optional. if set, video decoder will allocate the cabac bin buffer from this heap. */
    NEXUS_ModuleHandle audio; /* Handle to Audio module. See NEXUS_AudioModule_Init. needed for ZSP and DSP video decode. */
    NEXUS_ModuleHandle pictureDecoder; /* Handle to PictureDecoder module. See NEXUS_PictureDecoderModule_Init. */
    NEXUS_ModuleHandle core; /* Handle to Core module. See NEXUS_Core_Init. */
    NEXUS_ModuleHandle security; /* Handle to Security module. */
    bool deferInit; /* if set to true, HW and FW initialization will be deferred until actually used */
    struct {
        /* location of all HVD FW */
        NEXUS_MemoryBlockHandle block;
        unsigned size;
    } firmware;
} NEXUS_VideoDecoderModuleInternalSettings;

/**
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new strucutre members in the future.

See Also:
NEXUS_VideoDecoderModule_Init
**/
void NEXUS_VideoDecoderModule_GetDefaultInternalSettings(
    NEXUS_VideoDecoderModuleInternalSettings *pSettings, /* [out] */
    const NEXUS_VideoDecoderModuleSettings *pModuleSettings
    );

struct NEXUS_Core_PreInitState;

unsigned NEXUS_VideoDecoderModule_GetFirmwareSize(
    const struct NEXUS_Core_PreInitState *preInitState,
    const NEXUS_VideoDecoderModuleSettings *pModuleSettings
    );

void NEXUS_VideoDecoderModule_GetDefaultSettings(
    NEXUS_VideoDecoderModuleSettings *pSettings /* [out] */
    );

/**
Summary:
Initialize the VideoDecoder module.

Description:
This function is called by NEXUS_Platform_Init, not by applications.
If you want to modify these settings from your application, you can do this 
through NEXUS_PlatformSettings as follows:

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.videoDecoderModuleSettings.xxx = xxx;
    NEXUS_Platform_Init(&platformSettings);

**/
NEXUS_ModuleHandle NEXUS_VideoDecoderModule_Init(
    const NEXUS_VideoDecoderModuleInternalSettings *pModuleSettings,
    const NEXUS_VideoDecoderModuleSettings *pSettings
    );

/**
Summary:
Uninitialize the VideoDecoder module.

Description:
Called by NEXUS_Platform_Uninit
**/
void NEXUS_VideoDecoderModule_Uninit(void);

#ifdef __cplusplus
}
#endif

#endif
