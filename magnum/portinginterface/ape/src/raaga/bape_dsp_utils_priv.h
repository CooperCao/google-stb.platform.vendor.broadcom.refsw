/***************************************************************************
 *     Copyright (c) 2006-2014, Broadcom Corporation
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
 * Module Description: DSP Utility Functions
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bape_types.h"
#include "bdsp.h"
#include "bdsp_task.h"
#include "bdsp_audio_task.h"

#ifndef BAPE_DSP_UTILS_PRIV_H_
#define BAPE_DSP_UTILS_PRIV_H_

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

#define BAPE_P_GetCodecName(codec) (BAPE_P_GetCodecAttributes_isrsafe((codec))->pName)
#define BAPE_P_GetCodecAudioDecode(codec) (BAPE_P_GetCodecAttributes_isrsafe((codec))->decodeAlgorithm)
#define BAPE_P_GetCodecAudioPassthrough(codec) (BAPE_P_GetCodecAttributes_isrsafe((codec))->passthroughAlgorithm)
#define BAPE_P_GetCodecAudioEncode(codec) (BAPE_P_GetCodecAttributes_isrsafe((codec))->encodeAlgorithm)
#define BAPE_P_GetCodecMixer(void) (BAPE_P_GetCodecMixer_isrsafe())
#define BAPE_P_GetCodecMultichannelFormat(codec) (BAPE_P_GetCodecAttributes_isrsafe((codec))->multichannelFormat)
#define BAPE_P_GetDolbyMSVersion(void) (BAPE_P_GetDolbyMSVersion_isrsafe())
#define BAPE_P_GetDolbyMS12Config(void) (BAPE_P_GetDolbyMS12Config_isrsafe())
#define BAPE_P_CodecSupportsPassthrough(codec) (BAPE_P_GetCodecAttributes_isrsafe((codec))->compressedOutputValid)
#define BAPE_P_CodecRequiresSrc(codec) (BAPE_P_GetCodecAttributes_isrsafe((codec))->srcRequired)
#define BAPE_P_CodecRequiresGenericPassthru(codec) (BAPE_P_GetCodecAttributes_isrsafe((codec))->genericPassthruRequired)
#define BAPE_P_CodecSupportsSimulMode(codec) (BAPE_P_GetCodecAttributes_isrsafe((codec))->compressedOutputValid && BAPE_P_GetCodecAttributes_isrsafe((codec))->simulValid)
#define BAPE_P_CodecSupportsMono(codec) (BAPE_P_GetCodecAttributes_isrsafe((codec))->monoOutputValid)
#define BAPE_P_CodecSupportsCompressed4x(codec) (BAPE_P_GetCodecAttributes_isrsafe((codec))->compressed4xOutputValid)
#define BAPE_P_CodecSupportsCompressed16x(codec) (BAPE_P_GetCodecAttributes_isrsafe((codec))->compressed16xOutputValid)

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
#define BAPE_DSP_P_SET_VARIABLE(st,var,val) do { st.var = (val); BDBG_MSG(("%s: %s = %#x", __FUNCTION__, #var, (st.var))); } while (0)

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

uint32_t 
BAPE_P_FloatToQ230(
    int16_t floatVar
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
    BAPE_P_FloatToQ815(
    uint32_t floatVar,
    unsigned int uiRange
    );

uint32_t 
    BAPE_P_FloatToQ527(
    uint32_t floatVar,
    unsigned int uiRange
    );

uint32_t 
    BAPE_P_FloatToQ329(
    uint32_t floatVal,
    unsigned int uiRange
    );

uint32_t 
    BAPE_P_FloatToQ428(
    uint32_t floatVal,
    unsigned int uiRange
    );

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


#endif
