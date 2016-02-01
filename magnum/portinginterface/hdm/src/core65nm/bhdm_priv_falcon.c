/***************************************************************************
*	  (c)2004-2014 Broadcom Corporation
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
*  1.	  This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*
*  2.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
*  USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#include "bhdm.h"
#include "bhdm_priv.h"

BDBG_MODULE(BHDM_PRIV) ;


/******************************************************************************
BERR_Code BHDM_P_EnableTmdsData_isr
Summary: Enable/drive (Display) TMDS Output lines (Clock, Tx0, Tx1, Tx2)
*******************************************************************************/
void BHDM_P_EnableTmdsData_isr(
   const BHDM_Handle hHDMI,		/* [in] HDMI handle */
   bool bEnableTmdsOutput	/* [in] boolean to enable/disable */
)
{
	uint32_t Register ;
	uint32_t TmdsOutput ;
	uint8_t DeviceAttached ;

	BDBG_ENTER(BHDM_P_EnableTmdsData_isr) ;

#if BHDM_CONFIG_DVO_SUPPORT
	/* TMDS is always off when DVO is enabled */
	bEnableTmdsOutput = false ;
#endif

	if (bEnableTmdsOutput)
		TmdsOutput = 0x0 ; /* TMDS ON */
	else
		TmdsOutput = 0x1 ; /* TMDS OFF */

	BHDM_P_RxDeviceAttached_isr(hHDMI,&DeviceAttached) ;

#if BHDM_CONFIG_DEBUG_RSEN
	BDBG_WRN(("Configure TMDS Data  %s", TmdsOutput ? "OFF" : "ON"));
	BDBG_WRN(("Configure TMDS Clock %s", !DeviceAttached && TmdsOutput ? "OFF" : "ON"));
#endif

	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL) ;
	Register &=
		~( BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, reserved0)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, reserved1)) ;

	/* set TMDS lines to power on*/
	Register &=
		~( BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, PLL_PWRDN)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, TX_CK_PWRDN)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, TX_2_PWRDN)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, TX_1_PWRDN)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, TX_0_PWRDN))  ;

	/* take TMDS lines out of reset */
	Register &=
		~( BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, TX_CK_RESET)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, TX_2_RESET)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, TX_1_RESET)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, TX_0_RESET))  ;

	Register &=
		~( BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, D_RESET)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, A_RESET)) ;
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, Register) ;

	/* set TMDS lines to requested value on/off */
	Register |=
		  BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, PLL_PWRDN, TmdsOutput)

		  /* If an Rx device is still attached,
					the CLOCK signal may be needed to (rx)Sense changes in the clock line to wake up the STB;
	       */
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, TX_CK_PWRDN, !DeviceAttached && TmdsOutput)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, TX_2_PWRDN, TmdsOutput)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, TX_1_PWRDN, TmdsOutput)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, TX_0_PWRDN, TmdsOutput)  ;
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, Register) ;


	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_1) ;
	Register &=
		~( BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, ICPX)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, RZ)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, RN)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, CN)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, LF_ORDER)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, CZ))  ;

	Register |=
		  BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, ICPX, 0x3)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, RZ, 1)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, RN, 0)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, CN, 1)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, LF_ORDER, 1)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, CZ, 1);
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, Register) ;


	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_2) ;
	Register &=
		~( BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, PTAP_ADJ)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, CTAP_ADJ)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, LOWCUR_EN)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, BIASIN_EN)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, NDIV_MOD))	;

	Register |=
		  BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, PTAP_ADJ, 1)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, CTAP_ADJ, 1)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, LOWCUR_EN, 1)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, BIASIN_EN, 1)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, NDIV_MOD, 4) ;

	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, Register) ;

	hHDMI->DeviceStatus.tmds.dataEnabled = bEnableTmdsOutput ;
	BDBG_LEAVE(BHDM_P_EnableTmdsData_isr) ;
	return ;
}


/******************************************************************************
BERR_Code BHDM_P_EnableTmdsClock_isr
Summary: Enable/drive (Display) TMDS Clock lines (Clock)
*******************************************************************************/
void BHDM_P_EnableTmdsClock_isr(
   const BHDM_Handle hHDMI,	/* [in] HDMI handle */
   bool bEnableTmdsClock	/* [in] boolean to enable/disable */
)
{
	uint32_t Register ;
	uint32_t TmdsOutput ;

	BDBG_ENTER(BHDM_P_EnableTmdsClock_isr) ;

#if BHDM_CONFIG_DVO_SUPPORT
	/* TMDS is always off when DVO is enabled */
	bEnableTmdsClock = false ;
#endif

	if (bEnableTmdsClock)
		TmdsOutput = 0x0 ; /* TMDS ON */
	else
		TmdsOutput = 0x1 ; /* TMDS OFF */

#if BHDM_CONFIG_DEBUG_RSEN
	BDBG_WRN(("Configure TMDS Clock %s", !DeviceAttached && TmdsOutput ? "OFF" : "ON"));
#endif

	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL) ;
	Register &=
		~( BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, reserved0)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, reserved1)) ;

	/* set TMDS lines to power on*/
	Register &=
		   BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, TX_CK_PWRDN) ;

	/* take TMDS lines out of reset */
	Register &=
		~( BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, TX_CK_RESET)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, TX_2_RESET)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, TX_1_RESET)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, TX_0_RESET))  ;

	Register &=
		~( BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, D_RESET)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, A_RESET)) ;
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, Register) ;

	/* set TMDS lines to requested value on/off */
	Register |=
		  BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, PLL_PWRDN, TmdsOutput)

		  /* If an Rx device is still attached,
					the CLOCK signal may be needed to (rx)Sense changes in the clock line to wake up the STB;
	       */
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, TX_CK_PWRDN, TmdsOutput) ;
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, Register) ;


	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_1) ;
	Register &=
		~( BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, ICPX)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, RZ)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, RN)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, CN)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, LF_ORDER)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, CZ))  ;

	Register |=
		  BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, ICPX, 0x3)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, RZ, 1)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, RN, 0)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, CN, 1)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, LF_ORDER, 1)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, CZ, 1);
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, Register) ;


	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_2) ;
	Register &=
		~( BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, PTAP_ADJ)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, CTAP_ADJ)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, LOWCUR_EN)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, BIASIN_EN)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, NDIV_MOD))	;

	Register |=
		  BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, PTAP_ADJ, 1)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, CTAP_ADJ, 1)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, LOWCUR_EN, 1)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, BIASIN_EN, 1)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, NDIV_MOD, 4) ;

	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, Register) ;

	hHDMI->DeviceStatus.tmds.clockEnabled = bEnableTmdsClock ;
	BDBG_LEAVE(BHDM_P_EnableTmdsClock_isr) ;
	return ;
}


BERR_Code BHDM_P_ConfigurePhy(const BHDM_Handle hHDMI, const BHDM_Settings *NewHdmiSettings)
{
	BERR_Code rc = BERR_SUCCESS ;
	BSTD_UNUSED(hHDMI) ;
	BSTD_UNUSED(NewHdmiSettings) ;

	/* do nothing for falcon platforms */
	BDBG_ENTER(BHDM_P_ConfigurePhy) ;
	BDBG_LEAVE(BHDM_P_ConfigurePhy) ;

	 return rc ;
}


/* For boot loader usage */
#ifndef BHDM_FOR_BOOTUPDATER
/******************************************************************************
Summary:
Configure the MAI Audio Input Bus
*******************************************************************************/
void BHDM_P_ConfigureInputAudioFmt(
   const BHDM_Handle hHDMI,		   /* [in] HDMI handle */
   const BAVC_HDMI_AudioInfoFrame *stAudioInfoFrame
)
{
	uint32_t Register ;
	uint8_t ChannelMask = 0x03 ;  /* default to 2 channels */

	switch (stAudioInfoFrame->SpeakerAllocation)
	{
	 case BHDM_ChannelAllocation_e_xx__xx__xx__xx__xx___xx_FR__FL :
		ChannelMask = 0x03 ;		break ;

	 case BHDM_ChannelAllocation_e_xx__xx__xx__xx__xx__LFE_FR__FL  :
		ChannelMask = 0x07 ;		break ;

	 case BHDM_ChannelAllocation_e_xx__xx__xx__xx__FC___xx_FR__FL  :
		ChannelMask = 0x0B ;		break ;

	 case BHDM_ChannelAllocation_e_xx__xx__xx__xx__FC__LFE_FR__FL  :
		ChannelMask = 0x0F ;		break ;

	 case BHDM_ChannelAllocation_e_xx__xx__xx__RC__xx___xx_FR__FL :
		ChannelMask = 0x13 ;		break ;

	 case BHDM_ChannelAllocation_e_xx__xx__xx__RC__xx__LFE_FR__FL :
		ChannelMask = 0x17 ;		break ;

	 case BHDM_ChannelAllocation_e_xx__xx__xx__RC__FC___xx_FR__FL :
		ChannelMask = 0x1B ;		break ;

	 case BHDM_ChannelAllocation_e_xx__xx__xx__RC__FC__LFE_FR__FL :
		ChannelMask = 0x1F ;		break ;

	 case BHDM_ChannelAllocation_e_xx__xx__RR__RL__xx___xx_FR__FL :
		ChannelMask = 0x33 ;		break ;

	 case BHDM_ChannelAllocation_e_xx__xx__RR__RL__xx__LFE_FR__FL :
		ChannelMask = 0x37 ;		break ;

	 case BHDM_ChannelAllocation_e_xx__xx__RR__RL__FC___xx_FR__FL :
		ChannelMask = 0x3B ;		break ;

	 case BHDM_ChannelAllocation_e_xx__xx__RR__RL__FC__LFE_FR__FL :
		ChannelMask = 0x3F ;		break ;

	 case BHDM_ChannelAllocation_e_xx__RC__RR__RL__xx___xx_FR__FL :
		ChannelMask = 0x73 ;		break ;

	 case BHDM_ChannelAllocation_e_xx__RC__RR__RL__xx__LFE_FR__FL :
		ChannelMask = 0x73 ;		break ;

	 case BHDM_ChannelAllocation_e_xx__RC__RR__RL__FC___xx_FR__FL :
		ChannelMask = 0x7B ;		break ;

	 case BHDM_ChannelAllocation_e_xx__RC__RR__RL__FC__LFE_FR__FL :
		ChannelMask = 0x7F ;		break ;

	 case BHDM_ChannelAllocation_e_RRC_RLC__RR__RL__FC_LFE_FR__FL :
		ChannelMask = 0xFF ;		break ;

	default :
		BDBG_WRN(("UnSupported Speaker/Channel Mapping; %#X",
			stAudioInfoFrame->SpeakerAllocation)) ;

	}
	/*CP*  10 Configure the MAI Bus */
	/****		Set Channel Mask	*/
	/* clear MAI_BIT_REVERSE bit  - reset value */
	/* set MAI_CHANNEL_MASK = 3   - reset value */

	Register =
		  BCHP_FIELD_DATA(HDMI_MAI_CONFIG, MAI_BIT_REVERSE, 0)
		| BCHP_FIELD_DATA(HDMI_MAI_CONFIG, MAI_CHANNEL_MASK, ChannelMask) ;
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_MAI_CONFIG, Register) ;

	/*CP*  11 Configure Audio */

	/* clear ZERO_DATA_ON_SAMPLE_FLAT		- reset value */
	/* clear AUDIO_SAMPLE_FLAT = 4'd0		- reset value */
	/* clear ZERO_DATA_ON_INACTIVE_CHANNELS - reset value */
	/* clear SAMPLE_PRESENT = 4'd0			- reset value */
	/* clear FORCE_SAMPLE_PRESENT			- reset value */
	/* clear FORCE_B_FRAME					- reset value */
	/* clear B_FRAME = 4'd0					- reset value */
	/* clear B_FRAME_IDENTIFIER = 4'd1					  */
	/* clear AUDIO_LAYOUT					- reset value */
	/* clear FORCE_AUDIO_LAYOUT				- reset value */
	/* clear AUDIO_CEA_MASK = 8'd0			- reset value */
	Register =
		  BCHP_FIELD_DATA(HDMI_AUDIO_PACKET_CONFIG, ZERO_DATA_ON_SAMPLE_FLAT, 1)
		| BCHP_FIELD_DATA(HDMI_AUDIO_PACKET_CONFIG, AUDIO_SAMPLE_FLAT, 0)
		| BCHP_FIELD_DATA(HDMI_AUDIO_PACKET_CONFIG, ZERO_DATA_ON_INACTIVE_CHANNELS, 1)
		| BCHP_FIELD_DATA(HDMI_AUDIO_PACKET_CONFIG, SAMPLE_PRESENT, 0)
		| BCHP_FIELD_DATA(HDMI_AUDIO_PACKET_CONFIG, FORCE_SAMPLE_PRESENT, 0)
		| BCHP_FIELD_DATA(HDMI_AUDIO_PACKET_CONFIG, FORCE_B_FRAME, 0)
		| BCHP_FIELD_DATA(HDMI_AUDIO_PACKET_CONFIG, B_FRAME, 0)
		| BCHP_FIELD_DATA(HDMI_AUDIO_PACKET_CONFIG, B_FRAME_IDENTIFIER, 1)
		| BCHP_FIELD_DATA(HDMI_AUDIO_PACKET_CONFIG, AUDIO_LAYOUT, 0)
		| BCHP_FIELD_DATA(HDMI_AUDIO_PACKET_CONFIG, FORCE_AUDIO_LAYOUT, 0)
		| BCHP_FIELD_DATA(HDMI_AUDIO_PACKET_CONFIG, AUDIO_CEA_MASK, ChannelMask) ;
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_AUDIO_PACKET_CONFIG, Register) ;


#if BHDM_CONFIG_DEBUG_AUDIO_INFOFRAME
	BDBG_WRN(("Channel Mask: %#x", ChannelMask)) ;
#endif
}


#if BHDM_CONFIG_PLL_KICKSTART_WORKAROUND
BERR_Code BHDM_Debug_GetPllLockStatus(
	const BHDM_Handle hHDMI, BHDM_DEBUG_PLL_LOCK_STATUS *stPllStatus)
{
	uint32_t Register ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_RM_VCXO_CTRL) ;
	stPllStatus->Lock = BCHP_GET_FIELD_DATA(Register, HDMI_RM_VCXO_CTRL, LOCK) ;
	stPllStatus->VcxoRegister =  Register ;
	stPllStatus->uiPllKickStartCount = hHDMI->uiPllKickStartCount ;

	if (!stPllStatus->Lock)
	{
		BDBG_ERR(("_-_-_-_-_-_- HDMI PLL UnLocked _-_-_-_-_-_-")) ;
		BDBG_ERR(("VCXO Register %x", stPllStatus->VcxoRegister)) ;
	}

	return BERR_SUCCESS ;
}


BERR_Code BHDM_Debug_KickStartPll(const BHDM_Handle hHDMI)
{
	uint32_t Register ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	/* kick start PLL */
	/*
	FILRSTB -----|__________________|---
	DIVRST	________|-----------|_______
	RESETB	------------|___|-----------
	*/

	hHDMI->uiPllKickStartCount++ ;
	BDBG_WRN(("Kickstarting HDMI PLL... Kickstart Count: %d",
		hHDMI->uiPllKickStartCount)) ;

	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_RM_VCXO_CTRL) ;
	Register &= ~BCHP_MASK(HDMI_RM_VCXO_CTRL, reserved0) ;

	/* FILTER RESET */
	Register &= ~BCHP_MASK(HDMI_RM_VCXO_CTRL, FILRSTB) ;  /* 0 */
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_RM_VCXO_CTRL, Register) ;

		/* DIV_RST */
		Register &= ~BCHP_MASK(HDMI_RM_VCXO_CTRL, DIV_RST) ;		 /* 0 */
		BREG_Write32(hHDMI->hRegister, BCHP_HDMI_RM_VCXO_CTRL, Register) ;

		Register |= BCHP_MASK(HDMI_RM_VCXO_CTRL, DIV_RST) ;			 /* 1 */
		BREG_Write32(hHDMI->hRegister, BCHP_HDMI_RM_VCXO_CTRL, Register) ;
		BKNI_Delay(1) ;  /* active high reset */

			/* INT_RESTB */
			Register |= BCHP_MASK(HDMI_RM_VCXO_CTRL, INT_RESETB) ;		 /* 1 */
			BREG_Write32(hHDMI->hRegister, BCHP_HDMI_RM_VCXO_CTRL, Register) ;

			Register &= ~BCHP_MASK(HDMI_RM_VCXO_CTRL, INT_RESETB) ;		 /* 0 */
			BREG_Write32(hHDMI->hRegister, BCHP_HDMI_RM_VCXO_CTRL, Register) ;
			BKNI_Delay(1) ;  /* active low reset */

			Register |= BCHP_MASK(HDMI_RM_VCXO_CTRL, INT_RESETB) ;		 /* 1 */
			BREG_Write32(hHDMI->hRegister, BCHP_HDMI_RM_VCXO_CTRL, Register) ;
			BKNI_Delay(1) ;  /* active low reset */

		Register &= ~BCHP_MASK(HDMI_RM_VCXO_CTRL, DIV_RST) ;		 /* 0 */
		BREG_Write32(hHDMI->hRegister, BCHP_HDMI_RM_VCXO_CTRL, Register) ;
		BKNI_Delay(1) ;

	Register |= BCHP_MASK(HDMI_RM_VCXO_CTRL, FILRSTB) ;   /* 1 */
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_RM_VCXO_CTRL, Register) ;

	/* allow some PLL settle time */
	BKNI_Delay(10) ;

	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_RM_CONTROL) ;
	BDBG_WRN(("HDMI_RM_CONTROL: %x", Register)) ;

	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_RM_RATE_RATIO) ;
	BDBG_WRN(("HDMI_RM_RATE_RATIO: %x", Register)) ;

	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_RM_SAMPLE_INC) ;
	BDBG_WRN(("HDMI_RM_SAMPLE_INC	: %x", Register)) ;

	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_RM_PHASE_INC) ;
	BDBG_WRN((" HDMI_RM_PHASE_INC	: %x", Register)) ;

	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_RM_INTEGRATOR) ;
	BDBG_WRN(("HDMI_RM_INTEGRATOR	: %x", Register)) ;

	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_RM_VCXO_CTRL) ;
	BDBG_WRN(("HDMI_RM_VCXO_CTRL	: %x", Register)) ;

	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_RM_FORMAT	) ;
	BDBG_WRN(("HDMI_RM_FORMAT	: %x", Register)) ;

	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_ANALOG_CTL	) ;
	BDBG_WRN(("HDMI_ANALOG_CTL	: %x", Register)) ;

	return BERR_SUCCESS ;
}
#endif
#endif /* #ifndef BHDM_FOR_BOOTUPDATER */


/****************************
**			PRIVATE FUNCTIONS
*****************************/
void BHDM_P_PowerOnPhy (const BHDM_Handle hHDMI)
{
	uint32_t Register;

	/* Assert the fields to prevent DRIFT FIFO UNDERFLOW when trying to
		authenticate with DVI receivers */
	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_FIFO_CTL);
	Register |= BCHP_FIELD_DATA(HDMI_FIFO_CTL, USE_EMPTY, 1)
				| BCHP_FIELD_DATA(HDMI_FIFO_CTL, USE_FULL, 1);
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_FIFO_CTL, Register) ;


	/* PR 28685: Enable Random bit block power pown. */
	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_0);
	Register &= ~ BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, RND_PWRDN);
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, Register) ;

	/* power PLL etc. */
	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL);
	Register &=
		~( BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, PLL_PWRDN)
		| BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, REF_COMP_PWRDN)
		| BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, PWRDN_BG)
		| BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, PWRDN_REFAMP)) ;
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, Register) ;

	hHDMI->phyPowered = true ;
	return;
}


void BHDM_P_PowerOffPhy (const BHDM_Handle hHDMI)
{
	uint32_t Register;


	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_0);
	Register &= ~ BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, RND_PWRDN);
	Register |= BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, RND_PWRDN, 1);
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, Register) ;

	/* power PLL etc. */
	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL);
	Register &=
		~( BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, PLL_PWRDN)
		| BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, REF_COMP_PWRDN)
		| BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, PWRDN_BG)
		| BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, PWRDN_REFAMP)) ;

	Register |=
			BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, PLL_PWRDN, 1)
		  | BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, REF_COMP_PWRDN, 1)
		  | BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, PWRDN_BG, 1)
		  | BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, PWRDN_REFAMP, 1);
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, Register) ;

	hHDMI->phyPowered = false ;
	return;
}


void BHDM_P_SetPreEmphasisMode (const BHDM_Handle hHDMI, uint8_t uValue, uint8_t uDriverAmp)
{
	uint32_t Register;

	Register = BREG_Read32( hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_0) ;
	Register &= ~(BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, DRV_AMP)
			  | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_CK)
			  | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_2)
			  | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_1)
			  | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_0));

	Register |= BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, DRV_AMP, uDriverAmp)
			| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_CK, uValue)
			| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_2, uValue)
			| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_1, uValue)
			| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_0, uValue) ;
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, Register) ;

	return;
}


BERR_Code BHDM_P_GetPreEmphasisConfiguration (
	const BHDM_Handle hHDMI,
	BHDM_PreEmphasis_Configuration *stPreEmphasisConfig
)
{
	BERR_Code rc = BERR_SUCCESS;
	uint32_t Register;

	stPreEmphasisConfig->eCore = BHDM_Core65nm ;

	Register = BREG_Read32( hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_0) ;
		stPreEmphasisConfig->uiDriverAmp =
			BCHP_GET_FIELD_DATA(Register,HDMI_TX_PHY_HDMI_TX_PHY_CTL_0,DRV_AMP);
		stPreEmphasisConfig->uiPreEmphasis_CK =
			BCHP_GET_FIELD_DATA(Register,HDMI_TX_PHY_HDMI_TX_PHY_CTL_0,PREEMP_CK);
		stPreEmphasisConfig->uiPreEmphasis_Ch2 =
			BCHP_GET_FIELD_DATA(Register,HDMI_TX_PHY_HDMI_TX_PHY_CTL_0,PREEMP_2);
		stPreEmphasisConfig->uiPreEmphasis_Ch1 =
			BCHP_GET_FIELD_DATA(Register,HDMI_TX_PHY_HDMI_TX_PHY_CTL_0,PREEMP_1);
		stPreEmphasisConfig->uiPreEmphasis_Ch0 =
			BCHP_GET_FIELD_DATA(Register,HDMI_TX_PHY_HDMI_TX_PHY_CTL_0,PREEMP_0);

	return rc;
}


BERR_Code BHDM_P_SetPreEmphasisConfiguration(
	const BHDM_Handle hHDMI,
	BHDM_PreEmphasis_Configuration *stPreEmphasisConfig)
{
	BERR_Code rc = BERR_SUCCESS;
	uint32_t Register;

	/* Set Preemphasis configurations */
	Register = BREG_Read32( hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_0) ;
	Register &= ~(
		  BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, DRV_AMP)
		| BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_CK)
		| BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_2)
		| BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_1)
		| BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_0)) ;

	Register |=
		  BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, DRV_AMP, stPreEmphasisConfig->uiDriverAmp)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_CK, stPreEmphasisConfig->uiPreEmphasis_CK)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_2, stPreEmphasisConfig->uiPreEmphasis_Ch2)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_1, stPreEmphasisConfig->uiPreEmphasis_Ch1)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_0, stPreEmphasisConfig->uiPreEmphasis_Ch0) ;

	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, Register) ;

	return rc;
}



/* For boot loader usage */
#ifndef BHDM_FOR_BOOTUPDATER
void BHDM_P_ClearHotPlugInterrupt(
   const BHDM_Handle hHDMI		/* [in] HDMI handle */
)
{
	uint32_t Register ;

	/* MI must have 1 written; followed by a 0 */
	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_HOTPLUG_INT) ;
	Register &= ~BCHP_MASK(HDMI_HOTPLUG_INT, MI) ;
	Register |= BCHP_FIELD_DATA(HDMI_HOTPLUG_INT, MI, 1) ;
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_HOTPLUG_INT, Register) ;

	Register &= ~BCHP_MASK(HDMI_HOTPLUG_INT, MI) ;
	Register |= BCHP_FIELD_DATA(HDMI_HOTPLUG_INT, MI, 0) ;
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_HOTPLUG_INT, Register) ;

	return;
}

void BHDM_P_CheckHotPlugInterrupt(
	const BHDM_Handle hHDMI,		 /* [in] HDMI handle */
	uint8_t *bHotPlugInterrupt	/* [out] Interrupt asserted or not */
)
{
	uint32_t Register ;

	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_HOTPLUG_INT) ;

	if (Register & BCHP_MASK(HDMI_HOTPLUG_INT, MI))
		*bHotPlugInterrupt = true;
	else
		*bHotPlugInterrupt = false;

	return;
}
#endif

void BHDM_P_RxDeviceAttached_isr(
	const BHDM_Handle hHDMI,		 /* [in] HDMI handle */
	uint8_t *bDeviceAttached	/* [out] Device Attached Status  */
)
{
	uint32_t Register ;

	BDBG_ENTER(BHDM_P_RxDeviceAttached) ;

	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_HOTPLUG) ;
	*bDeviceAttached =
		BCHP_GET_FIELD_DATA(Register, HDMI_HOTPLUG, HOTPLUG_STATE) ;

	BDBG_LEAVE(BHDM_P_RxDeviceAttached) ;
	return ;
}

