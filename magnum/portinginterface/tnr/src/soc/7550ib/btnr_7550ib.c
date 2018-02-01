/******************************************************************************
* Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*
* This program is the proprietary software of Broadcom and/or its licensors,
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
*****************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "btnr.h"
#include "bmem.h"
#include "btnr_priv.h"
#include "btnr_7550ib.h"
#include "btnr_75xxib_priv.h"
#include "bchp_ds_qdsafe_0.h"
#include "bchp_thd_core.h"
#include "btnr_7550ib_tune.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_gfd_1.h"

BDBG_MODULE(btnr_7550ib);
#define	DEV_MAGIC_ID			((BERR_TNR_ID<<16) | 0xFACE)


/******************************************************************************
  BTNR_7550_P_TimerFunc_isr()
 ******************************************************************************/
BERR_Code BTNR_7550_P_TimerFunc_isr(void *myParam1, int myParam2)
{
  BTNR_Handle hDev = (BTNR_Handle)myParam1;
  BTNR_P_75xx_Handle *p75xx = (BTNR_P_75xx_Handle *)(hDev->hDevImpl);
  BSTD_UNUSED(myParam2);
  BKNI_SetEvent(p75xx->hInterruptEvent); 
  return BERR_SUCCESS;
}



/******************************************************************************
  BTNR_7550_P_GetInterruptEventHandle()
 ******************************************************************************/
BERR_Code BTNR_7550_GetInterruptEventHandle(BTNR_Handle h, BKNI_EventHandle* hEvent)
{
    *hEvent = ((BTNR_P_75xx_Handle *)(h->hDevImpl))->hInterruptEvent;
    return BERR_SUCCESS;
}

/******************************************************************************
  BTNR_7550_P_ProcessInterruptEvent()
 ******************************************************************************/
BERR_Code BTNR_7550_ProcessInterruptEvent(BTNR_Handle hDev)
{
  BTNR_P_75xx_Handle *p75xx = (BTNR_P_75xx_Handle *)(hDev->hDevImpl);
  BTNR_7550_TuneSettings pInitSettings; 

  if (p75xx->pTnrModeData->BBSNexusConnectMode & TNR_NexusStatusMode_EnableStatusForNexus)
  {
	BTNR_7550_P_Get_Status_TNR_Core0(p75xx);
	p75xx->pTnrModeData->BBSNexusConnectMode &= (~TNR_NexusStatusMode_EnableStatusForNexus);
  }
 
  if (p75xx->pTnrModeData->BBSConnectMode & TNR_TuneStartMode_Manual)
  {
	p75xx->pTnrModeData->BBSConnectMode &= (~TNR_TuneStartMode_Manual);
	pInitSettings.Bandwidth = p75xx->pTnrModeData->TunerBw;
	pInitSettings.AcquisitionMode = p75xx->pTnrModeData->TunerAcquisitionMode;
	pInitSettings.Freq = p75xx->pTnrModeData->TunerFreq;
	pInitSettings.TunerSetup = p75xx->pTnrModeData->TunerFreq;
	BTNR_7550_Tune(hDev, &pInitSettings);
  }
  else if (p75xx->pTnrModeData->BBSConnectMode & TNR_TuneStartMode_DPMStatus)
  {
	p75xx->pTnrModeData->BBSConnectMode &= (~TNR_TuneStartMode_DPMStatus);
	BTNR_7550_P_Get_DPM(p75xx);
  }
  else if (p75xx->pTnrModeData->BBSConnectMode & TNR_TuneStartMode_ResetStatus)
  {
	p75xx->pTnrModeData->BBSConnectMode &= (~TNR_TuneStartMode_ResetStatus);
	BTNR_7550_P_Reset_Status_TNR_Core0(p75xx);
  }

  return BERR_SUCCESS;
}

/*******************************************************************************/
BERR_Code BTNR_7550_Open(
    BTNR_Handle *phDev,                 /* [output] Returns handle */
	BREG_Handle hRegister,				/* Register handle */
    BTNR_7550_Settings *pSettings		/* [Input] settings structure */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BTNR_75xx_Handle h75xxDev;
    BTNR_P_75xx_Settings *pTnrImplData;
	BTMR_Settings sTimerSettings;
    BTNR_Handle hDev;
	uint32_t BufSrc;
	uint32_t reg;
	static bool first_tune = false;

    BDBG_ENTER(BTNR_7550_Open);
    BSTD_UNUSED( pSettings );
    
    hDev = NULL;
    /* Alloc memory from the system heap */
    h75xxDev = (BTNR_75xx_Handle) BKNI_Malloc( sizeof( BTNR_P_75xx_Handle ) );
    if( h75xxDev == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BTNR_7550_Open: BKNI_malloc() failed"));
        goto done;
    }
    BKNI_Memset( h75xxDev, 0x00, sizeof( BTNR_P_75xx_Handle ) );

    hDev = (BTNR_Handle) BKNI_Malloc( sizeof( BTNR_P_Handle ) );
    if( hDev == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BTNR_7550_Open: BKNI_malloc() failed"));
        BKNI_Free( h75xxDev );
        goto done;
    }
    BKNI_Memset( hDev, 0x00, sizeof( BTNR_P_Handle ) );

	h75xxDev->pTnrModeData = (BTNR_P_75xx_AcquireSettings_t *)BMEM_AllocAligned(pSettings->hHeap, sizeof(BTNR_P_75xx_AcquireSettings_t), 0, 0 );
	BKNI_Memset( h75xxDev->pTnrModeData, 0x00, sizeof( BTNR_P_75xx_AcquireSettings_t ) );
	BMEM_ConvertAddressToOffset(pSettings->hHeap, h75xxDev->pTnrModeData, &BufSrc );
	BREG_Write32(hRegister, BCHP_GFD_1_SCRATCH0, BufSrc);
	h75xxDev->pTnrStatus = (BTNR_75xx_P_Status_t *)BMEM_AllocAligned(pSettings->hHeap, sizeof(BTNR_75xx_P_Status_t), 0, 0 );
	BKNI_Memset( h75xxDev->pTnrStatus, 0x00, sizeof( BTNR_75xx_P_Status_t ) );
	BMEM_ConvertAddressToOffset(pSettings->hHeap, h75xxDev->pTnrStatus, &BufSrc );
	h75xxDev->pTnrModeData->StartSturctureAddress = BufSrc;
	h75xxDev->hI2cTHD = pSettings->hI2cTHD;
	h75xxDev->hI2cADS = pSettings->hI2cADS;
    h75xxDev->magicId = DEV_MAGIC_ID;
	h75xxDev->hHeap = pSettings->hHeap;
    pTnrImplData = &h75xxDev->settings;
    pTnrImplData->rfFreq = 0;
    pTnrImplData->tunerMode = BTNR_TunerMode_eLast;
	h75xxDev->hRegister = hRegister;

    hDev->hDevImpl = (void *) h75xxDev;
    hDev->magicId = DEV_MAGIC_ID;
    hDev->pSetRfFreq = (BTNR_SetRfFreqFunc) BTNR_75xx_SetRfFreq;
    hDev->pGetRfFreq = (BTNR_GetRfFreqFunc) BTNR_75xx_GetRfFreq;
    hDev->pGetAgcRegVal = (BTNR_GetAgcRegValFunc) BTNR_P_75xx_GetAgcRegVal;
    hDev->pSetAgcRegVal = (BTNR_SetAgcRegValFunc) NULL;
    hDev->pGetInfo = (BTNR_GetInfoFunc) BTNR_75xx_GetInfo;
    hDev->pClose = (BTNR_CloseFunc) BTNR_75xx_Close;
    hDev->pGetPowerSaver = (BTNR_GetPowerSaverFunc) BTNR_75xx_GetPowerSaver;
    hDev->pSetPowerSaver = (BTNR_SetPowerSaverFunc) BTNR_75xx_SetPowerSaver;

	BKNI_CreateEvent(&(h75xxDev->hInterruptEvent));

	    /* Create timer for status lock check */
    BTMR_GetDefaultTimerSettings(&sTimerSettings);
    sTimerSettings.type = BTMR_Type_ePeriodic;
    sTimerSettings.cb_isr = (BTMR_CallbackFunc)BTNR_7550_P_TimerFunc_isr;
    sTimerSettings.pParm1 = (void*)hDev;
    sTimerSettings.parm2 = 0;
    sTimerSettings.exclusive = false;

    retCode = BTMR_CreateTimer (pSettings->hTmr, &h75xxDev->hTimer, &sTimerSettings);
    if ( retCode != BERR_SUCCESS )
    {
        BDBG_ERR(("BTHD_Open: Create Timer Failed"));
        retCode = BERR_TRACE(retCode);
        goto done;
    }	

	if (first_tune == false)
	{
		reg = BREG_Read32(h75xxDev->hRegister, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14);
		reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, sgpio_02) |
				BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, sgpio_03));
		reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, sgpio_02, 1) | 
				BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, sgpio_03, 1);
		BREG_Write32 (h75xxDev->hRegister, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14, reg);
#ifdef REF_7550_BOARD
		h75xxDev->pTnrModeData->BBS_Cable_LNA_Address = CABLE_LNA_BASE_ADDRESS;
		BTNR_7550_P_Probe_Second_LNA(h75xxDev); 
#else
		BTNR_7550_P_Probe_First_LNA(h75xxDev);
#endif
		first_tune = true;
	}
	
done:
    *phDev = hDev;
    BDBG_LEAVE(BTNR_7550_Open);
    return( retCode );
}

BERR_Code BTNR_7550_GetDefaultSettings(
    BTNR_7550_Settings *pDefSettings  /* [out] Returns default setting */
    )
{
    BDBG_ASSERT(NULL != pDefSettings);

    BKNI_Memset(pDefSettings, 0, sizeof(*pDefSettings));

    return BERR_SUCCESS;
}

/********************************************
 Tuner default settings 
 *******************************************/


static const BTNR_7550_TuneSettings defTuneParams = 
{
    BTNR_7550_Bandwidth_e8MHz,		/* Bandwidth */
	BTNR_7550_Standard_eDVBT,       /* Inband mode */
    666000000,                      /* Tuner Frequency */
	BTNR_7550_Setup_eTerrestrial,			/* Tuner Mode */
};

/***************************************************************************
Summary:
	BTNR_7550_GetDefaultTuneSettings
****************************************************************************/
BERR_Code BTNR_7550_GetDefaultTuneSettings(BTNR_7550_TuneSettings *pSettings)
{
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = defTuneParams; 
	return BERR_SUCCESS;
}


/***************************************************************************
Summary:
	BTNR_7550_Tune
****************************************************************************/
BERR_Code BTNR_7550_Tune(
    BTNR_Handle hTnr,
    const BTNR_7550_TuneSettings *pSettings
    )
{

	BTNR_P_75xx_Handle *h75Dev=((BTNR_P_75xx_Handle *)(hTnr->hDevImpl));
	BTNR_7550_TuneSettings pInitSettings; 
	static BTNR_7550_Standard TunerAcquisitionMode = 0;
	static BTNR_7550_Bandwidth TunerBw = 0;
	static uint32_t TunerFreq = 0;
	static uint32_t TunerSetup = 0;

	BDBG_MSG(("BTNR_7550_Tune"));


	
	if (h75Dev->pTnrModeData->NexusParamsDisable  & TNR_NexusParamsMode_eDisable)
	{
		pInitSettings.Bandwidth = h75Dev->pTnrModeData->TunerBw;
		pInitSettings.AcquisitionMode = h75Dev->pTnrModeData->TunerAcquisitionMode;
		pInitSettings.Freq = h75Dev->pTnrModeData->TunerFreq;
		pInitSettings.TunerSetup = h75Dev->pTnrModeData->TunerSetup;
	}
	else
	{
		pInitSettings.Bandwidth = pSettings->Bandwidth;
		pInitSettings.AcquisitionMode = pSettings->AcquisitionMode;
		pInitSettings.Freq = pSettings->Freq;
		pInitSettings.TunerSetup =pSettings->TunerSetup;
	}
	if ((pInitSettings.AcquisitionMode != TunerAcquisitionMode) || (pInitSettings.TunerSetup != TunerSetup))
	{
		h75Dev->pTnrModeData->TunerBw = pInitSettings.Bandwidth;
		h75Dev->pTnrModeData->TunerAcquisitionMode = pInitSettings.AcquisitionMode;
		h75Dev->pTnrModeData->TunerFreq = pInitSettings.Freq;
		h75Dev->pTnrModeData->TunerSetup = pInitSettings.TunerSetup;
		h75Dev->pTnrModeData->TunerAcquisitionType = BTNR_7550_TunerAcquisitionType_eInitTune;
		BTNR_7550_P_Tune_TNR_Core0(h75Dev);
	}
	else if ((pInitSettings.Bandwidth != TunerBw) || (pInitSettings.Freq != TunerFreq))
	{
		h75Dev->pTnrModeData->TunerBw = pInitSettings.Bandwidth;
		h75Dev->pTnrModeData->TunerFreq = pInitSettings.Freq;
		h75Dev->pTnrModeData->TunerAcquisitionType = BTNR_7550_TunerAcquisitionType_eTune;
		BTNR_7550_P_Tune_TNR_Core0(h75Dev);
	}
	else
	{
		h75Dev->pTnrModeData->TunerAcquisitionType = BTNR_7550_TunerAcquisitionType_eMiniTune;
		BTNR_7550_P_Tune_TNR_Core0(h75Dev);
	}
	TunerAcquisitionMode = pInitSettings.AcquisitionMode;
	TunerFreq = pInitSettings.Freq;
	TunerBw = pInitSettings.Bandwidth;
	TunerSetup = pInitSettings.TunerSetup;

	if (h75Dev->pTnrModeData->TunerAcquisitionType != BTNR_7550_TunerAcquisitionType_eMiniTune)
	{
		BTMR_StopTimer(h75Dev->hTimer);
		BTMR_StartTimer(h75Dev->hTimer, 500000);   /* the timer is in Micro second */
		BDBG_MSG(("BTNR_7550_Tune"));
	}

	return BERR_SUCCESS;
}


/***************************************************************************
Summary:
	BTNR_7550_SetRFByPassMode
****************************************************************************/
BERR_Code BTNR_7550_SetRFByPassMode(
    BTNR_Handle hTnr,
    bool mode
    )
{

	BTNR_P_75xx_Handle *h75Dev=((BTNR_P_75xx_Handle *)(hTnr->hDevImpl));
	h75Dev->pTnrModeData->RFbypassMode = mode;
	BTNR_7550_P_Set_Rf_ByPass_mode(h75Dev, mode);
	return BERR_SUCCESS;
}

/***************************************************************************
Summary:
	BTNR_7550_GetRFByPassMode
****************************************************************************/
BERR_Code BTNR_7550_GetRFByPassMode(
    BTNR_Handle hTnr,
    bool *mode
    )
{

	BTNR_P_75xx_Handle *h75Dev=((BTNR_P_75xx_Handle *)(hTnr->hDevImpl));
	*mode = h75Dev->pTnrModeData->RFbypassMode;
	return BERR_SUCCESS;
}

/***************************************************************************
Summary:
	BTNR_7550_GetTunerStatus
****************************************************************************/
BERR_Code BTNR_7550_GetTunerStatus(
    BTNR_Handle hTnr,
    BTNR_7550_TunerStatus *pStatus
    )
{

	BTNR_P_75xx_Handle *h75Dev=((BTNR_P_75xx_Handle *)(hTnr->hDevImpl));
	BTNR_7550_P_Get_Status_TNR_Core0(h75Dev);
	pStatus->rfAgcVal = (int32_t)(h75Dev->pTnrStatus->RF_AGC_LEVEL);
	pStatus->ifAgcVal = (int32_t)(h75Dev->pTnrStatus->IF_AGC_LEVEL);
	pStatus->lnaAgcVal = (int32_t)(h75Dev->pTnrStatus->LNA_AGC_LEVEL);
	pStatus->agcStatus = (int32_t)(h75Dev->pTnrStatus->Tuner_AGC_Status);
	if ((h75Dev->pTnrModeData->TunerAcquisitionMode == BTNR_7550_Standard_eDVBT) || (h75Dev->pTnrModeData->TunerAcquisitionMode == BTNR_7550_Standard_eISDBT))
		pStatus->signalStrength = (int32_t)(h75Dev->pTnrStatus->InputPower_256db);
	if ((h75Dev->pTnrModeData->TunerAcquisitionMode == BTNR_7550_Standard_eQAM) && (h75Dev->pTnrModeData->TunerSetup == BTNR_7550_Setup_eCable))
	{	
		pStatus->dpmLvel = (int32_t)(h75Dev->pTnrStatus->DPM_Gain_256dbmV);
	}
	else
		pStatus->dpmLvel = 0;

	return BERR_SUCCESS;
}

/***************************************************************************
Summary:
	BTNR_7550_GetDPM
****************************************************************************/
BERR_Code BTNR_7550_EnableDPM(
    BTNR_Handle hTnr
    )
{
	BTNR_P_75xx_Handle *h75Dev=((BTNR_P_75xx_Handle *)(hTnr->hDevImpl));
	BTNR_7550_P_Get_DPM(h75Dev);
	return BERR_SUCCESS;
}

/***************************************************************************
BTNR_InstallDeviceInterruptCallback: Used to enable and install an application
                                     callback for Device relevant Interrupt.
****************************************************************************/
BERR_Code BTNR_7550_InstallAdsInterruptCallback(BTNR_Handle						hDev,
												BTNR_7550_AdsInterruptCallback  fCallBack_isr,
												void                            *pParam1,
												int					            param2
												)
{
   BTNR_P_75xx_Handle *p75xx = (BTNR_P_75xx_Handle *)(hDev->hDevImpl);
   BTNR_P_CallbackInfo *callback;

   BDBG_ENTER(BTNR_InstallDeviceInterruptCallback);
   callback =&(p75xx->InterruptCallbackInfo);

   BKNI_EnterCriticalSection();
   callback->func = (BTNR_7550_AdsInterruptCallback)fCallBack_isr;
   callback->pParm1 = pParam1;
   callback->Parm2 = param2;
   BKNI_LeaveCriticalSection();
   

   BDBG_LEAVE(BTNR_InstallDeviceInterruptCallback);

   return BERR_TRACE(BERR_SUCCESS);
}


/******************************************************************************
BTNR_75xx_UnInstallInterruptCallback()
******************************************************************************/ 
BERR_Code BTNR_7550_RemoveAdsInterruptCallback(
    BTNR_Handle handle  /* [in] BTNR handle */
)
{
    BTNR_P_CallbackInfo *callback;
    BTNR_P_75xx_Handle *p75xx = (BTNR_P_75xx_Handle *)(handle->hDevImpl);

    BDBG_ASSERT(handle);
    
    callback = &(p75xx->InterruptCallbackInfo);

    callback->func = NULL;
    callback->pParm1 = NULL;
    callback->Parm2 = (int)NULL;

    return BERR_TRACE(BERR_SUCCESS);
} 

/***************************************************************************
Summary:
	BTNR_7550_SetLNABoost
****************************************************************************/
BERR_Code BTNR_7550_SetLNABoost(
    BTNR_Handle hTnr,
    bool on
    )
{
    BTNR_P_75xx_Handle *h75Dev=((BTNR_P_75xx_Handle *)(hTnr->hDevImpl));

    if (h75Dev->pTnrModeData->TunerSetup == BTNR_7550_Setup_eCable)
        h75Dev->pTnrModeData->BBS_DS_LNA_Boost_On = (uint32_t)(on == true) ? BTNR_ON : BTNR_OFF;
    else if (h75Dev->pTnrModeData->TunerSetup == BTNR_7550_Setup_eTerrestrial)
        h75Dev->pTnrModeData->BBS_THD_LNA_Boost_On = (uint32_t)(on == true) ? BTNR_ON : BTNR_OFF;
    else
        BDBG_ERR(("Invalid TunerSetup setting!"));

    BTNR_7550_P_Set_LNA_Boost(h75Dev);
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
	BTNR_7550_SetLNATilt
****************************************************************************/
BERR_Code BTNR_7550_SetLNATilt(
    BTNR_Handle hTnr,
    bool on
    )
{
    BTNR_P_75xx_Handle *h75Dev=((BTNR_P_75xx_Handle *)(hTnr->hDevImpl));

    if (h75Dev->pTnrModeData->TunerSetup == BTNR_7550_Setup_eCable)
        h75Dev->pTnrModeData->BBS_DS_LNA_Tilt_On = (uint32_t)(on == true) ? BTNR_ON : BTNR_OFF;
    else if (h75Dev->pTnrModeData->TunerSetup == BTNR_7550_Setup_eTerrestrial)
        h75Dev->pTnrModeData->BBS_THD_LNA_Tilt_On = (uint32_t)(on == true) ? BTNR_ON : BTNR_OFF;
    else
        BDBG_ERR(("Invalid TunerSetup setting!"));

    BTNR_7550_P_Set_LNA_Tilt(h75Dev);
    return BERR_SUCCESS;
}

