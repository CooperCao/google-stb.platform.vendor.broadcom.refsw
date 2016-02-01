/***************************************************************************
*      (c)2007-2012 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its licensors,
* and may only be used, duplicated, modified or distributed pursuant to the terms and
* conditions of a separate, written license agreement executed between you and Broadcom
* (an "Authorized License").    Except as set forth in an Authorized License, Broadcom grants
* no license (express or implied), right to use, or waiver of any kind with respect to the
* Software, and Broadcom expressly reserves all rights in and to the Software and all
* intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1.       This program, including its structure, sequence and organization, constitutes the valuable trade
* secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
* and to use this information only in connection with your use of Broadcom integrated circuit products.
*
*   2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
* AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
* WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
* THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
* OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
* LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
* OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
* USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3.       TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
* LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
* EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
* USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
* ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
* LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
* ANY LIMITED REMEDY.
*
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Module Description:
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "nexus_cec_module.h"
#include "nexus_base.h"
#include "priv/nexus_core.h"
#include "nexus_cec_init.h"

BDBG_MODULE(nexus_cec);

/* global module handle & data */
NEXUS_ModuleHandle g_NEXUS_cecModule;
NEXUS_CecModuleSettings g_NEXUS_cecModuleSettings;

void NEXUS_CecModule_GetDefaultSettings(
    NEXUS_CecModuleSettings *pSettings   /* [out] */
    )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_ModuleHandle NEXUS_CecModule_Init(
    const NEXUS_CecModuleSettings *pSettings  /* NULL will use default settings */
    )
{
    NEXUS_ModuleSettings moduleSettings;
    NEXUS_Error errCode;

    BDBG_ASSERT(NULL == g_NEXUS_cecModule);

    if (!pSettings) {
        NEXUS_CecModule_GetDefaultSettings(&g_NEXUS_cecModuleSettings);
        pSettings = &g_NEXUS_cecModuleSettings;
    }

    /* init global module handle */
    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_ModulePriority_eLowActiveStandby; /* cec interface a slow interface */
    moduleSettings.dbgPrint = NEXUS_CecModule_Print;
    moduleSettings.dbgModules = "nexus_cec";
    g_NEXUS_cecModule = NEXUS_Module_Create("cec", &moduleSettings);
    if ( NULL == g_NEXUS_cecModule )
    {
        errCode = BERR_TRACE(BERR_OS_ERROR);
        return NULL;
    }
    NEXUS_LockModule();

    if (pSettings != &g_NEXUS_cecModuleSettings) {
        g_NEXUS_cecModuleSettings = *pSettings;
    }

    /* Success */
    NEXUS_UnlockModule();
    return g_NEXUS_cecModule;
}


void NEXUS_CecModule_Uninit(void)
{
    NEXUS_LockModule();
    NEXUS_Cec_P_Shutdown();
    NEXUS_UnlockModule();
    NEXUS_Module_Destroy(g_NEXUS_cecModule);
    BKNI_Memset(&g_NEXUS_cecModuleSettings, 0, sizeof(g_NEXUS_cecModuleSettings));
    g_NEXUS_cecModule = NULL;
}



