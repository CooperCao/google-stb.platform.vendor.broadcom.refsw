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
*
* Revision History:
*
* $brcm_Log: $
* 
**************************************************************************/
#include "nexus_frontend_7552.h"
#include "nexus_frontend_7552_priv.h"

/***************************************************************************
 * file scope functions
 ***************************************************************************/

/***************************************************************************
 * Module callback functions for tuning
 ***************************************************************************/ 
static NEXUS_Error NEXUS_Frontend_P_7552_TuneOob(void *handle, const NEXUS_FrontendOutOfBandSettings *pSettings);
#if 0
static void NEXUS_Frontend_P_7552_Close(NEXUS_FrontendHandle handle);
static void NEXUS_Frontend_P_7552_Untune(void *handle);
static NEXUS_Error NEXUS_Frontend_P_7552_GetOobStatus(void *handle, NEXUS_FrontendOutOfBandStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_7552_GetSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length);
static void NEXUS_Frontend_P_7552_ResetStatus(void *handle);
#endif
/***************************************************************************
Summary:
    Lock callback handler for a 7552 Inband device
 ***************************************************************************/
static void NEXUS_Frontend_P_7552_LockChange_isr(void *pParam)
{
    NEXUS_IsrCallbackHandle callback = (NEXUS_IsrCallbackHandle)pParam;

    if (NULL != callback) {
        NEXUS_IsrCallback_Fire_isr(callback);
    }
}
#if 0
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
#endif

/***************************************************************************
Summary:
    Get the default settings for a BCM7552 Frontend
See Also:
    NEXUS_Frontend_Open7552
 ***************************************************************************/
void NEXUS_Frontend_GetDefault7552OutOfBandSettings(
    NEXUS_7552OutOfBandFrontendSettings *pSettings   /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof*(pSettings));
}

/***************************************************************************
Summary:
    Open a handle to a BCM7552 device.
 ***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_Open7552OutOfBand(
    const NEXUS_7552OutOfBandFrontendSettings *pSettings
    )
{
    BERR_Code errCode;
    NEXUS_FrontendHandle frontendHandle;
    NEXUS_7552 *pDevice=NULL;
    BAOB_Settings oobSettings;
    NEXUS_IsrCallbackHandle callbackLockChange;

    BDBG_ASSERT(NULL != pSettings);

    errCode = NEXUS_Frontend_P_Open7552();
    if ( errCode != BERR_SUCCESS ) goto err_scan_event;

    pDevice = p_7552device;

    /* get default settings and update */
    errCode = BAOB_GetDefaultSettings(&oobSettings, NULL);
    if ( errCode != BERR_SUCCESS ) goto err_scan_event;
    
    oobSettings.hTmr = g_pCoreHandles->tmr;
    oobSettings.hHeap = g_pCoreHandles->heap[0];

    /* Open AOB device */
    errCode = BAOB_Open(&pDevice->oobHandle, g_pCoreHandles->chp, g_pCoreHandles->reg, g_pCoreHandles->bint, &oobSettings);
    if ( errCode != BERR_SUCCESS ) goto err_scan_event;

    /* Create a Nexus frontend handle */
    frontendHandle = NEXUS_Frontend_P_Create(pDevice);
    if ( errCode != BERR_SUCCESS ) goto err_open_channel;

    /* Establish device capabilities */
    frontendHandle->capabilities.outOfBand = true;  
    frontendHandle->tuneOutOfBand = NEXUS_Frontend_P_7552_TuneOob;
#if 0
    frontendHandle->getOutOfBandStatus = NEXUS_Frontend_P_7552_GetOobStatus;    
    frontendHandle->close = NEXUS_Frontend_P_7552_Close;
    frontendHandle->untune = NEXUS_Frontend_P_7552_Untune;
    frontendHandle->getSoftDecisions = NEXUS_Frontend_P_7552_GetSoftDecisions;
    frontendHandle->resetStatus = NEXUS_Frontend_P_7552_ResetStatus;
#endif  
    callbackLockChange = NEXUS_IsrCallback_Create(frontendHandle, NULL);
    if ( errCode != BERR_SUCCESS ) goto err_frontendhandle;

    errCode = BAOB_InstallCallback(pDevice->oobHandle, BAOB_Callback_eLockChange, (BAOB_CallbackFunc)NEXUS_Frontend_P_7552_LockChange_isr, (void*)callbackLockChange);
    if ( errCode != BERR_SUCCESS ) goto err_lockchangeisr_callback;

    pDevice->oobLockAppCallback = callbackLockChange;
    pDevice->frontendcount++;

    /* Success */
    pDevice->oobSettings = *pSettings;
    return frontendHandle;

err_lockchangeisr_callback:
    NEXUS_IsrCallback_Destroy(callbackLockChange);
err_frontendhandle:
    NEXUS_Frontend_P_Destroy(frontendHandle);
err_open_channel:
    BAOB_Close(pDevice->oobHandle);
err_scan_event:
    return NULL;
}

static NEXUS_Error NEXUS_Frontend_P_7552_TuneOob(
    void *handle,
    const NEXUS_FrontendOutOfBandSettings *pSettings
    )
{
    NEXUS_Error rc;
    NEXUS_7552 *pDevice = (NEXUS_7552 *)handle;
    BTNR_PowerSaverSettings pwrSettings;
    BTNR_Settings tnrSettings;
    BAOB_AcquireParam obParams;
    
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7552);
    BDBG_ASSERT(NULL != pSettings);

    if (pDevice->oobTnrHandle)
    {
        /* TODO: we should be able to call BTNR_SetSettings after BTNR_SetPowerSaver. verify */
        pwrSettings.enable = false;     
        rc = BTNR_SetPowerSaver(pDevice->oobTnrHandle, &pwrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = BTNR_GetSettings(pDevice->oobTnrHandle, &tnrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}  

        tnrSettings.rfInputMode = pDevice->rfInput;       
        rc = BTNR_SetSettings(pDevice->oobTnrHandle, &tnrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        
        rc = BAOB_DisablePowerSaver(pDevice->oobHandle);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        
        rc = BTNR_SetTunerRfFreq(pDevice->oobTnrHandle, pSettings->frequency, BTNR_TunerMode_eDigital);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    obParams.autoAcquire = pSettings->autoAcquire;
    obParams.modType = (BAOB_ModulationType)pSettings->mode ;
    obParams.symbolRate = pSettings->symbolRate;
    obParams.berSrc = pSettings->bertSource;
    obParams.spectrum = pSettings->spectrum;

    rc = BAOB_Acquire(pDevice->oobHandle, &obParams);
    if(rc){rc = BERR_TRACE(rc); goto done;}
    
    return NEXUS_SUCCESS;
done:
    return rc;
}


#if 0
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

static NEXUS_Error NEXUS_Frontend_P_7552_GetOobStatus( void *handle, NEXUS_FrontendOobStatus *pStatus )
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
    pStatus->receiverLock = adsStatus.isOobLock;
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
#endif
