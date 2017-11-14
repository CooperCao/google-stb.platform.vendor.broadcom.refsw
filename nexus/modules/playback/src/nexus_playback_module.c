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
 * Module Description:
 *
 **************************************************************************/
#include "nexus_playback_module.h"

NEXUS_ModuleHandle NEXUS_PlaybackModule;
NEXUS_PlaybackModuleSettings g_NEXUS_PlaybackModulesSettings;

void 
NEXUS_PlaybackModule_GetDefaultSettings(NEXUS_PlaybackModuleSettings *pSettings)
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    return;
}

NEXUS_ModuleHandle 
NEXUS_PlaybackModule_Init(const NEXUS_PlaybackModuleSettings *pSettings)
{
    NEXUS_ModuleSettings moduleSettings;

    if(pSettings==NULL) {
        BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err_settings;
    }
    if(pSettings->modules.file == NULL || pSettings->modules.videoDecoder == NULL || pSettings->modules.playpump == NULL) {
        BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err_settings;
    }
#if NEXUS_HAS_AUDIO
    if(pSettings->modules.audioDecoder == NULL) {
        BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err_settings;
    }
#endif
    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_ModulePriority_eLow;
    NEXUS_PlaybackModule = NEXUS_Module_Create("playback", &moduleSettings);
    if(!NEXUS_PlaybackModule) { BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_module; }

    NEXUS_UseModule(pSettings->modules.file);
    NEXUS_UseModule(pSettings->modules.videoDecoder);
#if NEXUS_HAS_AUDIO
    NEXUS_UseModule(pSettings->modules.audioDecoder);
#endif
    NEXUS_UseModule(pSettings->modules.playpump);
    g_NEXUS_PlaybackModulesSettings = *pSettings;

    return NEXUS_PlaybackModule;

err_module:
err_settings:
    return NULL;
}
    
void 
NEXUS_PlaybackModule_Uninit(void)
{
    NEXUS_Module_Destroy(NEXUS_PlaybackModule);
    NEXUS_PlaybackModule = NULL;
    return;
}
