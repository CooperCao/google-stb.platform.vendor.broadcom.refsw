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
#ifndef NEXUS_DISPLAY_INIT_H__
#define NEXUS_DISPLAY_INIT_H__

#include "nexus_memory.h"
#include "nexus_display_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
The Display module's down modules
**/
typedef struct NEXUS_DisplayModuleDependencies
{
    NEXUS_ModuleHandle videoDecoder;
    NEXUS_ModuleHandle surface;
    NEXUS_ModuleHandle hdmiInput;
    NEXUS_ModuleHandle hdmiDvo;
    NEXUS_ModuleHandle hdmiOutput;
    NEXUS_ModuleHandle rfm;
    NEXUS_ModuleHandle pwm; /* needed for panel backlight control */
    NEXUS_ModuleHandle transport;
} NEXUS_DisplayModuleDependencies;


/**
Summary:
Settings used in NEXUS_DisplayModule_Init
**/
typedef struct NEXUS_DisplayModuleInternalSettings
{
    NEXUS_DisplayModuleDependencies modules;
} NEXUS_DisplayModuleInternalSettings;

struct NEXUS_Core_PreInitState;

/**
Summary:
Get defaults before calling NEXUS_DisplayModule_Init
**/
void NEXUS_DisplayModule_GetDefaultInternalSettings(
    NEXUS_DisplayModuleInternalSettings *pSettings /* [out] */
    );

/**
Summary:
Get defaults before calling NEXUS_DisplayModule_Init
**/
void NEXUS_DisplayModule_GetDefaultSettings(
    const struct NEXUS_Core_PreInitState *preInitState,
    NEXUS_DisplayModuleSettings *pSettings /* [out] */
    );

/**
Summary:
Initialize the Display module

Description:
This function is called by NEXUS_Platform_Init, not by applications.
If you want to modify these settings from your application, you can do this
through NEXUS_PlatformSettings as follows:

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.displayModuleSettings.xxx = xxx;
    NEXUS_Platform_Init(&platformSettings);

**/
NEXUS_ModuleHandle NEXUS_DisplayModule_Init(
    const NEXUS_DisplayModuleInternalSettings *pModuleSettings,
    const NEXUS_DisplayModuleSettings *pSettings
    );

/**
Summary:
Uninitialize the Display module
**/
void NEXUS_DisplayModule_Uninit(void);

/**
Summary:
Get the settings that were used in NEXUS_DisplayModule_Init.

Description:
These cannot be changed without calling NEXUS_DisplayModule_Uninit then NEXUS_DisplayModule_Init.
This is for informational purposes.
**/
void NEXUS_DisplayModule_GetSettings(
    NEXUS_DisplayModuleSettings *pSettings /* [out] */
    );

#define NEXUS_DISPLAY_WINDOW_MAIN (0x1)
#define NEXUS_DISPLAY_WINDOW_PIP  (0x2)
#define NEXUS_DISPLAY_WINDOW_MONITOR (0x4)

#define NEXUS_DISPLAY_INPUT_DIGITAL (0x1000)
#define NEXUS_DISPLAY_INPUT_ANALOG  (0x2000)

/**
Summary:
Get the settings that were used in NEXUS_DisplayModule_Init.

Description:
These cannot be changed without calling NEXUS_DisplayModule_Uninit then NEXUS_DisplayModule_Init.
This is for informational purposes.
**/
NEXUS_Error NEXUS_DisplayModule_GetMemorySettings(
    unsigned configurationId,                           /* Configuration ID */
    uint32_t mask,                                      /* Must contain at least one window and at least one input */
    NEXUS_DisplayBufferTypeSettings *pFullHdBuffers,    /* [out] Full HD buffer requirements */
    NEXUS_DisplayBufferTypeSettings *pHdBuffers,        /* [out] HD buffer requirements */
    NEXUS_DisplayBufferTypeSettings *pSdBuffers,        /* [out] SD buffer requirements */
    unsigned *pHeapSize                                 /* [out] Heap size in bytes */
    );

/**
Summary:
Set the VideoDecoder module dependency

Description:
This allows for faster system boot time. The Display module and VideoDecoder module can init separately, then the link can be made after both are initialized.
**/
void NEXUS_DisplayModule_SetVideoDecoderModule(
    NEXUS_ModuleHandle videoDecoder /* Set to NULL or to the VideoDecoder module */
    );

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_DISPLAY_INIT_H__ */
