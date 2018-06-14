/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
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
#include "bhab_7255.h"
#include "bads_7255.h"
#include "bhab_7255_fw.h"
#include "bhab.h"
#include "bads.h"
#include "btnr.h"
#include "btnr_7255ib.h"
#include "priv/nexus_transport_priv.h"
#include "bhab_ctfe_img.h"
#include "priv/nexus_core_img.h"
#include "priv/nexus_core_img_id.h"

#include "bchp_leap_host_l1.h"
#include "bhab_7255_priv.h"
#include "bchp_leap_ctrl.h"


/* set to 1 to enable L1 interrupt messages */
#define NEXUS_FRONTEND_DEBUG_IRQ 0

BDBG_MODULE(nexus_frontend_7255);

BDBG_OBJECT_ID(NEXUS_7255Device);

#define BLEAP_HOST_L1_INTERRUPT_ID     BCHP_INT_ID_CREATE(BCHP_LEAP_HOST_L1_INTR_W0_STATUS, BCHP_LEAP_HOST_L1_INTR_W0_STATUS_LEAP_INTR_SHIFT)

typedef struct NEXUS_PreviousStatus
{
    unsigned fecCorrected;
    unsigned fecUncorrected;
    unsigned fecClean;
    unsigned viterbiUncorrectedBits;
    unsigned viterbiTotalBits;
    NEXUS_Time time;
} NEXUS_PreviousStatus;

#define NEXUS_MAX_7255_FRONTENDS 18
#define NEXUS_7255_MAX_DOWNSTREAM_CHANNELS 16
#define NEXUS_7255_OOB_CHANNEL NEXUS_7255_MAX_DOWNSTREAM_CHANNELS
/***************************************************************************
 * Internal Module Structure
 ***************************************************************************/
typedef struct NEXUS_7255Device
{
    BDBG_OBJECT(NEXUS_7255Device)
    BLST_S_ENTRY(NEXUS_7255Device) node;
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
    BINT_CallbackHandle irqCallback;
    BADS_Handle ads;
    NEXUS_FrontendDeviceSettings deviceSettings;
    NEXUS_FrontendHandle    frontendHandle[NEXUS_MAX_7255_FRONTENDS];
    BADS_ChannelHandle  ads_chn[NEXUS_7255_MAX_DOWNSTREAM_CHANNELS];
    BTNR_Handle tnr[NEXUS_7255_MAX_DOWNSTREAM_CHANNELS];
    NEXUS_FrontendQamSettings   qam[NEXUS_7255_MAX_DOWNSTREAM_CHANNELS];
    NEXUS_FrontendOutOfBandSettings oob;
    NEXUS_FrontendQamSettings   last_ads[NEXUS_7255_MAX_DOWNSTREAM_CHANNELS];
    BKNI_EventHandle isrEvent;
    NEXUS_EventCallbackHandle isrEventCallback;
    NEXUS_IsrCallbackHandle lockAppCallback[NEXUS_MAX_7255_FRONTENDS];
    NEXUS_IsrCallbackHandle updateGainAppCallback;
    NEXUS_IsrCallbackHandle asyncStatusAppCallback[NEXUS_MAX_7255_FRONTENDS];
    bool                    isInternalAsyncStatusCall[NEXUS_MAX_7255_FRONTENDS];
    bool isTunerPoweredOn[NEXUS_MAX_7255_FRONTENDS];
    bool isPoweredOn[NEXUS_MAX_7255_FRONTENDS];
    bool acquireInProgress[NEXUS_MAX_7255_FRONTENDS];
    unsigned count[NEXUS_MAX_7255_FRONTENDS];
    NEXUS_PreviousStatus previousStatus[NEXUS_MAX_7255_FRONTENDS];
    NEXUS_FrontendDevice *pGenericDeviceHandle;
    NEXUS_TunerHandle  ifDacHandle;
    NEXUS_FrontendHandle  ifDacFeHandle;
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
    unsigned oobChannelNumber;
    NEXUS_TransportWakeupSettings wakeupSettings;
    void *leapBuffer;
} NEXUS_7255Device;

typedef struct NEXUS_7255Channel
{
    unsigned chn_num; /* channel number */
    NEXUS_7255Device *pDevice; /* 7255 device*/
} NEXUS_7255Channel;

static void NEXUS_Frontend_P_7255_callback_isr(void *pParam)
{
    NEXUS_FrontendHandle frontendHandle = NULL;
    NEXUS_7255Channel *pChannel;
    NEXUS_7255Device *pDevice;
    BDBG_ASSERT(NULL != pParam);
    frontendHandle = *((NEXUS_FrontendHandle *)pParam);
    BDBG_ASSERT(NULL != frontendHandle);
    pChannel = frontendHandle->pDeviceHandle;
    BDBG_ASSERT(NULL != pChannel);
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7255Device);

    BDBG_MSG(("NEXUS_Frontend_P_7255_callback_isr"));

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

static void NEXUS_Frontend_P_7255_AsyncStatusCallback_isr(void *pParam)
{
    NEXUS_FrontendHandle frontendHandle = NULL;
    NEXUS_7255Channel *pChannel;
    NEXUS_7255Device *pDevice;
    BDBG_ASSERT(NULL != pParam);
    frontendHandle = *((NEXUS_FrontendHandle *)pParam);
    BDBG_ASSERT(NULL != frontendHandle);
    pChannel = frontendHandle->pDeviceHandle;
    BDBG_ASSERT(NULL != pChannel);
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7255Device);

    if(!pDevice->isInternalAsyncStatusCall[pChannel->chn_num]){
        if (pDevice->asyncStatusAppCallback[pChannel->chn_num])
        {
            NEXUS_IsrCallback_Fire_isr(pDevice->asyncStatusAppCallback[pChannel->chn_num]);
        }
    }
}

/***************************************************************************
Summary:
    Enable/Disable interrupts for a 7255 device
 ***************************************************************************/
static void NEXUS_Frontend_P_7255_L1IsrControl_isr(bool enable, void *pParam)
{
    NEXUS_7255Device *pDevice = (NEXUS_7255Device *)pParam;
    BERR_Code rc;
    BDBG_ASSERT(NULL != pParam);

#if NEXUS_FRONTEND_DEBUG_IRQ
    BDBG_MSG(("%s 7255 Interrupt %p", enable ? "Enable" : "Disable", pParam));
#endif
    if (enable) {
        if (pDevice->openSettings.interruptMode != NEXUS_FrontendInterruptMode_ePolling) {
            rc = BINT_EnableCallback_isr(pDevice->irqCallback);
            if(rc){rc = BERR_TRACE(rc);}
        }
    } else {
        if (pDevice->openSettings.interruptMode != NEXUS_FrontendInterruptMode_ePolling) {
            rc = BINT_DisableCallback_isr(pDevice->irqCallback);
            if(rc){rc = BERR_TRACE(rc);}
        }
    }
}

static void NEXUS_Frontend_P_7255_L1_isr(void *pParam1, int pParam2)
{
    NEXUS_7255Device *pDevice = (NEXUS_7255Device *)pParam1;
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHAB_Handle hab;
    BDBG_ASSERT(NULL != pParam1);
    hab = (BHAB_Handle)pDevice->hab;
    BSTD_UNUSED(pParam2);

#if NEXUS_FRONTEND_DEBUG_IRQ
    BDBG_MSG(("7255 L1 ISR (hab: %p)", (void *)hab));
#endif
    if(hab != NULL){
        rc = BHAB_HandleInterrupt_isr(hab);
        if(rc){rc = BERR_TRACE(rc);}
    }
#if NEXUS_FRONTEND_DEBUG_IRQ
    BDBG_MSG(("Done: 7255 L1 ISR (hab: %p)", (void *)hab));
#endif
}

/***************************************************************************
Summary:
    ISR Event Handler for a 7255 device
 ***************************************************************************/
static void NEXUS_Frontend_P_7255_IsrEvent(void *pParam)
{
    NEXUS_7255Device *pDevice = (NEXUS_7255Device *)pParam;
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHAB_Handle hab;
    BDBG_ASSERT(NULL != pParam);
    hab = (BHAB_Handle)pDevice->hab;

#if NEXUS_FRONTEND_DEBUG_IRQ
    BDBG_MSG(("7255 ISR Callback (hab: %p)", (void *)hab));
#endif
    if(hab != NULL){
        rc = BHAB_ProcessInterruptEvent(hab);
        if(rc) BERR_TRACE(rc);
    }
}

static void NEXUS_Frontend_7255DevicePollingThread(void *arg)
{
    NEXUS_7255Device *pDevice = (NEXUS_7255Device *)arg;

    while(1){
        BKNI_Sleep(100);

        BKNI_EnterCriticalSection();
        if((!pDevice->pGenericDeviceHandle->abortThread) && pDevice->hab )
            NEXUS_Frontend_P_7255_L1_isr((void *)pDevice, 0);
        else
            goto done;
        BKNI_LeaveCriticalSection();
     }
done:
    BKNI_LeaveCriticalSection();
    return;
}

static NEXUS_Error NEXUS_Frontend_P_7255_WriteRegister(void *handle, unsigned address, uint32_t value)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7255Device *pDevice;
    NEXUS_7255Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7255Channel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7255Device);

    rc = BHAB_WriteRegister(pDevice->hab, address, &value);
    if(rc){rc = BERR_TRACE(rc);}

    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7255_ReadRegister(void *handle, unsigned address, uint32_t *value   )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7255Device *pDevice;
    NEXUS_7255Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7255Channel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7255Device);

    rc = BHAB_ReadRegister(pDevice->hab, address, value);
    if(rc){rc = BERR_TRACE(rc);}

    return rc;
}

static NEXUS_Error NEXUS_FrontendDevice_P_7255_GetStatus(void *handle, NEXUS_FrontendDeviceStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7255Device *pDevice;
    BHAB_AvsData avsData;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7255Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7255Device);
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

#if 0
static NEXUS_Error NEXUS_FrontendDevice_P_7255_Recalibrate(void *handle, const NEXUS_FrontendDeviceRecalibrateSettings *pSettings)
{
    BERR_Code rc = BERR_SUCCESS;
    BHAB_RecalibrateSettings recalibrateSettings;
    NEXUS_7255Device *pDevice;

    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7255Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7255Device);

    NEXUS_IsrCallback_Set(pDevice->cppmAppCallback, &(pSettings->cppm.powerLevelChange));
    NEXUS_IsrCallback_Set(pDevice->cppmDoneCallback, &(pSettings->cppm.calibrationComplete));

    rc = BHAB_7255_GetDefaultRecalibrateSettings(&recalibrateSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    recalibrateSettings.cppm.enabled = pSettings->cppm.enabled;
    recalibrateSettings.cppm.threshold = pSettings->cppm.threshold;
    recalibrateSettings.cppm.thresholdHysteresis = pSettings->cppm.thresholdHysteresis;

    rc = BHAB_SetRecalibrateSettings(pDevice->hab, &recalibrateSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}
#endif

static void NEXUS_FrontendDevice_P_7255_GetCapabilities(void *handle, NEXUS_FrontendDeviceCapabilities *pCapabilities)
{
    NEXUS_7255Device *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7255Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7255Device);

    /* The device is already initialized completely. Hence just copy the tuner capabilities. */
    pCapabilities->numTuners = pDevice->numChannels;

}

static void NEXUS_FrontendDevice_P_7255_GetTunerCapabilities(void *handle, unsigned tunerIndex, NEXUS_FrontendCapabilities *pCapabilities)
{
    NEXUS_7255Device *pDevice;
    BHAB_ChannelCapability *channelCapability;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7255Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7255Device);
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

static NEXUS_Error NEXUS_FrontendDevice_P_7255_GetSettings(void *handle, NEXUS_FrontendDeviceSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7255Device *pDevice = (NEXUS_7255Device *)handle;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7255Device);

    *pSettings = pDevice->deviceSettings;

    return rc;
}

static NEXUS_Error NEXUS_FrontendDevice_P_7255_SetSettings(void *handle, const NEXUS_FrontendDeviceSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7255Device *pDevice = (NEXUS_7255Device *)handle;
    BHAB_ConfigSettings habConfigSettings;
    bool poweredUp = false;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7255Device);

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
        habConfigSettings.loopthroughGain = pSettings->loopthroughGain == NEXUS_FrontendLoopthroughGain_eHigh ? BHAB_LoopthroughGain_eHigh : BHAB_LoopthroughGain_eLow;

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

static NEXUS_Error NEXUS_FrontendDevice_P_7255_GetDeviceAmplifierStatus(void *handle, NEXUS_FrontendDeviceAmplifierStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7255Device *pDevice = (NEXUS_7255Device *)handle;
    BHAB_LnaStatus lnaStatus;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7255Device);

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

static NEXUS_Error NEXUS_Frontend_P_7255_ReadSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length, size_t *pNumRead)
{
    #define TOTAL_ADS_SOFTDECISIONS 30

    NEXUS_Error rc = NEXUS_SUCCESS;
    int16_t d_i[TOTAL_ADS_SOFTDECISIONS], d_q[TOTAL_ADS_SOFTDECISIONS];
    int16_t return_length;
    NEXUS_7255Device *pDevice;
    unsigned i;
    NEXUS_7255Channel *pChannel;

    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7255Channel *)handle;
    pDevice = (NEXUS_7255Device *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7255Device);

    BKNI_Memset(pDecisions, 0, (sizeof(NEXUS_FrontendSoftDecision) * length));

    if (pChannel->chn_num >= NEXUS_7255_MAX_DOWNSTREAM_CHANNELS)
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

NEXUS_Error NEXUS_Frontend_P_7255_ReapplyTransportSettings(void *handle)
{
    BSTD_UNUSED(handle);

    return NEXUS_SUCCESS;
}

static void NEXUS_Frontend_P_7255_ResetQamStatus(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7255Channel *pChannel;
    NEXUS_7255Device *pDevice;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7255Channel *)handle;
    pDevice = (NEXUS_7255Device *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7255Device);

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
    Retrieve the chip family id, chip id, chip version and firmware version.
***************************************************************************/
static void NEXUS_Frontend_P_7255_GetType(void *handle, NEXUS_FrontendType *type)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BFEC_SystemVersionInfo  versionInfo;
    NEXUS_7255Channel *pChannel;
    NEXUS_7255Device *p7255Device;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7255Channel *)handle;
    p7255Device = (NEXUS_7255Device *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(p7255Device, NEXUS_7255Device);

    rc = BHAB_GetVersionInfo(p7255Device->hab, &versionInfo);
    if(rc) BERR_TRACE(rc);

    type->chip.familyId = (uint32_t)p7255Device->chipFamilyId;
    type->chip.id = (uint32_t)p7255Device->chipId;
    type->chip.version.major = (p7255Device->revId >> 8) + 1;
    type->chip.version.minor = p7255Device->revId & 0xff;
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
    if(p7255Device->revId != versionInfo.chipVersion){
        BDBG_ERR(("Type mismatch while retreiving chip version."));
        BERR_TRACE(BERR_UNKNOWN); goto done;
    }
    done:
#endif
    return;
}

static NEXUS_Error NEXUS_Frontend_P_7255_GetFastStatus(void *handle, NEXUS_FrontendFastStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7255Device *p7255Device;
    NEXUS_7255Channel *pChannel;
    BADS_LockStatus eLock = BADS_LockStatus_eUnlocked;
    NEXUS_FrontendDeviceHandle hFrontendDevice=NULL;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7255Channel *)handle;
    p7255Device = (NEXUS_7255Device *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(p7255Device, NEXUS_7255Device);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    pStatus->lockStatus = NEXUS_FrontendLockStatus_eUnknown;

    if(pChannel->chn_num < NEXUS_7255_MAX_DOWNSTREAM_CHANNELS){
        rc = BADS_GetLockStatus(p7255Device->ads_chn[pChannel->chn_num],  &eLock);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pStatus->lockStatus = NEXUS_Frontend_P_GetAdsLockStatus(eLock);
        BSTD_UNUSED(hFrontendDevice);
    }
    pStatus->acquireInProgress = p7255Device->acquireInProgress[pChannel->chn_num];
    return BERR_SUCCESS;
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7255_RequestQamAsyncStatus(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7255Device *p7255Device;
    NEXUS_7255Channel *pChannel;
    BTNR_7255_GainInfo gainInfo;
    BDBG_ASSERT(NULL != handle);
    pChannel = handle;
    p7255Device = (NEXUS_7255Device *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(p7255Device, NEXUS_7255Device);

    if (pChannel->chn_num >= NEXUS_7255_MAX_DOWNSTREAM_CHANNELS)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    BTNR_7255_GetTunerGain(p7255Device->last_ads[pChannel->chn_num].frequency, &gainInfo);

    BREG_Write32(g_pCoreHandles->reg, BCHP_LEAP_CTRL_GP58, gainInfo.rssi);
    BREG_Write32(g_pCoreHandles->reg, BCHP_LEAP_CTRL_GP59, gainInfo.rfGain);

    rc = BADS_RequestAsyncStatus(p7255Device->ads_chn[pChannel->chn_num]);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    return BERR_SUCCESS;
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7255_GetQamAsyncStatus(void *handle, NEXUS_FrontendQamStatus *pStatus)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    struct BADS_Status st;
    uint64_t totalbits=0, uncorrectedBits=0;
    unsigned cleanBlock = 0, correctedBlock = 0, unCorrectedBlock = 0, totalBlock = 0;
    NEXUS_Time currentTime;
    NEXUS_PreviousStatus *prevStatus;
    NEXUS_7255Device *p7255Device;
    NEXUS_7255Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7255Channel *)handle;
    p7255Device = (NEXUS_7255Device *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(p7255Device, NEXUS_7255Device);

    if (pChannel->chn_num >= NEXUS_7255_MAX_DOWNSTREAM_CHANNELS)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    rc = BADS_GetAsyncStatus(p7255Device->ads_chn[pChannel->chn_num],  &st);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    prevStatus = &p7255Device->previousStatus[pChannel->chn_num];
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
    pStatus->settings = p7255Device->last_ads[pChannel->chn_num];
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
        if(pStatus->settings.annex == NEXUS_FrontendQamAnnex_eA || pStatus->settings.annex == NEXUS_FrontendQamAnnex_eC)
            pStatus->postRsBer = ((uint64_t)unCorrectedBlock * 2097152 * 1024 )/((uint64_t)totalBlock*8*187);
        else if(pStatus->settings.annex == NEXUS_FrontendQamAnnex_eB)
            pStatus->postRsBer = ((uint64_t)unCorrectedBlock * 2097152 * 1024)/((uint64_t)totalBlock*7*122);
    }

    pStatus->viterbiUncorrectedBits = st.correctedBits + (uint32_t)((uint64_t)pStatus->fecUncorrected * 11224)/1000;
    if(pStatus->viterbiUncorrectedBits > prevStatus->viterbiUncorrectedBits)
        uncorrectedBits = pStatus->viterbiUncorrectedBits - prevStatus->viterbiUncorrectedBits;

    if(pStatus->settings.annex == NEXUS_FrontendQamAnnex_eA || pStatus->settings.annex == NEXUS_FrontendQamAnnex_eC){
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
static NEXUS_Error NEXUS_Frontend_P_7255_GetQamStatus(void *handle, NEXUS_FrontendQamStatus *pStatus)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned j=0;
    uint32_t buf=0;
    NEXUS_7255Device *p7255Device;
    NEXUS_7255Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7255Channel *)handle;
    p7255Device = (NEXUS_7255Device *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(p7255Device, NEXUS_7255Device);

    if (pChannel->chn_num >= NEXUS_7255_MAX_DOWNSTREAM_CHANNELS)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    if (p7255Device->isPoweredOn[pChannel->chn_num]) {
        uint32_t addr = BCHP_LEAP_CTRL_SW_SPARE0;
        uint32_t mask = BHAB_ADS_CHN0_STATUS_RDY;

        if (pChannel->chn_num > 7) {
            addr = BCHP_LEAP_CTRL_GP7;
            mask = 1;
        }
        p7255Device->isInternalAsyncStatusCall[pChannel->chn_num] = true;

        rc = NEXUS_Frontend_P_7255_RequestQamAsyncStatus(pChannel);
        if (rc) { rc = BERR_TRACE(rc); goto done; }

        for (j=0; j < 200; j++) {

            BKNI_Sleep(20);

            rc = BHAB_ReadRegister(p7255Device->hab, addr, &buf);
            if (rc) { rc = BERR_TRACE(rc); goto done; }

            if (buf & (mask << pChannel->chn_num % 8)) {
                rc = NEXUS_Frontend_P_7255_GetQamAsyncStatus(pChannel, pStatus);
                if (rc) { rc = BERR_TRACE(rc); goto done; }
                break;
            }
        }
    }
    else{
        BDBG_MSG(("The downstream core is Powered Off"));
    }

    p7255Device->isInternalAsyncStatusCall[pChannel->chn_num] = false;
    return BERR_SUCCESS;
done:
    return rc;
}

static void NEXUS_Frontend_P_7255_UnTuneQam(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BADS_InbandParam ibParam;
    NEXUS_7255Device *pDevice;
    NEXUS_7255Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7255Channel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7255Device);

    if (pChannel->chn_num >= NEXUS_7255_MAX_DOWNSTREAM_CHANNELS)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    if (pDevice->frontendHandle[pChannel->chn_num]) {
        NEXUS_Frontend_P_UnsetMtsifConfig(pDevice->frontendHandle[pChannel->chn_num]);
    }

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
}

static NEXUS_Error NEXUS_Frontend_P_7255_GetQamScanStatus(void *handle, NEXUS_FrontendQamScanStatus *pScanStatus)
{
    NEXUS_Error  rc;
    struct BADS_ScanStatus st;
    NEXUS_7255Device *pDevice;
    NEXUS_7255Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7255Channel *)handle;
    pDevice = (NEXUS_7255Device *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7255Device);

    BKNI_Memset(pScanStatus, 0, sizeof(*pScanStatus));

    if (pChannel->chn_num >= NEXUS_7255_MAX_DOWNSTREAM_CHANNELS)
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

static NEXUS_Error NEXUS_Frontend_P_7255_TuneQam(void *handle, const NEXUS_FrontendQamSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7255Device *pDevice;
    NEXUS_7255Channel *pChannel;
    BADS_InbandParam ibParam;
    unsigned temp_frequency;

    BDBG_ASSERT(handle);
    pChannel = (NEXUS_7255Channel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7255Device);

    if (pSettings->annex != NEXUS_FrontendQamAnnex_eA || pSettings->annex == NEXUS_FrontendQamAnnex_eC) {
        BDBG_WRN(("7255 only supports AnnexA"));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    temp_frequency = pDevice->last_ads[pChannel->chn_num].frequency;
    pDevice->last_ads[pChannel->chn_num].frequency = pSettings->frequency;

    rc = NEXUS_Frontend_P_SetMtsifConfig(pDevice->frontendHandle[pChannel->chn_num]);
    if (rc) { rc = BERR_TRACE(rc); goto done; }

    {
        BDBG_MSG(("tuning tuner"));
        if (pDevice->tnr[pChannel->chn_num])
        {
            BTNR_Settings tnrSettings;
            BTNR_PowerSaverSettings pwrSettings;

            BDBG_MSG(("have a tuner"));
            if(!pDevice->isTunerPoweredOn[pChannel->chn_num]){
                BDBG_MSG(("turning tuner on"));
                pwrSettings.enable = false;
                rc = BTNR_SetPowerSaver(pDevice->tnr[pChannel->chn_num], &pwrSettings);
                if(rc){rc = BERR_TRACE(rc); goto retrack;}

                pDevice->isTunerPoweredOn[pChannel->chn_num] = true;
            }

            rc = BTNR_GetSettings(pDevice->tnr[pChannel->chn_num], &tnrSettings);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            tnrSettings.bandwidth = pSettings->bandwidth;
            if (pSettings->symbolRate > 5214000 && tnrSettings.bandwidth < NEXUS_FrontendQamBandwidth_e8Mhz) {
                BDBG_WRN(("Tuner bandwidth too low for requested symbolrate, increasing to 8MHz"));
                tnrSettings.bandwidth = NEXUS_FrontendQamBandwidth_e8Mhz;
            }
            tnrSettings.std = BTNR_Standard_eQam;

            rc = BTNR_SetSettings(pDevice->tnr[pChannel->chn_num], &tnrSettings);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            pDevice->count[pChannel->chn_num] = 0;
            pDevice->acquireInProgress[pChannel->chn_num] = true;
            BDBG_MSG(("setting tuner rf frequency"));
            rc = BTNR_SetTunerRfFreq(pDevice->tnr[pChannel->chn_num], pSettings->frequency, BTNR_TunerMode_eDigital);
            if(rc){rc = BERR_TRACE(rc); goto retrack;}

            pDevice->last_ads[pChannel->chn_num] = *pSettings;
        }
    }

    rc = BADS_DisablePowerSaver(pDevice->ads_chn[pChannel->chn_num]);
    if(rc){rc = BERR_TRACE(rc); goto done;}
    pDevice->isPoweredOn[pChannel->chn_num] = true;

    rc = BADS_GetDefaultAcquireParams(pDevice->ads_chn[pChannel->chn_num], &ibParam);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    BDBG_MSG(("AnnexA"));
    switch (pSettings->mode) {
    case NEXUS_FrontendQamMode_e16:
        ibParam.modType = BADS_ModulationType_eAnnexAQam16; break;
    case NEXUS_FrontendQamMode_e32:
        ibParam.modType = BADS_ModulationType_eAnnexAQam32; break;
    case NEXUS_FrontendQamMode_e64:
        ibParam.modType = BADS_ModulationType_eAnnexAQam64; break;
    case NEXUS_FrontendQamMode_e128:
        ibParam.modType = BADS_ModulationType_eAnnexAQam128; break;
    case NEXUS_FrontendQamMode_e256:
        ibParam.modType = BADS_ModulationType_eAnnexAQam256; break;
    case NEXUS_FrontendQamMode_e1024:
        ibParam.modType = BADS_ModulationType_eAnnexAQam1024; break;
    default:
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
        break;
    }
    if( pSettings->annex == NEXUS_FrontendQamAnnex_eC ){
        if( (pSettings->symbolRate != 5274000) && (pSettings->symbolRate != 5307000))
            BDBG_WRN((" Symbol rate for Annex-C is incorrect."));
    }
    ibParam.enableNullPackets = pSettings->enableNullPackets;

    NEXUS_IsrCallback_Set(pDevice->lockAppCallback[pChannel->chn_num], &(pSettings->lockCallback));
    NEXUS_IsrCallback_Set(pDevice->asyncStatusAppCallback[pChannel->chn_num], &(pSettings->asyncStatusReadyCallback));

    /* Scan Parameters */
    if((pSettings->acquisitionMode == NEXUS_FrontendQamAcquisitionMode_eScan) || (pSettings->acquisitionMode == NEXUS_FrontendQamAcquisitionMode_eAuto)){
        BADS_ChannelScanSettings scanParam;
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
        if(pSettings->scan.mode[NEXUS_FrontendQamAnnex_eC][NEXUS_FrontendQamMode_e64]) scanParam.A64 = true;
        if(pSettings->scan.mode[NEXUS_FrontendQamAnnex_eC][NEXUS_FrontendQamMode_e256]) scanParam.A256 = true;
        if(pSettings->scan.frequencyOffset){
            scanParam.CO = true;
            scanParam.carrierSearch = pSettings->scan.frequencyOffset/256;
        }
        scanParam.upperBaudSearch = pSettings->scan.upperBaudSearch;
        scanParam.lowerBaudSearch = pSettings->scan.lowerBaudSearch;

        rc = BADS_SetScanParam(pDevice->ads_chn[pChannel->chn_num], &scanParam );
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    if(pSettings->acquisitionMode == NEXUS_FrontendQamAcquisitionMode_eScan) {
        ibParam.tuneAcquire = true;
    }
    else{
        ibParam.tuneAcquire = false;
    }

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

retrack:
    pDevice->last_ads[pChannel->chn_num].frequency = temp_frequency;
    return rc;
}

#if 0
static void NEXUS_FrontendDevice_P7255_GetWakeupFilter(NEXUS_FrontendDeviceHandle handle, NEXUS_TransportWakeupSettings *pSettings)
{
    NEXUS_7255Device *pDevice = handle->pDevice;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7255Device);

    *pSettings = pDevice->wakeupSettings;

    return;
}

static NEXUS_Error NEXUS_FrontendDevice_P7255_SetWakeupFilter(NEXUS_FrontendDeviceHandle handle, const NEXUS_TransportWakeupSettings *pSettings)
{
    BERR_Code rc;
    NEXUS_7255Device *pDevice = handle->pDevice;
    BMXT_Wakeup_Settings wakeupSettings;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7255Device);

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
            if (rc) { BKNI_Free(Filter); return BERR_TRACE(rc); }
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
#endif

static void NEXUS_Frontend_P_Uninit7255(NEXUS_7255Device *pDevice, bool uninitHab);

void NEXUS_FrontendDevice_P_Close7255(void *handle)
{
    NEXUS_7255Device *pDevice = (NEXUS_7255Device *)handle;

    if (pDevice) {
        NEXUS_Frontend_P_Uninit7255(pDevice, true);
        if (pDevice->pGenericDeviceHandle)
            BKNI_Free(pDevice->pGenericDeviceHandle);
        BKNI_Memset(pDevice, 0, sizeof(*pDevice));
        BKNI_Free(pDevice);
    }
}

static NEXUS_Error NEXUS_FrontendDevice_P_7255_Standby(void *handle, const NEXUS_StandbySettings *pSettings);

NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_P_7255_Alloc(unsigned index, const NEXUS_FrontendDeviceOpenSettings *pSettings)
{
    NEXUS_FrontendDevice *pFrontendDevice = NULL;
    NEXUS_7255Device *pDevice = NULL;

#if 0
    /* TODO: handle multiple devices */
    for ( pDevice = BLST_S_FIRST(&g_deviceList); NULL != pDevice; pDevice = BLST_S_NEXT(pDevice, node) )
    {
        break;
    }
#endif

    if ( NULL == pDevice)
    {
        pFrontendDevice = NEXUS_FrontendDevice_P_Create();
        if (NULL == pFrontendDevice) { BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err; }

        pDevice = BKNI_Malloc(sizeof(*pDevice));
        if (NULL == pDevice) { BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err; }
        BKNI_Memset(pDevice, 0, sizeof(*pDevice));
        BDBG_OBJECT_SET(pDevice, NEXUS_7255Device);
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
    NEXUS_FrontendDevice_P_Close7255(pFrontendDevice);
    return NULL;
}

NEXUS_Error NEXUS_FrontendDevice_P_7255_PreInitAp(NEXUS_FrontendDeviceHandle pFrontendDevice, bool initHab)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned int i;
    NEXUS_7255Device *pDevice = NULL;
    NEXUS_FrontendDeviceOpenSettings *pSettings = NULL;

    BERR_Code errCode;
    BHAB_Settings habSettings;
    BHAB_Handle habHandle;
    void *regHandle;

    BDBG_ASSERT(pFrontendDevice);

    pDevice = (NEXUS_7255Device *)pFrontendDevice->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7255Device);

    NEXUS_FrontendDevice_GetDefaultSettings(&pDevice->deviceSettings);

    pSettings = &pDevice->openSettings;

    BHAB_7255_GetDefaultSettings(&habSettings);

    habSettings.isMtsif = true;

    if (initHab) {
        regHandle = (void *)g_pCoreHandles->reg;
        habSettings.pChp = g_pCoreHandles->chp;
        if (pSettings->interruptMode != NEXUS_FrontendInterruptMode_ePolling) {
            habSettings.interruptEnableFunc = NEXUS_Frontend_P_7255_L1IsrControl_isr;
            habSettings.interruptEnableFuncParam = (void*)pDevice;
        }
        BDBG_MSG(("Calling BHAB_Open"));
        BDBG_MSG(("Calling BHAB_Open(%p,%p,%p)",(void *)&habHandle, (void *)regHandle, (void *)&habSettings));
        errCode = BHAB_Open(&habHandle, regHandle, &habSettings);
        BDBG_MSG(("Calling BHAB_Open...Done: hab: %p",(void *)habHandle));
        if (errCode) { BERR_TRACE(errCode); goto err; }

        pDevice->hab = habHandle;
    }

    errCode = BHAB_GetInterruptEventHandle(pDevice->hab, &pDevice->isrEvent);
    if(errCode) { errCode = BERR_TRACE(errCode); goto err; }

    pDevice->isrEventCallback = NEXUS_RegisterEvent(pDevice->isrEvent, NEXUS_Frontend_P_7255_IsrEvent, (void *)pDevice);
    if ( NULL == pDevice->isrEventCallback ) { errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err; }

    if (pSettings->interruptMode == NEXUS_FrontendInterruptMode_ePolling) {
        NEXUS_ThreadSettings thread_settings;

        NEXUS_Thread_GetDefaultSettings(&thread_settings);
        thread_settings.priority = 0;
        if((pDevice->pollingThread == NULL) /*&& (pDevice->openSettings.interruptMode == NEXUS_FrontendInterruptMode_ePolling)*/){
            pDevice->pollingThread = NEXUS_Thread_Create("pollingThread",
                                                    NEXUS_Frontend_7255DevicePollingThread,
                                                    (void*)pDevice,
                                                    &thread_settings);
        }
    } else {
        rc = BINT_CreateCallback(&(pDevice->irqCallback), g_pCoreHandles->bint, BLEAP_HOST_L1_INTERRUPT_ID, NEXUS_Frontend_P_7255_L1_isr, (void *)pDevice, 0);
        BDBG_ASSERT(rc == BERR_SUCCESS);

         rc = BINT_EnableCallback(pDevice->irqCallback);
        BDBG_ASSERT(rc == BERR_SUCCESS);
    }

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

    /* Open tuners */
    {
        BTNR_7255_Settings tnrIb7255_cfg;
        unsigned i;

        if (pSettings->tuner.i2c.device){
            pDevice->i2cRegHandle = NEXUS_I2c_GetRegHandle(pSettings->tuner.i2c.device , NEXUS_MODULE_SELF);
            if(pDevice->i2cRegHandle == NULL ){rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto done;}
        }

        for (i=0; i < pDevice->capabilities.totalTunerChannels; i++) {
            BDBG_MSG(("Opening tuner %d", i));
            BTNR_7255_GetDefaultSettings(&tnrIb7255_cfg);
            tnrIb7255_cfg.channelNo = i;
            tnrIb7255_cfg.i2cRegHandle = pDevice->i2cRegHandle;
            tnrIb7255_cfg.i2cAddr = pSettings->tuner.i2c.address;
            tnrIb7255_cfg.regHandle = (void *)g_pCoreHandles->reg;
            rc =  BTNR_7255_Open(&pDevice->tnr[i],&tnrIb7255_cfg, pDevice->hab);
            BDBG_MSG(("tuner %d is %p", i, (void *)pDevice->tnr[i]));
        }
    }

    BDBG_MSG(("pDevice->capabilities.totalTunerChannels: %d", pDevice->capabilities.totalTunerChannels));
    for(i=0; i<pDevice->capabilities.totalTunerChannels; i++){
        if(pDevice->capabilities.channelCapabilities[i].demodCoreType.ads) pDevice->adsPresent = true;
        if(pDevice->capabilities.channelCapabilities[i].demodCoreType.aob) pDevice->aobPresent = true;
        if(pDevice->capabilities.channelCapabilities[i].demodCoreType.ifdac) pDevice->ifDacPresent = true;

        if(pDevice->capabilities.channelCapabilities[i].demodCoreType.ifdac) { pDevice->ifDacChannelNumber = i; BDBG_MSG(("channel %d is ifdac", i)); BDBG_WRN(("Unexpected ifdac support")); }
        if(pDevice->capabilities.channelCapabilities[i].demodCoreType.aob) { pDevice->oobChannelNumber = i; BDBG_MSG(("channel %d is oob", i)); BDBG_WRN(("Unexpected oob support")); }
    }

    BDBG_MSG(("Filling out API tree"));
    pFrontendDevice->getCapabilities = NEXUS_FrontendDevice_P_7255_GetCapabilities;
    pFrontendDevice->getTunerCapabilities = NEXUS_FrontendDevice_P_7255_GetTunerCapabilities;
#if 0
    pFrontendDevice->recalibrate = NEXUS_FrontendDevice_P_7255_Recalibrate;
#endif
    pFrontendDevice->getStatus = NEXUS_FrontendDevice_P_7255_GetStatus;
#if 0
    pFrontendDevice->getInternalGain = NEXUS_FrontendDevice_P_7255_GetInternalGain;
    pFrontendDevice->getExternalGain = NEXUS_FrontendDevice_P_7255_GetExternalGain;
    pFrontendDevice->setExternalGain = NEXUS_FrontendDevice_P_7255_SetExternalGain;
#endif
    pFrontendDevice->getSettings = NEXUS_FrontendDevice_P_7255_GetSettings;
    pFrontendDevice->setSettings = NEXUS_FrontendDevice_P_7255_SetSettings;
    pFrontendDevice->getDeviceAmplifierStatus = NEXUS_FrontendDevice_P_7255_GetDeviceAmplifierStatus;

#if 0
    pFrontendDevice->getWakeupSettings = NEXUS_FrontendDevice_P7255_GetWakeupFilter;
    pFrontendDevice->setWakeupSettings = NEXUS_FrontendDevice_P7255_SetWakeupFilter;
#endif

    pDevice->chipFamilyId = 0x7255;
    pFrontendDevice->familyId = 0x7255;
    pFrontendDevice->close = NEXUS_FrontendDevice_P_Close7255;

    pFrontendDevice->mode = NEXUS_StandbyMode_eOn;
    pFrontendDevice->standby = NEXUS_FrontendDevice_P_7255_Standby;

    BDBG_MSG(("Finished creating frontend device %p", (void *)pFrontendDevice));
    return NEXUS_SUCCESS;
done:
    return rc;
err:
    NEXUS_FrontendDevice_P_Close7255(pFrontendDevice);
    return NEXUS_UNKNOWN;
}

NEXUS_Error NEXUS_FrontendDevice_P_7255_InitAp(NEXUS_FrontendDeviceHandle device, bool initHab)
{
    NEXUS_7255Device *pDevice = (NEXUS_7255Device *)device->pDevice;
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
            NEXUS_MemoryAllocationSettings allocSettings;

            rc = Nexus_Core_P_Img_Create(NEXUS_CORE_IMG_ID_FRONTEND_7255, &pImgContext, &imgInterface);
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
        fw_image = bcm7255_leap_image;
#endif

        {
            unsigned magic;
            unsigned family;
            unsigned chip;
            unsigned rev;

            magic  = (fw_image[ 0] << 24) + (fw_image[ 1] << 16) + (fw_image[ 2] << 8) + (fw_image[ 3]);
            family = (fw_image[36] << 24) + (fw_image[37] << 16) + (fw_image[38] << 8) + (fw_image[39]);
            chip   = (fw_image[40] << 24) + (fw_image[41] << 16) + (fw_image[42] << 8) + (fw_image[43]);
            rev    = (fw_image[44] << 24) + (fw_image[45] << 16) + (fw_image[46] << 8) + (fw_image[47]);

            if ((magic != 0xaaaaaaaa) || (family != 0x7255) || (chip != 0x7255)) {
                BDBG_WRN(("Possible 7255 firmware corruption, expected values not matched."));
            }
            BSTD_UNUSED(rev);
        }

        {
            NEXUS_MemoryAllocationSettings memSettings;
            BHAB_7255_Settings hab7255Settings;

            BDBG_MSG(("Configuring 7255 with external memory"));
            NEXUS_Memory_GetDefaultAllocationSettings(&memSettings);
            memSettings.alignment = 0x100000;
            rc = NEXUS_Memory_Allocate(0x100000,&memSettings,&pDevice->leapBuffer);
            if(rc){BDBG_ERR(("Unable to allocate memory for 7255 LEAP"));rc = BERR_TRACE(rc); goto done;}

            hab7255Settings.bUseInternalMemory = false;
            hab7255Settings.pRam = pDevice->leapBuffer;
            hab7255Settings.physAddr = NEXUS_AddrToOffset(pDevice->leapBuffer);
            hab7255Settings.heap = NEXUS_Core_P_AddressToHeap(pDevice->leapBuffer, NULL);

            BDBG_MSG(("pRam: %p, physAddr: 0x%08x",hab7255Settings.pRam,hab7255Settings.physAddr));
            rc = BHAB_7255_Configure(pDevice->hab, &hab7255Settings);
            BDBG_MSG(("...configured"));
            if(rc){rc = BERR_TRACE(rc); goto done;}
        }

        BDBG_MSG(("BHAB_InitAp"));
        rc = BHAB_InitAp(pDevice->hab, fw_image);
        BDBG_MSG(("...done BHAB_InitAp"));

done:
#if NEXUS_MODE_driver
        NEXUS_Memory_Free(fw);
#else
        BSTD_UNUSED(fw);
#endif
    }

    return rc;
}

NEXUS_Error NEXUS_FrontendDevice_P_7255_PostInitAp(NEXUS_FrontendDeviceHandle device)
{
    NEXUS_7255Device *pDevice = (NEXUS_7255Device *)device->pDevice;
    BERR_Code errCode;
    BADS_Settings adsConfig;
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned totalChannels = 0;
    unsigned i;

    /* Open ADS */
    errCode = BADS_7255_GetDefaultSettings( &adsConfig, NULL);
    if (errCode) { errCode = BERR_TRACE(errCode); goto done; }

    adsConfig.hGeneric = pDevice->hab;
    adsConfig.transportConfig = BADS_TransportData_eSerial;
    BDBG_MSG(("BADS_Open"));
    errCode = BADS_Open(&pDevice->ads, NULL, NULL, NULL, &adsConfig);
    if (errCode) { errCode = BERR_TRACE(errCode); goto done; }
    BDBG_MSG(("BADS_Init"));
    errCode = BADS_Init(pDevice->ads);
    if (errCode) { errCode = BERR_TRACE(errCode); goto done; }

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

        errCode = BADS_InstallCallback(pDevice->ads_chn[i], BADS_Callback_eLockChange, (BADS_CallbackFunc)NEXUS_Frontend_P_7255_callback_isr, (void*)&pDevice->frontendHandle[i]);
        if (errCode) { errCode = BERR_TRACE(rc); goto done; }
        errCode = BADS_InstallCallback(pDevice->ads_chn[i], BADS_Callback_eNoSignal, (BADS_CallbackFunc)NEXUS_Frontend_P_7255_callback_isr, (void*)&pDevice->frontendHandle[i]);
        if (errCode) { errCode = BERR_TRACE(rc); goto done; }
        errCode = BADS_InstallCallback(pDevice->ads_chn[i], BADS_Callback_eAsyncStatusReady, (BADS_CallbackFunc)NEXUS_Frontend_P_7255_AsyncStatusCallback_isr, (void*)&pDevice->frontendHandle[i]);
        if (errCode) { errCode = BERR_TRACE(rc); goto done; }
#if 0
        /* TODO */
        errCode = BADS_InstallCallback(pDevice->ads_chn[adsChannels], BADS_Callback_eUpdateGain, (BADS_CallbackFunc)NEXUS_Frontend_P_7255_updateGainCallback_isr, (void*)pDevice->updateGainAppCallback);
        if (errCode) { errCode = BERR_TRACE(rc); goto done; }
#endif
    }

#if 0
    pDevice->cppmAppCallback = NEXUS_IsrCallback_Create(pDevice, NULL);
    if ( NULL == pDevice->cppmAppCallback ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto done;}

    pDevice->cppmDoneCallback = NEXUS_IsrCallback_Create(pDevice, NULL);
    if ( NULL == pDevice->cppmDoneCallback ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto done;}

    BHAB_InstallInterruptCallback(pDevice->hab, BHAB_DevId_eGlobal,
                                (BHAB_IntCallbackFunc)NEXUS_Frontend_P_7255_cppm_callback_isr,
                                (void *)pDevice,
                                BHAB_Interrupt_eCppmPowerLevelChange);

    BHAB_InstallInterruptCallback(pDevice->hab, BHAB_DevId_eGlobal,
                                (BHAB_IntCallbackFunc)NEXUS_Frontend_P_7255_cppm_complete_callback_isr,
                                (void *)pDevice,
                                BHAB_Interrupt_eCalibrationComplete);
#endif

done:
    return rc;
}

static NEXUS_Error NEXUS_FrontendDevice_P_Init7255(NEXUS_7255Device *pDevice, bool initHab)
{
    NEXUS_Error errCode;

    BDBG_MSG(("NEXUS_FrontendDevice_P_Init7255: NEXUS_FrontendDevice_P_7255_PreInitAp"));
    errCode = NEXUS_FrontendDevice_P_7255_PreInitAp(pDevice->pGenericDeviceHandle, initHab);
    if (!errCode) {
        BDBG_MSG(("NEXUS_FrontendDevice_P_Init7255: NEXUS_FrontendDevice_P_7255_InitAp"));
        errCode = NEXUS_FrontendDevice_P_7255_InitAp(pDevice->pGenericDeviceHandle, initHab);
    } else BERR_TRACE(errCode);
    if (!errCode) {
        BDBG_MSG(("NEXUS_FrontendDevice_P_Init7255: NEXUS_FrontendDevice_P_7255_PostInitAp"));
        errCode = NEXUS_FrontendDevice_P_7255_PostInitAp(pDevice->pGenericDeviceHandle);
    } else BERR_TRACE(errCode);

#if 0
    /* Hook in case of post-async-firmware requirements */
    if (!errCode)
        errCode = NEXUS_Frontend_7255_P_DelayedInitialization(pDevice->pGenericDeviceHandle);
#endif

    BDBG_MSG(("NEXUS_FrontendDevice_P_Init7255: returning %d", errCode));
    return errCode;
}

static void NEXUS_Frontend_P_Uninit7255(NEXUS_7255Device *pDevice, bool uninitHab)
{
    unsigned i;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7255Device);

    BDBG_MSG(("Closing 7255 device %p handles", (void *)pDevice));

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
    if(pDevice->cppmDoneCallback) {
        NEXUS_IsrCallback_Destroy(pDevice->cppmDoneCallback);
        pDevice->cppmDoneCallback = NULL;
    }
    if(pDevice->cppmAppCallback) {
        NEXUS_IsrCallback_Destroy(pDevice->cppmAppCallback);
        pDevice->cppmAppCallback = NULL;
    }
    if (pDevice->ifDacHandle) {
        if(pDevice->ifDacAppCallback){
            NEXUS_IsrCallback_Destroy(pDevice->ifDacAppCallback);
            pDevice->ifDacAppCallback = NULL;
        }
        if(pDevice->ifDacAsyncStatusAppCallback){
            NEXUS_IsrCallback_Destroy(pDevice->ifDacAsyncStatusAppCallback);
            pDevice->ifDacAsyncStatusAppCallback = NULL;
        }
        NEXUS_Tuner_Close(pDevice->ifDacHandle);
    }
    for (i=0; i < pDevice->capabilities.totalTunerChannels; i++) {
        if (pDevice->tnr[i]) {
            BTNR_Close(pDevice->tnr[i]);
            pDevice->tnr[i] = NULL;
        }
    }
    if (pDevice->ads) {
        BADS_Close(pDevice->ads);
        pDevice->ads = NULL;
    }
    if (pDevice->isrEventCallback) {
        NEXUS_UnregisterEvent(pDevice->isrEventCallback);
        pDevice->isrEventCallback = NULL;
    }

    if (pDevice->openSettings.isrNumber) {
        NEXUS_Core_DisconnectInterrupt(pDevice->openSettings.isrNumber);
    } else if (pDevice->openSettings.gpioInterrupt) {
        NEXUS_GpioSettings gpioSettings;
        NEXUS_Gpio_SetInterruptCallback_priv(pDevice->openSettings.gpioInterrupt, NULL, NULL, 0);
        NEXUS_Gpio_GetSettings(pDevice->openSettings.gpioInterrupt, &gpioSettings);
        gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
        NEXUS_Gpio_SetSettings(pDevice->openSettings.gpioInterrupt, &gpioSettings);
    }

    if(pDevice->pollingThread){
        NEXUS_Thread_Destroy(pDevice->pollingThread);
        pDevice->pollingThread = NULL;
    }
    if (uninitHab && pDevice->hab) {
        BHAB_Close(pDevice->hab);
        pDevice->hab = NULL;
    }
    if (pDevice->leapBuffer) {
        NEXUS_Memory_Free(pDevice->leapBuffer);
        pDevice->leapBuffer = NULL;
    }
    if (pDevice->capabilities.channelCapabilities) {
        BKNI_Free(pDevice->capabilities.channelCapabilities);
        pDevice->capabilities.channelCapabilities = NULL;
    }
}

void NEXUS_FrontendDevice_P_7255_S3Standby(NEXUS_7255Device *pDevice, bool uninitHab)
{
    NEXUS_Frontend_P_Uninit7255(pDevice, uninitHab);
}

static NEXUS_Error NEXUS_Frontend_P_7255_RestoreCallbacks(NEXUS_7255Device *pDevice) {
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned i;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7255Device);

    for (i=0; i < pDevice->numChannels; i++) {
        NEXUS_IsrCallbackHandle lockCallback;
        NEXUS_IsrCallbackHandle qamAsyncStatusReadyCallback = NULL;
        NEXUS_FrontendHandle frontendHandle = pDevice->frontendHandle[i];

        if (frontendHandle) {
            /* lock callback */
            lockCallback = NEXUS_IsrCallback_Create(frontendHandle, NULL);
            if ( NULL == lockCallback ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err;}
            pDevice->lockAppCallback[i] = lockCallback;

            qamAsyncStatusReadyCallback = NEXUS_IsrCallback_Create(frontendHandle, NULL);
            if ( NULL == qamAsyncStatusReadyCallback ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err;}

            pDevice->asyncStatusAppCallback[i] = qamAsyncStatusReadyCallback;
        }
    }
    return rc;
err:
    BERR_TRACE(rc);
    return rc;
}

static NEXUS_Error NEXUS_FrontendDevice_P_7255_Standby(void *handle, const NEXUS_StandbySettings *pSettings)
{

    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7255Device *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7255Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7255Device);

    /* entering S3 */
    if ((pDevice->pGenericDeviceHandle->mode != NEXUS_StandbyMode_eDeepSleep) && (pSettings->mode == NEXUS_StandbyMode_eDeepSleep)) {

        BDBG_MSG(("NEXUS_FrontendDevice_P_7255_Standby: Entering deep sleep..."));

        NEXUS_FrontendDevice_P_7255_S3Standby(pDevice, !pDevice->wakeupSettings.enabled);

        BKNI_EnterCriticalSection();
        pDevice->pGenericDeviceHandle->openPending = true;
        pDevice->pGenericDeviceHandle->openFailed = false;
        BKNI_LeaveCriticalSection();

    } else /* waking from S3 */
        if ((pDevice->pGenericDeviceHandle->mode == NEXUS_StandbyMode_eDeepSleep) && (pSettings->mode != NEXUS_StandbyMode_eDeepSleep)) {

        BDBG_MSG(("NEXUS_FrontendDevice_P_7255_Standby: Waking up..."));

        BDBG_MSG(("NEXUS_FrontendDevice_P_7255_Standby: reinitializing..."));
        rc = NEXUS_FrontendDevice_P_Init7255(pDevice, !pDevice->wakeupSettings.enabled);
        if (rc) { rc = BERR_TRACE(rc); goto done;}

        NEXUS_Frontend_P_7255_RestoreCallbacks(pDevice);

        BKNI_EnterCriticalSection();
        pDevice->pGenericDeviceHandle->openPending = false;
        pDevice->pGenericDeviceHandle->openFailed = false;
        BKNI_LeaveCriticalSection();
    }

    /* entering S2 */
    if ((pDevice->pGenericDeviceHandle->mode != NEXUS_StandbyMode_ePassive) && (pSettings->mode == NEXUS_StandbyMode_ePassive)) {
        BDBG_MSG(("NEXUS_FrontendDevice_P_7255_Standby: Entering S2..."));

    } else /* waking from S2 */
        if ((pDevice->pGenericDeviceHandle->mode == NEXUS_StandbyMode_ePassive) && (pSettings->mode != NEXUS_StandbyMode_ePassive)) {

            BDBG_MSG(("NEXUS_FrontendDevice_P_7255_Standby: Waking up from S2..."));

    }

done:
    BDBG_MSG(("NEXUS_FrontendDevice_P_7255_Standby(%d->%d): returning %d...", pDevice->pGenericDeviceHandle->mode, pSettings->mode, rc));
    return rc;
}

NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_P_Open7255(unsigned index, const NEXUS_FrontendDeviceOpenSettings *pSettings)
{
    NEXUS_FrontendDevice *device;
    NEXUS_Error rc;

    BDBG_ASSERT(pSettings);

    device = NEXUS_FrontendDevice_P_7255_Alloc(index, pSettings);
    if (!device) goto done;

    rc = NEXUS_FrontendDevice_P_7255_PreInitAp(device, true);
    if (rc) {
        NEXUS_FrontendDevice_P_Close7255(device);
        device = NULL; goto done;
    }

    rc = NEXUS_FrontendDevice_P_7255_InitAp(device, true);
    if (rc) {
        NEXUS_FrontendDevice_P_Close7255(device);
        device = NULL; goto done;
    }
    rc = NEXUS_FrontendDevice_P_7255_PostInitAp(device);
    if (rc) {
        NEXUS_FrontendDevice_P_Close7255(device);
        device = NULL; goto done;
    }

done:
    return device;
}

static void NEXUS_Frontend_P_7255_Close(NEXUS_FrontendHandle handle)
{
    NEXUS_7255Device *pDevice;
    NEXUS_7255Channel *pChannel;

    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    pChannel =(NEXUS_7255Channel *)handle->pDeviceHandle;
    BDBG_ASSERT(NULL != pChannel);
    pDevice = (NEXUS_7255Device *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7255Device);

    NEXUS_Frontend_P_Destroy(handle);
    pDevice->frontendHandle[pChannel->chn_num] = NULL;
    BKNI_Memset(pChannel, 0, sizeof(*pChannel));
    BKNI_Free(pChannel);

    return;
}

static void NEXUS_Frontend_P_7255_UninstallCallbacks(void *handle)
{
    NEXUS_7255Device *pDevice;
    NEXUS_7255Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7255Channel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7255Device);

    if (pChannel->chn_num >= NEXUS_MAX_7255_FRONTENDS)
    {
        BDBG_ERR((" Unsupported channel."));
        BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    if (pDevice->lockAppCallback[pChannel->chn_num])
        NEXUS_IsrCallback_Set(pDevice->lockAppCallback[pChannel->chn_num], NULL);
    if (pDevice->asyncStatusAppCallback[pChannel->chn_num])
        NEXUS_IsrCallback_Set(pDevice->asyncStatusAppCallback[pChannel->chn_num], NULL);

done:
    return;
}

static NEXUS_Error NEXUS_Frontend_P_7255_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7255Device *pDevice;
    NEXUS_7255Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_7255Channel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7255Device);

    BSTD_UNUSED(enabled);
    BSTD_UNUSED(pSettings);

    return rc;
}

NEXUS_FrontendHandle NEXUS_Frontend_P_Open7255(const NEXUS_FrontendChannelSettings *pSettings)
{
    NEXUS_FrontendDeviceHandle pFrontendDevice;
    NEXUS_7255Device *p7255Device;
    NEXUS_7255Channel *pChannel;
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
    p7255Device = (NEXUS_7255Device *)pFrontendDevice->pDevice;

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

    /* If already opened, return the previously opened handle */
    if ( p7255Device->frontendHandle[channelNumber] != NULL )
    {
        return p7255Device->frontendHandle[channelNumber];
    }

    pChannel = (NEXUS_7255Channel*)BKNI_Malloc(sizeof(*pChannel));
    if ( NULL == pChannel ) { BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err;}
    BKNI_Memset(pChannel, 0, sizeof(*pChannel));

    /* Create a Nexus frontend handle */
    frontendHandle = NEXUS_Frontend_P_Create(pChannel);
    if ( NULL == frontendHandle ) { BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err;}

    /* Set capabilities and channel type parameters */
    frontendHandle->capabilities.qam = true;
    frontendHandle->capabilities.outOfBand = false;
    frontendHandle->capabilities.upstream = false;
    BKNI_Memset(frontendHandle->capabilities.qamModes, true, sizeof(frontendHandle->capabilities.qamModes));

    /* bind functions*/
    frontendHandle->tuneQam = NEXUS_Frontend_P_7255_TuneQam;
    frontendHandle->untune = NEXUS_Frontend_P_7255_UnTuneQam;
    frontendHandle->getQamStatus = NEXUS_Frontend_P_7255_GetQamStatus;
    frontendHandle->requestQamAsyncStatus = NEXUS_Frontend_P_7255_RequestQamAsyncStatus;
    frontendHandle->getQamAsyncStatus = NEXUS_Frontend_P_7255_GetQamAsyncStatus;
    frontendHandle->resetStatus = NEXUS_Frontend_P_7255_ResetQamStatus;
    frontendHandle->getQamScanStatus = NEXUS_Frontend_P_7255_GetQamScanStatus;

    frontendHandle->readSoftDecisions = NEXUS_Frontend_P_7255_ReadSoftDecisions;
    frontendHandle->reapplyTransportSettings = NEXUS_Frontend_P_7255_ReapplyTransportSettings;
    frontendHandle->close = NEXUS_Frontend_P_7255_Close;
    frontendHandle->getFastStatus = NEXUS_Frontend_P_7255_GetFastStatus;
    frontendHandle->writeRegister = NEXUS_Frontend_P_7255_WriteRegister;
    frontendHandle->readRegister = NEXUS_Frontend_P_7255_ReadRegister;
    frontendHandle->getType = NEXUS_Frontend_P_7255_GetType;
    frontendHandle->standby = NEXUS_Frontend_P_7255_Standby;
    frontendHandle->uninstallCallbacks = NEXUS_Frontend_P_7255_UninstallCallbacks;

    {
        NEXUS_IsrCallbackHandle lockCallback;
        /* lock callback */
        lockCallback = NEXUS_IsrCallback_Create(frontendHandle, NULL);
        if ( NULL == lockCallback ) { BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err;}
        p7255Device->lockAppCallback[channelNumber] = lockCallback;
    }

    if (type == NEXUS_FrontendChannelType_eCable)
    {
        NEXUS_IsrCallbackHandle qamAsyncStatusReadyCallback = NULL;
        qamAsyncStatusReadyCallback = NEXUS_IsrCallback_Create(frontendHandle, NULL);
        if ( NULL == qamAsyncStatusReadyCallback ) { BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err;}

        p7255Device->asyncStatusAppCallback[channelNumber] = qamAsyncStatusReadyCallback;
    }
    else if(type == NEXUS_FrontendChannelType_eCableOutOfBand)
    {
        NEXUS_IsrCallbackHandle oobAsyncStatusReadyCallback = NULL;
        oobAsyncStatusReadyCallback = NEXUS_IsrCallback_Create(frontendHandle, NULL);
        if ( NULL == oobAsyncStatusReadyCallback ) { BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err;}

        p7255Device->asyncStatusAppCallback[channelNumber] = oobAsyncStatusReadyCallback;
        channelNumber = NEXUS_7255_OOB_CHANNEL;
    }

    frontendHandle->pGenericDeviceHandle = pFrontendDevice;

    p7255Device->frontendHandle[channelNumber] = frontendHandle;
    /* save channel number in pChannel*/
    pChannel->chn_num = channelNumber;
    pChannel->pDevice = p7255Device;
#if 0
    /* TODO: Fill out family/chip ID correctly */
    frontendHandle->chip.familyId = pDevice->chipFamilyId;
    frontendHandle->chip.id = pDevice->chipId;
#else
    frontendHandle->chip.familyId = 0x7255;
    frontendHandle->chip.id = 0x7255;
#endif

    frontendHandle->userParameters.isMtsif = false;
    frontendHandle->userParameters.param1 = 4;

    return frontendHandle;

err:
    if (pChannel) BKNI_Free(pChannel);
    return NULL;
}
