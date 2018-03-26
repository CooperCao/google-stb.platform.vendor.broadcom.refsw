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
 ***************************************************************************/

/* Begin Includes */
#include "nexus_frontend_7364_priv.h"
#if !NEXUS_FRONTEND_7364_DISABLE_TERRESTRIAL
#include "bchp_leap_host_l1.h"
#include "bhab_leap_priv.h"
#endif
/* End includes */

BDBG_MODULE(nexus_frontend_7364_terrestrial);

#if !NEXUS_FRONTEND_7364_DISABLE_TERRESTRIAL
static void NEXUS_Frontend_P_7364_ResetStatus(void *handle);
static void NEXUS_FrontendDevice_P_7364_UninstallCallbacks(void *handle);

static NEXUS_Error NEXUS_Frontend_P_7364_Terrestrial_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings);

#define BLEAP_HOST_L1_INTERRUPT_ID     BCHP_INT_ID_CREATE(BCHP_LEAP_HOST_L1_INTR_W0_STATUS, BCHP_LEAP_HOST_L1_INTR_W0_STATUS_LEAP_INTR_SHIFT)

static void NEXUS_Frontend_P_7364_AsyncStatusCallback_isr(void *pParam)
{
    NEXUS_7364Device *pDevice;
    BDBG_ASSERT(pParam != NULL);
    pDevice = (NEXUS_7364Device *)pParam;

    if (pDevice->asyncStatusAppCallback[pDevice->lastChannel])
    {
        pDevice->isStatusReady = true;
        NEXUS_IsrCallback_Fire_isr(pDevice->asyncStatusAppCallback[pDevice->lastChannel]);
    }
}

static void NEXUS_Frontend_P_7364_callback_isr(void *pParam)
{
    NEXUS_7364Device *pDevice;
    BDBG_ASSERT(pParam != NULL);
    pDevice = (NEXUS_7364Device *)pParam;

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

static void NEXUS_Frontend_P_7364_IsrControl_isr(bool enable, void *pParam)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned *isrNumber = (unsigned *)pParam;

    BDBG_MSG(("NEXUS_Frontend_P_7364_IsrControl_isr: (%p) enabled: %s", pParam, enable ? "true" : "false"));
    if ( enable )
    {
        rc = NEXUS_Core_EnableInterrupt_isr(*isrNumber);
        if(rc) BERR_TRACE(rc);
    }
    else
    {
        NEXUS_Core_DisableInterrupt_isr(*isrNumber);
    }
}

static void NEXUS_Frontend_P_7364_L1_isr(void *pParam1, int pParam2)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7364Device *pDevice = (NEXUS_7364Device *)pParam1;
    BHAB_Handle hab = pDevice->hab;
    BSTD_UNUSED(pParam2);

    BDBG_MSG(("NEXUS_Frontend_P_7364_L1_isr: (%p,%p)", pParam1, (void *)hab));
    if(hab){
        rc = BHAB_HandleInterrupt_isr(hab);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
done:
    return;
}

static void NEXUS_Frontend_P_7364_IsrEvent(void *pParam)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHAB_Handle hab = (BHAB_Handle)pParam;

    BDBG_MSG(("NEXUS_Frontend_P_7364_IsrEvent: %p",(void *)hab));
    if(hab){
        rc = BHAB_ProcessInterruptEvent(hab);
        if(rc) BERR_TRACE(rc);
    }
}

void NEXUS_FrontendDevice_P_Uninit_7364_Hab(NEXUS_7364Device *pDevice)
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

    if(pDevice->isrEvent) pDevice->isrEvent = NULL;
    if(pDevice->isrEventCallback) NEXUS_UnregisterEvent(pDevice->isrEventCallback);
    pDevice->isrEventCallback = NULL;
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

NEXUS_Error NEXUS_FrontendDevice_P_Init_7364_Hab(NEXUS_7364Device *pDevice, const NEXUS_FrontendDeviceOpenSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHAB_Settings stHabSettings;

    rc = BHAB_7364_GetDefaultSettings(&stHabSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if(pSettings->isrNumber) {
        stHabSettings.interruptEnableFunc = NEXUS_Frontend_P_7364_IsrControl_isr;
        stHabSettings.interruptEnableFuncParam = (void*)&pSettings->isrNumber;
        stHabSettings.pChp = g_pCoreHandles->chp;
    }

    rc = BHAB_Open( &pDevice->hab, (void *)g_pCoreHandles->reg, &stHabSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

#if 0
    /* Success opeining Hab.  Connect Interrupt */
    if(pSettings->isrNumber) {
        rc = NEXUS_Core_ConnectInterrupt(pSettings->isrNumber, NEXUS_Frontend_P_7364_L1_isr, (void *)pDevice, (int)pDevice->hab);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    else if(pSettings->gpioInterrupt){
        NEXUS_Gpio_SetInterruptCallback_priv(pSettings->gpioInterrupt, NEXUS_Frontend_P_7364_L1_isr, (void *)pDevice, (int)pDevice->hab);
    }
#else
    rc = BINT_CreateCallback(&(pDevice->cbHandle), g_pCoreHandles->bint, BLEAP_HOST_L1_INTERRUPT_ID, NEXUS_Frontend_P_7364_L1_isr, (void *)pDevice, 0);
    BDBG_ASSERT(rc == BERR_SUCCESS);

    rc = BINT_EnableCallback(pDevice->cbHandle);
    BDBG_ASSERT(rc == BERR_SUCCESS);
#endif

    {
        NEXUS_MemoryAllocationSettings memSettings;
        BHAB_7364_Settings hab7364Settings;

        BDBG_MSG(("Configuring 7364 with external memory"));
        NEXUS_Memory_GetDefaultAllocationSettings(&memSettings);
        memSettings.alignment = 0x100000;
        rc = NEXUS_Memory_Allocate(0x100000,&memSettings,&pDevice->leapBuffer);
        if(rc){BDBG_ERR(("Unable to allocate memory for 7364 LEAP"));rc = BERR_TRACE(rc); goto done;}

        hab7364Settings.bUseInternalMemory = false;
        hab7364Settings.pRam = pDevice->leapBuffer;
        hab7364Settings.physAddr = NEXUS_AddrToOffset(pDevice->leapBuffer);
        hab7364Settings.heap = NEXUS_Core_P_AddressToHeap(pDevice->leapBuffer, NULL);

        BDBG_MSG(("pRam: %p, physAddr: 0x%08x",hab7364Settings.pRam,hab7364Settings.physAddr));
        rc = BHAB_7364_Configure(pDevice->hab, &hab7364Settings);
         if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    /* Get events and register callbacks */
    rc = BHAB_GetInterruptEventHandle(pDevice->hab, &pDevice->isrEvent);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pDevice->isrEventCallback = NEXUS_RegisterEvent(pDevice->isrEvent, NEXUS_Frontend_P_7364_IsrEvent, pDevice->hab);
    if ( NULL == pDevice->isrEventCallback ){rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done; }

    if (pSettings->loadAP)
    {
        uint8_t *fw = NULL;
        const uint8_t *fw_image = NULL;
#if NEXUS_MODE_driver
        {
            unsigned fw_size = 0;
            BIMG_Interface imgInterface;
            void *pImgContext;
            void *pImg;
            uint8_t *pImage;
            unsigned header_size = 313;
            unsigned code_size = 0;
            unsigned num_chunks, chunk_size = MAX_CTFE_IMG_CHUNK_SIZE;
            unsigned chunk;
            NEXUS_MemoryAllocationSettings allocSettings;

            rc = Nexus_Core_P_Img_Create(NEXUS_CORE_IMG_ID_FRONTEND_7364, &pImgContext, &imgInterface);
            if (rc) { BERR_TRACE(rc); goto done; }
            rc = imgInterface.open((void*)pImgContext, &pImg, 0);
            if (rc) { BERR_TRACE(rc); goto done; }
            rc = imgInterface.next(pImg, 0, (const void **)&pImage, header_size);
            if (rc) { BERR_TRACE(rc); goto done; }
            code_size = (pImage[72] << 24) | (pImage[73] << 16) | (pImage[74] << 8) | pImage[75];
            fw_size = code_size + header_size;
            NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
            allocSettings.heap = g_pCoreHandles->heap[0].nexus;
            rc = NEXUS_Memory_Allocate(fw_size, &allocSettings, (void **)&fw);
            if (rc) { BERR_TRACE(rc); goto done; }

            num_chunks = fw_size / chunk_size;
            if (fw_size % chunk_size != 0) num_chunks++;

            BKNI_Memset(fw, 0, fw_size);
            for (chunk=0; chunk < num_chunks; chunk++) {
                unsigned num_to_read = chunk_size;
                if (chunk==num_chunks-1) num_to_read = fw_size % chunk_size;
                rc = imgInterface.next(pImg, chunk, (const void **)&pImage, num_to_read);
                if (rc) { BERR_TRACE(rc); goto done; }
                BKNI_Memcpy(fw + (chunk*chunk_size), pImage, num_to_read);
            }
            imgInterface.close(pImg);
            fw_image = fw;
        }
#else
        BSTD_UNUSED(fw);
        fw_image = bcm7364_leap_image;
#endif
        rc = BHAB_InitAp(pDevice->hab, fw_image);
#if NEXUS_MODE_driver
        NEXUS_Memory_Free(fw);
#endif
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    return BERR_SUCCESS;

done:
    NEXUS_FrontendDevice_P_Uninit_7364_Hab(pDevice);
    return rc;
}

void NEXUS_FrontendDevice_P_Uninit7364(NEXUS_7364Device *pDevice)
{
    NEXUS_Error rc = BERR_SUCCESS;
    unsigned int i;

    for ( i = 0; i < NEXUS_MAX_7364_TUNERS && NULL != pDevice->tnr[i]; i++) {
        if(pDevice->tnr[i]){
            rc = BTNR_Close(pDevice->tnr[i]);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->tnr[i] = NULL;
        }
    }
    for ( i = 0; i < NEXUS_7364_MAX_OFDM_CHN && NULL != pDevice->ods_chn[i]; i++) {
        if(pDevice->ods_chn[i]){
            rc = BODS_CloseChannel(pDevice->ods_chn[i]);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            (pDevice->ods_chn[i]) = NULL;
        }
        if(pDevice->tfe_chn[0]){
            rc = BTFE_CloseChannel(pDevice->tfe_chn[0]);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            (pDevice->tfe_chn[0]) = NULL;
        }
    }
    if (pDevice->ods) {
        rc = BODS_Close(pDevice->ods);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pDevice->ods = NULL;
    }
    if (pDevice->tfe) {
        rc = BTFE_Close(pDevice->tfe);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pDevice->tfe = NULL;
    }
done:
    return;
}

static NEXUS_Error NEXUS_FrontendDevice_P_Init7364(NEXUS_7364Device *pDevice)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned int i, j, num_ch;
    BTNR_7364Ib_Settings tnr7364_cfg;
    BODS_Settings odsCfg;
    BODS_ChannelSettings odsChnCfg;
    unsigned acc_chn_num = 0;
    unsigned numChannels;
    BTFE_Settings tfeSettings;
    BTFE_ChannelSettings channelSettings;

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

    rc = BODS_7364_GetDefaultSettings( &odsCfg, NULL);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    odsCfg.hGeneric = pDevice->hab;
    rc = BODS_Open(&pDevice->ods, NULL, NULL, NULL, &odsCfg);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    rc = BODS_Init(pDevice->ods);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    for (i=0;i<BODS_Standard_eLast;i++) {
        rc = BODS_GetTotalChannels(pDevice->ods, (BODS_Standard)i, &num_ch);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        if(num_ch > NEXUS_MAX_7364_TUNERS) {
            BDBG_ERR(("The maximum number of channels is incorrect. num_ch = %d", num_ch));
            rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
        }

        rc = BODS_GetChannelDefaultSettings( pDevice->ods, (BODS_Standard)i, &odsChnCfg);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        if((i == BODS_Standard_eDvbt2) || (i == BODS_Standard_eIsdbt)){
            odsChnCfg.hHeap = g_pCoreHandles->heap[0].mem;
        }
        for (j=0;j<num_ch;j++) {
            odsChnCfg.channelNo = j;
            if(acc_chn_num + j >= NEXUS_7364_MAX_OFDM_CHN) {
                BDBG_ERR(("The maximum number of total ODS channels %d is more than expected. ", acc_chn_num + j));
                rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
            }
            rc = BODS_OpenChannel( pDevice->ods, &pDevice->ods_chn[acc_chn_num + j], &odsChnCfg);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            rc = BODS_InstallCallback(pDevice->ods_chn[acc_chn_num + j], BODS_Callback_eLockChange, (BODS_CallbackFunc)NEXUS_Frontend_P_7364_callback_isr, (void*)pDevice);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            rc = BODS_InstallCallback(pDevice->ods_chn[acc_chn_num + j], BODS_Callback_eAsyncStatusReady, (BODS_CallbackFunc)NEXUS_Frontend_P_7364_AsyncStatusCallback_isr, (void*)pDevice);
            if(rc){rc = BERR_TRACE(rc); goto done;}
        }

        acc_chn_num += num_ch;
    }

    rc = BTFE_GetDefaultSettings( &tfeSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    tfeSettings.hGeneric = pDevice->hab;
    rc = BTFE_Open(&pDevice->tfe, g_pCoreHandles->chp, g_pCoreHandles->reg, g_pCoreHandles->bint, &tfeSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    rc = BTFE_GetTotalChannels(pDevice->tfe, &numChannels);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    rc = BTFE_GetChannelDefaultSettings(pDevice->tfe, 0, &channelSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    rc = BTFE_OpenChannel(pDevice->tfe, &pDevice->tfe_chn[0], 0, &channelSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    rc = BTFE_Initialize(pDevice->tfe, 0);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    for (i=0;i<NEXUS_MAX_7364_TUNERS;i++) {
        rc = BTNR_7364Ib_GetDefaultSettings(&tnr7364_cfg);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        tnr7364_cfg.channelNo = i;
        rc =  BTNR_7364Ib_Open(&pDevice->tnr[i],&tnr7364_cfg, pDevice->hab);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    return BERR_SUCCESS;

done:
    NEXUS_FrontendDevice_P_Uninit7364(pDevice);
    return rc;
}

static void NEXUS_Frontend_P_7364_UnTune(void *handle)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7364Device *pDevice = (NEXUS_7364Device *)handle;
    BODS_PowerSaverSettings pwrSettings;
    unsigned chn_num=0;

    BDBG_MSG(("Untune: pDevice = %p", (void *)pDevice));
    BDBG_MSG(("Tuner is not powered down for now to decrease channel change time."));

    switch ( pDevice->lastChannel )
    {
    case NEXUS_7364_VSB_CHN:
        rc = BTFE_EnablePowerSaver(pDevice->tfe_chn[0]); /* TFE handle or channel handle???? */
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pDevice->isPoweredOn[chn_num] = false;
        break;
    case NEXUS_7364_ISDBT_CHN:
    case NEXUS_7364_DVBT_CHN:
    case NEXUS_7364_DVBT2_CHN:
        if(pDevice->isPoweredOn[pDevice->lastChannel]) {
            rc = BODS_EnablePowerSaver(pDevice->ods_chn[pDevice->lastChannel], &pwrSettings);
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

static void NEXUS_Frontend_P_7364_Close(NEXUS_FrontendHandle handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7364Device *pDevice;
    unsigned chn_num=0;
    BTNR_PowerSaverSettings pwrSettings;

    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    pDevice = (NEXUS_7364Device *) handle->pDeviceHandle;

    for(chn_num=0; chn_num< NEXUS_MAX_7364_T_FRONTENDS; chn_num++){
        pDevice->lastChannel = chn_num;
        if(pDevice->isPoweredOn[chn_num]) {
            NEXUS_Frontend_P_7364_UnTune(pDevice);
        }
    }
    if(pDevice->isTunerPoweredOn){
        pwrSettings.enable = true;
        rc = BTNR_SetPowerSaver(pDevice->tnr[0], &pwrSettings);
        if(rc){rc = BERR_TRACE(rc);}
    }

    for(chn_num=0; chn_num < NEXUS_MAX_7364_T_FRONTENDS; chn_num++){
        if ( pDevice->lockAppCallback[chn_num] ){
            NEXUS_IsrCallback_Destroy(pDevice->lockAppCallback[chn_num]);
        }
        if(pDevice->ods_chn[chn_num]){
            BODS_InstallCallback(pDevice->ods_chn[chn_num], BODS_Callback_eLockChange, NULL, NULL);
            BODS_InstallCallback(pDevice->ods_chn[chn_num], BODS_Callback_eAsyncStatusReady, NULL, NULL);
        }
        if(pDevice->tfe_chn[0]){
            BTFE_InstallCallback(pDevice->tfe_chn[0], BTFE_Callback_eLockChange, NULL, NULL);
            BTFE_InstallCallback(pDevice->tfe_chn[0], BTFE_Callback_eAsyncStatusReady, NULL, NULL);
        }
        if ( pDevice->asyncStatusAppCallback[chn_num] ){
             NEXUS_IsrCallback_Destroy(pDevice->asyncStatusAppCallback[chn_num]);
        }
    }
    if ( pDevice->updateGainAppCallback[0] )
        NEXUS_IsrCallback_Destroy(pDevice->updateGainAppCallback[0]);

    NEXUS_Frontend_P_Destroy(handle);

    pDevice->frontendHandle = NULL;
}

NEXUS_Error NEXUS_FrontendDevice_P_Init7364_Terrestrial(NEXUS_7364Device *pDevice)
{
    NEXUS_Error rc;
    rc = NEXUS_FrontendDevice_P_Init_7364_Hab(pDevice, &pDevice->openSettings);
    if (!rc)
        rc = NEXUS_FrontendDevice_P_Init7364(pDevice);

    /* restore LNA */
    {
        BHAB_ConfigSettings habConfigSettings;
        rc = BHAB_GetConfigSettings(pDevice->hab, &habConfigSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        habConfigSettings.daisyChain = pDevice->rfDaisyChain;
        habConfigSettings.enableLoopThrough = pDevice->enableRfLoopThrough;

        habConfigSettings.rfInputMode = pDevice->rfInput;
        habConfigSettings.tnrApplication = BHAB_TunerApplication_eTerrestrial;

        rc = BHAB_SetConfigSettings(pDevice->hab, &habConfigSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
done:
    return rc;
}

void NEXUS_FrontendDevice_P_Uninit7364_Terrestrial(NEXUS_7364Device *pDevice)
{
    NEXUS_FrontendDevice_P_7364_UninstallCallbacks(pDevice);
    NEXUS_FrontendDevice_P_Uninit7364(pDevice);
    NEXUS_FrontendDevice_P_Uninit_7364_Hab(pDevice);
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

static NEXUS_Error NEXUS_Frontend_P_7364_GetFastStatus(void *handle, NEXUS_FrontendFastStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7364Device *pDevice = (NEXUS_7364Device *)handle;
    unsigned lock;

    switch ( pDevice->lastChannel )
    {
    case NEXUS_7364_ISDBT_CHN:
    case NEXUS_7364_DVBT2_CHN:
    case NEXUS_7364_DVBT_CHN:
        rc = BODS_GetLockStatus(pDevice->ods_chn[pDevice->lastChannel],  &lock);
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

static void NEXUS_FrontendDevice_P_7364_UninstallCallbacks(void *handle)
{
    NEXUS_7364Device *pDevice = (NEXUS_7364Device *)handle;

    if(pDevice->lockAppCallback[pDevice->lastChannel])NEXUS_IsrCallback_Set(pDevice->lockAppCallback[pDevice->lastChannel], NULL);
    if(pDevice->asyncStatusAppCallback[pDevice->lastChannel])NEXUS_IsrCallback_Set(pDevice->asyncStatusAppCallback[pDevice->lastChannel], NULL);
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

static BODS_SelectiveAsyncStatusType NEXUS_Frontend_P_7364_t2StatusTypeToOds(NEXUS_FrontendDvbt2StatusType type)
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

static NEXUS_Error NEXUS_Frontend_P_7364_RequestDvbt2AsyncStatus(void *handle, NEXUS_FrontendDvbt2StatusType type)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7364Device *pDevice;
    BODS_SelectiveAsyncStatusType statusType;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7364Device *)handle;

    if(pDevice->lastChannel != NEXUS_7364_DVBT2_CHN){
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto done;
    }

    statusType = NEXUS_Frontend_P_7364_t2StatusTypeToOds(type);

    rc = BODS_RequestSelectiveAsyncStatus(pDevice->ods_chn[NEXUS_7364_DVBT2_CHN], statusType);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7364_GetDvbt2AsyncStatusReady(void *handle, NEXUS_FrontendDvbt2StatusReady *pStatusReady)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    BODS_SelectiveAsyncStatusReadyType readyType;
    NEXUS_7364Device *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7364Device *)handle;

    BKNI_Memset(pStatusReady, 0, sizeof(NEXUS_FrontendDvbt2StatusReady));
    BKNI_Memset(&readyType, 0, sizeof(readyType));

    if(pDevice->lastChannel != NEXUS_7364_DVBT2_CHN){
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto done;
    }

    rc = BODS_GetSelectiveAsyncStatusReadyType(pDevice->ods_chn[NEXUS_7364_DVBT2_CHN], &readyType);
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

static NEXUS_Error NEXUS_Frontend_P_7364_GetDvbt2AsyncFecStatistics(void *handle, NEXUS_FrontendDvbt2StatusType type, NEXUS_FrontendDvbt2FecStatistics *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    BODS_SelectiveAsyncStatusType statusType;
    NEXUS_7364Device *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7364Device *)handle;

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    BKNI_Memset(&pDevice->odsStatus, 0, sizeof(pDevice->odsStatus));

    statusType = NEXUS_Frontend_P_7364_t2StatusTypeToOds(type);

    rc = BODS_GetSelectiveAsyncStatus(pDevice->ods_chn[NEXUS_7364_DVBT2_CHN], statusType, &pDevice->odsStatus);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if(statusType != pDevice->odsStatus.type){
        BDBG_ERR(("Requested nexus status type %d does not match the returned pi status type %d.",type, pDevice->odsStatus.type));
        rc = BERR_TRACE(NEXUS_UNKNOWN); goto done;
    }

    switch ( pDevice->odsStatus.type )
    {
    case BODS_SelectiveAsyncStatusType_eDvbt2FecStatisticsL1Pre:
    case BODS_SelectiveAsyncStatusType_eDvbt2FecStatisticsL1Post:
    case BODS_SelectiveAsyncStatusType_eDvbt2FecStatisticsPlpA:
    case BODS_SelectiveAsyncStatusType_eDvbt2FecStatisticsPlpB:
        pStatus->lock = pDevice->odsStatus.status.dvbt2FecStatistics.lock;
        pStatus->snrData = pDevice->odsStatus.status.dvbt2FecStatistics.snrData/256;
        pStatus->ldpcAvgIter = pDevice->odsStatus.status.dvbt2FecStatistics.ldpcAvgIter;
        pStatus->ldpcTotIter = pDevice->odsStatus.status.dvbt2FecStatistics.ldpcTotIter;
        pStatus->ldpcTotFrm = pDevice->odsStatus.status.dvbt2FecStatistics.ldpcTotFrm;
        pStatus->ldpcUncFrm = pDevice->odsStatus.status.dvbt2FecStatistics.ldpcUncFrm;
        pStatus->ldpcBER = pDevice->odsStatus.status.dvbt2FecStatistics.ldpcBER;
        pStatus->bchCorBit = pDevice->odsStatus.status.dvbt2FecStatistics.bchCorBit;
        pStatus->bchTotBlk = pDevice->odsStatus.status.dvbt2FecStatistics.bchTotBlk;
        pStatus->bchClnBlk = pDevice->odsStatus.status.dvbt2FecStatistics.bchClnBlk;
        pStatus->bchCorBlk = pDevice->odsStatus.status.dvbt2FecStatistics.bchCorBlk;
        pStatus->bchUncBlk = pDevice->odsStatus.status.dvbt2FecStatistics.bchUncBlk;
        break;
    default:
        BDBG_ERR((" Unsupported status type."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED);
    }
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7364_GetDvbt2AsyncL1PreStatus(void *handle, NEXUS_FrontendDvbt2L1PreStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7364Device *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7364Device *)handle;

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    BKNI_Memset(&pDevice->odsStatus, 0, sizeof(pDevice->odsStatus));

    rc = BODS_GetSelectiveAsyncStatus(pDevice->ods_chn[NEXUS_7364_DVBT2_CHN], BODS_SelectiveAsyncStatusType_eDvbt2L1Pre, &pDevice->odsStatus);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if(BODS_SelectiveAsyncStatusType_eDvbt2L1Pre != pDevice->odsStatus.type){
        BDBG_ERR(("Requested nexus status type BODS_SelectiveAsyncStatusType_eDvbt2L1Pre does not match the returned pi status type %d.", pDevice->odsStatus.type));
        rc = BERR_TRACE(NEXUS_UNKNOWN); goto done;
    }

    pStatus->streamType = pDevice->odsStatus.status.dvbt2L1Pre.streamType;
    pStatus->bwtExt = pDevice->odsStatus.status.dvbt2L1Pre.bwtExt;
    pStatus->s1 = pDevice->odsStatus.status.dvbt2L1Pre.s1;
    pStatus->s2 = pDevice->odsStatus.status.dvbt2L1Pre.s2;
    pStatus->l1RepetitionFlag = pDevice->odsStatus.status.dvbt2L1Pre.l1RepetitionFlag;
    pStatus->guardInterval= pDevice->odsStatus.status.dvbt2L1Pre.guardInterval;
    pStatus->papr = pDevice->odsStatus.status.dvbt2L1Pre.papr;
    pStatus->l1Mod = pDevice->odsStatus.status.dvbt2L1Pre.l1Modulation;
    pStatus->l1CodeRate = pDevice->odsStatus.status.dvbt2L1Pre.l1CodeRate;
    pStatus->l1FecType = pDevice->odsStatus.status.dvbt2L1Pre.l1FecType;
    pStatus->pilotPattern = pDevice->odsStatus.status.dvbt2L1Pre.pilotPattern;
    pStatus->regenFlag = pDevice->odsStatus.status.dvbt2L1Pre.regenFlag;
    pStatus->l1PostExt = pDevice->odsStatus.status.dvbt2L1Pre.l1PostExt;
    pStatus->numRf = pDevice->odsStatus.status.dvbt2L1Pre.numRf;
    pStatus->currentRfIndex = pDevice->odsStatus.status.dvbt2L1Pre.currentRfIndex;
    pStatus->txIdAvailability = pDevice->odsStatus.status.dvbt2L1Pre.txIdAvailability;
    pStatus->numT2Frames = pDevice->odsStatus.status.dvbt2L1Pre.numT2Frames;
    pStatus->numDataSymbols = pDevice->odsStatus.status.dvbt2L1Pre.numDataSymbols;
    pStatus->cellId = pDevice->odsStatus.status.dvbt2L1Pre.cellId;
    pStatus->networkId = pDevice->odsStatus.status.dvbt2L1Pre.networkId;
    pStatus->t2SystemId = pDevice->odsStatus.status.dvbt2L1Pre.t2SystemId;
    pStatus->l1PostSize = pDevice->odsStatus.status.dvbt2L1Pre.l1PostSize;
    pStatus->l1PostInfoSize = pDevice->odsStatus.status.dvbt2L1Pre.l1PostInfoSize;

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7364_GetDvbt2AsyncL1PostConfigurableStatus(void *handle, NEXUS_FrontendDvbt2L1PostConfigurableStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7364Device *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7364Device *)handle;

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    BKNI_Memset(&pDevice->odsStatus, 0, sizeof(pDevice->odsStatus));

    rc = BODS_GetSelectiveAsyncStatus(pDevice->ods_chn[NEXUS_7364_DVBT2_CHN], BODS_SelectiveAsyncStatusType_eDvbt2L1PostConfigurable, &pDevice->odsStatus);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if(BODS_SelectiveAsyncStatusType_eDvbt2L1PostConfigurable != pDevice->odsStatus.type){
        BDBG_ERR(("Requested nexus status type BODS_SelectiveAsyncStatusType_eDvbt2L1PostConfigurable does not match the returned pi status type %d.", pDevice->odsStatus.type));
        rc = BERR_TRACE(NEXUS_UNKNOWN); goto done;
    }

    pStatus->subSlicesPerFrame = pDevice->odsStatus.status.dvbt2L1PostConfigurable.subSlicesPerFrame;
    pStatus->numPlp = pDevice->odsStatus.status.dvbt2L1PostConfigurable.numPlp;
    pStatus->numAux = pDevice->odsStatus.status.dvbt2L1PostConfigurable.numAux;
    pStatus->fefType = pDevice->odsStatus.status.dvbt2L1PostConfigurable.fefType;
    pStatus->rfIdx = pDevice->odsStatus.status.dvbt2L1PostConfigurable.rfIdx;
    pStatus->fefInterval = pDevice->odsStatus.status.dvbt2L1PostConfigurable.fefInterval;
    pStatus->frequency = pDevice->odsStatus.status.dvbt2L1PostConfigurable.frequency;
    pStatus->fefLength = pDevice->odsStatus.status.dvbt2L1PostConfigurable.fefLength;
    pStatus->auxStreamType = pDevice->odsStatus.status.dvbt2L1PostConfigurable.auxStreamType;
    pStatus->auxPrivateConf = pDevice->odsStatus.status.dvbt2L1PostConfigurable.auxPrivateConf;

    pStatus->plpA.plpId = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.plpId;
    pStatus->plpA.plpGroupId = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.plpGroupId;
    pStatus->plpA.plpType = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.plpType;
    pStatus->plpA.plpPayloadType = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.plpPayloadType;
    pStatus->plpA.ffFlag = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.ffFlag;
    pStatus->plpA.firstRfIdx = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.firstRfIdx;
    pStatus->plpA.firstFrameIdx = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.firstFrameIdx;
    pStatus->plpA.plpCodeRate = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.codeRate;
    pStatus->plpA.plpMod = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.modulation;
    pStatus->plpA.plpRotation = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.plpRotation;
    pStatus->plpA.plpFecType = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.plpFecType;
    pStatus->plpA.plpNumBlocksMax = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.plpNumBlocksMax;
    pStatus->plpA.frameInterval = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.frameInterval;
    pStatus->plpA.timeIlLength = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.timeIlLength;
    pStatus->plpA.timeIlType= pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.timeIlType;
    pStatus->plpA.inBandFlag = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.inBandFlag;

    pStatus->plpB.plpId = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.plpId;
    pStatus->plpB.plpGroupId = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.plpGroupId;
    pStatus->plpB.plpType = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.plpType;
    pStatus->plpB.plpPayloadType = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.plpPayloadType;
    pStatus->plpB.ffFlag = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.ffFlag;
    pStatus->plpB.firstRfIdx = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.firstRfIdx;
    pStatus->plpB.firstFrameIdx = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.firstFrameIdx;
    pStatus->plpB.plpCodeRate = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.codeRate;
    pStatus->plpB.plpMod = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.modulation;
    pStatus->plpB.plpRotation = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.plpRotation;
    pStatus->plpB.plpFecType = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.plpFecType;
    pStatus->plpB.plpNumBlocksMax = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.plpNumBlocksMax;
    pStatus->plpB.frameInterval = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.frameInterval;
    pStatus->plpB.timeIlLength = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.timeIlLength;
    pStatus->plpB.timeIlType= pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.timeIlType;
    pStatus->plpB.inBandFlag = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.inBandFlag;

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7364_GetDvbt2AsyncPostDynamicStatus(void *handle, NEXUS_FrontendDvbt2L1PostDynamicStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7364Device *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7364Device *)handle;

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    BKNI_Memset(&pDevice->odsStatus, 0, sizeof(pDevice->odsStatus));

    rc = BODS_GetSelectiveAsyncStatus(pDevice->ods_chn[NEXUS_7364_DVBT2_CHN], BODS_SelectiveAsyncStatusType_eDvbt2L1PostDynamic, &pDevice->odsStatus);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if(BODS_SelectiveAsyncStatusType_eDvbt2L1PostDynamic != pDevice->odsStatus.type){
        BDBG_ERR(("Requested nexus status type BODS_SelectiveAsyncStatusType_eDvbt2L1PostDynamic does not match the returned pi status type %d.", pDevice->odsStatus.type));
        rc = BERR_TRACE(NEXUS_UNKNOWN); goto done;
    }

    pStatus->frameIdx = pDevice->odsStatus.status.dvbt2L1PostDynamic.frameIdx;
    pStatus->l1ChanlgeCounter = pDevice->odsStatus.status.dvbt2L1PostDynamic.l1ChanlgeCounter;
    pStatus->startRfIdx = pDevice->odsStatus.status.dvbt2L1PostDynamic.startRfIdx;
    pStatus->subSliceInterval = pDevice->odsStatus.status.dvbt2L1PostDynamic.subSliceInterval;
    pStatus->type2Start = pDevice->odsStatus.status.dvbt2L1PostDynamic.type2Start;
    pStatus->auxPrivateDyn_31_0 = pDevice->odsStatus.status.dvbt2L1PostDynamic.auxPrivateDyn_31_0;
    pStatus->auxPrivateDyn_47_32 = pDevice->odsStatus.status.dvbt2L1PostDynamic.auxPrivateDyn_47_32;
    pStatus->plpA.plpId = pDevice->odsStatus.status.dvbt2L1PostDynamic.plpA.plpId;
    pStatus->plpA.plpNumBlocks = pDevice->odsStatus.status.dvbt2L1PostDynamic.plpA.plpNumBlocks;
    pStatus->plpA.plpStart = pDevice->odsStatus.status.dvbt2L1PostDynamic.plpA.plpNumBlocks;
    pStatus->plpB.plpId = pDevice->odsStatus.status.dvbt2L1PostDynamic.plpB.plpId;
    pStatus->plpB.plpNumBlocks = pDevice->odsStatus.status.dvbt2L1PostDynamic.plpB.plpNumBlocks;
    pStatus->plpB.plpStart = pDevice->odsStatus.status.dvbt2L1PostDynamic.plpB.plpNumBlocks;
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7364_GetDvbt2AsyncL1PlpStatus(void *handle, NEXUS_FrontendDvbt2L1PlpStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    unsigned i=0;
    NEXUS_7364Device *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7364Device *)handle;

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    BKNI_Memset(&pDevice->odsStatus, 0, sizeof(pDevice->odsStatus));

    rc = BODS_GetSelectiveAsyncStatus(pDevice->ods_chn[NEXUS_7364_DVBT2_CHN], BODS_SelectiveAsyncStatusType_eDvbt2L1Plp, &pDevice->odsStatus);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if(BODS_SelectiveAsyncStatusType_eDvbt2L1Plp != pDevice->odsStatus.type){
        BDBG_ERR(("Requested nexus status type BODS_SelectiveAsyncStatusType_eDvbt2L1Plp does not match the returned pi status type %d.", pDevice->odsStatus.type));
        rc = BERR_TRACE(NEXUS_UNKNOWN); goto done;
    }

    pStatus->numPlp = pDevice->odsStatus.status.dvbt2L1Plp.numPlp;
    for(i=0; i<pStatus->numPlp; i++) {
        pStatus->plp[i].plpId = pDevice->odsStatus.status.dvbt2L1Plp.plp[i].plpId;
        pStatus->plp[i].plpGroupId = pDevice->odsStatus.status.dvbt2L1Plp.plp[i].plpGroupId;
        pStatus->plp[i].plpPayloadType = pDevice->odsStatus.status.dvbt2L1Plp.plp[i].plpPayloadType;
        pStatus->plp[i].plpType = pDevice->odsStatus.status.dvbt2L1Plp.plp[i].plpType;
    }
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7364_GetDvbt2AsyncBasicStatus(void *handle, NEXUS_FrontendDvbt2BasicStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7364Device *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7364Device *)handle;

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    BKNI_Memset(&pDevice->odsStatus, 0, sizeof(pDevice->odsStatus));

    rc = BODS_GetSelectiveAsyncStatus(pDevice->ods_chn[NEXUS_7364_DVBT2_CHN], BODS_SelectiveAsyncStatusType_eDvbt2Short, &pDevice->odsStatus);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if(BODS_SelectiveAsyncStatusType_eDvbt2Short != pDevice->odsStatus.type){
        BDBG_ERR(("Requested nexus status type BODS_SelectiveAsyncStatusType_eDvbt2Short does not match the returned pi status type %d.", pDevice->odsStatus.type));
        rc = BERR_TRACE(NEXUS_UNKNOWN); goto done;
    }

    pStatus->fecLock = pDevice->odsStatus.status.dvbt2Short.lock;
    pStatus->spectrumInverted = pDevice->odsStatus.status.dvbt2Short.spectrumInverted;
    pStatus->snr = pDevice->odsStatus.status.dvbt2Short.snrEstimate*100/256;
    pStatus->gainOffset = pDevice->odsStatus.status.dvbt2Short.gainOffset*100/256;
    pStatus->carrierOffset = pDevice->odsStatus.status.dvbt2Short.carrierFreqOffset;
    pStatus->timingOffset = pDevice->odsStatus.status.dvbt2Short.timingOffset;
    pStatus->signalStrength = pDevice->odsStatus.status.dvbt2Short.signalStrength/10;
    pStatus->signalLevelPercent = pDevice->odsStatus.status.dvbt2Short.signalLevelPercent;
    pStatus->signalQualityPercent = pDevice->odsStatus.status.dvbt2Short.signalQualityPercent;
    pStatus->reacquireCount = pDevice->odsStatus.status.dvbt2Short.reacqCount;
    pStatus->profile = pDevice->odsStatus.status.dvbt2Short.profile;

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


static NEXUS_Error NEXUS_Frontend_P_7364_GetOfdmAsyncStatus(void *handle, NEXUS_FrontendOfdmStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_FrontendDvbt2StatusReady t2StatusReady;
    NEXUS_FrontendDvbt2BasicStatus basic;
    NEXUS_7364Device *pDevice = (NEXUS_7364Device *)handle;

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    switch ( pDevice->lastChannel )
    {
    case NEXUS_7364_DVBT2_CHN:
        BKNI_Memset(&pDevice->t2PartialStatus, 0, sizeof(pDevice->t2PartialStatus));

        rc =  NEXUS_Frontend_P_7364_GetDvbt2AsyncStatusReady(pDevice, &t2StatusReady);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        if((t2StatusReady.type[NEXUS_FrontendDvbt2StatusType_eBasic]) && (t2StatusReady.type[NEXUS_FrontendDvbt2StatusType_eFecStatisticsPlpA]) &&
           (t2StatusReady.type[NEXUS_FrontendDvbt2StatusType_eL1Pre]) && (t2StatusReady.type[NEXUS_FrontendDvbt2StatusType_eL1PostConfigurable]))
        {
            rc = NEXUS_Frontend_P_7364_GetDvbt2AsyncBasicStatus(handle, &basic);
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

            rc = NEXUS_Frontend_P_7364_GetDvbt2AsyncFecStatistics(handle, NEXUS_FrontendDvbt2StatusType_eFecStatisticsPlpA, &pStatus->dvbt2Status.plpAStatistics);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            rc = NEXUS_Frontend_P_7364_GetDvbt2AsyncL1PreStatus(handle, &pStatus->dvbt2Status.l1PreStatus);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            rc = NEXUS_Frontend_P_7364_GetDvbt2AsyncL1PostConfigurableStatus(handle, &pStatus->dvbt2Status.l1PostCfgStatus);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            rc = NEXUS_Frontend_P_7364_GetDvbt2AsyncL1PlpStatus(handle, &pStatus->dvbt2Status.l1PlpStatus);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            NEXUS_Frontend_P_PrintOfdmStatus(pStatus);
        }
        else{
            BDBG_ERR(("Status not ready. Error reading status."));
            rc = BERR_TRACE(rc); goto done;
        }
        break;
    case NEXUS_7364_DVBT_CHN:
        BKNI_Memset(&pDevice->odsStatus, 0, sizeof(pDevice->odsStatus));

        rc = BODS_GetSelectiveAsyncStatus(pDevice->ods_chn[NEXUS_7364_DVBT_CHN], BODS_SelectiveAsyncStatusType_eDvbt, &pDevice->odsStatus);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        pStatus->receiverLock = pDevice->odsStatus.status.dvbt.receiverLock;
        pStatus->fecLock = pDevice->odsStatus.status.dvbt.fecLock;
        pStatus->noSignalDetected = pDevice->odsStatus.status.dvbt.noSignalDetected;
        pStatus->transmissionMode = NEXUS_Frontend_P_OdsToTransmissionMode(pDevice->odsStatus.status.dvbt.transmissionMode);
        pStatus->guardInterval = NEXUS_Frontend_P_OdsToGuardInterval(pDevice->odsStatus.status.dvbt.guardInterval);
        pStatus->signalStrength = pDevice->odsStatus.status.dvbt.signalStrength/10;
        pStatus->signalLevelPercent = pDevice->odsStatus.status.dvbt.signalLevelPercent;
        pStatus->signalQualityPercent = pDevice->odsStatus.status.dvbt.signalQualityPercent;
        pStatus->carrierOffset = pDevice->odsStatus.status.dvbt.carrierOffset;
        pStatus->timingOffset = pDevice->odsStatus.status.dvbt.timingOffset;
        pStatus->snr = pDevice->odsStatus.status.dvbt.snr*100/256;
        pStatus->spectrumInverted = pDevice->odsStatus.status.dvbt.spectrumInverted;
        pStatus->reacquireCount = pDevice->odsStatus.status.dvbt.reacqCount;
        pStatus->modulation = NEXUS_Frontend_P_THDToModulation(pDevice->odsStatus.status.dvbt.modulation);
        pStatus->codeRate = NEXUS_Frontend_P_THDToCodeRate(pDevice->odsStatus.status.dvbt.codeRate);
        pStatus->hierarchy = NEXUS_Frontend_P_THDToHierarchy(pDevice->odsStatus.status.dvbt.hierarchy);
        pStatus->cellId = pDevice->odsStatus.status.dvbt.cellId;
        pStatus->fecCorrectedBlocks = pDevice->odsStatus.status.dvbt.rsCorrectedBlocks;
        pStatus->fecUncorrectedBlocks = pDevice->odsStatus.status.dvbt.rsUncorrectedBlocks;
        pStatus->fecCleanBlocks = pDevice->odsStatus.status.dvbt.rsCleanBlocks;
        pStatus->reacquireCount = pDevice->odsStatus.status.dvbt.reacqCount;
        pStatus->viterbiErrorRate = pDevice->odsStatus.status.dvbt.viterbiBer;
        pStatus->preViterbiErrorRate = pDevice->odsStatus.status.dvbt.preViterbiBer;
        break;
    case NEXUS_7364_ISDBT_CHN:
        BKNI_Memset(&pDevice->odsStatus, 0, sizeof(pDevice->odsStatus));

        rc = BODS_GetSelectiveAsyncStatus(pDevice->ods_chn[NEXUS_7364_ISDBT_CHN], BODS_SelectiveAsyncStatusType_eIsdbt, &pDevice->odsStatus);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        pStatus->receiverLock = pDevice->odsStatus.status.isdbt.receiverLock;
        pStatus->fecLock = pDevice->odsStatus.status.isdbt.fecLock;
        pStatus->noSignalDetected = pDevice->odsStatus.status.isdbt.noSignalDetected;
        pStatus->transmissionMode = NEXUS_Frontend_P_OdsToTransmissionMode(pDevice->odsStatus.status.isdbt.transmissionMode);
        pStatus->guardInterval = NEXUS_Frontend_P_OdsToGuardInterval(pDevice->odsStatus.status.isdbt.guardInterval);
        /*pStatus->gainOffset = pDevice->odsStatus.status.isdbt.gainOffset;*/
        pStatus->carrierOffset = pDevice->odsStatus.status.isdbt.carrierOffset;
        pStatus->timingOffset = pDevice->odsStatus.status.isdbt.timingOffset;
        pStatus->signalStrength = pDevice->odsStatus.status.isdbt.signalStrength/10;
        pStatus->snr = pDevice->odsStatus.status.isdbt.snr*100/256;
        pStatus->spectrumInverted = pDevice->odsStatus.status.isdbt.spectrumInverted;
        pStatus->reacquireCount = pDevice->odsStatus.status.isdbt.reacqCount;
        pStatus->ews = pDevice->odsStatus.status.isdbt.ews;
        pStatus->partialReception = pDevice->odsStatus.status.isdbt.partialReception;
        pStatus->fecCorrectedBlocks = pDevice->odsStatus.status.isdbt.layerAStatus.rsCorrectedBlocks + pDevice->odsStatus.status.isdbt.layerBStatus.rsCorrectedBlocks;
        pStatus->fecUncorrectedBlocks = pDevice->odsStatus.status.isdbt.layerAStatus.rsUncorrectedBlocks + pDevice->odsStatus.status.isdbt.layerBStatus.rsUncorrectedBlocks;
        pStatus->fecCleanBlocks = pDevice->odsStatus.status.isdbt.layerAStatus.rsCleanBlocks + pDevice->odsStatus.status.isdbt.layerBStatus.rsCleanBlocks;

        pStatus->modulationA =NEXUS_Frontend_P_ODSToModulation(pDevice->odsStatus.status.isdbt.layerAStatus.modulation);
        pStatus->codeRateA = NEXUS_Frontend_P_ODSToCodeRate(pDevice->odsStatus.status.isdbt.layerAStatus.codeRate);
        pStatus->timeInterleavingA = NEXUS_Frontend_P_ODSToTimeInterleaving(pDevice->odsStatus.status.isdbt.layerAStatus.timeInterleaving);
        pStatus->numSegmentsA = pDevice->odsStatus.status.isdbt.layerAStatus.numSegments;
        pStatus->fecCorrectedBlocksA = pDevice->odsStatus.status.isdbt.layerAStatus.rsCorrectedBlocks;
        pStatus->fecUncorrectedBlocksA = pDevice->odsStatus.status.isdbt.layerAStatus.rsUncorrectedBlocks;
        pStatus->fecCleanBlocksA = pDevice->odsStatus.status.isdbt.layerAStatus.rsCleanBlocks;
        pStatus->signalLevelPercentA = pDevice->odsStatus.status.isdbt.layerAStatus.signalLevelPercent;
        pStatus->signalQualityPercentA = pDevice->odsStatus.status.isdbt.layerAStatus.signalQualityPercent;

        pStatus->modulationB =NEXUS_Frontend_P_ODSToModulation(pDevice->odsStatus.status.isdbt.layerBStatus.modulation);
        pStatus->codeRateB = NEXUS_Frontend_P_ODSToCodeRate(pDevice->odsStatus.status.isdbt.layerBStatus.codeRate);
        pStatus->timeInterleavingB = NEXUS_Frontend_P_ODSToTimeInterleaving(pDevice->odsStatus.status.isdbt.layerBStatus.timeInterleaving);
        pStatus->numSegmentsB = pDevice->odsStatus.status.isdbt.layerBStatus.numSegments;
        pStatus->fecCorrectedBlocksB = pDevice->odsStatus.status.isdbt.layerBStatus.rsCorrectedBlocks;
        pStatus->fecUncorrectedBlocksB = pDevice->odsStatus.status.isdbt.layerBStatus.rsUncorrectedBlocks;
        pStatus->fecCleanBlocksB = pDevice->odsStatus.status.isdbt.layerBStatus.rsCleanBlocks;
        pStatus->signalLevelPercentB = pDevice->odsStatus.status.isdbt.layerBStatus.signalLevelPercent;
        pStatus->signalQualityPercentB = pDevice->odsStatus.status.isdbt.layerBStatus.signalQualityPercent;

        pStatus->modulationC =NEXUS_Frontend_P_ODSToModulation(pDevice->odsStatus.status.isdbt.layerCStatus.modulation);
        pStatus->codeRateC = NEXUS_Frontend_P_ODSToCodeRate(pDevice->odsStatus.status.isdbt.layerCStatus.codeRate);
        pStatus->timeInterleavingC = NEXUS_Frontend_P_ODSToTimeInterleaving(pDevice->odsStatus.status.isdbt.layerCStatus.timeInterleaving);
        pStatus->numSegmentsC = pDevice->odsStatus.status.isdbt.layerCStatus.numSegments;
        pStatus->fecCorrectedBlocksC = pDevice->odsStatus.status.isdbt.layerCStatus.rsCorrectedBlocks;
        pStatus->fecUncorrectedBlocksC = pDevice->odsStatus.status.isdbt.layerCStatus.rsUncorrectedBlocks;
        pStatus->fecCleanBlocksC = pDevice->odsStatus.status.isdbt.layerCStatus.rsCleanBlocks;
        pStatus->signalLevelPercentC = pDevice->odsStatus.status.isdbt.layerCStatus.signalLevelPercent;
        pStatus->signalQualityPercentC = pDevice->odsStatus.status.isdbt.layerCStatus.signalQualityPercent;
        break;
    default:
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }

    pStatus->settings = pDevice->last_ofdm[0];

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7364_RequestOfdmAsyncStatus(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_InternalGainSettings settings;
    NEXUS_GainParameters params;
    NEXUS_ExternalGainSettings externalGainSettings;
    NEXUS_7364Device *pDevice = (NEXUS_7364Device *)handle;

    if(pDevice->pGenericDeviceHandle->parent){

        params.rfInput = pDevice->pGenericDeviceHandle->linkSettings.rfInput;
        params.accumulateTillRootDevice = true;
        params.frequency = pDevice->last_ofdm[0].frequency;

        rc = NEXUS_FrontendDevice_P_GetInternalGain(pDevice->pGenericDeviceHandle->parent, &params, &settings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        externalGainSettings.totalGain = settings.totalVariableGain;

        rc = NEXUS_FrontendDevice_P_SetExternalGain(pDevice->pGenericDeviceHandle, &externalGainSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

    }

    switch ( pDevice->lastChannel )
    {
    case NEXUS_7364_ISDBT_CHN:
        rc = BODS_RequestSelectiveAsyncStatus(pDevice->ods_chn[pDevice->lastChannel], BODS_SelectiveAsyncStatusType_eIsdbt);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        break;
    case NEXUS_7364_DVBT2_CHN:
        rc = NEXUS_Frontend_P_7364_RequestDvbt2AsyncStatus(pDevice, NEXUS_FrontendDvbt2StatusType_eBasic);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = NEXUS_Frontend_P_7364_RequestDvbt2AsyncStatus(pDevice, NEXUS_FrontendDvbt2StatusType_eFecStatisticsPlpA);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = NEXUS_Frontend_P_7364_RequestDvbt2AsyncStatus(pDevice, NEXUS_FrontendDvbt2StatusType_eL1Pre);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = NEXUS_Frontend_P_7364_RequestDvbt2AsyncStatus(pDevice, NEXUS_FrontendDvbt2StatusType_eL1PostConfigurable);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        break;
    case NEXUS_7364_DVBT_CHN:
        rc = BODS_RequestSelectiveAsyncStatus(pDevice->ods_chn[NEXUS_7364_DVBT_CHN], BODS_SelectiveAsyncStatusType_eDvbt);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        break;
    default:
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }
done:
    return rc;
}


static NEXUS_Error NEXUS_Frontend_P_7364_RequestIsdbtAsyncStatus(void *handle, NEXUS_FrontendIsdbtStatusType type)
{
    NEXUS_Error rc = BERR_SUCCESS;
    NEXUS_7364Device *pDevice;
    BSTD_UNUSED(type);
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7364Device *)handle;

    pDevice->isStatusReady = false;

    rc = NEXUS_Frontend_P_7364_RequestOfdmAsyncStatus(handle);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}


static NEXUS_Error NEXUS_Frontend_P_7364_GetIsdbtAsyncStatusReady(void *handle, NEXUS_FrontendIsdbtStatusReady *pAsyncStatusReady)
{
    NEXUS_Error rc = BERR_SUCCESS;
    NEXUS_7364Device *pDevice;

    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7364Device *)handle;

    if(pDevice->isStatusReady){
        pAsyncStatusReady->type[NEXUS_FrontendIsdbtStatusType_eBasic] = true;
    }
    else{
        pAsyncStatusReady->type[NEXUS_FrontendIsdbtStatusType_eBasic] = false;
    }

    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7364_GetIsdbtAsyncStatus(void *handle, NEXUS_FrontendIsdbtStatusType type, NEXUS_FrontendIsdbtStatus *pStatus)
{
    NEXUS_Error rc = BERR_SUCCESS;
    NEXUS_7364Device *pDevice;
    unsigned chn_num;
    BODS_IsdbtStatus  *status;

    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7364Device *)handle;
    chn_num = pDevice->lastChannel;

    if (chn_num >= NEXUS_7364_MAX_OFDM_CHN)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    BKNI_Memset(&pDevice->odsStatus, 0, sizeof(pDevice->odsStatus));

    pStatus->status.basic.settings = pDevice->last_ofdm[chn_num];

    if(type == NEXUS_FrontendIsdbtStatusType_eBasic){
        rc = BODS_GetSelectiveAsyncStatus(pDevice->ods_chn[chn_num], BODS_SelectiveAsyncStatusType_eIsdbt, &pDevice->odsStatus);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        status = &pDevice->odsStatus.status.isdbt;

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
                    NEXUS_Frontend_P_7364_ResetStatus(handle);
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
                    NEXUS_Frontend_P_7364_ResetStatus(handle);
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
                    NEXUS_Frontend_P_7364_ResetStatus(handle);
                }
            }
        }
    }

    pDevice->isStatusReady = false;
done:
    return rc;

}

NEXUS_Error NEXUS_Tuner_P_7364_GetNSetGain(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_GainParameters params;
    NEXUS_InternalGainSettings settings;
    NEXUS_ExternalGainSettings externalGainSettings;
    NEXUS_7364Device *pDevice = (NEXUS_7364Device *)handle;

    BKNI_Memset(&externalGainSettings, 0, sizeof(externalGainSettings));

    if(pDevice->pGenericDeviceHandle->parent){

        params.rfInput = pDevice->pGenericDeviceHandle->linkSettings.rfInput;
        params.accumulateTillRootDevice = true;
        params.frequency = pDevice->last_ofdm[0].frequency;

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

static NEXUS_Error NEXUS_Frontend_P_7364_RequestDvbtAsyncStatus(void *handle, NEXUS_FrontendDvbtStatusType type)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7364Device *pDevice = (NEXUS_7364Device *)handle;
    BSTD_UNUSED(type);

    pDevice->isStatusReady = false;
    rc = NEXUS_Frontend_P_7364_RequestOfdmAsyncStatus(pDevice);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7364_GetDvbtAsyncStatusReady(void *handle, NEXUS_FrontendDvbtStatusReady *pAsyncStatusReady)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7364Device *pDevice = (NEXUS_7364Device *)handle;

    if(pDevice->isStatusReady){
        pAsyncStatusReady->type[NEXUS_FrontendDvbtStatusType_eBasic] = true;
    }
    else{
        pAsyncStatusReady->type[NEXUS_FrontendDvbtStatusType_eBasic] = false;
    }

    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7364_GetDvbtAsyncStatus(void *handle, NEXUS_FrontendDvbtStatusType type, NEXUS_FrontendDvbtStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7364Device *pDevice = (NEXUS_7364Device *)handle;

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    BKNI_Memset(&pDevice->odsStatus, 0, sizeof(pDevice->odsStatus));

    if(type == NEXUS_FrontendDvbtStatusType_eBasic){
        rc = BODS_GetSelectiveAsyncStatus(pDevice->ods_chn[NEXUS_7364_DVBT_CHN], BODS_SelectiveAsyncStatusType_eDvbt, &pDevice->odsStatus);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        pStatus->status.basic.fecLock = pDevice->odsStatus.status.dvbt.fecLock;
        pStatus->status.basic.spectrumInverted = pDevice->odsStatus.status.dvbt.spectrumInverted;
        pStatus->status.basic.snr = pDevice->odsStatus.status.dvbt.snr*100/256;
        pStatus->status.basic.carrierOffset = pDevice->odsStatus.status.dvbt.carrierOffset;
        pStatus->status.basic.timingOffset = pDevice->odsStatus.status.dvbt.timingOffset;
        pStatus->status.basic.gainOffset = pDevice->odsStatus.status.dvbt.gainOffset;
        pStatus->status.basic.signalStrength = pDevice->odsStatus.status.dvbt.signalStrength/10;
        pStatus->status.basic.signalLevelPercent = pDevice->odsStatus.status.dvbt.signalLevelPercent;
        pStatus->status.basic.signalQualityPercent = pDevice->odsStatus.status.dvbt.signalQualityPercent;
        pStatus->status.basic.reacquireCount = pDevice->odsStatus.status.dvbt.reacqCount;
        pStatus->status.basic.viterbiErrorRate.rate = pDevice->odsStatus.status.dvbt.viterbiBer;

        pStatus->status.basic.tps.modulation = NEXUS_Frontend_P_THDToModulation(pDevice->odsStatus.status.dvbt.modulation);
        pStatus->status.basic.tps.transmissionMode = NEXUS_Frontend_P_OdsToTransmissionMode(pDevice->odsStatus.status.dvbt.transmissionMode);
        pStatus->status.basic.tps.guardInterval = NEXUS_Frontend_P_OdsToGuardInterval(pDevice->odsStatus.status.dvbt.guardInterval);
        pStatus->status.basic.tps.codeRate = NEXUS_Frontend_P_THDToCodeRate(pDevice->odsStatus.status.dvbt.codeRate);
        pStatus->status.basic.tps.hierarchy = NEXUS_Frontend_P_THDToHierarchy(pDevice->odsStatus.status.dvbt.hierarchy);
        pStatus->status.basic.tps.cellId = pDevice->odsStatus.status.dvbt.cellId;
        pStatus->status.basic.tps.inDepthSymbolInterleave = pDevice->odsStatus.status.dvbt.inDepthSymbolInterleave;
        pStatus->status.basic.tps.timeSlicing = pDevice->odsStatus.status.dvbt.timeSlicing;
        pStatus->status.basic.tps.mpeFec = pDevice->odsStatus.status.dvbt.mpeFec;

        pStatus->status.basic.fecBlockCounts.corrected = pDevice->odsStatus.status.dvbt.rsCorrectedBlocks;
        pStatus->status.basic.fecBlockCounts.uncorrected = pDevice->odsStatus.status.dvbt.rsUncorrectedBlocks;
        pStatus->status.basic.fecBlockCounts.clean = pDevice->odsStatus.status.dvbt.rsCleanBlocks;
    }

    pStatus->status.basic.settings = pDevice->last_ofdm[0];
    pDevice->isStatusReady = false;
done:
    return rc;

}


static NEXUS_Error NEXUS_Frontend_P_7364_TuneOfdm(void *handle, const NEXUS_FrontendOfdmSettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7364Device *pDevice = (NEXUS_7364Device *)handle;
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
        chn_num = NEXUS_7364_DVBT_CHN;
        break;
    case NEXUS_FrontendOfdmMode_eIsdbt:
        chn_num = NEXUS_7364_ISDBT_CHN;
        break;
    case NEXUS_FrontendOfdmMode_eDvbt2:
        chn_num = NEXUS_7364_DVBT2_CHN;
        break;
    default:
        /* It is IMPORTANT to check this condition here. Because it will not be checked later. */
        BDBG_ERR(("Wrong Ofdm mode selected."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }

    pDevice->frontendHandle->mtsif.inputBand = NEXUS_FRONTEND_7364_OFDM_INPUT_BAND;
    rc = NEXUS_Frontend_P_SetMtsifConfig(pDevice->frontendHandle);
    if (rc) { return BERR_TRACE(rc); }

    /* This is done as DVBT and DVBT2 are in a single core and needs to be powered down*/
    if(pDevice->last_ofdm[0].mode != pSettings->mode){
        NEXUS_Frontend_P_7364_UnTune(handle);
    }

    rc = NEXUS_Tuner_P_7364_GetNSetGain(pDevice);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    rc = BODS_GetDefaultAcquireParams(pDevice->ods_chn[chn_num], &odsParam);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    NEXUS_IsrCallback_Set(pDevice->lockAppCallback[chn_num], &(pSettings->lockCallback));
    NEXUS_IsrCallback_Set(pDevice->asyncStatusAppCallback[chn_num], &(pSettings->asyncStatusReadyCallback));

    if(!pDevice->isPoweredOn[chn_num] || !pDevice->isTunerPoweredOn)
        goto full_acquire;

    if((pSettings->acquisitionMode == NEXUS_FrontendOfdmAcquisitionMode_eScan) && (pDevice->lastAcquisitionMode[chn_num] == NEXUS_FrontendOfdmAcquisitionMode_eScan)){
        temp_frequency = pDevice->last_ofdm[0].frequency;
        pDevice->last_ofdm[0].frequency = pSettings->frequency ;
        temp_mode = pDevice->last_ofdm[0].mode;
        pDevice->last_ofdm[0].mode = pSettings->mode;

        if(!BKNI_Memcmp(pSettings, &pDevice->last_ofdm[0], sizeof(NEXUS_FrontendOfdmSettings))) {
            if (pDevice->tnr[0])
            {
                pDevice->acquireInProgress = true;
                pDevice->last_ofdm[0] = *pSettings;
                pDevice->lastChannel = chn_num;
                rc = BTNR_SetTunerRfFreq(pDevice->tnr[0], pSettings->frequency, BTNR_TunerMode_eDigital);
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

    if (pDevice->tnr[0])
    {
        if(!pDevice->isTunerPoweredOn){
            tnrPwrSettings.enable = false;
            rc = BTNR_SetPowerSaver(pDevice->tnr[0], &tnrPwrSettings);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->isTunerPoweredOn = true;
        }
        if(!pDevice->isPoweredOn[chn_num]){
            rc = BODS_DisablePowerSaver(pDevice->ods_chn[chn_num], &odsPwrSettings);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->isPoweredOn[chn_num] = true;

        }

        rc = BODS_SetAcquireParams( pDevice->ods_chn[chn_num], &odsParam );
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = BTNR_GetSettings(pDevice->tnr[0], &tnrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        if(pSettings->mode == NEXUS_FrontendOfdmMode_eDvbt2){
            tnrSettings.std = BTNR_Standard_eDvbt2;
        }
        else if(pSettings->mode == NEXUS_FrontendOfdmMode_eDvbt){
            tnrSettings.std = BTNR_Standard_eDvbt;
        }
        else if(pSettings->mode == NEXUS_FrontendOfdmMode_eIsdbt){
            tnrSettings.std = BTNR_Standard_eIsdbt;
        }
        tnrSettings.bandwidth = pSettings->bandwidth;

        rc = BTNR_SetSettings(pDevice->tnr[0], &tnrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = BTNR_SetTunerRfFreq(pDevice->tnr[0], pSettings->frequency, BTNR_TunerMode_eDigital);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    if ( pSettings->acquisitionMode != NEXUS_FrontendOfdmAcquisitionMode_eScan) {
        switch ( pSettings->mode )
        {
        case NEXUS_FrontendOfdmMode_eIsdbt:
        case NEXUS_FrontendOfdmMode_eDvbt2:
        case NEXUS_FrontendOfdmMode_eDvbt:
            BKNI_Sleep(500);
            rc = BODS_Acquire(pDevice->ods_chn[chn_num], &odsParam);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            break;
        default:
            break;
        }
    }

    pDevice->last_ofdm[0] = *pSettings;
    return BERR_SUCCESS;
retrack:
    pDevice->last_ofdm[0].frequency = temp_frequency;
    pDevice->last_ofdm[0].mode = temp_mode;
done:
    NEXUS_Frontend_P_7364_UnTune(handle);
    return rc;
}

static void NEXUS_Frontend_P_7364_ResetStatus(void *handle)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7364Device *pDevice = (NEXUS_7364Device *)handle;

    switch ( pDevice->lastChannel )
    {
    case NEXUS_7364_ISDBT_CHN:
    case NEXUS_7364_DVBT_CHN:
    case NEXUS_7364_DVBT2_CHN:
        if(pDevice->isPoweredOn[pDevice->lastChannel]) {
            rc = BODS_ResetStatus(pDevice->ods_chn[pDevice->lastChannel]);
            if(rc){rc = BERR_TRACE(rc); goto done;}
        }
        else{
            BDBG_MSG(("The core is Powered Off"));
        }
        break;
    case NEXUS_7364_VSB_CHN:
        rc = BTFE_ResetStatus(pDevice->tfe_chn[0]);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        break;
    default:
        BDBG_ERR((" Unsupported channel."));
        BERR_TRACE(BERR_NOT_SUPPORTED);
    }
done:
    return;
}

static NEXUS_Error NEXUS_Frontend_P_7364_ReadSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length, size_t *pNumRead)
{
    size_t i;
    NEXUS_Error rc = NEXUS_SUCCESS;
    int16_t return_length;
    NEXUS_7364Device *pDevice = (NEXUS_7364Device *)handle;
    int16_t d_i[TOTAL_SOFTDECISIONS], d_q[TOTAL_SOFTDECISIONS];

    BKNI_Memset(pDecisions, 0, (sizeof(NEXUS_FrontendSoftDecision) * length));

    switch ( pDevice->lastChannel )
    {
    case NEXUS_7364_ISDBT_CHN:
    case NEXUS_7364_DVBT_CHN:
    case NEXUS_7364_DVBT2_CHN:
            rc = BODS_GetSoftDecision(pDevice->ods_chn[pDevice->lastChannel], (int16_t)TOTAL_SOFTDECISIONS, d_i, d_q, &return_length);
            if(rc){rc = BERR_TRACE(rc); goto done;}
        break;
    case NEXUS_7364_VSB_CHN:
            rc = BTFE_GetSoftDecision(pDevice->tfe_chn[0], (int16_t)TOTAL_SOFTDECISIONS, d_i, d_q, &return_length);
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
NEXUS_Error NEXUS_FrontendDevice_Open7364_Terrestrial(NEXUS_7364Device *pDevice)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    rc = NEXUS_FrontendDevice_P_Init_7364_Hab(pDevice, &pDevice->openSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    /* Initialize THD and T2 channels*/
    rc = NEXUS_FrontendDevice_P_Init7364(pDevice);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}
/* End terrestrial frontend device implementation */

static NEXUS_Error NEXUS_Frontend_P_7364_RequestVsbAsyncStatus(void *handle)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7364Device *pDevice = (NEXUS_7364Device *)handle;
    BDBG_ASSERT(NULL != handle);

    pDevice->isStatusReady = false;

    rc = BTFE_RequestSelectiveAsyncStatus(pDevice->tfe_chn[0], BTFE_SelectiveAsyncStatusType_eDemod);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7364_GetVsbAsyncStatus(void *handle, NEXUS_FrontendVsbStatus *pStatus)
{
    NEXUS_Error rc = BERR_SUCCESS;
    NEXUS_7364Device *pDevice;

    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7364Device *)handle;

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    BKNI_Memset(&pDevice->vsbStatus, 0, sizeof(pDevice->vsbStatus));

    rc = BTFE_GetSelectiveAsyncStatus(pDevice->tfe_chn[0], BTFE_SelectiveAsyncStatusType_eDemod, &pDevice->vsbStatus);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pStatus->receiverLock = pDevice->vsbStatus.status.demodStatus.lock;
    pStatus->fecLock = pDevice->vsbStatus.status.demodStatus.lock;
    pStatus->reacquireCount = pDevice->vsbStatus.status.demodStatus.reacqCount;
    pStatus->snrEstimate = pDevice->vsbStatus.status.demodStatus.equalizerSNR;
    pStatus->carrierOffset = pDevice->vsbStatus.status.demodStatus.carrierOffset;
    pStatus->symbolRateError = pDevice->vsbStatus.status.demodStatus.timingOffset;
    pStatus->fecUncorrected = pDevice->vsbStatus.status.demodStatus.rSUncorrectableErrors;
    pStatus->fecCorrected = pDevice->vsbStatus.status.demodStatus.rSCorrectableErrors;
    pStatus->fecClean = pDevice->vsbStatus.status.demodStatus.numRSpacketsTotal - pStatus->fecCorrected - pStatus->fecUncorrected;

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7364_GetVsbStatus(void *handle, NEXUS_FrontendVsbStatus *pStatus)
{
    NEXUS_Error rc = BERR_SUCCESS;
    NEXUS_7364Device *pDevice;
    unsigned j=0;
    uint32_t buf=0;

    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7364Device *)handle;

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    BKNI_Memset(&pDevice->vsbStatus, 0, sizeof(pDevice->vsbStatus));

    rc = NEXUS_Frontend_P_7364_RequestVsbAsyncStatus(pDevice);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    for(j=0; j < 200; j++) {

        BKNI_Sleep(20);

        rc = BHAB_ReadRegister(pDevice->hab, BCHP_LEAP_CTRL_GP10 , &buf);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        if (buf & (BHAB_VSB_STATUS_RDY)) {
            rc = NEXUS_Frontend_P_7364_GetVsbAsyncStatus(pDevice, pStatus);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            break;
        }
    }

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7364_TuneVsb(void *handle, const NEXUS_FrontendVsbSettings *pSettings)
{

    NEXUS_Error  rc = NEXUS_SUCCESS;

    unsigned chn_num = 0;
    BTFE_AcquireParams tfeParams;
    BTNR_PowerSaverSettings pwrSettings;
    BTNR_Settings tnrSettings;
    NEXUS_FrontendDeviceHandle genericDeviceHandle = NULL;
    NEXUS_7364Device *pDevice = (NEXUS_7364Device *)handle;

    BDBG_ASSERT(NULL != pSettings);
    BSTD_UNUSED(pDevice);

    rc = BTFE_InstallCallback(pDevice->tfe_chn[0], BTFE_Callback_eLockChange, (BTFE_CallbackFunc)NEXUS_Frontend_P_7364_callback_isr, (void*)pDevice);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    rc = BTFE_InstallCallback(pDevice->tfe_chn[0], BTFE_Callback_eAsyncStatusReady, (BTFE_CallbackFunc)NEXUS_Frontend_P_7364_AsyncStatusCallback_isr, (void*)pDevice);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    chn_num = NEXUS_7364_VSB_CHN;

    NEXUS_IsrCallback_Set(pDevice->lockAppCallback[chn_num], &(pSettings->lockCallback));
    NEXUS_IsrCallback_Set(pDevice->asyncStatusAppCallback[chn_num], &(pSettings->asyncStatusReadyCallback));
    genericDeviceHandle = pDevice->pGenericDeviceHandle;
    if(genericDeviceHandle == NULL) {
        BDBG_WRN(("Generic device handle for this device is corrupted.")); goto done;
    }

    pDevice->frontendHandle->mtsif.inputBand = NEXUS_FRONTEND_7364_VSB_INPUT_BAND;
    rc = NEXUS_Frontend_P_SetMtsifConfig(pDevice->frontendHandle);
    if (rc) { return BERR_TRACE(rc); }

#if 0
    do{
        if(genericDeviceHandle->application != NEXUS_FrontendDeviceApplication_eTerrestrial){
            BDBG_ERR(("Wrong Tuner application set for device 0x%x. Set NEXUS_TunerApplication_eTerrestrial as its tuner application", (unsigned int)pDevice->pGenericDeviceHandle));
            goto done;
        }
        genericDeviceHandle = genericDeviceHandle->parent;
    }while(genericDeviceHandle != NULL);
#endif
    rc = NEXUS_Tuner_P_7364_GetNSetGain(pDevice);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if (pDevice->tnr[0])
    {
        if(!pDevice->isTunerPoweredOn){
            pwrSettings.enable = false;
            rc = BTNR_SetPowerSaver(pDevice->tnr[0], &pwrSettings);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->isTunerPoweredOn = true;
        }

            rc = BTFE_DisablePowerSaver(pDevice->tfe_chn[0]); /* TFE handle or channel handle???? */
            if(rc){rc = BERR_TRACE(rc); goto done;}

            pDevice->isPoweredOn[chn_num] = true;

        rc = BTNR_GetSettings(pDevice->tnr[0], &tnrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        tnrSettings.std = BTNR_Standard_eVsb;
        tnrSettings.bandwidth = 6000000; /* HARDCODED FOR NOW */

        rc = BTNR_SetSettings(pDevice->tnr[0], &tnrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        pDevice->count = 0;
        pDevice->acquireInProgress = true;
        pDevice->lastChannel = chn_num;
        rc = BTNR_SetTunerRfFreq(pDevice->tnr[0], pSettings->frequency, BTNR_TunerMode_eDigital);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    rc = BTFE_GetAcquireParams(pDevice->tfe_chn[0], &tfeParams);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    tfeParams.spectrumInversion = true;
    tfeParams.tunerIf.center = 13500000; /* 13.5 MHz */
    tfeParams.ifFrequency = 0;

    rc = BTFE_SetAcquireParams(pDevice->tfe_chn[0], &tfeParams );
    if(rc){rc = BERR_TRACE(rc); goto done;}

    rc = BTFE_Acquire(pDevice->tfe_chn[0], &tfeParams );
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pDevice->last_tfe[0] = *pSettings;
done:
    return rc;
}

/* Terrestrial implementation */
NEXUS_FrontendHandle NEXUS_Frontend_Open7364_Terrestrial(const NEXUS_FrontendChannelSettings *pSettings)
{
    NEXUS_FrontendHandle frontendHandle = NULL;
    NEXUS_7364Device *pDevice = NULL;
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
    if ( NULL == frontendHandle ) {BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_alloc;}

    /* Establish device capabilities */
    frontendHandle->capabilities.vsb = true;
    frontendHandle->capabilities.ofdm = true;
    frontendHandle->capabilities.ofdmModes[NEXUS_FrontendOfdmMode_eDvbt] = true;
    frontendHandle->capabilities.ofdmModes[NEXUS_FrontendOfdmMode_eDvbt2] = true;
    frontendHandle->capabilities.ofdmModes[NEXUS_FrontendOfdmMode_eIsdbt] = true;

    frontendHandle->requestDvbtAsyncStatus = NEXUS_Frontend_P_7364_RequestDvbtAsyncStatus;
    frontendHandle->getDvbtAsyncStatusReady = NEXUS_Frontend_P_7364_GetDvbtAsyncStatusReady;
    frontendHandle->getDvbtAsyncStatus = NEXUS_Frontend_P_7364_GetDvbtAsyncStatus;
    frontendHandle->requestDvbt2AsyncStatus = NEXUS_Frontend_P_7364_RequestDvbt2AsyncStatus;
    frontendHandle->getDvbt2AsyncStatusReady = NEXUS_Frontend_P_7364_GetDvbt2AsyncStatusReady;
    frontendHandle->getDvbt2AsyncFecStatistics = NEXUS_Frontend_P_7364_GetDvbt2AsyncFecStatistics;
    frontendHandle->getDvbt2AsyncL1PreStatus = NEXUS_Frontend_P_7364_GetDvbt2AsyncL1PreStatus;
    frontendHandle->getDvbt2AsyncL1PostConfigurableStatus = NEXUS_Frontend_P_7364_GetDvbt2AsyncL1PostConfigurableStatus;
    frontendHandle->getDvbt2AsyncPostDynamicStatus = NEXUS_Frontend_P_7364_GetDvbt2AsyncPostDynamicStatus;
    frontendHandle->getDvbt2AsyncL1PlpStatus = NEXUS_Frontend_P_7364_GetDvbt2AsyncL1PlpStatus;
    frontendHandle->getDvbt2AsyncBasicStatus = NEXUS_Frontend_P_7364_GetDvbt2AsyncBasicStatus;
    frontendHandle->requestIsdbtAsyncStatus = NEXUS_Frontend_P_7364_RequestIsdbtAsyncStatus;
    frontendHandle->getIsdbtAsyncStatusReady = NEXUS_Frontend_P_7364_GetIsdbtAsyncStatusReady;
    frontendHandle->getIsdbtAsyncStatus = NEXUS_Frontend_P_7364_GetIsdbtAsyncStatus;
    frontendHandle->requestOfdmAsyncStatus = NEXUS_Frontend_P_7364_RequestOfdmAsyncStatus;
    frontendHandle->getOfdmAsyncStatus = NEXUS_Frontend_P_7364_GetOfdmAsyncStatus;
    frontendHandle->tuneOfdm = NEXUS_Frontend_P_7364_TuneOfdm;
    frontendHandle->untune = NEXUS_Frontend_P_7364_UnTune;
    frontendHandle->resetStatus = NEXUS_Frontend_P_7364_ResetStatus;
    frontendHandle->readSoftDecisions = NEXUS_Frontend_P_7364_ReadSoftDecisions;
    frontendHandle->close = NEXUS_Frontend_P_7364_Close;
    frontendHandle->getFastStatus = NEXUS_Frontend_P_7364_GetFastStatus;
    frontendHandle->uninstallCallbacks = NEXUS_FrontendDevice_P_7364_UninstallCallbacks;
    frontendHandle->tuneVsb = NEXUS_Frontend_P_7364_TuneVsb;
    frontendHandle->requestVsbAsyncStatus = NEXUS_Frontend_P_7364_RequestVsbAsyncStatus;
    frontendHandle->getVsbAsyncStatus = NEXUS_Frontend_P_7364_GetVsbAsyncStatus;
    frontendHandle->getVsbStatus = NEXUS_Frontend_P_7364_GetVsbStatus;

    frontendHandle->pGenericDeviceHandle = pFrontendDevice;

    /* Create app callbacks */
    for(chn_num=0; chn_num < NEXUS_MAX_7364_T_FRONTENDS; chn_num++){

        pDevice->lockAppCallback[chn_num] = NEXUS_IsrCallback_Create(frontendHandle, NULL);
        if ( NULL == pDevice->lockAppCallback[chn_num] ) { BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err_cbk_create;}

        pDevice->asyncStatusAppCallback[chn_num] = NEXUS_IsrCallback_Create(frontendHandle, NULL);
        if ( NULL == pDevice->asyncStatusAppCallback[chn_num] ) {BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err_cbk_create;}
     }

    pDevice->updateGainAppCallback[0] = NEXUS_IsrCallback_Create(frontendHandle, NULL);
    if ( NULL == pDevice->updateGainAppCallback[0] ) {BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err_cbk_create;}

    frontendHandle->userParameters.isMtsif = true;

    frontendHandle->standby = NEXUS_Frontend_P_7364_Terrestrial_Standby;

    pDevice->frontendHandle = frontendHandle;
    frontendHandle->chip.familyId = 0x7364;
    frontendHandle->chip.id = 0x7364;
    return frontendHandle;

err_cbk_create:
    for(chn_num=0; chn_num < NEXUS_MAX_7364_T_FRONTENDS; chn_num++){
        if ( pDevice->lockAppCallback[chn_num] ) NEXUS_IsrCallback_Destroy(pDevice->lockAppCallback[chn_num]);
        if ( pDevice->asyncStatusAppCallback[chn_num] ) NEXUS_IsrCallback_Destroy(pDevice->asyncStatusAppCallback[chn_num]);
    }
    if ( pDevice->updateGainAppCallback[0]) NEXUS_IsrCallback_Destroy(pDevice->updateGainAppCallback[0]);
    if ( frontendHandle ) BKNI_Free(frontendHandle);
err_alloc:
    NEXUS_FrontendDevice_P_Uninit7364(pDevice);
done:
    return NULL;
}
/* End of Terrestrial channel open implementation */

void NEXUS_FrontendDevice_Close7364_Terrestrial(NEXUS_7364Device *pDevice)
{
    /* Terrestrial teardown */
    NEXUS_FrontendDevice_P_Uninit7364(pDevice);
    NEXUS_FrontendDevice_P_Uninit_7364_Hab(pDevice);

    if(pDevice->capabilities.channelCapabilities)
        BKNI_Free(pDevice->capabilities.channelCapabilities);
    pDevice->capabilities.channelCapabilities = NULL;

    return;
    /* End of terrestrial teardown */
}

static NEXUS_Error NEXUS_Frontend_P_7364_Terrestrial_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7364Device *pDevice = (NEXUS_7364Device *)handle;
    BTNR_PowerSaverSettings pwrSettings;
    BODS_PowerSaverSettings odsPwrSettings;

    BSTD_UNUSED(enabled);

    if (pSettings->mode < NEXUS_StandbyMode_ePassive) {
        if (!pDevice->isTunerPoweredOn) {
            pwrSettings.enable = false;
            rc = BTNR_SetPowerSaver(pDevice->tnr[0], &pwrSettings);
            if (rc) { rc = BERR_TRACE(rc); }
            pDevice->isTunerPoweredOn = true;
        }
        switch (pDevice->lastChannel)
        {
        case NEXUS_7364_VSB_CHN:
            rc = BTFE_DisablePowerSaver(pDevice->tfe_chn[0]); /* TFE handle or channel handle???? */
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->isPoweredOn[pDevice->lastChannel] = true;
            break;
        case NEXUS_7364_ISDBT_CHN:
        case NEXUS_7364_DVBT_CHN:
        case NEXUS_7364_DVBT2_CHN:
            if (pDevice->isPoweredOn[pDevice->lastChannel]) {
                rc = BODS_DisablePowerSaver(pDevice->ods_chn[pDevice->lastChannel], &odsPwrSettings);
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
        case NEXUS_7364_VSB_CHN:
            rc = BTFE_EnablePowerSaver(pDevice->tfe_chn[0]); /* TFE handle or channel handle???? */
            if (rc) { rc = BERR_TRACE(rc); goto done; }
            pDevice->isPoweredOn[pDevice->lastChannel] = false;
            break;
        case NEXUS_7364_ISDBT_CHN:
        case NEXUS_7364_DVBT_CHN:
        case NEXUS_7364_DVBT2_CHN:
            if (pDevice->isPoweredOn[pDevice->lastChannel]) {
                rc = BODS_EnablePowerSaver(pDevice->ods_chn[pDevice->lastChannel], &odsPwrSettings);
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
            rc = BTNR_SetPowerSaver(pDevice->tnr[0], &pwrSettings);
            if (rc) { rc = BERR_TRACE(rc); goto done; }
            pDevice->isTunerPoweredOn = false;
        }
    }

done:
    return rc;
}
#else
NEXUS_Error NEXUS_FrontendDevice_P_Init7364_Terrestrial(NEXUS_7364Device *pDevice)
{
    BSTD_UNUSED(pDevice);
    return NEXUS_UNKNOWN;
}

void NEXUS_FrontendDevice_P_Uninit7364_Terrestrial(NEXUS_7364Device *pDevice)
{
    BSTD_UNUSED(pDevice);
}

NEXUS_Error NEXUS_FrontendDevice_Open7364_Terrestrial(NEXUS_7364Device *pDevice)
{
    BSTD_UNUSED(pDevice);
    return NEXUS_UNKNOWN;
}

NEXUS_FrontendHandle NEXUS_Frontend_Open7364_Terrestrial(const NEXUS_FrontendChannelSettings *pSettings)
{
    BSTD_UNUSED(pSettings);
    return NULL;
}

void NEXUS_FrontendDevice_Close7364_Terrestrial(NEXUS_7364Device *pDevice)
{
    BSTD_UNUSED(pDevice);
}
#endif
