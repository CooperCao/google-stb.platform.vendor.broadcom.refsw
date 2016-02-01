/***************************************************************************
*     (c) 2011 Broadcom Corporation
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
* Module Description:
*       The frontend_7552 module defines the common functions in signal
*   demodulation process, including opening or closing a frontend_7552 device,
*   acquire a DVB-T or ISDB-T signal, getting the status of the acquired
*   channel, setting the inband transport output interface settings, detecting
*   certain type of signal, install signal lock callback, install signal
*   unlock callback, and etc.
*
* Revision History:
*
* $brcm_Log: $
* 
**************************************************************************/
#include "nexus_frontend_7552_priv.h"

/***************************************************************************
 * file scope functions
 ***************************************************************************/
static BTHD_IsdbtTimeInterleaving NEXUS_Frontend_P_TimeInterleavingToTHD(NEXUS_FrontendOfdmTimeInterleaving nexus);
static NEXUS_FrontendOfdmTimeInterleaving NEXUS_Frontend_P_THDToTimeInterleaving(BTHD_IsdbtTimeInterleaving magnum);
static BTHD_CodeRate NEXUS_Frontend_P_CodeRateToTHD(NEXUS_FrontendOfdmCodeRate nexusCodeRate);
static NEXUS_FrontendOfdmCodeRate NEXUS_Frontend_P_THDToCodeRate(BTHD_CodeRate magnum);
static BTHD_Modulation NEXUS_Frontend_P_ModulationToTHD(NEXUS_FrontendOfdmModulation nexus);
static NEXUS_FrontendOfdmModulation NEXUS_Frontend_P_THDToModulation(BTHD_Modulation magnum);
static NEXUS_FrontendLockStatus  NEXUS_Frontend_P_GetLockStatus(unsigned lockStatus);

/***************************************************************************
 * Module callback functions for tuning
 ***************************************************************************/
static void NEXUS_Frontend_P_7552_Close(NEXUS_FrontendHandle handle);
static void NEXUS_Frontend_P_7552_Untune(void *handle);
static NEXUS_Error NEXUS_Frontend_P_7552_TuneOfdm(void *handle, const NEXUS_FrontendOfdmSettings *pSettings);
static NEXUS_Error NEXUS_Frontend_P_7552_GetOfdmStatus(void *handle, NEXUS_FrontendOfdmStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_7552_GetSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length);
static void NEXUS_Frontend_P_7552_ResetStatus(void *handle);
static NEXUS_Error NEXUS_Frontend_P_7552_ScanFrequency(void *handle, const NEXUS_FrontendScanSettings *pSettings, NEXUS_FrontendScanResults *pResults);
static void        NEXUS_Frontend_P_7552_GetScanThresholds(void *handle, NEXUS_FrontendScanThresholds *pThresholds);
static NEXUS_Error NEXUS_Frontend_P_7552_SetScanThresholds(void *handle, const NEXUS_FrontendScanThresholds *pThresholds);
static void NEXUS_Frontend_P_7552_Tuner_isr(void *pParam);
static NEXUS_Error NEXUS_Frontend_P_7552_GetFastStatus(void *handle, NEXUS_FrontendFastStatus *pStatus);

/***************************************************************************
Summary:
    ISR Event Handler for a 7552 device
 ***************************************************************************/
static void NEXUS_Frontend_P_7552_IsrEvent(void *pParam)
{
    NEXUS_7552 *pDevice = pParam;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);

    BDBG_MSG(("Calling BTHD_ProcessInterruptEvent"));
    BTHD_ProcessInterruptEvent(pDevice->thdHandle);
}

static void NEXUS_Frontend_P_7552_BbsEvent(void *pParam)
{
    NEXUS_7552 *pDevice = pParam;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);

    BDBG_MSG(("Calling BTHD_ProcessBBSInterruptEvent"));
    BTHD_ProcessBBSInterruptEvent(pDevice->thdHandle);
}

static void NEXUS_Frontend_P_7552_Tuner_isr(void *pParam)
{
    /* NOTE: Even though the pDevice is passed in as the pParam, NEXUS_Frontend_P_7552_Tuner_isr gets called with BTHD_P_ThdCallbackData_t  as its pParam. */
    BTHD_P_ThdCallbackData_t *pCallback = (BTHD_P_ThdCallbackData_t *)pParam;
    BTNR_3x7x_RfStatus_t RfCallbackStatus;
    NEXUS_7552 *pDevice = (NEXUS_7552 *) pCallback->hTunerChn;

    if (NULL == pCallback) return;

    if (BTHD_CallbackMode_eSetMode == pCallback->Mode) {
        /* TODO: call the tuner with pCallback->Freq_Offset */
        BTNR_3x7x_Set_RF_Offset( pDevice->tnrHandle, pCallback->Freq_Offset, pCallback->Symbol_Rate);
              
    } else {
        /* magic numbers from hardware team */

        BTNR_3x7x_Get_RF_Status(pDevice->tnrHandle, &RfCallbackStatus);
        pCallback->RF_Freq = RfCallbackStatus.RF_Freq;
        pCallback->Total_Mix_After_ADC = RfCallbackStatus.Total_Mix_After_ADC;
        pCallback->PreADC_Gain_x256db  = RfCallbackStatus.PreADC_Gain_x256db;
        pCallback->PostADC_Gain_x256db = RfCallbackStatus.PostADC_Gain_x256db;
        pCallback->External_Gain_x256db = RfCallbackStatus.External_Gain_x256db;   
                        
    }
}

/***************************************************************************
Summary:
    Lock callback handler for a 7552 device
 ***************************************************************************/
static void NEXUS_Frontend_P_7552_LockEvent(void *pParam)
{
    NEXUS_7552 *pDevice = pParam;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BDBG_MSG(("OFDM lock event set"));
    NEXUS_TaskCallback_Fire(pDevice->thdLockAppCallback);
}

static void NEXUS_Frontend_P_7552_EwsEvent(void *pParam)
{
    NEXUS_7552 *pDevice = pParam;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BDBG_MSG(("OFDM EWS event set"));
    NEXUS_TaskCallback_Fire(pDevice->ewsAppCallback);
}

static void NEXUS_Frontend_P_7552_ChannelScanResult(void *pParam, int param)
{
    NEXUS_7552 *pDevice = pParam;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BSTD_UNUSED(param);
}

/***************************************************************************
Summary:
    Get the default settings for a BCM7552 Frontend
See Also:
    NEXUS_Frontend_Open7552
 ***************************************************************************/
void NEXUS_Frontend_GetDefault7552OfdmSettings(
    NEXUS_7552OfdmFrontendSettings *pSettings   /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->supportIsdbt = true;
}

/***************************************************************************
Summary:
    Open a handle to a BCM7552 device.
 ***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_Open7552Ofdm(
    const NEXUS_7552OfdmFrontendSettings *pSettings
    )
{
    NEXUS_Error errCode;
    BTHD_Settings thdSettings;
    NEXUS_FrontendHandle frontendHandle = NULL;
    NEXUS_7552 *pDevice=NULL;
    BDBG_ASSERT(NULL != pSettings);
    
    errCode = NEXUS_Frontend_P_Open7552();
    if ( errCode != BERR_SUCCESS ) goto err_scan_event;

    pDevice = p_7552device;
    
    BTHD_3x7x_GetDefaultSettings(&thdSettings);
    thdSettings.hTmr = g_pCoreHandles->tmr;
    thdSettings.hHeap = g_pCoreHandles->heap[0];
    thdSettings.supportIsdbt = pSettings->supportIsdbt;

    /* Open THD device */
    errCode = BTHD_Open(&pDevice->thdHandle, g_pCoreHandles->chp, (void*)g_pCoreHandles->reg, g_pCoreHandles->bint, &thdSettings);
    if ( errCode != BERR_SUCCESS ) goto err_scan_event;

    /* Initialize Acquisition Processor */
    BDBG_MSG(("Initializating OFDM core..."));
    errCode = BTHD_Init(pDevice->thdHandle, NULL, 0);
    if ( errCode != BERR_SUCCESS ) goto err_open_thd;
    BDBG_MSG(("Initializating OFDM core... Done"));

    /* Get events and register callbacks */
    errCode = BTHD_GetInterruptEventHandle(pDevice->thdHandle, &pDevice->thdIsrEvent);
    if ( errCode != BERR_SUCCESS ) goto err_open_thd;

    pDevice->thdIsrEventCallback = NEXUS_RegisterEvent(pDevice->thdIsrEvent, NEXUS_Frontend_P_7552_IsrEvent, pDevice);
    if ( errCode != BERR_SUCCESS ) goto err_open_thd;

    errCode = BTHD_GetLockStateChangeEventHandle(pDevice->thdHandle, &pDevice->thdLockEvent);
    if ( errCode != BERR_SUCCESS ) goto err_isr_callback;

    pDevice->thdLockEventCallback = NEXUS_RegisterEvent(pDevice->thdLockEvent, NEXUS_Frontend_P_7552_LockEvent, pDevice);
    if ( errCode != BERR_SUCCESS ) goto err_scan_event;

    errCode = BTHD_GetEWSEventHandle(pDevice->thdHandle, &pDevice->ewsEvent);
    if ( errCode != BERR_SUCCESS ) goto err_lock_callback;

    pDevice->ewsEventCallback = NEXUS_RegisterEvent(pDevice->ewsEvent, NEXUS_Frontend_P_7552_EwsEvent, pDevice);
    if ( errCode != BERR_SUCCESS ) goto err_lock_callback;

    errCode = BTHD_GetBBSInterruptEventHandle(pDevice->thdHandle, &pDevice->bbsEvent);
    if ( errCode != BERR_SUCCESS ) goto err_ews_callback;

    pDevice->bbsEventCallback = NEXUS_RegisterEvent(pDevice->bbsEvent, NEXUS_Frontend_P_7552_BbsEvent, pDevice);
    if ( errCode != BERR_SUCCESS ) goto err_ews_callback;

    /* NOTE: Even though the pDevice is passed in as the pParam, NEXUS_Frontend_P_7552_Tuner_isr gets called with BTHD_P_ThdCallbackData_t  as its pParam. */
    errCode = BTHD_InstallCallback(pDevice->thdHandle, BTHD_Callback_eTuner, (BADS_CallbackFunc)NEXUS_Frontend_P_7552_Tuner_isr, (void*)pDevice);
    if ( errCode != BERR_SUCCESS ) goto err_ews_callback;

    /* Create a Nexus frontend handle */
    frontendHandle = NEXUS_Frontend_P_Create(pDevice);
    if ( errCode != BERR_SUCCESS ) goto err_isdbtMemory;

    pDevice->frontendcount++;

    pDevice->thdLockAppCallback = NEXUS_TaskCallback_Create(frontendHandle, NULL);
    if ( errCode != BERR_SUCCESS ) goto err_frontendhandle;

    pDevice->ewsAppCallback = NEXUS_TaskCallback_Create(frontendHandle, NULL);
    if ( errCode != BERR_SUCCESS ) goto err_lock_app_callback;
    
    /* Establish device capabilities */
    frontendHandle->capabilities.ofdm = true;
#if 0
    frontendHandle->capabilities.scan = true;
#endif
    /* DVB-T is supported, but ISDB-T is supported only when it is requested */
    frontendHandle->capabilities.ofdmModes[NEXUS_FrontendOfdmMode_eDvbt] = true;
    frontendHandle->capabilities.ofdmModes[NEXUS_FrontendOfdmMode_eIsdbt] = pSettings->supportIsdbt;

    /* Bind required callbacks */
    frontendHandle->close = NEXUS_Frontend_P_7552_Close;
    frontendHandle->untune = NEXUS_Frontend_P_7552_Untune;
    frontendHandle->tuneOfdm = NEXUS_Frontend_P_7552_TuneOfdm;
    frontendHandle->getOfdmStatus = NEXUS_Frontend_P_7552_GetOfdmStatus;
    frontendHandle->getSoftDecisions = NEXUS_Frontend_P_7552_GetSoftDecisions;
    frontendHandle->resetStatus = NEXUS_Frontend_P_7552_ResetStatus;
    frontendHandle->scanFrequency = NEXUS_Frontend_P_7552_ScanFrequency;
    frontendHandle->setScanThresholds =  NEXUS_Frontend_P_7552_SetScanThresholds;
    frontendHandle->getScanThresholds = NEXUS_Frontend_P_7552_GetScanThresholds;
    frontendHandle->getFastStatus = NEXUS_Frontend_P_7552_GetFastStatus;

    /* Success */
    pDevice->ofdmSettings = *pSettings;
    pDevice->currentMode = UNKNOWN_7552;
    return frontendHandle;

err_lock_app_callback:
    NEXUS_TaskCallback_Destroy(pDevice->thdLockAppCallback);
err_frontendhandle:
    NEXUS_Frontend_P_Destroy(frontendHandle);
err_isdbtMemory:
    NEXUS_UnregisterEvent(pDevice->bbsEventCallback);
err_ews_callback:
    NEXUS_UnregisterEvent(pDevice->ewsEventCallback);
err_lock_callback:
    NEXUS_UnregisterEvent(pDevice->thdLockEventCallback);
err_isr_callback:
    NEXUS_UnregisterEvent(pDevice->thdIsrEventCallback);
err_open_thd:
    BTHD_Close(pDevice->thdHandle);
err_scan_event:
#if 0
    BKNI_DestroyEvent(pDevice->channelScanEvent);
#endif

    return NULL;
}

/***************************************************************************
Summary:
    Close a handle to a BCM7552 device.
See Also:
    NEXUS_Frontend_Open7552
 ***************************************************************************/
static void NEXUS_Frontend_P_7552_Close(
    NEXUS_FrontendHandle handle
    )
{
    NEXUS_7552 *pDevice;

    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    pDevice = (NEXUS_7552 *)handle->pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);

    if(pDevice->ewsEventCallback)NEXUS_UnregisterEvent(pDevice->ewsEventCallback);
    pDevice->ewsEventCallback = NULL;
    if(pDevice->thdLockEventCallback)NEXUS_UnregisterEvent(pDevice->thdLockEventCallback);
    pDevice->thdLockEventCallback = NULL;
    if(pDevice->thdIsrEventCallback)NEXUS_UnregisterEvent(pDevice->thdIsrEventCallback);
    pDevice->thdIsrEventCallback = NULL;
    if(pDevice->bbsEventCallback)NEXUS_UnregisterEvent(pDevice->bbsEventCallback);
    pDevice->bbsEventCallback = NULL;

#if 0
    BKNI_DestroyEvent(pDevice->channelScanEvent);
#endif
    
    if(pDevice->thdHandle)BTHD_Close(pDevice->thdHandle);
    if (pDevice->thdLockAppCallback) NEXUS_TaskCallback_Destroy(pDevice->thdLockAppCallback);
    if (pDevice->ewsAppCallback) NEXUS_TaskCallback_Destroy(pDevice->ewsAppCallback);

    NEXUS_Frontend_P_Destroy(handle);
    pDevice->frontendcount--;
    
    if(!pDevice->frontendcount){
        if (pDevice->tnrIsrEventCallback) NEXUS_UnregisterEvent(pDevice->tnrIsrEventCallback);
        pDevice->tnrIsrEventCallback = NULL;
        if(pDevice->tnrHandle) BTNR_Close(pDevice->tnrHandle);
        BDBG_OBJECT_DESTROY(pDevice, NEXUS_7552);
        BKNI_Free(pDevice);
    }
}

/***************************************************************************
 * Module callback functions for tuning
 ***************************************************************************/
static void NEXUS_Frontend_P_7552_Untune(void *handle)
{
    NEXUS_7552 *pDevice = handle;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);

    if (!pDevice->untuned) {
        if(pDevice->thdIsrEventCallback)NEXUS_UnregisterEvent(pDevice->thdIsrEventCallback); 
        pDevice->thdIsrEventCallback = NULL;
        if(pDevice->adsIsrEventCallback)NEXUS_UnregisterEvent(pDevice->adsIsrEventCallback);    
        pDevice->adsIsrEventCallback = NULL;
        pDevice->untuned = true;
    }

    /* abort the current acquisition (stop demodulation) if there is any */
    pDevice->currentMode = UNKNOWN_7552;
}

static BERR_Code NEXUS_Frontend_P_7552_TuneOfdm(
    void *handle,
    const NEXUS_FrontendOfdmSettings *pSettings
    )
{
    NEXUS_Error errCode;
    NEXUS_7552 *pDevice = handle;
    BTHD_InbandParams params;
    BTNR_PowerSaverSettings pwrSettings;
    BTNR_Settings tnrSettings;
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BDBG_ASSERT(NULL != pSettings);;

    if (pDevice->untuned) {
        /* Get interrupt event and register callback */
        errCode = BTHD_GetInterruptEventHandle(pDevice->thdHandle, &pDevice->thdIsrEvent);
        if (BERR_SUCCESS != errCode) return BERR_TRACE(errCode);

        pDevice->thdIsrEventCallback = NEXUS_RegisterEvent(pDevice->thdIsrEvent, NEXUS_Frontend_P_7552_IsrEvent, pDevice);
        if (NULL == pDevice->thdIsrEventCallback) return BERR_TRACE(BERR_OS_ERROR);
        pDevice->untuned = false;
    }

    pDevice->currentMode = OFDM_7552;

    if (pDevice->tnrHandle)
    {      
        rc = BTNR_GetSettings(pDevice->tnrHandle, &tnrSettings);
        if (rc!=BERR_SUCCESS) { return BERR_TRACE(rc);} 

        pwrSettings.enable = false;     
        rc = BTNR_SetPowerSaver(pDevice->tnrHandle, &pwrSettings);
        if (rc!=BERR_SUCCESS) { return BERR_TRACE(rc);}

        tnrSettings.tnrApplication = pSettings->terrestrial?BTNR_TunerApplication_eTerrestrial:BTNR_TunerApplication_eCable;
        tnrSettings.bandwidth = pSettings->bandwidth;
        tnrSettings.rfInputMode = pDevice->rfInput;
        rc = BTNR_SetSettings(pDevice->tnrHandle, &tnrSettings);
        if (rc!=BERR_SUCCESS) { return BERR_TRACE(rc);}

        rc = BTNR_SetTunerRfFreq(pDevice->tnrHandle, pSettings->frequency, BTNR_TunerMode_eDigital);
        if (rc!=BERR_SUCCESS) { return BERR_TRACE(rc);}
    }

    BTHD_GetDefaultInbandParams(pDevice->thdHandle, &params);

    switch (pSettings->mode) {
    case NEXUS_FrontendOfdmMode_eDvbt:
        params.mode = BTHD_InbandMode_eDvbt;
        break;
    case NEXUS_FrontendOfdmMode_eIsdbt:
        params.mode = BTHD_InbandMode_eIsdbt;
        break;
    default:
        BDBG_ERR(("Invalid OFDM Mode"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
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
    params.eModeGuardAcquire = !pSettings->manualModeSettings;
    if (pSettings->manualModeSettings) {
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

    NEXUS_TaskCallback_Set(pDevice->thdLockAppCallback, &pSettings->lockCallback);
    NEXUS_TaskCallback_Set(pDevice->ewsAppCallback, &pSettings->ewsCallback);
    pDevice->lastSettings.ofdm = *pSettings;

    errCode = BTHD_TuneAcquire(pDevice->thdHandle, &params);
    if (BERR_SUCCESS != errCode) return BERR_TRACE(errCode);

    return NEXUS_SUCCESS;
}

static NEXUS_FrontendLockStatus  NEXUS_Frontend_P_GetLockStatus(unsigned lockStatus)
{
    BDBG_MSG(("lockStatus = %d ", lockStatus));
    
    switch ( lockStatus )
    {
    /*BADS_LockStatus_eUnlocked == BTHD_LockStatus_eUnlocked == BTC2_LockStatus_eUnlocked*/
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
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7552 *pDevice = (NEXUS_7552 *)handle;
    unsigned lock;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BDBG_ASSERT(NULL != pStatus);

    rc = BTHD_GetThdLockStatus(pDevice->thdHandle,  &lock);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pStatus->lockStatus = NEXUS_Frontend_P_GetLockStatus(lock);
    return BERR_SUCCESS;
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7552_GetOfdmStatus(
    void *handle,
    NEXUS_FrontendOfdmStatus *pStatus
    )
{
    NEXUS_Error errCode;
    NEXUS_7552 *pDevice = handle;
    BTHD_THDStatus status;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof(NEXUS_FrontendOfdmStatus));

    errCode = BTHD_GetThdStatus(pDevice->thdHandle, &status);
    if (BERR_SUCCESS != errCode) return BERR_TRACE(errCode);

    pStatus->settings = pDevice->lastSettings.ofdm;
    pStatus->fecLock = status.bFecLock;
    pStatus->receiverLock = status.bReceiverLock;
    pStatus->noSignalDetected = status.bNoSignalDetected;
    pStatus->spectrumInverted = status.bSpectrumInverted;
    pStatus->ews = status.bIsdbtEWS;
    if(pStatus->settings.mode == NEXUS_FrontendOfdmMode_eDvbt) {
        switch (status.eModulation) {
        case BTHD_Modulation_eQpsk:
            pStatus->modulation = NEXUS_FrontendOfdmModulation_eQpsk;
            break;
        case BTHD_Modulation_e16Qam:
            pStatus->modulation = NEXUS_FrontendOfdmModulation_eQam16;
            break;
        case BTHD_Modulation_e64Qam:
            pStatus->modulation = NEXUS_FrontendOfdmModulation_eQam64;
            break;
        default:
            BDBG_WRN(("Unrecognized modulation mode (%d) reported by BTHD",
                status.eModulation));
            pStatus->modulation = NEXUS_FrontendOfdmModulation_eMax;
            break;
        }

            pStatus->codeRate = NEXUS_Frontend_P_THDToCodeRate(status.eCodeRate);
        switch (status.eHierarchy) {
        case BTHD_Hierarchy_0:
            pStatus->hierarchy = NEXUS_FrontendOfdmHierarchy_e0;
            break;
        case BTHD_Hierarchy_1:
            pStatus->hierarchy = NEXUS_FrontendOfdmHierarchy_e1;
            break;
        case BTHD_Hierarchy_2:
            pStatus->hierarchy = NEXUS_FrontendOfdmHierarchy_e2;
            break;
        case BTHD_Hierarchy_4:
            pStatus->hierarchy = NEXUS_FrontendOfdmHierarchy_e4;
            break;
        default:
            BDBG_WRN(("Unrecognized hierarchy (%d) reported by BTHD",
                status.eHierarchy));
            pStatus->hierarchy = NEXUS_FrontendOfdmHierarchy_eMax;
            break;
        }
    }
    switch (status.eTransmissionMode) {
    case BTHD_TransmissionMode_e2K:
        pStatus->transmissionMode = NEXUS_FrontendOfdmTransmissionMode_e2k;
        break;
    case BTHD_TransmissionMode_e4K:
        pStatus->transmissionMode = NEXUS_FrontendOfdmTransmissionMode_e4k;
        break;
    case BTHD_TransmissionMode_e8K:
        pStatus->transmissionMode = NEXUS_FrontendOfdmTransmissionMode_e8k;
        break;
    default:
        BDBG_WRN(("Unrecognized transmission mode (%d) reported by BTHD",
            status.eTransmissionMode));
        pStatus->transmissionMode = NEXUS_FrontendOfdmTransmissionMode_eMax;
        break;
    }
    switch (status.eGuardInterval) {
    case BTHD_GuardInterval_e1_32:
        pStatus->guardInterval = NEXUS_FrontendOfdmGuardInterval_e1_32;
        break;
    case BTHD_GuardInterval_e1_16:
        pStatus->guardInterval = NEXUS_FrontendOfdmGuardInterval_e1_16;
        break;
    case BTHD_GuardInterval_e1_8:
        pStatus->guardInterval = NEXUS_FrontendOfdmGuardInterval_e1_8;
        break;
    case BTHD_GuardInterval_e1_4:
        pStatus->guardInterval = NEXUS_FrontendOfdmGuardInterval_e1_4;
        break;
    default:
        BDBG_WRN(("Unrecognized guard interval (%d) reported by BTHD",
            status.eGuardInterval));
        pStatus->guardInterval = NEXUS_FrontendOfdmGuardInterval_eMax;
        break;
    }

    pStatus->cellId = status.nCellId;
    pStatus->carrierOffset = status.lCarrierOffset;
    pStatus->timingOffset = status.lTimingOffset;
    pStatus->snr = (status.nSnr*100)/256;   
    pStatus->fecCorrectedBlocks = status.ulRsCorrectedBlocks;
    pStatus->fecUncorrectedBlocks = status.ulRsUncorrectedBlocks;
    pStatus->fecCleanBlocks = status.ulRsCleanBlocks;
    pStatus->reacquireCount = status.ulReacqCount;
    pStatus->viterbiUncorrectedBits = status.ulViterbiUncorrectedBits;
    pStatus->viterbiTotalBits = status.ulViterbiTotalbits;
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
    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_7552_GetSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length)
{
   #define TOTAL_THD_SOFTDECISIONS 30

    int j;
    size_t i;
    NEXUS_Error errCode;
    NEXUS_7552 *pDevice = handle;
    int16_t d_i[TOTAL_THD_SOFTDECISIONS], d_q[TOTAL_THD_SOFTDECISIONS];

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);

    for( i=0; i<length; i += TOTAL_THD_SOFTDECISIONS )
    {

        errCode = BTHD_GetSoftDecisionBuf(pDevice->thdHandle, d_i, d_q);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        for ( j=0; j<TOTAL_THD_SOFTDECISIONS && i+j<length; j++ )
        {
            pDecisions[i+j].i = d_i[j] * 2; /* Multiply by two is just a scaling factor. */
            pDecisions[i+j].q = d_q[j] * 2;
        }
    }
    return BERR_SUCCESS;
}

static void NEXUS_Frontend_P_7552_ResetStatus(
    void *handle
    )
{
    NEXUS_Error errCode;
    NEXUS_7552 *pDevice = handle;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);

    errCode = BTHD_ResetInbandStatus(pDevice->thdHandle);
    if (BERR_SUCCESS != errCode) {
        BDBG_WRN(("failed BTHD_ResetInbandStatus()"));
        (void) BERR_TRACE(errCode);
    }
}

static NEXUS_Error NEXUS_Frontend_P_7552_ScanFrequency(
    void *handle,
    const NEXUS_FrontendScanSettings *pSettings,
    NEXUS_FrontendScanResults *pResults
    )
{
    NEXUS_Error errCode;
    NEXUS_7552 *pDevice = handle;
    unsigned i;
    int numSignalTypesToDetect;
    BTHD_InbandParams params;
    unsigned lastBandwidth;
    NEXUS_CallbackDesc channelScanCallback;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pResults);

    /* parameter validity checking */
    numSignalTypesToDetect = 0;
    for (i=0; i<NEXUS_FrontendSignalType_eMax; ++i) {
        if (pSettings->signalTypes[i]) ++numSignalTypesToDetect;
    }
    switch (numSignalTypesToDetect) {
    case 0:
        BDBG_ERR(("Please specify a signal type to detect."));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
        break;
    case 1:
        /* do nothing */
        break;
    default:
        BDBG_ERR(("Too many signal types to detect at the same time."));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
        break;
    }

    if (!pSettings->signalTypes[NEXUS_FrontendSignalType_eDvbt] &&
        !pSettings->signalTypes[NEXUS_FrontendSignalType_eIsdbt])
    {
        BDBG_ERR(("nothing to scan"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    pDevice->currentMode = SCAN_7552;
    pDevice->lastSettings.scan = *pSettings;

    if (pDevice->untuned) {
        /* Get interrupt event and register callback */
        errCode = BTHD_GetInterruptEventHandle(pDevice->thdHandle,
            &pDevice->thdIsrEvent);
        if (BERR_SUCCESS != errCode) return BERR_TRACE(errCode);

        pDevice->thdIsrEventCallback = NEXUS_RegisterEvent(pDevice->thdIsrEvent,
            NEXUS_Frontend_P_7552_IsrEvent, pDevice);
        if (NULL == pDevice->thdIsrEventCallback) return BERR_TRACE(BERR_OS_ERROR);
        pDevice->untuned = false;
    }

    /* THD inband parameters */
    BTHD_GetDefaultInbandParams(pDevice->thdHandle, &params);
    params.ifFreq = pSettings->ifFrequency;
    params.eAcquisitionMode = BTHD_ThdAcquisitionMode_eScan;
    params.tunerFreq = pSettings->frequency;
    params.ePullinRange = BTHD_PullInRange_eWide;
    params.cciMode = BTHD_CCI_Auto;
    params.bTpsAcquire = true;
    params.eModeGuardAcquire = true;
    params.bTmccAcquire = true;

    /* DFE and THD inband based on DVB-T or ISDB-T */
    if (pSettings->signalTypes[NEXUS_FrontendSignalType_eDvbt]) {
        params.mode = BTHD_InbandMode_eDvbt;
    } else {
        /* DVB-T or ISDB-T, nothing else */
        params.mode = BTHD_InbandMode_eIsdbt;
    }

    lastBandwidth = 0;
    for (i=0;
         i<sizeof(pSettings->bandwidths)/sizeof*(pSettings->bandwidths);
         ++i)
    {
        const unsigned bandwidth = pSettings->bandwidths[i];
        BTHD_THDStatus thdStatus;

        /* end-of-bandwidths: 0 (zero) or repetition */
        if (0 == bandwidth) break;
        if (bandwidth == lastBandwidth) break;
        lastBandwidth = bandwidth;

        switch (bandwidth) {
        case NEXUS_FrontendBandwidth_e5Mhz:
            params.bandwidth = BTHD_Bandwidth_5Mhz;
            break;
        case NEXUS_FrontendBandwidth_e6Mhz:
            params.bandwidth = BTHD_Bandwidth_6Mhz;
            break;
        case NEXUS_FrontendBandwidth_e7Mhz:
            params.bandwidth = BTHD_Bandwidth_7Mhz;
            break;
        default:
            BDBG_WRN(("Unrecognized bandwidth setting, defaulting to 8MHz"));
            /* fall through */
        case NEXUS_FrontendBandwidth_e8Mhz:
            params.bandwidth = BTHD_Bandwidth_8Mhz;
            break;
        }

#if 0
        BKNI_ResetEvent(pDevice->channelScanEvent);
#endif
        BKNI_Memset(&channelScanCallback, 0, sizeof(channelScanCallback));
        channelScanCallback.callback = NEXUS_Frontend_P_7552_ChannelScanResult;
        channelScanCallback.context = pDevice;
        NEXUS_TaskCallback_Set(pDevice->thdLockAppCallback, &channelScanCallback);

        errCode = BTHD_TuneAcquire(pDevice->thdHandle, &params);
        if (BERR_SUCCESS != errCode) return BERR_TRACE(errCode);
#if 0
        /* open a narrow window for allowing the lock callback to be called
         * back from THD */
        NEXUS_UnlockModule();
        errCode = BKNI_WaitForEvent(pDevice->channelScanEvent,
            pDevice->channelScanTimeout);
        NEXUS_LockModule();
        if (BERR_SUCCESS != errCode) {
            (void) BERR_TRACE(errCode);
            BDBG_ERR(("Channel Scan result did not finish in %d milliseconds, "
                "errCode %d", pDevice->channelScanTimeout, errCode));
            /* keep going */
        }
#endif

        errCode = BTHD_GetThdStatus(pDevice->thdHandle, &thdStatus);
        if (BERR_SUCCESS != errCode) return BERR_TRACE(errCode);

        if (thdStatus.bNoSignalDetected) continue;

        /* stop acquisition to prevent unnecessary callback */
        NEXUS_Frontend_P_7552_Untune(pDevice);

        BKNI_Memset(pResults, 0, sizeof*(pResults));
        if (pSettings->signalTypes[NEXUS_FrontendSignalType_eDvbt]) {
            pResults->signalType = NEXUS_FrontendSignalType_eDvbt;
        } else {
            /* DVB-T or ISDB-T, nothing else */
            pResults->signalType = NEXUS_FrontendSignalType_eIsdbt;
        }
        pResults->bandwidth = bandwidth;
        pResults->spectrumInverted = thdStatus.bSpectrumInverted;
        pResults->frequencyOffset = thdStatus.lCarrierOffset;
        return NEXUS_SUCCESS;
    }

    return NEXUS_TIMEOUT;
}

static void NEXUS_Frontend_P_7552_GetScanThresholds(
    void *handle,
    NEXUS_FrontendScanThresholds *pThresholds
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pThresholds);
    /* does nothing for now */
}

static NEXUS_Error NEXUS_Frontend_P_7552_SetScanThresholds(
    void *handle,
    const NEXUS_FrontendScanThresholds *pThresholds
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pThresholds);

    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

static BTHD_IsdbtTimeInterleaving NEXUS_Frontend_P_TimeInterleavingToTHD(
    NEXUS_FrontendOfdmTimeInterleaving nexus
    )
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

static NEXUS_FrontendOfdmTimeInterleaving
    NEXUS_Frontend_P_THDToTimeInterleaving(
        BTHD_IsdbtTimeInterleaving magnum
        )
{
    switch (magnum) {
    default:
        BDBG_WRN(("Unknown Magnum time interleaving %d, defaulting to 0x",
            magnum));
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

static BTHD_CodeRate NEXUS_Frontend_P_CodeRateToTHD(
    NEXUS_FrontendOfdmCodeRate nexusCodeRate
    )
{
    switch (nexusCodeRate) {
    default:
        BDBG_WRN(("Unsupported code rate value %d, defaulting to 1_2"));
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

static NEXUS_FrontendOfdmCodeRate NEXUS_Frontend_P_THDToCodeRate(
    BTHD_CodeRate magnum
    )
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


static BTHD_Modulation NEXUS_Frontend_P_ModulationToTHD(
    NEXUS_FrontendOfdmModulation nexus
    )
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

static NEXUS_FrontendOfdmModulation NEXUS_Frontend_P_THDToModulation(
    BTHD_Modulation magnum
    )
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

