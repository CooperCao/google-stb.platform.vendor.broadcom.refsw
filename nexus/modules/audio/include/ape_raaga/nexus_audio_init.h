/***************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 *      Audio Initialization Routines
 *
 **************************************************************************/
#ifndef NEXUS_AUDIO_INIT_H__
#define NEXUS_AUDIO_INIT_H__

#include "nexus_types.h"
#include "nexus_audio.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************** 
Summary: 
    Nexus audio module version
***************************************************************************/
#define NEXUS_AUDIO_MODULE_FAMILY (NEXUS_AUDIO_MODULE_FAMILY_APE_RAAGA)

typedef struct NEXUS_AudioModuleInternalSettings
{
    struct 
    {
        NEXUS_ModuleHandle transport;
        NEXUS_ModuleHandle hdmiInput;   /* Only required for platforms that support HDMI input */
        NEXUS_ModuleHandle hdmiOutput;  /* Only required for platforms that support HDMI output */
        NEXUS_ModuleHandle rfm;         /* Only required for platforms that support RFM */
        NEXUS_ModuleHandle frontend;    /* Only required for platforms that support RfAudioDecoder */
        NEXUS_ModuleHandle surface; 
        NEXUS_ModuleHandle core;        /* Handle to Core module. See NEXUS_Core_Init. */
        NEXUS_ModuleHandle security;
        NEXUS_ModuleHandle sage;
    } modules;
} NEXUS_AudioModuleInternalSettings;

struct NEXUS_Core_PreInitState;

void NEXUS_AudioModule_GetDefaultSettings(
    const struct NEXUS_Core_PreInitState *preInitState,
    NEXUS_AudioModuleSettings *pSettings    /* [out] */
    );

void NEXUS_AudioModule_GetDefaultInternalSettings(
    NEXUS_AudioModuleInternalSettings *pSettings    /* [out] */
    );

/**
Summary:
Initialize the audio module

Description:
This function is called by NEXUS_Platform_Init, not by applications.
If you want to modify these settings from your application, you can do this 
through NEXUS_PlatformSettings as follows:

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.audioModuleSettings.xxx = xxx;
    NEXUS_Platform_Init(&platformSettings);

**/
NEXUS_ModuleHandle NEXUS_AudioModule_Init(
    const NEXUS_AudioModuleSettings *pSettings,  /* NULL will use default settings */
    const NEXUS_AudioModuleInternalSettings *pInternalSettings
    );

/**
Summary:
Un-Initialize the audio module
**/
void NEXUS_AudioModule_Uninit(void);

/***************************************************************************
Summary:
Audio Module Memory Estimate
**************************************************************************/
typedef struct NEXUS_AudioModuleMemoryEstimate
{
    struct {
        unsigned general; /* bytes allocated */
    } memc[NEXUS_MAX_MEMC];
} NEXUS_AudioModuleMemoryEstimate;

/**
Summary:
Get Default Usage Settings
**/
void NEXUS_AudioModule_GetDefaultUsageSettings(
    NEXUS_AudioModuleUsageSettings *pSettings   /* [out] */
    );

/**
Summary:
Get Memory Estimate

Description:
Get an estimated amount of memory required for specified usage
cases.
**/
NEXUS_Error NEXUS_AudioModule_GetMemoryEstimate(
    const struct NEXUS_Core_PreInitState *preInitState,
    const NEXUS_AudioModuleUsageSettings *pSettings,
    NEXUS_AudioModuleMemoryEstimate *pEstimate  /* [out] */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_AUDIO_INIT_H__ */
