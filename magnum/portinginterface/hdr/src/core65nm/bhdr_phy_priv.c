/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bhdr_phy_priv.h"

#include "bhdr_fe.h"
#include "bhdr_fe_priv.h"

#include "bhdr.h"
#include "bhdr_priv.h"

#include "bhdr_config.h"

BDBG_MODULE(BHDR_PHY) ;


#if BHDR_CONFIG_SYMBOL_LOSS_SM

static char * SL_TEXT [BHDR_P_SYMBOL_LOSS_SM_eCount] =
{
	/* 0 */	"WaitAfterHotPlug",
	/* 1 */	"Locked",
	/* 2 */	"ClearAuth",
	/* 3 */	"ResetPll",	
	/* 4 */	"Final",
	/* 5 */	"ClrDCThreshold",
	/* 6 */	"SetDCThreshold"
}  ;


BERR_Code BHDR_UsageMasterSlaveSet(
   BHDR_Handle hHDRMaster,  BHDR_Handle hHDRSlave)
{
	BERR_Code rc = BERR_SUCCESS ;

	BDBG_ENTER(BHDR_UsageMasterSlaveSet) ;
	BDBG_OBJECT_ASSERT(hHDRMaster, BHDR_P_Handle) ;
	BDBG_OBJECT_ASSERT(hHDRSlave, BHDR_P_Handle) ;

	hHDRMaster->hHDRSlave = hHDRSlave ;
	hHDRMaster->eUsage = BHDR_Usage_eMaster ;
	hHDRSlave->eUsage = BHDR_Usage_eSlave ;

	BDBG_WRN(("Disable CH%d (slave) interrupts", hHDRSlave->eCoreId)) ;
	BHDR_P_EnableInterrupts_isr(hHDRSlave, false) ;
	
	BDBG_LEAVE(BHDR_UsageMasterSlaveSet) ;

	return rc ;
}


BERR_Code BHDR_UsageMasterSlaveClear(
   BHDR_Handle hHDRMaster,  BHDR_Handle hHDRSlave)
{
	BERR_Code rc = BERR_SUCCESS ;

	BDBG_ENTER(BHDR_UsageMasterSlaveClear) ;
	BDBG_OBJECT_ASSERT(hHDRMaster, BHDR_P_Handle) ;
	BDBG_OBJECT_ASSERT(hHDRSlave, BHDR_P_Handle) ;

	BHDR_InitializePacketRAM(hHDRMaster) ;
	BHDR_InitializePacketRAM(hHDRSlave) ;
	
	hHDRMaster->eUsage = BHDR_Usage_eNormal ;
	hHDRSlave->eUsage = BHDR_Usage_eNormal ;

	BDBG_LEAVE(BHDR_UsageMasterSlaveClear) ;
	return rc ;
}



/*******************************************************
PRIVATE FUNCTIONS
*******************************************************/

static void BHDR_P_VerifyHdcpMasterSlave_isr(BHDR_Handle hHDR) 
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset   ;

	uint16_t ucMasterHdcpRi, ucSlaveHdcpRi ;

	BDBG_ENTER(BHDR_P_VerifyHdcpMasterSlave_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle);
	
	hRegister = hHDR->hRegister ;
	
	ulOffset = hHDR->ulOffset ;
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDCP_STATUS + ulOffset) ;
	ucMasterHdcpRi = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_HDCP_STATUS, RX_RI) ;

	ulOffset = hHDR->hHDRSlave->ulOffset ;
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDCP_STATUS + ulOffset) ;
	ucSlaveHdcpRi = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_HDCP_STATUS, RX_RI) ;

	if (ucMasterHdcpRi != ucSlaveHdcpRi)
	{
		BDBG_WRN(("Master Ri %x is not equal to Slave Ri %x; Clear authentication",
			ucMasterHdcpRi, ucSlaveHdcpRi)) ;
		
		BHDR_P_ClearCoreAuthentication_isr(hHDR) ;
		BHDR_P_ClearCoreAuthentication_isr(hHDR->hHDRSlave) ;
	}
	else
	{
		BDBG_MSG(("Master Ri %x = Slave Ri %x; HDCP OK",
			ucMasterHdcpRi, ucSlaveHdcpRi)) ;
	}
	BDBG_LEAVE(BHDR_P_VerifyHdcpMasterSlave_isr) ;
}



const char * BHDR_P_SymbolLossSM_StateToStr(
	uint8_t state )
{
	uint8_t entries ;
	
	entries=
		sizeof(SL_TEXT) /
		sizeof(*SL_TEXT) ;
		
	if (state < entries)
		return SL_TEXT[state] ;
	else
	{
		BDBG_WRN(("Unknown Symbol Loss SM: %d; Default to Wait for HP", state)) ;
		return SL_TEXT[0] ;
	}
}



void BHDR_P_HdmiRxCoreReset_isr(BHDR_Handle hHDR)
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset ;

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_RESET_CONTROL + ulOffset ) ;

	Register |= BCHP_MASK(HDMI_RX_0_RESET_CONTROL, CORE_RESET) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_RESET_CONTROL + ulOffset , Register) ;
	
	Register &= ~ BCHP_MASK(HDMI_RX_0_RESET_CONTROL, CORE_RESET) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_RESET_CONTROL + ulOffset , Register) ;
}

/******************************************************************************
void BHDR_P_ResetFrontEndAnalogChannels_isr
Summary: Reset the Front End Analog Channels 
*******************************************************************************/
void BHDR_P_ResetFrontEndAnalogChannels_isr(BHDR_Handle hHDR) 
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset  ;
	uint8_t checkPllLock ;

	BDBG_ENTER(BHDR_P_ResetFrontEndAnalogChannels_isr) ;
	
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;
	hRegister = hHDR->hRegister ;

	/* use front end channel register offset */	
	ulOffset = hHDR->hFeChannel->ulOffset ;
	
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_STATUS_2 + ulOffset) ;
	checkPllLock = (bool) BCHP_GET_FIELD_DATA(Register, HDMI_RX_FE_0_ANALOG_STATUS_2, PLL_LOCK) ;

	if (!checkPllLock)
	{
		BDBG_WRN(("HDMI_RX_FE_%d PLL Lock: %d  IS UNLOCKED Prior to Channel Reset-_-_-_-_UNLOCKED -_-_-_-_", 
			hHDR->hFeChannel->eChannel, 	checkPllLock)) ;
	}
	else
	{
		BDBG_WRN(("HDMI_RX_FE_%d PLL Lock: %d  IS  LOCKED Prior to Channel Reset---------LOCKED-----------",
			hHDR->hFeChannel->eChannel, checkPllLock)) ;
	}

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulOffset) ;
	
	Register |= BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_0_RESET) ;
	Register |= BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_1_RESET) ;
	Register |= BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_2_RESET) ;
	
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulOffset, Register) ;
	
	Register &= ~ BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_0_RESET) ;
	Register &= ~ BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_1_RESET) ;
	Register &= ~ BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_2_RESET) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulOffset, Register) ;

	BDBG_LEAVE(BHDR_P_ResetFrontEndAnalogChannels_isr) ;

}


void BHDR_P_SymbolLossSM_ConfigureAfterClockChange_isr(BHDR_Handle hHDR, bool bClockStatus)
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset ;
	uint32_t ulFeOffset ;

	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;
	
	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset ;
	ulFeOffset = hHDR->hFeChannel->ulOffset ;

	
	/* always clear AvMute on a Hot Plug connect or disconnect */
	hHDR->AvMute = 0 ;

	if (bClockStatus) /* connected */
	{
		/* make sure the excessive interrupt is armed */
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_BCH_ECC_CFG + ulOffset) ;
		Register &= ~ BCHP_MASK(HDMI_RX_0_BCH_ECC_CFG, RDB_CLEAR_PACKET_ERRORS) ;
		BREG_Write32(hRegister,  BCHP_HDMI_RX_0_BCH_ECC_CFG + ulOffset, Register) ;
		
		
		/* Reset the I2C slave */
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_RESET_CONTROL + ulOffset) ;

		Register |= BCHP_MASK(HDMI_RX_0_RESET_CONTROL, RESET_I2C_SLAVE) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_0_RESET_CONTROL + ulOffset,  Register) ;
		
		Register &= ~ BCHP_MASK(HDMI_RX_0_RESET_CONTROL, RESET_I2C_SLAVE) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_0_RESET_CONTROL + ulOffset,  Register) ;

		/* Set KVCO to default to max frequency */
		Register = BREG_Read32(hRegister, 
			BCHP_HDMI_RX_FE_0_PLL_CTRL_0 + hHDR->hFeChannel->ulOffset) ;
		Register &= ~ BCHP_MASK(HDMI_RX_FE_0_PLL_CTRL_0, KVCO_XS) ;
		Register &= ~ BCHP_MASK(HDMI_RX_FE_0_PLL_CTRL_0, KVCO_XF) ;
		
		Register |= BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, KVCO_XS, 7) ;
		Register |= BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, KVCO_XF, 7) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_PLL_CTRL_0 + ulFeOffset, Register) ;

		/* reset input FIFO*/
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulFeOffset ) ;
		
		Register |= BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, RESET_INPUT_FIFO) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulFeOffset, Register) ;

		Register &= ~ BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, RESET_INPUT_FIFO) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulFeOffset , Register) ;
		
		BREG_Write32(hRegister, 
			BCHP_HDMI_RX_0_RESET_CONTROL + ulOffset, 0) ;

		/****************************************************************************************/
		/* Note: This routine should be called with bClockStatus == TRUE  ** AFTER ** BHDR_FE_SetPowerState */
		/*          is called with stPowerSettings->bHdmiRxPowered == TRUE.                                                      */
		/****************************************************************************************/
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulFeOffset ) ;

		if (Register != 0)
		{
			BDBG_WRN(("CH%d BHDR_ConfigureAfterHotPlug @ line %d: Enabling HDMI I2C before the PHY is out of Reset.  Reset value is 0x%08X.", 
				hHDR->eCoreId, __LINE__, Register)) ;
			BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulFeOffset , 0) ;
		}

		hHDR->SymbolLossIntrCount = 0 ;
		hHDR->eSymbolLossState = BHDR_P_SYMBOL_LOSS_SM_eWaitAfterHotPlug ;
		
		BTMR_ReadTimer_isr(hHDR->timerGeneric, &hHDR->SymbolLossIntrTimeStamp) ;

		/* report changes in state for debug purposes; eliminate repeating messages */
		if (hHDR->eSymbolLossState != hHDR->ePreviousSymbolLossState)
		{
			BDBG_WRN(("^^^ CH%d SL SM (Conf after Hotplug): State changed from %s (%d) to %s (%d)", 
				hHDR->eCoreId,
				BHDR_P_SymbolLossSM_StateToStr(hHDR->ePreviousSymbolLossState),
				hHDR->ePreviousSymbolLossState,
				BHDR_P_SymbolLossSM_StateToStr(hHDR->eSymbolLossState), 
				hHDR->eSymbolLossState)) ; 
			hHDR->ePreviousSymbolLossState = hHDR->eSymbolLossState ;
		}
 
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_PIX_CLK_CNT + 
			hHDR->hFeChannel->ulOffset) ;
		
		hHDR->bClockStopped = BCHP_GET_FIELD_DATA(Register, HDMI_RX_FE_0_PIX_CLK_CNT, CLOCK_STOPPED) ;
		hHDR->bClockStopped = false ;

		BHDR_P_HotPlugConnect_isr(hHDR);
		
	}
	else  /* Unconnected */
	{
		BHDR_P_HdmiRxCoreReset_isr(hHDR)	;
		
		/* Debug Monitoring: check the reset controller to ensure we are not in reset */
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulFeOffset ) ;
		BDBG_MSG(("CH%d FE RESET CONTROL: %x", hHDR->hFeChannel->eChannel, Register)) ;
		
		/* Debug Monitoring: Reset the I2C transaction counter */
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_I2C_FREQ_DETECTION_CNTRL1 + ulOffset) ;
		Register |= 
			BCHP_MASK(HDMI_RX_0_I2C_FREQ_DETECTION_CNTRL1, FD_RST_TRANSACTION_COUNT) ;
		Register &= 
			~ BCHP_MASK(HDMI_RX_0_I2C_FREQ_DETECTION_CNTRL1, FD_RST_TRANSACTION_COUNT) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_0_I2C_FREQ_DETECTION_CNTRL1 + ulOffset, Register) ;		

		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_DIGITAL_FRONT_END_CTL + ulOffset) ;

		/* Clear HDMI mode */
		Register |= BCHP_MASK(HDMI_RX_0_DIGITAL_FRONT_END_CTL, CLEAR_HDMI) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_0_DIGITAL_FRONT_END_CTL + ulOffset, Register) ;
		
		Register &= ~ BCHP_MASK(HDMI_RX_0_DIGITAL_FRONT_END_CTL, CLEAR_HDMI) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_0_DIGITAL_FRONT_END_CTL + ulOffset, Register) ;	

		/* Interrupts may stop before HDMI state variables can be updated; Update values here */

		/* update HdmiMode */
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_DIGITAL_FRONT_END_CFG_1 + ulOffset) ;
		hHDR->bHdmiMode = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_DIGITAL_FRONT_END_CFG_1, HDMI_MODE) ;

		/* set ClockStopped to true */
		hHDR->bClockStopped = true ;

		/* Debug Monitoring: check the reset controller to ensure we are not in reset */
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulFeOffset ) ;
		if (Register)
		{
			BDBG_WRN(("CH%d Front End (HDMI_RX_FE_0_RESET_CONTROL) held in RESET: %x", 
				hHDR->eCoreId, Register)) ;
		}

		BHDR_P_HotPlugRemove_isr(hHDR);
	}
}


void BHDR_P_SymbolLossSM_HdcpInit_isr(BHDR_Handle hHDR)
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset   ;
	
	uint8_t Ri ;
	bool bI2cEnabled ;
	uint32_t timeStamp ;

	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	/* make sure FrontEnd Channel has been attached */
	if (!hHDR->hFeChannel) 
		return ;
	
	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset ;


	BTMR_ReadTimer_isr(hHDR->timerGeneric, &timeStamp) ;

	hHDR->RequestKeysIntrCount++ ;
	BDBG_WRN(("CH%d HDCP Request #%d was %d ms since last request\n",
		hHDR->eCoreId, 
		hHDR->RequestKeysIntrCount++ , (timeStamp - hHDR->RequestKeysTimeStamp) / 1000)) ;
	hHDR->RequestKeysTimeStamp = timeStamp ;
	
	hHDR->bFirstPj = true ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDCP_RX_I2C_MISC_CFG_2 + ulOffset) ;
	bI2cEnabled = BCHP_GET_FIELD_DATA(Register,  
		HDMI_RX_0_HDCP_RX_I2C_MISC_CFG_2, I2C_ENABLE) ;
			
	if (bI2cEnabled)
	{
		hHDR->AuthReqCnt++ ;
		BDBG_WRN(("CH%d Incremented AuthReqCnt to %d", 
			hHDR->eCoreId, hHDR->AuthReqCnt)) ;
		
		/* Check Ri */
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDCP_MON_RI_PJ + ulOffset) ;
		Ri = BCHP_GET_FIELD_DATA(Register,  HDMI_RX_0_HDCP_MON_RI_PJ, HDCP_RX_MON_RI) ;
		BDBG_MSG(("CH%d HDCP Ri: %d",  hHDR->eCoreId, Ri)) ;
		if (!Ri)
			hHDR->R0Equal0Count++ ;
		else
			hHDR->R0Equal0Count = 0 ;

		if (hHDR->AuthReqCnt >= 4)
		{
			BHDR_P_ResetFrontEndAnalogChannels_isr(hHDR) ;

			hHDR->AuthReqCnt = 0 ;
			hHDR->R0Equal0Count = 0 ;
			BDBG_WRN(("--->> CH%d Cleared AuthReqCnt (A) to %d", 
				hHDR->eCoreId, hHDR->AuthReqCnt)) ;
		}
		else if (hHDR->R0Equal0Count >= 2)
		{
			BHDR_P_ResetFrontEndAnalogChannels_isr(hHDR) ;

			hHDR->AuthReqCnt = 0 ;
			hHDR->R0Equal0Count = 0 ;
			BDBG_WRN(("--->> CH%d Cleared R0Equal0Count to %d", 
				hHDR->eCoreId, hHDR->R0Equal0Count)) ;
		}
	} 
	else
	{	/* This happens when hotplug is low */
		hHDR->AuthReqCnt = 0 ;
		hHDR->R0Equal0Count = 0;
		BDBG_WRN(("CH%d Hotplug is low", hHDR->eCoreId));
	}
	
	/* make sure the excessive interrupt is armed */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_BCH_ECC_CFG + ulOffset) ;
	Register &= ~ BCHP_MASK(HDMI_RX_0_BCH_ECC_CFG, RDB_CLEAR_PACKET_ERRORS) ;
	BREG_Write32(hRegister,  BCHP_HDMI_RX_0_BCH_ECC_CFG + ulOffset, Register) ;
}


/******************************************************************************
void BHDR_P_SymbolLossSM_Initialize
Summary: Initialize Symbol Loss State machine variables 
*******************************************************************************/
void	BHDR_P_SymbolLossSM_Initialize(BHDR_Handle hHDR) 
{
	hHDR->SymbolLossIntrCount = 0 ;
	hHDR->ePreviousSymbolLossState = BHDR_P_SYMBOL_LOSS_SM_eWaitAfterHotPlug ;
	hHDR->eSymbolLossState = BHDR_P_SYMBOL_LOSS_SM_eWaitAfterHotPlug ;
} 


/******************************************************************************
void BHDR_P_SymbolLossSM_isr
Summary: State machine to attempt recovery from SYmbol Loss errors
*******************************************************************************/
void BHDR_P_SymbolLossSM_isr(BHDR_Handle hHDR) 
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset, ulFeOffset;
	uint32_t timeStamp ;
	bool bSymbolLock ;
	bool bClockStopped ;

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset ;
	ulFeOffset = hHDR->hFeChannel->ulOffset ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_PIX_CLK_CNT + ulFeOffset) ;
	bClockStopped = BCHP_GET_FIELD_DATA(Register, HDMI_RX_FE_0_PIX_CLK_CNT, CLOCK_STOPPED) ;

	if ((bClockStopped ))
	{
		hHDR->AuthReqCnt = 0 ;
		hHDR->bFirstPj = true ;
		hHDR->R0Equal0Count = 0;
		hHDR->eSymbolLossState = BHDR_P_SYMBOL_LOSS_SM_eLocked ;

		/* report clock stop only once */
		if (hHDR->bClockStopped != bClockStopped)
		{
			BDBG_MSG(("^^^CH%d SL SM: Top level sw should pulse HDMI hotplug to allow fastest recovery\n", 
				hHDR->eCoreId)) ;
			BDBG_MSG(("^^^CH%d SL SM bClockStopped changed from %d to %d", 
				hHDR->eCoreId, hHDR->bClockStopped, bClockStopped)) ;
		
			BHDR_P_SymbolLossSM_ConfigureAfterClockChange_isr(hHDR, !bClockStopped) ;
		}		

		hHDR->AuthReqCnt = 0 ;
		BTMR_ReadTimer_isr(hHDR->timerGeneric, &hHDR->ClockStoppedTimer) ;
	}	
	hHDR->bClockStopped = bClockStopped ;
	
	if (bClockStopped) 
	{
		if (!hHDR->bClockStopped)
		{
			Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDCP_MON_RI_PJ + ulOffset) ;
			BDBG_MSG(("^^^CH%d SL SM: CLOCK HAS STOPPED DO NOT DISABLE I2C !!!!!!!!!!!!!!!!!! %X",
				hHDR->eCoreId, Register)) ;
		}
	}


	if ((!hHDR->hFeChannel->bTxDeviceAttached) || (hHDR->bClockStopped))
		return ;
	
	/* check for Symbol Lock */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_DIGITAL_FRONT_END_TST_CFG + ulOffset) ;
	bSymbolLock = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_DIGITAL_FRONT_END_TST_CFG, SYMBOL_LOCK) ;

	/* CLEAR_AUTH  set lock only once */
	if ((bSymbolLock) 
	&& ((hHDR->eSymbolLossState != BHDR_P_SYMBOL_LOSS_SM_eLocked)  /* CLEAR_AUTH */
	&&  (hHDR->eSymbolLossState != BHDR_P_SYMBOL_LOSS_SM_eClrDCThreshold)))
	{
		BAVC_HDMI_GcpData GcpData;	

		/* report the status */		
		BDBG_WRN(("^^^CH%d SL SM: Symbol Lock!; Reset SymbolLossIntrCount (%d)", 
			hHDR->eCoreId, hHDR->SymbolLossIntrCount)) ;
		
		/* Update the Symbol Loss State to LOCKED */
		hHDR->eSymbolLossState = BHDR_P_SYMBOL_LOSS_SM_eLocked ;		
		
		BHDR_GetGeneralControlPacketData(hHDR, &GcpData);
		if (!GcpData.eColorDepth)
		{
			BDBG_WRN(("^^^CH%d SL SM: Color Depth: %d; ReInit Packet RAM", 
				hHDR->eCoreId, GcpData.eColorDepth)) ;
			BHDR_P_InitializePacketRAM_isr(hHDR) ;
		}
		return ;
	}


	BTMR_ReadTimer_isr(hHDR->timerGeneric, &timeStamp) ;
	
	/* Check Symbol Loss status */
	switch (hHDR->eSymbolLossState)
	{
	default :
		BDBG_WRN(("^^^CH%d SL SM: UNKNOWN SYMBOL LOSS STATE: %d", 
			hHDR->eCoreId, hHDR->eSymbolLossState)) ;
		
		/* go to  to first state */
		hHDR->eSymbolLossState = BHDR_P_SYMBOL_LOSS_SM_eLocked ;
				
		break ;
		
	case BHDR_P_SYMBOL_LOSS_SM_eWaitAfterHotPlug :
		hHDR->SymbolLossIntrCount = 0 ;

		/* TIME_CHANGE from 800 to 200 */
		/* allow time for hw to settle */
		if ((timeStamp - hHDR->SymbolLossIntrTimeStamp) > (200 * BHDR_P_MILLISECOND)) 
		{
			hHDR->SymbolLossIntrTimeStamp = timeStamp ;
			hHDR->eSymbolLossState = BHDR_P_SYMBOL_LOSS_SM_eClrDCThreshold ;
		}
		
		break ;
		
 	case BHDR_P_SYMBOL_LOSS_SM_eLocked :
		/*
		if we get to this state; and the SM is running...
		we have lost lock; but let's make sure we continually see interrupts before resetting  
		the HDMI Rx FE channels 
		*/
		if (!hHDR->SymbolLossIntrCount)
			return ;
		
		if (hHDR->SymbolLossIntrCount  < BHDR_P_MAX_SYMBOL_LOSS_COUNT)
		{
			BDBG_WRN(("^^^CH%d SL SM: SYMBOL_LOSS! Wait %d MAX ints, before leaving '%s' state",
				hHDR->eCoreId, BHDR_P_MAX_SYMBOL_LOSS_COUNT, 
				BHDR_P_SymbolLossSM_StateToStr(hHDR->eSymbolLossState))) ;
			break  ;
		}
		
		BDBG_WRN(("^^^CH%d SL SM: %d LOSS Interrupts since LOCK; try Reset FE Analog Channels",
			hHDR->eCoreId, hHDR->SymbolLossIntrCount));
	
		BHDR_P_ResetFrontEndAnalogChannels_isr(hHDR) ;

		hHDR->SymbolLossIntrCount = 0;
		hHDR->SymbolLossIntrTimeStamp = timeStamp ;
		hHDR->eSymbolLossState = BHDR_P_SYMBOL_LOSS_SM_eResetPll ;
		
		break ;
		
		
	case BHDR_P_SYMBOL_LOSS_SM_eResetPll :
		/* 
		before resetting the PLL,...
		allow time for the Front End Analog Channel Reset to work 
		*/
		if ((timeStamp - hHDR->SymbolLossIntrTimeStamp) < (50 * BHDR_P_MILLISECOND)) 
		{
			break ;
		}

		BDBG_WRN(("^^^CH%d SL SM: %d LOSS Interrupts since Reset FE Channels; try Reset HDMI Rx FE",
			hHDR->eCoreId, hHDR->SymbolLossIntrCount));
		
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulFeOffset) ;

		Register |= BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, RESET_HDMI_FE) ;
		BREG_Write32(hRegister , BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulFeOffset, Register) ;
		
		Register &= ~ BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, RESET_HDMI_FE) ;
		BREG_Write32(hRegister , BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulFeOffset, Register) ;

		hHDR->SymbolLossIntrCount = 0 ;
		hHDR->SymbolLossIntrTimeStamp = timeStamp ;
		hHDR->eSymbolLossState = BHDR_P_SYMBOL_LOSS_SM_eFinal ;
		
		break ;

	case BHDR_P_SYMBOL_LOSS_SM_eFinal :
		/* allow time for the HDMI Front End Reset  to work */
		if ((timeStamp - hHDR->SymbolLossIntrTimeStamp) < (50 * BHDR_P_MILLISECOND) )
		{
			break ;
		}

		/* Symbol Lock has not recovered; retry state machine */
		BDBG_WRN(("^^^CH%d SL SM: SYMBOL LOSS did not recover; retry SM...", 
			hHDR->eCoreId)) ;
		
		hHDR->SymbolLossIntrCount = 0 ;
		hHDR->SymbolLossIntrTimeStamp = timeStamp ;
		hHDR->eSymbolLossState = BHDR_P_SYMBOL_LOSS_SM_eLocked ;
		
		break ;
		
	case BHDR_P_SYMBOL_LOSS_SM_eClrDCThreshold :
		BHDR_P_InitializePacketRAM_isr(hHDR) ;
		
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDMI_13_FEATURES_CFG_1 + ulOffset) ;
		Register &= ~ BCHP_MASK(HDMI_RX_0_HDMI_13_FEATURES_CFG_1, GCP_CD_FRAME_COUNT_THRESHOLD) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDMI_13_FEATURES_CFG_1 + ulOffset,  Register) ;
			
		hHDR->eSymbolLossState = BHDR_P_SYMBOL_LOSS_SM_eLocked ;

		hHDR->SymbolLossIntrCount = 0;
		hHDR->SymbolLossIntrTimeStamp = timeStamp ;

		hHDR->bClockStopped = false  ;
		
		break ;
		
	case BHDR_P_SYMBOL_LOSS_SM_eSetDCThreshold :
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDMI_13_FEATURES_CFG_1 + ulOffset) ;
		Register &= ~ BCHP_MASK(HDMI_RX_0_HDMI_13_FEATURES_CFG_1, GCP_CD_FRAME_COUNT_THRESHOLD) ;
		Register |= BCHP_FIELD_DATA(HDMI_RX_0_HDMI_13_FEATURES_CFG_1, GCP_CD_FRAME_COUNT_THRESHOLD, 4) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDMI_13_FEATURES_CFG_1 + ulOffset, Register) ;
		
		hHDR->eSymbolLossState = BHDR_P_SYMBOL_LOSS_SM_eLocked ;
				
		break ;
	}
	
#if BDBG_DEBUG_BUILD	
	/* report changes in state for debug purposes; eliminate repeating messages */
	if (hHDR->eSymbolLossState != hHDR->ePreviousSymbolLossState)
	{
		if ((hHDR->eSymbolLossState < BHDR_P_SYMBOL_LOSS_SM_eCount)
		&& (hHDR->ePreviousSymbolLossState < BHDR_P_SYMBOL_LOSS_SM_eCount))
		{
			BDBG_WRN(("^^^CH%d SL SM: State changed from %s (%d) to %s (%d)", 
				hHDR->eCoreId,
				BHDR_P_SymbolLossSM_StateToStr(hHDR->ePreviousSymbolLossState), 
				hHDR->ePreviousSymbolLossState,
				BHDR_P_SymbolLossSM_StateToStr(hHDR->eSymbolLossState), 
				hHDR->eSymbolLossState)) ; 
		}
		hHDR->ePreviousSymbolLossState = hHDR->eSymbolLossState ;
	}
#endif	
}


#endif

