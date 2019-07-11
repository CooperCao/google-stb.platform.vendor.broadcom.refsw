/******************************************************************************
 * Copyright (C) 2019 Broadcom.
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
 *****************************************************************************/

#include "bape_types.h"
#include "bdsp.h"

#ifndef BAPE_DSP_UTILS_PRIV_H_
#define BAPE_DSP_UTILS_PRIV_H_

#if defined BDSP_MS10_SUPPORT || defined BDSP_DOLBY_DCV_SUPPORT
  #define BAPE_DSP_LEGACY_DDP_ALGO        1
#else /* Standalone and MS12 use UDC Algo */
  #define BAPE_DSP_LEGACY_DDP_ALGO        0
#endif

/***************************************************************************
Summary:
Codec Attribute Table Entry
***************************************************************************/
typedef struct BAPE_CodecAttributes
{
    BAVC_AudioCompressionStd codec;
    BDSP_Algorithm decodeAlgorithm;
    BDSP_Algorithm passthroughAlgorithm;
    BDSP_Algorithm encodeAlgorithm;
    const char *pName;
    BAPE_MultichannelFormat multichannelFormat;
    bool compressedOutputValid;
    bool srcRequired;               /* True if this codec supports non-standard sample rates such as LSF/QSF */
    bool genericPassthruRequired;   /* True if GenericPassthru should be added for this decode algorithm */
    bool simulValid;                /* True if this codec supports simul mode of decode + passthrough.  */
    bool monoOutputValid;           /* True if this codec supports mono output */
    bool compressed4xOutputValid;   /* True if compressed 4x is valid for this codec */
    bool compressed16xOutputValid;  /* True if compressed 16x is valid for this codec */
} BAPE_CodecAttributes;

/***************************************************************************
Summary:
Get Codec Attributes
***************************************************************************/
const BAPE_CodecAttributes *BAPE_P_GetCodecAttributes_isrsafe(
    BAVC_AudioCompressionStd codec
    );

/***************************************************************************
Summary:
Get Mixer Algo
***************************************************************************/
BDSP_Algorithm BAPE_P_GetCodecMixer_isrsafe(void);
BAPE_DolbyMSVersion BAPE_P_GetDolbyMSVersion_isrsafe(void);
BAPE_DolbyMs12Config BAPE_P_GetDolbyMS12Config_isrsafe(void);
BAPE_DolbyMSVersion BAPE_P_FwMixer_GetDolbyUsageVersion_isrsafe(BAPE_MixerHandle handle);

bool BAPE_P_DolbyCapabilities_Ac3Encode_isrsafe(void);
bool BAPE_P_DolbyCapabilities_DdpEncode_isrsafe(BAPE_MultichannelFormat format);
BAPE_MultichannelFormat BAPE_P_DolbyCapabilities_MultichannelPcmFormat_isrsafe(void);
bool BAPE_P_DolbyCapabilities_Dapv2_isrsafe(void);


#define BAPE_P_GetCodecName(codec) (BAPE_P_GetCodecAttributes_isrsafe((codec))->pName)
#define BAPE_P_GetCodecAudioDecode(codec) (BAPE_P_GetCodecAttributes_isrsafe((codec))->decodeAlgorithm)
#define BAPE_P_GetCodecAudioPassthrough(codec) (BAPE_P_GetCodecAttributes_isrsafe((codec))->passthroughAlgorithm)
#define BAPE_P_GetCodecAudioEncode(codec) (BAPE_P_GetCodecAttributes_isrsafe((codec))->encodeAlgorithm)
#define BAPE_P_GetCodecMixer(void) (BAPE_P_GetCodecMixer_isrsafe())
#define BAPE_P_GetCodecMultichannelFormat(codec) (BAPE_P_GetCodecAttributes_isrsafe((codec))->multichannelFormat)
#define BAPE_P_GetDolbyMSVersion(void) (BAPE_P_GetDolbyMSVersion_isrsafe())
#define BAPE_P_GetDolbyMS12Config(void) (BAPE_P_GetDolbyMS12Config_isrsafe())
#define BAPE_P_FwMixer_GetDolbyUsageVersion(handle) BAPE_P_FwMixer_GetDolbyUsageVersion_isrsafe(handle)
#define BAPE_P_CodecSupportsPassthrough(codec) (BAPE_P_GetCodecAttributes_isrsafe((codec))->compressedOutputValid)
#define BAPE_P_CodecRequiresSrc(codec) (BAPE_P_GetCodecAttributes_isrsafe((codec))->srcRequired)
#define BAPE_P_CodecRequiresGenericPassthru(codec) (BAPE_P_GetCodecAttributes_isrsafe((codec))->genericPassthruRequired)
#define BAPE_P_CodecSupportsSimulMode(codec) (BAPE_P_GetCodecAttributes_isrsafe((codec))->compressedOutputValid && BAPE_P_GetCodecAttributes_isrsafe((codec))->simulValid)
#define BAPE_P_CodecSupportsMono(codec) (BAPE_P_GetCodecAttributes_isrsafe((codec))->monoOutputValid)
#define BAPE_P_CodecSupportsCompressed4x(codec) (BAPE_P_GetCodecAttributes_isrsafe((codec))->compressed4xOutputValid)
#define BAPE_P_CodecSupportsCompressed16x(codec) (BAPE_P_GetCodecAttributes_isrsafe((codec))->compressed16xOutputValid)

#define BAPE_P_DolbyCapabilities_Ac3Encode(void) (BAPE_P_DolbyCapabilities_Ac3Encode_isrsafe())
#define BAPE_P_DolbyCapabilities_DdpEncode(format) (BAPE_P_DolbyCapabilities_DdpEncode_isrsafe(format))
#define BAPE_P_DolbyCapabilities_MultichannelPcmFormat(void) (BAPE_P_DolbyCapabilities_MultichannelPcmFormat_isrsafe())
#define BAPE_P_DolbyCapabilities_Dapv2(void) (BAPE_P_DolbyCapabilities_Dapv2_isrsafe())

/***************************************************************************
Summary:
Check if an algorithm is supported
***************************************************************************/
bool BAPE_DSP_P_AlgorithmSupported(BAPE_Handle hApe, BDSP_Algorithm algorithm);

/***************************************************************************
Summary:
Check if an algorithm is supported by both BDSP and APE
***************************************************************************/
bool BAPE_DSP_P_AlgorithmSupportedByApe(BAPE_Handle hApe, BDSP_Algorithm algorithm);

/***************************************************************************
Summary:
Define inter-task buffer size
***************************************************************************/
#define BAPE_P_INTER_TASK_BUFFER_SIZE (4+BDSP_AF_P_INTERTASK_IOBUFFER_SIZE)

/***************************************************************************
Summary:
Helper to print a variable name and value when assigning DSP settings structures
***************************************************************************/
#define BAPE_DSP_P_SET_VARIABLE(st,var,val) do { st.var = (val); BDBG_MSG(("%s: %s = %#x", BSTD_FUNCTION, #var, (st.var))); } while (0)

/***************************************************************************
Summary:
Helper to validate lower and upper range of variable
***************************************************************************/
#define BAPE_DSP_P_VALIDATE_VARIABLE_RANGE(var,low,high) (var<low?low:var>high?high:var)

/***************************************************************************
Summary:
Helper to validate lower and upper range of variable
***************************************************************************/
#define BAPE_DSP_P_VALIDATE_VARIABLE_UPPER(var,high) (var>high?high:var)

/***************************************************************************
Summary:
Helper to determine if LFE should be enabled
***************************************************************************/
#define BAPE_DSP_P_IsLfePermitted(channelMode) ((channelMode) > BAPE_ChannelMode_e2_0?true:false)

/***************************************************************************
Summary:
Helper to determine appropriate decoder output mode
***************************************************************************/
BAPE_ChannelMode BAPE_DSP_P_GetChannelMode(BAVC_AudioCompressionStd codec, BAPE_ChannelMode outputMode, bool multichannelOutput, BAPE_MultichannelFormat maxFormat);

/***************************************************************************
Summary:
Helper to convert APE Channel Mode to DSP ACMOD
***************************************************************************/
uint32_t BAPE_DSP_P_ChannelModeToDsp(BAPE_ChannelMode outputMode);

/***************************************************************************
Summary:
Helper to setup channel matrix based on output mode and LFE
***************************************************************************/
void BAPE_DSP_P_GetChannelMatrix(BAPE_ChannelMode outputMode, bool lfe, uint32_t *pChannelMatrix);

/***************************************************************************
Summary:
Helper to setup true mono channel matrix, which is different from the
typical mono->stereo output from a DSP node.
***************************************************************************/
void BAPE_DSP_P_GetMonoChannelMatrix(uint32_t *pChannelMatrix);

/***************************************************************************
Summary:
Unit conversion routines
***************************************************************************/
uint32_t
BAPE_P_FloatToQ131(
    int32_t floatVal,
    unsigned int uiRange
    );

int32_t
BAPE_P_FloatToQ923(
    uint32_t floatVar,
    unsigned int uiRange
    );

uint32_t
BAPE_P_FloatToQ521(
    uint32_t floatVar,
    unsigned int uiRange
    );

uint32_t
    BAPE_P_FloatToQ815(
    uint32_t floatVar,
    unsigned int uiRange
    );

uint32_t
    BAPE_P_FloatToQ329(
    uint32_t floatVal,
    unsigned int uiRange
    );

uint32_t
BAPE_P_FloatToQ230(
    int16_t floatVar
    );

#if !B_REFSW_MINIMAL
int32_t
BAPE_P_FloatToQ824(
    int32_t floatVar,
    unsigned int uiRange
    );

int32_t
BAPE_P_FloatToQ1022(
    int32_t floatVar,
    unsigned int uiRange
    );

uint32_t
BAPE_P_FloatToQ518(
    uint32_t floatVar,
    unsigned int uiRange
    );

uint32_t
    BAPE_P_FloatToQ527(
    uint32_t floatVar,
    unsigned int uiRange
    );

uint32_t
    BAPE_P_FloatToQ428(
    uint32_t floatVal,
    unsigned int uiRange
    );

#endif

/***************************************************************************
Summary:
Add FMM Buffer Output to a stage
***************************************************************************/
BERR_Code BAPE_DSP_P_AddFmmBuffer(
    BAPE_PathConnection *pConnection
    );

/***************************************************************************
Summary:
Get Connector Data Type
***************************************************************************/
BDSP_DataType BAPE_DSP_P_GetDataTypeFromConnector(
    BAPE_Connector connector
    );


/***************************************************************************
Summary:
Init an FMM buffer descriptor from a DFIFO group
***************************************************************************/
BERR_Code BAPE_DSP_P_InitFmmInputDescriptor(
    BAPE_DfifoGroupHandle hDfifoGroup,
    BDSP_FmmBufferDescriptor *pDesc
    );

/***************************************************************************
Summary:
Build the path between a DSP node and one consumer
***************************************************************************/
BERR_Code BAPE_DSP_P_AllocatePathToOutput(
    BAPE_PathNode *pNode,
    BAPE_PathConnection *pConnection
    );

/***************************************************************************
Summary:
Configure the path between a DSP node and one consumer
***************************************************************************/
BERR_Code BAPE_DSP_P_ConfigPathToOutput(
    BAPE_PathNode *pNode,
    BAPE_PathConnection *pConnection
    );

/***************************************************************************
Summary:
Start the path from a DSP node to one consumer
***************************************************************************/
BERR_Code BAPE_DSP_P_StartPathToOutput(
    BAPE_PathNode *pNode,
    BAPE_PathConnection *pConnection
    );

/***************************************************************************
Summary:
Stop the path from a DSP node to one consumer
***************************************************************************/
void BAPE_DSP_P_StopPathToOutput(
    BAPE_PathNode *pNode,
    BAPE_PathConnection *pConnection
    );

/***************************************************************************
Summary:
Initialize task start settings based on other filter-graph members
***************************************************************************/
BERR_Code BAPE_DSP_P_DeriveTaskStartSettings(
    BAPE_PathNode *pNode,
    BDSP_TaskStartSettings *pStartSettings
    );

/***************************************************************************
Summary:
Converts DSP samplerate to unsigned
***************************************************************************/
unsigned BAPE_P_BDSPSampleFrequencyToInt( BDSP_AF_P_SampFreq bdspSF );

void BAPE_Decoder_P_GetAFDecoderType(
    BAPE_DecoderHandle handle,
    BDSP_AF_P_DecoderType *pType
    );

/***************************************************************************
Summary:
Get the Algorithm Specific Name
***************************************************************************/
const char * BAPE_P_GetAlgoSpecificName(
    BAPE_DecoderHandle handle
    );

#endif
