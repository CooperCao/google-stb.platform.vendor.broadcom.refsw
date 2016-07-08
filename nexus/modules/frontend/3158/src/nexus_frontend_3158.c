/******************************************************************************
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
 ******************************************************************************/
#include "nexus_frontend_module.h"
#include "nexus_platform_features.h"
#include "nexus_spi.h"
#include "priv/nexus_spi_priv.h"
#include "nexus_i2c.h"
#include "priv/nexus_i2c_priv.h"
#if NEXUS_HAS_GPIO
#include "priv/nexus_gpio_priv.h"
#endif
#include "bhab_3158.h"
#include "bads_3158.h"
#include "bhab_3158_fw.h"
#include "bhab.h"
#include "bads.h"
#include "priv/nexus_transport_priv.h"
#include "bhab_ctfe_img.h"
#include "priv/nexus_core_img.h"
#include "priv/nexus_core_img_id.h"

#if NEXUS_HAS_MXT
#include "bmxt.h"
#include "bmxt_wakeup.h"
#endif
#if NEXUS_FRONTEND_315x_OOB
#include "baob.h"
#endif
#include "bchp_3158_leap_ctrl.h"

/* set to 1 to enable L1 interrupt messages */
#define NEXUS_FRONTEND_DEBUG_IRQ 0

BDBG_MODULE(nexus_frontend_3158);

BDBG_OBJECT_ID(NEXUS_3158Device);

typedef struct NEXUS_PreviousStatus
{
    unsigned fecCorrected;
    unsigned fecUncorrected;
    unsigned fecClean;
    unsigned viterbiUncorrectedBits;
    unsigned viterbiTotalBits;
    NEXUS_Time time;
} NEXUS_PreviousStatus;

#define NEXUS_MAX_3158_FRONTENDS 18
#define NEXUS_3158_MAX_DOWNSTREAM_CHANNELS 16
#define NEXUS_3158_OOB_CHANNEL NEXUS_3158_MAX_DOWNSTREAM_CHANNELS
/***************************************************************************
 * Internal Module Structure
 ***************************************************************************/
typedef struct NEXUS_3158Device
{
    BDBG_OBJECT(NEXUS_3158Device)
    BLST_S_ENTRY(NEXUS_3158Device) node;
    uint16_t  chipFamilyId;
    uint16_t  chipId;
    uint16_t revId;
    unsigned deviceIndex;
    BHAB_Handle hab;
    BHAB_Capabilities capabilities;
    unsigned    numfrontends;
    unsigned    numChannels;
    bool adsPresent;
    bool aobPresent;
    bool ifDacPresent;
    bool isMtsif;
    BADS_Handle ads;
    NEXUS_FrontendDeviceSettings deviceSettings;
    NEXUS_FrontendHandle    frontendHandle[NEXUS_MAX_3158_FRONTENDS];
    BADS_ChannelHandle  ads_chn[NEXUS_3158_MAX_DOWNSTREAM_CHANNELS];
    NEXUS_FrontendQamSettings   qam[NEXUS_3158_MAX_DOWNSTREAM_CHANNELS];
    NEXUS_FrontendOutOfBandSettings oob;
    NEXUS_FrontendQamSettings   last_ads[NEXUS_3158_MAX_DOWNSTREAM_CHANNELS];
    BKNI_EventHandle isrEvent;
    NEXUS_EventCallbackHandle isrEventCallback;
    NEXUS_IsrCallbackHandle lockAppCallback[NEXUS_MAX_3158_FRONTENDS];
    NEXUS_IsrCallbackHandle updateGainAppCallback;
    NEXUS_IsrCallbackHandle asyncStatusAppCallback[NEXUS_MAX_3158_FRONTENDS];
    NEXUS_TaskCallbackHandle spectrumDataAppCallback[NEXUS_MAX_3158_FRONTENDS];
    bool                    isInternalAsyncStatusCall[NEXUS_MAX_3158_FRONTENDS];
    uint8_t numTunerPoweredOn;
    bool isTunerPoweredOn[NEXUS_MAX_3158_FRONTENDS];
    bool isPoweredOn[NEXUS_MAX_3158_FRONTENDS];
    uint32_t *spectrumDataPointer;
    unsigned spectrumDataLength;
    BKNI_EventHandle spectrumEvent;
    NEXUS_EventCallbackHandle spectrumEventCallback;
    bool acquireInProgress[NEXUS_MAX_3158_FRONTENDS];
    unsigned count[NEXUS_MAX_3158_FRONTENDS];
    NEXUS_PreviousStatus previousStatus[NEXUS_MAX_3158_FRONTENDS];
    NEXUS_FrontendDevice *pGenericDeviceHandle;
    NEXUS_TunerHandle  ifDacHandle;
    unsigned ifDacChannelNumber;
    NEXUS_IsrCallbackHandle ifDacAppCallback;
    NEXUS_IsrCallbackHandle ifDacAsyncStatusAppCallback;
    NEXUS_TunerSettings ifDacTunerSettings;
    bool ifdacTuneComplete;
    NEXUS_ThreadHandle deviceOpenThread;
    NEXUS_ThreadHandle pollingThread;
    NEXUS_FrontendDeviceOpenSettings openSettings;
    bool isInternalSetSettings;
    BREG_I2C_Handle i2cRegHandle;
    BREG_SPI_Handle spiRegHandle;
    bool settingsInitialized;
    NEXUS_IsrCallbackHandle cppmAppCallback;
    NEXUS_IsrCallbackHandle cppmDoneCallback;
#if NEXUS_FRONTEND_315x_OOB
    unsigned oobChannelNumber;
    BAOB_Handle aob;
    NEXUS_FrontendOutOfBandSettings last_aob;
#endif
    NEXUS_TransportWakeupSettings wakeupSettings;
} NEXUS_3158Device;

typedef struct NEXUS_3158Channel
{
    unsigned chn_num; /* channel number */
    NEXUS_3158Device *pDevice; /* 3158 device*/
} NEXUS_3158Channel;

/***************************************************************************
 * Module callback functions
 ***************************************************************************/
static void NEXUS_Frontend_P_3158_spectrumDataReadyCallback(void *pParam)
{
    BDBG_ASSERT(NULL != pParam);
    BDBG_MSG(("NEXUS_Frontend_P_3158_spectrumDataReadyCallback: %p", (void *)pParam));
    BKNI_SetEvent((BKNI_EventHandle)pParam);
}

static void NEXUS_Frontend_P_3158_spectrumEventCallback(void *pParam)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BADS_SpectrumData  spectrumData;
    NEXUS_3158Channel *pChannel;
    NEXUS_3158Device *pDevice;
    BDBG_ASSERT(NULL != pParam);
    pChannel = (NEXUS_3158Channel *)pParam;
    pDevice = (NEXUS_3158Device *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3158Device);

    BDBG_MSG(("NEXUS_Frontend_P_3158_spectrumEventCallback: %p", (void *)pParam));

    spectrumData.data = pDevice->spectrumDataPointer;

    rc = BADS_GetSpectrumAnalyzerData(pDevice->ads_chn[pChannel->chn_num], &spectrumData);
    if (rc){rc = BERR_TRACE(rc);  goto done;}

    pDevice->spectrumDataLength += spectrumData.datalength;
    pDevice->spectrumDataPointer += spectrumData.datalength;

    BDBG_MSG(("NEXUS_Frontend_P_3158_spectrumEventCallback: pDevice->spectrumDataAppCallback[%d]: %p", pChannel->chn_num, (void *)pDevice->spectrumDataAppCallback[pChannel->chn_num]));
    if (!spectrumData.moreData) {
        if (pDevice->spectrumDataAppCallback[pChannel->chn_num])
        {
            pDevice->spectrumDataLength = 0;
            NEXUS_TaskCallback_Fire(pDevice->spectrumDataAppCallback[pChannel->chn_num]);
        }
    }

done:
    return;
}

static void NEXUS_Frontend_P_3158_callback_isr(void *pParam)
{
    NEXUS_FrontendHandle frontendHandle = NULL;
    NEXUS_3158Channel *pChannel;
    NEXUS_3158Device *pDevice;
    BDBG_ASSERT(NULL != pParam);
    frontendHandle = *((NEXUS_FrontendHandle *)pParam);
    BDBG_ASSERT(NULL != frontendHandle);
    pChannel = frontendHandle->pDeviceHandle;
    BDBG_ASSERT(NULL != pChannel);
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3158Device);

    BDBG_MSG(("NEXUS_Frontend_P_3158_callback_isr"));

    if(pDevice->acquireInProgress[pChannel->chn_num]){
        pDevice->count[pChannel->chn_num]++;
    }
    if(pDevice->count[pChannel->chn_num] == 2){
        pDevice->acquireInProgress[pChannel->chn_num] = false;
        pDevice->count[pChannel->chn_num] = 0;
    }

    if (pDevice->lockAppCallback[pChannel->chn_num])
    {
        NEXUS_IsrCallback_Fire_isr(pDevice->lockAppCallback[pChannel->chn_num]);
    }
}

/***************************************************************************
Summary:
    Lock callback handler for a 3158 Inband device
 ***************************************************************************/
static void NEXUS_Frontend_P_3158_AsyncStatusCallback_isr(void *pParam)
{
    NEXUS_FrontendHandle frontendHandle = NULL;
    NEXUS_3158Channel *pChannel;
    NEXUS_3158Device *pDevice;
    BDBG_ASSERT(NULL != pParam);
    frontendHandle = *((NEXUS_FrontendHandle *)pParam);
    BDBG_ASSERT(NULL != frontendHandle);
    pChannel = frontendHandle->pDeviceHandle;
    BDBG_ASSERT(NULL != pChannel);
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3158Device);

    if(!pDevice->isInternalAsyncStatusCall[pChannel->chn_num]){
        if (pDevice->asyncStatusAppCallback[pChannel->chn_num])
        {
            NEXUS_IsrCallback_Fire_isr(pDevice->asyncStatusAppCallback[pChannel->chn_num]);
        }
    }
}

/***************************************************************************
Summary:
    Enable/Disable interrupts for a 3158 device
 ***************************************************************************/
static void NEXUS_Frontend_P_3158_L1_isr(void *pParam1, int pParam2)
{
    NEXUS_3158Device *pDevice = (NEXUS_3158Device *)pParam1;
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHAB_Handle hab;
    BDBG_ASSERT(NULL != pParam1);
    hab = (BHAB_Handle)pDevice->hab;
    BSTD_UNUSED(pParam2);

#if NEXUS_FRONTEND_DEBUG_IRQ
    BDBG_MSG(("3158 L1 ISR (hab: %p)", (void *)hab));
#endif
    if(hab != NULL){
        rc = BHAB_HandleInterrupt_isr(hab);
        if(rc){rc = BERR_TRACE(rc);}
    }
#if NEXUS_FRONTEND_DEBUG_IRQ
    BDBG_MSG(("Done: 3158 L1 ISR (hab: %p)", (void *)hab));
#endif
}

#if NEXUS_HAS_GPIO
/***************************************************************************
Summary:
    Enable/Disable interrupts for a 3158 device
 ***************************************************************************/
static void NEXUS_Frontend_P_3158_GpioIsrControl_isr(bool enable, void *pParam)
{
    NEXUS_GpioHandle gpioHandle;
    BDBG_ASSERT(NULL != pParam);
    gpioHandle = (NEXUS_GpioHandle)pParam;

#if NEXUS_FRONTEND_DEBUG_IRQ
    BDBG_MSG(("%s 3158 Gpio Interrupt %p", enable ? "Enable" : "Disable", (void *)gpioHandle));
#endif
    NEXUS_Gpio_SetInterruptEnabled_isr(gpioHandle, enable);
}
#endif

/***************************************************************************
Summary:
    Enable/Disable interrupts for a 3158 device
 ***************************************************************************/
static void NEXUS_Frontend_P_3158_IsrControl_isr(bool enable, void *pParam)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned *isrNumber = (unsigned *)pParam;
    BDBG_ASSERT(NULL != pParam);

    if ( enable )
    {
#if NEXUS_FRONTEND_DEBUG_IRQ
        BDBG_MSG(("Enable 3158 Interrupt %u", *isrNumber));
#endif
        rc = NEXUS_Core_EnableInterrupt_isr(*isrNumber);
        if(rc) BERR_TRACE(rc);
    }
    else
    {
#if NEXUS_FRONTEND_DEBUG_IRQ
        BDBG_MSG(("Disable 3158 Interrupt %u", *isrNumber));
#endif
        NEXUS_Core_DisableInterrupt_isr(*isrNumber);
    }
}

/***************************************************************************
Summary:
    ISR Event Handler for a 3158 device
 ***************************************************************************/
static void NEXUS_Frontend_P_3158_IsrEvent(void *pParam)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHAB_Handle hab;
    BDBG_ASSERT(NULL != pParam);
    hab = (BHAB_Handle)pParam;

#if NEXUS_FRONTEND_DEBUG_IRQ
    BDBG_MSG(("3158 ISR Callback (hab: %p)", (void *)hab));
#endif
    if(hab != NULL){
        rc = BHAB_ProcessInterruptEvent(hab);
        if(rc) BERR_TRACE(rc);
    }
}

static NEXUS_Error NEXUS_Frontend_P_3158_WriteRegister(void *handle, unsigned address, uint32_t value)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3158Device *pDevice;
    NEXUS_3158Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3158Channel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3158Device);

    rc = BHAB_WriteRegister(pDevice->hab, address, &value);
    if(rc){rc = BERR_TRACE(rc);}

    return rc;
}
static NEXUS_Error NEXUS_Frontend_P_3158_ReadRegister(void *handle, unsigned address, uint32_t *value   )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3158Device *pDevice;
    NEXUS_3158Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3158Channel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3158Device);

    rc = BHAB_ReadRegister(pDevice->hab, address, value);
    if(rc){rc = BERR_TRACE(rc);}

    return rc;
}

static NEXUS_Error NEXUS_FrontendDevice_P_3158_GetStatus(void *handle, NEXUS_FrontendDeviceStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3158Device *pDevice;
    BHAB_AvsData avsData;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_3158Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3158Device);
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

static void NEXUS_FrontendDevice_P_3158_GetCapabilities(void *handle, NEXUS_FrontendDeviceCapabilities *pCapabilities)
{
    NEXUS_3158Device *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_3158Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3158Device);

    /* The device is already initialized completely. Hence just copy the tuner capabilities. */
    pCapabilities->numTuners = pDevice->numChannels;

}

static void NEXUS_FrontendDevice_P_3158_GetTunerCapabilities(void *handle, unsigned tunerIndex, NEXUS_FrontendCapabilities *pCapabilities)
{
    NEXUS_3158Device *pDevice;
    BHAB_ChannelCapability *channelCapability;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_3158Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3158Device);
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

static NEXUS_Error NEXUS_FrontendDevice_P_3158_GetSettings(void *handle, NEXUS_FrontendDeviceSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3158Device *pDevice = (NEXUS_3158Device *)handle;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3158Device);

    *pSettings = pDevice->deviceSettings;

    return rc;
}

static NEXUS_Error NEXUS_FrontendDevice_P_3158_SetSettings(void *handle, const NEXUS_FrontendDeviceSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3158Device *pDevice = (NEXUS_3158Device *)handle;
    BHAB_ConfigSettings habConfigSettings;
    bool poweredUp = false;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3158Device);

    if (!pDevice->ads_chn[0]) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto done; }
    if (!pDevice->isPoweredOn[0]) {
        rc = BADS_DisablePowerSaver(pDevice->ads_chn[0]);
        if (rc) { rc = BERR_TRACE(rc); }
        poweredUp = true;
    }

    if (pSettings->enableRfLoopThrough != pDevice->deviceSettings.enableRfLoopThrough) {
        rc = BHAB_GetConfigSettings(pDevice->hab, &habConfigSettings);
        if (rc) { rc = BERR_TRACE(rc); goto done; }

        habConfigSettings.enableLoopThrough = pSettings->enableRfLoopThrough;

        rc = BHAB_SetConfigSettings(pDevice->hab, &habConfigSettings);
        if (rc) { rc = BERR_TRACE(rc); goto done; }

        pDevice->deviceSettings.enableRfLoopThrough = pSettings->enableRfLoopThrough;
    }

done:
    if (poweredUp) {
        rc = BADS_EnablePowerSaver(pDevice->ads_chn[0]);
        if (rc) { rc = BERR_TRACE(rc); }
    }
    return rc;
}

static NEXUS_Error NEXUS_FrontendDevice_P_3158_GetDeviceAmplifierStatus(void *handle, NEXUS_FrontendDeviceAmplifierStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3158Device *pDevice = (NEXUS_3158Device *)handle;
    BHAB_LnaStatus lnaStatus;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3158Device);

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

#if NEXUS_FRONTEND_315x_OOB
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

static NEXUS_Error NEXUS_Frontend_P_3158_ReadSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length, size_t *pNumRead)
{
    #define TOTAL_ADS_SOFTDECISIONS 30

    NEXUS_Error rc = NEXUS_SUCCESS;
    int16_t d_i[TOTAL_ADS_SOFTDECISIONS], d_q[TOTAL_ADS_SOFTDECISIONS];
    int16_t return_length;
    NEXUS_3158Device *pDevice;
    unsigned i;
    NEXUS_3158Channel *pChannel;

    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3158Channel *)handle;
    pDevice = (NEXUS_3158Device *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3158Device);

    BKNI_Memset(pDecisions, 0, (sizeof(NEXUS_FrontendSoftDecision) * length));

    if (pChannel->chn_num >= NEXUS_3158_MAX_DOWNSTREAM_CHANNELS)
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

NEXUS_Error NEXUS_Frontend_P_3158_ReapplyTransportSettings(void *handle)
{
#if NEXUS_HAS_MXT
    NEXUS_3158Channel *pChannel = (NEXUS_3158Channel *)handle;
    NEXUS_3158Device *pDevice;
    NEXUS_Error rc;

    BDBG_ASSERT(pChannel);
    pDevice = (NEXUS_3158Device *)pChannel->pDevice;
    BDBG_ASSERT(pDevice);
    rc = NEXUS_Frontend_P_SetMtsifConfig(pDevice->frontendHandle[pChannel->chn_num]);
    if (rc) { return BERR_TRACE(rc); }
#else
    BSTD_UNUSED(handle);
#endif

    return NEXUS_SUCCESS;
}

static void NEXUS_Frontend_P_3158_ResetQamStatus(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3158Channel *pChannel;
    NEXUS_3158Device *pDevice;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3158Channel *)handle;
    pDevice = (NEXUS_3158Device *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3158Device);

    if(pDevice->isPoweredOn[pChannel->chn_num]) {
        rc = BADS_ResetStatus(pDevice->ads_chn[pChannel->chn_num]);
        if (rc){BERR_TRACE(rc);}
    }
    else{
        BDBG_MSG(("The downstream core is Powered Off"));
    }
}

#if NEXUS_FRONTEND_315x_OOB
static NEXUS_Error NEXUS_Frontend_P_3158_TuneOob(void *handle, const NEXUS_FrontendOutOfBandSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BAOB_AcquireParam obParams;
    NEXUS_3158Device *pDevice;
    NEXUS_3158Channel *pChannel;
    NEXUS_FrontendDeviceHandle hFrontendDevice = NULL;

    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3158Channel *)handle;
    pDevice = (NEXUS_3158Device *)pChannel->pDevice;
    BDBG_ASSERT(pChannel->chn_num == NEXUS_3158_OOB_CHANNEL);
#if 0 /* TODO: may not be applicable to 3158 */
    if(pDevice->deviceSettings.outOfBand.outputMode == NEXUS_FrontendOutOfBandOutputMode_eMax){
        BDBG_ERR(("Out of band output mode set to %d is not supported. Use NEXUS_FrontendDevice_Set3158Settings() to set the right settings.", pDevice->deviceSettings.outOfBand.outputMode));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }
#endif
#if defined(NEXUS_FRONTEND_DOCSIS) && defined(NEXUS_PLATFORM_DOCSIS_PLUS_BCM3158_IB_SUPPORT)
    hFrontendDevice = (NEXUS_FrontendDeviceHandle)pDevice->pGenericDeviceHandle;
    BDBG_ASSERT(hFrontendDevice);
    if(hFrontendDevice->parent)
    {
        uint32_t agcVal=0;
        uint32_t buf=0;
        NEXUS_FrontendDevice_P_GetDocsisLnaDeviceAgcValue(hFrontendDevice->parent,&agcVal);
        BDBG_MSG(("%s DOCSIS agcVal:%#lx <<<",__FUNCTION__agcVal));
        rc = BHAB_ReadRegister(pDevice->hab, BCHP_TM_SFT0, &buf);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        if(buf & 0x80000000)
        {
            /* If the CPPM bit is set by docsis and is being consumed by 3158(once consumed, 3158 sets this bit to 0), then update 3158 CPPM */
            buf = agcVal;
            buf |= 0x80000000;
        }
        else
        {
            buf = pSettings->agcValue;
        }
        pDevice->deviceSettings.agcValue = agcVal;
        rc = BHAB_WriteRegister(pDevice->hab, BCHP_TM_SFT0, &buf);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
#else
    BSTD_UNUSED(hFrontendDevice);
#endif

    rc = NEXUS_Frontend_P_SetMtsifConfig(pDevice->frontendHandle[pChannel->chn_num]);
    if (rc) { rc = BERR_TRACE(rc); goto done; }
    NEXUS_IsrCallback_Set(pDevice->lockAppCallback[pChannel->chn_num], &(pSettings->lockCallback));
    NEXUS_IsrCallback_Set(pDevice->asyncStatusAppCallback[pChannel->chn_num], &(pSettings->asyncStatusReadyCallback));

    if(!pDevice->isPoweredOn[NEXUS_3158_MAX_DOWNSTREAM_CHANNELS]){
        rc = BAOB_DisablePowerSaver(pDevice->aob);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        pDevice->isPoweredOn[NEXUS_3158_MAX_DOWNSTREAM_CHANNELS] = true;
    }
#if 0 /* TODO */
    rc = BAOB_GetConfigSettings(pDevice->aob, &aobConfigSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}
    aobConfigSettings.outputMode = pSettings->outOfBand.outputMode;
    rc = BAOB_SetConfigSettings(pDevice->aob, &aobConfigSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}
    pDevice->deviceSettings.outOfBand.outputMode = pSettings->outOfBand.outputMode;
#endif
    rc = BAOB_GetDefaultAcquireParams(&obParams);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    obParams.frequency = pSettings->frequency;
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

static void NEXUS_Frontend_P_3158_UnTuneOob(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3158Device *pDevice;
    NEXUS_3158Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3158Channel *)handle;
    pDevice = (NEXUS_3158Device *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3158Device);
    BDBG_ASSERT(pChannel->chn_num == NEXUS_3158_OOB_CHANNEL);

    NEXUS_Frontend_P_UnsetMtsifConfig(pDevice->frontendHandle[pChannel->chn_num]);

    if (pDevice->isPoweredOn[pChannel->chn_num]) {
        rc = BAOB_EnablePowerSaver(pDevice->aob);
        if (rc) {rc = BERR_TRACE(rc); goto done;}
        pDevice->isPoweredOn[pChannel->chn_num] = false;
    }
done:
    return;
}

static NEXUS_Error NEXUS_Frontend_P_3158_RequestOobAsyncStatus(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3158Device *pDevice;
    NEXUS_3158Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3158Channel *)handle;
    pDevice = (NEXUS_3158Device *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3158Device);
    BDBG_ASSERT(pChannel->chn_num == NEXUS_3158_OOB_CHANNEL);

    rc = BAOB_RequestAsyncStatus(pDevice->aob);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    return BERR_SUCCESS;
done:
    return rc;
}


static NEXUS_Error NEXUS_Frontend_P_3158_GetOobAsyncStatus(void *handle, NEXUS_FrontendOutOfBandStatus *pStatus)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    struct BAOB_Status st;
    NEXUS_3158Device *pDevice;
    NEXUS_3158Channel *pChannel;
    uint64_t elapsedSymbols=0;
    unsigned unCorrectedBlock = 0;
    NEXUS_Time currentTime;
    NEXUS_PreviousStatus *prevStatus;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3158Channel *)handle;
    pDevice = (NEXUS_3158Device *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3158Device);
    BDBG_ASSERT(pChannel->chn_num == NEXUS_3158_OOB_CHANNEL);

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
    pStatus->ifFreq = 0;/* Not required on 3158*/
    pStatus->loFreq = 0;/* Not required on 3158*/
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

    BDBG_MSG((" OOB ASYNC STATUS : fec_lock = %d,  qam_lock = %d, snr_estimate = %d, fec_corr_cnt = %d",
                st.isFecLock, st.isQamLock, pStatus->snrEstimate, st.correctedCount));


    return BERR_SUCCESS;
done:
    return rc;
}

#define BHAB_OOB_SPARE_STATUS_RDY    0x00000100
static NEXUS_Error NEXUS_Frontend_P_3158_GetOobStatus(void *handle, NEXUS_FrontendOutOfBandStatus *pStatus)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned j=0;
    uint32_t buf=0;
    NEXUS_3158Device *pDevice;
    NEXUS_3158Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3158Channel *)handle;
    pDevice = (NEXUS_3158Device *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3158Device);
    BDBG_ASSERT(pChannel->chn_num == NEXUS_3158_OOB_CHANNEL);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    pDevice->isInternalAsyncStatusCall[pChannel->chn_num] = true;

    rc = NEXUS_Frontend_P_3158_RequestOobAsyncStatus(pChannel);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    for(j=0; j < 20; j++) {

        BKNI_Sleep(20);

        rc = BHAB_ReadRegister(pDevice->hab, BCHP_LEAP_CTRL_SW_SPARE0 , &buf);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        if(buf & BHAB_OOB_SPARE_STATUS_RDY) {
            rc = NEXUS_Frontend_P_3158_GetOobAsyncStatus(pChannel, pStatus);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            break;
        }
    }

    pDevice->isInternalAsyncStatusCall[pChannel->chn_num] = false;

done:
    return rc;
}

static void NEXUS_Frontend_P_3158_ResetOobStatus(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3158Device *pDevice;
    NEXUS_3158Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3158Channel *)handle;
    pDevice = (NEXUS_3158Device *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3158Device);
    BDBG_ASSERT(pChannel->chn_num == NEXUS_3158_OOB_CHANNEL);

    if(pDevice->isPoweredOn[pChannel->chn_num]) {
        rc = BAOB_ResetStatus(pDevice->aob);
        if(rc){rc = BERR_TRACE(rc);}
    }
    else{
        BDBG_MSG(("The out of band core is Powered Off."));
    }
    return;
}
#endif

/***************************************************************************
Summary:
    Retrieve the chip family id, chip id, chip version and firmware version.
***************************************************************************/
static void NEXUS_Frontend_P_3158_GetType(void *handle, NEXUS_FrontendType *type)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BFEC_SystemVersionInfo  versionInfo;
    NEXUS_3158Channel *pChannel;
    NEXUS_3158Device *p3158Device;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3158Channel *)handle;
    p3158Device = (NEXUS_3158Device *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(p3158Device, NEXUS_3158Device);

    rc = BHAB_GetVersionInfo(p3158Device->hab, &versionInfo);
    if(rc) BERR_TRACE(rc);

    type->chip.familyId = (uint32_t)p3158Device->chipFamilyId;
    type->chip.id = (uint32_t)p3158Device->chipId;
    type->chip.version.major = (p3158Device->revId >> 8) + 1;
    type->chip.version.minor = p3158Device->revId & 0xff;
    type->chip.bondoutOptions[0] = versionInfo.bondoutOptions[0];
    type->chip.bondoutOptions[1] = versionInfo.bondoutOptions[1];
    type->chip.version.buildType = 0;
    type->chip.version.buildId = 0;

    type->firmwareVersion.major = versionInfo.firmware.majorVersion;
    type->firmwareVersion.minor = versionInfo.firmware.minorVersion;
    type->firmwareVersion.buildType = versionInfo.firmware.buildType;
    type->firmwareVersion.buildId = versionInfo.firmware.buildId;

#if 0
    if((type->chip.familyId != versionInfo.familyId) || (type->chip.id != versionInfo.chipId)){
        BDBG_ERR(("Type mismatch while retreiving chip id and family id."));
        BERR_TRACE(BERR_UNKNOWN); goto done;
    }
    if(p3158Device->revId != versionInfo.chipVersion){
        BDBG_ERR(("Type mismatch while retreiving chip version."));
        BERR_TRACE(BERR_UNKNOWN); goto done;
    }
    done:
#endif
    return;
}

static NEXUS_Error NEXUS_Frontend_P_3158_RequestSpectrumAnalyzerData(void *handle, const NEXUS_FrontendSpectrumSettings *pSettings )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BADS_SpectrumSettings settings;
    NEXUS_3158Channel *pChannel;
    NEXUS_3158Device *pDevice;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3158Channel *)handle;
    pDevice = (NEXUS_3158Device *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3158Device);
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
        pDevice->spectrumEventCallback = NEXUS_RegisterEvent(pDevice->spectrumEvent, NEXUS_Frontend_P_3158_spectrumEventCallback, pChannel);
        if ( NULL == pDevice->spectrumEventCallback ){rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done; }
    }

    rc = BADS_DisablePowerSaver(pDevice->ads_chn[pChannel->chn_num]);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    rc = BADS_RequestSpectrumAnalyzerData(pDevice->ads_chn[pChannel->chn_num], &settings);
    if (rc){BERR_TRACE(rc);}
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_3158_GetFastStatus(void *handle, NEXUS_FrontendFastStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3158Device *p3158Device;
    NEXUS_3158Channel *pChannel;
    BADS_LockStatus eLock = BADS_LockStatus_eUnlocked;
#if NEXUS_FRONTEND_315x_OOB
    BAOB_LockStatus bLock = BAOB_LockStatus_eUnlocked;
#endif
    NEXUS_FrontendDeviceHandle hFrontendDevice=NULL;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3158Channel *)handle;
    p3158Device = (NEXUS_3158Device *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(p3158Device, NEXUS_3158Device);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    pStatus->lockStatus = NEXUS_FrontendLockStatus_eUnknown;

    if(pChannel->chn_num < NEXUS_3158_MAX_DOWNSTREAM_CHANNELS){
        rc = BADS_GetLockStatus(p3158Device->ads_chn[pChannel->chn_num],  &eLock);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pStatus->lockStatus = NEXUS_Frontend_P_GetAdsLockStatus(eLock);
        BSTD_UNUSED(hFrontendDevice);
    }
#if NEXUS_FRONTEND_315x_OOB
    else if(pChannel->chn_num == NEXUS_3158_OOB_CHANNEL){
            if(p3158Device->aobPresent){
                rc = BAOB_GetLockStatus(p3158Device->aob,  &bLock);
                if(rc){rc = BERR_TRACE(rc); goto done;}
                pStatus->lockStatus = NEXUS_Frontend_P_GetAobLockStatus(bLock);
            }
            else{
                BDBG_WRN(("Out of band is Unsupported on 0x%x chip.", p3158Device->chipId));
            }
    }
#endif
    pStatus->acquireInProgress = p3158Device->acquireInProgress[pChannel->chn_num];
    return BERR_SUCCESS;
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_3158_RequestQamAsyncStatus(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3158Device *p3158Device;
    NEXUS_3158Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = handle;
    p3158Device = (NEXUS_3158Device *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(p3158Device, NEXUS_3158Device);

    if (pChannel->chn_num >= NEXUS_3158_MAX_DOWNSTREAM_CHANNELS)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    rc = BADS_RequestAsyncStatus(p3158Device->ads_chn[pChannel->chn_num]);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    return BERR_SUCCESS;
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_3158_GetQamAsyncStatus(void *handle, NEXUS_FrontendQamStatus *pStatus)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    struct BADS_Status st;
    uint64_t totalbits=0, uncorrectedBits=0;
    unsigned cleanBlock = 0, correctedBlock = 0, unCorrectedBlock = 0, totalBlock = 0;
    NEXUS_Time currentTime;
    NEXUS_PreviousStatus *prevStatus;
    NEXUS_3158Device *p3158Device;
    NEXUS_3158Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3158Channel *)handle;
    p3158Device = (NEXUS_3158Device *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(p3158Device, NEXUS_3158Device);

    if (pChannel->chn_num >= NEXUS_3158_MAX_DOWNSTREAM_CHANNELS)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    rc = BADS_GetAsyncStatus(p3158Device->ads_chn[pChannel->chn_num],  &st);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    prevStatus = &p3158Device->previousStatus[pChannel->chn_num];
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
    pStatus->symbolRateError = st.symbolRateError;
    pStatus->berRawCount = st.berRawCount;
    pStatus->dsChannelPower = st.dsChannelPower;
    pStatus->mainTap = st.mainTap;
    pStatus->settings = p3158Device->last_ads[pChannel->chn_num];
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


#define BHAB_ADS_CHN0_STATUS_RDY    0x00010000
static NEXUS_Error NEXUS_Frontend_P_3158_GetQamStatus(void *handle, NEXUS_FrontendQamStatus *pStatus)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned j=0;
    uint32_t buf=0;
    NEXUS_3158Device *p3158Device;
    NEXUS_3158Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3158Channel *)handle;
    p3158Device = (NEXUS_3158Device *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(p3158Device, NEXUS_3158Device);

    if (pChannel->chn_num >= NEXUS_3158_MAX_DOWNSTREAM_CHANNELS)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    if (p3158Device->isPoweredOn[pChannel->chn_num]) {
        uint32_t addr = BCHP_LEAP_CTRL_SW_SPARE0;
        uint32_t mask = BHAB_ADS_CHN0_STATUS_RDY;

        if (pChannel->chn_num > 7) {
            addr = BCHP_LEAP_CTRL_GP7;
            mask = 1;
        }
        p3158Device->isInternalAsyncStatusCall[pChannel->chn_num] = true;

        rc = NEXUS_Frontend_P_3158_RequestQamAsyncStatus(pChannel);
        if (rc) { rc = BERR_TRACE(rc); goto done; }

        for (j=0; j < 200; j++) {

            BKNI_Sleep(20);

            rc = BHAB_ReadRegister(p3158Device->hab, addr, &buf);
            if (rc) { rc = BERR_TRACE(rc); goto done; }

            if (buf & (mask << pChannel->chn_num % 8)) {
                rc = NEXUS_Frontend_P_3158_GetQamAsyncStatus(pChannel, pStatus);
                if (rc) { rc = BERR_TRACE(rc); goto done; }
                break;
            }
        }
    }
    else{
        BDBG_MSG(("The downstream core is Powered Off"));
    }

    p3158Device->isInternalAsyncStatusCall[pChannel->chn_num] = false;
    return BERR_SUCCESS;
done:
    return rc;
}

static void NEXUS_Frontend_P_3158_UnTuneQam(void *handle)
{
#if 0
    NEXUS_Error rc = NEXUS_SUCCESS;
    BADS_InbandParam ibParam;
    NEXUS_3158Device *pDevice;
    NEXUS_3158Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3158Channel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3158Device);

    if (pChannel->chn_num >= NEXUS_3158_MAX_DOWNSTREAM_CHANNELS)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    NEXUS_Frontend_P_UnsetMtsifConfig(pDevice->frontendHandle[pChannel->chn_num]);

    if(pDevice->isPoweredOn[pChannel->chn_num]){
        BKNI_Memset(&ibParam, 0x0, sizeof(ibParam));
        rc = BADS_SetAcquireParams(pDevice->ads_chn[pChannel->chn_num], &ibParam );
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = BADS_EnablePowerSaver(pDevice->ads_chn[pChannel->chn_num]);
       if(rc){rc = BERR_TRACE(rc); goto done;}

       pDevice->isPoweredOn[pChannel->chn_num] = false;
    }

    BKNI_Memset(&pDevice->last_ads[pChannel->chn_num], 0x0, sizeof(NEXUS_FrontendQamSettings));

done:
    return;
#else
    BSTD_UNUSED(handle);
#endif
}

static NEXUS_Error NEXUS_Frontend_P_3158_TuneQam(void *handle, const NEXUS_FrontendQamSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3158Device *pDevice;
    NEXUS_3158Channel *pChannel;
    BADS_InbandParam ibParam;

    BDBG_ASSERT(handle);
    pChannel = (NEXUS_3158Channel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3158Device);

    rc = NEXUS_Frontend_P_SetMtsifConfig(pDevice->frontendHandle[pChannel->chn_num]);
    if (rc) { rc = BERR_TRACE(rc); goto done; }

    rc = BADS_DisablePowerSaver(pDevice->ads_chn[pChannel->chn_num]);
    if(rc){rc = BERR_TRACE(rc); goto done;}
    pDevice->isPoweredOn[pChannel->chn_num] = true;

    rc = BADS_GetDefaultAcquireParams(pDevice->ads_chn[pChannel->chn_num], &ibParam);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if (pSettings->annex == NEXUS_FrontendQamAnnex_eB) {
        BDBG_MSG(("AnnexB"));
        switch (pSettings->mode) {
        case NEXUS_FrontendQamMode_e64:
            ibParam.modType = BADS_ModulationType_eAnnexBQam64; break;
        case NEXUS_FrontendQamMode_e256:
            ibParam.modType = BADS_ModulationType_eAnnexBQam256; break;
        default:
            rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
            break;
        }
    } else if (pSettings->annex == NEXUS_FrontendQamAnnex_eA || pSettings->annex == NEXUS_FrontendQamAnnex_eC){
        BDBG_MSG(("Annex%s", (pSettings->annex==NEXUS_FrontendQamAnnex_eA) ? "A" : "C"));
        switch (pSettings->mode) {
        case NEXUS_FrontendQamMode_e64:
            ibParam.modType = BADS_ModulationType_eAnnexAQam64; break;
        case NEXUS_FrontendQamMode_e256:
            ibParam.modType = BADS_ModulationType_eAnnexAQam256; break;
        default:
            rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
            break;
        }
    } else {
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }

    NEXUS_IsrCallback_Set(pDevice->lockAppCallback[pChannel->chn_num], &(pSettings->lockCallback));
    NEXUS_IsrCallback_Set(pDevice->asyncStatusAppCallback[pChannel->chn_num], &(pSettings->asyncStatusReadyCallback));

    ibParam.frequency = pSettings->frequency;
    ibParam.symbolRate = pSettings->symbolRate;
    ibParam.autoAcquire = pSettings->autoAcquire;
    ibParam.enableDpm = pSettings->enablePowerMeasurement;
    ibParam.spectrum = pSettings->spectrumMode;
    ibParam.invertSpectrum = pSettings->spectralInversion;
    ibParam.frequencyOffset =  pSettings->frequencyOffset;
    ibParam.acquisitionType = pSettings->acquisitionMode;

    rc = BADS_SetAcquireParams(pDevice->ads_chn[pChannel->chn_num], &ibParam );
    if(rc){rc = BERR_TRACE(rc); goto done;}

    rc = BADS_Acquire(pDevice->ads_chn[pChannel->chn_num], &ibParam );
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pDevice->acquireInProgress[pChannel->chn_num] = true;
    pDevice->last_ads[pChannel->chn_num] = *pSettings;

done:
    return rc;
}

static void NEXUS_FrontendDevice_P3158_GetWakeupFilter(NEXUS_FrontendDeviceHandle handle, NEXUS_TransportWakeupSettings *pSettings)
{
    NEXUS_3158Device *pDevice = handle->pDevice;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3158Device);

    *pSettings = pDevice->wakeupSettings;

    return;
}

static NEXUS_Error NEXUS_FrontendDevice_P3158_SetWakeupFilter(NEXUS_FrontendDeviceHandle handle, const NEXUS_TransportWakeupSettings *pSettings)
{
    BERR_Code rc;
    NEXUS_3158Device *pDevice = handle->pDevice;
    BMXT_Wakeup_Settings wakeupSettings;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3158Device);

    {
        unsigned i,j;
        BMXT_Wakeup_PacketFilter *Filter = BKNI_Malloc(sizeof(BMXT_Wakeup_PacketFilter)*NEXUS_WAKEUP_PACKET_SIZE);
        if (!Filter) {return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);}
        BKNI_Memset(Filter, 0, sizeof(BMXT_Wakeup_PacketFilter)*NEXUS_WAKEUP_PACKET_SIZE);

        for (i=0; i<NEXUS_MAX_WAKEUP_PACKETS; i++) {
            for (j=0; j<pSettings->packetLength; j++) {
                Filter[j].CompareByte = pSettings->filter[i].packet[j].comparePattern;
                Filter[j].Mask = pSettings->filter[i].packet[j].compareMask;
                Filter[j].MaskType = pSettings->filter[i].packet[j].maskType;
            }
            rc = BMXT_Wakeup_SetPacketFilterBytes(pDevice->pGenericDeviceHandle->mtsifConfig.mxt, i, Filter);
            if (rc) {return BERR_TRACE(rc);}
        }
        BKNI_Free(Filter);

        BMXT_Wakeup_GetSettings(pDevice->pGenericDeviceHandle->mtsifConfig.mxt, &wakeupSettings);
        wakeupSettings.InputBand = 0;
        wakeupSettings.PacketLength = pSettings->packetLength;
        wakeupSettings.ErrorInputIgnore = pSettings->errorIgnore;
        rc = BMXT_Wakeup_SetSettings(pDevice->pGenericDeviceHandle->mtsifConfig.mxt, &wakeupSettings);
        if (rc) {return BERR_TRACE(rc);}
    }

    BMXT_Wakeup_Enable(pDevice->pGenericDeviceHandle->mtsifConfig.mxt, pSettings->enabled);

    pDevice->wakeupSettings = *pSettings;

    return NEXUS_SUCCESS;
}

static void NEXUS_Frontend_P_Uninit3158(NEXUS_3158Device *pDevice, bool uninitHab);

void NEXUS_FrontendDevice_P_Close3158(void *handle)
{
    NEXUS_3158Device *pDevice = (NEXUS_3158Device *)handle;

    if (pDevice) {
        NEXUS_Frontend_P_Uninit3158(pDevice, true);
        if (pDevice->pGenericDeviceHandle)
            BKNI_Free(pDevice->pGenericDeviceHandle);
        BKNI_Memset(pDevice, 0, sizeof(*pDevice));
        BKNI_Free(pDevice);
    }
}

static NEXUS_Error NEXUS_FrontendDevice_P_3158_Standby(void *handle, const NEXUS_StandbySettings *pSettings);

NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_P_3158_Alloc(unsigned index, const NEXUS_FrontendDeviceOpenSettings *pSettings)
{
    NEXUS_FrontendDevice *pFrontendDevice = NULL;
    NEXUS_3158Device *pDevice = NULL;

#if 0
    /* TODO: handle multiple devices */
    for ( pDevice = BLST_S_FIRST(&g_deviceList); NULL != pDevice; pDevice = BLST_S_NEXT(pDevice, node) )
    {
        break;
    }
#endif

    if ( NULL == pDevice)
    {
        pFrontendDevice = BKNI_Malloc(sizeof(*pFrontendDevice));
        if (NULL == pFrontendDevice) { BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err; }
        BKNI_Memset(pFrontendDevice, 0, sizeof(*pFrontendDevice));

        pDevice = BKNI_Malloc(sizeof(*pDevice));
        if (NULL == pDevice) { BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err; }
        BKNI_Memset(pDevice, 0, sizeof(*pDevice));
        BDBG_OBJECT_SET(pDevice, NEXUS_3158Device);
        pDevice->openSettings = *pSettings;
        pDevice->pGenericDeviceHandle = pFrontendDevice;
        pDevice->deviceIndex = index;

#if 0
        BLST_S_INSERT_HEAD(&g_deviceList, pDevice, node);
#endif
    }
#if 0
    else {
        pFrontendDevice = pDevice->pGenericDeviceHandle;
    }
#endif

    pFrontendDevice->pDevice = pDevice;
    return pFrontendDevice;

err:
    NEXUS_FrontendDevice_P_Close3158(pFrontendDevice);
    return NULL;
}

NEXUS_Error NEXUS_FrontendDevice_P_3158_PreInitAp(NEXUS_FrontendDeviceHandle pFrontendDevice, bool initHab)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned int i;
    NEXUS_3158Device *pDevice = NULL;
    NEXUS_FrontendDeviceOpenSettings *pSettings = NULL;

    BERR_Code errCode;
    BHAB_Settings habSettings;
    BHAB_Handle habHandle;
    void *regHandle;

    BDBG_ASSERT(pFrontendDevice);

    pDevice = (NEXUS_3158Device *)pFrontendDevice->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3158Device);

    NEXUS_FrontendDevice_GetDefaultSettings(&pDevice->deviceSettings);

    pSettings = &pDevice->openSettings;

    BHAB_3158_GetDefaultSettings(&habSettings);

    if (pSettings->spiDevice) {
        BDBG_MSG(("Configuring for SPI"));
        habSettings.chipAddr = 0x20;
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
        habSettings.interruptEnableFunc = NEXUS_Frontend_P_3158_IsrControl_isr;
        habSettings.interruptEnableFuncParam = (void*)&pDevice->openSettings.isrNumber;
    }
    else if(pSettings->gpioInterrupt){
        BDBG_MSG(("Configuring for GPIO interrupt"));
        habSettings.interruptEnableFunc = NEXUS_Frontend_P_3158_GpioIsrControl_isr;
        habSettings.interruptEnableFuncParam = (void*)pSettings->gpioInterrupt;
    }
    BDBG_ASSERT(regHandle);

    if(pSettings->isrNumber) {
        BDBG_MSG(("Connecting external interrupt"));
        errCode = NEXUS_Core_ConnectInterrupt(pSettings->isrNumber,
                                             NEXUS_Frontend_P_3158_L1_isr,
                                             (void *)pDevice,
                                             0);
        if (errCode) { BERR_TRACE(errCode); goto err; }
    }
    else if(pSettings->gpioInterrupt){
        BDBG_MSG(("Connecting GPIO interrupt"));
        NEXUS_Gpio_SetInterruptCallback_priv(pSettings->gpioInterrupt,
                                             NEXUS_Frontend_P_3158_L1_isr,
                                             (void *)pDevice,
                                             0);
    }

    if (initHab) {
        BDBG_MSG(("Calling BHAB_Open"));
        errCode = BHAB_Open(&habHandle, regHandle, &habSettings);
        BDBG_MSG(("Calling BHAB_Open...Done: hab: %p",(void *)habHandle));
        if (errCode) { BERR_TRACE(errCode); goto err; }

        pDevice->hab = habHandle;
    }

    errCode = BHAB_GetInterruptEventHandle(pDevice->hab, &pDevice->isrEvent);
    if(errCode) { errCode = BERR_TRACE(errCode); goto err; }

    pDevice->isrEventCallback = NEXUS_RegisterEvent(pDevice->isrEvent, NEXUS_Frontend_P_3158_IsrEvent, pDevice->hab);
    if ( NULL == pDevice->isrEventCallback ) { errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err; }

    errCode =  BHAB_GetTunerChannels(pDevice->hab, &pDevice->capabilities.totalTunerChannels);
    if (errCode) { BERR_TRACE(errCode); goto err; }
    pDevice->numChannels = pDevice->capabilities.totalTunerChannels;
    if(!pDevice->capabilities.channelCapabilities){
        pDevice->capabilities.channelCapabilities = (BHAB_ChannelCapability *) BKNI_Malloc( pDevice->capabilities.totalTunerChannels*sizeof(BHAB_ChannelCapability));
        if(!pDevice->capabilities.channelCapabilities){
            rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done;
        }
    }

    rc =  BHAB_GetCapabilities(pDevice->hab, &pDevice->capabilities);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    BDBG_MSG(("pDevice->capabilities.totalTunerChannels: %d", pDevice->capabilities.totalTunerChannels));
    for(i=0; i<pDevice->capabilities.totalTunerChannels; i++){
        if(pDevice->capabilities.channelCapabilities[i].demodCoreType.ads) pDevice->adsPresent = true;
        if(pDevice->capabilities.channelCapabilities[i].demodCoreType.aob) pDevice->aobPresent = true;
        if(pDevice->capabilities.channelCapabilities[i].demodCoreType.ifdac) pDevice->ifDacPresent = true;

        if(pDevice->capabilities.channelCapabilities[i].demodCoreType.ifdac) { BDBG_MSG(("channel %d is ifdac", i)); }
        if(pDevice->capabilities.channelCapabilities[i].demodCoreType.aob) { BDBG_MSG(("channel %d is oob", i)); }
    }

    /* TODO: When IFDAC support is added, remove this temporary workaround. */
    if (pDevice->ifDacPresent) {
        BDBG_MSG((">>>Removing IFDAC from possible channels<<<"));
        pDevice->capabilities.totalTunerChannels--;
        pDevice->ifDacPresent = false;
        pDevice->numChannels--;
    }

    BDBG_MSG(("Filling out API tree"));
    pFrontendDevice->getCapabilities = NEXUS_FrontendDevice_P_3158_GetCapabilities;
    pFrontendDevice->getTunerCapabilities = NEXUS_FrontendDevice_P_3158_GetTunerCapabilities;
    pFrontendDevice->getStatus = NEXUS_FrontendDevice_P_3158_GetStatus;
#if 0
    pFrontendDevice->getInternalGain = NEXUS_FrontendDevice_P_3158_GetInternalGain;
    pFrontendDevice->getExternalGain = NEXUS_FrontendDevice_P_3158_GetExternalGain;
    pFrontendDevice->setExternalGain = NEXUS_FrontendDevice_P_3158_SetExternalGain;
#endif
    pFrontendDevice->getSettings = NEXUS_FrontendDevice_P_3158_GetSettings;
    pFrontendDevice->setSettings = NEXUS_FrontendDevice_P_3158_SetSettings;
    pFrontendDevice->getDeviceAmplifierStatus = NEXUS_FrontendDevice_P_3158_GetDeviceAmplifierStatus;

    pFrontendDevice->getWakeupSettings = NEXUS_FrontendDevice_P3158_GetWakeupFilter;
    pFrontendDevice->setWakeupSettings = NEXUS_FrontendDevice_P3158_SetWakeupFilter;

    pDevice->chipFamilyId = 0x3158;
    pFrontendDevice->familyId = 0x3158;
    pFrontendDevice->close = NEXUS_FrontendDevice_P_Close3158;

    pFrontendDevice->mode = NEXUS_StandbyMode_eOn;
    pFrontendDevice->standby = NEXUS_FrontendDevice_P_3158_Standby;

    BDBG_MSG(("Finished creating frontend device %p", (void *)pFrontendDevice));
    return NEXUS_SUCCESS;
done:
    return rc;
err:
    NEXUS_FrontendDevice_P_Close3158(pFrontendDevice);
    return NEXUS_UNKNOWN;
}

NEXUS_Error NEXUS_FrontendDevice_P_3158_InitAp(NEXUS_FrontendDeviceHandle device, bool initHab)
{
    NEXUS_3158Device *pDevice = (NEXUS_3158Device *)device->pDevice;
    NEXUS_Error rc = NEXUS_SUCCESS;

    if (initHab) {
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

            rc = Nexus_Core_P_Img_Create(NEXUS_CORE_IMG_ID_FRONTEND_3158, &pImgContext, &imgInterface);
            if (rc) { BERR_TRACE(rc); goto done; }
            rc = imgInterface.open((void*)pImgContext, &pImg, 0);
            if (rc) { BERR_TRACE(rc); goto done; }
            rc = imgInterface.next(pImg, 0, (const void **)&pImage, header_size);
            if (rc) { BERR_TRACE(rc); goto done; }
            code_size = (pImage[72] << 24) | (pImage[73] << 16) | (pImage[74] << 8) | pImage[75];
            fw_size = code_size + header_size;
            rc = NEXUS_Memory_Allocate(fw_size, NULL, (void **)&fw);
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
        fw_image = bcm3158_leap_image;
#endif
        BDBG_MSG(("BHAB_InitAp"));
        rc = BHAB_InitAp(pDevice->hab, fw_image);
        BDBG_MSG(("...done BHAB_InitAp"));
#if NEXUS_MODE_driver
done:
        NEXUS_Memory_Free(fw);
#endif
    }

    return rc;
}

NEXUS_Error NEXUS_FrontendDevice_P_3158_PostInitAp(NEXUS_FrontendDeviceHandle device)
{
    NEXUS_3158Device *pDevice = (NEXUS_3158Device *)device->pDevice;
    BERR_Code errCode;
    BADS_Settings adsConfig;
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned totalChannels = 0;
    unsigned i;
#if NEXUS_FRONTEND_315x_OOB
    BAOB_Settings aob_cfg;
#endif
#if NEXUS_HAS_MXT
    BMXT_Settings mxtSettings;
#endif

    /* Open ADS */
    errCode = BADS_3158_GetDefaultSettings( &adsConfig, NULL);
    if (errCode) { errCode = BERR_TRACE(errCode); goto done; }

    adsConfig.hGeneric = pDevice->hab;
#if 0
    adsConfig.transportConfig = BADS_TransportData_eSerial;
    adsConfig.isOpenDrain = pDevice->openSettings.inBandOpenDrain;
#endif
    BDBG_MSG(("BADS_Open"));
    errCode = BADS_Open(&pDevice->ads, NULL, NULL, NULL, &adsConfig);
    if (errCode) { errCode = BERR_TRACE(errCode); goto done; }
    BDBG_MSG(("BADS_Init"));
    errCode = BADS_Init(pDevice->ads);
    if (errCode) { errCode = BERR_TRACE(errCode); goto done; }

    /* Create device events, etc. */
    errCode = BKNI_CreateEvent(&pDevice->spectrumEvent);
    if (errCode) { errCode = BERR_TRACE(rc); goto done; }

    /* get total ADS channel number*/
    BDBG_MSG(("BADS_GetTotalChannels"));
    errCode = BADS_GetTotalChannels(pDevice->ads, &totalChannels);
    if (errCode) { errCode = BERR_TRACE(errCode); goto done; }
#if 0
    /* TODO: temporarily ignoring this error because of the  earlier BHAB_GetTotalChannels call */
    if (totalChannels != pDevice->numChannels) {
        BDBG_ERR(("ADS and HAB disagree on number of channels"));
        errCode = BERR_TRACE(NEXUS_UNKNOWN); goto done;
    }
    BDBG_MSG(("channels: %d", totalChannels));
    pDevice->numChannels = totalChannels;
#endif

    for (i=0; i < totalChannels; i++) {
        BADS_ChannelSettings adsChannelSettings;

        errCode = BADS_GetChannelDefaultSettings( pDevice->ads, i, &adsChannelSettings);
        if (errCode) { errCode = BERR_TRACE(rc); goto done; }

        errCode = BADS_OpenChannel( pDevice->ads, &pDevice->ads_chn[i], i, &adsChannelSettings);
        if (errCode) { errCode = BERR_TRACE(rc); goto done; }

        errCode = BADS_InstallCallback(pDevice->ads_chn[i], BADS_Callback_eLockChange, (BADS_CallbackFunc)NEXUS_Frontend_P_3158_callback_isr, (void*)&pDevice->frontendHandle[i]);
        if (errCode) { errCode = BERR_TRACE(rc); goto done; }
        errCode = BADS_InstallCallback(pDevice->ads_chn[i], BADS_Callback_eNoSignal, (BADS_CallbackFunc)NEXUS_Frontend_P_3158_callback_isr, (void*)&pDevice->frontendHandle[i]);
        if (errCode) { errCode = BERR_TRACE(rc); goto done; }
        errCode = BADS_InstallCallback(pDevice->ads_chn[i], BADS_Callback_eAsyncStatusReady, (BADS_CallbackFunc)NEXUS_Frontend_P_3158_AsyncStatusCallback_isr, (void*)&pDevice->frontendHandle[i]);
        if (errCode) { errCode = BERR_TRACE(rc); goto done; }
#if 0
        /* TODO */
        errCode = BADS_InstallCallback(pDevice->ads_chn[adsChannels], BADS_Callback_eUpdateGain, (BADS_CallbackFunc)NEXUS_Frontend_P_3158_updateGainCallback_isr, (void*)pDevice->updateGainAppCallback);
        if (errCode) { errCode = BERR_TRACE(rc); goto done; }
#endif
        errCode = BADS_InstallCallback(pDevice->ads_chn[i], BADS_Callback_eSpectrumDataReady, (BADS_CallbackFunc)NEXUS_Frontend_P_3158_spectrumDataReadyCallback, (void*)pDevice->spectrumEvent);
        if (errCode) { errCode = BERR_TRACE(rc); goto done; }
    }

#if NEXUS_FRONTEND_315x_OOB
    /* Generally there is only one OOB channel on a given platform. */
    if((pDevice->aob == NULL) && (pDevice->aobPresent)){
        rc = BAOB_GetDefaultSettings( &aob_cfg, NULL);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        /* There is only one AOB device in 3158/3114. so the device id is left to default settings. */
        aob_cfg.hGeneric = pDevice->hab;
        aob_cfg.serialData = true;
#if 0 /* AT */
        aob_cfg.isOpenDrain = pDevice->openSettings.outOfBand.openDrain;
        aob_cfg.nyquist = pDevice->openSettings.outOfBand.nyquist;
#endif
        rc = BAOB_Open(&pDevice->aob, NULL, NULL, NULL, &aob_cfg);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = BAOB_InstallCallback(pDevice->aob, BAOB_Callback_eLockChange, (BAOB_CallbackFunc)NEXUS_Frontend_P_3158_callback_isr, (void*)&pDevice->frontendHandle[NEXUS_3158_OOB_CHANNEL]);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = BAOB_InstallCallback(pDevice->aob, BAOB_Callback_eAsyncStatusReady, (BAOB_CallbackFunc)NEXUS_Frontend_P_3158_AsyncStatusCallback_isr, (void*)&pDevice->frontendHandle[NEXUS_3158_OOB_CHANNEL]);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    else{
        BDBG_ERR(("Out of band channel is already opened."));
        rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto done;
    }
#endif

#if NEXUS_HAS_MXT
    /* open MXT */
    BMXT_GetDefaultSettings(&mxtSettings);
    mxtSettings.chip = BMXT_Chip_e3158;
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

done:
    return rc;
}

static NEXUS_Error NEXUS_FrontendDevice_P_Init3158(NEXUS_3158Device *pDevice, bool initHab)
{
    NEXUS_Error errCode;

    BDBG_MSG(("NEXUS_FrontendDevice_P_Init3158: NEXUS_FrontendDevice_P_3158_PreInitAp"));
    errCode = NEXUS_FrontendDevice_P_3158_PreInitAp(pDevice->pGenericDeviceHandle, initHab);
    if (!errCode) {
        BDBG_MSG(("NEXUS_FrontendDevice_P_Init3158: NEXUS_FrontendDevice_P_3158_InitAp"));
        errCode = NEXUS_FrontendDevice_P_3158_InitAp(pDevice->pGenericDeviceHandle, initHab);
    } else BERR_TRACE(errCode);
    if (!errCode) {
        BDBG_MSG(("NEXUS_FrontendDevice_P_Init3158: NEXUS_FrontendDevice_P_3158_PostInitAp"));
        errCode = NEXUS_FrontendDevice_P_3158_PostInitAp(pDevice->pGenericDeviceHandle);
    } else BERR_TRACE(errCode);

#if 0
    /* Hook in case of post-async-firmware requirements */
    if (!errCode)
        errCode = NEXUS_Frontend_3158_P_DelayedInitialization(pDevice->pGenericDeviceHandle);
#endif

    BDBG_MSG(("NEXUS_FrontendDevice_P_Init3158: returning %d", errCode));
    return errCode;
}

static void NEXUS_Frontend_P_Uninit3158(NEXUS_3158Device *pDevice, bool uninitHab)
{
    unsigned i;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3158Device);

    BDBG_MSG(("Closing 3158 device %p handles", (void *)pDevice));

    for (i=0; i < pDevice->numChannels; i++) {
        if (pDevice->ads_chn[i]) {
            BADS_InstallCallback(pDevice->ads_chn[i], BADS_Callback_eLockChange, NULL, (void*)&pDevice->frontendHandle[i]);
            BADS_InstallCallback(pDevice->ads_chn[i], BADS_Callback_eNoSignal, NULL, (void*)&pDevice->frontendHandle[i]);
            if (pDevice->lockAppCallback[i]) {
                NEXUS_IsrCallback_Destroy(pDevice->lockAppCallback[i]);
                pDevice->lockAppCallback[i] = NULL;
            }
            if (pDevice->asyncStatusAppCallback[i]) {
                NEXUS_IsrCallback_Destroy(pDevice->asyncStatusAppCallback[i]);
                pDevice->asyncStatusAppCallback[i] = NULL;
            }
            if (pDevice->spectrumDataAppCallback[i]) {
                NEXUS_TaskCallback_Destroy(pDevice->spectrumDataAppCallback[i]);
                pDevice->spectrumDataAppCallback[i] = NULL;
            }
            BADS_CloseChannel(pDevice->ads_chn[i]);
            pDevice->ads_chn[i] = NULL;
        } else if (i == pDevice->numChannels-1) { /* OOB channel */
            if (pDevice->lockAppCallback[i]) {
                NEXUS_IsrCallback_Destroy(pDevice->lockAppCallback[i]);
                pDevice->lockAppCallback[i] = NULL;
            }
            if (pDevice->asyncStatusAppCallback[i]) {
                NEXUS_IsrCallback_Destroy(pDevice->asyncStatusAppCallback[i]);
                pDevice->asyncStatusAppCallback[i] = NULL;
            }
        }
    }
    if (pDevice->spectrumEvent) {
        BKNI_DestroyEvent(pDevice->spectrumEvent);
        pDevice->spectrumEvent = NULL;
    }
    if (pDevice->ads) {
        BADS_Close(pDevice->ads);
        pDevice->ads = NULL;
    }
#if NEXUS_FRONTEND_315x_OOB
    if (pDevice->aob){
        BAOB_Close(pDevice->aob);
        pDevice->aob = NULL;
    }
#endif
    if (pDevice->isrEventCallback) {
        NEXUS_UnregisterEvent(pDevice->isrEventCallback);
        pDevice->isrEventCallback = NULL;
    }
#if NEXUS_HAS_MXT
    if (pDevice->pGenericDeviceHandle->mtsifConfig.mxt) {
        BMXT_Close(pDevice->pGenericDeviceHandle->mtsifConfig.mxt);
        pDevice->pGenericDeviceHandle->mtsifConfig.mxt = NULL;
    }
#endif
    if (uninitHab && pDevice->hab) {
        BHAB_Close(pDevice->hab);
        pDevice->hab = NULL;
    }
    if (pDevice->capabilities.channelCapabilities) {
        BKNI_Free(pDevice->capabilities.channelCapabilities);
        pDevice->capabilities.channelCapabilities = NULL;
    }
}

void NEXUS_FrontendDevice_P_3158_S3Standby(NEXUS_3158Device *pDevice, bool uninitHab)
{
    BHAB_3158_StandbySettings standbySettings;
    NEXUS_Frontend_P_Uninit3158(pDevice, uninitHab);

    BHAB_3158_P_GetStandbySettings(pDevice->hab, &standbySettings);
    standbySettings.mode = BHAB_3158_StandbyMode_eDeepSleep;
    BHAB_3158_P_SetStandbySettings(pDevice->hab, &standbySettings);
}

static NEXUS_Error NEXUS_Frontend_P_3158_RestoreCallbacks(NEXUS_3158Device *pDevice) {
    NEXUS_Error rc;
    unsigned i;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3158Device);

    for (i=0; i < pDevice->numChannels; i++) {
        NEXUS_IsrCallbackHandle lockCallback;
        NEXUS_IsrCallbackHandle qamAsyncStatusReadyCallback = NULL;
        NEXUS_TaskCallbackHandle spectrumDataCallback  = NULL;
        NEXUS_FrontendHandle frontendHandle = pDevice->frontendHandle[i];

        if (frontendHandle) {
            /* lock callback */
            lockCallback = NEXUS_IsrCallback_Create(frontendHandle, NULL);
            if ( NULL == lockCallback ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err;}
            pDevice->lockAppCallback[i] = lockCallback;

            qamAsyncStatusReadyCallback = NEXUS_IsrCallback_Create(frontendHandle, NULL);
            if ( NULL == qamAsyncStatusReadyCallback ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err;}

            spectrumDataCallback = NEXUS_TaskCallback_Create(frontendHandle, NULL);
            if ( NULL == spectrumDataCallback ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err;}

            pDevice->spectrumDataAppCallback[i] = spectrumDataCallback;
            pDevice->asyncStatusAppCallback[i] = qamAsyncStatusReadyCallback;
        }
    }
    return rc;
err:
    BERR_TRACE(rc);
    return rc;
}

static NEXUS_Error NEXUS_FrontendDevice_P_3158_Standby(void *handle, const NEXUS_StandbySettings *pSettings)
{

    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3158Device *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_3158Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3158Device);

    /* entering S3 */
    if ((pDevice->pGenericDeviceHandle->mode != NEXUS_StandbyMode_eDeepSleep) && (pSettings->mode == NEXUS_StandbyMode_eDeepSleep)) {

        BDBG_MSG(("NEXUS_FrontendDevice_P_3158_Standby: Entering deep sleep..."));

        NEXUS_FrontendDevice_P_3158_S3Standby(pDevice, false);

        BKNI_EnterCriticalSection();
        pDevice->pGenericDeviceHandle->openPending = true;
        pDevice->pGenericDeviceHandle->openFailed = false;
        BKNI_LeaveCriticalSection();

    } else /* waking from S3 */
        if ((pDevice->pGenericDeviceHandle->mode == NEXUS_StandbyMode_eDeepSleep) && (pSettings->mode != NEXUS_StandbyMode_eDeepSleep)) {

        BDBG_MSG(("NEXUS_FrontendDevice_P_3158_Standby: Waking up..."));

        {
            BHAB_3158_StandbySettings standbySettings;
            BHAB_3158_P_GetStandbySettings(pDevice->hab, &standbySettings);
            standbySettings.mode = BHAB_3158_StandbyMode_eOn;
            BHAB_3158_P_SetStandbySettings(pDevice->hab, &standbySettings);
        }

        BDBG_MSG(("NEXUS_FrontendDevice_P_3158_Standby: reinitializing..."));
        rc = NEXUS_FrontendDevice_P_Init3158(pDevice, false);
        if (rc) { rc = BERR_TRACE(rc); goto done;}

        NEXUS_Frontend_P_3158_RestoreCallbacks(pDevice);

        BKNI_EnterCriticalSection();
        pDevice->pGenericDeviceHandle->openPending = false;
        pDevice->pGenericDeviceHandle->openFailed = false;
        BKNI_LeaveCriticalSection();
    }

    /* entering S2 */
    if ((pDevice->pGenericDeviceHandle->mode != NEXUS_StandbyMode_ePassive) && (pSettings->mode == NEXUS_StandbyMode_ePassive)) {
        BDBG_MSG(("NEXUS_FrontendDevice_P_3158_Standby: Entering S2..."));

    } else /* waking from S2 */
        if ((pDevice->pGenericDeviceHandle->mode == NEXUS_StandbyMode_ePassive) && (pSettings->mode != NEXUS_StandbyMode_ePassive)) {

            BDBG_MSG(("NEXUS_FrontendDevice_P_3158_Standby: Waking up from S2..."));

    }

done:
    BDBG_MSG(("NEXUS_FrontendDevice_P_3158_Standby(%d->%d): returning %d...", pDevice->pGenericDeviceHandle->mode, pSettings->mode, rc));
    return rc;
}


NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_P_Open3158(unsigned index, const NEXUS_FrontendDeviceOpenSettings *pSettings)
{
    NEXUS_FrontendDevice *device;
    NEXUS_Error rc;

    BDBG_ASSERT(pSettings);

    device = NEXUS_FrontendDevice_P_3158_Alloc(index, pSettings);
    if (!device) goto done;

    rc = NEXUS_FrontendDevice_P_3158_PreInitAp(device, true);
    if (rc) {
        NEXUS_FrontendDevice_P_Close3158(device);
        device = NULL; goto done;
    }

    rc = NEXUS_FrontendDevice_P_3158_InitAp(device, true);
    if (rc) {
        NEXUS_FrontendDevice_P_Close3158(device);
        device = NULL; goto done;
    }
    rc = NEXUS_FrontendDevice_P_3158_PostInitAp(device);
    if (rc) {
        NEXUS_FrontendDevice_P_Close3158(device);
        device = NULL; goto done;
    }

done:
    return device;
}

static void NEXUS_Frontend_P_3158_Close(NEXUS_FrontendHandle handle)
{
    NEXUS_3158Device *pDevice;
    NEXUS_3158Channel *pChannel;

    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    pChannel =(NEXUS_3158Channel *)handle->pDeviceHandle;
    BDBG_ASSERT(NULL != pChannel);
    pDevice = (NEXUS_3158Device *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3158Device);

    NEXUS_Frontend_P_Destroy(handle);
#if NEXUS_FRONTEND_315x_OOB
    if(pDevice->frontendHandle[pChannel->chn_num]->capabilities.outOfBand == true){
        if(pDevice->aob) BAOB_InstallCallback(pDevice->aob, BAOB_Callback_eLockChange, NULL, NULL);
        if(pDevice->aob) BAOB_InstallCallback(pDevice->aob, BAOB_Callback_eAsyncStatusReady, NULL, NULL);
    }
#endif
    pDevice->frontendHandle[pChannel->chn_num] = NULL;
    BKNI_Memset(pChannel, 0, sizeof(*pChannel));
    BKNI_Free(pChannel);

    return;
}

static void NEXUS_Frontend_P_3158_UninstallCallbacks(void *handle)
{
    NEXUS_3158Device *pDevice;
    NEXUS_3158Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3158Channel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3158Device);

    if (pChannel->chn_num >= NEXUS_MAX_3158_FRONTENDS)
    {
        BDBG_ERR((" Unsupported channel."));
        BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    if (pDevice->lockAppCallback[pChannel->chn_num])
        NEXUS_IsrCallback_Set(pDevice->lockAppCallback[pChannel->chn_num], NULL);
    if (pDevice->asyncStatusAppCallback[pChannel->chn_num])
        NEXUS_IsrCallback_Set(pDevice->asyncStatusAppCallback[pChannel->chn_num], NULL);
    if (pDevice->spectrumDataAppCallback[pChannel->chn_num])
        NEXUS_TaskCallback_Set(pDevice->spectrumDataAppCallback[pChannel->chn_num], NULL);

done:
    return;
}

static NEXUS_Error NEXUS_Frontend_P_3158_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3158Device *pDevice;
    NEXUS_3158Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3158Channel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3158Device);

    BSTD_UNUSED(enabled);
    BSTD_UNUSED(pSettings);

    return rc;
}

NEXUS_FrontendHandle NEXUS_Frontend_P_Open3158(const NEXUS_FrontendChannelSettings *pSettings)
{
    NEXUS_FrontendDeviceHandle pFrontendDevice;
    NEXUS_3158Device *p3158Device;
    NEXUS_3158Channel *pChannel;
    BERR_Code rc;
    unsigned channelNumber;
    NEXUS_FrontendChannelType type = NEXUS_FrontendChannelType_eCable;

    NEXUS_FrontendHandle frontendHandle = NULL;

    BDBG_ASSERT(pSettings);
    BDBG_ASSERT(pSettings->device);

    if (pSettings->device == NULL) {
        BDBG_WRN(("No device..."));
        return NULL;
    }

    pFrontendDevice = pSettings->device;
    p3158Device = (NEXUS_3158Device *)pFrontendDevice->pDevice;

    /*
     * Ignore pSettings->type, and guess based on capabilities. (This allows for generic frontend
     * handle initialization based solely on numTuners.)
     *
     * Cable inband channels are 0..num inband.
     * OOB is the next channel. numTuners = num inband + num oob.
     *
     * In the future, pSettings->type will be deprecated to only be valid on hybrid frontend chips,
     * where cable, oob, ifdac, terrestrial, and/or satellite channels are separately 0-indexed.
     */
    channelNumber = pSettings->channelNumber;
#if NEXUS_FRONTEND_315x_OOB
    if (channelNumber == p3158Device->numChannels-1 && p3158Device->aobPresent) {
        BDBG_MSG(("opening OOB"));
        type = NEXUS_FrontendChannelType_eCableOutOfBand;
    }
#endif

    /* If already opened, return the previously opened handle */
    if ( p3158Device->frontendHandle[channelNumber] != NULL )
    {
        return p3158Device->frontendHandle[channelNumber];
    }

    pChannel = (NEXUS_3158Channel*)BKNI_Malloc(sizeof(*pChannel));
    if ( NULL == pChannel ) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err;}
    BKNI_Memset(pChannel, 0, sizeof(*pChannel));

    /* Create a Nexus frontend handle */
    frontendHandle = NEXUS_Frontend_P_Create(pChannel);
    if ( NULL == frontendHandle ) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err;}

    /* Set capabilities and channel type parameters */
    if (type == NEXUS_FrontendChannelType_eCable) {
        frontendHandle->capabilities.qam = true;
        frontendHandle->capabilities.outOfBand = false;
        frontendHandle->capabilities.upstream = false;
        BKNI_Memset(frontendHandle->capabilities.qamModes, true, sizeof(frontendHandle->capabilities.qamModes));
        /* bind functions*/
        frontendHandle->tuneQam = NEXUS_Frontend_P_3158_TuneQam;
        frontendHandle->untune = NEXUS_Frontend_P_3158_UnTuneQam;
        frontendHandle->getQamStatus = NEXUS_Frontend_P_3158_GetQamStatus;
        frontendHandle->requestQamAsyncStatus = NEXUS_Frontend_P_3158_RequestQamAsyncStatus;
        frontendHandle->getQamAsyncStatus = NEXUS_Frontend_P_3158_GetQamAsyncStatus;
        frontendHandle->resetStatus = NEXUS_Frontend_P_3158_ResetQamStatus;
#if 0
        frontendHandle->getQamScanStatus = NEXUS_Frontend_P_3158_GetQamScanStatus;
#endif
    }
    else if(type == NEXUS_FrontendChannelType_eCableOutOfBand) {
        frontendHandle->capabilities.qam = false;
        frontendHandle->capabilities.outOfBand = true;
        frontendHandle->capabilities.upstream = false;
        BKNI_Memset(frontendHandle->capabilities.outOfBandModes, true, sizeof(frontendHandle->capabilities.outOfBandModes));
        /* bind functions*/
#if NEXUS_FRONTEND_315x_OOB
        frontendHandle->tuneOutOfBand = NEXUS_Frontend_P_3158_TuneOob;
        frontendHandle->getOutOfBandStatus = NEXUS_Frontend_P_3158_GetOobStatus;
        frontendHandle->resetStatus = NEXUS_Frontend_P_3158_ResetOobStatus;
        frontendHandle->untune = NEXUS_Frontend_P_3158_UnTuneOob;
        frontendHandle->requestOutOfBandAsyncStatus = NEXUS_Frontend_P_3158_RequestOobAsyncStatus;
        frontendHandle->getOutOfBandAsyncStatus = NEXUS_Frontend_P_3158_GetOobAsyncStatus;
#endif
    }

    frontendHandle->readSoftDecisions = NEXUS_Frontend_P_3158_ReadSoftDecisions;
    frontendHandle->reapplyTransportSettings = NEXUS_Frontend_P_3158_ReapplyTransportSettings;
    frontendHandle->close = NEXUS_Frontend_P_3158_Close;
    frontendHandle->getFastStatus = NEXUS_Frontend_P_3158_GetFastStatus;
    frontendHandle->writeRegister = NEXUS_Frontend_P_3158_WriteRegister;
    frontendHandle->readRegister = NEXUS_Frontend_P_3158_ReadRegister;
    frontendHandle->getType = NEXUS_Frontend_P_3158_GetType;
    frontendHandle->requestSpectrumData = NEXUS_Frontend_P_3158_RequestSpectrumAnalyzerData;
    frontendHandle->standby = NEXUS_Frontend_P_3158_Standby;
    frontendHandle->uninstallCallbacks = NEXUS_Frontend_P_3158_UninstallCallbacks;

    {
        NEXUS_IsrCallbackHandle lockCallback;
        /* lock callback */
        lockCallback = NEXUS_IsrCallback_Create(frontendHandle, NULL);
        if ( NULL == lockCallback ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err;}
        p3158Device->lockAppCallback[channelNumber] = lockCallback;
    }

    if (type == NEXUS_FrontendChannelType_eCable)
    {
        NEXUS_IsrCallbackHandle qamAsyncStatusReadyCallback = NULL;
        NEXUS_TaskCallbackHandle spectrumDataCallback  = NULL;
        qamAsyncStatusReadyCallback = NEXUS_IsrCallback_Create(frontendHandle, NULL);
        if ( NULL == qamAsyncStatusReadyCallback ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err;}

        spectrumDataCallback = NEXUS_TaskCallback_Create(frontendHandle, NULL);
        if ( NULL == spectrumDataCallback ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err;}

        p3158Device->spectrumDataAppCallback[channelNumber] = spectrumDataCallback;
        p3158Device->asyncStatusAppCallback[channelNumber] = qamAsyncStatusReadyCallback;
    }
    else if(type == NEXUS_FrontendChannelType_eCableOutOfBand)
    {
        NEXUS_IsrCallbackHandle oobAsyncStatusReadyCallback = NULL;
        oobAsyncStatusReadyCallback = NEXUS_IsrCallback_Create(frontendHandle, NULL);
        if ( NULL == oobAsyncStatusReadyCallback ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err;}

        p3158Device->asyncStatusAppCallback[channelNumber] = oobAsyncStatusReadyCallback;
        channelNumber = NEXUS_3158_OOB_CHANNEL;
    }

    frontendHandle->pGenericDeviceHandle = pFrontendDevice;

    p3158Device->frontendHandle[channelNumber] = frontendHandle;
    /* save channel number in pChannel*/
    pChannel->chn_num = channelNumber;
    pChannel->pDevice = p3158Device;
#if 0
    /* TODO: Fill out family/chip ID correctly */
    frontendHandle->chip.familyId = pDevice->chipFamilyId;
    frontendHandle->chip.id = pDevice->chipId;
#else
    frontendHandle->chip.familyId = 0x3158;
    frontendHandle->chip.id = 0x3158;
#endif

    frontendHandle->userParameters.isMtsif = true;
    frontendHandle->mtsif.inputBand = pSettings->channelNumber;

    return frontendHandle;

err:
    if (pChannel) BKNI_Free(pChannel);
    return NULL;
}
