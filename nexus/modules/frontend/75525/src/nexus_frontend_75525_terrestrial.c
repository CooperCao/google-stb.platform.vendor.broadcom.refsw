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
*    Terrestrial Tuner/Demodulator Device.
*
***************************************************************************/
/* Begin Includes */
#include "nexus_frontend_75525_priv.h"
#include "bchp_leap_host_l1.h"
#include "bhab_leap_priv.h"
#include "bhab_75525_fw.h"
/* End includes */

BDBG_MODULE(nexus_frontend_75525_terrestrial);

/* set to 1 to enable L1 interrupt messages */
#define NEXUS_FRONTEND_DEBUG_IRQ 0

static void NEXUS_Frontend_P_75525_ResetStatus(void *handle);
static void NEXUS_FrontendDevice_P_75525_UninstallCallbacks(void *handle);

static NEXUS_Error NEXUS_Frontend_P_75525_Terrestrial_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings);

#define BLEAP_HOST_L1_INTERRUPT_ID     BCHP_INT_ID_CREATE(BCHP_LEAP_HOST_L1_INTR_W0_STATUS, BCHP_LEAP_HOST_L1_INTR_W0_STATUS_LEAP_INTR_SHIFT)

static void NEXUS_Frontend_P_75525_AsyncStatusCallback_isr(void *pParam)
{
    NEXUS_75525Device *pDevice;
    BDBG_ASSERT(pParam != NULL);
    pDevice = (NEXUS_75525Device *)pParam;

    if (pDevice->asyncStatusAppCallback[pDevice->lastChannel])
    {
        pDevice->isStatusReady = true;
        NEXUS_IsrCallback_Fire_isr(pDevice->asyncStatusAppCallback[pDevice->lastChannel]);
    }
}

static void NEXUS_Frontend_P_75525_Ewscallback_isr(void *pParam)
{
    NEXUS_75525Device *pDevice;
    BDBG_ASSERT(pParam != NULL);
    pDevice = (NEXUS_75525Device *)pParam;
    if (pDevice->ewsAppCallback[0])
    {
        pDevice->isStatusReady = true;
        NEXUS_IsrCallback_Fire_isr(pDevice->ewsAppCallback[0]);
    }
}

static void NEXUS_Frontend_P_75525_callback_isr(void *pParam)
{
    NEXUS_75525Device *pDevice;
    BDBG_ASSERT(pParam != NULL);
    pDevice = (NEXUS_75525Device *)pParam;

    if(pDevice->acquireInProgress){
        pDevice->count++;
    }
    if(pDevice->count == 2){
        pDevice->acquireInProgress = false;
        pDevice->count = 0;
    }
    if ( pDevice->lockAppCallback[pDevice->lastChannel] )
    {
        NEXUS_IsrCallback_Fire_isr(pDevice->lockAppCallback[pDevice->lastChannel]);
    }
}

static void NEXUS_Frontend_P_75525_IsrControl_isr(bool enable, void *pParam)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    int isrnumber = (int)pParam;

#if NEXUS_FRONTEND_DEBUG_IRQ
    BDBG_MSG(("NEXUS_Frontend_P_75525_IsrControl_isr: (%p) enabled: %s", pParam, enable ? "true" : "false"));
#endif
    if ( enable )
    {
        rc = NEXUS_Core_EnableInterrupt_isr(isrnumber);
        if(rc) BERR_TRACE(rc);
    }
    else
    {
        NEXUS_Core_DisableInterrupt_isr(isrnumber);
    }
}

static void NEXUS_Frontend_P_75525_L1_isr(void *pParam1, int pParam2)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHAB_Handle hab = (BHAB_Handle)pParam2;
    BSTD_UNUSED(pParam1);

#if NEXUS_FRONTEND_DEBUG_IRQ
    BDBG_MSG(("NEXUS_Frontend_P_75525_L1_isr: (%p,%p)", pParam1, hab));
#endif
    if(hab){
        rc = BHAB_HandleInterrupt_isr(hab);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
done:
    return;
}

static void NEXUS_Frontend_P_75525_IsrEvent(void *pParam)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHAB_Handle hab = (BHAB_Handle)pParam;

#if NEXUS_FRONTEND_DEBUG_IRQ
    BDBG_MSG(("NEXUS_Frontend_P_75525_IsrEvent: %p",hab));
#endif
    if(hab){
        rc = BHAB_ProcessInterruptEvent(hab);
        if(rc) BERR_TRACE(rc);
    }
}

void NEXUS_FrontendDevice_P_Uninit_75525_Hab(NEXUS_75525Device *pDevice)
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

NEXUS_Error NEXUS_FrontendDevice_P_Init_75525_Hab(NEXUS_75525Device *pDevice, const NEXUS_FrontendDeviceOpenSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHAB_Settings stHabSettings;

    rc = BHAB_75525_GetDefaultSettings(&stHabSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if(pSettings->isrNumber) {
        stHabSettings.interruptEnableFunc = NEXUS_Frontend_P_75525_IsrControl_isr;
        stHabSettings.interruptEnableFuncParam = (void*)pSettings->isrNumber;
        stHabSettings.pChp = g_pCoreHandles->chp;
    }

    rc = BHAB_Open( &pDevice->hab, (void *)g_pCoreHandles->reg, &stHabSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    rc = BINT_CreateCallback(&(pDevice->cbHandle), g_pCoreHandles->bint, BLEAP_HOST_L1_INTERRUPT_ID, NEXUS_Frontend_P_75525_L1_isr, (void *)pDevice, (int)pDevice->hab);
    BDBG_ASSERT(rc == BERR_SUCCESS);

    rc = BINT_EnableCallback(pDevice->cbHandle);
    BDBG_ASSERT(rc == BERR_SUCCESS);

    {
        NEXUS_MemoryAllocationSettings memSettings;
        BHAB_75525_Settings hab75525Settings;

        BDBG_MSG(("Configuring 75525 with external memory"));
        NEXUS_Memory_GetDefaultAllocationSettings(&memSettings);
        memSettings.alignment = 0x100000;
        rc = NEXUS_Memory_Allocate(0x100000,&memSettings,&pDevice->leapBuffer);
        if(rc){BDBG_ERR(("Unable to allocate memory for 75525 LEAP"));rc = BERR_TRACE(rc); goto done;}

        hab75525Settings.bUseInternalMemory = false;
        hab75525Settings.pRam = pDevice->leapBuffer;
        hab75525Settings.physAddr = NEXUS_AddrToOffset(pDevice->leapBuffer);
        hab75525Settings.heap = NEXUS_Core_P_AddressToHeap(pDevice->leapBuffer, NULL);

        BDBG_MSG(("pRam: %p, physAddr: 0x%08x",hab75525Settings.pRam,hab75525Settings.physAddr));
        rc = BHAB_75525_Configure(pDevice->hab, &hab75525Settings);
         if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    /* Get events and register callbacks */
    rc = BHAB_GetInterruptEventHandle(pDevice->hab, &pDevice->terrestrial.isrEvent);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pDevice->terrestrial.isrEventCallback = NEXUS_RegisterEvent(pDevice->terrestrial.isrEvent, NEXUS_Frontend_P_75525_IsrEvent, pDevice->hab);
    if ( NULL == pDevice->terrestrial.isrEventCallback ){rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done; }
    if (pSettings->loadAP)
    {
        rc = BHAB_InitAp(pDevice->hab, bcm75525_leap_image);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    return BERR_SUCCESS;

done:
    NEXUS_FrontendDevice_P_Uninit_75525_Hab(pDevice);
    return rc;
}

void NEXUS_FrontendDevice_P_Uninit75525(NEXUS_75525Device *pDevice)
{
    NEXUS_Error rc = BERR_SUCCESS;
    unsigned int i;

    for ( i = 0; i < NEXUS_MAX_75525_TUNERS && NULL != pDevice->terrestrial.tnr[i]; i++) {
        if(pDevice->terrestrial.tnr[i]){
            rc = BTNR_Close(pDevice->terrestrial.tnr[i]);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->terrestrial.tnr[i] = NULL;
        }
    }
    for ( i = 0; i < NEXUS_75525_MAX_OFDM_CHN && NULL != pDevice->terrestrial.ods_chn[i]; i++) {
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
done:
    return;
}

static NEXUS_Error NEXUS_FrontendDevice_P_Init75525(NEXUS_75525Device *pDevice)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned int i, j, num_ch;
    BTNR_75525Ib_Settings tnr75525_cfg;
    BODS_Settings odsCfg;
    BODS_ChannelSettings odsChnCfg;
    unsigned acc_chn_num = 0;

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

    rc = BODS_75525_GetDefaultSettings( &odsCfg, NULL);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    odsCfg.hGeneric = pDevice->hab;
    rc = BODS_Open(&pDevice->terrestrial.ods, g_pCoreHandles->chp, g_pCoreHandles->reg, g_pCoreHandles->bint, &odsCfg);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    rc = BODS_Init(pDevice->terrestrial.ods);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    for (i=0;i<BODS_Standard_eLast;i++) {
        rc = BODS_GetTotalChannels(pDevice->terrestrial.ods, (BODS_Standard)i, &num_ch);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        if(num_ch > NEXUS_MAX_75525_TUNERS) {
            BDBG_ERR(("The maximum number of channels is incorrect. num_ch = %d", num_ch));
            rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
        }

        rc = BODS_GetChannelDefaultSettings( pDevice->terrestrial.ods, (BODS_Standard)i, &odsChnCfg);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        if(i == BODS_Standard_eIsdbt){
            odsChnCfg.hHeap = g_pCoreHandles->heap[0].mem;
        }
        for (j=0;j<num_ch;j++) {
            odsChnCfg.channelNo = j;
            if(acc_chn_num + j >= NEXUS_75525_MAX_OFDM_CHN) {
                BDBG_ERR(("The maximum number of total ODS channels %d is more than expected. ", acc_chn_num + j));
                rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
            }
            rc = BODS_OpenChannel( pDevice->terrestrial.ods, &pDevice->terrestrial.ods_chn[acc_chn_num + j], &odsChnCfg);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            rc = BODS_InstallCallback(pDevice->terrestrial.ods_chn[acc_chn_num + j], BODS_Callback_eLockChange, (BODS_CallbackFunc)NEXUS_Frontend_P_75525_callback_isr, (void*)pDevice);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            rc = BODS_InstallCallback(pDevice->terrestrial.ods_chn[acc_chn_num + j], BODS_Callback_eAsyncStatusReady, (BODS_CallbackFunc)NEXUS_Frontend_P_75525_AsyncStatusCallback_isr, (void*)pDevice);
            if(rc){rc = BERR_TRACE(rc); goto done;}
        }

        acc_chn_num += num_ch;
    }
#if 1
    BHAB_InstallInterruptCallback(pDevice->hab, BHAB_DevId_eGlobal, (BHAB_IntCallbackFunc)NEXUS_Frontend_P_75525_Ewscallback_isr, (void *)pDevice, BHAB_Interrupt_eEmergencyWarningSystem);
#endif

    for (i=0;i<NEXUS_MAX_75525_TUNERS;i++) {
        rc = BTNR_75525Ib_GetDefaultSettings(&tnr75525_cfg);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        tnr75525_cfg.channelNo = i;
        rc =  BTNR_75525Ib_Open(&pDevice->terrestrial.tnr[i],&tnr75525_cfg, pDevice->hab);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    return BERR_SUCCESS;

done:
    NEXUS_FrontendDevice_P_Uninit75525(pDevice);
    return rc;
}

static void NEXUS_Frontend_P_75525_UnTune(void *handle)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_75525Device *pDevice = (NEXUS_75525Device *)handle;
    BODS_PowerSaverSettings pwrSettings;

    BDBG_MSG(("Untune: pDevice = 0x%x", pDevice));
    BDBG_MSG(("Tuner is not powered down for now to decrease channel change time."));

    switch ( pDevice->lastChannel )
    {
    case NEXUS_75525_ISDBT_CHN:
    case NEXUS_75525_DVBT_CHN:
        if(pDevice->isPoweredOn[pDevice->lastChannel]) {
            rc = BODS_EnablePowerSaver(pDevice->terrestrial.ods_chn[pDevice->lastChannel], &pwrSettings);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->isPoweredOn[pDevice->lastChannel] = false;
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

static void NEXUS_Frontend_P_75525_Close(NEXUS_FrontendHandle handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_75525Device *pDevice;
    unsigned chn_num=0;
    BTNR_PowerSaverSettings pwrSettings;

    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    pDevice = (NEXUS_75525Device *) handle->pDeviceHandle;

    for(chn_num=0; chn_num< NEXUS_MAX_75525_T_FRONTENDS; chn_num++){
        pDevice->lastChannel = chn_num;
        if(pDevice->isPoweredOn[chn_num]) {
            NEXUS_Frontend_P_75525_UnTune(pDevice);
        }
    }
    if(pDevice->isTunerPoweredOn){
        pwrSettings.enable = true;
        rc = BTNR_SetPowerSaver(pDevice->terrestrial.tnr[0], &pwrSettings);
        if(rc){rc = BERR_TRACE(rc);}
    }

    for(chn_num=0; chn_num < NEXUS_MAX_75525_T_FRONTENDS; chn_num++){
        if ( pDevice->lockAppCallback[chn_num] ){
            NEXUS_IsrCallback_Destroy(pDevice->lockAppCallback[chn_num]);
        }
        if(pDevice->terrestrial.ods_chn[chn_num]){
            BODS_InstallCallback(pDevice->terrestrial.ods_chn[chn_num], BODS_Callback_eLockChange, NULL, NULL);
            BODS_InstallCallback(pDevice->terrestrial.ods_chn[chn_num], BODS_Callback_eAsyncStatusReady, NULL, NULL);
        }
        if ( pDevice->asyncStatusAppCallback[chn_num] ){
             NEXUS_IsrCallback_Destroy(pDevice->asyncStatusAppCallback[chn_num]);
        }
    }
    if ( pDevice->ewsAppCallback[0] )
        NEXUS_IsrCallback_Destroy(pDevice->ewsAppCallback[0]);

    if ( pDevice->updateGainAppCallback[0] )
        NEXUS_IsrCallback_Destroy(pDevice->updateGainAppCallback[0]);
    NEXUS_Frontend_P_Destroy(handle);

    pDevice->terrestrial.frontendHandle = NULL;
}

NEXUS_Error NEXUS_FrontendDevice_P_Init75525_Terrestrial(NEXUS_75525Device *pDevice)
{
    NEXUS_Error rc;
    rc = NEXUS_FrontendDevice_P_Init_75525_Hab(pDevice, &pDevice->openSettings);
    if (!rc)
        rc = NEXUS_FrontendDevice_P_Init75525(pDevice);

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

void NEXUS_FrontendDevice_P_Uninit75525_Terrestrial(NEXUS_75525Device *pDevice)
{
    NEXUS_FrontendDevice_P_75525_UninstallCallbacks(pDevice);
    NEXUS_FrontendDevice_P_Uninit75525(pDevice);
    NEXUS_FrontendDevice_P_Uninit_75525_Hab(pDevice);
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

static NEXUS_Error NEXUS_Frontend_P_75525_GetFastStatus(void *handle, NEXUS_FrontendFastStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_75525Device *pDevice = (NEXUS_75525Device *)handle;
    unsigned lock;

    switch ( pDevice->lastChannel )
    {
    case NEXUS_75525_ISDBT_CHN:
    case NEXUS_75525_DVBT_CHN:
        rc = BODS_GetLockStatus(pDevice->terrestrial.ods_chn[pDevice->lastChannel],  &lock);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        break;
    default:
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }
    pStatus->lockStatus = NEXUS_Frontend_P_GetLockStatus(lock);
    BKNI_EnterCriticalSection();
    pStatus->acquireInProgress = pDevice->acquireInProgress;
    BKNI_LeaveCriticalSection();
    return BERR_SUCCESS;
done:
    return rc;
}

static void NEXUS_FrontendDevice_P_75525_UninstallCallbacks(void *handle)
{
    NEXUS_75525Device *pDevice = (NEXUS_75525Device *)handle;

    if(pDevice->lockAppCallback[pDevice->lastChannel])NEXUS_IsrCallback_Set(pDevice->lockAppCallback[pDevice->lastChannel], NULL);
    if(pDevice->asyncStatusAppCallback[pDevice->lastChannel])NEXUS_IsrCallback_Set(pDevice->asyncStatusAppCallback[pDevice->lastChannel], NULL);
    if(pDevice->ewsAppCallback[0])NEXUS_IsrCallback_Set(pDevice->ewsAppCallback[0], NULL);
    return;
}

/* Terrestrial-specific functions */
static NEXUS_FrontendOfdmTimeInterleaving NEXUS_Frontend_P_ODSToTimeInterleaving(BODS_IsdbtTimeInterleaving magnum)
{
    switch (magnum) {
    default:
        BDBG_WRN(("Unknown Magnum time interleaving %d, defaulting to 0x", magnum));
        /* fall-through */
    case BODS_IsdbtTimeInterleaving_e0x:
        return NEXUS_FrontendOfdmTimeInterleaving_e0x;
    case BODS_IsdbtTimeInterleaving_e1x:
        return NEXUS_FrontendOfdmTimeInterleaving_e1x;
    case BODS_IsdbtTimeInterleaving_e2x:
        return NEXUS_FrontendOfdmTimeInterleaving_e2x;
    case BODS_IsdbtTimeInterleaving_e3x:
        return NEXUS_FrontendOfdmTimeInterleaving_e3x;
    }
}

static NEXUS_FrontendOfdmModulation NEXUS_Frontend_P_ODSToModulation( BODS_IsdbtModulation magnum )
{
    switch (magnum) {
    case BODS_IsdbtModulation_e64Qam:
        return NEXUS_FrontendOfdmModulation_eQam64;
    case BODS_IsdbtModulation_e16Qam:
        return NEXUS_FrontendOfdmModulation_eQam16;
    case BODS_IsdbtModulation_eDqpsk:
        return NEXUS_FrontendOfdmModulation_eDqpsk;
    default:
        BDBG_WRN(("Unrecognized ofdm modulation %d, defaulting to QPSK", magnum));
        /* fall-through */
    case BODS_IsdbtModulation_eQpsk:
        return NEXUS_FrontendOfdmModulation_eQpsk;
    }
}

static NEXUS_FrontendOfdmCodeRate NEXUS_Frontend_P_ODSToCodeRate( BODS_IsdbtCodeRate magnum )
{
    switch (magnum) {
    case BODS_IsdbtCodeRate_e1_2:
        return NEXUS_FrontendOfdmCodeRate_e1_2;
    case BODS_IsdbtCodeRate_e2_3:
        return NEXUS_FrontendOfdmCodeRate_e2_3;
    case BODS_IsdbtCodeRate_e3_4:
        return NEXUS_FrontendOfdmCodeRate_e3_4;
    case BODS_IsdbtCodeRate_e5_6:
        return NEXUS_FrontendOfdmCodeRate_e5_6;
    case BODS_IsdbtCodeRate_e7_8:
        return NEXUS_FrontendOfdmCodeRate_e7_8;
    default:
        return NEXUS_FrontendOfdmCodeRate_eMax;
    }
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


static NEXUS_Error NEXUS_Frontend_P_75525_GetOfdmAsyncStatus(void *handle, NEXUS_FrontendOfdmStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_75525Device *pDevice = (NEXUS_75525Device *)handle;
    unsigned chn_num;

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    switch ( pDevice->lastChannel )
    {
    case NEXUS_75525_DVBT_CHN:
        BKNI_Memset(&pDevice->terrestrial.odsStatus, 0, sizeof(pDevice->terrestrial.odsStatus));

        rc = BODS_GetSelectiveAsyncStatus(pDevice->terrestrial.ods_chn[NEXUS_75525_DVBT_CHN], BODS_SelectiveAsyncStatusType_eDvbt, &pDevice->terrestrial.odsStatus);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        chn_num = NEXUS_75525_DVBT_CHN;
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
    case NEXUS_75525_ISDBT_CHN:
        BKNI_Memset(&pDevice->terrestrial.odsStatus, 0, sizeof(pDevice->terrestrial.odsStatus));

        rc = BODS_GetSelectiveAsyncStatus(pDevice->terrestrial.ods_chn[NEXUS_75525_ISDBT_CHN], BODS_SelectiveAsyncStatusType_eIsdbt, &pDevice->terrestrial.odsStatus);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        chn_num = NEXUS_75525_ISDBT_CHN;
        pStatus->receiverLock = pDevice->terrestrial.odsStatus.status.isdbt.receiverLock;
        pStatus->fecLock = pDevice->terrestrial.odsStatus.status.isdbt.fecLock;
        pStatus->noSignalDetected = pDevice->terrestrial.odsStatus.status.isdbt.noSignalDetected;
        pStatus->transmissionMode = NEXUS_Frontend_P_OdsToTransmissionMode(pDevice->terrestrial.odsStatus.status.isdbt.transmissionMode);
        pStatus->guardInterval = NEXUS_Frontend_P_OdsToGuardInterval(pDevice->terrestrial.odsStatus.status.isdbt.guardInterval);
        /*pStatus->gainOffset = pDevice->terrestrial.odsStatus.status.isdbt.gainOffset;*/
        pStatus->carrierOffset = pDevice->terrestrial.odsStatus.status.isdbt.carrierOffset;
        pStatus->timingOffset = pDevice->terrestrial.odsStatus.status.isdbt.timingOffset;
        pStatus->signalStrength = pDevice->terrestrial.odsStatus.status.isdbt.signalStrength/10;
        pStatus->snr = pDevice->terrestrial.odsStatus.status.isdbt.snr*100/256;
        pStatus->spectrumInverted = pDevice->terrestrial.odsStatus.status.isdbt.spectrumInverted;
        pStatus->reacquireCount = pDevice->terrestrial.odsStatus.status.isdbt.reacqCount;
        pStatus->ews = pDevice->terrestrial.odsStatus.status.isdbt.ews;
        pStatus->partialReception = pDevice->terrestrial.odsStatus.status.isdbt.partialReception;
        pStatus->fecCorrectedBlocks = pDevice->terrestrial.odsStatus.status.isdbt.layerAStatus.rsCorrectedBlocks + pDevice->terrestrial.odsStatus.status.isdbt.layerBStatus.rsCorrectedBlocks;
        pStatus->fecUncorrectedBlocks = pDevice->terrestrial.odsStatus.status.isdbt.layerAStatus.rsUncorrectedBlocks + pDevice->terrestrial.odsStatus.status.isdbt.layerBStatus.rsUncorrectedBlocks;
        pStatus->fecCleanBlocks = pDevice->terrestrial.odsStatus.status.isdbt.layerAStatus.rsCleanBlocks + pDevice->terrestrial.odsStatus.status.isdbt.layerBStatus.rsCleanBlocks;

        pStatus->modulationA =NEXUS_Frontend_P_ODSToModulation(pDevice->terrestrial.odsStatus.status.isdbt.layerAStatus.modulation);
        pStatus->codeRateA = NEXUS_Frontend_P_ODSToCodeRate(pDevice->terrestrial.odsStatus.status.isdbt.layerAStatus.codeRate);
        pStatus->timeInterleavingA = NEXUS_Frontend_P_ODSToTimeInterleaving(pDevice->terrestrial.odsStatus.status.isdbt.layerAStatus.timeInterleaving);
        pStatus->numSegmentsA = pDevice->terrestrial.odsStatus.status.isdbt.layerAStatus.numSegments;
        pStatus->fecCorrectedBlocksA = pDevice->terrestrial.odsStatus.status.isdbt.layerAStatus.rsCorrectedBlocks;
        pStatus->fecUncorrectedBlocksA = pDevice->terrestrial.odsStatus.status.isdbt.layerAStatus.rsUncorrectedBlocks;
        pStatus->fecCleanBlocksA = pDevice->terrestrial.odsStatus.status.isdbt.layerAStatus.rsCleanBlocks;
        pStatus->signalLevelPercentA = pDevice->terrestrial.odsStatus.status.isdbt.layerAStatus.signalLevelPercent;
        pStatus->signalQualityPercentA = pDevice->terrestrial.odsStatus.status.isdbt.layerAStatus.signalQualityPercent;

        pStatus->modulationB =NEXUS_Frontend_P_ODSToModulation(pDevice->terrestrial.odsStatus.status.isdbt.layerBStatus.modulation);
        pStatus->codeRateB = NEXUS_Frontend_P_ODSToCodeRate(pDevice->terrestrial.odsStatus.status.isdbt.layerBStatus.codeRate);
        pStatus->timeInterleavingB = NEXUS_Frontend_P_ODSToTimeInterleaving(pDevice->terrestrial.odsStatus.status.isdbt.layerBStatus.timeInterleaving);
        pStatus->numSegmentsB = pDevice->terrestrial.odsStatus.status.isdbt.layerBStatus.numSegments;
        pStatus->fecCorrectedBlocksB = pDevice->terrestrial.odsStatus.status.isdbt.layerBStatus.rsCorrectedBlocks;
        pStatus->fecUncorrectedBlocksB = pDevice->terrestrial.odsStatus.status.isdbt.layerBStatus.rsUncorrectedBlocks;
        pStatus->fecCleanBlocksB = pDevice->terrestrial.odsStatus.status.isdbt.layerBStatus.rsCleanBlocks;
        pStatus->signalLevelPercentB = pDevice->terrestrial.odsStatus.status.isdbt.layerBStatus.signalLevelPercent;
        pStatus->signalQualityPercentB = pDevice->terrestrial.odsStatus.status.isdbt.layerBStatus.signalQualityPercent;

        pStatus->modulationC =NEXUS_Frontend_P_ODSToModulation(pDevice->terrestrial.odsStatus.status.isdbt.layerCStatus.modulation);
        pStatus->codeRateC = NEXUS_Frontend_P_ODSToCodeRate(pDevice->terrestrial.odsStatus.status.isdbt.layerCStatus.codeRate);
        pStatus->timeInterleavingC = NEXUS_Frontend_P_ODSToTimeInterleaving(pDevice->terrestrial.odsStatus.status.isdbt.layerCStatus.timeInterleaving);
        pStatus->numSegmentsC = pDevice->terrestrial.odsStatus.status.isdbt.layerCStatus.numSegments;
        pStatus->fecCorrectedBlocksC = pDevice->terrestrial.odsStatus.status.isdbt.layerCStatus.rsCorrectedBlocks;
        pStatus->fecUncorrectedBlocksC = pDevice->terrestrial.odsStatus.status.isdbt.layerCStatus.rsUncorrectedBlocks;
        pStatus->fecCleanBlocksC = pDevice->terrestrial.odsStatus.status.isdbt.layerCStatus.rsCleanBlocks;
        pStatus->signalLevelPercentC = pDevice->terrestrial.odsStatus.status.isdbt.layerCStatus.signalLevelPercent;
        pStatus->signalQualityPercentC = pDevice->terrestrial.odsStatus.status.isdbt.layerCStatus.signalQualityPercent;
        break;
    default:
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }

    pStatus->settings = pDevice->terrestrial.last_ofdm[0];

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_75525_RequestOfdmAsyncStatus(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_InternalGainSettings settings;
    NEXUS_GainParameters params;
    NEXUS_ExternalGainSettings externalGainSettings;
    NEXUS_75525Device *pDevice = (NEXUS_75525Device *)handle;

    if(pDevice->pGenericDeviceHandle->parent){

        params.rfInput = pDevice->pGenericDeviceHandle->linkSettings.rfInput;
        params.accumulateTillRootDevice = true;
        params.frequency = pDevice->terrestrial.last_ofdm[0].frequency;

        rc = NEXUS_FrontendDevice_P_GetInternalGain(pDevice->pGenericDeviceHandle->parent, &params, &settings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        externalGainSettings.totalGain = settings.totalVariableGain;

        rc = NEXUS_FrontendDevice_P_SetExternalGain(pDevice->pGenericDeviceHandle, &externalGainSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

    }

    switch ( pDevice->lastChannel )
    {
    case NEXUS_75525_ISDBT_CHN:
        rc = BODS_RequestSelectiveAsyncStatus(pDevice->terrestrial.ods_chn[pDevice->lastChannel], BODS_SelectiveAsyncStatusType_eIsdbt);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        break;
    case NEXUS_75525_DVBT_CHN:
        rc = BODS_RequestSelectiveAsyncStatus(pDevice->terrestrial.ods_chn[NEXUS_75525_DVBT_CHN], BODS_SelectiveAsyncStatusType_eDvbt);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        break;
    default:
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }
done:
    return rc;
}


static NEXUS_Error NEXUS_Frontend_P_75525_RequestIsdbtAsyncStatus(void *handle, NEXUS_FrontendIsdbtStatusType type)
{
    NEXUS_Error rc = BERR_SUCCESS;
    NEXUS_75525Device *pDevice;
    BSTD_UNUSED(type);
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_75525Device *)handle;

    pDevice->isStatusReady = false;

    rc = NEXUS_Frontend_P_75525_RequestOfdmAsyncStatus(handle);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}


static NEXUS_Error NEXUS_Frontend_P_75525_GetIsdbtAsyncStatusReady(void *handle, NEXUS_FrontendIsdbtStatusReady *pAsyncStatusReady)
{
    NEXUS_Error rc = BERR_SUCCESS;
    NEXUS_75525Device *pDevice;

    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_75525Device *)handle;

    if(pDevice->isStatusReady){
        pAsyncStatusReady->type[NEXUS_FrontendIsdbtStatusType_eBasic] = true;
    }
    else{
        pAsyncStatusReady->type[NEXUS_FrontendIsdbtStatusType_eBasic] = false;
    }

    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_75525_GetIsdbtAsyncStatus(void *handle, NEXUS_FrontendIsdbtStatusType type, NEXUS_FrontendIsdbtStatus *pStatus)
{
    NEXUS_Error rc = BERR_SUCCESS;
    NEXUS_75525Device *pDevice;
    unsigned chn_num;
    BODS_IsdbtStatus  *status;

    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_75525Device *)handle;
    chn_num = pDevice->lastChannel;

    if (chn_num >= NEXUS_75525_MAX_OFDM_CHN)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    BKNI_Memset(&pDevice->terrestrial.odsStatus, 0, sizeof(pDevice->terrestrial.odsStatus));

    pStatus->status.basic.settings = pDevice->terrestrial.last_ofdm[0];

    if(type == NEXUS_FrontendIsdbtStatusType_eBasic){
        rc = BODS_GetSelectiveAsyncStatus(pDevice->terrestrial.ods_chn[chn_num], BODS_SelectiveAsyncStatusType_eIsdbt, &pDevice->terrestrial.odsStatus);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        status = &pDevice->terrestrial.odsStatus.status.isdbt;

        pStatus->status.basic.fecLock = status->fecLock;
        pStatus->status.basic.tmcc.transmissionMode = NEXUS_Frontend_P_OdsToTransmissionMode(status->transmissionMode);
        pStatus->status.basic.tmcc.guardInterval = NEXUS_Frontend_P_OdsToGuardInterval(status->guardInterval);
        pStatus->status.basic.tmcc.ews = status->ews;
        pStatus->status.basic.tmcc.partialReception = status->partialReception;
        pStatus->status.basic.carrierOffset = status->carrierOffset;
        pStatus->status.basic.timingOffset = status->timingOffset;
        pStatus->status.basic.signalStrength = status->signalStrength/10;
        pStatus->status.basic.snr = status->snr*100/256;
        pStatus->status.basic.spectrumInverted = status->spectrumInverted;
        pStatus->status.basic.reacquireCount = status->reacqCount;

        pStatus->status.basic.layerA.modulation = NEXUS_Frontend_P_ODSToModulation(status->layerAStatus.modulation);
        pStatus->status.basic.layerA.codeRate = NEXUS_Frontend_P_ODSToCodeRate(status->layerAStatus.codeRate);
        pStatus->status.basic.layerA.timeInterleaving = NEXUS_Frontend_P_ODSToTimeInterleaving(status->layerAStatus.timeInterleaving);
        pStatus->status.basic.layerA.numSegments = status->layerAStatus.numSegments;
        pStatus->status.basic.layerA.fecBlockCounts.corrected = status->layerAStatus.rsCorrectedBlocks;
        pStatus->status.basic.layerA.fecBlockCounts.uncorrected= status->layerAStatus.rsUncorrectedBlocks;
        pStatus->status.basic.layerA.fecBlockCounts.clean = status->layerAStatus.rsCleanBlocks;
        pStatus->status.basic.layerA.signalLevelPercent = status->layerAStatus.signalLevelPercent;
        pStatus->status.basic.layerA.signalQualityPercent = status->layerAStatus.signalQualityPercent;

        pStatus->status.basic.layerB.modulation =NEXUS_Frontend_P_ODSToModulation(status->layerBStatus.modulation);
        pStatus->status.basic.layerB.codeRate = NEXUS_Frontend_P_ODSToCodeRate(status->layerBStatus.codeRate);
        pStatus->status.basic.layerB.timeInterleaving = NEXUS_Frontend_P_ODSToTimeInterleaving(status->layerBStatus.timeInterleaving);
        pStatus->status.basic.layerB.numSegments = status->layerBStatus.numSegments;
        pStatus->status.basic.layerB.fecBlockCounts.corrected = status->layerBStatus.rsCorrectedBlocks;
        pStatus->status.basic.layerB.fecBlockCounts.uncorrected = status->layerBStatus.rsUncorrectedBlocks;
        pStatus->status.basic.layerB.fecBlockCounts.clean = status->layerBStatus.rsCleanBlocks;
        pStatus->status.basic.layerB.signalLevelPercent = status->layerBStatus.signalLevelPercent;
        pStatus->status.basic.layerB.signalQualityPercent = status->layerBStatus.signalQualityPercent;

        pStatus->status.basic.layerC.modulation =NEXUS_Frontend_P_ODSToModulation(status->layerCStatus.modulation);
        pStatus->status.basic.layerC.codeRate = NEXUS_Frontend_P_ODSToCodeRate(status->layerCStatus.codeRate);
        pStatus->status.basic.layerC.timeInterleaving = NEXUS_Frontend_P_ODSToTimeInterleaving(status->layerCStatus.timeInterleaving);
        pStatus->status.basic.layerC.numSegments = status->layerCStatus.numSegments;
        pStatus->status.basic.layerC.fecBlockCounts.corrected = status->layerCStatus.rsCorrectedBlocks;
        pStatus->status.basic.layerC.fecBlockCounts.uncorrected = status->layerCStatus.rsUncorrectedBlocks;
        pStatus->status.basic.layerC.fecBlockCounts.clean = status->layerCStatus.rsCleanBlocks;
        pStatus->status.basic.layerC.signalLevelPercent = status->layerCStatus.signalLevelPercent;
        pStatus->status.basic.layerC.signalQualityPercent = status->layerCStatus.signalQualityPercent;

        if(pStatus->status.basic.settings.bert.enabled == true){
            pStatus->status.basic.layerA.bert.locked = status->layerAStatus.bertSync;
            pStatus->status.basic.layerA.bert.errorRate.count = status->layerAStatus.bertErrorBits;
            pStatus->status.basic.layerA.bert.errorRate.total = status->layerAStatus.bertTotalBits;

            if(pStatus->status.basic.layerA.bert.locked){
                if(status->layerAStatus.bertErrorBits <= status->layerAStatus.bertTotalBits){
                    /* This is equvivalent to left shifting the numerator by 15 and right shifting the denominator by 16. */
                    if(status->layerAStatus.bertTotalBits/65536)
                        pStatus->status.basic.layerA.bert.errorRate.rate = (status->layerAStatus.bertErrorBits * 32768)/(status->layerAStatus.bertTotalBits/65536);
                }
                else{
                    NEXUS_Frontend_P_75525_ResetStatus(handle);
                }
            }

            pStatus->status.basic.layerB.bert.locked = status->layerBStatus.bertSync;
            pStatus->status.basic.layerB.bert.errorRate.count = status->layerBStatus.bertErrorBits;
            pStatus->status.basic.layerB.bert.errorRate.total = status->layerBStatus.bertTotalBits;
            if(pStatus->status.basic.layerB.bert.locked){
                if(status->layerBStatus.bertErrorBits <= status->layerBStatus.bertTotalBits){
                    if(status->layerBStatus.bertTotalBits/65536)
                        pStatus->status.basic.layerB.bert.errorRate.rate = (status->layerBStatus.bertErrorBits * 32768)/(status->layerBStatus.bertTotalBits/65536);
                }
                else{
                    NEXUS_Frontend_P_75525_ResetStatus(handle);
                }
            }

            pStatus->status.basic.layerC.bert.locked = status->layerCStatus.bertSync;
            pStatus->status.basic.layerC.bert.errorRate.count = status->layerCStatus.bertErrorBits;
            pStatus->status.basic.layerC.bert.errorRate.total = status->layerCStatus.bertTotalBits;
            if(pStatus->status.basic.layerC.bert.locked){
                if(status->layerCStatus.bertErrorBits <= status->layerCStatus.bertTotalBits){
                    if(status->layerCStatus.bertTotalBits/65536)
                        pStatus->status.basic.layerC.bert.errorRate.rate = (status->layerCStatus.bertErrorBits * 32768)/(status->layerCStatus.bertTotalBits/65536);
                }
                else{
                    NEXUS_Frontend_P_75525_ResetStatus(handle);
                }
            }
        }
    }

    pDevice->isStatusReady = false;
done:
    return rc;

}

NEXUS_Error NEXUS_Tuner_P_75525_GetNSetGain(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_GainParameters params;
    NEXUS_InternalGainSettings settings;
    NEXUS_ExternalGainSettings externalGainSettings;
    NEXUS_75525Device *pDevice = (NEXUS_75525Device *)handle;

    BKNI_Memset(&externalGainSettings, 0, sizeof(externalGainSettings));

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

static NEXUS_Error NEXUS_Frontend_P_75525_RequestDvbtAsyncStatus(void *handle, NEXUS_FrontendDvbtStatusType type)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_75525Device *pDevice = (NEXUS_75525Device *)handle;
    BSTD_UNUSED(type);

    pDevice->isStatusReady = false;
    rc = NEXUS_Frontend_P_75525_RequestOfdmAsyncStatus(pDevice);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_75525_GetDvbtAsyncStatusReady(void *handle, NEXUS_FrontendDvbtStatusReady *pAsyncStatusReady)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_75525Device *pDevice = (NEXUS_75525Device *)handle;

    if(pDevice->isStatusReady){
        pAsyncStatusReady->type[NEXUS_FrontendDvbtStatusType_eBasic] = true;
    }
    else{
        pAsyncStatusReady->type[NEXUS_FrontendDvbtStatusType_eBasic] = false;
    }

    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_75525_GetDvbtAsyncStatus(void *handle, NEXUS_FrontendDvbtStatusType type, NEXUS_FrontendDvbtStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_75525Device *pDevice = (NEXUS_75525Device *)handle;

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    BKNI_Memset(&pDevice->terrestrial.odsStatus, 0, sizeof(pDevice->terrestrial.odsStatus));

    if(type == NEXUS_FrontendDvbtStatusType_eBasic){
        rc = BODS_GetSelectiveAsyncStatus(pDevice->terrestrial.ods_chn[NEXUS_75525_DVBT_CHN], BODS_SelectiveAsyncStatusType_eDvbt, &pDevice->terrestrial.odsStatus);
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
    pDevice->isStatusReady = false;
done:
    return rc;

}


static NEXUS_Error NEXUS_Frontend_P_75525_TuneOfdm(void *handle, const NEXUS_FrontendOfdmSettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_75525Device *pDevice = (NEXUS_75525Device *)handle;
    BODS_AcquireParams odsParam;
    unsigned chn_num = 0;
    BTNR_Settings tnrSettings;
    unsigned temp_frequency;
    NEXUS_FrontendOfdmMode temp_mode;
    BODS_PowerSaverSettings odsPwrSettings;
    BTNR_PowerSaverSettings tnrPwrSettings;

    switch ( pSettings->mode )
    {
    case NEXUS_FrontendOfdmMode_eDvbt:
        chn_num = NEXUS_75525_DVBT_CHN;
        break;
    case NEXUS_FrontendOfdmMode_eIsdbt:
        chn_num = NEXUS_75525_ISDBT_CHN;
        break;
    default:
        /* It is IMPORTANT to check this condition here. Because it will not be checked later. */
        BDBG_ERR(("Wrong Ofdm mode selected."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }

    /* This is done as DVBT and ISDBT are in a single core and needs to be powered down*/
    if(pDevice->terrestrial.last_ofdm[0].mode != pSettings->mode){
        NEXUS_Frontend_P_75525_UnTune(handle);
    }

    rc = NEXUS_Tuner_P_75525_GetNSetGain(pDevice);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    rc = BODS_GetDefaultAcquireParams(pDevice->terrestrial.ods_chn[chn_num], &odsParam);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    NEXUS_IsrCallback_Set(pDevice->lockAppCallback[chn_num], &(pSettings->lockCallback));
    NEXUS_IsrCallback_Set(pDevice->asyncStatusAppCallback[chn_num], &(pSettings->asyncStatusReadyCallback));
    NEXUS_IsrCallback_Set(pDevice->ewsAppCallback[0], &(pSettings->ewsCallback));

    if(!pDevice->isPoweredOn[chn_num] || !pDevice->isTunerPoweredOn)
        goto full_acquire;

    if((pSettings->acquisitionMode == NEXUS_FrontendOfdmAcquisitionMode_eScan) && (pDevice->lastAcquisitionMode[chn_num] == NEXUS_FrontendOfdmAcquisitionMode_eScan)){
        temp_frequency = pDevice->terrestrial.last_ofdm[0].frequency;
        pDevice->terrestrial.last_ofdm[0].frequency = pSettings->frequency ;
        temp_mode = pDevice->terrestrial.last_ofdm[0].mode;
        pDevice->terrestrial.last_ofdm[0].mode = pSettings->mode;

        if(!BKNI_Memcmp(pSettings, &pDevice->terrestrial.last_ofdm[0], sizeof(NEXUS_FrontendOfdmSettings))) {
            if (pDevice->terrestrial.tnr[0])
            {
                pDevice->acquireInProgress = true;
                pDevice->terrestrial.last_ofdm[0] = *pSettings;
                pDevice->lastChannel = chn_num;
                rc = BTNR_SetTunerRfFreq(pDevice->terrestrial.tnr[0], pSettings->frequency, BTNR_TunerMode_eDigital);
                if(rc){rc = BERR_TRACE(rc); goto retrack;}

                return rc;
            }
        }
    }

full_acquire:
    pDevice->acquireInProgress = true;
    pDevice->lastChannel = chn_num;
    pDevice->lastAcquisitionMode[chn_num] = pSettings->acquisitionMode;

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
    case NEXUS_FrontendOfdmMode_eIsdbt:
       switch ( pSettings->bandwidth )
        {
        case 1700000:
            odsParam.acquireParams.isdbt.bandwidth = BODS_IsdbtBandwidth_e1p7MHz;
            break;
        case 5000000:
            odsParam.acquireParams.isdbt.bandwidth = BODS_IsdbtBandwidth_e5MHz;
            break;
        case 6000000:
            odsParam.acquireParams.isdbt.bandwidth = BODS_IsdbtBandwidth_e6MHz;
            break;
        case 7000000:
            odsParam.acquireParams.isdbt.bandwidth = BODS_IsdbtBandwidth_e7MHz;
            break;
        case 8000000:
            odsParam.acquireParams.isdbt.bandwidth = BODS_IsdbtBandwidth_e8MHz;
            break;
        default:
            BDBG_MSG((" Unsupported bandwidth."));
            rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
        }
        switch (pSettings->cciMode)
        {
        case NEXUS_FrontendOfdmCciMode_eNone:
            odsParam.acquireParams.isdbt.cciMode = BODS_IsdbtCciMode_eNone;
            break;
        case NEXUS_FrontendOfdmCciMode_eAuto:
            odsParam.acquireParams.isdbt.cciMode = BODS_IsdbtCciMode_eAuto;
            break;
        default:
            BDBG_MSG((" Unsupported bandwidth."));
            rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
        }

        odsParam.spurDetectAndCancel = pSettings->spurDetectAndCancel;
        odsParam.acquireParams.isdbt.carrierRange = (BODS_IsdbtCarrierRange)pSettings->pullInRange;
        if(pSettings->manualModeSettings){
            switch (pSettings->modeSettings.modeGuard)
            {
            case NEXUS_FrontendOfdmModeGuard_eAutoIsdbtJapan:
                odsParam.acquireParams.isdbt.transGuardMode = BODS_IsdbtOfdmMode_eAutoJapan;
                break;
            case NEXUS_FrontendOfdmModeGuard_eAutoIsdbtBrazil:
                odsParam.acquireParams.isdbt.transGuardMode = BODS_IsdbtOfdmMode_eAutoBrazil;
                break;
            case NEXUS_FrontendOfdmModeGuard_eManual:
                odsParam.acquireParams.isdbt.transGuardMode = BODS_IsdbtOfdmMode_eManual;
                odsParam.acquireParams.isdbt.guardInterval = pSettings->modeSettings.guardInterval;
                if((pSettings->modeSettings.mode>NEXUS_FrontendOfdmTransmissionMode_e1k) && (pSettings->modeSettings.mode<NEXUS_FrontendOfdmTransmissionMode_e16k))
                    odsParam.acquireParams.isdbt.transmissionMode = pSettings->modeSettings.mode;
                else {
                    BDBG_MSG((" Unsupported ISDBT Transmission mode."));
                    rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
                }
                break;
            default:
                BDBG_MSG((" Unsupported transGuardMode."));
                rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
            }
        }
        else {
            odsParam.acquireParams.isdbt.transGuardMode = BODS_IsdbtOfdmMode_eAutoBrazil;
        }

        odsParam.acquireParams.isdbt.tmccAcquire = true; /* CHECK THIS. */

        if(pSettings->bert.enabled){
            odsParam.bertEnable = true;
            odsParam.bertHeaderLength = pSettings->bert.header;
            odsParam.bertPolynomial = pSettings->bert.polynomial;
        }
        else
            odsParam.bertEnable = false;
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

    if (pDevice->terrestrial.tnr[0])
    {
        if(!pDevice->isTunerPoweredOn){
            tnrPwrSettings.enable = false;
            rc = BTNR_SetPowerSaver(pDevice->terrestrial.tnr[0], &tnrPwrSettings);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->isTunerPoweredOn = true;
        }
        if(!pDevice->isPoweredOn[chn_num]){
            rc = BODS_DisablePowerSaver(pDevice->terrestrial.ods_chn[chn_num], &odsPwrSettings);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->isPoweredOn[chn_num] = true;
        }

        rc = BODS_SetAcquireParams( pDevice->terrestrial.ods_chn[chn_num], &odsParam );
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = BTNR_GetSettings(pDevice->terrestrial.tnr[0], &tnrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        else if(pSettings->mode == NEXUS_FrontendOfdmMode_eDvbt){
            tnrSettings.std = BTNR_Standard_eDvbt;
        }
        else if(pSettings->mode == NEXUS_FrontendOfdmMode_eIsdbt){
            tnrSettings.std = BTNR_Standard_eIsdbt;
        }
        tnrSettings.bandwidth = pSettings->bandwidth;

        rc = BTNR_SetSettings(pDevice->terrestrial.tnr[0], &tnrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = BTNR_SetTunerRfFreq(pDevice->terrestrial.tnr[0], pSettings->frequency, BTNR_TunerMode_eDigital);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    if ( pSettings->acquisitionMode != NEXUS_FrontendOfdmAcquisitionMode_eScan) {
        switch ( pSettings->mode )
        {
        case NEXUS_FrontendOfdmMode_eIsdbt:
        case NEXUS_FrontendOfdmMode_eDvbt:
            BKNI_Sleep(500);
            rc = BODS_Acquire(pDevice->terrestrial.ods_chn[chn_num], &odsParam);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            break;
        default:
            break;
        }
    }

    pDevice->terrestrial.last_ofdm[0] = *pSettings;
    return BERR_SUCCESS;
retrack:
    pDevice->terrestrial.last_ofdm[0].frequency = temp_frequency;
    pDevice->terrestrial.last_ofdm[0].mode = temp_mode;
done:
    NEXUS_Frontend_P_75525_UnTune(handle);
    return rc;
}

static void NEXUS_Frontend_P_75525_ResetStatus(void *handle)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_75525Device *pDevice = (NEXUS_75525Device *)handle;

    switch ( pDevice->lastChannel )
    {
    case NEXUS_75525_ISDBT_CHN:
    case NEXUS_75525_DVBT_CHN:
        if(pDevice->isPoweredOn[pDevice->lastChannel]) {
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

static NEXUS_Error NEXUS_Frontend_P_75525_ReadSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length, size_t *pNumRead)
{
    size_t i;
    NEXUS_Error rc = NEXUS_SUCCESS;
    int16_t return_length;
    NEXUS_75525Device *pDevice = (NEXUS_75525Device *)handle;
    int16_t d_i[TOTAL_SOFTDECISIONS], d_q[TOTAL_SOFTDECISIONS];

    BKNI_Memset(pDecisions, 0, (sizeof(NEXUS_FrontendSoftDecision) * length));

    switch ( pDevice->lastChannel )
    {
    case NEXUS_75525_ISDBT_CHN:
    case NEXUS_75525_DVBT_CHN:
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

/* Terrestrial frontend device implementation */
NEXUS_Error NEXUS_FrontendDevice_Open75525_Terrestrial(NEXUS_75525Device *pDevice)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    rc = NEXUS_FrontendDevice_P_Init_75525_Hab(pDevice, &pDevice->openSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    /* Initialize THD and T2 channels*/
    rc = NEXUS_FrontendDevice_P_Init75525(pDevice);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}
/* End terrestrial frontend device implementation */

/* Terrestrial implementation */
NEXUS_FrontendHandle NEXUS_Frontend_Open75525_Terrestrial(const NEXUS_FrontendChannelSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_FrontendHandle frontendHandle = NULL;
    NEXUS_75525Device *pDevice = NULL;
    unsigned chn_num=0;
    NEXUS_FrontendDevice *pFrontendDevice = NULL;

    if(pSettings->device != NULL) {
        pFrontendDevice =  pSettings->device;
        pDevice = pFrontendDevice->pDevice;
    }
    else{
        BDBG_WRN(("Open the device handle first before opening the frontend channel handle"));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        goto done;
    }

    /* Create a Nexus frontend handle */
    frontendHandle = NEXUS_Frontend_P_Create(pDevice);
    if ( NULL == frontendHandle ) {rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_alloc;}

    /* Establish device capabilities */
    frontendHandle->capabilities.ofdm = true;
    frontendHandle->capabilities.ofdmModes[NEXUS_FrontendOfdmMode_eDvbt] = true;
    frontendHandle->capabilities.ofdmModes[NEXUS_FrontendOfdmMode_eIsdbt] = true;

    frontendHandle->requestDvbtAsyncStatus = NEXUS_Frontend_P_75525_RequestDvbtAsyncStatus;
    frontendHandle->getDvbtAsyncStatusReady = NEXUS_Frontend_P_75525_GetDvbtAsyncStatusReady;
    frontendHandle->getDvbtAsyncStatus = NEXUS_Frontend_P_75525_GetDvbtAsyncStatus;
    frontendHandle->requestIsdbtAsyncStatus = NEXUS_Frontend_P_75525_RequestIsdbtAsyncStatus;
    frontendHandle->getIsdbtAsyncStatusReady = NEXUS_Frontend_P_75525_GetIsdbtAsyncStatusReady;
    frontendHandle->getIsdbtAsyncStatus = NEXUS_Frontend_P_75525_GetIsdbtAsyncStatus;
    frontendHandle->requestOfdmAsyncStatus = NEXUS_Frontend_P_75525_RequestOfdmAsyncStatus;
    frontendHandle->getOfdmAsyncStatus = NEXUS_Frontend_P_75525_GetOfdmAsyncStatus;
    frontendHandle->tuneOfdm = NEXUS_Frontend_P_75525_TuneOfdm;
    frontendHandle->untune = NEXUS_Frontend_P_75525_UnTune;
    frontendHandle->resetStatus = NEXUS_Frontend_P_75525_ResetStatus;
    frontendHandle->readSoftDecisions = NEXUS_Frontend_P_75525_ReadSoftDecisions;
    frontendHandle->close = NEXUS_Frontend_P_75525_Close;
    frontendHandle->getFastStatus = NEXUS_Frontend_P_75525_GetFastStatus;
    frontendHandle->uninstallCallbacks = NEXUS_FrontendDevice_P_75525_UninstallCallbacks;

    frontendHandle->pGenericDeviceHandle = pFrontendDevice;

    /* Create app callbacks */
    for(chn_num=0; chn_num < NEXUS_MAX_75525_T_FRONTENDS; chn_num++){

        pDevice->lockAppCallback[chn_num] = NEXUS_IsrCallback_Create(frontendHandle, NULL);
        if ( NULL == pDevice->lockAppCallback[chn_num] ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err_cbk_create;}

        pDevice->asyncStatusAppCallback[chn_num] = NEXUS_IsrCallback_Create(frontendHandle, NULL);
        if ( NULL == pDevice->asyncStatusAppCallback[chn_num] ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err_cbk_create;}
     }

    pDevice->ewsAppCallback[0] = NEXUS_IsrCallback_Create(frontendHandle, NULL);
    if ( NULL == pDevice->ewsAppCallback[0] ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err_cbk_create;}

    pDevice->updateGainAppCallback[0] = NEXUS_IsrCallback_Create(frontendHandle, NULL);
    if ( NULL == pDevice->updateGainAppCallback[0] ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err_cbk_create;}

    frontendHandle->userParameters.isMtsif = true;

    frontendHandle->standby = NEXUS_Frontend_P_75525_Terrestrial_Standby;

    pDevice->terrestrial.frontendHandle = frontendHandle;
    frontendHandle->chip.familyId = 0x75525;
    frontendHandle->chip.id = 0x75525;
    return frontendHandle;

err_cbk_create:
    for(chn_num=0; chn_num < NEXUS_MAX_75525_T_FRONTENDS; chn_num++){
        if ( pDevice->lockAppCallback[chn_num] ) NEXUS_IsrCallback_Destroy(pDevice->lockAppCallback[chn_num]);
        if ( pDevice->asyncStatusAppCallback[chn_num] ) NEXUS_IsrCallback_Destroy(pDevice->asyncStatusAppCallback[chn_num]);
    }
    if ( pDevice->ewsAppCallback[0] ) NEXUS_IsrCallback_Destroy(pDevice->ewsAppCallback[0]);
    if ( pDevice->updateGainAppCallback[0]) NEXUS_IsrCallback_Destroy(pDevice->updateGainAppCallback[0]);
    if ( frontendHandle ) BKNI_Free(frontendHandle);
err_alloc:
    NEXUS_FrontendDevice_P_Uninit75525(pDevice);
done:
    return NULL;
}
/* End of Terrestrial channel open implementation */

void NEXUS_FrontendDevice_Close75525_Terrestrial(NEXUS_75525Device *pDevice)
{
    /* Terrestrial teardown */
    NEXUS_FrontendDevice_P_Uninit75525(pDevice);
    NEXUS_FrontendDevice_P_Uninit_75525_Hab(pDevice);

    if(pDevice->capabilities.channelCapabilities)
        BKNI_Free(pDevice->capabilities.channelCapabilities);
    pDevice->capabilities.channelCapabilities = NULL;

    return;
    /* End of terrestrial teardown */
}

static NEXUS_Error NEXUS_Frontend_P_75525_Terrestrial_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_75525Device *pDevice = (NEXUS_75525Device *)handle;
    BTNR_PowerSaverSettings pwrSettings;
    BODS_PowerSaverSettings odsPwrSettings;

    BSTD_UNUSED(enabled);

    if (pSettings->mode < NEXUS_StandbyMode_ePassive) {
        if (!pDevice->isTunerPoweredOn) {
            pwrSettings.enable = false;
            rc = BTNR_SetPowerSaver(pDevice->terrestrial.tnr[0], &pwrSettings);
            if (rc) { rc = BERR_TRACE(rc); }
            pDevice->isTunerPoweredOn = true;
        }
        switch (pDevice->lastChannel)
        {
        case NEXUS_75525_ISDBT_CHN:
        case NEXUS_75525_DVBT_CHN:
            if (pDevice->isPoweredOn[pDevice->lastChannel]) {
                rc = BODS_DisablePowerSaver(pDevice->terrestrial.ods_chn[pDevice->lastChannel], &odsPwrSettings);
                if (rc) { rc = BERR_TRACE(rc); goto done; }
                pDevice->isPoweredOn[pDevice->lastChannel] = true;
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
        case NEXUS_75525_ISDBT_CHN:
        case NEXUS_75525_DVBT_CHN:
            if (pDevice->isPoweredOn[pDevice->lastChannel]) {
                rc = BODS_EnablePowerSaver(pDevice->terrestrial.ods_chn[pDevice->lastChannel], &odsPwrSettings);
                if (rc) { rc = BERR_TRACE(rc); goto done; }
                pDevice->isPoweredOn[pDevice->lastChannel] = false;
            }
            break;
        default:
            BDBG_ERR((" Unsupported channel."));
            rc = BERR_TRACE(BERR_NOT_SUPPORTED);
            break;
        }
        if (pDevice->isTunerPoweredOn) {
            pwrSettings.enable = true;
            rc = BTNR_SetPowerSaver(pDevice->terrestrial.tnr[0], &pwrSettings);
            if (rc) { rc = BERR_TRACE(rc); goto done; }
            pDevice->isTunerPoweredOn = false;
        }
    }

done:
    return rc;
}
