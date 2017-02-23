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
	uint32_t CTS_0_REPEAT ;
	uint32_t CTS_1_REPEAT ;
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
