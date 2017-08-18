/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/
#ifndef BDSP_ALGORITHM_
#define BDSP_ALGORITHM_

/***************************************************************************
 Summary:
	 DSP Processing Algorithms.
***************************************************************************/
typedef enum BDSP_Algorithm
{
	/* Audio Decode/Encode/Passthrough Algorithms */
	BDSP_Algorithm_eMpegAudioDecode,
	BDSP_Algorithm_eMpegAudioPassthrough,
	BDSP_Algorithm_eMpegAudioEncode,
	BDSP_Algorithm_eAacAdtsDecode,
	BDSP_Algorithm_eAacAdtsPassthrough,
	BDSP_Algorithm_eAacLoasDecode,
	BDSP_Algorithm_eAacLoasPassthrough,
	BDSP_Algorithm_eAacEncode,
	BDSP_Algorithm_eDolbyPulseAdtsDecode,
	BDSP_Algorithm_eDolbyPulseLoasDecode,
	BDSP_Algorithm_eAc3Decode,
	BDSP_Algorithm_eAc3Passthrough,
	BDSP_Algorithm_eAc3Encode,
	BDSP_Algorithm_eAc3PlusDecode,
	BDSP_Algorithm_eAc3PlusPassthrough,
	BDSP_Algorithm_eDtsCoreEncode,
	BDSP_Algorithm_eDtsHdDecode,
	BDSP_Algorithm_eDtsHdPassthrough,
	BDSP_Algorithm_eDts14BitDecode,
	BDSP_Algorithm_eDts14BitPassthrough,
	BDSP_Algorithm_eDtsLbrDecode,
	BDSP_Algorithm_eWmaStdDecode,
	BDSP_Algorithm_eWmaProDecode,
	BDSP_Algorithm_eMlpDecode,
	BDSP_Algorithm_eMlpPassthrough,
	BDSP_Algorithm_eAmrNbDecode,
	BDSP_Algorithm_eAmrNbEncode,
	BDSP_Algorithm_eAmrWbDecode,
	BDSP_Algorithm_eAmrWbEncode,
	BDSP_Algorithm_eDraDecode,
	BDSP_Algorithm_eCookDecode,
	BDSP_Algorithm_eVorbisDecode,
	BDSP_Algorithm_eFlacDecode,
	BDSP_Algorithm_eMacDecode,
	BDSP_Algorithm_eG711Decode,
	BDSP_Algorithm_eG711Encode,
	BDSP_Algorithm_eG726Decode,
	BDSP_Algorithm_eG726Encode,
	BDSP_Algorithm_eG729Decode,
	BDSP_Algorithm_eG729Encode,
	BDSP_Algorithm_eG723_1Decode,
	BDSP_Algorithm_eG723_1Encode,
	BDSP_Algorithm_eLpcmDvdDecode,
	BDSP_Algorithm_eLpcm1394Decode,
	BDSP_Algorithm_eLpcmBdDecode,
	BDSP_Algorithm_ePcmWavDecode,
	BDSP_Algorithm_ePcmDecode,
	BDSP_Algorithm_eAdpcmDecode,
	BDSP_Algorithm_eiLBCDecode,
	BDSP_Algorithm_eiSACDecode,
	BDSP_Algorithm_eiLBCEncode,
	BDSP_Algorithm_eiSACEncode,
	BDSP_Algorithm_eLpcmEncode,
	BDSP_Algorithm_eUdcDecode,
	BDSP_Algorithm_eUdcPassthrough,
	BDSP_Algorithm_eDolbyAacheAdtsDecode,
	BDSP_Algorithm_eDolbyAacheLoasDecode,
	BDSP_Algorithm_eOpusDecode,
	BDSP_Algorithm_eALSDecode,
	BDSP_Algorithm_eALSLoasDecode,
	BDSP_Algorithm_eAC4Decode,
	BDSP_Algorithm_eOpusEncode,
	BDSP_Algorithm_eDDPEncode,
	BDSP_Algorithm_eGenericPassthrough,
	BDSP_Algorithm_eMixer, 			 /* FW Mixer */
	BDSP_Algorithm_eMixerDapv2,

	/* Audio Processing Algorithms */
	BDSP_Algorithm_eAudioProcessing_StrtIdx,
	BDSP_Algorithm_eSrc = BDSP_Algorithm_eAudioProcessing_StrtIdx,/* Sample Rate Conversion */
	BDSP_Algorithm_eDsola, 			 /* DSOLA */
	BDSP_Algorithm_eGenCdbItb, 		   /* Generate CdbItb algorithm */
	BDSP_Algorithm_eBrcmAvl,			 /* Brcm Automated Volume Level control. */
	BDSP_Algorithm_eBrcm3DSurround,	 /* Brcm 3D Surround  */
	BDSP_Algorithm_eSrsTruSurroundHd,	 /* TruSurroundHD. */
	BDSP_Algorithm_eSrsTruVolume,		 /* SRS Tru Volume */
	BDSP_Algorithm_eDdre,				 /* DDRE post processing  */
	BDSP_Algorithm_eDv258, 			 /* Dolby Volume */
	BDSP_Algorithm_eDpcmr, 			 /* Dolby PCM Renderer */
	BDSP_Algorithm_eCustomVoice,		 /* Custom Voice Algorithm */
	BDSP_Algorithm_eBtscEncoder,		 /* BTSC Encoder */
	BDSP_Algorithm_eKaraoke,			 /* Karaoke */
	BDSP_Algorithm_eOutputFormatter,	 /* OutputFormatter */
	BDSP_Algorithm_eVocalPP,			 /* Vocal PP */
	BDSP_Algorithm_eFadeCtrl,			 /* Fade-Control */
    BDSP_Algorithm_eAmbisonics,         /* Ambisonics */
	BDSP_Algorithm_eTsmCorrection, 	 /* Tsm-Correction */
	BDSP_Algorithm_eAudioProcessing_EndIdx = BDSP_Algorithm_eTsmCorrection,

	/*Echo Canceller Algorithms*/
	BDSP_Algorithm_eSpeexAec,			 /* Speex acoustic echo canceller */

	/* Video Algorithms */
	BDSP_Algorithm_eVp6Decode,
	BDSP_Algorithm_eH264Encode,
	BDSP_Algorithm_eX264Encode,
	BDSP_Algorithm_eXVP8Encode,

	/* Security Algorithms */
	BDSP_Algorithm_eSecurityA,
	BDSP_Algorithm_eSecurityB,
	BDSP_Algorithm_eSecurityC,

	/* Last Entry */
	BDSP_Algorithm_eMax
} BDSP_Algorithm;
#endif /*BDSP_ALGORITHM_*/
