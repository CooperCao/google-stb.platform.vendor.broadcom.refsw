/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
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
* API Description:
*   API name: Frontend 3510
*    APIs to open, close, and setup initial settings for a BCM3510
*    Demodulator Device.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "nexus_frontend_module.h"
#include "nexus_i2c.h"
#include "breg_i2c_priv.h"
#include "bvsb.h"
#include "priv/nexus_i2c_priv.h"
#include "bvsb.h"
#include "bchp_3510.h"
#include "bcm3510ap.h"

BDBG_MODULE(nexus_frontend_3510);

BTRC_MODULE_DECLARE(ChnChange_Tune);
BTRC_MODULE_DECLARE(ChnChange_TuneLock);

BDBG_OBJECT_ID(NEXUS_3510);

/***************************************************************************
 * Internal Module Structure
 ***************************************************************************/
typedef struct NEXUS_3510
{
	BDBG_OBJECT(NEXUS_3510)
    BVSB_Handle vsbHandle;
    unsigned isrNumber;
    NEXUS_AmplifierHandle amplifier;
    unsigned amplifierPortNumber;
    NEXUS_TunerHandle tuner;
    NEXUS_IfdHandle ifd;
    BKNI_EventHandle isrEvent;
    NEXUS_EventCallbackHandle isrEventCallback;
    BKNI_EventHandle lockEvent;
    NEXUS_EventCallbackHandle lockEventCallback;
    NEXUS_FrontendHandle frontendHandle;
    NEXUS_TaskCallbackHandle lockAppCallback;
    uint16_t i2cAddr;
    NEXUS_I2cHandle i2cHandle;
    unsigned lockCount;
    union
    {
        NEXUS_FrontendVsbSettings vsb;
        NEXUS_FrontendQamSettings qam;
    } lastSettings;
    enum
    {
        UNKNOWN_3510,
        QAM_3510,
        VSB_3510
    } currentMode;
} NEXUS_3510;

/***************************************************************************
 * Module callback functions for tuning
 ***************************************************************************/
static void NEXUS_Frontend_P_3510_Close(NEXUS_FrontendHandle handle);
static NEXUS_Error NEXUS_Frontend_P_3510_TuneVsb(void *handle, const NEXUS_FrontendVsbSettings *pSettings);
static NEXUS_Error NEXUS_Frontend_P_3510_GetVsbStatus(void *handle, NEXUS_FrontendVsbStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_3510_TuneQam(void *handle, const NEXUS_FrontendQamSettings *pSettings);
static NEXUS_Error NEXUS_Frontend_P_3510_GetQamStatus(void *handle, NEXUS_FrontendQamStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_3510_TuneDigital(NEXUS_3510 *pDevice, unsigned frequency, BVSB_AcqParams *pParams);
static NEXUS_Error NEXUS_Frontend_P_3510_ReadSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length, size_t *pNumRead);

/***************************************************************************
Summary:
	ISR Event Handler for a 3510 device
 ***************************************************************************/
static void NEXUS_Frontend_P_3510_IsrEvent(void *pParam)
{
    NEXUS_3510 *pDevice = pParam;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3510);
    BVSB_ProcessInterruptEvent(pDevice->vsbHandle, NULL /* Unused */);
}

/***************************************************************************
Summary:
	Lock callback handler for a 3510 device
 ***************************************************************************/
static void NEXUS_Frontend_P_3510_LockEvent(void *pParam)
{
    NEXUS_3510 *pDevice = pParam;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3510);
    BDBG_MSG(("3510 lock event set"));

    if (pDevice->lockCount == 0) {
        BTRC_TRACE(ChnChange_TuneLock, STOP);
        pDevice->lockCount++;
    }

    if ( pDevice->lockAppCallback )
    {
        NEXUS_TaskCallback_Fire(pDevice->lockAppCallback);
    }
}

/***************************************************************************
Summary:
	Enable/Disable interrupts for a 3510 device
 ***************************************************************************/
static void NEXUS_Frontend_P_3510_L1_isr(void *param1, int param2)
{
    BVSB_Handle vsbHandle = (BVSB_Handle)param2;
    BSTD_UNUSED(param1);
    BDBG_MSG(("3510 L1 ISR"));
    BVSB_HandleInterrupt_isr(vsbHandle);
}

/***************************************************************************
Summary:
	Enable/Disable interrupts for a 3510 device
 ***************************************************************************/
static void NEXUS_Frontend_P_3510_IsrControl_isr(bool enable, void *pParam)
{
    NEXUS_3510 *pDevice = (NEXUS_3510 *)pParam;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3510);
    if ( enable )
    {
        BDBG_MSG(("Enabling Interrupt"));
        NEXUS_Core_EnableInterrupt_isr(pDevice->isrNumber);
    }
    else
    {
        BDBG_MSG(("Disabling Interrupt"));
        NEXUS_Core_DisableInterrupt_isr(pDevice->isrNumber);
    }
}

/***************************************************************************
Summary:
	Get the default settings for a BCM3510 tuner
See Also:
    NEXUS_Frontend_Open3510
 ***************************************************************************/
void NEXUS_Frontend_GetDefault3510Settings(
    NEXUS_3510Settings *pSettings   /* [out] */
    )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->i2cAddr = 0x10;
}

/***************************************************************************
Summary:
	Open a handle to a BCM3510 device.
 ***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_Open3510(
    const NEXUS_3510Settings *pSettings
    )
{
    BERR_Code errCode;
    BVSB_Settings vsbSettings;
    NEXUS_FrontendHandle frontendHandle;
    BREG_I2C_Handle i2cHandle;
    NEXUS_3510 *pDevice;
    uint8_t chipId, apVer, scrVer, cfgVer;

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pSettings->i2cDevice);

    BVSB_GetDefaultSettings(&vsbSettings);

    pDevice = BKNI_Malloc(sizeof(NEXUS_3510));
    if ( NULL == pDevice )
    {
        errCode = BERR_TRACE(BERR_OS_ERROR);
        goto err_malloc;
    }
    BDBG_OBJECT_SET(pDevice, NEXUS_3510);

    vsbSettings.chipAddr = pSettings->i2cAddr;
    vsbSettings.interruptEnableFunc = NEXUS_Frontend_P_3510_IsrControl_isr;
    vsbSettings.interruptEnableFuncParam = (void *)pDevice;
    pDevice->i2cAddr = pSettings->i2cAddr;
    pDevice->i2cHandle = pSettings->i2cDevice;

    /* Populate device structure */
    pDevice->isrNumber = pSettings->isrNumber;
    pDevice->amplifier = pSettings->devices.amplifier;
    pDevice->amplifierPortNumber = pSettings->devices.amplifierPortNumber;
    pDevice->tuner = pSettings->devices.tuner;
    pDevice->ifd = pSettings->devices.ifd;

    /* Grab required BREG_I2C_Handle */
    i2cHandle = NEXUS_I2c_GetRegHandle(pSettings->i2cDevice, NEXUS_MODULE_SELF);
    BDBG_ASSERT(NULL != i2cHandle);

    /* Open VSB device */
    errCode = BVSB_Open(&pDevice->vsbHandle,
                        NULL /* chip */,
                        (void*)i2cHandle,
                        &vsbSettings);
    if ( errCode != BERR_SUCCESS )
    {
        errCode = BERR_TRACE(errCode);
        goto err_vsb;
    }

    /* Success opeining the VSB Device.  Connect Interrupt */
    errCode = NEXUS_Core_ConnectInterrupt(pDevice->isrNumber,
                                          NEXUS_Frontend_P_3510_L1_isr,
                                          (void *)pDevice,
                                          (int)pDevice->vsbHandle);

    if ( errCode != BERR_SUCCESS )
    {
        errCode = BERR_TRACE(errCode);
        goto err_isr;
    }

    /* Initialize Acquisition Processor */
    BDBG_WRN(("Initializating 3510 VSB core..."));
    /* We need a better way to do this, should read the chip id and decide which FW to use */
#if BCHP_CHIP == 7405 && BCHP_VER >= BCHP_VER_B0
    errCode = BVSB_InitAp(pDevice->vsbHandle, bcm3510_ap_b2_image);
#else
    errCode = BVSB_InitAp(pDevice->vsbHandle, bcm3510_ap_image);
#endif
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_init;
    }
    BDBG_WRN(("Initializating 3510 VSB core... Done"));

    /* Get events and register callbacks */
    errCode = BVSB_GetInterruptEventHandle(pDevice->vsbHandle, &pDevice->isrEvent);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_init;
    }
    pDevice->isrEventCallback = NEXUS_RegisterEvent(pDevice->isrEvent, NEXUS_Frontend_P_3510_IsrEvent, pDevice);
    if ( NULL == pDevice->isrEventCallback )
    {
        errCode = BERR_TRACE(BERR_OS_ERROR);
        goto err_init;
    }
    errCode = BVSB_GetLockStateChangeEventHandle(pDevice->vsbHandle, &pDevice->lockEvent);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_lock;
    }
    pDevice->lockEventCallback = NEXUS_RegisterEvent(pDevice->lockEvent, NEXUS_Frontend_P_3510_LockEvent, pDevice);
    if ( NULL == pDevice->lockEventCallback )
    {
        errCode = BERR_TRACE(BERR_OS_ERROR);
        goto err_lock;
    }

    /* Create a Nexus frontend handle */
    pDevice->frontendHandle = frontendHandle = NEXUS_Frontend_P_Create(pDevice);
    if ( NULL == pDevice->frontendHandle )
    {
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err_handle;
    }

    errCode = BVSB_GetApVersion(pDevice->vsbHandle, &chipId, &apVer, &scrVer, &cfgVer);
    if (errCode) {
        errCode = BERR_TRACE(errCode);
        goto err_getapversion;
    }
    BDBG_MSG(("Chip ID 0x%x, ap 0x%x, script 0x%x, cfg 0x%x", chipId, apVer, scrVer, cfgVer));

    /* Establish device capabilities */
    frontendHandle->capabilities.vsb = true;
    frontendHandle->capabilities.qam = true;
    frontendHandle->capabilities.analog = false;
    frontendHandle->capabilities.ifd = false;
    BKNI_Memset(frontendHandle->capabilities.vsbModes, true, sizeof(frontendHandle->capabilities.vsbModes));
    BKNI_Memset(frontendHandle->capabilities.qamModes, true, sizeof(frontendHandle->capabilities.qamModes));

    /* Bind required callbacks */
    frontendHandle->close = NEXUS_Frontend_P_3510_Close;
    frontendHandle->tuneVsb = NEXUS_Frontend_P_3510_TuneVsb;
    frontendHandle->getVsbStatus = NEXUS_Frontend_P_3510_GetVsbStatus;
    frontendHandle->tuneQam = NEXUS_Frontend_P_3510_TuneQam;
    frontendHandle->getQamStatus = NEXUS_Frontend_P_3510_GetQamStatus;
    frontendHandle->readSoftDecisions = NEXUS_Frontend_P_3510_ReadSoftDecisions;

    /* Create app callback */
    pDevice->lockAppCallback = NEXUS_TaskCallback_Create(frontendHandle, NULL);
    if ( NULL == pDevice->lockAppCallback )
    {
        goto err_callback;
    }

    /* Success */
    pDevice->currentMode = UNKNOWN_3510;
    return frontendHandle;
err_callback:
err_getapversion:
    NEXUS_Frontend_P_Destroy(frontendHandle);
err_handle:
    NEXUS_UnregisterEvent(pDevice->lockEventCallback);
err_lock:
    NEXUS_UnregisterEvent(pDevice->isrEventCallback);
err_init:
    NEXUS_Core_DisconnectInterrupt(pDevice->isrNumber);
err_isr:
    BVSB_Close(pDevice->vsbHandle);
err_vsb:
    BKNI_Free(pDevice);
err_malloc:
    return NULL;
}

/***************************************************************************
Summary:
	Close a handle to a BCM3510 device.
See Also:
    NEXUS_Frontend_Open3510
 ***************************************************************************/
static void NEXUS_Frontend_P_3510_Close(
    NEXUS_FrontendHandle handle
    )
{
    NEXUS_3510 *pDevice;

    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    pDevice = (NEXUS_3510 *)handle->pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3510);

    NEXUS_TaskCallback_Destroy(pDevice->lockAppCallback);
    NEXUS_Frontend_P_Destroy(handle);
    NEXUS_UnregisterEvent(pDevice->lockEventCallback);
    NEXUS_UnregisterEvent(pDevice->isrEventCallback);
    NEXUS_Core_DisconnectInterrupt(pDevice->isrNumber);
    BVSB_Close(pDevice->vsbHandle);
    BDBG_OBJECT_DESTROY(pDevice, NEXUS_3510);
    BKNI_Free(pDevice);
}

/***************************************************************************
Summary:
	Probe to see if a BCM3510 device exists with the specified settings
See Also:
    NEXUS_Frontend_Open3510
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Probe3510(
    const NEXUS_3510Settings *pSettings,
    NEXUS_3510ProbeResults *pResults
    )
{
    uint8_t byte=1;
    NEXUS_Error errCode;

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pSettings->i2cDevice);
    BDBG_ASSERT(NULL != pResults);

    BKNI_Memset(pResults, 0, sizeof(*pResults));

    errCode = NEXUS_I2c_Write(pSettings->i2cDevice, pSettings->i2cAddr,
                              BCM3510_HCTL1, &byte, 1) ;
    if ( errCode )
    {
        return errCode;
    }

    errCode = NEXUS_I2c_Read(pSettings->i2cDevice, pSettings->i2cAddr,
                             BCM3510_REVID, &pResults->revision, 1);
    if ( errCode )
    {
        return errCode;
    }

    /* Success */
    return NEXUS_SUCCESS;
}

/***************************************************************************
 * Module callback functions for tuning
 ***************************************************************************/
static NEXUS_Error NEXUS_Frontend_P_3510_TuneVsb(void *handle, const NEXUS_FrontendVsbSettings *pSettings)
{
    BVSB_AcqParams params;
    NEXUS_3510 *pDevice = handle;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3510);
    BDBG_ASSERT(NULL != pSettings);

    BKNI_Memset(&params, 0, sizeof(params));

    switch ( pSettings->mode )
    {
    case NEXUS_FrontendVsbMode_e8:
        params.mode = BVSB_ModulationType_eAnnexDVsb8;
        break;
    case NEXUS_FrontendVsbMode_e16:
        params.mode = BVSB_ModulationType_eAnnexDVsb16;
        break;
    default:
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if ( pSettings->symbolRate )
    {
        params.symbolRate = pSettings->symbolRate;
    }
    else
    {
        params.symbolRate = 5381119;    /* Default for VSB */
    }
    if ( pSettings->ifFrequency )
    {
        params.ifFreq = pSettings->ifFrequency;
    }
    else if ( pDevice->tuner && pDevice->tuner->ifFrequency )
    {
        /* Grab tuner IF frequency if available */
        BDBG_MSG(("Using tuner IF frequency of %u Hz", pDevice->tuner->ifFrequency));
        params.ifFreq = pDevice->tuner->ifFrequency;
    }
    else
    {
        params.ifFreq = 44000000;   /* Default to 44MHz */
    }
    params.symbolRateOffset = pSettings->symbolRateOffset;
    params.ifFreqOffset = pSettings->ifFrequencyOffset;
    params.ntscOffset = pSettings->ntscOffset;

    pDevice->currentMode = VSB_3510;
    pDevice->lastSettings.vsb = *pSettings;
    NEXUS_TaskCallback_Set(pDevice->lockAppCallback, &(pSettings->lockCallback));
    BDBG_MSG(("Acquiring VSB at %u Hz", pSettings->frequency));
    return NEXUS_Frontend_P_3510_TuneDigital(pDevice, pSettings->frequency, &params);
}

static NEXUS_Error NEXUS_Frontend_P_3510_GetVsbStatus(void *handle, NEXUS_FrontendVsbStatus *pStatus)
{
    NEXUS_Error errCode;
    NEXUS_3510 *pDevice = handle;
    struct BVSB_Status status;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3510);
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    pStatus->settings = pDevice->lastSettings.vsb;
    if ( pDevice->currentMode != VSB_3510 )
    {
        /* don't print an error to the console. the status is: no vsb. */
        return BERR_INVALID_PARAMETER;
    }

    BKNI_Memset(&status, 0, sizeof(status));
    errCode = BVSB_GetStatus(pDevice->vsbHandle, &status);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    pStatus->receiverLock = status.bRcvrLocked;
    pStatus->fecLock = status.bFecLocked;
    pStatus->opllLock = status.bOPllLocked;

    pStatus->symbolRate = status.symbolRate;
    pStatus->symbolRateError = status.symbolRateError;

    pStatus->ifFrequency = status.ifFreq;
    pStatus->ifFrequencyError = status.ifFreqError;

    pStatus->ifAgcLevel = status.intAgcLevel;
    pStatus->rfAgcLevel = status.extAgcLevel;
    pStatus->snrEstimate = (status.snrEstimate*100)/256;    /* Normalize to nexus units of 1/100 dB */

    pStatus->fecUncorrected = status.ucorrBlockCount;
    pStatus->viterbiUncorrectedBits = status.corrErrCount;

#if 0
/* not available */
    pStatus->fecCorrected = status.corrBlockCount;
    pStatus->fecClean = status.cleanBlockCount;
    pStatus->bitErrCorrected = status.corrErrCount; /* deprecated */
    pStatus->viterbiTotalBits = (pStatus->fecCorrected + pStatus->fecUncorrected + pStatus->fecClean) * 188 * 8;
    if (pStatus->viterbiTotalBits) {
        pStatus->viterbiErrorRate = (uint32_t)((uint64_t)pStatus->viterbiUncorrectedBits * 2097152 * 1024 / (unsigned)pStatus->viterbiTotalBits);
    }
#endif

    return BERR_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_3510_TuneQam(void *handle, const NEXUS_FrontendQamSettings *pSettings)
{
    BVSB_AcqParams params;
    NEXUS_3510 *pDevice = handle;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3510);
    BDBG_ASSERT(NULL != pSettings);

    BKNI_Memset(&params, 0, sizeof(params));

    if ( pSettings->annex == NEXUS_FrontendQamAnnex_eA )
    {
        switch ( pSettings->mode )
        {
        case NEXUS_FrontendQamMode_e16:
            params.mode = BVSB_ModulationType_eAnnexAQam16;
            break;
        case NEXUS_FrontendQamMode_e32:
            params.mode = BVSB_ModulationType_eAnnexAQam32;
            break;
        case NEXUS_FrontendQamMode_e64:
            params.mode = BVSB_ModulationType_eAnnexAQam64;
            break;
        case NEXUS_FrontendQamMode_e128:
            params.mode = BVSB_ModulationType_eAnnexAQam128;
            break;
        case NEXUS_FrontendQamMode_e256:
            params.mode = BVSB_ModulationType_eAnnexAQam256;
            break;
        default:
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }
    else if ( pSettings->annex == NEXUS_FrontendQamAnnex_eB )
    {
        switch ( pSettings->mode )
        {
        case NEXUS_FrontendQamMode_e64:
            params.mode = BVSB_ModulationType_eAnnexBQam64;
            break;
        case NEXUS_FrontendQamMode_e256:
            params.mode = BVSB_ModulationType_eAnnexBQam256;
            break;
        default:
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }
    else
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if ( pSettings->symbolRate )
    {
        params.symbolRate = pSettings->symbolRate;
    }
    else
    {
        params.symbolRate = NEXUS_Frontend_P_GetDefaultQamSymbolRate(pSettings->mode, pSettings->annex);
    }

    if ( pDevice->tuner && pDevice->tuner->ifFrequency )
    {
        /* Grab tuner IF frequency if available */
        BDBG_MSG(("Using tuner IF frequency of %u Hz", pDevice->tuner->ifFrequency));
        params.ifFreq = pDevice->tuner->ifFrequency;
    }
    else
    {
        params.ifFreq = 44000000;   /* Default to 44MHz */
    }

    pDevice->currentMode = QAM_3510;
    pDevice->lastSettings.qam = *pSettings;
    NEXUS_TaskCallback_Set(pDevice->lockAppCallback, &(pSettings->lockCallback));
    BDBG_MSG(("Acquiring QAM at %u Hz", pSettings->frequency));
    return NEXUS_Frontend_P_3510_TuneDigital(pDevice, pSettings->frequency, &params);
}

static NEXUS_Error NEXUS_Frontend_P_3510_GetQamStatus(void *handle, NEXUS_FrontendQamStatus *pStatus)
{
    NEXUS_Error errCode;
    NEXUS_3510 *pDevice = handle;
    struct BVSB_Status status;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3510);
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    pStatus->settings = pDevice->lastSettings.qam;
    if ( pDevice->currentMode != QAM_3510 )
    {
        /* don't print an error to the console. the status is: no qam. */
        return BERR_INVALID_PARAMETER;
    }

    BKNI_Memset(&status, 0, sizeof(status));
    errCode = BVSB_GetStatus(pDevice->vsbHandle, &status);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    pStatus->receiverLock = status.bRcvrLocked;
    pStatus->fecLock = status.bFecLocked;
    pStatus->opllLock = status.bOPllLocked;

    pStatus->symbolRate = status.symbolRate;
    pStatus->symbolRateError = status.symbolRateError;

    pStatus->ifAgcLevel = status.intAgcLevel;
    pStatus->rfAgcLevel = status.extAgcLevel;
    pStatus->snrEstimate = (status.snrEstimate*100)/256;    /* Normalize to nexus units of 1/100 dB */

    return BERR_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_3510_TuneDigital(NEXUS_3510 *pDevice, unsigned frequency, BVSB_AcqParams *pParams)
{
    NEXUS_Error errCode;

    BDBG_ASSERT(NULL != pDevice);
    BDBG_ASSERT(NULL != pParams);

    if ( pDevice->amplifier )
    {
        errCode = NEXUS_Amplifier_SetInBandMode(pDevice->amplifier, pDevice->amplifierPortNumber, NEXUS_AmplifierInBandMode_eDigital);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    if ( pDevice->tuner )
    {
        BDBG_MSG(("Setting tuner frequency to %u Hz", frequency));
        errCode = NEXUS_Tuner_SetFrequency(pDevice->tuner, NEXUS_TunerMode_eDigital, frequency);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    errCode = BVSB_Acquire(pDevice->vsbHandle, pParams);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    BTRC_TRACE(ChnChange_Tune, STOP);
    BTRC_TRACE(ChnChange_TuneLock, START);
    pDevice->lockCount = 0;

    return BERR_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_3510_ReadSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length, size_t *pNumRead)
{
#define TOTAL_VSB_SOFTDECISIONS 30
    unsigned i;
    NEXUS_Error errCode;
    NEXUS_3510 *pDevice = handle;
    int8_t d_i[TOTAL_VSB_SOFTDECISIONS+1], d_q[TOTAL_VSB_SOFTDECISIONS+1];

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3510);

    errCode = BVSB_GetSoftDecisionBuf(pDevice->vsbHandle, d_i, d_q);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    for ( i=0; i<TOTAL_VSB_SOFTDECISIONS && i<length; i++ )
    {
        pDecisions[i].i = (int16_t)d_i[i] * 256 * 2;
        pDecisions[i].q = (int16_t)d_q[i] * 256 * 2;
    }
    *pNumRead = i;

    return BERR_SUCCESS;
}

