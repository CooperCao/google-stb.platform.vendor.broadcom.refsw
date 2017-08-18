/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 ******************************************************************************/


/***************************************
*** THIS IS for 40nm B0 (rev2)
***************************************/

#include "bstd.h"
#include "bdbg.h"
#include "bavc_hdmi.h"

#include "bcec.h"
#include "bcec_priv.h"
#include "bcec_config.h"


BDBG_MODULE(BCEC_PRIV) ;


/******************************************************************************
void BCEC_P_HandleInterrupt_isr
Summary: Handle interrupts for CEC core.
*******************************************************************************/
void BCEC_P_HandleInterrupt_isr(
	void *pParam1,						/* [in] Device handle */
	int parm2							/* [in] */
)
{
	uint32_t Register, ulOffset ;
	BCEC_Handle hCEC ;

#if BCEC_CONFIG_DEBUG_INTERRUPTS
	static const char *MsgStatus[] = {"NoACK", "ACK"} ;
	static const char *EomStatus[] = {"No", "Yes"} ;
#endif

	hCEC = (BCEC_Handle) pParam1 ;
	ulOffset = hCEC->ulRegisterOffset;
	BDBG_OBJECT_ASSERT(hCEC, BCEC_P_Handle) ;

	/*
	-- Interrupts to be handled
	00 CEC_INTR_SENT
	01 CEC_INTR_RECEIVED
	*/

	switch (parm2)
	{
	case BCEC_MAKE_INTR_ENUM(SENT) : 					   /* 00 */

		Register = BREG_Read32(hCEC->stDependencies.hRegister, REGADDR_CEC_CNTRL_1 + ulOffset) ;
		hCEC->lastTransmitMessageStatus.uiStatus =
			BCHP_GET_FIELD_DATA(Register, REGNAME_CEC_CNTRL_1, TX_STATUS_GOOD) ;

		hCEC->lastTransmitMessageStatus.uiMessageLength =
			BCHP_GET_FIELD_DATA(Register, REGNAME_CEC_CNTRL_1, MESSAGE_LENGTH) ;

		hCEC->lastTransmitMessageStatus.uiEOM =
			BCHP_GET_FIELD_DATA(Register, REGNAME_CEC_CNTRL_1, TX_EOM) ;

#if BCEC_CONFIG_DEBUG_INTERRUPTS
		BDBG_WRN(("Message SENT Interrupt (0x%x)!", parm2));
		BDBG_WRN(("Transmitted Length: %d; Xmit Status: %s ",
						hCEC->lastTransmitMessageStatus.uiMessageLength,
						MsgStatus[hCEC->lastTransmitMessageStatus.uiStatus])) ;
#endif

		/* Reset XMIT_BEGIN back to 0 to avoid spurious interrupt if the core get reset */
		Register = BREG_Read32(hCEC->stDependencies.hRegister, REGADDR_CEC_CNTRL_1 + ulOffset) ;
		Register &=  ~ BCHP_MASK_CEC_CNTRL(CEC_CNTRL_1, START_XMIT_BEGIN) ;
		BREG_Write32(hCEC->stDependencies.hRegister, REGADDR_CEC_CNTRL_1 + ulOffset, Register) ;   /* 0 */

#if BCEC_CONFIG_ENABLE_COMPLIANCE_TEST_WORKAROUND
		/* got sent interrupt, stop the armed timer.
		If Timer hasn't started, this should make no difference */
		if (hCEC->hTimer) {
			BTMR_StopTimer_isr(hCEC->hTimer);
		}
#endif

		/* Set CEC Sent Event */
		BKNI_SetEvent(hCEC->BCEC_EventCec_Transmitted);
		break;


	case BCEC_MAKE_INTR_ENUM(RECEIVED) :

		Register = BREG_Read32(hCEC->stDependencies.hRegister, REGADDR_CEC_CNTRL_1 + ulOffset) ;
		hCEC->lastReceivedMessageStatus.uiStatus =
			BCHP_GET_FIELD_DATA(Register, REGNAME_CEC_CNTRL_1, RX_STATUS_GOOD) ;

		hCEC->lastReceivedMessageStatus.uiEOM =
			BCHP_GET_FIELD_DATA(Register, REGNAME_CEC_CNTRL_1, RX_EOM) ;

			/* number of rx CEC words that came in */
		hCEC->lastReceivedMessageStatus.uiMessageLength =
			BCHP_GET_FIELD_DATA(Register, REGNAME_CEC_CNTRL_1, REC_WRD_CNT) ;

#if BCEC_CONFIG_DEBUG_INTERRUPTS
		BDBG_WRN(("Message RECEIVED interrupt (0x%x)!", parm2));
		BDBG_WRN(("Received %d Bytes, EOM status: %s",
			hCEC->lastReceivedMessageStatus.uiMessageLength,
			EomStatus[hCEC->lastReceivedMessageStatus.uiEOM])) ;
#endif

		/* Check for wake up message */
		hCEC->lastReceivedMessageStatus.bWokeUp = false;

		/* Save wake1Reason and wake2Reason before clearing */
		Register = BREG_Read32(hCEC->stDependencies.hRegister, REGADDR_CEC_TX_AUTO_CEC_STATUS_0 + ulOffset) ;
		hCEC->wakeReason1 = BCHP_GET_FIELD_DATA(Register, REGNAME_CEC_TX_AUTO_CEC_STATUS_0, WAKE1_REASON);
		hCEC->wakeReason2 = BCHP_GET_FIELD_DATA(Register, REGNAME_CEC_TX_AUTO_CEC_STATUS_0, WAKE2_REASON);
		hCEC->wakeReason3 = BCHP_GET_FIELD_DATA(Register, REGNAME_CEC_TX_AUTO_CEC_STATUS_0, WAKE3_REASON);
		hCEC->wakeReason4 = BCHP_GET_FIELD_DATA(Register, REGNAME_CEC_TX_AUTO_CEC_STATUS_0, WAKE4_REASON);

#if BCEC_CONFIG_DEBUG_INTERRUPTS
		BDBG_WRN(("%s: wakeReason1=0x%08x, wakeReason2=0x%08x, wakeReason3=0x%08x, wakeReason4=0x%08x [STATUS_0=0x%08x]",
			BSTD_FUNCTION, hCEC->wakeReason1, hCEC->wakeReason2, hCEC->wakeReason3, hCEC->wakeReason4, Register));
#endif

		/*************************************************************
		* If the last received message was a wake up message, AUTOCEC will automatically
		* ACK and CLEAR_RECEIVE_OFF. Thus, the message length will be reset to 0, which
		* is incorrect. Need to override the message length
		**************************************************************/
		if (hCEC->wakeReason1 || hCEC->wakeReason2 || hCEC->wakeReason3 || hCEC->wakeReason4)
		{
			hCEC->lastReceivedMessageStatus.bWokeUp = true;

			/* Check RDB description for WAKE1_REASON and WAKE2_REASON usage */
			switch(hCEC->wakeReason1)
			{
			case BCEC_Wake_Reason_1_ImageViewOn:
			case BCEC_Wake_Reason_1_TextViewOn:
				hCEC->lastReceivedMessageStatus.uiMessageLength = 1;
				break;

			case BCEC_Wake_Reason_1_PowerOnFunction:
			case BCEC_Wake_Reason_1_PowerToggleFunction:
				hCEC->lastReceivedMessageStatus.uiMessageLength = 2;
				break;

			case BCEC_Wake_Reason_1_ActiveSource:
				hCEC->lastReceivedMessageStatus.uiMessageLength = 3;
				break;

			default: /* do nothing */
				break;
			}

			switch(hCEC->wakeReason2)
			{
			case BCEC_Wake_Reason_2_PlayForward:
			case BCEC_Wake_Reason_2_DeckControl_Eject:
				hCEC->lastReceivedMessageStatus.uiMessageLength = 2;
				break;

			default: /* do nothing */
				break;
			}

			switch(hCEC->wakeReason3)
			{
			case BCEC_Wake_Reason_3_SetStreamPath:
				hCEC->lastReceivedMessageStatus.uiMessageLength = 3;
				break;

			default: /* do nothing */
				break;
			}

			switch(hCEC->wakeReason4)
			{
			case BCEC_Wake_Reason_4_UserControlPress_Power:
				hCEC->lastReceivedMessageStatus.uiMessageLength = 2;
				break;

			default: /* do nothing */
				break;
			}

			/* Clear WakeReason after read */
			Register = BREG_Read32(hCEC->stDependencies.hRegister, REGADDR_CEC_TX_AUTO_CEC_CNTRL + ulOffset) ;
			Register |= BCHP_MASK_DVP(AUTO_CEC_CNTRL, CLEAR_WAKE_REASON);
			BREG_Write32(hCEC->stDependencies.hRegister, REGADDR_CEC_TX_AUTO_CEC_CNTRL + ulOffset, Register) ;	/* Wr 1 */
		}

		/* Set CEC Received Event */
		BKNI_SetEvent(hCEC->BCEC_EventCec_Received) ;
		break ;

	default :
		BDBG_ERR(("BCEC Unknown Interrupt ID=0x%x !", parm2 ));
		break;
	}

	return;
}

void BCEC_P_ClearWakeupStatus(BCEC_Handle hCEC, uint32_t StatusControlRegAddr)
{
	uint32_t Register;

	Register = BREG_Read32(hCEC->stDependencies.hRegister, StatusControlRegAddr) ;
	Register |= BCHP_MASK_DVP(CEC_ENERGYSTAR_CNTRL, HOTPLUG_CLR_INTERRUPT_DET)
			| BCHP_MASK_DVP(CEC_ENERGYSTAR_CNTRL, HOTPLUG_ACTIVITY_CLEAR)
			| BCHP_MASK_DVP(CEC_ENERGYSTAR_CNTRL, CEC_CLR_LOW_INTERRUPT_DET)
			| BCHP_MASK_DVP(CEC_ENERGYSTAR_CNTRL, CEC_CLR_INTERRUPT_DET)
			| BCHP_MASK_DVP(CEC_ENERGYSTAR_CNTRL, CEC_ACTIVITY_CLEAR);
	BREG_Write32(hCEC->stDependencies.hRegister, StatusControlRegAddr, Register) ;  /* Wr 1 */

	Register &= ~(BCHP_MASK_DVP(CEC_ENERGYSTAR_CNTRL, HOTPLUG_CLR_INTERRUPT_DET)
			| BCHP_MASK_DVP(CEC_ENERGYSTAR_CNTRL, HOTPLUG_ACTIVITY_CLEAR)
			| BCHP_MASK_DVP(CEC_ENERGYSTAR_CNTRL, CEC_CLR_LOW_INTERRUPT_DET)
			| BCHP_MASK_DVP(CEC_ENERGYSTAR_CNTRL, CEC_CLR_INTERRUPT_DET)
			| BCHP_MASK_DVP(CEC_ENERGYSTAR_CNTRL, CEC_ACTIVITY_CLEAR));
	BREG_Write32(hCEC->stDependencies.hRegister, StatusControlRegAddr, Register) ;  /* Wr 0 */

	return;
}


void BCEC_P_MaskWakeupEvents(BCEC_Handle hCEC, uint32_t ConfigRegisterAddr, uint32_t ConfigRegisterAddr2)
{
	uint32_t Register;

	/* mask  "CEC address match" to prevent it from waking up the chip.
	If this event is enabled, any CEC messages with the address specified in the
	AON_HDMI_TX.CEC_CNTRL.CEC_ADDR will cause the chip to wake up.
	This is not the desired behavior */

	Register = BREG_Read32(hCEC->stDependencies.hRegister, ConfigRegisterAddr) ;
		Register |=
			  BCHP_MASK_DVP(CEC_ENERGYSTAR_CFG2, CEC_MASK_IRQ)           /* CEC Addr Match */
			| BCHP_MASK_DVP(CEC_ENERGYSTAR_CFG2, CEC_MASK_LOW_IRQ) ;/* Invalid CEC Command */

		/* mask the hot plug signal.
		    TODO should chip wake up if device is connected/removed on Tx port ??? */
		Register |=
			  BCHP_MASK_DVP(CEC_ENERGYSTAR_CFG2, HOTPLUG_MASK_DETACH)  /* HP Remove */
			|BCHP_MASK_DVP(CEC_ENERGYSTAR_CFG2, HOTPLUG_MASK_IRQ) ;      /* HP Connect */
	BREG_Write32(hCEC->stDependencies.hRegister, ConfigRegisterAddr, Register) ;  /* Wr 1 */


	/* Don't wake up if received other messages*/
	Register = BREG_Read32(hCEC->stDependencies.hRegister, ConfigRegisterAddr2) ;
		Register &= ~(BCHP_MASK_DVP(AUTO_CEC_FEATURE_ABORT_CFG, WAKE_ON_NO_MATCH));
	BREG_Write32(hCEC->stDependencies.hRegister, ConfigRegisterAddr2, Register);

	return;
}


/* Enable CEC AutoOn feature */
void BCEC_P_EnableAutoOn(BCEC_Handle hCEC, bool enable)
{
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset;
	BCHP_Info chipInfo;

	hRegister = hCEC->stDependencies.hRegister ;
	ulOffset = hCEC->ulRegisterOffset;


	/* whether enabling or disabling AutoON; clear all WakeUp status values
	     and mask any unwanted Wake Up events */

	BCEC_P_ClearWakeupStatus(hCEC, REGADDR_CEC_TX_CEC_ENERGYSTAR_CNTRL) ;

	BCEC_P_MaskWakeupEvents(hCEC, REGADDR_CEC_TX_CEC_ENERGYSTAR_CFG2,
			REGADDR_CEC_TX_AUTO_CEC_FEATURE_ABORT_CFG) ;


	/*************************************
	* This is a SW work-around to fix an issue with CEC HW for 7366 only.
	* The 7366 HW has total 2 HDMI TX cores and 1 HDMI RX core. However,
	* only 1 HDMI_TX core is made visible. Since the other 2 cores are invisible,
	* CEC PI does not mask the HOTPLUG_MASK_IRQ (default no mask). Thus,
	* the result is after enter standby, the chip will be waken up immediately due to
	* hotplug_irq wake up.
	**************************************/
	BCHP_GetInfo(hCEC->stDependencies.hChip, &chipInfo) ;
	if (chipInfo.familyId == 0x7366 && chipInfo.rev >= 0x10)
	{
		/* HDMI_TX1_AON is 0x800 offset from HDMI_TX_AON - blindly set HOTPLUG_MASK_IRQ bit to 1 */
		BREG_Write32(hRegister, REGADDR_CEC_TX_CEC_ENERGYSTAR_CFG2 + 0x800, 0x000000C0);

		/* HDMI_RX_AON is 0xA00 offset from HDMI_TX_AON - blindly set HOTPLUG_MASK_IRQ bit to 1 */
		BREG_Write32(hRegister, REGADDR_CEC_TX_CEC_ENERGYSTAR_CFG2 + 0xA00, 0x000000C0);
	}
	if (chipInfo.familyId == 0x74371 && chipInfo.rev == 0x00)
	{
		BREG_Write32(hRegister, REGADDR_CEC_TX_CEC_ENERGYSTAR_CFG2 + 0x100, 0x000000C0);
		BREG_Write32(hRegister, REGADDR_CEC_TX_CEC_ENERGYSTAR_CFG2 + 0x200, 0x000000C0);
	}


#if BCEC_CONFIG_HAS_HDMI_RX
	/* If HDMI_RX is also on this chip,
	    be sure to it's WakeUp status and mask unwanted events */

	BCEC_P_ClearWakeupStatus(hCEC, REGADDR_CEC_RX_CEC_ENERGYSTAR_CNTRL) ;

	BCEC_P_MaskWakeupEvents(hCEC, REGADDR_CEC_RX_CEC_ENERGYSTAR_CFG2,
			REGADDR_CEC_RX_AUTO_CEC_FEATURE_ABORT_CFG) ;

#endif


	if (enable)
	{
		BDBG_MSG(("Enable Auto-On CEC function"));

		/* Toggle to clear last AutoCEC Event */
		Register = BREG_Read32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CNTRL + ulOffset) ;
		Register |= BCHP_MASK_DVP(AUTO_CEC_CNTRL, CLEAR_AUTO_CEC_XACTION_MATCH)
				| BCHP_MASK_DVP(AUTO_CEC_CNTRL, CLEAR_SET_AUTOCEC_RUN)
				| BCHP_MASK_DVP(AUTO_CEC_CNTRL, CLEAR_WAKE_REASON);
		BREG_Write32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CNTRL + ulOffset, Register) ;  /* Wr 1 */

		Register &= ~(BCHP_MASK_DVP(AUTO_CEC_CNTRL, CLEAR_AUTO_CEC_XACTION_MATCH)
				| BCHP_MASK_DVP(AUTO_CEC_CNTRL, CLEAR_SET_AUTOCEC_RUN)
				| BCHP_MASK_DVP(AUTO_CEC_CNTRL, CLEAR_WAKE_REASON));
		BREG_Write32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CNTRL + ulOffset, Register) ;  /* Wr 0 */


		/* Enable Auto CEC */
		Register = BREG_Read32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CFG + ulOffset) ;
		Register |= BCHP_MASK_DVP(AUTO_CEC_CFG, SELECT_CEC_TX)
				| BCHP_MASK_DVP(AUTO_CEC_CFG, AUTO_CEC_EN)
				| BCHP_MASK_DVP(AUTO_CEC_CFG, AUTO_CLEAR_CEC_INTR_DET);
		BREG_Write32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CFG + ulOffset, Register) ;  /* Wr 1 */

		/* Need to toggle AUTO_CLEAR_CEC_INTR_DET */
		Register = BREG_Read32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CFG + ulOffset) ;
		Register &= ~(BCHP_MASK_DVP(AUTO_CEC_CFG, AUTO_CLEAR_CEC_INTR_DET));
		BREG_Write32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CFG + ulOffset, Register) ;  /* Wr 0 */


		/*********************************************************
		  * Configure/enable specific CEC messages to wake up the device	  *
		  * The chip will only wake up if one of these CEC messages are	  *
		  * received												  *
		  *********************************************************/

		/* Configure Check Power Status - Opcode 0x8F.
			The hardware will automatically response to request of current
			power status. The chip will not be	waken up when this CEC message is received
		*********/
		Register = BREG_Read32(hRegister, REGADDR_CEC_TX_AUTO_CEC_POWER_STATUS_CFG + ulOffset) ;
		Register &= ~(BCHP_MASK_DVP(AUTO_CEC_POWER_STATUS_CFG, POWER_STAT_PARAM)
				| BCHP_MASK_DVP(AUTO_CEC_POWER_STATUS_CFG, POWER_STAT_EN)
				| BCHP_MASK_DVP(AUTO_CEC_POWER_STATUS_CFG, POWER_STAT_MODE)
				| BCHP_MASK_DVP(AUTO_CEC_POWER_STATUS_CFG, POWER_STAT_REARM)
				| BCHP_MASK_DVP(AUTO_CEC_POWER_STATUS_CFG, POWER_STAT_SENSITIVITY));
		Register |= BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_POWER_STATUS_CFG, POWER_STAT_PARAM, 1)
				| BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_POWER_STATUS_CFG, POWER_STAT_EN, 1)
				| BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_POWER_STATUS_CFG, POWER_STAT_MODE, 1)
				| BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_POWER_STATUS_CFG, POWER_STAT_REARM, 1)
				| BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_POWER_STATUS_CFG, POWER_STAT_SENSITIVITY, 7);
		BREG_Write32(hRegister, REGADDR_CEC_TX_AUTO_CEC_POWER_STATUS_CFG + ulOffset, Register) ;


		if (hCEC->stSettings.eDeviceType == BCEC_DeviceType_eTv)
		{
			/********************************
			   enable <ImageViewOn> - OpCode 0x04
			   and     <TextViewOn>  -  OpCode 0x0D
			********************************/
			Register = BREG_Read32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CHECK_WAKE1_CFG_0 + ulOffset) ;
			Register &= ~(BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE1_CFG_0, TEXT_VIEW_EN)
					| BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE1_CFG_0, TEXT_VIEW_SENSITIVITY)
					| BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE1_CFG_0, IMAGE_VIEW_EN)
					| BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE1_CFG_0, IMAGE_VIEW_SENSITIVITY));
			Register |= BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE1_CFG_0, TEXT_VIEW_EN, 1)
					| BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE1_CFG_0, TEXT_VIEW_SENSITIVITY, 7)
					| BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE1_CFG_0, IMAGE_VIEW_EN, 1)
					| BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE1_CFG_0, IMAGE_VIEW_SENSITIVITY, 7);
			BREG_Write32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CHECK_WAKE1_CFG_0 + ulOffset, Register) ;
		}

		if (hCEC->stSettings.eDeviceType == BCEC_DeviceType_eTv
		|| hCEC->stSettings.eDeviceType == BCEC_DeviceType_ePureCecSwitch)
		{

			/********************************
			 enable <ActiveSouce> - OpCode 0x82
			********************************/
			Register = BREG_Read32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CHECK_WAKE1_CFG_1 + ulOffset) ;
			Register &= ~(BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE1_CFG_1, ACTIVE_SOURCE_EN)
					| BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE1_CFG_1, ACTIVE_SOURCE_SENSITIVITY));
			Register |= BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE1_CFG_1, ACTIVE_SOURCE_EN, 1)
					| BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE1_CFG_1, ACTIVE_SOURCE_SENSITIVITY, 7);
			BREG_Write32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CHECK_WAKE1_CFG_1 + ulOffset, Register) ;
		}


		/********************************
		 enable <UserPowerOn>
		    and <UserPowerToggle> - OpCode 0x44
		********************************/
		Register = BREG_Read32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CHECK_WAKE1_CFG_3 + ulOffset) ;
			Register &= ~(BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE1_CFG_3, USER_POWER_ON_EN)

			/* Not all platforms with AUTO_ON_CEC has this particular field */
#ifdef BCHP_AON_HDMI_TX_AUTO_CEC_CHECK_WAKE1_CFG_3_USER_POWER_TOGGLE_WAKE1_EN_MASK
					| BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE1_CFG_3, USER_POWER_TOGGLE_WAKE1_EN)
#endif
					| BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE1_CFG_3, USER_POWER_ON_SENSITIVITY)
					| BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE1_CFG_3, USER_POWER_TOGGLE_EN)
					| BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE1_CFG_3, USER_POWER_TOGGLE_SENSITIVITY));

			Register |= BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE1_CFG_3, USER_POWER_ON_EN, 1)

			/* Not all platforms with AUTO_ON_CEC has this particular field */
#ifdef BCHP_AON_HDMI_TX_AUTO_CEC_CHECK_WAKE1_CFG_3_USER_POWER_TOGGLE_WAKE1_EN_MASK
					| BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE1_CFG_3, USER_POWER_TOGGLE_WAKE1_EN, 1)
#endif
					| BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE1_CFG_3, USER_POWER_ON_SENSITIVITY, 7)
					| BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE1_CFG_3, USER_POWER_TOGGLE_EN, 1)
					| BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE1_CFG_3, USER_POWER_TOGGLE_SENSITIVITY, 7);
		BREG_Write32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CHECK_WAKE1_CFG_3 + ulOffset, Register) ;

		/* Update HW power status */
		Register = BREG_Read32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CNTRL + ulOffset) ;
			Register &= ~(BCHP_MASK_DVP(AUTO_CEC_CNTRL, POWER_STATE_CONTROL));
			Register |= BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CNTRL, POWER_STATE_CONTROL, 3);
		BREG_Write32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CNTRL + ulOffset, Register) ;


		/* Ensure Feature_Abort message respond with an appropriate reason */
		Register = BREG_Read32(hRegister, REGADDR_CEC_TX_AUTO_CEC_FEATURE_ABORT_CFG + ulOffset) ;
		Register &= ~(BCHP_MASK_DVP(AUTO_CEC_FEATURE_ABORT_CFG, F_ABORT_PARAM)
				| BCHP_MASK_DVP(AUTO_CEC_FEATURE_ABORT_CFG, F_ABORT_EN));
		Register |= BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_FEATURE_ABORT_CFG, F_ABORT_PARAM, 1)
				| BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_FEATURE_ABORT_CFG, F_ABORT_EN, 1); /* "Not in correct mode to repond" */
		BREG_Write32(hRegister, REGADDR_CEC_TX_AUTO_CEC_FEATURE_ABORT_CFG + ulOffset, Register) ;

		/* Rearm WAKE1 configurations to enable rechecking on the above CEC messages */
		Register = BREG_Read32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CHECK_WAKE1_CFG_0 + ulOffset) ;
		Register |= BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE1_CFG_0, WAKE1_REARM);
		BREG_Write32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CHECK_WAKE1_CFG_0 + ulOffset, Register) ;  /* Wr 1 */


		if (hCEC->stSettings.stDeviceFeatures.supportsDeckControl &&
				(hCEC->stSettings.eDeviceType == BCEC_DeviceType_eRecordingDevice
				|| hCEC->stSettings.eDeviceType == BCEC_DeviceType_ePlaybackDevice))
		{
			/********************************
			   enable <PlayForward> - OpCode 0x41
			********************************/
			Register = BREG_Read32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CHECK_WAKE2_CFG_0 + ulOffset) ;
			Register &= ~(BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE2_CFG_0, PLAY_FORWARD_EN)
					| BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE2_CFG_0, PLAY_FORWARD_SENSITIVITY));
			Register |= BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE2_CFG_0, PLAY_FORWARD_EN, 1)
					| BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE2_CFG_0, PLAY_FORWARD_SENSITIVITY, 7);
			BREG_Write32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CHECK_WAKE2_CFG_0 + ulOffset, Register) ;

			/********************************
			enable <DeckControlEject> - OpCode 0x42
			********************************/
			Register = BREG_Read32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CHECK_WAKE2_CFG_1 + ulOffset) ;
			Register &= ~(BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE2_CFG_1, EJECT_EN)
				 | BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE2_CFG_1, EJECT_SENSITIVITY));
			Register |= BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE2_CFG_1, EJECT_EN, 1)
				 | BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE2_CFG_1, EJECT_SENSITIVITY, 7);
			BREG_Write32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CHECK_WAKE2_CFG_1 + ulOffset, Register) ;
		}

		/* Rearm WAKE2 configurations to enable rechecking */
		Register = BREG_Read32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CHECK_WAKE2_CFG_0 + ulOffset) ;
		Register |= BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE2_CFG_0, WAKE2_REARM);
		BREG_Write32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CHECK_WAKE2_CFG_0 + ulOffset, Register) ;  /* Wr 1 */


		if (hCEC->stSettings.eDeviceType == BCEC_DeviceType_eRecordingDevice
		|| hCEC->stSettings.eDeviceType == BCEC_DeviceType_eTuner
		|| hCEC->stSettings.eDeviceType == BCEC_DeviceType_ePlaybackDevice
		|| hCEC->stSettings.eDeviceType == BCEC_DeviceType_ePureCecSwitch
		|| hCEC->stSettings.eDeviceType == BCEC_DeviceType_eVideoProcessor)
		{
			/********************************
			   enable <Set Stream Path> - OpCode 0x86
			********************************/
			Register = BREG_Read32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CHECK_WAKE3_CFG_0 + ulOffset) ;
			Register &= ~(BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE3_CFG_0, CUSTOM3_OP)
						| BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE3_CFG_0, CUSTOM3_PARAM1)
						| BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE3_CFG_0, CUSTOM3_PARAM2)
						| BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE3_CFG_0, CUSTOM3_EN)
						| BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE3_CFG_0, CUSTOM3_SENSITIVITY)
						| BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE3_CFG_0, CUSTOM3_PARAMETER1_EN)
						| BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE3_CFG_0, CUSTOM3_PARAMETER2_EN));

			Register |= BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE3_CFG_0,
								CUSTOM3_OP, BCEC_OpCode_SetStreamPath)
						| BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE3_CFG_0, CUSTOM3_EN, 1)
						| BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE3_CFG_0,
											CUSTOM3_SENSITIVITY, 8)
						| BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE3_CFG_0,
											CUSTOM3_PARAM1, hCEC->stSettings.CecPhysicalAddr[0])
						| BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE3_CFG_0,
											CUSTOM3_PARAM2, hCEC->stSettings.CecPhysicalAddr[1])
						| BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE3_CFG_0,
							  CUSTOM3_PARAMETER1_EN, 1)
						| BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE3_CFG_0,
							  CUSTOM3_PARAMETER2_EN, 1);
			BREG_Write32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CHECK_WAKE3_CFG_0 + ulOffset, Register) ;

			/* Disable CUSTOM3_RESPOND WAKE3 configurations */
			Register = BREG_Read32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CHECK_WAKE3_CFG_1 + ulOffset) ;
			Register &= ~(BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE3_CFG_1, RESPOND3_EN)
						| BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE3_CFG_1, WAKE3_EN)
						| BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE3_CFG_1, WAKE3_REARM));

			Register |= BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE3_CFG_1, RESPOND3_EN, 0)
					| BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE3_CFG_1, WAKE3_EN, 1);
			BREG_Write32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CHECK_WAKE3_CFG_1 + ulOffset, Register) ;
		}

#if BCEC_CONFIG_AUTO_ON_CEC_20_SUPPORT
		/********************************
		   enable <UserControlPressed> (0x44) - [UI Command] = Power (0x40) parameter
		   as one of the wake up messages
		********************************/
		Register = BREG_Read32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CHECK_WAKE4_CFG_0 + ulOffset) ;
		Register &= ~(BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE4_CFG_0, CUSTOM4_OP)
					| BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE4_CFG_0, CUSTOM4_PARAM1)
					| BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE4_CFG_0, CUSTOM4_PARAM2)
					| BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE4_CFG_0, CUSTOM4_EN)
					| BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE4_CFG_0, CUSTOM4_SENSITIVITY)
					| BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE4_CFG_0, CUSTOM4_PARAMETER1_EN)
					| BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE4_CFG_0, CUSTOM4_PARAMETER2_EN));

		Register |= BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE4_CFG_0,
							CUSTOM4_OP, BCEC_OpCode_UserControlPressed)
					| BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE4_CFG_0, CUSTOM4_EN, 1)
					| BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE4_CFG_0,
										CUSTOM4_SENSITIVITY, 7)		/* direct message */
					| BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE4_CFG_0,
										CUSTOM4_PARAM1, 0x40)	/*	[UI Command] POWER = 0x40 */
					| BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE4_CFG_0,
						  CUSTOM4_PARAMETER1_EN, 1);
		BREG_Write32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CHECK_WAKE4_CFG_0 + ulOffset, Register) ;

		/* Disable CUSTOM4_RESPOND & Rearm WAKE4 configurations to enable rechecking */
		Register = BREG_Read32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CHECK_WAKE4_CFG_1 + ulOffset) ;
		Register &= ~(BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE4_CFG_1, RESPOND4_EN)
					| BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE4_CFG_1, WAKE4_EN)
					| BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE4_CFG_1, WAKE4_REARM));

		Register |= BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE4_CFG_1, RESPOND4_EN, 0)
				| BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE4_CFG_1, WAKE4_EN, 1);
		BREG_Write32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CHECK_WAKE4_CFG_1 + ulOffset, Register) ;

		/* Make sure AutoCEC HW circuit respond to this message */
		Register = BREG_Read32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CHECK_WAKE1_CFG_2 + ulOffset) ;
		Register &= ~(BCHP_MASK_DVP(AUTO_CEC_CHECK_WAKE1_CFG_2, USER_CONTROL_CUSTOM_PARAM));

		Register |= BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_CHECK_WAKE1_CFG_2, USER_CONTROL_CUSTOM_PARAM, 0x40);
		BREG_Write32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CHECK_WAKE1_CFG_2 + ulOffset, Register) ;


		/********************************
			Configure GIVE_CEC_VERSION - Opcode 0x9F.
			The hardware will automatically response to request of current
			power status. The chip will not be	waken up when this CEC message is received
		********************************/
		Register = BREG_Read32(hRegister, REGADDR_CEC_TX_AUTO_CEC_GIVE_VERSION_CFG + ulOffset) ;
		Register &= ~(BCHP_MASK_DVP(AUTO_CEC_GIVE_VERSION_CFG, GIVE_VERSION_EN)
					 | BCHP_MASK_DVP(AUTO_CEC_GIVE_VERSION_CFG, GIVE_VERSION_SENSITIVITY)
					 | BCHP_MASK_DVP(AUTO_CEC_GIVE_VERSION_CFG, REPORT_VERSION_WAKE)
					 | BCHP_MASK_DVP(AUTO_CEC_GIVE_VERSION_CFG, REPORT_VERSION_REARM)
					 | BCHP_MASK_DVP(AUTO_CEC_GIVE_VERSION_CFG, REPORT_VERSION_PARAM));

		Register |= BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_GIVE_VERSION_CFG, GIVE_VERSION_EN, 1)
				  | BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_GIVE_VERSION_CFG, GIVE_VERSION_SENSITIVITY, 7)
				  | BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_GIVE_VERSION_CFG, REPORT_VERSION_REARM, 1)
				  | BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_GIVE_VERSION_CFG, REPORT_VERSION_PARAM, BCEC_VERSION);
		BREG_Write32(hRegister, REGADDR_CEC_TX_AUTO_CEC_GIVE_VERSION_CFG + ulOffset, Register) ;
#endif
	}
	else
	{
		BDBG_MSG(("Disable Auto-On CEC function"));

		Register = BREG_Read32(hRegister, REGADDR_CEC_TX_AUTO_CEC_POWER_STATUS_CFG + ulOffset) ;
		Register &= ~(BCHP_MASK_DVP(AUTO_CEC_POWER_STATUS_CFG, POWER_STAT_PARAM)
				| BCHP_MASK_DVP(AUTO_CEC_POWER_STATUS_CFG, POWER_STAT_EN)
				| BCHP_MASK_DVP(AUTO_CEC_POWER_STATUS_CFG, POWER_STAT_MODE)
				| BCHP_MASK_DVP(AUTO_CEC_POWER_STATUS_CFG, POWER_STAT_REARM)
				| BCHP_MASK_DVP(AUTO_CEC_POWER_STATUS_CFG, POWER_STAT_SENSITIVITY));
		Register |= BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_POWER_STATUS_CFG, POWER_STAT_PARAM, 0)
				| BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_POWER_STATUS_CFG, POWER_STAT_EN, 1)
				| BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_POWER_STATUS_CFG, POWER_STAT_MODE, 1)
				| BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_POWER_STATUS_CFG, POWER_STAT_REARM, 1)
				| BCHP_FIELD_DATA(REGNAME_CEC_TX_AUTO_CEC_POWER_STATUS_CFG, POWER_STAT_SENSITIVITY, 7);
		BREG_Write32(hRegister, REGADDR_CEC_TX_AUTO_CEC_POWER_STATUS_CFG + ulOffset, Register) ;


		Register = BREG_Read32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CFG + ulOffset) ;
		Register &= ~(BCHP_MASK_DVP(AUTO_CEC_CFG, SELECT_CEC_TX)
				| BCHP_MASK_DVP(AUTO_CEC_CFG, AUTO_CEC_EN));									/* Wr 0 */
		Register |= BCHP_MASK_DVP(AUTO_CEC_CFG, AUTO_CLEAR_CEC_INTR_DET);						/* Wr 1 */
		BREG_Write32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CFG + ulOffset, Register) ;

		/* Need to toggle AUTO_CLEAR_CEC_INTR_DET */
		Register = BREG_Read32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CFG + ulOffset) ;
		Register &= ~(BCHP_MASK_DVP(AUTO_CEC_CFG, AUTO_CLEAR_CEC_INTR_DET)); 		 		 	 /* Wr 0 */
		BREG_Write32(hRegister, REGADDR_CEC_TX_AUTO_CEC_CFG + ulOffset, Register) ;
	}

	return;
}


