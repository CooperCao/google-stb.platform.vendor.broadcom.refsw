/***************************************************************************
 *  Copyright (C) 2010-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#ifndef NEXUS_VIDEO_ENCODER_INIT_H__
#define NEXUS_VIDEO_ENCODER_INIT_H__

#include "nexus_video_encoder_types.h"
#include "nexus_display_types.h" /* defines NEXUS_MAX_VIDEO_ENCODERS */

#ifdef __cplusplus
extern "C" {
#endif

#define NEXUS_MAX_VCE_DEVICES   2

/**
Summary:
Settings used to configure the VideoEncoder module.

Description:

See Also:
NEXUS_VideoEncoderModule_GetDefaultSettings
NEXUS_VideoEncoderModule_Init
**/

typedef struct NEXUS_VideoEncoderModuleInternalSettings
{
    NEXUS_VideoEncoder_MemoryConfig heapSize[NEXUS_MAX_VCE_DEVICES];
    struct {
        unsigned general;  /* deprecated */
        unsigned firmware[2];
        unsigned secure;
        unsigned picture;
        unsigned system; /* should be accessible by CPU */
        unsigned output; /* heap for compressed output */
    } heapIndex[NEXUS_MAX_VCE_DEVICES];
    NEXUS_ModuleHandle display;
    NEXUS_ModuleHandle transport;
    NEXUS_ModuleHandle audio;
    NEXUS_ModuleHandle core; /* Handle to Core module. See NEXUS_Core_Init. */
    NEXUS_ModuleHandle security; /* Handle to Security module. */
    struct {
        int device; /* VCE core to use. -1 if unused. */
        int channel; /* channel of VCE core to use. -1 if unused.  */
    } vceMapping[NEXUS_MAX_VIDEO_ENCODERS]; /* index of NEXUS_VideoEncoder */
    struct {
        NEXUS_VideoEncoderMemory memory;
    } videoEncoder[NEXUS_MAX_VIDEO_ENCODERS]; /* index of NEXUS_VideoEncoder */
} NEXUS_VideoEncoderModuleInternalSettings;

/**
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new strucutre members in the future.

See Also:
NEXUS_VideoEncoderModule_Init
**/
void NEXUS_VideoEncoderModule_GetDefaultSettings(
    NEXUS_VideoEncoderModuleSettings *pSettings /* [out] */
    );

/**
Summary:
Initialize the VideoEncoder module.

Description:
This function is called by NEXUS_Platform_Init, not by applications.
If you want to modify these settings from your application, you can do this 
through NEXUS_PlatformSettings as follows:

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.videoEncoderSettings.xxx = xxx;
    NEXUS_Platform_Init(&platformSettings);

**/
NEXUS_ModuleHandle NEXUS_VideoEncoderModule_Init(
    const NEXUS_VideoEncoderModuleInternalSettings *pInternalSettings,
    const NEXUS_VideoEncoderModuleSettings *pSettings
    );

/**
Summary:
Uninitialize the VideoEncoder module.

Description:
Called by NEXUS_Platform_Uninit
**/
void NEXUS_VideoEncoderModule_Uninit(void);

void NEXUS_VideoEncoderModule_GetDefaultInternalSettings(
    NEXUS_VideoEncoderModuleInternalSettings *pSettings
    );




#ifdef __cplusplus
}
#endif


#endif /* NEXUS_VIDEO_ENCODER_INIT_H__ */

