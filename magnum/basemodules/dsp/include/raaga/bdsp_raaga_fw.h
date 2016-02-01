/******************************************************************************
 * (c) 2004-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
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
 *****************************************************************************/


#ifndef __BRAAGA_FW_PRIV_H
#define __BRAAGA_FW_PRIV_H

#include "bstd.h"
#include "bdsp_types.h"

#define RAAGA_SCM_SUPPORT 1
/* Enable the Capture Port cfg to FW */
#define CAP_PORT_SUPPORT
#define DUMMY_PORT_SUPPORT

/*  Based on the current DTS algorithm which has 5 decode stages
    One node is intially reserved for Frame sync */
#define BDSP_AF_P_MAX_NUM_NODES_IN_ALGO         ( 5 + 1 )

/*  Max number of nodes for the current worst case scenario */
#define BDSP_AF_P_MAX_NODES                 ( (uint32_t) 10 )

/*  Max channel for 7.1 + concurrent stereo o/p */
#define BDSP_AF_P_MAX_CHANNELS              ( (uint32_t) 8 )

#define BDSP_AF_P_MAX_CHANNEL_PAIR          (uint32_t)((BDSP_AF_P_MAX_CHANNELS+1)>>1)

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

/*  Max number of output ports. Currently we have accomadated
    for three branches, each branch having two o/p ports*/
#define BDSP_AF_P_MAX_BRANCHES              ( (uint32_t) 3 )
#define BDSP_AF_P_MAX_OP_PORTS_PER_BRANCH   ( (uint32_t) 2 )
#define BDSP_AF_P_MAX_OP_PORTS              ( BDSP_AF_P_MAX_BRANCHES * \
                                            BDSP_AF_P_MAX_OP_PORTS_PER_BRANCH )

/*Maximum number of Adaptive blocks
    PPM logic is using BDSP_AF_P_MAX_CHANNEL_PAIR for its operations as the CIT-PI interface is based on BDSP_AF_P_MAX_CHANNEL_PAIR.
    And BDSP_AF_P_MAX_ADAPTIVE_RATE_BLOCKS is used for better readability in CIT-FW interface.
    Actual HW block number is 4
*/
#ifdef BCM3548
    /* Maximum number of FMM Io buffer ports in a task*/
    #define  BDSP_AF_P_MAX_FMM_OP_PORTS_IN_TASK (6)

    /*  This defines the maximum number of different sources the node
        can take input from */
    #define BDSP_AF_P_MAX_IP_FORKS              4
    /*  This defines the maximum number of different destinations the node
        can output */
#define BDSP_AF_P_MAX_OP_FORKS                  6

    /*Maximum number of Adaptive blocks
            PPM logic is using BDSP_AF_P_MAX_CHANNEL_PAIR for its operations as the CIT-PI interface is based on BDSP_AF_P_MAX_CHANNEL_PAIR.
            And BDSP_AF_P_MAX_ADAPTIVE_RATE_BLOCKS is used for better readability in CIT-FW interface.
            Actual HW block number is 4
    */
    #define BDSP_AF_P_MAX_ADAPTIVE_RATE_BLOCKS      BDSP_AF_P_MAX_CHANNEL_PAIR

#else
    /* Maximum number of FMM Io buffer ports in a task*/
    #define  BDSP_AF_P_MAX_FMM_OP_PORTS_IN_TASK (6)

    /*  This defines the maximum number of different sources the node
        can take input from */
    #define BDSP_AF_P_MAX_IP_FORKS              4
    /*  This defines the maximum number of different destinations the node
        can output */
#define BDSP_AF_P_MAX_OP_FORKS                  6

    #define BDSP_AF_P_MAX_ADAPTIVE_RATE_BLOCKS      8
#endif

/*  Defines for the FMM o/p buff configuration */
#define BDSP_AF_P_MAX_NUM_DAC_PORTS         3   /* There are only 2 HIFIDACS for 7405/7325 family */
                                                /* There are only 3 HIFIDACS for 3548 family */
#define BDSP_AF_P_MAX_NUM_SPDIF_PORTS       2
#define BDSP_AF_P_MAX_NUM_PLLS              4
#define BDSP_AF_P_MAX_NUM_I2S_PORTS         4   /* 4 I2S ports: STEREO_0, STEREO_1, MULTI_0, MULTI_1 */
#define BDSP_AF_P_MAX_NUM_CAP_PORTS         4
#define BDSP_AF_P_MAX_NUM_DUMMY_PORTS       4   /* Dummy Ports */


/**************************************************************************
        Inter Task Communication Buffer allocation
***************************************************************************/
#define BDSP_AF_P_INTERTASK_IOBUFFER_SIZE       (uint32_t)(11200*4)
#define BDSP_AF_P_INTERTASK_IOGENBUFFER_SIZE    (uint32_t)(17408*2)

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
    Enum data type describing MS decode mode

Description:
        Single Decode
        MS10/Ms11/MSSoundEffect
See Also:
    None.
****************************************************************************/

typedef enum BDSP_AF_P_DolbyMsUsageMode
{
    BDSP_AF_P_DolbyMsUsageMode_eSingleDecodeMode=0,
    BDSP_AF_P_DolbyMsUsageMode_eMS10DecodeMode,
    BDSP_AF_P_DolbyMsUsageMode_eMS11DecodeMode,
    BDSP_AF_P_DolbyMsUsageMode_eMpegConformanceMode,
    BDSP_AF_P_DolbyMsUsageMode_eMS11SoundEffectMixing,
    BDSP_AF_P_DolbyMsUsageMode_eMS12DecodeMode,
    BDSP_AF_P_DolbyMsUsageMode_eLAST,
    BDSP_AF_P_DolbyMsUsageMode_eINVALID=0x7fffffff

} BDSP_AF_P_DolbyMsUsageMode;

/***************************************************************************
Summary:
    Enum data type describing MS decoder Type

Description:
        Primary/Secondary/SoundEffect
See Also:
    None.
****************************************************************************/
typedef enum
{
   BDSP_AF_P_DecoderType_ePrimary = 0,
   BDSP_AF_P_DecoderType_eSecondary,
   BDSP_AF_P_DecoderType_eSoundEffects,
   BDSP_AF_P_DecoderType_eApplicationAudio,
   BDSP_AF_P_DecoderType_eLAST,
   BDSP_AF_P_DecoderType_eINVALID = 0x7fffffff
}BDSP_AF_P_DecoderType ;

/***************************************************************************
Summary:
    Enum data type describing I2S MCLK Rate

Description:

  BDSP_AF_P_e256fs  = 0;
  BDSP_AF_P_e384fs  = 1;

See Also:
    None.
****************************************************************************/
typedef enum BDSP_AF_P_I2sMclkRate
{
    BDSP_AF_P_e256fs = 0x0,
    BDSP_AF_P_e384fs  = 0x1,
    BDSP_AF_P_I2sMclkRate_eMax,
    BDSP_AF_P_I2sMclkRate_eInvalid = 0x7FFFFFFF

}BDSP_AF_P_I2sMclkRate;

/***************************************************************************
Summary:
    Enum data type describing PLL or NCO Select

Description:

        PLL Select
        NCO Select

See Also:
    None.
****************************************************************************/
typedef enum BDSP_AF_P_PllNcoSelect
{
    BDSP_AF_P_NoneSelect = 0x0, /*This is for default case */
    BDSP_AF_P_PllSelect  = 0x1,
    BDSP_AF_P_NcoSelect  = 0x2,

    BDSP_AF_P_PllNcoSelect_eMax,
    BDSP_AF_P_PllNcoSelect_eInvalid = 0x7FFFFFFF

}BDSP_AF_P_PllNcoSelect;


/***************************************************************************
Summary:
    Enum data type describing the types of Port Enable Fields

Description:
    Enum data type describing the types of Port Enable Fields

  Enable =0;
  Disable=1;

See Also:
    None.
****************************************************************************/
typedef enum BDSP_AF_P_PortEnableDisable
{
    BDSP_AF_P_PortEnable  = 0x0,
    BDSP_AF_P_PortDisable = 0x1,
    BDSP_AF_P_PortEnableDisable_eMax,
    BDSP_AF_P_PortEnableDisable_eInvalid = 0x7FFFFFFF

}BDSP_AF_P_PortEnableDisable;

/***************************************************************************
Summary:
    Enum data type describing Enable and Disable

Description:

  Valid  =1;
  Invalid=0;

See Also:
    None.
****************************************************************************/
typedef enum BDSP_AF_P_EnableDisable
{
    BDSP_AF_P_eDisable = 0x0,
    BDSP_AF_P_eEnable  = 0x1,
    BDSP_AF_P_EnableDisable_eMax,
    BDSP_AF_P_EnableDisable_eInvalid = 0x7FFFFFFF

}BDSP_AF_P_EnableDisable;

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
    Enum data type describing the FMM Destination's associated sinking rate

Description:

     FMM Destination can be. FMM port or Capture port.
     There are a few rates classified for FMM Destination.

                    1)BaseRate
                    2)StreamSamplingRate
                    3)2xBaseRate
                    4)4xBaseRate
                    5)HBRRate

See Also:
    None.
****************************************************************************/

typedef enum BDSP_AF_P_FmmDstFsRate
{
    BDSP_AF_P_FmmDstFsRate_eBaseRate            = 0x0,
    BDSP_AF_P_FmmDstFsRate_eStreamSamplingRate  = 0x1,
    BDSP_AF_P_FmmDstFsRate_e2xBaseRate          = 0x2,
    BDSP_AF_P_FmmDstFsRate_e4xBaseRate          = 0x3,
    BDSP_AF_P_FmmDstFsRate_e16xBaseRate         = 0x4,
    BDSP_AF_P_FmmDstFsRate_eHBRRate             = BDSP_AF_P_FmmDstFsRate_e16xBaseRate,
    BDSP_AF_P_FmmDstFsRate_eInvalid,

    BDSP_AF_P_FmmDstFsRate_eMax                 = 0x7FFFFFFF

}BDSP_AF_P_FmmDstFsRate;

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
    BDSP_AF_P_AlgoId_eAmrFrameSync,
    BDSP_AF_P_AlgoId_eDraFrameSync,
    BDSP_AF_P_AlgoId_eRealAudioLbrFrameSync,
    BDSP_AF_P_AlgoId_eMs10DdpFrameSync,
    BDSP_AF_P_AlgoId_eAdpcmFrameSync,
    BDSP_AF_P_AlgoId_eG711G726FrameSync,
    BDSP_AF_P_AlgoId_eG729FrameSync,
    BDSP_AF_P_AlgoId_eVorbisFrameSync,
    BDSP_AF_P_AlgoId_eG723_1FrameSync,
    BDSP_AF_P_AlgoId_eFlacFrameSync,
    BDSP_AF_P_AlgoId_eMacFrameSync,
    BDSP_AF_P_AlgoId_eAmrWbFrameSync,
    BDSP_AF_P_AlgoId_eiLBCFrameSync,
    BDSP_AF_P_AlgoId_eiSACFrameSync,
    BDSP_AF_P_AlgoId_eUdcFrameSync,
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

/* Time base of the System */
typedef enum BDSP_AF_P_TimeBaseType
{
    BDSP_AF_P_TimeBaseType_e45Khz = 0,
    BDSP_AF_P_TimeBaseType_e27Mhz,
    BDSP_AF_P_TimeBaseType_eLast,
    BDSP_AF_P_TimeBaseType_eInvalid = 0x7FFFFFFF
}BDSP_AF_P_TimeBaseType;


/***************************************************************************
Summary:
    Enum data type describing the types of Sampling Frequency in the audio
    firmware.

    Structure definition to map the input sampling frequency to output
    sampling frequency.

See Also:
    None.
****************************************************************************/

typedef enum BDSP_AF_P_SampFreq
{
    BDSP_AF_P_SampFreq_e8Khz = 0,
    BDSP_AF_P_SampFreq_e11_025Khz,
    BDSP_AF_P_SampFreq_e12Khz,
    BDSP_AF_P_SampFreq_e16Khz,
    BDSP_AF_P_SampFreq_e22_05Khz,
    BDSP_AF_P_SampFreq_e24Khz,
    BDSP_AF_P_SampFreq_e32Khz,
    BDSP_AF_P_SampFreq_e44_1Khz,
    BDSP_AF_P_SampFreq_e48Khz,
    BDSP_AF_P_SampFreq_e64Khz,
    BDSP_AF_P_SampFreq_e88_2Khz,
    BDSP_AF_P_SampFreq_e96Khz,
    BDSP_AF_P_SampFreq_e128Khz,
    BDSP_AF_P_SampFreq_e176_4Khz,
    BDSP_AF_P_SampFreq_e192Khz,
    BDSP_AF_P_SampFreq_eMax,
    BDSP_AF_P_SampFreq_eInvalid = 0x7FFFFFFF

}BDSP_AF_P_SampFreq;

typedef struct BDSP_AF_P_sOpSamplingFreq
{
    uint32_t ui32OpSamplingFrequency[BDSP_AF_P_SampFreq_eMax];

}BDSP_AF_P_sOpSamplingFreq;

/***************************************************************************
Summary:
    Enum data type describing the types of inter-frame buffers in the audio
    firmware.

Description:
    This enum describes the type of the inter-frame buffer being used by the
    node. The inter-frame buffer can be present, absent or shared. when shared,
    it means that the current node of the algo shares the inter-frame buffer
    with the previous node in the same algo.

See Also:
    None.
****************************************************************************/

typedef enum BDSP_AF_P_InterFrameBuffType
{
    BDSP_AF_P_InterFrameBuffType_eAbsent = 0x0,
    BDSP_AF_P_InterFrameBuffType_ePresent,
    BDSP_AF_P_InterFrameBuffType_eShared,

    BDSP_AF_P_InterFrameBuffType_eMax,
    BDSP_AF_P_InterFrameBuffType_eInvalid = 0x7FFFFFFF

}BDSP_AF_P_InterFrameBuffType;


/*********************************************************************
Summary:
    This enum defines the DSP mode output type

Description:

See Also:
**********************************************************************/
typedef enum BDSP_AF_P_DistinctOpType
{
    BDSP_AF_P_DistinctOpType_e7_1_MixPcm,
    BDSP_AF_P_DistinctOpType_e5_1_MixPcm,       /* mixed 5.1 PCM */
    BDSP_AF_P_DistinctOpType_eStereo_MixPcm,        /* mixed stereo PCM */
    BDSP_AF_P_DistinctOpType_e7_1_PCM,              /* 7.1 PCM */
    BDSP_AF_P_DistinctOpType_e5_1_PCM,              /* 5.1 PCM */
    BDSP_AF_P_DistinctOpType_eStereo_PCM,           /* Downmixed to stereo PCM */
    BDSP_AF_P_DistinctOpType_eMono_PCM,                 /* Mono output type to be used with speech codecs (G.7XX) and AEC modules */
    BDSP_AF_P_DistinctOpType_eCompressed,           /* Compressed Data */
    BDSP_AF_P_DistinctOpType_eAuxilaryDataOut,      /* Auxillary Data like 1) Converter input data 2) ITB generation data etc*/
    BDSP_AF_P_DistinctOpType_eGenericIsData,        /* Any kind of data on Interstage buffer*/
    BDSP_AF_P_DistinctOpType_eCdb,                  /* Compressed Elementary stream to be filled in CDB */
    BDSP_AF_P_DistinctOpType_eItb,                  /* ITB data going out of DSP */
    BDSP_AF_P_DistinctOpType_eDolbyReEncodeAuxDataOut,  /* Re-encode auxiliary data out */
    BDSP_AF_P_DistinctOpType_eCompressed4x,         /* 4x Compressed Rate - for DD+/DTS-HRA Passthru */
    BDSP_AF_P_DistinctOpType_eCompressedHBR,        /* HBR Passthru - True-HD/DTS-MA Passthru  */
    BDSP_AF_P_DistinctOpType_eAncillaryData,        /* MPEG Ancillary Data  */
    BDSP_AF_P_DistinctOpType_eDescriptorQueue,      /* Descriptor Queue for the Pooled buffers*/
    BDSP_AF_P_DistinctOpType_eMax,
    BDSP_AF_P_DistinctOpType_eInvalid = 0x7FFFFFFF

}BDSP_AF_P_DistinctOpType;


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

/*********************************************************************
Summary:
    This enum defines the FMM buffer content:
    1) PCM content
    2) Compressed content
Description:
    This enum is inially defined to inform FW to do Zero / Pause bursts
    during ZERO- Filling in Decode task

See Also:
**********************************************************************/

typedef enum BDSP_AF_P_FmmContentType
{

    BDSP_AF_P_FmmContentType_ePcm,              /* PCM */
    BDSP_AF_P_FmmContentType_eCompressed,       /* Compressed Data */
    BDSP_AF_P_FmmContentType_eAnalogCompressed, /* Analog Compressed Data for BTSC Encoder*/
    BDSP_AF_P_FmmContentType_eMax,
    BDSP_AF_P_FmmContentType_eInvalid = 0x7FFFFFFF

}BDSP_AF_P_FmmContentType;

/***************************************************************************
Summary:
    Enum data type describing the types of Firmware-Status buffers in the audio
    firmware.

Description:
    This enum describes the type of the Firmware-Status buffer being used by the
    node. The Firmware-Status buffer can be present, absent or shared. when shared,
    it means that the current node of the algo shares the Firmware-Status buffer
    with the previous node in the same algo.

See Also:
    None.
****************************************************************************/

typedef enum
{
    BDSP_AF_P_FwStatus_eAbsent = 0x0,
    BDSP_AF_P_FwStatus_ePresent,
    BDSP_AF_P_FwStatus_eShared,

    BDSP_AF_P_FwStatus_eMax,
    BDSP_AF_P_FwStatus_eInvalid = 0x7FFFFFFF
}BDSP_AF_P_FwStatus;
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
    Enum data types used to check a field is valid/invalid
Description:

See Also:
    None.
****************************************************************************/
typedef enum BDSP_AF_P_ValidInvalid
{
    BDSP_AF_P_eInvalid  = 0x0,
    BDSP_AF_P_eValid    = 0x1,
    BDSP_AF_P_ValidInvalid_eMax,
    BDSP_AF_P_ValidInvalid_eInvalid = 0x7FFFFFFF

}BDSP_AF_P_ValidInvalid;

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
    uint32_t            ui32PageStart;
    uint32_t            ui32PageEnd;
    uint32_t            ui32DramAddr;
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
    Node info structure between the firmware and the PI in the audio
    firmware.

Description:
    Each executable to provide info to the CIT generation module on the
    sizes of individual buffers. This is to be done through this structure.
    The max frame size and the max num channels supported will be used to
    internally partition the inter-stage I/O buffer into different channels

    How these two fields should be set for a post processing algorithm: The
    frame size must be set to the minimum chunk the post proc algo can
    work on. The number of channels supported must be the max it can support.
    Ex: DDBM works on 512 samples and 7.1 channels on 7440.

    To save DRAM buffer, the file where this structure sits for an algorithm
    can be made chip specific. Some chips requiring to support 7.1 will need
    additional memory

    To save some more DRAM buffer, while filling the data for post proc, we
    can ensure that the number of channels is set to a minimum (say stereo)
    this will ensure that the acmod of the decoder/encoder dictate the number
    of channel buffers required.
    The above is a drawback in a pure post process task where we may be
    working on 5.1 channels.

See Also:
    None.
****************************************************************************/

typedef struct BDSP_AF_P_sNODE_INFO
{
    uint32_t                        ui32CodeSize;
    uint32_t                        ui32RomTableSize;
    uint32_t                        ui32InterFrmBuffSize;
    uint32_t                        ui32InterStgIoBuffSize;
    uint32_t                        ui32InterStgGenericBuffSize;
    uint32_t                        ui32ScratchBuffSize;
    uint32_t                        ui32UserCfgBuffSize;
    uint32_t                        ui32MaxSizePerChan;
    uint32_t                        ui32MaxNumChansSupported;
    BDSP_AF_P_InterFrameBuffType    eInterFrameBuffType;

    BDSP_AF_P_FwStatus              eStatusBuffType;
    uint32_t                        ui32StatusBuffSize;


} BDSP_AF_P_sNODE_INFO;


/***************************************************************************
Summary:
    Enum to hold the type of the buffer.

Description:
    The buffer type indicates the type of i/o buffer being configured as
    input or output for the current node.

See Also:
    None.
****************************************************************************/

typedef enum BDSP_AF_P_BufferType
{
    BDSP_AF_P_BufferType_eDRAM      = 0,        /*  This is used in case of a DRAM ring buffer,
                                                where in the read, write pointers are also in DRAM. */
    BDSP_AF_P_BufferType_eRDB,                  /*  This is used in case of a ring buffer,
                                                where in the read, write pointers are in RDB registers. */
    BDSP_AF_P_BufferType_eFMM,                  /*  Used in case of FMM ring buffers. The address of the
                                                registers holding the read,write pointers etc will be
                                                programmed by the host.In this case, a GISB access in done
                                                by the firmware to get the actual read,write etc. pointers */
    BDSP_AF_P_BufferType_eRAVE,                 /*  Used in case of RAVE ring buffers. Need to differentiate
                                                between FMM and RAVE registers as RAVE registers use the
                                                wrap bit for buffer wrap around indication */
    BDSP_AF_P_BufferType_eDRAM_IS,              /*  DRAM inter stage buffer. Only the start address of the
                                                buffer has to be programmed by the host. */

    BDSP_AF_P_BufferType_eFMMSlave,             /* This type shows that the Buffer is FMM and its is configured
                                                as Slave */

    BDSP_AF_P_BufferType_eRDBPool,                 /* This type is used when a pool of buffers are allocated in DRAM,
                                                   the pointers to which are in DRAM, but the pointer is passed as data
                                                   through RDB registers */

    BDSP_AF_P_BufferType_eLAST,
    BDSP_AF_P_BufferType_eInvalid = 0x7FFFFFFF
}BDSP_AF_P_BufferType;

/***************************************************************************
Summary:
    Structure that holds all the genereic circular buffer parameters.

Description:
    This structure contains the unified circular buffer structure. All the
    circular buffer parameters like read/write base and end parameters are
    present in this structure. The circular buffer structure has been unified
    taking into account both the FMM and RAVE type of ciruclar buffer.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_AF_P_sDRAM_CIRCULAR_BUFFER
{
    raaga_dramaddr  ui32BaseAddr;       /*  Circular buffer's base address */
    raaga_dramaddr  ui32EndAddr;        /*  Circular buffer's End address */
    raaga_dramaddr  ui32ReadAddr;       /*  Circular buffer's read address */
    raaga_dramaddr  ui32WriteAddr;  /*  Circular buffer's write address */
    raaga_dramaddr  ui32WrapAddr;       /*  Circular buffer's wrap address */
}BDSP_AF_P_sDRAM_CIRCULAR_BUFFER;





/***************************************************************************
Summary:
    The I/O buffer structure used for as configuration for each node.

Description:
    This structure contains the I/O buffer configuration of a node.

See Also:
    None.
****************************************************************************/

typedef struct BDSP_AF_P_sIO_BUFFER
{
    uint32_t                        ui32NumBuffers; /*  Defines the number of
                                                        channels in the input
                                                        or output */
    BDSP_AF_P_BufferType            eBufferType;    /*  Defines the the location
                                                        of the input or output buffer.
                                                        This can take values defined
                                                        by eBUFFER_TYPE */
    BDSP_AF_P_sDRAM_CIRCULAR_BUFFER     sCircBuffer[BDSP_AF_P_MAX_CHANNELS];    /*  All circular buffer
                                                                                parameters wrt to each
                                                                                input buffer comes */

}BDSP_AF_P_sIO_BUFFER;

/***************************************************************************
Summary:
    The Generic buffer structure used as configuration for each node.

Description:
    This structure contains the configuration for the genric buffer of a node.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_AF_P_sIO_GENERIC_BUFFER
{
    uint32_t                            ui32NumBuffers; /*  Defines the number of
                                                        channels in the input
                                                        or output */
    BDSP_AF_P_BufferType                eBufferType;    /*  Defines the the location
                                                        of the input or output buffer.
                                                        This can take values defined
                                                        by eBUFFER_TYPE */
    BDSP_AF_P_sDRAM_CIRCULAR_BUFFER     sCircBuffer;    /*  All circular buffer parameters
                                                        wrt to each input buffer comes */

}BDSP_AF_P_sIO_GENERIC_BUFFER;

/***************************************************************************
Summary:
    The single circ buffer structure used to provide detaios of one circular
    buffer of any buffer type

Description:

See Also:
    None.
****************************************************************************/
typedef struct BDSP_AF_P_sSINGLE_CIRC_BUFFER
{

    BDSP_AF_P_BufferType                eBufferType;    /*  Defines the the location
                                                        of the input or output buffer.
                                                        This can take values defined
                                                        by eBUFFER_TYPE */
    BDSP_AF_P_sDRAM_CIRCULAR_BUFFER     sCircBuffer;    /*  All circular buffer parameters
                                                        wrt to each input buffer comes */

}BDSP_AF_P_sSINGLE_CIRC_BUFFER;

/***************************************************************************
Summary:
    The simple DRAM buffer structure which contains a DRAM address and Size.

See Also:
    None.
****************************************************************************/

typedef struct BDSP_AF_P_sDRAM_BUFFER
{
    raaga_dramaddr              ui32DramBufferAddress;
    uint32_t                    ui32BufferSizeInBytes;
}BDSP_AF_P_sDRAM_BUFFER;



/* GATE OPEN CONFIGURATION */

/***************************************************************************
Summary:
    The gate open Configuration of a port

Description:

    This contains the Start write pointers of all port channels and the
    Dram address of IO Buffer configuration


See Also:
    None.
****************************************************************************/

typedef struct BDSP_AF_P_sFMM_GATE_OPEN_CONFIG
{
    uint32_t                    ui32IndepDelay;

    raaga_dramaddr              uin32RingBufStartWrPointAddr[BDSP_AF_P_MAX_CHANNELS];

    raaga_dramaddr              uin32DramIoConfigAddr;

    /*FMM buffer content : Compressed / PCM */
    BDSP_AF_P_FmmContentType    eFMMContentType;

    /*FMM buffer Sinking rate */
    BDSP_AF_P_FmmDstFsRate      eFmmDstFsRate;

    uint32_t                    ui32BufferDepthThreshold; /* Threshold value to determine the depth of the FMM buffer  */

} BDSP_AF_P_sFMM_GATE_OPEN_CONFIG;


/***************************************************************************
Summary:
    The gate open Configuration of a task

Description:

    This contains the Start write pointers of all port channels and the
    Dram address of IO Buffer configuration of a task.


See Also:
    None.
****************************************************************************/
typedef struct BDSP_AF_P_TASK_sFMM_GATE_OPEN_CONFIG
{
    uint32_t    ui32NumPorts;

    uint32_t    ui32MaxIndepDelay ;

    BDSP_AF_P_sFMM_GATE_OPEN_CONFIG  sFmmGateOpenConfig[BDSP_AF_P_MAX_FMM_OP_PORTS_IN_TASK];

}BDSP_AF_P_TASK_sFMM_GATE_OPEN_CONFIG;


/***************************************************************************
Summary:
    The structure that contains the PPM valid flag and the PPM cfg address.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_AF_P_sPPM_CFG
{
    BDSP_AF_P_EnableDisable ePPMChannel;
    raaga_dramaddr          ui32PPMCfgAddr;

}BDSP_AF_P_sPPM_CFG;

/***************************************************************************
Summary:

    The structure that contains all hardware cfg RDBs used by FW .

See Also:
    None.
****************************************************************************/

typedef struct BDSP_AF_P_sFW_HW_CFG
{
    /*PPM Configuration */
    BDSP_AF_P_sPPM_CFG      sPpmCfg[BDSP_AF_P_MAX_ADAPTIVE_RATE_BLOCKS];

}BDSP_AF_P_sFW_HW_CFG;


/* PORT CONFIGURATION */
/***************************************************************************
Summary:
    The structure that contains the destination configuration for a NCO
    port.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_AF_P_sFMM_DEST_NCO_CFG
{
    uint32_t                    ui32NcoSampleIncRegAddr;
    uint32_t                    ui32NcoPhaseIncRegAddr;
    uint32_t                    ui32NcoRateRatioRegAddr;

}BDSP_AF_P_sFMM_DEST_NCO_CFG;

/***************************************************************************
Summary:
    The structure that contains the destination configuration for a DAC o/p
    port.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_AF_P_sFMM_DEST_DAC_CFG
{
    uint32_t                    ui32DacEnabled;

    BDSP_AF_P_FmmDstFsRate      eFmmDstFsRate;  /* FMM Destination's associated
                                                   sinking rate
                                                */

    uint32_t                    ui32HifidacRmSampleIncRegAddr;
    uint32_t                    ui32HifidacRmPhaseIncRegAddr;
    uint32_t                    ui32HifidacRmRateRatioRegAddr;

    uint32_t                    ui32HifidacRateManagerCfgAddr;  /* HIFIDAC CONTROL REGISTER  */

}BDSP_AF_P_sFMM_DEST_DAC_CFG;


/***************************************************************************
Summary:
    The structure that contains the destination configuration for a SPDIF o/p
    port.

See Also:
    None.
****************************************************************************/

/* Host info for SPDIF parameters from Scheduler */
typedef struct
{
    int32_t i32ProfessionalModeFlag;
    int32_t i32SoftwareCopyrightAsserted;
    int32_t i32CategoryCode;
    int32_t i32ClockAccuracy;
    int32_t i32bSeparateLRChanNum;
    int32_t i32CgmsA;
    /* Range values for this field.

        0       CopyFreely      Unlimited copies may be made of the content.
        1       CopyNoMore      One generation of copies has already been made; no further copying is allowed.
        2       CopyOnce        One generation of copies may be made
        3       CopyNever       No copies may be made of the content.

        default =0
    */
} BDSP_AF_P_sSPDIF_USER_CFG;


typedef struct
{

    uint32_t                    ui32SpdifEnabled;               /* Valid True and its values*/

    BDSP_AF_P_FmmDstFsRate      eFmmDstFsRate;

    uint32_t                    ui32AudioSpdifClkMacroRegAddr;      /* If index is 0-->AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0 */
                                                                /* If index is 1-->AUD_FMM_OP_CTRL_MCLK_CFG_FLEX */
                                                                /*..BUT doesnt exist for 7405 */

    uint32_t                    ui32SpdifCbitCtrlReg;           /* If  index 0 -->AUD_FMM_MS_CTRL_FW_CBIT_CTRL_0 */
                                                                /* If  index 1 -->AUD_FMM_MS_CTRL_FW_CBIT_CTRL_1 */

    uint32_t                    ui32CbitBasePingAddr;           /* If index 0--> AUD_FMM_MS_CTRL_FW_CBITS0 */
                                                                /* If index 1--> AUD_FMM_MS_CTRL_FW_CBITS24 */

    uint32_t                    ui32CbitBasePongAddr;           /* If index 0--> AUD_FMM_MS_CTRL_FW_CBITS12 */
                                                                /* If index 1--> AUD_FMM_MS_CTRL_FW_CBITS36 */

    uint32_t                    ui32PingPongMask;               /* If Index 0-->AUD_FMM_MS_ESR_STATUS_CBIT_PING_PONG0_MASK */
                                                                /* If Index 1-->AUD_FMM_MS_ESR_STATUS_CBIT_PING_PONG1_MASK */

    uint32_t                    ui32AudFmmMsCtrlHwSpdifCfg;

    uint32_t                    ui32SpdifDramConfigPtr;         /* Address to BDSP_AF_P_sSPDIF_USER_CFG Structure in Dram */

    uint32_t                    ui32PcmOnSpdif; /* 0=  PCM  and 1= Compressed*/

}BDSP_AF_P_sFMM_DEST_SPDIF_CLK_CBIT_CFG;

typedef struct
{
    uint32_t                    ui32AudFmmMsEsrStatusRegAddr;       /**From RDB: AUD_FMM_MS_ESR_STATUS **/
    uint32_t                    ui32AudFmmMsEsrStatusClearRegAddr;  /**From RDB: AUD_FMM_MS_ESR_STATUS_CLEAR*/
}BDSP_AF_P_sFMM_ESR_REGISTER;


typedef struct
{
    BDSP_AF_P_sFMM_DEST_SPDIF_CLK_CBIT_CFG          sFmmDestSpdifClkCbitCfg[BDSP_AF_P_MAX_NUM_SPDIF_PORTS];
    BDSP_AF_P_sFMM_ESR_REGISTER                     sFmmEsrRegister;

}BDSP_AF_P_sFMM_DEST_SPDIF_CFG;

/***************************************************************************
Summary:
    The structure that contains the destination configuration for a I2S o/p
    port.

See Also:
    None.
****************************************************************************/
typedef struct
{
    uint32_t                    ui32I2SEnabled;

    BDSP_AF_P_FmmDstFsRate      eFmmDstFsRate;  /* FMM Destination's associated
                                                   sinking rate
                                                */
    uint32_t                    ui32AudioI2SClkMacroRegAddr;/* If Index 0 -> AUD_FMM_OP_CTRL_MCLK_CFG_I2S_STEREO_0   */
                                                            /* If Index 1 -> AUD_FMM_OP_CTRL_MCLK_CFG_I2S_STEREO_1   */
                                                            /* If Index 2 --> AUD_FMM_OP_CTRL_MCLK_CFG_I2S_MULTI_0   */
                                                            /* If Index 3 --> AUD_FMM_OP_CTRL_MCLK_CFG_I2S_MULTI_1   */
}BDSP_AF_P_sFMM_DEST_I2S_CLK_CFG;

/***************************************************************************
Summary:
    The structure that contains the destination configuration for a Cap port
    port.

See Also:
    None.
****************************************************************************/
typedef struct
{
    uint32_t                    ui32CapPortEnabled;

    BDSP_AF_P_FmmDstFsRate      eFmmDstFsRate;  /* FMM Destination's associated
                                                   sinking rate
                                                */

    uint32_t                    ui32AudioCapPortClkMacroRegAddr;

}BDSP_AF_P_sFMM_DEST_CAP_PORT_CLK_CFG;
/***************************************************************************
Summary:
    The structure that contains the destination configuration for a Dummy port
    port.

See Also:
    None.
****************************************************************************/
typedef struct
{
    uint32_t                    ui32DummyPortEnabled;

    BDSP_AF_P_FmmDstFsRate      eFmmDstFsRate;  /* FMM Destination's associated
                                                   sinking rate
                                                */

    uint32_t                    ui32DummyPortClkMacroRegAddr;

}BDSP_AF_P_sFMM_DEST_DUMMY_PORT_CLK_CFG;
/***************************************************************************
Summary:
    The structure that contains the destination configuration for o/p PLL.

See Also:
    None.
****************************************************************************/
/*

There are 2 plls in system . But can be 4...
In a task only one PLL
*/

typedef struct{

                                                            /* Valid True and its values*/
    uint32_t                        ui32PllEnabled;
    uint32_t                        ui32PllIndex;           /* PLL Index . This is for individual PLL configuration*/
                                                            /* Used in I2S/SPDIF configuration */

    uint32_t                        ui32AudioPllMacroRegAddr; /*If Index 0 -->AUD_FMM_PLL0_MACRO */
                                                              /*If Index 1 -->AUD_FMM_PLL1_MACRO */

    /*
     * For Raaga ui32AudioPllMacroRegAddr is BCHP_VCXO_MISC_AUDIO_MODE_CTRL in both cases.
     */
    uint32_t                        ui32AudioPllCtrl0Addr;      /*If Index 0 -->AUDIO0_PLL_PLL_6CH_CTRL_0*/
                                                                /*If Index 1 -->AUDIO1_PLL_PLL_6CH_CTRL_0*/
    uint32_t                        ui32AudioPllCtrl1Addr;      /*If Index 0 -->AUDIO0_PLL_PLL_6CH_CTRL_1*/
                                                                /*If Index 1 -->AUDIO1_PLL_PLL_6CH_CTRL_1*/
    uint32_t                        ui32AudioPllCtrl2Addr;      /*If Index 0 -->AUDIO0_PLL_PLL_6CH_CTRL_1*/
                                                                /*If Index 1 -->AUDIO1_PLL_PLL_6CH_CTRL_1*/
    uint32_t                        ui32AudioPllCtrl3Addr;      /*If Index 0 -->AUDIO0_PLL_PLL_6CH_CTRL_1*/
                                                                /*If Index 1 -->AUDIO1_PLL_PLL_6CH_CTRL_1*/
    uint32_t                        ui32AudioPllCtrl4Addr;      /*If Index 0 -->AUDIO0_PLL_PLL_6CH_CTRL_1*/
                                                                /*If Index 1 -->AUDIO1_PLL_PLL_6CH_CTRL_1*/
    uint32_t                        ui32AudioPllCtrl5Addr;      /*If Index 0 -->AUDIO0_PLL_PLL_6CH_CTRL_1*/
                                                                /*If Index 1 -->AUDIO1_PLL_PLL_6CH_CTRL_1*/
    uint32_t                        ui32AudioPllCtrl6Addr;      /*If Index 0 -->AUDIO0_PLL_PLL_6CH_CTRL_1*/
                                                                /*If Index 1 -->AUDIO1_PLL_PLL_6CH_CTRL_1*/
    uint32_t                        ui32AudioPllCtrlOut0Addr;   /*If Index 0 -->AUDIO0_PLL_PLL_6CH_CTRL_OUT_CH0*/
                                                                /*If Index 1 -->AUDIO1_PLL_PLL_6CH_CTRL_OUT_CH0*/
    uint32_t                        ui32AudioPllCtrlOut1Addr;   /*If Index 0 -->AUDIO0_PLL_PLL_6CH_CTRL_OUT_CH1*/
                                                                /*If Index 1 -->AUDIO1_PLL_PLL_6CH_CTRL_OUT_CH1*/
    uint32_t                        ui32AudioPllCtrlOut2Addr;   /*If Index 0 -->AUDIO0_PLL_PLL_6CH_CTRL_OUT_CH2*/
                                                                /*If Index 1 -->AUDIO1_PLL_PLL_6CH_CTRL_OUT_CH2*/
    uint32_t                        ui32AudioPllCtrlOut3Addr;   /*If Index 0 -->AUDIO0_PLL_PLL_6CH_CTRL_OUT_CH2*/
                                                                /*If Index 1 -->AUDIO1_PLL_PLL_6CH_CTRL_OUT_CH2*/
    uint32_t                        ui32AudioPllCtrlOut4Addr;   /*If Index 0 -->AUDIO0_PLL_PLL_6CH_CTRL_OUT_CH2*/
                                                                /*If Index 1 -->AUDIO1_PLL_PLL_6CH_CTRL_OUT_CH2*/
    uint32_t                        ui32AudioPllCtrlOut5Addr;   /*If Index 0 -->AUDIO0_PLL_PLL_6CH_CTRL_OUT_CH2*/
                                                                /*If Index 1 -->AUDIO1_PLL_PLL_6CH_CTRL_OUT_CH2*/
    BDSP_AF_P_I2sMclkRate           eFmmI2sMclRate;
}BDSP_AF_P_sFMM_DEST_PLL_CFG;


/***************************************************************************
Summary:
    The structure that contains the FMM destination configuration registers.

See Also:
    None.
****************************************************************************/
typedef struct
{

    BDSP_AF_P_PllNcoSelect                  eFmmPllNcoSelect;
    BDSP_AF_P_sFMM_DEST_NCO_CFG             sFmmDestNcoCfg;
    BDSP_AF_P_sFMM_DEST_PLL_CFG             sFmmDestPllCfg;
    BDSP_AF_P_sFMM_DEST_I2S_CLK_CFG         sFmmDestI2SClkCfg[BDSP_AF_P_MAX_NUM_I2S_PORTS];
    BDSP_AF_P_sFMM_DEST_DAC_CFG             sFmmDestDacCfg[BDSP_AF_P_MAX_NUM_DAC_PORTS];
    BDSP_AF_P_sFMM_DEST_SPDIF_CFG           sFmmDestSpdifCfg;
    /*uint32_t                              ui32HbrEnable;*/
    uint32_t                                ui32HwCbitsEnable;

    BDSP_AF_P_FmmDstFsRate                  eHdmiFsRate; /* Sinking Rate of HDMI.
                                                            This information is used by Data Sync to raise Fs interrupt.
                                                          */

    /* Capture port Cfg */
#ifdef CAP_PORT_SUPPORT
    BDSP_AF_P_sFMM_DEST_CAP_PORT_CLK_CFG    sFmmCapPortClkCfg[BDSP_AF_P_MAX_NUM_CAP_PORTS];
#endif
#if defined DUMMY_PORT_SUPPORT
    BDSP_AF_P_sFMM_DEST_DUMMY_PORT_CLK_CFG  sFmmDummyPortClkCfg[BDSP_AF_P_MAX_NUM_DUMMY_PORTS];
#endif
}BDSP_AF_P_sFMM_DEST_CFG;

/***************************************************************************
Summary:
    The structure contains all information regarding soft increment of STC

Description:
    This structure contains configuration info of soft STC increment.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_AF_P_sStcTrigConfig
{
    /* If soft triggering is required. Default = BDSP_AF_P_eDisable */
    BDSP_AF_P_EnableDisable eStcTrigRequired;
    /* High and Low part of registers to tell the amount of STC increment. */
    uint32_t                ui32StcIncHiAddr;
    uint32_t                ui32StcIncLowAddr;
    /* Address of register to send trigger for incrementing STC */
    uint32_t                ui32StcIncTrigAddr;
/* Trigger bit in the above register. Bit count [031]*/
    uint32_t                ui32TriggerBit;

}BDSP_AF_P_sStcTrigConfig;

/***************************************************************************
Summary:
    The structure that contains all the o/p port configurations.

Description:
    This is the address where the o/p buffer configuration for the port is
    present.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_AF_P_sOP_PORT_CFG
{
    uint32_t                    ui32OpPortCfgAddr;
    uint32_t                    ui32FmmAdaptRateCntrlRegAddr;
    uint32_t                    ui32IndependentDelaySign;
    uint32_t                    ui32IndependentDelayValue;
}BDSP_AF_P_sOP_PORT_CFG;


/***************************************************************************
Summary:
    The structure contains all the global configurations of a task.

Description:
    All the configuration which are common to the entire task are placed
    in the global task configuration.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_AF_P_sGLOBAL_TASK_CONFIG
{
    uint32_t                    ui32NumberOfNodesInTask;
    uint32_t                    ui32NumberOfZeroFillSamples;
    uint32_t                    ui32StartNodeIndexOfCoreAudioAlgorithm; /*  This node id defines
                                                                            the starting index of the
                                                                            core audio processing stages.
                                                                            For a decoder task, which has
                                                                            frame sync and TSM codes in
                                                                            separate executable, Node 0
                                                                            and Node 1 will be reserved
                                                                            for frame sync and TSM. The
                                                                            field "StartNodeIndexOfCoreAudioAlgorithm"
                                                                            shall be set to 2 */



    uint32_t                    ui32FmmDestCfgAddr;
                                                                    /*  FMM destination configuration information. This structure is required
                                                                        one per Task. How to associate an o/p port with the correct sampling
                                                                        frequency to be programmed in the case of SRC? */

    uint32_t                    ui32FmmGateOpenConfigAddr;

    uint32_t                    ui32FwOpSamplingFreqMapLutAddr;     /*  This is the FW Input-Output sampling frequency mapping LUT*/

    uint32_t                    ui32NumOpPorts;                     /*  This tells the number of output ports */

    BDSP_AF_P_sOP_PORT_CFG      sOpPortCfg[BDSP_AF_P_MAX_OP_PORTS]; /*  This will have the DRAM address of Output port's IO_buffer
                                                                        Structure **/
    BDSP_AF_P_sDRAM_BUFFER      sDramScratchBuffer;                 /*  The scratch buffer is being moved to global task config */

    uint32_t                    ui32TaskFwHwCfgAddr;                /*  This address contains the structure of BDSP_AF_P_sFW_HW_CFG*/

    BDSP_AF_P_TimeBaseType      eTimeBaseType;                      /*  Time base type for a task 45Khz or 27 Mhz (Direct TV) */

    uint32_t                    ui32StcTrigConfigAddr;              /* DRAM address where STC trigger configuratio is passed */

    /*  These fields are reserved for future usage */
    uint32_t                    ui32Reserved0;
    uint32_t                    ui32Reserved1;
    uint32_t                    ui32Reserved2;
    uint32_t                    ui32Reserved3;
}BDSP_AF_P_sGLOBAL_TASK_CONFIG;


/***************************************************************************
Summary:
    The structure contains the configurations for an individual node.

See Also:
    None.
****************************************************************************/
typedef struct
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

    raaga_dramaddr              ui32NodeIpBuffCfgAddr[BDSP_AF_P_MAX_IP_FORKS];
    raaga_dramaddr              ui32NodeIpGenericDataBuffCfgAddr[BDSP_AF_P_MAX_IP_FORKS];

    uint32_t                    ui32NodeOpBuffCfgAddr[BDSP_AF_P_MAX_OP_FORKS];
    uint32_t                    ui32NodeOpGenericDataBuffCfgAddr[BDSP_AF_P_MAX_OP_FORKS];

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
typedef struct
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
    Display and reference buffer info structure.

Description:
       This structure contains info required to calculate the size and number of display and
       reference buffers to allocated for any specifc algo Id.
       Calculation as below
       ReferenceBufferSize_Luma = (DispBufferWidth_luma + (2 * ui32PaddingX))
                                                   * (DispBufferHeight_luma + (2 * ui32PaddingY) ;
       ReferenceBufferSize_Chrom = (DispBufferWidth_chrom + (2 * ui32PaddingX))
                                                   * (DispBufferHeight_chrom + (2 * ui32PaddingY) ;

See Also:
    None.
****************************************************************************/
typedef struct
{
    /* Ref Buffer info */
    uint32_t        ui32NumRefBuff;
    uint32_t        ui32MinNumRefBuff;
    BDSP_VF_P_ePixelDataType   eRefPixelDataType;  /* Storage datatype used by a pixel */

    /* Disp Buffer info */
    uint32_t        ui32NumDispBuff;
    uint32_t        ui32MinNumDispBuff;
    BDSP_VF_P_ePixelDataType   eDispPixelDataType;  /* Storage datatype used by a pixel */

    uint32_t        ui32PaddingX;
    uint32_t        ui32PaddingY;

}BDSP_VF_P_sOutputBuffInfo;

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
    BDSP_Raaga_Video_DCCparse_Format_AFD53,
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


typedef enum BDSP_PtsType
{
 BDSP_PtsType_eCoded,
 BDSP_PtsType_eInterpolatedFromValidPTS,
 BDSP_PtsType_eInterpolatedFromInvalidPTS = 3,
 BDSP_PtsType_eLast,
 BDSP_PtsType_eInvalid = 0x7FFFFFFF
}BDSP_PtsType;

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
    uint32_t ui32FrameBufBaseAddressLow;        /* The start address of this frame */
    uint32_t ui32FrameBufBaseAddressHigh;
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
