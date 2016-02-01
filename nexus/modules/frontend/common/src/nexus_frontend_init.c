/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
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
*   API name: Frontend Module
*    Frontend module private APIs and module data.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#include "nexus_frontend_module.h"
#include "nexus_frontend_init.h"

BDBG_MODULE(nexus_frontend_init);

NEXUS_ModuleHandle g_NEXUS_frontendModule;
NEXUS_FrontendModuleSettings g_NEXUS_frontendModuleSettings;

extern NEXUS_FrontendDeviceList g_frontendDeviceList;
extern NEXUS_FrontendList g_frontendList;
extern NEXUS_TunerList g_tunerList;

/***************************************************************************
Summary:
    Get Default settings for the frontend module
See Also:
    NEXUS_FrontendModule_Init
 ***************************************************************************/
void NEXUS_FrontendModule_GetDefaultSettings(
    NEXUS_FrontendModuleSettings *pSettings /* [out] */
    )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_GetDefaultCommonModuleSettings(&pSettings->common);
    pSettings->common.enabledDuringActiveStandby = true;
}

/***************************************************************************
Summary:
    Initialize the frontend module
See Also:
    NEXUS_FrontendModule_Uninit
 ***************************************************************************/
NEXUS_ModuleHandle NEXUS_FrontendModule_Init(
    const NEXUS_FrontendModuleSettings *pSettings
    )
{
    NEXUS_ModuleSettings moduleSettings;
    int rc = 0;

    BSTD_UNUSED(pSettings);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pSettings->i2cModule);
    BDBG_ASSERT(NULL != pSettings->transport);
    BDBG_ASSERT(NULL == g_NEXUS_frontendModule);

    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_AdjustModulePriority(NEXUS_ModulePriority_eIdle, &pSettings->common); /* frontend interfaces are very slow */
    moduleSettings.dbgPrint = NEXUS_FrontendModule_P_Print;
    moduleSettings.dbgModules = "nexus_frontend_proc";
    g_NEXUS_frontendModule = NEXUS_Module_Create("frontend", &moduleSettings);

    if (NULL ==g_NEXUS_frontendModule) {
        rc = BERR_TRACE(BERR_OS_ERROR);
        return NULL;
    }

    NEXUS_Frontend_P_Init();

    g_NEXUS_frontendModuleSettings = *pSettings;
    return g_NEXUS_frontendModule;
}

/***************************************************************************
Summary:
    Un-Initialize the frontend module
See Also:
    NEXUS_FrontendModule_Init
 ***************************************************************************/
void NEXUS_FrontendModule_Uninit(void)
{
    NEXUS_Module_Destroy(g_NEXUS_frontendModule);
    g_NEXUS_frontendModule = NULL;
}

NEXUS_Error NEXUS_FrontendModule_Standby_priv(bool enabled, const NEXUS_StandbySettings *pSettings)
{
#if NEXUS_POWER_MANAGEMENT
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_FrontendHandle frontend;
    NEXUS_TunerHandle tuner;
    bool handleFound = false;
    NEXUS_FrontendDeviceHandle tempHandle = NULL, deviceHandle = NULL;
    BLST_D_INIT(&g_frontendDeviceList.deviceList);

    for (frontend = BLST_SQ_FIRST(&g_frontendList.frontends); frontend; frontend = BLST_SQ_NEXT(frontend, link))
    {
        handleFound = false;
        deviceHandle = NEXUS_Frontend_GetDevice(frontend);

        if(deviceHandle != NULL){
            for (tempHandle = BLST_D_FIRST(&g_frontendDeviceList.deviceList); tempHandle; tempHandle = BLST_D_NEXT(tempHandle, node)) {
                if(tempHandle == deviceHandle){
                    handleFound = true;
                }

            }
            if(!handleFound){
                BLST_D_INSERT_HEAD(&g_frontendDeviceList.deviceList, deviceHandle, node);

            }
        }
        if((frontend->mode < NEXUS_StandbyMode_ePassive) && (pSettings->mode >= NEXUS_StandbyMode_ePassive)){
            rc = NEXUS_Frontend_Standby_priv(frontend, enabled, pSettings);
            if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
            frontend->mode = pSettings->mode;
        }
    }

    for (tuner = BLST_SQ_FIRST(&g_tunerList.tuners); tuner; tuner = BLST_SQ_NEXT(tuner, link))
    {
        handleFound = false;
        deviceHandle = tuner->pGenericDeviceHandle;
        if(deviceHandle != NULL){
            for (tempHandle = BLST_D_FIRST(&g_frontendDeviceList.deviceList); tempHandle; tempHandle = BLST_D_NEXT(tempHandle, node)) {
                if(tempHandle == deviceHandle){
                    handleFound = true;
                }
            }
            if(!handleFound){
                BLST_D_INSERT_HEAD(&g_frontendDeviceList.deviceList, deviceHandle, node);
            }
        }
        if((tuner->mode < NEXUS_StandbyMode_ePassive) && (pSettings->mode >= NEXUS_StandbyMode_ePassive)){
            rc = NEXUS_Tuner_Standby_priv(tuner, enabled, pSettings);
            if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
            tuner->mode = pSettings->mode;
        }
    }

    for (tempHandle = BLST_D_FIRST(&g_frontendDeviceList.deviceList); tempHandle; tempHandle = BLST_D_NEXT(tempHandle, node)) {
        rc = NEXUS_FrontendDevice_P_Standby(tempHandle, pSettings);
        if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
        tempHandle->mode = pSettings->mode;
    }


    for (frontend = BLST_SQ_FIRST(&g_frontendList.frontends); frontend; frontend = BLST_SQ_NEXT(frontend, link))
    {
        if((frontend->mode >= NEXUS_StandbyMode_ePassive) && (pSettings->mode < NEXUS_StandbyMode_ePassive)){
            rc = NEXUS_Frontend_Standby_priv(frontend, enabled, pSettings);
            if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
            frontend->mode = pSettings->mode;
        }
    }

    for (tuner = BLST_SQ_FIRST(&g_tunerList.tuners); tuner; tuner = BLST_SQ_NEXT(tuner, link))
    {
        if((tuner->mode >= NEXUS_StandbyMode_ePassive) && pSettings->mode < NEXUS_StandbyMode_ePassive){
            rc = NEXUS_Tuner_Standby_priv(tuner, enabled, pSettings);
            if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
            tuner->mode = pSettings->mode;
        }
    }

error:
    for (tempHandle = BLST_D_FIRST(&g_frontendDeviceList.deviceList); tempHandle; tempHandle = BLST_D_NEXT(tempHandle, node)) {
        BLST_D_REMOVE(&g_frontendDeviceList.deviceList, tempHandle, node);
    }
    return rc;

#else
    BSTD_UNUSED(enabled);
    BSTD_UNUSED(pSettings);
#endif
    return NEXUS_SUCCESS;
}



