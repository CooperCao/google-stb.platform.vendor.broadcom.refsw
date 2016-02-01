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

#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif


BDBG_MODULE(BHDM_PRIV) ;

/* For boot loader usage */
#ifndef BHDM_FOR_BOOTUPDATER
/******************************************************************************
Summary:
HDMI Pixel Clock Rate Text - Useful for debug messages
NOTE: These entries match the number of entries in the BHDM_InputPixelClock enum
*******************************************************************************/


/* HDMI Rate Manager now updated by VDC for smoother transitions */


/******************************************************************************
BERR_Code BHDM_SetAudioMute
Summary: Implements HDMI Audio (only) mute enable/disable.
*******************************************************************************/
BERR_Code BHDM_SetAudioMute(
	const BHDM_Handle hHDMI,			   /* [in] HDMI handle */
	bool bEnableAudioMute		   /* [in] boolean to enable/disable */
)
{
	BERR_Code rc = BERR_SUCCESS;
	BDBG_ENTER(BHDM_SetAudioMute) ;

#if BHDM_CONFIG_AUDIO_MAI_BUS_DISABLE_SUPPORT
{
	uint32_t Register ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	/* AudioMute valid for HDMI only */
	if	(hHDMI->DeviceSettings.eOutputFormat != BHDM_OutputFormat_eHDMIMode)
	{
		BDBG_ERR(("Audio Mute only applies in HDMI mode."));
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		goto done ;
	}
	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_MAI_CONFIG) ;
	if (bEnableAudioMute) {
		Register |= BCHP_FIELD_DATA(HDMI_MAI_CONFIG, DISABLE_MAI_AUDIO, 1) ;
	}
	else
	{
		Register &= ~BCHP_MASK(HDMI_MAI_CONFIG, DISABLE_MAI_AUDIO);
	}
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_MAI_CONFIG, Register) ;

	hHDMI->AudioMuteState = bEnableAudioMute ;
	BDBG_MSG(("AudioMute %d", bEnableAudioMute)) ;
}

done:
#else

	BSTD_UNUSED(hHDMI) ;
	BSTD_UNUSED(bEnableAudioMute) ;

#endif

	BDBG_LEAVE(BHDM_SetAudioMute) ;
	return rc ;
}  /* END BHDM_SetAudioMute */


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

#if BHDM_CONFIG_AUDIO_MAI_BUS_DISABLE_SUPPORT
	uint8_t DisableMai = hHDMI->AudioMuteState == true ? 1 : 0;
#endif

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
		ChannelMask = 0x77 ;		break ;

	 case BHDM_ChannelAllocation_e_xx__RC__RR__RL__FC___xx_FR__FL :
		ChannelMask = 0x7B ;		break ;

	 case BHDM_ChannelAllocation_e_xx__RC__RR__RL__FC__LFE_FR__FL :
		ChannelMask = 0x7F ;		break ;

	case BHDM_ChannelAllocation_e_RRC_RLC__RR__RL__xx__xx_FR__FL :
		ChannelMask = 0xF3 ;	   break ;

	case BHDM_ChannelAllocation_e_RRC_RLC__RR__RL__xx_LFE_FR__FL :
		ChannelMask = 0xF7 ;	   break ;

	case BHDM_ChannelAllocation_e_RRC_RLC__RR__RL__FC__xx_FR__FL:
		ChannelMask = 0xFB ;	   break ;

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

	Register = BCHP_FIELD_DATA(HDMI_MAI_CONFIG, MAI_BIT_REVERSE, 0)
#if BHDM_CONFIG_AUDIO_MAI_BUS_DISABLE_SUPPORT
		| BCHP_FIELD_DATA(HDMI_MAI_CONFIG, DISABLE_MAI_AUDIO, DisableMai)
#endif
		| BCHP_FIELD_DATA(HDMI_MAI_CONFIG, MAI_CHANNEL_MASK, 0xFF) ;
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
	BDBG_MSG(("Channel Mask: %#x", ChannelMask)) ;
#endif
}


/******************************************************************************
Summary:
	Set pixel data override
*******************************************************************************/
BERR_Code BHDM_SetPixelDataOverride(
	const BHDM_Handle hHDMI,		   /* [in] HDMI handle */
	uint8_t red,
	uint8_t green,
	uint8_t blue
)
{
	BERR_Code rc = BERR_SUCCESS;
	uint32_t Register;

	uint16_t uiRed12bits   = red;
	uint16_t uiGreen12bits = green;
	uint16_t uiBlue12bits  = blue;

	BDBG_ENTER(BHDM_SetPixelDataOverride) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;


#if BHDM_CONFIG_PIXEL_OVERRIDE_UPDATE
	/* Red */
	Register = BREG_Read32(hHDMI->hRegister, BCHP_DVP_HT_TVG_BAR_CFG_1A) ;
	Register &= ~ BCHP_MASK(DVP_HT_TVG_BAR_CFG_1A, CH2);
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_TVG_BAR_CFG_1A, Register);

	Register |= BCHP_FIELD_DATA(DVP_HT_TVG_BAR_CFG_1A, CH2, (uiRed12bits << 4));
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_TVG_BAR_CFG_1A, Register);


	Register = BREG_Read32(hHDMI->hRegister, BCHP_DVP_HT_TVG_BAR_CFG_2A) ;
	Register &= ~ BCHP_MASK(DVP_HT_TVG_BAR_CFG_2A, CH2);
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_TVG_BAR_CFG_2A, Register);

	Register |= BCHP_FIELD_DATA(DVP_HT_TVG_BAR_CFG_2A, CH2, (uiRed12bits << 4));
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_TVG_BAR_CFG_2A, Register);


	/* Green */
	Register = BREG_Read32(hHDMI->hRegister, BCHP_DVP_HT_TVG_BAR_CFG_1A) ;
	Register &= ~ BCHP_MASK(DVP_HT_TVG_BAR_CFG_1A, CH1);
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_TVG_BAR_CFG_1A, Register);

	Register |= BCHP_FIELD_DATA(DVP_HT_TVG_BAR_CFG_1A, CH1, (uiGreen12bits << 4));
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_TVG_BAR_CFG_1A, Register);


	Register = BREG_Read32(hHDMI->hRegister, BCHP_DVP_HT_TVG_BAR_CFG_2A) ;
	Register &= ~ BCHP_MASK(DVP_HT_TVG_BAR_CFG_2A, CH1);
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_TVG_BAR_CFG_2A, Register);

	Register |= BCHP_FIELD_DATA(DVP_HT_TVG_BAR_CFG_2A, CH1, (uiGreen12bits << 4));
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_TVG_BAR_CFG_2A, Register);


	/* Blue */
	Register = BREG_Read32(hHDMI->hRegister, BCHP_DVP_HT_TVG_BAR_CFG_1B) ;
	Register &= ~ BCHP_MASK(DVP_HT_TVG_BAR_CFG_1B, CH0);
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_TVG_BAR_CFG_1B, Register);

	Register |= BCHP_FIELD_DATA(DVP_HT_TVG_BAR_CFG_1B, CH0, (uiBlue12bits << 4));
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_TVG_BAR_CFG_1B, Register);


	Register = BREG_Read32(hHDMI->hRegister, BCHP_DVP_HT_TVG_BAR_CFG_2B) ;
	Register &= ~ BCHP_MASK(DVP_HT_TVG_BAR_CFG_2B, CH0);
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_TVG_BAR_CFG_2B, Register);

	Register |= BCHP_FIELD_DATA(DVP_HT_TVG_BAR_CFG_2B, CH0, (uiBlue12bits << 4));
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_TVG_BAR_CFG_2B, Register);


	/* Setup mode & Enable */
	Register = BREG_Read32(hHDMI->hRegister, BCHP_DVP_HT_TVG_CFG_0) ;
	Register &= ~( BCHP_MASK(DVP_HT_TVG_CFG_0, PATTERN_SELECT)
				| BCHP_MASK(DVP_HT_TVG_CFG_0, TEST_MODE));
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_TVG_CFG_0, Register) ;

	Register |= BCHP_FIELD_DATA(DVP_HT_TVG_CFG_0, PATTERN_SELECT, 4)
			| BCHP_FIELD_DATA(DVP_HT_TVG_CFG_0, TEST_MODE, 3);
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_TVG_CFG_0, Register);

#else
	/* Red */
	Register = BREG_Read32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_10) ;
	Register &=
		~( BCHP_MASK(DVP_HT_HDMI_TX_0_TDG_CFG_10, DVO_0_FLAT_FIELD_1_RED)
		 | BCHP_MASK(DVP_HT_HDMI_TX_0_TDG_CFG_10, DVO_0_FLAT_FIELD_2_RED) );
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_10, Register);

	Register |= BCHP_FIELD_DATA(DVP_HT_HDMI_TX_0_TDG_CFG_10, DVO_0_FLAT_FIELD_1_RED, (uiRed12bits << 4))
			| BCHP_FIELD_DATA(DVP_HT_HDMI_TX_0_TDG_CFG_10, DVO_0_FLAT_FIELD_2_RED, (uiRed12bits << 4)) ;
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_10, Register);

	/* Green */
	Register = BREG_Read32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_11) ;
	Register &=
		~( BCHP_MASK(DVP_HT_HDMI_TX_0_TDG_CFG_11, DVO_0_FLAT_FIELD_1_GREEN)
		| BCHP_MASK(DVP_HT_HDMI_TX_0_TDG_CFG_11, DVO_0_FLAT_FIELD_2_GREEN) );
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_11, Register);

	Register |= BCHP_FIELD_DATA(DVP_HT_HDMI_TX_0_TDG_CFG_11, DVO_0_FLAT_FIELD_1_GREEN, (uiGreen12bits << 4))
			| BCHP_FIELD_DATA(DVP_HT_HDMI_TX_0_TDG_CFG_11, DVO_0_FLAT_FIELD_2_GREEN, (uiGreen12bits << 4)) ;
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_11, Register);

	/* Blue */
	Register = BREG_Read32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_12) ;
	Register &=
		~( BCHP_MASK(DVP_HT_HDMI_TX_0_TDG_CFG_12, DVO_0_FLAT_FIELD_1_BLUE)
		| BCHP_MASK(DVP_HT_HDMI_TX_0_TDG_CFG_12, DVO_0_FLAT_FIELD_2_BLUE) );
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_12, Register);

	Register |= BCHP_FIELD_DATA(DVP_HT_HDMI_TX_0_TDG_CFG_12, DVO_0_FLAT_FIELD_1_BLUE, (uiBlue12bits << 4))
			| BCHP_FIELD_DATA(DVP_HT_HDMI_TX_0_TDG_CFG_12, DVO_0_FLAT_FIELD_2_BLUE, (uiBlue12bits << 4)) ;
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_12, Register);


	/* Setup mode */
	Register = BREG_Read32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_13) ;
	Register &= ~BCHP_MASK(DVP_HT_HDMI_TX_0_TDG_CFG_13, DVO_0_GEN_TEST_MODE) ;
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_13, Register) ;

	Register |= BCHP_FIELD_DATA(DVP_HT_HDMI_TX_0_TDG_CFG_13, DVO_0_GEN_TEST_MODE, 4);
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_13, Register);


	/* Enable */
	Register = BREG_Read32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_0) ;
	Register &= ~BCHP_MASK(DVP_HT_HDMI_TX_0_TDG_CFG_0, DVO_0_TEST_MODE) ;
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_0, Register) ;

	Register |= BCHP_FIELD_DATA(DVP_HT_HDMI_TX_0_TDG_CFG_0, DVO_0_TEST_MODE, 3);
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_0, Register);
#endif

	BDBG_LEAVE(BHDM_SetPixelDataOverride) ;
	return rc;
}


/******************************************************************************
Summary:
	Clear pixel data override
*******************************************************************************/
BERR_Code BHDM_ClearPixelDataOverride(
	const BHDM_Handle hHDMI		   /* [in] HDMI handle */
)
{
	BERR_Code rc = BERR_SUCCESS;
	uint32_t Register;

	BDBG_ENTER(BHDM_ClearPixelDataOverride) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;


#if BHDM_CONFIG_PIXEL_OVERRIDE_UPDATE

	/* Disable */
	Register = BREG_Read32(hHDMI->hRegister, BCHP_DVP_HT_TVG_CFG_0) ;
	Register &= ~ BCHP_MASK(DVP_HT_TVG_CFG_0, TEST_MODE);
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_TVG_CFG_0, Register) ;

	Register |= BCHP_FIELD_DATA(DVP_HT_TVG_CFG_0, TEST_MODE, 0);
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_TVG_CFG_0, Register);

#else
	/* Disable */
	Register = BREG_Read32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_0) ;
	Register &= ~BCHP_MASK(DVP_HT_HDMI_TX_0_TDG_CFG_0, DVO_0_TEST_MODE) ;
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_0, Register) ;

	Register |= BCHP_FIELD_DATA(DVP_HT_HDMI_TX_0_TDG_CFG_0, DVO_0_TEST_MODE, 0);
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_0, Register);
#endif

	BDBG_LEAVE(BHDM_ClearPixelDataOverride) ;
	return rc;
}


/******************************************************************************
Summary:
	Wait for stable video in HDMI core a specific amount of time
*******************************************************************************/
BERR_Code BHDM_WaitForStableVideo(
	const BHDM_Handle hHDMI,			/* [in] HDMI handle */
	uint32_t stablePeriod,		/* [in] Period of time video should be stable */
	uint32_t maxWait			/* [in] Max amount of time to wait */
)
{
	BERR_Code rc = BERR_TIMEOUT;
	uint32_t Register;

	uint32_t waitThusFar = 0;
	uint32_t stableTime  = 0;
	uint32_t waitIncr	 = 10;
	uint8_t bHPInterrupt = false;
	uint32_t driftFifoErrors = 0;
	uint32_t prevLineCount1 = 0;
	uint32_t prevLineCount2 = 0;
	uint32_t currLineCount;
	bool masterMode;

	BDBG_ENTER(BHDM_WaitForStableVideo);
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BHDM_ClearHotPlugInterrupt(hHDMI);

	BHDM_GetHdmiDataTransferMode(hHDMI, &masterMode);

	while (waitThusFar < maxWait)
	{
		uint8_t notStable = false;

		/*
		* First, ensure video is really flowing in from the VEC
		*/
		Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_FORMAT_DET_7) ;
		currLineCount = BCHP_GET_FIELD_DATA(Register, HDMI_FORMAT_DET_7, UUT_CURRENT_LINE_COUNT) ;
		if (currLineCount == prevLineCount1 && currLineCount == prevLineCount2)
		{
			notStable = true;
		}
		else
		{
			prevLineCount2 = prevLineCount1;
			prevLineCount1 = currLineCount;

			Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_FORMAT_DET_UPDATE_STATUS) ;

			if (Register & (BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_HAP)
							| BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_VBLANK2)
							| BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_VBLANK1)
							| BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_HSYNC_HIGH)
							| BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_HSYNC_LOW)
							| BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_HFP)
							| BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_HSP)
							| BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_HBP)
							| BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_VAL1)
							| BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_VFP_1)
							| BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_VBP_1)
							| BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_VSPO_1)
							| BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_VAL2)
							| BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_VFP_2)
							| BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_VBP_2)
							| BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_VSPO_2)))
			{
				Register = BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_HAP)
							| BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_VBLANK2)
							| BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_VBLANK1)
							| BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_HSYNC_HIGH)
							| BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_HSYNC_LOW)
							| BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_HFP)
							| BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_HSP)
							| BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_HBP)
							| BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_VAL1)
							| BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_VFP_1)
							| BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_VBP_1)
							| BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_VSPO_1)
							| BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_VAL2)
							| BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_VFP_2)
							| BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_VBP_2)
							| BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_VSPO_2) ;

				BREG_Write32(hHDMI->hRegister, BCHP_HDMI_FORMAT_DET_UPDATE_CLEAR, Register) ;
				BREG_Write32(hHDMI->hRegister, BCHP_HDMI_FORMAT_DET_UPDATE_CLEAR, 0) ;
				notStable = true;
			}

			else if (masterMode == false)
			{
				/*
				* Capture (pointers) status before we read it.
				*/
				Register = BREG_Read32( hHDMI->hRegister, BCHP_HDMI_FIFO_CTL) ;

				Register &= ~BCHP_MASK(HDMI_FIFO_CTL, CAPTURE_POINTERS) ;
				BREG_Write32(hHDMI->hRegister, BCHP_HDMI_FIFO_CTL, Register) ;

				Register |= BCHP_FIELD_DATA(HDMI_FIFO_CTL, CAPTURE_POINTERS, 1) ;
				BREG_Write32(hHDMI->hRegister, BCHP_HDMI_FIFO_CTL, Register) ;

				Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_READ_POINTERS) ;
				if (Register & (BCHP_MASK(HDMI_READ_POINTERS, DRIFT_UNDERFLOW)
								| BCHP_MASK(HDMI_READ_POINTERS, DRIFT_OVERFLOW)))
				{
					notStable = true;

					/*
					* Re-center the Drift FIFO if we get excessive overflow or underflow
					* errors. There is a bug with the 76xx where the auto re-center
					* logic (use_full, use_empty) does not work as expected in terms of
					* clearing these errors.
					*/
					if (++driftFifoErrors % 10 == 0)
					{
						Register = BREG_Read32( hHDMI->hRegister, BCHP_HDMI_FIFO_CTL) ;

						Register &= ~BCHP_MASK(HDMI_FIFO_CTL, RECENTER) ;
						BREG_Write32(hHDMI->hRegister, BCHP_HDMI_FIFO_CTL, Register) ;

						Register |= BCHP_FIELD_DATA(HDMI_FIFO_CTL, RECENTER, 1) ;
						BREG_Write32(hHDMI->hRegister, BCHP_HDMI_FIFO_CTL, Register) ;
					}
				}
			}
		}

		BKNI_Sleep(waitIncr);
		waitThusFar += waitIncr;

		if (notStable == false)
		{
			stableTime += waitIncr;
			if (stableTime >= stablePeriod)
			{
				rc = BERR_SUCCESS;
				goto done;
			}
		}
		else
		{
			stableTime = 0;
		}

		BHDM_CheckHotPlugInterrupt(hHDMI, &bHPInterrupt);
		if (bHPInterrupt == true)
			goto done;
	}

done:
	BDBG_LEAVE(BHDM_WaitForStableVideo);
	return rc;

}

#endif /*#ifndef BHDM_FOR_BOOTUPDATER */


/******************************************************************************
BERR_Code BHDM_EnableTmdsOutput_isr
Summary: Enable (Display) TMDS Output
*******************************************************************************/
void BHDM_P_EnableTmdsData_isr(
   const BHDM_Handle hHDMI,		/* [in] HDMI handle */
   bool bEnableTmdsOutput	/* [in] boolean to enable/disable */
)
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t TmdsOutput ;
	uint8_t DeviceAttached ;
	uint8_t ClockSetting ;

	BDBG_ENTER(BHDM_P_EnableTmdsData_isr) ;

	hRegister = hHDMI->hRegister ;
#if BHDM_CONFIG_DVO_SUPPORT
	/* TMDS is always off when DVO is enabled */
	bEnableTmdsOutput = false ;
#endif

	if (bEnableTmdsOutput)
		TmdsOutput = 0x0 ; /* TMDS ON */
	else
		TmdsOutput = 0x1 ; /* TMDS OFF */

	BHDM_P_RxDeviceAttached_isr(hHDMI, &DeviceAttached) ;

#if BHDM_CONFIG_DEBUG_RSEN
	BDBG_WRN(("Configure TMDS Data  %s", TmdsOutput ? "OFF" : "ON"));
#endif

	Register = BREG_Read32(hRegister, BCHP_HDMI_TX_PHY_POWERDOWN_CTL) ;

		/* remember Clock Setting for restoration */
		#if BHDM_CONFIG_SWAP_DEFAULT_PHY_CHANNELS
		ClockSetting = BCHP_GET_FIELD_DATA(Register, HDMI_TX_PHY_POWERDOWN_CTL, TX_2_PWRDN) ;
		#else
		ClockSetting = BCHP_GET_FIELD_DATA(Register, HDMI_TX_PHY_POWERDOWN_CTL, TX_CK_PWRDN) ;
		#endif

	/* clear previous TMDS settings */
	Register &=
		~( BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, TX_CK_PWRDN)
		 | BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, TX_2_PWRDN)
		 | BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, TX_1_PWRDN)
		 | BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, TX_0_PWRDN))  ;

	/* set TMDS lines to requested value on/off */
#if BHDM_CONFIG_SWAP_DEFAULT_PHY_CHANNELS
	Register |=
		  /* If an Rx device is still attached,
			the CLOCK signal may be needed to (rx)Sense changes in the clock line to wake up the STB;
	       */

		/* <<<SWAPPED>>> TX_2 is the CLOCK; TX_CK is CHANNEL 2 */
		  BCHP_FIELD_DATA(HDMI_TX_PHY_POWERDOWN_CTL, TX_2_PWRDN, ClockSetting)

		  /* set remaining channels as requested */
		| BCHP_FIELD_DATA(HDMI_TX_PHY_POWERDOWN_CTL, TX_CK_PWRDN, TmdsOutput)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_POWERDOWN_CTL, TX_1_PWRDN, TmdsOutput)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_POWERDOWN_CTL, TX_0_PWRDN, TmdsOutput)  ;
#else
	Register |=
		  /* If an Rx device is still attached,
			the CLOCK signal may be needed to (rx)Sense changes in the clock line to wake up the STB;
	       */
		  BCHP_FIELD_DATA(HDMI_TX_PHY_POWERDOWN_CTL, TX_CK_PWRDN, ClockSetting)

		  /* set remaining channels as requested */
		| BCHP_FIELD_DATA(HDMI_TX_PHY_POWERDOWN_CTL, TX_2_PWRDN, TmdsOutput)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_POWERDOWN_CTL, TX_1_PWRDN, TmdsOutput)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_POWERDOWN_CTL, TX_0_PWRDN, TmdsOutput)  ;
#endif

	BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_POWERDOWN_CTL, Register) ;

	if (bEnableTmdsOutput)
	{
		BHDM_MONITOR_P_StatusChanges_isr(hHDMI) ;
	}

	hHDMI->DeviceStatus.tmds.dataEnabled = bEnableTmdsOutput ;
	BDBG_LEAVE(BHDM_P_EnableTmdsData_isr) ;
}


/******************************************************************************
BERR_Code BHDM_EnableTmdsClock_isr
Summary: Enable/Disable  TMDS Clock
*******************************************************************************/
void BHDM_P_EnableTmdsClock_isr(
   const BHDM_Handle hHDMI,		/* [in] HDMI handle */
   bool bEnableTmdsClock	/* [in] boolean to enable/disable */
)
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t TmdsOutput ;

	BDBG_ENTER(BHDM_P_EnableTmdsClock_isr) ;

	hRegister = hHDMI->hRegister ;
#if BHDM_CONFIG_DVO_SUPPORT
	/* TMDS is always off when DVO is enabled */
	bEnableTmdsClock = false ;
#endif

	if (bEnableTmdsClock)
		TmdsOutput = 0x0 ; /* TMDS ON */
	else
		TmdsOutput = 0x1 ; /* TMDS OFF */

#if BHDM_CONFIG_DEBUG_RSEN
	BDBG_WRN(("Configure TMDS Clock %s", bEnableTmdsClock ? "OFF" : "ON"));
#endif

	Register = BREG_Read32(hRegister, BCHP_HDMI_TX_PHY_POWERDOWN_CTL) ;
#if BHDM_CONFIG_SWAP_DEFAULT_PHY_CHANNELS
	/* set TMDS Clock */
		Register &=
			~( BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, TX_2_PWRDN) )  ;

		/* set TMDS CK to requested value on/off */
		Register |=
			BCHP_FIELD_DATA(HDMI_TX_PHY_POWERDOWN_CTL, TX_2_PWRDN, TmdsOutput) ;
#else
	/* set TMDS Clock */
		Register &=
			~( BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, TX_CK_PWRDN) )  ;

		/* set TMDS CK to requested value on/off */
		Register |=
			BCHP_FIELD_DATA(HDMI_TX_PHY_POWERDOWN_CTL, TX_CK_PWRDN, TmdsOutput) ;

#endif
	BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_POWERDOWN_CTL, Register) ;

	if (bEnableTmdsClock)
	{
		/* take TMDS lines out of reset */
		Register = BREG_Read32(hRegister, BCHP_HDMI_TX_PHY_RESET_CTL) ;
			Register &=
				~( BCHP_MASK(HDMI_TX_PHY_RESET_CTL, TX_CK_RESET)
				 | BCHP_MASK(HDMI_TX_PHY_RESET_CTL, TX_2_RESET)
				 | BCHP_MASK(HDMI_TX_PHY_RESET_CTL, TX_1_RESET)
				 | BCHP_MASK(HDMI_TX_PHY_RESET_CTL, TX_0_RESET))  ;

			Register |=
				  BCHP_FIELD_DATA(HDMI_TX_PHY_RESET_CTL, PLL_RESETB, 1)
				| BCHP_FIELD_DATA(HDMI_TX_PHY_RESET_CTL, PLLDIV_RSTB, 1) ;
		BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_RESET_CTL, Register) ;

		BHDM_MONITOR_P_StatusChanges_isr(hHDMI) ;
	}

	hHDMI->DeviceStatus.tmds.clockEnabled = bEnableTmdsClock ;
	BDBG_LEAVE(BHDM_P_EnableTmdsClock_isr) ;
}


BERR_Code BHDM_P_ConfigurePhy(const BHDM_Handle hHDMI, const BHDM_Settings *NewHdmiSettings)
{
	BERR_Code rc = BERR_SUCCESS ;
	BHDM_PreEmphasis_Configuration stPreEmphasisConfig;

	BDBG_ENTER(BHDM_P_ConfigurePhy) ;

	rc = BHDM_GetPreEmphasisConfiguration(hHDMI, &stPreEmphasisConfig);
	if (rc) BERR_TRACE(rc);

	/* Majority of the cases default to this config */
	stPreEmphasisConfig.uiHfEn = 0;
	stPreEmphasisConfig.uiPreEmphasis_Ch0 = 0xa;
	stPreEmphasisConfig.uiPreEmphasis_Ch1 = 0xa;
	stPreEmphasisConfig.uiPreEmphasis_Ch2 = 0xa;
	stPreEmphasisConfig.uiPreEmphasis_CK = 0xa;

	/*** Additional setup of Tx PHY */
	switch (NewHdmiSettings->eInputVideoFmt)
	{
	case BFMT_VideoFmt_e1080p:
	case BFMT_VideoFmt_e1080p_50Hz:
#ifdef BHDM_CONFIG_BLURAY_3D_SUPPORT
	case BFMT_VideoFmt_e1080p_24Hz_3D :
	case BFMT_VideoFmt_e720p_3D :
	case BFMT_VideoFmt_e720p_50Hz_3D :
#endif

		switch (NewHdmiSettings->stColorDepth.eBitsPerPixel)
		{
		case BAVC_HDMI_BitsPerPixel_e24bit:
			stPreEmphasisConfig.uiHfEn = 3;

#if (BCHP_CHIP == 7231)
			/* 7231 is an exception compare to 40nm in this format */
			stPreEmphasisConfig.uiPreEmphasis_Ch0 = 0x3d;
			stPreEmphasisConfig.uiPreEmphasis_Ch1 = 0x3d;
			stPreEmphasisConfig.uiPreEmphasis_Ch2 = 0x3d;
			stPreEmphasisConfig.uiPreEmphasis_CK = 0x3d;
#else
			stPreEmphasisConfig.uiPreEmphasis_Ch0 = 0x3c;
			stPreEmphasisConfig.uiPreEmphasis_Ch1 = 0x3c;
			stPreEmphasisConfig.uiPreEmphasis_Ch2 = 0x3c;
			stPreEmphasisConfig.uiPreEmphasis_CK = 0x3c;
#endif

			break;

		case BAVC_HDMI_BitsPerPixel_e30bit:
		case BAVC_HDMI_BitsPerPixel_e36bit:
			stPreEmphasisConfig.uiHfEn = 3;
			stPreEmphasisConfig.uiPreEmphasis_Ch0 = 0x7f;
			stPreEmphasisConfig.uiPreEmphasis_Ch1 = 0x7f;
			stPreEmphasisConfig.uiPreEmphasis_Ch2 = 0x7f;
			stPreEmphasisConfig.uiPreEmphasis_CK = 0x7f;
			break;

		default:
			BDBG_ERR(("Unknown color depth")) ;
			rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
			goto done ;
		}
		break;

	case BFMT_VideoFmt_e480p   :
	case BFMT_VideoFmt_e576p_50Hz :
		if ((NewHdmiSettings->ePixelRepetition == BAVC_HDMI_PixelRepetition_e4x)
			&&
			((NewHdmiSettings->stColorDepth.eBitsPerPixel == BAVC_HDMI_BitsPerPixel_e30bit)
			|| (NewHdmiSettings->stColorDepth.eBitsPerPixel == BAVC_HDMI_BitsPerPixel_e36bit)))
		{
			stPreEmphasisConfig.uiHfEn = 3;
			stPreEmphasisConfig.uiPreEmphasis_Ch0 = 0x3c;
			stPreEmphasisConfig.uiPreEmphasis_Ch1 = 0x3c;
			stPreEmphasisConfig.uiPreEmphasis_Ch2 = 0x3c;
			stPreEmphasisConfig.uiPreEmphasis_CK = 0x3c;
		}
		break;

	case BFMT_VideoFmt_e3840x2160p_24Hz :
	case BFMT_VideoFmt_e3840x2160p_25Hz :
	case BFMT_VideoFmt_e3840x2160p_30Hz :
		stPreEmphasisConfig.uiHfEn = 3;
		stPreEmphasisConfig.uiPreEmphasis_Ch0 = 0x7f ;
		stPreEmphasisConfig.uiPreEmphasis_Ch1 = 0x7f ;
		stPreEmphasisConfig.uiPreEmphasis_Ch2 = 0x7f ;
		stPreEmphasisConfig.uiPreEmphasis_CK = 0x7f ;

		break;

	default:
		break;
	}

	/* Set pre-emp configuration */
	BHDM_SetPreEmphasisConfiguration(hHDMI, &stPreEmphasisConfig);

done:

	BDBG_LEAVE(BHDM_P_ConfigurePhy);
	return rc;
}


/**********************************
**				PRIVATE FUNCTIONS
***********************************/
void BHDM_P_PowerOffPhy (const BHDM_Handle hHDMI)
{

#ifdef BCHP_PWR_RESOURCE_HDMI_TX_PHY
	if (hHDMI->phyPowered)
	{
		BDBG_MSG(("Release BCHP_PWR_RESOURCE_HDMI_TX_PHY")) ;
		BCHP_PWR_ReleaseResource(hHDMI->hChip, hHDMI->phyPwrResource[hHDMI->eCoreId]) ;
	}
#else
	BDBG_MSG(("Power Management not enabled")) ;
#endif

	hHDMI->phyPowered = false ;

}


void BHDM_P_PowerOnPhy (const BHDM_Handle hHDMI)
{
	BREG_Handle hRegister = hHDMI->hRegister ;
	uint32_t Register;
	bool masterMode;

	BHDM_GetHdmiDataTransferMode(hHDMI, &masterMode);

#if BHDM_CONFIG_CLOCK_STOP_SUPPORT
	Register = BREG_Read32(hRegister, BCHP_DVP_HT_CLOCK_STOP);
	Register &= ~ BCHP_MASK(DVP_HT_CLOCK_STOP, PIXEL);
	BREG_Write32(hRegister, BCHP_DVP_HT_CLOCK_STOP, Register) ;
#endif

	/* Assert the fields to prevent DRIFT FIFO UNDERFLOW when trying to
		authenticate with DVI receivers */
	Register = BREG_Read32(hRegister, BCHP_HDMI_FIFO_CTL);
	Register &= ~( BCHP_MASK(HDMI_FIFO_CTL, USE_EMPTY)
				| BCHP_MASK(HDMI_FIFO_CTL, USE_FULL));

	Register |= BCHP_FIELD_DATA(HDMI_FIFO_CTL, USE_EMPTY, 1)
				| BCHP_FIELD_DATA(HDMI_FIFO_CTL, USE_FULL, masterMode?0:1);
	BREG_Write32(hRegister, BCHP_HDMI_FIFO_CTL, Register) ;


#ifdef BCHP_PWR_RESOURCE_HDMI_TX_PHY
	if (!hHDMI->phyPowered)
	{
		BDBG_MSG(("Acquire BCHP_PWR_RESOURCE_HDMI_TX_PHY")) ;
		BCHP_PWR_AcquireResource(hHDMI->hChip, hHDMI->phyPwrResource[hHDMI->eCoreId]);
	}
#else
	BDBG_MSG(("Power Management not enabled")) ;
	/* make sure phy is left on */
	Register = BREG_Read32(hRegister, BCHP_HDMI_TX_PHY_POWERDOWN_CTL);
	Register &= ~(
		  BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, RNDGEN_PWRDN)
		| BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, PLL_PWRDN)
		| BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, BIAS_PWRDN)
		| BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, PHY_PWRDN)) ;
	BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_POWERDOWN_CTL, Register) ;
#endif


	/* Bring PLL PHY out of reset */
	Register = BREG_Read32(hRegister, BCHP_HDMI_TX_PHY_RESET_CTL);
	Register &= ~( BCHP_MASK(HDMI_TX_PHY_RESET_CTL, PLL_RESETB)
				| BCHP_MASK(HDMI_TX_PHY_RESET_CTL, PLLDIV_RSTB)) ;

	Register |= BCHP_FIELD_DATA(HDMI_TX_PHY_RESET_CTL, PLL_RESETB, 1)
				| BCHP_FIELD_DATA(HDMI_TX_PHY_RESET_CTL, PLLDIV_RSTB, 1);
	BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_RESET_CTL, Register) ;

#if BHDM_CONFIG_SWAP_DEFAULT_PHY_CHANNELS
	/* Program HDMI_TX_PHY.CHANNEL_SWAP with defaults; required swaps located in bhdm_config.h */
	Register = BREG_Read32(hRegister, BCHP_HDMI_TX_PHY_CHANNEL_SWAP);
		Register &= ~(
			  BCHP_MASK(HDMI_TX_PHY_CHANNEL_SWAP, TXCK_OUT_INV)
			| BCHP_MASK(HDMI_TX_PHY_CHANNEL_SWAP, TXCK_OUT_SEL)
			| BCHP_MASK(HDMI_TX_PHY_CHANNEL_SWAP, TX2_OUT_INV)
			| BCHP_MASK(HDMI_TX_PHY_CHANNEL_SWAP, TX2_OUT_SEL)
			| BCHP_MASK(HDMI_TX_PHY_CHANNEL_SWAP, TX1_OUT_INV)
			| BCHP_MASK(HDMI_TX_PHY_CHANNEL_SWAP, TX1_OUT_SEL)
			| BCHP_MASK(HDMI_TX_PHY_CHANNEL_SWAP, TX0_OUT_INV)
			| BCHP_MASK(HDMI_TX_PHY_CHANNEL_SWAP, TX0_OUT_SEL));

		Register |=
			  BCHP_FIELD_DATA(HDMI_TX_PHY_CHANNEL_SWAP, TXCK_OUT_INV,
				BHDM_CONFIG_HDMI_TX_PHY_CHANNEL_SWAP_TXCK_OUT_INV)
			| BCHP_FIELD_DATA(HDMI_TX_PHY_CHANNEL_SWAP, TXCK_OUT_SEL,
				BHDM_CONFIG_HDMI_TX_PHY_CHANNEL_SWAP_TXCK_OUT_SEL)
			| BCHP_FIELD_DATA(HDMI_TX_PHY_CHANNEL_SWAP, TX2_OUT_INV,
				BHDM_CONFIG_HDMI_TX_PHY_CHANNEL_SWAP_TX2_OUT_INV)
			| BCHP_FIELD_DATA(HDMI_TX_PHY_CHANNEL_SWAP, TX2_OUT_SEL,
				BHDM_CONFIG_HDMI_TX_PHY_CHANNEL_SWAP_TX2_OUT_SEL)
			| BCHP_FIELD_DATA(HDMI_TX_PHY_CHANNEL_SWAP, TX1_OUT_INV,
				BHDM_CONFIG_HDMI_TX_PHY_CHANNEL_SWAP_TX1_OUT_INV)
			| BCHP_FIELD_DATA(HDMI_TX_PHY_CHANNEL_SWAP, TX1_OUT_SEL,
				BHDM_CONFIG_HDMI_TX_PHY_CHANNEL_SWAP_TX1_OUT_SEL)
			| BCHP_FIELD_DATA(HDMI_TX_PHY_CHANNEL_SWAP, TX0_OUT_INV,
				BHDM_CONFIG_HDMI_TX_PHY_CHANNEL_SWAP_TX0_OUT_INV)
			| BCHP_FIELD_DATA(HDMI_TX_PHY_CHANNEL_SWAP, TX0_OUT_SEL,
				BHDM_CONFIG_HDMI_TX_PHY_CHANNEL_SWAP_TX0_OUT_SEL);
	BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_CHANNEL_SWAP, Register) ;
#endif

	hHDMI->phyPowered = true ;
	return;
}


void BHDM_P_SetPreEmphasisMode (const BHDM_Handle hHDMI, uint8_t uValue, uint8_t uDriverAmp)
{
	uint32_t Register;

	Register = BREG_Read32( hHDMI->hRegister, BCHP_HDMI_TX_PHY_CTL_0) ;
	Register &= ~(BCHP_MASK(HDMI_TX_PHY_CTL_0, PREEMP_2)
			  | BCHP_MASK(HDMI_TX_PHY_CTL_0, PREEMP_1)
			  | BCHP_MASK(HDMI_TX_PHY_CTL_0, PREEMP_0));

	Register |= BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_0, PREEMP_2, uValue)
			| BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_0, PREEMP_1, uValue)
			| BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_0, PREEMP_0, uValue) ;
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_CTL_0, Register) ;


	Register = BREG_Read32( hHDMI->hRegister, BCHP_HDMI_TX_PHY_CTL_1) ;
	Register &= ~(BCHP_MASK(HDMI_TX_PHY_CTL_1, PREEMP_CK));
	Register |= BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_1, PREEMP_CK, uValue);
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_CTL_1, Register) ;

	BSTD_UNUSED(uDriverAmp);
	return;
}


BERR_Code BHDM_P_GetPreEmphasisConfiguration (
	const BHDM_Handle hHDMI,
	BHDM_PreEmphasis_Configuration *stPreEmphasisConfig
)
{
	BERR_Code rc = BERR_SUCCESS;
	uint32_t Register;

	Register = BREG_Read32( hHDMI->hRegister, BCHP_HDMI_TX_PHY_CTL_0) ;
	stPreEmphasisConfig->uiHfEn = BCHP_GET_FIELD_DATA(Register, HDMI_TX_PHY_CTL_0, HF_EN);
	stPreEmphasisConfig->uiPreEmphasis_Ch2 = BCHP_GET_FIELD_DATA(Register,HDMI_TX_PHY_CTL_0,PREEMP_2);
	stPreEmphasisConfig->uiPreEmphasis_Ch1 = BCHP_GET_FIELD_DATA(Register,HDMI_TX_PHY_CTL_0,PREEMP_1);
	stPreEmphasisConfig->uiPreEmphasis_Ch0 = BCHP_GET_FIELD_DATA(Register,HDMI_TX_PHY_CTL_0,PREEMP_0);

	Register = BREG_Read32( hHDMI->hRegister, BCHP_HDMI_TX_PHY_CTL_1) ;
	stPreEmphasisConfig->uiPreEmphasis_CK = BCHP_GET_FIELD_DATA(Register,HDMI_TX_PHY_CTL_1,PREEMP_CK);
	stPreEmphasisConfig->uiCurrentRatioSel = BCHP_GET_FIELD_DATA(Register, HDMI_TX_PHY_CTL_1, CURRENT_RATIO_SEL);

	Register = BREG_Read32( hHDMI->hRegister, BCHP_HDMI_TX_PHY_CTL_2) ;
	stPreEmphasisConfig->uiKP = BCHP_GET_FIELD_DATA(Register, HDMI_TX_PHY_CTL_2, KP);
	stPreEmphasisConfig->uiKI = BCHP_GET_FIELD_DATA(Register, HDMI_TX_PHY_CTL_2, KI);
	stPreEmphasisConfig->uiKA = BCHP_GET_FIELD_DATA(Register, HDMI_TX_PHY_CTL_2, KA);

	stPreEmphasisConfig->eCore = BHDM_Core40nm ;

	return rc;
}


BERR_Code BHDM_P_SetPreEmphasisConfiguration(
	const BHDM_Handle hHDMI,
	BHDM_PreEmphasis_Configuration *stPreEmphasisConfig)
{
	BERR_Code rc = BERR_SUCCESS;
	uint32_t Register;

	/* Set Preemphasis configurations */
	Register = BREG_Read32( hHDMI->hRegister, BCHP_HDMI_TX_PHY_CTL_0) ;
	Register &= ~(
		 BCHP_MASK(HDMI_TX_PHY_CTL_0, HF_EN)
		| BCHP_MASK(HDMI_TX_PHY_CTL_0, PREEMP_2)
		| BCHP_MASK(HDMI_TX_PHY_CTL_0, PREEMP_1)
		| BCHP_MASK(HDMI_TX_PHY_CTL_0, PREEMP_0)) ;

	Register |=
		 BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_0, HF_EN, stPreEmphasisConfig->uiHfEn)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_0, PREEMP_2, stPreEmphasisConfig->uiPreEmphasis_Ch2)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_0, PREEMP_1, stPreEmphasisConfig->uiPreEmphasis_Ch1)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_0, PREEMP_0, stPreEmphasisConfig->uiPreEmphasis_Ch0) ;
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_CTL_0, Register) ;

	Register = BREG_Read32( hHDMI->hRegister, BCHP_HDMI_TX_PHY_CTL_1) ;
	Register &= ~ (
		 BCHP_MASK(HDMI_TX_PHY_CTL_1, PREEMP_CK)
		| BCHP_MASK(HDMI_TX_PHY_CTL_1, CURRENT_RATIO_SEL));
	Register |=
		 BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_1, PREEMP_CK, stPreEmphasisConfig->uiPreEmphasis_CK)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_1, CURRENT_RATIO_SEL, stPreEmphasisConfig->uiCurrentRatioSel);
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_CTL_1, Register) ;

	Register = BREG_Read32( hHDMI->hRegister, BCHP_HDMI_TX_PHY_CTL_2) ;
	Register &= ~ (
		 BCHP_MASK(HDMI_TX_PHY_CTL_2, KP)
		| BCHP_MASK(HDMI_TX_PHY_CTL_2, KI)
		| BCHP_MASK(HDMI_TX_PHY_CTL_2, KA));
	Register |=
		 BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_2, KP, stPreEmphasisConfig->uiKP)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_2, KI, stPreEmphasisConfig->uiKI)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_2, KA, stPreEmphasisConfig->uiKA);
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_CTL_2, Register) ;


	return rc;
}


/* For boot loader usage */
#ifndef BHDM_FOR_BOOTUPDATER
void BHDM_P_ClearHotPlugInterrupt(
   const BHDM_Handle hHDMI		/* [in] HDMI handle */
)
{
	uint32_t Register ;

#if BHDM_CONFIG_DUAL_HPD_SUPPORT
	/* reset boolean status */
	hHDMI->hotplugInterruptFired = false;
	BSTD_UNUSED(Register);

#else
	Register = BREG_Read32(hHDMI->hRegister, BCHP_AON_HDMI_TX_HDMI_HOTPLUG_CONTROL) ;
	Register &= ~BCHP_MASK(AON_HDMI_TX_HDMI_HOTPLUG_CONTROL, CLEAR_HOTPLUG_INT_STATUS) ;
	Register |= BCHP_FIELD_DATA(AON_HDMI_TX_HDMI_HOTPLUG_CONTROL, CLEAR_HOTPLUG_INT_STATUS, 1) ;
	BREG_Write32(hHDMI->hRegister, BCHP_AON_HDMI_TX_HDMI_HOTPLUG_CONTROL, Register) ;

	Register &= ~BCHP_MASK(AON_HDMI_TX_HDMI_HOTPLUG_CONTROL, CLEAR_HOTPLUG_INT_STATUS) ;
	Register |= BCHP_FIELD_DATA(AON_HDMI_TX_HDMI_HOTPLUG_CONTROL, CLEAR_HOTPLUG_INT_STATUS, 0) ;
	BREG_Write32(hHDMI->hRegister, BCHP_AON_HDMI_TX_HDMI_HOTPLUG_CONTROL, Register) ;
#endif

	return;
}


void BHDM_P_CheckHotPlugInterrupt(
	const BHDM_Handle hHDMI,		 /* [in] HDMI handle */
	uint8_t *bHotPlugInterrupt	/* [out] Interrupt asserted or not */
)
{
	uint32_t Register ;

#if BHDM_CONFIG_DUAL_HPD_SUPPORT
	*bHotPlugInterrupt = hHDMI->hotplugInterruptFired;
	BSTD_UNUSED(Register);

#else
	Register = BREG_Read32(hHDMI->hRegister, BCHP_AON_HDMI_TX_HDMI_HOTPLUG_STATUS) ;

	if (Register & BCHP_MASK(AON_HDMI_TX_HDMI_HOTPLUG_STATUS, HOTPLUG_INT_STATUS))
		*bHotPlugInterrupt = true;
	else
		*bHotPlugInterrupt = false;
#endif

	return;
}

#endif /* #ifndef BHDM_FOR_BOOTUPDATER */


void BHDM_P_RxDeviceAttached_isr(
	const BHDM_Handle hHDMI,		 /* [in] HDMI handle */
	uint8_t *bDeviceAttached	/* [out] Device Attached Status  */
)
{
	uint32_t Register ;

	BDBG_ENTER(BHDM_P_RxDeviceAttached_isr) ;

	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_HOTPLUG_STATUS) ;
	*bDeviceAttached =
		BCHP_GET_FIELD_DATA(Register, HDMI_HOTPLUG_STATUS, HOTPLUG_STATUS) ;

	BDBG_LEAVE(BHDM_P_RxDeviceAttached_isr) ;
	return ;
}



