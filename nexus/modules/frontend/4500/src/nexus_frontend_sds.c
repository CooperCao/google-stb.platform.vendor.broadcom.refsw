/******************************************************************************
 *    (c)2008-2012 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
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
 *****************************************************************************/
#include "nexus_frontend_module.h"
#include "nexus_frontend_sds.h"
#include "bi2c.h" /* status error codes */

BDBG_MODULE(nexus_frontend_sds);
BDBG_OBJECT_ID(NEXUS_SdsDevice);

BTRC_MODULE_DECLARE(ChnChange_Tune);
BTRC_MODULE_DECLARE(ChnChange_TuneLock);

/***************************************************************************
Frontend Callback Prototypes
***************************************************************************/
static NEXUS_Error NEXUS_Frontend_P_Sds_TuneSatellite(void *handle, const NEXUS_FrontendSatelliteSettings *pSettings);
static void NEXUS_Frontend_P_Sds_Untune(void *handle);
static NEXUS_Error NEXUS_Frontend_P_Sds_GetSatelliteStatus(void *handle, NEXUS_FrontendSatelliteStatus *pStatus);
static void NEXUS_Frontend_P_Sds_GetDiseqcSettings(void *handle, NEXUS_FrontendDiseqcSettings *pSettings);
static NEXUS_Error NEXUS_Frontend_P_Sds_SetDiseqcSettings(void *handle, const NEXUS_FrontendDiseqcSettings *pSettings);
static NEXUS_Error NEXUS_Frontend_P_Sds_GetDiseqcStatus( void *handle, NEXUS_FrontendDiseqcStatus *pStatus );
static NEXUS_Error NEXUS_Frontend_P_Sds_SendDiseqcMessage(void *handle, const uint8_t *pSendData, size_t sendDataSize, const NEXUS_CallbackDesc *pSendComplete);
static NEXUS_Error NEXUS_Frontend_P_Sds_GetDiseqcReply(void *handle, NEXUS_FrontendDiseqcMessageStatus *pStatus, uint8_t *pReplyBuffer, size_t pReplyBufferSize, size_t *pReplyLength);
static NEXUS_Error NEXUS_Frontend_P_Sds_ResetDiseqc(void *handle, uint8_t options);
static NEXUS_Error NEXUS_Frontend_P_Sds_SendDiseqcAcw(void *handle, uint8_t codeWord);
static void NEXUS_Frontend_P_Sds_ResetStatus(void *handle);
static void NEXUS_Frontend_P_Sds_Close(NEXUS_FrontendHandle handle);
static NEXUS_Error NEXUS_Frontend_P_Sds_GetSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length);
static void NEXUS_Frontend_P_Sds_LockEventHandler(void *pParam);
static NEXUS_Error NEXUS_Frontend_P_Sds_RegisterExtension(NEXUS_FrontendHandle parentHandle, NEXUS_FrontendHandle extensionHandle);

/***************************************************************************
NEXUS -> PI Conversion Routines
***************************************************************************/
typedef struct NEXUS_SdsModeEntry
{
    const NEXUS_FrontendSatelliteCodeRate *pCodeRate;
    BSDS_ModulationType mode;
} NEXUS_SdsModeEntry;

static const NEXUS_FrontendSatelliteCodeRate
    g_cr_scan = {0,0,NEXUS_FrontendSatelliteCodeRateMode_eStandard,0},
    g_cr_1_4 = {1,4,NEXUS_FrontendSatelliteCodeRateMode_eStandard,0},
    g_cr_1_2 = {1,2,NEXUS_FrontendSatelliteCodeRateMode_eStandard,0},
    g_cr_2_3 = {2,3,NEXUS_FrontendSatelliteCodeRateMode_eStandard,0},
    g_cr_3_4 = {3,4,NEXUS_FrontendSatelliteCodeRateMode_eStandard,0},
    g_cr_5_6 = {5,6,NEXUS_FrontendSatelliteCodeRateMode_eStandard,0},
    g_cr_6_7 = {6,7,NEXUS_FrontendSatelliteCodeRateMode_eStandard,0},
    g_cr_7_8 = {7,8,NEXUS_FrontendSatelliteCodeRateMode_eStandard,0},
    g_cr_5_11 = {5,11,NEXUS_FrontendSatelliteCodeRateMode_eStandard,0},
    g_cr_3_5 = {3,5,NEXUS_FrontendSatelliteCodeRateMode_eStandard,0},
    g_cr_4_5 = {4,5,NEXUS_FrontendSatelliteCodeRateMode_eStandard,0},
    g_cr_9_10 = {9,10,NEXUS_FrontendSatelliteCodeRateMode_eStandard,0},
    g_cr_8_9 = {8,9,NEXUS_FrontendSatelliteCodeRateMode_eStandard,0},
    g_cr_3_4_2050 = {3,4,NEXUS_FrontendSatelliteCodeRateMode_eStandard,2050},
    g_cr_3_4_2100 = {3,4,NEXUS_FrontendSatelliteCodeRateMode_eStandard,2100},
    g_cr_3_4_2200 = {3,4,NEXUS_FrontendSatelliteCodeRateMode_eStandard,2200};

static const struct NEXUS_SdsModeEntry
g_sds_dvb_modes[] = {
    {&g_cr_1_2, BSDS_ModulationType_eDvb1_2},
    {&g_cr_2_3, BSDS_ModulationType_eDvb2_3},
    {&g_cr_3_4, BSDS_ModulationType_eDvb3_4},
    {&g_cr_5_6, BSDS_ModulationType_eDvb5_6},
    {&g_cr_7_8, BSDS_ModulationType_eDvb7_8},
    {NULL, BSDS_ModulationType_eDvbScan}
},
g_sds_dss_modes[] = {
    {&g_cr_1_2, BSDS_ModulationType_eDss1_2},
    {&g_cr_2_3, BSDS_ModulationType_eDss2_3},
    {&g_cr_6_7, BSDS_ModulationType_eDss6_7},
    {NULL, BSDS_ModulationType_eDssScan}
},
g_sds_dcii_modes[] = {
    {&g_cr_1_2, BSDS_ModulationType_eDCII1_2},
    {&g_cr_2_3, BSDS_ModulationType_eDCII2_3},
    {&g_cr_3_5, BSDS_ModulationType_eDCII3_5},
    {&g_cr_3_4, BSDS_ModulationType_eDCII3_4},
    {&g_cr_4_5, BSDS_ModulationType_eDCII4_5},
    {&g_cr_5_6, BSDS_ModulationType_eDCII5_6},
    {&g_cr_7_8, BSDS_ModulationType_eDCII7_8},
    {&g_cr_5_11, BSDS_ModulationType_eDCII5_11},
    {NULL, BSDS_ModulationType_eDCIIScan}
},
g_sds_qpsk_turbo_modes[] = {
    {&g_cr_1_2, BSDS_ModulationType_eTurboQpsk1_2},
    {&g_cr_2_3, BSDS_ModulationType_eTurboQpsk2_3},
    {&g_cr_3_4, BSDS_ModulationType_eTurboQpsk3_4},
    {&g_cr_5_6, BSDS_ModulationType_eTurboQpsk5_6},
    {&g_cr_7_8, BSDS_ModulationType_eTurboQpsk7_8},
    {NULL, BSDS_ModulationType_eTurboScan}
},
g_sds_8psk_turbo_modes[] = {
    {&g_cr_2_3, BSDS_ModulationType_eTurbo8psk2_3},
    {&g_cr_5_6, BSDS_ModulationType_eTurbo8psk5_6},
    {&g_cr_8_9, BSDS_ModulationType_eTurbo8psk8_9},
    {&g_cr_3_4_2050,BSDS_ModulationType_eTurbo8psk3_4_2_05},
    {&g_cr_3_4_2100,BSDS_ModulationType_eTurbo8psk3_4_2_10},
    {&g_cr_3_4_2200,BSDS_ModulationType_eTurbo8psk3_4_2_20},
    {NULL, BSDS_ModulationType_eTurboScan}
},
g_sds_turbo_modes[] = {
    {&g_cr_1_2, BSDS_ModulationType_eTurboQpsk1_2},
    {&g_cr_2_3, BSDS_ModulationType_eTurboQpsk2_3},
    {&g_cr_3_4, BSDS_ModulationType_eTurboQpsk3_4},
    {&g_cr_5_6, BSDS_ModulationType_eTurboQpsk5_6},
    {&g_cr_7_8, BSDS_ModulationType_eTurboQpsk7_8},
    {&g_cr_2_3, BSDS_ModulationType_eTurbo8psk2_3},
    {&g_cr_5_6, BSDS_ModulationType_eTurbo8psk5_6},
    {&g_cr_8_9, BSDS_ModulationType_eTurbo8psk8_9},
    {&g_cr_3_4_2050,BSDS_ModulationType_eTurbo8psk3_4_2_05},
    {&g_cr_3_4_2100,BSDS_ModulationType_eTurbo8psk3_4_2_10},
    {&g_cr_3_4_2200,BSDS_ModulationType_eTurbo8psk3_4_2_20},
    {NULL, BSDS_ModulationType_eTurboScan}
};

static BSDS_ModulationType NEXUS_Frontend_P_Sds_GetMode(const struct NEXUS_SdsModeEntry *pModes, const NEXUS_FrontendSatelliteCodeRate *pCodeRate)
{
    /* NOTE: there are not non-zero bitsPerSymbol values in the look up tables. if someone specifies a non-zero bitsPerSymbol,
    this algorithm will result in a scan. */
    for ( ;; pModes++ )
    {
        if ( NULL == pModes->pCodeRate )
        {
            BDBG_MSG(("Use scan for coderate %d:%d:%d", pCodeRate->numerator, pCodeRate->denominator, pCodeRate->bitsPerSymbol));
            return pModes->mode;
        }
        if ( pModes->pCodeRate->numerator == pCodeRate->numerator &&
             pModes->pCodeRate->denominator == pCodeRate->denominator &&
             pModes->pCodeRate->bitsPerSymbol == pCodeRate->bitsPerSymbol )
        {
            return pModes->mode;
        }
    }
}

void NEXUS_Frontend_P_Sds_GetDefaultSettings( NEXUS_FrontendSdsSettings *pSettings )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_FrontendHandle NEXUS_Frontend_P_Sds_Create( const NEXUS_FrontendSdsSettings *pSettings )
{
    BERR_Code errCode;
    NEXUS_SdsDevice *pDevice;

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pSettings->sdsHandle);

    pDevice = BKNI_Malloc(sizeof(NEXUS_SdsDevice));
    if ( NULL == pDevice )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    BKNI_Memset(pDevice, 0, sizeof(*pDevice));

    pDevice->settings = *pSettings;     /* Save Settings */
    pDevice->sdsHandle = pSettings->sdsHandle;


	errCode = BSDS_GetLockStateChangeEventHandle(pDevice->sdsHandle, &pDevice->lockEvent);
	if ( errCode )
	{
		errCode = BERR_TRACE(errCode);
		goto err_lock_event;
	}

	errCode = BSDS_SetDiseqc22KhzMode(pDevice->sdsHandle, BSDS_Diseqc22KhzMode_eEnvelope);
	if ( errCode )
	{
		errCode = BERR_TRACE(errCode);
		goto err_set_diseqc_mode;
	}
	pDevice->diseqcSettings.toneMode = NEXUS_FrontendDiseqcToneMode_eEnvelope;

    pDevice->lockEventCallback = NEXUS_RegisterEvent(pDevice->lockEvent,
                                                     NEXUS_Frontend_P_Sds_LockEventHandler,
                                                     pDevice);
    if ( NULL == pDevice->lockEventCallback )
    {
        errCode = BERR_TRACE(BERR_OS_ERROR);
        goto err_lock_event_callback;
    }

    /* Create Frontend Handle */
    pDevice->frontendHandle = NEXUS_Frontend_P_Create(pDevice);
    if ( NULL == pDevice->frontendHandle )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_frontend_handle;
    }

    /* Set Capabilities */
    pDevice->frontendHandle->capabilities.satellite = true;
    pDevice->frontendHandle->capabilities.satelliteModes[NEXUS_FrontendSatelliteMode_eDvb] = true;
    pDevice->frontendHandle->capabilities.satelliteModes[NEXUS_FrontendSatelliteMode_eDss] = true;
    pDevice->frontendHandle->capabilities.satelliteModes[NEXUS_FrontendSatelliteMode_eDcii] = true;
    pDevice->frontendHandle->capabilities.satelliteModes[NEXUS_FrontendSatelliteMode_eQpskLdpc] = false;
    pDevice->frontendHandle->capabilities.satelliteModes[NEXUS_FrontendSatelliteMode_e8pskLdpc] = false;
    pDevice->frontendHandle->capabilities.satelliteModes[NEXUS_FrontendSatelliteMode_eQpskTurbo] = true;
    pDevice->frontendHandle->capabilities.satelliteModes[NEXUS_FrontendSatelliteMode_e8pskTurbo] = true;
    pDevice->frontendHandle->capabilities.satelliteModes[NEXUS_FrontendSatelliteMode_eLdpc] = false;
    pDevice->frontendHandle->capabilities.satelliteModes[NEXUS_FrontendSatelliteMode_eTurbo] = true;

    /* Install Hooks */
    pDevice->frontendHandle->close = NEXUS_Frontend_P_Sds_Close;
    pDevice->frontendHandle->untune = NEXUS_Frontend_P_Sds_Untune;
    pDevice->frontendHandle->getDiseqcReply = NEXUS_Frontend_P_Sds_GetDiseqcReply;
    pDevice->frontendHandle->getDiseqcSettings = NEXUS_Frontend_P_Sds_GetDiseqcSettings;
    pDevice->frontendHandle->sendDiseqcAcw = NEXUS_Frontend_P_Sds_SendDiseqcAcw;
    pDevice->frontendHandle->sendDiseqcMessage = NEXUS_Frontend_P_Sds_SendDiseqcMessage;
    pDevice->frontendHandle->setDiseqcSettings = NEXUS_Frontend_P_Sds_SetDiseqcSettings;
    pDevice->frontendHandle->getDiseqcStatus = NEXUS_Frontend_P_Sds_GetDiseqcStatus;
    pDevice->frontendHandle->resetDiseqc = NEXUS_Frontend_P_Sds_ResetDiseqc;
    pDevice->frontendHandle->getSatelliteStatus = NEXUS_Frontend_P_Sds_GetSatelliteStatus;
    pDevice->frontendHandle->getSoftDecisions = NEXUS_Frontend_P_Sds_GetSoftDecisions;
    pDevice->frontendHandle->tuneSatellite = NEXUS_Frontend_P_Sds_TuneSatellite;
    pDevice->frontendHandle->resetStatus = NEXUS_Frontend_P_Sds_ResetStatus;
    pDevice->frontendHandle->registerExtension = NEXUS_Frontend_P_Sds_RegisterExtension;

    pDevice->diseqcAppCallback = NEXUS_TaskCallback_Create(pDevice->frontendHandle, NULL);
    if ( NULL == pDevice->diseqcAppCallback )
    {
	    errCode = BERR_TRACE(BERR_OS_ERROR);
	    goto err_diseqc_app_callback;
    }

    pDevice->lockAppCallback = NEXUS_TaskCallback_Create(pDevice->frontendHandle, NULL);
    if ( NULL == pDevice->lockAppCallback )
    {
        errCode = BERR_TRACE(BERR_OS_ERROR);
        goto err_lock_app_callback;
    }

    /* Success */
    BDBG_OBJECT_SET(pDevice, NEXUS_SdsDevice);
    return pDevice->frontendHandle;

err_lock_app_callback:
    NEXUS_TaskCallback_Destroy(pDevice->diseqcAppCallback);
err_diseqc_app_callback:
    NEXUS_Frontend_P_Destroy(pDevice->frontendHandle);
err_frontend_handle:
    NEXUS_UnregisterEvent(pDevice->lockEventCallback);
err_set_diseqc_mode:
err_lock_event_callback:
err_lock_event:
    BKNI_Free(pDevice);

    return NULL;
}

static void NEXUS_Frontend_P_Sds_Close(NEXUS_FrontendHandle handle)
{
    NEXUS_SdsDevice *pDevice;

    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    pDevice = handle->pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_SdsDevice);

    NEXUS_UnregisterEvent(pDevice->lockEventCallback);
    NEXUS_TaskCallback_Destroy(pDevice->lockAppCallback);
	NEXUS_TaskCallback_Destroy(pDevice->diseqcAppCallback);

    NEXUS_Frontend_P_Destroy(pDevice->frontendHandle);

    /* Call post-close callback */
    if ( pDevice->settings.closeFunction )
    {
        pDevice->settings.closeFunction(handle, pDevice->settings.pCloseParam);
    }

    BKNI_Memset(pDevice, 0, sizeof(*pDevice));
    BKNI_Free(pDevice);
}

NEXUS_Error NEXUS_Frontend_P_Sds_SetDevices( NEXUS_FrontendHandle handle, const NEXUS_FrontendSdsDevices *pDevices )
{
    NEXUS_SdsDevice *pDevice;

    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    BDBG_ASSERT(NULL != pDevices);

    pDevice = handle->pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_SdsDevice);

    pDevice->settings.devices = *pDevices;

    return BERR_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_Sds_TuneSatellite( void *handle, const NEXUS_FrontendSatelliteSettings *pSettings )
{
    BERR_Code errCode;
    NEXUS_SdsDevice *pDevice = handle;
	BSDS_AcqParams acqParams;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_SdsDevice);
    BDBG_ASSERT(NULL != pSettings);

    BKNI_Memset(&acqParams, 0, sizeof(acqParams));

    /* BAST_ACQSETTINGS_DEFAULT was applied in NEXUS_Frontend_GetDefaultSatelliteSettings */

	acqParams.acqControl = (BSDS_CLKINV | BSDS_TEI);

    if ( !pSettings->pnData )
        acqParams.acqControl |= BSDS_MPEG;
    if ( pSettings->bertEnable )
        acqParams.acqControl |= BSDS_ENABLE_BERT;

    switch ( pSettings->mode )
    {
    case NEXUS_FrontendSatelliteMode_eDvb:
        BDBG_MSG(("Tune DVB"));
        acqParams.mode = NEXUS_Frontend_P_Sds_GetMode(g_sds_dvb_modes, &pSettings->codeRate);
        break;
    case NEXUS_FrontendSatelliteMode_eDss:
        BDBG_MSG(("Tune DSS"));
        acqParams.mode = NEXUS_Frontend_P_Sds_GetMode(g_sds_dss_modes, &pSettings->codeRate);
        break;
    case NEXUS_FrontendSatelliteMode_eDcii:
        BDBG_MSG(("Tune DCII"));
        acqParams.mode = NEXUS_Frontend_P_Sds_GetMode(g_sds_dcii_modes, &pSettings->codeRate);
        break;
    case NEXUS_FrontendSatelliteMode_eQpskTurbo:
        BDBG_MSG(("Tune Turbo QPSK"));
        acqParams.mode = NEXUS_Frontend_P_Sds_GetMode(g_sds_qpsk_turbo_modes, &pSettings->codeRate);
        break;
    case NEXUS_FrontendSatelliteMode_e8pskTurbo:
        BDBG_MSG(("Tune Turbo 8PSK"));
        acqParams.mode = NEXUS_Frontend_P_Sds_GetMode(g_sds_8psk_turbo_modes, &pSettings->codeRate);
        break;
    case NEXUS_FrontendSatelliteMode_eTurbo:
        BDBG_MSG(("Tune Turbo"));
        acqParams.mode = NEXUS_Frontend_P_Sds_GetMode(g_sds_turbo_modes, &pSettings->codeRate);
        break;
    default:
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

	if (pDevice->settings.devSettings.bitWideSync)
	{
		acqParams.acqControl |= BSDS_SYNC1;
	}
    BDBG_MSG(("acqParams.acqControl %x", acqParams.acqControl));
    pDevice->lastSettings = *pSettings; /* Save */
    acqParams.symbolRate = pSettings->symbolRate;
    BDBG_MSG(("Freq %u, Symbol Rate %u", pSettings->frequency, acqParams.symbolRate));
    acqParams.carrierOffset = pSettings->carrierOffset;

    NEXUS_TaskCallback_Set(pDevice->lockAppCallback, &pSettings->lockCallback);

	errCode = BSDS_Tune(pDevice->sdsHandle, pSettings->frequency);
	if ( errCode )
	{
		return BERR_TRACE(errCode);
	}

	BKNI_Sleep(10); /* wait for tune */
    errCode = BSDS_Acquire(pDevice->sdsHandle, &acqParams);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    BTRC_TRACE(ChnChange_Tune, STOP);
    BTRC_TRACE(ChnChange_TuneLock, START);

    return BERR_SUCCESS;
}

static void NEXUS_Frontend_P_Sds_Untune(void *handle)
{
    BDBG_MSG(("untune"));
    BSTD_UNUSED(handle);
}

static NEXUS_Error NEXUS_Frontend_P_Sds_GetSatelliteStatus( void *handle, NEXUS_FrontendSatelliteStatus *pStatus )
{
    NEXUS_Error errCode;
    NEXUS_SdsDevice *pDevice = handle;
    NEXUS_Time now;
	uint8_t acqId;	/* (not used) acquisition script version ID */
	BSDS_ModulationType mode;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_SdsDevice);
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

	pStatus->version.chipId = 4500;
	pStatus->version.bondingOption = 0;

    errCode = BSDS_GetApVersion(pDevice->sdsHandle,
        &pStatus->version.chipVersion,
        &pStatus->version.apMicrocodeVersion,
		&acqId,
        &pStatus->version.hostConfigurationVersion);
    if (errCode) {
        if (errCode == BI2C_ERR_NO_ACK) {
            pStatus->statusError = NEXUS_FrontendSatelliteStatusError_eI2cNoAck;
            return 0;
        }
        else if (errCode == BERR_TIMEOUT) {
            pStatus->statusError = NEXUS_FrontendSatelliteStatusError_eHabTimeout;
            return 0;
        }
        else {
            /* unknown error. status function should fail. */
            return BERR_TRACE(errCode);
        }
    }

	if (pDevice->lastSettings.mode < NEXUS_FrontendSatelliteMode_eQpskTurbo)
	{	/* legacy QPSK modes */
		BSDS_QpskStatus sdsQpskStatus;
		errCode = BSDS_GetQpskStatus(pDevice->sdsHandle, &sdsQpskStatus);
		
		if ( errCode ) {
			/* if not locked, BSDS_GetChannelStatus will fail. this is not an error. the status structure should just report not locked and no status available. */
			pStatus->statusError = NEXUS_FrontendSatelliteStatusError_eGetChannelStatusError;
			return 0;
		}
		
		NEXUS_Time_Get(&now);
		pStatus->timeElapsed = NEXUS_Time_Diff(&now, &pDevice->frontendHandle->resetStatusTime);
		
		pStatus->settings = pDevice->lastSettings; /* return last settings used in NEXUS_Frontend_TuneSatellite */
		pStatus->channel = 0;
		pStatus->sampleClock = 0; /* zeroing stats not provided by SDS */
		pStatus->carrierError =  sdsQpskStatus.carrierError;
		pStatus->outputBitRate = sdsQpskStatus.symbolRate * 2;
		pStatus->symbolRate = sdsQpskStatus.symbolRate;
		pStatus->symbolRateError = sdsQpskStatus.symbolRateError;
		pStatus->snrEstimate = sdsQpskStatus.snrEstimate * 100/256;  /* convert to 1/100 units */
		pStatus->tunerLocked = true; /* for lack of better info */
		pStatus->demodLocked = sdsQpskStatus.bRsLocked && sdsQpskStatus.bVitLocked;
		pStatus->bertLocked = sdsQpskStatus.bBertLocked;
		pStatus->mpegErrors = 0;
		pStatus->mpegCount = 0;
		pStatus->berErrorCount = 0;
		pStatus->ifAgc = 0;
		pStatus->rfAgc =  0;
		pStatus->agf =  0;
		pStatus->frequency = sdsQpskStatus.tunerFreq;
		
		pStatus->fecCorrected = sdsQpskStatus.rsCorrCount;
		pStatus->fecUncorrected = sdsQpskStatus.rsUncorrCount;
		pStatus->preViterbiErrorCount = 0;
		pStatus->spectralInversion = NEXUS_FrontendSatelliteInversion_eScan;
		pStatus->fecPhase = 0;
		mode = sdsQpskStatus.mode;
	}
	else
	{ 	/* Turbo modes */
		BSDS_TurboStatus sdsTurboStatus;

		errCode = BSDS_GetTurboStatus(pDevice->sdsHandle, &sdsTurboStatus);

		if ( errCode ) {
			/* if not locked, BSDS_GetChannelStatus will fail. this is not an error. the status structure should just report not locked and no status available. */
			pStatus->statusError = NEXUS_FrontendSatelliteStatusError_eGetChannelStatusError;
			return 0;
		}

		NEXUS_Time_Get(&now);
		pStatus->timeElapsed = NEXUS_Time_Diff(&now, &pDevice->frontendHandle->resetStatusTime);

		pStatus->settings = pDevice->lastSettings; /* return last settings used in NEXUS_Frontend_TuneSatellite */
		pStatus->channel = 0;
		pStatus->sampleClock = 0; /* zeroing stats not provided by SDS */
		pStatus->carrierError =  sdsTurboStatus.carrierError;
		pStatus->outputBitRate = sdsTurboStatus.symbolRate * sdsTurboStatus.bitsPerSym;
		pStatus->symbolRate = sdsTurboStatus.symbolRate;
		pStatus->symbolRateError = sdsTurboStatus.symbolRateError;
		pStatus->snrEstimate = sdsTurboStatus.snrEstimate * 100/256;  /* convert to 1/100 units */
		pStatus->tunerLocked = sdsTurboStatus.bRcvrLocked;
		pStatus->demodLocked = sdsTurboStatus.bFecLocked;
		pStatus->bertLocked = sdsTurboStatus.bBertLocked;
		pStatus->mpegErrors = 0;
		pStatus->mpegCount = 0;
		pStatus->berErrorCount = 0;
		pStatus->ifAgc = 0;
		pStatus->rfAgc =  0;
		pStatus->agf =  0;
		pStatus->frequency = sdsTurboStatus.tunerFreq;

		pStatus->fecCorrected = sdsTurboStatus.corrBlockCount;
		pStatus->fecUncorrected = sdsTurboStatus.badBlockCount;
		pStatus->preViterbiErrorCount = 0;
		pStatus->spectralInversion = NEXUS_FrontendSatelliteInversion_eScan;
		pStatus->fecPhase = 0;
		mode = sdsTurboStatus.mode;
	}
	#if 0 /*MCW todo how? */
	{
	BSDS_LnaStatus lnaStatus;
	BSDS_GetLnaStatus(pDevice->settings.sdsHandle, &lnaStatus);
	pStatus->lnaStatus.intConfig = lnaStatus.int_config;
	pStatus->lnaStatus.extConfig = lnaStatus.ext_config;
	pStatus->lnaStatus.version = lnaStatus.version;
	pStatus->lnaStatus.agc0 = lnaStatus.agc0;
	pStatus->lnaStatus.agc1 = lnaStatus.agc1;
	}
	#endif

    /* Check mode to get coderate */
    switch ( mode )
    {
    case BSDS_ModulationType_eDvbScan:
    case BSDS_ModulationType_eDssScan:
    case BSDS_ModulationType_eDCIIScan:
    case BSDS_ModulationType_eTurboScan:
        pStatus->codeRate = g_cr_scan;
        break;

    case BSDS_ModulationType_eDvb1_2:
    case BSDS_ModulationType_eDss1_2:
    case BSDS_ModulationType_eDCII1_2:
    case BSDS_ModulationType_eTurboQpsk1_2:
        pStatus->codeRate = g_cr_1_2;
        break;

    case BSDS_ModulationType_eDvb2_3:
    case BSDS_ModulationType_eDss2_3:
    case BSDS_ModulationType_eDCII2_3:
    case BSDS_ModulationType_eTurboQpsk2_3:
    case BSDS_ModulationType_eTurbo8psk2_3:
        pStatus->codeRate = g_cr_2_3;
        break;

    case BSDS_ModulationType_eDvb3_4:
    case BSDS_ModulationType_eDCII3_4:
    case BSDS_ModulationType_eTurboQpsk3_4:
        pStatus->codeRate = g_cr_3_4;
        break;

	case BSDS_ModulationType_eTurbo8psk3_4_2_05:
        pStatus->codeRate = g_cr_3_4_2050;
        break;

	case BSDS_ModulationType_eTurbo8psk3_4_2_10:
		pStatus->codeRate = g_cr_3_4_2100;
		break;

	case BSDS_ModulationType_eTurbo8psk3_4_2_20:
		pStatus->codeRate = g_cr_3_4_2200;
		break;

    case BSDS_ModulationType_eDCII3_5:
        pStatus->codeRate = g_cr_3_5;
        break;

    case BSDS_ModulationType_eDCII4_5:
        pStatus->codeRate = g_cr_4_5;
        break;

    case BSDS_ModulationType_eDvb5_6:
    case BSDS_ModulationType_eDCII5_6:
    case BSDS_ModulationType_eTurboQpsk5_6:
    case BSDS_ModulationType_eTurbo8psk5_6:
        pStatus->codeRate = g_cr_5_6;
        break;

    case BSDS_ModulationType_eDss6_7:
        pStatus->codeRate = g_cr_6_7;
        break;

    case BSDS_ModulationType_eDvb7_8:
    case BSDS_ModulationType_eDCII7_8:
    case BSDS_ModulationType_eTurboQpsk7_8:
        pStatus->codeRate = g_cr_7_8;
        break;

    case BSDS_ModulationType_eDCII5_11:
        pStatus->codeRate = g_cr_5_11;
        break;

    default:
        break;
    }

    switch ( mode )
    {
    default:
    case BSDS_ModulationType_eDvbScan:
    case BSDS_ModulationType_eDvb1_2:
    case BSDS_ModulationType_eDvb2_3:
    case BSDS_ModulationType_eDvb3_4:
    case BSDS_ModulationType_eDvb5_6:
    case BSDS_ModulationType_eDvb7_8:
        pStatus->mode = NEXUS_FrontendSatelliteMode_eDvb;
        break;

    case BSDS_ModulationType_eDssScan:
    case BSDS_ModulationType_eDss1_2:
    case BSDS_ModulationType_eDss2_3:
    case BSDS_ModulationType_eDss6_7:
        pStatus->mode = NEXUS_FrontendSatelliteMode_eDss;
        break;

    case BSDS_ModulationType_eDCIIScan:
    case BSDS_ModulationType_eDCII1_2:
    case BSDS_ModulationType_eDCII2_3:
    case BSDS_ModulationType_eDCII3_4:
    case BSDS_ModulationType_eDCII3_5:
    case BSDS_ModulationType_eDCII4_5:
    case BSDS_ModulationType_eDCII5_6:
    case BSDS_ModulationType_eDCII7_8:
    case BSDS_ModulationType_eDCII5_11:
        pStatus->mode = NEXUS_FrontendSatelliteMode_eDcii;
        break;

    case BSDS_ModulationType_eTurboScan:
    case BSDS_ModulationType_eTurboQpsk1_2:
    case BSDS_ModulationType_eTurboQpsk2_3:
    case BSDS_ModulationType_eTurboQpsk3_4:
    case BSDS_ModulationType_eTurboQpsk5_6:
    case BSDS_ModulationType_eTurboQpsk7_8:
        pStatus->mode = NEXUS_FrontendSatelliteMode_eTurboQpsk;
        break;

    case BSDS_ModulationType_eTurbo8psk2_3:
    case BSDS_ModulationType_eTurbo8psk5_6:
	case BSDS_ModulationType_eTurbo8psk8_9:
	case BSDS_ModulationType_eTurbo8psk3_4_2_05:
	case BSDS_ModulationType_eTurbo8psk3_4_2_10:
	case BSDS_ModulationType_eTurbo8psk3_4_2_20:
        pStatus->mode = NEXUS_FrontendSatelliteMode_eTurbo8psk;
        break;
    }

    return BERR_SUCCESS;
}

/* ISL6421 control bits.  On 7400, actually an ISL6425. */
#define ISL6421_SR    0x80
#define ISL6421_DCL   0x40
#define ISL6421_ISEL1 0x20
#define ISL6421_ENT1  0x10
#define ISL6421_LLC1  0x08
#define ISL6421_VSEL1 0x04
#define ISL6421_EN1   0x02
#define ISL6421_OLF1  0x01

static void NEXUS_Frontend_P_Sds_GetDiseqcSettings( void *handle, NEXUS_FrontendDiseqcSettings *pSettings )
{
    NEXUS_SdsDevice *pDevice = handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_SdsDevice);

	BDBG_ASSERT(NULL != pSettings);
	*pSettings = pDevice->diseqcSettings;
}

static NEXUS_Error NEXUS_Frontend_P_Sds_SetDiseqcSettings( void *handle, const NEXUS_FrontendDiseqcSettings *pSettings )
{
    BERR_Code rc;
    NEXUS_Error errCode = NEXUS_SUCCESS;
    NEXUS_SdsDevice *pDevice = handle;
	BSDS_DiseqcVoltage voltage;
	BDBG_OBJECT_ASSERT(pDevice, NEXUS_SdsDevice);
	BDBG_ASSERT(NULL != pSettings);

	switch(pSettings->voltage)
	{
	case NEXUS_FrontendDiseqcVoltage_e13v:
		voltage = BSDS_DiseqcVoltage_eBot;
		break;
	case NEXUS_FrontendDiseqcVoltage_e18v:
		voltage = BSDS_DiseqcVoltage_eTop;
		break;
	default:
		return BERR_TRACE(NEXUS_INVALID_PARAMETER);
	}

	if (pDevice->settings.devSettings.diseqcChip == 6425)
	{
		uint8_t controlValue[3];
		controlValue[0] = ISL6421_ISEL1;
		controlValue[1] = ISL6421_DCL | ISL6421_LLC1 | ISL6421_EN1 | ISL6421_OLF1;
		controlValue[2] = (pSettings->voltage == NEXUS_FrontendDiseqcVoltage_e13v) ?
						  ISL6421_DCL | ISL6421_ISEL1 | ISL6421_ENT1 :
						  ISL6421_DCL | ISL6421_ISEL1 | ISL6421_ENT1 | ISL6421_EN1;
		errCode = NEXUS_I2c_WriteNoAddr(pDevice->settings.devSettings.i2cDevice,
										pDevice->settings.devSettings.diseqcI2cAddr,
										&controlValue[0], 3);
	}
	else if (pDevice->settings.devSettings.diseqcChip == 6421)
	{
		uint8_t controlValue = (pSettings->voltage == NEXUS_FrontendDiseqcVoltage_e13v) ?
								ISL6421_ISEL1 | ISL6421_EN1 :
								ISL6421_ISEL1 | ISL6421_LLC1 | ISL6421_VSEL1 | ISL6421_EN1;
		errCode = NEXUS_I2c_WriteNoAddr(pDevice->settings.devSettings.i2cDevice,
										pDevice->settings.devSettings.diseqcI2cAddr,
										&controlValue, 1);
	}
	if (errCode!=NEXUS_SUCCESS) {
		BDBG_ERR(("ISL%d DiSEqC chip (I2C 0x%02x) is likely to be either faulty or missing.",
				  pDevice->settings.devSettings.diseqcChip,
				  pDevice->settings.devSettings.diseqcI2cAddr));
		return BERR_TRACE(errCode);
	}

	rc = BSDS_SetDiseqc22KhzMode(pDevice->sdsHandle, (pSettings->toneMode == NEXUS_FrontendDiseqcToneMode_eEnvelope) ? 
								 BSDS_Diseqc22KhzMode_eEnvelope : BSDS_Diseqc22KhzMode_eTone);
	if (rc) return BERR_TRACE(rc);

	rc = BSDS_ConfigDiseqc(pDevice->sdsHandle, pSettings->toneEnabled ?
						   BSDS_DiseqcTone_ePresent:BSDS_DiseqcTone_eAbsent, voltage);
	if (rc) return BERR_TRACE(rc);

	pDevice->diseqcSettings = *pSettings;
	return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_Sds_GetDiseqcStatus( void *handle, NEXUS_FrontendDiseqcStatus *pStatus )
{
    NEXUS_SdsDevice *pDevice = handle;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_SdsDevice);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

	pStatus->toneEnabled = pDevice->diseqcSettings.toneEnabled;
	pStatus->voltage = pDevice->diseqcSettings.voltage;
    return 0;
}

static NEXUS_Error NEXUS_Frontend_P_Sds_SendDiseqcMessage( void *handle, const uint8_t *pSendData,
    size_t sendDataSize, const NEXUS_CallbackDesc *pSendComplete )
{
    NEXUS_SdsDevice *pDevice = handle;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_SdsDevice);
    BDBG_ASSERT(NULL != pSendData);
    BDBG_ASSERT(sendDataSize > 0);

	NEXUS_TaskCallback_Set(pDevice->diseqcAppCallback, pSendComplete);
	errCode = BSDS_SendDiseqcCommand(pDevice->sdsHandle, 0, /* Diseqc send options 0 for lack of better info */
									 pSendData, sendDataSize,
									 pDevice->diseqcReplybuffer, &pDevice->diseqcReplylen);
	if ( errCode )
	{
		NEXUS_TaskCallback_Set(pDevice->diseqcAppCallback, NULL);
		switch (errCode)
		{
		case BSDS_ERR_DISEQC_BUSY: 
			pDevice->diseqcReplyStatus = NEXUS_FrontendDiseqcMessageStatus_eBusy;
			break;
		case BSDS_ERR_DISEQC_OVERFLOW:
			pDevice->diseqcReplyStatus = NEXUS_FrontendDiseqcMessageStatus_eRxOverflow;
			break;
		case BSDS_ERR_DISEQC_RRTO:
			pDevice->diseqcReplyStatus = NEXUS_FrontendDiseqcMessageStatus_eRxReplyTimeout;
			break;
		case BSDS_ERR_DISEQC_DEMOD:
			pDevice->diseqcReplyStatus = NEXUS_FrontendDiseqcMessageStatus_eAcwTimeout; /* for lack of better code */
			break;
		case BSDS_ERR_DISEQC_PARITY:
			pDevice->diseqcReplyStatus = NEXUS_FrontendDiseqcMessageStatus_eRxParityError;
			break;
		}
		return BERR_TRACE(errCode);
	}
	else
		pDevice->diseqcReplyStatus = NEXUS_FrontendDiseqcMessageStatus_eSuccess;

    NEXUS_TaskCallback_Fire(pDevice->diseqcAppCallback); /* BSDS_SendDiseqcCommand() is synchronous, it's done. */

	return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_Sds_ResetDiseqc( void *handle, const uint8_t options )
{
    NEXUS_SdsDevice *pDevice = handle;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_SdsDevice);
	BSTD_UNUSED(options);

    errCode = BSDS_SetGpioMode(pDevice->sdsHandle, false, false, false);
	if ( errCode )
	{
		return BERR_TRACE(errCode);
	}
    errCode = BSDS_SetDiseqc22KhzMode(pDevice->sdsHandle, BSDS_Diseqc22KhzMode_eEnvelope);
	if ( errCode )
	{
		return BERR_TRACE(errCode);
	}
	return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_Sds_GetDiseqcReply( void *handle, NEXUS_FrontendDiseqcMessageStatus *pStatus, uint8_t *pReplyBuffer,
    size_t replyBufferSize, size_t *pReplyLength )
{
    NEXUS_SdsDevice *pDevice = handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_SdsDevice);

    BDBG_ASSERT(NULL != pStatus);
    BDBG_ASSERT(NULL != pReplyBuffer);
    BDBG_ASSERT(replyBufferSize > 0);
    BDBG_ASSERT(NULL != pReplyLength);


	NEXUS_TaskCallback_Set(pDevice->diseqcAppCallback, NULL);   /* just to be safe */

	*pStatus = pDevice->diseqcReplyStatus;

	if (pDevice->diseqcReplyStatus) {
		return BERR_TRACE(pDevice->diseqcReplyStatus);
	}

	*pReplyLength = replyBufferSize<pDevice->diseqcReplylen?replyBufferSize:pDevice->diseqcReplylen;
	if (*pReplyLength) {
		BKNI_Memcpy(pReplyBuffer, pDevice->diseqcReplybuffer, *pReplyLength);
	}

    return BERR_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_Sds_SendDiseqcAcw( void *handle, uint8_t codeWord )
{
    NEXUS_Error errCode;
    NEXUS_SdsDevice *pDevice = handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_SdsDevice);

	errCode = BSDS_SendDiseqcAcw(pDevice->sdsHandle, codeWord);
	if ( errCode )
	{
		return BERR_TRACE(errCode);
	}
    return BERR_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_Sds_GetSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length)
{
#define TOTAL_SDS_SOFTDECISIONS 25  /* What an odd number... */
    int j;
    size_t i;
    NEXUS_Error errCode;
    NEXUS_SdsDevice *pDevice = handle;
    int8_t d_i[TOTAL_SDS_SOFTDECISIONS], d_q[TOTAL_SDS_SOFTDECISIONS];

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_SdsDevice);

    for( i=0; i<length; i += TOTAL_SDS_SOFTDECISIONS )
    {
        errCode = BSDS_GetSoftDecisionBuf(pDevice->sdsHandle, d_i, d_q);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        for ( j=0; j<TOTAL_SDS_SOFTDECISIONS && i+j<length; j++ )
        {
            pDecisions[i+j].i = d_i[j] * 256 * 2;	/*MCW check */
            pDecisions[i+j].q = d_q[j] * 256 * 2;
        }
    }

    return BERR_SUCCESS;
}

static void NEXUS_Frontend_P_Sds_ResetStatus( void *handle )
{
    NEXUS_SdsDevice *pDevice = handle;
    BSDS_ResetDecoderStats(pDevice->sdsHandle);
}

static void NEXUS_Frontend_P_Sds_LockEventHandler(void *pParam)
{
    NEXUS_SdsDevice *pDevice = pParam;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_SdsDevice);
    BDBG_MSG(("SDS Lock Event"));

    BTRC_TRACE(ChnChange_TuneLock, STOP);
    NEXUS_TaskCallback_Fire(pDevice->lockAppCallback);
}

static NEXUS_Error NEXUS_Frontend_P_Sds_RegisterExtension(NEXUS_FrontendHandle parentHandle, NEXUS_FrontendHandle extensionHandle)
{
    NEXUS_SdsDevice *pDevice;

    BDBG_OBJECT_ASSERT(parentHandle, NEXUS_Frontend);
    pDevice = parentHandle->pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_SdsDevice);

    if (extensionHandle == NULL) {
        extensionHandle = parentHandle;
    }
    else {
        BDBG_OBJECT_ASSERT(extensionHandle, NEXUS_Frontend);
    }

    /* recreate callbacks with the extension handle. this allows NEXUS_StopCallbacks to work. */
    if (pDevice->diseqcAppCallback) {
        NEXUS_TaskCallback_Destroy(pDevice->diseqcAppCallback);
        pDevice->diseqcAppCallback = NEXUS_TaskCallback_Create(extensionHandle, NULL);
        if ( NULL == pDevice->diseqcAppCallback ) {
            return BERR_TRACE(BERR_OS_ERROR);
        }
    }
    if (pDevice->lockAppCallback) {
        NEXUS_TaskCallback_Destroy(pDevice->lockAppCallback);
        pDevice->lockAppCallback = NEXUS_TaskCallback_Create(extensionHandle, NULL);
        if ( NULL == pDevice->lockAppCallback ) {
            return BERR_TRACE(BERR_OS_ERROR);
        }
    }

    return 0;
}

NEXUS_SdsDevice *NEXUS_Frontend_P_GetSdsDevice(NEXUS_FrontendHandle handle)
{
    NEXUS_SdsDevice *pDevice;

    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    pDevice = handle->pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_SdsDevice);
    return pDevice;
}

