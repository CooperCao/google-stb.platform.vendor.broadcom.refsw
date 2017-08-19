/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bhdr.h"
#include "bhdr_priv.h"

#include "bhdr_fe.h"
#include "bhdr_fe_priv.h"

#include "bchp_hdmi_rx_eq_0.h"
#include "bchp_aon_hdmi_rx.h"

#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif


BDBG_MODULE(BHDR_FE_PRIV) ;

#define	BHDR_CHECK_RC( rc, func )	          \
do                                                \
{										          \
	if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
	{										      \
		goto done;							      \
	}										      \
} while(0)


typedef struct BHDR_FE_P_InterruptCbTable
{
	BINT_Id       IntrId;
	int               iParam2;
	bool             enable ; /* debug purposes */
} BHDR_FE_P_InterruptCbTable ;

static const BHDR_FE_P_InterruptCbTable BHDR_FE_P_ChannelIntr0[MAKE_INTR_ENUM(LAST)] =
{
#if BHDR_CONFIG_DUAL_HPD_SUPPORT
	/* 00 */   { BCHP_INT_ID_HP_CONNECTED_0, BHDR_FE_CHN_INTR_eHPD_CONNECTED, true},
	/* 01 */   { BCHP_INT_ID_HP_REMOVED_0, BHDR_FE_CHN_INTR_eHPD_REMOVED, true},
#else
	/* 00 */   { BCHP_INT_ID_RX_HOTPLUG_UPDATE_0, BHDR_FE_CHN_INTR_eRX_HOTPLUG_UPDATE, true},
#endif
	/* 01 */   { BCHP_INT_ID_CLOCK_STOP_0, BHDR_FE_CHN_INTR_eCLOCK_STOP_0, true},
	/* 02*/   { BCHP_INT_ID_PLL_UNLOCK_0, BHDR_FE_CHN_INTR_ePLL_UNLOCK_0, true},
	/* 03 */   { BCHP_INT_ID_PLL_LOCK_0, BHDR_FE_CHN_INTR_ePLL_LOCK_0, true},
	/* 04 */   { BCHP_INT_ID_FREQ_CHANGE_0,  BHDR_FE_CHN_INTR_eFREQ_CHANGE_0, true},
} ;


static void BHDR_FE_P_Channel_isr(void *pParam1, int parm2) ;



void BHDR_FE_P_Initialize(BHDR_FE_Handle hFrontEnd)
{

	BREG_Handle hRegister ;
	uint32_t Register ;

	BDBG_ENTER(BHDR_FE_P_Initialize) ;
	BDBG_OBJECT_ASSERT(hFrontEnd, BHDR_FE_P_Handle) ;

	hRegister = hFrontEnd->hRegister ;

	Register = BREG_Read32(hRegister, BCHP_DVP_HR_TOP_SW_INIT) ;
	Register &= ~BCHP_MASK(DVP_HR_TOP_SW_INIT, RX_0) ;
	BREG_Write32(hRegister, BCHP_DVP_HR_TOP_SW_INIT, Register) ;

	Register = BREG_Read32(hRegister, BCHP_DVP_HR_TOP_SW_INIT) ;
	Register &= ~ BCHP_MASK(DVP_HR_TOP_SW_INIT, FE_0) ;
	BREG_Write32(hRegister, BCHP_DVP_HR_TOP_SW_INIT, Register) ;

	BDBG_LEAVE(BHDR_FE_P_Initialize) ;

}


void BHDR_FE_P_OpenChannel(
	BHDR_FE_ChannelHandle hFeChannel) /* [out] Created channel handle */
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset ;

	BDBG_ENTER(BHDR_FE_P_OpenChannel) ;
	BDBG_OBJECT_ASSERT(hFeChannel, BHDR_FE_P_ChannelHandle);

	hRegister = hFeChannel->hRegister ;
	ulOffset = hFeChannel->ulOffset ;


	/* update hot plug defaults */
	Register = BREG_Read32( hRegister, BCHP_AON_HDMI_RX_HDMI_HOTPLUG_CONTROL + ulOffset) ;

#if ((BCHP_CHIP == 7425) && (BCHP_VER >= BCHP_VER_B0))
		/* 7425Bx settings */
		Register &= ~ BCHP_MASK(AON_HDMI_RX_HDMI_HOTPLUG_CONTROL, MANUAL_DVP_HR_HOTPLUG) ;
		Register |=    BCHP_FIELD_DATA(AON_HDMI_RX_HDMI_HOTPLUG_CONTROL, MANUAL_DVP_HR_HOTPLUG, 0) ;

		Register &= ~ BCHP_MASK(AON_HDMI_RX_HDMI_HOTPLUG_CONTROL, SELECT_DVP_HR_HOTPLUG) ;

		Register &= ~ BCHP_MASK(AON_HDMI_RX_HDMI_HOTPLUG_CONTROL, ENABLE_TEST_HOTPLUG_OUT) ;
		Register |=    BCHP_FIELD_DATA(AON_HDMI_RX_HDMI_HOTPLUG_CONTROL, ENABLE_TEST_HOTPLUG_OUT, 0) ;

		Register &= ~ BCHP_MASK(AON_HDMI_RX_HDMI_HOTPLUG_CONTROL, TEST_HOTPLUG_OUT) ;
#else
		/* 7429, 7435 */
		Register &= ~ BCHP_MASK(AON_HDMI_RX_HDMI_HOTPLUG_CONTROL, OVERRIDE_DVP_HR_HOTPLUG) ;
		Register |=    BCHP_FIELD_DATA(AON_HDMI_RX_HDMI_HOTPLUG_CONTROL, OVERRIDE_DVP_HR_HOTPLUG, 0) ;

	 	Register &= ~ BCHP_MASK(AON_HDMI_RX_HDMI_HOTPLUG_CONTROL, OVERRIDE_HOTPLUG_OUT) ;
	 	Register |=    BCHP_FIELD_DATA(AON_HDMI_RX_HDMI_HOTPLUG_CONTROL, OVERRIDE_HOTPLUG_OUT, 0) ;

	 	Register &= ~ BCHP_MASK(AON_HDMI_RX_HDMI_HOTPLUG_CONTROL, OVERRIDE_HOTPLUG_OUT_VALUE) ;
	 	Register &= ~ BCHP_MASK(AON_HDMI_RX_HDMI_HOTPLUG_CONTROL, SELECT_DVP_HR_HOTPLUG) ;
#endif

	BREG_Write32( hRegister, BCHP_AON_HDMI_RX_HDMI_HOTPLUG_CONTROL + ulOffset, Register) ;

	/* adjust equalizer defaults */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulOffset) ;
	Register &= ~ BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_ADC_POWER_DOWN) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulOffset, Register) ;

	/* allow the Hot Plug to power down the PHY */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_HDMI_RX_PHY_RESET_AND_POWER_CFG_1 + ulOffset) ;
	Register &= ~ BCHP_MASK(HDMI_RX_FE_0_HDMI_RX_PHY_RESET_AND_POWER_CFG_1,
		ALLOW_HOTPLUG_TO_POWER_DOWN_PHY) ;
	Register |= BCHP_FIELD_DATA(HDMI_RX_FE_0_HDMI_RX_PHY_RESET_AND_POWER_CFG_1,
		ALLOW_HOTPLUG_TO_POWER_DOWN_PHY, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_HDMI_RX_PHY_RESET_AND_POWER_CFG_1 + ulOffset, Register) ;

	Register = BREG_Read32( hRegister, BCHP_HDMI_RX_FE_0_HOTPLUG_CONTROL  + ulOffset) ;
	Register |= BCHP_MASK(HDMI_RX_FE_0_HOTPLUG_CONTROL, ENABLE_CLOCK_STOPPED_AS_HOTPLUG) ; /* 1 */
#if BHDR_CONFIG_HDCP_REPEATER
	/* HDMI_TODO Hot Plug bypass must be disabled to force HPD signal */
	Register &= ~ BCHP_MASK(HDMI_RX_FE_0_HOTPLUG_CONTROL, HOTPLUG_BYPASS) ;
#endif
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_HOTPLUG_CONTROL + ulOffset, Register) ;


	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH0 + ulOffset) ;
	Register &= ~ (
		  BCHP_MASK(HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH0, MON_CORRECT_EN)
		| BCHP_MASK(HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH0, MON_MARGIN_VAL))  ;
	Register |=
		  BCHP_FIELD_DATA(HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH0, MON_CORRECT_EN, 1)
		| BCHP_FIELD_DATA(HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH0, MON_MARGIN_VAL, 0x51) ;
	BREG_Write32(hRegister,
		BCHP_HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH0 + ulOffset, Register) ;


	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH1 + ulOffset) ;
	Register &= ~ (
		  BCHP_MASK(HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH1, MON_CORRECT_EN)
		| BCHP_MASK(HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH1, MON_MARGIN_VAL))  ;
	Register |=
		  BCHP_FIELD_DATA(HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH1, MON_CORRECT_EN, 1)
		| BCHP_FIELD_DATA(HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH1, MON_MARGIN_VAL, 0x51) ;
	BREG_Write32(hRegister,
		BCHP_HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH1 + ulOffset, Register) ;


	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH2 + ulOffset) ;
	Register &= ~ (
		  BCHP_MASK(HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH2, MON_CORRECT_EN)
		| BCHP_MASK(HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH2, MON_MARGIN_VAL))  ;
	Register |=
		  BCHP_FIELD_DATA(HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH2, MON_CORRECT_EN, 1)
		| BCHP_FIELD_DATA(HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH2, MON_MARGIN_VAL, 0x51) ;
	BREG_Write32(hRegister,
		BCHP_HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH2 + ulOffset, Register) ;

	BDBG_LEAVE(BHDR_FE_P_OpenChannel) ;
}


void BHDR_FE_P_CloseChannel(BHDR_FE_ChannelHandle hFeChannel)
{
	BDBG_ENTER(BHDR_FE_P_CloseChannel) ;
	BDBG_OBJECT_ASSERT(hFeChannel, BHDR_FE_P_ChannelHandle) ;

	BDBG_LEAVE(BHDR_FE_P_CloseChannel) ;
}

void BHDR_FE_P_CreateInterrupts(
	BHDR_FE_Handle hFrontEnd,       /* [in] HDMI Rx handle */
	BHDR_FE_ChannelHandle hFeChannel, /* [out] Created channel handle */
	const BHDR_FE_ChannelSettings  *pChannelSettings) /* [in] default HDMI settings */
{
	BERR_Code rc = BERR_SUCCESS ;
	uint8_t i ;
	const BHDR_FE_P_InterruptCbTable *pInterrupts ;

	BDBG_ENTER(BHDR_FE_P_CreateInterrupts) ;
	/* Register/enable interrupt callbacks for channel */


	pInterrupts = BHDR_FE_P_ChannelIntr0 ;

	for (i = 0; i < MAKE_INTR_FE_CHN_ENUM(LAST) ; i++)
	{
		BHDR_CHECK_RC( rc, BINT_CreateCallback(	&(hFeChannel->hCallback[i]),
			hFrontEnd->hInterrupt, pInterrupts[i].IntrId,
			BHDR_FE_P_Channel_isr, (void *) hFeChannel, i ));

		/* clear interrupt callback */
		BHDR_CHECK_RC(rc, BINT_ClearCallback( hFeChannel->hCallback[i])) ;

		/* skip interrupt if not enabled in table...  */
		if (!pInterrupts[i].enable)
			continue ;

		/* enable interrupts if HPD signal is connected  */
		/* i.e. direct connection or switch with HPD connected */
		if (!pChannelSettings->bHpdDisconnected)
		{
			BHDR_CHECK_RC( rc, BINT_EnableCallback( hFeChannel->hCallback[i] )) ;
		}
	}

done :
	BDBG_LEAVE(BHDR_FE_P_CreateInterrupts) ;
}

/**************************************************************************
Summary: Enable/Disable Frontend Interrupts
**************************************************************************/
void BHDR_FE_P_EnableInterrupts_isr(BHDR_FE_ChannelHandle hFeChannel, bool enable)
{
	BERR_Code rc  ;
	uint8_t i ;
	const BHDR_FE_P_InterruptCbTable *pInterrupts ;

	BDBG_ENTER(BHDR_FE_P_EnableInterrupts_isr) ;

	/* do not enable/disable interrupts unless HPD signal is disconnected  */
	if (!hFeChannel->settings.bHpdDisconnected)
		return ;

	/* get offset for Front End */
	pInterrupts = BHDR_FE_P_ChannelIntr0 ;

	for (i = 0; i < MAKE_INTR_FE_CHN_ENUM(LAST) ; i++)
	{
		/* clear interrupt callback */
		rc =  BINT_ClearCallback_isr( hFeChannel->hCallback[i]) ;
		if (rc)
		{
			rc = BERR_TRACE(rc) ;
		}

		/* skip interrupt if not enabled in table...  */
		if (!pInterrupts[i].enable)
			continue ;

 		if (enable)
			BINT_EnableCallback_isr( hFeChannel->hCallback[i] ) ;

	   /* never disable hot plug interrupt */
#if BHDR_CONFIG_DUAL_HPD_SUPPORT
		else if ((i != MAKE_INTR_FE_CHN_ENUM(HPD_CONNECTED))
		&&		 (i != MAKE_INTR_FE_CHN_ENUM(HPD_REMOVED)))
#else
		else if (i != MAKE_INTR_FE_CHN_ENUM(RX_HOTPLUG_UPDATE))
#endif
		{
			BINT_DisableCallback_isr( hFeChannel->hCallback[i] ) ;
			BDBG_WRN(("Disable Interrupt %d; ", i)) ;
		}
		else
		{
			BDBG_WRN(("Keep Interrupt %d Enabled", i)) ;
		}
	}
	BDBG_LEAVE(BHDR_FE_P_EnableInterrupts_isr) ;
}


/******************************************************************************
void BHDR_FE_P_ClearHdcpAuthentication_isr
Summary: Clear internal HDCP engine
*******************************************************************************/
static void BHDR_FE_P_ClearHdcpAuthentication_isr(BHDR_FE_ChannelHandle hFeChannel)
{
	BREG_Handle hRegister ;
	uint32_t ulOffset, Register  ;

	BDBG_OBJECT_ASSERT(hFeChannel, BHDR_FE_P_ChannelHandle);
	hRegister= hFeChannel->hRegister ;

	ulOffset = hFeChannel->ulHdrOffset ;

	/* clear HDCP authenticated status in HDMI Rx Core */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDCP_DEBUG + ulOffset) ;

	Register &= ~ BCHP_MASK(HDMI_RX_0_HDCP_DEBUG, CLEAR_RX_AUTHENTICATED_P) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDCP_DEBUG + ulOffset , Register) ;

	Register |= BCHP_MASK(HDMI_RX_0_HDCP_DEBUG, CLEAR_RX_AUTHENTICATED_P) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDCP_DEBUG  + ulOffset , Register) ;
}


static void BHDR_FE_P_FireHotPlugCb_isr(BHDR_FE_ChannelHandle hFeChannel)
{
#if BHDR_CONFIG_DEBUG_FRONT_END
	BDBG_WRN(("FE_%d RX HOT PLUG update (HPD) : %s ",
		hFeChannel->eChannel,
		hFeChannel->bTxDeviceAttached ? "HIGH" : "LOW")) ;
#endif

	/* inform higher level of Connect/Disconnect interrupt */
	if (hFeChannel->pfHotPlugCallback_isr)
	{
		hFeChannel->pfHotPlugCallback_isr(hFeChannel->pvHotPlugParm1,
			hFeChannel->iHotPlugParm2, &hFeChannel->bTxDeviceAttached) ;
	}
	else
	{
		BDBG_WRN(("FE_%d No HotPlug callback installed...",
			hFeChannel->eChannel)) ;
	}

	/* clear HDCP authenticated status in HDMI Rx Core */
	BHDR_FE_P_ClearHdcpAuthentication_isr(hFeChannel) ;

}


/******************************************************************************
void BHDR_FE_P_Channel_isr
Summary: Handle interrupts from the HDMI core.
*******************************************************************************/
void BHDR_FE_P_Channel_isr(
	void *pParam1,						/* [in] Device handle */
	int parm2							/* [in] Interrupt ID */
)
{
	BHDR_FE_ChannelHandle hFeChannel  ;
#if ! BHDR_CONFIG_DUAL_HPD_SUPPORT
	uint32_t Register ;
	uint32_t ulOffset ;
	BREG_Handle hRegister ;
#endif
	bool bPllLocked;

	BDBG_ENTER(BHDR_FE_P_Channel_isr) ;
	hFeChannel = (BHDR_FE_ChannelHandle) pParam1 ;
	BDBG_OBJECT_ASSERT(hFeChannel, BHDR_FE_P_ChannelHandle) ;

	if (hFeChannel->uiHdrSel == BAVC_HDMI_CoreId_eNone)
	{
#if BHDR_CONFIG_DEBUG_FRONT_END
		BDBG_WRN(("No HDMI Rx core (not ready) attached to frontend")) ;
#endif
		return ;
	}

	switch (parm2)
	{
#if BHDR_CONFIG_DUAL_HPD_SUPPORT
	case MAKE_INTR_FE_CHN_ENUM(HPD_CONNECTED) :

		hFeChannel->bTxDeviceAttached = true ;
		BHDR_FE_P_FireHotPlugCb_isr(hFeChannel) ;

		break ;

	case MAKE_INTR_FE_CHN_ENUM(HPD_REMOVED) :

		hFeChannel->bTxDeviceAttached = false  ;
		BHDR_FE_P_FireHotPlugCb_isr(hFeChannel) ;

		break ;

#else

	case MAKE_INTR_FE_CHN_ENUM(RX_HOTPLUG_UPDATE) :
		hRegister = hFeChannel->hRegister ;
		/* get offset for Front End */
		ulOffset = hFeChannel->ulOffset ;

		Register = BREG_Read32( hRegister, BCHP_HDMI_RX_FE_0_HOTPLUG_STATUS + ulOffset) ;
		hFeChannel->bTxDeviceAttached = BCHP_GET_FIELD_DATA(Register,
			HDMI_RX_FE_0_HOTPLUG_STATUS, RX_HOTPLUG_IN) ;

		BHDR_FE_P_FireHotPlugCb_isr(hFeChannel) ;

		break ;
#endif


	case MAKE_INTR_FE_CHN_ENUM(PLL_UNLOCK_0) :
#if BHDR_CONFIG_DEBUG_FRONT_END
		BDBG_WRN(("FE_%d   PLL -_-UNLOCKED-_-", hFeChannel->eChannel)) ;
#endif
		BHDR_FE_P_GetPllLockStatus_isr(hFeChannel, &bPllLocked) ;

		break ;

	case MAKE_INTR_FE_CHN_ENUM(PLL_LOCK_0) :

#if BHDR_CONFIG_DEBUG_FRONT_END
		BDBG_WRN(("FE_%d   PLL ----LOCKED----", hFeChannel->eChannel)) ;
#endif
		BHDR_FE_P_GetPllLockStatus_isr(hFeChannel, &bPllLocked) ;

#if BHDR_CONFIG_RESET_CLOCK_AND_DATA_CHANNELS
		/* enable the count down timer if not already enabled */
	 	BHDR_FE_P_ResetFeDataChannels_isr(hFeChannel) ;
		BHDR_FE_P_ResetFeClockChannel_isr(hFeChannel) ;
#endif

 		break ;

	case MAKE_INTR_FE_CHN_ENUM(FREQ_CHANGE_0) :
		hFeChannel->PreviousPixelClockCount = 0 ;
#if BHDR_CONFIG_DEBUG_FRONT_END
		BDBG_WRN(("FE_%d Frequency Changed", hFeChannel->eChannel)) ;
#endif

		break ;


	case MAKE_INTR_FE_CHN_ENUM(CLOCK_STOP_0) :
#if BHDR_CONFIG_DEBUG_FRONT_END
		BDBG_WRN(("FE_%d Clock STOPPED", hFeChannel->eChannel)) ;
#endif
		break ;


	default	:
		BDBG_ERR(("Unknown Interrupt ID:%d", parm2)) ;
	} ;

	/* L2 interrupts are reset automatically */
	BDBG_LEAVE(BHDR_FE_P_Channel_isr) ;
}




void BHDR_FE_P_ResetPixelClockEstimation_isr(BHDR_FE_ChannelHandle hFeChannel)
{
	BREG_Handle hRegister ;
	uint32_t Register ;

	BDBG_ENTER(BHDR_FE_P_ResetPixelClockEstimation_isr) ;
	BDBG_OBJECT_ASSERT(hFeChannel, BHDR_FE_P_ChannelHandle) ;

	hRegister = hFeChannel->hRegister;

	Register = BREG_Read32(hRegister, BCHP_DVP_HR_HDMI_FE_0_SW_INIT ) ;
	Register &= ~ (BCHP_MASK(DVP_HR_HDMI_FE_0_SW_INIT, FREQ_EST)) ;

	Register |= BCHP_FIELD_DATA(DVP_HR_HDMI_FE_0_SW_INIT, FREQ_EST, 1) ;
	BREG_Write32(hRegister, BCHP_DVP_HR_HDMI_FE_0_SW_INIT, Register) ;

	Register &= ~ (BCHP_MASK(DVP_HR_HDMI_FE_0_SW_INIT, FREQ_EST)) ;
	BREG_Write32(hRegister, BCHP_DVP_HR_HDMI_FE_0_SW_INIT, Register) ;

	BDBG_LEAVE(BHDR_FE_P_ResetPixelClockEstimation_isr) ;
}


/******************************************************************************
void BHDR_FE_P_GetPixelClockStatus_isr
Summary: Reads appropriate FE registers to get Pixel Clock data
*******************************************************************************/
void BHDR_FE_P_GetPixelClockStatus_isr(BHDR_FE_ChannelHandle hFeChannel,
	BHDR_FE_P_PixelClockStatus *ClockStatus)

{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulFeOffset ;
	uint32_t RegAddr ;
#if BHDR_CONFIG_FE_MULTI_CLOCK_SUPPORT
	uint8_t Channel ;
#endif

	BDBG_ENTER(BHDR_FE_P_GetPixelClockStatus_isr) ;
	BDBG_OBJECT_ASSERT(hFeChannel, BHDR_FE_P_ChannelHandle) ;

	hRegister = hFeChannel->hRegister ;
	ulFeOffset = hFeChannel->ulOffset ;


#if BHDR_CONFIG_FE_MULTI_CLOCK_SUPPORT

	RegAddr = BCHP_HDMI_RX_FE_0_PIX_CLK_CNT + ulFeOffset ;
	for (Channel = 0 ; Channel < BHDR_FE_P_CLOCK_eChMax ; Channel++)
	{
		Register = BREG_Read32(hRegister, RegAddr) ;
		ClockStatus[Channel].PixelCount =
			BCHP_GET_FIELD_DATA(Register, HDMI_RX_FE_0_PIX_CLK_CNT, PIX_CLK_CNT) ;

		RegAddr = RegAddr + 4 ;

		Register = BREG_Read32(hRegister, BCHP_DVP_HR_FREQ_MEASURE_CONTROL) ;
		Register &= ~ BCHP_MASK(DVP_HR_FREQ_MEASURE_CONTROL, SOURCE) ;
		Register |= BCHP_FIELD_DATA(DVP_HR_FREQ_MEASURE_CONTROL, SOURCE, Channel) ;
		BREG_Write32(hRegister, BCHP_DVP_HR_FREQ_MEASURE_CONTROL, Register) ;

		Register = BREG_Read32(hRegister, BCHP_DVP_HR_FREQ_MEASURE) ;

		ClockStatus[Channel].Frequency =
			BCHP_GET_FIELD_DATA(Register, DVP_HR_FREQ_MEASURE, VALUE) ;
	}


	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_CLK_CNT_STATUS + ulFeOffset) ;
	ClockStatus[BHDR_FE_P_CLOCK_eChRef].bClockStopped =
		BCHP_GET_FIELD_DATA(Register, HDMI_RX_FE_0_CLK_CNT_STATUS, CLOCK_STOPPED_REF) ;

	ClockStatus[BHDR_FE_P_CLOCK_eCh0].bClockStopped =
		BCHP_GET_FIELD_DATA(Register, HDMI_RX_FE_0_CLK_CNT_STATUS, CLOCK_STOPPED_CH0) ;
	ClockStatus[BHDR_FE_P_CLOCK_eCh1].bClockStopped =
		BCHP_GET_FIELD_DATA(Register, HDMI_RX_FE_0_CLK_CNT_STATUS, CLOCK_STOPPED_CH1) ;
	ClockStatus[BHDR_FE_P_CLOCK_eCh2].bClockStopped =
		BCHP_GET_FIELD_DATA(Register, HDMI_RX_FE_0_CLK_CNT_STATUS, CLOCK_STOPPED_CH2) ;

#else
	BSTD_UNUSED(RegAddr) ;

	{
		/* use reference clock vs ch1,2,or 3 */
		uint8_t ClockId = 0 ;

		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_PIX_CLK_EST_CTL + ulFeOffset) ;
		Register &= ~ BCHP_MASK(HDMI_RX_FE_0_PIX_CLK_EST_CTL, PIX_CLK_SELECT) ;

		Register |= BCHP_FIELD_DATA(HDMI_RX_FE_0_PIX_CLK_EST_CTL, PIX_CLK_SELECT, ClockId) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_PIX_CLK_EST_CTL + ulFeOffset, Register) ;

		BKNI_Delay(300) ;

		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_PIX_CLK_CNT + ulFeOffset) ;
		ClockStatus[BHDR_FE_P_CLOCK_eChRef].PixelCount =
			BCHP_GET_FIELD_DATA(Register, HDMI_RX_FE_0_PIX_CLK_CNT, PIX_CLK_CNT) ;

		ClockStatus[BHDR_FE_P_CLOCK_eChRef].bClockStopped =
			BCHP_GET_FIELD_DATA(Register, HDMI_RX_FE_0_PIX_CLK_CNT, CLOCK_STOPPED) ;
	}
#endif


#if BHDR_CONFIG_DEBUG_FRONT_END
	BDBG_WRN(("Frontend Clock Status:")) ;
	for (Channel = 0 ; Channel < BHDR_FE_P_CLOCK_eChMax ; Channel++)
	{
		BDBG_WRN(("Channel_%d  Pixel Count= %d  Frequency= %d Clock Status: %s",
			Channel,
			ClockStatus[Channel].PixelCount, ClockStatus[Channel].Frequency,
			ClockStatus[Channel].bClockStopped ? "Stopped" : "Running")) ;
	}
#endif

	BDBG_LEAVE(BHDR_FE_P_GetPixelClockStatus_isr) ;
}


#if !BHDR_CONFIG_FE_MULTI_CLOCK_SUPPORT
/******************************************************************************
void BHDR_FE_P_GetPixelClockData_isr
Summary: Reads appropriate FE registers to get Pixel Clock data
*******************************************************************************/
void BHDR_FE_P_GetPixelClockData_isr(BHDR_FE_ChannelHandle hFeChannel, uint32_t *PixelClockCount, uint8_t *ClockStopped)
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulFeOffset ;
	uint8_t ClockId = 0;

	BDBG_ENTER(BHDR_FE_P_GetPixelClockData_isr) ;
	BDBG_OBJECT_ASSERT(hFeChannel, BHDR_FE_P_ChannelHandle) ;

	hRegister = hFeChannel->hRegister ;
	ulFeOffset = hFeChannel->ulOffset ;

	BDBG_ERR(("****************")) ;
	BDBG_ERR(("BHDR_FE_P_GetPixelClockData_isr will be depracated; use BHDR_FE_P_GetPixelClockStatus_isr instead")) ;
	BDBG_ERR(("****************")) ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_PIX_CLK_EST_CTL + ulFeOffset) ;
	Register &= ~ BCHP_MASK(HDMI_RX_FE_0_PIX_CLK_EST_CTL, PIX_CLK_SELECT) ;
	Register |= BCHP_FIELD_DATA(HDMI_RX_FE_0_PIX_CLK_EST_CTL, PIX_CLK_SELECT, ClockId) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_PIX_CLK_EST_CTL + ulFeOffset, Register) ;

	BKNI_Delay(300) ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_PIX_CLK_CNT + ulFeOffset) ;
	*PixelClockCount = BCHP_GET_FIELD_DATA(Register,  HDMI_RX_FE_0_PIX_CLK_CNT, PIX_CLK_CNT) ;

	*ClockStopped = BCHP_GET_FIELD_DATA(Register, HDMI_RX_FE_0_PIX_CLK_CNT, CLOCK_STOPPED) ;

#if BHDR_CONFIG_DEBUG_FRONT_END
	BDBG_WRN(("Frontend Clock Status:")) ;
	BDBG_WRN(("Pixel Count= %d  ClockStopped %d",
		*PixelClockCount, *ClockStopped)) ;
#endif

	BDBG_LEAVE(BHDR_FE_P_GetPixelClockData_isr) ;
}
#endif


/******************************************************************************
Summary:
*******************************************************************************/
void BHDR_FE_P_SetHotPlug(BHDR_FE_ChannelHandle hFeChannel, BHDR_HotPlugSignal eHotPlugSignal)
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulFeOffset  ;

	BDBG_ENTER(BHDR_FE_P_SetHotPlug) ;
	BDBG_OBJECT_ASSERT(hFeChannel, BHDR_FE_P_ChannelHandle) ;

	hRegister= hFeChannel->hRegister ;
	ulFeOffset = hFeChannel->ulOffset ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_HOTPLUG_CONTROL + ulFeOffset) ;

#if BHDR_FE_HP_LEGACY_SUPPORT
	if (eHotPlugSignal == BHDR_HotPlugSignal_eHigh)
	{
		Register &= ~ BCHP_MASK(HDMI_RX_FE_0_HOTPLUG_CONTROL, RX_HOTPLUG_OUT_FORCE_LOW) ; /* 0 */
	}
	else
	{
		Register |= BCHP_MASK(HDMI_RX_FE_0_HOTPLUG_CONTROL, RX_HOTPLUG_OUT_FORCE_LOW) ; /* 1 */
	}
#else

	Register &= ~ (
		  BCHP_MASK(HDMI_RX_FE_0_HOTPLUG_CONTROL, HOTPLUG_BYPASS)
		| BCHP_MASK(HDMI_RX_FE_0_HOTPLUG_CONTROL, OVERRIDE_HOTPLUG_OUT)
		| BCHP_MASK(HDMI_RX_FE_0_HOTPLUG_CONTROL, OVERRIDE_HOTPLUG_OUT_VALUE))  ;

	Register |=
		  BCHP_FIELD_DATA(HDMI_RX_FE_0_HOTPLUG_CONTROL, HOTPLUG_BYPASS, eHotPlugSignal)
		| BCHP_FIELD_DATA(HDMI_RX_FE_0_HOTPLUG_CONTROL, OVERRIDE_HOTPLUG_OUT, !eHotPlugSignal)
		| BCHP_FIELD_DATA(HDMI_RX_FE_0_HOTPLUG_CONTROL, OVERRIDE_HOTPLUG_OUT_VALUE, eHotPlugSignal) ;

#endif


	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_HOTPLUG_CONTROL + ulFeOffset, Register) ;

	BDBG_LEAVE(BHDR_FE_P_SetHotPlug) ;
}


void BHDR_FE_P_PowerResourceAcquire_DVP_HR(BHDR_FE_Handle hFrontEnd)
{
	BDBG_ENTER(BHDR_FE_P_PowerResourceAcquire_DVP_HR) ;
	BDBG_OBJECT_ASSERT(hFrontEnd, BHDR_FE_P_Handle) ;

#if BHDR_CONFIG_DEBUG_HDR_PWR
	BDBG_WRN(("Acquire HDMI_RX0_CLK Resource at line %d", __LINE__)) ;
#endif
#if BCHP_PWR_RESOURCE_HDMI_RX0_CLK
	BCHP_PWR_AcquireResource(hFrontEnd->hChip, BCHP_PWR_RESOURCE_HDMI_RX0_CLK);
#endif

	BDBG_LEAVE(BHDR_FE_P_PowerResourceAcquire_DVP_HR) ;
}


void BHDR_FE_P_PowerResourceRelease_DVP_HR(BHDR_FE_Handle hFrontEnd)
{
	BDBG_ENTER(BHDR_FE_P_PowerResourceRelease_DVP_HR) ;
	BDBG_OBJECT_ASSERT(hFrontEnd, BHDR_FE_P_Handle) ;

#if BHDR_CONFIG_DEBUG_HDR_PWR
	BDBG_WRN(("Release HDMI_RX_0_CLK Resource at line %d", __LINE__)) ;
#endif
#if BCHP_PWR_RESOURCE_HDMI_RX0_CLK
	BCHP_PWR_ReleaseResource(hFrontEnd->hChip, BCHP_PWR_RESOURCE_HDMI_RX0_CLK);
#endif

	BDBG_LEAVE(BHDR_FE_P_PowerResourceRelease_DVP_HR) ;
}


void BHDR_FE_P_PowerResourceAcquire_HDMI_RX_FE(BHDR_FE_ChannelHandle hFeChannel)
{
	BDBG_ENTER(BHDR_FE_P_PowerResourceAcquire_HDMI_RX_FE) ;
	BDBG_OBJECT_ASSERT(hFeChannel, BHDR_FE_P_ChannelHandle);

#if BHDR_CONFIG_DEBUG_HDR_PWR
	BDBG_WRN(("Acquire HDMI_RX0_PHY Resource at line %d", __LINE__)) ;
#endif
#if BCHP_PWR_RESOURCE_HDMI_RX0_PHY
	BCHP_PWR_AcquireResource(hFeChannel->hChip, BCHP_PWR_RESOURCE_HDMI_RX0_PHY);
#endif

	BDBG_LEAVE(BHDR_FE_P_PowerResourceAcquire_HDMI_RX_FE) ;
}


void BHDR_FE_P_PowerResourceRelease_HDMI_RX_FE(BHDR_FE_ChannelHandle hFeChannel)
{
	BDBG_ENTER(BHDR_FE_P_PowerResourceAcquire_HDMI_RX_FE) ;
	BDBG_OBJECT_ASSERT(hFeChannel, BHDR_FE_P_ChannelHandle);

#if BHDR_CONFIG_DEBUG_HDR_PWR
	BDBG_WRN(("Release HDMI_RX0_PHY Resource at line %d", __LINE__)) ;
#endif
#if BCHP_PWR_RESOURCE_HDMI_RX0_PHY
	BCHP_PWR_ReleaseResource(hFeChannel->hChip, BCHP_PWR_RESOURCE_HDMI_RX0_PHY);
#endif

	BDBG_LEAVE(BHDR_FE_P_PowerResourceAcquire_HDMI_RX_FE) ;
}


