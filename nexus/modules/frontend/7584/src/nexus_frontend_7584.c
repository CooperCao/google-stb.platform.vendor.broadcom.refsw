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
*   API name: Frontend 7584
*    APIs to open, close, and setup initial settings for a BCM7584
*    Frontend Device.
*
***************************************************************************/
#include "nexus_frontend_module.h"
#include "nexus_platform_features.h"
#include "nexus_i2c.h"
#include "priv/nexus_i2c_priv.h"
#include "priv/nexus_gpio_priv.h"
#include "btnr.h"
#include "btnr_7584ib.h"
#include "bhab_7584.h"
#include "bads_7584.h"
#include "bhab_7584_fw.h"
#include "bhab.h"
#include "bads.h"
#include "bchp_sun_top_ctrl.h"
#include "bhab_7584_priv.h"
#include "nexus_avs.h"
#include "bhab_ctfe_img.h"
#include "priv/nexus_core_img.h"
#include "priv/nexus_core_img_id.h"

#if NEXUS_FRONTEND_7584_OOB
#include "baob.h"
#include "btnr_7584ob.h"
#endif

BDBG_MODULE(nexus_frontend_7584);

BDBG_OBJECT_ID(NEXUS_7584);

/* Currently there are eight ADS channels and one AOB channel on 7584 */
#define NEXUS_MAX_7584_FRONTENDS (NEXUS_3128_MAX_DOWNSTREAM_CHANNELS + NEXUS_3128_MAX_OUTOFBAND_CHANNELS)

typedef struct NEXUS_PreviousStatus
{
    unsigned fecCorrected;
    unsigned fecUncorrected;
    unsigned fecClean;
    unsigned viterbiUncorrectedBits;
    unsigned viterbiTotalBits;
    NEXUS_Time time;
}NEXUS_PreviousStatus;

/***************************************************************************
 * Internal Module Structure
 ***************************************************************************/
typedef struct NEXUS_7584
{
    BDBG_OBJECT(NEXUS_7584)
    BLST_S_ENTRY(NEXUS_7584) node;
    uint32_t  chipFamilyId;
    uint32_t  chipId;
    BHAB_Handle hab;
    BHAB_Capabilities capabilities;
    unsigned    isrNumber;
    NEXUS_GpioHandle gpioInterrupt;
    uint16_t    i2cAddr;
    NEXUS_I2cHandle i2cHandle;
    unsigned    numfrontends;
    bool adsPresent;
    bool aobPresent;
    bool ifDacPresent;
    BADS_Handle ads;
    BTNR_Handle tnr[NEXUS_MAX_7584_FRONTENDS];
    NEXUS_FrontendHandle    frontendHandle[NEXUS_MAX_7584_FRONTENDS];
    BADS_ChannelHandle  ads_chn[NEXUS_MAX_7584_FRONTENDS];
    NEXUS_FrontendQamSettings   qam[NEXUS_MAX_7584_FRONTENDS];
    NEXUS_FrontendOutOfBandSettings oob;
    NEXUS_FrontendQamSettings   last_ads[NEXUS_MAX_7584_FRONTENDS];
    BKNI_EventHandle isrEvent;
    NEXUS_EventCallbackHandle isrEventCallback;
    NEXUS_IsrCallbackHandle lockAppCallback[NEXUS_MAX_7584_FRONTENDS];
    NEXUS_IsrCallbackHandle updateGainAppCallback;
    NEXUS_IsrCallbackHandle asyncStatusAppCallback[NEXUS_MAX_7584_FRONTENDS];
    NEXUS_TaskCallbackHandle spectrumDataAppCallback[NEXUS_MAX_7584_FRONTENDS];
    bool                    isInternalAsyncStatusCall[NEXUS_MAX_7584_FRONTENDS];
    NEXUS_IsrCallbackHandle     cppmAppCallback;
    NEXUS_IsrCallbackHandle     cppmDoneCallback;
    uint16_t revId; /* 0x00 = A0 */
    NEXUS_FrontendDevice3128Settings deviceSettings;
    bool isMtsif;
    uint8_t numTunerPoweredOn;
    bool isTunerPoweredOn[NEXUS_MAX_7584_FRONTENDS];
    bool isPoweredOn[NEXUS_MAX_7584_FRONTENDS];
    uint32_t *spectrumDataPointer;
    unsigned spectrumDataLength;
    BKNI_EventHandle spectrumEvent;
    NEXUS_EventCallbackHandle spectrumEventCallback;
    bool acquireInProgress[NEXUS_MAX_7584_FRONTENDS];
    unsigned count[NEXUS_MAX_7584_FRONTENDS];
    NEXUS_PreviousStatus previousStatus[NEXUS_MAX_7584_FRONTENDS];
    NEXUS_FrontendDevice *pGenericDeviceHandle;
    NEXUS_TunerHandle  ifDacHandle;
    unsigned ifDacChannelNumber;
    NEXUS_IsrCallbackHandle ifDacAppCallback;
    NEXUS_TunerSettings ifDacTunerSettings;
    bool ifdacTuneComplete;
    unsigned oobChannelNumber;
    NEXUS_StandbyMode previousStandbyMode;
#if NEXUS_FRONTEND_7584_OOB
    BAOB_Handle aob;
    NEXUS_FrontendOutOfBandSettings last_aob;
#endif
} NEXUS_7584;

static BLST_S_HEAD(devList, NEXUS_7584) g_7584DevList = BLST_S_INITIALIZER(g_7584DevList);


typedef struct NEXUS_7584Channel
{
    unsigned chn_num; /* channel number */
    NEXUS_7584 *pDevice; /* 7584 device*/
} NEXUS_7584Channel;

/***************************************************************************
 * Module callback functions for tuning
 ***************************************************************************/
static void NEXUS_Frontend_P_7584_UninstallCallbacks(void *handle);
static NEXUS_Error NEXUS_Frontend_P_7584_TuneQam(void *handle, const NEXUS_FrontendQamSettings *pSettings);
static void NEXUS_Frontend_P_7584_UnTuneQam(void *handle);
static NEXUS_Error NEXUS_Frontend_P_7584_GetQamStatus(void *handle, NEXUS_FrontendQamStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_7584_GetQamAsyncStatus(void *handle, NEXUS_FrontendQamStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_7584_RequestQamAsyncStatus(void *handle);
static NEXUS_Error NEXUS_Frontend_P_7584_ReadSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length, size_t *pNumRead);
static void NEXUS_Frontend_P_7584_ResetQamStatus(void *handle);
static void NEXUS_Frontend_P_UnInit7584(NEXUS_7584 *pDevice);
static void NEXUS_Frontend_P_7584_Close( NEXUS_FrontendHandle handle);
static NEXUS_Error NEXUS_Frontend_P_7584_GetFastStatus(void *handle, NEXUS_FrontendFastStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_7584_GetQamScanStatus(void *handle, NEXUS_FrontendQamScanStatus *pScanStatus);
static void NEXUS_Frontend_P_7584_GetType(void *handle, NEXUS_FrontendType *type);
static NEXUS_Error NEXUS_Frontend_P_7584_RequestSpectrumAnalyzerData(void *handle, const NEXUS_FrontendSpectrumSettings *pSettings );
static NEXUS_Error NEXUS_Frontend_P_7584_WriteRegister(void *handle, unsigned address, uint32_t value);
static NEXUS_Error NEXUS_Frontend_P_7584_ReadRegister(void *handle, unsigned address, uint32_t *value   );
static NEXUS_Error NEXUS_Frontend_P_7584_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings);

static NEXUS_Error NEXUS_FrontendDevice_P_7584_Standby(void * handle, const NEXUS_StandbySettings *pSettings);
static NEXUS_Error NEXUS_Tuner_P_7584_GetNSetGain(void *handle);
static NEXUS_Error NEXUS_FrontendDevice_P_7584_GetStatus(void * handle, NEXUS_FrontendDeviceStatus *pStatus);
static NEXUS_Error NEXUS_FrontendDevice_P_7584_GetInternalGain(void *handle, const NEXUS_GainParameters *params, NEXUS_InternalGainSettings *pSettings);
static NEXUS_Error NEXUS_FrontendDevice_P_7584_GetExternalGain(void *handle, NEXUS_ExternalGainSettings *pSettings);
static NEXUS_Error NEXUS_FrontendDevice_P_7584_SetExternalGain(void *handle, const NEXUS_ExternalGainSettings *pSettings);
NEXUS_Error NEXUS_FrontendDevice_P_7584_Recalibrate(void *handle, const NEXUS_FrontendDeviceRecalibrateSettings *pSettings);
static void NEXUS_FrontendDevice_P_7584_GetCapabilities(void *handle, NEXUS_FrontendDeviceCapabilities *pCapabilities);
static void NEXUS_FrontendDevice_P_7584_GetTunerCapabilities(void *handle, unsigned tunerIndex, NEXUS_FrontendCapabilities *pCapabilities);
static void NEXUS_FrontendDevice_P_7584_Close(void *handle);
static void NEXUS_Tuner_P_7584_UnTune(void *handle);
static NEXUS_Error NEXUS_Tuner_P_7584_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings);
#if NEXUS_AMPLIFIER_SUPPORT
NEXUS_Error NEXUS_FrontendDevice_P_7584_GetAmplifierStatus(void *handle, NEXUS_AmplifierStatus *pStatus);
NEXUS_Error NEXUS_FrontendDevice_P_7584_SetAmplifierStatus(void *handle, const NEXUS_AmplifierStatus *pStatus);
#endif

#if NEXUS_FRONTEND_7584_OOB
static void NEXUS_Frontend_P_7584_UnTuneOob(void *handle);
static NEXUS_Error NEXUS_Frontend_P_7584_TuneOob(void *handle, const NEXUS_FrontendOutOfBandSettings *pSettings);
static NEXUS_Error NEXUS_Frontend_P_7584_GetOobStatus(void *handle, NEXUS_FrontendOutOfBandStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_7584_GetOobAsyncStatus(void *handle, NEXUS_FrontendOutOfBandStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_7584_RequestOobAsyncStatus(void *handle);
static void NEXUS_Frontend_P_7584_ResetOobStatus(void *handle);
#endif
/***************************************************************************
Summary:
    Lock callback handler for a 7584 Inband device
 ***************************************************************************/
static void NEXUS_Frontend_P_7584_cppm_callback_isr(void *pParam)
{
    NEXUS_7584 *pDevice;
    BDBG_ASSERT(NULL != pParam);
    pDevice = (NEXUS_7584 *)pParam;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);

    if ( pDevice->cppmAppCallback)
    {
        NEXUS_IsrCallback_Fire_isr(pDevice->cppmAppCallback);
    }
}

static void NEXUS_Frontend_P_7584_cppm_complete_callback_isr(void *pParam)
{
    NEXUS_7584 *pDevice;
    BDBG_ASSERT(NULL != pParam);
    pDevice = (NEXUS_7584 *)pParam;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);

    if ( pDevice->cppmDoneCallback)
    {
        NEXUS_IsrCallback_Fire_isr(pDevice->cppmDoneCallback);
    }
}

static void NEXUS_Frontend_P_7584_spetctrumDataReadyCallback(void *pParam)
{
    BDBG_ASSERT(NULL != pParam);
    BKNI_SetEvent((BKNI_EventHandle)pParam);
}

static void NEXUS_Frontend_P_7584_spetctrumEventCallback(void *pParam)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BTNR_SpectrumData  spectrumData;
    NEXUS_7584Channel *pChannel;
    NEXUS_7584 *pDevice;
    BDBG_ASSERT(NULL != pParam);
    pChannel = (NEXUS_7584Channel *)pParam;
    pDevice = (NEXUS_7584 *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);

    spectrumData.data = pDevice->spectrumDataPointer;

    rc = BTNR_GetSpectrumAnalyzerData(pDevice->tnr[pChannel->chn_num], &spectrumData);
    if (rc){rc = BERR_TRACE(rc);  goto done;}

    pDevice->spectrumDataLength += spectrumData.datalength;
    pDevice->spectrumDataPointer += spectrumData.datalength;

    if(!spectrumData.moreData) {
        if ( pDevice->spectrumDataAppCallback[pChannel->chn_num] )
        {
            pDevice->spectrumDataLength = 0;
            NEXUS_TaskCallback_Fire(pDevice->spectrumDataAppCallback[pChannel->chn_num]);
        }
    }

done:
    return;
}

static void NEXUS_Tuner_P_7584_Callback(void *pParam)
{
    NEXUS_7584Channel *pChannel;
    NEXUS_7584 *pDevice;
    BDBG_ASSERT(NULL != pParam);
    pChannel = (NEXUS_7584Channel *)pParam;
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);

    if(pDevice->ifDacAppCallback)
    {
        NEXUS_IsrCallback_Fire_isr(pDevice->ifDacAppCallback);
    }
}


static void NEXUS_Frontend_P_7584_callback_isr(void *pParam)
{
    NEXUS_7584Channel *pChannel;
    NEXUS_7584 *pDevice;
    BDBG_ASSERT(NULL != pParam);
    pChannel = (NEXUS_7584Channel *)pParam;
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);

    if(pDevice->acquireInProgress[pChannel->chn_num]){
        pDevice->count[pChannel->chn_num]++;
    }
    if(pDevice->count[pChannel->chn_num] == 2){
        pDevice->acquireInProgress[pChannel->chn_num] = false;
        pDevice->count[pChannel->chn_num] = 0;
    }

    if ( pDevice->lockAppCallback[pChannel->chn_num] )
    {
        NEXUS_IsrCallback_Fire_isr(pDevice->lockAppCallback[pChannel->chn_num]);
    }
}
static void NEXUS_Frontend_P_7584_updateGainCallback_isr(void *pParam)
{
    NEXUS_IsrCallbackHandle callback;
    BDBG_ASSERT(NULL != pParam);
    callback = (NEXUS_IsrCallbackHandle)pParam;

     if ( callback )
    {
        NEXUS_IsrCallback_Fire_isr(callback);
    }
}

/***************************************************************************
Summary:
    Lock callback handler for a 7584 Inband device
 ***************************************************************************/
static void NEXUS_Frontend_P_7584_AsyncStatusCallback_isr(void *pParam)
{
    NEXUS_7584Channel *pChannel;
    NEXUS_7584 *pDevice;
    BDBG_ASSERT(NULL != pParam);
    pChannel = (NEXUS_7584Channel *)pParam;
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);

    if(!pDevice->isInternalAsyncStatusCall[pChannel->chn_num]){
        if(pDevice->ifDacChannelNumber == pChannel->chn_num){
            pDevice->ifdacTuneComplete = true;
        }
        if (pDevice->asyncStatusAppCallback[pChannel->chn_num])
        {
            NEXUS_IsrCallback_Fire_isr(pDevice->asyncStatusAppCallback[pChannel->chn_num]);
        }
    }
}
/***************************************************************************
Summary:
    Enable/Disable interrupts for a 7584 device
 ***************************************************************************/
static void NEXUS_Frontend_P_7584_L1_isr(void *pParam1, int pParam2)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHAB_Handle hab;
    BDBG_ASSERT(NULL != pParam1);
    hab = (BHAB_Handle)pParam1;
    BSTD_UNUSED(pParam2);

    rc = BHAB_HandleInterrupt_isr(hab);
    if(rc){rc = BERR_TRACE(rc);}
}
/***************************************************************************
Summary:
    Enable/Disable interrupts for a 7584 device
 ***************************************************************************/
static void NEXUS_Frontend_P_7584_GpioIsrControl_isr(bool enable, void *pParam)
{
    NEXUS_GpioHandle gpioHandle;
    BDBG_ASSERT(NULL != pParam);
    gpioHandle = (NEXUS_GpioHandle)pParam;

    if(enable){
        NEXUS_Gpio_SetInterruptEnabled_isr(gpioHandle, true);
    }
    else {
        NEXUS_Gpio_SetInterruptEnabled_isr(gpioHandle, false);
    }
}

/***************************************************************************
Summary:
    Enable/Disable interrupts for a 7584 device
 ***************************************************************************/
static void NEXUS_Frontend_P_7584_IsrControl_isr(bool enable, void *pParam)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    int isrnumber;
    BDBG_ASSERT(NULL != pParam);
    isrnumber = (int)pParam;

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

/***************************************************************************
Summary:
    ISR Event Handler for a 7584 device
 ***************************************************************************/
static void NEXUS_Frontend_P_7584_IsrEvent(void *pParam)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHAB_Handle hab;
    BDBG_ASSERT(NULL != pParam);
    hab = (BHAB_Handle)pParam;

    rc = BHAB_ProcessInterruptEvent(hab);
    if(rc) BERR_TRACE(rc);
}
/***************************************************************************
Summary:
    Initialize Hab for a 7584 device
***************************************************************************/
static void NEXUS_Frontend_P_UnInit_7584_Hab(NEXUS_7584 *pDevice)
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
        rc = NEXUS_Gpio_SetSettings(pDevice->gpioInterrupt, &gpioSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    if(pDevice->isrEvent) pDevice->isrEvent = NULL;
    if(pDevice->isrEventCallback)NEXUS_UnregisterEvent(pDevice->isrEventCallback);
    pDevice->isrEventCallback = NULL;
    if(pDevice->hab) {
        rc = BHAB_Close(pDevice->hab);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    pDevice->hab = NULL;
done:
    return;
}

/***************************************************************************
Summary:
    Initialize Hab for a 7584 device
***************************************************************************/
static NEXUS_Error NEXUS_Frontend_P_Init_7584_Hab(NEXUS_7584 *pDevice, const NEXUS_FrontendDevice3128OpenSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHAB_Settings stHabSettings;
    BHAB_ExternalGainSettings gain;

    rc = BHAB_7584_GetDefaultSettings(&stHabSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    stHabSettings.slaveChipAddr = pSettings->i2cSlaveAddr;
    stHabSettings.isMtsif = pSettings->isMtsif;

    if(pSettings->isrNumber) {
        stHabSettings.interruptEnableFunc = NEXUS_Frontend_P_7584_IsrControl_isr;
        stHabSettings.interruptEnableFuncParam = (void*)pSettings->isrNumber;
    }
    else if(pSettings->gpioInterrupt){
        stHabSettings.interruptEnableFunc = NEXUS_Frontend_P_7584_GpioIsrControl_isr;
        stHabSettings.interruptEnableFuncParam = (void*)pSettings->gpioInterrupt;
    }

    rc = BHAB_Open( &pDevice->hab, (void *)g_pCoreHandles->reg, &stHabSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    /* Success opeining Hab.  Connect Interrupt */
    if(pSettings->isrNumber) {
        rc = NEXUS_Core_ConnectInterrupt(pSettings->isrNumber, NEXUS_Frontend_P_7584_L1_isr, (void *)pDevice->hab, 0);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    else if(pSettings->gpioInterrupt){
        NEXUS_Gpio_SetInterruptCallback_priv(pSettings->gpioInterrupt, NEXUS_Frontend_P_7584_L1_isr, (void *)pDevice->hab, 0);
    }

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

            rc = Nexus_Core_P_Img_Create(NEXUS_CORE_IMG_ID_FRONTEND_7584, &pImgContext, &imgInterface);
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
        fw_image = bcm7584_leap_image;
#endif
        rc = BHAB_InitAp(pDevice->hab, fw_image);
#if NEXUS_MODE_driver
        NEXUS_Memory_Free(fw);
#endif
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    /* Get events and register callbacks */
    rc = BHAB_GetInterruptEventHandle(pDevice->hab, &pDevice->isrEvent);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pDevice->isrEventCallback = NEXUS_RegisterEvent(pDevice->isrEvent, NEXUS_Frontend_P_7584_IsrEvent, pDevice->hab);
    if ( NULL == pDevice->isrEventCallback ){rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done; }

    BHAB_InstallInterruptCallback( pDevice->hab, BHAB_DevId_eGlobal,
                                   (BHAB_IntCallbackFunc)NEXUS_Frontend_P_7584_cppm_callback_isr,
                                   (void *)pDevice,
                                   BHAB_Interrupt_eCppmPowerLevelChange);

    BHAB_InstallInterruptCallback( pDevice->hab, BHAB_DevId_eGlobal,
                                   (BHAB_IntCallbackFunc)NEXUS_Frontend_P_7584_cppm_complete_callback_isr,
                                   (void *)pDevice,
                                   BHAB_Interrupt_eCalibrationComplete);

    gain.externalGainBypassable = pSettings->externalFixedGain.bypassable;
    gain.externalGainTotal = pSettings->externalFixedGain.total;

    rc = BHAB_SetExternalGain(pDevice->hab, &gain);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    return BERR_SUCCESS;
done:
    NEXUS_Frontend_P_UnInit_7584_Hab(pDevice);
    return rc;
}

/***************************************************************************
Summary:
    Get the default settings for a BCM7584 tuner
See Also:
    NEXUS_Frontend_Open7584
 ***************************************************************************/
void NEXUS_Frontend_GetDefault3128Settings(
    NEXUS_3128Settings *pSettings   /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->type = NEXUS_3128ChannelType_eInBand;
    pSettings->channelNumber = 0;
    pSettings->inBandOpenDrain = true;
    pSettings->loadAP = true;
    pSettings->i2cSlaveAddr = 0x60;
    pSettings->isMtsif = false;
    pSettings->outOfBand.openDrain = true;
}
/***************************************************************************
Summary:
    Initialize all ADS/OOB channels.
 ***************************************************************************/
static void NEXUS_Frontend_P_UnInit7584(NEXUS_7584 *pDevice)
{
    NEXUS_Error rc = BERR_SUCCESS;
    unsigned int i;

    for ( i = 0; i < NEXUS_MAX_7584_FRONTENDS && NULL != pDevice->tnr[i]; i++) {
        if(pDevice->tnr[i]){
            rc = BTNR_Close(pDevice->tnr[i]);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->tnr[i] = NULL;
        }
    }
    for ( i = 0; i < NEXUS_3128_MAX_DOWNSTREAM_CHANNELS && NULL != pDevice->ads_chn[i]; i++) {
        if(pDevice->ads_chn[i]){
            rc = BADS_CloseChannel(pDevice->ads_chn[i]);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            (pDevice->ads_chn[i]) = NULL;
        }
    }
    if (pDevice->ads) {
        rc = BADS_Close(pDevice->ads);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pDevice->ads = NULL;
    }
#if NEXUS_FRONTEND_7584_OOB
    if (pDevice->aob){
        rc = BAOB_Close(pDevice->aob);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pDevice->aob = NULL;
    }
#endif
done:
    return;
}
/***************************************************************************
Summary:
    Initialize all ADS/OOB channels.
 ***************************************************************************/
static NEXUS_Error NEXUS_Frontend_P_Init7584(NEXUS_7584 *pDevice, const NEXUS_FrontendDevice3128OpenSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned int i;
    BTNR_7584Ib_Settings tnrIb7584_cfg;
    BADS_Settings ads_cfg;
    BADS_ChannelSettings chn_cfg;
    unsigned    totalChannels=0, adsChannels=0;
#if NEXUS_FRONTEND_7584_OOB
    BAOB_Settings aob_cfg;
    BTNR_7584Ob_Settings tnrOb7584_cfg;
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

    for(i=0; i<pDevice->capabilities.totalTunerChannels; i++){
        if(pDevice->capabilities.channelCapabilities[i].demodCoreType.ads) pDevice->adsPresent = true;
        if(pDevice->capabilities.channelCapabilities[i].demodCoreType.aob) pDevice->aobPresent = true;
        if(pDevice->capabilities.channelCapabilities[i].demodCoreType.ifdac) pDevice->ifDacPresent = true;
    }

    rc = BTNR_7584Ib_GetDefaultSettings(&tnrIb7584_cfg);
    if(rc){rc = BERR_TRACE(rc); goto done;}
#if NEXUS_FRONTEND_7584_OOB
    rc = BTNR_7584Ob_GetDefaultSettings(&tnrOb7584_cfg);
    if(rc){rc = BERR_TRACE(rc); goto done;}
#endif

    for(i=0;i<pDevice->capabilities.totalTunerChannels; i++){
        if(!pDevice->capabilities.channelCapabilities[i].demodCoreType.aob){
            tnrIb7584_cfg.channelNo = i;
            rc =  BTNR_7584Ib_Open(&pDevice->tnr[i],&tnrIb7584_cfg, pDevice->hab);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            if(pDevice->capabilities.channelCapabilities[i].demodCoreType.ifdac){
                pDevice->ifDacChannelNumber = i;
            }
        }
        else{
#if NEXUS_FRONTEND_7584_OOB
            if(pDevice->aobPresent){
                tnrOb7584_cfg.ifFreq = pSettings->outOfBand.ifFrequency;
                switch(pSettings->outOfBand.useWidebandAtoD)
                {
                case NEXUS_3128OutOfBandInput_eNarrowBandAtoD:
                    tnrOb7584_cfg.inputMode = BTNR_7584Ob_OutOfBandInputMode_eOutOfBandAdc;
                    break;
                case NEXUS_3128OutOfBandInput_eWideBandAtoD:
                    tnrOb7584_cfg.inputMode = BTNR_7584Ob_OutOfBandInputMode_eWideBandAdc;
                    break;
                case NEXUS_3128OutOfBandInput_eBandPassFilteredNarrowBandAtoD:
                    tnrOb7584_cfg.inputMode = BTNR_7584Ob_OutOfBandInputMode_eBandPassFilteredOutOfBandAdc;
                    break;
                default:
                    BDBG_ERR(("Out of band input type not supported"));
                    rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
                }
                rc = BTNR_7584Ob_Open(&pDevice->tnr[i], &tnrOb7584_cfg, pDevice->hab);
                if(rc){rc = BERR_TRACE(rc); goto done;}
                pDevice->oobChannelNumber = i;
            }
#endif
        }
    }

    for(i=0;i<pDevice->capabilities.totalTunerChannels; i++){
        if(pDevice->capabilities.channelCapabilities[i].demodCoreType.ads){
            if(pDevice->ads == NULL){
                rc = BADS_7584_GetDefaultSettings( &ads_cfg, NULL);
                if(rc){rc = BERR_TRACE(rc); goto done;}

                ads_cfg.transportConfig = BADS_TransportData_eGpioParallel;
                ads_cfg.hGeneric = pDevice->hab;
                ads_cfg.isOpenDrain = pSettings->inBandOpenDrain;
                rc = BADS_Open(&pDevice->ads, NULL, NULL, NULL, &ads_cfg);
                if(rc){rc = BERR_TRACE(rc); goto done;}

                rc = BADS_Init(pDevice->ads);
                if(rc){rc = BERR_TRACE(rc); goto done;}

                /* get total ADS channel number*/
                rc = BADS_GetTotalChannels(pDevice->ads, &totalChannels);
                if(rc){rc = BERR_TRACE(rc); goto done;}
            }

            if(adsChannels < totalChannels){
                rc = BADS_GetChannelDefaultSettings( pDevice->ads, adsChannels, &chn_cfg);
                if(rc){rc = BERR_TRACE(rc); goto done;}
                rc = BADS_OpenChannel( pDevice->ads, &pDevice->ads_chn[adsChannels], adsChannels, &chn_cfg);
                if(rc){rc = BERR_TRACE(rc); goto done;}
                adsChannels++;
            }
            else{
                BDBG_ERR(("Mismatch in total channels reported by HAB and ADS Porting Interface."));
                rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto done;
            }
        }
        if(pDevice->capabilities.channelCapabilities[i].demodCoreType.aob){
#if NEXUS_FRONTEND_7584_OOB
            /* Generally there is only one OOB channel on a given platform. */
            if((pDevice->aob == NULL) && pDevice->aobPresent){
                rc = BAOB_GetDefaultSettings( &aob_cfg, NULL);
                if(rc){rc = BERR_TRACE(rc); goto done;}
                /* There is only one AOB device in 7584/3114. so the device id is left to default settings. */
                aob_cfg.hGeneric = pDevice->hab;
                aob_cfg.serialData = true;
                aob_cfg.isOpenDrain = pSettings->outOfBand.openDrain;
                aob_cfg.nyquist = pSettings->outOfBand.nyquist;
                rc = BAOB_Open(&pDevice->aob, NULL, NULL, NULL, &aob_cfg);
                if(rc){rc = BERR_TRACE(rc); goto done;}
            }
            else{
                BDBG_ERR(("Out of band channel is already opened."));
                rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto done;
            }
#endif
        }
    }

    return BERR_SUCCESS;
done:
    NEXUS_Frontend_P_UnInit7584(pDevice);
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7584_WriteRegister(void *handle, unsigned address, uint32_t value)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7584 *pDevice;
    NEXUS_7584Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7584Channel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);

    rc = BHAB_WriteRegister(pDevice->hab, address, &value);
    if(rc){rc = BERR_TRACE(rc);}

    return rc;
}
static NEXUS_Error NEXUS_Frontend_P_7584_ReadRegister(void *handle, unsigned address, uint32_t *value   )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7584 *pDevice;
    NEXUS_7584Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7584Channel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);

    rc = BHAB_ReadRegister(pDevice->hab, address, value);
    if(rc){rc = BERR_TRACE(rc);}

    return rc;
}

/***************************************************************************
Summary:
    Get the default settings for a BCM7584 device.
See Also:
    NEXUS_FrontendDevice_Open7584
 ***************************************************************************/
void NEXUS_FrontendDevice_GetDefault3128OpenSettings(NEXUS_FrontendDevice3128OpenSettings *pSettings)
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->inBandOpenDrain = true;
    pSettings->loadAP = true;
    pSettings->i2cSlaveAddr = 0x60;
    pSettings->isMtsif = false;
    pSettings->outOfBand.openDrain = true;
    pSettings->externalFixedGain.total = 800;  /* 8 dB (1/100 dB units) */
    pSettings->externalFixedGain.bypassable = 1600; /* 16 dB (1/100 dB units) */
}

NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_Open3128(unsigned index, const NEXUS_FrontendDevice3128OpenSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7584 *pDevice = NULL;
    NEXUS_FrontendDevice *pFrontendDevice = NULL;
    NEXUS_3128ProbeResults results;
    BSTD_UNUSED(index);

    BDBG_ASSERT(NULL != pSettings);

    rc = NEXUS_Frontend_Probe3128(pSettings, &results);
    if(rc){rc = BERR_TRACE(rc); goto err_create;}

    for ( pDevice = BLST_S_FIRST(&g_7584DevList); NULL != pDevice; pDevice = BLST_S_NEXT(pDevice, node) )
    {
        break;
    }

    if ( NULL == pDevice)
    {
        BCHP_Info info;
        BCHP_GetInfo(g_pCoreHandles->chp, &info);

        pFrontendDevice = NEXUS_FrontendDevice_P_Create();
        if (NULL == pFrontendDevice) {rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_create;}

        pDevice = BKNI_Malloc(sizeof(NEXUS_7584));
        if (NULL == pDevice) {rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_create;}

        BKNI_Memset(pDevice, 0, sizeof(NEXUS_7584));
        BDBG_OBJECT_SET(pDevice, NEXUS_7584);

        pDevice->chipFamilyId = info.familyId;
        pDevice->chipId = info.productId;
        pDevice->revId = info.rev;
        pDevice->i2cAddr = pSettings->i2cAddr;
        pDevice->isrNumber = pSettings->isrNumber;
        pDevice->gpioInterrupt = pSettings->gpioInterrupt;
        pDevice->isMtsif = pSettings->isMtsif;
        pDevice->pGenericDeviceHandle = pFrontendDevice;
        pDevice->previousStandbyMode = NEXUS_StandbyMode_eOn;
        NEXUS_FrontendDevice_GetDefault3128Settings(&pDevice->deviceSettings);
        rc = NEXUS_Frontend_P_Init_7584_Hab(pDevice, pSettings);
        if(rc){rc = BERR_TRACE(rc); goto err_init_hab;}

        /* Initialize ADS channels and OOB */
        rc = NEXUS_Frontend_P_Init7584(pDevice, pSettings);
        if(rc){rc = BERR_TRACE(rc); goto err_init;}

        pDevice->updateGainAppCallback = NEXUS_IsrCallback_Create(NULL, NULL);
        if ( NULL == pDevice->updateGainAppCallback ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err_ch_alloc;}

        pDevice->cppmAppCallback = NEXUS_IsrCallback_Create(pDevice, NULL);
        if ( NULL == pDevice->cppmAppCallback ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err_callback_destroy;}

        pDevice->cppmDoneCallback = NEXUS_IsrCallback_Create(pDevice, NULL);
        if ( NULL == pDevice->cppmDoneCallback ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err_callback_destroy;}

        rc = BKNI_CreateEvent(&pDevice->spectrumEvent);
        if(rc){rc = BERR_TRACE(rc); goto err_callback_destroy;}

        BLST_S_INSERT_HEAD(&g_7584DevList, pDevice, node);
    }
    else
    {
        BDBG_MSG(("Found 312x device"));
        return pDevice->pGenericDeviceHandle;
    }

    pFrontendDevice->pDevice = pDevice;
    pFrontendDevice->familyId = results.chip.familyId;
    pFrontendDevice->mode = NEXUS_StandbyMode_eOn;
    pFrontendDevice->bypassableFixedGain = pSettings->externalFixedGain.bypassable;
    pFrontendDevice->totalFixedBoardGain = pSettings->externalFixedGain.total;
    pFrontendDevice->application = NEXUS_FrontendDeviceApplication_eCable;
    pFrontendDevice->getInternalGain = NEXUS_FrontendDevice_P_7584_GetInternalGain;
    pFrontendDevice->getExternalGain = NEXUS_FrontendDevice_P_7584_GetExternalGain;
    pFrontendDevice->setExternalGain = NEXUS_FrontendDevice_P_7584_SetExternalGain;
    pFrontendDevice->standby = NEXUS_FrontendDevice_P_7584_Standby;
    pFrontendDevice->getStatus = NEXUS_FrontendDevice_P_7584_GetStatus;
    pFrontendDevice->getCapabilities = NEXUS_FrontendDevice_P_7584_GetCapabilities;
    pFrontendDevice->getTunerCapabilities = NEXUS_FrontendDevice_P_7584_GetTunerCapabilities;
    pFrontendDevice->recalibrate = NEXUS_FrontendDevice_P_7584_Recalibrate;
    pFrontendDevice->close = NEXUS_FrontendDevice_P_7584_Close;
#if NEXUS_AMPLIFIER_SUPPORT
    pFrontendDevice->getAmplifierStatus = NEXUS_FrontendDevice_P_7584_GetAmplifierStatus;
    pFrontendDevice->setAmplifierStatus = NEXUS_FrontendDevice_P_7584_SetAmplifierStatus;
#endif
    return pFrontendDevice;

err_callback_destroy:
    if ( pDevice->updateGainAppCallback ) NEXUS_IsrCallback_Destroy(pDevice->updateGainAppCallback);
    if ( pDevice->cppmDoneCallback ) NEXUS_IsrCallback_Destroy(pDevice->cppmDoneCallback);
    if ( pDevice->cppmAppCallback ) NEXUS_IsrCallback_Destroy(pDevice->cppmAppCallback);
err_ch_alloc:
    NEXUS_Frontend_P_UnInit7584(pDevice);
err_init:
    NEXUS_Frontend_P_UnInit_7584_Hab(pDevice);
err_init_hab:
    if ( pDevice ) BKNI_Free(pDevice);
    pDevice = NULL;
err_create:
    if(pFrontendDevice) BKNI_Free(pFrontendDevice);
    return NULL;

}


/***************************************************************************
Summary:
    Open a handle to a BCM7584 device.
 ***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_Open3128(const NEXUS_3128Settings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_FrontendHandle frontendHandle = NULL;
    NEXUS_7584 *pDevice = NULL;
    unsigned int chn_num=0;
    NEXUS_IsrCallbackHandle callback;
    NEXUS_IsrCallbackHandle qamAsyncStatusReadyCallback = NULL;
    NEXUS_TaskCallbackHandle spectrumDataCallback  = NULL;
    NEXUS_7584Channel *pChannel;
    NEXUS_FrontendDevice3128OpenSettings openSettings;
    NEXUS_FrontendDevice *pFrontendDevice = NULL;
#if NEXUS_FRONTEND_7584_OOB
    NEXUS_IsrCallbackHandle oobAsyncStatusReadyCallback = NULL;
#endif

    BDBG_ASSERT(NULL != pSettings);

    if ( ((pSettings->channelNumber >= NEXUS_3128_MAX_DOWNSTREAM_CHANNELS)&& pSettings->type == NEXUS_3128ChannelType_eInBand)
        || (pSettings->channelNumber >= NEXUS_MAX_7584_FRONTENDS ))
    {
        BDBG_ERR((" channel number exceeds available one"));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto err_create;
    }

    if(pSettings->device == NULL) {
        NEXUS_FrontendDevice_GetDefault3128OpenSettings(&openSettings);
        openSettings.configureWatchdog = pSettings->configureWatchdog;
        openSettings.gpioInterrupt = pSettings->gpioInterrupt;
        openSettings.i2cSlaveAddr  = pSettings->i2cSlaveAddr;
        openSettings.inBandOpenDrain  = pSettings->inBandOpenDrain;
        openSettings.isMtsif  = pSettings->isMtsif;
        openSettings.isrNumber  = pSettings->isrNumber;
        openSettings.loadAP  = pSettings->loadAP;
#if NEXUS_FRONTEND_7584_OOB
        openSettings.outOfBand.nyquist  = pSettings->outOfBand.nyquist;
        openSettings.outOfBand.ifFrequency  = pSettings->ifFrequency;
        openSettings.outOfBand.useWidebandAtoD  = pSettings->outOfBand.useWidebandAtoD;
        openSettings.outOfBand.openDrain  = pSettings->outOfBand.openDrain;
#endif
        pFrontendDevice = NEXUS_FrontendDevice_Open3128(0, &openSettings);
        pDevice = (NEXUS_7584 *)pFrontendDevice->pDevice;
    }
    else {
        pDevice = (NEXUS_7584 *)pSettings->device->pDevice;
        pFrontendDevice =  pSettings->device;
    }

    switch(pSettings->type)
    {
    case NEXUS_3128ChannelType_eInBand:
            chn_num = pSettings->channelNumber;
        break;
    case NEXUS_3128ChannelType_eOutOfBand:
        if(pDevice->aobPresent){
            chn_num = pDevice->oobChannelNumber;
            break;
        }
        else
            goto error_out;
    break;
error_out:
    default:
        BDBG_ERR((" channel type not supported"));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto err_create;
    }

    /* chekc if fronthandle is already opened*/
    if ( pDevice->frontendHandle[chn_num] != NULL )
    {
        return pDevice->frontendHandle[chn_num];
    }

    pChannel = (NEXUS_7584Channel*)BKNI_Malloc(sizeof(NEXUS_7584Channel));
    if ( NULL == pChannel ) {rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_create;}

    /* Create a Nexus frontend handle */
    frontendHandle = NEXUS_Frontend_P_Create(pChannel);
    if ( NULL == frontendHandle ) {rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_alloc;}

    /* Establish device capabilities */
    if ( pSettings->type == NEXUS_3128ChannelType_eInBand) {
        frontendHandle->capabilities.qam = true;
        frontendHandle->capabilities.outOfBand = false;
        frontendHandle->capabilities.upstream = false;
        BKNI_Memset(frontendHandle->capabilities.qamModes, true, sizeof(frontendHandle->capabilities.qamModes));
        /* bind functions*/
        frontendHandle->tuneQam = NEXUS_Frontend_P_7584_TuneQam;
        frontendHandle->getQamStatus = NEXUS_Frontend_P_7584_GetQamStatus;
        frontendHandle->readSoftDecisions = NEXUS_Frontend_P_7584_ReadSoftDecisions;
        frontendHandle->resetStatus = NEXUS_Frontend_P_7584_ResetQamStatus;
        frontendHandle->untune = NEXUS_Frontend_P_7584_UnTuneQam;
        frontendHandle->requestQamAsyncStatus = NEXUS_Frontend_P_7584_RequestQamAsyncStatus;
        frontendHandle->getQamAsyncStatus = NEXUS_Frontend_P_7584_GetQamAsyncStatus;
        frontendHandle->getQamScanStatus = NEXUS_Frontend_P_7584_GetQamScanStatus;
    }
#if NEXUS_FRONTEND_7584_OOB
    else if ( pSettings->type == NEXUS_3128ChannelType_eOutOfBand) {
        frontendHandle->capabilities.qam = false;
        frontendHandle->capabilities.outOfBand = true;
        frontendHandle->capabilities.upstream = false;
        BKNI_Memset(frontendHandle->capabilities.outOfBandModes, true, sizeof(frontendHandle->capabilities.outOfBandModes));
        /* bind functions*/
        frontendHandle->tuneOutOfBand = NEXUS_Frontend_P_7584_TuneOob;
        frontendHandle->getOutOfBandStatus = NEXUS_Frontend_P_7584_GetOobStatus;
        frontendHandle->readSoftDecisions = NEXUS_Frontend_P_7584_ReadSoftDecisions;
        frontendHandle->resetStatus = NEXUS_Frontend_P_7584_ResetOobStatus;
        frontendHandle->untune = NEXUS_Frontend_P_7584_UnTuneOob;
        frontendHandle->requestOutOfBandAsyncStatus = NEXUS_Frontend_P_7584_RequestOobAsyncStatus;
        frontendHandle->getOutOfBandAsyncStatus = NEXUS_Frontend_P_7584_GetOobAsyncStatus;
    }
#endif

    frontendHandle->close = NEXUS_Frontend_P_7584_Close;
    frontendHandle->getFastStatus = NEXUS_Frontend_P_7584_GetFastStatus;
    frontendHandle->getType = NEXUS_Frontend_P_7584_GetType;
    frontendHandle->requestSpectrumData = NEXUS_Frontend_P_7584_RequestSpectrumAnalyzerData;
    frontendHandle->writeRegister = NEXUS_Frontend_P_7584_WriteRegister;
    frontendHandle->readRegister = NEXUS_Frontend_P_7584_ReadRegister;
    frontendHandle->standby = NEXUS_Frontend_P_7584_Standby;
    frontendHandle->uninstallCallbacks = NEXUS_Frontend_P_7584_UninstallCallbacks;

    frontendHandle->pGenericDeviceHandle = pFrontendDevice;

    /* Create app callback */
    callback = NEXUS_IsrCallback_Create(frontendHandle, NULL);
    if ( NULL == callback ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err_cbk_create;}
    /* install callback to  notify of lock/unlock change */
    if ( pSettings->type == NEXUS_3128ChannelType_eInBand)
    {
        qamAsyncStatusReadyCallback = NEXUS_IsrCallback_Create(frontendHandle, NULL);
        if ( NULL == qamAsyncStatusReadyCallback ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err_callback_destroy;}

        spectrumDataCallback = NEXUS_TaskCallback_Create(frontendHandle, NULL);
        if ( NULL == spectrumDataCallback ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err_callback_destroy;}

        rc = BADS_InstallCallback(pDevice->ads_chn[chn_num], BADS_Callback_eLockChange, (BADS_CallbackFunc)NEXUS_Frontend_P_7584_callback_isr, (void*)pChannel);
        if(rc){rc = BERR_TRACE(rc); goto err_callback_destroy;}

        rc = BADS_InstallCallback(pDevice->ads_chn[chn_num], BADS_Callback_eUpdateGain, (BADS_CallbackFunc)NEXUS_Frontend_P_7584_updateGainCallback_isr, (void*)pDevice->updateGainAppCallback);
        if(rc){rc = BERR_TRACE(rc); goto err_callback_destroy;}

        rc = BADS_InstallCallback(pDevice->ads_chn[chn_num], BADS_Callback_eAsyncStatusReady, (BADS_CallbackFunc)NEXUS_Frontend_P_7584_AsyncStatusCallback_isr, (void*)pChannel);
        if(rc){rc = BERR_TRACE(rc); goto err_callback_destroy;}

        /* For now this is hooked up to the lock/unlock callback. */
        rc = BADS_InstallCallback(pDevice->ads_chn[chn_num], BADS_Callback_eNoSignal, (BADS_CallbackFunc)NEXUS_Frontend_P_7584_callback_isr, (void*)pChannel);
        if(rc){rc = BERR_TRACE(rc); goto err_callback_destroy;}

        rc = BTNR_InstallCallback(pDevice->tnr[chn_num], BTNR_Callback_eSpectrumDataReady, (BTNR_CallbackFunc)NEXUS_Frontend_P_7584_spetctrumDataReadyCallback, (void*)pDevice->spectrumEvent);
        if(rc){rc = BERR_TRACE(rc); goto err_callback_destroy;}

        pDevice->spectrumDataAppCallback[chn_num] = spectrumDataCallback;
        pDevice->asyncStatusAppCallback[chn_num] = qamAsyncStatusReadyCallback;
    }
    else if(pSettings->type == NEXUS_3128ChannelType_eOutOfBand)
    {
#if NEXUS_FRONTEND_7584_OOB
        if(pDevice->aobPresent){
            oobAsyncStatusReadyCallback = NEXUS_IsrCallback_Create(frontendHandle, NULL);
            if ( NULL == oobAsyncStatusReadyCallback ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err_oob_async_create;}

            rc = BAOB_InstallCallback(pDevice->aob, BAOB_Callback_eLockChange, (BAOB_CallbackFunc)NEXUS_Frontend_P_7584_callback_isr, (void*)pChannel);
            if(rc){rc = BERR_TRACE(rc); goto err_oob_async_create;}

            rc = BAOB_InstallCallback(pDevice->aob, BAOB_Callback_eAsyncStatusReady, (BAOB_CallbackFunc)NEXUS_Frontend_P_7584_AsyncStatusCallback_isr, (void*)pChannel);
            if(rc){rc = BERR_TRACE(rc); goto err_oob_async_create;}

            pDevice->asyncStatusAppCallback[chn_num] = oobAsyncStatusReadyCallback ;
        }
        else{
            BDBG_ERR(("Out of band is not supported on the 0x%x chip", pDevice->chipId));
        }
#endif
    }

    /* See if upstream needs a callback. */
    pDevice->lockAppCallback[chn_num] = callback;
    pDevice->frontendHandle[chn_num] = frontendHandle;
    /* save channel number in pChannel*/
    pChannel->chn_num = chn_num;
    pChannel->pDevice = pDevice;
    frontendHandle->chip.familyId = pDevice->chipFamilyId;
    frontendHandle->chip.id = pDevice->chipId;
    pDevice->numfrontends++;
    return frontendHandle;

#if NEXUS_FRONTEND_7584_OOB
err_oob_async_create:
    if ( oobAsyncStatusReadyCallback ) NEXUS_IsrCallback_Destroy(oobAsyncStatusReadyCallback);
#endif
err_callback_destroy:
    if ( spectrumDataCallback ) NEXUS_TaskCallback_Destroy(spectrumDataCallback);
    if ( qamAsyncStatusReadyCallback ) NEXUS_IsrCallback_Destroy(qamAsyncStatusReadyCallback);
    if ( callback ) NEXUS_IsrCallback_Destroy(callback);
err_cbk_create:
    if ( frontendHandle ) BKNI_Free(frontendHandle);
err_alloc:
    if ( pChannel ) BKNI_Free(pChannel);
    if(pDevice->numfrontends != 0)
        return NULL;
err_create:
    return NULL;
}

/***************************************************************************
Summary:
    Close a handle to a BCM7584 device.
See Also:
    NEXUS_Frontend_Open7584
 ***************************************************************************/
static void NEXUS_Frontend_P_7584_Close(
    NEXUS_FrontendHandle handle
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7584 *pDevice;
    NEXUS_7584Channel *pChannel;

    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    pChannel =(NEXUS_7584Channel*) handle->pDeviceHandle;
    BDBG_ASSERT(NULL != pChannel);
    pDevice = (NEXUS_7584 *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);

    if (pChannel->chn_num >= NEXUS_MAX_7584_FRONTENDS)
    {
        BDBG_ERR((" Unsupported Frontend Handle"));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    if((pDevice->frontendHandle[pChannel->chn_num]->capabilities.qam == true) &&(pDevice->ads_chn[pChannel->chn_num])){
        BADS_InstallCallback(pDevice->ads_chn[pChannel->chn_num], BADS_Callback_eLockChange, NULL, NULL);
        BADS_InstallCallback(pDevice->ads_chn[pChannel->chn_num], BADS_Callback_eUpdateGain, NULL, NULL);
        BADS_InstallCallback(pDevice->ads_chn[pChannel->chn_num], BADS_Callback_eAsyncStatusReady, NULL, NULL);
        BADS_InstallCallback(pDevice->ads_chn[pChannel->chn_num], BADS_Callback_eNoSignal, NULL, NULL);
        BTNR_InstallCallback(pDevice->tnr[pChannel->chn_num], BTNR_Callback_eSpectrumDataReady, NULL, NULL);
    }

#if NEXUS_FRONTEND_312x_OOB
    if(pDevice->frontendHandle[pChannel->chn_num]->capabilities.outOfBand == true){
        BAOB_InstallCallback(pDevice->aob, BAOB_Callback_eLockChange, NULL, NULL);
        BAOB_InstallCallback(pDevice->aob, BAOB_Callback_eAsyncStatusReady, NULL, NULL);
    }
#endif

    if ( NULL != pDevice->lockAppCallback[pChannel->chn_num])
        NEXUS_IsrCallback_Destroy(pDevice->lockAppCallback[pChannel->chn_num]);
    if ( NULL != pDevice->asyncStatusAppCallback[pChannel->chn_num])
        NEXUS_IsrCallback_Destroy(pDevice->asyncStatusAppCallback[pChannel->chn_num]);

    if (pChannel->chn_num < NEXUS_3128_MAX_DOWNSTREAM_CHANNELS) {
        if(pDevice->spectrumDataAppCallback[pChannel->chn_num]) {
            NEXUS_TaskCallback_Destroy(pDevice->spectrumDataAppCallback[pChannel->chn_num]);
        }
    }

    NEXUS_Frontend_P_Destroy(handle);

    pDevice->frontendHandle[pChannel->chn_num] = NULL;
    BKNI_Free(pChannel);

    pDevice->numfrontends--;
done:
    return;
}

static void NEXUS_FrontendDevice_P_7584_Close(void *handle)
{
    NEXUS_7584 *pDevice;
    unsigned i=0;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7584 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);

    if(pDevice->numfrontends){
        for(i = 0; i< NEXUS_MAX_7584_FRONTENDS; i++){
            if(pDevice->frontendHandle[i])NEXUS_Frontend_P_7584_Close(pDevice->frontendHandle[i]);
        }
    }

    if(pDevice->updateGainAppCallback) {
        NEXUS_IsrCallback_Destroy(pDevice->updateGainAppCallback);
        pDevice->updateGainAppCallback = NULL;
    }
    if(pDevice->cppmDoneCallback) {
        NEXUS_IsrCallback_Destroy(pDevice->cppmDoneCallback);
        pDevice->cppmDoneCallback = NULL;
    }
    if(pDevice->cppmAppCallback) {
        NEXUS_IsrCallback_Destroy(pDevice->cppmAppCallback);
        pDevice->cppmAppCallback = NULL;
    }

    if(pDevice->spectrumEventCallback)NEXUS_UnregisterEvent(pDevice->spectrumEventCallback);
    pDevice->spectrumEventCallback = NULL;

    if (pDevice->spectrumEvent) {
        BKNI_DestroyEvent(pDevice->spectrumEvent);
        pDevice->spectrumEvent = NULL;
    }

    NEXUS_Frontend_P_UnInit7584(pDevice);
    NEXUS_Frontend_P_UnInit_7584_Hab(pDevice);

    NEXUS_FrontendDevice_Unlink(pDevice->pGenericDeviceHandle, NULL);
    BKNI_Free(pDevice->pGenericDeviceHandle);
    pDevice->pGenericDeviceHandle = NULL;

    if(pDevice->capabilities.channelCapabilities)
        BKNI_Free(pDevice->capabilities.channelCapabilities);
    pDevice->capabilities.channelCapabilities = NULL;

    BLST_S_REMOVE(&g_7584DevList, pDevice, NEXUS_7584, node);
    BDBG_OBJECT_DESTROY(pDevice, NEXUS_7584);
    BKNI_Free(pDevice);
    pDevice = NULL;

    return;
}

#if NEXUS_AMPLIFIER_SUPPORT
NEXUS_Error NEXUS_FrontendDevice_P_7584_GetAmplifierStatus(void *handle, NEXUS_AmplifierStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7584 *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7584 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));


    if(pDevice->pGenericDeviceHandle->parent != NULL){
        rc = NEXUS_FrontendDevice_P_GetAmplifierStatus(pDevice->pGenericDeviceHandle->parent, pStatus);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    else if(pDevice->pGenericDeviceHandle->amplifier != NULL){
        rc = NEXUS_Amplifier_GetStatus(pDevice->pGenericDeviceHandle->amplifier, pStatus);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    else {
        BDBG_ERR(("Amplifier not linked to the parent device."));
    }

done:
    return rc;
}

NEXUS_Error NEXUS_FrontendDevice_P_7584_SetAmplifierStatus(void *handle, const NEXUS_AmplifierStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7584 *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7584 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);

    BSTD_UNUSED(pStatus);
    /* Set LNA/Amplifier parameters. */

    return rc;

}
#endif

static NEXUS_Error NEXUS_FrontendDevice_P_7584_GetInternalGain(void *handle, const NEXUS_GainParameters *params, NEXUS_InternalGainSettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7584 *pDevice;
    BHAB_InternalGainInputParams inputParams;
    BHAB_InternalGainSettings internalGain;
    NEXUS_GainParameters gainParams;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7584 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);


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

static NEXUS_Error NEXUS_FrontendDevice_P_7584_GetExternalGain(void *handle, NEXUS_ExternalGainSettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7584 *pDevice;
    BHAB_ExternalGainSettings gain;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7584 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);
    BSTD_UNUSED(pSettings);


    rc = BHAB_GetExternalGain(pDevice->hab, &gain);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pSettings->bypassableGain = gain.externalGainBypassable;
    pSettings->totalGain = gain.externalGainTotal;

done:
    return rc;

}

static NEXUS_Error NEXUS_FrontendDevice_P_7584_SetExternalGain(void *handle, const NEXUS_ExternalGainSettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7584 *pDevice;
    BHAB_ExternalGainSettings externalGain;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7584 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);


    externalGain.externalGainTotal = pSettings->totalGain;
    externalGain.externalGainBypassable = pSettings->bypassableGain;

    rc = BHAB_SetExternalGain(pDevice->hab, &externalGain);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}

NEXUS_Error NEXUS_FrontendDevice_P_7584_Recalibrate(void *handle, const NEXUS_FrontendDeviceRecalibrateSettings *pSettings)
{
    BERR_Code rc = BERR_SUCCESS;
    BHAB_RecalibrateSettings recalibrateSettings;
    NEXUS_7584 *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7584 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);
    BDBG_ASSERT(NULL != handle);

    NEXUS_IsrCallback_Set(pDevice->cppmAppCallback, &(pSettings->cppm.powerLevelChange));
    NEXUS_IsrCallback_Set(pDevice->cppmDoneCallback, &(pSettings->cppm.calibrationComplete));

    rc = BHAB_7584_GetDefaultRecalibrateSettings(&recalibrateSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    recalibrateSettings.cppm.enabled = pSettings->cppm.enabled;
    recalibrateSettings.cppm.threshold = pSettings->cppm.threshold;
    recalibrateSettings.cppm.thresholdHysteresis = pSettings->cppm.thresholdHysteresis;

    rc = BHAB_SetRecalibrateSettings(pDevice->hab, &recalibrateSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}

static void NEXUS_FrontendDevice_P_7584_GetCapabilities(void *handle, NEXUS_FrontendDeviceCapabilities *pCapabilities)
{
    NEXUS_7584 *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7584 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);

    pCapabilities->numTuners = pDevice->capabilities.totalTunerChannels;
    return;
}

static void NEXUS_FrontendDevice_P_7584_GetTunerCapabilities(void *handle, unsigned tunerIndex, NEXUS_FrontendCapabilities *pCapabilities)
{
    NEXUS_7584 *pDevice;
    BHAB_ChannelCapability *channelCapability;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7584 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);
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


static NEXUS_Error NEXUS_Frontend_P_7584_TuneQam(void *handle, const NEXUS_FrontendQamSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BADS_InbandParam ibParam;
    unsigned chn_num;
    BTNR_PowerSaverSettings pwrSettings;
    NEXUS_7584 *pDevice;
    NEXUS_7584Channel *pChannel;
    BADS_ChannelScanSettings scanParam;
    unsigned temp_frequency;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7584Channel *)handle;
    pDevice = pChannel->pDevice;
    chn_num = pChannel->chn_num;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);

    if (chn_num >= NEXUS_3128_MAX_DOWNSTREAM_CHANNELS)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    rc = NEXUS_Tuner_P_7584_GetNSetGain(pChannel);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    temp_frequency = pDevice->last_ads[chn_num].frequency;
    pDevice->last_ads[chn_num].frequency = pSettings->frequency;

    if((!BKNI_Memcmp(pSettings, &pDevice->last_ads[chn_num], sizeof(NEXUS_FrontendQamSettings))) && (pSettings->acquisitionMode == NEXUS_FrontendQamAcquisitionMode_eScan)){
        if (pDevice->tnr[chn_num])
        {
            if(!pDevice->isTunerPoweredOn[chn_num]){
                pwrSettings.enable = false;
                rc = BTNR_SetPowerSaver(pDevice->tnr[chn_num], &pwrSettings);
                if(rc){rc = BERR_TRACE(rc); goto retrack;}
                pDevice->isTunerPoweredOn[chn_num] = true;
                pDevice->numTunerPoweredOn++;
            }
            if(!pDevice->isPoweredOn[chn_num]){
                rc = BADS_DisablePowerSaver(pDevice->ads_chn[chn_num]);
                if(rc){rc = BERR_TRACE(rc); goto retrack;}
                pDevice->isPoweredOn[chn_num] = true;
            }

            pDevice->count[chn_num] = 0;
            pDevice->acquireInProgress[chn_num] = true;
            rc = BTNR_SetTunerRfFreq(pDevice->tnr[chn_num], pSettings->frequency, BTNR_TunerMode_eDigital);
            if(rc){rc = BERR_TRACE(rc); goto retrack;}

            pDevice->last_ads[chn_num] = *pSettings;

            return rc;
        }
    }

    rc = BADS_GetDefaultAcquireParams(pDevice->ads_chn[chn_num], &ibParam);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if (( pSettings->annex == NEXUS_FrontendQamAnnex_eA ) || ( pSettings->annex == NEXUS_FrontendQamAnnex_eC ))
    {
        switch ( pSettings->mode )
        {
        case NEXUS_FrontendQamMode_e16:
            ibParam.modType = BADS_ModulationType_eAnnexAQam16;
            break;
        case NEXUS_FrontendQamMode_e32:
            ibParam.modType = BADS_ModulationType_eAnnexAQam32;
            break;
        case NEXUS_FrontendQamMode_e64:
            ibParam.modType = BADS_ModulationType_eAnnexAQam64;
            break;
        case NEXUS_FrontendQamMode_e128:
            ibParam.modType = BADS_ModulationType_eAnnexAQam128;
            break;
        case NEXUS_FrontendQamMode_e256:
            ibParam.modType = BADS_ModulationType_eAnnexAQam256;
            break;
        default:
            rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
        }

        ibParam.enableNullPackets = pSettings->enableNullPackets;
        if( pSettings->annex == NEXUS_FrontendQamAnnex_eC ){
            if( (pSettings->symbolRate != 5274000) && (pSettings->symbolRate != 5307000))
                BDBG_WRN((" Symbol rate for Annex-C is incorrect."));
        }
    }
    else if ( pSettings->annex == NEXUS_FrontendQamAnnex_eB )
    {
        switch ( pSettings->mode )
        {
        case NEXUS_FrontendQamMode_e64:
            ibParam.modType = BADS_ModulationType_eAnnexBQam64;
            break;
        case NEXUS_FrontendQamMode_e256:
            ibParam.modType = BADS_ModulationType_eAnnexBQam256;
            break;
        default:
            rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
        }

        ibParam.enableNullPackets = false;
    }
    else
    {
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }

    NEXUS_IsrCallback_Set(pDevice->lockAppCallback[chn_num], &(pSettings->lockCallback));
    NEXUS_IsrCallback_Set(pDevice->asyncStatusAppCallback[chn_num], &(pSettings->asyncStatusReadyCallback));

    if (pDevice->tnr[chn_num])
    {
        if(!pDevice->isTunerPoweredOn[chn_num]){
            pwrSettings.enable = false;
            rc = BTNR_SetPowerSaver(pDevice->tnr[chn_num], &pwrSettings);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->isTunerPoweredOn[chn_num] = true;
            pDevice->numTunerPoweredOn++;
        }
        if(!pDevice->isPoweredOn[chn_num]){
            rc = BADS_DisablePowerSaver(pDevice->ads_chn[chn_num]);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            pDevice->isPoweredOn[chn_num] = true;
        }
        pDevice->count[chn_num] = 0;
        pDevice->acquireInProgress[chn_num] = true;
        rc = BTNR_SetTunerRfFreq(pDevice->tnr[chn_num], pSettings->frequency, BTNR_TunerMode_eDigital);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    /* Scan Parameters */
    if((pSettings->acquisitionMode == NEXUS_FrontendQamAcquisitionMode_eScan) || (pSettings->acquisitionMode == NEXUS_FrontendQamAcquisitionMode_eAuto)){
        BKNI_Memset(&scanParam, 0, sizeof(scanParam));
        scanParam.QM = true;
        scanParam.TO = true;
        if( pSettings->spectrumMode == NEXUS_FrontendQamSpectrumMode_eAuto) scanParam.AI = true;
        if(pSettings->scan.mode[NEXUS_FrontendQamAnnex_eA][NEXUS_FrontendQamMode_e16]) scanParam.A16 = true;
        if(pSettings->scan.mode[NEXUS_FrontendQamAnnex_eA][NEXUS_FrontendQamMode_e32]) scanParam.A32= true;
        if(pSettings->scan.mode[NEXUS_FrontendQamAnnex_eA][NEXUS_FrontendQamMode_e64]) scanParam.A64 = true;
        if(pSettings->scan.mode[NEXUS_FrontendQamAnnex_eA][NEXUS_FrontendQamMode_e128]) scanParam.A128 = true;
        if(pSettings->scan.mode[NEXUS_FrontendQamAnnex_eA][NEXUS_FrontendQamMode_e256]) scanParam.A256 = true;
        if(pSettings->scan.mode[NEXUS_FrontendQamAnnex_eA][NEXUS_FrontendQamMode_e512]) scanParam.A512 = true;
        if(pSettings->scan.mode[NEXUS_FrontendQamAnnex_eA][NEXUS_FrontendQamMode_e1024]) scanParam.A1024 = true;
        if(pSettings->scan.mode[NEXUS_FrontendQamAnnex_eB][NEXUS_FrontendQamMode_e64]) scanParam.B64 = true;
        if(pSettings->scan.mode[NEXUS_FrontendQamAnnex_eB][NEXUS_FrontendQamMode_e256]) scanParam.B256 = true;
        if(pSettings->scan.mode[NEXUS_FrontendQamAnnex_eB][NEXUS_FrontendQamMode_e1024]) scanParam.B1024 = true;
        if(pSettings->scan.frequencyOffset){
            scanParam.CO = true;
            scanParam.carrierSearch = pSettings->scan.frequencyOffset/256;
        }
        scanParam.upperBaudSearch = pSettings->scan.upperBaudSearch;
        scanParam.lowerBaudSearch = pSettings->scan.lowerBaudSearch;

        rc = BADS_SetScanParam(pDevice->ads_chn[chn_num], &scanParam );
        if(rc){rc = BERR_TRACE(rc); goto done;}

    }

    if(pSettings->acquisitionMode == NEXUS_FrontendQamAcquisitionMode_eScan) {
        ibParam.tuneAcquire = true;
    }
    else{
        ibParam.tuneAcquire = false;
    }

    ibParam.symbolRate = pSettings->symbolRate;
    ibParam.autoAcquire = pSettings->autoAcquire;
    ibParam.enableDpm = pSettings->enablePowerMeasurement;
    ibParam.spectrum = pSettings->spectrumMode;
    ibParam.invertSpectrum = pSettings->spectralInversion;
    ibParam.frequencyOffset =  pSettings->frequencyOffset;
    ibParam.acquisitionType = pSettings->acquisitionMode;

    rc = BADS_SetAcquireParams(pDevice->ads_chn[chn_num], &ibParam );
    if(rc){rc = BERR_TRACE(rc); goto done;}

    rc = BADS_Acquire(pDevice->ads_chn[chn_num], &ibParam );
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pDevice->last_ads[chn_num] = *pSettings;

    return BERR_SUCCESS;

retrack:
    pDevice->last_ads[chn_num].frequency = temp_frequency;
done:
    NEXUS_Frontend_P_7584_UnTuneQam(handle);
    return rc;
}

static void NEXUS_Frontend_P_7584_UninstallCallbacks(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7584 *pDevice;
    NEXUS_7584Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7584Channel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);

    if (pChannel->chn_num >= NEXUS_3128_MAX_DOWNSTREAM_CHANNELS)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    if(pDevice->lockAppCallback[pChannel->chn_num])NEXUS_IsrCallback_Set(pDevice->lockAppCallback[pChannel->chn_num], NULL);
    if(pDevice->asyncStatusAppCallback[pChannel->chn_num])NEXUS_IsrCallback_Set(pDevice->asyncStatusAppCallback[pChannel->chn_num], NULL);
    if(pDevice->spectrumDataAppCallback[pChannel->chn_num])NEXUS_TaskCallback_Set(pDevice->spectrumDataAppCallback[pChannel->chn_num], NULL);

done:
    return;
}

static void NEXUS_Frontend_P_7584_UnTuneQam(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BADS_InbandParam ibParam;
    BTNR_PowerSaverSettings pwrSettings;
    NEXUS_7584 *pDevice;
    NEXUS_7584Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7584Channel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);

    if (pChannel->chn_num >= NEXUS_3128_MAX_DOWNSTREAM_CHANNELS)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    if(pDevice->isPoweredOn[pChannel->chn_num]){
        BKNI_Memset(&ibParam, 0x0, sizeof(ibParam));
        rc = BADS_SetAcquireParams(pDevice->ads_chn[pChannel->chn_num], &ibParam );
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = BADS_EnablePowerSaver(pDevice->ads_chn[pChannel->chn_num]);
        if(rc){rc = BERR_TRACE(rc); goto done;}

       pDevice->isPoweredOn[pChannel->chn_num] = false;
    }

    if(pDevice->numTunerPoweredOn){
        pwrSettings.enable = true;
        rc = BTNR_SetPowerSaver(pDevice->tnr[pChannel->chn_num], &pwrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pDevice->isTunerPoweredOn[pChannel->chn_num] = false;
        pDevice->numTunerPoweredOn--;
    }

    BKNI_Memset(&pDevice->last_ads[pChannel->chn_num], 0x0, sizeof(NEXUS_FrontendQamSettings));

done:
    return;
}

static NEXUS_Error NEXUS_FrontendDevice_P_7584_Standby(void *handle, const NEXUS_StandbySettings *pSettings)
{

    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7584 *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7584 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);

    if((pDevice->previousStandbyMode != NEXUS_StandbyMode_eDeepSleep) && (pSettings->mode == NEXUS_StandbyMode_eDeepSleep)){
        rc = BHAB_Reset(pDevice->hab);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    else if((pDevice->previousStandbyMode == NEXUS_StandbyMode_eDeepSleep) && (pSettings->mode != NEXUS_StandbyMode_eDeepSleep)){
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

            rc = Nexus_Core_P_Img_Create(NEXUS_CORE_IMG_ID_FRONTEND_7584, &pImgContext, &imgInterface);
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
        fw_image = bcm7584_leap_image;
#endif
        rc = BHAB_InitAp(pDevice->hab, fw_image);
#if NEXUS_MODE_driver
        NEXUS_Memory_Free(fw);
#endif
        if ( rc != BERR_SUCCESS ) goto done;
    }
    pDevice->previousStandbyMode = pSettings->mode;

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7584_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BTNR_PowerSaverSettings pwrSettings;
    NEXUS_7584 *pDevice;
    NEXUS_7584Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7584Channel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);

    BSTD_UNUSED(enabled);

    if (pChannel->chn_num >= NEXUS_MAX_7584_FRONTENDS)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    if(pSettings->mode >= NEXUS_StandbyMode_ePassive){
        NEXUS_Frontend_P_7584_UninstallCallbacks(handle);
        if(pDevice->isPoweredOn[pChannel->chn_num]){
            if ((pDevice->aobPresent) && (pChannel->chn_num == pDevice->oobChannelNumber)) {
#if NEXUS_FRONTEND_7584_OOB
                BDBG_ASSERT(pSettings);
                if (!pSettings->wakeupSettings.transport) {
                    rc = BAOB_EnablePowerSaver(pDevice->aob);
                    if(rc){rc = BERR_TRACE(rc); goto done;}
                }
#endif
            }
            else{
                rc = BADS_EnablePowerSaver(pDevice->ads_chn[pChannel->chn_num]);
                if(rc){rc = BERR_TRACE(rc); goto done;}
            }
            pDevice->isPoweredOn[pChannel->chn_num] = false;
        }
        if(pDevice->isTunerPoweredOn[pChannel->chn_num]){
            pwrSettings.enable = true;
            rc = BTNR_SetPowerSaver(pDevice->tnr[pChannel->chn_num], &pwrSettings);
            if (rc){rc = BERR_TRACE(rc);}
            pDevice->isTunerPoweredOn[pChannel->chn_num] = false;
            pDevice->numTunerPoweredOn--;
        }
    }

done:
    return rc;
}

static NEXUS_FrontendLockStatus  NEXUS_Frontend_P_GetAdsLockStatus(BADS_LockStatus lockStatus)
{
    switch ( lockStatus )
    {
    case BADS_LockStatus_eUnlocked:
        return NEXUS_FrontendLockStatus_eUnlocked;
    case BADS_LockStatus_eLocked:
        return NEXUS_FrontendLockStatus_eLocked;
    case BADS_LockStatus_eNoSignal:
        return NEXUS_FrontendLockStatus_eNoSignal;
    default:
        BDBG_WRN(("Unrecognized lock status (%d) ", lockStatus));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return NEXUS_FrontendLockStatus_eUnknown;
    }
}

#if NEXUS_FRONTEND_7584_OOB
static NEXUS_FrontendLockStatus  NEXUS_Frontend_P_GetAobLockStatus(BAOB_LockStatus lockStatus)
{
    switch ( lockStatus )
    {
    case BAOB_LockStatus_eUnlocked:
        return NEXUS_FrontendLockStatus_eUnlocked;
    case BAOB_LockStatus_eLocked:
        return NEXUS_FrontendLockStatus_eLocked;
    case BAOB_LockStatus_eNoSignal:
        return NEXUS_FrontendLockStatus_eNoSignal;
    default:
        BDBG_WRN(("Unrecognized lock status (%d) ", lockStatus));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return NEXUS_FrontendLockStatus_eUnknown;
    }
}
#endif

static NEXUS_Error NEXUS_Frontend_P_7584_GetFastStatus(void *handle, NEXUS_FrontendFastStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7584 *pDevice;
    NEXUS_7584Channel *pChannel;
    BADS_LockStatus eLock = BADS_LockStatus_eUnlocked;
#if NEXUS_FRONTEND_7584_OOB
    BAOB_LockStatus bLock = BAOB_LockStatus_eUnlocked;
#endif
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7584Channel *)handle;
    pDevice = (NEXUS_7584 *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);

    pStatus->lockStatus = NEXUS_FrontendLockStatus_eUnknown;

    if(pDevice->ads_chn[pChannel->chn_num] && (pChannel->chn_num < NEXUS_3128_MAX_DOWNSTREAM_CHANNELS)){
        rc = BADS_GetLockStatus(pDevice->ads_chn[pChannel->chn_num],  &eLock);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pStatus->lockStatus = NEXUS_Frontend_P_GetAdsLockStatus(eLock);
    }
#if NEXUS_FRONTEND_7584_OOB
    else if(pChannel->chn_num == pDevice->oobChannelNumber){
        rc = BAOB_GetLockStatus(pDevice->aob,  &bLock);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pStatus->lockStatus = NEXUS_Frontend_P_GetAobLockStatus(bLock);
    }
#endif
    pStatus->acquireInProgress = pDevice->acquireInProgress[pChannel->chn_num];
    return BERR_SUCCESS;
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7584_GetQamScanStatus(void *handle, NEXUS_FrontendQamScanStatus *pScanStatus)
{
    NEXUS_Error  rc;
    struct BADS_ScanStatus st;
    NEXUS_7584 *pDevice;
    NEXUS_7584Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7584Channel *)handle;
    pDevice = (NEXUS_7584 *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);

    BKNI_Memset(pScanStatus, 0, sizeof(*pScanStatus));

    if (pChannel->chn_num >= NEXUS_3128_MAX_DOWNSTREAM_CHANNELS)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    rc = BADS_GetScanStatus(pDevice->ads_chn[pChannel->chn_num],  &st);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pScanStatus->spectrumInverted = st.isSpectrumInverted;
    pScanStatus->symbolRate = st.symbolRate;
    pScanStatus->frequencyOffset = st.carrierFreqOffset;
    pScanStatus->interleaver = st.interleaver;
    pScanStatus->acquisitionStatus = st.acquisitionStatus;
    if(st.modType < BADS_ModulationType_eAnnexBQam16) {
        pScanStatus->annex = NEXUS_FrontendQamAnnex_eA;
        pScanStatus->mode = st.modType;
    }
    else if(st.modType < BADS_ModulationType_eAnnexCQam16) {
        pScanStatus->annex = NEXUS_FrontendQamAnnex_eB;
        pScanStatus->mode = st.modType - BADS_ModulationType_eAnnexBQam16;
    }
    else
        BDBG_ERR(("Unsupported Annex."));
    return BERR_SUCCESS;
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7584_GetQamStatus(void *handle, NEXUS_FrontendQamStatus *pStatus)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned j=0;
    uint32_t buf=0;
    NEXUS_7584 *pDevice;
    NEXUS_7584Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7584Channel *)handle;
    pDevice = (NEXUS_7584 *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);

    if (pChannel->chn_num >= NEXUS_3128_MAX_DOWNSTREAM_CHANNELS)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    if(pDevice->isPoweredOn[pChannel->chn_num]) {
        pDevice->isInternalAsyncStatusCall[pChannel->chn_num] = true;

        rc = NEXUS_Frontend_P_7584_RequestQamAsyncStatus(pChannel);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        for(j=0; j < 100; j++) {

            BKNI_Sleep(20);

            rc = BHAB_ReadRegister(pDevice->hab, BCHP_LEAP_CTRL_SW_SPARE0 , &buf);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            if (buf & (BHAB_ADS_CHN0_STATUS_RDY << pChannel->chn_num)) {
                rc = NEXUS_Frontend_P_7584_GetQamAsyncStatus(pChannel, pStatus);
                if(rc){rc = BERR_TRACE(rc); goto done;}
                break;
            }
        }

        pDevice->isInternalAsyncStatusCall[pChannel->chn_num] = false;
    }
    else{
        BDBG_MSG(("The downstream core is Powered Off"));
    }
    return BERR_SUCCESS;
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7584_GetQamAsyncStatus(void *handle, NEXUS_FrontendQamStatus *pStatus)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    struct BADS_Status st;
    uint64_t totalbits=0, uncorrectedBits=0;
    unsigned cleanBlock = 0, correctedBlock = 0, unCorrectedBlock = 0, totalBlock = 0;
    NEXUS_Time currentTime;
    NEXUS_PreviousStatus *prevStatus;
    NEXUS_7584 *pDevice;
    NEXUS_7584Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7584Channel *)handle;
    pDevice = (NEXUS_7584 *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);

    if (pChannel->chn_num >= NEXUS_3128_MAX_DOWNSTREAM_CHANNELS)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    rc = BADS_GetAsyncStatus(pDevice->ads_chn[pChannel->chn_num],  &st);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    prevStatus = &pDevice->previousStatus[pChannel->chn_num];
    NEXUS_Time_Get(&currentTime);
    pStatus->fecLock = st.isFecLock;
    pStatus->receiverLock = st.isQamLock;
    pStatus->rfAgcLevel = st.agcExtLevel;
    pStatus->ifAgcLevel = st.agcIntLevel;
    pStatus->snrEstimate = st.snrEstimate*100/256;
    pStatus->fecCorrected = st.accCorrectedCount;
    pStatus->fecUncorrected = st.accUncorrectedCount;
    pStatus->fecClean = st.accCleanCount;
    pStatus->carrierPhaseOffset = st.carrierPhaseOffset;
    pStatus->carrierFreqOffset = st.carrierFreqOffset;
    pStatus->symbolRate = st.rxSymbolRate;
    pStatus->berRawCount = st.berRawCount;
    pStatus->dsChannelPower = st.dsChannelPower;
    pStatus->mainTap = st.mainTap;
    pStatus->settings = pDevice->last_ads[pChannel->chn_num];
    pStatus->spectrumInverted = st.isSpectrumInverted;
    pStatus->highResEqualizerGain = st.equalizerGain;
    pStatus->digitalAgcGain = st.digitalAgcGain;
    pStatus->frontendGain= st.feGain;

    pStatus->postRsBerElapsedTime = NEXUS_Time_Diff(&currentTime, &prevStatus->time);

    if(pStatus->fecUncorrected  > prevStatus->fecUncorrected)
        unCorrectedBlock = pStatus->fecUncorrected - prevStatus->fecUncorrected;
    if(pStatus->fecClean > prevStatus->fecClean)
        cleanBlock = pStatus->fecClean - prevStatus->fecClean;
    if(pStatus->fecCorrected > prevStatus->fecCorrected)
        correctedBlock = pStatus->fecCorrected - prevStatus->fecCorrected;

    totalBlock = (uint64_t)(unCorrectedBlock + cleanBlock + correctedBlock);

    if(totalBlock > unCorrectedBlock){
        unCorrectedBlock = (uint64_t)unCorrectedBlock * 11224 / 1000;
        if(pStatus->settings.annex == NEXUS_FrontendQamAnnex_eA)
            pStatus->postRsBer = ((uint64_t)unCorrectedBlock * 2097152 * 1024 )/((uint64_t)totalBlock*8*187);
        else if(pStatus->settings.annex == NEXUS_FrontendQamAnnex_eB)
            pStatus->postRsBer = ((uint64_t)unCorrectedBlock * 2097152 * 1024)/((uint64_t)totalBlock*7*122);
    }

    pStatus->viterbiUncorrectedBits = st.correctedBits + (uint32_t)((uint64_t)pStatus->fecUncorrected * 11224)/1000;
    if(pStatus->viterbiUncorrectedBits > prevStatus->viterbiUncorrectedBits)
        uncorrectedBits = pStatus->viterbiUncorrectedBits - prevStatus->viterbiUncorrectedBits;

    if(pStatus->settings.annex == NEXUS_FrontendQamAnnex_eA){
        pStatus->viterbiTotalBits = (uint32_t)(((uint64_t)pStatus->fecCorrected + (uint64_t)pStatus->fecUncorrected + (uint64_t)pStatus->fecClean) * 204 * 8);
    }
    else if(pStatus->settings.annex == NEXUS_FrontendQamAnnex_eB){
        pStatus->viterbiTotalBits = (uint32_t)(((uint64_t)pStatus->fecCorrected + (uint64_t)pStatus->fecUncorrected + (uint64_t)pStatus->fecClean) * 127 * 7);
    }

    if(pStatus->viterbiTotalBits > prevStatus->viterbiTotalBits)
        totalbits =  pStatus->viterbiTotalBits - prevStatus->viterbiTotalBits;

    if (totalbits > uncorrectedBits) {
        pStatus->viterbiErrorRate = (uint32_t)((uint64_t)uncorrectedBits * 2097152 * 1024 / totalbits);
    }

    prevStatus->fecUncorrected = pStatus->fecUncorrected;
    prevStatus->fecClean = pStatus->fecClean;
    prevStatus->fecCorrected = pStatus->fecCorrected;
    prevStatus->viterbiUncorrectedBits = pStatus->viterbiUncorrectedBits;
    prevStatus->viterbiTotalBits = pStatus->viterbiTotalBits;
    prevStatus->time = currentTime;

    return BERR_SUCCESS;
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7584_RequestQamAsyncStatus(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7584 *pDevice;
    NEXUS_7584Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = handle;
    pDevice = (NEXUS_7584 *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);

    if (pChannel->chn_num >= NEXUS_3128_MAX_DOWNSTREAM_CHANNELS)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    rc = BADS_RequestAsyncStatus(pDevice->ads_chn[pChannel->chn_num]);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    return BERR_SUCCESS;
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7584_ReadSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length, size_t *pNumRead)
{
    #define TOTAL_ADS_SOFTDECISIONS 30

    NEXUS_Error rc = NEXUS_SUCCESS;
    int16_t d_i[TOTAL_ADS_SOFTDECISIONS], d_q[TOTAL_ADS_SOFTDECISIONS];
    int16_t return_length;
    NEXUS_7584 *pDevice;
    unsigned i;
    NEXUS_7584Channel *pChannel;

    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7584Channel *)handle;
    pDevice = (NEXUS_7584 *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);

    BKNI_Memset(pDecisions, 0, (sizeof(NEXUS_FrontendSoftDecision) * length));

    if (pChannel->chn_num >= NEXUS_3128_MAX_DOWNSTREAM_CHANNELS)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    /* only make one call to ADS. if app needs more, they can loop. */
    rc = BADS_GetSoftDecision(pDevice->ads_chn[pChannel->chn_num], (int16_t)TOTAL_ADS_SOFTDECISIONS, d_i, d_q, &return_length);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    for (i=0; (int)i<return_length && i<length; i++)
    {
        pDecisions[i].i = d_i[i];
        pDecisions[i].q = d_q[i];
    }
    *pNumRead = i;

    return BERR_SUCCESS;

done:
    return rc;
}

void NEXUS_FrontendDevice_GetDefault3128Settings(NEXUS_FrontendDevice3128Settings *pSettings)
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->agcValue = 0x1f; /* Max gain*/
    NEXUS_CallbackDesc_Init(&pSettings->updateGainCallback);
#if NEXUS_FRONTEND_7584_OOB
    pSettings->outOfBand.outputMode = NEXUS_FrontendOutOfBandOutputMode_eMax; /* Setting default. */
#endif
}

NEXUS_Error NEXUS_FrontendDevice_Get3128Settings(NEXUS_FrontendDeviceHandle handle, NEXUS_FrontendDevice3128Settings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7584 *pDevice = NULL;
    BDBG_ASSERT(handle != NULL);

    pDevice = handle->pDevice;
    if (!pDevice) { rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto done; }
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);

    *pSettings = pDevice->deviceSettings;
done:
    return rc;

}

NEXUS_Error NEXUS_FrontendDevice_Set3128Settings(NEXUS_FrontendDeviceHandle handle, const NEXUS_FrontendDevice3128Settings *pSettings )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7584 *pDevice = NULL;
    BHAB_ConfigSettings habConfigSettings;
#if NEXUS_FRONTEND_7584_OOB
    BTNR_PowerSaverSettings pwrSettings;
    BAOB_ConfigSettings aobConfigSettings;
#endif

    BDBG_ASSERT(handle != NULL);
    pDevice = handle->pDevice;
    if (!pDevice) { rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto done; }
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);

    NEXUS_IsrCallback_Set(pDevice->updateGainAppCallback, &(pSettings->updateGainCallback));
    pDevice->deviceSettings.updateGainCallback = pSettings->updateGainCallback;
#if 0
    rc = BHAB_ReadRegister(pDevice->hab, BCHP_TM_SFT0, &buf);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if(buf & 0x80000000) {
        /* If the CPPM bit is set by docsis and is being consumed by 7584(once consumed, 7584 sets this bit to 0), then update 7584 CPPM */
        buf = pSettings->agcValue;
        buf |= 0x80000000;
    }

    pDevice->deviceSettings.agcValue = pSettings->agcValue;

    rc = BHAB_WriteRegister(pDevice->hab, BCHP_TM_SFT0, &buf);
    if(rc){rc = BERR_TRACE(rc); goto done;}
#endif

    if(pSettings->enableRfLoopThrough != pDevice->deviceSettings.enableRfLoopThrough) {
        rc = BHAB_GetConfigSettings(pDevice->hab, &habConfigSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        habConfigSettings.enableLoopThrough = pSettings->enableRfLoopThrough;

        rc = BHAB_SetConfigSettings(pDevice->hab, &habConfigSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        pDevice->deviceSettings.enableRfLoopThrough = pSettings->enableRfLoopThrough;
    }

#if NEXUS_FRONTEND_7584_OOB
    if((pDevice->aobPresent) && (pSettings->outOfBand.outputMode != pDevice->deviceSettings.outOfBand.outputMode) ){
        if(!pDevice->isTunerPoweredOn[pDevice->oobChannelNumber]){
            pwrSettings.enable = false;
            rc = BTNR_SetPowerSaver(pDevice->tnr[pDevice->oobChannelNumber], &pwrSettings);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->isTunerPoweredOn[pDevice->oobChannelNumber] = true;
            pDevice->numTunerPoweredOn++;
        }
        if(!pDevice->isPoweredOn[pDevice->oobChannelNumber]){
            rc = BAOB_DisablePowerSaver(pDevice->aob);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            pDevice->isPoweredOn[pDevice->oobChannelNumber] = true;
        }
        rc = BAOB_GetConfigSettings(pDevice->aob, &aobConfigSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        aobConfigSettings.outputMode = pSettings->outOfBand.outputMode;
        rc = BAOB_SetConfigSettings(pDevice->aob, &aobConfigSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        pDevice->deviceSettings.outOfBand.outputMode = pSettings->outOfBand.outputMode;
    }
#endif
done:
    return rc;
}


static NEXUS_Error NEXUS_FrontendDevice_P_7584_GetStatus(void *handle, NEXUS_FrontendDeviceStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7584 *pDevice;
    NEXUS_AvsStatus avsData;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7584 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);
    BDBG_ASSERT(NULL != handle);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    rc = NEXUS_GetAvsStatus(&avsData);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if(avsData.enabled){
        pStatus->avs.enabled = true;
        pStatus->avs.voltage = avsData.voltage;
        pStatus->temperature = avsData.temperature;
    }
    else
        pStatus->avs.enabled = false;

    BDBG_MSG(("pStatus->avs.enabled = %d", pStatus->avs.enabled));
    BDBG_MSG(("pStatus->avs.voltage = %d", pStatus->avs.voltage));
    BDBG_MSG(("pStatus->temperature = %d", pStatus->temperature));
done:
    return rc;
}

static void NEXUS_Frontend_P_7584_ResetQamStatus(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7584Channel *pChannel;
    NEXUS_7584 *pDevice;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7584Channel *)handle;
    pDevice = (NEXUS_7584 *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);

    if(pDevice->isPoweredOn[pChannel->chn_num]) {
        rc = BADS_ResetStatus(pDevice->ads_chn[pChannel->chn_num]);
        if (rc){BERR_TRACE(rc);}
    }
    else{
        BDBG_MSG(("The downstream core is Powered Off"));
    }
}

/***************************************************************************
Summary:
  Probe to see if a BCM7584 device exists with the specified settings

Description:
  Probe to see if a BCM7584 device exists with the specified settings

See Also:
    NEXUS_Frontend_Open7584
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Probe3128(const NEXUS_FrontendDevice3128OpenSettings *pSettings, NEXUS_3128ProbeResults *pResults)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BCHP_Info info;

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pResults);

    BCHP_GetInfo(g_pCoreHandles->chp, &info);
    pResults->chip.id = info.productId;
    pResults->chip.familyId = info.familyId;
    if ((pResults->chip.familyId != 0x7584) && !((pResults->chip.familyId >= 0x75840) && (pResults->chip.familyId <= 0x75849)))
    {
        BDBG_ERR(("pResults->chip.familyId = 0x%x", pResults->chip.familyId));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }
    info.rev += 0x10; /* convert to nexus revision */
    pResults->chip.version.major = info.rev >> 4;
    pResults->chip.version.minor = info.rev & 0xF;

done:
    return rc;
}

/***************************************************************************
Summary:
    Retrieve the chip family id, chip id, chip version and firmware version.
***************************************************************************/
static void NEXUS_Frontend_P_7584_GetType(void *handle, NEXUS_FrontendType *type)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BFEC_SystemVersionInfo  versionInfo;
    NEXUS_7584Channel *pChannel;
    NEXUS_7584 *pDevice;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7584Channel *)handle;
    pDevice = (NEXUS_7584 *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);
    rc = BHAB_GetVersionInfo(pDevice->hab, &versionInfo);
    if(rc) BERR_TRACE(rc);

    type->chip.familyId = (uint32_t)pDevice->chipFamilyId;
    type->chip.id = (uint32_t)versionInfo.chipId;
    type->chip.version.major = (pDevice->revId >> 4) + 1;
    type->chip.version.minor = pDevice->revId & 0xf;
    type->chip.bondoutOptions[0] = versionInfo.bondoutOptions[0];
    type->chip.bondoutOptions[1] = versionInfo.bondoutOptions[1];
    type->chip.version.buildType = 0;
    type->chip.version.buildId = 0;

    type->firmwareVersion.major = versionInfo.firmware.majorVersion;
    type->firmwareVersion.minor = versionInfo.firmware.minorVersion;
    type->firmwareVersion.buildType = versionInfo.firmware.buildType;
    type->firmwareVersion.buildId = versionInfo.firmware.buildId;

    if(type->chip.familyId != versionInfo.familyId){
        BDBG_ERR(("Type mismatch while retreiving family id."));
        rc = BERR_TRACE(BERR_UNKNOWN); goto done;
    }
    if(pDevice->revId != versionInfo.chipVersion){
        BDBG_ERR(("Type mismatch while retreiving chip version."));
        rc = BERR_TRACE(BERR_UNKNOWN); goto done;
    }
done:
    return;
}

static NEXUS_Error NEXUS_Frontend_P_7584_RequestSpectrumAnalyzerData(void *handle, const NEXUS_FrontendSpectrumSettings *pSettings )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BTNR_SpectrumSettings settings;
    BTNR_PowerSaverSettings pwrSettings;
    NEXUS_7584Channel *pChannel;
    NEXUS_7584 *pDevice;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7584Channel *)handle;
    pDevice = (NEXUS_7584 *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);
    BDBG_ASSERT(NULL != pSettings);

    switch(pSettings->fftSize)
    {
    case 64:
        settings.fftSize = 0x6;
        break;
    case 128:
        settings.fftSize = 0x7;
        break;
    case 256:
        settings.fftSize = 0x8;
        break;
    case 512:
        settings.fftSize = 0x9;
        break;
    case 1024:
        settings.fftSize = 0xa;
        break;
    case 2048:
        settings.fftSize = 0xb;
        break;
    case 4096:
        settings.fftSize = 0xc;
        break;
    default:
        BDBG_ERR(("FFT size not supported. Current supported FFT sizes are 64, 128, 256, 512, 1024 and 2048"));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    settings.numSamples = pSettings->numSamples;
    settings.startFreq = pSettings->startFrequency;
    settings.stopFreq = pSettings->stopFrequency;
#if 0 /* binAverage is not used any more */
    settings.binAverage = pSettings->binAverage;
#endif

    pDevice->spectrumDataPointer = (uint32_t *)pSettings->data;

    NEXUS_TaskCallback_Set(pDevice->spectrumDataAppCallback[pChannel->chn_num], &(pSettings->dataReadyCallback));

    if(pDevice->spectrumEventCallback == NULL) {
        pDevice->spectrumEventCallback = NEXUS_RegisterEvent(pDevice->spectrumEvent, NEXUS_Frontend_P_7584_spetctrumEventCallback, pChannel);
        if ( NULL == pDevice->spectrumEventCallback ){rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done; }
    }

    if (pDevice->tnr[pChannel->chn_num])
    {
        pwrSettings.enable = false;
        rc = BTNR_SetPowerSaver(pDevice->tnr[pChannel->chn_num], &pwrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    rc = BTNR_RequestSpectrumAnalyzerData(pDevice->tnr[pChannel->chn_num], &settings);
    if (rc){BERR_TRACE(rc);}
done:
    return rc;
}

static void NEXUS_Tuner_P_7584_Close(void *handle)
{
    unsigned chn_num;
    NEXUS_7584 *pDevice;
    NEXUS_7584Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7584Channel *)handle;
    pDevice = pChannel->pDevice;
    chn_num = pChannel->chn_num;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);
    BDBG_ASSERT(pChannel->chn_num == pDevice->ifDacChannelNumber);

    if(pDevice->ifDacAppCallback){
        NEXUS_IsrCallback_Destroy(pDevice->ifDacAppCallback);
    }
    if(pDevice->asyncStatusAppCallback[pChannel->chn_num]){
        NEXUS_IsrCallback_Destroy(pDevice->asyncStatusAppCallback[pChannel->chn_num]);
    }
    if(pChannel){
        BKNI_Free(pChannel);
        pChannel = NULL;
    }
}

NEXUS_Error NEXUS_Tuner_P_7584_GetNSetGain(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_GainParameters params;
    NEXUS_InternalGainSettings settings;
    NEXUS_ExternalGainSettings externalGainSettings;
    NEXUS_7584 *pDevice;
    NEXUS_7584Channel *pChannel;
    pChannel = (NEXUS_7584Channel *)handle;
    pDevice = pChannel->pDevice;


    BKNI_Memset(&externalGainSettings, 0, sizeof(externalGainSettings));

    if(pDevice->pGenericDeviceHandle->parent){

        params.rfInput = pDevice->pGenericDeviceHandle->linkSettings.rfInput;
        params.accumulateTillRootDevice = true;
        params.frequency = pDevice->last_ads[pChannel->chn_num].frequency;

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

NEXUS_Error NEXUS_Tuner_P_7584_Tune(void *handle, const NEXUS_TunerTuneSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BTNR_PowerSaverSettings pwrSettings;
    NEXUS_7584 *pDevice;
    NEXUS_7584Channel *pChannel;
    BTNR_IfDacSettings ifdacSettings;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7584Channel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);
    BDBG_ASSERT(pChannel->chn_num == pDevice->ifDacChannelNumber);

    rc = NEXUS_Tuner_P_7584_GetNSetGain(pChannel);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    NEXUS_IsrCallback_Set(pDevice->ifDacAppCallback, &(pSettings->tuneCompleteCallback));
    NEXUS_IsrCallback_Set(pDevice->asyncStatusAppCallback[pChannel->chn_num], &(pSettings->asyncStatusReadyCallback));
    pDevice->ifdacTuneComplete = false;

    if(pSettings->mode == NEXUS_TunerMode_eQam){
        ifdacSettings.std = BTNR_Standard_eQam;
    }
    else {
        BDBG_ERR(("Unsupported tuner mode."));
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto done;
    }
    ifdacSettings.frequency = pSettings->frequency;
    ifdacSettings.bandwidth = pDevice->ifDacTunerSettings.bandwidth;
    ifdacSettings.dacAttenuation = pDevice->ifDacTunerSettings.dacAttenuation;
    ifdacSettings.outputFrequency = pDevice->ifDacTunerSettings.ifFrequency;

    if(!pDevice->isTunerPoweredOn[pChannel->chn_num]){
        pwrSettings.enable = false;
        rc = BTNR_SetPowerSaver(pDevice->tnr[pChannel->chn_num], &pwrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        pDevice->isTunerPoweredOn[pChannel->chn_num] = true;
        pDevice->numTunerPoweredOn++;
    }

    rc = BTNR_TuneIfDac(pDevice->tnr[pChannel->chn_num], &ifdacSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}

NEXUS_Error NEXUS_Tuner_P_7584_RequestAsyncStatus(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned chn_num;
    NEXUS_7584 *pDevice;
    NEXUS_7584Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7584Channel *)handle;
    pDevice = pChannel->pDevice;
    chn_num = pChannel->chn_num;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);
    BDBG_ASSERT(pChannel->chn_num == pDevice->ifDacChannelNumber);

    rc = BTNR_RequestIfDacStatus(pDevice->tnr[pChannel->chn_num]);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}

NEXUS_Error NEXUS_Tuner_P_7584_GetAsyncStatus(void *handle, NEXUS_TunerStatus *pStatus)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned chn_num;
    BTNR_IfDacStatus status;
    NEXUS_7584 *pDevice;
    NEXUS_7584Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7584Channel *)handle;
    pDevice = pChannel->pDevice;
    chn_num = pChannel->chn_num;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);
    BDBG_ASSERT(pChannel->chn_num == pDevice->ifDacChannelNumber);

    rc = BTNR_GetIfDacStatus(pDevice->tnr[pChannel->chn_num], &status);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pStatus->rssi = status.rssi/100;
    pStatus->tuneComplete = pDevice->ifdacTuneComplete;

done:
    return rc;
}

static void NEXUS_Tuner_P_7584_UninstallCallbacks(void *handle)
{
    NEXUS_7584 *pDevice;
    NEXUS_7584Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7584Channel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);
    BDBG_ASSERT(pChannel->chn_num == pDevice->ifDacChannelNumber);

    if(pDevice->ifDacAppCallback)NEXUS_IsrCallback_Set(pDevice->ifDacAppCallback, NULL);

    return;
}

static void NEXUS_Tuner_P_7584_UnTune(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BTNR_PowerSaverSettings pwrSettings;
    NEXUS_7584 *pDevice;
    NEXUS_7584Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7584Channel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);
    BDBG_ASSERT(pChannel->chn_num == pDevice->ifDacChannelNumber);

    if(pDevice->isTunerPoweredOn[pChannel->chn_num]){
        pwrSettings.enable = true;
        rc = BTNR_SetPowerSaver(pDevice->tnr[pChannel->chn_num], &pwrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        pDevice->isTunerPoweredOn[pChannel->chn_num] = false;
        pDevice->numTunerPoweredOn--;
    }
done:
    return;
}

static NEXUS_Error NEXUS_Tuner_P_7584_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BTNR_PowerSaverSettings pwrSettings;
    NEXUS_7584 *pDevice;
    NEXUS_7584Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7584Channel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);
    BDBG_ASSERT(pChannel->chn_num == pDevice->ifDacChannelNumber);
    BSTD_UNUSED(pSettings);

    if(enabled && pDevice->isTunerPoweredOn[pChannel->chn_num]){
        pwrSettings.enable = true;
        rc = BTNR_SetPowerSaver(pDevice->tnr[pChannel->chn_num], &pwrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        pDevice->isTunerPoweredOn[pChannel->chn_num] = false;
        pDevice->numTunerPoweredOn--;
    }

done:
    return rc;
}

void NEXUS_Tuner_P_7584_GetDefaultTuneSettings(NEXUS_TunerMode mode, NEXUS_TunerTuneSettings *pSettings)
{
    BSTD_UNUSED(mode);
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

}

NEXUS_Error NEXUS_Tuner_P_7584_GetSettings(void *handle, NEXUS_TunerSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned chn_num;
    NEXUS_7584 *pDevice;
    NEXUS_7584Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7584Channel *)handle;
    pDevice = pChannel->pDevice;
    chn_num = pChannel->chn_num;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);

    pSettings->bandwidth = pDevice->ifDacTunerSettings.bandwidth;
    pSettings->ifFrequency = pDevice->ifDacTunerSettings.ifFrequency;
    pSettings->dacAttenuation = pDevice->ifDacTunerSettings.dacAttenuation;

    return rc;
}

NEXUS_Error NEXUS_Tuner_P_7584_SetSettings(void *handle, const NEXUS_TunerSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned chn_num;
    NEXUS_7584 *pDevice;
    NEXUS_7584Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7584Channel *)handle;
    pDevice = pChannel->pDevice;
    chn_num = pChannel->chn_num;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);

    pDevice->ifDacTunerSettings.bandwidth = pSettings->bandwidth;
    pDevice->ifDacTunerSettings.ifFrequency = pSettings->ifFrequency;
    pDevice->ifDacTunerSettings.dacAttenuation = pSettings->dacAttenuation;

    return rc;
}

void NEXUS_Tuner_GetDefaultOpen3128Settings(NEXUS_TunerOpen3128Settings *pSettings)
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_TunerHandle NEXUS_Tuner_Open3128(unsigned index, const NEXUS_TunerOpen3128Settings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7584 *pDevice = NULL;
    NEXUS_7584Channel *pChannel = NULL;
    NEXUS_FrontendDevice *pFrontendDevice = NULL;
    NEXUS_IsrCallbackHandle ifdacAsyncStatusReadyCallback = NULL;
    BSTD_UNUSED(index);

    BDBG_ASSERT(NULL != pSettings);

    if(pSettings->device == NULL) {
        BDBG_ERR(("Open the 3128 device handle first by calling NEXUS_FrontendDevice_Open3128()."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto error;
    }
    else {
        pDevice = (NEXUS_7584 *)pSettings->device->pDevice;
        pFrontendDevice =  pSettings->device;
    }

    /* check if tuner handle is already opened*/
    if (pDevice->ifDacHandle != NULL){
        return pDevice->ifDacHandle;
    }

    pChannel = (NEXUS_7584Channel*)BKNI_Malloc(sizeof(NEXUS_7584Channel));
    if(NULL == pChannel){rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto error;}

    /* save channel number in pChannel*/
    pChannel->chn_num = pDevice->ifDacChannelNumber;
    pChannel->pDevice = pDevice;

    rc = BTNR_InstallCallback(pDevice->tnr[pChannel->chn_num], BTNR_Callback_eIfDacAcquireComplete, (BTNR_CallbackFunc)NEXUS_Tuner_P_7584_Callback, pChannel);
    if(rc){rc = BERR_TRACE(rc); goto error;}

    rc = BTNR_InstallCallback(pDevice->tnr[pChannel->chn_num], BTNR_Callback_eIfDacStatusReady, (BTNR_CallbackFunc)NEXUS_Frontend_P_7584_AsyncStatusCallback_isr, pChannel);
    if(rc){rc = BERR_TRACE(rc); goto error;}

    /* Create a Nexus frontend handle */
    pDevice->ifDacHandle = NEXUS_Tuner_P_Create(pChannel);
    if(NULL == pDevice->ifDacHandle){rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto error;}

    pDevice->ifDacAppCallback = NEXUS_IsrCallback_Create(pDevice->ifDacHandle, NULL);
    if ( NULL == pDevice->ifDacAppCallback ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto error;}

    ifdacAsyncStatusReadyCallback = NEXUS_IsrCallback_Create(pDevice->ifDacHandle, NULL);
    if ( NULL == ifdacAsyncStatusReadyCallback ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto error;}

    pDevice->asyncStatusAppCallback[pChannel->chn_num] = ifdacAsyncStatusReadyCallback;

    /* bind functions*/
    pDevice->ifDacHandle->close = NEXUS_Tuner_P_7584_Close;
    pDevice->ifDacHandle->requestAsyncStatus = NEXUS_Tuner_P_7584_RequestAsyncStatus;
    pDevice->ifDacHandle->getAsyncStatus = NEXUS_Tuner_P_7584_GetAsyncStatus;
    pDevice->ifDacHandle->getDefaultTuneSettings = NEXUS_Tuner_P_7584_GetDefaultTuneSettings;
    pDevice->ifDacHandle->tune = NEXUS_Tuner_P_7584_Tune;
    pDevice->ifDacHandle->untune = NEXUS_Tuner_P_7584_UnTune;
    pDevice->ifDacHandle->standby = NEXUS_Tuner_P_7584_Standby;
    pDevice->ifDacHandle->getSettings = NEXUS_Tuner_P_7584_GetSettings;
    pDevice->ifDacHandle->setSettings = NEXUS_Tuner_P_7584_SetSettings;
    pDevice->ifDacHandle->uninstallCallbacks = NEXUS_Tuner_P_7584_UninstallCallbacks;
    pDevice->ifDacHandle->pGenericDeviceHandle = pFrontendDevice;

    return pDevice->ifDacHandle;

error:
    if (pDevice) {
        if(pDevice->ifDacAppCallback){
            NEXUS_IsrCallback_Destroy(pDevice->ifDacAppCallback);
        }
        if(ifdacAsyncStatusReadyCallback){
            NEXUS_IsrCallback_Destroy(ifdacAsyncStatusReadyCallback);
        }
        if(pChannel){
            BKNI_Free(pChannel);
            pChannel = NULL;
        }
        if(pDevice->ifDacHandle){
            BKNI_Free(pDevice->ifDacHandle);
        }
    }
    return NULL;
}

#if NEXUS_FRONTEND_7584_OOB
static NEXUS_Error NEXUS_Frontend_P_7584_TuneOob(void *handle, const NEXUS_FrontendOutOfBandSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BAOB_AcquireParam obParams;
    BTNR_PowerSaverSettings pwrSettings;
    NEXUS_7584 *pDevice;
    NEXUS_7584Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7584Channel *)handle;
    pDevice = (NEXUS_7584 *)pChannel->pDevice;

    if(pDevice->deviceSettings.outOfBand.outputMode == NEXUS_FrontendOutOfBandOutputMode_eMax){
        BDBG_ERR(("Out of band output mode set to %d is not supported. Use NEXUS_FrontendDevice_Set7584Settings() to set the right settings.",
         pDevice->deviceSettings.outOfBand.outputMode));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    rc = NEXUS_Tuner_P_7584_GetNSetGain(pChannel);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    NEXUS_IsrCallback_Set(pDevice->lockAppCallback[pDevice->oobChannelNumber], &(pSettings->lockCallback));
    NEXUS_IsrCallback_Set(pDevice->asyncStatusAppCallback[pDevice->oobChannelNumber], &(pSettings->asyncStatusReadyCallback));

    if (pDevice->tnr[pDevice->oobChannelNumber])
    {
        if(!pDevice->isTunerPoweredOn[pDevice->oobChannelNumber]){
            pwrSettings.enable = false;
            rc = BTNR_SetPowerSaver(pDevice->tnr[pDevice->oobChannelNumber], &pwrSettings);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            pDevice->isTunerPoweredOn[pDevice->oobChannelNumber] = true;
            pDevice->numTunerPoweredOn++;
        }
        if(!pDevice->isPoweredOn[pDevice->oobChannelNumber]){
            rc = BAOB_DisablePowerSaver(pDevice->aob);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            pDevice->isPoweredOn[pDevice->oobChannelNumber] = true;
        }
        pDevice->acquireInProgress[pDevice->oobChannelNumber] = true;
        rc = BTNR_SetTunerRfFreq(pDevice->tnr[pDevice->oobChannelNumber], pSettings->frequency, BTNR_TunerMode_eDigital);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    rc = BAOB_GetDefaultAcquireParams(&obParams);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    obParams.autoAcquire = pSettings->autoAcquire;
    obParams.modType = (BAOB_ModulationType)pSettings->mode ;
    obParams.symbolRate = pSettings->symbolRate;
    obParams.berSrc = pSettings->bertSource;
    obParams.spectrum = pSettings->spectrum;
    obParams.bertPolynomial = pSettings->bert.polynomial;

    rc = BAOB_SetAcquireParams(pDevice->aob, &obParams);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    rc = BAOB_Acquire(pDevice->aob, &obParams);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pDevice->last_aob = *pSettings;
    return BERR_SUCCESS;
done:
    return rc;
}

static void NEXUS_Frontend_P_7584_UnTuneOob(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7584Channel *pChannel;
    NEXUS_7584 *pDevice;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7584Channel *)handle;
    pDevice = (NEXUS_7584 *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);
    BDBG_ASSERT(pChannel->chn_num == pDevice->oobChannelNumber);

    if ((pChannel->chn_num == pDevice->oobChannelNumber) && (pDevice->isPoweredOn[pDevice->oobChannelNumber])){
        rc = BAOB_EnablePowerSaver(pDevice->aob);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        pDevice->isPoweredOn[pDevice->oobChannelNumber] = false;
    }
done:
    return;
}

static NEXUS_Error NEXUS_Frontend_P_7584_GetOobStatus(void *handle, NEXUS_FrontendOutOfBandStatus *pStatus)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned j=0;
    uint32_t buf=0;
    NEXUS_7584 *pDevice;
    NEXUS_7584Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7584Channel *)handle;
    pDevice = (NEXUS_7584 *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);
    BDBG_ASSERT(pChannel->chn_num == pDevice->oobChannelNumber);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    pDevice->isInternalAsyncStatusCall[pChannel->chn_num] = true;

    rc = NEXUS_Frontend_P_7584_RequestOobAsyncStatus(pChannel);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    for(j=0; j < 20; j++) {
        BKNI_Sleep(20);

        rc = BHAB_ReadRegister(pDevice->hab, BCHP_LEAP_CTRL_SW_SPARE0 , &buf);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        if(buf & BHAB_OOB_SPARE_STATUS_RDY) {
            rc = NEXUS_Frontend_P_7584_GetOobAsyncStatus(pChannel, pStatus);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            break;
        }
    }

    pDevice->isInternalAsyncStatusCall[pChannel->chn_num] = false;

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7584_GetOobAsyncStatus(void *handle, NEXUS_FrontendOutOfBandStatus *pStatus)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    struct BAOB_Status st;
    NEXUS_7584Channel *pChannel;
    NEXUS_7584 *pDevice;
    uint64_t elapsedSymbols=0;
    unsigned unCorrectedBlock = 0;
    NEXUS_Time currentTime;
    NEXUS_PreviousStatus *prevStatus;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7584Channel *)handle;
    pDevice = (NEXUS_7584 *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);
    BDBG_ASSERT(pChannel->chn_num == pDevice->oobChannelNumber);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    rc = BAOB_GetAsyncStatus(pDevice->aob,  &st);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pStatus->isFecLocked = st.isFecLock;
    pStatus->isQamLocked = st.isQamLock;
    pStatus->isBertLocked = st.bertSync;
    pStatus->symbolRate = st.symbolRate;
    pStatus->snrEstimate = st.snrEstimate*100/256;
    pStatus->agcIntLevel = st.agcIntLevel;
    pStatus->agcExtLevel = st.agcExtLevel;
    pStatus->carrierFreqOffset = st.carrierFreqOffset;
    pStatus->carrierPhaseOffset = st.carrierPhaseOffset;
    pStatus->correctedCount = st.correctedCount;
    pStatus->uncorrectedCount = st.uncorrectedCount;
    pStatus->mode = (NEXUS_FrontendOutOfBandMode)st.modType;
    pStatus->symbolRate = st.symbolRate;
    pStatus->ifFreq = 0;/* Not required on 7584*/
    pStatus->loFreq = 0;/* Not required on 7584*/
    pStatus->sysXtalFreq = st.sysXtalFreq;
    pStatus->berErrorCount = st.berErrorCount;
    pStatus->fdcChannelPower = st.aobChannelPower/10;
    pStatus->frontendGain = st.feGain;
    pStatus->digitalAgcGain = st.digitalAgcGain;
    pStatus->highResEqualizerGain = st.equalizerGain;
    pStatus->acquisitionTime = st.acquisitionTime;
    pStatus->totalAcquisitionTime = st.totalAcquisitionTime;
    pStatus->postRsBer = 0;

    prevStatus = &pDevice->previousStatus[pChannel->chn_num];
    NEXUS_Time_Get(&currentTime);
    if(pStatus->uncorrectedCount  > prevStatus->fecUncorrected)
       unCorrectedBlock = pStatus->uncorrectedCount - prevStatus->fecUncorrected;

    pStatus->postRsBerElapsedTime = NEXUS_Time_Diff(&currentTime, &prevStatus->time);

    elapsedSymbols = pStatus->postRsBerElapsedTime * pStatus->symbolRate;
    if(elapsedSymbols){
        if(pStatus->settings.mode == NEXUS_FrontendOutOfBandMode_eAnnexAQpsk){
            pStatus->postRsBer = ((uint64_t)unCorrectedBlock * 2097152 * 1024 * 55)/(elapsedSymbols * 2 * 53);
        }
        else if(pStatus->settings.mode == NEXUS_FrontendOutOfBandMode_eDvs178Qpsk){
            /*pStatus->postRsBer = ((uint64_t)unCorrectedBlock * 2097152 * 1024 * 96 * 11)/(elapsedSymbols * 94 * 2 * 100000);*/
            pStatus->postRsBer = ((uint64_t)unCorrectedBlock * 2097152 * 1024 * 3 * 11)/(elapsedSymbols * 47 * 12500);
        }
    }

    prevStatus->fecUncorrected = pStatus->uncorrectedCount;
    prevStatus->time = currentTime;
    pStatus->settings = pDevice->last_aob;

    BDBG_MSG((" OOB ASYNC STATUS : fec_lock = %d,  qam_lock = %d, snr_estimate = %d, fec_corr_cnt = %d \
                fec_uncorrected_cnt = %d, ber_err_cnt = %d " , st.isFecLock, st.isQamLock, st.snrEstimate,
            st.correctedCount, st.uncorrectedCount, st.berErrorCount));

    return BERR_SUCCESS;
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7584_RequestOobAsyncStatus(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7584Channel *pChannel;
    NEXUS_7584 *pDevice;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7584Channel *)handle;
    pDevice = (NEXUS_7584 *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);
    BDBG_ASSERT(pChannel->chn_num == pDevice->oobChannelNumber);

    rc = BAOB_RequestAsyncStatus(pDevice->aob);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    return BERR_SUCCESS;
done:
    return rc;
}

static void NEXUS_Frontend_P_7584_ResetOobStatus(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7584Channel *pChannel;
    NEXUS_7584 *pDevice;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7584Channel *)handle;
    pDevice = (NEXUS_7584 *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7584);
    BDBG_ASSERT(pChannel->chn_num == pDevice->oobChannelNumber);

    rc = BAOB_ResetStatus(pDevice->aob);
    if(rc){rc = BERR_TRACE(rc);}
    return;
}
#endif
