/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
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
*   API name: BCM7125 Tuner
*    APIs to open, close, and setup initial settings for a BCM7125 Tuner Device.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/


#include "nexus_frontend_module.h"
#include "btnr_7125.h"
#include "priv/nexus_i2c_priv.h"
#include "nexus_frontend_7125_priv.h"

BDBG_MODULE(nexus_tuner_7125);

BDBG_OBJECT_ID(NEXUS_7125tnr);
typedef struct NEXUS_7125tnr
{
    BDBG_OBJECT(NEXUS_7125tnr)
    BTNR_Handle tnrHandle;
    BKNI_EventHandle isrEvent;
    NEXUS_7125TunerSettings settings;
    NEXUS_EventCallbackHandle isrEventCallback;
} NEXUS_7125tnr;

static void NEXUS_Tuner_P_7125_Close(void *pDev);

void NEXUS_Tuner_GetDefault7125Settings(
    NEXUS_7125TunerSettings *pSettings   /* [out] */
    )
{
	BTNR_7125_Settings tnrSettings;
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
	BTNR_7125_GetDefaultSettings(&tnrSettings);
	pSettings->Max_AGC_Threshold = tnrSettings.VgaCal.Max_AGC_Threshold;
	pSettings->Min_AGC_Threshold = tnrSettings.VgaCal.Min_AGC_Threshold;
}


/***************************************************************************
Summary:
    ISR Event Handler for a 7125 device
 ***************************************************************************/
static void NEXUS_Tuner_P_7125_IsrEvent(void *pParam)
{
    NEXUS_7125tnr *pDevice = pParam;
    BDBG_MSG(("Calling BTNR_ProcessInterruptEvent"));
    BTNR_7125_ProcessInterruptEvent(pDevice->tnrHandle);
}

NEXUS_TunerHandle NEXUS_Tuner_Open7125( /* attr{destructor=NEXUS_Tuner_Close} */
    const NEXUS_7125TunerSettings *pSettings
    )
{
    NEXUS_Tuner *pTuner;
    NEXUS_7125tnr *pDevice;
    NEXUS_Error errCode;
    BTNR_7125_Settings tnrSettings;
    BTNR_Handle tnrHandle;

    pDevice = BKNI_Malloc(sizeof(NEXUS_7125tnr));
    if ( NULL == pDevice )
    {
        errCode = BERR_TRACE(BERR_OS_ERROR);
        goto err_malloc;
    }
    BKNI_Memset(pDevice, 0, sizeof(NEXUS_7125tnr));
    BDBG_OBJECT_SET(pDevice, NEXUS_7125tnr);

    BDBG_ASSERT(NULL != pSettings);

    BTNR_7125_GetDefaultSettings(&tnrSettings);

    /* TODO */
    #if 1
    if ( NULL != pSettings->qamLnaI2c )
    {
        tnrSettings.hI2cLNA = NEXUS_I2c_GetRegHandle(pSettings->qamLnaI2c, NEXUS_MODULE_SELF);
        BDBG_ASSERT(NULL != tnrSettings.hI2cLNA);
    }
    #endif

    tnrSettings.hTmr = g_pCoreHandles->tmr;
    tnrSettings.hHeap = g_pCoreHandles->heap[0].mem;
	tnrSettings.unitNumber = pSettings->unitNumber;
	tnrSettings.VgaCal.Max_AGC_Threshold = pSettings->Max_AGC_Threshold;
	tnrSettings.VgaCal.Min_AGC_Threshold = pSettings->Min_AGC_Threshold;
    tnrSettings.skipInit = pSettings->skipInit;

    errCode = BTNR_7125_Open(&tnrHandle, g_pCoreHandles->reg, &tnrSettings);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_tnr_open;
    }

    pDevice->tnrHandle = tnrHandle;
    errCode = BTNR_7125_GetInterruptEventHandle(tnrHandle, &pDevice->isrEvent);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_init;
    }
    pDevice->isrEventCallback = NEXUS_RegisterEvent(pDevice->isrEvent, NEXUS_Tuner_P_7125_IsrEvent, pDevice);
    if ( NULL == pDevice->isrEventCallback )
    {
        errCode = BERR_TRACE(BERR_OS_ERROR);
        goto err_init;
    }

    pTuner = NEXUS_Tuner_P_Create(pDevice);
    if ( NULL == pTuner )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_tuner_create;
    }
    pTuner->close = NEXUS_Tuner_P_7125_Close;

    errCode = NEXUS_Tuner_Set7125Settings(pTuner, pSettings);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        NEXUS_Tuner_Close(pTuner);
        return NULL;
    }

    return pTuner;

err_tuner_create:
    NEXUS_UnregisterEvent(pDevice->isrEventCallback);
err_init:
    BTNR_Close(tnrHandle);
err_tnr_open:
    BDBG_OBJECT_DESTROY(pDevice, NEXUS_7125tnr);
    BKNI_Free(pDevice);
err_malloc:
    return NULL;
}

/***************************************************************************
Summary:
    Get settings for a BCM7125 tuner
 ***************************************************************************/
void NEXUS_Tuner_GetDefault7125TuneSettings(
    NEXUS_7125TuneSettings *pSettings  /* [out] */
    )
{
    BTNR_7125_TuneSettings tnrSettings;

    BDBG_ASSERT(NULL != pSettings);

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    BTNR_7125_GetDefaultTuneSettings(&tnrSettings);
    switch ( tnrSettings.Bandwidth )
    {
    case BTNR_7125_Bandwidth_e8MHz:
        pSettings->bandwidth = 8;
        break;
    case BTNR_7125_Bandwidth_e7MHz:
        pSettings->bandwidth = 7;
        break;
    case BTNR_7125_Bandwidth_e6MHz:
        pSettings->bandwidth = 6;
        break;
    case BTNR_7125_Bandwidth_e5MHz:
        pSettings->bandwidth = 5;
        break;
    /* default intentionally omitted to generate warnings if needed */
    }
    pSettings->frequency = tnrSettings.Freq;
}

/***************************************************************************
Summary:
    Tune a BCM7125 tuner
 ***************************************************************************/
NEXUS_Error NEXUS_Tuner_Tune7125(
    NEXUS_TunerHandle handle,
    const NEXUS_7125TuneSettings *pSettings
    )
{
    BTNR_7125_TuneSettings tnrSettings;
    BERR_Code errCode;
    NEXUS_7125tnr *pDevice;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pSettings);

    pDevice = (NEXUS_7125tnr *)handle->pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7125tnr);

    BTNR_7125_GetDefaultTuneSettings(&tnrSettings);
    switch ( pSettings->bandwidth )
    {
    case 8:
        tnrSettings.Bandwidth = BTNR_7125_Bandwidth_e8MHz;
        break;
    case 7:
        tnrSettings.Bandwidth = BTNR_7125_Bandwidth_e7MHz;
        break;
    case 6:
        tnrSettings.Bandwidth = BTNR_7125_Bandwidth_e6MHz;
        break;
    case 5:
        tnrSettings.Bandwidth = BTNR_7125_Bandwidth_e5MHz;
        break;
    /* default intentionally omitted to generate warnings if needed */
    }
    tnrSettings.Freq = pSettings->frequency;

#ifdef NEXUS_POWER_MANAGEMENT
		{
            BTNR_PowerSaverSettings settings;
            settings.enable = false;
            errCode = BTNR_SetPowerSaver(pDevice->tnrHandle,&settings);
            if (errCode!=BERR_SUCCESS) { return BERR_TRACE(errCode);}
		}
#endif

    errCode = BTNR_7125_Tune(pDevice->tnrHandle, &tnrSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

/***************************************************************************
 * Module callback functions for tuning
 ***************************************************************************/
static void NEXUS_Tuner_P_7125_Close(
    void *pDev
    )
{
    NEXUS_7125tnr *pDevice;

    pDevice = (NEXUS_7125tnr *)pDev;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7125tnr);

    NEXUS_UnregisterEvent(pDevice->isrEventCallback);

    BTNR_Close(pDevice->tnrHandle);
    BDBG_OBJECT_DESTROY(pDevice, NEXUS_7125tnr);
    BKNI_Free(pDevice);
}

void NEXUS_Tuner_Get7125Settings(
    NEXUS_TunerHandle handle,
    NEXUS_7125TunerSettings *pSettings  /* [out] */
    )
{
    NEXUS_7125tnr *pDevice;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pSettings);

    pDevice = handle->pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7125tnr);

    *pSettings = pDevice->settings;
}

NEXUS_Error NEXUS_Tuner_Set7125Settings(
    NEXUS_TunerHandle handle,
    const NEXUS_7125TunerSettings *pSettings
    )
{
    NEXUS_7125tnr *pDevice;
    NEXUS_Error errCode = BERR_SUCCESS;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pSettings);

    pDevice = handle->pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7125tnr);
    
    if ( pSettings->rfBypassEnabled != pDevice->settings.rfBypassEnabled )
    {
        errCode = BTNR_7125_SetRFByPassMode(pDevice->tnrHandle, pSettings->rfBypassEnabled);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        pDevice->settings.rfBypassEnabled = pSettings->rfBypassEnabled;
    }

    return errCode;
}

void NEXUS_Tuner_Get7125Status(
    NEXUS_TunerHandle handle,
    NEXUS_7125TunerStatus *pStatus  /* [out] */
    )
{
    NEXUS_7125tnr *pDevice;
    BTNR_7125_TunerStatus tnrStatus;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pStatus);

    pDevice = handle->pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7125tnr);

    (void)BTNR_7125_GetTunerStatus(pDevice->tnrHandle, &tnrStatus);
    pStatus->rfAgcLevel = tnrStatus.rfAgcVal;
	pStatus->ifAgcLevel = tnrStatus.ifAgcVal;
	pStatus->lnaAgcLevel = tnrStatus.lnaAgcVal;
	pStatus->dpmLvel = tnrStatus.dpmLvel;
}

NEXUS_Error NEXUS_Tuner_P_Install7125AdsInterrupt(
    NEXUS_TunerHandle handle,
    BTNR_7125_AdsInterruptCallback callback,
    void *pParam1, 
    int param2
    )
{    
    NEXUS_7125tnr *pDevice;
    NEXUS_Error errCode;

    BDBG_ASSERT(NULL != handle);

    pDevice = handle->pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7125tnr);

    errCode = BTNR_7125_InstallAdsInterruptCallback(pDevice->tnrHandle, callback, pParam1, param2);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
}

void NEXUS_Tuner_P_Remove7125AdsInterrupt(
    NEXUS_TunerHandle handle
    )
{
    NEXUS_7125tnr *pDevice;

    BDBG_ASSERT(NULL != handle);

    pDevice = handle->pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7125tnr);
    
    (void)BTNR_7125_RemoveAdsInterruptCallback(pDevice->tnrHandle);
}

/***************************************************************************
Summary:
    Untune a BCM7125 tuner (Power it down).
 ***************************************************************************/
NEXUS_Error NEXUS_Tuner_Untune7125(
    NEXUS_TunerHandle handle
    )
{
#ifdef NEXUS_POWER_MANAGEMENT
    NEXUS_7125tnr *pDevice;
    BERR_Code rc;
    BTNR_PowerSaverSettings settings;

    BDBG_ASSERT(NULL != handle);

    pDevice = handle->pDeviceHandle;
    settings.enable = true;
    BDBG_MSG(("%s() call BTNR_SetPowerSaver()",__FUNCTION__));
    rc = BTNR_SetPowerSaver(pDevice->tnrHandle,&settings);
    if (rc) {rc = BERR_TRACE(rc);}
    return (NEXUS_Error) rc;
#else
    BSTD_UNUSED(handle);
    return NEXUS_SUCCESS;
#endif
}

/***************************************************************************
Summary:
    Pass the LNA gain value to the tuner.
 ***************************************************************************/
NEXUS_Error NEXUS_Tuner_SetTunerConfigSettings7125(
    NEXUS_TunerHandle handle,
    const NEXUS_7125ConfigSettings *pConfigSettings
    )
{
    NEXUS_7125tnr *pDevice;
    BERR_Code rc;

    BDBG_ASSERT(NULL != handle);

    pDevice = handle->pDeviceHandle;
    {
		unsigned agcValue = pConfigSettings->agcValue;
		if (pConfigSettings->gainBoostEnabled) {
			agcValue |= BTNR_7125_LNA_GAIN_BOOST_ON;
		}
		if (pConfigSettings->superBoostEnabled) {
			agcValue |= BTNR_7125_LNA_SUPERBOOST_ON;
		}
		if (pConfigSettings->tiltEnabled) {
			agcValue |= BTNR_7125_LNA_TILT_ON;
		}
		BDBG_MSG(("%s(%u) setting agcValue = 0x%04x",__FUNCTION__,pConfigSettings->agcValue,agcValue));

        rc = BTNR_SetTunerAgcRegVal(pDevice->tnrHandle, 0, &agcValue);
		if (rc) {rc = BERR_TRACE(rc);}
		return (NEXUS_Error) rc;
    }
}

