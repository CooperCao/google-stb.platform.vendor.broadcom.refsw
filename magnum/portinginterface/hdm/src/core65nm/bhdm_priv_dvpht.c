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


#if BHDM_CONFIG_BLURAY_PLATFORMS		/* configure PHY for Bluray Platforms */
BERR_Code BHDM_P_ConfigurePhy(const BHDM_Handle hHDMI, const BHDM_Settings *NewHdmiSettings)
{
	BERR_Code rc = BERR_SUCCESS ;
	uint32_t Register ;

	uint8_t uDrvAmp;
	uint8_t uDrvRefCntl;
	uint8_t uTermR;

	BDBG_ENTER(BHDM_P_ConfigurePhy) ;


	/* Default */
	uDrvAmp = 3;
	uDrvRefCntl = 0;
	uTermR = 0;

	/*** Additional setup of Tx PHY */
	switch (NewHdmiSettings->eInputVideoFmt)
	{
	case BFMT_VideoFmt_eNTSC :
	case BFMT_VideoFmt_eNTSC_J :
	case BFMT_VideoFmt_ePAL_B  :
	case BFMT_VideoFmt_ePAL_B1 :
	case BFMT_VideoFmt_ePAL_D  :
	case BFMT_VideoFmt_ePAL_D1 :
	case BFMT_VideoFmt_ePAL_G  :
	case BFMT_VideoFmt_ePAL_H  :
	case BFMT_VideoFmt_ePAL_I  :
	case BFMT_VideoFmt_ePAL_K  :
	case BFMT_VideoFmt_ePAL_M  :
	case BFMT_VideoFmt_ePAL_N  :
	case BFMT_VideoFmt_ePAL_NC :
	case BFMT_VideoFmt_eSECAM :
		if ((NewHdmiSettings->ePixelRepetition == BAVC_HDMI_PixelRepetition_e2x) &&
			(NewHdmiSettings->stColorDepth.eBitsPerPixel == BAVC_HDMI_BitsPerPixel_e36bit))
		{
			uDrvAmp = 1;
			uDrvRefCntl = 1;
			uTermR = 3;
		}
		break;

	case BFMT_VideoFmt_e480p   :
	case BFMT_VideoFmt_e576p_50Hz :
		if ((NewHdmiSettings->ePixelRepetition == BAVC_HDMI_PixelRepetition_e4x) &&
			(NewHdmiSettings->stColorDepth.eBitsPerPixel == BAVC_HDMI_BitsPerPixel_e36bit))
		{
			uDrvAmp = 1;
			uDrvRefCntl = 1;
			uTermR = 3;
		}
		break;

	case BFMT_VideoFmt_e1080p:
	case BFMT_VideoFmt_e1080p_50Hz:
		switch (NewHdmiSettings->stColorDepth.eBitsPerPixel)
		{
		case BAVC_HDMI_BitsPerPixel_e24bit:
			uDrvAmp = 8;
			uDrvRefCntl = 0;
			uTermR = 0;
			break;

		case BAVC_HDMI_BitsPerPixel_e30bit:
		case BAVC_HDMI_BitsPerPixel_e36bit:
			uDrvAmp = 1;
			uDrvRefCntl = 1;
			uTermR = 3;
			break;

		default:
			BDBG_ERR(("Unknown color depth")) ;
			rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
			goto done ;
		}
		break;
	default:
		break;
	}

	Register = BREG_Read32( hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_0 ) ;
	Register &= ~( BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, DRV_AMP)
				| BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, DRV_REFCNTL)) ;

	Register |= BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, DRV_AMP, uDrvAmp)
			| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, DRV_REFCNTL, uDrvRefCntl) ;
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, Register) ;


	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_2) ;
	Register &= ~( BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, TERMR) );
	Register |= BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, TERMR, uTermR) ;
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, Register) ;

done:

	BDBG_LEAVE(BHDM_P_ConfigurePhy);
	return rc;
}


#else	/* Configure PHY for STB platforms  */
BERR_Code BHDM_P_ConfigurePhy(const BHDM_Handle hHDMI, const BHDM_Settings *NewHdmiSettings)
{
	BERR_Code rc = BERR_SUCCESS ;
	BHDM_PreEmphasis_Configuration stPreEmphasisConfig;

	BDBG_ENTER(BHDM_P_ConfigurePhy) ;


	/* Default settings for most formats*/
	BHDM_CHECK_RC(rc, BHDM_P_GetPreEmphasisConfiguration(hHDMI, &stPreEmphasisConfig));
	stPreEmphasisConfig.uiPreDriverAmp = 0;
	stPreEmphasisConfig.uiDriverAmp = 3;
	stPreEmphasisConfig.uiRefCntl = 0;
	stPreEmphasisConfig.uiPreEmphasis_Ch0 = 0;
	stPreEmphasisConfig.uiPreEmphasis_Ch1 = 0;
	stPreEmphasisConfig.uiPreEmphasis_Ch2 = 0;
	stPreEmphasisConfig.uiPreEmphasis_CK = 0;
	stPreEmphasisConfig.uiTermR = 0;


	/*** Tweak TX PHY settings based on formats */
	switch (NewHdmiSettings->eInputVideoFmt)
	{
	case BFMT_VideoFmt_e480p   :
	case BFMT_VideoFmt_e576p_50Hz :
		if ((NewHdmiSettings->ePixelRepetition == BAVC_HDMI_PixelRepetition_e4x) &&
			(NewHdmiSettings->stColorDepth.eBitsPerPixel == BAVC_HDMI_BitsPerPixel_e36bit))
		{
			stPreEmphasisConfig.uiDriverAmp = 1;
			stPreEmphasisConfig.uiRefCntl = 1;
			stPreEmphasisConfig.uiTermR = 3;
		}
		break;

	case BFMT_VideoFmt_e1080p:
	case BFMT_VideoFmt_e1080p_50Hz:
		stPreEmphasisConfig.uiDriverAmp = 1;
		stPreEmphasisConfig.uiRefCntl = 1;
		stPreEmphasisConfig.uiTermR = 3;

		switch (NewHdmiSettings->stColorDepth.eBitsPerPixel)
		{
		case BAVC_HDMI_BitsPerPixel_e24bit:
			break;

		case BAVC_HDMI_BitsPerPixel_e30bit:
		case BAVC_HDMI_BitsPerPixel_e36bit:
			stPreEmphasisConfig.uiPreEmphasis_Ch0 = 4;
			stPreEmphasisConfig.uiPreEmphasis_Ch1 = 4;
			stPreEmphasisConfig.uiPreEmphasis_Ch2 = 4;
			stPreEmphasisConfig.uiPreEmphasis_CK = 4;
			break;
		default:
			BDBG_ERR(("Unknown color depth")) ;
			rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
			goto done ;
		}
		break;

	default:
		/* default values define above */
		break;
	}

	/* Set configuration */
	BHDM_CHECK_RC(rc, BHDM_P_SetPreEmphasisConfiguration(hHDMI, &stPreEmphasisConfig));

done:

	BDBG_LEAVE(BHDM_P_ConfigurePhy);
	return rc;
}
#endif


/******************************************************************************
void BHDM_P_EnableTmdsData_isr
Summary: Enable (Display) TMDS Output
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
		~( BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, TX_CK_PWRDN)
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

	/* set TMDS lines to requested value on/off */
	Register |=
		  /* Rx may need CLOCK signal to wake up; always leave clock powered */
		  BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, TX_CK_PWRDN, !DeviceAttached && TmdsOutput)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, TX_2_PWRDN, TmdsOutput)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, TX_1_PWRDN, TmdsOutput)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, TX_0_PWRDN, TmdsOutput)  ;

	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, Register) ;


#if BHDM_CONFIG_BLURAY_PLATFORMS
	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_1) ;
	Register &=
		~( BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, ICPX)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, RZ)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, CZ)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, CP)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, RN_2_1)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, RN_0)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, CN)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, LF_ORDER))	;

	Register |=
		  BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, ICPX, 31)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, RZ, 1)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, CZ, 1)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, CP, 1)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, RN_2_1, 0)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, RN_0, 0)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, CN, 0)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, LF_ORDER, 0) ;

	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, Register) ;


	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_2) ;
	Register &=
		~( BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, PTAP_ADJ)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, CTAP_ADJ)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, BIASIN_EN)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, NDIV_MOD)
		 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, KVCO_XS))  ;

	Register |=
		  BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, PTAP_ADJ, 1)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, CTAP_ADJ, 1)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, BIASIN_EN, 1)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, NDIV_MOD, 3)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, KVCO_XS, 7) ;

	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, Register) ;

#endif

	hHDMI->DeviceStatus.tmds.dataEnabled = bEnableTmdsOutput ;
	BDBG_LEAVE(BHDM_P_EnableTmdsData_isr) ;
	return ;
}


/******************************************************************************
void BHDM_P_EnableTmdsClock_isr
Summary: Enable (Display) TMDS Clock
*******************************************************************************/
void BHDM_P_EnableTmdsClock_isr(
   const BHDM_Handle hHDMI,		/* [in] HDMI handle */
   bool bEnableTmdsClock	/* [in] boolean to enable/disable */
)
{
	uint32_t Register ;
	uint32_t TmdsOutput ;

	BDBG_ENTER(BHDM_P_EnableTmdsClock_isr) ;

	if (bEnableTmdsClock)
		TmdsOutput = 0x0 ; /* TMDS ON */
	else
		TmdsOutput = 0x1 ; /* TMDS OFF */

#if BHDM_CONFIG_DEBUG_RSEN
	BDBG_WRN(("Configure TMDS Clock %s", TmdsOutput ? "OFF" : "ON"));
#endif


	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL) ;
		Register &=
			~( BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, reserved0)
			 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, reserved1)) ;

		/* set TMDS lines to power on*/
		Register &=
			~( BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, TX_CK_PWRDN)) ;

		/* take TMDS lines out of reset */
		Register &=
			~( BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, TX_CK_RESET) ) ;

		Register &=
			~( BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, D_RESET)
			 | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, A_RESET)) ;

		/* set TMDS lines to requested value on/off */
		Register |=
			  /* Rx may need CLOCK signal to wake up; always leave clock powered */
			  BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, TX_CK_PWRDN, TmdsOutput) ;

	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, Register) ;

	hHDMI->DeviceStatus.tmds.clockEnabled = bEnableTmdsClock ;
	BDBG_LEAVE(BHDM_P_EnableTmdsClock_isr) ;
	return ;
}


/* For boot loader usage */
#ifndef BHDM_FOR_BOOTUPDATER
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
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

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


	/****		Set Channel Map		*/
	/* set CHANNEL_0_MAP = 0  - reset value */
	/* set CHANNEL_1_MAP = 1  - reset value */
	/* set CHANNEL_2_MAP = 2  - reset value */
	/* set CHANNEL_3_MAP = 3  - reset value */
	/* set CHANNEL_4_MAP = 4  - reset value */
	/* set CHANNEL_5_MAP = 5  - reset value */
	/* set CHANNEL_6_MAP = 6  - reset value */
	/* set CHANNEL_7_MAP = 7  - reset value */
	Register =
		   BCHP_FIELD_DATA(HDMI_MAI_CHANNEL_MAP, CHANNEL_0_MAP, 0)
		| BCHP_FIELD_DATA(HDMI_MAI_CHANNEL_MAP, CHANNEL_1_MAP, 1)
		| BCHP_FIELD_DATA(HDMI_MAI_CHANNEL_MAP, CHANNEL_2_MAP, 4)
		| BCHP_FIELD_DATA(HDMI_MAI_CHANNEL_MAP, CHANNEL_3_MAP, 5)
		| BCHP_FIELD_DATA(HDMI_MAI_CHANNEL_MAP, CHANNEL_4_MAP, 3)
		| BCHP_FIELD_DATA(HDMI_MAI_CHANNEL_MAP, CHANNEL_5_MAP, 2)
		| BCHP_FIELD_DATA(HDMI_MAI_CHANNEL_MAP, CHANNEL_6_MAP, 6)
		| BCHP_FIELD_DATA(HDMI_MAI_CHANNEL_MAP, CHANNEL_7_MAP, 7)
		| BCHP_FIELD_DATA(HDMI_MAI_CHANNEL_MAP, reserved0, 0) ;
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_MAI_CHANNEL_MAP, Register) ;


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

	/* Red */
	Register = BREG_Read32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_10) ;
	Register &=
		~( BCHP_MASK(DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_10, DVO_0_FLAT_FIELD_1_RED)
		 | BCHP_MASK(DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_10, DVO_0_FLAT_FIELD_2_RED) );
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_10, Register);

	Register |= BCHP_FIELD_DATA(DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_10, DVO_0_FLAT_FIELD_1_RED, (uiRed12bits << 4))
			| BCHP_FIELD_DATA(DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_10, DVO_0_FLAT_FIELD_2_RED, (uiRed12bits << 4)) ;
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_10, Register);

	/* Green */
	Register = BREG_Read32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_11) ;
	Register &=
		~( BCHP_MASK(DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_11, DVO_0_FLAT_FIELD_1_GREEN)
		| BCHP_MASK(DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_11, DVO_0_FLAT_FIELD_2_GREEN) );
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_11, Register);

	Register |= BCHP_FIELD_DATA(DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_11, DVO_0_FLAT_FIELD_1_GREEN, (uiGreen12bits << 4))
			| BCHP_FIELD_DATA(DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_11, DVO_0_FLAT_FIELD_2_GREEN, (uiGreen12bits << 4)) ;
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_11, Register);

	/* Blue */
	Register = BREG_Read32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_12) ;
	Register &=
		~( BCHP_MASK(DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_12, DVO_0_FLAT_FIELD_1_BLUE)
		| BCHP_MASK(DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_12, DVO_0_FLAT_FIELD_2_BLUE) );
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_12, Register);

	Register |= BCHP_FIELD_DATA(DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_12, DVO_0_FLAT_FIELD_1_BLUE, (uiBlue12bits << 4))
			| BCHP_FIELD_DATA(DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_12, DVO_0_FLAT_FIELD_2_BLUE, (uiBlue12bits << 4)) ;
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_12, Register);


	/* Setup mode */
	Register = BREG_Read32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_13) ;
	Register &= ~BCHP_MASK(DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_13, DVO_0_GEN_TEST_MODE) ;
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_13, Register) ;

	Register |= BCHP_FIELD_DATA(DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_13, DVO_0_GEN_TEST_MODE, 4);
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_13, Register);


	/* Enable */
	Register = BREG_Read32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_0) ;
	Register &= ~BCHP_MASK(DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_0, DVO_0_TEST_MODE) ;
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_0, Register) ;

	Register |= BCHP_FIELD_DATA(DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_0, DVO_0_TEST_MODE, 3);
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_0, Register);


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

	/* Disable */
	Register = BREG_Read32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_0) ;
	Register &= ~BCHP_MASK(DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_0, DVO_0_TEST_MODE) ;
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_0, Register) ;

	Register |= BCHP_FIELD_DATA(DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_0, DVO_0_TEST_MODE, 0);
	BREG_Write32(hHDMI->hRegister, BCHP_DVP_HT_HDMI_TX_0_TEST_DATA_GEN_CFG_0, Register);


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

	BDBG_ENTER(BHDM_WaitForStableVideo);
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BHDM_ClearHotPlugInterrupt(hHDMI);

	while (waitThusFar < maxWait)
	{
		uint8_t notStable = false;

		Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_FORMAT_DET_UPDATE_STATUS) ;

		if (Register & (BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_HAP)
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
		else
		{
			Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_READ_POINTERS) ;
			if (Register & (BCHP_MASK(HDMI_READ_POINTERS, DRIFT_UNDERFLOW)
						  | BCHP_MASK(HDMI_READ_POINTERS, DRIFT_OVERFLOW)))
			{
				notStable = true;
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
#endif /* #ifndef BHDM_FOR_BOOTUPDATER */


/****************************
**		PRIVATE FUNCTIONS
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
	Register &= ~(BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_CK)
			  | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_2)
			  | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_1)
			  | BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_0));

	Register |= BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_CK, uValue)
			| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_2, uValue)
			| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_1, uValue)
			| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_0, uValue) ;
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, Register) ;

	Register = BREG_Read32( hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_0) ;
	Register &= ~ BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, DRV_AMP);
	Register |=  BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, DRV_AMP, uDriverAmp);
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

		stPreEmphasisConfig->uiPreDriverAmp = BCHP_GET_FIELD_DATA(Register,
			HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREDRV_AMP);
		stPreEmphasisConfig->uiDriverAmp = BCHP_GET_FIELD_DATA(Register,
			HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, DRV_AMP);
		stPreEmphasisConfig->uiRefCntl =		BCHP_GET_FIELD_DATA(Register,
			HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, DRV_REFCNTL);
		stPreEmphasisConfig->uiPreEmphasis_CK = BCHP_GET_FIELD_DATA(Register,
			HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_CK);
		stPreEmphasisConfig->uiPreEmphasis_Ch2 = BCHP_GET_FIELD_DATA(Register,
			HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_2);
		stPreEmphasisConfig->uiPreEmphasis_Ch1 = BCHP_GET_FIELD_DATA(Register,
			HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_1);
		stPreEmphasisConfig->uiPreEmphasis_Ch0 =		BCHP_GET_FIELD_DATA(Register,
			HDMI_TX_PHY_HDMI_TX_PHY_CTL_0,PREEMP_0);

	Register = BREG_Read32( hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_2) ;
		stPreEmphasisConfig->uiTermR = BCHP_GET_FIELD_DATA(Register,
			HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, TERMR);

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
		  BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREDRV_AMP)
		| BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, DRV_AMP)
		| BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, DRV_REFCNTL)
		| BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_CK)
		| BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_2)
		| BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_1)
		| BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_0)) ;

	Register |=
		  BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREDRV_AMP, stPreEmphasisConfig->uiPreDriverAmp)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, DRV_AMP, stPreEmphasisConfig->uiDriverAmp)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, DRV_REFCNTL, stPreEmphasisConfig->uiRefCntl)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_CK, stPreEmphasisConfig->uiPreEmphasis_CK)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_2, stPreEmphasisConfig->uiPreEmphasis_Ch2)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_1, stPreEmphasisConfig->uiPreEmphasis_Ch1)
		| BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, PREEMP_0, stPreEmphasisConfig->uiPreEmphasis_Ch0) ;
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_0, Register) ;

	Register = BREG_Read32( hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_2) ;
	Register &= ~ BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, TERMR);
	Register |= BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, TERMR, stPreEmphasisConfig->uiTermR);
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, Register) ;

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
#endif /* #ifndef BHDM_FOR_BOOTUPDATER */


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

