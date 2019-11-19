/*******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 ******************************************************************************/
#include "bdsp_types.h"
#include "bdsp_raaga_fw.h"

const BDSP_VOM_Algo_Start_Addr BDSP_sAlgoStartAddr =
{
	{
		/* BDSP_AF_P_AlgoId_eMpegDecode */
		0x10018000,

		/* BDSP_AF_P_AlgoId_eAc3Decode */
		0x10025000,

		/* BDSP_AF_P_AlgoId_eAacDecode */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eAacHeLpSbrDecode */
		0x100c8800,

		/* BDSP_AF_P_AlgoId_eDdpDecode */
		0x10065000,

		/* BDSP_AF_P_AlgoId_eDdLosslessDecode */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eLpcmCustomDecode */
		0x10293800,

		/* BDSP_AF_P_AlgoId_eBdLpcmDecode */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eDvdLpcmDecode */
		0x10293800,

		/* BDSP_AF_P_AlgoId_eHdDvdLpcmDecode */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eMpegMcDecode */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eWmaStdDecode */
		0x10112034,

		/* BDSP_AF_P_AlgoId_eWmaProStdDecode */
		0x1017b978,

		/* BDSP_AF_P_AlgoId_eMlpDecode */
		0x10460800,

		/* BDSP_AF_P_AlgoId_eDdp71Decode */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eDtsDecode */
		0x1014a800,

		/* BDSP_AF_P_AlgoId_eDtsLbrDecode */
		0x10124800,

		/* BDSP_AF_P_AlgoId_eDtsHdDecode */
		0x1014a800,

		/* BDSP_AF_P_AlgoId_ePcmWavDecode */
		0x10176000,

		/* BDSP_AF_P_AlgoId_eAmrDecode */
		0x1032c800,

		/* BDSP_AF_P_AlgoId_eDraDecode */
		0x101d6800,

		/* BDSP_AF_P_AlgoId_eRealAudioLbrDecode */
		0x10299800,

		/* BDSP_AF_P_AlgoId_eDolbyPulseDecode */
		0x101e0800,

		/* BDSP_AF_P_AlgoId_eMs10DdpDecode */
		0x100a5000,

		/* BDSP_AF_P_AlgoId_eAdpcmDecode */
		0x102b5800,

		/* BDSP_AF_P_AlgoId_eG711G726Decode */
		0x10385000,

		/* BDSP_AF_P_AlgoId_eG729Decode */
		0x1038a000,

		/* BDSP_AF_P_AlgoId_eVorbisDecode */
		0x103c1800,

		/* BDSP_AF_P_AlgoId_eG723_1Decode */
		0x103d1800,

		/* BDSP_AF_P_AlgoId_eFlacDecode */
		0x104526e4,

		/* BDSP_AF_P_AlgoId_eMacDecode */
		0x1045a000,

		/* BDSP_AF_P_AlgoId_eAmrWbDecode */
		0x1047a800,

		/* BDSP_AF_P_AlgoId_eiLBCDecode */
		0x104e04d0,

		/* BDSP_AF_P_AlgoId_eiSACDecode */
		0x104fb044,

		/* BDSP_AF_P_AlgoId_eUdcDecode */
		0x10530000,

		/* BDSP_AF_P_AlgoId_eDolbyAacheDecode */
		0x105fe800,

		/* BDSP_AF_P_AlgoId_eOpusDecode */
		0x106b6b60,

		/* BDSP_AF_P_AlgoId_eALSDecode */
		0x106eed70,

		/* BDSP_AF_P_AlgoId_eAC4Decode */
		0x106f3f10,

		/* BDSP_AF_P_AlgoId_eEndOfAudioDecodeAlgos */
		0x00000000,

		/* BDSP_VF_P_AlgoId_eRealVideo9Decode */
		0x00000000,

		/* BDSP_VF_P_AlgoId_eVP6Decode */
		0x102c7800,

		/* BDSP_AF_P_AlgoId_eEndOfDecodeAlgos */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eMpegFrameSync */
		0x10000004,

		/* BDSP_AF_P_AlgoId_eMpegMcFrameSync */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eAdtsFrameSync */
		0x10000004,

		/* BDSP_AF_P_AlgoId_eLoasFrameSync */
		0x10000004,

		/* BDSP_AF_P_AlgoId_eWmaStdFrameSync */
		0x10000004,

		/* BDSP_AF_P_AlgoId_eWmaProFrameSync */
		0x10000004,

		/* BDSP_AF_P_AlgoId_eAc3FrameSync */
		0x10000004,

		/* BDSP_AF_P_AlgoId_eDdpFrameSync */
		0x10000004,

		/* BDSP_AF_P_AlgoId_eDdp71FrameSync */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eDtsFrameSync */
		0x10000004,

		/* BDSP_AF_P_AlgoId_eDtsLbrFrameSync */
		0x10000004,

		/* BDSP_AF_P_AlgoId_eDtsHdFrameSync */
		0x10000004,

		/* BDSP_AF_P_AlgoId_eDtsHdFrameSync_1 */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eDtsHdHdDvdFrameSync */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eDdLosslessFrameSync */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eMlpFrameSync */
		0x10000004,

		/* BDSP_AF_P_AlgoId_eMlpHdDvdFrameSync */
		0x00000000,

		/* BDSP_AF_P_AlgoId_ePesFrameSync */
		0x10000004,

		/* BDSP_AF_P_AlgoId_eBdLpcmFrameSync */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eHdDvdLpcmFrameSync */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eDvdLpcmFrameSync */
		0x10000004,

		/* BDSP_AF_P_AlgoId_eDvdLpcmFrameSync_1 */
		0x00000000,

		/* BDSP_AF_P_AlgoId_ePcmWavFrameSync */
		0x10000004,

		/* BDSP_AF_P_AlgoId_eDraFrameSync */
		0x10000004,

		/* BDSP_AF_P_AlgoId_eRealAudioLbrFrameSync */
		0x10000004,

		/* BDSP_AF_P_AlgoId_eMs10DdpFrameSync */
		0x10000004,

		/* BDSP_AF_P_AlgoId_eVorbisFrameSync */
		0x10000004,

		/* BDSP_AF_P_AlgoId_eFlacFrameSync */
		0x10000004,

		/* BDSP_AF_P_AlgoId_eMacFrameSync */
		0x10000004,

		/* BDSP_AF_P_AlgoId_eUdcFrameSync */
		0x10000004,

		/* BDSP_AF_P_AlgoId_eAC4FrameSync */
		0x10000004,

		/* BDSP_AF_P_AlgoId_eALSFrameSync */
		0x10000004,

		/* BDSP_AF_P_AlgoId_eEndOfAudioDecFsAlgos */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eRealVideo9FrameSync */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eVP6FrameSync */
		0x10000004,

		/* BDSP_AF_P_AlgoId_eEndOfDecFsAlgos */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eAc3Encode */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eMpegL2Encode */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eMpegL3Encode */
		0x10376800,

		/* BDSP_AF_P_AlgoId_eAacLcEncode */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eAacHeEncode */
		0x102e36a8,

		/* BDSP_AF_P_AlgoId_eDtsEncode */
		0x10281000,

		/* BDSP_AF_P_AlgoId_eDtsBroadcastEncode */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eSbcEncode */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eMs10DDTranscode */
		0x10263000,

		/* BDSP_AF_P_AlgoId_eG711G726Encode */
		0x10395800,

		/* BDSP_AF_P_AlgoId_eG729Encode */
		0x1039b000,

		/* BDSP_AF_P_AlgoId_eG723_1Encode */
		0x103dd000,

		/* BDSP_AF_P_AlgoId_eG722Encode */
		0x10476000,

		/* BDSP_AF_P_AlgoId_eAmrEncode */
		0x1048c800,

		/* BDSP_AF_P_AlgoId_eAmrwbEncode */
		0x104ae000,

		/* BDSP_AF_P_AlgoId_eiLBCEncode */
		0x104cb000,

		/* BDSP_AF_P_AlgoId_eiSACEncode */
		0x104ec000,

		/* BDSP_AF_P_AlgoId_eLpcmEncode */
		0x1052c000,

		/* BDSP_AF_P_AlgoId_eOpusEncode */
		0x10570000,

		/* BDSP_AF_P_AlgoId_eDDPEncode */
		0x105ae800,

		/* BDSP_AF_P_AlgoId_eEndOfAudioEncodeAlgos */
		0x00000000,

		/* BDSP_VF_P_AlgoId_eH264Encode */
		0x10401800,

		/* BDSP_VF_P_AlgoId_eX264Encode */
		0x1042a800,

		/* BDSP_VF_P_AlgoId_eXVP8Encode */
		0x10439800,

		/* BDSP_AF_P_AlgoId_eEndOfEncodeAlgos */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eAc3EncFrameSync */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eMpegL3EncFrameSync */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eMpegL2EncFrameSync */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eAacLcEncFrameSync */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eAacHeEncFrameSync */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eDtsEncFrameSync */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eEndOfEncFsAlgos */
		0x00000000,

		/* BDSP_AF_P_AlgoId_ePassThru */
		0x10106524,

		/* BDSP_AF_P_AlgoId_eMlpPassThru */
		0x10478800,

		/* BDSP_AF_P_AlgoId_eEndOfAuxAlgos */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eSrsTruSurroundPostProc */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eSrcPostProc */
		0x1025c000,

		/* BDSP_AF_P_AlgoId_eDdbmPostProc */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eDownmixPostProc */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eCustomSurroundPostProc */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eCustomBassPostProc */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eKaraokeCapablePostProc */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eCustomVoicePostProc */
		0x101cd800,

		/* BDSP_AF_P_AlgoId_ePeqPostProc */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eAvlPostProc */
		0x102b9800,

		/* BDSP_AF_P_AlgoId_ePl2PostProc */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eXenPostProc */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eBbePostProc */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eDsolaPostProc */
		0x10291000,

		/* BDSP_AF_P_AlgoId_eDtsNeoPostProc */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eDDConvert */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eAudioDescriptorFadePostProc */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eAudioDescriptorPanPostProc */
		0x00000000,

		/* BDSP_AF_P_AlgoId_ePCMRouterPostProc */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eWMAPassThrough */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eSrsTruSurroundHDPostProc */
		0x102ac800,

		/* BDSP_AF_P_AlgoId_eSrsTruVolumePostProc */
		0x102a3000,

		/* BDSP_AF_P_AlgoId_eDolbyVolumePostProc */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eBrcm3DSurroundPostProc */
		0x102be800,

		/* BDSP_AF_P_AlgoId_eFWMixerPostProc */
		0x10273000,

		/* BDSP_AF_P_AlgoId_eMonoDownMixPostProc */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eMs10DDConvert */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eDdrePostProc */
		0x1033c000,

		/* BDSP_AF_P_AlgoId_eDv258PostProc */
		0x1034a000,

		/* BDSP_AF_P_AlgoId_eDpcmrPostProc */
		0x1035cab4,

		/* BDSP_AF_P_AlgoId_eGenCdbItbPostProc */
		0x102c2000,

		/* BDSP_AF_P_AlgoId_eBtscEncoderPostProc */
		0x103ba04c,

		/* BDSP_AF_P_AlgoId_eSpeexAECPostProc */
		0x103ec378,

		/* BDSP_AF_P_AlgoId_eKaraokePostProc */
		0x1052d800,

		/* BDSP_AF_P_AlgoId_eMixerDapv2PostProc */
		0x10676000,

		/* BDSP_AF_P_AlgoId_eOutputFormatterPostProc */
		0x106d0800,

		/* BDSP_AF_P_AlgoId_eVocalPostProc */
		0x106d2800,

		/* BDSP_AF_P_AlgoId_eFadeCtrlPostProc */
		0x106d4000,

		/* BDSP_AF_P_AlgoId_eAmbisonicsPostProc */
		0x106d9478,

		/* BDSP_AF_P_AlgoId_eTsmCorrectionPostProc */
		0x10776800,

		/* BDSP_AF_P_AlgoId_eEndOfPpAlgos */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eMixerFrameSync */
		0x10375000,

		/* BDSP_AF_P_AlgoId_eMixerDapv2FrameSync */
		0x1066f000,

		/* BDSP_AF_P_AlgoId_eEndOfPpFsAlgos */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eSysLib */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eAlgoLib */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eIdsCommon */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eVidIdsCommon */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eEndOfLibAlgos */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eScm1 */
		0x10505000,

		/* BDSP_AF_P_AlgoId_eScm2 */
		0x10505800,

		/* BDSP_AF_P_AlgoId_eScm3 */
		0x0000130c,

		/* BDSP_AF_P_AlgoId_eEndOfScmAlgos */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eScmTask */
		0x10526000,

		/* BDSP_AF_P_AlgoId_eVideoDecodeTask */
		0x10528000,

		/* BDSP_AF_P_AlgoId_eVideoEncodeTask */
		0x1052a000,

		/* BDSP_AF_P_AlgoId_eEndOfTaskAlgos */
		0x00000000,

		/* BDSP_AF_P_AlgoId_eEndOfAlgos */
		0x00000000,

	},
};
