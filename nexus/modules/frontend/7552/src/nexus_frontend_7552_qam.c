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
*   acquire QAM signal (J.83ABC), getting the status of the acquired channel,
*   setting the inband transport output interface settings, detecting certain
*   type of signal, install signal lock callback, install signal unlock
*   callback, and etc.
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
static NEXUS_Error NEXUS_Frontend_P_7552_ScanAtsc(NEXUS_7552 *pDevice, const NEXUS_FrontendScanSettings *pSettings, NEXUS_FrontendScanResults *pResults);
static NEXUS_Error NEXUS_Frontend_P_7552_ScanDvbc(NEXUS_7552 *pDevice, const NEXUS_FrontendScanSettings *pSettings, NEXUS_FrontendScanResults *pResults);
static NEXUS_FrontendQamMode NEXUS_Frontend_P_Ads2NexusQamMode(BADS_ModulationType modulationType);

/***************************************************************************
 * Module callback functions for tuning
 ***************************************************************************/
static void NEXUS_Frontend_P_7552_Close(NEXUS_FrontendHandle handle);
static void NEXUS_Frontend_P_7552_Untune(void *handle);
static NEXUS_Error NEXUS_Frontend_P_7552_TuneQam(void *handle, const NEXUS_FrontendQamSettings *pSettings);
static NEXUS_Error NEXUS_Frontend_P_7552_GetQamStatus(void *handle, NEXUS_FrontendQamStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_7552_GetSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length);
static void NEXUS_Frontend_P_7552_ResetStatus(void *handle);
static NEXUS_Error NEXUS_Frontend_P_7552_ScanFrequency(void *handle, const NEXUS_FrontendScanSettings *pSettings, NEXUS_FrontendScanResults *pResults);
static void        NEXUS_Frontend_P_7552_GetScanThresholds(void *handle, NEXUS_FrontendScanThresholds *pThresholds);
static NEXUS_Error NEXUS_Frontend_P_7552_SetScanThresholds(void *handle, const NEXUS_FrontendScanThresholds *pThresholds);
static NEXUS_Error NEXUS_Frontend_P_7552_GetFastStatus(void *handle, NEXUS_FrontendFastStatus *pStatus);
#if 0
static NEXUS_Error NEXUS_Frontend_P_7552_GetQamScanStatus(void *handle, NEXUS_FrontendQamScanStatus *pScanStatus);
#endif
/***************************************************************************
Summary:
    Lock callback handler for a 7552 Inband device
 ***************************************************************************/
static void NEXUS_Frontend_P_7552_LockChange_isr(void *pParam)
{
    NEXUS_IsrCallbackHandle callback = (NEXUS_IsrCallbackHandle)pParam;

    if (NULL != callback) NEXUS_IsrCallback_Fire_isr(callback);
}

static void NEXUS_Frontend_P_7552_Tuner_isr(void *pParam)
{
    /* NOTE: Even though the pDevice is passed in as the pParam, NEXUS_Frontend_P_7552_Tuner_isr gets called with BADS_P_AdsCallbackData_s as its pParam. */
    BADS_P_AdsCallbackData_t *pCallback = (BADS_P_AdsCallbackData_t *)pParam;
    BTNR_3x7x_RfStatus_t RfCallbackStatus;
    NEXUS_7552 *pDevice = (NEXUS_7552 *) pCallback->hTunerChn;

    if (NULL == pCallback) return;

 

    if (BADS_CallbackMode_eSetMode == pCallback->Mode) {
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
        pCallback->Freq_Offset = RfCallbackStatus.RF_Offset;
        pCallback->Symbol_Rate = RfCallbackStatus.Symbol_Rate;
    }
}

/***************************************************************************
Summary:
    ISR Event Handler for a 7552 device
 ***************************************************************************/
static void NEXUS_Frontend_P_7552_IsrEvent(void *pParam)
{
    NEXUS_7552 *pDevice = pParam;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);

    BDBG_MSG(("Calling BADS_ProcessInterruptEvent"));
    BADS_ProcessInterruptEvent(pDevice->adsHandle);
}

static void NEXUS_Frontend_P_7552_ChannelScanResult(void *pParam, int param)
{
    NEXUS_7552 *pDevice = pParam;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BSTD_UNUSED(param);

    if (SCAN_7552 == pDevice->currentMode) {
        BKNI_SetEvent(pDevice->channelScanEvent);
    }
}

/***************************************************************************
Summary:
    Get the default settings for a BCM7552 Frontend
See Also:
    NEXUS_Frontend_Open7552
 ***************************************************************************/
void NEXUS_Frontend_GetDefault7552QamSettings(
    NEXUS_7552QamFrontendSettings *pSettings   /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof*(pSettings));
}

/***************************************************************************
Summary:
    Open a handle to a BCM7552 device.
 ***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_Open7552Qam(
    const NEXUS_7552QamFrontendSettings *pSettings
    )
{
    BERR_Code errCode;
    NEXUS_FrontendHandle frontendHandle;
    NEXUS_7552 *pDevice=NULL;
    BADS_Settings adsSettings;
    BADS_ChannelSettings channelSettings;
    NEXUS_IsrCallbackHandle callbackLockChange;
    unsigned numChannels;

    BDBG_ASSERT(NULL != pSettings);

    errCode = NEXUS_Frontend_P_Open7552();
    if ( errCode != BERR_SUCCESS ) goto err_scan_event;

    pDevice = p_7552device;

    /* get default settings and update */
    errCode = BADS_3x7x_GetDefaultSettings(&adsSettings, NULL);
    if ( errCode != BERR_SUCCESS ) goto err_scan_event;
    
    adsSettings.hTmr = g_pCoreHandles->tmr;
    adsSettings.hHeap = g_pCoreHandles->heap[0];

    /* Open ADS device */
    errCode = BADS_Open(&pDevice->adsHandle, g_pCoreHandles->chp, g_pCoreHandles->reg, g_pCoreHandles->bint, &adsSettings);
    if ( errCode != BERR_SUCCESS ) goto err_scan_event;

    /* Initialize Acquisition Processor */
    BDBG_MSG(("Initializating QAM core..."));
    errCode = BADS_Init(pDevice->adsHandle);
    if ( errCode != BERR_SUCCESS ) goto err_open_ads;
    BDBG_MSG(("Initializating QAM core... Done"));

    /* Open the ADS channel handles */
    errCode = BADS_GetTotalChannels(pDevice->adsHandle, &numChannels);
    if ( errCode != BERR_SUCCESS ) goto err_open_ads;

    /* only one channel in 7552 is asserted */
    BDBG_ASSERT(1 == numChannels);

    errCode = BADS_GetChannelDefaultSettings(pDevice->adsHandle, 0, &channelSettings);
    if ( errCode != BERR_SUCCESS ) goto err_open_ads;

    errCode = BADS_OpenChannel(pDevice->adsHandle, &pDevice->channelHandle, 0, &channelSettings);
    if ( errCode != BERR_SUCCESS ) goto err_open_ads;

    /* Create a Nexus frontend handle */
    frontendHandle = NEXUS_Frontend_P_Create(pDevice);
    if ( errCode != BERR_SUCCESS ) goto err_open_channel;

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
    frontendHandle->close = NEXUS_Frontend_P_7552_Close;
    frontendHandle->untune = NEXUS_Frontend_P_7552_Untune;
    frontendHandle->tuneQam = NEXUS_Frontend_P_7552_TuneQam;
    frontendHandle->getQamStatus = NEXUS_Frontend_P_7552_GetQamStatus;
    frontendHandle->getSoftDecisions = NEXUS_Frontend_P_7552_GetSoftDecisions;
    frontendHandle->resetStatus = NEXUS_Frontend_P_7552_ResetStatus;
    frontendHandle->scanFrequency = NEXUS_Frontend_P_7552_ScanFrequency;
    frontendHandle->setScanThresholds = NEXUS_Frontend_P_7552_SetScanThresholds;
    frontendHandle->getScanThresholds = NEXUS_Frontend_P_7552_GetScanThresholds;
    frontendHandle->getFastStatus = NEXUS_Frontend_P_7552_GetFastStatus;
#if 0   
    frontendHandle->getQamScanStatus = NEXUS_Frontend_P_7552_GetQamScanStatus;
#endif
    callbackLockChange = NEXUS_IsrCallback_Create(frontendHandle, NULL);
    if ( errCode != BERR_SUCCESS ) goto err_frontendhandle;

    errCode = BADS_InstallCallback(pDevice->channelHandle, BADS_Callback_eLockChange, (BADS_CallbackFunc)NEXUS_Frontend_P_7552_LockChange_isr, (void*)callbackLockChange);
    if ( errCode != BERR_SUCCESS ) goto err_lockchangeisr_callback;

    /* NOTE: Even though the pDevice is passed in as the pParam, NEXUS_Frontend_P_7552_Tuner_isr gets called with BADS_P_AdsCallbackData_s as its pParam. */
    errCode = BADS_InstallCallback(pDevice->channelHandle, BADS_Callback_eTuner, (BADS_CallbackFunc)NEXUS_Frontend_P_7552_Tuner_isr, (void*)pDevice);
    if ( errCode != BERR_SUCCESS ) goto err_tunerisr_callback;

    pDevice->adsLockAppCallback = callbackLockChange;
    pDevice->frontendcount++;

    /* Get events and register callbacks */
    errCode = BADS_GetInterruptEventHandle(pDevice->adsHandle, &pDevice->adsIsrEvent);
    if ( errCode != BERR_SUCCESS ) goto err_tunerisr_callback;

    pDevice->adsIsrEventCallback = NEXUS_RegisterEvent(pDevice->adsIsrEvent, NEXUS_Frontend_P_7552_IsrEvent, pDevice);
    if ( errCode != BERR_SUCCESS ) goto err_tunerisr_callback;

    /* Success */
    pDevice->qamSettings = *pSettings;
    pDevice->currentMode = UNKNOWN_7552;
    return frontendHandle;

err_tunerisr_callback:
    NEXUS_IsrCallback_Destroy(callbackLockChange);
err_lockchangeisr_callback:
    NEXUS_IsrCallback_Destroy(callbackLockChange);
err_frontendhandle:
    NEXUS_Frontend_P_Destroy(frontendHandle);
err_open_channel:
    BADS_CloseChannel(pDevice->channelHandle);
err_open_ads:
    BADS_Close(pDevice->adsHandle);
err_scan_event:
    BKNI_DestroyEvent(pDevice->channelScanEvent);
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

    if (pDevice->tnrIsrEventCallback) NEXUS_UnregisterEvent(pDevice->tnrIsrEventCallback);
    pDevice->tnrIsrEventCallback = NULL;    
    if (pDevice->adsIsrEventCallback) NEXUS_UnregisterEvent(pDevice->adsIsrEventCallback);
    pDevice->adsIsrEventCallback = NULL;      
    if(pDevice->adsLockAppCallback) NEXUS_IsrCallback_Destroy(pDevice->adsLockAppCallback);
    pDevice->adsLockAppCallback = NULL;    
    
    if(pDevice->channelScanEvent)BKNI_DestroyEvent(pDevice->channelScanEvent);

    if(pDevice->channelHandle)BADS_CloseChannel(pDevice->channelHandle);
    if(pDevice->adsHandle)BADS_Close(pDevice->adsHandle);

    if(handle) NEXUS_Frontend_P_Destroy(handle);
    pDevice->frontendcount--;
    
    if(!pDevice->frontendcount){
        if (pDevice->tnrIsrEventCallback) NEXUS_UnregisterEvent(pDevice->tnrIsrEventCallback);
        pDevice->tnrIsrEventCallback = NULL;
        if(pDevice->tnrHandle) BTNR_Close(pDevice->tnrHandle);
        BDBG_OBJECT_DESTROY(pDevice, NEXUS_7552);
        if (pDevice) BKNI_Free(pDevice);
    }
}

/***************************************************************************
 * Module callback functions for tuning
 ***************************************************************************/
static void NEXUS_Frontend_P_7552_Untune(
    void *handle
    )
{
    BERR_Code errCode;
    NEXUS_7552 *pDevice = handle;
    BTNR_PowerSaverSettings pwrSettings;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);

    if (!pDevice->untuned) { 
        if(pDevice->thdIsrEventCallback)NEXUS_UnregisterEvent(pDevice->thdIsrEventCallback); 
        pDevice->thdIsrEventCallback = NULL;
        if(pDevice->adsIsrEventCallback)NEXUS_UnregisterEvent(pDevice->adsIsrEventCallback);    
        pDevice->adsIsrEventCallback = NULL;
        
        errCode = BADS_Untune(pDevice->adsHandle);
        if (errCode) { BERR_TRACE(errCode);}
        pDevice->untuned = true;

        errCode = BADS_EnablePowerSaver(pDevice->channelHandle);
        if(errCode){errCode = BERR_TRACE(errCode);}
        pwrSettings.enable = true;

        errCode = BTNR_SetPowerSaver(pDevice->tnrHandle, &pwrSettings);
        if (errCode){errCode = BERR_TRACE(errCode);}
    }

    pDevice->currentMode = UNKNOWN_7552;
}

static NEXUS_Error NEXUS_Frontend_P_7552_TuneQam(
    void *handle,
    const NEXUS_FrontendQamSettings *pSettings
    )
{
    NEXUS_Error errCode;
    NEXUS_7552 *pDevice = (NEXUS_7552 *)handle;
    BADS_InbandParam params;
    BADS_ModulationType qam_mode;
    uint32_t symbolRate = 0;
    BTNR_PowerSaverSettings pwrSettings;
    BTNR_Settings tnrSettings;
    BERR_Code rc;
    BADS_ChannelScanSettings scanParam;
    
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BDBG_ASSERT(NULL != pSettings);

    pDevice->currentMode = QAM_7552;
    pDevice->lastSettings.qam = *pSettings;

    if (pDevice->untuned) {
        /* Get interrupt event and register callback */
        errCode = BADS_GetInterruptEventHandle(pDevice->adsHandle, &pDevice->adsIsrEvent);
        if (BERR_SUCCESS != errCode) return BERR_TRACE(errCode);

        pDevice->adsIsrEventCallback = NEXUS_RegisterEvent(pDevice->adsIsrEvent, NEXUS_Frontend_P_7552_IsrEvent, pDevice);
        if (NULL == pDevice->adsIsrEventCallback) return BERR_TRACE(BERR_OS_ERROR);
    }

    if (pDevice->tnrHandle)
    {
        /* TODO: we should be able to call BTNR_SetSettings after BTNR_SetPowerSaver. verify */
        pwrSettings.enable = false;     
        rc = BTNR_SetPowerSaver(pDevice->tnrHandle, &pwrSettings);
        if (rc!=BERR_SUCCESS) { return BERR_TRACE(rc);}

        rc = BTNR_GetSettings(pDevice->tnrHandle, &tnrSettings);
        if (rc!=BERR_SUCCESS) { return BERR_TRACE(rc);}  

        tnrSettings.tnrApplication = pSettings->terrestrial?BTNR_TunerApplication_eTerrestrial:BTNR_TunerApplication_eCable;
        tnrSettings.bandwidth = pSettings->bandwidth;
        tnrSettings.rfInputMode = pDevice->rfInput;       
        rc = BTNR_SetSettings(pDevice->tnrHandle, &tnrSettings);
        if (rc!=BERR_SUCCESS) { return BERR_TRACE(rc);}
        
        rc = BADS_DisablePowerSaver(pDevice->channelHandle);
        if (rc!=BERR_SUCCESS){BERR_TRACE(rc);}
        
        rc = BTNR_SetTunerRfFreq(pDevice->tnrHandle, pSettings->frequency, BTNR_TunerMode_eDigital);
        if (rc!=BERR_SUCCESS) { return BERR_TRACE(rc);}
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
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
        break;
    case NEXUS_FrontendQamAnnex_eB:
        /* only 6MHz, and 64/256QAM */
        if (NEXUS_FrontendBandwidth_e6Mhz != pSettings->bandwidth) {
            BDBG_ERR(("J.83B with non-6MHz bandwidth (%d)", pSettings->bandwidth));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
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
            return BERR_TRACE(BERR_INVALID_PARAMETER);
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
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
        break;
    default:
        BDBG_ERR(("Invalid annex (%d)", pSettings->annex));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
        break;
    }

    /* Scan Parameters */ 
    if(pSettings->acquisitionMode == NEXUS_FrontendQamAcquisitionMode_eScan){
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
        if(rc){rc = BERR_TRACE(rc);}
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

    BDBG_MSG(("Calling BADS_Acquire() with autoAcquire %d, frequencyOffset %d, enableDpm %d, spectrum %d",
        params.autoAcquire, params.frequencyOffset, params.enableDpm, params.spectrum));

    BDBG_MSG(("Calling BADS_Acquire() with modType %d, enableDpm %d, invertSpectrum %d, spectrum %d, symbolRate %d",
        params.modType, params.enableDpm, params.invertSpectrum, params.spectrum, params.symbolRate));

    NEXUS_IsrCallback_Set(pDevice->adsLockAppCallback, &pSettings->lockCallback);

    errCode = BADS_Acquire(pDevice->channelHandle, &params);
    if (BERR_SUCCESS != errCode) return BERR_TRACE(errCode);

    pDevice->untuned = false;

    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_7552_GetFastStatus(void *handle, NEXUS_FrontendFastStatus *pStatus)
{
    NEXUS_Error rc;
    NEXUS_7552 *pDevice = handle;
    BADS_LockStatus lockStatus;
     
    rc = BADS_GetLockStatus(pDevice->channelHandle,  &lockStatus);
    if(rc){rc = BERR_TRACE(rc);} 
    switch ( lockStatus )
    {
    case BADS_LockStatus_eUnlocked:
        pStatus->lockStatus = NEXUS_FrontendLockStatus_eUnlocked;
        break;
    case BADS_LockStatus_eLocked:
        pStatus->lockStatus = NEXUS_FrontendLockStatus_eLocked;
        break;
    case BADS_LockStatus_eNoSignal:
        pStatus->lockStatus = NEXUS_FrontendLockStatus_eNoSignal;
        break;
    default:
        BDBG_WRN(("Unrecognized lock status (%d) ", lockStatus));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return NEXUS_FrontendLockStatus_eUnknown;
    }

    return rc;
}
#if 0
static NEXUS_Error NEXUS_Frontend_P_7552_GetQamScanStatus(void *handle, NEXUS_FrontendQamScanStatus *pScanStatus)
{

    NEXUS_Error rc;
    NEXUS_7552 *pDevice = (NEXUS_7552 *)handle;
    struct BADS_ScanStatus st;
    
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BDBG_ASSERT(NULL != pScanStatus);


    BKNI_Memset(pScanStatus, 0, sizeof(*pScanStatus));

    rc = BADS_GetScanStatus(pDevice->channelHandle,  &st);
    if(rc){rc = BERR_TRACE(rc); goto done;}
    
    pScanStatus->spectrumInverted = st.isSpectrumInverted;
    pScanStatus->symbolRate = st.symbolRate;
    pScanStatus->frequencyOffset = st.carrierFreqOffset;
    pScanStatus->interleaver = st.interleaver;
    pScanStatus->acquisitionStatus = st.acquisitionStatus;
    return BERR_SUCCESS;
done:
    return rc;
}
#endif
static NEXUS_Error NEXUS_Frontend_P_7552_GetQamStatus( void *handle, NEXUS_FrontendQamStatus *pStatus )
{
    NEXUS_Error errCode;
    NEXUS_7552 *pDevice = handle;
    BADS_Status adsStatus;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BDBG_ASSERT(NULL != pStatus);

    errCode = BADS_GetStatus(pDevice->channelHandle, &adsStatus);
    if (BERR_SUCCESS != errCode) return BERR_TRACE(errCode);
    BKNI_Memset(pStatus, 0, sizeof*(pStatus));

    pStatus->fecLock = adsStatus.isFecLock;
    pStatus->receiverLock = adsStatus.isQamLock;
    pStatus->rfAgcLevel = adsStatus.agcExtLevel;
    pStatus->ifAgcLevel = adsStatus.agcIntLevel;
    pStatus->dsChannelPower = adsStatus.dsChannelPower;
    pStatus->snrEstimate = (adsStatus.snrEstimate * 100)/256;
    pStatus->fecCorrected = adsStatus.correctedCount;
    pStatus->fecUncorrected = adsStatus.uncorrectedCount;
    pStatus->carrierPhaseOffset = adsStatus.carrierPhaseOffset;
    pStatus->carrierFreqOffset = adsStatus.carrierFreqOffset;
    pStatus->symbolRate = adsStatus.rxSymbolRate;
    pStatus->symbolRateError = adsStatus.symbolRateError;
    pStatus->berEstimate = adsStatus.berRawCount;
    pStatus->mainTap = adsStatus.mainTap;
    pStatus->equalizerGain = adsStatus.equalizerGain;
    pStatus->postRsBer = 0;/* Not supported */
    pStatus->postRsBerElapsedTime = 0;/* Not supported */
    pStatus->settings = pDevice->lastSettings.qam;
    pStatus->spectrumInverted = adsStatus.isSpectrumInverted;
    pStatus->fecClean = adsStatus.cleanCount;
    pStatus->intAgcLevel = adsStatus.agcAGFLevel;
    pStatus->bitErrCorrected = adsStatus.correctedBits;

    return BERR_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_7552_GetSoftDecisions(
    void *handle,
    NEXUS_FrontendSoftDecision *pDecisions,
    size_t length
    )
{
    #define TOTAL_ADS_SOFTDECISIONS 30

    int j;
    size_t i;
    NEXUS_Error errCode;
    int16_t return_length;
    NEXUS_7552 *pDevice = handle;
    int16_t d_i[TOTAL_ADS_SOFTDECISIONS], d_q[TOTAL_ADS_SOFTDECISIONS];

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);

    for( i=0; i<length; i += TOTAL_ADS_SOFTDECISIONS )
    {

        errCode = BADS_GetSoftDecision(pDevice->channelHandle, (int16_t)TOTAL_ADS_SOFTDECISIONS,  d_i, d_q, &return_length);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        for ( j=0; j<TOTAL_ADS_SOFTDECISIONS && i+j<length; j++ )
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
    BSTD_UNUSED(handle);

    /*(void) BERR_TRACE(BERR_NOT_SUPPORTED);*/
}

static NEXUS_Error NEXUS_Frontend_P_7552_ScanFrequency(
    void *handle,
    const NEXUS_FrontendScanSettings *pSettings,
    NEXUS_FrontendScanResults *pResults
    )
{
    NEXUS_7552 *pDevice = handle;
    int i;
    int numSignalTypesToDetect;

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

    if (pSettings->signalTypes[NEXUS_FrontendSignalType_eQam]) {
        return NEXUS_Frontend_P_7552_ScanAtsc(pDevice, pSettings, pResults);
    }
    if (pSettings->signalTypes[NEXUS_FrontendSignalType_eDvbc]) {
        return NEXUS_Frontend_P_7552_ScanDvbc(pDevice, pSettings, pResults);
    }

    BDBG_ERR(("nothing to scan"));
    return BERR_TRACE(BERR_INVALID_PARAMETER);
}

static NEXUS_Error NEXUS_Frontend_P_7552_ScanAtsc(
    NEXUS_7552 *pDevice,
    const NEXUS_FrontendScanSettings *pSettings,
    NEXUS_FrontendScanResults *pResults
    )
{
    NEXUS_Error errCode;
    BADS_ChannelScanSettings adsScanSettings;
    BADS_InbandParam params;
    NEXUS_CallbackDesc channelScanCallback;
    BADS_Status adsStatus;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pResults);

    /* set channel scan parameters */
    BKNI_Memset(&adsScanSettings, 0, sizeof(adsScanSettings));
    adsScanSettings.AI = true; /* auto spectral inversion detection */
    adsScanSettings.QM = true; /* enable constellation search */
    adsScanSettings.CO = true; /* enable carrier offset search */
    adsScanSettings.TO = false; /* disable auto symbol rate search */
    adsScanSettings.B256 = true;
    adsScanSettings.B64 = true;
    adsScanSettings.carrierSearch = 500000/256; /* +/- 500KHz, unit *256Hz */
    adsScanSettings.upperBaudSearch = J83B_SYMBOLRATE_QAM256;
    adsScanSettings.lowerBaudSearch = J83B_SYMBOLRATE_QAM64;

    errCode = BADS_SetScanParam(pDevice->channelHandle, &adsScanSettings);
    if (BERR_SUCCESS != errCode) return BERR_TRACE(errCode);

    pDevice->currentMode = SCAN_7552;
    pDevice->lastSettings.scan = *pSettings;

    if (pDevice->untuned) {
        /* Get interrupt event and register callback */
        errCode = BADS_GetInterruptEventHandle(pDevice->adsHandle, &pDevice->adsIsrEvent);
        if (BERR_SUCCESS != errCode) return BERR_TRACE(errCode);

        pDevice->adsIsrEventCallback = NEXUS_RegisterEvent(pDevice->adsIsrEvent, NEXUS_Frontend_P_7552_IsrEvent, pDevice);
        if (NULL == pDevice->adsIsrEventCallback) return BERR_TRACE(BERR_OS_ERROR);

        pDevice->untuned = false;
    }

    BKNI_ResetEvent(pDevice->channelScanEvent);
    BKNI_Memset(&channelScanCallback, 0, sizeof(channelScanCallback));
    channelScanCallback.callback = NEXUS_Frontend_P_7552_ChannelScanResult;
    channelScanCallback.context = pDevice;
    NEXUS_IsrCallback_Set(pDevice->adsLockAppCallback, &channelScanCallback);

    /* set inband parameters */
    BKNI_Memset(&params, 0, sizeof(params));
    params.modType = BADS_ModulationType_eAnnexBQam256;
    params.symbolRate = J83B_SYMBOLRATE_QAM256;
    params.invertSpectrum = BADS_InvertSpectrum_eNoInverted;
    params.spectrum = BADS_SpectrumMode_eAuto;
    params.enableDpm = false;
    params.autoAcquire = false;
    params.enableNullPackets = false;
    params.acquisitionType = BADS_AcquireType_eScan;
    errCode = BADS_Acquire(pDevice->channelHandle, &params);
    if (BERR_SUCCESS != errCode) return BERR_TRACE(errCode);

    errCode = BKNI_WaitForEvent(pDevice->channelScanEvent,
        pDevice->channelScanTimeout);
    if (BERR_SUCCESS != errCode) {
        (void) BERR_TRACE(errCode);
        BDBG_ERR(("Channel Scan result was not delivered in %d milliseconds, "
            "errCode %d", pDevice->channelScanTimeout, errCode));
        /* keep going */
    }

    errCode = BADS_GetStatus(pDevice->channelHandle, &adsStatus);
    if (BERR_SUCCESS != errCode) return BERR_TRACE(errCode);

    /* stop acquisition to prevent unnecessary callback */
    NEXUS_Frontend_P_7552_Untune(pDevice);

    if (adsStatus.isFecLock) {
        /* TODO: or to use adsStatus.isQamLock ? */
        BKNI_Memset(pResults, 0, sizeof*(pResults));
        pResults->signalType = NEXUS_FrontendSignalType_eQam;
        pResults->bandwidth = 6000000; /* 6MHz */
        pResults->spectrumInverted = adsStatus.isSpectrumInverted;
        pResults->frequencyOffset = adsStatus.carrierFreqOffset / 1000;
        pResults->symbolRate = adsStatus.rxSymbolRate;
        pResults->mode.qam =
            NEXUS_Frontend_P_Ads2NexusQamMode(adsStatus.modType);
        return NEXUS_SUCCESS;
    }

    return NEXUS_TIMEOUT;
}

static NEXUS_Error NEXUS_Frontend_P_7552_ScanDvbc(
    NEXUS_7552 *pDevice,
    const NEXUS_FrontendScanSettings *pSettings,
    NEXUS_FrontendScanResults *pResults
    )
{
    NEXUS_Error errCode;
    BADS_ChannelScanSettings adsScanSettings;
    BADS_InbandParam params;
    NEXUS_CallbackDesc channelScanCallback;
    BADS_Status adsStatus;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pResults);

    /* set channel scan parameters */
    BKNI_Memset(&adsScanSettings, 0, sizeof(adsScanSettings));
    adsScanSettings.AI = true; /* auto spectral inversion detection */
    adsScanSettings.QM = true; /* enable constellation search */
    adsScanSettings.CO = true; /* enable carrier offset search */
    adsScanSettings.TO = true; /* enable auto symbol rate search */
    adsScanSettings.A256 = true;
    adsScanSettings.A128 = true;
    adsScanSettings.A64 = true;
    adsScanSettings.A32 = true; /* TODO: not in Nordig spec */
    adsScanSettings.A16 = true;
    adsScanSettings.carrierSearch = 500000/256; /* +/- 500KHz, unit *256Hz */
    adsScanSettings.upperBaudSearch = pDevice->upperBaudSearch;
    adsScanSettings.lowerBaudSearch = pDevice->lowerBaudSearch;

    errCode = BADS_SetScanParam(pDevice->channelHandle, &adsScanSettings);
    if (BERR_SUCCESS != errCode) return BERR_TRACE(errCode);

    pDevice->currentMode = SCAN_7552;
    pDevice->lastSettings.scan = *pSettings;

    if (pDevice->untuned) {
        /* Get interrupt event and register callback */
        errCode = BADS_GetInterruptEventHandle(pDevice->adsHandle, &pDevice->adsIsrEvent);
        if (BERR_SUCCESS != errCode) return BERR_TRACE(errCode);

        pDevice->adsIsrEventCallback = NEXUS_RegisterEvent(pDevice->adsIsrEvent, NEXUS_Frontend_P_7552_IsrEvent, pDevice);
        if (NULL == pDevice->adsIsrEventCallback) return BERR_TRACE(BERR_OS_ERROR);

        pDevice->untuned = false;
    }

    BKNI_ResetEvent(pDevice->channelScanEvent);
    BKNI_Memset(&channelScanCallback, 0, sizeof(channelScanCallback));
    channelScanCallback.callback = NEXUS_Frontend_P_7552_ChannelScanResult;
    channelScanCallback.context = pDevice;
    NEXUS_IsrCallback_Set(pDevice->adsLockAppCallback, &channelScanCallback);

    /* set inband parameters */
    BKNI_Memset(&params, 0, sizeof(params));
    params.modType = BADS_ModulationType_eAnnexAQam256;
    params.symbolRate = J83A_SYMBOLRATE_DEFAULT;
    params.invertSpectrum = BADS_InvertSpectrum_eNoInverted;
    params.spectrum = BADS_SpectrumMode_eAuto;
    params.enableDpm = false;
    params.autoAcquire = false;
    params.enableNullPackets = false;
    params.acquisitionType = BADS_AcquireType_eScan;
    errCode = BADS_Acquire(pDevice->channelHandle, &params);
    if (BERR_SUCCESS != errCode) return BERR_TRACE(errCode);

    errCode = BKNI_WaitForEvent(pDevice->channelScanEvent,
        pDevice->channelScanTimeout);
    if (BERR_SUCCESS != errCode) {
        (void) BERR_TRACE(errCode);
        BDBG_ERR(("Channel Scan result was not delivered in %d milliseconds, "
            "errCode %d", pDevice->channelScanTimeout, errCode));
        /* keep going */
    }

    errCode = BADS_GetStatus(pDevice->channelHandle, &adsStatus);
    if (BERR_SUCCESS != errCode) return BERR_TRACE(errCode);

    /* stop acquisition to prevent unnecessary callback */
    NEXUS_Frontend_P_7552_Untune(pDevice);

    if (adsStatus.isFecLock) {
        /* TODO: or to use adsStatus.isQamLock ? */
        BKNI_Memset(pResults, 0, sizeof*(pResults));
        pResults->signalType = NEXUS_FrontendSignalType_eDvbc;
        pResults->bandwidth = 8000000; /* 8MHz */
        pResults->spectrumInverted = adsStatus.isSpectrumInverted;
        pResults->frequencyOffset = adsStatus.carrierFreqOffset / 1000;
        pResults->symbolRate = adsStatus.rxSymbolRate;
        pResults->mode.qam =
            NEXUS_Frontend_P_Ads2NexusQamMode(adsStatus.modType);
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

static NEXUS_FrontendQamMode NEXUS_Frontend_P_Ads2NexusQamMode(
    BADS_ModulationType modulationType
    )
{
    switch (modulationType) {
    case BADS_ModulationType_eAnnexAQam16:
    case BADS_ModulationType_eAnnexBQam16:
    case BADS_ModulationType_eAnnexCQam16:
        return NEXUS_FrontendQamMode_e16;
        break;
    case BADS_ModulationType_eAnnexAQam32:
    case BADS_ModulationType_eAnnexBQam32:
    case BADS_ModulationType_eAnnexCQam32:
        return NEXUS_FrontendQamMode_e32;
        break;
    case BADS_ModulationType_eAnnexAQam64:
    case BADS_ModulationType_eAnnexBQam64:
    case BADS_ModulationType_eAnnexCQam64:
        return NEXUS_FrontendQamMode_e64;
        break;
    case BADS_ModulationType_eAnnexAQam128:
    case BADS_ModulationType_eAnnexBQam128:
    case BADS_ModulationType_eAnnexCQam128:
        return NEXUS_FrontendQamMode_e128;
        break;
    case BADS_ModulationType_eAnnexAQam256:
    case BADS_ModulationType_eAnnexBQam256:
    case BADS_ModulationType_eAnnexCQam256:
        return NEXUS_FrontendQamMode_e256;
        break;
    case BADS_ModulationType_eAnnexAQam1024:
    case BADS_ModulationType_eAnnexBQam1024:
    case BADS_ModulationType_eAnnexCQam1024:
        return NEXUS_FrontendQamMode_e1024;
        break;
    case BADS_ModulationType_eAnnexAQam2048:
    case BADS_ModulationType_eAnnexBQam2048:
    case BADS_ModulationType_eAnnexCQam2048:
        return NEXUS_FrontendQamMode_e2048;
        break;
    case BADS_ModulationType_eAnnexAQam4096:
    case BADS_ModulationType_eAnnexBQam4096:
    case BADS_ModulationType_eAnnexCQam4096:
        return NEXUS_FrontendQamMode_e4096;
        break;
    default:
        return NEXUS_FrontendQamMode_eMax;
        break;
    }

    return NEXUS_FrontendQamMode_eMax;
}

