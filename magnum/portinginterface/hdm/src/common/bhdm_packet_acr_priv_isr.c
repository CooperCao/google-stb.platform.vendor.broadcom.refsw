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
#include "bhdm_priv.h"


BDBG_MODULE(BHDM_PACKET_ACR_PRIV_ISR) ;

/* Compare value a == (b +/- delta) */
#define BHDM_P_CMP_AB_DELTA(value_a, value_b, delta) \
	(((value_a) <= ((value_b) + (delta))) && \
	 ((value_b) <= ((value_a) + (delta))))

#if BHDM_HAS_HDMI_20_SUPPORT
static BHDM_P_TmdsMode BHDM_P_GetTmdsClockMode_isr(BHDM_P_TmdsClock eTmdsClock)
{
	BHDM_P_TmdsMode eTmdsMode ;
	switch (eTmdsClock)
	{
	case BHDM_P_TmdsClock_e371_25 :
	case BHDM_P_TmdsClock_e371_25_DIV_1_001 :
	case BHDM_P_TmdsClock_e445_5 :
	case BHDM_P_TmdsClock_e445_5_DIV_1_001 :
	case BHDM_P_TmdsClock_e594 :
	case BHDM_P_TmdsClock_e594_DIV_1_001 :
		eTmdsMode = BHDM_P_TmdsMode_eHigh ;
		break ;

	default :
		eTmdsMode = BHDM_P_TmdsMode_eLow ;
	}

	BDBG_MSG(("eTmdsClock (%d) = %s  TmdsMode: %s",
		eTmdsClock, BHDM_P_TmdsClockToText_isrsafe(eTmdsClock),
		(eTmdsMode == BHDM_P_TmdsMode_eHigh) ? "High" : "Low" )) ;
	return eTmdsMode ;
}
#endif

/***************************************************************************
BHDM_AudioVideoRateChangeCB_isr
Summary: Configure the Rate Manager to match the Rate Manager for the
Video Display System.
****************************************************************************/

void BHDM_AudioVideoRateChangeCB_isr(
	const BHDM_Handle hHDMI,
	int   CallbackType,
	void *pvAudioOrVideoData)
{
	BERR_Code rc ;
	BREG_Handle hRegister ;
	uint32_t	Register, ulOffset ;
	bool masterMode;

	BHDM_P_AUDIO_CLK_VALUES stAcrPacket ;
	BHDM_P_TmdsClock eTmdsClock ;

	BAVC_VdcDisplay_Info *pVdcRateInfo ; /* VIDEO callback */
	uint64_t ulPixelClkRate64BitMask;             /* see defines in bfmt.h */
	uint32_t ulPixelClockRate;           /* see defines in bfmt.h */
	uint32_t ulVertRefreshRate;          /* see defines in bfmt.h */
	BAVC_HDMI_BitsPerPixel eBitsPerPixel;
	BAVC_HDMI_PixelRepetition ePixelRepetition;
	BAVC_Colorspace eColorSpace ;
										/* OR */

	BAVC_Audio_Info *pAudioData ;		/* AUDIO callback */
	BAVC_AudioSamplingRate eInputAudioSamplingRate ;
	bool videoRateChange = false;
	uint8_t deviceAttached ;

	BDBG_ENTER(BHDM_AudioVideoRateChangeCB_isr) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	BHDM_P_RxDeviceAttached_isr(hHDMI, &deviceAttached) ;
	if (!deviceAttached)
	{
		BDBG_MSG(("No Rx device attached; HDMI ACR Packet not updated")) ;
		return ;
	}

	if ((CallbackType != BHDM_Callback_Type_eVideoChange)
	&&  (CallbackType != BHDM_Callback_Type_eAudioChange)
	&&  (CallbackType != BHDM_Callback_Type_eManualAudioChange))
	{
		BDBG_ERR(("Tx%d: Error in Callback Type %d; Use BHDM_Callback_Type_eXXX as int argument",
			hHDMI->eCoreId, CallbackType)) ;
		goto done ;
	}

	if (CallbackType == BHDM_Callback_Type_eVideoChange)
	{
		pVdcRateInfo = (BAVC_VdcDisplay_Info *) pvAudioOrVideoData ;
			/* 64 bit mask indicates 1 or more supported Pixel Clock frequencies */
			ulPixelClkRate64BitMask    = pVdcRateInfo->ulPixelClkRate ;
			ulPixelClockRate  = pVdcRateInfo->ulPixelClockRate ;
			ulVertRefreshRate = pVdcRateInfo->ulVertRefreshRate ;

		/* update the video settings only if VDC callback info has been updated */
		if ((hHDMI->ulPixelClkRate64BitMask == ulPixelClkRate64BitMask)
		&& (hHDMI->ulPixelClockRate == ulPixelClockRate)
		&& (hHDMI->ulVertRefreshRate == ulVertRefreshRate))
		{
			BDBG_MSG(("No change in video settings...")) ;
			/* Remove pending debug of No Audio SWANDROID-3614 */
			/* goto done ; */
		}

		/* modify the video settings if there is a change in the refresh or pixel clock rate.
		 * Ignore "frequency shift" (e.g. differences for 59.94 vs 60.00hz clock rates) */
		videoRateChange =
			!BHDM_P_CMP_AB_DELTA(hHDMI->ulPixelClockRate, pVdcRateInfo->ulPixelClockRate, BFMT_FREQ_FACTOR);

		hHDMI->ulPixelClkRate64BitMask = ulPixelClkRate64BitMask ;
		hHDMI->ulPixelClockRate = ulPixelClockRate ;
		hHDMI->ulVertRefreshRate = ulVertRefreshRate ;

		eBitsPerPixel = hHDMI->DeviceSettings.stVideoSettings.eBitsPerPixel ;
		eColorSpace = hHDMI->DeviceSettings.stVideoSettings.eColorSpace ;
		ePixelRepetition = hHDMI->DeviceSettings.ePixelRepetition ;

		BDBG_MSG(("Tx%d: Video Rate Change Callback <Frequency %s> ",
			hHDMI->eCoreId, videoRateChange ? "Change" : "Shift")) ;

		eInputAudioSamplingRate = hHDMI->DeviceSettings.eAudioSamplingRate ;

		if (videoRateChange)
		{
			BDBG_MSG(("************************************")) ;
			BDBG_MSG(("**** Video Rate Manager Updated ****")) ;
			BDBG_MSG(("************************************")) ;
			BDBG_MSG((" ")) ;
			BDBG_MSG(("------------ Tx%d Video Change Callback Parameters ------------", hHDMI->eCoreId)) ;
			BDBG_MSG(("PxlClkRateMask " BDBG_UINT64_FMT " ", BDBG_UINT64_ARG(ulPixelClkRate64BitMask) ));
			BDBG_MSG(("Color Space : %d; Color Depth %d", eColorSpace, eBitsPerPixel)) ;
			BDBG_MSG(("Pixel Repetition %d", ePixelRepetition)) ;
			BDBG_MSG(("------------------------------------------------------------")) ;
		}
	}
	else  /* Audio Callback Only */
	{
#if BDBG_DEBUG_BUILD
		BDBG_MSG(("********************************************")) ;

		if	(CallbackType == BHDM_Callback_Type_eManualAudioChange) {
			BDBG_MSG(("**** Manual Sample Rate Change Callback ****")) ;
		}
		else {
			BDBG_MSG(("**** Audio Sample Rate Change Callback ****")) ;
		}

		BDBG_MSG(("********************************************")) ;
#endif

		/* get the new Audio Sampling Rate to use... */
		pAudioData = (BAVC_Audio_Info *) pvAudioOrVideoData ;
		eInputAudioSamplingRate = pAudioData->eAudioSamplingRate ;

		/* return if there is no change in Audio Sample Rate */
		if (eInputAudioSamplingRate == hHDMI->DeviceSettings.eAudioSamplingRate)
		{
			BDBG_MSG(("No Change from Audio Rate %d", eInputAudioSamplingRate)) ;
			goto done ;
		}

		BDBG_WRN(("Audio Sample Rate Change from %d to %d",
			hHDMI->DeviceSettings.eAudioSamplingRate,
			eInputAudioSamplingRate)) ;

		hHDMI->DeviceSettings.eAudioSamplingRate = eInputAudioSamplingRate ;

		/* get previous stored video settings */
		ulPixelClkRate64BitMask  = hHDMI->ulPixelClkRate64BitMask ;
		ulPixelClockRate = hHDMI->ulPixelClockRate ;
		ulVertRefreshRate = hHDMI->ulVertRefreshRate ;
		eBitsPerPixel = hHDMI->DeviceSettings.stVideoSettings.eBitsPerPixel ;
		eColorSpace = hHDMI->DeviceSettings.stVideoSettings.eColorSpace ;
		ePixelRepetition = hHDMI->DeviceSettings.ePixelRepetition ;

#if 0
		/* use current Pixel Clock Rate for adjusting the Audio parameters */
		eTmdsClock = hHDMI->eTmdsClock ;
#endif
	}
	BSTD_UNUSED(eColorSpace);
	BSTD_UNUSED(eBitsPerPixel);


	/* during initialization Pixel Clock can be unknown;  if so ACR packet cannot be set */
	if (ulPixelClkRate64BitMask == 0)
	{
		BDBG_MSG(("Unknown Pixel Clock Rate at this time; unable to set ACR Packet")) ;
		goto done ;
	}

	/* Look up N/CTS values based on the audio and video rates */
	rc = BHDM_PACKET_ACR_P_TableLookup_isrsafe(hHDMI,
		eInputAudioSamplingRate, /* audio */
		ulPixelClkRate64BitMask, &hHDMI->DeviceSettings.stVideoSettings, ePixelRepetition, /* video */

		&eTmdsClock,
		&stAcrPacket /* packet parameters*/) ;

	if ((rc) || (eTmdsClock == BHDM_P_TmdsClock_eMax))
	{
		BDBG_WRN(("Unable to look up ACR Settings; ACR Packet not updated")) ;
		BDBG_WRN(("Possible loss of audio!!!")) ;
		goto done ;
	}

#if BHDM_HAS_HDMI_20_SUPPORT
	{
		BHDM_P_TmdsMode newTmdsMode ;

		/******************************************************/
		/* check for a TMDS Clock Ratio Change (Low <--> High) */
		/******************************************************/
		newTmdsMode = BHDM_P_GetTmdsClockMode_isr(eTmdsClock) ;
		if  (newTmdsMode != hHDMI->eTmdsMode)
		{
			hHDMI->TmdsBitClockRatioChange = true ;
			hHDMI->eTmdsMode = newTmdsMode ;
			videoRateChange = true ;
			BDBG_MSG(("TMDS Clock Ratio Change detected...")) ;
		}
	}
#endif


	/******************************************************/
	/* check for / save TMDS Clock Changes  */
	/******************************************************/
	if (hHDMI->eTmdsClock != eTmdsClock)
	{
		hHDMI->eTmdsClock = eTmdsClock ;
	}


	/* Set AV Rate Change Event to notify HDMI core of video rate changes */
	/* changes for audio (ACR Packet) are done below */
	if (videoRateChange)
	{
		/* set event to notify HDMI core of video ratechanges */
		BDBG_MSG(("Video Rate Change Event set...")) ;
		BKNI_SetEvent_isr(hHDMI->BHDM_EventAvRateChange) ;
	}


	/*
	  * If HW (External) CTS,
	  *    then we don't have to program any of the CTS registers.
	  * If SW CTS,
	  *    we get all the values from our audio parameters table.
	  */

	/* HW (External) CTS */
	if (hHDMI->DeviceSettings.CalculateCts)
	{
		Register =
			  BCHP_FIELD_DATA(HDMI_CRP_CFG, USE_MAI_BUS_SYNC_FOR_CTS_GENERATION, 0)
			| BCHP_FIELD_DATA(HDMI_CRP_CFG, CRP_DISABLE, 0)
			| BCHP_FIELD_DATA(HDMI_CRP_CFG, EXTERNAL_CTS_EN, 1)
			| BCHP_FIELD_DATA(HDMI_CRP_CFG, N_VALUE, stAcrPacket.HW_NValue);
		BREG_Write32(hRegister, BCHP_HDMI_CRP_CFG + ulOffset, Register) ;

		BREG_Write32(hRegister, BCHP_HDMI_CTS_0 + ulOffset, 0) ;
		BREG_Write32(hRegister, BCHP_HDMI_CTS_1 + ulOffset, 0) ;
		BREG_Write32(hRegister, BCHP_HDMI_CTS_PERIOD_0 + ulOffset, 0) ;
		BREG_Write32(hRegister, BCHP_HDMI_CTS_PERIOD_1 + ulOffset, 0) ;
	}
	else	/* SW CTS */
	{
		Register =
			  BCHP_FIELD_DATA(HDMI_CRP_CFG, USE_MAI_BUS_SYNC_FOR_CTS_GENERATION, 0)
			| BCHP_FIELD_DATA(HDMI_CRP_CFG, CRP_DISABLE, 0)
			| BCHP_FIELD_DATA(HDMI_CRP_CFG, EXTERNAL_CTS_EN, 0)
			| BCHP_FIELD_DATA(HDMI_CRP_CFG, N_VALUE, stAcrPacket.NValue);
		BREG_Write32(hRegister, BCHP_HDMI_CRP_CFG + ulOffset, Register) ;


		/*********/
		/* CTS 0 */
		/*********/
		Register = BCHP_FIELD_DATA(HDMI_CTS_0, CTS_0, stAcrPacket.CTS_0) ;
		BREG_Write32(hRegister, BCHP_HDMI_CTS_0 + ulOffset, Register) ;


		/*********/
		/* CTS 1 */
		/*********/
		Register = BCHP_FIELD_DATA(HDMI_CTS_1, CTS_1, stAcrPacket.CTS_1) ;
		BREG_Write32(hRegister, BCHP_HDMI_CTS_1 + ulOffset, Register) ;


		/***************/
		/* CTS PERIOD 0 */
		/***************/
		Register =
			  BCHP_FIELD_DATA(HDMI_CTS_PERIOD_0, CTS_0_REPEAT, stAcrPacket.CTS_0)
			| BCHP_FIELD_DATA(HDMI_CTS_PERIOD_0, CTS_PERIOD_0, stAcrPacket.CTS_0) ;
		BREG_Write32(hRegister, BCHP_HDMI_CTS_PERIOD_0 + ulOffset, Register) ;

		/***************/
		/* CTS PERIOD 1 */
		/***************/
		Register =
			  BCHP_FIELD_DATA(HDMI_CTS_PERIOD_1, CTS_1_REPEAT, stAcrPacket.CTS_1)
			| BCHP_FIELD_DATA(HDMI_CTS_PERIOD_1, CTS_PERIOD_1, stAcrPacket.CTS_1) ;
		BREG_Write32(hRegister, BCHP_HDMI_CTS_PERIOD_1 + ulOffset, Register) ;
	}

	/* Additional settings for FIFO_CTL register only if HDMI is not configured in Master Mode */
	Register = BREG_Read32( hRegister, BCHP_HDMI_FIFO_CTL + ulOffset) ;
	masterMode =
		BCHP_GET_FIELD_DATA(Register, HDMI_FIFO_CTL, MASTER_OR_SLAVE_N) !=0;

	/* In master mode, No RECENTER and USE_FULL needs to be set to 0 */
	if (masterMode)
	{
		Register = BREG_Read32( hRegister, BCHP_HDMI_FIFO_CTL + ulOffset) ;
			Register &= ~ BCHP_MASK(HDMI_FIFO_CTL, USE_FULL);
			Register |= BCHP_FIELD_DATA(HDMI_FIFO_CTL, USE_FULL, 0);
		BREG_Write32(hRegister, BCHP_HDMI_FIFO_CTL + ulOffset, Register);
		goto done ;
	}


	/* Recenter FIFO for video format changes only */
	if ((CallbackType == BHDM_Callback_Type_eVideoChange)
	&& (videoRateChange))
	{
		BDBG_MSG((">>>> RECENTER FIFO <<<<")) ;
		/* Set to 1 */
		Register = BREG_Read32( hRegister, BCHP_HDMI_FIFO_CTL + ulOffset) ;
			Register &= ~ BCHP_MASK(HDMI_FIFO_CTL, RECENTER) ;
			Register &= ~ BCHP_MASK(HDMI_FIFO_CTL, CAPTURE_POINTERS) ;

			Register |=
				  BCHP_FIELD_DATA(HDMI_FIFO_CTL, RECENTER, 1)
				| BCHP_FIELD_DATA(HDMI_FIFO_CTL, CAPTURE_POINTERS, 1);
		BREG_Write32(hRegister, BCHP_HDMI_FIFO_CTL + ulOffset, Register);

		/* Now set to 0 */
			Register &= ~ BCHP_MASK(HDMI_FIFO_CTL, RECENTER) ;
			Register &= ~ BCHP_MASK(HDMI_FIFO_CTL, CAPTURE_POINTERS) ;
		BREG_Write32(hRegister, BCHP_HDMI_FIFO_CTL + ulOffset, Register);


		/* Set additional fields */
			Register &= ~ BCHP_MASK(HDMI_FIFO_CTL, USE_FULL) ;
			Register &= ~ BCHP_MASK(HDMI_FIFO_CTL, USE_EMPTY) ;

			Register |=
			  BCHP_FIELD_DATA(HDMI_FIFO_CTL, USE_FULL, 1)
			| BCHP_FIELD_DATA(HDMI_FIFO_CTL, USE_EMPTY, 1);
		BREG_Write32(hRegister, BCHP_HDMI_FIFO_CTL + ulOffset, Register);
	}

done:
	BDBG_LEAVE(BHDM_AudioVideoRateChangeCB_isr) ;
	return ;
}
