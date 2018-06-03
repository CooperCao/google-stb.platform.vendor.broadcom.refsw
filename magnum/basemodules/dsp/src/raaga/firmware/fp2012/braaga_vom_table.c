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
 ******************************************************************************/
#include "bdsp_types.h"
#include "bdsp_raaga_fw.h"

const BDSP_VOM_Table BDSP_sVomTable =
{
	{
		/* BDSP_AF_P_AlgoId_eMpegDecode */
		{
			0x00000023,
			0x00000034,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAc3Decode */
		{
			0x0000003d,
			0x00000097,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAacDecode */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAacHeLpSbrDecode */
		{
			0x00000184,
			0x000001d5,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDdpDecode */
		{
			0x000000bd,
			0x00000117,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDdLosslessDecode */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eLpcmCustomDecode */
		{
			0x0000051a,
			0x00000522,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eBdLpcmDecode */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDvdLpcmDecode */
		{
			0x0000051a,
			0x00000522,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eHdDvdLpcmDecode */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eMpegMcDecode */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eWmaStdDecode */
		{
			0x00000217,
			0x0000022e,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eWmaProStdDecode */
		{
			0x000002ea,
			0x00000377,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eMlpDecode */
		{
			0x000008b4,
			0x000008d1,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDdp71Decode */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDtsDecode */
		{
			0x00000288,
			0x000002ae,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDtsLbrDecode */
		{
			0x0000023c,
			0x00000270,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDtsHdDecode */
		{
			0x00000288,
			0x000002ae,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_ePcmWavDecode */
		{
			0x000002df,
			0x000002e4,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAmrDecode */
		{
			0x0000064c,
			0x0000065f,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDraDecode */
		{
			0x000003a0,
			0x000003ac,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eRealAudioLbrDecode */
		{
			0x00000526,
			0x00000533,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDolbyPulseDecode */
		{
			0x000003b4,
			0x00000477,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eMs10DdpDecode */
		{
			0x0000013d,
			0x00000173,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAdpcmDecode */
		{
			0x0000055e,
			0x00000562,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eG711G726Decode */
		{
			0x000006fd,
			0x00000703,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eG729Decode */
		{
			0x00000707,
			0x00000719,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eVorbisDecode */
		{
			0x00000776,
			0x0000078e,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eG723_1Decode */
		{
			0x00000796,
			0x000007a5,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eFlacDecode */
		{
			0x0000088d,
			0x00000899,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eMacDecode */
		{
			0x000008a7,
			0x000008ae,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAmrWbDecode */
		{
			0x000008e8,
			0x000008ff,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eiLBCDecode */
		{
			0x000009b3,
			0x000009c1,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eiSACDecode */
		{
			0x000009e9,
			0x000009f8,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eUdcDecode */
		{
			0x00000a53,
			0x00000aab,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDolbyAacheDecode */
		{
			0x00000bf0,
			0x00000ca9,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eOpusDecode */
		{
			0x00000d5c,
			0x00000d8b,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eALSDecode */
		{
			0x00000dcb,
			0x00000dd8,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAC4Decode */
		{
			0x00000dda,
			0x00000eb1,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eEndOfAudioDecodeAlgos */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_VF_P_AlgoId_eRealVideo9Decode */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_VF_P_AlgoId_eVP6Decode */
		{
			0x00000582,
			0x0000059e,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eEndOfDecodeAlgos */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eMpegFrameSync */
		{
			0x00000037,
			0x0000003b,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eMpegMcFrameSync */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAdtsFrameSync */
		{
			0x000001f7,
			0x000001f8,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eLoasFrameSync */
		{
			0x000001fa,
			0x000001fc,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eWmaStdFrameSync */
		{
			0x00000213,
			0x00000215,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eWmaProFrameSync */
		{
			0x0000038a,
			0x0000038c,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAc3FrameSync */
		{
			0x0000017e,
			0x00000182,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDdpFrameSync */
		{
			0x0000017e,
			0x00000182,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDdp71FrameSync */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDtsFrameSync */
		{
			0x0000049a,
			0x0000049c,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDtsLbrFrameSync */
		{
			0x000002d3,
			0x000002d6,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDtsHdFrameSync */
		{
			0x0000049e,
			0x000004a3,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDtsHdFrameSync_1 */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDtsHdHdDvdFrameSync */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDdLosslessFrameSync */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eMlpFrameSync */
		{
			0x000008b0,
			0x000008b2,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eMlpHdDvdFrameSync */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_ePesFrameSync */
		{
			0x00000210,
			0x00000211,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eBdLpcmFrameSync */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eHdDvdLpcmFrameSync */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDvdLpcmFrameSync */
		{
			0x0000050d,
			0x0000050f,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDvdLpcmFrameSync_1 */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_ePcmWavFrameSync */
		{
			0x000006fa,
			0x000006fc,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDraFrameSync */
		{
			0x0000039d,
			0x0000039e,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eRealAudioLbrFrameSync */
		{
			0x00000511,
			0x00000512,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eMs10DdpFrameSync */
		{
			0x0000017e,
			0x00000182,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eVorbisFrameSync */
		{
			0x00000772,
			0x00000774,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eFlacFrameSync */
		{
			0x00000889,
			0x0000088a,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eMacFrameSync */
		{
			0x000008a3,
			0x000008a4,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eUdcFrameSync */
		{
			0x0000017e,
			0x00000182,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAC4FrameSync */
		{
			0x00000ed4,
			0x00000ed6,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eALSFrameSync */
		{
			0x00000ed7,
			0x00000edb,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eEndOfAudioDecFsAlgos */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eRealVideo9FrameSync */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eVP6FrameSync */
		{
			0x0000057f,
			0x00000580,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eEndOfDecFsAlgos */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAc3Encode */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eMpegL2Encode */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eMpegL3Encode */
		{
			0x000006e0,
			0x000006f2,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAacLcEncode */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAacHeEncode */
		{
			0x000005a9,
			0x00000625,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDtsEncode */
		{
			0x000004f5,
			0x00000504,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDtsBroadcastEncode */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eSbcEncode */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eMs10DDTranscode */
		{
			0x000004b9,
			0x000004cb,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eG711G726Encode */
		{
			0x0000071e,
			0x00000723,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eG729Encode */
		{
			0x00000729,
			0x00000753,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eG723_1Encode */
		{
			0x000007ad,
			0x000007c0,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eG722Encode */
		{
			0x000008df,
			0x000008e1,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAmrEncode */
		{
			0x0000090c,
			0x0000092f,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAmrwbEncode */
		{
			0x0000094f,
			0x00000975,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eiLBCEncode */
		{
			0x00000989,
			0x00000997,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eiSACEncode */
		{
			0x000009cb,
			0x000009e4,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eLpcmEncode */
		{
			0x00000a4b,
			0x00000a4c,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eOpusEncode */
		{
			0x00000ad3,
			0x00000b29,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDDPEncode */
		{
			0x00000b50,
			0x00000bd6,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eEndOfAudioEncodeAlgos */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_VF_P_AlgoId_eH264Encode */
		{
			0x000007f6,
			0x00000834,
			0x00000000,
		},
		/* BDSP_VF_P_AlgoId_eX264Encode */
		{
			0x00000848,
			0x0000085f,
			0x00000000,
		},
		/* BDSP_VF_P_AlgoId_eXVP8Encode */
		{
			0x00000866,
			0x00000884,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eEndOfEncodeAlgos */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAc3EncFrameSync */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eMpegL3EncFrameSync */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eMpegL2EncFrameSync */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAacLcEncFrameSync */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAacHeEncFrameSync */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDtsEncFrameSync */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eEndOfEncFsAlgos */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_ePassThru */
		{
			0x000001fe,
			0x0000020c,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eMlpPassThru */
		{
			0x000008e4,
			0x000008e7,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eEndOfAuxAlgos */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eSrsTruSurroundPostProc */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eSrcPostProc */
		{
			0x000004ab,
			0x000004b2,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDdbmPostProc */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDownmixPostProc */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eCustomSurroundPostProc */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eCustomBassPostProc */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eKaraokeCapablePostProc */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eCustomVoicePostProc */
		{
			0x0000038e,
			0x00000397,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_ePeqPostProc */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAvlPostProc */
		{
			0x00000566,
			0x0000056a,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_ePl2PostProc */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eXenPostProc */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eBbePostProc */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDsolaPostProc */
		{
			0x00000515,
			0x00000517,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDtsNeoPostProc */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDDConvert */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAudioDescriptorFadePostProc */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAudioDescriptorPanPostProc */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_ePCMRouterPostProc */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eWMAPassThrough */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eSrsTruSurroundHDPostProc */
		{
			0x0000054c,
			0x00000557,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eSrsTruVolumePostProc */
		{
			0x00000539,
			0x00000543,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDolbyVolumePostProc */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eBrcm3DSurroundPostProc */
		{
			0x00000570,
			0x00000573,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eFWMixerPostProc */
		{
			0x000004d9,
			0x000004e8,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eMonoDownMixPostProc */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eMs10DDConvert */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDdrePostProc */
		{
			0x0000066b,
			0x0000067b,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDv258PostProc */
		{
			0x00000687,
			0x0000069e,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDpcmrPostProc */
		{
			0x000006ab,
			0x000006c2,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eGenCdbItbPostProc */
		{
			0x00000577,
			0x0000057b,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eBtscEncoderPostProc */
		{
			0x00000767,
			0x0000076e,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eSpeexAECPostProc */
		{
			0x000007cb,
			0x000007e5,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eKaraokePostProc */
		{
			0x00000a4e,
			0x00000a51,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eMixerDapv2PostProc */
		{
			0x00000cdf,
			0x00000d3f,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eOutputFormatterPostProc */
		{
			0x00000d94,
			0x00000d94,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eVocalPostProc */
		{
			0x00000d98,
			0x00000d9a,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eFadeCtrlPostProc */
		{
			0x00000d9b,
			0x00000d9c,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAmbisonicsPostProc */
		{
			0x00000da3,
			0x00000da9,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eTsmCorrectionPostProc */
		{
			0x00000ee0,
			0x00000ee4,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eEndOfPpAlgos */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eMixerFrameSync */
		{
			0x000006dd,
			0x000006de,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eMixerDapv2FrameSync */
		{
			0x00000cd1,
			0x00000cd5,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eEndOfPpFsAlgos */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eSysLib */
		{
			0x00000013,
			0x0000001a,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAlgoLib */
		{
			0x0000001c,
			0x00000022,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eIdsCommon */
		{
			0x00000000,
			0x00000011,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eVidIdsCommon */
		{
			0x000007f2,
			0x000007f5,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eEndOfLibAlgos */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eScm1 */
		{
			0x000009fd,
			0x000009fd,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eScm2 */
		{
			0x000009fe,
			0x000009fe,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eScm3 */
		{
			0x000009ff,
			0x00000a36,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eEndOfScmAlgos */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eScmTask */
		{
			0x00000a3f,
			0x00000a40,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eVideoDecodeTask */
		{
			0x00000a43,
			0x00000a45,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eVideoEncodeTask */
		{
			0x00000a47,
			0x00000a49,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eEndOfTaskAlgos */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eEndOfAlgos */
		{
			0x00000000,
			0x00000000,
			0x00000000,
		},
	},
};
