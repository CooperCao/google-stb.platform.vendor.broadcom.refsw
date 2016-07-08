/*******************************************************************************
 * Broadcom Proprietary and Confidential. (c) 2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
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
			0x00000095,
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
			0x000001df,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDdpDecode */
		{
			0x000000bd,
			0x00000115,
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
			0x000004cd,
			0x000004d5,
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
			0x000004cd,
			0x000004d5,
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
			0x00000203,
			0x00000222,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eWmaProStdDecode */
		{
			0x000002d6,
			0x00000360,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eMlpDecode */
		{
			0x000008b2,
			0x000008d8,
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
			0x00000274,
			0x0000029b,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDtsLbrDecode */
		{
			0x00000228,
			0x0000025d,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDtsHdDecode */
		{
			0x00000274,
			0x0000029b,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_ePcmWavDecode */
		{
			0x000002cb,
			0x000002d1,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAmrDecode */
		{
			0x000005ff,
			0x0000061a,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDraDecode */
		{
			0x0000038c,
			0x0000039d,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eRealAudioLbrDecode */
		{
			0x000004d9,
			0x000004e6,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDolbyPulseDecode */
		{
			0x000003a0,
			0x00000412,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eMs10DdpDecode */
		{
			0x0000013d,
			0x00000179,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAdpcmDecode */
		{
			0x00000511,
			0x00000515,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eG711G726Decode */
		{
			0x000006fb,
			0x00000701,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eG729Decode */
		{
			0x00000705,
			0x00000717,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eVorbisDecode */
		{
			0x00000774,
			0x0000078c,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eG723_1Decode */
		{
			0x00000794,
			0x000007a7,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eFlacDecode */
		{
			0x0000088b,
			0x0000089b,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eMacDecode */
		{
			0x000008a5,
			0x000008ac,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAmrWbDecode */
		{
			0x000008e6,
			0x00000907,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eiLBCDecode */
		{
			0x000009b1,
			0x000009c4,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eiSACDecode */
		{
			0x000009e7,
			0x000009f7,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eUdcDecode */
		{
			0x00000a51,
			0x00000aa7,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDolbyAacheDecode */
		{
			0x00000be9,
			0x00000c51,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eOpusDecode */
		{
			0x00000d50,
			0x00000d7e,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eALSDecode */
		{
			0x00000d92,
			0x00000d9e,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAC4Decode */
		{
			0x00000da1,
			0x00000e66,
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
			0x00000535,
			0x00000559,
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
			0x000001e3,
			0x000001e4,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eLoasFrameSync */
		{
			0x000001e6,
			0x000001e7,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eWmaStdFrameSync */
		{
			0x000001ff,
			0x00000201,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eWmaProFrameSync */
		{
			0x00000376,
			0x00000378,
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
			0x0000044d,
			0x0000044f,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDtsLbrFrameSync */
		{
			0x000002bf,
			0x000002c2,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDtsHdFrameSync */
		{
			0x00000451,
			0x00000456,
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
			0x000008ae,
			0x000008b0,
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
			0x000001fc,
			0x000001fd,
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
			0x000004c0,
			0x000004c2,
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
			0x000006f8,
			0x000006fa,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAmrFrameSync */
		{
			0x000006f8,
			0x000006fa,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDraFrameSync */
		{
			0x00000389,
			0x0000038a,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eRealAudioLbrFrameSync */
		{
			0x000004c4,
			0x000004c6,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eMs10DdpFrameSync */
		{
			0x0000017e,
			0x00000182,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAdpcmFrameSync */
		{
			0x000006f8,
			0x000006fa,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eG711G726FrameSync */
		{
			0x000006f8,
			0x000006fa,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eG729FrameSync */
		{
			0x000006f8,
			0x000006fa,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eVorbisFrameSync */
		{
			0x00000770,
			0x00000772,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eG723_1FrameSync */
		{
			0x000006f8,
			0x000006fa,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eFlacFrameSync */
		{
			0x00000887,
			0x00000889,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eMacFrameSync */
		{
			0x000008a1,
			0x000008a3,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAmrWbFrameSync */
		{
			0x000006f8,
			0x000006fa,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eiLBCFrameSync */
		{
			0x000006f8,
			0x000006fa,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eiSACFrameSync */
		{
			0x000006f8,
			0x000006fa,
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
			0x00000ecd,
			0x00000ecf,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eALSFrameSync */
		{
			0x00000ed0,
			0x00000ed8,
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
			0x00000532,
			0x00000533,
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
			0x000006de,
			0x000006f6,
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
			0x0000055c,
			0x000005fa,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDtsEncode */
		{
			0x000004a8,
			0x000004b7,
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
			0x0000046c,
			0x00000485,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eG711G726Encode */
		{
			0x0000071c,
			0x00000721,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eG729Encode */
		{
			0x00000727,
			0x0000075d,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eG723_1Encode */
		{
			0x000007ab,
			0x000007c4,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eG722Encode */
		{
			0x000008dd,
			0x000008df,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAmrEncode */
		{
			0x0000090a,
			0x00000937,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eAmrwbEncode */
		{
			0x0000094d,
			0x00000973,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eiLBCEncode */
		{
			0x00000987,
			0x00000995,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eiSACEncode */
		{
			0x000009c9,
			0x000009e2,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eLpcmEncode */
		{
			0x00000a49,
			0x00000a4a,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eOpusEncode */
		{
			0x00000ad1,
			0x00000b28,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDDPEncode */
		{
			0x00000b4e,
			0x00000bd2,
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
			0x000007f4,
			0x00000845,
			0x00000000,
		},
		/* BDSP_VF_P_AlgoId_eX264Encode */
		{
			0x00000846,
			0x00000846,
			0x00000000,
		},
		/* BDSP_VF_P_AlgoId_eXVP8Encode */
		{
			0x00000864,
			0x00000864,
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
			0x000001ea,
			0x000001f6,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eMlpPassThru */
		{
			0x000008e2,
			0x000008e5,
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
			0x0000045e,
			0x00000467,
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
			0x0000037a,
			0x00000386,
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
			0x00000519,
			0x0000051d,
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
			0x000004c8,
			0x000004ca,
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
			0x000004ff,
			0x0000050e,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eSrsTruVolumePostProc */
		{
			0x000004ec,
			0x000004fa,
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
			0x00000523,
			0x00000526,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eFWMixerPostProc */
		{
			0x0000048c,
			0x0000049b,
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
			0x0000061e,
			0x00000632,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDv258PostProc */
		{
			0x0000063a,
			0x00000658,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eDpcmrPostProc */
		{
			0x0000065e,
			0x0000067c,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eGenCdbItbPostProc */
		{
			0x0000052a,
			0x00000530,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eBtscEncoderPostProc */
		{
			0x00000765,
			0x0000076e,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eSpeexAECPostProc */
		{
			0x000007c9,
			0x000007ec,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eKaraokePostProc */
		{
			0x00000a4c,
			0x00000a50,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eMixerDapv2PostProc */
		{
			0x00000c6f,
			0x00000cc7,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eOutputFormatterPostProc */
		{
			0x00000d83,
			0x00000d84,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eVocalPostProc */
		{
			0x00000d87,
			0x00000d89,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eFadeCtrlPostProc */
		{
			0x00000d8a,
			0x00000d8b,
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
			0x000006db,
			0x000006dc,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eMixerDapv2FrameSync */
		{
			0x00000c61,
			0x00000c64,
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
			0x0000001b,
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
			0x00000010,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eVidIdsCommon */
		{
			0x000007f0,
			0x000007f3,
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
			0x000009fb,
			0x000009fb,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eScm2 */
		{
			0x000009fc,
			0x000009fc,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eScm3 */
		{
			0x000009fd,
			0x00000a33,
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
			0x00000a3d,
			0x00000a3f,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eVideoDecodeTask */
		{
			0x00000a41,
			0x00000a43,
			0x00000000,
		},
		/* BDSP_AF_P_AlgoId_eVideoEncodeTask */
		{
			0x00000a45,
			0x00000a47,
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
