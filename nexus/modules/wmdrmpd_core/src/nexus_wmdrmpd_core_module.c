/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
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
*   API name: wmdrmpd_io module
*    WMDRMPD Module Initialization.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "nexus_wmdrmpd_core_module.h"

NEXUS_ModuleHandle g_NEXUS_wmDrmPdCoreModule;
NEXUS_WmDrmPdCoreModuleData g_NEXUS_wmDrmPdCoreModuleData;

void NEXUS_WmDrmPdCoreModule_GetDefaultSettings(
    NEXUS_WmDrmPdCoreModuleSettings *pSettings    /* [out] */
    )
{
    BKNI_Memset(pSettings, 0, sizeof(NEXUS_WmDrmPdCoreModuleSettings));
    pSettings->ioRequestTimeout = 2000;
}

NEXUS_ModuleHandle NEXUS_WmDrmPdCoreModule_Init(
    const NEXUS_WmDrmPdCoreModuleSettings *pSettings
    )
{
    NEXUS_Error errCode;
    NEXUS_ModuleHandle hModule;

    hModule = NEXUS_Module_Create("wmdrmpd_core", NULL);
    if ( NULL == hModule )
    {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_module;
    }
    
    if ( pSettings )
    {
        g_NEXUS_wmDrmPdCoreModuleData.settings = *pSettings;
    }
    else
    {
        NEXUS_WmDrmPdCoreModule_GetDefaultSettings(&g_NEXUS_wmDrmPdCoreModuleData.settings);
    }

    if ( pSettings->modules.wmdrmpdIo == NULL ) 
    {
        goto err_module;
    }
        
    errCode = BKNI_CreateEvent(&g_NEXUS_wmDrmPdCoreModuleData.hIoEvent);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_event;
    }

    g_NEXUS_wmDrmPdCoreModule = hModule;
    return hModule;

err_event:
    NEXUS_Module_Destroy(g_NEXUS_wmDrmPdCoreModule);
    g_NEXUS_wmDrmPdCoreModule = NULL;
err_module:
    return NULL;            
}

void NEXUS_WmDrmPdCoreModule_Uninit(
    void
    )
{
    BKNI_DestroyEvent(g_NEXUS_wmDrmPdCoreModuleData.hIoEvent);
    NEXUS_Module_Destroy(g_NEXUS_wmDrmPdCoreModule);
    g_NEXUS_wmDrmPdCoreModule = NULL;
}

