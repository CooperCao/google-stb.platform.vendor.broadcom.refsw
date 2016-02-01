/***************************************************************************
 *     (c)2007-2013 Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 **************************************************************************/
#include "nexus_i2c_module.h"
#include "nexus_i2c_init.h"

BDBG_MODULE(nexus_i2c_module);

/* global module handle & data */
BI2C_Handle g_NEXUS_i2cHandle;
NEXUS_ModuleHandle g_NEXUS_i2cModule;

NEXUS_ModuleHandle NEXUS_I2cModule_Init(const NEXUS_I2cModuleSettings *pSettings)
{
    BERR_Code errCode;
    NEXUS_ModuleSettings moduleSettings;
    BI2C_Settings i2cSettings;

    BDBG_ASSERT(NULL == g_NEXUS_i2cModule);
    BSTD_UNUSED(pSettings);

    /* init global module handle */
    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_AdjustModulePriority(NEXUS_ModulePriority_eIdle, &pSettings->common); /* i2c interface is extremely slow */
    g_NEXUS_i2cModule = NEXUS_Module_Create("i2c", &moduleSettings);
    if ( NULL == g_NEXUS_i2cModule )
    {
        errCode=BERR_TRACE(BERR_NOT_SUPPORTED);
        return NULL;
    }

    BI2C_GetDefaultSettings(&i2cSettings, g_pCoreHandles->chp);
    BDBG_MSG(("Opening I2C Device"));
    errCode = BI2C_Open(&g_NEXUS_i2cHandle, g_pCoreHandles->chp, g_pCoreHandles->reg, g_pCoreHandles->bint, &i2cSettings);
    if ( errCode )
    {
        BDBG_ERR(("Unable to open I2C Device"));
        errCode = BERR_TRACE(errCode);
        NEXUS_Module_Destroy(g_NEXUS_i2cModule);
        return NULL;
    }

    /* Success */
    return g_NEXUS_i2cModule;
}

void NEXUS_I2cModule_Uninit(void)
{
    NEXUS_I2c_P_CloseAll();
    BI2C_Close(g_NEXUS_i2cHandle);
    NEXUS_Module_Destroy(g_NEXUS_i2cModule);
    g_NEXUS_i2cModule = NULL;
}

void NEXUS_I2cModule_GetDefaultSettings(
    NEXUS_I2cModuleSettings *pSettings    /* [out] */
    )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_GetDefaultCommonModuleSettings(&pSettings->common);
    pSettings->common.enabledDuringActiveStandby = true;
}

