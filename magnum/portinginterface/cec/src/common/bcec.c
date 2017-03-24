/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

#include "bstd.h"
#include "berr_ids.h"

#include "bcec.h"
#include "bcec_priv.h"
#include "bcec_config.h"
#include "bavc_hdmi.h"


#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif


BDBG_MODULE(BCEC) ;
BDBG_OBJECT_ID(BCEC_P_Handle);


#define BCEC_CHECK_RC( rc, func )				  \
do												  \
{												  \
	if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
	{											  \
		goto done;								  \
	}											  \
} while(0)


#if BCEC_CONFIG_DEBUG_OPCODE

BCEC_OpcodeTextTable opCodeTextTable[BCEC_MAX_OPCODES] = {
{ BCEC_OpCode_FeatureAbort, 			"FeatureAbort" },
{ BCEC_OpCode_ImageViewOn,				"ImageViewOn" },
{ BCEC_OpCode_TunerStepIncrement,		"TunerStepIncrement" },
{ BCEC_OpCode_TunerStepDecrement,		"TunerStepDecrement" },
{ BCEC_OpCode_TunerDeviceStatus,		"TunerDeviceStatus" },
{ BCEC_OpCode_DiveTunerDeviceStatus,	"DiveTunerDeviceStatus" },
{ BCEC_OpCode_RecordOn, 				"RecordOn" },
{ BCEC_OpCode_RecordStatus, 			"RecordStatus" },
{ BCEC_OpCode_RecordOff,				"RecordOff" },
{ BCEC_OpCode_TextViewOn,				"TextViewOn" },
{ BCEC_OpCode_RecordTVScreen,			"RecordTVScreen" },
{ BCEC_OpCode_GiveDeckStatus,			"GiveDeckStatus" },
{ BCEC_OpCode_DeckStatus,				"DeckStatus" },
{ BCEC_OpCode_SetMenuLanguage,			"SetMenuLanguage" },
{ BCEC_OpCode_ClearAnalogueTimer,		"ClearAnalogueTimer" },
{ BCEC_OpCode_SetAnalogueTimer, 		"SetAnalogueTimer" },
{ BCEC_OpCode_TimerStatus,				"TimerStatus" },
{ BCEC_OpCode_Standby,					"Standby" },
{ BCEC_OpCode_Play, 					"Play" },
{ BCEC_OpCode_DeckControl,				"DeckControl" },
{ BCEC_OpCode_TimerClearedStatus,		"TimerClearedStatus" },
{ BCEC_OpCode_UserControlPressed,		"UserControlPressed" },
{ BCEC_OpCode_UserControlReleased,		"UserControlReleased" },
{ BCEC_OpCode_GiveOSDName,				"GiveOSDName" },
{ BCEC_OpCode_SetOSDName,				"SetOSDName" },
{ BCEC_OpCode_SetOSDString, 			"SetOSDString" },
{ BCEC_OpCode_SetTimerProgramTitle, 	"SetTimerProgramTitle" },
{ BCEC_OpCode_SystemAudioModeRequest,	"SystemAudioModeRequest" },
{ BCEC_OpCode_GiveAudioStatus,			"GiveAudioStatus" },
{ BCEC_OpCode_SetSystemAudioMode,		"SetSystemAudioMode" },
{ BCEC_OpCode_ReportAudioStatus,		"ReportAudioStatus" },
{ BCEC_OpCode_GiveSystemAudioModeStatus,"GiveSystemAudioModeStatus" },
{ BCEC_OpCode_SystemAudioModeStatus,	"SystemAudioModeStatus" },
{ BCEC_OpCode_RoutingChange,			"RoutingChange" },
{ BCEC_OpCode_RoutingInformation,		"RoutingInformation" },
{ BCEC_OpCode_ActiveSource, 			"ActiveSource" },
{ BCEC_OpCode_GivePhysicalAddress,		"GivePhysicalAddress" },
{ BCEC_OpCode_ReportPhysicalAddress,	"ReportPhysicalAddress" },
{ BCEC_OpCode_RequestActiveSource,		"RequestActiveSource" },
{ BCEC_OpCode_SetStreamPath,			"SetStreamPath" },
{ BCEC_OpCode_DeviceVendorID,			"DeviceVendorID" },
{ BCEC_OpCode_VendorCommand,			"VendorCommand" },
{ BCEC_OpCode_VendorRemoteButtonDown,	"VendorRemoteButtonDown" },
{ BCEC_OpCode_VendorRemoteButtonUp, 	"VendorRemoteButtonUp" },
{ BCEC_OpCode_GiveDeviceVendorID,		"GiveDeviceVendorID" },
{ BCEC_OpCode_MenuRequest,				"MenuRequest" },
{ BCEC_OpCode_MenuStatus,				"MenuStatus" },
{ BCEC_OpCode_GiveDevicePowerStatus,	"GiveDevicePowerStatus" },
{ BCEC_OpCode_ReportPowerStatus,		"ReportPowerStatus" },
{ BCEC_OpCode_GetMenuLanguage,			"GetMenuLanguage" },
{ BCEC_OpCode_SelectAnalogueService,	"SelectAnalogueService" },
{ BCEC_OpCode_SelectDigitalService, 	"SelectDigitalService" },
{ BCEC_OpCode_SetDigitalTimer,			"SetDigitalTimer" },
{ BCEC_OpCode_ClearDigitalTimer,		"ClearDigitalTimer" },
{ BCEC_OpCode_SetAudioRate, 			"SetAudioRate" },
{ BCEC_OpCode_InActiveSource,			"InActiveSource" },
{ BCEC_OpCode_CECVersion,				"CECVersion" },
{ BCEC_OpCode_GetCECVersion,			"GetCECVersion" },
{ BCEC_OpCode_VendorCommandWithID,		"VendorCommandWithID" },
{ BCEC_OpCode_ClearExternalTimer,		"ClearExternalTimer" },
{ BCEC_OpCode_SetExternalTimer, 		"SetExternalTimer" },
{ BCEC_OpCode_Abort,					"Abort" }
} ;

static char unsupportedOpCode[] = "Unsupported OpCode" ;
static const char *BCEC_OpcodeToString(uint8_t cecOpCode)
{
	uint8_t i ;

	i = 0 ;
	while  (i < BCEC_MAX_OPCODES)
	{
		if (opCodeTextTable[i].opCode == cecOpCode)
			return opCodeTextTable[i].opText ;
		i++ ;
	}
	return unsupportedOpCode ;
}
#endif

#if BCEC_CONFIG_ENABLE_COMPLIANCE_TEST_WORKAROUND
#define BCEC_START_BIT_XMIT_TIME	5	/* round up 4.5 ms */
#define BCEC_PACKET_SIZE	10	/* bits -- message + EOM + ACK */
#define BCEC_NOMINAL_BIT_TIME	3	/* round up 2.4 ms */
#endif



/******************************************************************************
Summary:
Interrupt Callback Table to describe interrupt Names, ISRs, and Masks
*******************************************************************************/
typedef struct BCEC_P_InterruptCbTable
{
	BINT_Id 		  IntrId;
	BINT_CallbackFunc pfCallback;
	int 			  iParam2;
	bool			  enable ; /* debug purposes */
} BCEC_P_InterruptCbTable ;


#define BCEC_INT_CB_DEF(intr,  id, enable) \
	{intr, BCEC_P_HandleInterrupt_isr, id, enable},


/* CEC Tx interrupts */
static const BCEC_P_InterruptCbTable BCEC_Interrupts[] =
{
#if BCEC_CONFIG_DUAL_INTERRUPT
	BCEC_INT_CB_DEF(BCHP_INT_ID_CEC_RCVD_INTR_TX,
		BCEC_INTR_eRECEIVED, 1)
	BCEC_INT_CB_DEF(BCHP_INT_ID_CEC_SENT_INTR_TX,
		BCEC_INTR_eSENT, 1)

#elif BCEC_CONFIG_SINGLE_INTERRUPT
	BCEC_INT_CB_DEF(BCHP_INT_ID_CEC_INTR_TX,
		BCEC_INTR_eCEC, 1)

#else	/* legacy platforms 65nm and earlier */
	BCEC_INT_CB_DEF(BCHP_INT_ID_CEC_INTR,
		BCEC_INTR_eCEC, 1)
#endif
} ;

/* CEC Rx interrupts */
#if BCEC_CONFIG_HAS_HDMI_RX
static const BCEC_P_InterruptCbTable BCEC_RxInterrupts[] =
{
	BCEC_INT_CB_DEF(BCHP_INT_ID_CEC_RCVD_INTR_RX,
		BCEC_INTR_eRECEIVED, 1)
	BCEC_INT_CB_DEF(BCHP_INT_ID_CEC_SENT_INTR_RX,
		BCEC_INTR_eSENT, 1)
} ;
#endif


/********************
*	PRIVATE APIs 	*
*********************/
static void BCEC_P_Initialize(BCEC_Handle hCEC, uint32_t ulOffset)
{
	BREG_Handle hRegister ;
	uint32_t Register;

	BDBG_ENTER(BCEC_P_Initialize) ;

	hRegister = hCEC->stDependencies.hRegister ;

	/*************************
	** adjust default CEC timing **
	**************************/
	Register = BREG_Read32(hRegister, REGADDR_CEC_CNTRL_1 + ulOffset) ;
	Register &= ~ BCHP_MASK_CEC_CNTRL(CEC_CNTRL_1, DIV_CLK_CNT);
	Register |= BCHP_FIELD_DATA(REGNAME_CEC_CNTRL_1, DIV_CLK_CNT,
				BCEC_CNTRL_1_DIV_CLK_CNT_VALUE);
	BREG_Write32(hRegister, REGADDR_CEC_CNTRL_1 + ulOffset, Register) ;

	Register = BREG_Read32(hRegister, REGADDR_CEC_CNTRL_2 + ulOffset) ;
	Register &=
		~(BCHP_MASK_CEC_CNTRL(CEC_CNTRL_2, CNT_TO_400_US)
		| BCHP_MASK_CEC_CNTRL(CEC_CNTRL_2, CNT_TO_600_US)
		| BCHP_MASK_CEC_CNTRL(CEC_CNTRL_2, CNT_TO_800_US)
		| BCHP_MASK_CEC_CNTRL(CEC_CNTRL_2, CNT_TO_1300_US)
		| BCHP_MASK_CEC_CNTRL(CEC_CNTRL_2, CNT_TO_1500_US));

	Register |=
		  BCHP_FIELD_DATA(REGNAME_CEC_CNTRL_2, CNT_TO_400_US,
			BCEC_CNTRL_2_CNT_TO_400_US_VALUE)
		| BCHP_FIELD_DATA(REGNAME_CEC_CNTRL_2, CNT_TO_600_US,
			BCEC_CNTRL_2_CNT_TO_600_US_VALUE)
		| BCHP_FIELD_DATA(REGNAME_CEC_CNTRL_2, CNT_TO_800_US,
			BCEC_CNTRL_2_CNT_TO_800_US_VALUE)
		| BCHP_FIELD_DATA(REGNAME_CEC_CNTRL_2, CNT_TO_1300_US,
			BCEC_CNTRL_2_CNT_TO_1300_US_VALUE)
		| BCHP_FIELD_DATA(REGNAME_CEC_CNTRL_2, CNT_TO_1500_US,
			BCEC_CNTRL_2_CNT_TO_1500_US_VALUE);
	BREG_Write32(hRegister, REGADDR_CEC_CNTRL_2 + ulOffset, Register) ;


	Register = BREG_Read32(hRegister, REGADDR_CEC_CNTRL_3 + ulOffset) ;
	Register &=
		~(BCHP_MASK_CEC_CNTRL(CEC_CNTRL_3, CNT_TO_1700_US)
		| BCHP_MASK_CEC_CNTRL(CEC_CNTRL_3, CNT_TO_2050_US)
		| BCHP_MASK_CEC_CNTRL(CEC_CNTRL_3, CNT_TO_2400_US)
		| BCHP_MASK_CEC_CNTRL(CEC_CNTRL_3, CNT_TO_2750_US));

	Register |=
		  BCHP_FIELD_DATA(REGNAME_CEC_CNTRL_3, CNT_TO_1700_US,
			BCEC_CNTRL_3_CNT_TO_1700_US_VALUE)
		| BCHP_FIELD_DATA(REGNAME_CEC_CNTRL_3, CNT_TO_2050_US,
			BCEC_CNTRL_3_CNT_TO_2050_US_VALUE)
		| BCHP_FIELD_DATA(REGNAME_CEC_CNTRL_3, CNT_TO_2400_US,
			BCEC_CNTRL_3_CNT_TO_2400_US_VALUE)
		| BCHP_FIELD_DATA(REGNAME_CEC_CNTRL_3, CNT_TO_2750_US,
			BCEC_CNTRL_3_CNT_TO_2750_US_VALUE);
	BREG_Write32(hRegister, REGADDR_CEC_CNTRL_3 + ulOffset, Register) ;


	Register = BREG_Read32(hRegister, REGADDR_CEC_CNTRL_4 + ulOffset) ;
	Register &=
		~(BCHP_MASK_CEC_CNTRL(CEC_CNTRL_4, CNT_TO_3500_US)
		| BCHP_MASK_CEC_CNTRL(CEC_CNTRL_4, CNT_TO_3600_US)
		| BCHP_MASK_CEC_CNTRL(CEC_CNTRL_4, CNT_TO_3900_US)
		| BCHP_MASK_CEC_CNTRL(CEC_CNTRL_4, CNT_TO_4300_US));

	Register |=
		  BCHP_FIELD_DATA(REGNAME_CEC_CNTRL_4, CNT_TO_3500_US,
			BCEC_CNTRL_4_CNT_TO_3500_US_VALUE)
		| BCHP_FIELD_DATA(REGNAME_CEC_CNTRL_4, CNT_TO_3600_US,
			BCEC_CNTRL_4_CNT_TO_3600_US_VALUE)
		| BCHP_FIELD_DATA(REGNAME_CEC_CNTRL_4, CNT_TO_3900_US,
			BCEC_CNTRL_4_CNT_TO_3900_US_VALUE)
		| BCHP_FIELD_DATA(REGNAME_CEC_CNTRL_4, CNT_TO_4300_US,
			BCEC_CNTRL_4_CNT_TO_4300_US_VALUE);
	BREG_Write32(hRegister, REGADDR_CEC_CNTRL_4 + ulOffset, Register) ;


	Register = BREG_Read32(hRegister, REGADDR_CEC_CNTRL_5 + ulOffset) ;
	Register &=
		~(BCHP_MASK_CEC_CNTRL(CEC_CNTRL_5, CNT_TO_4500_US)
		| BCHP_MASK_CEC_CNTRL(CEC_CNTRL_5, CNT_TO_4700_US));

	Register |=
		  BCHP_FIELD_DATA(REGNAME_CEC_CNTRL_5, CNT_TO_4500_US,
			BCEC_CNTRL_5_CNT_TO_4500_US_VALUE)
		| BCHP_FIELD_DATA(REGNAME_CEC_CNTRL_5, CNT_TO_4700_US,
				BCEC_CNTRL_5_CNT_TO_4700_US_VALUE) ;
	BREG_Write32(hRegister, REGADDR_CEC_CNTRL_5 + ulOffset, Register) ;


#if BCEC_CONFIG_DEBUG_CEC_TIMING
	BDBG_WRN(("CNT_TO_400_US_VALUE %d", BCEC_CNTRL_2_CNT_TO_400_US_VALUE )) ;
	BDBG_WRN(("CNT_TO_600_US_VALUE %d", BCEC_CNTRL_2_CNT_TO_600_US_VALUE )) ;
	BDBG_WRN(("CNT_TO_800_US_VALUE %d", BCEC_CNTRL_2_CNT_TO_800_US_VALUE )) ;
	BDBG_WRN(("CNT_TO_1300_US_VALUE %d", BCEC_CNTRL_2_CNT_TO_1300_US_VALUE)) ;
	BDBG_WRN(("CNT_TO_1500_US_VALUE %d", BCEC_CNTRL_2_CNT_TO_1500_US_VALUE)) ;


	BDBG_WRN(("CNT_TO_1700_US_VALUE %d", BCEC_CNTRL_3_CNT_TO_1700_US_VALUE));
	BDBG_WRN(("CNT_TO_2050_US_VALUE %d", BCEC_CNTRL_3_CNT_TO_2050_US_VALUE)) ;
	BDBG_WRN(("CNT_TO_2400_US_VALUE %d", BCEC_CNTRL_3_CNT_TO_2400_US_VALUE)) ;
	BDBG_WRN(("CNT_TO_2750_US_VALUE %d", BCEC_CNTRL_3_CNT_TO_2750_US_VALUE)) ;

	BDBG_WRN(("CNT_TO_3500_US_VALUE %d", BCEC_CNTRL_4_CNT_TO_3500_US_VALUE)) ;
	BDBG_WRN(("CNT_TO_3600_US_VALUE %d", BCEC_CNTRL_4_CNT_TO_3600_US_VALUE)) ;
	BDBG_WRN(("CNT_TO_3900_US_VALUE %d", BCEC_CNTRL_4_CNT_TO_3900_US_VALUE)) ;
	BDBG_WRN(("CNT_TO_4300_US_VALUE", BCEC_CNTRL_4_CNT_TO_4300_US_VALUE)) ;


	BDBG_WRN(("CNT_TO_4500_US_VALUE %d", BCEC_CNTRL_5_CNT_TO_4500_US_VALUE)) ;
	BDBG_WRN(("CNT_TO_4700_US_VALUE %d", BCEC_CNTRL_5_CNT_TO_4700_US_VALUE)) ;
#endif

	/* set flag to indicate first CEC message */
	hCEC->firstCecMessage = true ;

	BDBG_LEAVE(BCEC_P_Initialize) ;
}

#if BCEC_CONFIG_HAS_HDMI_RX
static void BCEC_P_DisableUnusedCecCore(BREG_Handle hRegister, BCEC_Hdmi eCecHdmiPath)
{
	uint32_t Register;
	uint32_t ulOffset = (BCHP_AON_HDMI_RX_REG_START - BCHP_AON_HDMI_TX_REG_START);


	/***************
	  * Make sure to disable the CEC core that is not in used
	  ***************/
	if (eCecHdmiPath == BCEC_Hdmi_eTx0)
	{
		/* Disable CEC Rx core - not in used */
		Register = BREG_Read32(hRegister, REGADDR_CEC_CNTRL_5 + ulOffset) ;
		Register |= BCHP_MASK_CEC_CNTRL(CEC_CNTRL_5, DISABLE_CEC_TX)
					| BCHP_MASK_CEC_CNTRL(CEC_CNTRL_5, DISABLE_CEC_RX);
		BREG_Write32(hRegister, REGADDR_CEC_CNTRL_5 + ulOffset, Register) ;

		/* And make sure CEC Tx core is enabled */
		Register = BREG_Read32(hRegister, REGADDR_CEC_CNTRL_5) ;
		Register &= ~(BCHP_MASK_CEC_CNTRL(CEC_CNTRL_5, DISABLE_CEC_TX)
					| BCHP_MASK_CEC_CNTRL(CEC_CNTRL_5, DISABLE_CEC_RX));
		BREG_Write32(hRegister, REGADDR_CEC_CNTRL_5, Register) ;
	}
	else
	{
		/* Disable CEC Tx core - not in used */
		Register = BREG_Read32(hRegister, REGADDR_CEC_CNTRL_5) ;
		Register |= BCHP_MASK_CEC_CNTRL(CEC_CNTRL_5, DISABLE_CEC_TX)
					| BCHP_MASK_CEC_CNTRL(CEC_CNTRL_5, DISABLE_CEC_RX);
		BREG_Write32(hRegister, REGADDR_CEC_CNTRL_5, Register) ;


		/* And make sure CEC Rx core is enabled */
		Register = BREG_Read32(hRegister, REGADDR_CEC_CNTRL_5 + ulOffset) ;
		Register &= ~(BCHP_MASK_CEC_CNTRL(CEC_CNTRL_5, DISABLE_CEC_TX)
					| BCHP_MASK_CEC_CNTRL(CEC_CNTRL_5, DISABLE_CEC_RX));
		BREG_Write32(hRegister, REGADDR_CEC_CNTRL_5 + ulOffset, Register) ;
	}
}
#endif

/***************************************************************************
BERR_Code BCEC_P_ResetCore_isr
Summary: Reset CEC core
****************************************************************************/
static void BCEC_P_ResetCore_isr (BCEC_Handle hCEC, bool bReset)
{
	uint32_t Register, ulOffset;
	BREG_Handle hRegister;

	hRegister = hCEC->stDependencies.hRegister ;
	ulOffset = hCEC->ulRegisterOffset;

	Register = BREG_Read32(hRegister, REGADDR_CEC_CNTRL_5 + ulOffset) ;

#if BCEC_CONFIG_40NM_SUPPORT || BCEC_CONFIG_28NM_SUPPORT
	Register &= ~(
		  BCHP_MASK_DVP(CEC_CNTRL_5, CEC_TX_SW_INIT)
		| BCHP_MASK_DVP(CEC_CNTRL_5, CEC_RX_SW_INIT));

	Register |=
		  BCHP_FIELD_DATA(REGNAME_CEC_CNTRL_5, CEC_TX_SW_INIT, bReset)
		| BCHP_FIELD_DATA(REGNAME_CEC_CNTRL_5, CEC_RX_SW_INIT, bReset);

#else
	Register &= ~(
		  BCHP_MASK_CEC_CNTRL(CEC_CNTRL_5, CEC_TX_SW_RESET)
		| BCHP_MASK_CEC_CNTRL(CEC_CNTRL_5, CEC_RX_SW_RESET));

	Register |=
		  BCHP_FIELD_DATA(REGNAME_CEC_CNTRL_5, CEC_TX_SW_RESET, bReset)
		| BCHP_FIELD_DATA(REGNAME_CEC_CNTRL_5, CEC_RX_SW_RESET, bReset);
#endif

	BREG_Write32(hRegister, REGADDR_CEC_CNTRL_5 + ulOffset, Register) ;


#ifdef BCEC_CONFIG_CEC_USE_PAD_SW_RESET
	Register = BREG_Read32(hRegister, REGADDR_CEC_CNTRL_5 + ulOffset) ;
	Register &= ~(BCHP_MASK_CEC_CNTRL(CEC_CNTRL_5, CEC_PAD_SW_RESET));

	Register |= BCHP_FIELD_DATA(REGNAME_CEC_CNTRL_5, CEC_PAD_SW_RESET, bReset);
	BREG_Write32(hRegister, REGADDR_CEC_CNTRL_5 + ulOffset, Register) ;
#endif

	return;
}


/***************************************************************************
BERR_Code BCEC_P_EnableReceive_isr
Summary: Enable Receive - isr
****************************************************************************/
static void BCEC_P_EnableReceive_isr(
	BCEC_Handle hCEC	 /* [in] CEC handle */
)
{
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset ;

	BDBG_ENTER(BCEC_P_EnableReceive_isr);

	hRegister = hCEC->stDependencies.hRegister ;
	ulOffset = hCEC->ulRegisterOffset;

	/* Enable Receive */
	Register = BREG_Read32(hRegister, REGADDR_CEC_CNTRL_1 + ulOffset) ;

	Register |= BCHP_MASK_CEC_CNTRL(CEC_CNTRL_1, CLEAR_RECEIVE_OFF) ;
	BREG_Write32(hRegister, REGADDR_CEC_CNTRL_1 + ulOffset, Register) ;  /* Wr 1 */

	Register &= ~ BCHP_MASK_CEC_CNTRL(CEC_CNTRL_1, CLEAR_RECEIVE_OFF) ;
	BREG_Write32(hRegister, REGADDR_CEC_CNTRL_1 + ulOffset, Register) ;  /* Wr 0 */

	BDBG_LEAVE(BCEC_P_EnableReceive_isr);
	return ;
}


#if BCEC_CONFIG_ENABLE_COMPLIANCE_TEST_WORKAROUND
static void BCEC_P_TimerExpiration_isr (BCEC_Handle hCEC, int parm2)
{
	uint32_t Register, ulOffset;
	uint8_t i;
	BREG_Handle hRegister;

	BDBG_ENTER(BCEC_P_TimerExpiration_isr);
	BDBG_ASSERT(hCEC);
	BSTD_UNUSED(parm2);

	hRegister = hCEC->stDependencies.hRegister;
	ulOffset = hCEC->ulRegisterOffset;

	Register = BREG_Read32(hRegister, REGADDR_CEC_CNTRL_1 + ulOffset) ;
	i = BCHP_GET_FIELD_DATA(Register, REGNAME_CEC_CNTRL_1, TX_EOM) ;
	if (i != 0)
	{
		/* successfull transmit last CEC message - no action needed */
		goto done ;
	}

	BDBG_MSG(("%s: Stop HW from continue retrying", __FUNCTION__));
	Register = BREG_Read32(hRegister, REGADDR_CEC_CNTRL_1 + ulOffset) ;
	Register &=  ~ BCHP_MASK_CEC_CNTRL(CEC_CNTRL_1, START_XMIT_BEGIN) ;
	BREG_Write32(hRegister, REGADDR_CEC_CNTRL_1 + ulOffset, Register) ;   /* 0 */

	/* reset TX to clear possible state machine */
	{
		Register = BREG_Read32(hRegister, REGADDR_CEC_CNTRL_5 + ulOffset) ;
		Register &= ~(BCHP_MASK_DVP(CEC_CNTRL_5, CEC_TX_SW_INIT));
		Register |=
			  BCHP_FIELD_DATA(REGNAME_CEC_CNTRL_5, CEC_TX_SW_INIT, 1);
		BREG_Write32(hRegister, REGADDR_CEC_CNTRL_5 + ulOffset, Register) ;

		Register &= ~(BCHP_MASK_DVP(CEC_CNTRL_5, CEC_TX_SW_INIT));
		BREG_Write32(hRegister, REGADDR_CEC_CNTRL_5 + ulOffset, Register) ;
	}

	/* override status */
	hCEC->lastTransmitMessageStatus.uiStatus = 1;
	hCEC->lastTransmitMessageStatus.uiEOM = 1;

	/* Manually fire CEC Sent Event */
	BKNI_SetEvent(hCEC->BCEC_EventCec_Transmitted);

done:

	BDBG_LEAVE(BCEC_P_TimerExpiration_isr);
}
#endif

/********************
*	PUBLIC APIs		*
*********************/

/******************************************************************************
BERR_Code BCEC_Open
Summary: Open/Initialize the CEC device
*******************************************************************************/
BERR_Code BCEC_Open(
   BCEC_Handle *phCEC,	  /* [out] CEC handle */
   const BCEC_Dependencies *pstDependencies
)
{
	BERR_Code	rc = BERR_SUCCESS;

#ifdef BCHP_PWR_RESOURCE_HDMI_TX_CEC
	BCHP_Handle hChip ;
#endif
	BINT_Handle hInterrupt ;
	BREG_Handle hRegister ;
	BCEC_Handle hCEC = NULL ;

	uint32_t Register, ulOffset ;
	uint8_t i ;

	BDBG_ENTER(BCEC_Open) ;


	/* verify parameters */
	BDBG_ASSERT(pstDependencies->hChip) ;
	BDBG_ASSERT(pstDependencies->hRegister) ;
	BDBG_ASSERT(pstDependencies->hInterrupt) ;

#ifdef BCHP_PWR_RESOURCE_HDMI_TX_CEC
	hChip       = pstDependencies->hChip ;
#endif
	hInterrupt = pstDependencies->hInterrupt ;
	hRegister  = pstDependencies->hRegister ;
	ulOffset = 0;

#if BCEC_CONFIG_HAS_HDMI_RX
	ulOffset += BCEC_P_GET_REG_OFFSET(pstDependencies->eCecHdmiPath,
			BCHP_AON_HDMI_RX_REG_START, BCHP_AON_HDMI_TX_REG_START);

	BCEC_P_DisableUnusedCecCore(hRegister, pstDependencies->eCecHdmiPath);

#else
	if (pstDependencies->eCecHdmiPath != BCEC_Hdmi_eTx0)
	{
		BDBG_ERR(("*****************************************")) ;
		BDBG_ERR(("** Platform %d does not has an HDMI Rx **", BCHP_CHIP)) ;
		BDBG_ERR(("** port. Invalid CecHdmiPath setting   **")) ;
		BDBG_ERR(("*****************************************")) ;

		rc = BERR_NOT_SUPPORTED;
		goto done;
	}
#endif


	/* create the CEC Handle */
	hCEC = BKNI_Malloc(sizeof(BCEC_P_Handle)) ;
	if (!hCEC)
	{
		BDBG_ERR(("Unable to allocate memory for CEC Handle")) ;
		rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		goto done ;
	}

	/* zero out all memory associated with the CEC Device Handle before using */
	BKNI_Memset(hCEC, 0, sizeof(BCEC_P_Handle)) ;
	BDBG_OBJECT_SET(hCEC, BCEC_P_Handle) ;

	/* assign the handles passed in as parameters */
	BKNI_Memcpy(&hCEC->stDependencies, pstDependencies, sizeof(BCEC_Dependencies)) ;

	/* save ulOffset */
	hCEC->ulRegisterOffset = ulOffset;

	/* set default settings */
	BCEC_GetDefaultSettings (&hCEC->stSettings);
	hCEC->stSettings.eCecHdmiPath = pstDependencies->eCecHdmiPath;

#ifdef BCHP_PWR_RESOURCE_HDMI_TX_CEC
	BCHP_PWR_AcquireResource(hChip, BCHP_PWR_RESOURCE_HDMI_TX_CEC);
#endif

	/* Initialize CEC core */
	BCEC_P_Initialize(hCEC, ulOffset);

	/* Unmasked any previously masked interrupt to prevent missing interrupt
		when coming out of cold boot */
#if BCEC_CONFIG_AUTO_ON_SUPPORT
	BCEC_P_EnableAutoOn(hCEC, false);
#endif


	/* set default CEC address to 15 */
	Register = BREG_Read32(hRegister, REGADDR_CEC_CNTRL_1 + ulOffset) ;
	Register &= ~ BCHP_MASK_CEC_CNTRL(CEC_CNTRL_1, CEC_ADDR);
	Register |= BCHP_FIELD_DATA(REGNAME_CEC_CNTRL_1, CEC_ADDR,
							0x3);
	BREG_Write32(hRegister, REGADDR_CEC_CNTRL_1 + ulOffset, Register) ;


	hCEC->stSettings.CecLogicalAddr = BCEC_CONFIG_UNINITIALIZED_LOGICAL_ADDR;

#if BCEC_CONFIG_40NM_SUPPORT	/* the default value works for 40nm rev2 */
	Register = BREG_Read32(hRegister, REGADDR_CEC_CNTRL_6 + ulOffset) ;
		Register &= ~ (
			  BCHP_MASK_CEC_CNTRL(CEC_CNTRL_6, CEC_ADDR_1)
			| BCHP_MASK_CEC_CNTRL(CEC_CNTRL_6, CEC_ADDR_2));

		Register |= (
			  BCHP_FIELD_DATA(REGNAME_CEC_CNTRL_6, CEC_ADDR_1,
			    BAVC_HDMI_CEC_AllDevices_eUnRegistered)
			| BCHP_FIELD_DATA(REGNAME_CEC_CNTRL_6, CEC_ADDR_2,
			    BAVC_HDMI_CEC_AllDevices_eUnRegistered));
	BREG_Write32(hRegister, REGADDR_CEC_CNTRL_6 + ulOffset, Register) ;

#elif BCEC_CONFIG_65NM_SUPPORT
	Register = BREG_Read32(hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL) ;
		Register &= ~BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, PWRDN_CEC) ;
	BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, Register) ;
#endif


	/* Create Events for use with Interrupts */
	BCEC_CHECK_RC(rc, BKNI_CreateEvent(&(hCEC->BCEC_EventCec_Transmitted))) ;
	BCEC_CHECK_RC(rc, BKNI_CreateEvent(&(hCEC->BCEC_EventCec_Received))) ;

	/* register/enable interrupt callbacks */
	for( i = 0; i < BCEC_MAKE_INTR_ENUM(LAST) ; i++ )
	{
		/*
		** DEBUG
		** Create ALL interrupt callbacks
		** enable debug callbacks as needed;
		*/

		/* Use CEC Tx interrupts */
		if (pstDependencies->eCecHdmiPath == BCEC_Hdmi_eTx0)
		{
			BCEC_CHECK_RC( rc, BINT_CreateCallback(
				&(hCEC->hCallback[i]), hInterrupt,
				BCEC_Interrupts[i].IntrId,
				BCEC_P_HandleInterrupt_isr, (void *) hCEC, i ));

			/* clear interrupt callback */
			BCEC_CHECK_RC(rc, BINT_ClearCallback( hCEC->hCallback[i])) ;

			/* now enable it; if specified for startup */
			if (!BCEC_Interrupts[i].enable)
				continue ;
		}
#if BCEC_CONFIG_HAS_HDMI_RX
		else		/* Use CEC Rx interrupts */
		{
			BCEC_CHECK_RC( rc, BINT_CreateCallback(
				&(hCEC->hCallback[i]), hInterrupt,
				BCEC_RxInterrupts[i].IntrId,
				BCEC_P_HandleInterrupt_isr, (void *) hCEC, i ));

			/* clear interrupt callback */
			BCEC_CHECK_RC(rc, BINT_ClearCallback( hCEC->hCallback[i])) ;

			/* now enable it; if specified for startup */
			if (!BCEC_RxInterrupts[i].enable)
				continue ;
		}
#endif
		BCEC_CHECK_RC( rc, BINT_EnableCallback( hCEC->hCallback[i] ) );
	}

/* create TimerHandle for the compliance test workaround for 28nm */
#if BCEC_CONFIG_ENABLE_COMPLIANCE_TEST_WORKAROUND
	BTMR_GetDefaultTimerSettings(&hCEC->stTmrSettings) ;
		hCEC->stTmrSettings.type =  BTMR_Type_eCountDown ;
		hCEC->stTmrSettings.cb_isr = (BTMR_CallbackFunc) BCEC_P_TimerExpiration_isr ;
		hCEC->stTmrSettings.pParm1 = hCEC;
		hCEC->stTmrSettings.parm2 = 0 ;
		hCEC->stTmrSettings.exclusive = false ;
	rc = BTMR_CreateTimer(pstDependencies->hTmr, &hCEC->hTimer, &hCEC->stTmrSettings) ;
	if(rc != BERR_SUCCESS)
	{
		rc = BERR_TRACE(BERR_LEAKED_RESOURCE);
		goto done ;
	}

#endif

	/* keep created pointer */
	*phCEC = hCEC ;

done:
	if (rc != BERR_SUCCESS)
	{
		if (hCEC != NULL)
		{
			/* destroy cec events if needed */
			if (hCEC->BCEC_EventCec_Transmitted!= NULL)
				BKNI_DestroyEvent(hCEC->BCEC_EventCec_Transmitted);

			if (hCEC->BCEC_EventCec_Received!= NULL)
				BKNI_DestroyEvent(hCEC->BCEC_EventCec_Received);

			for( i = 0; i < BCEC_MAKE_INTR_ENUM(LAST); i++ )
			{
				if (hCEC->hCallback[i]) {
					/* all interrupts are now created; disable and destroy all on close */
					BINT_DisableCallback( hCEC->hCallback[i] );
					BINT_DestroyCallback( hCEC->hCallback[i] );
				}
			}

			BKNI_Free(hCEC);
		}
		*phCEC=NULL;


#ifdef BCHP_PWR_RESOURCE_HDMI_TX_CEC
		/* on failure, power everything down */
		BCHP_PWR_ReleaseResource(hChip, BCHP_PWR_RESOURCE_HDMI_TX_CEC);
#endif
	}

	BDBG_LEAVE(BCEC_Open);
	return rc;

}


/******************************************************************************
BERR_Code BCEC_Close
Summary: Close the CEC handle
*******************************************************************************/
void BCEC_Close(
   BCEC_Handle hCEC  /* [in] CEC handle */
)
{
	uint32_t i ;

	BDBG_ENTER(BCEC_Close) ;
	BDBG_OBJECT_ASSERT(hCEC, BCEC_P_Handle) ;

	/* Disable and Destroy the Callbacks */
	for( i = 0; i < BCEC_MAKE_INTR_ENUM(LAST); i++ )
	{
		if (hCEC->hCallback[i]) {
			/* all interrupts are now created; disable and destroy all on close */
			BINT_DisableCallback( hCEC->hCallback[i] );
			BINT_DestroyCallback( hCEC->hCallback[i] );
		}
	}

	/* destroy Cec events if needed */
	if (hCEC->BCEC_EventCec_Transmitted!= NULL)
		BKNI_DestroyEvent(hCEC->BCEC_EventCec_Transmitted);

	if (hCEC->BCEC_EventCec_Received!= NULL)
		BKNI_DestroyEvent(hCEC->BCEC_EventCec_Received);

#if BCEC_CONFIG_AUTO_ON_SUPPORT
	/* if autoOn feature is enabled, do not hold CEC core in reset */
	if (hCEC->stSettings.enableAutoOn)
		goto done;
#endif

#if BCEC_CONFIG_ENABLE_COMPLIANCE_TEST_WORKAROUND
	if (hCEC->hTimer) {
		if (BTMR_DestroyTimer(hCEC->hTimer) != BERR_SUCCESS) {
			BDBG_ERR(("Error destroying timer"));
		}
		hCEC->hTimer=NULL;
	}
#endif

	BKNI_EnterCriticalSection();
		/* Hold CEC core in reset; i.e. CEC will not respond */
		BCEC_P_ResetCore_isr (hCEC, 1);
	BKNI_LeaveCriticalSection();


#if BCEC_CONFIG_AUTO_ON_SUPPORT
done:
#endif

#ifdef BCHP_PWR_RESOURCE_HDMI_TX_CEC
	/* release the CEC	*/
	BCHP_PWR_ReleaseResource(hCEC->stDependencies.hChip, BCHP_PWR_RESOURCE_HDMI_TX_CEC);
#endif


	/* free memory associated with the CEC handle */
	BKNI_Memset(hCEC, 0, sizeof(BCEC_P_Handle)) ;
	BKNI_Free( (void *) hCEC) ;


	BDBG_LEAVE(BCEC_Close) ;
	return ;
}


/***************************************************************************
BERR_Code BCEC_GetEventHandle
Summary: Get the event handle for checking CEC events.
****************************************************************************/
BERR_Code BCEC_GetEventHandle(
   BCEC_Handle hCEC,			/* [in] HDMI handle */
   BCEC_EventType eEventType,
   BKNI_EventHandle *pBCECEvent /* [out] event handle */
)
{
	BERR_Code	   rc = BERR_SUCCESS;

	BDBG_ENTER(BCEC_GetEventHandle) ;
	BDBG_OBJECT_ASSERT(hCEC, BCEC_P_Handle) ;

	switch (eEventType)
	{
	case BCEC_EventCec_eTransmitted:
		*pBCECEvent = hCEC->BCEC_EventCec_Transmitted ;
		break ;

	case BCEC_EventCec_eReceived:
		*pBCECEvent = hCEC->BCEC_EventCec_Received ;
		break ;

	default :
		BDBG_ERR(("Invalid Event Type: %d", eEventType)) ;
		rc = BERR_INVALID_PARAMETER ;
		goto done ;
	}


done:
	BDBG_LEAVE(BCEC_GetEventHandle) ;
	return rc ;
}


void BCEC_GetDefaultSettings(
	BCEC_Settings *pstDefaultCecSettings
)
{
	BKNI_Memset(pstDefaultCecSettings,0,sizeof(BCEC_Settings));

	pstDefaultCecSettings->enable = true;
	pstDefaultCecSettings->CecLogicalAddr = BCEC_CONFIG_UNINITIALIZED_LOGICAL_ADDR;
	pstDefaultCecSettings->CecPhysicalAddr[0] = 0xFF;
	pstDefaultCecSettings->CecPhysicalAddr[1] = 0xFF;
	pstDefaultCecSettings->eDeviceType = BCEC_CONFIG_DEVICE_TYPE;
	pstDefaultCecSettings->enableAutoOn = false;
	pstDefaultCecSettings->eCecHdmiPath = BCEC_Hdmi_eTx0;
	pstDefaultCecSettings->cecVersion = BCEC_VERSION;
	return ;
}


BERR_Code BCEC_GetSettings(
	BCEC_Handle hCEC,		  /* [in] CEC handle */
	BCEC_Settings *pstCecSettings
)
{
	BERR_Code rc = BERR_SUCCESS ;

	BDBG_ENTER(BCEC_GetSettings);
	BDBG_OBJECT_ASSERT(hCEC, BCEC_P_Handle) ;

	BKNI_Memset(pstCecSettings, 0, sizeof(BCEC_Settings));

	if (hCEC->stSettings.CecLogicalAddr == BCEC_CONFIG_UNINITIALIZED_LOGICAL_ADDR) {
		BDBG_MSG(("CEC Logical Address Uninitialized"));
	}

	pstCecSettings->enable = hCEC->stSettings.enable;
	pstCecSettings->CecLogicalAddr = hCEC->stSettings.CecLogicalAddr;
	BKNI_Memcpy(pstCecSettings->CecPhysicalAddr, &hCEC->stSettings.CecPhysicalAddr[0], 2);
	pstCecSettings->eDeviceType = hCEC->stSettings.eDeviceType;
	pstCecSettings->enableAutoOn = hCEC->stSettings.enableAutoOn;
	pstCecSettings->eCecHdmiPath = hCEC->stSettings.eCecHdmiPath;
	pstCecSettings->cecVersion = hCEC->stSettings.cecVersion;
	pstCecSettings->stDeviceFeatures = hCEC->stSettings.stDeviceFeatures;


	BDBG_MSG(("BCM%d Device's Logical Addr: %d", BCHP_CHIP, pstCecSettings->CecLogicalAddr));
	BDBG_MSG(("BCM%d Device's Physical Addr: %02X %02X", BCHP_CHIP,
		pstCecSettings->CecPhysicalAddr[0], pstCecSettings->CecPhysicalAddr[1])) ;


	BDBG_LEAVE(BCEC_GetSettings);
	return rc ;
}


BERR_Code BCEC_SetSettings(
	BCEC_Handle hCEC,		  /* [in] CEC handle */
	const BCEC_Settings *pstCecSettings
)
{
	BERR_Code rc = BERR_SUCCESS ;
	BREG_Handle hRegister ;
	BINT_Handle hInterrupt ;
	uint32_t Register, ulOffset ;
	uint8_t i;

	BDBG_ENTER(BCEC_SetSettings);
	BDBG_OBJECT_ASSERT(hCEC, BCEC_P_Handle) ;

	hRegister = hCEC->stDependencies.hRegister ;
	hInterrupt = hCEC->stDependencies.hInterrupt ;
	ulOffset = 0;

#if BCEC_CONFIG_HAS_HDMI_RX
	ulOffset += BCEC_P_GET_REG_OFFSET(pstCecSettings->eCecHdmiPath,
			BCHP_AON_HDMI_RX_REG_START, BCHP_AON_HDMI_TX_REG_START);

	BCEC_P_DisableUnusedCecCore(hRegister, pstCecSettings->eCecHdmiPath);

#else
	if (pstCecSettings->eCecHdmiPath != BCEC_Hdmi_eTx0)
	{
		BDBG_ERR(("*****************************************")) ;
		BDBG_ERR(("** Platform %d does not has an HDMI Rx **", BCHP_CHIP)) ;
		BDBG_ERR(("** port. Invalid CecHdmiPath setting   **")) ;
		BDBG_ERR(("*****************************************")) ;

		rc = BERR_NOT_SUPPORTED;
		goto done;
	}
#endif

	/* Update to use a different CEC core (Tx or Rx) requires re-initialize interrupts */
	if (hCEC->stSettings.eCecHdmiPath != pstCecSettings->eCecHdmiPath)
	{
		/* Disable and Destroy the Callbacks */
		for( i = 0; i < BCEC_MAKE_INTR_ENUM(LAST); i++ )
		{
			if (hCEC->hCallback[i]) {
				/* all interrupts are now created; disable and destroy all on close */
				BINT_DisableCallback( hCEC->hCallback[i] );
				BINT_DestroyCallback( hCEC->hCallback[i] );
			}
		}

		/* Re-register/re-enable interrupt callbacks */
		for( i = 0; i < BCEC_MAKE_INTR_ENUM(LAST) ; i++ )
		{
			/*
			** DEBUG
			** Create ALL interrupt callbacks
			** enable debug callbacks as needed;
			*/

			/* Use CEC Tx interrupts */
			if (pstCecSettings->eCecHdmiPath == BCEC_Hdmi_eTx0)
			{
				BCEC_CHECK_RC( rc, BINT_CreateCallback(
					&(hCEC->hCallback[i]), hInterrupt,
					BCEC_Interrupts[i].IntrId,
					BCEC_P_HandleInterrupt_isr, (void *) hCEC, i ));

				/* clear interrupt callback */
				BCEC_CHECK_RC(rc, BINT_ClearCallback( hCEC->hCallback[i])) ;

				/* now enable it; if specified for startup */
				if (!BCEC_Interrupts[i].enable)
					continue ;
			}

#if BCEC_CONFIG_HAS_HDMI_RX
			else		/* Use CEC Rx interrupts */
			{
				BCEC_CHECK_RC( rc, BINT_CreateCallback(
					&(hCEC->hCallback[i]), hInterrupt,
					BCEC_RxInterrupts[i].IntrId,
					BCEC_P_HandleInterrupt_isr, (void *) hCEC, i ));

				/* clear interrupt callback */
				BCEC_CHECK_RC(rc, BINT_ClearCallback( hCEC->hCallback[i])) ;

				/* now enable it; if specified for startup */
				if (!BCEC_RxInterrupts[i].enable)
					continue ;
			}
#endif
			BCEC_CHECK_RC( rc, BINT_EnableCallback( hCEC->hCallback[i] ) );
		}

	}


	BKNI_EnterCriticalSection();
		if (pstCecSettings->enable)
		{
			/* a change in logical address or change in CEC core usage require a reset of the CEC core */
			if ((pstCecSettings->CecLogicalAddr != hCEC->stSettings.CecLogicalAddr)
			|| (pstCecSettings->eCecHdmiPath != hCEC->stSettings.eCecHdmiPath)
			|| (!hCEC->stSettings.enable))
			{
				Register = BREG_Read32(hRegister, REGADDR_CEC_CNTRL_1 + ulOffset) ;
					Register &= ~ BCHP_MASK_CEC_CNTRL(CEC_CNTRL_1, CEC_ADDR) ;
					Register |= BCHP_FIELD_DATA(REGNAME_CEC_CNTRL_1, CEC_ADDR, pstCecSettings->CecLogicalAddr) ;
				BREG_Write32(hRegister, REGADDR_CEC_CNTRL_1 + ulOffset, Register) ;

				/* if current state is enabled */
				if (hCEC->stSettings.enable)
				{
					/* toggle reset CEC core */
					BCEC_P_ResetCore_isr (hCEC, 1);
					BCEC_P_ResetCore_isr (hCEC, 0);
				}
			}
		}

		/* only enable/disable CEC core when there's a change  */
		if (hCEC->stSettings.enable != pstCecSettings->enable) {
			BCEC_P_ResetCore_isr (hCEC, !pstCecSettings->enable);
		}
	BKNI_LeaveCriticalSection();


	/* Save settings */
	hCEC->stSettings.enable = pstCecSettings->enable;
	BKNI_Memcpy(hCEC->stSettings.CecPhysicalAddr, pstCecSettings->CecPhysicalAddr, 2) ;
	hCEC->stSettings.eDeviceType = pstCecSettings->eDeviceType;
	hCEC->stSettings.CecLogicalAddr = pstCecSettings->CecLogicalAddr;
	hCEC->stSettings.enableAutoOn = pstCecSettings->enableAutoOn;
	hCEC->stSettings.eCecHdmiPath = pstCecSettings->eCecHdmiPath;
	hCEC->ulRegisterOffset = ulOffset;
	hCEC->stSettings.cecVersion = pstCecSettings->cecVersion;
	hCEC->stSettings.stDeviceFeatures = pstCecSettings->stDeviceFeatures;

	/* Reset FirstCecMessage */
	hCEC->firstCecMessage = true;

done:

	BDBG_LEAVE(BCEC_SetSettings);
	return rc ;
}


BERR_Code BCEC_GetTransmitMessageStatus(
	BCEC_Handle hCEC,	   /* [in] CEC handle */
	BCEC_MessageStatus *pstCecMessageStatus
)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BCEC_GetTransmitMessageStatus);
	BDBG_OBJECT_ASSERT(hCEC, BCEC_P_Handle) ;
	BDBG_ASSERT(pstCecMessageStatus);

	BKNI_EnterCriticalSection();
		pstCecMessageStatus->uiStatus = hCEC->lastTransmitMessageStatus.uiStatus;
		pstCecMessageStatus->uiMessageLength = hCEC->lastTransmitMessageStatus.uiMessageLength;
		pstCecMessageStatus->uiEOM = hCEC->lastTransmitMessageStatus.uiEOM;
		pstCecMessageStatus->bWokeUp = false;
	BKNI_LeaveCriticalSection();

	BDBG_LEAVE(BCEC_GetTransmitMessageStatus);
	return rc;
}


BERR_Code BCEC_GetReceivedMessageStatus(
	BCEC_Handle hCEC,	   /* [in] CEC handle */
	BCEC_MessageStatus *pstCecMessageStatus
)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BCEC_GetReceivedMessageStatus);
	BDBG_OBJECT_ASSERT(hCEC, BCEC_P_Handle) ;
	BDBG_ASSERT(pstCecMessageStatus);

	BKNI_EnterCriticalSection();
		pstCecMessageStatus->uiStatus = hCEC->lastReceivedMessageStatus.uiStatus;
		pstCecMessageStatus->uiMessageLength = hCEC->lastReceivedMessageStatus.uiMessageLength;
		pstCecMessageStatus->uiEOM = hCEC->lastReceivedMessageStatus.uiEOM;
		pstCecMessageStatus->bWokeUp = hCEC->lastReceivedMessageStatus.bWokeUp;
	BKNI_LeaveCriticalSection();

	BDBG_LEAVE(BCEC_GetReceivedMessageStatus);
	return rc;
}


BERR_Code BCEC_PingLogicalAddr(
   BCEC_Handle hCEC,	  /* [in] HDMI handle */
   uint8_t uiLogicalAddr	/* [in] device logical address */
)
{
	BERR_Code rc = BERR_SUCCESS ;
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset ;
	uint8_t i ;

	BDBG_ENTER(BCEC_PingLogicalAddr);
	BDBG_OBJECT_ASSERT(hCEC, BCEC_P_Handle) ;

	hRegister = hCEC->stDependencies.hRegister ;
	ulOffset = hCEC->ulRegisterOffset;

	BKNI_EnterCriticalSection();
		if (hCEC->firstCecMessage)
		{
			hCEC->firstCecMessage = false ;
		}
		else
		{
			/* verify no message is currently being transmitted */
			Register = BREG_Read32(hRegister, REGADDR_CEC_CNTRL_1 + ulOffset) ;
			i = BCHP_GET_FIELD_DATA(Register, REGNAME_CEC_CNTRL_1, TX_EOM) ;
			if (i == 0)
			{
				BDBG_WRN(("Transmit CEC is Busy (TX_EOM: %d)!!... retry Ping later", i)) ;
				rc = BCEC_TRANSMIT_BUSY ;
				goto done ;
			}
		}

		Register = 0 ;
		/* load the first nibble with the source/destination addr */
		Register = (uiLogicalAddr << 4) | uiLogicalAddr ;
		BDBG_MSG(("CecMsg[00]: Initiator %02X, Destination %02X", (Register >> 4),(Register & 0x0F))) ;

		/* write the first nibble in case there is no PayLoad i.e zero length message */
		BREG_Write32(hRegister, REGADDR_CEC_TX_DATA_1 + ulOffset, Register) ;

		/* set up the message length (=0) before xmit */
		Register = BREG_Read32(hRegister, REGADDR_CEC_CNTRL_1 + ulOffset) ;
		Register &= ~ BCHP_MASK_CEC_CNTRL(CEC_CNTRL_1, MESSAGE_LENGTH) ;
		Register |= BCHP_FIELD_DATA(REGNAME_CEC_CNTRL_1, MESSAGE_LENGTH, 0) ;
		BREG_Write32(hRegister, REGADDR_CEC_CNTRL_1 + ulOffset, Register) ;

		/* toggle the start xmit bit */
		Register &=  ~ BCHP_MASK_CEC_CNTRL(CEC_CNTRL_1, START_XMIT_BEGIN) ;
		BREG_Write32(hRegister, REGADDR_CEC_CNTRL_1 + ulOffset, Register) ;   /* 0 */

		Register |= BCHP_FIELD_DATA(REGNAME_CEC_CNTRL_1, START_XMIT_BEGIN, 1) ;
		BREG_Write32(hRegister, REGADDR_CEC_CNTRL_1 + ulOffset, Register) ;   /* 1 */
done:
	BKNI_LeaveCriticalSection();

	BDBG_LEAVE(BCEC_PingLogicalAddr);
	return rc ;
}



BERR_Code BCEC_XmitMessage(
	BCEC_Handle hCEC,	  /* [in] CEC handle */
	const BAVC_HDMI_CEC_MessageData *pMessageData 	/* [in] ptr to storage for CEC msg */
)
{
	BERR_Code rc = BERR_SUCCESS ;
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset ;
	uint32_t TxDataRegisterOffset ;
	uint8_t i, j ;
	uint8_t XmitMessageLength ;

#if BCEC_CONFIG_DEBUG_MESSAGE_TX
	/* allocate 3 bytes  for each OpCode / Parameter followed by a space i.e. "%02X "
		Also allow for message header byte */
	char XmitMessage[3 *(BCEC_CONFIG_P_MAX_MESSAGE_BUFFER + 1)];
	uint8_t debugMsgOffset = 0;
#endif

	BDBG_ENTER(BCEC_XmitMessage);
	BDBG_OBJECT_ASSERT(hCEC, BCEC_P_Handle) ;

	hRegister = hCEC->stDependencies.hRegister ;
	ulOffset = hCEC->ulRegisterOffset;


	if (pMessageData->messageLength > BAVC_HDMI_CEC_MAX_XMIT_LENGTH)
	{
		/* configure for Continuous Mode */
		BDBG_ERR(("CEC Continuous Mode not implemented yet")) ;
		rc = BERR_TRACE(BCEC_NOT_IMPLEMENTED);
		goto done ;
	}

	if (hCEC->stSettings.CecLogicalAddr == BCEC_CONFIG_UNINITIALIZED_LOGICAL_ADDR)
	{
		BDBG_WRN(("CEC Logical Address has not been initialized; Unable to Send message")) ;
		rc = BERR_TRACE(BERR_NOT_INITIALIZED);
		goto done ;
	}


	if (hCEC->firstCecMessage)
	{
		hCEC->firstCecMessage = false ;
	}
	else
	{
#if BCEC_CONFIG_ENABLE_COMPLIANCE_TEST_WORKAROUND
		/* Don't check TX_EOM status since it could be invalid */
		if ((pMessageData->messageBuffer[0] == BCEC_OpCode_FeatureAbort)
		|| (pMessageData->messageBuffer[0] == BCEC_OpCode_ReportPhysicalAddress))
		{
			/* Don't bother checking status of last xmit message */
		}
		else
#endif
		{
			/*	verify no message is currently being transmitted.
				TX_EOM field is not modified in any other software context (only update by the hardware)
				Thus, this is essentially an atomic read
			*/
			Register = BREG_Read32(hRegister, REGADDR_CEC_CNTRL_1 + ulOffset) ;
			i = BCHP_GET_FIELD_DATA(Register, REGNAME_CEC_CNTRL_1, TX_EOM) ;
			if (i == 0)
			{
				BDBG_WRN(("Transmit CEC is Busy (TX_EOM: %d)!!... retry transmit later", i)) ;
				rc = BERR_TRACE(BCEC_TRANSMIT_BUSY);
				goto done ;
			}
		}
	}


	/* load the CEC Msg Bytes */
	Register = 0 ;
	TxDataRegisterOffset = 0 ;

	/* load the first nibble with the source/destination addr */
	j = 1 ;
	Register = (hCEC->stSettings.CecLogicalAddr << 4) | pMessageData->destinationAddr;

#if BCEC_CONFIG_DEBUG_MESSAGE_TX
	/* Only fordebug purposes. BKNI_Snprintf should never be used in production
		code due to timing effects */
	debugMsgOffset += BKNI_Snprintf(XmitMessage+debugMsgOffset,
		sizeof (XmitMessage) - debugMsgOffset, "%02X ", Register) ;
#endif

	/* write the first nibble in case there is no PayLoad i.e zero length message */
	BREG_Write32(hRegister, REGADDR_CEC_TX_DATA_1 + ulOffset + TxDataRegisterOffset, Register) ;

	XmitMessageLength = pMessageData->messageLength /*+ 1*/ ;
	for ( i = 0 ; XmitMessageLength && (i <= XmitMessageLength) ; i = i + 4)
	{
		for ( ; j < 4 ; j++)
		{
			Register |= pMessageData->messageBuffer[j+i-1] << (8 * j) ;

			if (j + i == XmitMessageLength)
				break ;
		}

		BREG_Write32(hRegister, REGADDR_CEC_TX_DATA_1  + ulOffset + TxDataRegisterOffset, Register) ;

		BDBG_MSG(("CEC TxReg %#08x: %#08x",
			REGADDR_CEC_TX_DATA_1 + ulOffset + TxDataRegisterOffset, Register)) ;

		j = 0 ;
		Register = 0 ;
		TxDataRegisterOffset += 4 ;
	}


	/* set up  the length */
	BKNI_EnterCriticalSection();
		Register = BREG_Read32(hRegister, REGADDR_CEC_CNTRL_1 + ulOffset) ;
		Register &= ~ BCHP_MASK_CEC_CNTRL(CEC_CNTRL_1, MESSAGE_LENGTH) ;
		Register |= BCHP_FIELD_DATA(REGNAME_CEC_CNTRL_1, MESSAGE_LENGTH, pMessageData->messageLength) ;
		BREG_Write32(hRegister, REGADDR_CEC_CNTRL_1 + ulOffset, Register) ;

#if BCEC_CONFIG_ENABLE_COMPLIANCE_TEST_WORKAROUND
		/**** Always enable work-around for all CEC communication ****/
		if (1)
		{
			uint16_t msDelay=0;

			BDBG_MSG(("%s: Enable CEC work-around to prevent continuous retransmission", __FUNCTION__));

			Register = BREG_Read32(hRegister, REGADDR_CEC_CNTRL_1 + ulOffset) ;
			Register &=  ~ BCHP_MASK_CEC_CNTRL(CEC_CNTRL_1, START_XMIT_BEGIN) ;
			BREG_Write32(hRegister, REGADDR_CEC_CNTRL_1 + ulOffset, Register) ;   /* 0 */


			Register |= BCHP_FIELD_DATA(REGNAME_CEC_CNTRL_1, START_XMIT_BEGIN, 1) ;
			BREG_Write32(hRegister, REGADDR_CEC_CNTRL_1 + ulOffset, Register) ;   /* 1 */

			/* Calculate delay needed and kick-start count down timer
				Delay needed for 3 additional retries */
			msDelay = (uint16_t)
				/* Required time to send message 4 times, including up to 3 retries*/
				4 * (BCEC_START_BIT_XMIT_TIME + BCEC_PACKET_SIZE*BCEC_NOMINAL_BIT_TIME /* header */+
					BCEC_PACKET_SIZE*BCEC_NOMINAL_BIT_TIME * pMessageData->messageLength)
				+
				/* required wait time before retransmission if any >=3 */
				(3 * 4 * BCEC_NOMINAL_BIT_TIME);
			BKNI_LeaveCriticalSection();

			/* Arm timer */
			if (hCEC->hTimer) {
				BTMR_StopTimer(hCEC->hTimer);
				BTMR_StartTimer(hCEC->hTimer, msDelay * 1000) ;
			}
		}
		else
#endif
		{
			/* toggle the start xmit bit */
			Register = BREG_Read32(hRegister, REGADDR_CEC_CNTRL_1 + ulOffset) ;
			Register &=  ~ BCHP_MASK_CEC_CNTRL(CEC_CNTRL_1, START_XMIT_BEGIN) ;
			BREG_Write32(hRegister, REGADDR_CEC_CNTRL_1 + ulOffset, Register) ;   /* 0 */

			Register |= BCHP_FIELD_DATA(REGNAME_CEC_CNTRL_1, START_XMIT_BEGIN, 1) ;
			BREG_Write32(hRegister, REGADDR_CEC_CNTRL_1 + ulOffset, Register) ;   /* 1 */

			BKNI_LeaveCriticalSection();
		}


#if BCEC_CONFIG_DEBUG_MESSAGE_TX
	/* CEC print/process message, only for debug purposes.
		BKNI_Snprintf should never be used in production code due to timing effects */
	for (i = 0; i < XmitMessageLength && debugMsgOffset<sizeof(XmitMessage); i++)
	{
		debugMsgOffset += BKNI_Snprintf(XmitMessage+debugMsgOffset,
			sizeof (XmitMessage) - debugMsgOffset, "%02X ", pMessageData->messageBuffer[i]) ;
	}

	BDBG_WRN(("CEC Message Length %d transmitted: %s", XmitMessageLength, XmitMessage)) ;
#endif

#if BCEC_CONFIG_DEBUG_OPCODE
	BDBG_WRN(("Transmitted CEC Mesage <%s> - Opcode [0x%x]",
		BCEC_OpcodeToString(pMessageData->messageBuffer[0]),
		pMessageData->messageBuffer[0]));
#endif

done:

	BDBG_LEAVE(BCEC_XmitMessage);
	return rc ;
}


BERR_Code BCEC_GetReceivedMessage(
	BCEC_Handle hCEC,			/* [in] HDMI handle */
	BAVC_HDMI_CEC_MessageData *pRecvMessageData 	/* [out] ptr to storage for received CEC msg */
)
{
	BERR_Code rc = BERR_SUCCESS ;
	uint8_t cecMessageBuffer[BCEC_CONFIG_P_MAX_MESSAGE_BUFFER];
	uint8_t i, j ;
	uint8_t RxCecWordCount ;
	uint32_t RegisterOffset ;
	uint32_t Register, ulOffset ;

#if BCEC_CONFIG_DEBUG_MESSAGE_RX
	/* allocate 3 bytes  for each OpCode / Parameter followed by a space i.e. "%02X "
		Also allow for message header byte */
	char receivedMsg[3 * (BCEC_CONFIG_P_MAX_MESSAGE_BUFFER + 1)]="";
#endif

	BDBG_ENTER(BCEC_GetReceivedMessage);
	BDBG_OBJECT_ASSERT(hCEC, BCEC_P_Handle) ;

	ulOffset = hCEC->ulRegisterOffset;

	RxCecWordCount = hCEC->lastReceivedMessageStatus.uiMessageLength;
	pRecvMessageData->messageLength = RxCecWordCount ;
	RxCecWordCount++ ;

	/* get the received words and place into the buffer */
	RegisterOffset = 0 ;
	for (i = 0 ; i < RxCecWordCount ; i = i + 4)
	{
		Register = BREG_Read32(hCEC->stDependencies.hRegister, REGADDR_CEC_RX_DATA_1 + ulOffset + RegisterOffset) ;
		for (j = 0 ; j + i < RxCecWordCount; j++)
			cecMessageBuffer[i+j] = Register >> (8 * j) & 0xFF ;

		RegisterOffset = RegisterOffset + 4 ;
	}

	/* save the received message */
	pRecvMessageData->initiatorAddr = (cecMessageBuffer[0] >> 4) & 0x0F;
	pRecvMessageData->destinationAddr = cecMessageBuffer[0] & 0x0F;
	BKNI_Memcpy(pRecvMessageData->messageBuffer, cecMessageBuffer+1, pRecvMessageData->messageLength);

#if BCEC_CONFIG_DEBUG_MESSAGE_RX
	/* For debugging purposes only. BKNI_Snprintf should never be used
		in production code due to timing effects */
	for (i = 0, j = 0; i < pRecvMessageData->messageLength && j<(sizeof(receivedMsg)-1); i++) {
		j += BKNI_Snprintf(receivedMsg+j, sizeof(receivedMsg)-j, "%02X ",
			pRecvMessageData->messageBuffer[i]) ;
	}

	BDBG_MSG(("CEC Message Length %d Received: %s",
		pRecvMessageData->messageLength, receivedMsg)) ;
#endif


#if BCEC_CONFIG_DEBUG_OPCODE
	if (pRecvMessageData->messageLength > 0)
	{
		BDBG_WRN(("Received CEC Mesage <%s> - Opcode [0x%x]",
			BCEC_OpcodeToString(pRecvMessageData->messageBuffer[0]),
			pRecvMessageData->messageBuffer[0]));
	}
#endif


	BDBG_LEAVE(BCEC_GetReceivedMessage);
	return rc ;
}


BERR_Code BCEC_EnableReceive(
	BCEC_Handle hCEC	 /* [in] HDMI handle */
)
{
	BERR_Code rc = BERR_SUCCESS ;
	BDBG_ENTER(BCEC_EnableReceive);
	BDBG_OBJECT_ASSERT(hCEC, BCEC_P_Handle) ;

	BDBG_MSG(("Enable CEC Receive Mode")) ;
	BKNI_EnterCriticalSection();
		BCEC_P_EnableReceive_isr(hCEC);
	BKNI_LeaveCriticalSection();

	BDBG_LEAVE(BCEC_EnableReceive);
	return rc ;
}


BERR_Code BCEC_ReportPhysicalAddress(
	BCEC_Handle hCEC	  /* [in] HDMI handle */
)
{
	BERR_Code rc = BERR_SUCCESS ;
	BAVC_HDMI_CEC_MessageData stMessageData;

	BDBG_ENTER(BCEC_ReportPhysicalAddress);
	BDBG_OBJECT_ASSERT(hCEC, BCEC_P_Handle) ;
	BKNI_Memset(&stMessageData, 0, sizeof(BAVC_HDMI_CEC_MessageData));

	if (hCEC->stSettings.CecLogicalAddr == BCEC_CONFIG_UNINITIALIZED_LOGICAL_ADDR)
	{
		BDBG_WRN(("CEC Logical Address has not been initialized; Unable to Send message")) ;
		rc = BERR_TRACE(BERR_NOT_INITIALIZED);
		goto done ;
	}

	/**********************************
		CEC Message Buffer consists of:
			hexOpCode
			device physical address
			device type
	***********************************/

	/* CEC message opcode = 0x84 */
	stMessageData.messageBuffer[0] = BCEC_OpCode_ReportPhysicalAddress;

	/* [Device Physical Address] */
	stMessageData.messageBuffer[1] = hCEC->stSettings.CecPhysicalAddr[0];
	stMessageData.messageBuffer[2] = hCEC->stSettings.CecPhysicalAddr[1];

	/* Device Type */
	stMessageData.messageBuffer[3] = hCEC->stSettings.eDeviceType;

	/* Broadcast CEC message */
	stMessageData.initiatorAddr = hCEC->stSettings.CecLogicalAddr;
	stMessageData.destinationAddr = BCEC_BROADCAST_ADDR;
	stMessageData.messageLength = 4;
	rc = BCEC_XmitMessage(hCEC, &stMessageData);

done:
	BDBG_LEAVE(BCEC_ReportPhysicalAddress);
	return rc ;
}


/******************************************************************************
Summary: Enter standby mode
*******************************************************************************/
BERR_Code BCEC_Standby(
	BCEC_Handle hCEC, /* [in] CEC Handle */
	const BCEC_StandbySettings *pSettings
	)
{
	BERR_Code rc = BERR_SUCCESS;

#if BCEC_CONFIG_AUTO_ON_SUPPORT
	BDBG_OBJECT_ASSERT(hCEC, BCEC_P_Handle) ;

	if (hCEC->standby) {
		BDBG_ERR(("Already in standby"));
		rc = BERR_TRACE(BERR_UNKNOWN);
		goto done;
	}

	if (!hCEC->stSettings.enableAutoOn)
	{
		BDBG_ERR(("AutoOn feature is disabled. Enable through CEC settings"));
#ifdef BCHP_PWR_RESOURCE_HDMI_TX_CEC
		BCHP_PWR_ReleaseResource(hCEC->stDependencies.hChip, BCHP_PWR_RESOURCE_HDMI_TX_CEC);
#endif
		rc = BERR_TRACE(BERR_NOT_SUPPORTED);
		goto done;
	}

#if BCEC_CONFIG_ENABLE_COMPLIANCE_TEST_WORKAROUND
	BTMR_StopTimer(hCEC->hTimer);
	if (BTMR_DestroyTimer(hCEC->hTimer) != BERR_SUCCESS) {
		BDBG_ERR(("Error destroying timer"));
	}
	hCEC->hTimer=NULL;
#endif

	/* enable autoOn CEC */
	BCEC_P_EnableAutoOn(hCEC, true);
	hCEC->standby = true;

done:

#else
	/* No AutoON Support */
	BSTD_UNUSED(hCEC) ;
#endif

	BSTD_UNUSED(pSettings);
	return rc;
}


/******************************************************************************
Summary: Resume standby mode
*******************************************************************************/
BERR_Code BCEC_Resume(
	BCEC_Handle hCEC /* [in] CEC Handle */
	)
{
	BERR_Code rc = BERR_SUCCESS;

#if BCEC_CONFIG_AUTO_ON_SUPPORT
	BDBG_OBJECT_ASSERT(hCEC, BCEC_P_Handle) ;

	if (!hCEC->standby)
	{
		BDBG_ERR(("Not in standby"));
		rc = BERR_TRACE(BERR_UNKNOWN);
		goto done;
	}

	if (!hCEC->stSettings.enableAutoOn)
	{
#ifdef BCHP_PWR_RESOURCE_HDMI_TX_CEC
		BCHP_PWR_AcquireResource(hCEC->stDependencies.hChip, BCHP_PWR_RESOURCE_HDMI_TX_CEC);
#endif
	}

	/* create TimerHandle for the compliance test workaround for 28nm */
#if BCEC_CONFIG_ENABLE_COMPLIANCE_TEST_WORKAROUND
	BTMR_GetDefaultTimerSettings(&hCEC->stTmrSettings) ;
		hCEC->stTmrSettings.type =	BTMR_Type_eCountDown ;
		hCEC->stTmrSettings.cb_isr = (BTMR_CallbackFunc) BCEC_P_TimerExpiration_isr ;
		hCEC->stTmrSettings.pParm1 = hCEC;
		hCEC->stTmrSettings.parm2 = 0 ;
		hCEC->stTmrSettings.exclusive = false ;
	rc = BTMR_CreateTimer(hCEC->stDependencies.hTmr, &hCEC->hTimer, &hCEC->stTmrSettings) ;

	if(rc != BERR_SUCCESS)
	{
		rc = BERR_TRACE(BERR_LEAKED_RESOURCE);
		goto done ;
	}
#endif

	hCEC->standby = false;
	BCEC_P_EnableAutoOn(hCEC, false);

done:

#else
	BSTD_UNUSED(hCEC) ;
#endif

	return rc ;
}



/* End of file. */

