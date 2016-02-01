/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*                            DO NOT HAND EDIT!!!                            */
/*                                                                           */
/* Parameters generated on 04/02/2014 at 01:43:36 PM                         */
/*                                                                           */
/*                            DO NOT HAND EDIT!!!                            */
/*                                                                           */
/*    WARNING!!! This file is auto-generated file.  The ordering of the      */
/*               various enums and tables is important to the overall        */
/*               functionality!!                                             */
/*                                                                           */
/*   Did we mention?:        DO NOT HAND EDIT!!!                             */
/*                                                                           */
/*****************************************************************************/

typedef struct BHDM_BAVC_Clock
 {
	uint64_t ulPixelClkRate64BitMask ;
	BAVC_HDMI_BitsPerPixel eBitsPerPixel;
	BAVC_HDMI_PixelRepetition ePixelRepetition;
	BAVC_HDMI_AviInfoFrame_Colorspace eAviInfoFrame_Colorspace;
	BHDM_P_TmdsClock eTmdsClock ;
 } BHDM_BAVC_Clock ;


typedef struct _BHDM_P_AUDIO_CLK_VALUES_
 {
	uint32_t NValue ;
	uint32_t HW_NValue ;
	uint32_t CTS_0 ;
	uint32_t CTS_1 ;
	uint32_t CTS_PERIOD_0;
	uint32_t CTS_PERIOD_1;
 } BHDM_P_AUDIO_CLK_VALUES ;


BERR_Code BHDM_PACKET_ACR_P_LookupTmdsClock_isrsafe(
	BHDM_Handle hHDMI,
	uint64_t ulPixelClkRate64BitMask,
	const BHDM_Video_Settings *videoSettings, BAVC_HDMI_PixelRepetition ePixelRepetition,

	BHDM_P_TmdsClock *eTmdsClock
) ;


BERR_Code BHDM_PACKET_ACR_P_LookupN_CTSValues_isrsafe(
	BHDM_Handle hHDMI,
	BAVC_AudioSamplingRate eAudioSamplingRate, BHDM_P_TmdsClock eTmdsClock,

	BHDM_P_AUDIO_CLK_VALUES *stAcrPacket
) ;


BERR_Code BHDM_PACKET_ACR_P_TableLookup_isrsafe(
	BHDM_Handle hHDMI,
	BAVC_AudioSamplingRate eAudioSamplingRate,
	uint64_t ulPixelClkRate64BitMask, const BHDM_Video_Settings *stVideoSettings, BAVC_HDMI_PixelRepetition ePixelRepetition,

	BHDM_P_TmdsClock *eTmdsClock,
	BHDM_P_AUDIO_CLK_VALUES *stAcrPacket
) ;
