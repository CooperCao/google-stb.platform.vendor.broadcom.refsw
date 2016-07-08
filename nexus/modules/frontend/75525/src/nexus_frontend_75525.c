/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
* API Description:
*   API name: Frontend 75525
*    APIs to open, close, and setup initial settings for a BCM75525
*    Tuner/Demodulator Device.
*
***************************************************************************/
/* General includes */
#include "nexus_frontend_75525_priv.h"

BDBG_MODULE(nexus_frontend_75525);

BDBG_OBJECT_ID(NEXUS_75525Device);

static BLST_S_HEAD(devList, NEXUS_75525Device) g_deviceList = BLST_S_INITIALIZER(g_deviceList);

/* Generic function declarations */
static void NEXUS_FrontendDevice_P_75525_GetCapabilities(void *handle, NEXUS_FrontendDeviceCapabilities *pCapabilities);
static void NEXUS_Frontend_P_75525_DestroyDevice(void *handle);
static void NEXUS_FrontendDevice_P_75525_GetCapabilities(void * handle, NEXUS_FrontendDeviceCapabilities *pCapabilities);
NEXUS_Error NEXUS_FrontendDevice_P_75525_GetInternalGain(void *handle, const NEXUS_GainParameters *params, NEXUS_InternalGainSettings *pSettings);
NEXUS_Error NEXUS_FrontendDevice_P_75525_GetExternalGain(void *handle, NEXUS_ExternalGainSettings *pSettings);
NEXUS_Error NEXUS_FrontendDevice_P_75525_SetExternalGain(void *handle, const NEXUS_ExternalGainSettings *pSettings);

static NEXUS_Error NEXUS_FrontendDevice_P_75525_Standby(void * handle, const NEXUS_StandbySettings *pSettings);
/* End of generic function declarations */

NEXUS_Error NEXUS_FrontendDevice_P_75525_GetInternalGain(void *handle, const NEXUS_GainParameters *params, NEXUS_InternalGainSettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_75525Device *pDevice;
    BHAB_InternalGainInputParams inputParams;
    BHAB_InternalGainSettings internalGain;
    NEXUS_GainParameters gainParams;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_75525Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_75525Device);

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
NEXUS_Error NEXUS_FrontendDevice_P_75525_GetExternalGain(void *handle, NEXUS_ExternalGainSettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_75525Device *pDevice;
    BHAB_ExternalGainSettings gain;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_75525Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_75525Device);
    BSTD_UNUSED(pSettings);

    rc = BHAB_GetExternalGain(pDevice->hab, &gain);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pSettings->bypassableGain = gain.externalGainBypassable;
    pSettings->totalGain = gain.externalGainTotal;

done:
    return rc;

}
NEXUS_Error NEXUS_FrontendDevice_P_75525_SetExternalGain(void *handle, const NEXUS_ExternalGainSettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_75525Device *pDevice;
    BHAB_ExternalGainSettings externalGain;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_75525Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_75525Device);

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
NEXUS_Error NEXUS_FrontendDevice_P_Get75525Settings(void *handle, NEXUS_FrontendDeviceSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_75525Device *pDevice = NULL;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_75525Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_75525Device);

    pSettings->rfDaisyChain = pDevice->terrestrial.rfDaisyChain;
    pSettings->enableRfLoopThrough = pDevice->terrestrial.enableRfLoopThrough;
    pSettings->rfInput = pDevice->terrestrial.rfInput;

    return rc;
}

NEXUS_Error NEXUS_FrontendDevice_P_Set75525Settings(void *handle, const NEXUS_FrontendDeviceSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHAB_ConfigSettings habConfigSettings;
    NEXUS_75525Device *pDevice = NULL;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_75525Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_75525Device);

    rc = BHAB_GetConfigSettings(pDevice->hab, &habConfigSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    habConfigSettings.daisyChain = pSettings->rfDaisyChain;
    habConfigSettings.enableLoopThrough = pSettings->enableRfLoopThrough;

    habConfigSettings.rfInputMode = pSettings->rfInput;
    habConfigSettings.tnrApplication = BHAB_TunerApplication_eTerrestrial;

    rc = BHAB_SetConfigSettings(pDevice->hab, &habConfigSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pDevice->terrestrial.rfDaisyChain = pSettings->rfDaisyChain;
    pDevice->terrestrial.enableRfLoopThrough = pSettings->enableRfLoopThrough;
    pDevice->terrestrial.rfInput = pSettings->rfInput;

done:
    return rc;
}

NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_P_Open75525(unsigned index, const NEXUS_FrontendDeviceOpenSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_FrontendDevice *pFrontendDevice = NULL;
    NEXUS_75525Device *pDevice=NULL;

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
        BDBG_OBJECT_SET(pDevice, NEXUS_75525Device);
        pDevice->openSettings = *pSettings;
        pDevice->pGenericDeviceHandle = pFrontendDevice;

        if (pSettings->cable.enabled) {
            rc = NEXUS_FrontendDevice_Open75525_Cable(pDevice);
            if (rc) { BERR_TRACE(rc); goto err; }
        }

        if (pSettings->terrestrial.enabled) {
            rc = NEXUS_FrontendDevice_Open75525_Terrestrial(pDevice);
            if (rc) { BERR_TRACE(rc); goto err; }
        }

        BLST_S_INSERT_HEAD(&g_deviceList, pDevice, node);
    }
    else
    {
        BDBG_MSG(("Found 75525 device"));
        return pDevice->pGenericDeviceHandle;
    }

    pFrontendDevice->pDevice = pDevice;
    pFrontendDevice->getCapabilities = NEXUS_FrontendDevice_P_75525_GetCapabilities;
    pFrontendDevice->getCapabilities = NEXUS_FrontendDevice_P_75525_GetCapabilities;
    pFrontendDevice->getInternalGain = NEXUS_FrontendDevice_P_75525_GetInternalGain;
    pFrontendDevice->getExternalGain = NEXUS_FrontendDevice_P_75525_GetExternalGain;
    pFrontendDevice->setExternalGain = NEXUS_FrontendDevice_P_75525_SetExternalGain;
    pFrontendDevice->getSettings = NEXUS_FrontendDevice_P_Get75525Settings;
    pFrontendDevice->setSettings = NEXUS_FrontendDevice_P_Set75525Settings;
    pFrontendDevice->familyId = 0x75525;
    pFrontendDevice->close = NEXUS_Frontend_P_75525_DestroyDevice;

    pFrontendDevice->mode = NEXUS_StandbyMode_eOn;
    pFrontendDevice->standby = NEXUS_FrontendDevice_P_75525_Standby;

   return pFrontendDevice;

err:
    NEXUS_Frontend_P_75525_DestroyDevice(pFrontendDevice);
    return NULL;
}

NEXUS_FrontendHandle NEXUS_Frontend_P_Open75525( const NEXUS_FrontendChannelSettings *pSettings )
{
    BDBG_ASSERT(pSettings);
    BDBG_ASSERT(pSettings->device);

    if (pSettings->type == NEXUS_FrontendChannelType_eCable || pSettings->type == NEXUS_FrontendChannelType_eCableOutOfBand) {
        return NEXUS_Frontend_Open75525_Cable(pSettings);
    } else if (pSettings->type == NEXUS_FrontendChannelType_eTerrestrial) {
        return NEXUS_Frontend_Open75525_Terrestrial(pSettings);
    }
    return NULL;
}

NEXUS_Error NEXUS_FrontendDevice_P_75525_S3Wakeup(NEXUS_75525Device *pDevice)
{
    NEXUS_Error errCode, rc = NEXUS_SUCCESS;
    if (pDevice->openSettings.cable.enabled) {
        errCode = NEXUS_FrontendDevice_P_Init75525_Cable(pDevice);
        if (errCode) { rc = errCode; BERR_TRACE(errCode); }
    }
    if (pDevice->openSettings.terrestrial.enabled) {
        errCode = NEXUS_FrontendDevice_P_Init75525_Terrestrial(pDevice);
        if (errCode) { rc = errCode; BERR_TRACE(errCode); }
    }
    return rc;
}

void NEXUS_FrontendDevice_P_75525_S3Standby(NEXUS_75525Device *pDevice)
{
    if (pDevice->openSettings.cable.enabled)
        NEXUS_FrontendDevice_P_Uninit75525_Cable(pDevice);
    if (pDevice->openSettings.terrestrial.enabled)
        NEXUS_FrontendDevice_P_Uninit75525_Terrestrial(pDevice);
}

void NEXUS_Frontend_P_75525_DestroyDevice(void *handle)
{
    NEXUS_75525Device *pDevice = (NEXUS_75525Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_75525Device);

    /* Terrestrial teardown */
    NEXUS_FrontendDevice_Close75525_Terrestrial(pDevice);
    /* End of terrestrial teardown */

    /* Cable teardown */
    NEXUS_FrontendDevice_Close75525_Cable(pDevice);
    /* End of cable teardown */

    BKNI_Free(pDevice->pGenericDeviceHandle);
    pDevice->pGenericDeviceHandle = NULL;

    BLST_S_REMOVE(&g_deviceList, pDevice, NEXUS_75525Device, node);

    BDBG_OBJECT_DESTROY(pDevice, NEXUS_75525Device);
    BKNI_Free(pDevice);
    pDevice = NULL;
}

static NEXUS_Error NEXUS_FrontendDevice_P_75525_Standby(void *handle, const NEXUS_StandbySettings *pSettings)
{

    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_75525Device *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_75525Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_75525Device);

    if ((pDevice->pGenericDeviceHandle->mode != NEXUS_StandbyMode_eDeepSleep) && (pSettings->mode == NEXUS_StandbyMode_eDeepSleep)) {

        BDBG_MSG(("NEXUS_FrontendDevice_P_75525_Standby: Entering deep sleep..."));

        NEXUS_FrontendDevice_P_75525_S3Standby(pDevice);

    } else if ((pDevice->pGenericDeviceHandle->mode == NEXUS_StandbyMode_eDeepSleep) && (pSettings->mode != NEXUS_StandbyMode_eDeepSleep)) {

        BDBG_MSG(("NEXUS_FrontendDevice_P_75525_Standby: Waking up..."));
        rc = NEXUS_FrontendDevice_P_75525_S3Wakeup(pDevice);
        if (rc) { rc = BERR_TRACE(rc); goto done;}
    }

done:
    return rc;
}

static void NEXUS_FrontendDevice_P_75525_GetCapabilities(void *handle, NEXUS_FrontendDeviceCapabilities *pCapabilities)
{
    NEXUS_75525Device *pDevice = (NEXUS_75525Device *)handle;

    BDBG_ASSERT(handle);
    BDBG_ASSERT(pCapabilities);
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_75525Device);

}
