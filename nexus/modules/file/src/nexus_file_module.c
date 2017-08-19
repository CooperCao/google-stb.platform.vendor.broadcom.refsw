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
 *
 * Module Description:
 *
 **************************************************************************/
#include "nexus_file_module.h"
#include "priv/nexus_core.h"

BDBG_MODULE("nexus_file_module");

NEXUS_ModuleHandle NEXUS_FileModule;

void
NEXUS_FileModule_GetDefaultSettings(NEXUS_FileModuleSettings *pSettings)
{
    unsigned i;
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->maxQueuedElements = 30;
    pSettings->workerThreads = 2;
    for (i=0;i<NEXUS_FILE_MAX_IOWORKERS;i++) {
        NEXUS_Thread_GetDefaultSettings(&pSettings->schedulerSettings[i]);
    }
    pSettings->common.standbyLevel = NEXUS_ModuleStandbyLevel_eActive;
    return;
}

NEXUS_ModuleHandle
NEXUS_FileModule_Init(const NEXUS_FileModuleSettings *pSettings)
{
    BERR_Code rc;
    NEXUS_FileModuleSettings settings;
    NEXUS_ModuleSettings moduleSettings;

    if(pSettings==NULL) {
        NEXUS_FileModule_GetDefaultSettings(&settings);
        pSettings = &settings;
    }
    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_ModulePriority_eDefaultActiveStandby; /* AdjustModulePriority is not available in client mode */
    NEXUS_FileModule = NEXUS_Module_Create("file", &moduleSettings);
    if(!NEXUS_FileModule) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_module; }
    NEXUS_LockModule();
    rc = NEXUS_File_P_Scheduler_Start(pSettings);
    if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_scheduler;}
    NEXUS_UnlockModule();
    return NEXUS_FileModule;

err_scheduler:
    NEXUS_UnlockModule();
    NEXUS_Module_Destroy(NEXUS_FileModule);
err_module:
    return NULL;
}

void
NEXUS_FileModule_Uninit(void)
{
    NEXUS_LockModule();
    NEXUS_File_P_Scheduler_Stop();
    NEXUS_UnlockModule();
    NEXUS_Module_Destroy(NEXUS_FileModule);
    NEXUS_FileModule = NULL;
    return;
}
