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
* API Description:
*   API name: Frontend 3466
*    APIs to open, close, and setup initial settings for a BCM3466
*    Tuner/Demodulator Device.
*
***************************************************************************/
/* General includes */
#include "nexus_frontend_3466_priv.h"

#include "bchp_3466_leap_host_l1.h"
#include "bhab_3466_priv.h"
#include "bhab_3466_fw.h"
#include "bchp_3466_tm.h"

BDBG_MODULE(nexus_frontend_3466);

BDBG_OBJECT_ID(NEXUS_3466Device);

static BLST_S_HEAD(devList, NEXUS_3466Device) g_deviceList = BLST_S_INITIALIZER(g_deviceList);

/* Generic function declarations */
static void NEXUS_FrontendDevice_P_3466_GetCapabilities(void *handle, NEXUS_FrontendDeviceCapabilities *pCapabilities);
static void NEXUS_FrontendDevice_P_3466_GetTunerCapabilities(void *handle, unsigned tunerIndex, NEXUS_FrontendCapabilities *pCapabilities);
static void NEXUS_Frontend_P_3466_DestroyDevice(void *handle);
NEXUS_Error NEXUS_FrontendDevice_P_3466_GetInternalGain(void *handle, const NEXUS_GainParameters *params, NEXUS_InternalGainSettings *pSettings);
NEXUS_Error NEXUS_FrontendDevice_P_3466_GetExternalGain(void *handle, NEXUS_ExternalGainSettings *pSettings);
NEXUS_Error NEXUS_FrontendDevice_P_3466_SetExternalGain(void *handle, const NEXUS_ExternalGainSettings *pSettings);
static NEXUS_Error NEXUS_FrontendDevice_P_3466_GetStatus(void *handle, NEXUS_FrontendDeviceStatus *pStatus);

static NEXUS_Error NEXUS_Frontend_P_3466_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings);
static NEXUS_Error NEXUS_FrontendDevice_P_3466_Standby(void * handle, const NEXUS_StandbySettings *pSettings);

static void NEXUS_Frontend_P_3466_Close(NEXUS_FrontendHandle handle);
static NEXUS_Error NEXUS_FrontendDevice_P_3466_Link(NEXUS_FrontendDeviceHandle parentHandle, NEXUS_FrontendDeviceHandle childHandle, const NEXUS_FrontendDeviceLinkSettings *pSettings);
static void NEXUS_Frontend_P_3466_SetUserParameters(NEXUS_FrontendHandle handle, const NEXUS_FrontendUserParameters *pParams);
/* End of generic function declarations */

#if NEXUS_HAS_GPIO
/***************************************************************************
Summary:
    Enable/Disable interrupts for a 3466 device
 ***************************************************************************/
static void NEXUS_Frontend_P_3466_GpioIsrControl_isr(bool enable, void *pParam)
{
    NEXUS_GpioHandle gpioHandle;
    BDBG_ASSERT(NULL != pParam);
    gpioHandle = (NEXUS_GpioHandle)pParam;

#if NEXUS_FRONTEND_DEBUG_IRQ
    BDBG_MSG(("%s 3466 Gpio Interrupt %p", enable ? "Enable" : "Disable", (void *)gpioHandle));
#endif
    NEXUS_Gpio_SetInterruptEnabled_isr(gpioHandle, enable);
}
#endif

static void NEXUS_Frontend_P_3466_IsrControl_isr(bool enable, void *pParam)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned *isrNumber = (unsigned *)pParam;
    BDBG_ASSERT(NULL != pParam);

    if ( enable )
    {
#if NEXUS_FRONTEND_DEBUG_IRQ
        BDBG_MSG(("Enable 3466 Interrupt %u", *isrNumber));
#endif
        rc = NEXUS_Core_EnableInterrupt_isr(*isrNumber);
        if(rc) BERR_TRACE(rc);
    }
    else
    {
#if NEXUS_FRONTEND_DEBUG_IRQ
        BDBG_MSG(("Disable 3466 Interrupt %u", *isrNumber));
#endif
        NEXUS_Core_DisableInterrupt_isr(*isrNumber);
    }
}

static void NEXUS_Frontend_P_3466_L1_isr(void *pParam1, int pParam2)
{
    NEXUS_3466Device *pDevice = (NEXUS_3466Device *)pParam1;
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHAB_Handle hab;
    BDBG_ASSERT(NULL != pParam1);
    hab = (BHAB_Handle)pDevice->hab;
    BSTD_UNUSED(pParam2);

#if NEXUS_FRONTEND_DEBUG_IRQ
    BDBG_MSG(("3466 L1 ISR (hab: %p)", (void *)hab));
#endif
    if(hab != NULL){
        rc = BHAB_HandleInterrupt_isr(hab);
        if(rc){rc = BERR_TRACE(rc);}
    }
#if NEXUS_FRONTEND_DEBUG_IRQ
    BDBG_MSG(("Done: 3466 L1 ISR (hab: %p)", (void *)hab));
#endif
}

static void NEXUS_Frontend_P_3466_IsrEvent(void *pParam)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHAB_Handle hab;
    BDBG_ASSERT(NULL != pParam);
    hab = (BHAB_Handle)pParam;

#if NEXUS_FRONTEND_DEBUG_IRQ
    BDBG_MSG(("3466 ISR Callback (hab: %p)", (void *)hab));
#endif
    if(hab != NULL){
        rc = BHAB_ProcessInterruptEvent(hab);
        if(rc) BERR_TRACE(rc);
    }
}

static NEXUS_Error NEXUS_Frontend_P_3466_WriteRegister(void *handle, unsigned address, uint32_t value)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3466Channel *pChannel = (NEXUS_3466Channel *)handle;
    NEXUS_3466Device *pDevice;
    BDBG_ASSERT(NULL != handle);

    pDevice = pChannel->pDevice;
    rc = BHAB_WriteRegister(pDevice->hab, address, &value);
    if(rc){rc = BERR_TRACE(rc);}

    return rc;
}
static NEXUS_Error NEXUS_Frontend_P_3466_ReadRegister(void *handle, unsigned address, uint32_t *value   )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3466Channel *pChannel = (NEXUS_3466Channel *)handle;
    NEXUS_3466Device *pDevice;
    BDBG_ASSERT(NULL != handle);

    pDevice = pChannel->pDevice;
    rc = BHAB_ReadRegister(pDevice->hab, address, value);
    if(rc){rc = BERR_TRACE(rc);}

    return rc;
}

void NEXUS_FrontendDevice_P_Uninit_3466_Hab(NEXUS_3466Device *pDevice)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_GpioSettings gpioSettings;

    if(pDevice->isrNumber) {
        NEXUS_Core_DisconnectInterrupt(pDevice->isrNumber);
    }
    else if(pDevice->gpioInterrupt){
        NEXUS_Gpio_SetInterruptCallback_priv(pDevice->gpioInterrupt, NULL, NULL, 0);
        NEXUS_Gpio_GetSettings(pDevice->gpioInterrupt,  &gpioSettings);
        gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
        gpioSettings.interrupt.callback = NULL;
        NEXUS_Gpio_SetSettings(pDevice->gpioInterrupt, &gpioSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    if (pDevice->openSettings.cable.enabled) {
        NEXUS_FrontendDevice_P_Uninit_3466_Hab_Cable(pDevice);
    }
    if (pDevice->openSettings.terrestrial.enabled) {
        NEXUS_FrontendDevice_P_Uninit_3466_Hab_Terrestrial(pDevice);
    }

    if(pDevice->isrEvent) pDevice->isrEvent = NULL;
    if(pDevice->isrEventCallback) {
        NEXUS_UnregisterEvent(pDevice->isrEventCallback);
    }
    pDevice->isrEventCallback = NULL;

#if NEXUS_HAS_MXT
    if (pDevice->pGenericDeviceHandle) {
        if (pDevice->pGenericDeviceHandle->mtsifConfig.mxt) {
            BMXT_Close(pDevice->pGenericDeviceHandle->mtsifConfig.mxt);
            pDevice->pGenericDeviceHandle->mtsifConfig.mxt = NULL;
        }
        BKNI_Memset((void *)&pDevice->pGenericDeviceHandle->mtsifConfig, 0, sizeof(pDevice->pGenericDeviceHandle->mtsifConfig));
    }
#endif
    if (pDevice->hab) {
        BHAB_Close(pDevice->hab);
        pDevice->hab = NULL;
    }

    if (pDevice->cbHandle) {
        BINT_EnableCallback(pDevice->cbHandle);
        BINT_DestroyCallback(pDevice->cbHandle);
        pDevice->cbHandle = NULL;
    }

    if (pDevice->leapBuffer) {
        NEXUS_Memory_Free(pDevice->leapBuffer);
        pDevice->leapBuffer = NULL;
    }

    if(pDevice->pollingThread){
        NEXUS_Thread_Destroy(pDevice->pollingThread);
        pDevice->pollingThread = NULL;
    }

    BKNI_Memset((void *)&pDevice->isTunerPoweredOn, 0, sizeof(pDevice->isTunerPoweredOn));
    BKNI_Memset((void *)&pDevice->cable.isPoweredOn, 0, sizeof(pDevice->cable.isPoweredOn));
    BKNI_Memset((void *)&pDevice->terrestrial.isPoweredOn, 0, sizeof(pDevice->terrestrial.isPoweredOn));
done:
    return;
}

static void NEXUS_Frontend_3466DevicePollingThread(void *arg)
{
    NEXUS_3466Device *pDevice = (NEXUS_3466Device *)arg;
    BDBG_WRN(("NEXUS_Frontend_3466DevicePollingThread......"));
    while (1) {
        BKNI_Sleep(100);

        BKNI_EnterCriticalSection();
        if ((!pDevice->pGenericDeviceHandle->abortThread) && pDevice->hab)
            NEXUS_Frontend_P_3466_L1_isr((void *)pDevice, 0);
        else {
            BKNI_LeaveCriticalSection();
            goto done;
        }
        BKNI_LeaveCriticalSection();

    }
done:
    return;
}

NEXUS_Error NEXUS_FrontendDevice_P_Init_3466_Hab(NEXUS_3466Device *pDevice, const NEXUS_FrontendDeviceOpenSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHAB_Settings habSettings;
    uint32_t  familyId, chipId;
    BHAB_Handle habHandle;

    void *regHandle;

    rc = BHAB_3466_GetDefaultSettings(&habSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if (pSettings->spiDevice) {
        BDBG_MSG(("Configuring for SPI"));
        habSettings.chipAddr = 0x24;
        habSettings.isSpi = true;
        regHandle = (void *)NEXUS_Spi_GetRegHandle(pSettings->spiDevice);
    } else if (pSettings->i2cDevice) {
        BDBG_MSG(("Configuring for I2C"));
        habSettings.chipAddr = pSettings->i2cAddress;
        habSettings.isSpi = false;
        regHandle = (void *)NEXUS_I2c_GetRegHandle(pSettings->i2cDevice, NEXUS_MODULE_SELF);
    } else {
        regHandle = NULL;
    }
    BDBG_ASSERT(regHandle);
    habSettings.isMtsif = true;

    if(pDevice->openSettings.interruptMode != NEXUS_FrontendInterruptMode_ePolling){
        if(pSettings->isrNumber) {
            BDBG_MSG(("Configuring for external interrupt"));
            habSettings.interruptEnableFunc = NEXUS_Frontend_P_3466_IsrControl_isr;
            habSettings.interruptEnableFuncParam = (void*)&pDevice->openSettings.isrNumber;
        }
        else if(pSettings->gpioInterrupt){
            BDBG_MSG(("Configuring for GPIO interrupt"));
            habSettings.interruptEnableFunc = NEXUS_Frontend_P_3466_GpioIsrControl_isr;
            habSettings.interruptEnableFuncParam = (void*)pSettings->gpioInterrupt;
        }
    }

    if(pDevice->openSettings.interruptMode != NEXUS_FrontendInterruptMode_ePolling){
        if(pSettings->isrNumber) {
            BDBG_MSG(("Connecting external interrupt"));
            rc = NEXUS_Core_ConnectInterrupt(pSettings->isrNumber,
                                                 NEXUS_Frontend_P_3466_L1_isr,
                                                 (void *)pDevice,
                                                 0);
            if (rc) { BERR_TRACE(rc); goto done; }
        }
        else if(pSettings->gpioInterrupt){
            BDBG_MSG(("Connecting GPIO interrupt"));
            NEXUS_Gpio_SetInterruptCallback_priv(pSettings->gpioInterrupt,
                                                 NEXUS_Frontend_P_3466_L1_isr,
                                                 (void *)pDevice,
                                                 0);
        }
    }

    rc = BHAB_Open(&habHandle, regHandle, &habSettings);
    if (rc) { BERR_TRACE(rc); goto done; }

    pDevice->hab = habHandle;

    /* Get events and register callbacks */
    rc = BHAB_GetInterruptEventHandle(pDevice->hab, &pDevice->isrEvent);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if((pDevice->pollingThread == NULL) && (pDevice->openSettings.interruptMode == NEXUS_FrontendInterruptMode_ePolling)){
        NEXUS_ThreadSettings thread_settings;
        NEXUS_Thread_GetDefaultSettings(&thread_settings);
        thread_settings.priority = 0;
        pDevice->pollingThread = NEXUS_Thread_Create("pollingThread",
                                                NEXUS_Frontend_3466DevicePollingThread,
                                                (void*)pDevice,
                                                &thread_settings);
    }

    pDevice->isrEventCallback = NEXUS_RegisterEvent(pDevice->isrEvent, NEXUS_Frontend_P_3466_IsrEvent, pDevice->hab);
    if ( NULL == pDevice->isrEventCallback ){rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done; }

    BDBG_MSG(("Loading firmware..."));

    rc = BHAB_ReadRegister(pDevice->hab, BCHP_TM_FAMILY_ID, &familyId);
    if(rc){rc = BERR_TRACE(rc);}

    rc = BHAB_ReadRegister(pDevice->hab, BCHP_TM_PRODUCT_ID, &chipId);
    if(rc){rc = BERR_TRACE(rc);}

    if (pSettings->loadAP)
    {
        {
            unsigned magic;
            unsigned family;
            unsigned rev;
            unsigned probedRev;
            const uint8_t *fw_image = NULL;

            if((familyId >> 16) == 0x3465)
                fw_image = bcm3465_leap_image;
            else
                fw_image = bcm3466_leap_image;

            magic  = (fw_image[ 0] << 24) + (fw_image[ 1] << 16) + (fw_image[ 2] << 8) + (fw_image[ 3]);
            family = (fw_image[36] << 24) + (fw_image[37] << 16) + (fw_image[38] << 8) + (fw_image[39]);
            rev    = (fw_image[44] << 24) + (fw_image[45] << 16) + (fw_image[46] << 8) + (fw_image[47]);

            if ((magic != 0xaaaaaaaa) || (family != (familyId >> 16))) {
                BDBG_WRN(("Possible 3466 firmware corruption, expected values not matched."));
            }

            probedRev = ((chipId & 0xFF)  == 0x10) ? 0xB0 : 0xA0;

            if (probedRev != 0x00000000) {
                if (probedRev != rev) {
                    BDBG_WRN(("Chip revision does not match firmware revision. Does NEXUS_FRONTEND_3466_VER=%X need to be set?", probedRev));
                }
            }
        }

        if((familyId >> 16) == 0x3466) {
            rc = BHAB_InitAp(pDevice->hab, bcm3466_leap_image);
            if(rc){rc = BERR_TRACE(rc); goto done;}
        }
        else if((familyId >> 16) == 0x3465) {
            rc = BHAB_InitAp(pDevice->hab, bcm3465_leap_image);
            if(rc){rc = BERR_TRACE(rc); goto done;}
        }
        else {
            BDBG_WRN(("Invalid Frontend Chip"));
            goto done;
        }
    }

    rc =  BHAB_GetTunerChannels(pDevice->hab, &pDevice->capabilities.totalTunerChannels);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if(!pDevice->capabilities.channelCapabilities){
        pDevice->capabilities.channelCapabilities = (BHAB_ChannelCapability *) BKNI_Malloc( pDevice->capabilities.totalTunerChannels*sizeof(BHAB_ChannelCapability));
        if(!pDevice->capabilities.channelCapabilities){
            rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done;
        }
    }

    rc =  BHAB_GetCapabilities(pDevice->hab, &pDevice->capabilities);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    BDBG_MSG(("pDevice->capabilities.totalTunerChannels: %d", pDevice->capabilities.totalTunerChannels));
    /* Open tuners */
    {
        BTNR_3466_Settings tnrIb3466_cfg;
        unsigned i;
        for (i=0; i < pDevice->capabilities.totalTunerChannels; i++) {
            BDBG_MSG(("Opening tuner %d", i));
            BTNR_3466_GetDefaultSettings(&tnrIb3466_cfg);
            tnrIb3466_cfg.channelNo = i;
            rc =  BTNR_3466_Open(&pDevice->tnr[i],&tnrIb3466_cfg, pDevice->hab);
            BDBG_MSG(("tuner %d is %p", i, (void *)pDevice->tnr[i]));
        }
    }

    if (pDevice->openSettings.cable.enabled) {
        BDBG_MSG(("Opening cable device"));
        rc = NEXUS_FrontendDevice_P_Init_3466_Hab_Cable(pDevice, pSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    if (pDevice->openSettings.terrestrial.enabled) {
        BDBG_MSG(("Opening terrestrial device"));
        rc = NEXUS_FrontendDevice_P_Init_3466_Hab_Terrestrial(pDevice, pSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

#if NEXUS_HAS_MXT
        {
            BMXT_Settings mxtSettings;
            unsigned i;

            /* open MXT */
            BMXT_GetDefaultSettings(&mxtSettings);
            mxtSettings.chip = BMXT_Chip_e3466;
            mxtSettings.chipRev = BMXT_ChipRev_eA0;
            for (i=0; i < BMXT_NUM_MTSIF; i++) {
                mxtSettings.MtsifTxCfg[i].TxClockPolarity = 0;
                mxtSettings.MtsifTxCfg[i].Enable = true;
                NEXUS_Module_Lock(g_NEXUS_frontendModuleSettings.transport);
                mxtSettings.MtsifTxCfg[i].Encrypt = NEXUS_TransportModule_P_IsMtsifEncrypted();
                NEXUS_Module_Unlock(g_NEXUS_frontendModuleSettings.transport);
            }
            mxtSettings.MtsifRxCfg[0].RxClockPolarity = !(mxtSettings.MtsifTxCfg[0].TxClockPolarity);
            mxtSettings.MtsifRxCfg[0].Enable = true;
            mxtSettings.MtsifRxCfg[0].Decrypt = mxtSettings.MtsifTxCfg[0].Encrypt;
            mxtSettings.hHab = pDevice->hab;
            rc = BMXT_Open(&pDevice->pGenericDeviceHandle->mtsifConfig.mxt, NULL, NULL, &mxtSettings);
            if (rc!=BERR_SUCCESS) { BERR_TRACE(rc); goto done; }

            rc = NEXUS_Frontend_P_InitMtsifConfig(&pDevice->pGenericDeviceHandle->mtsifConfig, &mxtSettings);
            if (rc!=BERR_SUCCESS) { BERR_TRACE(rc); goto done; }
        }
#endif

    return rc;

done:
    NEXUS_FrontendDevice_P_Uninit_3466_Hab(pDevice);
    return rc;
}



NEXUS_Error NEXUS_FrontendDevice_P_3466_GetInternalGain(void *handle, const NEXUS_GainParameters *params, NEXUS_InternalGainSettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3466Device *pDevice;
    BHAB_InternalGainInputParams inputParams;
    BHAB_InternalGainSettings internalGain;
    NEXUS_GainParameters gainParams;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_3466Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3466Device);

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
NEXUS_Error NEXUS_FrontendDevice_P_3466_GetExternalGain(void *handle, NEXUS_ExternalGainSettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3466Device *pDevice;
    BHAB_ExternalGainSettings gain;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_3466Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3466Device);
    BSTD_UNUSED(pSettings);

    rc = BHAB_GetExternalGain(pDevice->hab, &gain);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pSettings->bypassableGain = gain.externalGainBypassable;
    pSettings->totalGain = gain.externalGainTotal;

done:
    return rc;

}
NEXUS_Error NEXUS_FrontendDevice_P_3466_SetExternalGain(void *handle, const NEXUS_ExternalGainSettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3466Device *pDevice;
    BHAB_ExternalGainSettings externalGain;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_3466Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3466Device);

    externalGain.externalGainTotal = pSettings->totalGain;
    externalGain.externalGainBypassable = pSettings->bypassableGain;

    rc = BHAB_SetExternalGain(pDevice->hab, &externalGain);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}
/* End of generic function declarations */

/* Public API entry point */
static NEXUS_Error NEXUS_FrontendDevice_P_3466_GetDeviceAmplifierStatus(void *handle, NEXUS_FrontendDeviceAmplifierStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3466Device *pDevice = (NEXUS_3466Device *)handle;
    BHAB_LnaStatus lnaStatus;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3466Device);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    rc = BHAB_GetLnaStatus(pDevice->hab, &lnaStatus);
    if (rc) { BERR_TRACE(rc); }
    else {
        if (lnaStatus.externalFixedGainLnaState == BHAB_FixedGainLnaState_eBypass ||
            lnaStatus.externalFixedGainLnaState == BHAB_FixedGainLnaState_eEnabled)
        {
            pStatus->externalFixedGain = (lnaStatus.externalFixedGainLnaState == BHAB_FixedGainLnaState_eBypass) ? NEXUS_FrontendDeviceAmplifierState_eBypass : NEXUS_FrontendDeviceAmplifierState_eEnabled;
        } else {
            rc = BERR_TRACE(NEXUS_UNKNOWN);
        }
    }
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_3466_GetFastStatus(void *handle, NEXUS_FrontendFastStatus *pStatus)
{
    NEXUS_3466Device *p3466Device;
    NEXUS_3466Channel *pChannel;

    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3466Channel *)handle;
    p3466Device = (NEXUS_3466Device *)pChannel->pDevice;

    if (p3466Device->lastTuneQam) {
        if (p3466Device->cable.api.getFastStatus)
            return p3466Device->cable.api.getFastStatus(handle, pStatus);
    } else {
        if (p3466Device->terrestrial.api.getFastStatus)
            return p3466Device->terrestrial.api.getFastStatus(handle, pStatus);
    }
    return NEXUS_NOT_SUPPORTED;
}

static NEXUS_Error NEXUS_Frontend_P_3466_ReadSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length, size_t *pNumRead)
{
    NEXUS_3466Device *p3466Device;
    NEXUS_3466Channel *pChannel;

    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3466Channel *)handle;
    p3466Device = (NEXUS_3466Device *)pChannel->pDevice;

    if (p3466Device->lastTuneQam) {
        if (p3466Device->cable.api.readSoftDecisions)
            return p3466Device->cable.api.readSoftDecisions(handle, pDecisions, length, pNumRead);
    } else {
        if (p3466Device->terrestrial.api.readSoftDecisions)
            return p3466Device->terrestrial.api.readSoftDecisions(handle, pDecisions, length, pNumRead);
    }
    return NEXUS_NOT_SUPPORTED;
}

static void NEXUS_Frontend_P_3466_UnTune(void *handle)
{
    NEXUS_3466Device *p3466Device;
    NEXUS_3466Channel *pChannel;

    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3466Channel *)handle;
    p3466Device = (NEXUS_3466Device *)pChannel->pDevice;

    if (p3466Device->lastTuneQam) {
        if (p3466Device->cable.api.untune)
            p3466Device->cable.api.untune(handle);
    } else {
        if (p3466Device->terrestrial.api.untune)
            p3466Device->terrestrial.api.untune(handle);
    }
    if (p3466Device->isTunerPoweredOn[pChannel->chn_num]) {
        BTNR_PowerSaverSettings pwrSettings;
        if (NEXUS_SUCCESS == BTNR_GetPowerSaver(p3466Device->tnr[pChannel->chn_num], &pwrSettings)) {
            BERR_Code rc;
            pwrSettings.enable = true;
            rc = BTNR_SetPowerSaver(p3466Device->tnr[pChannel->chn_num], &pwrSettings);
            if (rc)
                BDBG_WRN(("Disabling power failed."));
            p3466Device->isTunerPoweredOn[pChannel->chn_num] = false;
        }
    }
}

NEXUS_Error NEXUS_FrontendDevice_P_Get3466Settings(void *handle, NEXUS_FrontendDeviceSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3466Device *pDevice = NULL;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_3466Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3466Device);

    pSettings->rfDaisyChain = pDevice->terrestrial.rfDaisyChain;
    pSettings->enableRfLoopThrough = pDevice->terrestrial.enableRfLoopThrough;
    pSettings->rfInput = pDevice->terrestrial.rfInput;
    pSettings->terrestrial.numDiversityChannels = pDevice->terrestrial.numDiversityChannels;

    return rc;
}
NEXUS_Error NEXUS_FrontendDevice_P_Set3466Settings(void *handle, const NEXUS_FrontendDeviceSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHAB_ConfigSettings habConfigSettings;
    BODS_ConfigSettings odsConfigSettings;
    NEXUS_3466Device *pDevice = NULL;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_3466Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3466Device);

    pDevice->terrestrial.numDiversityChannels = pSettings->terrestrial.numDiversityChannels;
    pDevice->terrestrial.currentDiversityChannel = 0;

    odsConfigSettings.numDiversityChannels = pDevice->terrestrial.numDiversityChannels;
    rc = BODS_SetConfigSettings(pDevice->terrestrial.ods, &odsConfigSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

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

NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_P_Open3466(unsigned index, const NEXUS_FrontendDeviceOpenSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_FrontendDevice *pFrontendDevice = NULL;
    NEXUS_3466Device *pDevice=NULL;

    BSTD_UNUSED(index);
    BDBG_ASSERT(pSettings);

    for ( pDevice = BLST_S_FIRST(&g_deviceList); NULL != pDevice; pDevice = BLST_S_NEXT(pDevice, node) )
    {
        BDBG_OBJECT_ASSERT(pDevice, NEXUS_3466Device);
        if(((pSettings->i2cAddress == pDevice->openSettings.i2cAddress)
                && (pSettings->i2cDevice == pDevice->openSettings.i2cDevice))
                && (pSettings->spiDevice == pDevice->openSettings.spiDevice))
        {
            break;
        }
    }

    if ( NULL == pDevice)
    {
        pFrontendDevice = NEXUS_FrontendDevice_P_Create();
        if (NULL == pFrontendDevice) { BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err; }

        pDevice = BKNI_Malloc(sizeof(*pDevice));
        if (NULL == pDevice) { BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err; }
        BKNI_Memset(pDevice, 0, sizeof(*pDevice));
        BDBG_OBJECT_SET(pDevice, NEXUS_3466Device);
        pDevice->openSettings = *pSettings;
        pDevice->pGenericDeviceHandle = pFrontendDevice;

        BDBG_MSG(("init hab"));
        rc = NEXUS_FrontendDevice_P_Init_3466_Hab(pDevice, &pDevice->openSettings);
        if(rc){rc = BERR_TRACE(rc); goto err;}

        /* cable support is not the default and must be explicitly specified. */
        if (pSettings->cable.enabled) {
            BDBG_MSG(("Open cable..."));
            rc = NEXUS_FrontendDevice_Open3466_Cable(pDevice);
            if (rc) { BERR_TRACE(rc); goto err; }
        }
        if (pSettings->terrestrial.enabled) {
            BDBG_MSG(("Open terrestrial..."));
            rc = NEXUS_FrontendDevice_Open3466_Terrestrial(pDevice);
            if (rc) { BERR_TRACE(rc); goto err; }
        }

        BLST_S_INSERT_HEAD(&g_deviceList, pDevice, node);
    }
    else
    {
        BDBG_MSG(("Found 3466 device"));
        return pDevice->pGenericDeviceHandle;
    }

    pFrontendDevice->pDevice = pDevice;
    pFrontendDevice->getCapabilities = NEXUS_FrontendDevice_P_3466_GetCapabilities;
    pFrontendDevice->getTunerCapabilities = NEXUS_FrontendDevice_P_3466_GetTunerCapabilities;
    pFrontendDevice->getInternalGain = NEXUS_FrontendDevice_P_3466_GetInternalGain;
    pFrontendDevice->getExternalGain = NEXUS_FrontendDevice_P_3466_GetExternalGain;
    pFrontendDevice->setExternalGain = NEXUS_FrontendDevice_P_3466_SetExternalGain;
    pFrontendDevice->getSettings = NEXUS_FrontendDevice_P_Get3466Settings;
    pFrontendDevice->setSettings = NEXUS_FrontendDevice_P_Set3466Settings;
    pFrontendDevice->getDeviceAmplifierStatus = NEXUS_FrontendDevice_P_3466_GetDeviceAmplifierStatus;
    pFrontendDevice->getStatus = NEXUS_FrontendDevice_P_3466_GetStatus;
    pFrontendDevice->familyId = 0x3466;
    pFrontendDevice->close = NEXUS_Frontend_P_3466_DestroyDevice;

    pFrontendDevice->mode = NEXUS_StandbyMode_eOn;
    pFrontendDevice->standby = NEXUS_FrontendDevice_P_3466_Standby;

   return pFrontendDevice;

err:
    NEXUS_Frontend_P_3466_DestroyDevice(pFrontendDevice);
    return NULL;
}

static NEXUS_Error NEXUS_Frontend_P_3466_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3466Device *pDevice;
    NEXUS_3466Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3466Channel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3466Device);

    if (pDevice->openSettings.cable.enabled && pDevice->cable.api.standby) {
        rc = pDevice->cable.api.standby(handle, enabled, pSettings);
    }
    if (pDevice->openSettings.terrestrial.enabled && pDevice->terrestrial.api.standby) {
        rc = pDevice->terrestrial.api.standby(handle, enabled, pSettings);
    }

    BSTD_UNUSED(enabled);
    BSTD_UNUSED(pSettings);

    return rc;
}

NEXUS_FrontendHandle NEXUS_Frontend_P_Open3466( const NEXUS_FrontendChannelSettings *pSettings )
{
    NEXUS_3466Device *p3466Device = NULL;
    NEXUS_3466Channel *pChannel;
    unsigned channelNumber;
    NEXUS_FrontendDeviceHandle pFrontendDevice;

    NEXUS_FrontendHandle frontend = NULL;

    BDBG_ASSERT(pSettings);
    BDBG_ASSERT(pSettings->device);

    if (pSettings->device == NULL) {
        BDBG_WRN(("No device..."));
        return NULL;
    }

    pFrontendDevice = pSettings->device;
    p3466Device = (NEXUS_3466Device *)pFrontendDevice->pDevice;

    channelNumber = pSettings->channelNumber;

    if ( p3466Device->frontendHandle[channelNumber] != NULL )
    {
        return p3466Device->frontendHandle[channelNumber];
    }

    pChannel = (NEXUS_3466Channel*)BKNI_Malloc(sizeof(*pChannel));
    if ( NULL == pChannel ) { BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err;}
    BKNI_Memset(pChannel, 0, sizeof(*pChannel));
    pChannel->chn_num = channelNumber;
    pChannel->pDevice = p3466Device;

    /* Create a Nexus frontend handle */
    frontend = NEXUS_Frontend_P_Create(pChannel);
    if ( NULL == frontend ) { BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err;}

    p3466Device->frontendHandle[channelNumber] = frontend;

    if (p3466Device->openSettings.cable.enabled) {
        BDBG_MSG(("opening cable..."));
        NEXUS_Frontend_Open3466_Cable(pSettings, frontend);
    }
    if (p3466Device->openSettings.terrestrial.enabled) {
        BDBG_MSG(("opening terrestrial..."));
        NEXUS_Frontend_Open3466_Terrestrial(pSettings, frontend);
    }

    frontend->writeRegister = NEXUS_Frontend_P_3466_WriteRegister;
    frontend->readRegister = NEXUS_Frontend_P_3466_ReadRegister;
    frontend->getFastStatus = NEXUS_Frontend_P_3466_GetFastStatus;
    frontend->readSoftDecisions = NEXUS_Frontend_P_3466_ReadSoftDecisions;
    frontend->untune = NEXUS_Frontend_P_3466_UnTune;
    frontend->close = NEXUS_Frontend_P_3466_Close;

    frontend->postSetUserParameters = NEXUS_Frontend_P_3466_SetUserParameters;

    frontend->standby = NEXUS_Frontend_P_3466_Standby;

    /* save channel number in pChannel*/
#if 0
    /* TODO: Fill out family/chip ID correctly */
    frontend->chip.familyId = pDevice->chipFamilyId;
    frontend->chip.id = pDevice->chipId;
#else
    frontend->chip.familyId = 0x3466;
    frontend->chip.id = 0x3466;
#endif

    frontend->userParameters.isMtsif = true;
    frontend->mtsif.inputBand = pSettings->channelNumber;

    frontend->pGenericDeviceHandle = pFrontendDevice;

    return frontend;

err:
/* TODO: proper error handling */
    return NULL;
}

NEXUS_Error NEXUS_FrontendDevice_P_3466_S3Wakeup(NEXUS_3466Device *pDevice)
{
    NEXUS_Error errCode, rc = NEXUS_SUCCESS;
    errCode = NEXUS_FrontendDevice_P_Init_3466_Hab(pDevice, &pDevice->openSettings);
    if (errCode) { rc = errCode; BERR_TRACE(errCode); }
    if (pDevice->openSettings.cable.enabled) {
        errCode = NEXUS_FrontendDevice_P_Init3466_Cable(pDevice);
        if (errCode) { rc = errCode; BERR_TRACE(errCode); }
    }
    if (pDevice->openSettings.terrestrial.enabled) {
        errCode = NEXUS_FrontendDevice_P_Init3466_Terrestrial(pDevice);
        if (errCode) { rc = errCode; BERR_TRACE(errCode); }
    }
    return rc;
}

void NEXUS_FrontendDevice_P_3466_S3Standby(NEXUS_3466Device *pDevice)
{
    if (pDevice->openSettings.cable.enabled) {
        NEXUS_FrontendDevice_P_Uninit3466_Cable(pDevice);
    }
    if (pDevice->openSettings.terrestrial.enabled) {
        NEXUS_FrontendDevice_P_Uninit3466_Terrestrial(pDevice);
    }
    NEXUS_FrontendDevice_P_Uninit_3466_Hab(pDevice);
}

static void NEXUS_Frontend_P_3466_Close(NEXUS_FrontendHandle handle)
{
    NEXUS_3466Device *pDevice;
    NEXUS_3466Channel *pChannel;

    pChannel =(NEXUS_3466Channel *)handle->pDeviceHandle;
    BDBG_ASSERT(NULL != pChannel);
    pDevice = (NEXUS_3466Device *)pChannel->pDevice;

    if (pDevice->cable.api.close)
        pDevice->cable.api.close(handle);
    if (pDevice->terrestrial.api.close)
        pDevice->terrestrial.api.close(handle);

    NEXUS_Frontend_P_Destroy(handle);
    pDevice->frontendHandle[pChannel->chn_num] = NULL;
    BKNI_Free(pChannel);

}

void NEXUS_Frontend_P_3466_DestroyDevice(void *handle)
{
    NEXUS_3466Device *pDevice = (NEXUS_3466Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3466Device);

    if (pDevice->pGenericDeviceHandle) {
        pDevice->pGenericDeviceHandle->abortThread = true;
    }

    if (pDevice->openSettings.cable.enabled) {
        /* Cable teardown */
        NEXUS_FrontendDevice_Close3466_Cable(pDevice);
        /* End of cable teardown */
    }
    if (pDevice->openSettings.terrestrial.enabled) {
        /* Terrestrial teardown */
        NEXUS_FrontendDevice_Close3466_Terrestrial(pDevice);
        /* End of terrestrial teardown */
    }
    NEXUS_FrontendDevice_P_Uninit_3466_Hab(pDevice);
    if (pDevice->capabilities.channelCapabilities) {
        BKNI_Free(pDevice->capabilities.channelCapabilities);
        pDevice->capabilities.channelCapabilities = NULL;
    }

    BKNI_Free(pDevice->pGenericDeviceHandle);
    pDevice->pGenericDeviceHandle = NULL;

    BLST_S_REMOVE(&g_deviceList, pDevice, NEXUS_3466Device, node);

    BDBG_OBJECT_DESTROY(pDevice, NEXUS_3466Device);
    BKNI_Free(pDevice);
    pDevice = NULL;
}

static NEXUS_Error NEXUS_FrontendDevice_P_3466_Standby(void *handle, const NEXUS_StandbySettings *pSettings)
{

    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3466Device *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_3466Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3466Device);

    if ((pDevice->pGenericDeviceHandle->mode != NEXUS_StandbyMode_eDeepSleep) && (pSettings->mode == NEXUS_StandbyMode_eDeepSleep)) {

        BDBG_MSG(("NEXUS_FrontendDevice_P_3466_Standby: Entering deep sleep..."));

        NEXUS_FrontendDevice_P_3466_S3Standby(pDevice);

    } else if ((pDevice->pGenericDeviceHandle->mode == NEXUS_StandbyMode_eDeepSleep) && (pSettings->mode != NEXUS_StandbyMode_eDeepSleep)) {

        BDBG_MSG(("NEXUS_FrontendDevice_P_3466_Standby: Waking up..."));
        rc = NEXUS_FrontendDevice_P_3466_S3Wakeup(pDevice);
        if (rc) { rc = BERR_TRACE(rc); goto done;}
    }

done:
    return rc;
}

static void NEXUS_FrontendDevice_P_3466_GetCapabilities(void *handle, NEXUS_FrontendDeviceCapabilities *pCapabilities)
{
    NEXUS_3466Device *pDevice = (NEXUS_3466Device *)handle;

    BDBG_ASSERT(handle);
    BDBG_ASSERT(pCapabilities);
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3466Device);

    BKNI_Memset(pCapabilities, 0, sizeof(*pCapabilities));

    pCapabilities->numTuners = pDevice->capabilities.totalTunerChannels;
    BDBG_MSG(("NEXUS_FrontendDevice_P_3466_GetCapabilities: numTuners: %d", pCapabilities->numTuners));

    return;
}

static void NEXUS_FrontendDevice_P_3466_GetTunerCapabilities(void *handle, unsigned tunerIndex, NEXUS_FrontendCapabilities *pCapabilities)
{
    BHAB_ChannelCapability *channelCapability;
    NEXUS_3466Device *pDevice = (NEXUS_3466Device *)handle;
    BSTD_UNUSED(tunerIndex);

    channelCapability = pDevice->capabilities.channelCapabilities;

    BKNI_Memset(pCapabilities, 0, sizeof(*pCapabilities));

    if(channelCapability[tunerIndex].demodCoreType.ads) pCapabilities->qam = true;
    if(channelCapability[tunerIndex].demodCoreType.aob) pCapabilities->outOfBand = true;
    if(channelCapability[tunerIndex].demodCoreType.dvbt) {pCapabilities->ofdm = true;pCapabilities->ofdmModes[NEXUS_FrontendOfdmMode_eDvbt]= true;}
    if(channelCapability[tunerIndex].demodCoreType.dvbt2) {pCapabilities->ofdm = true;pCapabilities->ofdmModes[NEXUS_FrontendOfdmMode_eDvbt2]= true;}
    if(channelCapability[tunerIndex].demodCoreType.dvbc2) {pCapabilities->ofdm = true;pCapabilities->ofdmModes[NEXUS_FrontendOfdmMode_eDvbc2]= true;}
    if(channelCapability[tunerIndex].demodCoreType.isdbt) {pCapabilities->ofdm = true;pCapabilities->ofdmModes[NEXUS_FrontendOfdmMode_eIsdbt]= true;}
    if(channelCapability[tunerIndex].demodCoreType.ifdac) pCapabilities->ifdac = true;

    return;
}

static NEXUS_Error NEXUS_FrontendDevice_P_3466_GetStatus(void *handle, NEXUS_FrontendDeviceStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3466Device *pDevice;
    BHAB_AvsData avsData;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_3466Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3466Device);
    BDBG_ASSERT(NULL != handle);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    pStatus->openPending = pDevice->pGenericDeviceHandle->openPending;
    pStatus->openFailed = pDevice->pGenericDeviceHandle->openFailed;

    if(!(pStatus->openPending) && !(pStatus->openFailed)){
        rc = BHAB_GetAvsData(pDevice->hab, &avsData);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        if(avsData.enabled){
            pStatus->avs.enabled = true;
            pStatus->avs.voltage = avsData.voltage;
            pStatus->temperature = avsData.temperature;
        }
        else
            pStatus->avs.enabled = false;
    }
done:
    return rc;
}

NEXUS_Error NEXUS_Frontend_P_3466_SetMtsifConfig(NEXUS_FrontendHandle frontend)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3466Device *pDevice;
    NEXUS_3466Channel *pChannel;

    pChannel =(NEXUS_3466Channel *)frontend->pDeviceHandle;
    BDBG_ASSERT(NULL != pChannel);
    pDevice = (NEXUS_3466Device *)pChannel->pDevice;

    if (pDevice->pLinkedMtsifDevice) {
        if (NEXUS_FrontendDevice_P_CheckOpen(pDevice->pLinkedMtsifDevice)) {
            return BERR_TRACE(NEXUS_NOT_INITIALIZED);
        }
       BDBG_MSG(("3466 mtsif linked configuration, linked device: %p", (void *)pDevice->pLinkedMtsifDevice));
        if (pDevice->pLinkedMtsifDevice->mtsifConfig.mxt) {
            NEXUS_FrontendDeviceHandle savedDeviceHandle = pDevice->frontendHandle[pChannel->chn_num]->pGenericDeviceHandle;
            pDevice->frontendHandle[pChannel->chn_num]->pGenericDeviceHandle = pDevice->pLinkedMtsifDevice;
            rc = NEXUS_Frontend_P_SetMtsifConfig(pDevice->frontendHandle[pChannel->chn_num]);
            pDevice->frontendHandle[pChannel->chn_num]->pGenericDeviceHandle = savedDeviceHandle;
            if (rc) { return BERR_TRACE(rc); }
        } else {
            BDBG_ERR(("misconfiguration: 3466 has linked device, but mxt is not initialized..."));
        }
    } else {
        rc = NEXUS_Frontend_P_SetMtsifConfig(pDevice->frontendHandle[pChannel->chn_num]);
        if (rc) { return BERR_TRACE(rc); }
    }
    return rc;
}

void NEXUS_Frontend_P_3466_UnsetMtsifConfig(NEXUS_FrontendHandle frontend)
{
    NEXUS_3466Device *pDevice;
    NEXUS_3466Channel *pChannel;

    pChannel =(NEXUS_3466Channel *)frontend->pDeviceHandle;
    BDBG_ASSERT(NULL != pChannel);
    pDevice = (NEXUS_3466Device *)pChannel->pDevice;

    if (pDevice->pLinkedMtsifDevice) {
        if (NEXUS_FrontendDevice_P_CheckOpen(pDevice->pLinkedMtsifDevice)) {
            BERR_TRACE(NEXUS_NOT_INITIALIZED); return;
        }
       BDBG_MSG(("3466 mtsif linked configuration, linked device: %p", (void *)pDevice->pLinkedMtsifDevice));
        if (pDevice->pLinkedMtsifDevice->mtsifConfig.mxt) {
            NEXUS_FrontendDeviceHandle savedDeviceHandle = pDevice->frontendHandle[pChannel->chn_num]->pGenericDeviceHandle;
            pDevice->frontendHandle[pChannel->chn_num]->pGenericDeviceHandle = pDevice->pLinkedMtsifDevice;
            NEXUS_Frontend_P_UnsetMtsifConfig(pDevice->frontendHandle[pChannel->chn_num]);
            pDevice->frontendHandle[pChannel->chn_num]->pGenericDeviceHandle = savedDeviceHandle;
        } else {
            BDBG_ERR(("misconfiguration: 3466 has linked device, but mxt is not initialized..."));
        }
    } else {
        NEXUS_Frontend_P_UnsetMtsifConfig(pDevice->frontendHandle[pChannel->chn_num]);
    }
}

NEXUS_Error NEXUS_Frontend_P_3466_ReapplyTransportSettings(void *handle)
{
#if NEXUS_HAS_MXT
    NEXUS_3466Channel *pChannel = (NEXUS_3466Channel *)handle;
    NEXUS_3466Device *pDevice;
    NEXUS_Error rc;

    pDevice = pChannel->pDevice;
    BDBG_ASSERT(pDevice);
    rc = NEXUS_Frontend_P_3466_SetMtsifConfig(pDevice->frontendHandle[pChannel->chn_num]);
    if (rc) { return BERR_TRACE(rc); }
#else
    BSTD_UNUSED(handle);
#endif

    return NEXUS_SUCCESS;
}

static void NEXUS_Frontend_P_3466_SetUserParameters(NEXUS_FrontendHandle handle, const NEXUS_FrontendUserParameters *pParams)
{
    NEXUS_3466Device *pDevice;
    NEXUS_3466Channel *pChannel;

    BSTD_UNUSED(pParams);

    pChannel =(NEXUS_3466Channel *)handle->pDeviceHandle;
    BDBG_ASSERT(NULL != pChannel);
    pDevice = (NEXUS_3466Device *)pChannel->pDevice;

    pDevice->pGenericDeviceHandle->deviceLink = NEXUS_FrontendDevice_P_3466_Link;

}

static NEXUS_Error NEXUS_FrontendDevice_P_3466_Link(NEXUS_FrontendDeviceHandle parentHandle, NEXUS_FrontendDeviceHandle childHandle, const NEXUS_FrontendDeviceLinkSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BSTD_UNUSED(pSettings);

    if ((parentHandle->familyId == 0x3466) && (childHandle->familyId == 0x3466)) {
        NEXUS_3466Device *pParent;
        NEXUS_3466Device *pChild;
        unsigned i;
        pParent = (NEXUS_3466Device *)parentHandle->pDevice;
        pChild = (NEXUS_3466Device *)childHandle->pDevice;

        BDBG_ERR(("Parent is %s", pParent->frontendHandle[0]->userParameters.isMtsif ? "MTSIF" : "PKT"));
        BDBG_ERR(("Child is %s", pChild->frontendHandle[0]->userParameters.isMtsif ? "MTSIF" : "PKT"));

        if (pChild->frontendHandle[0]->userParameters.isMtsif) {
            NEXUS_3466Device *dummy;
            BDBG_ERR(("Switching parent/child"));
            dummy = pChild; pChild = pParent; pParent = dummy;
        }
        if (pChild->frontendHandle[0]->userParameters.isMtsif)
            goto done; /* nothing to be done, both are MTSIF, we should not have gotten here */

        /* assume Parent is MTSIF, Child is PKT */
        pChild->pLinkedMtsifDevice = parentHandle;
        for (i=0; i < NEXUS_MAX_3466_T_FRONTENDS; i++) {
            if (pChild->frontendHandle[i]) {
                NEXUS_FrontendUserParameters params;
                /* done this way to unpack/repack param1 for mtsif case */
                NEXUS_Frontend_GetUserParameters(pChild->frontendHandle[i], &params);
                params.isMtsif = true;
                NEXUS_Frontend_SetUserParameters(pChild->frontendHandle[i], &params);
            }
        }

    }
done:
    return rc;
}
