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
*   API name: Frontend 3520
*    APIs to open, close, and setup initial settings for a BCM3520
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
#include "bvsb_3520.h"
#include "bchp_3520.h"
#include "bcm3520ap.h"

BDBG_MODULE(nexus_frontend_3520);

#define REV_A1 0x1
#define REV_B0 0x10

BDBG_OBJECT_ID(NEXUS_3520);

/***************************************************************************
 * Internal Module Structure
 ***************************************************************************/
typedef struct NEXUS_3520
{
    BDBG_OBJECT(NEXUS_3520)
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
    NEXUS_I2cHandle deviceI2cHandle;                /* For the 3520 master I2C Controller */
    NEXUS_TaskCallbackHandle lockAppCallback;
    bool fastAcquisition;
    uint16_t i2cAddr;
    NEXUS_I2cHandle i2cHandle;
    union
    {
        NEXUS_FrontendAnalogSettings analog;
        NEXUS_FrontendVsbSettings vsb;
        NEXUS_FrontendQamSettings qam;
    } lastSettings;
    enum
    {
        UNKNOWN_3520,
        QAM_3520,
        VSB_3520,
        ANALOG_3520
    } currentMode;
} NEXUS_3520;

/***************************************************************************
 * Module callback functions for tuning
 ***************************************************************************/
static void NEXUS_Frontend_P_3520_Close(NEXUS_FrontendHandle handle);
static NEXUS_Error NEXUS_Frontend_P_3520_TuneAnalog(void *handle, const NEXUS_FrontendAnalogSettings *pSettings);
static NEXUS_Error NEXUS_Frontend_P_3520_GetAnalogStatus(void *handle, NEXUS_FrontendAnalogStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_3520_TuneVsb(void *handle, const NEXUS_FrontendVsbSettings *pSettings);
static NEXUS_Error NEXUS_Frontend_P_3520_GetVsbStatus(void *handle, NEXUS_FrontendVsbStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_3520_TuneQam(void *handle, const NEXUS_FrontendQamSettings *pSettings);
static NEXUS_Error NEXUS_Frontend_P_3520_GetQamStatus(void *handle, NEXUS_FrontendQamStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_3520_TuneDigital(NEXUS_3520 *pDevice, unsigned frequency, BVSB_InbandParams *pParams);
static NEXUS_Error NEXUS_Frontend_P_3520_GetSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length);
/***************************************************************************
 * Module callback functions for I2C
 ***************************************************************************/
static BERR_Code NEXUS_Frontend_P_3520_I2cReadNoAddr(void * context, uint16_t chipAddr, uint8_t *pData, size_t length);
static BERR_Code NEXUS_Frontend_P_3520_I2cWriteNoAddr(void * context, uint16_t chipAddr, const uint8_t *pData, size_t length);

/***************************************************************************
Summary:
    ISR Event Handler for a 3520 device
 ***************************************************************************/
static void NEXUS_Frontend_P_3520_IsrEvent(void *pParam)
{
    NEXUS_3520 *pDevice = pParam;
#if BDBG_DEBUG_BUILD
    uint8_t stat, ie;

    NEXUS_I2c_Read(pDevice->i2cHandle, pDevice->i2cAddr,
                             BCM3520_SH_AP_SFR_H_STAT2, &stat, 1);
    NEXUS_I2c_Read(pDevice->i2cHandle, pDevice->i2cAddr,
                             BCM3520_SH_AP_SFR_H_IE2, &ie, 1);
    BDBG_MSG(("Calling BVSB_ProcessInterruptEvent - Stat=0x%02x, IE=0x%02x", stat, ie));
#endif
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3520);
    BVSB_ProcessInterruptEvent(pDevice->vsbHandle);
}

/***************************************************************************
Summary:
    Lock callback handler for a 3520 device
 ***************************************************************************/
static void NEXUS_Frontend_P_3520_LockEvent(void *pParam)
{
    NEXUS_3520 *pDevice = pParam;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3520);
    BDBG_MSG(("3520 lock event set"));

    if ( pDevice->lockAppCallback )
    {
        NEXUS_TaskCallback_Fire(pDevice->lockAppCallback);
    }
}

/***************************************************************************
Summary:
    Enable/Disable interrupts for a 3520 device
 ***************************************************************************/
static void NEXUS_Frontend_P_3520_L1_isr(void *param1, int param2)
{
    BVSB_Handle vsbHandle = (BVSB_Handle)param2;
    BSTD_UNUSED(param1);
    BDBG_MSG(("3520 L1 ISR"));
    BVSB_HandleInterrupt_isr(vsbHandle);
}

/***************************************************************************
Summary:
    Enable/Disable interrupts for a 3520 device
 ***************************************************************************/
static void NEXUS_Frontend_P_3520_IsrControl_isr(bool enable, void *pParam)
{
    NEXUS_3520 *pDevice = (NEXUS_3520 *)pParam;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3520);
    if ( enable )
    {
        NEXUS_Core_EnableInterrupt_isr(pDevice->isrNumber);
    }
    else
    {
        NEXUS_Core_DisableInterrupt_isr(pDevice->isrNumber);
    }
}

/***************************************************************************
Summary:
    Get the default settings for a BCM3520 tuner
See Also:
    NEXUS_Frontend_Open3520
 ***************************************************************************/
void NEXUS_Frontend_GetDefault3520Settings(
    NEXUS_3520Settings *pSettings   /* [out] */
    )
{
    BVSB_Settings vsbSettings;
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    BVSB_3520_GetDefaultSettings(&vsbSettings);
    pSettings->fastAcquisition = vsbSettings.vsb.bFastAcq;
    pSettings->bHead4 = vsbSettings.inbandOi.bHead4;
    pSettings->bSync1 = vsbSettings.inbandOi.bSync1;
    pSettings->bXBERT = vsbSettings.inbandOi.bXBERT;
    pSettings->bErrinv = vsbSettings.inbandOi.bErrinv;
    pSettings->bSyncinv = vsbSettings.inbandOi.bSyncinv;
    pSettings->bVldinv = vsbSettings.inbandOi.bVldinv;
    pSettings->bClksup = vsbSettings.inbandOi.bClksup;
    pSettings->bClkinv = vsbSettings.inbandOi.bClkinv;
    pSettings->bSerial = vsbSettings.inbandOi.bSerial;
    pSettings->i2cAddr = 0x0F;
}

/***************************************************************************
Summary:
    Open a handle to a BCM3520 device.
 ***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_Open3520(
    const NEXUS_3520Settings *pSettings
    )
{
    BERR_Code errCode;
    BVSB_Settings vsbSettings;
    NEXUS_FrontendHandle frontendHandle;
    BREG_I2C_Handle i2cHandle;
    NEXUS_3520 *pDevice;
    uint8_t revision;
    NEXUS_I2cCallbacks i2cCallbacks;
    uint16_t chipId, chipVer;
    uint8_t apVer, scrVer, cfgVer;

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pSettings->i2cDevice);

    BVSB_3520_GetDefaultSettings(&vsbSettings);

    vsbSettings.vsb.bFastAcq = pSettings->fastAcquisition;
    vsbSettings.qam.bFastAcq = pSettings->fastAcquisition;
    vsbSettings.inbandOi.bHead4 = pSettings->bHead4;
    vsbSettings.inbandOi.bSync1 = pSettings->bSync1;
    vsbSettings.inbandOi.bXBERT = pSettings->bXBERT;
    vsbSettings.inbandOi.bErrinv = pSettings->bErrinv;
    vsbSettings.inbandOi.bSyncinv = pSettings->bSyncinv;
    vsbSettings.inbandOi.bVldinv = pSettings->bVldinv;
    vsbSettings.inbandOi.bClksup = pSettings->bClksup;
    vsbSettings.inbandOi.bClkinv = pSettings->bClkinv;
    vsbSettings.inbandOi.bSerial = pSettings->bSerial;

    pDevice = BKNI_Malloc(sizeof(NEXUS_3520));
    if ( NULL == pDevice )
    {
        errCode = BERR_TRACE(BERR_OS_ERROR);
        goto err_malloc;
    }
    BDBG_OBJECT_SET(pDevice, NEXUS_3520);

    vsbSettings.i2c.chipAddr = pSettings->i2cAddr;
    vsbSettings.i2c.interruptEnableFunc = NEXUS_Frontend_P_3520_IsrControl_isr;
    vsbSettings.i2c.interruptEnableFuncParam = (void *)pDevice;
    pDevice->fastAcquisition = vsbSettings.vsb.bFastAcq;
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
                        NULL, /* only required for 356x platforms */
                        &vsbSettings);
    if ( errCode != BERR_SUCCESS )
    {
        errCode = BERR_TRACE(errCode);
        goto err_vsb;
    }

    /* Success opeining the VSB Device.  Connect Interrupt */
    errCode = NEXUS_Core_ConnectInterrupt(pDevice->isrNumber,
                                          NEXUS_Frontend_P_3520_L1_isr,
                                          (void *)pDevice,
                                          (int)pDevice->vsbHandle);

    if ( errCode != BERR_SUCCESS )
    {
        errCode = BERR_TRACE(errCode);
        goto err_isr;
    }

    /* Initialize Acquisition Processor */
    BDBG_WRN(("Initializating 3520 VSB core..."));
    BVSB_GetChipRevision(pDevice->vsbHandle, &revision );
    if ( revision == REV_A1 )
    {
        BDBG_MSG(("Forcing fast acq for REV A1"));
        vsbSettings.vsb.bFastAcq = true;
        vsbSettings.qam.bFastAcq = true;
        pDevice->fastAcquisition = true;
        errCode = BVSB_InitAp(pDevice->vsbHandle, bcm3520_ap_a1_image);
    }
    else
    {
        errCode = BVSB_InitAp(pDevice->vsbHandle, bcm3520_ap_b0_image);
    }
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_init;
    }
    BDBG_WRN(("Initializating 3520 VSB core... Done"));

    /* Get events and register callbacks */
    errCode = BVSB_GetInterruptEventHandle(pDevice->vsbHandle, &pDevice->isrEvent);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_init;
    }
    pDevice->isrEventCallback = NEXUS_RegisterEvent(pDevice->isrEvent, NEXUS_Frontend_P_3520_IsrEvent, pDevice);
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
    pDevice->lockEventCallback = NEXUS_RegisterEvent(pDevice->lockEvent, NEXUS_Frontend_P_3520_LockEvent, pDevice);
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

    errCode = BVSB_GetApVersion(pDevice->vsbHandle, &chipId, &chipVer, &apVer, &scrVer, &cfgVer);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_i2c;
    }
    else
    {
        BDBG_MSG(("Chip ID 0x%x, ver 0x%x, ap 0x%x, script 0x%x, cfg 0x%x", chipId, chipVer, apVer, scrVer, cfgVer));
    }

    /* Establish device capabilities */
    frontendHandle->capabilities.vsb = true;
    frontendHandle->capabilities.qam = true;
    frontendHandle->capabilities.analog = true;
    frontendHandle->capabilities.ifd = false;      /* TODO: Perhaps should be conditioned on 3520 vs 3517? */
    BKNI_Memset(frontendHandle->capabilities.vsbModes, true, sizeof(frontendHandle->capabilities.vsbModes));
    BKNI_Memset(frontendHandle->capabilities.qamModes, true, sizeof(frontendHandle->capabilities.qamModes));

    /* Bind required callbacks */
    frontendHandle->close = NEXUS_Frontend_P_3520_Close;
    frontendHandle->tuneAnalog = NEXUS_Frontend_P_3520_TuneAnalog;
    frontendHandle->getAnalogStatus = NEXUS_Frontend_P_3520_GetAnalogStatus;
    frontendHandle->tuneVsb = NEXUS_Frontend_P_3520_TuneVsb;
    frontendHandle->getVsbStatus = NEXUS_Frontend_P_3520_GetVsbStatus;
    frontendHandle->tuneQam = NEXUS_Frontend_P_3520_TuneQam;
    frontendHandle->getQamStatus = NEXUS_Frontend_P_3520_GetQamStatus;
    frontendHandle->getSoftDecisions = NEXUS_Frontend_P_3520_GetSoftDecisions;

    /* Set up I2C handle */
    NEXUS_I2c_InitCallbacks(&i2cCallbacks);
    i2cCallbacks.readNoAddr = NEXUS_Frontend_P_3520_I2cReadNoAddr;
    i2cCallbacks.writeNoAddr = NEXUS_Frontend_P_3520_I2cWriteNoAddr;
    pDevice->deviceI2cHandle = NEXUS_I2c_CreateHandle(NEXUS_MODULE_SELF,
                                                      pDevice,
                                                      &i2cCallbacks);

    if ( NULL == pDevice->deviceI2cHandle )
    {
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err_i2c;
    }

    /* Create app callback */
    pDevice->lockAppCallback = NEXUS_TaskCallback_Create(frontendHandle, NULL);
    if ( NULL == pDevice->lockAppCallback )
    {
        goto err_callback;
    }

    /* Success */
    pDevice->currentMode = UNKNOWN_3520;
    return frontendHandle;
err_callback:
    NEXUS_I2c_DestroyHandle(pDevice->deviceI2cHandle);
err_i2c:
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
    Close a handle to a BCM3520 device.
See Also:
    NEXUS_Frontend_Open3520
 ***************************************************************************/
static void NEXUS_Frontend_P_3520_Close(
    NEXUS_FrontendHandle handle
    )
{
    NEXUS_3520 *pDevice;

    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    pDevice = (NEXUS_3520 *)handle->pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3520);

    NEXUS_I2c_DestroyHandle(pDevice->deviceI2cHandle);
    NEXUS_Frontend_P_Destroy(handle);
    NEXUS_UnregisterEvent(pDevice->lockEventCallback);
    NEXUS_TaskCallback_Destroy(pDevice->lockAppCallback);
    NEXUS_UnregisterEvent(pDevice->isrEventCallback);
    NEXUS_Core_DisconnectInterrupt(pDevice->isrNumber);
    BVSB_Close(pDevice->vsbHandle);
    BDBG_OBJECT_DESTROY(pDevice, NEXUS_3520);
    BKNI_Free(pDevice);
}

/***************************************************************************
Summary:
    Probe to see if a BCM3520 device exists with the specified settings
See Also:
    NEXUS_Frontend_Open3520
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Probe3520(
    const NEXUS_3520Settings *pSettings,
    NEXUS_3520ProbeResults *pResults
    )
{
    uint8_t byte=1;
    NEXUS_Error errCode;

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pSettings->i2cDevice);
    BDBG_ASSERT(NULL != pResults);

    BKNI_Memset(pResults, 0, sizeof(*pResults));

    errCode = NEXUS_I2c_Write(pSettings->i2cDevice, pSettings->i2cAddr,
                              BCM3520_SH_AP_SFR_H_CTL1, &byte, 1) ;
    if ( errCode )
    {
        return errCode;
    }

    errCode = NEXUS_I2c_Read(pSettings->i2cDevice, pSettings->i2cAddr,
                             BCM3520_SH_AP_SFR_H_STAT1, &pResults->revision, 1);
    if ( errCode )
    {
        return errCode;
    }

    /* Success */
    return NEXUS_SUCCESS;
}

/***************************************************************************
Summary:
    Get an I2C Handle for the 3520's master I2C controller.
Description:
    The BCM3520 has an on-board I2C controller that can be used to control
    tuner/amplifier devices as opposed to using a host I2C Channel.
    Please be aware, this controller has limited functionality and only
    supports the ReadNoAddr and WriteNoAddr routines.
See Also:
    NEXUS_I2c_Open
    NEXUS_I2c_ReadNoAddr
    NEXUS_I2c_WriteNoAddr
 ***************************************************************************/
NEXUS_I2cHandle NEXUS_Frontend_Get3520MasterI2c(
    NEXUS_FrontendHandle handle
    )
{
    NEXUS_3520 *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_3520 *)handle->pDeviceHandle;
    return pDevice->deviceI2cHandle;
}

/***************************************************************************
 * Module callback functions for I2C -- I2C Auto-synchronizes these.
 ***************************************************************************/
static BERR_Code NEXUS_Frontend_P_3520_I2cReadNoAddr(void * context, uint16_t chipAddr, uint8_t *pData, size_t length)
{
    NEXUS_3520 *pDevice = context;
    uint8_t dummy;

    BDBG_ASSERT(NULL != context);

    /* Issue a read with no preceding write */
    return BVSB_Mi2cRead(pDevice->vsbHandle, chipAddr<<1, &dummy, 0, pData, length);
}

static BERR_Code NEXUS_Frontend_P_3520_I2cWriteNoAddr(void * context, uint16_t chipAddr, const uint8_t *pData, size_t length)
{
    NEXUS_3520 *pDevice = context;

    BDBG_ASSERT(NULL != context);

    /* BVSB does not have a const pointer for data */
    return BVSB_Mi2cWrite(pDevice->vsbHandle, chipAddr<<1, (uint8_t *)((unsigned long)pData), length);
}

/***************************************************************************
 * Module callback functions for tuning
 ***************************************************************************/
static NEXUS_Error NEXUS_Frontend_P_3520_TuneAnalog(void *handle, const NEXUS_FrontendAnalogSettings *pSettings)
{
    NEXUS_Error errCode;
    NEXUS_3520 *pDevice = handle;
    BVSB_InbandParams tuneParams;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3520);
    BDBG_ASSERT(NULL != pSettings);

    BKNI_Memset(&tuneParams, 0, sizeof(tuneParams));
    tuneParams.mode = BVSB_InbandMode_eNTSC;
    pDevice->lastSettings.analog = *pSettings;

    if ( pDevice->amplifier )
    {
        errCode = NEXUS_Amplifier_SetInBandMode(pDevice->amplifier, pDevice->amplifierPortNumber, NEXUS_AmplifierInBandMode_eAnalog);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    if ( pDevice->tuner )
    {
        errCode = NEXUS_Tuner_SetFrequency(pDevice->tuner, NEXUS_TunerMode_eAnalog, pSettings->frequency);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    /* Only try to acquire if we have an integrated IFD */
    pDevice->currentMode = ANALOG_3520;
    NEXUS_TaskCallback_Set(pDevice->lockAppCallback, &(pSettings->ifdSettings.lockCallback));
    if ( pDevice->frontendHandle->capabilities.ifd )
    {
        BVSB_Settings vsbSettings;

        BVSB_GetSettings(pDevice->vsbHandle, &vsbSettings);
        vsbSettings.ntsc.bFastAcq = pSettings->fastAcquisition;
        vsbSettings.ntsc.bTerr = pSettings->terrestrial;
        errCode = BVSB_SetSettings(pDevice->vsbHandle, &vsbSettings);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        errCode = BVSB_AcquireInband(pDevice->vsbHandle, &tuneParams);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    /* Success */
    return BERR_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_3520_GetAnalogStatus(void *handle, NEXUS_FrontendAnalogStatus *pStatus)
{
    NEXUS_Error errCode;
    NEXUS_3520 *pDevice = handle;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    pStatus->settings = pDevice->lastSettings.analog;
    if ( pDevice->currentMode != ANALOG_3520 )
    {
        /* don't print an error to the console. the status is: no analog. */
        return BERR_INVALID_PARAMETER;
    }

    if ( pDevice->frontendHandle->capabilities.ifd )
    {
        BVSB_NtscStatus status;
        BVSB_BtscStatus btscStatus;
        BKNI_Memset(&status, 0, sizeof(status));
        errCode = BVSB_GetNtscStatus(pDevice->vsbHandle, &status);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        errCode = BVSB_GetBtscStatus(pDevice->vsbHandle, &btscStatus);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        pStatus->locked = status.bHsyncLocked;     /* true if HSYNC is locked    */
        pStatus->carrierOffset = status.pixCarrFreqError;/* pix carrier error in Hz */
        pStatus->rfAgcLevel = status.rf_agc;           /* AGT_DELTA_SIGMA level in units of 1/10 percent */
        pStatus->ifAgcLevel = status.if_agc;           /* AGI_DELTA_SIGMA level in units of 1/10 percent */
        pStatus->sapDetected = btscStatus.bSapDetected;
        pStatus->stereoDetected = btscStatus.bStereoDetected;
    }

    return BERR_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_3520_TuneVsb(void *handle, const NEXUS_FrontendVsbSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BVSB_InbandParams params;
    NEXUS_3520 *pDevice = handle;
    unsigned ifFrequency;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pSettings);

    BKNI_Memset(&params, 0, sizeof(params));

    switch ( pSettings->mode )
    {
    case NEXUS_FrontendVsbMode_e8:
        params.mode = BVSB_InbandMode_e8VSB;
        break;
    case NEXUS_FrontendVsbMode_e16:
        params.mode = BVSB_InbandMode_e16VSB;
        break;
    default:
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    ifFrequency  = pSettings->ifFrequency;
    if(pSettings->ifFrequency > 27000000) {
         if(pSettings->spectralInversion == NEXUS_FrontendVsbSpectralInversion_eInverted)
            ifFrequency = 54000000 - pSettings->ifFrequency;
    }
    else {
         if(pSettings->spectralInversion == NEXUS_FrontendVsbSpectralInversion_eNormal)
            ifFrequency = 54000000 - pSettings->ifFrequency;     
    }

    rc = BVSB_SetInbandIfFreq(pDevice->vsbHandle, ifFrequency);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    /* NOTE: symbolRate is stored internally and there's no need to change them normally. ntscOffset is unused as well.*/
    params.symbolRateOffset = pSettings->symbolRateOffset;
    params.ifFreqOffset = pSettings->ifFrequencyOffset;

    pDevice->currentMode = VSB_3520;
    pDevice->lastSettings.vsb = *pSettings;
    NEXUS_TaskCallback_Set(pDevice->lockAppCallback, &(pSettings->lockCallback));
    
    rc = NEXUS_Frontend_P_3520_TuneDigital(pDevice, pSettings->frequency, &params);
    if(rc){rc = BERR_TRACE(rc); goto done;} 

done:
    return rc;  
}

static NEXUS_Error NEXUS_Frontend_P_3520_GetVsbStatus(void *handle, NEXUS_FrontendVsbStatus *pStatus)
{
    NEXUS_Error errCode;
    NEXUS_3520 *pDevice = handle;
    struct BVSB_VsbStatus status;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3520);
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    pStatus->settings = pDevice->lastSettings.vsb;
    if ( pDevice->currentMode != VSB_3520 )
    {
        /* don't print an error to the console. the status is: no vsb. */
        return BERR_INVALID_PARAMETER;
    }

    BKNI_Memset(&status, 0, sizeof(status));
    errCode = BVSB_GetVsbStatus(pDevice->vsbHandle, &status);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    pStatus->receiverLock = status.bFecLocked && status.bPllLocked;
    pStatus->fecLock = status.bFecLocked;
    pStatus->opllLock = status.bPllLocked;

    pStatus->symbolRate = status.symbolRate;
    pStatus->symbolRateError = status.symbolRateError;

    pStatus->ifFrequency = status.ifFreq;
    pStatus->ifFrequencyError = status.ifFreqError;

    pStatus->ifAgcLevel = status.if_agc;
    pStatus->rfAgcLevel = status.rf_agc;
    pStatus->intAgcLevel = status.agf;
    pStatus->snrEstimate = (status.snr*100)/256;    /* Normalize to nexus units of 1/100 dB */

    pStatus->fecCorrected = status.corrBlockCount;
    pStatus->fecUncorrected = status.ucorrBlockCount;
    pStatus->fecClean = status.cleanBlockCount;
    pStatus->bitErrCorrected = status.corrErrCount;
    pStatus->reacquireCount = status.reacqCount;

    return BERR_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_3520_TuneQam(void *handle, const NEXUS_FrontendQamSettings *pSettings)
{
    BVSB_InbandParams params;
    NEXUS_3520 *pDevice = handle;
    unsigned ifFrequency;
    NEXUS_Error errCode;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3520);
    BDBG_ASSERT(NULL != pSettings);

    BKNI_Memset(&params, 0, sizeof(params));

    if ( pSettings->annex == NEXUS_FrontendQamAnnex_eA )
    {
        switch ( pSettings->mode )
        {
        case NEXUS_FrontendQamMode_e16:
            params.mode = BVSB_InbandMode_e16QAM_A;
            break;
        case NEXUS_FrontendQamMode_e32:
            params.mode = BVSB_InbandMode_e32QAM_A;
            break;
        case NEXUS_FrontendQamMode_e64:
            params.mode = BVSB_InbandMode_e64QAM_A;
            break;
        case NEXUS_FrontendQamMode_e128:
            params.mode = BVSB_InbandMode_e128QAM_A;
            break;
        case NEXUS_FrontendQamMode_e256:
            params.mode = BVSB_InbandMode_e256QAM_A;
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
            params.mode = BVSB_InbandMode_e64QAM_B;
            break;
        case NEXUS_FrontendQamMode_e128:
            params.mode = BVSB_InbandMode_e128QAM_B;
            break;
        case NEXUS_FrontendQamMode_e256:
            params.mode = BVSB_InbandMode_e256QAM_B;
            break;
        case NEXUS_FrontendQamMode_e512:
            params.mode = BVSB_InbandMode_e512QAM_B;
            break;
        case NEXUS_FrontendQamMode_e1024:
            params.mode = BVSB_InbandMode_e1024QAM_B;
            break;
        default:
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }
    else
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    ifFrequency  = pSettings->ifFrequency;
    if(pSettings->ifFrequency > 27000000) {
         if(pSettings->spectralInversion == NEXUS_FrontendQamSpectralInversion_eInverted)
            ifFrequency = 54000000 - pSettings->ifFrequency;
    }
    else {
         if(pSettings->spectralInversion == NEXUS_FrontendQamSpectralInversion_eNormal)
            ifFrequency = 54000000 - pSettings->ifFrequency;     
    }

    errCode = BVSB_SetInbandIfFreq(pDevice->vsbHandle, ifFrequency);
    if(errCode){ return BERR_TRACE(errCode);}

    /* ifFrequency and symbolRate for various QAM modes are stored internally
       with no way to change them. */
    params.ifFreqOffset = 0;
    params.symbolRateOffset = 0;
    if ( pSettings->symbolRate )
    {
        params.symbolRate = pSettings->symbolRate;
    }
    else
    {
        params.symbolRate = NEXUS_Frontend_P_GetDefaultQamSymbolRate(pSettings->mode, pSettings->annex);
    }

    pDevice->currentMode = QAM_3520;
    pDevice->lastSettings.qam = *pSettings;
    NEXUS_TaskCallback_Set(pDevice->lockAppCallback, &(pSettings->lockCallback));
    return NEXUS_Frontend_P_3520_TuneDigital(pDevice, pSettings->frequency, &params);
}

static NEXUS_Error NEXUS_Frontend_P_3520_GetQamStatus(void *handle, NEXUS_FrontendQamStatus *pStatus)
{
    NEXUS_Error errCode;
    NEXUS_3520 *pDevice = handle;
    struct BVSB_QamStatus status;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3520);
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    pStatus->settings = pDevice->lastSettings.qam;
    if ( pDevice->currentMode != QAM_3520 )
    {
        /* don't print an error to the console. the status is: no vsb. */
        return BERR_INVALID_PARAMETER;
    }

    BKNI_Memset(&status, 0, sizeof(status));
    errCode = BVSB_GetQamStatus(pDevice->vsbHandle, &status);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    pStatus->receiverLock = status.bFecLocked && status.bPllLocked;
    pStatus->fecLock = status.bFecLocked;
    pStatus->opllLock = status.bPllLocked;
    pStatus->spectrumInverted = status.bSpinv;

    pStatus->symbolRate = status.symbolRate;
    pStatus->symbolRateError = status.symbolRateError;

    pStatus->berEstimate = 1;   /* Nothing. */

    pStatus->ifAgcLevel = status.if_agc;
    pStatus->rfAgcLevel = status.rf_agc;
    pStatus->intAgcLevel = status.agf;
    pStatus->snrEstimate = (status.snr*100)/256;

    pStatus->fecCorrected = status.corrBlockCount;
    pStatus->fecUncorrected = status.ucorrBlockCount;
    pStatus->fecClean = status.cleanBlockCount;
    pStatus->bitErrCorrected = status.berCount;
    pStatus->reacquireCount = status.reacqCount;

    return BERR_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_3520_TuneDigital(NEXUS_3520 *pDevice, unsigned frequency, BVSB_InbandParams *pParams)
{
    NEXUS_Error errCode;
    BVSB_Settings vsbSettings;

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
        errCode = NEXUS_Tuner_SetFrequency(pDevice->tuner, NEXUS_TunerMode_eDigital, frequency);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    BVSB_GetSettings(pDevice->vsbHandle, &vsbSettings);
    vsbSettings.vsb.bAutoAcq = pDevice->lastSettings.vsb.autoAcquire;
    vsbSettings.vsb.bNtscSweep = pDevice->lastSettings.vsb.ntscSweep;
    vsbSettings.vsb.bRfiSweep = pDevice->lastSettings.vsb.rfiSweep;
    errCode = BVSB_SetSettings(pDevice->vsbHandle, &vsbSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    errCode = BVSB_AcquireInband(pDevice->vsbHandle, pParams);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Get devices attached to 3520 device.
Description:
    This allows the platform layer to override the initial device attachments
    specified at device open time.  This is required if any of the attached
    devices make use of the 3520 I2C Controller as opposed to the host-based
    controllers.
See Also:
    NEXUS_Frontend_Open3520
 ***************************************************************************/
void NEXUS_Frontend_Get3520Devices(
    NEXUS_FrontendHandle handle,
    NEXUS_3520Devices *pDevices /* [out] */
    )
{
    NEXUS_3520 *pDevice;
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pDevices);

    pDevice = (NEXUS_3520 *)handle->pDeviceHandle;

    pDevices->amplifier = pDevice->amplifier;
    pDevices->tuner = pDevice->tuner;
    pDevices->ifd = pDevice->ifd;
    pDevices->amplifierPortNumber = pDevice->amplifierPortNumber;
}

/***************************************************************************
Summary:
    Set devices attached to 3520 device.
Description:
    This allows the platform layer to override the initial device attachments
    specified at device open time.  This is required if any of the attached
    devices make use of the 3520 I2C Controller as opposed to the host-based
    controllers.
See Also:
    NEXUS_Frontend_Open3520
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Set3520Devices(
    NEXUS_FrontendHandle handle,
    const NEXUS_3520Devices *pDevices
    )
{
    NEXUS_3520 *pDevice;
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pDevices);

    pDevice = (NEXUS_3520 *)handle->pDeviceHandle;

    BDBG_ASSERT(pDevice->currentMode == UNKNOWN_3520);  /* Must be called prior to first tune */

    pDevice->amplifier = pDevices->amplifier;
    pDevice->tuner = pDevices->tuner;
    pDevice->ifd = pDevices->ifd;
    pDevice->amplifierPortNumber = pDevices->amplifierPortNumber;

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Get an array of soft decisions for a constellation.

Description:
    I and Q range from 32767 to -32768 for all devices, however precision
    may vary per device.
***************************************************************************/
static NEXUS_Error NEXUS_Frontend_P_3520_GetSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length)
{
#define TOTAL_VSB_SOFTDECISIONS 30
    int j;
    size_t i;
    NEXUS_Error errCode;
    NEXUS_3520 *pDevice = handle;
    int16_t d_i[TOTAL_VSB_SOFTDECISIONS], d_q[TOTAL_VSB_SOFTDECISIONS];

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3520);

    for( i=0; i<length; i += TOTAL_VSB_SOFTDECISIONS )
    {
        errCode = BVSB_GetSoftDecisionBuf(pDevice->vsbHandle, d_i, d_q);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        for ( j=0; j<TOTAL_VSB_SOFTDECISIONS && i+j<length; j++ )
        {
            pDecisions[i+j].i = d_i[j] * 32;
            pDecisions[i+j].q = d_q[j] * 32;
        }
    }

    return BERR_SUCCESS;
}

