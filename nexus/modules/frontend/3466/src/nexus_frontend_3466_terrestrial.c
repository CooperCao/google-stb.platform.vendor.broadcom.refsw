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
*   API name: Frontend 3466
*    APIs to open, close, and setup initial settings for a BCM3466
*    Terrestrial Tuner/Demodulator Device.
*
***************************************************************************/
/* Begin Includes */
#include "nexus_frontend_3466_priv.h"
#include "bchp_3466_leap_host_l1.h"
#include "bhab_3466_priv.h"
#include "bhab_3466_fw.h"
#include "bchp_3466_tm.h"
#if NEXUS_HAS_GPIO
#include "priv/nexus_gpio_priv.h"
#endif
/* End includes */

#if NEXUS_HAS_MXT
#include "bmxt.h"
#include "bmxt_wakeup.h"
#endif

BDBG_MODULE(nexus_frontend_3466_terrestrial);

/* set to 1 to enable L1 interrupt messages */
#define NEXUS_FRONTEND_DEBUG_IRQ 0

static void NEXUS_Frontend_P_3466_ResetStatus(void *handle);
static void NEXUS_FrontendDevice_P_3466_UninstallCallbacks(void *handle);

static NEXUS_Error NEXUS_Frontend_P_3466_Terrestrial_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings);

static void NEXUS_Frontend_P_3466_AsyncStatusCallback_isr(void *pParam)
{
    NEXUS_FrontendHandle frontendHandle = NULL;
    NEXUS_3466Channel *pChannel;
    NEXUS_3466Device *pDevice;
    BDBG_ASSERT(NULL != pParam);
    frontendHandle = *((NEXUS_FrontendHandle *)pParam);
    BDBG_ASSERT(NULL != frontendHandle);
    pChannel = frontendHandle->pDeviceHandle;
    BDBG_ASSERT(NULL != pChannel);
    pDevice = pChannel->pDevice;

    if (pDevice->terrestrial.asyncStatusAppCallback[pChannel->chn_num])
    {
        pDevice->terrestrial.isStatusReady[pChannel->chn_num] = true;
        NEXUS_IsrCallback_Fire_isr(pDevice->terrestrial.asyncStatusAppCallback[pChannel->chn_num]);
    }
}

static void NEXUS_Frontend_P_3466_callback_isr(void *pParam)
{
    NEXUS_FrontendHandle frontendHandle = NULL;
    NEXUS_3466Channel *pChannel;
    NEXUS_3466Device *pDevice;
    BDBG_ASSERT(NULL != pParam);
    frontendHandle = *((NEXUS_FrontendHandle *)pParam);
    BDBG_ASSERT(NULL != frontendHandle);
    pChannel = frontendHandle->pDeviceHandle;
    BDBG_ASSERT(NULL != pChannel);
    pDevice = pChannel->pDevice;

    if(pDevice->terrestrial.acquireInProgress[pChannel->chn_num]){
        pDevice->terrestrial.count[pChannel->chn_num]++;
    }
    if(pDevice->terrestrial.count[pChannel->chn_num] == 2){
        pDevice->terrestrial.acquireInProgress[pChannel->chn_num] = false;
        pDevice->terrestrial.count[pChannel->chn_num] = 0;
    }

    if (pDevice->terrestrial.lockAppCallback[pChannel->chn_num])
    {
        NEXUS_IsrCallback_Fire_isr(pDevice->terrestrial.lockAppCallback[pChannel->chn_num]);
    }
}

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
    if (pDevice->cbHandle) {
        BINT_EnableCallback(pDevice->cbHandle);
        BINT_DestroyCallback(pDevice->cbHandle);
        pDevice->cbHandle = NULL;
    }

    if(pDevice->terrestrial.isrEvent) pDevice->terrestrial.isrEvent = NULL;
    if(pDevice->terrestrial.isrEventCallback) NEXUS_UnregisterEvent(pDevice->terrestrial.isrEventCallback);
    pDevice->terrestrial.isrEventCallback = NULL;
    if(pDevice->hab) {
        rc = BHAB_Close(pDevice->hab);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    pDevice->hab = NULL;

    if (pDevice->leapBuffer) {
        NEXUS_Memory_Free(pDevice->leapBuffer);
        pDevice->leapBuffer = NULL;
    }

done:
    return;
}

NEXUS_Error NEXUS_FrontendDevice_P_Init_3466_Hab(NEXUS_3466Device *pDevice, const NEXUS_FrontendDeviceOpenSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHAB_Settings habSettings;
    uint32_t  familyId;
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
    habSettings.isMtsif = true;

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
    BDBG_ASSERT(regHandle);

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

        BDBG_MSG(("Calling BHAB_Open"));
        rc = BHAB_Open(&habHandle, regHandle, &habSettings);
        BDBG_MSG(("Calling BHAB_Open...Done: hab: %p",(void *)habHandle));
        if (rc) { BERR_TRACE(rc); goto done; }

        pDevice->hab = habHandle;

    /* Get events and register callbacks */
    rc = BHAB_GetInterruptEventHandle(pDevice->hab, &pDevice->terrestrial.isrEvent);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pDevice->terrestrial.isrEventCallback = NEXUS_RegisterEvent(pDevice->terrestrial.isrEvent, NEXUS_Frontend_P_3466_IsrEvent, pDevice->hab);
    if ( NULL == pDevice->terrestrial.isrEventCallback ){rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done; }

    rc = BHAB_ReadRegister(pDevice->hab, BCHP_TM_FAMILY_ID, &familyId);
    if(rc){rc = BERR_TRACE(rc);}

    if (pSettings->loadAP)
    {
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
    return BERR_SUCCESS;

done:
    NEXUS_FrontendDevice_P_Uninit_3466_Hab(pDevice);
    return rc;
}

void NEXUS_FrontendDevice_P_Uninit3466(NEXUS_3466Device *pDevice)
{
    NEXUS_Error rc = BERR_SUCCESS;
    unsigned int i;
    BTNR_PowerSaverSettings pwrSettings;

    for ( i = 0; i < NEXUS_MAX_3466_T_FRONTENDS && NULL != pDevice->terrestrial.tnr[i]; i++) {
        if(pDevice->terrestrial.tnr[i]){
            if(pDevice->terrestrial.isTunerPoweredOn[i]){
                pwrSettings.enable = true;
                rc = BTNR_SetPowerSaver(pDevice->terrestrial.tnr[i], &pwrSettings);
                if(rc){rc = BERR_TRACE(rc);}
            }
            rc = BTNR_Close(pDevice->terrestrial.tnr[i]);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->terrestrial.tnr[i] = NULL;
        }
    }
    for ( i = 0; i < NEXUS_MAX_3466_T_FRONTENDS && NULL != pDevice->terrestrial.ods_chn[i]; i++) {
        if(pDevice->terrestrial.ods_chn[i]){
            BODS_InstallCallback(pDevice->terrestrial.ods_chn[i], BODS_Callback_eLockChange, NULL, (void*)&pDevice->terrestrial.frontendHandle[i]);
            BODS_InstallCallback(pDevice->terrestrial.ods_chn[i], BODS_Callback_eAsyncStatusReady, NULL, (void*)&pDevice->terrestrial.frontendHandle[i]);
        }

        if ( pDevice->terrestrial.lockAppCallback[i] ){
            NEXUS_IsrCallback_Destroy(pDevice->terrestrial.lockAppCallback[i]);
            pDevice->terrestrial.lockAppCallback[i] = NULL;
        }
        if ( pDevice->terrestrial.asyncStatusAppCallback[i] ){
             NEXUS_IsrCallback_Destroy(pDevice->terrestrial.asyncStatusAppCallback[i]);
             pDevice->terrestrial.asyncStatusAppCallback[i] = NULL;
        }
        if(pDevice->terrestrial.ods_chn[i]){
            rc = BODS_CloseChannel(pDevice->terrestrial.ods_chn[i]);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            (pDevice->terrestrial.ods_chn[i]) = NULL;
        }
    }
    if (pDevice->terrestrial.ods) {
        rc = BODS_Close(pDevice->terrestrial.ods);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pDevice->terrestrial.ods = NULL;
    }

#if NEXUS_HAS_MXT
    if (pDevice->pGenericDeviceHandle) {
        if (pDevice->pGenericDeviceHandle->mtsifConfig.mxt) {
            BMXT_Close(pDevice->pGenericDeviceHandle->mtsifConfig.mxt);
            pDevice->pGenericDeviceHandle->mtsifConfig.mxt = NULL;
        }
        BKNI_Memset((void *)&pDevice->pGenericDeviceHandle->mtsifConfig, 0, sizeof(pDevice->pGenericDeviceHandle->mtsifConfig));
    }
#endif
done:
    return;
}

static NEXUS_Error NEXUS_FrontendDevice_P_Init3466(NEXUS_3466Device *pDevice)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned int i;
    BTNR_3466_Settings tnr3466_cfg;
    BODS_Settings odsCfg;
    BODS_ChannelSettings odsChnCfg;
#if NEXUS_HAS_MXT
    BMXT_Settings mxtSettings;
#endif

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

    rc = BODS_3466_GetDefaultSettings( &odsCfg, NULL);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    odsCfg.hGeneric = pDevice->hab;
    rc = BODS_Open(&pDevice->terrestrial.ods, g_pCoreHandles->chp, g_pCoreHandles->reg, g_pCoreHandles->bint, &odsCfg);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    rc = BODS_Init(pDevice->terrestrial.ods);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    rc = BODS_GetChannelDefaultSettings( pDevice->terrestrial.ods, BTNR_Standard_eDvbt, &odsChnCfg);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    for (i=0;i<pDevice->capabilities.totalTunerChannels;i++) {
        odsChnCfg.channelNo = i;
        rc = BODS_OpenChannel( pDevice->terrestrial.ods, &pDevice->terrestrial.ods_chn[i], &odsChnCfg);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    pDevice->terrestrial.maxDvbtChannels=0;
    for(i=0; i<pDevice->capabilities.totalTunerChannels; i++) {
        if(pDevice->capabilities.channelCapabilities[i].demodCoreType.dvbt)
            pDevice->terrestrial.maxDvbtChannels++;
    }

    for (i=0;i<NEXUS_MAX_3466_T_FRONTENDS;i++) {
        rc = BTNR_3466_GetDefaultSettings(&tnr3466_cfg);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        tnr3466_cfg.channelNo = i;
        rc =  BTNR_3466_Open(&pDevice->terrestrial.tnr[i],&tnr3466_cfg, pDevice->hab);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
#if NEXUS_HAS_MXT
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
    mxtSettings.hHab = pDevice->hab;
    rc = BMXT_Open(&pDevice->pGenericDeviceHandle->mtsifConfig.mxt, NULL, NULL, &mxtSettings);
    if (rc!=BERR_SUCCESS) goto done;

    rc = NEXUS_Frontend_P_InitMtsifConfig(&pDevice->pGenericDeviceHandle->mtsifConfig, &mxtSettings);
    if (rc!=BERR_SUCCESS) goto done;
#endif
    return BERR_SUCCESS;

done:
    NEXUS_FrontendDevice_P_Uninit3466(pDevice);
    return rc;
}

static void NEXUS_Frontend_P_3466_UnTune(void *handle)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3466Device *pDevice = (NEXUS_3466Device *)handle;
    NEXUS_3466Channel *pChannel;
    BODS_PowerSaverSettings pwrSettings;

    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3466Channel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_MSG(("Untune: pDevice = 0x%p", (void *)pDevice));
    BDBG_MSG(("Tuner is not powered down for now to decrease channel change time."));

    switch ( pDevice->lastChannel )
    {
    case NEXUS_3466_DVBT2_CHN:
    case NEXUS_3466_DVBT_CHN:
        if(pDevice->terrestrial.isPoweredOn[pChannel->chn_num]) {
            rc = BODS_EnablePowerSaver(pDevice->terrestrial.ods_chn[pChannel->chn_num], &pwrSettings);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->terrestrial.isPoweredOn[pChannel->chn_num] = false;
        }
        break;
    default:
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED);
        break;
    }
done:
    return;
}

static void NEXUS_Frontend_P_3466_Close(NEXUS_FrontendHandle handle)
{
    NEXUS_3466Channel *pChannel;
    NEXUS_3466Device *pDevice;

    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    pChannel = (NEXUS_3466Channel *)handle->pDeviceHandle;

    if (pChannel) {
        pDevice = pChannel->pDevice;
        if(pDevice->terrestrial.isPoweredOn[pChannel->chn_num]) {
            NEXUS_Frontend_P_3466_UnTune(pDevice);
        }
    }

    NEXUS_Frontend_P_Destroy(handle);

    if (pChannel) {
        pDevice->terrestrial.frontendHandle[pChannel->chn_num] = NULL;
        BKNI_Memset(pChannel, 0, sizeof(*pChannel));
        BKNI_Free(pChannel);
    }
}

NEXUS_Error NEXUS_FrontendDevice_P_Init3466_Terrestrial(NEXUS_3466Device *pDevice)
{
    NEXUS_Error rc;
    rc = NEXUS_FrontendDevice_P_Init_3466_Hab(pDevice, &pDevice->openSettings);
    if (!rc)
        rc = NEXUS_FrontendDevice_P_Init3466(pDevice);

    /* restore LNA */
    {
        BHAB_ConfigSettings habConfigSettings;
        rc = BHAB_GetConfigSettings(pDevice->hab, &habConfigSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        habConfigSettings.daisyChain = pDevice->terrestrial.rfDaisyChain;
        habConfigSettings.enableLoopThrough = pDevice->terrestrial.enableRfLoopThrough;

        habConfigSettings.rfInputMode = pDevice->terrestrial.rfInput;
        habConfigSettings.tnrApplication = BHAB_TunerApplication_eTerrestrial;

        rc = BHAB_SetConfigSettings(pDevice->hab, &habConfigSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
done:
    return rc;
}

void NEXUS_FrontendDevice_P_Uninit3466_Terrestrial(NEXUS_3466Device *pDevice)
{
    NEXUS_FrontendDevice_P_3466_UninstallCallbacks(pDevice);
    NEXUS_FrontendDevice_P_Uninit3466(pDevice);
    NEXUS_FrontendDevice_P_Uninit_3466_Hab(pDevice);
}

static NEXUS_FrontendLockStatus  NEXUS_Frontend_P_GetLockStatus(unsigned lockStatus)
{
    switch ( lockStatus )
    {
    /*BODS_LockStatus_eUnlocked == BTHD_LockStatus_eUnlocked*/
    case BODS_LockStatus_eUnlocked:
        return NEXUS_FrontendLockStatus_eUnlocked;
    case BODS_LockStatus_eLocked:
        return NEXUS_FrontendLockStatus_eLocked;
    case BODS_LockStatus_eNoSignal:
        return NEXUS_FrontendLockStatus_eNoSignal;
    default:
        BDBG_WRN(("Unrecognized lock status (%d) ", lockStatus));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return NEXUS_FrontendLockStatus_eUnknown;
    }
}

static NEXUS_Error NEXUS_Frontend_P_3466_GetFastStatus(void *handle, NEXUS_FrontendFastStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3466Channel *pChannel = (NEXUS_3466Channel *)handle;
    NEXUS_3466Device *pDevice;
    unsigned lock;

    pDevice = pChannel->pDevice;
    rc = BODS_GetLockStatus(pDevice->terrestrial.ods_chn[pChannel->chn_num],  &lock);
    if(rc){rc = BERR_TRACE(rc); goto done;}
    pStatus->lockStatus = NEXUS_Frontend_P_GetLockStatus(lock);
    BKNI_EnterCriticalSection();
    pStatus->acquireInProgress = pDevice->terrestrial.acquireInProgress[pChannel->chn_num];
    BKNI_LeaveCriticalSection();
    return BERR_SUCCESS;
done:
    return rc;
}

static void NEXUS_FrontendDevice_P_3466_UninstallCallbacks(void *handle)
{
    NEXUS_3466Channel *pChannel = (NEXUS_3466Channel *)handle;
    NEXUS_3466Device *pDevice;

    pDevice = pChannel->pDevice;
    if(pDevice->terrestrial.lockAppCallback[pChannel->chn_num])NEXUS_IsrCallback_Set(pDevice->terrestrial.lockAppCallback[pChannel->chn_num], NULL);
    if(pDevice->terrestrial.asyncStatusAppCallback[pChannel->chn_num])NEXUS_IsrCallback_Set(pDevice->terrestrial.asyncStatusAppCallback[pChannel->chn_num], NULL);
    return;
}

/* Terrestrial-specific functions */

static BODS_SelectiveAsyncStatusType NEXUS_Frontend_P_3466_t2StatusTypeToOds(NEXUS_FrontendDvbt2StatusType type)
{
    switch (type)
    {
    case NEXUS_FrontendDvbt2StatusType_eFecStatisticsL1Pre:
        return BODS_SelectiveAsyncStatusType_eDvbt2FecStatisticsL1Pre;
    case NEXUS_FrontendDvbt2StatusType_eFecStatisticsL1Post:
        return BODS_SelectiveAsyncStatusType_eDvbt2FecStatisticsL1Post;
    case NEXUS_FrontendDvbt2StatusType_eFecStatisticsPlpA:
        return BODS_SelectiveAsyncStatusType_eDvbt2FecStatisticsPlpA;
    case NEXUS_FrontendDvbt2StatusType_eFecStatisticsPlpB:
        return BODS_SelectiveAsyncStatusType_eDvbt2FecStatisticsPlpB;
    case NEXUS_FrontendDvbt2StatusType_eL1Pre:
        return BODS_SelectiveAsyncStatusType_eDvbt2L1Pre;
    case NEXUS_FrontendDvbt2StatusType_eL1PostConfigurable:
        return BODS_SelectiveAsyncStatusType_eDvbt2L1PostConfigurable;
    case NEXUS_FrontendDvbt2StatusType_eL1PostDynamic:
        return BODS_SelectiveAsyncStatusType_eDvbt2L1PostDynamic;
    case NEXUS_FrontendDvbt2StatusType_eL1Plp:
        return BODS_SelectiveAsyncStatusType_eDvbt2L1Plp;
    case NEXUS_FrontendDvbt2StatusType_eBasic:
        return BODS_SelectiveAsyncStatusType_eDvbt2Short;
    default:
        BDBG_WRN((" Unsupported status type."));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return BODS_SelectiveAsyncStatusType_eDvbt2Short;
    }
}

static NEXUS_Error NEXUS_Frontend_P_3466_RequestDvbt2AsyncStatus(void *handle, NEXUS_FrontendDvbt2StatusType type)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3466Channel *pChannel = (NEXUS_3466Channel *)handle;
    NEXUS_3466Device *pDevice;
    BODS_SelectiveAsyncStatusType statusType;
    BDBG_ASSERT(handle != NULL);
    pDevice = pChannel->pDevice;

    statusType = NEXUS_Frontend_P_3466_t2StatusTypeToOds(type);

    rc = BODS_RequestSelectiveAsyncStatus(pDevice->terrestrial.ods_chn[pChannel->chn_num], statusType);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_3466_GetDvbt2AsyncStatusReady(void *handle, NEXUS_FrontendDvbt2StatusReady *pStatusReady)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    BODS_SelectiveAsyncStatusReadyType readyType;
    NEXUS_3466Channel *pChannel = (NEXUS_3466Channel *)handle;
    NEXUS_3466Device *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = pChannel->pDevice;

    BKNI_Memset(pStatusReady, 0, sizeof(NEXUS_FrontendDvbt2StatusReady));
    BKNI_Memset(&readyType, 0, sizeof(readyType));

    rc = BODS_GetSelectiveAsyncStatusReadyType(pDevice->terrestrial.ods_chn[pChannel->chn_num], &readyType);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pStatusReady->type[NEXUS_FrontendDvbt2StatusType_eFecStatisticsL1Pre] = readyType.dvbt2FecStatisticsL1Pre;
    pStatusReady->type[NEXUS_FrontendDvbt2StatusType_eFecStatisticsL1Post] = readyType.dvbt2FecStatisticsL1Post;
    pStatusReady->type[NEXUS_FrontendDvbt2StatusType_eFecStatisticsPlpA] = readyType.dvbt2FecStatisticsPlpA;
    pStatusReady->type[NEXUS_FrontendDvbt2StatusType_eFecStatisticsPlpB] = readyType.dvbt2FecStatisticsPlpB;
    pStatusReady->type[NEXUS_FrontendDvbt2StatusType_eL1Pre] = readyType.dvbt2L1Pre;
    pStatusReady->type[NEXUS_FrontendDvbt2StatusType_eL1PostConfigurable] = readyType.dvbt2L1PostConfigurable;
    pStatusReady->type[NEXUS_FrontendDvbt2StatusType_eL1PostDynamic] = readyType.dvbt2L1PostDynamic;
    pStatusReady->type[NEXUS_FrontendDvbt2StatusType_eL1Plp] = readyType.dvbt2L1Plp;
    pStatusReady->type[NEXUS_FrontendDvbt2StatusType_eBasic] = readyType.dvbt2ShortStatus;

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_3466_GetDvbt2AsyncFecStatistics(void *handle, NEXUS_FrontendDvbt2StatusType type, NEXUS_FrontendDvbt2FecStatistics *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    BODS_SelectiveAsyncStatusType statusType;
    NEXUS_3466Channel *pChannel = (NEXUS_3466Channel *)handle;
    NEXUS_3466Device *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = pChannel->pDevice;

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    BKNI_Memset(&pDevice->terrestrial.odsStatus, 0, sizeof(pDevice->terrestrial.odsStatus));

    statusType = NEXUS_Frontend_P_3466_t2StatusTypeToOds(type);

    rc = BODS_GetSelectiveAsyncStatus(pDevice->terrestrial.ods_chn[pChannel->chn_num], statusType, &pDevice->terrestrial.odsStatus);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if(statusType != pDevice->terrestrial.odsStatus.type){
        BDBG_ERR(("Requested nexus status type %d does not match the returned pi status type %d.",type, pDevice->terrestrial.odsStatus.type));
        rc = BERR_TRACE(NEXUS_UNKNOWN); goto done;
    }

    switch ( pDevice->terrestrial.odsStatus.type )
    {
    case BODS_SelectiveAsyncStatusType_eDvbt2FecStatisticsL1Pre:
    case BODS_SelectiveAsyncStatusType_eDvbt2FecStatisticsL1Post:
    case BODS_SelectiveAsyncStatusType_eDvbt2FecStatisticsPlpA:
    case BODS_SelectiveAsyncStatusType_eDvbt2FecStatisticsPlpB:
        pStatus->lock = pDevice->terrestrial.odsStatus.status.dvbt2FecStatistics.lock;
        pStatus->snrData = pDevice->terrestrial.odsStatus.status.dvbt2FecStatistics.snrData/256;
        pStatus->ldpcAvgIter = pDevice->terrestrial.odsStatus.status.dvbt2FecStatistics.ldpcAvgIter;
        pStatus->ldpcTotIter = pDevice->terrestrial.odsStatus.status.dvbt2FecStatistics.ldpcTotIter;
        pStatus->ldpcTotFrm = pDevice->terrestrial.odsStatus.status.dvbt2FecStatistics.ldpcTotFrm;
        pStatus->ldpcUncFrm = pDevice->terrestrial.odsStatus.status.dvbt2FecStatistics.ldpcUncFrm;
        pStatus->ldpcBER = pDevice->terrestrial.odsStatus.status.dvbt2FecStatistics.ldpcBER;
        pStatus->bchCorBit = pDevice->terrestrial.odsStatus.status.dvbt2FecStatistics.bchCorBit;
        pStatus->bchTotBlk = pDevice->terrestrial.odsStatus.status.dvbt2FecStatistics.bchTotBlk;
        pStatus->bchClnBlk = pDevice->terrestrial.odsStatus.status.dvbt2FecStatistics.bchClnBlk;
        pStatus->bchCorBlk = pDevice->terrestrial.odsStatus.status.dvbt2FecStatistics.bchCorBlk;
        pStatus->bchUncBlk = pDevice->terrestrial.odsStatus.status.dvbt2FecStatistics.bchUncBlk;
        break;
    default:
        BDBG_ERR((" Unsupported status type."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED);
    }
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_3466_GetDvbt2AsyncL1PreStatus(void *handle, NEXUS_FrontendDvbt2L1PreStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3466Channel *pChannel = (NEXUS_3466Channel *)handle;
    NEXUS_3466Device *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = pChannel->pDevice;

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    BKNI_Memset(&pDevice->terrestrial.odsStatus, 0, sizeof(pDevice->terrestrial.odsStatus));

    rc = BODS_GetSelectiveAsyncStatus(pDevice->terrestrial.ods_chn[pChannel->chn_num], BODS_SelectiveAsyncStatusType_eDvbt2L1Pre, &pDevice->terrestrial.odsStatus);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if(BODS_SelectiveAsyncStatusType_eDvbt2L1Pre != pDevice->terrestrial.odsStatus.type){
        BDBG_ERR(("Requested nexus status type BODS_SelectiveAsyncStatusType_eDvbt2L1Pre does not match the returned pi status type %d.", pDevice->terrestrial.odsStatus.type));
        rc = BERR_TRACE(NEXUS_UNKNOWN); goto done;
    }

    pStatus->streamType = pDevice->terrestrial.odsStatus.status.dvbt2L1Pre.streamType;
    pStatus->bwtExt = pDevice->terrestrial.odsStatus.status.dvbt2L1Pre.bwtExt;
    pStatus->s1 = pDevice->terrestrial.odsStatus.status.dvbt2L1Pre.s1;
    pStatus->s2 = pDevice->terrestrial.odsStatus.status.dvbt2L1Pre.s2;
    pStatus->l1RepetitionFlag = pDevice->terrestrial.odsStatus.status.dvbt2L1Pre.l1RepetitionFlag;
    pStatus->guardInterval= pDevice->terrestrial.odsStatus.status.dvbt2L1Pre.guardInterval;
    pStatus->papr = pDevice->terrestrial.odsStatus.status.dvbt2L1Pre.papr;
    pStatus->l1Mod = pDevice->terrestrial.odsStatus.status.dvbt2L1Pre.l1Modulation;
    pStatus->l1CodeRate = pDevice->terrestrial.odsStatus.status.dvbt2L1Pre.l1CodeRate;
    pStatus->l1FecType = pDevice->terrestrial.odsStatus.status.dvbt2L1Pre.l1FecType;
    pStatus->pilotPattern = pDevice->terrestrial.odsStatus.status.dvbt2L1Pre.pilotPattern;
    pStatus->regenFlag = pDevice->terrestrial.odsStatus.status.dvbt2L1Pre.regenFlag;
    pStatus->l1PostExt = pDevice->terrestrial.odsStatus.status.dvbt2L1Pre.l1PostExt;
    pStatus->numRf = pDevice->terrestrial.odsStatus.status.dvbt2L1Pre.numRf;
    pStatus->currentRfIndex = pDevice->terrestrial.odsStatus.status.dvbt2L1Pre.currentRfIndex;
    pStatus->txIdAvailability = pDevice->terrestrial.odsStatus.status.dvbt2L1Pre.txIdAvailability;
    pStatus->numT2Frames = pDevice->terrestrial.odsStatus.status.dvbt2L1Pre.numT2Frames;
    pStatus->numDataSymbols = pDevice->terrestrial.odsStatus.status.dvbt2L1Pre.numDataSymbols;
    pStatus->cellId = pDevice->terrestrial.odsStatus.status.dvbt2L1Pre.cellId;
    pStatus->networkId = pDevice->terrestrial.odsStatus.status.dvbt2L1Pre.networkId;
    pStatus->t2SystemId = pDevice->terrestrial.odsStatus.status.dvbt2L1Pre.t2SystemId;
    pStatus->l1PostSize = pDevice->terrestrial.odsStatus.status.dvbt2L1Pre.l1PostSize;
    pStatus->l1PostInfoSize = pDevice->terrestrial.odsStatus.status.dvbt2L1Pre.l1PostInfoSize;

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_3466_GetDvbt2AsyncL1PostConfigurableStatus(void *handle, NEXUS_FrontendDvbt2L1PostConfigurableStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3466Channel *pChannel = (NEXUS_3466Channel *)handle;
    NEXUS_3466Device *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = pChannel->pDevice;

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    BKNI_Memset(&pDevice->terrestrial.odsStatus, 0, sizeof(pDevice->terrestrial.odsStatus));

    rc = BODS_GetSelectiveAsyncStatus(pDevice->terrestrial.ods_chn[pChannel->chn_num], BODS_SelectiveAsyncStatusType_eDvbt2L1PostConfigurable, &pDevice->terrestrial.odsStatus);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if(BODS_SelectiveAsyncStatusType_eDvbt2L1PostConfigurable != pDevice->terrestrial.odsStatus.type){
        BDBG_ERR(("Requested nexus status type BODS_SelectiveAsyncStatusType_eDvbt2L1PostConfigurable does not match the returned pi status type %d.", pDevice->terrestrial.odsStatus.type));
        rc = BERR_TRACE(NEXUS_UNKNOWN); goto done;
    }

    pStatus->subSlicesPerFrame = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.subSlicesPerFrame;
    pStatus->numPlp = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.numPlp;
    pStatus->numAux = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.numAux;
    pStatus->fefType = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.fefType;
    pStatus->rfIdx = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.rfIdx;
    pStatus->fefInterval = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.fefInterval;
    pStatus->frequency = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.frequency;
    pStatus->fefLength = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.fefLength;
    pStatus->auxStreamType = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.auxStreamType;
    pStatus->auxPrivateConf = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.auxPrivateConf;

    pStatus->plpA.plpId = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpA.plpId;
    pStatus->plpA.plpGroupId = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpA.plpGroupId;
    pStatus->plpA.plpType = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpA.plpType;
    pStatus->plpA.plpPayloadType = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpA.plpPayloadType;
    pStatus->plpA.ffFlag = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpA.ffFlag;
    pStatus->plpA.firstRfIdx = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpA.firstRfIdx;
    pStatus->plpA.firstFrameIdx = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpA.firstFrameIdx;
    pStatus->plpA.plpCodeRate = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpA.codeRate;
    pStatus->plpA.plpMod = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpA.modulation;
    pStatus->plpA.plpRotation = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpA.plpRotation;
    pStatus->plpA.plpFecType = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpA.plpFecType;
    pStatus->plpA.plpNumBlocksMax = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpA.plpNumBlocksMax;
    pStatus->plpA.frameInterval = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpA.frameInterval;
    pStatus->plpA.timeIlLength = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpA.timeIlLength;
    pStatus->plpA.timeIlType= pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpA.timeIlType;
    pStatus->plpA.inBandFlag = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpA.inBandFlag;

    pStatus->plpB.plpId = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpB.plpId;
    pStatus->plpB.plpGroupId = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpB.plpGroupId;
    pStatus->plpB.plpType = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpB.plpType;
    pStatus->plpB.plpPayloadType = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpB.plpPayloadType;
    pStatus->plpB.ffFlag = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpB.ffFlag;
    pStatus->plpB.firstRfIdx = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpB.firstRfIdx;
    pStatus->plpB.firstFrameIdx = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpB.firstFrameIdx;
    pStatus->plpB.plpCodeRate = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpB.codeRate;
    pStatus->plpB.plpMod = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpB.modulation;
    pStatus->plpB.plpRotation = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpB.plpRotation;
    pStatus->plpB.plpFecType = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpB.plpFecType;
    pStatus->plpB.plpNumBlocksMax = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpB.plpNumBlocksMax;
    pStatus->plpB.frameInterval = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpB.frameInterval;
    pStatus->plpB.timeIlLength = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpB.timeIlLength;
    pStatus->plpB.timeIlType= pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpB.timeIlType;
    pStatus->plpB.inBandFlag = pDevice->terrestrial.odsStatus.status.dvbt2L1PostConfigurable.plpB.inBandFlag;

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_3466_GetDvbt2AsyncPostDynamicStatus(void *handle, NEXUS_FrontendDvbt2L1PostDynamicStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3466Channel *pChannel = (NEXUS_3466Channel *)handle;
    NEXUS_3466Device *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = pChannel->pDevice;

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    BKNI_Memset(&pDevice->terrestrial.odsStatus, 0, sizeof(pDevice->terrestrial.odsStatus));

    rc = BODS_GetSelectiveAsyncStatus(pDevice->terrestrial.ods_chn[pChannel->chn_num], BODS_SelectiveAsyncStatusType_eDvbt2L1PostDynamic, &pDevice->terrestrial.odsStatus);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if(BODS_SelectiveAsyncStatusType_eDvbt2L1PostDynamic != pDevice->terrestrial.odsStatus.type){
        BDBG_ERR(("Requested nexus status type BODS_SelectiveAsyncStatusType_eDvbt2L1PostDynamic does not match the returned pi status type %d.", pDevice->terrestrial.odsStatus.type));
        rc = BERR_TRACE(NEXUS_UNKNOWN); goto done;
    }

    pStatus->frameIdx = pDevice->terrestrial.odsStatus.status.dvbt2L1PostDynamic.frameIdx;
    pStatus->l1ChanlgeCounter = pDevice->terrestrial.odsStatus.status.dvbt2L1PostDynamic.l1ChanlgeCounter;
    pStatus->startRfIdx = pDevice->terrestrial.odsStatus.status.dvbt2L1PostDynamic.startRfIdx;
    pStatus->subSliceInterval = pDevice->terrestrial.odsStatus.status.dvbt2L1PostDynamic.subSliceInterval;
    pStatus->type2Start = pDevice->terrestrial.odsStatus.status.dvbt2L1PostDynamic.type2Start;
    pStatus->auxPrivateDyn_31_0 = pDevice->terrestrial.odsStatus.status.dvbt2L1PostDynamic.auxPrivateDyn_31_0;
    pStatus->auxPrivateDyn_47_32 = pDevice->terrestrial.odsStatus.status.dvbt2L1PostDynamic.auxPrivateDyn_47_32;
    pStatus->plpA.plpId = pDevice->terrestrial.odsStatus.status.dvbt2L1PostDynamic.plpA.plpId;
    pStatus->plpA.plpNumBlocks = pDevice->terrestrial.odsStatus.status.dvbt2L1PostDynamic.plpA.plpNumBlocks;
    pStatus->plpA.plpStart = pDevice->terrestrial.odsStatus.status.dvbt2L1PostDynamic.plpA.plpNumBlocks;
    pStatus->plpB.plpId = pDevice->terrestrial.odsStatus.status.dvbt2L1PostDynamic.plpB.plpId;
    pStatus->plpB.plpNumBlocks = pDevice->terrestrial.odsStatus.status.dvbt2L1PostDynamic.plpB.plpNumBlocks;
    pStatus->plpB.plpStart = pDevice->terrestrial.odsStatus.status.dvbt2L1PostDynamic.plpB.plpNumBlocks;
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_3466_GetDvbt2AsyncL1PlpStatus(void *handle, NEXUS_FrontendDvbt2L1PlpStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    unsigned i=0;
    NEXUS_3466Channel *pChannel = (NEXUS_3466Channel *)handle;
    NEXUS_3466Device *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = pChannel->pDevice;

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    BKNI_Memset(&pDevice->terrestrial.odsStatus, 0, sizeof(pDevice->terrestrial.odsStatus));

    rc = BODS_GetSelectiveAsyncStatus(pDevice->terrestrial.ods_chn[pChannel->chn_num], BODS_SelectiveAsyncStatusType_eDvbt2L1Plp, &pDevice->terrestrial.odsStatus);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if(BODS_SelectiveAsyncStatusType_eDvbt2L1Plp != pDevice->terrestrial.odsStatus.type){
        BDBG_ERR(("Requested nexus status type BODS_SelectiveAsyncStatusType_eDvbt2L1Plp does not match the returned pi status type %d.", pDevice->terrestrial.odsStatus.type));
        rc = BERR_TRACE(NEXUS_UNKNOWN); goto done;
    }

    pStatus->numPlp = pDevice->terrestrial.odsStatus.status.dvbt2L1Plp.numPlp;
    for(i=0; i<pStatus->numPlp; i++) {
        pStatus->plp[i].plpId = pDevice->terrestrial.odsStatus.status.dvbt2L1Plp.plp[i].plpId;
        pStatus->plp[i].plpGroupId = pDevice->terrestrial.odsStatus.status.dvbt2L1Plp.plp[i].plpGroupId;
        pStatus->plp[i].plpPayloadType = pDevice->terrestrial.odsStatus.status.dvbt2L1Plp.plp[i].plpPayloadType;
        pStatus->plp[i].plpType = pDevice->terrestrial.odsStatus.status.dvbt2L1Plp.plp[i].plpType;
    }
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_3466_GetDvbt2AsyncBasicStatus(void *handle, NEXUS_FrontendDvbt2BasicStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3466Channel *pChannel = (NEXUS_3466Channel *)handle;
    NEXUS_3466Device *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = pChannel->pDevice;

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    BKNI_Memset(&pDevice->terrestrial.odsStatus, 0, sizeof(pDevice->terrestrial.odsStatus));

    rc = BODS_GetSelectiveAsyncStatus(pDevice->terrestrial.ods_chn[pChannel->chn_num], BODS_SelectiveAsyncStatusType_eDvbt2Short, &pDevice->terrestrial.odsStatus);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if(BODS_SelectiveAsyncStatusType_eDvbt2Short != pDevice->terrestrial.odsStatus.type){
        BDBG_ERR(("Requested nexus status type BODS_SelectiveAsyncStatusType_eDvbt2Short does not match the returned pi status type %d.", pDevice->terrestrial.odsStatus.type));
        rc = BERR_TRACE(NEXUS_UNKNOWN); goto done;
    }

    pStatus->fecLock = pDevice->terrestrial.odsStatus.status.dvbt2Short.lock;
    pStatus->spectrumInverted = pDevice->terrestrial.odsStatus.status.dvbt2Short.spectrumInverted;
    pStatus->snr = pDevice->terrestrial.odsStatus.status.dvbt2Short.snrEstimate*100/256;
    pStatus->gainOffset = pDevice->terrestrial.odsStatus.status.dvbt2Short.gainOffset*100/256;
    pStatus->carrierOffset = pDevice->terrestrial.odsStatus.status.dvbt2Short.carrierFreqOffset;
    pStatus->timingOffset = pDevice->terrestrial.odsStatus.status.dvbt2Short.timingOffset;
    pStatus->signalStrength = pDevice->terrestrial.odsStatus.status.dvbt2Short.signalStrength/10;
    pStatus->signalLevelPercent = pDevice->terrestrial.odsStatus.status.dvbt2Short.signalLevelPercent;
    pStatus->signalQualityPercent = pDevice->terrestrial.odsStatus.status.dvbt2Short.signalQualityPercent;
    pStatus->reacquireCount = pDevice->terrestrial.odsStatus.status.dvbt2Short.reacqCount;
    pStatus->profile = pDevice->terrestrial.odsStatus.status.dvbt2Short.profile;

done:
    return rc;
}

static void NEXUS_Frontend_P_PrintOfdmStatus(NEXUS_FrontendOfdmStatus *pStatus)
{
    BDBG_MSG(("pStatus->modulation = %d", pStatus->modulation));
    BDBG_MSG(("pStatus->receiverLock = %d",pStatus->receiverLock));
    BDBG_MSG(("pStatus->fecLock = %d",pStatus->fecLock));
    BDBG_MSG(("pStatus->spectrumInverted = %d",pStatus->spectrumInverted));
    BDBG_MSG(("pStatus->reacquireCount = %d",pStatus->reacquireCount));
    BDBG_MSG(("pStatus->snr = %d",pStatus->snr));
    BDBG_MSG(("pStatus->carrierOffset = %d",pStatus->carrierOffset));
    BDBG_MSG(("pStatus->timingOffset = %d",pStatus->timingOffset));
    BDBG_MSG(("pStatus->signalStrength = %d",pStatus->signalStrength));
    BDBG_MSG(("pStatus->guardInterval = %d", pStatus->dvbt2Status.l1PreStatus.guardInterval));
    BDBG_MSG(("pStatus->dvbt2Status.gainOffset = %d",pStatus->dvbt2Status.gainOffset));
}

static NEXUS_FrontendOfdmTransmissionMode NEXUS_Frontend_P_OdsToTransmissionMode(BODS_DvbtTransmissionMode mode)
{
    switch ( mode )
    {
    case BODS_DvbtTransmissionMode_e2K:
        return NEXUS_FrontendOfdmTransmissionMode_e2k;
    case BODS_DvbtTransmissionMode_e4K:
        return NEXUS_FrontendOfdmTransmissionMode_e4k;
    case BODS_DvbtTransmissionMode_e8K:
        return NEXUS_FrontendOfdmTransmissionMode_e8k;
    default:
        BDBG_WRN(("Unrecognized transmission mode."));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return BODS_DvbtTransmissionMode_e8K;
    }
}

static NEXUS_FrontendOfdmModulation NEXUS_Frontend_P_THDToModulation(BODS_DvbtModulation modulation)
{
    switch ( modulation )
    {
    case BODS_DvbtModulation_eQpsk:
        return NEXUS_FrontendOfdmModulation_eQpsk;
    case BODS_DvbtModulation_e16Qam:
        return NEXUS_FrontendOfdmModulation_eQam16;
    case BODS_DvbtModulation_e64Qam:
        return NEXUS_FrontendOfdmModulation_eQam64;
    default:
        BDBG_WRN(("Unrecognized modulation mode (%d) reported by BODS", modulation));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return NEXUS_FrontendOfdmModulation_eQam64;
    }
}
static NEXUS_FrontendOfdmCodeRate NEXUS_Frontend_P_THDToCodeRate(BODS_DvbtCodeRate codeRate)
{
    switch ( codeRate )
    {
    case BODS_DvbtCodeRate_e1_2:
        return NEXUS_FrontendOfdmCodeRate_e1_2;
    case BODS_DvbtCodeRate_e2_3:
        return NEXUS_FrontendOfdmCodeRate_e2_3;
    case BODS_DvbtCodeRate_e3_4:
        return NEXUS_FrontendOfdmCodeRate_e3_4;
    case BODS_DvbtCodeRate_e5_6:
        return NEXUS_FrontendOfdmCodeRate_e5_6;
    case BODS_DvbtCodeRate_e7_8:
        return NEXUS_FrontendOfdmCodeRate_e7_8;
    default:
        BDBG_WRN(("Unrecognized codeRate (%d) reported by BODS", codeRate));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return NEXUS_FrontendOfdmCodeRate_e1_2;
    }
}

static NEXUS_FrontendOfdmGuardInterval NEXUS_Frontend_P_OdsToGuardInterval(BODS_DvbtGuardInterval guard)
{
    switch ( guard )
    {
    case BODS_DvbtGuardInterval_e1_4:
        return NEXUS_FrontendOfdmGuardInterval_e1_4;
    case BODS_DvbtGuardInterval_e1_8:
        return NEXUS_FrontendOfdmGuardInterval_e1_8;
    case BODS_DvbtGuardInterval_e1_16:
        return NEXUS_FrontendOfdmGuardInterval_e1_16;
    case BODS_DvbtGuardInterval_e1_32:
        return NEXUS_FrontendOfdmGuardInterval_e1_32;
    default:
        BDBG_WRN(("Unrecognized guard interval (%d) reported by BODS", guard));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return NEXUS_FrontendOfdmGuardInterval_e1_4;
    }
}

static NEXUS_FrontendOfdmHierarchy NEXUS_Frontend_P_THDToHierarchy(BODS_DvbtHierarchy magnum)
{
    switch ( magnum )
    {
    case BODS_DvbtHierarchy_e0:
        return NEXUS_FrontendOfdmHierarchy_e0;
    case BODS_DvbtHierarchy_e1:
        return NEXUS_FrontendOfdmHierarchy_e1;
    case BODS_DvbtHierarchy_e2:
        return NEXUS_FrontendOfdmHierarchy_e2;
    case BODS_DvbtHierarchy_e4:
        return NEXUS_FrontendOfdmHierarchy_e4;
    default:
        BDBG_WRN(("Unrecognized hierarchy (%d) reported by BODS", magnum));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return NEXUS_FrontendOfdmHierarchy_e0;
    }
}


static NEXUS_Error NEXUS_Frontend_P_3466_GetOfdmAsyncStatus(void *handle, NEXUS_FrontendOfdmStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_FrontendDvbt2StatusReady t2StatusReady;
    NEXUS_3466Channel *pChannel = (NEXUS_3466Channel *)handle;
    NEXUS_3466Device *pDevice;
    unsigned chn_num;
    NEXUS_FrontendDvbt2BasicStatus basic;

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    pDevice = pChannel->pDevice;

    switch ( pDevice->lastChannel )
    {
    case NEXUS_3466_DVBT2_CHN:
        rc =  NEXUS_Frontend_P_3466_GetDvbt2AsyncStatusReady(pDevice, &t2StatusReady);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        if((t2StatusReady.type[NEXUS_FrontendDvbt2StatusType_eBasic]) && (t2StatusReady.type[NEXUS_FrontendDvbt2StatusType_eFecStatisticsPlpA]) &&
           (t2StatusReady.type[NEXUS_FrontendDvbt2StatusType_eL1Pre]) && (t2StatusReady.type[NEXUS_FrontendDvbt2StatusType_eL1PostConfigurable]))
        {
            rc = NEXUS_Frontend_P_3466_GetDvbt2AsyncBasicStatus(handle, &basic);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pStatus->fecLock = basic.fecLock;
            pStatus->receiverLock = basic.fecLock;
            pStatus->spectrumInverted = basic.spectrumInverted;
            pStatus->snr = basic.snr;
            pStatus->dvbt2Status.gainOffset = basic.gainOffset;
            pStatus->carrierOffset = basic.carrierOffset;
            pStatus->timingOffset = basic.timingOffset;
            pStatus->signalStrength = basic.signalStrength;
            pStatus->signalLevelPercent = basic.signalLevelPercent;
            pStatus->signalQualityPercent = basic.signalQualityPercent;
            pStatus->reacquireCount = basic.reacquireCount;

            rc = NEXUS_Frontend_P_3466_GetDvbt2AsyncFecStatistics(handle, NEXUS_FrontendDvbt2StatusType_eFecStatisticsPlpA, &pStatus->dvbt2Status.plpAStatistics);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            rc = NEXUS_Frontend_P_3466_GetDvbt2AsyncL1PreStatus(handle, &pStatus->dvbt2Status.l1PreStatus);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            rc = NEXUS_Frontend_P_3466_GetDvbt2AsyncL1PostConfigurableStatus(handle, &pStatus->dvbt2Status.l1PostCfgStatus);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            rc = NEXUS_Frontend_P_3466_GetDvbt2AsyncL1PlpStatus(handle, &pStatus->dvbt2Status.l1PlpStatus);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            NEXUS_Frontend_P_PrintOfdmStatus(pStatus);
        }
        else{
            BDBG_ERR(("Status not ready. Eror reading status."));
            rc = BERR_TRACE(rc); goto done;
        }

        break;
    case NEXUS_3466_DVBT_CHN:
        BKNI_Memset(&pDevice->terrestrial.odsStatus, 0, sizeof(pDevice->terrestrial.odsStatus));

        rc = BODS_GetSelectiveAsyncStatus(pDevice->terrestrial.ods_chn[NEXUS_3466_DVBT_CHN], BODS_SelectiveAsyncStatusType_eDvbt, &pDevice->terrestrial.odsStatus);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        chn_num = NEXUS_3466_DVBT_CHN;
        pStatus->receiverLock = pDevice->terrestrial.odsStatus.status.dvbt.receiverLock;
        pStatus->fecLock = pDevice->terrestrial.odsStatus.status.dvbt.fecLock;
        pStatus->noSignalDetected = pDevice->terrestrial.odsStatus.status.dvbt.noSignalDetected;
        pStatus->transmissionMode = NEXUS_Frontend_P_OdsToTransmissionMode(pDevice->terrestrial.odsStatus.status.dvbt.transmissionMode);
        pStatus->guardInterval = NEXUS_Frontend_P_OdsToGuardInterval(pDevice->terrestrial.odsStatus.status.dvbt.guardInterval);
        pStatus->signalStrength = pDevice->terrestrial.odsStatus.status.dvbt.signalStrength/10;
        pStatus->signalLevelPercent = pDevice->terrestrial.odsStatus.status.dvbt.signalLevelPercent;
        pStatus->signalQualityPercent = pDevice->terrestrial.odsStatus.status.dvbt.signalQualityPercent;
        pStatus->carrierOffset = pDevice->terrestrial.odsStatus.status.dvbt.carrierOffset;
        pStatus->timingOffset = pDevice->terrestrial.odsStatus.status.dvbt.timingOffset;
        pStatus->snr = pDevice->terrestrial.odsStatus.status.dvbt.snr*100/256;
        pStatus->spectrumInverted = pDevice->terrestrial.odsStatus.status.dvbt.spectrumInverted;
        pStatus->reacquireCount = pDevice->terrestrial.odsStatus.status.dvbt.reacqCount;
        pStatus->modulation = NEXUS_Frontend_P_THDToModulation(pDevice->terrestrial.odsStatus.status.dvbt.modulation);
        pStatus->codeRate = NEXUS_Frontend_P_THDToCodeRate(pDevice->terrestrial.odsStatus.status.dvbt.codeRate);
        pStatus->hierarchy = NEXUS_Frontend_P_THDToHierarchy(pDevice->terrestrial.odsStatus.status.dvbt.hierarchy);
        pStatus->cellId = pDevice->terrestrial.odsStatus.status.dvbt.cellId;
        pStatus->fecCorrectedBlocks = pDevice->terrestrial.odsStatus.status.dvbt.rsCorrectedBlocks;
        pStatus->fecUncorrectedBlocks = pDevice->terrestrial.odsStatus.status.dvbt.rsUncorrectedBlocks;
        pStatus->fecCleanBlocks = pDevice->terrestrial.odsStatus.status.dvbt.rsCleanBlocks;
        pStatus->reacquireCount = pDevice->terrestrial.odsStatus.status.dvbt.reacqCount;
        pStatus->viterbiErrorRate = pDevice->terrestrial.odsStatus.status.dvbt.viterbiBer;
        pStatus->preViterbiErrorRate = pDevice->terrestrial.odsStatus.status.dvbt.preViterbiBer;
        break;
    default:
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }

    pStatus->settings = pDevice->terrestrial.last_ofdm[0];

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_3466_RequestOfdmAsyncStatus(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3466Channel *pChannel = (NEXUS_3466Channel *)handle;
    NEXUS_3466Device *pDevice;

    pDevice = pChannel->pDevice;

    if ( pDevice->lastChannel < pDevice->capabilities.totalTunerChannels )
    {
        rc = BODS_RequestSelectiveAsyncStatus(pDevice->terrestrial.ods_chn[pDevice->lastChannel], BODS_SelectiveAsyncStatusType_eDvbt);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    else
    {
        rc = NEXUS_Frontend_P_3466_RequestDvbt2AsyncStatus(pDevice, NEXUS_FrontendDvbt2StatusType_eBasic);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = NEXUS_Frontend_P_3466_RequestDvbt2AsyncStatus(pDevice, NEXUS_FrontendDvbt2StatusType_eFecStatisticsPlpA);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = NEXUS_Frontend_P_3466_RequestDvbt2AsyncStatus(pDevice, NEXUS_FrontendDvbt2StatusType_eL1Pre);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = NEXUS_Frontend_P_3466_RequestDvbt2AsyncStatus(pDevice, NEXUS_FrontendDvbt2StatusType_eL1PostConfigurable);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
done:
    return rc;
}

NEXUS_Error NEXUS_Tuner_P_3466_GetNSetGain(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_GainParameters params;
    NEXUS_InternalGainSettings settings;
    NEXUS_ExternalGainSettings externalGainSettings;
    NEXUS_3466Channel *pChannel = (NEXUS_3466Channel *)handle;
    NEXUS_3466Device *pDevice;

    BKNI_Memset(&externalGainSettings, 0, sizeof(externalGainSettings));

    pDevice = pChannel->pDevice;
    if(pDevice->pGenericDeviceHandle->parent){

        params.rfInput = pDevice->pGenericDeviceHandle->linkSettings.rfInput;
        params.accumulateTillRootDevice = true;
        params.frequency = pDevice->terrestrial.last_ofdm[0].frequency;

        BKNI_Memset(&settings, 0, sizeof(settings));

        rc = NEXUS_FrontendDevice_P_GetInternalGain(pDevice->pGenericDeviceHandle->parent, &params, &settings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        externalGainSettings.totalGain = settings.totalVariableGain;

        if(settings.isExternalFixedGainBypassed){
            externalGainSettings.totalGain -= pDevice->pGenericDeviceHandle->bypassableFixedGain;
        }
    }

    externalGainSettings.bypassableGain = pDevice->pGenericDeviceHandle->bypassableFixedGain;
    externalGainSettings.totalGain += pDevice->pGenericDeviceHandle->totalFixedBoardGain;

    rc = NEXUS_FrontendDevice_P_SetExternalGain(pDevice->pGenericDeviceHandle, &externalGainSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}

static NEXUS_FrontendOfdmModulation NEXUS_Frontend_P_ModulationToDvbt(NEXUS_FrontendOfdmModulation modulation)
{
    switch ( modulation )
    {
    case NEXUS_FrontendOfdmModulation_eQpsk:
        return BODS_DvbtModulation_eQpsk;
    case NEXUS_FrontendOfdmModulation_eQam16:
        return BODS_DvbtModulation_e16Qam;
    case NEXUS_FrontendOfdmModulation_eQam64:
        return BODS_DvbtModulation_e64Qam;
    default:
        BDBG_WRN(("Unrecognized modulation (%d)", modulation));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return BODS_DvbtModulation_eQpsk;
    }
}

static NEXUS_FrontendOfdmModulation NEXUS_Frontend_P_CodeRateToDvbt(NEXUS_FrontendOfdmCodeRate codeRate)
{
    switch ( codeRate )
    {
    case NEXUS_FrontendOfdmCodeRate_e1_2:
        return BODS_DvbtCodeRate_e1_2;
    case NEXUS_FrontendOfdmCodeRate_e2_3:
        return BODS_DvbtCodeRate_e2_3;
    case NEXUS_FrontendOfdmCodeRate_e3_4:
        return BODS_DvbtCodeRate_e3_4;
    case NEXUS_FrontendOfdmCodeRate_e5_6:
        return BODS_DvbtCodeRate_e5_6;
    case NEXUS_FrontendOfdmCodeRate_e7_8:
        return BODS_DvbtCodeRate_e7_8;
    default:
        BDBG_WRN(("Unrecognized code rate (%d)", codeRate));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return BODS_DvbtCodeRate_e1_2;
    }
}

static NEXUS_FrontendOfdmModulation NEXUS_Frontend_P_HierarchyToDvbt(NEXUS_FrontendOfdmHierarchy hierarchy)
{
    switch ( hierarchy )
    {
    case NEXUS_FrontendOfdmHierarchy_e0:
        return BODS_DvbtHierarchy_e0;
    case NEXUS_FrontendOfdmHierarchy_e1:
        return BODS_DvbtHierarchy_e1;
    case NEXUS_FrontendOfdmHierarchy_e2:
        return BODS_DvbtHierarchy_e2;
    case NEXUS_FrontendOfdmHierarchy_e4:
        return BODS_DvbtHierarchy_e4;
    default:
        BDBG_WRN(("Unrecognized hierarchy (%d)", hierarchy));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return BODS_DvbtHierarchy_e0;
    }
}

static NEXUS_Error NEXUS_Frontend_P_3466_RequestDvbtAsyncStatus(void *handle, NEXUS_FrontendDvbtStatusType type)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3466Channel *pChannel = (NEXUS_3466Channel *)handle;
    NEXUS_3466Device *pDevice;
    BSTD_UNUSED(type);

    pDevice = pChannel->pDevice;
    pDevice->terrestrial.isStatusReady[pChannel->chn_num] = false;
    rc = BODS_RequestSelectiveAsyncStatus(pDevice->terrestrial.ods_chn[pChannel->chn_num], BODS_SelectiveAsyncStatusType_eDvbt);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_3466_GetDvbtAsyncStatusReady(void *handle, NEXUS_FrontendDvbtStatusReady *pAsyncStatusReady)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3466Channel *pChannel = (NEXUS_3466Channel *)handle;
    NEXUS_3466Device *pDevice;

    pDevice = pChannel->pDevice;
    if(pDevice->terrestrial.isStatusReady[pChannel->chn_num]){
        pAsyncStatusReady->type[NEXUS_FrontendDvbtStatusType_eBasic] = true;
    }
    else{
        pAsyncStatusReady->type[NEXUS_FrontendDvbtStatusType_eBasic] = false;
    }

    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_3466_GetDvbtAsyncStatus(void *handle, NEXUS_FrontendDvbtStatusType type, NEXUS_FrontendDvbtStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3466Channel *pChannel = (NEXUS_3466Channel *)handle;
    NEXUS_3466Device *pDevice;

    pDevice = pChannel->pDevice;
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    BKNI_Memset(&pDevice->terrestrial.odsStatus, 0, sizeof(pDevice->terrestrial.odsStatus));

    if(type == NEXUS_FrontendDvbtStatusType_eBasic){
        rc = BODS_GetSelectiveAsyncStatus(pDevice->terrestrial.ods_chn[pChannel->chn_num], BODS_SelectiveAsyncStatusType_eDvbt, &pDevice->terrestrial.odsStatus);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        pStatus->status.basic.fecLock = pDevice->terrestrial.odsStatus.status.dvbt.fecLock;
        pStatus->status.basic.spectrumInverted = pDevice->terrestrial.odsStatus.status.dvbt.spectrumInverted;
        pStatus->status.basic.snr = pDevice->terrestrial.odsStatus.status.dvbt.snr*100/256;
        pStatus->status.basic.carrierOffset = pDevice->terrestrial.odsStatus.status.dvbt.carrierOffset;
        pStatus->status.basic.timingOffset = pDevice->terrestrial.odsStatus.status.dvbt.timingOffset;
        pStatus->status.basic.gainOffset = pDevice->terrestrial.odsStatus.status.dvbt.gainOffset;
        pStatus->status.basic.signalStrength = pDevice->terrestrial.odsStatus.status.dvbt.signalStrength/10;
        pStatus->status.basic.signalLevelPercent = pDevice->terrestrial.odsStatus.status.dvbt.signalLevelPercent;
        pStatus->status.basic.signalQualityPercent = pDevice->terrestrial.odsStatus.status.dvbt.signalQualityPercent;
        pStatus->status.basic.reacquireCount = pDevice->terrestrial.odsStatus.status.dvbt.reacqCount;
        pStatus->status.basic.viterbiErrorRate.rate = pDevice->terrestrial.odsStatus.status.dvbt.viterbiBer;

        pStatus->status.basic.tps.modulation = NEXUS_Frontend_P_THDToModulation(pDevice->terrestrial.odsStatus.status.dvbt.modulation);
        pStatus->status.basic.tps.transmissionMode = NEXUS_Frontend_P_OdsToTransmissionMode(pDevice->terrestrial.odsStatus.status.dvbt.transmissionMode);
        pStatus->status.basic.tps.guardInterval = NEXUS_Frontend_P_OdsToGuardInterval(pDevice->terrestrial.odsStatus.status.dvbt.guardInterval);
        pStatus->status.basic.tps.codeRate = NEXUS_Frontend_P_THDToCodeRate(pDevice->terrestrial.odsStatus.status.dvbt.codeRate);
        pStatus->status.basic.tps.hierarchy = NEXUS_Frontend_P_THDToHierarchy(pDevice->terrestrial.odsStatus.status.dvbt.hierarchy);
        pStatus->status.basic.tps.cellId = pDevice->terrestrial.odsStatus.status.dvbt.cellId;
        pStatus->status.basic.tps.inDepthSymbolInterleave = pDevice->terrestrial.odsStatus.status.dvbt.inDepthSymbolInterleave;
        pStatus->status.basic.tps.timeSlicing = pDevice->terrestrial.odsStatus.status.dvbt.timeSlicing;
        pStatus->status.basic.tps.mpeFec = pDevice->terrestrial.odsStatus.status.dvbt.mpeFec;

        pStatus->status.basic.fecBlockCounts.corrected = pDevice->terrestrial.odsStatus.status.dvbt.rsCorrectedBlocks;
        pStatus->status.basic.fecBlockCounts.uncorrected = pDevice->terrestrial.odsStatus.status.dvbt.rsUncorrectedBlocks;
        pStatus->status.basic.fecBlockCounts.clean = pDevice->terrestrial.odsStatus.status.dvbt.rsCleanBlocks;
    }

    pStatus->status.basic.settings = pDevice->terrestrial.last_ofdm[0];
    pDevice->terrestrial.isStatusReady[pChannel->chn_num] = false;
done:
    return rc;

}


static NEXUS_Error NEXUS_Frontend_P_3466_TuneOfdm(void *handle, const NEXUS_FrontendOfdmSettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3466Channel *pChannel = (NEXUS_3466Channel *)handle;
    NEXUS_3466Device *pDevice;
    BODS_AcquireParams odsParam;
    BTNR_Settings tnrSettings;
    unsigned temp_frequency;
    NEXUS_FrontendOfdmMode temp_mode;
    BODS_PowerSaverSettings odsPwrSettings;
    BTNR_PowerSaverSettings tnrPwrSettings;
    BODS_ChannelSettings odsChnCfg;

    pDevice = pChannel->pDevice;

    rc = BODS_GetChannelDefaultSettings( pDevice->terrestrial.ods, BODS_Standard_eDvbt2, &odsChnCfg);
    if(rc){rc = BERR_TRACE(rc); goto done;}
    switch ( pSettings->mode )
    {
    case NEXUS_FrontendOfdmMode_eDvbt:
        odsChnCfg.standard=BODS_Standard_eDvbt;
        break;
    case NEXUS_FrontendOfdmMode_eDvbt2:
        odsChnCfg.standard=BODS_Standard_eDvbt2;
        break;
    default:
        BDBG_ERR(("Wrong Ofdm mode selected."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }

    /* This is done as DVBT and DVBT2 are in a single demod core and need to be powered down*/
    if(pDevice->terrestrial.last_ofdm[pChannel->chn_num].mode != pSettings->mode){
        NEXUS_Frontend_P_3466_UnTune(handle);
        if(pDevice->terrestrial.ods_chn[pChannel->chn_num]){
            rc = BODS_CloseChannel(pDevice->terrestrial.ods_chn[pChannel->chn_num]);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            (pDevice->terrestrial.ods_chn[pChannel->chn_num]) = NULL;
            odsChnCfg.channelNo = pChannel->chn_num;
            rc = BODS_OpenChannel( pDevice->terrestrial.ods, &pDevice->terrestrial.ods_chn[pChannel->chn_num], &odsChnCfg);
            if(rc){rc = BERR_TRACE(rc); goto done;}
        }
    }

    rc = BODS_InstallCallback(pDevice->terrestrial.ods_chn[pChannel->chn_num], BODS_Callback_eLockChange, (BODS_CallbackFunc)NEXUS_Frontend_P_3466_callback_isr, (void*)&pDevice->terrestrial.frontendHandle[pChannel->chn_num]);
    if(rc){rc = BERR_TRACE(rc); goto done;}
    rc = BODS_InstallCallback(pDevice->terrestrial.ods_chn[pChannel->chn_num], BODS_Callback_eAsyncStatusReady, (BODS_CallbackFunc)NEXUS_Frontend_P_3466_AsyncStatusCallback_isr, (void*)&pDevice->terrestrial.frontendHandle[pChannel->chn_num]);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pDevice->terrestrial.frontendHandle[pChannel->chn_num]->mtsif.inputBand = NEXUS_FRONTEND_3466_OFDM_INPUT_BAND+pChannel->chn_num;
    rc = NEXUS_Frontend_P_SetMtsifConfig(pDevice->terrestrial.frontendHandle[pChannel->chn_num]);

    rc = BODS_GetDefaultAcquireParams(pDevice->terrestrial.ods_chn[pChannel->chn_num], &odsParam);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    NEXUS_IsrCallback_Set(pDevice->terrestrial.lockAppCallback[pChannel->chn_num], &(pSettings->lockCallback));
    NEXUS_IsrCallback_Set(pDevice->terrestrial.asyncStatusAppCallback[pChannel->chn_num], &(pSettings->asyncStatusReadyCallback));

    if(!pDevice->terrestrial.isPoweredOn[pChannel->chn_num] || !pDevice->terrestrial.isTunerPoweredOn[pChannel->chn_num])
        goto full_acquire;

    if((pSettings->acquisitionMode == NEXUS_FrontendOfdmAcquisitionMode_eScan) && (pDevice->terrestrial.lastAcquisitionMode[pChannel->chn_num] == NEXUS_FrontendOfdmAcquisitionMode_eScan)){
        temp_frequency = pDevice->terrestrial.last_ofdm[pChannel->chn_num].frequency;
        pDevice->terrestrial.last_ofdm[pChannel->chn_num].frequency = pSettings->frequency ;
        temp_mode = pDevice->terrestrial.last_ofdm[pChannel->chn_num].mode;
        pDevice->terrestrial.last_ofdm[pChannel->chn_num].mode = pSettings->mode;

        if(!BKNI_Memcmp(pSettings, &pDevice->terrestrial.last_ofdm[pChannel->chn_num], sizeof(NEXUS_FrontendOfdmSettings))) {
            if (pDevice->terrestrial.tnr[pChannel->chn_num])
            {
                pDevice->terrestrial.acquireInProgress[pChannel->chn_num] = true;
                pDevice->terrestrial.last_ofdm[pChannel->chn_num] = *pSettings;
                pDevice->lastChannel = pChannel->chn_num;
                pDevice->terrestrial.lastAcquisitionMode[pChannel->chn_num] = pSettings->acquisitionMode;
                rc = BTNR_SetTunerRfFreq(pDevice->terrestrial.tnr[pChannel->chn_num], pSettings->frequency, BTNR_TunerMode_eDigital);
                if(rc){rc = BERR_TRACE(rc); goto retrack;}

                return rc;
            }
        }
    }

full_acquire:
    pDevice->terrestrial.acquireInProgress[pChannel->chn_num] = true;
    pDevice->lastChannel = pChannel->chn_num;
    pDevice->terrestrial.lastAcquisitionMode[pChannel->chn_num] = pSettings->acquisitionMode;

    switch ( pSettings->acquisitionMode )
    {
    case NEXUS_FrontendOfdmAcquisitionMode_eScan:
        odsParam.tuneAcquire = BODS_TuneAcquire_eAcquireAfterTune;
        /* No break as we set the modes to manual for scan.*/
    case NEXUS_FrontendOfdmAcquisitionMode_eAuto:
        /* Due to get default parameters, the odsParam.tuneAcquire and thdParam.bTuneAcquire are set to false. */
        odsParam.acquisitionMode = BODS_AcquisitionMode_eAuto;
        break;
    case NEXUS_FrontendOfdmAcquisitionMode_eManual:
        /* Due to get default parameters, the odsParam.tuneAcquire and thdParam.bTuneAcquire are set to false. */
        odsParam.acquisitionMode = BODS_AcquisitionMode_eManual;
        break;
    default:
        BDBG_ERR((" Unsupported Acquisition mode."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }

    odsParam.spectrumMode = (BODS_SpectrumMode)pSettings->spectrumMode;
    odsParam.invertSpectrum = (BODS_InvertSpectrum)pSettings->spectralInversion;
    if(pSettings->spectrum){
        odsParam.spectrumMode = (BODS_SpectrumMode)BODS_SpectrumMode_eManual;
        switch ( pSettings->spectrumMode )
        {
        case NEXUS_FrontendOfdmSpectrum_eNonInverted:
            odsParam.invertSpectrum = BODS_InvertSpectrum_eNormal;
            break;
        case NEXUS_FrontendOfdmSpectrum_eInverted:
            odsParam.invertSpectrum = BODS_InvertSpectrum_eInverted;
            break;
        default:
            BDBG_ERR((" Unsupported spectrum inversion mode."));
            rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
        }
    }

    switch ( pSettings->mode )
    {
    case NEXUS_FrontendOfdmMode_eDvbt2:
        switch ( pSettings->bandwidth )
        {
        case 1700000:
            odsParam.acquireParams.dvbt2.bandwidth = BODS_Dvbt2Bandwidth_e1p7MHz;
            break;
        case 5000000:
            odsParam.acquireParams.dvbt2.bandwidth = BODS_Dvbt2Bandwidth_e5MHz;
            break;
        case 6000000:
            odsParam.acquireParams.dvbt2.bandwidth = BODS_Dvbt2Bandwidth_e6MHz;
            break;
        case 7000000:
            odsParam.acquireParams.dvbt2.bandwidth = BODS_Dvbt2Bandwidth_e7MHz;
            break;
        case 8000000:
            odsParam.acquireParams.dvbt2.bandwidth = BODS_Dvbt2Bandwidth_e8MHz;
            break;
        default:
            BDBG_ERR((" Unsupported bandwidth."));
            rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
        }
        odsParam.acquireParams.dvbt2.plpMode = pSettings->dvbt2Settings.plpMode;
        if(!pSettings->dvbt2Settings.plpMode)
            odsParam.acquireParams.dvbt2.plpId = pSettings->dvbt2Settings.plpId;
        switch ( pSettings->dvbt2Settings.profile )
        {
        case NEXUS_FrontendDvbt2Profile_eBase:
            odsParam.acquireParams.dvbt2.profile= BODS_Dvbt2Profile_eBase;
            break;
        case NEXUS_FrontendDvbt2Profile_eLite:
            odsParam.acquireParams.dvbt2.profile = BODS_Dvbt2Profile_eLite;
            break;
        default:
            BDBG_ERR((" Unsupported profile."));
            rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
        }
        break;
    case NEXUS_FrontendOfdmMode_eDvbt:
        switch ( pSettings->bandwidth )
        {
        case 1700000:
            odsParam.acquireParams.dvbt.bandwidth = BODS_DvbtBandwidth_e1p7MHz;
            break;
        case 5000000:
            odsParam.acquireParams.dvbt.bandwidth = BODS_DvbtBandwidth_e5MHz;
            break;
        case 6000000:
            odsParam.acquireParams.dvbt.bandwidth = BODS_DvbtBandwidth_e6MHz;
            break;
        case 7000000:
            odsParam.acquireParams.dvbt.bandwidth = BODS_DvbtBandwidth_e7MHz;
            break;
        case 8000000:
            odsParam.acquireParams.dvbt.bandwidth = BODS_DvbtBandwidth_e8MHz;
            break;
        default:
            BDBG_ERR((" Unsupported bandwidth."));
            rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
        }
        odsParam.acquireParams.dvbt.carrierRange = pSettings->pullInRange;
        odsParam.acquireParams.dvbt.cciMode = (BODS_DvbtCciMode)pSettings->cciMode;
        /* Nexus to PI, there is reversal in logic. */
        if(pSettings->manualTpsSettings) {
            odsParam.acquireParams.dvbt.tpsMode = BODS_DvbtTpsMode_eManual;
            odsParam.acquireParams.dvbt.codeRateHighPriority = NEXUS_Frontend_P_CodeRateToDvbt(pSettings->tpsSettings.highPriorityCodeRate);
            odsParam.acquireParams.dvbt.codeRateLowPriority = NEXUS_Frontend_P_CodeRateToDvbt(pSettings->tpsSettings.lowPriorityCodeRate);
            odsParam.acquireParams.dvbt.hierarchy = NEXUS_Frontend_P_HierarchyToDvbt(pSettings->tpsSettings.hierarchy);
            odsParam.acquireParams.dvbt.modulation = NEXUS_Frontend_P_ModulationToDvbt(pSettings->tpsSettings.modulation);
        }
        else
            odsParam.acquireParams.dvbt.tpsMode = BODS_DvbtTpsMode_eAuto;
        if(pSettings->manualModeSettings){
            odsParam.acquireParams.dvbt.transGuardMode = BODS_DvbtOfdmMode_eManual;
            odsParam.acquireParams.dvbt.guardInterval = pSettings->modeSettings.guardInterval;
            if((pSettings->modeSettings.mode>NEXUS_FrontendOfdmTransmissionMode_e1k) && (pSettings->modeSettings.mode<NEXUS_FrontendOfdmTransmissionMode_e16k))
                odsParam.acquireParams.dvbt.transmissionMode = pSettings->modeSettings.mode;
            else {
                BDBG_ERR((" Unsupported DVBT Transmission mode."));
                rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
            }

        }
        else
            odsParam.acquireParams.dvbt.transGuardMode = BODS_DvbtOfdmMode_eAuto;
        if(pSettings->priority == NEXUS_FrontendOfdmPriority_eHigh)
            odsParam.acquireParams.dvbt.decodeMode = BODS_DvbtDecodeMode_eHighPriority;
        else
            odsParam.acquireParams.dvbt.decodeMode = BODS_DvbtDecodeMode_eLowPriority;
        break;
    default:
        BDBG_ERR((" Unsupported mode."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
        break;
    }

    if (pDevice->terrestrial.tnr[pChannel->chn_num])
    {
        if(!pDevice->terrestrial.isTunerPoweredOn[pChannel->chn_num]){
            tnrPwrSettings.enable = false;
            rc = BTNR_SetPowerSaver(pDevice->terrestrial.tnr[pChannel->chn_num], &tnrPwrSettings);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->terrestrial.isTunerPoweredOn[pChannel->chn_num] = true;
        }
        if(!pDevice->terrestrial.isPoweredOn[pChannel->chn_num]){
            rc = BODS_DisablePowerSaver(pDevice->terrestrial.ods_chn[pChannel->chn_num], &odsPwrSettings);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->terrestrial.isPoweredOn[pChannel->chn_num] = true;
        }

        rc = BODS_SetAcquireParams( pDevice->terrestrial.ods_chn[pChannel->chn_num], &odsParam );
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = BTNR_GetSettings(pDevice->terrestrial.tnr[pChannel->chn_num], &tnrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        if(pSettings->mode == NEXUS_FrontendOfdmMode_eDvbt2){
            tnrSettings.std = BTNR_Standard_eDvbt2;
        }
        else if(pSettings->mode == NEXUS_FrontendOfdmMode_eDvbt){
            tnrSettings.std = BTNR_Standard_eDvbt;
        }
        tnrSettings.bandwidth = pSettings->bandwidth;

        rc = BTNR_SetSettings(pDevice->terrestrial.tnr[pChannel->chn_num], &tnrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = BTNR_SetTunerRfFreq(pDevice->terrestrial.tnr[pChannel->chn_num], pSettings->frequency, BTNR_TunerMode_eDigital);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    if ( pSettings->acquisitionMode != NEXUS_FrontendOfdmAcquisitionMode_eScan) {
        switch ( pSettings->mode )
        {
        case NEXUS_FrontendOfdmMode_eDvbt2:
        case NEXUS_FrontendOfdmMode_eDvbt:
            BKNI_Sleep(500);
            rc = BODS_Acquire(pDevice->terrestrial.ods_chn[pChannel->chn_num], &odsParam);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            break;
        default:
            break;
        }
    }

    pDevice->terrestrial.last_ofdm[pChannel->chn_num] = *pSettings;
    return BERR_SUCCESS;
retrack:
    pDevice->terrestrial.last_ofdm[pChannel->chn_num].frequency = temp_frequency;
    pDevice->terrestrial.last_ofdm[pChannel->chn_num].mode = temp_mode;
done:
    NEXUS_Frontend_P_3466_UnTune(handle);
    return rc;
}

static void NEXUS_Frontend_P_3466_ResetStatus(void *handle)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3466Channel *pChannel = (NEXUS_3466Channel *)handle;
    NEXUS_3466Device *pDevice;

    pDevice = pChannel->pDevice;
    switch ( pDevice->lastChannel )
    {
    case NEXUS_3466_DVBT2_CHN:
    case NEXUS_3466_DVBT_CHN:
        if(pDevice->terrestrial.isPoweredOn[pDevice->lastChannel]) {
            rc = BODS_ResetStatus(pDevice->terrestrial.ods_chn[pDevice->lastChannel]);
            if(rc){rc = BERR_TRACE(rc); goto done;}
        }
        else{
            BDBG_MSG(("The core is Powered Off"));
        }
        break;
    default:
        BDBG_ERR((" Unsupported channel."));
        BERR_TRACE(BERR_NOT_SUPPORTED);
    }
done:
    return;
}

static NEXUS_Error NEXUS_Frontend_P_3466_ReadSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length, size_t *pNumRead)
{
    size_t i;
    NEXUS_Error rc = NEXUS_SUCCESS;
    int16_t return_length;
    NEXUS_3466Channel *pChannel = (NEXUS_3466Channel *)handle;
    NEXUS_3466Device *pDevice;
    int16_t d_i[TOTAL_SOFTDECISIONS], d_q[TOTAL_SOFTDECISIONS];

    pDevice = pChannel->pDevice;
    BKNI_Memset(pDecisions, 0, (sizeof(NEXUS_FrontendSoftDecision) * length));

    switch ( pDevice->lastChannel )
    {
    case NEXUS_3466_DVBT2_CHN:
    case NEXUS_3466_DVBT_CHN:
            rc = BODS_GetSoftDecision(pDevice->terrestrial.ods_chn[pDevice->lastChannel], (int16_t)TOTAL_SOFTDECISIONS, d_i, d_q, &return_length);
            if(rc){rc = BERR_TRACE(rc); goto done;}
        break;
    default:
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }

    for (i=0; (int)i<return_length && i<length; i++)
    {
        pDecisions[i].i = d_i[i];
        pDecisions[i].q = d_q[i];
    }
    *pNumRead = i;

done:
    return rc;
}
/* End of terrestrial-specific function declarations */

NEXUS_Error NEXUS_Frontend_P_3466_ReapplyTransportSettings(void *handle)
{
#if NEXUS_HAS_MXT
    NEXUS_3466Channel *pChannel = (NEXUS_3466Channel *)handle;
    NEXUS_3466Device *pDevice;
    NEXUS_Error rc;

    pDevice = pChannel->pDevice;
    BDBG_ASSERT(pDevice);
    rc = NEXUS_Frontend_P_SetMtsifConfig(pDevice->terrestrial.frontendHandle[pChannel->chn_num]);
    if (rc) { return BERR_TRACE(rc); }
#else
    BSTD_UNUSED(handle);
#endif

    return NEXUS_SUCCESS;
}

/* Terrestrial frontend device implementation */
NEXUS_Error NEXUS_FrontendDevice_Open3466_Terrestrial(NEXUS_3466Device *pDevice)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    rc = NEXUS_FrontendDevice_P_Init_3466_Hab(pDevice, &pDevice->openSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    /* Initialize THD and T2 channels*/
    rc = NEXUS_FrontendDevice_P_Init3466(pDevice);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}
/* End terrestrial frontend device implementation */

/* Terrestrial implementation */
NEXUS_FrontendHandle NEXUS_Frontend_Open3466_Terrestrial(const NEXUS_FrontendChannelSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_FrontendHandle frontendHandle = NULL;
    NEXUS_3466Device *p3466Device;
    NEXUS_3466Channel *pChannel;
    NEXUS_FrontendDevice *pFrontendDevice = NULL;

    if (pSettings->device == NULL) {
        BDBG_WRN(("No device..."));
        return NULL;
    }

    pFrontendDevice = pSettings->device;
    p3466Device = (NEXUS_3466Device *)pFrontendDevice->pDevice;

    /* If already opened, return the previously opened handle */
    if ( p3466Device->terrestrial.frontendHandle[pSettings->channelNumber] != NULL )
    {
        return p3466Device->terrestrial.frontendHandle[pSettings->channelNumber];
    }

    pChannel = (NEXUS_3466Channel*)BKNI_Malloc(sizeof(*pChannel));
    if ( NULL == pChannel ) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_alloc;}
    BKNI_Memset(pChannel, 0, sizeof(*pChannel));

    /* Create a Nexus frontend handle */
    frontendHandle = NEXUS_Frontend_P_Create(pChannel);
    if ( NULL == frontendHandle) {rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_alloc;}

    /* Establish device capabilities */
    frontendHandle->capabilities.ofdm = true;
    frontendHandle->capabilities.ofdmModes[NEXUS_FrontendOfdmMode_eDvbt] = true;
    frontendHandle->capabilities.ofdmModes[NEXUS_FrontendOfdmMode_eDvbt2] = true;

    frontendHandle->requestDvbtAsyncStatus = NEXUS_Frontend_P_3466_RequestDvbtAsyncStatus;
    frontendHandle->getDvbtAsyncStatusReady = NEXUS_Frontend_P_3466_GetDvbtAsyncStatusReady;
    frontendHandle->getDvbtAsyncStatus = NEXUS_Frontend_P_3466_GetDvbtAsyncStatus;
    frontendHandle->requestDvbt2AsyncStatus = NEXUS_Frontend_P_3466_RequestDvbt2AsyncStatus;
    frontendHandle->getDvbt2AsyncStatusReady = NEXUS_Frontend_P_3466_GetDvbt2AsyncStatusReady;
    frontendHandle->getDvbt2AsyncFecStatistics = NEXUS_Frontend_P_3466_GetDvbt2AsyncFecStatistics;
    frontendHandle->getDvbt2AsyncL1PreStatus = NEXUS_Frontend_P_3466_GetDvbt2AsyncL1PreStatus;
    frontendHandle->getDvbt2AsyncL1PostConfigurableStatus = NEXUS_Frontend_P_3466_GetDvbt2AsyncL1PostConfigurableStatus;
    frontendHandle->getDvbt2AsyncPostDynamicStatus = NEXUS_Frontend_P_3466_GetDvbt2AsyncPostDynamicStatus;
    frontendHandle->getDvbt2AsyncL1PlpStatus = NEXUS_Frontend_P_3466_GetDvbt2AsyncL1PlpStatus;
    frontendHandle->getDvbt2AsyncBasicStatus = NEXUS_Frontend_P_3466_GetDvbt2AsyncBasicStatus;
    frontendHandle->requestOfdmAsyncStatus = NEXUS_Frontend_P_3466_RequestOfdmAsyncStatus;
    frontendHandle->getOfdmAsyncStatus = NEXUS_Frontend_P_3466_GetOfdmAsyncStatus;
    frontendHandle->tuneOfdm = NEXUS_Frontend_P_3466_TuneOfdm;
    frontendHandle->untune = NEXUS_Frontend_P_3466_UnTune;
    frontendHandle->resetStatus = NEXUS_Frontend_P_3466_ResetStatus;
    frontendHandle->readSoftDecisions = NEXUS_Frontend_P_3466_ReadSoftDecisions;
    frontendHandle->reapplyTransportSettings = NEXUS_Frontend_P_3466_ReapplyTransportSettings;
    frontendHandle->close = NEXUS_Frontend_P_3466_Close;
    frontendHandle->getFastStatus = NEXUS_Frontend_P_3466_GetFastStatus;
    frontendHandle->writeRegister = NEXUS_Frontend_P_3466_WriteRegister;
    frontendHandle->readRegister = NEXUS_Frontend_P_3466_ReadRegister;
    frontendHandle->uninstallCallbacks = NEXUS_FrontendDevice_P_3466_UninstallCallbacks;

    frontendHandle->pGenericDeviceHandle = pFrontendDevice;

    /* Create app callbacks */
    p3466Device->terrestrial.lockAppCallback[pSettings->channelNumber] = NEXUS_IsrCallback_Create(frontendHandle, NULL);
    if ( NULL == p3466Device->terrestrial.lockAppCallback[pSettings->channelNumber] ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err_cbk_create;}

    p3466Device->terrestrial.asyncStatusAppCallback[pSettings->channelNumber] = NEXUS_IsrCallback_Create(frontendHandle, NULL);
    if ( NULL == p3466Device->terrestrial.asyncStatusAppCallback[pSettings->channelNumber] ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err_cbk_create;}

    frontendHandle->userParameters.isMtsif = true;
    frontendHandle->mtsif.inputBand = NEXUS_FRONTEND_3466_OFDM_INPUT_BAND+pSettings->channelNumber;

    frontendHandle->standby = NEXUS_Frontend_P_3466_Terrestrial_Standby;

    p3466Device->terrestrial.frontendHandle[pSettings->channelNumber] = frontendHandle;
    /* save channel number in pChannel*/
    pChannel->chn_num = pSettings->channelNumber;
    pChannel->pDevice = p3466Device;
    frontendHandle->chip.familyId = 0x3466;
    frontendHandle->chip.id = 0x3466;
    return frontendHandle;

err_cbk_create:
    if ( p3466Device->terrestrial.lockAppCallback[pSettings->channelNumber] ) NEXUS_IsrCallback_Destroy(p3466Device->terrestrial.lockAppCallback[pSettings->channelNumber]);
    if ( p3466Device->terrestrial.asyncStatusAppCallback[pSettings->channelNumber] ) NEXUS_IsrCallback_Destroy(p3466Device->terrestrial.asyncStatusAppCallback[pSettings->channelNumber]);
    if ( frontendHandle ) BKNI_Free(frontendHandle);
err_alloc:
    NEXUS_FrontendDevice_P_Uninit3466(p3466Device);
    if (pChannel) BKNI_Free(pChannel);
    return NULL;
}
/* End of Terrestrial channel open implementation */

void NEXUS_FrontendDevice_Close3466_Terrestrial(NEXUS_3466Device *pDevice)
{
    /* Terrestrial teardown */
    NEXUS_FrontendDevice_P_Uninit3466(pDevice);
    NEXUS_FrontendDevice_P_Uninit_3466_Hab(pDevice);

    if(pDevice->capabilities.channelCapabilities)
        BKNI_Free(pDevice->capabilities.channelCapabilities);
    pDevice->capabilities.channelCapabilities = NULL;

    return;
    /* End of terrestrial teardown */
}

static NEXUS_Error NEXUS_Frontend_P_3466_Terrestrial_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3466Channel *pChannel = (NEXUS_3466Channel *)handle;
    NEXUS_3466Device *pDevice;
    BTNR_PowerSaverSettings pwrSettings;
    BODS_PowerSaverSettings odsPwrSettings;

    BSTD_UNUSED(enabled);

    pDevice = pChannel->pDevice;
    if (pSettings->mode < NEXUS_StandbyMode_ePassive) {
        if (!pDevice->terrestrial.isTunerPoweredOn[pChannel->chn_num]) {
            pwrSettings.enable = false;
            rc = BTNR_SetPowerSaver(pDevice->terrestrial.tnr[pChannel->chn_num], &pwrSettings);
            if (rc) { rc = BERR_TRACE(rc); }
            pDevice->terrestrial.isTunerPoweredOn[pChannel->chn_num] = true;
        }
        switch (pDevice->lastChannel)
        {
        case NEXUS_3466_DVBT2_CHN:
        case NEXUS_3466_DVBT_CHN:
            if (pDevice->terrestrial.isPoweredOn[pDevice->lastChannel]) {
                rc = BODS_DisablePowerSaver(pDevice->terrestrial.ods_chn[pDevice->lastChannel], &odsPwrSettings);
                if (rc) { rc = BERR_TRACE(rc); goto done; }
                pDevice->terrestrial.isPoweredOn[pDevice->lastChannel] = true;
            }
            break;
        default:
            BDBG_ERR((" Unsupported channel."));
            rc = BERR_TRACE(BERR_NOT_SUPPORTED);
            break;
        }
    }
    else {
        switch (pDevice->lastChannel)
        {
        case NEXUS_3466_DVBT2_CHN:
        case NEXUS_3466_DVBT_CHN:
            if (pDevice->terrestrial.isPoweredOn[pDevice->lastChannel]) {
                rc = BODS_EnablePowerSaver(pDevice->terrestrial.ods_chn[pDevice->lastChannel], &odsPwrSettings);
                if (rc) { rc = BERR_TRACE(rc); goto done; }
                pDevice->terrestrial.isPoweredOn[pDevice->lastChannel] = false;
            }
            break;
        default:
            BDBG_ERR((" Unsupported channel."));
            rc = BERR_TRACE(BERR_NOT_SUPPORTED);
            break;
        }
        if (pDevice->terrestrial.isTunerPoweredOn[pChannel->chn_num]) {
            pwrSettings.enable = true;
            rc = BTNR_SetPowerSaver(pDevice->terrestrial.tnr[pChannel->chn_num], &pwrSettings);
            if (rc) { rc = BERR_TRACE(rc); goto done; }
            pDevice->terrestrial.isTunerPoweredOn[pChannel->chn_num] = false;
        }
    }

done:
    return rc;
}
