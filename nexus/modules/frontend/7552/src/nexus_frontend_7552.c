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
* Module Description:
*       The frontend_7552 module defines the common functions in signal
*   demodulation process, including opening or closing a frontend_7552 device,
*   acquire a DVB-T or ISDB-T signal, getting the status of the acquired
*   channel, setting the inband transport output interface settings, detecting
*   certain type of signal, install signal lock callback, install signal
*   unlock callback, and etc.
*
**************************************************************************/
#include "nexus_frontend_module.h"
#include "btnr_3x7x.h"
#include "bthd.h"
#include "bthd_3x7x.h"
#include "bads.h"
#include "bads_priv.h"
#include "bads_3x7x_priv.h"
#include "bads_7552.h"
#include "bchp_sun_top_ctrl.h"
#ifndef NEXUS_FRONTEND_7552_A0
#include "baob.h"
#include "btnr_ob_3x7x.h"
#endif
#include "nexus_avs.h"

BDBG_MODULE(nexus_frontend_7552);
BDBG_OBJECT_ID(NEXUS_7552);
/*BDBG_OBJECT_ID_DECLARE(NEXUS_7552);*/

typedef struct NEXUS_PreviousStatus
{
    unsigned fecCorrected;
    unsigned fecUncorrected;
    unsigned fecClean;
    unsigned viterbiUncorrectedBits;
    unsigned viterbiTotalBits;
    NEXUS_Time time;
}NEXUS_PreviousStatus;

typedef struct NEXUS_7552
{
    BDBG_OBJECT(NEXUS_7552)
    union
    {
        NEXUS_FrontendQamSettings qam;
        NEXUS_FrontendOfdmSettings ofdm;
    } lastSettings;
    enum
    {
        UNKNOWN_7552,
        QAM_7552,
        OFDM_7552
    } currentMode;
    BTNR_Handle tnrHandle;
    BTHD_Handle thdHandle;
    BADS_Handle adsHandle;
    BADS_ChannelHandle channelHandle; /* 7552 has only one channel */
    BKNI_EventHandle tnrIsrEvent;
    BKNI_EventHandle thdIsrEvent;
    BKNI_EventHandle adsIsrEvent;
    BKNI_EventHandle ewsEvent;
    BKNI_EventHandle bbsEvent;
    NEXUS_EventCallbackHandle tnrIsrEventCallback;
    NEXUS_EventCallbackHandle thdIsrEventCallback;
    NEXUS_EventCallbackHandle adsIsrEventCallback;
    NEXUS_EventCallbackHandle ewsEventCallback;
    NEXUS_EventCallbackHandle bbsEventCallback;
    NEXUS_IsrCallbackHandle thdLockAppCallback;
    NEXUS_TaskCallbackHandle thdAsyncStatusAppCallback;
    NEXUS_TaskCallbackHandle adsAsyncStatusAppCallback;
    NEXUS_IsrCallbackHandle  adsLockAppCallback;
    NEXUS_TaskCallbackHandle ewsAppCallback;
    NEXUS_IsrCallbackHandle tunerIsrCallback;
    bool isTunerPowered;
    bool isAdsPoweredOn;
    bool isThdPoweredOn;
    unsigned channelScanTimeout; /* in milliseconds */
    unsigned upperBaudSearch; /* in symbols per sec */
    unsigned lowerBaudSearch; /* in symbols per sec */
    unsigned frontendcount;
    NEXUS_7552TunerRfInput rfInput;
    bool enableRfLoopThrough;
    bool terrestrial;
    bool acquireInProgress;
    unsigned count;
    NEXUS_PreviousStatus previousQamStatus;
    NEXUS_FrontendDevice *pGenericDeviceHandle;
    BKNI_EventHandle asyncStatusEvent;
    bool   isInternalAsyncStatusCall;
    bool   isInternalAsyncStatusReady;
    bool   isAsyncStatusReady;
    NEXUS_StandbyMode previousStandbyMode;
#ifndef NEXUS_FRONTEND_7552_A0
    BAOB_Handle oobHandle;
    BTNR_Handle oobTnrHandle;
    BKNI_EventHandle oobIsrEvent;
    BKNI_EventHandle oobLockEvent;
    NEXUS_EventCallbackHandle oobTnrIsrEventCallback;
    NEXUS_EventCallbackHandle oobIsrEventCallback;
    NEXUS_IsrCallbackHandle  oobLockAppCallback;
    NEXUS_IsrCallbackHandle oobTunerIsrCallback;
    bool isOobPoweredOn;
    bool oobUntuned;
#endif
} NEXUS_7552;

static NEXUS_7552 *p_7552device = NULL;

/***************************************************************************
 * Module callback functions for tuning
 ***************************************************************************/
static void NEXUS_Frontend_P_7552_Close(NEXUS_FrontendHandle handle);
static void NEXUS_Frontend_P_7552_Untune(void *handle);
static NEXUS_Error NEXUS_Frontend_P_7552_TuneQam(void *handle, const NEXUS_FrontendQamSettings *pSettings);
static NEXUS_Error NEXUS_Frontend_P_7552_GetQamStatus(void *handle, NEXUS_FrontendQamStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_7552_ReadSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length, size_t *pNumRead);
static void NEXUS_Frontend_P_7552_ResetStatus(void *handle);
static NEXUS_Error NEXUS_Frontend_P_7552_GetFastStatus(void *handle, NEXUS_FrontendFastStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_7552_GetQamScanStatus(void *handle, NEXUS_FrontendQamScanStatus *pScanStatus);
static NEXUS_Error NEXUS_Frontend_P_7552_GetQamAsyncStatus(void *handle, NEXUS_FrontendQamStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_7552_RequestQamAsyncStatus(void *handle);

static NEXUS_Error NEXUS_Frontend_P_7552_TuneOfdm(void *handle, const NEXUS_FrontendOfdmSettings *pSettings);
static NEXUS_Error NEXUS_Frontend_P_7552_GetOfdmStatus(void *handle, NEXUS_FrontendOfdmStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_7552_GetOfdmAsyncStatus(void *handle, NEXUS_FrontendOfdmStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_7552_RequestOfdmAsyncStatus(void *handle);
static NEXUS_Error NEXUS_Frontend_P_7552_RequestIsdbtAsyncStatus(void *handle, NEXUS_FrontendIsdbtStatusType type);
static NEXUS_Error NEXUS_Frontend_P_7552_GetIsdbtAsyncStatusReady(void *handle, NEXUS_FrontendIsdbtStatusReady *pAsyncStatusReady);
static NEXUS_Error NEXUS_Frontend_P_7552_GetIsdbtAsyncStatus(void *handle, NEXUS_FrontendIsdbtStatusType type, NEXUS_FrontendIsdbtStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_7552_RequestDvbtAsyncStatus(void *handle, NEXUS_FrontendDvbtStatusType type);
static NEXUS_Error NEXUS_Frontend_P_7552_GetDvbtAsyncStatusReady(void *handle, NEXUS_FrontendDvbtStatusReady *pAsyncStatusReady);
static NEXUS_Error NEXUS_Frontend_P_7552_GetDvbtAsyncStatus(void *handle, NEXUS_FrontendDvbtStatusType type, NEXUS_FrontendDvbtStatus *pStatus);

static BTHD_IsdbtTimeInterleaving NEXUS_Frontend_P_TimeInterleavingToTHD(NEXUS_FrontendOfdmTimeInterleaving nexus);
static NEXUS_FrontendOfdmTimeInterleaving NEXUS_Frontend_P_THDToTimeInterleaving(BTHD_IsdbtTimeInterleaving magnum);
static BTHD_CodeRate NEXUS_Frontend_P_CodeRateToTHD(NEXUS_FrontendOfdmCodeRate nexusCodeRate);
static NEXUS_FrontendOfdmCodeRate NEXUS_Frontend_P_THDToCodeRate(BTHD_CodeRate magnum);
static BTHD_Modulation NEXUS_Frontend_P_ModulationToTHD(NEXUS_FrontendOfdmModulation nexus);
static NEXUS_FrontendOfdmModulation NEXUS_Frontend_P_THDToModulation(BTHD_Modulation magnum);
static NEXUS_FrontendOfdmTransmissionMode  NEXUS_Frontend_P_THDToTransmissionMode( BTHD_TransmissionMode magnum );
static NEXUS_FrontendOfdmGuardInterval NEXUS_Frontend_P_THDToGuardInterval(BTHD_GuardInterval magnum);
static NEXUS_FrontendOfdmHierarchy NEXUS_Frontend_P_THDToHierarchy(BTHD_Hierarchy magnum);


static NEXUS_Error NEXUS_Frontend_P_7552_WriteRegister(void *handle, unsigned address, uint32_t value);
static NEXUS_Error NEXUS_Frontend_P_7552_ReadRegister(void *handle, unsigned address, uint32_t *value   );
static NEXUS_Error NEXUS_Frontend_P_7552_WriteRegister(void *handle, unsigned address, uint32_t value);
static NEXUS_Error NEXUS_Frontend_P_7552_ReadRegister(void *handle, unsigned address, uint32_t *value   );
static NEXUS_Error NEXUS_Frontend_P_7552_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings);

static NEXUS_Error NEXUS_FrontendDevice_P_7552_Standby(void * handle, const NEXUS_StandbySettings *pSettings);
static NEXUS_Error NEXUS_FrontendDevice_P_7552_GetStatus(void * handle, NEXUS_FrontendDeviceStatus *pStatus);
NEXUS_Error NEXUS_FrontendDevice_P_7552_GetInternalGain(void *handle, const NEXUS_GainParameters *params, NEXUS_InternalGainSettings *pSettings);
NEXUS_Error NEXUS_FrontendDevice_P_7552_GetExternalGain(void *handle, NEXUS_ExternalGainSettings *pSettings);
NEXUS_Error NEXUS_FrontendDevice_P_7552_SetExternalGain(void *handle, const NEXUS_ExternalGainSettings *pSettings);

static void NEXUS_FrontendDevice_P_7552_Close(void *handle);

#ifndef NEXUS_FRONTEND_7552_A0
static NEXUS_Error NEXUS_Frontend_P_7552_TuneOob(void *handle, const NEXUS_FrontendOutOfBandSettings *pSettings);
#endif
/***************************************************************************
Summary:
    Lock callback handler for a 7552 Inband device
 ***************************************************************************/
static void NEXUS_Frontend_P_TnrIsrBBSEvent(void *pParam)
{
    NEXUS_7552 *pDevice = pParam;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BDBG_MSG(("Calling BTNR_3x7x_ProcessInterruptEvent"));
    BTNR_3x7x_ProcessInterruptEvent(pDevice->tnrHandle);
}
static void NEXUS_Frontend_P_7552_ThdIsrEvent(void *pParam)
{
    NEXUS_7552 *pDevice = pParam;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);

    BTHD_ProcessInterruptEvent(pDevice->thdHandle);
}
static void NEXUS_Frontend_P_7552_ThdLockChange_isr(void *pParam)
{
    NEXUS_7552 *pDevice = pParam;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);

    if(pDevice->acquireInProgress){
        pDevice->count++;
    }
    if(pDevice->count == 2){
        pDevice->acquireInProgress = false;
        pDevice->count = 0;
    }
    if (NULL != pDevice->thdLockAppCallback) NEXUS_IsrCallback_Fire_isr(pDevice->thdLockAppCallback);
}

static void NEXUS_Frontend_P_7552_ThdEwsEvent(void *pParam)
{
    NEXUS_7552 *pDevice = pParam;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BDBG_MSG(("OFDM EWS event set"));
    NEXUS_TaskCallback_Fire(pDevice->ewsAppCallback);
}
static void NEXUS_Frontend_P_7552_ThdBbsEvent(void *pParam)
{
    NEXUS_7552 *pDevice = pParam;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);

    BDBG_MSG(("Calling BTHD_ProcessBBSInterruptEvent"));
    BTHD_ProcessBBSInterruptEvent(pDevice->thdHandle);
}
static void NEXUS_Frontend_P_7552_ThdTuner_isr(void *pParam)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    /* NOTE: Even though the pDevice is passed in as the pParam, NEXUS_Frontend_P_7552_Tuner_isr gets called with BTHD_P_ThdCallbackData_t  as its pParam. */
    BTHD_P_ThdCallbackData_t *pCallback;
    BTNR_3x7x_RfStatus_t RfCallbackStatus;
    NEXUS_7552 *pDevice;
    BDBG_ASSERT(NULL != pParam);
    pCallback = (BTHD_P_ThdCallbackData_t *)pParam;
    pDevice = (NEXUS_7552 *) pCallback->hTunerChn;

    if (BTHD_CallbackMode_eSetMode == pCallback->Mode) {
        /* TODO: call the tuner with pCallback->Freq_Offset */
        rc = BTNR_3x7x_Set_RF_Offset( pDevice->tnrHandle, pCallback->Freq_Offset, pCallback->Symbol_Rate);
        if(rc){BERR_TRACE(rc); goto done;}

    } else {
        /* magic numbers from hardware team */

        rc = BTNR_3x7x_Get_RF_Status(pDevice->tnrHandle, &RfCallbackStatus);
        if(rc){BERR_TRACE(rc); goto done;}
        pCallback->RF_Freq = RfCallbackStatus.RF_Freq;
        pCallback->Total_Mix_After_ADC = RfCallbackStatus.Total_Mix_After_ADC;
        pCallback->PreADC_Gain_x256db  = RfCallbackStatus.PreADC_Gain_x256db;
        pCallback->PostADC_Gain_x256db = RfCallbackStatus.PostADC_Gain_x256db;
        pCallback->External_Gain_x256db = RfCallbackStatus.External_Gain_x256db;

    }
done:
    return;
}

static void NEXUS_Frontend_P_7552_AdsLockChange_isr(void *pParam)
{
    NEXUS_7552 *pDevice = pParam;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);

    if(pDevice->acquireInProgress){
        pDevice->count++;
    }
    if(pDevice->count == 2){
        pDevice->acquireInProgress = false;
        pDevice->count = 0;
    }

    if (NULL != pDevice->adsLockAppCallback) NEXUS_IsrCallback_Fire_isr(pDevice->adsLockAppCallback);
}
static void NEXUS_Frontend_P_7552_OobLockChange_isr(void *pParam)
{
    NEXUS_IsrCallbackHandle callback = (NEXUS_IsrCallbackHandle)pParam;

    if (NULL != callback) NEXUS_IsrCallback_Fire_isr(callback);
}

static void NEXUS_Frontend_P_7552_AdsTuner_isr(void *pParam)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    /* NOTE: Even though the pDevice is passed in as the pParam, NEXUS_Frontend_P_7552_Ads_Tuner_isr gets called with BADS_P_AdsCallbackData_s as its pParam. */
    BADS_P_AdsCallbackData_t *pCallback;
    BTNR_3x7x_RfStatus_t RfCallbackStatus;
    NEXUS_7552 *pDevice;
    BDBG_ASSERT(NULL != pParam);
    pCallback = (BADS_P_AdsCallbackData_t *)pParam;
    pDevice = (NEXUS_7552 *) pCallback->hTunerChn;

    if (BADS_CallbackMode_eSetMode == pCallback->Mode) {
        /* TODO: call the tuner with pCallback->Freq_Offset */
        rc = BTNR_3x7x_Set_RF_Offset( pDevice->tnrHandle, pCallback->Freq_Offset, pCallback->Symbol_Rate);
        if(rc){BERR_TRACE(rc); goto done;}
    } else {
        /* magic numbers from hardware team */

        rc = BTNR_3x7x_Get_RF_Status(pDevice->tnrHandle, &RfCallbackStatus);
        if(rc){BERR_TRACE(rc); goto done;}
        pCallback->RF_Freq = RfCallbackStatus.RF_Freq;
        pCallback->Total_Mix_After_ADC = RfCallbackStatus.Total_Mix_After_ADC;
        pCallback->PreADC_Gain_x256db  = RfCallbackStatus.PreADC_Gain_x256db;
        pCallback->PostADC_Gain_x256db = RfCallbackStatus.PostADC_Gain_x256db;
        pCallback->External_Gain_x256db = RfCallbackStatus.External_Gain_x256db;
        pCallback->Freq_Offset = RfCallbackStatus.RF_Offset;
        pCallback->Symbol_Rate = RfCallbackStatus.Symbol_Rate;
    }
done:
    return;
}

/***************************************************************************
Summary:
    ISR Event Handler for a 7552 device
 ***************************************************************************/
static void NEXUS_Frontend_P_7552_AdsIsrEvent(void *pParam)
{
    NEXUS_7552 *pDevice = pParam;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);

    BDBG_MSG(("Calling BADS_ProcessInterruptEvent"));
    BADS_ProcessInterruptEvent(pDevice->adsHandle);
}

#ifndef NEXUS_FRONTEND_7552_A0
static void NEXUS_Frontend_P_oobIsrBBSEvent(void *pParam)
{
    NEXUS_7552 *pDevice = pParam;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BDBG_MSG(("Calling BTNR_Ob_3x7x_ProcessInterruptEvent"));
    BTNR_Ob_3x7x_ProcessInterruptEvent(pDevice->oobTnrHandle);
}
static void NEXUS_Frontend_P_oobIsrEvent(void *pParam)
{
    NEXUS_7552 *pDevice = pParam;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BDBG_MSG(("Calling BAOB_ProcessInterruptEvent"));
    BAOB_ProcessInterruptEvent(pDevice->oobHandle);
}
#endif

#if NEXUS_AMPLIFIER_SUPPORT
NEXUS_Error NEXUS_FrontendDevice_P_7552_GetAmplifierStatus(void *handle, NEXUS_AmplifierStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7552 *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7552 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    if(pDevice->pGenericDeviceHandle->parent != NULL){
        rc = NEXUS_Frontend_P_GetAmplifierStatus(pDevice->pGenericDeviceHandle->parent, pStatus);
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

NEXUS_Error NEXUS_FrontendDevice_P_7552_SetAmplifierStatus(void *handle, const NEXUS_AmplifierStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7552 *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7552 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);

    BSTD_UNUSED(pStatus);
    /* Set LNA/Amplifier parameters. */

    return rc;
}
#endif

NEXUS_Error NEXUS_FrontendDevice_P_7552_GetInternalGain(void *handle, const NEXUS_GainParameters *params, NEXUS_InternalGainSettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7552 *pDevice;
    BTNR_3x7x_InternalGainInputParams inputParams;
    BTNR_3x7x_InternalGainSettings internalGain;
    NEXUS_GainParameters gainParams;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7552 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);

    pSettings->frequency = params->frequency;
    inputParams.frequency = params->frequency;

    rc = BTNR_3x7x_GetInternalGain(pDevice->tnrHandle, &inputParams, &internalGain);
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
NEXUS_Error NEXUS_FrontendDevice_P_7552_GetExternalGain(void *handle, NEXUS_ExternalGainSettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7552 *pDevice;
    BTNR_3x7x_ExternalGainSettings gain;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7552 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BSTD_UNUSED(pSettings);

    rc = BTNR_3x7x_GetExternalGain(pDevice->tnrHandle, &gain);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pSettings->bypassableGain = gain.externalGainBypassable;
    pSettings->totalGain = gain.externalGainTotal;

done:
    return rc;

}
NEXUS_Error NEXUS_FrontendDevice_P_7552_SetExternalGain(void *handle, const NEXUS_ExternalGainSettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7552 *pDevice;
    BTNR_3x7x_ExternalGainSettings externalGain;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7552 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);

    externalGain.externalGainTotal = pSettings->totalGain;
    externalGain.externalGainBypassable = pSettings->bypassableGain;

    rc = BTNR_3x7x_SetExternalGain(pDevice->tnrHandle, &externalGain);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}

static NEXUS_Error NEXUS_FrontendDevice_P_7552_GetStatus(void *handle, NEXUS_FrontendDeviceStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7552 *pDevice;
    NEXUS_AvsStatus avsData;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7552 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
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

done:
    return rc;
}

/***************************************************************************
Summary:
    Get the default settings for a BCM7552 device.
See Also:
    NEXUS_FrontendDevice_Open7552
 ***************************************************************************/
void NEXUS_FrontendDevice_GetDefault7552OpenSettings(NEXUS_FrontendDevice7552OpenSettings *pSettings)
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_Open7552(unsigned index, const NEXUS_FrontendDevice7552OpenSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7552 *pDevice = p_7552device;
    NEXUS_FrontendDevice *pFrontendDevice = NULL;
    BTNR_3x7x_Settings tnrSettings;
    BADS_Settings adsSettings;
    BTHD_Settings thdSettings;
    unsigned numChannels;
    BADS_ChannelSettings channelSettings;
    NEXUS_7552ProbeResults results;
#ifndef NEXUS_FRONTEND_7552_A0
    BTNR_Ob_3x7x_Settings oobTnrSettings;
    BAOB_Settings oobSettings;
#endif

    BSTD_UNUSED(index);
    BDBG_ASSERT(NULL != pSettings);

    rc = NEXUS_Frontend_Probe7552(pSettings, &results);
    if(rc){rc = BERR_TRACE(rc); goto err_open_device;}

    if ( NULL == pDevice)
    {
        pFrontendDevice = NEXUS_FrontendDevice_P_Create();
        if (NULL == pFrontendDevice) {rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_open_device;}

        pDevice = (NEXUS_7552 *) BKNI_Malloc(sizeof(*pDevice));
        if (NULL == pDevice) {rc = BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY); goto err_open_device;}

        BKNI_Memset(pDevice, 0, sizeof(*pDevice));
        BDBG_OBJECT_SET(pDevice, NEXUS_7552);

        tnrSettings.i2cAddr = 0;
        tnrSettings.hTmr = g_pCoreHandles->tmr;
        tnrSettings.hHeap = g_pCoreHandles->heap[0].mem;

        rc = BTNR_3x7x_Open(&pDevice->tnrHandle, &tnrSettings, g_pCoreHandles->reg);
        if(rc){rc = BERR_TRACE(rc); goto err_open_device;}

        rc = BTNR_3x7x_GetInterruptEventHandle(pDevice->tnrHandle, &pDevice->tnrIsrEvent);
        if(rc){rc = BERR_TRACE(rc); goto err_open_device;}

        pDevice->tnrIsrEventCallback = NEXUS_RegisterEvent(pDevice->tnrIsrEvent, NEXUS_Frontend_P_TnrIsrBBSEvent, pDevice);
        if (NULL == pDevice->tnrIsrEventCallback){rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_open_device; }

        pDevice->frontendcount = 0;
        pDevice->pGenericDeviceHandle = pFrontendDevice;
        pDevice->previousStandbyMode = NEXUS_StandbyMode_eOn;
        p_7552device = pDevice;
    }
    else
    {
        return pDevice->pGenericDeviceHandle;
    }

    if((results.chip.id == 0x7581) || (results.chip.id == 0x7582) ||
       (results.chip.id == 0x7591) || (results.chip.id == 0x7592) ||
       (results.chip.id == 0x7574) || (results.chip.id == 0x7023))
    {
        /* get default settings and update */
        rc = BADS_7552_GetDefaultSettings(&adsSettings, NULL);
        if(rc){rc = BERR_TRACE(rc); goto err_open_device;}

        adsSettings.hTmr = g_pCoreHandles->tmr;
        adsSettings.hHeap = g_pCoreHandles->heap[0].mem;

        /* Open ADS device */
        rc = BADS_Open(&pDevice->adsHandle, g_pCoreHandles->chp, g_pCoreHandles->reg, g_pCoreHandles->bint, &adsSettings);
        if(rc){rc = BERR_TRACE(rc); goto err_open_device;}

        /* Initialize Acquisition Processor */
        rc = BADS_Init(pDevice->adsHandle);
        if(rc){rc = BERR_TRACE(rc); goto err_open_device;}

        /* Open the ADS channel handles */
        rc = BADS_GetTotalChannels(pDevice->adsHandle, &numChannels);
        if(rc){rc = BERR_TRACE(rc); goto err_open_device;}

        /* only one channel in 7552 is asserted */
        BDBG_ASSERT(1 == numChannels);

        rc = BADS_GetChannelDefaultSettings(pDevice->adsHandle, 0, &channelSettings);
        if(rc){rc = BERR_TRACE(rc); goto err_open_device;}

        rc = BADS_OpenChannel(pDevice->adsHandle, &pDevice->channelHandle, 0, &channelSettings);
        if(rc){rc = BERR_TRACE(rc); goto err_open_device;}
    }

    if((results.chip.id == 0x7531) || (results.chip.id == 0x7532) ||
       (results.chip.id == 0x7541) || (results.chip.id == 0x7542) ||
       (results.chip.id == 0x7551) || (results.chip.id == 0x7552) ||
       (results.chip.id == 0x7591) || (results.chip.id == 0x7592))
    {
        BTHD_3x7x_GetDefaultSettings(&thdSettings);
        thdSettings.hTmr = g_pCoreHandles->tmr;
        thdSettings.hHeap = g_pCoreHandles->heap[0].mem;
        thdSettings.supportIsdbt = pSettings->supportIsdbt;

        /* Open THD device */
        rc = BTHD_Open(&pDevice->thdHandle, g_pCoreHandles->chp, (void*)g_pCoreHandles->reg, g_pCoreHandles->bint, &thdSettings);
        if(rc){rc = BERR_TRACE(rc); goto err_open_device;}

        /* Initialize Acquisition Processor */
        rc = BTHD_Init(pDevice->thdHandle, NULL, 0);
        if(rc){rc = BERR_TRACE(rc); goto err_open_device;}

        rc = BTHD_GetEWSEventHandle(pDevice->thdHandle, &pDevice->ewsEvent);
        if(rc){rc = BERR_TRACE(rc); goto err_open_device;}

        pDevice->ewsEventCallback = NEXUS_RegisterEvent(pDevice->ewsEvent, NEXUS_Frontend_P_7552_ThdEwsEvent, pDevice);
        if (NULL == pDevice->ewsEventCallback){rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_open_device; }

        rc = BTHD_GetBBSInterruptEventHandle(pDevice->thdHandle, &pDevice->bbsEvent);
        if(rc){rc = BERR_TRACE(rc); goto err_open_device;}

        pDevice->bbsEventCallback = NEXUS_RegisterEvent(pDevice->bbsEvent, NEXUS_Frontend_P_7552_ThdBbsEvent, pDevice);
        if (NULL == pDevice->bbsEventCallback){rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_open_device; }

        /* NOTE: Even though the pDevice is passed in as the pParam, NEXUS_Frontend_P_7552_Tuner_isr gets called with BTHD_P_ThdCallbackData_t  as its pParam. */
        rc = BTHD_InstallCallback(pDevice->thdHandle, BTHD_Callback_eTuner, (BTHD_CallbackFunc)NEXUS_Frontend_P_7552_ThdTuner_isr, (void*)pDevice);
        if(rc){rc = BERR_TRACE(rc); goto err_open_device;}
    }

#ifndef NEXUS_FRONTEND_7552_A0
    oobTnrSettings.i2cAddr = 0;
    oobTnrSettings.hTmr = g_pCoreHandles->tmr;
    oobTnrSettings.hHeap = g_pCoreHandles->heap[0].mem;

    rc = BTNR_Ob_3x7x_Open(&pDevice->oobTnrHandle, &oobTnrSettings, g_pCoreHandles->reg);
    if(rc){rc = BERR_TRACE(rc); goto err_open_device;}

    rc = BTNR_Ob_3x7x_GetInterruptEventHandle(pDevice->oobTnrHandle, &pDevice->tnrIsrEvent);
    if(rc){rc = BERR_TRACE(rc); goto err_open_device;}

    pDevice->oobTnrIsrEventCallback = NEXUS_RegisterEvent(pDevice->tnrIsrEvent, NEXUS_Frontend_P_oobIsrBBSEvent, pDevice);
    if (NULL == pDevice->oobTnrIsrEventCallback){rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_open_device; }

    /* get default settings and update */
    rc = BAOB_GetDefaultSettings(&oobSettings, NULL);
    if(rc){rc = BERR_TRACE(rc); goto err_open_device;}

    oobSettings.hTmr = g_pCoreHandles->tmr;
    oobSettings.hHeap = g_pCoreHandles->heap[0].mem;

    /* Open AOB device */
    rc = BAOB_Open(&pDevice->oobHandle, g_pCoreHandles->chp, g_pCoreHandles->reg, g_pCoreHandles->bint, &oobSettings);
    if(rc){rc = BERR_TRACE(rc); goto err_open_device;}

    rc = BAOB_GetInterruptEventHandle(pDevice->oobHandle, &pDevice->oobIsrEvent);
    if(rc){rc = BERR_TRACE(rc); goto err_open_device;}

    pDevice->oobIsrEventCallback = NEXUS_RegisterEvent(pDevice->oobIsrEvent, NEXUS_Frontend_P_oobIsrEvent, pDevice);
    if (NULL == pDevice->oobIsrEventCallback){rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_open_device; }

#endif

    pFrontendDevice->pDevice = pDevice;
    pFrontendDevice->familyId = results.chip.familyId;
    pFrontendDevice->bypassableFixedGain = pSettings->externalFixedGain.bypassable;
    pFrontendDevice->totalFixedBoardGain = pSettings->externalFixedGain.total;
    pFrontendDevice->application = NEXUS_FrontendDeviceApplication_eUnset;
    pFrontendDevice->close = NEXUS_FrontendDevice_P_7552_Close;

    pFrontendDevice->getInternalGain = NEXUS_FrontendDevice_P_7552_GetInternalGain;
    pFrontendDevice->getExternalGain = NEXUS_FrontendDevice_P_7552_GetExternalGain;
    pFrontendDevice->setExternalGain = NEXUS_FrontendDevice_P_7552_SetExternalGain;
    pFrontendDevice->getStatus = NEXUS_FrontendDevice_P_7552_GetStatus;
    pFrontendDevice->standby = NEXUS_FrontendDevice_P_7552_Standby;
#if NEXUS_AMPLIFIER_SUPPORT
    pFrontendDevice->getAmplifierStatus = NEXUS_Frontend_P_7552_GetAmplifierStatus;
    pFrontendDevice->setAmplifierStatus = NEXUS_Frontend_P_7552_SetAmplifierStatus;
#endif
    return pFrontendDevice;


err_open_device:
    if(pDevice->oobHandle){
        BAOB_Close(pDevice->oobHandle);
        pDevice->oobHandle = NULL;
    }
    if(pDevice->oobTnrIsrEventCallback)NEXUS_UnregisterEvent(pDevice->oobTnrIsrEventCallback);
    if(pDevice->oobTnrHandle){
        BTNR_Close(pDevice->oobTnrHandle);
        pDevice->oobTnrHandle = NULL;
    }
    if(pDevice->bbsEventCallback)NEXUS_UnregisterEvent(pDevice->bbsEventCallback);
    if(pDevice->ewsEventCallback)NEXUS_UnregisterEvent(pDevice->ewsEventCallback);
    if(pDevice->thdIsrEventCallback)NEXUS_UnregisterEvent(pDevice->thdIsrEventCallback);
    if(pDevice->thdHandle){
        BTHD_Close(pDevice->thdHandle);
        pDevice->thdHandle = NULL;
    }
    if(pDevice->channelHandle){
        BADS_CloseChannel(pDevice->channelHandle);
        pDevice->channelHandle = NULL;
    }
    if(pDevice->adsHandle){
        BADS_Close(pDevice->adsHandle);
        pDevice->adsHandle = NULL;
    }
    if(pDevice->tnrIsrEventCallback)NEXUS_UnregisterEvent(pDevice->tnrIsrEventCallback);
    if(pDevice->tnrHandle){
        BTNR_Close(pDevice->tnrHandle);
        pDevice->tnrHandle = NULL;
    }
    if(pDevice){
        BDBG_OBJECT_DESTROY(pDevice, NEXUS_7552);
        BKNI_Free(pDevice);
        pDevice = NULL;
    }
    if(pFrontendDevice) BKNI_Free(pFrontendDevice);
    return NULL;
}

/***************************************************************************
Summary:
    Get the default settings for a BCM7552 Frontend
See Also:
    NEXUS_Frontend_Open7552
 ***************************************************************************/
void NEXUS_Frontend_GetDefault7552Settings(NEXUS_7552FrontendSettings *pSettings )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof*(pSettings));
    pSettings->supportIsdbt = true;
}

/***************************************************************************
Summary:
    Open a handle to a BCM7552 device.
 ***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_Open7552( const NEXUS_7552FrontendSettings *pSettings )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_FrontendHandle frontendHandle;
    NEXUS_7552 *pDevice=NULL;
    NEXUS_IsrCallbackHandle callbackLockChange=NULL;
    NEXUS_FrontendDevice7552OpenSettings openSettings;
    NEXUS_FrontendDevice *pFrontendDevice = NULL;

    BDBG_ASSERT(NULL != pSettings);

    if(pSettings->device == NULL) {
        NEXUS_FrontendDevice_GetDefault7552OpenSettings(&openSettings);
        openSettings.supportIsdbt = pSettings->supportIsdbt;
        pFrontendDevice = NEXUS_FrontendDevice_Open7552(0, &openSettings);
        pDevice = (NEXUS_7552 *)pFrontendDevice->pDevice;
    }
    else {
        pDevice = (NEXUS_7552 *)pSettings->device->pDevice;
        pFrontendDevice =  pSettings->device;
    }

    /* Create a Nexus frontend handle */
    frontendHandle = NEXUS_Frontend_P_Create(pDevice);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if (pSettings->type == NEXUS_7552ChannelType_eQam){
        /* Establish device capabilities */
        frontendHandle->capabilities.qam = true;
        frontendHandle->capabilities.scan = false;
        /* QAM 16/32/64/128/256 are supported, others are not */
        frontendHandle->capabilities.qamModes[NEXUS_FrontendQamMode_e16] = true;
        frontendHandle->capabilities.qamModes[NEXUS_FrontendQamMode_e32] = true;
        frontendHandle->capabilities.qamModes[NEXUS_FrontendQamMode_e64] = true;
        frontendHandle->capabilities.qamModes[NEXUS_FrontendQamMode_e128] = true;
        frontendHandle->capabilities.qamModes[NEXUS_FrontendQamMode_e256] = true;

        /* Bind required callbacks */
        frontendHandle->tuneQam = NEXUS_Frontend_P_7552_TuneQam;
        frontendHandle->getQamStatus = NEXUS_Frontend_P_7552_GetQamStatus;
        frontendHandle->getQamScanStatus = NEXUS_Frontend_P_7552_GetQamScanStatus;
        frontendHandle->requestQamAsyncStatus = NEXUS_Frontend_P_7552_RequestQamAsyncStatus;
        frontendHandle->getQamAsyncStatus = NEXUS_Frontend_P_7552_GetQamAsyncStatus;

        pDevice->adsAsyncStatusAppCallback = NEXUS_TaskCallback_Create(frontendHandle, NULL);
        if(rc){rc = BERR_TRACE(rc); goto error;}

        callbackLockChange = NEXUS_IsrCallback_Create(frontendHandle, NULL);
        if(rc){rc = BERR_TRACE(rc); goto error;}

        rc = BADS_InstallCallback(pDevice->channelHandle, BADS_Callback_eLockChange, (BADS_CallbackFunc)NEXUS_Frontend_P_7552_AdsLockChange_isr, (void*)pDevice);
        if(rc){rc = BERR_TRACE(rc); goto error;}

        /* NOTE: Even though the pDevice is passed in as the pParam, NEXUS_Frontend_P_7552_Ads_Tuner_isr gets called with BADS_P_AdsCallbackData_s as its pParam. */
        rc = BADS_InstallCallback(pDevice->channelHandle, BADS_Callback_eTuner, (BADS_CallbackFunc)NEXUS_Frontend_P_7552_AdsTuner_isr, (void*)pDevice);
        if(rc){rc = BERR_TRACE(rc); goto error;}

        pDevice->adsLockAppCallback = callbackLockChange;
        pDevice->frontendcount++;
    }
    else if(pSettings->type == NEXUS_7552ChannelType_eOfdm){

        pDevice->frontendcount++;

        rc = BTHD_InstallCallback(pDevice->thdHandle, BTHD_Callback_eLockChange, (BTHD_CallbackFunc)NEXUS_Frontend_P_7552_ThdLockChange_isr, (void*)pDevice);
        if(rc){rc = BERR_TRACE(rc); goto error;}

        pDevice->thdLockAppCallback = NEXUS_IsrCallback_Create(frontendHandle, NULL);
        if(rc){rc = BERR_TRACE(rc); goto error;}

        pDevice->ewsAppCallback = NEXUS_TaskCallback_Create(frontendHandle, NULL);
        if(rc){rc = BERR_TRACE(rc); goto error;}

        pDevice->thdAsyncStatusAppCallback = NEXUS_TaskCallback_Create(frontendHandle, NULL);
        if(rc){rc = BERR_TRACE(rc); goto error;}

        /* Establish device capabilities */
        frontendHandle->capabilities.ofdm = true;

        /* DVB-T is supported, but ISDB-T is supported only when it is requested */
        frontendHandle->capabilities.ofdmModes[NEXUS_FrontendOfdmMode_eDvbt] = true;
        frontendHandle->capabilities.ofdmModes[NEXUS_FrontendOfdmMode_eIsdbt] = pSettings->supportIsdbt;

        /* Bind required callbacks */
        frontendHandle->tuneOfdm = NEXUS_Frontend_P_7552_TuneOfdm;
        frontendHandle->getOfdmStatus = NEXUS_Frontend_P_7552_GetOfdmStatus;
        frontendHandle->requestOfdmAsyncStatus = NEXUS_Frontend_P_7552_RequestOfdmAsyncStatus;
        frontendHandle->getOfdmAsyncStatus = NEXUS_Frontend_P_7552_GetOfdmAsyncStatus;
        frontendHandle->requestDvbtAsyncStatus = NEXUS_Frontend_P_7552_RequestDvbtAsyncStatus;
        frontendHandle->getDvbtAsyncStatusReady = NEXUS_Frontend_P_7552_GetDvbtAsyncStatusReady;
        frontendHandle->getDvbtAsyncStatus = NEXUS_Frontend_P_7552_GetDvbtAsyncStatus;
        frontendHandle->requestIsdbtAsyncStatus = NEXUS_Frontend_P_7552_RequestIsdbtAsyncStatus;
        frontendHandle->getIsdbtAsyncStatusReady = NEXUS_Frontend_P_7552_GetIsdbtAsyncStatusReady;
        frontendHandle->getIsdbtAsyncStatus = NEXUS_Frontend_P_7552_GetIsdbtAsyncStatus;
    }
#ifndef NEXUS_FRONTEND_7552_A0
    else if(pSettings->type == NEXUS_7552ChannelType_eOutOfBand){

        pDevice->frontendcount++;

        frontendHandle->capabilities.outOfBand = true;
        frontendHandle->tuneOutOfBand = NEXUS_Frontend_P_7552_TuneOob;

        callbackLockChange = NEXUS_IsrCallback_Create(frontendHandle, NULL);
        if(rc){rc = BERR_TRACE(rc); goto error;}

        rc = BAOB_InstallCallback(pDevice->oobHandle, BAOB_Callback_eLockChange, (BAOB_CallbackFunc)NEXUS_Frontend_P_7552_OobLockChange_isr, (void*)callbackLockChange);
        if(rc){rc = BERR_TRACE(rc); goto error;}

        pDevice->oobLockAppCallback = callbackLockChange;
    }
#endif
    frontendHandle->close = NEXUS_Frontend_P_7552_Close;
    frontendHandle->untune = NEXUS_Frontend_P_7552_Untune;
    frontendHandle->getFastStatus = NEXUS_Frontend_P_7552_GetFastStatus;
    frontendHandle->resetStatus = NEXUS_Frontend_P_7552_ResetStatus;
    frontendHandle->readSoftDecisions = NEXUS_Frontend_P_7552_ReadSoftDecisions;
    frontendHandle->writeRegister = NEXUS_Frontend_P_7552_WriteRegister;
    frontendHandle->readRegister = NEXUS_Frontend_P_7552_ReadRegister;
    frontendHandle->standby = NEXUS_Frontend_P_7552_Standby;

    frontendHandle->pGenericDeviceHandle = pFrontendDevice;

    pDevice->currentMode = UNKNOWN_7552;
    frontendHandle->chip.familyId = 0x7552;
    return frontendHandle;

error:
    if(pDevice->adsAsyncStatusAppCallback)NEXUS_TaskCallback_Destroy(pDevice->adsAsyncStatusAppCallback);
    if(pDevice->thdAsyncStatusAppCallback)NEXUS_TaskCallback_Destroy(pDevice->thdAsyncStatusAppCallback);
    if(pDevice->thdLockAppCallback)NEXUS_IsrCallback_Destroy(pDevice->thdLockAppCallback);
    if(callbackLockChange)NEXUS_IsrCallback_Destroy(callbackLockChange);
    if(frontendHandle)NEXUS_Frontend_P_Destroy(frontendHandle);
done:
    return NULL;
}

/***************************************************************************
Summary:
    Close a handle to a BCM7552 device.
See Also:
    NEXUS_Frontend_Open7552
 ***************************************************************************/
static void NEXUS_Frontend_P_7552_Close( NEXUS_FrontendHandle handle )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7552 *pDevice;
    NEXUS_FrontendCapabilities capabilities;
    BTNR_PowerSaverSettings pwrSettings;
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    pDevice = (NEXUS_7552 *)handle->pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);

    if(pDevice->isTunerPowered){
        pwrSettings.enable = true;
        rc = BTNR_SetPowerSaver(pDevice->tnrHandle, &pwrSettings);
        pDevice->isTunerPowered = false;
        if(rc){rc = BERR_TRACE(rc);}
    }

    NEXUS_Frontend_GetCapabilities(handle, &capabilities);
    if ( capabilities.qam ){
        if(pDevice->adsLockAppCallback) NEXUS_IsrCallback_Destroy(pDevice->adsLockAppCallback);
        pDevice->adsLockAppCallback = NULL;
        BADS_InstallCallback(pDevice->channelHandle, BADS_Callback_eLockChange, NULL, NULL);
        BADS_InstallCallback(pDevice->channelHandle, BADS_Callback_eTuner, NULL, NULL);
        if (pDevice->adsIsrEventCallback) NEXUS_UnregisterEvent(pDevice->adsIsrEventCallback);
        pDevice->adsIsrEventCallback = NULL;
        if(pDevice->adsAsyncStatusAppCallback)NEXUS_TaskCallback_Destroy(pDevice->adsAsyncStatusAppCallback);

        if(handle) NEXUS_Frontend_P_Destroy(handle);
        pDevice->frontendcount--;
    }
    else if(capabilities.ofdm){
        BTHD_InstallCallback(pDevice->thdHandle, BTHD_Callback_eLockChange, NULL, NULL);
        if (pDevice->thdLockAppCallback) NEXUS_IsrCallback_Destroy(pDevice->thdLockAppCallback);
        pDevice->thdLockAppCallback = NULL;
        if (pDevice->ewsAppCallback) NEXUS_TaskCallback_Destroy(pDevice->ewsAppCallback);
        pDevice->ewsAppCallback = NULL;
        if(pDevice->thdAsyncStatusAppCallback)NEXUS_TaskCallback_Destroy(pDevice->thdAsyncStatusAppCallback);

        if(handle) NEXUS_Frontend_P_Destroy(handle);
        pDevice->frontendcount--;
    }
#ifndef NEXUS_FRONTEND_7552_A0
    else if(capabilities.outOfBand){
        BAOB_InstallCallback(pDevice->oobHandle, BAOB_Callback_eLockChange, NULL, NULL);
        if (pDevice->oobLockAppCallback) NEXUS_IsrCallback_Destroy(pDevice->oobLockAppCallback);
        pDevice->oobLockAppCallback = NULL;

        if(handle) NEXUS_Frontend_P_Destroy(handle);
        pDevice->frontendcount--;
    }
#endif
}

static void NEXUS_FrontendDevice_P_7552_Close(void *handle)
{
    NEXUS_7552 *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7552 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);

    NEXUS_FrontendDevice_Unlink(pDevice->pGenericDeviceHandle, NULL);
    BKNI_Free(pDevice->pGenericDeviceHandle);
    pDevice->pGenericDeviceHandle = NULL;

    if(pDevice->channelHandle)BADS_CloseChannel(pDevice->channelHandle);
    pDevice->channelHandle = NULL;
    if(pDevice->adsHandle)BADS_Close(pDevice->adsHandle);
    pDevice->adsHandle = NULL;

    if(pDevice->ewsEventCallback)NEXUS_UnregisterEvent(pDevice->ewsEventCallback);
    pDevice->ewsEventCallback = NULL;
    if(pDevice->thdIsrEventCallback)NEXUS_UnregisterEvent(pDevice->thdIsrEventCallback);
    pDevice->thdIsrEventCallback = NULL;
    if(pDevice->bbsEventCallback)NEXUS_UnregisterEvent(pDevice->bbsEventCallback);
    pDevice->bbsEventCallback = NULL;

    if(pDevice->thdHandle)BTHD_Close(pDevice->thdHandle);
    pDevice->thdHandle = NULL;
#ifndef NEXUS_FRONTEND_7552_A0
    if(pDevice->oobIsrEventCallback)NEXUS_UnregisterEvent(pDevice->oobIsrEventCallback);
    pDevice->oobIsrEventCallback = NULL;
    if(pDevice->oobHandle)BAOB_Close(pDevice->oobHandle);
    pDevice->oobHandle = NULL;

    if (pDevice->oobTnrIsrEventCallback) NEXUS_UnregisterEvent(pDevice->oobTnrIsrEventCallback);
    pDevice->oobTnrIsrEventCallback = NULL;
    if(pDevice->oobTnrHandle) BTNR_Close(pDevice->oobTnrHandle);
    pDevice->oobTnrHandle = NULL;
#endif

    if (pDevice->tnrIsrEventCallback) NEXUS_UnregisterEvent(pDevice->tnrIsrEventCallback);
    pDevice->tnrIsrEventCallback = NULL;
    if(pDevice->tnrHandle) BTNR_Close(pDevice->tnrHandle);
    pDevice->tnrHandle = NULL;

    BDBG_OBJECT_DESTROY(pDevice, NEXUS_7552);
    if (pDevice) BKNI_Free(pDevice);
    pDevice = NULL;
    p_7552device = NULL;

    return;
}


static NEXUS_Error NEXUS_Frontend_P_7552_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7552 *pDevice;
    BTNR_PowerSaverSettings pwrSettings;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7552 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BSTD_UNUSED(pSettings);

    if (enabled) {
        if(pDevice->currentMode == QAM_7552){
            if(pDevice->adsIsrEventCallback) {
                NEXUS_UnregisterEvent(pDevice->adsIsrEventCallback);
                pDevice->adsIsrEventCallback = NULL;
            }

            if(pDevice->isAdsPoweredOn){
                rc = BADS_EnablePowerSaver(pDevice->channelHandle);
                if(rc){rc = BERR_TRACE(rc); goto done;}
                pDevice->isAdsPoweredOn = false;
            }
        }
        else if(pDevice->currentMode == OFDM_7552){
            if(pDevice->thdIsrEventCallback) {
                NEXUS_UnregisterEvent(pDevice->thdIsrEventCallback);
                pDevice->thdIsrEventCallback = NULL;
            }

            if(pDevice->isThdPoweredOn){
                rc = BTHD_PowerDown(pDevice->thdHandle);
                if(rc){rc = BERR_TRACE(rc); goto done;}
                pDevice->isThdPoweredOn = false;
            }
        }


        if(pDevice->isTunerPowered){
            pwrSettings.enable = true;
            rc = BTNR_SetPowerSaver(pDevice->tnrHandle, &pwrSettings);
            pDevice->isTunerPowered = false;
            if(rc){rc = BERR_TRACE(rc); goto done;}
        }
    }
    else if(!enabled){
        if(!pDevice->isTunerPowered){
            pwrSettings.enable = false;
            rc = BTNR_SetPowerSaver(pDevice->tnrHandle, &pwrSettings);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            pDevice->isTunerPowered = true;
            rc = BTNR_3x7x_SetRfInputMode(pDevice->tnrHandle, BTNR_3x7x_RfInputMode_eInternalLna);
            if(rc){rc = BERR_TRACE(rc); goto done;}
        }

        if(pDevice->currentMode == QAM_7552){
            if(!pDevice->isAdsPoweredOn){
                rc = BADS_DisablePowerSaver(pDevice->channelHandle);
                if(rc){rc = BERR_TRACE(rc); goto done;}
                pDevice->isAdsPoweredOn = true;
            }
        }
        else if(pDevice->currentMode == OFDM_7552){
            if(!pDevice->isThdPoweredOn){
                rc = BTHD_PowerUp(pDevice->thdHandle);
                if(rc){rc = BERR_TRACE(rc); goto done;}
                pDevice->isThdPoweredOn = true;
            }
        }
    }
    else{
        BDBG_ERR(("Invalid tuner state."));
    }

done:
    return rc;
}

static NEXUS_Error NEXUS_FrontendDevice_P_7552_Standby(void *handle, const NEXUS_StandbySettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7552 *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7552 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BSTD_UNUSED(pSettings);
    return rc;
}

/***************************************************************************
 * Module callback functions for tuning
 ***************************************************************************/
static void NEXUS_Frontend_P_7552_Untune( void *handle )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7552 *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7552 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);

    if(pDevice->currentMode == QAM_7552){
        if(pDevice->adsIsrEventCallback)NEXUS_UnregisterEvent(pDevice->adsIsrEventCallback);
        pDevice->adsIsrEventCallback = NULL;

        if(pDevice->isAdsPoweredOn){
            rc = BADS_EnablePowerSaver(pDevice->channelHandle);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->isAdsPoweredOn = false;
        }
    }
    else if(pDevice->currentMode == OFDM_7552){
        if(pDevice->thdIsrEventCallback)NEXUS_UnregisterEvent(pDevice->thdIsrEventCallback);
        pDevice->thdIsrEventCallback = NULL;

        if(pDevice->isThdPoweredOn){
            rc = BTHD_PowerDown(pDevice->thdHandle);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->isThdPoweredOn = false;
        }
    }
done:
    return;
}

static NEXUS_Error NEXUS_Frontend_P_7552_WriteRegister(void *handle, unsigned address, uint32_t value)
{
    BSTD_UNUSED(handle);

    if((address >= 0xc00000)&&(address<0xd0ffff))
        BREG_Write32(g_pCoreHandles->reg, address, value);
    else {
        BDBG_WRN(("Invalid frontend address."));
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }


    return NEXUS_SUCCESS;
}
static NEXUS_Error NEXUS_Frontend_P_7552_ReadRegister(void *handle, unsigned address, uint32_t *value   )
{
    BSTD_UNUSED(handle);

    *value = BREG_Read32(g_pCoreHandles->reg, address);

    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_7552_TuneQam( void *handle, const NEXUS_FrontendQamSettings *pSettings )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7552 *pDevice;
    BADS_InbandParam params;
    BADS_ModulationType qam_mode;
    uint32_t symbolRate = 0;
    BTNR_PowerSaverSettings pwrSettings;
    BTNR_Settings tnrSettings;
    BADS_ChannelScanSettings scanParam;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7552 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BDBG_ASSERT(NULL != pSettings);

    pDevice->currentMode = QAM_7552;
    pDevice->lastSettings.qam = *pSettings;

    if (pDevice->tnrHandle)
    {
        if(!pDevice->isTunerPowered){
            /* TODO: we should be able to call BTNR_SetSettings after BTNR_SetPowerSaver. verify */
            pwrSettings.enable = false;
            rc = BTNR_SetPowerSaver(pDevice->tnrHandle, &pwrSettings);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->isTunerPowered = true;
        }

        rc = BTNR_GetSettings(pDevice->tnrHandle, &tnrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        tnrSettings.bandwidth = pSettings->bandwidth;
        tnrSettings.std = BTNR_Standard_eQam;
        rc = BTNR_SetSettings(pDevice->tnrHandle, &tnrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        pDevice->terrestrial = BTNR_TunerApplication_eCable;
        rc = BTNR_3x7x_SetTnrApplication(pDevice->tnrHandle, pDevice->terrestrial);
        if(rc){rc = BERR_TRACE(rc); goto done;}


        pDevice->acquireInProgress = true;
        rc = BTNR_SetTunerRfFreq(pDevice->tnrHandle, pSettings->frequency, BTNR_TunerMode_eDigital);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    if(!pDevice->isAdsPoweredOn){
        /* Get interrupt event and register callback */
        rc = BADS_GetInterruptEventHandle(pDevice->adsHandle, &pDevice->adsIsrEvent);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        pDevice->adsIsrEventCallback = NEXUS_RegisterEvent(pDevice->adsIsrEvent, NEXUS_Frontend_P_7552_AdsIsrEvent, pDevice);
        if (NULL == pDevice->adsIsrEventCallback){rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done; }

        rc = BADS_DisablePowerSaver(pDevice->channelHandle);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pDevice->isAdsPoweredOn = true;
    }

    switch (pSettings->annex) {
    case NEXUS_FrontendQamAnnex_eA:
        switch (pSettings->mode) {
        case NEXUS_FrontendQamMode_e16:
            qam_mode = BADS_ModulationType_eAnnexAQam16;
            break;
        case NEXUS_FrontendQamMode_e32:
            qam_mode = BADS_ModulationType_eAnnexAQam32;
            break;
        case NEXUS_FrontendQamMode_e64:
            qam_mode = BADS_ModulationType_eAnnexAQam64;
            break;
        case NEXUS_FrontendQamMode_e128:
            qam_mode = BADS_ModulationType_eAnnexAQam128;
            break;
        case NEXUS_FrontendQamMode_e256:
            qam_mode = BADS_ModulationType_eAnnexAQam256;
            break;
        default:
            BDBG_ERR(("J.83A invalid constellation (%d)", pSettings->mode));
            rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
        }
        break;
    case NEXUS_FrontendQamAnnex_eB:
        /* only 6MHz, and 64/256QAM */
        if (NEXUS_FrontendBandwidth_e6Mhz != pSettings->bandwidth) {
            BDBG_ERR(("J.83B with non-6MHz bandwidth (%d)", pSettings->bandwidth));
            rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
        }
        switch (pSettings->mode) {
        case NEXUS_FrontendQamMode_e64:
            BDBG_MSG(("J.83B with 64QAM and 5056941"));
            /* TODO: BADS_ModulationType_eAnnexBQam64 is not making ADS happy.  For now, stick to J.83A looking identifier */
            qam_mode = BADS_ModulationType_eAnnexBQam64;
            symbolRate = 5056941; /* QAM64 */
            break;
        case NEXUS_FrontendQamMode_e256:
            BDBG_MSG(("J.83B with 256QAM and 5360537"));
            /* TODO: BADS_ModulationType_eAnnexBQam256 is not making ADS happy. For now, stick to J.83A looking identifier */
            qam_mode = BADS_ModulationType_eAnnexBQam256;
            symbolRate = 5360537; /* QAM256 */
            break;
        default:
            BDBG_ERR(("J.83B invalid constellation (%d)", pSettings->mode));
            rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
        }
        break;
    case NEXUS_FrontendQamAnnex_eC:
        switch (pSettings->mode) {
        case NEXUS_FrontendQamMode_e64:
            qam_mode = BADS_ModulationType_eAnnexCQam64;
            symbolRate = 5274000; /* QAM64 */
            break;
        default:
            BDBG_ERR(("J.83C invalid constellation (%d)", pSettings->mode));
            rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
        }
        break;
    default:
        BDBG_ERR(("Invalid annex (%d)", pSettings->annex));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
        break;
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

        rc = BADS_SetScanParam(pDevice->channelHandle, &scanParam );
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    params.modType = qam_mode;
    params.enableDpm = pSettings->enablePowerMeasurement;
    params.acquisitionType = pSettings->acquisitionMode;
    params.invertSpectrum = pSettings->spectralInversion;
    params.spectrum = pSettings->spectrumMode;
    params.autoAcquire = pSettings->autoAcquire;
    params.frequencyOffset = pSettings->frequencyOffset;
    /* if non-zero, try using what we are asked to use */
    if (0 != pSettings->symbolRate) {
        params.symbolRate = pSettings->symbolRate;
    } else {
        params.symbolRate = symbolRate;
    }

    params.enableNullPackets = pSettings->enableNullPackets;


    BDBG_MSG(("Calling BADS_Acquire() with autoAcquire %d, frequencyOffset %d, enableDpm %d, spectrum %d",
        params.autoAcquire, params.frequencyOffset, params.enableDpm, params.spectrum));

    BDBG_MSG(("Calling BADS_Acquire() with modType %d, enableDpm %d, invertSpectrum %d, spectrum %d, symbolRate %d",
        params.modType, params.enableDpm, params.invertSpectrum, params.spectrum, params.symbolRate));

    NEXUS_IsrCallback_Set(pDevice->adsLockAppCallback, &pSettings->lockCallback);
    NEXUS_TaskCallback_Set(pDevice->adsAsyncStatusAppCallback, &pSettings->asyncStatusReadyCallback);

    rc = BADS_SetAcquireParams(pDevice->channelHandle, &params);
    if(rc){rc = BERR_TRACE(rc); goto done;}
    rc = BADS_Acquire(pDevice->channelHandle, &params);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}

static NEXUS_FrontendLockStatus  NEXUS_Frontend_P_GetLockStatus(unsigned lockStatus)
{
    switch ( lockStatus )
    {
    /*BADS_LockStatus_eUnlocked == BTHD_LockStatus_eUnlocked */
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

static NEXUS_Error NEXUS_Frontend_P_7552_GetFastStatus(void *handle, NEXUS_FrontendFastStatus *pStatus)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7552 *pDevice;
    unsigned lock;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7552 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BDBG_ASSERT(NULL != pStatus);

    if(pDevice->currentMode == QAM_7552){
        rc = BADS_GetLockStatus(pDevice->channelHandle,  &lock);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    else if(pDevice->currentMode == OFDM_7552){
        rc = BTHD_GetThdLockStatus(pDevice->thdHandle,  &lock);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    else {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }
    pStatus->lockStatus = NEXUS_Frontend_P_GetLockStatus(lock);
    pStatus->acquireInProgress = pDevice->acquireInProgress;

done:
    return rc;
}

static NEXUS_FrontendQamAnnex  NEXUS_Frontend_P_ADSToAnnex(BADS_ModulationType modulationType)
{
    if(modulationType < BADS_ModulationType_eAnnexBQam16)
        return NEXUS_FrontendQamAnnex_eA;
    else if(modulationType < BADS_ModulationType_eAnnexCQam16)
        return NEXUS_FrontendQamAnnex_eB;
    else {
        return NEXUS_FrontendQamAnnex_eB;
        BDBG_ERR(("Unsupported Annex."));
    }
}

static NEXUS_FrontendQamMode  NEXUS_Frontend_P_ADSToMode(BADS_ModulationType modulationType)
{
    switch (modulationType) {
    case BADS_ModulationType_eAnnexAQam16:
    case BADS_ModulationType_eAnnexBQam16:
        return NEXUS_FrontendQamMode_e16;
        break;
    case BADS_ModulationType_eAnnexAQam32:
    case BADS_ModulationType_eAnnexBQam32:
        return NEXUS_FrontendQamMode_e32;
        break;
    case BADS_ModulationType_eAnnexAQam64:
    case BADS_ModulationType_eAnnexBQam64:
        return NEXUS_FrontendQamMode_e64;
        break;
    case BADS_ModulationType_eAnnexAQam128:
    case BADS_ModulationType_eAnnexBQam128:
        return NEXUS_FrontendQamMode_e128;
        break;
    case BADS_ModulationType_eAnnexAQam256:
    case BADS_ModulationType_eAnnexBQam256:
        return NEXUS_FrontendQamMode_e256;
        break;
    case BADS_ModulationType_eAnnexAQam512:
    case BADS_ModulationType_eAnnexBQam512:
        return NEXUS_FrontendQamMode_e512;
    case BADS_ModulationType_eAnnexAQam1024:
    case BADS_ModulationType_eAnnexBQam1024:
        return NEXUS_FrontendQamMode_e1024;
        break;
    case BADS_ModulationType_eAnnexAQam2048:
    case BADS_ModulationType_eAnnexBQam2048:
        return NEXUS_FrontendQamMode_e2048;
        break;
    case BADS_ModulationType_eAnnexAQam4096:
    case BADS_ModulationType_eAnnexBQam4096:
        return NEXUS_FrontendQamMode_e4096;
        break;
    default:
        return NEXUS_FrontendQamMode_e64;
        BDBG_WRN(("Unrecognized QAM mode."));
        /* fall through */
    }
}

static NEXUS_Error NEXUS_Frontend_P_7552_GetQamScanStatus(void *handle, NEXUS_FrontendQamScanStatus *pScanStatus)
{

    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7552 *pDevice = (NEXUS_7552 *)handle;
    struct BADS_ScanStatus st;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BDBG_ASSERT(NULL != pScanStatus);


    BKNI_Memset(pScanStatus, 0, sizeof(*pScanStatus));

    rc = BADS_GetScanStatus(pDevice->channelHandle,  &st);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pScanStatus->symbolRate = st.symbolRate;
    pScanStatus->frequencyOffset = st.carrierFreqOffset;
    pScanStatus->interleaver = st.interleaver;
    pScanStatus->spectrumInverted = st.isSpectrumInverted;
    pScanStatus->acquisitionStatus = st.acquisitionStatus;
    pScanStatus->annex = NEXUS_Frontend_P_ADSToAnnex(st.modType);
    pScanStatus->mode = NEXUS_Frontend_P_ADSToMode(st.modType);

    return BERR_SUCCESS;
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7552_GetQamStatus( void *handle, NEXUS_FrontendQamStatus *pStatus )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned j=0;
    NEXUS_7552 *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7552 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof*(pStatus));

    pDevice->isInternalAsyncStatusCall = true;
    pDevice->isInternalAsyncStatusReady = false;

    rc = NEXUS_Frontend_P_7552_RequestQamAsyncStatus(pDevice);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    for(j=0; j < 10; j++) {

        BKNI_Sleep(10);
        if(pDevice->isInternalAsyncStatusReady) {
            rc = NEXUS_Frontend_P_7552_GetQamAsyncStatus(pDevice, pStatus);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            pDevice->isInternalAsyncStatusReady = false;
            break;
        }
    }
    pDevice->isInternalAsyncStatusCall = false;
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7552_ReadSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length, size_t *pNumRead)
{
    /* Since BTHD_GetSoftDecisionBuf returns 30 values, for now it is hardcoded to 30.*/
    #define TOTAL_SOFTDECISIONS 30
    NEXUS_Error rc = NEXUS_SUCCESS;
    size_t i;
    int16_t return_length;
    NEXUS_7552 *pDevice;
    int16_t d_i[TOTAL_SOFTDECISIONS], d_q[TOTAL_SOFTDECISIONS];
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7552 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BDBG_ASSERT(NULL != pDecisions);

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);

    /* only make one call to ADS. if app needs more, they can loop. */
    if(pDevice->currentMode == QAM_7552){
        rc = BADS_GetSoftDecision(pDevice->channelHandle, (int16_t)TOTAL_SOFTDECISIONS,  d_i, d_q, &return_length);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    else if(pDevice->currentMode == OFDM_7552){
        rc = BTHD_GetSoftDecisionBuf(pDevice->thdHandle, d_i, d_q);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    else{
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

static void NEXUS_Frontend_P_7552_ResetStatus( void *handle )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7552 *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7552 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);

    if(pDevice->currentMode == QAM_7552){
     rc = BADS_ResetStatus(pDevice->channelHandle);
     if (rc){BERR_TRACE(rc);}
    }
    else if(pDevice->currentMode == OFDM_7552){
     rc = BTHD_ResetInbandStatus(pDevice->thdHandle);
     if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    else{
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }

done:
    return;
}

static BERR_Code NEXUS_Frontend_P_7552_TuneOfdm( void *handle, const NEXUS_FrontendOfdmSettings *pSettings )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7552 *pDevice;
    BTHD_InbandParams params;
    BTNR_PowerSaverSettings pwrSettings;
    BTNR_Settings tnrSettings;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7552 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BDBG_ASSERT(NULL != pSettings);

    pDevice->currentMode = OFDM_7552;

    rc = BTHD_GetDefaultInbandParams(pDevice->thdHandle, &params);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if (pDevice->tnrHandle)
    {
        if(!pDevice->isTunerPowered){
            pwrSettings.enable = false;
            rc = BTNR_SetPowerSaver(pDevice->tnrHandle, &pwrSettings);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->isTunerPowered = true;
        }

        rc = BTNR_GetSettings(pDevice->tnrHandle, &tnrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}


        switch (pSettings->mode) {
        case NEXUS_FrontendOfdmMode_eDvbt:
            tnrSettings.std = BTNR_Standard_eDvbt;
            params.mode = BTHD_InbandMode_eDvbt;
            break;
        case NEXUS_FrontendOfdmMode_eIsdbt:
            tnrSettings.std = BTNR_Standard_eIsdbt;
            params.mode = BTHD_InbandMode_eIsdbt;
            break;
        default:
            BDBG_ERR(("Invalid OFDM Mode"));
            rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
        }

        tnrSettings.bandwidth = pSettings->bandwidth;
        rc = BTNR_SetSettings(pDevice->tnrHandle, &tnrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        pDevice->terrestrial = BTNR_TunerApplication_eTerrestrial;
        rc = BTNR_3x7x_SetTnrApplication(pDevice->tnrHandle, pDevice->terrestrial);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        pDevice->acquireInProgress = true;
        rc = BTNR_SetTunerRfFreq(pDevice->tnrHandle, pSettings->frequency, BTNR_TunerMode_eDigital);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    else {
        BDBG_ERR(("Invalid Tuner handle."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    if(!pDevice->isThdPoweredOn){
        /* Get interrupt event and register callback */
        rc = BTHD_GetInterruptEventHandle(pDevice->thdHandle, &pDevice->thdIsrEvent);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        pDevice->thdIsrEventCallback = NEXUS_RegisterEvent(pDevice->thdIsrEvent, NEXUS_Frontend_P_7552_ThdIsrEvent, pDevice);
        if (NULL == pDevice->thdIsrEventCallback) {rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done; }

        rc = BTHD_PowerUp(pDevice->thdHandle);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pDevice->isThdPoweredOn = true;
    }

    params.decodeMode = (NEXUS_FrontendOfdmPriority_eHigh == pSettings->priority) ? BTHD_Decode_Hp : BTHD_Decode_Lp;
    params.cciMode = (NEXUS_FrontendOfdmCciMode_eNone == pSettings->cciMode) ? BTHD_CCI_None : BTHD_CCI_Auto;
    params.ifFreq = pSettings->ifFrequency;

    switch (pSettings->bandwidth) {
    case NEXUS_FrontendOfdmBandwidth_e5Mhz:
        params.bandwidth = BTHD_Bandwidth_5Mhz;
        break;
    case NEXUS_FrontendOfdmBandwidth_e6Mhz:
        params.bandwidth = BTHD_Bandwidth_6Mhz;
        break;
    case NEXUS_FrontendOfdmBandwidth_e7Mhz:
        params.bandwidth = BTHD_Bandwidth_7Mhz;
        break;
    default:
        BDBG_WRN(("Unrecognized bandwidth setting, defaulting to 8MHz"));
        /* fall through */
    case NEXUS_FrontendOfdmBandwidth_e8Mhz:
        params.bandwidth = BTHD_Bandwidth_8Mhz;
        break;
    }
    params.bTpsAcquire = !pSettings->manualTpsSettings;
    if (pSettings->manualTpsSettings) {
        params.eCodeRateHP = NEXUS_Frontend_P_CodeRateToTHD(pSettings->tpsSettings.highPriorityCodeRate);
        params.eCodeRateLP = NEXUS_Frontend_P_CodeRateToTHD(pSettings->tpsSettings.lowPriorityCodeRate);
        switch (pSettings->tpsSettings.hierarchy) {
        default:
            BDBG_WRN(("Unrecognized hierarchy setting, defaulting to 0"));
            /* fall through */
        case NEXUS_FrontendOfdmHierarchy_e0:
            params.eHierarchy = BTHD_Hierarchy_0;
            break;
        case NEXUS_FrontendOfdmHierarchy_e1:
            params.eHierarchy = BTHD_Hierarchy_1;
            break;
        case NEXUS_FrontendOfdmHierarchy_e2:
            params.eHierarchy = BTHD_Hierarchy_2;
            break;
        case NEXUS_FrontendOfdmHierarchy_e4:
            params.eHierarchy = BTHD_Hierarchy_4;
            break;
        }
        params.eModulation = NEXUS_Frontend_P_ModulationToTHD(pSettings->tpsSettings.modulation);
    }

    if (pSettings->manualModeSettings) {
        switch (pSettings->modeSettings.modeGuard) {
        case NEXUS_FrontendOfdmModeGuard_eAutoDvbt:
            params.eModeGuardAcquire = BTHD_ModeGuard_eAutoDvbt;
            break;
        case NEXUS_FrontendOfdmModeGuard_eAutoIsdbtJapan:
            params.eModeGuardAcquire = BTHD_ModeGuard_eAutoIsdbt;
            break;
        case NEXUS_FrontendOfdmModeGuard_eAutoIsdbtBrazil:
            params.eModeGuardAcquire = BTHD_ModeGuard_eAuto;
            break;
        case NEXUS_FrontendOfdmModeGuard_eManual:
            {
                params.eModeGuardAcquire = BTHD_ModeGuard_eManual;
                switch (pSettings->modeSettings.mode) {
                default:
                    BDBG_WRN(("Unrecognized transmission mode, defaulting to 2k"));
                    /* fall through */
                case NEXUS_FrontendOfdmTransmissionMode_e2k:
                    params.eTransmissionMode = BTHD_TransmissionMode_e2K;
                    break;
                case NEXUS_FrontendOfdmTransmissionMode_e4k:
                    params.eTransmissionMode = BTHD_TransmissionMode_e4K;
                    break;
                case NEXUS_FrontendOfdmTransmissionMode_e8k:
                    params.eTransmissionMode = BTHD_TransmissionMode_e8K;
                    break;
                }
                switch (pSettings->modeSettings.guardInterval) {
                default:
                    BDBG_WRN(("Unrecognized guardInterval, defaulting to 1_32"));
                    /* fall through */
                case NEXUS_FrontendOfdmGuardInterval_e1_32:
                    params.eGuardInterval = BTHD_GuardInterval_e1_32;
                    break;
                case NEXUS_FrontendOfdmGuardInterval_e1_16:
                    params.eGuardInterval = BTHD_GuardInterval_e1_16;
                    break;
                case NEXUS_FrontendOfdmGuardInterval_e1_8:
                    params.eGuardInterval = BTHD_GuardInterval_e1_8;
                    break;
                case NEXUS_FrontendOfdmGuardInterval_e1_4:
                    params.eGuardInterval = BTHD_GuardInterval_e1_4;
                    break;
                }
            }
            break;
        default:
            BDBG_WRN(("Unrecognized transmission mode, defaulting to auto."));
            params.eModeGuardAcquire = BTHD_ModeGuard_eAuto;
        }
    }
    else {
         switch (pSettings->mode) {
        case NEXUS_FrontendOfdmMode_eDvbt:
            params.eModeGuardAcquire = BTHD_ModeGuard_eAutoDvbt;
            break;
        case NEXUS_FrontendOfdmMode_eIsdbt:
            params.eModeGuardAcquire = BTHD_ModeGuard_eAutoIsdbt;
            break;
        default:
            BDBG_ERR(("Invalid OFDM Mode"));
            rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
        }
    }

    switch (pSettings->acquisitionMode) {
    default:
        BDBG_WRN(("Unrecognized acquisitionMode, defaulting to Auto"));
        /* fall through */
    case NEXUS_FrontendOfdmAcquisitionMode_eAuto:
        params.eAcquisitionMode = BTHD_ThdAcquisitionMode_eAuto;
        break;
    case NEXUS_FrontendOfdmAcquisitionMode_eManual:
        params.eAcquisitionMode = BTHD_ThdAcquisitionMode_eManual;
        break;
    case NEXUS_FrontendOfdmAcquisitionMode_eScan:
        params.eAcquisitionMode = BTHD_ThdAcquisitionMode_eScan;
        break;
    }
    params.bTmccAcquire = !pSettings->manualTmccSettings;
    if (pSettings->manualTmccSettings) {
        params.bIsdbtPartialReception =
            pSettings->tmccSettings.partialReception;
        params.eIsdbtAModulation = NEXUS_Frontend_P_ModulationToTHD(
            pSettings->tmccSettings.modulationA);
        params.eIsdbtACodeRate = NEXUS_Frontend_P_CodeRateToTHD(
            pSettings->tmccSettings.codeRateA);
        params.eIsdbtATimeInterleaving = NEXUS_Frontend_P_TimeInterleavingToTHD(
            pSettings->tmccSettings.timeInterleavingA);
        params.eIsdbtASegments = pSettings->tmccSettings.numSegmentsA;
        params.eIsdbtBModulation = NEXUS_Frontend_P_ModulationToTHD(
            pSettings->tmccSettings.modulationB);
        params.eIsdbtBCodeRate = NEXUS_Frontend_P_CodeRateToTHD(
            pSettings->tmccSettings.codeRateB);
        params.eIsdbtBTimeInterleaving = NEXUS_Frontend_P_TimeInterleavingToTHD(
            pSettings->tmccSettings.timeInterleavingB);
        params.eIsdbtBSegments = pSettings->tmccSettings.numSegmentsB;
        params.eIsdbtCModulation = NEXUS_Frontend_P_ModulationToTHD(
            pSettings->tmccSettings.modulationC);
        params.eIsdbtCCodeRate = NEXUS_Frontend_P_CodeRateToTHD(
            pSettings->tmccSettings.codeRateC);
        params.eIsdbtCTimeInterleaving = NEXUS_Frontend_P_TimeInterleavingToTHD(
            pSettings->tmccSettings.timeInterleavingC);
        params.eIsdbtCSegments = pSettings->tmccSettings.numSegmentsC;
    }
    params.tunerFreq = pSettings->frequency;
    params.ePullinRange =
        (NEXUS_FrontendOfdmPullInRange_eNarrow == pSettings->pullInRange) ?
            BTHD_PullInRange_eNarrow : BTHD_PullInRange_eWide;

    NEXUS_IsrCallback_Set(pDevice->thdLockAppCallback, &pSettings->lockCallback);
    NEXUS_TaskCallback_Set(pDevice->ewsAppCallback, &pSettings->ewsCallback);
    NEXUS_TaskCallback_Set(pDevice->thdAsyncStatusAppCallback, &pSettings->asyncStatusReadyCallback);

    pDevice->lastSettings.ofdm = *pSettings;

    rc = BTHD_TuneAcquire(pDevice->thdHandle, &params);
    if(rc){rc = BERR_TRACE(rc);}

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7552_RequestQamAsyncStatus( void *handle )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_InternalGainSettings settings;
    NEXUS_GainParameters params;
    NEXUS_ExternalGainSettings externalGainSettings;
    NEXUS_7552 *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7552 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);

    BKNI_Memset(&externalGainSettings, 0, sizeof(externalGainSettings));

    if(pDevice->pGenericDeviceHandle->parent){

        params.rfInput = pDevice->pGenericDeviceHandle->linkSettings.rfInput;
        params.accumulateTillRootDevice = true;
        params.frequency = pDevice->lastSettings.qam.frequency;

        BKNI_Memset(&settings, 0, sizeof(settings));

        rc = NEXUS_FrontendDevice_P_GetInternalGain(pDevice->pGenericDeviceHandle->parent, &params, &settings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        externalGainSettings.totalGain = settings.totalVariableGain;

        if(settings.isExternalFixedGainBypassed){
            externalGainSettings.totalGain -= pDevice->pGenericDeviceHandle->bypassableFixedGain;
        }
    }
    else{
        externalGainSettings.bypassableGain = pDevice->pGenericDeviceHandle->bypassableFixedGain;
    }

    externalGainSettings.totalGain += pDevice->pGenericDeviceHandle->totalFixedBoardGain;

    rc = NEXUS_FrontendDevice_P_SetExternalGain(pDevice->pGenericDeviceHandle, &externalGainSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if(!pDevice->isInternalAsyncStatusCall)
        NEXUS_TaskCallback_Fire(pDevice->adsAsyncStatusAppCallback);
    else {
        pDevice->isInternalAsyncStatusReady = true;
    }
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7552_GetQamAsyncStatus( void *handle, NEXUS_FrontendQamStatus *pStatus )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    uint64_t totalbits=0, uncorrectedBits=0;
    unsigned cleanBlock = 0, correctedBlock = 0, unCorrectedBlock = 0, totalBlock = 0;
    NEXUS_Time currentTime;
    NEXUS_PreviousStatus *prevStatus;
    NEXUS_7552 *pDevice;
    BADS_Status adsStatus;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7552 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof*(pStatus));

    rc = BADS_GetStatus(pDevice->channelHandle, &adsStatus);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    prevStatus = &pDevice->previousQamStatus;
    NEXUS_Time_Get(&currentTime);
    pStatus->fecLock = adsStatus.isFecLock;
    pStatus->receiverLock = adsStatus.isQamLock;
    pStatus->rfAgcLevel = adsStatus.agcExtLevel;
    pStatus->ifAgcLevel = adsStatus.agcIntLevel;
    pStatus->dsChannelPower = adsStatus.dsChannelPower;
    pStatus->snrEstimate = (adsStatus.snrEstimate * 100)/256;
    pStatus->fecCorrected = adsStatus.accCorrectedCount;
    pStatus->fecUncorrected = adsStatus.accUncorrectedCount;
    pStatus->carrierPhaseOffset = adsStatus.carrierPhaseOffset;
    pStatus->carrierFreqOffset = adsStatus.carrierFreqOffset;
    pStatus->symbolRate = adsStatus.rxSymbolRate;
    pStatus->symbolRateError = adsStatus.symbolRateError;
    pStatus->berEstimate = adsStatus.berRawCount;
    pStatus->mainTap = adsStatus.mainTap;
    pStatus->settings = pDevice->lastSettings.qam;
    pStatus->spectrumInverted = adsStatus.isSpectrumInverted;
    pStatus->fecClean = adsStatus.accCleanCount;
    pStatus->intAgcLevel = adsStatus.agcAGFLevel;
    pStatus->bitErrCorrected = adsStatus.correctedBits;
    pStatus->highResEqualizerGain = adsStatus.equalizerGain;
    pStatus->frontendGain = adsStatus.feGain;
    pStatus->digitalAgcGain = adsStatus.digitalAgcGain;

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

    pStatus->viterbiUncorrectedBits = adsStatus.correctedBits + (uint32_t)((uint64_t)pStatus->fecUncorrected * 11224)/1000;
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
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7552_RequestOfdmAsyncStatus( void *handle )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_InternalGainSettings settings;
    NEXUS_GainParameters params;
    NEXUS_ExternalGainSettings externalGainSettings;
    NEXUS_7552 *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7552 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);

    BKNI_Memset(&externalGainSettings, 0, sizeof(externalGainSettings));

    if(pDevice->pGenericDeviceHandle->parent){

        params.rfInput = pDevice->pGenericDeviceHandle->linkSettings.rfInput;
        params.accumulateTillRootDevice = true;
        params.frequency = pDevice->lastSettings.ofdm.frequency;

        BKNI_Memset(&settings, 0, sizeof(settings));

        rc = NEXUS_FrontendDevice_P_GetInternalGain(pDevice->pGenericDeviceHandle->parent, &params, &settings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        externalGainSettings.totalGain = settings.totalVariableGain;

        if(settings.isExternalFixedGainBypassed){
            externalGainSettings.totalGain -= pDevice->pGenericDeviceHandle->bypassableFixedGain;
        }
    }
    else{
        externalGainSettings.bypassableGain = pDevice->pGenericDeviceHandle->bypassableFixedGain;
    }

    externalGainSettings.totalGain += pDevice->pGenericDeviceHandle->totalFixedBoardGain;

    rc = NEXUS_FrontendDevice_P_SetExternalGain(pDevice->pGenericDeviceHandle, &externalGainSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}
    if(!pDevice->isInternalAsyncStatusCall)
        NEXUS_TaskCallback_Fire(pDevice->thdAsyncStatusAppCallback);
    else {
        pDevice->isInternalAsyncStatusReady = true;
    }
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7552_RequestIsdbtAsyncStatus(void *handle, NEXUS_FrontendIsdbtStatusType type)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7552 *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7552 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BSTD_UNUSED(type);

    pDevice->isAsyncStatusReady = false;
    rc = NEXUS_Frontend_P_7552_RequestOfdmAsyncStatus(pDevice);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7552_RequestDvbtAsyncStatus(void *handle, NEXUS_FrontendDvbtStatusType type)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7552 *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7552 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BSTD_UNUSED(type);

    pDevice->isAsyncStatusReady = false;
    rc = NEXUS_Frontend_P_7552_RequestOfdmAsyncStatus(pDevice);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7552_GetIsdbtAsyncStatusReady(void *handle, NEXUS_FrontendIsdbtStatusReady *pAsyncStatusReady)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7552 *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7552 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);

    if(pDevice->isAsyncStatusReady){
        pAsyncStatusReady->type[NEXUS_FrontendIsdbtStatusType_eBasic] = true;
    }
    else{
        pAsyncStatusReady->type[NEXUS_FrontendIsdbtStatusType_eBasic] = false;
    }

    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7552_GetDvbtAsyncStatusReady(void *handle, NEXUS_FrontendDvbtStatusReady *pAsyncStatusReady)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7552 *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7552 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);

    if(pDevice->isAsyncStatusReady){
        pAsyncStatusReady->type[NEXUS_FrontendDvbtStatusType_eBasic] = true;
    }
    else{
        pAsyncStatusReady->type[NEXUS_FrontendDvbtStatusType_eBasic] = false;
    }

    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7552_GetOfdmAsyncStatus( void *handle, NEXUS_FrontendOfdmStatus *pStatus )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7552 *pDevice;
    BTHD_THDStatus status;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7552 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof(NEXUS_FrontendOfdmStatus));

    rc = BTHD_GetThdStatus(pDevice->thdHandle, &status);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pStatus->settings = pDevice->lastSettings.ofdm;
    pStatus->fecLock = status.bFecLock;
    pStatus->receiverLock = status.bReceiverLock;
    pStatus->noSignalDetected = status.bNoSignalDetected;
    pStatus->spectrumInverted = status.bSpectrumInverted;
    pStatus->ews = status.bIsdbtEWS;

    if(pStatus->settings.mode == NEXUS_FrontendOfdmMode_eDvbt) {
        pStatus->modulation = NEXUS_Frontend_P_THDToModulation(status.eModulation);
        pStatus->hierarchy = NEXUS_Frontend_P_THDToHierarchy(status.eHierarchy);
        pStatus->codeRate = NEXUS_Frontend_P_THDToCodeRate(status.eCodeRate);
    }

    pStatus->transmissionMode = NEXUS_Frontend_P_THDToTransmissionMode(status.eTransmissionMode);
    pStatus->guardInterval = NEXUS_Frontend_P_THDToGuardInterval(status.eGuardInterval);

    pStatus->cellId = status.nCellId;
    pStatus->carrierOffset = status.lCarrierOffset;
    pStatus->timingOffset = status.lTimingOffset;
    pStatus->snr = (status.nSnr*100)/256;
    pStatus->fecCorrectedBlocks = status.ulRsCorrectedBlocks;
    pStatus->fecUncorrectedBlocks = status.ulRsUncorrectedBlocks;
    pStatus->fecCleanBlocks = status.ulRsCleanBlocks;
    pStatus->reacquireCount = status.ulReacqCount;
    pStatus->viterbiErrorRate = status.ulViterbiBer;
    pStatus->preViterbiErrorRate = status.ulPreViterbiBer;
    pStatus->ifAgcLevel = status.ulIFAgc;
    pStatus->rfAgcLevel = status.ulRFAgc;

#if TODO_THD_TUNER_CONNECTION
    if (pDevice->tuner) {
        NEXUS_7552TunerStatus tunerStatus;
        NEXUS_Tuner_Get7552Status(pDevice->tuner, &tunerStatus);
        pStatus->rfAgcLevel = tunerStatus.rfAgcLevel;
        pStatus->signalStrength = tunerStatus.signalStrength;
    }
#endif
    pStatus->rfAgcLevel = status.ulRFAgc;
    pStatus->signalStrength = status.nSignalStrength;

    if(pStatus->settings.mode == NEXUS_FrontendOfdmMode_eIsdbt) {
        pStatus->signalLevelPercentA = status.signalLevelAPercent;
        pStatus->signalLevelPercentB = status.signalLevelBPercent;
        pStatus->signalLevelPercentC = status.signalLevelCPercent;
        pStatus->signalQualityPercentA = status.signalQualityAPercent;
        pStatus->signalQualityPercentB = status.signalQualityBPercent;
        pStatus->signalQualityPercentC = status.signalQualityBPercent;
        pStatus->fecCorrectedBlocksA = status.ulIsdbtARsCorrectedBlocks;
        pStatus->fecUncorrectedBlocksA = status.ulIsdbtARsUncorrectedBlocks;
        pStatus->fecCleanBlocksA = status.ulIsdbtARsCleanBlocks;
        pStatus->fecCorrectedBlocksB = status.ulIsdbtBRsCorrectedBlocks;
        pStatus->fecUncorrectedBlocksB = status.ulIsdbtBRsUncorrectedBlocks;
        pStatus->fecCleanBlocksB = status.ulIsdbtBRsCleanBlocks;
        pStatus->fecCorrectedBlocksC = status.ulIsdbtCRsCorrectedBlocks;
        pStatus->fecUncorrectedBlocksC = status.ulIsdbtCRsUncorrectedBlocks;
        pStatus->fecCleanBlocksC = status.ulIsdbtCRsCleanBlocks;
        pStatus->isdbtAPreRS = status.ulIsdbtAViterbiUncorrectedBits;
        pStatus->isdbtBPreRS = status.ulIsdbtBViterbiUncorrectedBits;
        pStatus->isdbtCPreRS = status.ulIsdbtCViterbiUncorrectedBits;
        pStatus->modulationA =NEXUS_Frontend_P_THDToModulation(status.eIsdbtAModulation);
        pStatus->codeRateA = NEXUS_Frontend_P_THDToCodeRate(status.eIsdbtACodeRate);
        pStatus->timeInterleavingA = NEXUS_Frontend_P_THDToTimeInterleaving(status.eIsdbtATimeInterleaving);
        pStatus->numSegmentsA = status.eIsdbtASegments;
        pStatus->fecCorrectedBlocksA = status.ulIsdbtARsCorrectedBlocks;
        pStatus->fecUncorrectedBlocksA = status.ulIsdbtARsUncorrectedBlocks;
        pStatus->fecCleanBlocksA = status.ulIsdbtARsCleanBlocks;
        pStatus->modulationB = NEXUS_Frontend_P_THDToModulation(status.eIsdbtBModulation);
        pStatus->codeRateB = NEXUS_Frontend_P_THDToCodeRate(status.eIsdbtBCodeRate);
        pStatus->timeInterleavingB = NEXUS_Frontend_P_THDToTimeInterleaving(status.eIsdbtBTimeInterleaving);
        pStatus->numSegmentsB = status.eIsdbtBSegments;
        pStatus->fecCorrectedBlocksB = status.ulIsdbtBRsCorrectedBlocks;
        pStatus->fecUncorrectedBlocksB = status.ulIsdbtBRsUncorrectedBlocks;
        pStatus->fecCleanBlocksB = status.ulIsdbtBRsCleanBlocks;
        pStatus->modulationC = NEXUS_Frontend_P_THDToModulation(status.eIsdbtCModulation);
        pStatus->codeRateC = NEXUS_Frontend_P_THDToCodeRate(status.eIsdbtCCodeRate);
        pStatus->timeInterleavingC = NEXUS_Frontend_P_THDToTimeInterleaving(status.eIsdbtCTimeInterleaving);
        pStatus->numSegmentsC = status.eIsdbtCSegments;
        pStatus->fecCorrectedBlocksC = status.ulIsdbtCRsCorrectedBlocks;
        pStatus->fecUncorrectedBlocksC = status.ulIsdbtCRsUncorrectedBlocks;
        pStatus->fecCleanBlocksC = status.ulIsdbtCRsCleanBlocks;
    }
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7552_GetIsdbtAsyncStatus(void *handle, NEXUS_FrontendIsdbtStatusType type, NEXUS_FrontendIsdbtStatus *pStatus)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BTHD_THDStatus status;
    NEXUS_7552 *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7552 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    pStatus->status.basic.settings = pDevice->lastSettings.ofdm;

    if(type == NEXUS_FrontendIsdbtStatusType_eBasic){
        rc = BTHD_GetThdStatus(pDevice->thdHandle, &status);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        pStatus->status.basic.fecLock = status.bFecLock;
        pStatus->status.basic.tmcc.transmissionMode = NEXUS_Frontend_P_THDToTransmissionMode(status.eTransmissionMode);
        pStatus->status.basic.tmcc.guardInterval = NEXUS_Frontend_P_THDToGuardInterval(status.eGuardInterval);
        pStatus->status.basic.tmcc.ews = status.bIsdbtEWS;
        pStatus->status.basic.tmcc.partialReception = status.bIsdbtPartialReception;
        pStatus->status.basic.carrierOffset = status.lCarrierOffset;
        pStatus->status.basic.timingOffset = status.lTimingOffset;
        pStatus->status.basic.signalStrength = status.nSignalStrength/10;
        pStatus->status.basic.snr = status.nSnr*100/256;
        pStatus->status.basic.spectrumInverted = status.bSpectrumInverted;
        pStatus->status.basic.reacquireCount = status.ulReacqCount;


        pStatus->status.basic.layerA.modulation = NEXUS_Frontend_P_THDToModulation(status.eIsdbtAModulation);
        pStatus->status.basic.layerA.codeRate = NEXUS_Frontend_P_THDToCodeRate(status.eIsdbtACodeRate);
        pStatus->status.basic.layerA.timeInterleaving = NEXUS_Frontend_P_THDToTimeInterleaving(status.eIsdbtATimeInterleaving);
        pStatus->status.basic.layerA.numSegments = status.eIsdbtASegments;
        pStatus->status.basic.layerA.fecBlockCounts.corrected = status.ulIsdbtARsCorrectedBlocks;
        pStatus->status.basic.layerA.fecBlockCounts.uncorrected= status.ulIsdbtARsUncorrectedBlocks;
        pStatus->status.basic.layerA.fecBlockCounts.clean = status.ulIsdbtARsCleanBlocks;
        pStatus->status.basic.layerA.signalLevelPercent = status.signalLevelAPercent;
        pStatus->status.basic.layerA.signalQualityPercent = status.signalQualityAPercent;

        pStatus->status.basic.layerB.modulation = NEXUS_Frontend_P_THDToModulation(status.eIsdbtBModulation);
        pStatus->status.basic.layerB.codeRate = NEXUS_Frontend_P_THDToCodeRate(status.eIsdbtBCodeRate);
        pStatus->status.basic.layerB.timeInterleaving = NEXUS_Frontend_P_THDToTimeInterleaving(status.eIsdbtBTimeInterleaving);
        pStatus->status.basic.layerB.numSegments = status.eIsdbtBSegments;
        pStatus->status.basic.layerB.fecBlockCounts.corrected = status.ulIsdbtBRsCorrectedBlocks;
        pStatus->status.basic.layerB.fecBlockCounts.uncorrected = status.ulIsdbtBRsUncorrectedBlocks;
        pStatus->status.basic.layerB.fecBlockCounts.clean = status.ulIsdbtBRsCleanBlocks;
        pStatus->status.basic.layerB.signalLevelPercent = status.signalLevelBPercent;
        pStatus->status.basic.layerB.signalQualityPercent = status.signalQualityBPercent;

        pStatus->status.basic.layerC.modulation = NEXUS_Frontend_P_THDToModulation(status.eIsdbtCModulation);
        pStatus->status.basic.layerC.codeRate = NEXUS_Frontend_P_THDToCodeRate(status.eIsdbtCCodeRate);
        pStatus->status.basic.layerC.timeInterleaving = NEXUS_Frontend_P_THDToTimeInterleaving(status.eIsdbtCTimeInterleaving);
        pStatus->status.basic.layerC.numSegments = status.eIsdbtCSegments;
        pStatus->status.basic.layerC.fecBlockCounts.corrected = status.ulIsdbtCRsCorrectedBlocks;
        pStatus->status.basic.layerC.fecBlockCounts.uncorrected = status.ulIsdbtCRsUncorrectedBlocks;
        pStatus->status.basic.layerC.fecBlockCounts.clean = status.ulIsdbtCRsCleanBlocks;
        pStatus->status.basic.layerC.signalLevelPercent = status.signalLevelCPercent;
        pStatus->status.basic.layerC.signalQualityPercent = status.signalQualityCPercent;
    }

    pDevice->isAsyncStatusReady = false;
done:
    return rc;

}

static NEXUS_Error NEXUS_Frontend_P_7552_GetDvbtAsyncStatus(void *handle, NEXUS_FrontendDvbtStatusType type, NEXUS_FrontendDvbtStatus *pStatus)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BTHD_THDStatus status;
    NEXUS_7552 *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7552 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    BKNI_Memset(&status, 0, sizeof(status));
    pStatus->status.basic.settings = pDevice->lastSettings.ofdm;

    if(type == NEXUS_FrontendDvbtStatusType_eBasic){
        rc = BTHD_GetThdStatus(pDevice->thdHandle,  &status);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        pStatus->status.basic.fecLock = status.bFecLock;
        pStatus->status.basic.spectrumInverted = status.bSpectrumInverted;
        pStatus->status.basic.snr = status.nSnr*100/256;
        pStatus->status.basic.carrierOffset = status.lCarrierOffset;
        pStatus->status.basic.timingOffset = status.lTimingOffset;
        pStatus->status.basic.gainOffset = status.lgainOffset;
        pStatus->status.basic.signalStrength = status.nSignalStrength/10;
        pStatus->status.basic.signalLevelPercent = status.signalLevelPercent;
        pStatus->status.basic.signalQualityPercent = status.signalQualityPercent;
        pStatus->status.basic.reacquireCount = status.ulReacqCount;
        pStatus->status.basic.viterbiErrorRate.rate = status.ulViterbiBer;

        pStatus->status.basic.tps.modulation = NEXUS_Frontend_P_THDToModulation(status.eModulation);
        pStatus->status.basic.tps.transmissionMode = NEXUS_Frontend_P_THDToTransmissionMode(status.eTransmissionMode);
        pStatus->status.basic.tps.guardInterval = NEXUS_Frontend_P_THDToGuardInterval(status.eGuardInterval);
        pStatus->status.basic.tps.codeRate = NEXUS_Frontend_P_THDToCodeRate(status.eCodeRate);
        pStatus->status.basic.tps.hierarchy = NEXUS_Frontend_P_THDToHierarchy(status.eHierarchy);
        pStatus->status.basic.tps.cellId = status.nCellId;
        pStatus->status.basic.tps.inDepthSymbolInterleave = status.inDepthSymbolInterleave;
        pStatus->status.basic.tps.timeSlicing = status.timeSlicing;
        pStatus->status.basic.tps.mpeFec = status.mpeFec;

        pStatus->status.basic.fecBlockCounts.corrected = status.ulRsCorrectedBlocks;
        pStatus->status.basic.fecBlockCounts.uncorrected = status.ulRsUncorrectedBlocks;
        pStatus->status.basic.fecBlockCounts.clean = status.ulRsCleanBlocks;
    }

    pDevice->isAsyncStatusReady = false;
done:
    return rc;

}

static NEXUS_Error NEXUS_Frontend_P_7552_GetOfdmStatus( void *handle, NEXUS_FrontendOfdmStatus *pStatus )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned j=0;
    NEXUS_7552 *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7552 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof*(pStatus));

    pDevice->isInternalAsyncStatusCall = true;
    pDevice->isInternalAsyncStatusReady = false;

    rc = NEXUS_Frontend_P_7552_RequestOfdmAsyncStatus(pDevice);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    for(j=0; j < 10; j++) {

        BKNI_Sleep(10);
        if(pDevice->isInternalAsyncStatusReady) {
            rc = NEXUS_Frontend_P_7552_GetOfdmAsyncStatus(pDevice, pStatus);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            pDevice->isInternalAsyncStatusReady = false;
            break;
        }
    }
    pDevice->isInternalAsyncStatusCall = false;
done:
    return rc;
}


static BTHD_IsdbtTimeInterleaving NEXUS_Frontend_P_TimeInterleavingToTHD( NEXUS_FrontendOfdmTimeInterleaving nexus )
{
    switch (nexus) {
    default:
        BDBG_WRN(("Unknown NEXUS time interleaving %d, defaulting to 0x",
            nexus));
        /* fall-through */
    case NEXUS_FrontendOfdmTimeInterleaving_e0x:
        return BTHD_IsdbtTimeInterleaving_0X;
        break;
    case NEXUS_FrontendOfdmTimeInterleaving_e1x:
        return BTHD_IsdbtTimeInterleaving_1X;
        break;
    case NEXUS_FrontendOfdmTimeInterleaving_e2x:
        return BTHD_IsdbtTimeInterleaving_2X;
        break;
    case NEXUS_FrontendOfdmTimeInterleaving_e3x:
        return BTHD_IsdbtTimeInterleaving_3X;
        break;
    }
}

static NEXUS_FrontendOfdmTimeInterleaving  NEXUS_Frontend_P_THDToTimeInterleaving(  BTHD_IsdbtTimeInterleaving magnum )
{
    switch (magnum) {
    default:
        BDBG_WRN(("Unknown Magnum time interleaving %d, defaulting to 0x", magnum));
        /* fall-through */
    case BTHD_IsdbtTimeInterleaving_0X:
        return NEXUS_FrontendOfdmTimeInterleaving_e0x;
    case BTHD_IsdbtTimeInterleaving_1X:
        return NEXUS_FrontendOfdmTimeInterleaving_e1x;
    case BTHD_IsdbtTimeInterleaving_2X:
        return NEXUS_FrontendOfdmTimeInterleaving_e2x;
    case BTHD_IsdbtTimeInterleaving_3X:
        return NEXUS_FrontendOfdmTimeInterleaving_e3x;
    }
}

static BTHD_CodeRate NEXUS_Frontend_P_CodeRateToTHD(  NEXUS_FrontendOfdmCodeRate nexusCodeRate )
{
    switch (nexusCodeRate) {
    default:
        BDBG_WRN(("Unsupported code rate value %d, defaulting to 1_2", nexusCodeRate));
        /* Fall through */
    case NEXUS_FrontendOfdmCodeRate_e1_2:
        return BTHD_CodeRate_e1_2;
    case NEXUS_FrontendOfdmCodeRate_e2_3:
        return BTHD_CodeRate_e2_3;
    case NEXUS_FrontendOfdmCodeRate_e3_4:
        return BTHD_CodeRate_e3_4;
    case NEXUS_FrontendOfdmCodeRate_e5_6:
        return BTHD_CodeRate_e5_6;
    case NEXUS_FrontendOfdmCodeRate_e7_8:
        return BTHD_CodeRate_e7_8;
    }
}

static NEXUS_FrontendOfdmCodeRate NEXUS_Frontend_P_THDToCodeRate( BTHD_CodeRate magnum )
{
    switch (magnum) {
    case BTHD_CodeRate_e1_2:
        return NEXUS_FrontendOfdmCodeRate_e1_2;
    case BTHD_CodeRate_e2_3:
        return NEXUS_FrontendOfdmCodeRate_e2_3;
    case BTHD_CodeRate_e3_4:
        return NEXUS_FrontendOfdmCodeRate_e3_4;
    case BTHD_CodeRate_e5_6:
        return NEXUS_FrontendOfdmCodeRate_e5_6;
    case BTHD_CodeRate_e7_8:
        return NEXUS_FrontendOfdmCodeRate_e7_8;
    default:
        return NEXUS_FrontendOfdmCodeRate_eMax;
    }
}

static BTHD_Modulation NEXUS_Frontend_P_ModulationToTHD( NEXUS_FrontendOfdmModulation nexus )
{
    switch (nexus) {
    default:
        BDBG_WRN(("Urecognized NEXUS modulation %d, defaulting to QAM64", nexus));
        /* fall through */
    case NEXUS_FrontendOfdmModulation_eQam64:
        return BTHD_Modulation_e64Qam;
    case NEXUS_FrontendOfdmModulation_eQam16:
        return BTHD_Modulation_e16Qam;
    case NEXUS_FrontendOfdmModulation_eQpsk:
        return BTHD_Modulation_eQpsk;
    }
}

static NEXUS_FrontendOfdmModulation NEXUS_Frontend_P_THDToModulation( BTHD_Modulation magnum )
{
    switch (magnum) {
    case BTHD_Modulation_e64Qam:
        return NEXUS_FrontendOfdmModulation_eQam64;
    case BTHD_Modulation_e16Qam:
        return NEXUS_FrontendOfdmModulation_eQam16;
    default:
        BDBG_WRN(("Unrecognized ofdm modulation %d, defaulting to QPSK", magnum));
        /* fall-through */
    case BTHD_Modulation_eQpsk:
        return NEXUS_FrontendOfdmModulation_eQpsk;
    }
}
#ifndef NEXUS_FRONTEND_7552_A0

static BAOB_ModulationType NEXUS_Frontend_P_ModulationToAOB( NEXUS_FrontendOutOfBandMode modeType )
{
    switch (modeType) {
    default:
        BDBG_WRN(("Urecognized out of band modulation."));
        /* fall through */
    case NEXUS_FrontendOutOfBandMode_eDvs167Qpsk:
        return BAOB_ModulationType_eDvs167Qpsk;
    case NEXUS_FrontendOutOfBandMode_eDvs178Qpsk:
        return BAOB_ModulationType_eDvs178Qpsk;
    case NEXUS_FrontendOutOfBandMode_ePod_Dvs167Qpsk:
        return BAOB_ModulationType_ePod_Dvs167Qpsk;
    case NEXUS_FrontendOutOfBandMode_ePod_Dvs178Qpsk:
        return BAOB_ModulationType_ePod_Dvs178Qpsk;
    }
}

static NEXUS_FrontendOfdmTransmissionMode  NEXUS_Frontend_P_THDToTransmissionMode( BTHD_TransmissionMode magnum )
{
    switch (magnum) {
    default:
        BDBG_WRN(("Unrecognized transmission mode (%d) reported by BTHD", magnum));
        /* fall-through */
    case BTHD_TransmissionMode_e2K:
        return NEXUS_FrontendOfdmTransmissionMode_e2k;
        break;
    case BTHD_TransmissionMode_e4K:
        return NEXUS_FrontendOfdmTransmissionMode_e4k;
        break;
    case BTHD_TransmissionMode_e8K:
        return NEXUS_FrontendOfdmTransmissionMode_e8k;
        break;
    }
}

static NEXUS_FrontendOfdmGuardInterval NEXUS_Frontend_P_THDToGuardInterval(BTHD_GuardInterval magnum)
{
    switch ( magnum )
    {
    default:
        BDBG_WRN(("Unrecognized guard interval (%d) reported by BTHD", magnum));
        /* fall-through */
    case BTHD_GuardInterval_e1_4:
        return NEXUS_FrontendOfdmGuardInterval_e1_4;
    case BTHD_GuardInterval_e1_8:
        return NEXUS_FrontendOfdmGuardInterval_e1_8;
    case BTHD_GuardInterval_e1_16:
        return NEXUS_FrontendOfdmGuardInterval_e1_16;
    case BTHD_GuardInterval_e1_32:
        return NEXUS_FrontendOfdmGuardInterval_e1_32;
    }
}

static NEXUS_FrontendOfdmHierarchy NEXUS_Frontend_P_THDToHierarchy(BTHD_Hierarchy magnum)
{
    switch ( magnum )
    {
    default:
        BDBG_WRN(("Unrecognized hierarchy (%d) reported by BTHD", magnum));
        /* fall-through */
    case BTHD_Hierarchy_0:
        return NEXUS_FrontendOfdmHierarchy_e0;
    case BTHD_Hierarchy_1:
        return NEXUS_FrontendOfdmHierarchy_e1;
    case BTHD_Hierarchy_2:
        return NEXUS_FrontendOfdmHierarchy_e2;
    case BTHD_Hierarchy_4:
        return NEXUS_FrontendOfdmHierarchy_e4;
    }
}

static NEXUS_Error NEXUS_Frontend_P_7552_TuneOob( void *handle, const NEXUS_FrontendOutOfBandSettings *pSettings )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7552 *pDevice;
    BTNR_PowerSaverSettings pwrSettings;
    BAOB_AcquireParam obParams;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7552 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BDBG_ASSERT(NULL != pSettings);

    if (pDevice->oobTnrHandle)
    {
        /* TODO: we should be able to call BTNR_SetSettings after BTNR_SetPowerSaver. verify */
        pwrSettings.enable = false;
        rc = BTNR_SetPowerSaver(pDevice->oobTnrHandle, &pwrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = BAOB_DisablePowerSaver(pDevice->oobHandle);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = BTNR_SetTunerRfFreq(pDevice->oobTnrHandle, pSettings->frequency, BTNR_TunerMode_eDigital);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    obParams.autoAcquire = pSettings->autoAcquire;
    obParams.modType = NEXUS_Frontend_P_ModulationToAOB(pSettings->mode);
    obParams.symbolRate = pSettings->symbolRate;
    obParams.berSrc = pSettings->bertSource;
    obParams.spectrum = pSettings->spectrum;

    rc = BAOB_Acquire(pDevice->oobHandle, &obParams);
    if(rc){rc = BERR_TRACE(rc);}

done:
    return rc;
}
#endif

void NEXUS_FrontendDevice_GetDefault7552Settings(NEXUS_FrontendDevice7552Settings *pSettings)
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

void NEXUS_FrontendDevice_Get7552Settings(NEXUS_FrontendDeviceHandle handle, NEXUS_FrontendDevice7552Settings *pSettings)
{
    NEXUS_7552 *pDevice = NULL;
    BDBG_ASSERT(handle != NULL);

    pDevice = handle->pDevice;
    if (!pDevice) { BERR_TRACE(NEXUS_INVALID_PARAMETER); goto done; }
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);

    pSettings->rfInput = pDevice->rfInput;
    pSettings->enableRfLoopThrough = pDevice->enableRfLoopThrough;
done:
    return;
}

NEXUS_Error NEXUS_FrontendDevice_Set7552Settings(NEXUS_FrontendDeviceHandle handle, const NEXUS_FrontendDevice7552Settings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7552 *pDevice = NULL;
    BDBG_ASSERT(handle != NULL);

    pDevice = handle->pDevice;
    if (!pDevice) { rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto done; }
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);

    if((pSettings->enableRfLoopThrough != pDevice->enableRfLoopThrough)) {
        rc = BTNR_3x7x_Set_RF_LoopThrough(pDevice->tnrHandle, pSettings->enableRfLoopThrough);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pDevice->enableRfLoopThrough = pSettings->enableRfLoopThrough;
    }
    if((pSettings->rfInput != pDevice->rfInput)) {
        rc = BTNR_3x7x_SetRfInputMode(pDevice->tnrHandle, pSettings->rfInput);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pDevice->rfInput = pSettings->rfInput;
    }

done:
    return rc;
}

/***************************************************************************
Summary:
  Probe to see if a BCM7552 device exists with the specified settings

Description:
  Probe to see if a BCM7552 device exists with the specified settings

See Also:
    NEXUS_Frontend_Open7552
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Probe7552(const NEXUS_FrontendDevice7552OpenSettings *pSettings, NEXUS_7552ProbeResults *pResults)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BCHP_Info info;

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pResults);

    BCHP_GetInfo(g_pCoreHandles->chp, &info);
    pResults->chip.id = info.productId;
    pResults->chip.familyId = info.familyId;
    info.rev += 0x10; /* convert to nexus revision */
    pResults->chip.version.major = info.rev >> 4;
    pResults->chip.version.minor = info.rev & 0xF;

    return rc;
}
