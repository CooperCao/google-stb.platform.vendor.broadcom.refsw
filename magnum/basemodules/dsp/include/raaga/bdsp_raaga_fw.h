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

#ifndef __BRAAGA_FW_PRIV_H
#define __BRAAGA_FW_PRIV_H

#include "bdsp_common_fw.h"

#define RAAGA_SCM_SUPPORT 1

/*Maximum frame size for an algorithm. PI uses this for allocating FMM ring buffer */
#define BDSP_AF_P_MAX_SAMPLING_RATE             ((uint32_t)48)  /* 48Khz (Max Sampling Frequency /1000) (to make in line of ms) */
#define BDSP_AF_P_MAX_INDEPENDENT_DELAY         ((uint32_t)500) /* Max independent delay in ms */

#define BDSP_AF_P_MAX_BLOCKING_TIME             BDSP_CIT_P_AUD_OFFSET /* AUD_OFFSET has to be >= to the worst case blocking time */
#define BDSP_AF_P_MAX_THRESHOLD                 BDSP_CIT_P_MINIMUM_ALGO_THRESHOLD

#define BDSP_AF_P_SAMPLE_PADDING                BDSP_CIT_P_MAXIMUM_RESIDUAL_COLLECTION /* Padding */
#define BDSP_AF_P_MAX_16X_BUF_SIZE              BDSP_CIT_P_MAT_BUF_SIZE /* MAT and DTSMA are the 16X compressed contents. MLP is the worst case now */

#define BDSP_AF_COMPUTE_RBUF_SIZE(delay, maxRate) \
    (BDSP_AF_P_MAX_BLOCKING_TIME * (maxRate) \
    + BDSP_AF_P_MAX_THRESHOLD + BDSP_AF_P_SAMPLE_PADDING \
    + (delay) * (maxRate)) \
    * 4                     /* 4 to make in bytes */

#define BDSP_AF_P_NON_DELAY_RBUF_SIZE   \
    (BDSP_AF_P_MAX_BLOCKING_TIME * BDSP_AF_P_MAX_SAMPLING_RATE \
    + BDSP_AF_P_MAX_THRESHOLD + BDSP_AF_P_SAMPLE_PADDING) \
    * 4                     /* 4 to make in bytes */

#define BDSP_AF_P_DELAY_RBUF_SIZE   \
    (BDSP_AF_P_MAX_BLOCKING_TIME * BDSP_AF_P_MAX_SAMPLING_RATE \
    + BDSP_AF_P_MAX_THRESHOLD + BDSP_AF_P_SAMPLE_PADDING  \
    + BDSP_AF_P_MAX_INDEPENDENT_DELAY * BDSP_AF_P_MAX_SAMPLING_RATE) \
    * 4                     /* 4 to make in bytes */

/**************************************************************************
        Inter Task Feed back buffer path
***************************************************************************/
#define BDSP_AF_P_INTERTASK_FEEDBACK_BUFFER_SIZE    (uint32_t)(128*4)
/***************************************************************************/
/**************************************************************************
        Video defines
***************************************************************************/
/* Considering ping-pong arrangement currently in discussion */
#define BDSP_FW_VIDEO_ENC_MAX_INTERRUPT_TO_DSP          2

/***************************************************************************
Summary:
    Enum data type describing the content type of SPDIF

Description:

      PCM =0;
      Compressed=1;

See Also:
    None.
****************************************************************************/
typedef enum BDSP_AF_P_SpdifContent
{
    BDSP_AF_P_PcmOnSpdif        = 0x0,
    BDSP_AF_P_CompressedOnSpdif = 0x1,
    BDSP_AF_P_SpdifContent_eMax,
    BDSP_AF_P_SpdifContent_eInvalid         = 0x7FFFFFFF

}BDSP_AF_P_SpdifContent;


/***************************************************************************
Summary:
    Enum data type having all algorithm ids of all the stages of algorithms
    supported in Audio Firmware.

Description:
    This is the enumerated data type used between Audio firmware and the PI
    to indicate the algorithm id of the code to be executed for a node. This
    enum is comprahensive and contains the stages of encode, decode & Post
    process algorithms. The frame sync and TSM executables of all the
    algorithms are also present in this enum.

See Also:
    BDSP_DSPCHN_AudioType
****************************************************************************/
typedef enum BDSP_SystemImgId
{
#if (BCHP_CHIP != 7278)
    BDSP_SystemImgId_eSystemCode,
    BDSP_SystemImgId_eSystemRdbvars,
    BDSP_SystemImgId_eSyslibCode,
    BDSP_SystemImgId_eAlgolibCode,
    BDSP_SystemImgId_eCommonIdsCode,
    BDSP_SystemImgId_eCommonVideoEncodeIdsCode,
    BDSP_SystemImgId_eCommonVideoEncodeIdsInterframe,
    BDSP_SystemImgId_eScm_Task_Code,
    BDSP_SystemImgId_eVideo_Decode_Task_Code,
    BDSP_SystemImgId_eVideo_Encode_Task_Code,
    BDSP_SystemImgId_eScm1_Digest,
    BDSP_SystemImgId_eScm2_Digest,
#else
    BDSP_SystemImgId_eSystemKernelCode,
    BDSP_SystemImgId_eSystemRdbvars,
    BDSP_SystemImgId_eSystemRomfsCode,
#endif
    BDSP_SystemImgId_eMax
} BDSP_SystemImgId;

typedef enum BDSP_AF_P_AlgoId
{
    /******************* Audio Algorithm Start ****************************/
    BDSP_AF_P_AlgoId_eAudioAlgoStartIdx = 0x0,          /*Audio Algorithm Start Index */
    BDSP_AF_P_AlgoId_eMpegDecode = BDSP_AF_P_AlgoId_eAudioAlgoStartIdx,
    BDSP_AF_P_AlgoId_eAc3Decode,
    BDSP_AF_P_AlgoId_eAacDecode,
    BDSP_AF_P_AlgoId_eAacHeLpSbrDecode,
    BDSP_AF_P_AlgoId_eDdpDecode,
    BDSP_AF_P_AlgoId_eDdLosslessDecode,
    BDSP_AF_P_AlgoId_eLpcmCustomDecode,
    BDSP_AF_P_AlgoId_eBdLpcmDecode,
    BDSP_AF_P_AlgoId_eDvdLpcmDecode,
    BDSP_AF_P_AlgoId_eHdDvdLpcmDecode,
    BDSP_AF_P_AlgoId_eMpegMcDecode,
    BDSP_AF_P_AlgoId_eWmaStdDecode,
    BDSP_AF_P_AlgoId_eWmaProStdDecode,
    BDSP_AF_P_AlgoId_eMlpDecode,
    BDSP_AF_P_AlgoId_eDdp71Decode,
    BDSP_AF_P_AlgoId_eDtsDecode,
    BDSP_AF_P_AlgoId_eDtsLbrDecode,
    BDSP_AF_P_AlgoId_eDtsHdDecode,
    BDSP_AF_P_AlgoId_ePcmWavDecode,
    BDSP_AF_P_AlgoId_eAmrDecode,
    BDSP_AF_P_AlgoId_eDraDecode,
    BDSP_AF_P_AlgoId_eRealAudioLbrDecode,
    BDSP_AF_P_AlgoId_eDolbyPulseDecode,
    BDSP_AF_P_AlgoId_eMs10DdpDecode,
    BDSP_AF_P_AlgoId_eAdpcmDecode,
    BDSP_AF_P_AlgoId_eG711G726Decode,
    BDSP_AF_P_AlgoId_eG729Decode,
    BDSP_AF_P_AlgoId_eVorbisDecode,
    BDSP_AF_P_AlgoId_eG723_1Decode,
    BDSP_AF_P_AlgoId_eFlacDecode,
    BDSP_AF_P_AlgoId_eMacDecode,
    BDSP_AF_P_AlgoId_eAmrWbDecode,
    BDSP_AF_P_AlgoId_eiLBCDecode,
    BDSP_AF_P_AlgoId_eiSACDecode,
    BDSP_AF_P_AlgoId_eUdcDecode,
    BDSP_AF_P_AlgoId_eDolbyAacheDecode,
    BDSP_AF_P_AlgoId_eOpusDecode,
    BDSP_AF_P_AlgoId_eALSDecode,
    BDSP_AF_P_AlgoId_eAC4Decode,
    BDSP_AF_P_AlgoId_eEndOfAudioDecodeAlgos,
    BDSP_VF_P_AlgoId_eVideoAlgoStartIdx,
    BDSP_VF_P_AlgoId_eRealVideo9Decode = BDSP_VF_P_AlgoId_eVideoAlgoStartIdx,
    BDSP_VF_P_AlgoId_eVP6Decode,
    BDSP_VF_P_AlgoId_eEndOfVideoDecodeAlgos,
    BDSP_AF_P_AlgoId_eEndOfDecodeAlgos = BDSP_VF_P_AlgoId_eEndOfVideoDecodeAlgos,

    /*  All the Algo Ids for Decoder Frame Sync */
    BDSP_AF_P_AlgoId_eMpegFrameSync,
    BDSP_AF_P_AlgoId_eMpegMcFrameSync,
    BDSP_AF_P_AlgoId_eAdtsFrameSync,
    BDSP_AF_P_AlgoId_eLoasFrameSync,
    BDSP_AF_P_AlgoId_eWmaStdFrameSync,
    BDSP_AF_P_AlgoId_eWmaProFrameSync,
    BDSP_AF_P_AlgoId_eAc3FrameSync,
    BDSP_AF_P_AlgoId_eDdpFrameSync,
    BDSP_AF_P_AlgoId_eDdp71FrameSync,
    BDSP_AF_P_AlgoId_eDtsFrameSync,
    BDSP_AF_P_AlgoId_eDtsLbrFrameSync,
    BDSP_AF_P_AlgoId_eDtsHdFrameSync,
    BDSP_AF_P_AlgoId_eDtsHdFrameSync_1,
    BDSP_AF_P_AlgoId_eDtsHdHdDvdFrameSync,
    BDSP_AF_P_AlgoId_eDdLosslessFrameSync,
    BDSP_AF_P_AlgoId_eMlpFrameSync,
    BDSP_AF_P_AlgoId_eMlpHdDvdFrameSync,
    BDSP_AF_P_AlgoId_ePesFrameSync,
    BDSP_AF_P_AlgoId_eBdLpcmFrameSync,
    BDSP_AF_P_AlgoId_eHdDvdLpcmFrameSync,
    BDSP_AF_P_AlgoId_eDvdLpcmFrameSync,
    BDSP_AF_P_AlgoId_eDvdLpcmFrameSync_1,
    BDSP_AF_P_AlgoId_ePcmWavFrameSync,
    BDSP_AF_P_AlgoId_eDraFrameSync,
    BDSP_AF_P_AlgoId_eRealAudioLbrFrameSync,
    BDSP_AF_P_AlgoId_eMs10DdpFrameSync,
    BDSP_AF_P_AlgoId_eVorbisFrameSync,
    BDSP_AF_P_AlgoId_eFlacFrameSync,
    BDSP_AF_P_AlgoId_eMacFrameSync,
    BDSP_AF_P_AlgoId_eUdcFrameSync,
    BDSP_AF_P_AlgoId_eAC4FrameSync,
    BDSP_AF_P_AlgoId_eALSFrameSync,
    BDSP_AF_P_AlgoId_eEndOfAudioDecFsAlgos,
    BDSP_VF_P_AlgoId_eVideoAlgoFsStartIdx,
    BDSP_VF_P_AlgoId_eRealVideo9FrameSync = BDSP_VF_P_AlgoId_eVideoAlgoFsStartIdx,
    BDSP_VF_P_AlgoId_eVP6FrameSync,
    BDSP_VF_P_AlgoId_eEndOfVideoDecFsAlgos,
    BDSP_AF_P_AlgoId_eEndOfDecFsAlgos = BDSP_VF_P_AlgoId_eEndOfVideoDecFsAlgos,

    /*  All the Algo Ids for the stages of encode algorithms */
    BDSP_AF_P_AlgoId_eAc3Encode,
    BDSP_AF_P_AlgoId_eMpegL2Encode,
    BDSP_AF_P_AlgoId_eMpegL3Encode,
    BDSP_AF_P_AlgoId_eAacLcEncode,
    BDSP_AF_P_AlgoId_eAacHeEncode,
    BDSP_AF_P_AlgoId_eDtsEncode,
    BDSP_AF_P_AlgoId_eDtsBroadcastEncode,
    BDSP_AF_P_AlgoId_eSbcEncode,
    BDSP_AF_P_AlgoId_eMs10DDTranscode,
    BDSP_AF_P_AlgoId_eG711G726Encode,
    BDSP_AF_P_AlgoId_eG729Encode,
    BDSP_AF_P_AlgoId_eG723_1Encode,
    BDSP_AF_P_AlgoId_eG722Encode,
    BDSP_AF_P_AlgoId_eAmrEncode,
    BDSP_AF_P_AlgoId_eAmrwbEncode,
    BDSP_AF_P_AlgoId_eiLBCEncode,
    BDSP_AF_P_AlgoId_eiSACEncode,
    BDSP_AF_P_AlgoId_eLpcmEncode,
    BDSP_AF_P_AlgoId_eOpusEncode,
    BDSP_AF_P_AlgoId_eDDPEncode,
    BDSP_AF_P_AlgoId_eEndOfAudioEncodeAlgos,
    BDSP_VF_P_AlgoId_eVideoEncodeAlgoStartIdx,
    BDSP_VF_P_AlgoId_eH264Encode = BDSP_VF_P_AlgoId_eVideoEncodeAlgoStartIdx,
    BDSP_VF_P_AlgoId_eX264Encode,
    BDSP_VF_P_AlgoId_eXVP8Encode,
    BDSP_VF_P_AlgoId_eEndOfVideoEncodeAlgos,
    BDSP_AF_P_AlgoId_eEndOfEncodeAlgos = BDSP_VF_P_AlgoId_eEndOfVideoEncodeAlgos,

    /*  All the Algo Ids for the stages of encode Algo Frame Syncs */
    BDSP_AF_P_AlgoId_eAc3EncFrameSync,
    BDSP_AF_P_AlgoId_eMpegL3EncFrameSync,
    BDSP_AF_P_AlgoId_eMpegL2EncFrameSync,
    BDSP_AF_P_AlgoId_eAacLcEncFrameSync,
    BDSP_AF_P_AlgoId_eAacHeEncFrameSync,
    BDSP_AF_P_AlgoId_eDtsEncFrameSync,
    BDSP_AF_P_AlgoId_eEndOfEncFsAlgos,

    /*  All the algo ids for the stages of passthrough */
    BDSP_AF_P_AlgoId_ePassThru,
    BDSP_AF_P_AlgoId_eMLPPassThru,
    BDSP_AF_P_AlgoId_eEndOfAuxAlgos,

    /*  All the Algo Ids for the stages of Post Proc algorithms */
    BDSP_AF_P_AlgoId_eSrsTruSurroundPostProc,
    BDSP_AF_P_AlgoId_eSrcPostProc,
    BDSP_AF_P_AlgoId_eDdbmPostProc,
    BDSP_AF_P_AlgoId_eDownmixPostProc,
    BDSP_AF_P_AlgoId_eCustomSurroundPostProc,
    BDSP_AF_P_AlgoId_eCustomBassPostProc,
    BDSP_AF_P_AlgoId_eKaraokeCapablePostProc,
    BDSP_AF_P_AlgoId_eCustomVoicePostProc,
    BDSP_AF_P_AlgoId_ePeqPostProc,
    BDSP_AF_P_AlgoId_eAvlPostProc,
    BDSP_AF_P_AlgoId_ePl2PostProc,
    BDSP_AF_P_AlgoId_eXenPostProc,
    BDSP_AF_P_AlgoId_eBbePostProc,
    BDSP_AF_P_AlgoId_eDsolaPostProc,
    BDSP_AF_P_AlgoId_eDtsNeoPostProc,
    BDSP_AF_P_AlgoId_eDDConvert,
    BDSP_AF_P_AlgoId_eAudioDescriptorFadePostProc,
    BDSP_AF_P_AlgoId_eAudioDescriptorPanPostProc,
    BDSP_AF_P_AlgoId_ePCMRouterPostProc,
    BDSP_AF_P_AlgoId_eWMAPassThrough,
    BDSP_AF_P_AlgoId_eSrsTruSurroundHDPostProc,
    BDSP_AF_P_AlgoId_eSrsTruVolumePostProc,
    BDSP_AF_P_AlgoId_eDolbyVolumePostProc,
    BDSP_AF_P_AlgoId_eBrcm3DSurroundPostProc,
    BDSP_AF_P_AlgoId_eFWMixerPostProc,
    BDSP_AF_P_AlgoId_eMonoDownMixPostProc,
    BDSP_AF_P_AlgoId_eMs10DDConvert,
    BDSP_AF_P_AlgoId_eDdrePostProc,
    BDSP_AF_P_AlgoId_eDv258PostProc,
    BDSP_AF_P_AlgoId_eDpcmrPostProc,
    BDSP_AF_P_AlgoId_eGenCdbItbPostProc,
    BDSP_AF_P_AlgoId_eBtscEncoderPostProc,
    BDSP_AF_P_AlgoId_eSpeexAECPostProc,
    BDSP_AF_P_AlgoId_eKaraokePostProc,
    BDSP_AF_P_AlgoId_eMixerDapv2PostProc,
    BDSP_AF_P_AlgoId_eOutputFormatterPostProc,
    BDSP_AF_P_AlgoId_eVocalPostProc,
    BDSP_AF_P_AlgoId_eFadeCtrlPostProc,
	BDSP_AF_P_AlgoId_eTsmCorrectionPostProc,
    BDSP_AF_P_AlgoId_eEndOfPpAlgos,

    /*  All algo Ids for post proc frame sync */
    BDSP_AF_P_AlgoId_eMixerFrameSync,
    BDSP_AF_P_AlgoId_eMixerDapv2FrameSync,
    BDSP_AF_P_AlgoId_eEndOfPpFsAlgos,

    /* All Algo ids for libraries*/
    BDSP_AF_P_AlgoId_eSysLib,
    BDSP_AF_P_AlgoId_eAlgoLib,
    BDSP_AF_P_AlgoId_eIDSCommonLib,
    BDSP_AF_P_AlgoId_eVidIDSCommonLib,

    BDSP_AF_P_AlgoId_eEndOfLibAlgos,

        BDSP_AF_P_AlgoId_eScm1,
        BDSP_AF_P_AlgoId_eScm2,
        BDSP_AF_P_AlgoId_eScm3,
        BDSP_AF_P_AlgoId_eEndOfScmAlgos,

    /* Algo IDs for SCM Task */
    BDSP_AF_P_AlgoId_eScmTask,
    BDSP_AF_P_AlgoId_eVideoDecodeTask,
    BDSP_AF_P_AlgoId_eVideoEncodeTask,
    BDSP_AF_P_AlgoId_eEndOfTaskAlgos,

    BDSP_AF_P_AlgoId_eEndOfAlgos,

    BDSP_AF_P_AlgoId_eMax,
    BDSP_AF_P_AlgoId_eInvalid = 0x7FFFFFFF
}BDSP_AF_P_AlgoId;


/***************************************************************************
Summary:
    This enumeration defines the Decode/PP/Enc audio datatype that is used
    by BSP's conditional Execution..

***************************************************************************/
typedef enum BDSP_AF_P_DecodeEncPPAlgoType
{
    BDSP_AF_P_DecodeAlgoType_eMpeg,             /* MPEG */
    BDSP_AF_P_DecodeAlgoType_eAac,              /* AAC */
    BDSP_AF_P_DecodeAlgoType_eAacAdts = BDSP_AF_P_DecodeAlgoType_eAac, /* AAC */
    BDSP_AF_P_DecodeAlgoType_eAacLoas,          /* AAC */
    BDSP_AF_P_DecodeAlgoType_eAacSbr,           /* AAC_SBR */
    BDSP_AF_P_DecodeAlgoType_eAacSbrAdts = BDSP_AF_P_DecodeAlgoType_eAacSbr, /* AAC_SBR */
    BDSP_AF_P_DecodeAlgoType_eAacSbrLoas,       /* AAC_SBR */
    BDSP_AF_P_DecodeAlgoType_eAc3,              /* AC3 */
    BDSP_AF_P_DecodeAlgoType_eAc3Plus,          /* AC3_PLUS */
    BDSP_AF_P_DecodeAlgoType_eDts,              /* DTS */
    BDSP_AF_P_DecodeAlgoType_eLpcmBd,           /* LPCM Blue Ray Disc */
    BDSP_AF_P_DecodeAlgoType_eLpcmHdDvd,        /* LPCM HD-DVD */
    BDSP_AF_P_DecodeAlgoType_eDtshd,            /* DTSHD */
    BDSP_AF_P_DecodeAlgoType_eLpcmDvd,          /* LPCM DVD */
    BDSP_AF_P_DecodeAlgoType_eWmaStd,           /* WMA Standard */
    BDSP_AF_P_DecodeAlgoType_eAc3Lossless,      /* AC3 in LOSSLESS*/
    BDSP_AF_P_DecodeAlgoType_eMlp,              /* MLP */
    BDSP_AF_P_DecodeAlgoType_ePcm,              /* PCM Data */
    BDSP_AF_P_DecodeAlgoType_eDtsLbr,           /* DTS-LBR */
    BDSP_AF_P_DecodeAlgoType_eDdp7_1,           /* DDP 7.1 */
    BDSP_AF_P_DecodeAlgoType_eMpegMc,           /* MPEG MC*/
    BDSP_AF_P_DecodeAlgoType_eWmaPro,           /* WMA Pro */
    BDSP_AF_P_DecodeAlgoType_eDtshdSub,         /* DTSHD SUB*/
    BDSP_AF_P_DecodeAlgoType_eLpcmDvdA,         /* LPCM A DVD*/
    BDSP_AF_P_DecodeAlgoType_eDtsBroadcast,     /* DTS Broadcast*/
    BDSP_AF_P_DecodeAlgoType_ePcmWav,           /* PCM WAV decoder*/
    BDSP_AF_P_DecodeAlgoType_eAmr,              /* AMR decoder */
    BDSP_AF_P_DecodeAlgoType_eDra,              /* DRA Decoder */
    BDSP_AF_P_DecodeAlgoType_eRealAudioLbr,     /* Real Audio Decoder */
    BDSP_AF_P_DecodeAlgoType_eAdpcm,            /* ADPCM Decoder */
    BDSP_AF_P_DecodeAlgoType_eG711G726,         /* G.711/726 Decoder */
    BDSP_AF_P_DecodeAlgoType_eG729,             /* G.729 Decoder */
    BDSP_AF_P_DecodeAlgoType_eVorbis,           /* Vorbis Decoder */
    BDSP_AF_P_DecodeAlgoType_eG723_1,           /* G.723.1 Decoder */
    BDSP_AF_P_DecodeAlgoType_eFlac,             /* Flac Decoder */
    BDSP_AF_P_DecodeAlgoType_eMac,             /* Mac Decoder */
    BDSP_AF_P_DecodeAlgoType_eAmrWb,            /* AMRWB decoder */
    BDSP_AF_P_DecodeAlgoType_eiLBC,            /* ILBC decoder */
    BDSP_AF_P_DecodeAlgoType_eiSAC,             /* iSAC decoder */
    BDSP_AF_P_DecodeAlgoType_eUdc,              /* UDC decoder */
    BDSP_AF_P_DecodeAlgoType_eOpus,             /* Opus Decoder */
    BDSP_AF_P_DecodeAlgoType_eALS,              /* ALS Decoder */
    BDSP_AF_P_DecodeAlgoType_eAC4,              /* AC4 Decoder */

    /* Extra Gap fillers*/
    BDSP_AF_P_DecodeAlgoType_eExtraDecodersStart,
    BDSP_AF_P_DecodeAlgoType_eExtraDecodersEnd = 49,
    /*MPEG Layers*/
    BDSP_AF_P_DecodeAlgoType_eMpegL2 = 50,
    BDSP_AF_P_DecodeAlgoType_eMpegL3 = 51,
    BDSP_AF_P_DecodeAlgoType_eEnd = BDSP_AF_P_DecodeAlgoType_eMpegL3,
    /*End of Decoders */

    /* Post processing */
    BDSP_AF_P_PostProcessingTypeStart,
    BDSP_AF_P_PostProcessingType_eDdbm = BDSP_AF_P_PostProcessingTypeStart,
    BDSP_AF_P_PostProcessingType_eDtsNeo,
    BDSP_AF_P_PostProcessingType_eAVL,
    BDSP_AF_P_PostProcessingType_eDDConvert,
    BDSP_AF_P_PostProcessingType_ePLll,
    BDSP_AF_P_PostProcessingType_eSrsXt,
    BDSP_AF_P_PostProcessingType_eXen,
    BDSP_AF_P_PostProcessingType_eBbe,
    BDSP_AF_P_PostProcessingType_eSrc,
    BDSP_AF_P_PostProcessingType_eCustomSurround,
    BDSP_AF_P_PostProcessingType_eCustomBass,
    BDSP_AF_P_PostProcessingType_eCustomVoice,
    BDSP_AF_P_PostProcessingType_ePeq,
    BDSP_AF_P_PostProcessingType_eAacDownmix,
    BDSP_AF_P_PostProcessingType_eAudioDescriptorFade,
    BDSP_AF_P_PostProcessingType_eAudioDescriptorPan,
    BDSP_AF_P_PostProcessingType_ePCMRouter,
    BDSP_AF_P_PostProcessingType_eWMAPassThrough,
    BDSP_AF_P_PostProcessingType_eDsola,
    BDSP_AF_P_PostProcessingType_eSrsHd,
    BDSP_AF_P_PostProcessingType_eGenericPassThru,
    BDSP_AF_P_PostProcessingType_eSrsTruVolume,
    BDSP_AF_P_PostProcessingType_eDolbyVolume,
    BDSP_AF_P_PostProcessingType_eBrcm3DSurround,
    BDSP_AF_P_PostProcessingType_eFWMixer,
    BDSP_AF_P_PostProcessingType_eDdre,
    BDSP_AF_P_PostProcessingType_eDv258,
    BDSP_AF_P_PostProcessingType_eDpcmr,
    BDSP_AF_P_PostProcessingType_eGenCdbItb,
    BDSP_AF_P_PostProcessingType_eBtscEncoder,
    BDSP_AF_P_PostProcessingType_eSpeexAec,     /* Echo Canceller */
    BDSP_AF_P_PostProcessingType_eKaraoke,      /* Karaoke */
    BDSP_AF_P_PostProcessingType_eMixerDapv2,
    BDSP_AF_P_PostProcessingType_eOutputFormatter,
    BDSP_AF_P_PostProcessingType_eVocalPP,
    BDSP_AF_P_PostProcessingType_eFadeCtrl,
	BDSP_AF_P_PostProcessingType_eTsmCorrection,
    /* Extra Gap fillers*/
    BDSP_AF_P_PostProcessingType_eExtraPPsStart,
    BDSP_AF_P_PostProcessingType_eExtraPPsEnd =96,
    BDSP_AF_P_PostProcessingType_eEnd = BDSP_AF_P_PostProcessingType_eExtraPPsEnd,
    /*End Post processing */

    /* Encoders processing */
    BDSP_AF_P_EncAudioTypeStart,
    BDSP_AF_P_EncAudioType_eMpeg1Layer3 = BDSP_AF_P_EncAudioTypeStart,
    BDSP_AF_P_EncAudioType_eMpeg1Layer2,
    BDSP_AF_P_EncAudioType_eDTS,
    BDSP_AF_P_EncAudioType_eAacLc,
    BDSP_AF_P_EncAudioType_eAacHe,
    BDSP_AF_P_EncAudioType_eAc3,
    BDSP_AF_P_EncAudioType_eDTSBroadcast,
    BDSP_AF_P_EncAudioType_eSbc,
    BDSP_AF_P_EncAudioType_eG711G726,
    BDSP_AF_P_EncAudioType_eG729,
    BDSP_AF_P_EncAudioType_eG723_1,
    BDSP_AF_P_EncAudioType_eG722,
    BDSP_AF_P_EncAudioType_eAmr,
    BDSP_AF_P_EncAudioType_eAmrwb,
    BDSP_AF_P_EncAudioType_eiLBC,
    BDSP_AF_P_EncAudioType_eiSAC,
    BDSP_AF_P_EncAudioType_eLpcm,
    BDSP_AF_P_EncAudioType_eOpus,
    BDSP_AF_P_EncAudioType_eDDP,
    /* Extra Gap fillers*/
    BDSP_AF_P_EncAudioType_eExtraEncsStart,
    BDSP_AF_P_EncAudioType_eExtraEncsEnd=114,
    BDSP_AF_P_EncAudioType_eEnd = BDSP_AF_P_EncAudioType_eExtraEncsEnd,

    BDSP_AF_P_VideoDecodeAlgoTypeStart,
    BDSP_AF_P_VideoDecodeAlgoType_eRealVideo9,
    BDSP_AF_P_VideoDecodeAlgoType_eVP6,
    BDSP_AF_P_VideoDecodeAlgoType_eEnd,
    BDSP_AF_P_ScmAlgoTypeStart,
    BDSP_AF_P_ScmAlgoType_eScm1 = BDSP_AF_P_ScmAlgoTypeStart,
    BDSP_AF_P_ScmAlgoType_eScm2,
    BDSP_AF_P_ScmAlgoType_eScm3,
    BDSP_AF_P_ScmAlgoType_eEnd,

    BDSP_AF_P_DecodeEncPPAlgoType_eMax,             /* Max value */
    BDSP_AF_P_DecodeEncPPAlgoType_eInvalid  = 0x7FFFFFFF

} BDSP_AF_P_DecodeEncPPAlgoType;


/***************************************************************************
Summary:
    The structure provides the outmode for various PP algos

See Also:
    None.
****************************************************************************/

typedef enum  BDSP_AF_P_OutModeType
{
    BDSP_AF_P_OutModeType_eBypass,       /*If Output Mode is same as input Mode */
    BDSP_AF_P_OutModeType_eFixed,        /*If Output type is always fixed, irrespective of input mode */
    BDSP_AF_P_OutModeType_eConfigurable  /*If Output mode can be configured by App */

}BDSP_AF_P_OutModeType;

typedef struct BDSP_AF_P_PpOutMode
{
    BDSP_AF_P_OutModeType       eOutModeType;
    BDSP_AF_P_DistinctOpType    eOpType; /*Valid only when eOutModeType is Fixed, Otherwise it will be invalid*/
}BDSP_AF_P_PpOutMode;


/****************************************************************************/

/***************************************************************************
Summary:
    Enum data types used to check the bit position status of a register
Description:

See Also:
    None.
****************************************************************************/
typedef enum BDSP_AF_P_SetReset
{
    BDSP_AF_P_eReset    = 0x0,
    BDSP_AF_P_eSet      = 0x1,
    BDSP_AF_P_SetReset_eMax,
    BDSP_AF_P_SetReset_eInvalid = 0x7FFFFFFF

}BDSP_AF_P_SetReset;


/***************************************************************************
Summary:
    Enum data types used to check the Licenseing permission for a Codec
Description:
            BDSP_AF_P_eExecuteAlgo      = Licence Present
            BDSP_AF_P_eDonotExecuteAlgo = No Licence Present
            BDSP_AF_P_eExecuteOnlyAACLC = Specific for AAC codec.

See Also:
    None.
****************************************************************************/
typedef enum BDSP_AF_P_AlgoExecLicenseStatus
{
    BDSP_AF_P_eExecuteAlgo          = 0x0,
    BDSP_AF_P_eDonotExecuteAlgo     = 0x1,
    BDSP_AF_P_eExecuteOnlyAACLC     = 0x2,

    BDSP_AF_P_AlgoExecLicenseStatus_eMax,
    BDSP_AF_P_AlgoExecLicenseStatus_eInvalid = 0x7FFFFFFF

}BDSP_AF_P_AlgoExecLicenseStatus;

/*********************************************************************
Summary:
    Structure to describe one VOM table entry

Description:
    This structure describes one entry of the VOM Table.

See Also:
**********************************************************************/
typedef struct BDSP_VOM_Table_Entry
{
#if (BCHP_CHIP == 7278)
    dramaddr_t          uiDramAddr;
    uint32_t            ui32AlgoSize;
#else
    uint32_t            ui32PageStart;
    uint32_t            ui32PageEnd;
    uint32_t            ui32DramAddr;
#endif
}BDSP_VOM_Table_Entry;

/*********************************************************************
Summary:
    Structure to describe VOM Table
Description:
    This structure describes VOM Table.
See Also:
**********************************************************************/
typedef struct BDSP_VOM_Table
{
    BDSP_VOM_Table_Entry sVomTableDetail[BDSP_AF_P_AlgoId_eMax];

}BDSP_VOM_Table;

/*********************************************************************
Summary:
    Structure to describe VOM code start address for every algorithm
    ID
Description:
    This structure describes the VOM code start address for every
    algorithm ID
See Also:
**********************************************************************/

typedef struct BDSP_VOM_Algo_Start_Addr
{
    uint32_t sVomAlgoStartAddr[BDSP_AF_P_AlgoId_eMax];

}BDSP_VOM_Algo_Start_Addr;

/***************************************************************************
Summary:
    The structure contains the configurations for an individual node.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_AF_P_sNODE_CONFIG
{
    uint32_t                    uiNodeId;
    BDSP_AF_P_EnableDisable     eCollectResidual;
    uint32_t                    ui32AudioAlgorithmType;
    BDSP_AF_P_AlgoId            eAlgoId;
    BDSP_AF_P_sDRAM_BUFFER      sDramUserConfigBuffer;
    BDSP_AF_P_sDRAM_BUFFER      sDramInterFrameBuffer;
    BDSP_AF_P_sDRAM_BUFFER      sDramAlgoCodeBuffer;
    BDSP_AF_P_sDRAM_BUFFER      sDramLookupTablesBuffer;
    BDSP_AF_P_sDRAM_BUFFER      sDramStatusBuffer;
    uint32_t                    ui32VomAlgoAddr;

    uint32_t                    ui32NumSrc;
    uint32_t                    ui32NumDst;

    /*The filed that tells whether the Node input is Valid/ Invalid : Valid =1 Invalid =0
      This field is required for Dynamic input port switching. All the input ports which
      are interstage buffers will be set to valid
    */
    BDSP_AF_P_ValidInvalid      eNodeIpValidFlag[BDSP_AF_P_MAX_IP_FORKS];

    dramaddr_t                  ui32NodeIpBuffCfgAddr[BDSP_AF_P_MAX_IP_FORKS];
    dramaddr_t                  ui32NodeIpGenericDataBuffCfgAddr[BDSP_AF_P_MAX_IP_FORKS];

    dramaddr_t                    ui32NodeOpBuffCfgAddr[BDSP_AF_P_MAX_OP_FORKS];
    dramaddr_t                    ui32NodeOpGenericDataBuffCfgAddr[BDSP_AF_P_MAX_OP_FORKS];

    BDSP_AF_P_DistinctOpType    eNodeOpBuffDataType[BDSP_AF_P_MAX_OP_FORKS];

}BDSP_AF_P_sNODE_CONFIG;


/***************************************************************************
Summary:
    The structure is complete task configuration structure. This contains
    the global task configuration and an array of node configuration
    structures.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_AF_P_sTASK_CONFIG
{
    BDSP_AF_P_sGLOBAL_TASK_CONFIG   sGlobalTaskConfig;
    BDSP_AF_P_sNODE_CONFIG          sNodeConfig[BDSP_AF_P_MAX_NODES];

}BDSP_AF_P_sTASK_CONFIG;


/***************************************************************************
Summary:
    This is a version configuration structure.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_VERSION_Table_Entry
{
    uint32_t AlgoId;
    char reference_version[32];
    char brcm_algo_version[32];
    char brcm_system_version[32];
}BDSP_VERSION_Table_Entry;


typedef struct BDSP_VERSION_Table
{
    BDSP_VERSION_Table_Entry sVersionTableDetail[BDSP_AF_P_AlgoId_eMax];
}BDSP_VERSION_Table;

/* Video specific definitions */
/****************************************************************************/
/****************************************************************************/
/************************* VIDEO TASK  **************************************/
/****************************************************************************/
/****************************************************************************/

#define BDSP_FWMAX_VIDEO_BUFF_AVAIL     (uint32_t)(16)
#define BDSP_FWMAX_VIDEO_REF_BUFF_AVAIL (uint32_t)(3)
#define BDSP_FWMAX_SCB_AVAIL            (uint32_t)(3)


/***************************************************************************
Summary:
    Data type in which the pixels are stored

Description:

See Also:
    None.
****************************************************************************/

typedef enum BDSP_VF_P_ePixelDataType
{
    BDSP_VF_P_ONE_BYTE = 0,
    BDSP_VF_P_TWO_BYTE,
    BDSP_VF_P_LAST,
    BDSP_VF_P_INVALID                       = 0x7FFFFFFF

}BDSP_VF_P_ePixelDataType;

/***************************************************************************
Summary:
    The structure contains the Picture Buffer addrs

Description:

See Also:
    None.
****************************************************************************/
typedef struct BDSP_VF_P_sFrameBuffParams
{
    BDSP_AF_P_sDRAM_BUFFER        sFrameBuffLuma;
    BDSP_AF_P_sDRAM_BUFFER        sFrameBuffChroma;

}BDSP_VF_P_sFrameBuffParams;

/***************************************************************************
Summary:
    The structure contains the buffer parameters

Description:

See Also:
    None.
****************************************************************************/

typedef struct
{
    uint32_t                    ui32NumBuffAvl;     /* Number of Valid entries in sBuffParams array */

    /* Stripe height of frame buffer allocated */
    uint32_t                    ui32LumaStripeHeight;
    uint32_t                    ui32ChromaStripeHeight;

    /* These structure will have DRAM start addresses of the different frame buffers */
    BDSP_VF_P_sFrameBuffParams  sBuffParams[BDSP_FWMAX_VIDEO_BUFF_AVAIL];

}sFrameBuffHandle;
/***************************************************************************
Summary:
    The structure contains all the global configurations of a task that comes
    as input from PI to CIT.

Description:
    All the input global configuration parameters that comes directly from PI
    to CIT are included in this structure. This structure will be passed as
    input argument to CIT generation function for Video.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_VF_P_sVDecodeBuffCfg
{
    /* This will have Ring buffer address associated with with PDQ */
    BDSP_AF_P_sDRAM_CIRCULAR_BUFFER sPDQ;

    /* This will have  Ring buffer address associated with with PRQ */
    BDSP_AF_P_sDRAM_CIRCULAR_BUFFER sPRQ;

    /* Max Picture size supported */
    uint32_t                    ui32MaxFrameHeight;
    uint32_t                    ui32MaxFrameWidth;

    /* Stripe width frame buffers allocated */
    uint32_t                    ui32StripeWidth;

    /* Decode and display buffer handles */
    sFrameBuffHandle            sDisplayFrameBuffParams;
    sFrameBuffHandle  sReferenceBuffParams;

    /* These structures will give DRAM start addresses of the UPBs
       Note: The virtual address of the Display buffer (luma and chroma) will be populated by CIT
     */
    BDSP_AF_P_sDRAM_BUFFER      sUPBs[BDSP_FWMAX_VIDEO_BUFF_AVAIL];

}BDSP_VF_P_sVDecodeBuffCfg;


/***************************************************************************
Summary:
    Enum to represent boolean

Description:

See Also:
    None.
****************************************************************************/
typedef enum
{
    BDSP_Raaga_Video_FALSE                                      = 0x0,
    BDSP_Raaga_Video_TRUE                                       = 0x1
}BDSP_Raaga_Video_eBOOLEAN;

typedef enum
{
    BDSP_Raaga_FALSE                                      = 0x0,
    BDSP_Raaga_TRUE                                       = 0x1
}BDSP_Raaga_eBOOLEAN;

typedef enum
{
    BDSP_Raaga_Video_Polarity_eTopField = 0,       /* Top field */
    BDSP_Raaga_Video_Polarity_eBotField,           /* Bottom field */
    BDSP_Raaga_Video_Polarity_eFrame,               /* Progressive frame */
    BDSP_Raaga_Video_Polarity_eInvalid      = 0x7FFFFFFF

} BDSP_Raaga_Video_Polarity;

typedef enum {
    BDSP_Raaga_Video_DCCparse_Format_Unknown = 0,
    BDSP_Raaga_Video_DCCparse_Format_DVS157,
    BDSP_Raaga_Video_DCCparse_Format_ATSC53,
    BDSP_Raaga_Video_DCCparse_Format_DVS053,
    BDSP_Raaga_Video_DCCparse_Format_SEI,
    BDSP_Raaga_Video_DCCparse_Format_SEI2,
    BDSP_Raaga_Video_DCCparse_Format_Divicom
}
BDSP_Raaga_Video_DCCparse_Format;

/***************************************************************************
Summary:
    The structure is for passing CC data from host to encoder.

See Also:
    None.
****************************************************************************/
typedef struct {
    uint32_t        cc_chunk_count;
    uint32_t        stgId;
    BDSP_Raaga_Video_eBOOLEAN        bIsAnalog;
    BDSP_Raaga_Video_Polarity        polarity;
    BDSP_Raaga_Video_DCCparse_Format format;
    uint32_t        cc_valid;
    uint32_t        cc_priority;
    uint32_t        line_offset;
    union {
        uint32_t field_number;  /* For DVS 157 formatted data   */
        uint32_t cc_type;       /* For all other formatted data */
    } seq;
    uint32_t        cc_data_1;
    uint32_t        cc_data_2;
    uint32_t        active_format;
}
BDSP_Raaga_Video_DCCparse_ccdata;


/***************************************************************************
Summary:
    The structure contains all the global configurations of a task.

Description:
    All the configuration which are common to the entire task are placed
    in the global task configuration.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_VF_P_sGLOBAL_TASK_CONFIG
{
    uint32_t                                ui32NumberOfNodesInTask;
    BDSP_VF_P_sVDecodeBuffCfg               sGlobalTaskConfigFromPI;

}BDSP_VF_P_sGLOBAL_TASK_CONFIG;

/***************************************************************************
Summary:
    The structure is complete task configuration structure. This contains
    the global task configuration and an array of node configuration
    structures.

See Also:
    None.
****************************************************************************/
typedef struct
{
    BDSP_VF_P_sGLOBAL_TASK_CONFIG   sGlobalTaskConfig;
    BDSP_AF_P_sNODE_CONFIG          sNodeConfig[BDSP_AF_P_MAX_NODES];

}BDSP_VF_P_sDEC_TASK_CONFIG;

/***************************************************************************
Summary:
    The structure contains all the global configurations of a task that comes
    as input from PI to CIT.

Description:
    All the input global configuration parameters that comes directly from PI
    to CIT are included in this structure. This structure will be passed as
    input argument to CIT generation function for Video.

See Also:
    None.
****************************************************************************/
/* Video Encoder Config */
/***************************************************************************
Summary:
    Defines the frame rate to encode.

Description:

See Also:
    None.
****************************************************************************/
typedef enum
{
   BDSP_VF_P_EncodeFrameRate_eUnknown = 0,
   BDSP_VF_P_EncodeFrameRate_e23_97,
   BDSP_VF_P_EncodeFrameRate_e24,
   BDSP_VF_P_EncodeFrameRate_e25,
   BDSP_VF_P_EncodeFrameRate_e29_97,
   BDSP_VF_P_EncodeFrameRate_e30,
   BDSP_VF_P_EncodeFrameRate_e50,
   BDSP_VF_P_EncodeFrameRate_e59_94,
   BDSP_VF_P_EncodeFrameRate_e60,
   BDSP_VF_P_EncodeFrameRate_e14_985,
   BDSP_VF_P_EncodeFrameRate_e7_493,
   BDSP_VF_P_EncodeFrameRate_e15,
   BDSP_VF_P_EncodeFrameRate_e10,
   BDSP_VF_P_EncodeFrameRate_e12_5,
   BDSP_VF_P_EncodeFrameRate_eMax,
   BDSP_VF_P_EncodeFrameRate_eInvalid = 0x7FFFFFFF
}BDSP_VF_P_eEncodeFrameRate;
/***************************************************************************
Summary:
    The structure contains the control parameters for the encoder

Description:

See Also:
    None.
****************************************************************************/
typedef struct BDSP_sENCODER_PARAMS
{
    BDSP_VF_P_eEncodeFrameRate          eEncodeFrameRate; /* Frame rate at which the encoder is expected to encode */
    uint32_t                            ui32Frames2Accum; /* = Gop2FrameAccumConvArray[Algo][BDSP_Raaga_Audio_eGOP_STRUCT] */
}BDSP_VF_P_sENCODER_PARAMS;

typedef struct BDSP_sEncodeParams
{
    BDSP_VF_P_eEncodeFrameRate          eEncodeFrameRate; /* Frame rate at which the encoder is expected to encode */
    uint32_t                            ui32Frames2Accum; /* = Gop2FrameAccumConvArray[Algo][BDSP_Raaga_Audio_eGOP_STRUCT] */

    /* This will hold the bit number [0...31] of ESR_SI register that will be used to interrupt DSP.
    * ping-pong design for interrupting raaga-dsp. Jason to confirm! */
    uint32_t                            ui32InterruptBit[BDSP_FW_VIDEO_ENC_MAX_INTERRUPT_TO_DSP];
    /* 32bit RDB address of ENCODER's STC */
    uint32_t                            ui32StcAddr;
    /* 32bit RDB address of ENCODER's STC_HI for 42bit STC. In case of 32bit stc, ui32StcAddr_hi should be zero */
    uint32_t                            ui32StcAddr_hi;
    /* 32bit RDB address from DSP page where pic metadata address will be updated. It will hold a DRAM address */
    uint32_t                            ui32RdbForPicDescp[BDSP_FW_VIDEO_ENC_MAX_INTERRUPT_TO_DSP];
    uint32_t                            IsGoBitInterruptEnabled;
	uint32_t							IsNrtModeEnabled;

}BDSP_VF_P_sEncodeParams;


typedef struct BDSP_VF_P_sVEncoderConfig
{
    BDSP_VF_P_sENCODER_PARAMS        sEncoderParams;

    /* Reference Buffer Params */
    /* Max Picture size supported */
    uint32_t                    ui32MaxFrameHeight;
    uint32_t                    ui32MaxFrameWidth;

    /* Stripe width frame buffers allocated */
    uint32_t                    ui32StripeWidth;


     /* Reference buff 2 buffers in case of B pictures */
    sFrameBuffHandle            sReferenceBuffParams;

    /* These structures will give DRAM start addresses of the PPBs  (picture parameter buffers)
       Note: The virtual address of the Display buffer (luma and chroma) will be populated by CIT
     */
    BDSP_AF_P_sDRAM_BUFFER      sPPBs[BDSP_FWMAX_VIDEO_BUFF_AVAIL];

}BDSP_VF_P_sVEncoderConfig;

   /* The above structure is exposed to VEE PI which is bad. Struct exposed to PI should not be in
     fw_priv include files. upper structure should go from fw_priv and below one should be used
     by firmware and bdsp handshake.
     */

typedef struct BDSP_VF_P_sVEncodeConfig
{
    BDSP_VF_P_sEncodeParams         sEncoderParams;

    /* Reference Buffer Params */
    /* Max Picture size supported */
    uint32_t                    ui32MaxFrameHeight;
    uint32_t                    ui32MaxFrameWidth;

    /* Stripe width frame buffers allocated */
    uint32_t                    ui32StripeWidth;

     /* Reference buff 2 buffers in case of B pictures */
    sFrameBuffHandle            sReferenceBuffParams;

    /* These structures will give DRAM start addresses of the PPBs  (picture parameter buffers)
       Note: The virtual address of the Display buffer (luma and chroma) will be populated by CIT
     */
    BDSP_AF_P_sDRAM_BUFFER      sPPBs[BDSP_FWMAX_VIDEO_BUFF_AVAIL];
    /* This will have the DRAM address of BDS_AF_P_IO_BUFFER structure which will have handle of RDQ and RRQ */
    BDSP_AF_P_sDRAM_BUFFER      sRawDataQueues;

}BDSP_VF_P_sVEncodeConfig;


/***************************************************************************
Summary:
    The structure contains all the global configurations of a task.

Description:
    All the configuration which are common to the entire task are placed
    in the global task configuration.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_VF_P_sENC_GLOBAL_TASK_CONFIG
{
    uint32_t                      ui32NumberOfNodesInTask;
    BDSP_VF_P_sVEncodeConfig      sGlobalVideoEncoderConfig;

}BDSP_VF_P_sENC_GLOBAL_TASK_CONFIG;



/***************************************************************************
Summary:
    The structure is complete task configuration structure. This contains
    the global task configuration and an array of node configuration
    structures.

See Also:
    None.
****************************************************************************/
typedef struct
{
    BDSP_VF_P_sENC_GLOBAL_TASK_CONFIG   sEncGlobalTaskConfig;
    BDSP_AF_P_sNODE_CONFIG          sNodeConfig[BDSP_AF_P_MAX_NODES];

}BDSP_VF_P_sENC_TASK_CONFIG;

/* Need to find a better place for this structure */

typedef enum
{
   BVENC_VF_ChromaSampling_e420,
   BVENC_VF_ChromaSampling_e422,
   BVENC_VF_ChromaSampling_e444,
   BVENC_VF_ChromaSampling_eLast,
   BVENC_VF_ChromaSampling_eInvalid               = 0x7FFFFFFF
}BVENC_VF_ChromaSampling;

typedef enum
{
    BVENC_VF_Frame_eInterlacedTop,
    BVENC_VF_Frame_eInterlacedBottom,
    BVENC_VF_Frame_eProgressive,
    BVENC_VF_Frame_eLast,
    BVENC_VF_Frame_eInvalid               = 0x7FFFFFFF
}BVENC_VF_FrameType;

typedef enum
{
   BVENC_VF_AspectRatio_eUnknown = 0,
   BVENC_VF_AspectRatio_eSquare,
   BVENC_VF_AspectRatio_e12_11,
   BVENC_VF_AspectRatio_e10_11,
   BVENC_VF_AspectRatio_e16_11,
   BVENC_VF_AspectRatio_e40_33,
   BVENC_VF_AspectRatio_e24_11,
   BVENC_VF_AspectRatio_e20_11,
   BVENC_VF_AspectRatio_e32_11,
   BVENC_VF_AspectRatio_e80_33,
   BVENC_VF_AspectRatio_e18_11,
   BVENC_VF_AspectRatio_e15_11,
   BVENC_VF_AspectRatio_e64_33,
   BVENC_VF_AspectRatio_e160_99,
   BVENC_VF_AspectRatio_e4_3,
   BVENC_VF_AspectRatio_e3_2,
   BVENC_VF_AspectRatio_e2_1,
   BVENC_VF_AspectRatio_e16_9,
   BVENC_VF_AspectRatio_e221_1,
   BVENC_VF_AspectRatio_eLast,
   BVENC_VF_AspectRatio_eInvalid      = 0x7FFFFFFF
}BVENC_VF_AspectRatio;

typedef enum {
    /* VDC definitions */
        BVENC_VF_PicStruct_eFrame = 0,
        BVENC_VF_PicStruct_eTopField,
        BVENC_VF_PicStruct_eBotField,
        BVENC_VF_PicStruct_eTopFirst,
        BVENC_VF_PicStruct_eBotFirst,
        BVENC_VF_PicStruct_eTopBotTopRepeat,
        BVENC_VF_PicStruct_eBotTopBotRepeat,
        BVENC_VF_PicStruct_eFrameDoubling,
        BVENC_VF_PicStruct_eFrameTripling,
        BVENC_VF_PicStruct_eReserved,
        BVENC_VF_PicStruct_eInvalid      = 0x7FFFFFFF
}BVENC_VF_sPicStruct ;

typedef struct
{
    uint32_t                            ui32PicHeight;
    uint32_t                            ui32PicWidth;

    BDSP_VF_P_eEncodeFrameRate          eEncodeFrameRate;
    BVENC_VF_FrameType                  eFrameType;
    uint32_t                            ui32OrignalPtsHigh;
    uint32_t                            ui32OrignalPtsLow;
    BVENC_VF_ChromaSampling             eChromaSampling;
    BDSP_Raaga_Video_eBOOLEAN           eStallStc;
    BDSP_Raaga_Video_eBOOLEAN           eIgnorePicture;
    uint32_t                            ui32AspectRatioIdc;
    uint32_t                            ui32SARWidth;
    uint32_t                            ui32SARHeight;

    uint32_t                            ulStripeWidth;
    uint32_t                            ulLumaNMBY;
    uint32_t                            ulChromaNMBY;
    uint32_t                            ulSTCSnapshotLo; /* lower 32-bit STC snapshot when picture received at the displayable point (in 27Mhz) */
    uint32_t                            ulSTCSnapshotHi; /* high 10-bit STC snapshot when picture received at the displayable point (in 27Mhz) */
    uint32_t                            ulPictureId;/* Displayable point Picture ID */


    uint32_t                            ui32BuffAddr2H1VY;
    uint32_t                            ui32BuffAddr2H1VUV;
    uint32_t                            ui32BuffAddr2H2VY;
    uint32_t                            ui32BuffAddr2H2VUV;

    BVENC_VF_sPicStruct                 ePicStruct;
    BDSP_Raaga_Video_Polarity           ePolarity;
    BDSP_Raaga_Video_eBOOLEAN           bStriped;
    BDSP_Raaga_Video_eBOOLEAN           bCadenceLocked;/* false if unused, Optional cadence info for PicAFF encode  */

}BVENC_sMETA_DATA;

typedef struct
{
    uint32_t                     ui32BuffAddrY;
    uint32_t                     ui32BuffAddrUV;
    uint32_t                     ui32CaptureTimeStamp;
    BVENC_sMETA_DATA             sPictureMetaData;
}BVENC_VF_sPicParamBuff;

typedef enum BDSP_OnDemandOutput_ErrorStatus
{
 BDSP_OnDemandOutput_ErrorStatus_eNone,
 BDSP_OnDemandOutput_ErrorStatus_eDecodeError,
 BDSP_OnDemandOutput_ErrorStatus_eBufferInsufficient,
 BDSP_OnDemandOutput_ErrorStatus_eChannelMapIncorrect,
 BDSP_OnDemandOutput_ErrorStatus_eInvalidInput,
 BDSP_OnDemandOutput_ErrorStatus_eSampleResolutionUnsupported,
 BDSP_OnDemandOutput_ErrorStatus_eMaxStatus=0x7FFFFFFF
}BDSP_OnDemandOutput_ErrorStatus;

typedef struct
{
    /* Elements sent from HOST to DSP */
    dramaddr_t ui32FrameBufBaseAddressLow;        /* The start address of this frame */
    dramaddr_t ui32FrameBufBaseAddressHigh;
    uint32_t ui32FrameBufferSizeInBytes;        /* The size of the buffer allocated for this frame. */

    /* Elements updated by DSP */
    uint32_t  ui32FrameValid;               /* Indicates whether the current frame is valid oor invalid */
    BDSP_OnDemandOutput_ErrorStatus  eErrorStatus;  /* Indicates of there is an error in the current frame */
    uint32_t  ui32SequenceNum;              /* Running counter. Updated everytime the Metadata is put out */

    uint32_t ui32PTS;                       /* PTS of the frame */
    BDSP_PtsType ePTSType;                  /* PTS Type */

    uint32_t ui32TotalBytesGenerated;       /* Size of the pcm in bytes after interleaving the samples */
    uint32_t ui32ActualBytesFilledInBuffer; /* Size of the interleaved pcm sample buffer uploaded to ADQ */

    uint32_t ui32SampleRate;                /* Sample Rate */

    uint32_t ui32StreamAcMod;                /* Acmod of the stream */
    uint32_t ui32OutAcMod;                   /* Output Acmod */
}BDSP_OnDemand_MetaDataInfo;

typedef struct
{
    BDSP_OnDemand_MetaDataInfo*     psMetaDataInfo;
}BDSP_OnDemandAudioInfo;

/***************************************************************************
Summary:
    Structure describing MPEG Ancillary Data Packetization

Description:
        None.

See Also:
    None.
****************************************************************************/

typedef struct BDSP_Raaga_Audio_MpegAncDataPacket
{
    uint32_t ui32Syncword;  /* Sync Word Pattern - should match BDSP_AF_P_MPEG_ANC_DATA_SYNCWORD - This sync word(0x4d504547) is basically 'M', 'P', 'E', 'G' put in a 4 byte number. */
    uint32_t ui32FrameNumber;  /* Decoded Frame Number */
    uint32_t ui32AncDataBitsWritten;  /* Number of bits of ancillary data written after the header and written in 32 bit aligned format */
} BDSP_Raaga_Audio_MpegAncDataPacket;



#ifdef RAAGA_SCM_SUPPORT

/****************************************************************************/
/****************************************************************************/
/**************************  SCM TASK  **************************************/
/****************************************************************************/
/****************************************************************************/

#define BDSP_AF_P_SCM_MAX_NODES    (uint32_t)1

/***************************************************************************
Summary:
    The structure contains all the global configurations of SCM task.

Description:
    All the configuration which are common to the entire task are placed
    in the global task configuration.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_SCM_P_sGLOBAL_TASK_CONFIG
{
    uint32_t                            ui32NumberOfNodesInTask;

    uint32_t                            ui32StartNodeIndexOfCoreScmAlgo;

    BDSP_AF_P_sDRAM_BUFFER              sDramScratchBuffer;

}BDSP_SCM_P_sGLOBAL_TASK_CONFIG;

/***************************************************************************
Summary:
    The structure is the Top level CIT datastructure for  SCM task.

Description:

     Contains
            1) Global Configuration
            2) Node Configuration

See Also:
    None.
****************************************************************************/
typedef struct  BDSP_SCM_P_sTASK_CONFIG
{
    BDSP_SCM_P_sGLOBAL_TASK_CONFIG  sGlobalTaskConfig;
    BDSP_AF_P_sNODE_CONFIG          sNodeConfig[BDSP_AF_P_SCM_MAX_NODES];

}BDSP_SCM_P_sTASK_CONFIG;


#endif

#endif
