/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *
 * Module Description:
 *
 ***************************************************************************/

#include "bhdm.h"
#include "bhdm_priv.h"

#if BHDM_CONFIG_RECEIVER_SENSE_SUPPORT
static void BHDM_P_GetReceiverSense_isr(const BHDM_Handle hHDMI, bool *RxSense) ;
#endif



BDBG_MODULE(BHDM_PWR) ;

#if BHDM_CONFIG_RECEIVER_SENSE_SUPPORT
static void BHDM_P_GetReceiverSense_isr(const BHDM_Handle hHDMI, bool *RxSense)
{
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset ;
	uint8_t ReceiverSense ;
	uint8_t Ch0, Ch1, Ch2, Clock ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	/* if TMDS lines are temporarily disabled due to writing the Rx BitClockRatio bit */
	/* return that TV is still powered */
	if (hHDMI->TmdsDisabledForBitClockRatioChange)
	{
		*RxSense = true ;
		return ;
	}


#if BHDM_CONFIG_65NM_SUPPORT
	Register = BREG_Read32(hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_STATUS + ulOffset) ;
	Clock = BCHP_GET_FIELD_DATA(Register, HDMI_TX_PHY_HDMI_TX_PHY_STATUS, STATUS_RSEN_CK) ;
	Ch2   = BCHP_GET_FIELD_DATA(Register, HDMI_TX_PHY_HDMI_TX_PHY_STATUS, STATUS_RSEN_2) ;
	Ch1   = BCHP_GET_FIELD_DATA(Register, HDMI_TX_PHY_HDMI_TX_PHY_STATUS, STATUS_RSEN_1) ;
	Ch0   = BCHP_GET_FIELD_DATA(Register, HDMI_TX_PHY_HDMI_TX_PHY_STATUS, STATUS_RSEN_0) ;
#else
	Register = BREG_Read32(hRegister, BCHP_HDMI_TX_PHY_STATUS + ulOffset) ;
#if BHDM_CONFIG_SWAP_DEFAULT_PHY_CHANNELS
	Ch2 = BCHP_GET_FIELD_DATA(Register, HDMI_TX_PHY_STATUS, RSEN_CK) ;
	Clock   = BCHP_GET_FIELD_DATA(Register, HDMI_TX_PHY_STATUS, RSEN_2) ;
#else
	Clock = BCHP_GET_FIELD_DATA(Register, HDMI_TX_PHY_STATUS, RSEN_CK) ;
	Ch2   = BCHP_GET_FIELD_DATA(Register, HDMI_TX_PHY_STATUS, RSEN_2) ;
#endif

	Ch1   = BCHP_GET_FIELD_DATA(Register, HDMI_TX_PHY_STATUS, RSEN_1) ;
	Ch0   = BCHP_GET_FIELD_DATA(Register, HDMI_TX_PHY_STATUS, RSEN_0) ;
#endif

	ReceiverSense = Clock + Ch2 + Ch1 + Ch0 ;

	/* If all ReceiverSense lines are all enabled */
	if (ReceiverSense == 4)
	{
		ReceiverSense = 0x0F;
		*RxSense = true ;
	}
	/* If only the Clock is enabled, consider all TMDS lines enabled */
	else if (Clock)
	{
		ReceiverSense = 0x08;
		*RxSense = true ;
	}
	else
	{
		*RxSense = false ;
	}

	if (*RxSense != hHDMI->rxSensePowerDetected)
	{
		/* notify of changes only if clock or clock/data are enabled
		     i.e. not in  standby mode
		*/
		if ((hHDMI->DeviceStatus.tmds.dataEnabled)
		|| (hHDMI->DeviceStatus.tmds.clockEnabled))
		{
			BDBG_MSG(("Clk %d Data %d  RxSense change %d",
				hHDMI->DeviceStatus.tmds.clockEnabled,
				hHDMI->DeviceStatus.tmds.dataEnabled, *RxSense)) ;
		}

		hHDMI->rxSensePowerDetected = *RxSense ;
		hHDMI->MonitorStatus.NumRxSenseChanges++ ;
	}

#if BHDM_CONFIG_MHL_SUPPORT
	if (hHDMI->bMhlMode && *RxSense == false)
	{
		if (hHDMI->hMhl->hostState == BHDM_P_Mhl_HostCpuState_eActive)
		{
			BHDM_P_DisableDisplay_isr(hHDMI);

			/* Go into S3 Mode */
			BHDM_P_Mhl_HandleHandoverToMpm_isr(hHDMI->hMhl);
		}
	}
#endif

	return;
}
#endif



/******************************************************************************
Summary:
	Get power status of attached receiver
*******************************************************************************/
#define BHDM_RECEIVER_SENSE_ON 0x0F
#define BHDM_RECEIVER_SENSE_OFF 0x00
BERR_Code BHDM_GetReceiverSense(
	const BHDM_Handle hHDMI, uint8_t *ReceiverSense)
{
	BERR_Code rc = BERR_SUCCESS ;
	uint8_t DeviceAttached;
	bool RxSense = false ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	/* check if a Receiver is Attached */
	rc = BHDM_RxDeviceAttached(hHDMI, &DeviceAttached);
	(void) BERR_TRACE(rc) ;

	if (!DeviceAttached)
	{
#if BHDM_CONFIG_DEBUG_RSEN
		BDBG_WRN(("Tx%d: No DVI/HDMI Device Attached; Unable to detect RxSense: ", hHDMI->eCoreId)) ;
#endif
		*ReceiverSense = false ;
		hHDMI->rxSensePowerDetected	= false ;
		rc = BERR_TRACE(BHDM_NO_RX_DEVICE) ;
		goto done ;
	}

	if ((!hHDMI->DeviceStatus.tmds.clockEnabled)
	&& (!hHDMI->TmdsDisabledForBitClockRatioChange))
	{
#if BHDM_CONFIG_DEBUG_RSEN
		BDBG_WRN(("Tx%d: Detect RxSense: Required HDMI (TMDS) Clock is disabled", hHDMI->eCoreId)) ;
#endif
		*ReceiverSense = false ;
		hHDMI->rxSensePowerDetected	= false ;
		goto done ;
	}

#if BHDM_CONFIG_RECEIVER_SENSE_SUPPORT

	/* interrupt should always update the latest updated RSEN value */
	BKNI_EnterCriticalSection() ;
		BHDM_P_GetReceiverSense_isr(hHDMI, &RxSense) ;
		*ReceiverSense = hHDMI->rxSensePowerDetected ?
			BHDM_RECEIVER_SENSE_ON : BHDM_RECEIVER_SENSE_OFF ;
	BKNI_LeaveCriticalSection() ;
#endif

done:
	return rc ;
}


void BHDM_GetDefaultStandbySettings(
	BHDM_StandbySettings *pSettings
	)
{
	pSettings->bEnableWakeup = true;
}


/******************************************************************************
Summary: Enter standby mode
*******************************************************************************/
BERR_Code BHDM_Standby(
	const BHDM_Handle hHDMI, /* [in] HDMI Handle */
	const BHDM_StandbySettings *pSettings
	)
{
	BHDM_StandbySettings settings;
	BERR_Code rc = BERR_SUCCESS ;

	BDBG_ENTER(BHDM_Standby) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	if (hHDMI->standby) {
		BDBG_ERR(("Tx%d: Already in standby", hHDMI->eCoreId));
		rc = BERR_TRACE(BERR_UNKNOWN) ;
		goto done ;
	}

	if (pSettings==NULL) {
		BHDM_GetDefaultStandbySettings(&settings);
	}
	else {
		settings = *pSettings;
	}


#if BHDM_CONFIG_HAS_HDCP22
    BHDM_AUTO_I2C_P_DisableInterrupts(hHDMI);
#endif
	BHDM_P_DisableInterrupts(hHDMI) ;

	if (!settings.bEnableWakeup) {
	    /* if wakeup from CEC is not needed, then it doesn't need to be powered */
	    /* Used only for legacy 65nm. not required for 28nm and 40nm */
#ifdef BCHP_PWR_RESOURCE_HDMI_TX_CEC
		BCHP_PWR_ReleaseResource(hHDMI->hChip, BCHP_PWR_RESOURCE_HDMI_TX_CEC);
#endif
	}

	hHDMI->enableWakeup = settings.bEnableWakeup;
	hHDMI->standby = true;
	rc = BHDM_DisableDisplay(hHDMI); /* this will release the HDMI_TX_TMDS resource */

	BHDM_P_FreeTimers(hHDMI) ;


#if BCHP_PWR_RESOURCE_HDMI_TX_CLK || BCHP_PWR_RESOURCE_HDMI_TX_1_CLK
	BCHP_PWR_ReleaseResource(hHDMI->hChip, hHDMI->clkPwrResource[hHDMI->eCoreId]) ;
#endif


done:
	BDBG_LEAVE(BHDM_Standby) ;
	return rc ;
}


/******************************************************************************
Summary: Resume standby mode
*******************************************************************************/
BERR_Code BHDM_Resume(
	const BHDM_Handle hHDMI /* [in] HDMI Handle */
	)
{
	BERR_Code rc = BERR_SUCCESS ;
	uint8_t RxDeviceAttached ;

	BDBG_ENTER(BHDM_Resume) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	if (!hHDMI->standby)
	{
		BDBG_ERR(("Tx%d: Not in standby", hHDMI->eCoreId));
		rc = BERR_TRACE(BERR_UNKNOWN) ;
		goto done ;
	}

#if BCHP_PWR_RESOURCE_HDMI_TX_CLK || BCHP_PWR_RESOURCE_HDMI_TX_1_CLK
	BCHP_PWR_AcquireResource(hHDMI->hChip, hHDMI->clkPwrResource[hHDMI->eCoreId]);
#endif

	if (!hHDMI->enableWakeup) {
	    /* Used only for legacy 65nm. not required for 28nm and 40nm */
#ifdef BCHP_PWR_RESOURCE_HDMI_TX_CEC
	    BCHP_PWR_AcquireResource(hHDMI->hChip, BCHP_PWR_RESOURCE_HDMI_TX_CEC);
#endif
	}

#if BHDM_CONFIG_BTMR_SUPPORT
	/* re-create a hot plug countdown timer */
	BHDM_P_AllocateTimers(hHDMI) ;
#endif


	hHDMI->standby = false;

	BKNI_EnterCriticalSection() ;
		BHDM_P_RxDeviceAttached_isr(hHDMI, &RxDeviceAttached) ;
	BKNI_LeaveCriticalSection() ;


	BKNI_SetEvent(hHDMI->BHDM_EventHotPlug);
	hHDMI->RxDeviceAttached = RxDeviceAttached;

	BHDM_P_EnableInterrupts(hHDMI) ;
#if BHDM_CONFIG_HAS_HDCP22
	BKNI_EnterCriticalSection() ;
		BHDM_P_ResetHDCPI2C_isr(hHDMI);
	BKNI_LeaveCriticalSection() ;

	BHDM_AUTO_I2C_P_EnableInterrupts(hHDMI);
#endif

	BDBG_LEAVE(BHDM_Resume) ;

done:
	return rc ;
}


#if !B_REFSW_MINIMAL
/******************************************************************************
Summary: install Receiver Sense Change Callback to notify of RxSense power detect changes
*******************************************************************************/
BERR_Code BHDM_InstallRxSenseChangeCallback(
	const BHDM_Handle hHDMI,			/* [in] HDMI Handle */
	const BHDM_CallbackFunc pfCallback_isr, /* [in] cb for Receiver Sense changes */
	void *pvParm1, /* [in] the first argument (void *) passed to the callback function */
	int iParm2)    /* [in] the second argument(int) passed to the callback function */
{
#if BHDM_CONFIG_RECEIVER_SENSE_SUPPORT
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BHDM_InstallRxSenseChangeCallback) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	/* Check if this is a valid function */
	if( pfCallback_isr == NULL )
	{
		rc = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto done ;
	}

	BKNI_EnterCriticalSection() ;
		hHDMI->pfRxSenseChangeCallback = pfCallback_isr ;
		hHDMI->pvRxSenseChangeParm1 = pvParm1 ;
		hHDMI->iRxSenseChangeParm2 = iParm2 ;
	BKNI_LeaveCriticalSection() ;

done:
	BDBG_LEAVE(BHDM_InstallRxSenseChangeCallback);
	return rc ;
#else
	BSTD_UNUSED( hHDMI) ;
	BSTD_UNUSED(pfCallback_isr) ;
	BSTD_UNUSED(pvParm1) ;
	BSTD_UNUSED(iParm2)  ;

	return BERR_SUCCESS ;

#endif
}


/******************************************************************************
Summary: Uninstall RxSense Change Callback
*******************************************************************************/
BERR_Code BHDM_UnInstallRxSenseChangeCallback(
	const BHDM_Handle hHDMI,						 /* [in] HDMI Handle */
	const BHDM_CallbackFunc pfCallback_isr) /* [in] cb for format changes */
{
#if BHDM_CONFIG_RECEIVER_SENSE_SUPPORT
	BERR_Code rc = BERR_SUCCESS ;
	BSTD_UNUSED(pfCallback_isr) ;

	BDBG_ENTER(BHDM_UnInstallRxSenseChangeCallback) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BKNI_EnterCriticalSection() ;
		hHDMI->pfRxSenseChangeCallback = (BHDM_CallbackFunc) NULL ;
	BKNI_LeaveCriticalSection();

	BDBG_LEAVE(BHDM_UnInstallRxSenseChangeCallback) ;
	return rc;
#else
	BSTD_UNUSED( hHDMI) ;
	BSTD_UNUSED(pfCallback_isr) ;
	return BERR_SUCCESS ;
#endif
}
#endif
