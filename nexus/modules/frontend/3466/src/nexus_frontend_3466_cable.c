/***************************************************************************
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
*
* API Description:
*   API name: Frontend 3466
*    APIs to open, close, and setup initial settings for a BCM3466
*    Cable Tuner/Demodulator Device.
*
***************************************************************************/
/* General includes */
#include "nexus_frontend_3466_priv.h"
/* End general includes */

/* Cable includes */
#include "bads_3466.h"
#include "bhab.h"
#include "bads.h"

#include "bchp_3466_leap_ctrl.h"
/* End cable includes */

BDBG_MODULE(nexus_frontend_3466_cable);

/* Cable-specific functions */
static NEXUS_Error NEXUS_Frontend_P_3466_TuneQam(void *handle, const NEXUS_FrontendQamSettings *pSettings);
static void NEXUS_Frontend_P_3466_UnTuneQam(void *handle);
static NEXUS_Error NEXUS_Frontend_P_3466_GetQamScanStatus(void *handle, NEXUS_FrontendQamScanStatus *pScanStatus);
static NEXUS_Error NEXUS_Frontend_P_3466_GetQamStatus(void *handle, NEXUS_FrontendQamStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_3466_RequestQamAsyncStatus(void *handle);
static NEXUS_Error NEXUS_Frontend_P_3466_GetQamAsyncStatus(void *handle, NEXUS_FrontendQamStatus *pStatus);
static void NEXUS_Frontend_P_3466_ResetQamStatus(void *handle);
static NEXUS_Error NEXUS_Frontend_P_3466_ReadSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length, size_t *pNumRead);
static void NEXUS_Frontend_P_3466_Close(NEXUS_FrontendHandle handle);
static NEXUS_Error NEXUS_Frontend_P_3466_GetFastStatus(void *handle, NEXUS_FrontendFastStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_3466_Standby_Cable(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings);
/* End of cable-specific function declarations */

static void NEXUS_Frontend_P_3466_spectrumDataReadyCallback(void *pParam)
{
    BDBG_ASSERT(NULL != pParam);
    BDBG_MSG(("NEXUS_Frontend_P_3466_spectrumDataReadyCallback: %p", (void *)pParam));
    BKNI_SetEvent((BKNI_EventHandle)pParam);
}

#if 0
static void NEXUS_Frontend_P_3466_spectrumEventCallback(void *pParam)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BADS_SpectrumData  spectrumData;
    NEXUS_3466Channel *pChannel;
    NEXUS_3466Device *pDevice;
    BDBG_ASSERT(NULL != pParam);
    pChannel = (NEXUS_3466Channel *)pParam;
    pDevice = (NEXUS_3466Device *)pChannel->pDevice;

    BDBG_MSG(("NEXUS_Frontend_P_3466_spectrumEventCallback: %p", (void *)pParam));

    spectrumData.data = pDevice->cable.spectrumDataPointer;

    rc = BADS_GetSpectrumAnalyzerData(pDevice->cable.ads_chn[pChannel->chn_num], &spectrumData);
    if (rc){rc = BERR_TRACE(rc);  goto done;}

    pDevice->cable.spectrumDataLength += spectrumData.datalength;
    pDevice->cable.spectrumDataPointer += spectrumData.datalength;

    BDBG_MSG(("NEXUS_Frontend_P_3466_spectrumEventCallback: pDevice->spectrumDataAppCallback[%d]: %p", pChannel->chn_num, (void *)pDevice->cable.spectrumDataAppCallback[pChannel->chn_num]));
    if (!spectrumData.moreData) {
        if (pDevice->cable.spectrumDataAppCallback[pChannel->chn_num])
        {
            pDevice->cable.spectrumDataLength = 0;
            NEXUS_TaskCallback_Fire(pDevice->cable.spectrumDataAppCallback[pChannel->chn_num]);
        }
    }

done:
    return;
}
#endif

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

    BDBG_MSG(("NEXUS_Frontend_P_3466_callback_isr"));

    if(pDevice->cable.acquireInProgress[pChannel->chn_num]){
        pDevice->cable.count[pChannel->chn_num]++;
    }
    if(pDevice->cable.count[pChannel->chn_num] == 2){
        pDevice->cable.acquireInProgress[pChannel->chn_num] = false;
        pDevice->cable.count[pChannel->chn_num] = 0;
    }

    if (pDevice->cable.lockAppCallback[pChannel->chn_num])
    {
        NEXUS_IsrCallback_Fire_isr(pDevice->cable.lockAppCallback[pChannel->chn_num]);
    }
}

/***************************************************************************
Summary:
    Lock callback handler for a 3466 Inband device
 ***************************************************************************/
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

    if(!pDevice->cable.isInternalAsyncStatusCall[pChannel->chn_num]){
        if (pDevice->cable.asyncStatusAppCallback[pChannel->chn_num])
        {
            NEXUS_IsrCallback_Fire_isr(pDevice->cable.asyncStatusAppCallback[pChannel->chn_num]);
        }
    }
}


/* Cable frontend device implementation */
NEXUS_Error NEXUS_FrontendDevice_Open3466_Cable(NEXUS_3466Device *pDevice)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_MSG(("init qam channels"));
    /* Initialize QAM channels */
    rc = NEXUS_FrontendDevice_P_Init3466_Cable(pDevice);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pDevice->cable.numChannels = pDevice->capabilities.totalTunerChannels;

    pDevice->cable.api.getFastStatus = NEXUS_Frontend_P_3466_GetFastStatus;
    pDevice->cable.api.readSoftDecisions = NEXUS_Frontend_P_3466_ReadSoftDecisions;
    pDevice->cable.api.untune = NEXUS_Frontend_P_3466_UnTuneQam;
    pDevice->cable.api.close = NEXUS_Frontend_P_3466_Close;
    pDevice->cable.api.standby = NEXUS_Frontend_P_3466_Standby_Cable;
#if 0
    pDevice->cable.api.uninstallCallbacks = NEXUS_Frontend_P_3466_UninstallCallbacks;
#endif

done:
    return rc;
}
/* End cable frontend device implementation */

/* Cable implementation */

static NEXUS_Error NEXUS_Frontend_CreateCallbacks(NEXUS_FrontendHandle frontendHandle)
{
    NEXUS_3466Device *p3466Device = NULL;
    NEXUS_3466Channel *pChannel;
    NEXUS_Error rc;
    unsigned channelNumber = 0;
    NEXUS_IsrCallbackHandle qamAsyncStatusReadyCallback = NULL;
    NEXUS_TaskCallbackHandle spectrumDataCallback  = NULL;
    NEXUS_IsrCallbackHandle lockCallback;

    pChannel = (NEXUS_3466Channel *)frontendHandle->pDeviceHandle;
    p3466Device = (NEXUS_3466Device *)pChannel->pDevice;
    channelNumber = pChannel->chn_num;

    /* lock callback */
    if (!p3466Device->cable.lockAppCallback[channelNumber]) {
        lockCallback = NEXUS_IsrCallback_Create(frontendHandle, NULL);
        if ( NULL == lockCallback ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err;}
        p3466Device->cable.lockAppCallback[channelNumber] = lockCallback;
    }

    if (!p3466Device->cable.asyncStatusAppCallback[channelNumber]) {
        qamAsyncStatusReadyCallback = NEXUS_IsrCallback_Create(frontendHandle, NULL);
        if ( NULL == qamAsyncStatusReadyCallback ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err;}
        p3466Device->cable.asyncStatusAppCallback[channelNumber] = qamAsyncStatusReadyCallback;
    }

    if (!p3466Device->cable.spectrumDataAppCallback[channelNumber]) {
        spectrumDataCallback = NEXUS_TaskCallback_Create(frontendHandle, NULL);
        if ( NULL == spectrumDataCallback ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err;}
        p3466Device->cable.spectrumDataAppCallback[channelNumber] = spectrumDataCallback;
    }

    return NEXUS_SUCCESS;
err:
    return rc;

}
NEXUS_FrontendHandle NEXUS_Frontend_Open3466_Cable(const NEXUS_FrontendChannelSettings *pSettings, NEXUS_FrontendHandle frontendHandle)
{
    BERR_Code rc;

    BSTD_UNUSED(pSettings);
    /* Set capabilities and channel type parameters */
    frontendHandle->capabilities.qam = true;
    frontendHandle->capabilities.outOfBand = false;
    frontendHandle->capabilities.upstream = false;
    BKNI_Memset(frontendHandle->capabilities.qamModes, true, sizeof(frontendHandle->capabilities.qamModes));
    /* bind functions*/
    frontendHandle->tuneQam = NEXUS_Frontend_P_3466_TuneQam;
    frontendHandle->getQamStatus = NEXUS_Frontend_P_3466_GetQamStatus;
    frontendHandle->requestQamAsyncStatus = NEXUS_Frontend_P_3466_RequestQamAsyncStatus;
    frontendHandle->getQamAsyncStatus = NEXUS_Frontend_P_3466_GetQamAsyncStatus;
    frontendHandle->resetStatus = NEXUS_Frontend_P_3466_ResetQamStatus;
    frontendHandle->getQamScanStatus = NEXUS_Frontend_P_3466_GetQamScanStatus;

    frontendHandle->reapplyTransportSettings = NEXUS_Frontend_P_3466_ReapplyTransportSettings;
#if 0
    frontendHandle->requestSpectrumData = NEXUS_Frontend_P_3466_RequestSpectrumAnalyzerData;
    frontendHandle->standby = NEXUS_Frontend_P_3466_Standby;
    frontendHandle->uninstallCallbacks = NEXUS_Frontend_P_3466_UninstallCallbacks;
#endif
    rc = NEXUS_Frontend_CreateCallbacks(frontendHandle);
    if (rc) { BERR_TRACE(rc); goto err; }

    return frontendHandle;
err:
    return NULL;
}
/* End of cable channel open implementation */

static void NEXUS_Frontend_P_Uninit3466_Cable(NEXUS_3466Device *pDevice, bool uninitHab)
{
    unsigned i;

    BSTD_UNUSED(uninitHab);

    for (i=0; i < pDevice->cable.numChannels; i++) {
        if (pDevice->cable.ads_chn[i]) {
            BADS_InstallCallback(pDevice->cable.ads_chn[i], BADS_Callback_eLockChange, NULL, (void*)&pDevice->frontendHandle[i]);
            BADS_InstallCallback(pDevice->cable.ads_chn[i], BADS_Callback_eNoSignal, NULL, (void*)&pDevice->frontendHandle[i]);
            if (pDevice->cable.lockAppCallback[i]) {
                NEXUS_IsrCallback_Destroy(pDevice->cable.lockAppCallback[i]);
                pDevice->cable.lockAppCallback[i] = NULL;
            }
            if (pDevice->cable.asyncStatusAppCallback[i]) {
                NEXUS_IsrCallback_Destroy(pDevice->cable.asyncStatusAppCallback[i]);
                pDevice->cable.asyncStatusAppCallback[i] = NULL;
            }
            if (pDevice->cable.spectrumDataAppCallback[i]) {
                NEXUS_TaskCallback_Destroy(pDevice->cable.spectrumDataAppCallback[i]);
                pDevice->cable.spectrumDataAppCallback[i] = NULL;
            }
            if(pDevice->tnr[i]){
                BTNR_Close(pDevice->tnr[i]);
                pDevice->tnr[i] = NULL;
            }
            BADS_CloseChannel(pDevice->cable.ads_chn[i]);
            pDevice->cable.ads_chn[i] = NULL;
        }
    }
    if (pDevice->cable.spectrumEvent) {
        BKNI_DestroyEvent(pDevice->cable.spectrumEvent);
        pDevice->cable.spectrumEvent = NULL;
    }
    if (pDevice->cable.ads) {
        BADS_Close(pDevice->cable.ads);
        pDevice->cable.ads = NULL;
    }
}

void NEXUS_FrontendDevice_Close3466_Cable(NEXUS_3466Device *pFrontendDevice)
{
    /* Cable teardown */
    if (pFrontendDevice) {
        NEXUS_Frontend_P_Uninit3466_Cable(pFrontendDevice, true);
    }
    /* End of cable teardown */
}

static NEXUS_Error NEXUS_Frontend_P_3466_Standby_Cable(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3466Channel *pChannel = (NEXUS_3466Channel *)handle;
    NEXUS_3466Device *pDevice;
    pDevice = pChannel->pDevice;

    BSTD_UNUSED(enabled);

    if (pSettings->mode != NEXUS_StandbyMode_eDeepSleep) {
        rc = NEXUS_Frontend_CreateCallbacks(pDevice->frontendHandle[pChannel->chn_num]);
        if (rc) { rc = BERR_TRACE(rc); goto done;}
    }
done:
    return rc;
}

NEXUS_Error NEXUS_FrontendDevice_P_Init3466_Cable(NEXUS_3466Device *pDevice)
{
    BADS_Settings adsConfig;
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned totalChannels = 0;
    unsigned i;

    /* Cable initialization (shared) */
    /* Open ADS */
    rc = BADS_3466_GetDefaultSettings( &adsConfig, NULL);
    if (rc) { rc = BERR_TRACE(rc); goto done; }

    adsConfig.hGeneric = pDevice->hab;
#if 0
    adsConfig.transportConfig = BADS_TransportData_eSerial;
    adsConfig.isOpenDrain = pDevice->openSettings.inBandOpenDrain;
#endif
    BDBG_MSG(("BADS_Open"));
    rc = BADS_Open(&pDevice->cable.ads, NULL, NULL, NULL, &adsConfig);
    if (rc) { rc = BERR_TRACE(rc); goto done; }
    BDBG_MSG(("BADS_Init"));
    rc = BADS_Init(pDevice->cable.ads);
    if (rc) { rc = BERR_TRACE(rc); goto done; }

    BDBG_MSG(("BADS_GetTotalChannels"));
    rc = BADS_GetTotalChannels(pDevice->cable.ads, &totalChannels);
    if (rc) { rc = BERR_TRACE(rc); goto done; }

    /* TODO HACK returning incorrect values, only init 1 channel */
    if (totalChannels > 2) totalChannels = 2;

    for (i=0; i < totalChannels; i++) {
        BADS_ChannelSettings adsChannelSettings;

        BDBG_MSG(("opening ADS channel %d", i));

        rc = BADS_GetChannelDefaultSettings( pDevice->cable.ads, i, &adsChannelSettings);
        if (rc) { rc = BERR_TRACE(rc); goto done; }

        rc = BADS_OpenChannel( pDevice->cable.ads, &pDevice->cable.ads_chn[i], i, &adsChannelSettings);
        if (rc) { rc = BERR_TRACE(rc); goto done; }

        rc = BADS_InstallCallback(pDevice->cable.ads_chn[i], BADS_Callback_eLockChange, (BADS_CallbackFunc)NEXUS_Frontend_P_3466_callback_isr, (void*)&pDevice->frontendHandle[i]);
        if (rc) { rc = BERR_TRACE(rc); goto done; }
        rc = BADS_InstallCallback(pDevice->cable.ads_chn[i], BADS_Callback_eNoSignal, (BADS_CallbackFunc)NEXUS_Frontend_P_3466_callback_isr, (void*)&pDevice->frontendHandle[i]);
        if (rc) { rc = BERR_TRACE(rc); goto done; }
        rc = BADS_InstallCallback(pDevice->cable.ads_chn[i], BADS_Callback_eAsyncStatusReady, (BADS_CallbackFunc)NEXUS_Frontend_P_3466_AsyncStatusCallback_isr, (void*)&pDevice->frontendHandle[i]);
        if (rc) { rc = BERR_TRACE(rc); goto done; }
#if 0
        /* TODO */
        rc = BADS_InstallCallback(pDevice->cable.ads_chn[adsChannels], BADS_Callback_eUpdateGain, (BADS_CallbackFunc)NEXUS_Frontend_P_3466_updateGainCallback_isr, (void*)pDevice->updateGainAppCallback);
        if (rc) { rc = BERR_TRACE(rc); goto done; }
#endif
        rc = BADS_InstallCallback(pDevice->cable.ads_chn[i], BADS_Callback_eSpectrumDataReady, (BADS_CallbackFunc)NEXUS_Frontend_P_3466_spectrumDataReadyCallback, (void*)pDevice->cable.spectrumEvent);
        if (rc) { rc = BERR_TRACE(rc); goto done; }
#if 0
        if (pDevice->cable.ifDacPresent && i==pDevice->cable.ifDacChannelNumber) {
            rc = BADS_InstallCallback(pDevice->cable.ads_chn[pDevice->ifDacChannelNumber], BADS_Callback_eIfDacAcquireComplete, (BADS_CallbackFunc)NEXUS_Tuner_P_3466_Callback, (void *)pDevice);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            rc = BADS_InstallCallback(pDevice->cable.ads_chn[pDevice->ifDacChannelNumber], BADS_Callback_eIfDacStatusReady, (BADS_CallbackFunc)NEXUS_Frontend_P_3466_TunerAsyncStatusCallback_isr, (void *)pDevice);
            if(rc){rc = BERR_TRACE(rc); goto done;}
        }
#endif
    }

    /* End of cable initialization (shared) */

    return NEXUS_SUCCESS;

done:
    return rc;
}

void NEXUS_FrontendDevice_P_Uninit3466_Cable(NEXUS_3466Device *pDevice)
{
    /* Cable uninitialization (shared) */
    NEXUS_Frontend_P_Uninit3466_Cable(pDevice, true);
    /* End of cable uninitialization (shared) */
}


void NEXUS_FrontendDevice_P_Uninit_3466_Hab_Cable(NEXUS_3466Device *pDevice)
{
    BSTD_UNUSED(pDevice);

    return;
}

NEXUS_Error NEXUS_FrontendDevice_P_Init_3466_Hab_Cable(NEXUS_3466Device *pDevice, const NEXUS_FrontendDeviceOpenSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BSTD_UNUSED(pDevice);
    BSTD_UNUSED(pSettings);

    return rc;

#if 0
done:
    NEXUS_FrontendDevice_P_Uninit_3466_Hab(pDevice);
    return rc;
#endif
}


static void NEXUS_Frontend_P_QamToModulationType(BADS_ModulationType modType, NEXUS_FrontendQamAnnex *pAnnex, NEXUS_FrontendQamMode *pMode)
{
    switch ( modType )
    {
    case BADS_ModulationType_eAnnexAQam16:
        *pAnnex = NEXUS_FrontendQamAnnex_eA; *pMode = NEXUS_FrontendQamMode_e16;
        break;
    case BADS_ModulationType_eAnnexAQam32:
        *pAnnex = NEXUS_FrontendQamAnnex_eA; *pMode = NEXUS_FrontendQamMode_e32;
        break;
    case BADS_ModulationType_eAnnexAQam64:
        *pAnnex = NEXUS_FrontendQamAnnex_eA; *pMode = NEXUS_FrontendQamMode_e64;
        break;
    case BADS_ModulationType_eAnnexAQam128:
        *pAnnex = NEXUS_FrontendQamAnnex_eA; *pMode = NEXUS_FrontendQamMode_e128;
        break;
    case BADS_ModulationType_eAnnexAQam256:
        *pAnnex = NEXUS_FrontendQamAnnex_eA; *pMode = NEXUS_FrontendQamMode_e256;
        break;
    case BADS_ModulationType_eAnnexAQam512:
        *pAnnex = NEXUS_FrontendQamAnnex_eA; *pMode = NEXUS_FrontendQamMode_e512;
        break;
    case BADS_ModulationType_eAnnexAQam1024:
        *pAnnex = NEXUS_FrontendQamAnnex_eA; *pMode = NEXUS_FrontendQamMode_e1024;
        break;
    case BADS_ModulationType_eAnnexAQam2048:
        *pAnnex = NEXUS_FrontendQamAnnex_eA; *pMode = NEXUS_FrontendQamMode_e2048;
        break;
    case BADS_ModulationType_eAnnexAQam4096:
        *pAnnex = NEXUS_FrontendQamAnnex_eA; *pMode = NEXUS_FrontendQamMode_e4096;
        break;

    case BADS_ModulationType_eAnnexBQam16:
        *pAnnex = NEXUS_FrontendQamAnnex_eB; *pMode = NEXUS_FrontendQamMode_e16;
        break;
    case BADS_ModulationType_eAnnexBQam32:
        *pAnnex = NEXUS_FrontendQamAnnex_eB; *pMode = NEXUS_FrontendQamMode_e32;
        break;
    case BADS_ModulationType_eAnnexBQam64:
        *pAnnex = NEXUS_FrontendQamAnnex_eB; *pMode = NEXUS_FrontendQamMode_e64;
        break;
    case BADS_ModulationType_eAnnexBQam128:
        *pAnnex = NEXUS_FrontendQamAnnex_eB; *pMode = NEXUS_FrontendQamMode_e128;
        break;
    case BADS_ModulationType_eAnnexBQam256:
        *pAnnex = NEXUS_FrontendQamAnnex_eB; *pMode = NEXUS_FrontendQamMode_e256;
        break;
    case BADS_ModulationType_eAnnexBQam512:
        *pAnnex = NEXUS_FrontendQamAnnex_eB; *pMode = NEXUS_FrontendQamMode_e512;
        break;
    case BADS_ModulationType_eAnnexBQam1024:
        *pAnnex = NEXUS_FrontendQamAnnex_eB; *pMode = NEXUS_FrontendQamMode_e1024;
        break;
    case BADS_ModulationType_eAnnexBQam2048:
        *pAnnex = NEXUS_FrontendQamAnnex_eB; *pMode = NEXUS_FrontendQamMode_e2048;
        break;
    case BADS_ModulationType_eAnnexBQam4096:
        *pAnnex = NEXUS_FrontendQamAnnex_eB; *pMode = NEXUS_FrontendQamMode_e4096;
        break;

    case BADS_ModulationType_eAnnexCQam16:
        *pAnnex = NEXUS_FrontendQamAnnex_eC; *pMode = NEXUS_FrontendQamMode_e16;
        break;
    case BADS_ModulationType_eAnnexCQam32:
        *pAnnex = NEXUS_FrontendQamAnnex_eC; *pMode = NEXUS_FrontendQamMode_e32;
        break;
    case BADS_ModulationType_eAnnexCQam64:
        *pAnnex = NEXUS_FrontendQamAnnex_eC; *pMode = NEXUS_FrontendQamMode_e64;
        break;
    case BADS_ModulationType_eAnnexCQam128:
        *pAnnex = NEXUS_FrontendQamAnnex_eC; *pMode = NEXUS_FrontendQamMode_e128;
        break;
    case BADS_ModulationType_eAnnexCQam256:
        *pAnnex = NEXUS_FrontendQamAnnex_eC; *pMode = NEXUS_FrontendQamMode_e256;
        break;
    case BADS_ModulationType_eAnnexCQam512:
        *pAnnex = NEXUS_FrontendQamAnnex_eC; *pMode = NEXUS_FrontendQamMode_e512;
        break;
    case BADS_ModulationType_eAnnexCQam1024:
        *pAnnex = NEXUS_FrontendQamAnnex_eC; *pMode = NEXUS_FrontendQamMode_e1024;
        break;
    case BADS_ModulationType_eAnnexCQam2048:
        *pAnnex = NEXUS_FrontendQamAnnex_eC; *pMode = NEXUS_FrontendQamMode_e2048;
        break;
    case BADS_ModulationType_eAnnexCQam4096:
        *pAnnex = NEXUS_FrontendQamAnnex_eC; *pMode = NEXUS_FrontendQamMode_e4096;
        break;

    default:
        BDBG_WRN(("Unrecognized QAM Modultation Type: value=%d", modType));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        *pAnnex = NEXUS_FrontendQamAnnex_eA; *pMode = NEXUS_FrontendQamMode_e256;
        return;
    }
}

static NEXUS_Error NEXUS_Frontend_P_3466_GetQamScanStatus(void *handle, NEXUS_FrontendQamScanStatus *pScanStatus)
{
    NEXUS_Error  rc;
    struct BADS_ScanStatus st;
    NEXUS_3466Device *pDevice;
    NEXUS_3466Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3466Channel *)handle;
    pDevice = (NEXUS_3466Device *)pChannel->pDevice;

    BKNI_Memset(pScanStatus, 0, sizeof(*pScanStatus));

    if (pChannel->chn_num >= NEXUS_MAX_3466_C_FRONTENDS)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    rc = BADS_GetScanStatus(pDevice->cable.ads_chn[pChannel->chn_num],  &st);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pScanStatus->spectrumInverted = st.isSpectrumInverted;
    pScanStatus->symbolRate = st.symbolRate;
    pScanStatus->frequencyOffset = st.carrierFreqOffset;
    pScanStatus->interleaver = st.interleaver;
    pScanStatus->acquisitionStatus = st.acquisitionStatus;
    if(st.modType < BADS_ModulationType_eAnnexCQam16) {
        NEXUS_Frontend_P_QamToModulationType(st.modType, &pScanStatus->annex, &pScanStatus->mode);
    }
    else
        BDBG_ERR(("Unsupported Annex C."));
    return BERR_SUCCESS;
done:
    return rc;

}

static NEXUS_Error NEXUS_Frontend_P_3466_TuneQam(void *handle, const NEXUS_FrontendQamSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3466Device *pDevice;
    NEXUS_3466Channel *pChannel;
    BADS_InbandParam ibParam;
    BTNR_PowerSaverSettings pwrSettings;
    unsigned temp_frequency;

    BDBG_ASSERT(handle);
    pChannel = (NEXUS_3466Channel *)handle;
    pDevice = pChannel->pDevice;

    pDevice->lastTuneQam = true;

    if (!pDevice->pLinkedMtsifDevice) {
        BDBG_MSG(("Current mtsif input band: %d, channel: %d", pDevice->frontendHandle[pChannel->chn_num]->mtsif.inputBand, pChannel->chn_num));
        pDevice->frontendHandle[pChannel->chn_num]->mtsif.inputBand = pChannel->chn_num;
        BDBG_MSG(("NEW mtsif input band: %d, channel: %d", pDevice->frontendHandle[pChannel->chn_num]->mtsif.inputBand, pChannel->chn_num));
    }
    rc = NEXUS_Frontend_P_3466_SetMtsifConfig(pDevice->frontendHandle[pChannel->chn_num]);
    if (rc) { rc = BERR_TRACE(rc); goto done; }

    temp_frequency = pDevice->cable.last_ads[pChannel->chn_num].frequency;
    pDevice->cable.last_ads[pChannel->chn_num].frequency = pSettings->frequency;

#if 0
    BDBG_MSG(("checking whether or not to tune tuner"));
    if((!BKNI_Memcmp(pSettings, &pDevice->cable.last_ads[pChannel->chn_num], sizeof(NEXUS_FrontendQamSettings))) && (pSettings->acquisitionMode == NEXUS_FrontendQamAcquisitionMode_eScan))
#endif
    {
        BDBG_MSG(("tuning tuner"));
        if (pDevice->tnr[pChannel->chn_num])
        {
            BHAB_ConfigSettings habSettings;
            BTNR_Settings tnrSettings;

            rc = BHAB_GetConfigSettings(pDevice->hab, &habSettings);
            if (habSettings.tnrApplication != BHAB_TunerApplication_eCable) {
                habSettings.tnrApplication = BHAB_TunerApplication_eCable;
                rc = BHAB_SetConfigSettings(pDevice->hab, &habSettings);
                if(rc){rc = BERR_TRACE(rc); goto done;}
            }

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
            tnrSettings.std = BTNR_Standard_eQam;

            rc = BTNR_SetSettings(pDevice->tnr[pChannel->chn_num], &tnrSettings);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            pDevice->cable.count[pChannel->chn_num] = 0;
            pDevice->cable.acquireInProgress[pChannel->chn_num] = true;
            BDBG_MSG(("setting tuner rf frequency"));
            rc = BTNR_SetTunerRfFreq(pDevice->tnr[pChannel->chn_num], pSettings->frequency, BTNR_TunerMode_eDigital);
            if(rc){rc = BERR_TRACE(rc); goto retrack;}

            pDevice->cable.last_ads[pChannel->chn_num] = *pSettings;
        }
    }

    if(!pDevice->cable.isPoweredOn[pChannel->chn_num]){
        BDBG_MSG(("turning downstream channel on"));
        rc = BADS_DisablePowerSaver(pDevice->cable.ads_chn[pChannel->chn_num]);
        if(rc){rc = BERR_TRACE(rc); goto retrack;}

        pDevice->cable.isPoweredOn[pChannel->chn_num] = true;
    }

    rc = BADS_GetDefaultAcquireParams(pDevice->cable.ads_chn[pChannel->chn_num], &ibParam);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if (pSettings->annex == NEXUS_FrontendQamAnnex_eB) {
        BDBG_MSG(("AnnexB"));
        switch (pSettings->mode) {
        case NEXUS_FrontendQamMode_e64:
            ibParam.modType = BADS_ModulationType_eAnnexBQam64; break;
        case NEXUS_FrontendQamMode_e256:
            ibParam.modType = BADS_ModulationType_eAnnexBQam256; break;
        case NEXUS_FrontendQamMode_e1024:
            ibParam.modType = BADS_ModulationType_eAnnexBQam1024; break;
        default:
            rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
            break;
        }
    } else if (pSettings->annex == NEXUS_FrontendQamAnnex_eA || pSettings->annex == NEXUS_FrontendQamAnnex_eC){
        BDBG_MSG(("Annex%s", (pSettings->annex==NEXUS_FrontendQamAnnex_eA) ? "A" : "C"));
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
        ibParam.enableNullPackets = pSettings->enableNullPackets;
        if( pSettings->annex == NEXUS_FrontendQamAnnex_eC ){
            if( (pSettings->symbolRate != 5274000) && (pSettings->symbolRate != 5307000))
                BDBG_WRN((" Symbol rate for Annex-C is incorrect."));
        }
    } else {
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }

    NEXUS_IsrCallback_Set(pDevice->cable.lockAppCallback[pChannel->chn_num], &(pSettings->lockCallback));
    NEXUS_IsrCallback_Set(pDevice->cable.asyncStatusAppCallback[pChannel->chn_num], &(pSettings->asyncStatusReadyCallback));

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
        if(pSettings->scan.mode[NEXUS_FrontendQamAnnex_eB][NEXUS_FrontendQamMode_e64]) scanParam.B64 = true;
        if(pSettings->scan.mode[NEXUS_FrontendQamAnnex_eB][NEXUS_FrontendQamMode_e256]) scanParam.B256 = true;
        if(pSettings->scan.mode[NEXUS_FrontendQamAnnex_eB][NEXUS_FrontendQamMode_e1024]) scanParam.B1024 = true;
        if(pSettings->scan.mode[NEXUS_FrontendQamAnnex_eC][NEXUS_FrontendQamMode_e64]) scanParam.A64 = true;
        if(pSettings->scan.mode[NEXUS_FrontendQamAnnex_eC][NEXUS_FrontendQamMode_e256]) scanParam.A256 = true;
        if(pSettings->scan.frequencyOffset){
            scanParam.CO = true;
            scanParam.carrierSearch = pSettings->scan.frequencyOffset/256;
        }
        scanParam.upperBaudSearch = pSettings->scan.upperBaudSearch;
        scanParam.lowerBaudSearch = pSettings->scan.lowerBaudSearch;

        rc = BADS_SetScanParam(pDevice->cable.ads_chn[pChannel->chn_num], &scanParam );
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

    rc = BADS_SetAcquireParams(pDevice->cable.ads_chn[pChannel->chn_num], &ibParam );
    if(rc){rc = BERR_TRACE(rc); goto done;}

    BDBG_MSG(("bads acquire..."));
    rc = BADS_Acquire(pDevice->cable.ads_chn[pChannel->chn_num], &ibParam );
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pDevice->cable.acquireInProgress[pChannel->chn_num] = true;
    pDevice->cable.last_ads[pChannel->chn_num] = *pSettings;

done:
    return rc;

retrack:
    pDevice->cable.last_ads[pChannel->chn_num].frequency = temp_frequency;
    return rc;
}

static void NEXUS_Frontend_P_3466_UnTuneQam(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3466Device *pDevice;
    NEXUS_3466Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3466Channel *)handle;
    pDevice = pChannel->pDevice;

    if (pChannel->chn_num >= NEXUS_MAX_3466_C_FRONTENDS)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    NEXUS_Frontend_P_3466_UnsetMtsifConfig(pDevice->frontendHandle[pChannel->chn_num]);

    if(pDevice->cable.isPoweredOn[pChannel->chn_num]){
#if 0
        BADS_InbandParam ibParam;

        /* TODO cleanup untune */
        BKNI_Memset(&ibParam, 0x0, sizeof(ibParam));
        rc = BADS_SetAcquireParams(pDevice->cable.ads_chn[pChannel->chn_num], &ibParam );
        if(rc){rc = BERR_TRACE(rc); goto done;}
#endif

        rc = BADS_EnablePowerSaver(pDevice->cable.ads_chn[pChannel->chn_num]);
       if(rc){rc = BERR_TRACE(rc); goto done;}

       pDevice->cable.isPoweredOn[pChannel->chn_num] = false;
    }

    BKNI_Memset(&pDevice->cable.last_ads[pChannel->chn_num], 0x0, sizeof(NEXUS_FrontendQamSettings));

done:
    return;
}

#define BHAB_ADS_CHN0_STATUS_RDY    0x00010000
static NEXUS_Error NEXUS_Frontend_P_3466_GetQamStatus(void *handle, NEXUS_FrontendQamStatus *pStatus)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned j=0;
    uint32_t buf=0;
    NEXUS_3466Device *p3466Device;
    NEXUS_3466Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3466Channel *)handle;
    p3466Device = (NEXUS_3466Device *)pChannel->pDevice;

    if (pChannel->chn_num >= NEXUS_MAX_3466_C_FRONTENDS)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    if (p3466Device->cable.isPoweredOn[pChannel->chn_num]) {
        uint32_t addr = BCHP_LEAP_CTRL_SW_SPARE0;
        uint32_t mask = BHAB_ADS_CHN0_STATUS_RDY;

        if (pChannel->chn_num > 7) {
            addr = BCHP_LEAP_CTRL_GP7;
            mask = 1;
        }
        p3466Device->cable.isInternalAsyncStatusCall[pChannel->chn_num] = true;

        rc = NEXUS_Frontend_P_3466_RequestQamAsyncStatus(pChannel);
        if (rc) { rc = BERR_TRACE(rc); goto done; }

        for (j=0; j < 200; j++) {

            BKNI_Sleep(20);

            rc = BHAB_ReadRegister(p3466Device->hab, addr, &buf);
            if (rc) { rc = BERR_TRACE(rc); goto done; }

            if (buf & (mask << pChannel->chn_num % 8)) {
                rc = NEXUS_Frontend_P_3466_GetQamAsyncStatus(pChannel, pStatus);
                if (rc) { rc = BERR_TRACE(rc); goto done; }
                break;
            }
        }
    }
    else{
        BDBG_MSG(("The downstream core is Powered Off"));
    }

    p3466Device->cable.isInternalAsyncStatusCall[pChannel->chn_num] = false;
    return BERR_SUCCESS;
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_3466_RequestQamAsyncStatus(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3466Device *p3466Device;
    NEXUS_3466Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = handle;
    p3466Device = (NEXUS_3466Device *)pChannel->pDevice;

    if (pChannel->chn_num >= NEXUS_MAX_3466_C_FRONTENDS)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    rc = BADS_RequestAsyncStatus(p3466Device->cable.ads_chn[pChannel->chn_num]);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    return BERR_SUCCESS;
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_3466_GetQamAsyncStatus(void *handle, NEXUS_FrontendQamStatus *pStatus)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    struct BADS_Status st;
    uint64_t totalbits=0, uncorrectedBits=0;
    unsigned cleanBlock = 0, correctedBlock = 0, unCorrectedBlock = 0, totalBlock = 0;
    NEXUS_Time currentTime;
    NEXUS_3466PreviousStatus *prevStatus;
    NEXUS_3466Device *p3466Device;
    NEXUS_3466Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3466Channel *)handle;
    p3466Device = (NEXUS_3466Device *)pChannel->pDevice;

    if (pChannel->chn_num >= NEXUS_MAX_3466_C_FRONTENDS)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    rc = BADS_GetAsyncStatus(p3466Device->cable.ads_chn[pChannel->chn_num],  &st);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    prevStatus = &p3466Device->cable.previousStatus[pChannel->chn_num];
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
    pStatus->settings = p3466Device->cable.last_ads[pChannel->chn_num];
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

static void NEXUS_Frontend_P_3466_ResetQamStatus(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3466Channel *pChannel;
    NEXUS_3466Device *pDevice;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3466Channel *)handle;
    pDevice = (NEXUS_3466Device *)pChannel->pDevice;

    if(pDevice->cable.isPoweredOn[pChannel->chn_num]) {
        rc = BADS_ResetStatus(pDevice->cable.ads_chn[pChannel->chn_num]);
        if (rc){BERR_TRACE(rc);}
    }
    else{
        BDBG_MSG(("The downstream core is Powered Off"));
    }
}

static NEXUS_Error NEXUS_Frontend_P_3466_ReadSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length, size_t *pNumRead)
{
#define TOTAL_ADS_SOFTDECISIONS 30

    NEXUS_Error rc = NEXUS_SUCCESS;
    int16_t d_i[TOTAL_ADS_SOFTDECISIONS], d_q[TOTAL_ADS_SOFTDECISIONS];
    int16_t return_length;
    NEXUS_3466Device *pDevice;
    unsigned i;
    NEXUS_3466Channel *pChannel;

    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3466Channel *)handle;
    pDevice = (NEXUS_3466Device *)pChannel->pDevice;

    BKNI_Memset(pDecisions, 0, (sizeof(NEXUS_FrontendSoftDecision) * length));

    if (pChannel->chn_num >= NEXUS_MAX_3466_C_FRONTENDS)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    /* only make one call to ADS. if app needs more, they can loop. */
    rc = BADS_GetSoftDecision(pDevice->cable.ads_chn[pChannel->chn_num], (int16_t)TOTAL_ADS_SOFTDECISIONS, d_i, d_q, &return_length);
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

static void NEXUS_Frontend_P_3466_Close(NEXUS_FrontendHandle handle)
{
    NEXUS_3466Device *pDevice;
    NEXUS_3466Channel *pChannel;

    BDBG_ASSERT(NULL != handle);
    pChannel =(NEXUS_3466Channel *)handle->pDeviceHandle;

    if (pChannel) {
        pDevice = (NEXUS_3466Device *)pChannel->pDevice;
        if(pDevice->cable.isPoweredOn[pChannel->chn_num]) {
            if (handle->untune) {
                handle->untune(pChannel);
            }
        }
    }
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

static NEXUS_Error NEXUS_Frontend_P_3466_GetFastStatus(void *handle, NEXUS_FrontendFastStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3466Device *p3466Device;
    NEXUS_3466Channel *pChannel;
    BADS_LockStatus eLock = BADS_LockStatus_eUnlocked;
    NEXUS_FrontendDeviceHandle hFrontendDevice=NULL;

    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3466Channel *)handle;
    p3466Device = (NEXUS_3466Device *)pChannel->pDevice;

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    pStatus->lockStatus = NEXUS_FrontendLockStatus_eUnknown;

    if(pChannel->chn_num < NEXUS_MAX_3466_C_FRONTENDS){
        rc = BADS_GetLockStatus(p3466Device->cable.ads_chn[pChannel->chn_num],  &eLock);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pStatus->lockStatus = NEXUS_Frontend_P_GetAdsLockStatus(eLock);
        BSTD_UNUSED(hFrontendDevice);
        pStatus->acquireInProgress = p3466Device->cable.acquireInProgress[pChannel->chn_num];
    } else
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
done:
    return rc;
}
