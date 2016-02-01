/***************************************************************************
*     (c)2004-2014 Broadcom Corporation
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
*   API name: Frontend 7364
*    APIs to open, close, and setup initial settings for a BCM7364
*    Dual-Channel Satellite Tuner/Demodulator Device.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
/* General includes */
#include "nexus_frontend_7364_priv.h"

BDBG_MODULE(nexus_frontend_7364);

BDBG_OBJECT_ID(NEXUS_7364Device);

static BLST_S_HEAD(devList, NEXUS_7364Device) g_deviceList = BLST_S_INITIALIZER(g_deviceList);

/* Generic function declarations */
static void NEXUS_FrontendDevice_P_7364_GetCapabilities(void *handle, NEXUS_FrontendDeviceCapabilities *pCapabilities);
static void NEXUS_Frontend_P_7364_DestroyDevice(void *handle);
static void NEXUS_FrontendDevice_P_7364_GetCapabilities(void * handle, NEXUS_FrontendDeviceCapabilities *pCapabilities);
NEXUS_Error NEXUS_FrontendDevice_P_7364_GetInternalGain(void *handle, const NEXUS_GainParameters *params, NEXUS_InternalGainSettings *pSettings);
NEXUS_Error NEXUS_FrontendDevice_P_7364_GetExternalGain(void *handle, NEXUS_ExternalGainSettings *pSettings);
NEXUS_Error NEXUS_FrontendDevice_P_7364_SetExternalGain(void *handle, const NEXUS_ExternalGainSettings *pSettings);

static NEXUS_Error NEXUS_FrontendDevice_P_7364_Standby(void * handle, const NEXUS_StandbySettings *pSettings);
/* End of generic function declarations */

NEXUS_Error NEXUS_FrontendDevice_P_7364_GetInternalGain(void *handle, const NEXUS_GainParameters *params, NEXUS_InternalGainSettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7364Device *pDevice;
    BHAB_InternalGainInputParams inputParams;
    BHAB_InternalGainSettings internalGain;
    NEXUS_GainParameters gainParams;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7364Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7364Device);

    pSettings->frequency = params->frequency;
    inputParams.frequency = params->frequency;

    rc = BHAB_GetInternalGain(pDevice->hab, &inputParams, &internalGain);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    if(params->accumulateTillRootDevice){
        if(pDevice->pGenericDeviceHandle->parent){
            gainParams.rfInput = pDevice->pGenericDeviceHandle->linkSettings.rfInput;
            gainParams.accumulateTillRootDevice = params->accumulateTillRootDevice;
            gainParams.frequency = params->frequency;
            rc = NEXUS_FrontendDevice_P_GetInternalGain(pDevice->pGenericDeviceHandle->parent, &gainParams, pSettings);
            if(rc){rc = BERR_TRACE(rc); goto done;}
        }
    }

    if(params->rfInput == NEXUS_FrontendDeviceRfInput_eDaisy){
        pSettings->totalVariableGain += internalGain.internalGainDaisy;
    }
    else if(params->rfInput == NEXUS_FrontendDeviceRfInput_eLoopThrough){
        pSettings->totalVariableGain += internalGain.internalGainLoopThrough;
    }

    pSettings->daisyGain += internalGain.internalGainDaisy;
    pSettings->loopThroughGain += internalGain.internalGainLoopThrough;
done:
    return rc;
}
NEXUS_Error NEXUS_FrontendDevice_P_7364_GetExternalGain(void *handle, NEXUS_ExternalGainSettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7364Device *pDevice;
    BHAB_ExternalGainSettings gain;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7364Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7364Device);
    BSTD_UNUSED(pSettings);
#if 1

    rc = BHAB_GetExternalGain(pDevice->hab, &gain);
    if(rc){rc = BERR_TRACE(rc); goto done;}
#endif
    pSettings->bypassableGain = gain.externalGainBypassable;
    pSettings->totalGain = gain.externalGainTotal;

done:
    return rc;

}
NEXUS_Error NEXUS_FrontendDevice_P_7364_SetExternalGain(void *handle, const NEXUS_ExternalGainSettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7364Device *pDevice;
    BHAB_ExternalGainSettings externalGain;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7364Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7364Device);

    externalGain.externalGainTotal = pSettings->totalGain;
    externalGain.externalGainBypassable = pSettings->bypassableGain;

#if 1
    rc = BHAB_SetExternalGain(pDevice->hab, &externalGain);
    if(rc){rc = BERR_TRACE(rc); goto done;}
#endif
done:
    return rc;
}
/* End of generic function declarations */

/* Public API entry point */
NEXUS_Error NEXUS_FrontendDevice_P_Get7364Settings(void *handle, NEXUS_FrontendDeviceSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7364Device *pDevice = NULL;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7364Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7364Device);

    pSettings->rfDaisyChain = pDevice->rfDaisyChain;
    pSettings->enableRfLoopThrough = pDevice->enableRfLoopThrough;
    pSettings->rfInput = pDevice->rfInput;

    return rc;
}

NEXUS_Error NEXUS_FrontendDevice_P_Set7364Settings(void *handle, const NEXUS_FrontendDeviceSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHAB_ConfigSettings habConfigSettings;
    NEXUS_7364Device *pDevice = NULL;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7364Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7364Device);

    rc = BHAB_GetConfigSettings(pDevice->hab, &habConfigSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    habConfigSettings.daisyChain = pSettings->rfDaisyChain;
    habConfigSettings.enableLoopThrough = pSettings->enableRfLoopThrough;

    habConfigSettings.rfInputMode = pSettings->rfInput;
    habConfigSettings.tnrApplication = BHAB_TunerApplication_eTerrestrial;

    rc = BHAB_SetConfigSettings(pDevice->hab, &habConfigSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pDevice->rfDaisyChain = pSettings->rfDaisyChain;
    pDevice->enableRfLoopThrough = pSettings->enableRfLoopThrough;
    pDevice->rfInput = pSettings->rfInput;

done:
    return rc;
}

static NEXUS_Error NEXUS_FrontendDevice_P_OpenMxt7364(NEXUS_7364Device *pDevice)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BSTD_UNUSED(pDevice);
#if NEXUS_HAS_MXT
    {
        /* open MXT */
        unsigned i;
        BERR_Code rc;
        BMXT_Settings mxtSettings;

        BMXT_7364_GetDefaultSettings(&mxtSettings);
#if (BCHP_CHIP==7364) && (BCHP_VER >= BCHP_VER_B0)
        mxtSettings.chipRev = BMXT_ChipRev_eB0;
#endif

        for (i=0; i<BMXT_NUM_MTSIF; i++) {
            mxtSettings.MtsifTxCfg[i].Enable = true;
            NEXUS_Module_Lock(g_NEXUS_frontendModuleSettings.transport);
            mxtSettings.MtsifTxCfg[i].Encrypt = NEXUS_TransportModule_P_IsMtsifEncrypted();
            NEXUS_Module_Unlock(g_NEXUS_frontendModuleSettings.transport);
        }

        mxtSettings.MtsifTxCfg[0].TxClockPolarity = 0;

        rc = BMXT_Open(&pDevice->pGenericDeviceHandle->mtsifConfig.mxt, g_pCoreHandles->chp, g_pCoreHandles->reg, &mxtSettings);
        if (rc!=BERR_SUCCESS) goto err;

        rc = NEXUS_Frontend_P_InitMtsifConfig(&pDevice->pGenericDeviceHandle->mtsifConfig, &mxtSettings);
        if (rc!=BERR_SUCCESS) goto err;
    }
err:
#endif
    return rc;
}

static void NEXUS_FrontendDevice_P_CloseMxt7364(NEXUS_7364Device *pDevice)
{
    BSTD_UNUSED(pDevice);
#if NEXUS_HAS_MXT
    if (pDevice->pGenericDeviceHandle->mtsifConfig.mxt) {
        BMXT_Close(pDevice->pGenericDeviceHandle->mtsifConfig.mxt);
        pDevice->pGenericDeviceHandle->mtsifConfig.mxt = NULL;
    }
#endif
}

NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_P_Open7364(unsigned index, const NEXUS_FrontendDeviceOpenSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_FrontendDevice *pFrontendDevice = NULL;
    NEXUS_7364Device *pDevice=NULL;

    BSTD_UNUSED(index);
    BDBG_ASSERT(pSettings);

    for ( pDevice = BLST_S_FIRST(&g_deviceList); NULL != pDevice; pDevice = BLST_S_NEXT(pDevice, node) )
    {
        break;
    }

    if ( NULL == pDevice)
    {
        pFrontendDevice = BKNI_Malloc(sizeof(*pFrontendDevice));
        if (NULL == pFrontendDevice) { BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err; }
        BKNI_Memset(pFrontendDevice, 0, sizeof(*pFrontendDevice));

        pDevice = BKNI_Malloc(sizeof(*pDevice));
        if (NULL == pDevice) { BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err; }
        BKNI_Memset(pDevice, 0, sizeof(*pDevice));
        BDBG_OBJECT_SET(pDevice, NEXUS_7364Device);
        pDevice->openSettings = *pSettings;
        pDevice->pGenericDeviceHandle = pFrontendDevice;

        if (pSettings->cable.enabled) {
            rc = NEXUS_FrontendDevice_Open7364_Cable(pDevice);
            if (rc) { BERR_TRACE(rc); goto err; }
        }

        if (pSettings->terrestrial.enabled) {
            rc = NEXUS_FrontendDevice_Open7364_Terrestrial(pDevice);
            if (rc) { BERR_TRACE(rc); goto err; }
        }

        if (pSettings->satellite.enabled) {
            rc = NEXUS_FrontendDevice_Open7364_Satellite(pDevice);
            if (rc) { BERR_TRACE(rc); goto err; }
        }

        BLST_S_INSERT_HEAD(&g_deviceList, pDevice, node);
    }
    else
    {
        BDBG_MSG(("Found 7364 device"));
        return pDevice->pGenericDeviceHandle;
    }

    pFrontendDevice->pDevice = pDevice;
    pFrontendDevice->getCapabilities = NEXUS_FrontendDevice_P_7364_GetCapabilities;
    pFrontendDevice->getCapabilities = NEXUS_FrontendDevice_P_7364_GetCapabilities;
    pFrontendDevice->getInternalGain = NEXUS_FrontendDevice_P_7364_GetInternalGain;
    pFrontendDevice->getExternalGain = NEXUS_FrontendDevice_P_7364_GetExternalGain;
    pFrontendDevice->setExternalGain = NEXUS_FrontendDevice_P_7364_SetExternalGain;
    pFrontendDevice->getSettings = NEXUS_FrontendDevice_P_Get7364Settings;
    pFrontendDevice->setSettings = NEXUS_FrontendDevice_P_Set7364Settings;
    pFrontendDevice->familyId = 0x7364;
    pFrontendDevice->close = NEXUS_Frontend_P_7364_DestroyDevice;

    pFrontendDevice->mode = NEXUS_StandbyMode_eOn;
    pFrontendDevice->standby = NEXUS_FrontendDevice_P_7364_Standby;

    rc = NEXUS_FrontendDevice_P_OpenMxt7364(pDevice);
    if (rc) goto err;

   return pFrontendDevice;

err:
    NEXUS_Frontend_P_7364_DestroyDevice(pFrontendDevice);
    return NULL;
}

NEXUS_FrontendHandle NEXUS_Frontend_P_Open7364( const NEXUS_FrontendChannelSettings *pSettings )
{
    BDBG_ASSERT(pSettings);
    BDBG_ASSERT(pSettings->device);

    if (pSettings->type == NEXUS_FrontendChannelType_eCable || pSettings->type == NEXUS_FrontendChannelType_eCableOutOfBand) {
        return NEXUS_Frontend_Open7364_Cable(pSettings);
    } else if (pSettings->type == NEXUS_FrontendChannelType_eTerrestrial) {
        return NEXUS_Frontend_Open7364_Terrestrial(pSettings);
    } else if (pSettings->type == NEXUS_FrontendChannelType_eSatellite) {
        return NEXUS_Frontend_Open7364_Satellite(pSettings);
    }
    return NULL;
}

NEXUS_Error NEXUS_FrontendDevice_P_7364_S3Wakeup(NEXUS_7364Device *pDevice)
{
    NEXUS_Error errCode, rc = NEXUS_SUCCESS;
    if (pDevice->openSettings.cable.enabled) {
        errCode = NEXUS_FrontendDevice_P_Init7364_Cable(pDevice);
        if (errCode) { rc = errCode; BERR_TRACE(errCode); }
    }
    if (pDevice->openSettings.satellite.enabled) {
        errCode = NEXUS_FrontendDevice_P_Init7364_Satellite(pDevice);
        if (errCode) { rc = errCode; BERR_TRACE(errCode); }
    }
    if (pDevice->openSettings.terrestrial.enabled) {
        errCode = NEXUS_FrontendDevice_P_Init7364_Terrestrial(pDevice);
        if (errCode) { rc = errCode; BERR_TRACE(errCode); }
    }
    errCode = NEXUS_FrontendDevice_P_OpenMxt7364(pDevice);
    if (errCode) { rc = errCode; BERR_TRACE(errCode); }
    return rc;
}

void NEXUS_FrontendDevice_P_7364_S3Standby(NEXUS_7364Device *pDevice)
{
    NEXUS_FrontendDevice_P_CloseMxt7364(pDevice);
    if (pDevice->openSettings.cable.enabled)
        NEXUS_FrontendDevice_P_Uninit7364_Cable(pDevice);
    if (pDevice->openSettings.satellite.enabled)
        NEXUS_FrontendDevice_P_Uninit7364_Satellite(pDevice);
    if (pDevice->openSettings.terrestrial.enabled)
        NEXUS_FrontendDevice_P_Uninit7364_Terrestrial(pDevice);
}

void NEXUS_Frontend_P_7364_DestroyDevice(void *handle)
{
    NEXUS_7364Device *pDevice = (NEXUS_7364Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7364Device);

    NEXUS_FrontendDevice_P_CloseMxt7364(pDevice);

    /* Cable teardown */
    NEXUS_FrontendDevice_Close7364_Satellite(pDevice);
    /* End of cable teardown */

    /* Terrestrial teardown */
    NEXUS_FrontendDevice_Close7364_Terrestrial(pDevice);
    /* End of terrestrial teardown */

    /* Satellite teardown */
    NEXUS_FrontendDevice_Close7364_Cable(pDevice);
    /* End of satellite teardown */

    BKNI_Free(pDevice->pGenericDeviceHandle);
    pDevice->pGenericDeviceHandle = NULL;

    BLST_S_REMOVE(&g_deviceList, pDevice, NEXUS_7364Device, node);

    BDBG_OBJECT_DESTROY(pDevice, NEXUS_7364Device);
    BKNI_Free(pDevice);
    pDevice = NULL;
}

static NEXUS_Error NEXUS_FrontendDevice_P_7364_Standby(void *handle, const NEXUS_StandbySettings *pSettings)
{

    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7364Device *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7364Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7364Device);

    if ((pDevice->pGenericDeviceHandle->mode != NEXUS_StandbyMode_eDeepSleep) && (pSettings->mode == NEXUS_StandbyMode_eDeepSleep)) {

        BDBG_MSG(("NEXUS_FrontendDevice_P_7364_Standby: Entering deep sleep..."));

        NEXUS_FrontendDevice_P_7364_S3Standby(pDevice);

    } else if ((pDevice->pGenericDeviceHandle->mode == NEXUS_StandbyMode_eDeepSleep) && (pSettings->mode != NEXUS_StandbyMode_eDeepSleep)) {

        BDBG_MSG(("NEXUS_FrontendDevice_P_7364_Standby: Waking up..."));
        rc = NEXUS_FrontendDevice_P_7364_S3Wakeup(pDevice);
        if (rc) { rc = BERR_TRACE(rc); goto done;}
    }

done:
    return rc;
}

static void NEXUS_FrontendDevice_P_7364_GetCapabilities(void *handle, NEXUS_FrontendDeviceCapabilities *pCapabilities)
{
    NEXUS_7364Device *pDevice = (NEXUS_7364Device *)handle;

    BDBG_ASSERT(handle);
    BDBG_ASSERT(pCapabilities);
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7364Device);

    pCapabilities->numTuners = pDevice->satellite.satDevice->numChannels;
}
