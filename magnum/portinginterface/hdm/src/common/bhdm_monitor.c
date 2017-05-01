/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bhdm_monitor.h"

static const char Yes[] = "Yes" ;
static const char No[] = "No" ;

BDBG_MODULE(BHDM_MONITOR) ;


void BHDM_MONITOR_P_CreateTimers(BHDM_Handle hHDMI)
{
	BERR_Code rc ;

	/* create a format change countdown timer */
	BHDM_CHECK_RC(rc, BHDM_P_CreateTimer(hHDMI,
		&hHDMI->TimerFormatChange, BHDM_P_TIMER_eFormatDetection)) ;

	/* create a status change countdown timer */
	BHDM_CHECK_RC(rc, BHDM_P_CreateTimer(hHDMI,
		&hHDMI->TimerStatusMonitor, BHDM_P_TIMER_eMonitorStatus)) ;

	/* create a hot plug detect change timer */
	BHDM_CHECK_RC(rc, BHDM_P_CreateTimer(hHDMI,
		&hHDMI->TimerHotPlugChange, BHDM_P_TIMER_eHotPlugChange)) ;

done :
	(void) BERR_TRACE(rc) ;
}


void BHDM_MONITOR_P_DestroyTimers(BHDM_Handle hHDMI)
{
	BERR_Code rc ;

	BHDM_CHECK_RC(rc,BHDM_P_DestroyTimer(hHDMI, hHDMI->TimerFormatChange, BHDM_P_TIMER_eFormatDetection)) ;
	BHDM_CHECK_RC(rc,BHDM_P_DestroyTimer(hHDMI, hHDMI->TimerStatusMonitor, BHDM_P_TIMER_eMonitorStatus)) ;
	BHDM_CHECK_RC(rc,BHDM_P_DestroyTimer(hHDMI, hHDMI->TimerHotPlugChange, BHDM_P_TIMER_eHotPlugChange)) ;

done :
	(void) BERR_TRACE(rc) ;

}


void BHDM_MONITOR_P_FormatChanges_isr(const BHDM_Handle hHDMI)
{
#if BHDM_CONFIG_MONITOR_FORMAT_CHANGE_SECONDS
	uint32_t Register, FormatChangeDetected, ulOffset;
	BREG_Handle hRegister ;

	ulOffset = hHDMI->ulOffset ;
	hRegister = hHDMI->hRegister ;

	FormatChangeDetected =
		BREG_Read32(hRegister, BCHP_HDMI_FORMAT_DET_UPDATE_STATUS + ulOffset) ;

	if (FormatChangeDetected)
	{
		if (!hHDMI->TimerFormatInitialChangeReported)
		{
			/* skip reporting first format change warning; initial change expected after a format change */
			BDBG_MSG(("Tx%d: Skip reporting format change that is expected after a format change",
				hHDMI->eCoreId)) ;
			hHDMI->TimerFormatInitialChangeReported = true ;
		}
		else
		{
			uint8_t PllLock ;
			uint16_t PllStatus = 0 ;

			/* an error has been detected */
			hHDMI->MonitorStatus.UnstableFormatDetectedCounter++ ;

			/* unstable condition may be power related */
			/* check the status of the power or reset first */
#if BCHP_HDMI_TX_PHY_HDMI_TX_PHY_STATUS
			Register = BREG_Read32(hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL + ulOffset) ;
			if (Register)
			{
				BDBG_ERR(("Tx%d: Portion of HDMI Tx Core may be in Reset; Status 0x%x",
					hHDMI->eCoreId, Register)) ;
			}
#else
			Register = BREG_Read32(hRegister, BCHP_HDMI_TX_PHY_POWERDOWN_CTL + ulOffset) ;
			if (Register)
			{
				BDBG_ERR(("Tx%d: Portion of HDMI Tx Core may be powered down; Status: 0x%x",
					hHDMI->eCoreId, Register)) ;
			}
#endif


			/* check the pll status next */
#if BCHP_HDMI_TX_PHY_HDMI_TX_PHY_STATUS
			Register = BREG_Read32(hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_STATUS + ulOffset) ;
			PllLock =  BCHP_GET_FIELD_DATA(Register, HDMI_TX_PHY_HDMI_TX_PHY_STATUS, PLL_LOCK) ;

			/* pll status is not available on earlier chips */
			BDBG_ERR(("Tx%d: Video format (%#x) into HDMI Tx Core is unstable...PLL Lock: %s ",
			hHDMI->eCoreId, FormatChangeDetected, PllLock ? Yes : No)) ;
#else
			Register = BREG_Read32(hRegister, BCHP_HDMI_TX_PHY_STATUS + ulOffset) ;
			PllLock =  BCHP_GET_FIELD_DATA(Register, HDMI_TX_PHY_STATUS, PLL_LOCK) ;
			PllStatus = BCHP_GET_FIELD_DATA(Register, HDMI_TX_PHY_STATUS, PLL_STAT) ;
			BDBG_ERR(("Tx%d: Video format (%x) into HDMI Tx Core is unstable... PLL Lock: %s PLL Status: %x",
				hHDMI->eCoreId, FormatChangeDetected, PllLock ? Yes : No, PllStatus)) ;
#endif
                        BSTD_UNUSED(PllStatus) ;
                        BSTD_UNUSED(PllLock) ;
		}

		Register = 0xFFFFFFFF ;
		BREG_Write32(hRegister, BCHP_HDMI_FORMAT_DET_UPDATE_CLEAR + ulOffset, Register) ;
		Register = 0 ;
		BREG_Write32(hRegister, BCHP_HDMI_FORMAT_DET_UPDATE_CLEAR + ulOffset, Register) ;
	}
	else
	{
		BDBG_MSG(("Tx%d: Stable Format into HDMI Tx Core...", hHDMI->eCoreId)) ;
	}
	BTMR_StartTimer_isr(hHDMI->TimerFormatChange,
		BHDM_P_SECOND * BHDM_CONFIG_MONITOR_FORMAT_CHANGE_SECONDS) ;
#else
	BSTD_UNUSED(hHDMI) ;
#endif
}


static void BHDM_MONITOR_P_FormatChangesStart(const BHDM_Handle hHDMI)
{
#if BHDM_CONFIG_MONITOR_FORMAT_CHANGE_SECONDS
	uint32_t Register, ulOffset ;
	BREG_Handle hRegister = hHDMI->hRegister ;

	ulOffset = hHDMI->ulOffset ;

	Register = 0xFFFFFFFF;
	BREG_Write32(hRegister, BCHP_HDMI_FORMAT_DET_UPDATE_CLEAR + ulOffset, Register) ;
	Register = 0;
	BREG_Write32(hRegister, BCHP_HDMI_FORMAT_DET_UPDATE_CLEAR + ulOffset, Register) ;

	BTMR_StartTimer(hHDMI->TimerFormatChange,
		BHDM_P_SECOND * BHDM_CONFIG_MONITOR_FORMAT_CHANGE_SECONDS) ;

	hHDMI->TimerFormatInitialChangeReported = false ;
#else
	BSTD_UNUSED(hHDMI) ;
#endif
}

/* BHDM_MONITOR_HP_CHANGE_SECONDS used for testing HP Change counter */
/* the default should be 1 second */
#define BHDM_MONITOR_HP_CHANGE_SECONDS 1

void BHDM_MONITOR_P_HotplugChanges_isr(const BHDM_Handle hHDMI)
{
#if BHDM_CONFIG_MONITOR_HPD_CHANGES
	/* update totals */
	hHDMI->MonitorStatus.TotalHotPlugChanges +=
		hHDMI->MonitorStatus.NumHotPlugChanges ;

	/* show updates only */
	if (hHDMI->MonitorStatus.NumHotPlugChanges)
	{
		BDBG_MSG(("HotPlug Changes in last %ds: %d   TOTAL:  %d",
			BHDM_MONITOR_HP_CHANGE_SECONDS,
			hHDMI->MonitorStatus.NumHotPlugChanges,
			hHDMI->MonitorStatus.TotalHotPlugChanges)) ;
	}

	hHDMI->MonitorStatus.NumHotPlugChanges = 0 ;

	BTMR_StartTimer_isr(hHDMI->TimerHotPlugChange,
		BHDM_P_SECOND * BHDM_MONITOR_HP_CHANGE_SECONDS) ;
#else
	BSTD_UNUSED(hHDMI) ;
#endif
}


static void BHDM_MONITOR_P_ReadStatus_isr(const BHDM_Handle hHDMI)
{
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

#ifdef BCHP_HDMI_TX_PHY_HDMI_TX_PHY_STATUS

	Register = BREG_Read32(hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL + ulOffset) ;
	hHDMI->MonitorStatus.EnabledTMDS_Clock =
		!(Register & BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, TX_CK_PWRDN)) ;

	hHDMI->MonitorStatus.EnabledTMDS_CH2 =
		!(Register & BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, TX_2_PWRDN)) ;
	hHDMI->MonitorStatus.EnabledTMDS_CH1 =
		!(Register & BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, TX_1_PWRDN));
	hHDMI->MonitorStatus.EnabledTMDS_CH0 =
		!(Register & BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, TX_0_PWRDN)) ;
#else
	Register = BREG_Read32(hRegister, BCHP_HDMI_TX_PHY_POWERDOWN_CTL + ulOffset) ;
	hHDMI->MonitorStatus.EnabledTMDS_Clock =
		!(Register & BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, TX_CK_PWRDN)) ;

	hHDMI->MonitorStatus.EnabledTMDS_CH2 =
		!(Register & BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, TX_2_PWRDN)) ;
	hHDMI->MonitorStatus.EnabledTMDS_CH1 =
		!(Register & BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, TX_1_PWRDN)) ;
	hHDMI->MonitorStatus.EnabledTMDS_CH0 =
		!(Register & BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, TX_0_PWRDN)) ;
#endif
}


/******************************************************************************
void BHDM_MONITOR_P_StatusChanges_isr
Summary: Check/detect changes in Rx Sense
*******************************************************************************/
void BHDM_MONITOR_P_StatusChanges_isr(const BHDM_Handle hHDMI)
{
#if BHDM_CONFIG_MONITOR_STATUS_SECONDS
	/* read the Power Down Control Register */
	BHDM_MONITOR_P_ReadStatus_isr(hHDMI) ;


	/* copy rxSense status to structure */
	hHDMI->MonitorStatus.RxSense = hHDMI->rxSensePowerDetected ;

	/* update totals */
	hHDMI->MonitorStatus.TotalRxSenseChanges +=
		hHDMI->MonitorStatus.NumRxSenseChanges ;

	/* post updates only */
	if (hHDMI->MonitorStatus.NumRxSenseChanges)
	{
		BDBG_MSG(("RxSense Changes in last %ds: %d   TOTAL:  %d",
			BHDM_CONFIG_MONITOR_STATUS_SECONDS,
			hHDMI->MonitorStatus.NumRxSenseChanges,
			hHDMI->MonitorStatus.TotalRxSenseChanges)) ;

		BDBG_MSG(("   Rx Powered: %s; TMDS Power> CK: %s  CH2: %s   CH1: %s   CH0: %s",
			hHDMI->MonitorStatus.RxSense ? Yes : No,
			hHDMI->MonitorStatus.EnabledTMDS_Clock ? Yes : No,
			hHDMI->MonitorStatus.EnabledTMDS_CH2   ? Yes : No,
			hHDMI->MonitorStatus.EnabledTMDS_CH1   ? Yes : No,
			hHDMI->MonitorStatus.EnabledTMDS_CH0   ? Yes : No)) ;
	}

	hHDMI->MonitorStatus.NumRxSenseChanges = 0 ;
	BTMR_StartTimer_isr(hHDMI->TimerStatusMonitor,
		BHDM_P_SECOND * BHDM_CONFIG_MONITOR_STATUS_SECONDS) ;
#else
	BSTD_UNUSED(hHDMI) ;
#endif
}

void BHDM_MONITOR_P_StartTimers(const BHDM_Handle hHDMI)
{
	/* schedule check to detect format changes */
	BHDM_MONITOR_P_FormatChangesStart(hHDMI) ;

#if BHDM_CONFIG_MONITOR_STATUS_SECONDS
	BTMR_StartTimer(hHDMI->TimerStatusMonitor,
		BHDM_P_SECOND * BHDM_CONFIG_MONITOR_STATUS_SECONDS) ;
#endif

#if BHDM_CONFIG_MONITOR_HPD_CHANGES
	if (hHDMI->DeviceSettings.HotplugDetectThreshold)
	{
		if (!hHDMI->HpdTimerEnabled)
		{
			BTMR_StartTimer(hHDMI->TimerHotPlugChange,
				BHDM_P_SECOND * BHDM_MONITOR_HP_CHANGE_SECONDS) ;
			BDBG_MSG(("Excessive HP INTR monitoring ENABLED;  Threshold: %d INTRs",
				hHDMI->DeviceSettings.HotplugDetectThreshold)) ;
			hHDMI->HpdTimerEnabled = true ;
		}
	}
	else
	{
		BDBG_MSG(("Excessive HP INTR monitoring is DISABLED")) ;
	}
#endif
	BSTD_UNUSED(hHDMI) ;
}


void BHDM_MONITOR_P_StopTimers_isr(const BHDM_Handle hHDMI)
{
#if BHDM_CONFIG_MONITOR_FORMAT_CHANGE_SECONDS
	BTMR_StopTimer_isr(hHDMI->TimerFormatChange) ;
#endif

#if BHDM_CONFIG_MONITOR_STATUS_SECONDS
	if (hHDMI->TimerStatusMonitor)
		BTMR_StopTimer_isr(hHDMI->TimerStatusMonitor) ;
#endif

#if BHDM_CONFIG_MONITOR_HPD_CHANGES
	if (hHDMI->DeviceSettings.HotplugDetectThreshold)
	{
		if (hHDMI->HpdTimerEnabled)
		{
			BTMR_StopTimer_isr(hHDMI->TimerHotPlugChange) ;
			hHDMI->HpdTimerEnabled = false ;
		}
	}
#endif
	BSTD_UNUSED(hHDMI) ;
}


void BHDM_MONITOR_P_HpdChanges_isr(BHDM_Handle hHDMI)
{
#if BHDM_CONFIG_MONITOR_HPD_CHANGES
	if (!hHDMI->HpdTimerEnabled)
		return ;

	hHDMI->MonitorStatus.NumHotPlugChanges++ ;
	if (hHDMI->DeviceSettings.HotplugDetectThreshold)
	{
		if (hHDMI->MonitorStatus.NumHotPlugChanges > hHDMI->DeviceSettings.HotplugDetectThreshold)
		{
			BDBG_LOG((" ")) ;
			BDBG_ERR(("Tx%d: Exceeded EXCESSIVE HP INTRs of %d", hHDMI->eCoreId,
				hHDMI->DeviceSettings.HotplugDetectThreshold)) ;
			BDBG_ERR(("HP INTRs have been DISABLED")) ;
			BDBG_LOG((" ")) ;

			hHDMI->MonitorStatus.TxHotPlugInterruptDisabled = true ;
#if BHDM_CONFIG_DUAL_HPD_SUPPORT
			BINT_DisableCallback_isr( hHDMI->hCallback[MAKE_INTR_ENUM(HOTPLUG_REMOVED)]) ;
			BINT_DisableCallback_isr( hHDMI->hCallback[MAKE_INTR_ENUM(HOTPLUG_CONNECTED)]) ;
#else
			BINT_DisableCallback_isr( hHDMI->hCallback[MAKE_INTR_ENUM(HOTPLUG)]) ;
#endif
		}
	}
#else
	BSTD_UNUSED(hHDMI) ;
#endif
}


BERR_Code BHDM_MONITOR_GetHwStatusTx(const BHDM_Handle hHDMI, BHDM_MONITOR_Status *TxStatus)
{
	BERR_Code rc = BERR_SUCCESS ;
#if BHDM_CONFIG_MONITOR_STATUS_SECONDS
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	TxStatus->TxHotPlugInterruptDisabled = hHDMI->MonitorStatus.TxHotPlugInterruptDisabled ;

	BKNI_EnterCriticalSection() ;
		BHDM_MONITOR_P_ReadStatus_isr(hHDMI) ;
	BKNI_LeaveCriticalSection() ;

	BKNI_Memcpy(TxStatus, &hHDMI->MonitorStatus, sizeof(BHDM_MONITOR_Status)) ;
	goto done ;

#else

	BSTD_UNUSED(hHDMI) ;
	BSTD_UNUSED(TxStatus) ;

	rc = BERR_TRACE(BERR_NOT_AVAILABLE) ;
	goto done ;
#endif

done:
	return rc ;
}
