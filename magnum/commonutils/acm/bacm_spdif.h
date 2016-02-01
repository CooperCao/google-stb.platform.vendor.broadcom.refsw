/***************************************************************************
*     Copyright (c) 2003-2007, Broadcom Corporation
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

#ifndef BAVC_AUDIO_H__
#define BAVC_AUDIO_H__

#include "bavc.h"
#include "bacm_spdif_defs.h"

/***************************************************************************
Summary:
	An enum of the channel status clock accuracies.
**************************************************************************/
typedef enum BACM_SPDIF_Application
{
	BACM_SPDIF_Application_eConsumer = BACM_SPDIF_CHAN_STAT0_APPLICATION_Consumer,
	BACM_SPDIF_Application_eProfessional = BACM_SPDIF_CHAN_STAT0_APPLICATION_Professional
} BACM_SPDIF_Application;

/***************************************************************************
Summary:
	This enumeration defines the stream type of incoming digaud data

***************************************************************************/
typedef enum BACM_SPDIF_StreamType
{
	BACM_SPDIF_StreamType_eLinear = BACM_SPDIF_CHAN_STAT0_STREAM_TYPE_Linear,
	BACM_SPDIF_StreamType_eNonlinear = BACM_SPDIF_CHAN_STAT0_STREAM_TYPE_Nonlinear
} BACM_SPDIF_StreamType;

/***************************************************************************
Summary:
	An enum of the channel status clock accuracies.
**************************************************************************/
typedef enum BACM_SPDIF_PCM_AdditionalFormatInfo
{
	BACM_SPDIF_PCM_AdditionalFormatInfo_2ChannelNoPreEmphasis = BACM_SPDIF_CHAN_STAT0_ADDED_FORMAT_INFO_PCM_2ChannelNoPreEmphasis,
	BACM_SPDIF_PCM_AdditionalFormatInfo_2ChannelPreEmphasis50us15us = BACM_SPDIF_CHAN_STAT0_ADDED_FORMAT_INFO_PCM_2ChannelPreEmphasis50us15us,
	BACM_SPDIF_PCM_AdditionalFormatInfo_2ChannelPreEmphasisReserved0 = BACM_SPDIF_CHAN_STAT0_ADDED_FORMAT_INFO_PCM_2ChannelPreEmphasisReserved0,
	BACM_SPDIF_PCM_AdditionalFormatInfo_2ChannelPreEmphasisReserved1 = BACM_SPDIF_CHAN_STAT0_ADDED_FORMAT_INFO_PCM_2ChannelPreEmphasisReserved1
} BACM_SPDIF_PCM_AdditionalFormatInfo;

/***************************************************************************
Summary:
	An enum of the channel status clock accuracies.
**************************************************************************/
typedef enum BACM_SPDIF_Nonlinear_AdditionalFormatInfo
{
	BACM_SPDIF_Nonlinear_AdditionalFormatInfo_Default = BACM_SPDIF_CHAN_STAT0_ADDED_FORMAT_INFO_NonLinear_Default
} BACM_SPDIF_Nonlinear_AdditionalFormatInfo;

/***************************************************************************
Summary:
	An enum of the channel status clock accuracies.
**************************************************************************/
typedef enum BACM_SPDIF_ChannelStatusMode
{
	BACM_SPDIF_ChannelStatusMode_e0 = BACM_SPDIF_CHAN_STAT0_MODE_Mode0
} BACM_SPDIF_ChannelStatusMode;

/***************************************************************************
Summary:
	An enum of the channel status clock accuracies.
**************************************************************************/
typedef enum BACM_SPDIF_SourceNumber
{
	BACM_SPDIF_SourceNumber_eUnspecified = BACM_SPDIF_CHAN_STAT2_SOURCE_NUM_Unspecified,
	BACM_SPDIF_SourceNumber_e1 = BACM_SPDIF_CHAN_STAT2_SOURCE_NUM_1,
	BACM_SPDIF_SourceNumber_e2 = BACM_SPDIF_CHAN_STAT2_SOURCE_NUM_2,
	BACM_SPDIF_SourceNumber_e3 = BACM_SPDIF_CHAN_STAT2_SOURCE_NUM_3,
	BACM_SPDIF_SourceNumber_e4 = BACM_SPDIF_CHAN_STAT2_SOURCE_NUM_4,
	BACM_SPDIF_SourceNumber_e5 = BACM_SPDIF_CHAN_STAT2_SOURCE_NUM_5,
	BACM_SPDIF_SourceNumber_e6 = BACM_SPDIF_CHAN_STAT2_SOURCE_NUM_6,
	BACM_SPDIF_SourceNumber_e7 = BACM_SPDIF_CHAN_STAT2_SOURCE_NUM_7,
	BACM_SPDIF_SourceNumber_e8 = BACM_SPDIF_CHAN_STAT2_SOURCE_NUM_8,
	BACM_SPDIF_SourceNumber_e9 = BACM_SPDIF_CHAN_STAT2_SOURCE_NUM_9,
	BACM_SPDIF_SourceNumber_e10 = BACM_SPDIF_CHAN_STAT2_SOURCE_NUM_10,
	BACM_SPDIF_SourceNumber_e11 = BACM_SPDIF_CHAN_STAT2_SOURCE_NUM_11,
	BACM_SPDIF_SourceNumber_e12 = BACM_SPDIF_CHAN_STAT2_SOURCE_NUM_12,
	BACM_SPDIF_SourceNumber_e13 = BACM_SPDIF_CHAN_STAT2_SOURCE_NUM_13,
	BACM_SPDIF_SourceNumber_e14 = BACM_SPDIF_CHAN_STAT2_SOURCE_NUM_14,
	BACM_SPDIF_SourceNumber_e15 = BACM_SPDIF_CHAN_STAT2_SOURCE_NUM_15
} BACM_SPDIF_SourceNumber;

/***************************************************************************
Summary:
	An enum of the channel status clock accuracies.
**************************************************************************/
typedef enum BACM_SPDIF_ChannelNumber
{
	BACM_SPDIF_ChannelNumber_eUnspecified = BACM_SPDIF_CHAN_STAT2_CHAN_NUM_Unspecified,
	BACM_SPDIF_ChannelNumber_e1 = BACM_SPDIF_CHAN_STAT2_CHAN_NUM_1,
	BACM_SPDIF_ChannelNumber_e2 = BACM_SPDIF_CHAN_STAT2_CHAN_NUM_2,
	BACM_SPDIF_ChannelNumber_e3 = BACM_SPDIF_CHAN_STAT2_CHAN_NUM_3,
	BACM_SPDIF_ChannelNumber_e4 = BACM_SPDIF_CHAN_STAT2_CHAN_NUM_4,
	BACM_SPDIF_ChannelNumber_e5 = BACM_SPDIF_CHAN_STAT2_CHAN_NUM_5,
	BACM_SPDIF_ChannelNumber_e6 = BACM_SPDIF_CHAN_STAT2_CHAN_NUM_6,
	BACM_SPDIF_ChannelNumber_e7 = BACM_SPDIF_CHAN_STAT2_CHAN_NUM_7,
	BACM_SPDIF_ChannelNumber_e8 = BACM_SPDIF_CHAN_STAT2_CHAN_NUM_8,
	BACM_SPDIF_ChannelNumber_e9 = BACM_SPDIF_CHAN_STAT2_CHAN_NUM_9,
	BACM_SPDIF_ChannelNumber_e10 = BACM_SPDIF_CHAN_STAT2_CHAN_NUM_10,
	BACM_SPDIF_ChannelNumber_e11 = BACM_SPDIF_CHAN_STAT2_CHAN_NUM_11,
	BACM_SPDIF_ChannelNumber_e12 = BACM_SPDIF_CHAN_STAT2_CHAN_NUM_12,
	BACM_SPDIF_ChannelNumber_e13 = BACM_SPDIF_CHAN_STAT2_CHAN_NUM_13,
	BACM_SPDIF_ChannelNumber_e14 = BACM_SPDIF_CHAN_STAT2_CHAN_NUM_14,
	BACM_SPDIF_ChannelNumber_e15 = BACM_SPDIF_CHAN_STAT2_CHAN_NUM_15
} BACM_SPDIF_ChannelNumber;

/***************************************************************************
Summary:
	An enum of the channel status clock accuracies.
**************************************************************************/
typedef enum BACM_SPDIF_SamplingFrequency
{
	BACM_SPDIF_SamplingFrequency_e44100 = BACM_SPDIF_CHAN_STAT3_FS_44100,
	BACM_SPDIF_SamplingFrequency_eUnspecified = BACM_SPDIF_CHAN_STAT3_FS_Unspecified,
	BACM_SPDIF_SamplingFrequency_e48000 = BACM_SPDIF_CHAN_STAT3_FS_48000,
	BACM_SPDIF_SamplingFrequency_e32000 = BACM_SPDIF_CHAN_STAT3_FS_32000,
	BACM_SPDIF_SamplingFrequency_e22050 = BACM_SPDIF_CHAN_STAT3_FS_22050,
	BACM_SPDIF_SamplingFrequency_e24000 = BACM_SPDIF_CHAN_STAT3_FS_24000,
	BACM_SPDIF_SamplingFrequency_e88200 = BACM_SPDIF_CHAN_STAT3_FS_88200,
	BACM_SPDIF_SamplingFrequency_e96000 = BACM_SPDIF_CHAN_STAT3_FS_96000,
	BACM_SPDIF_SamplingFrequency_e176400 = BACM_SPDIF_CHAN_STAT3_FS_176400,
	BACM_SPDIF_SamplingFrequency_e192000 = BACM_SPDIF_CHAN_STAT3_FS_192000
} BACM_SPDIF_SamplingFrequency;

/***************************************************************************
Summary:
	An enum of the channel status clock accuracies.
**************************************************************************/
typedef enum BACM_SPDIF_ClockAccuracy
{
	BACM_SPDIF_ClockAccuracy_eLevel2 = BACM_SPDIF_CHAN_STAT3_CLK_ACCURACY_LevelII,
	BACM_SPDIF_ClockAccuracy_eLevel1 = BACM_SPDIF_CHAN_STAT3_CLK_ACCURACY_LevelI,
	BACM_SPDIF_ClockAccuracy_eLevel3 = BACM_SPDIF_CHAN_STAT3_CLK_ACCURACY_LevelIII,
	BACM_SPDIF_ClockAccuracy_eUnmatched = BACM_SPDIF_CHAN_STAT3_CLK_ACCURACY_Unmatched
} BACM_SPDIF_ClockAccuracy;

/***************************************************************************
Summary:
	An enum of the channel status clock accuracies.
**************************************************************************/
typedef enum BACM_SPDIF_WordLength
{
	BACM_SPDIF_WordLength_eUnspecifiedOf20 = BACM_SPDIF_CHAN_STAT4_WORD_LEN_UNSPECIFIED_OF_20_BITS,
	BACM_SPDIF_WordLength_e16Of20 = BACM_SPDIF_CHAN_STAT4_WORD_LEN_16_OF_20_BITS,
	BACM_SPDIF_WordLength_e17Of20 = BACM_SPDIF_CHAN_STAT4_WORD_LEN_17_OF_20_BITS,
	BACM_SPDIF_WordLength_e18Of20 = BACM_SPDIF_CHAN_STAT4_WORD_LEN_18_OF_20_BITS,
	BACM_SPDIF_WordLength_e19Of20 = BACM_SPDIF_CHAN_STAT4_WORD_LEN_19_OF_20_BITS,
	BACM_SPDIF_WordLength_e20Of20 = BACM_SPDIF_CHAN_STAT4_WORD_LEN_20_OF_20_BITS,
	BACM_SPDIF_WordLength_eUnspecifiedOf24 = BACM_SPDIF_CHAN_STAT4_WORD_LEN_UNSPECIFIED_OF_24_BITS,
	BACM_SPDIF_WordLength_e20Of24 = BACM_SPDIF_CHAN_STAT4_WORD_LEN_20_OF_24_BITS,
	BACM_SPDIF_WordLength_e21Of24 = BACM_SPDIF_CHAN_STAT4_WORD_LEN_21_OF_24_BITS,
	BACM_SPDIF_WordLength_e22Of24 = BACM_SPDIF_CHAN_STAT4_WORD_LEN_22_OF_24_BITS,
	BACM_SPDIF_WordLength_e23Of24 = BACM_SPDIF_CHAN_STAT4_WORD_LEN_23_OF_24_BITS,
	BACM_SPDIF_WordLength_e24Of24 = BACM_SPDIF_CHAN_STAT4_WORD_LEN_24_OF_24_BITS
} BACM_SPDIF_WordLength;

/***************************************************************************
Summary:
	An enum of the channel status clock accuracies.
**************************************************************************/
typedef enum BACM_SPDIF_OriginalSamplingFrequency
{
	BACM_SPDIF_OriginalSamplingFrequency_eUnspecified = BACM_SPDIF_CHAN_STAT4_ORIG_FS_Unspecified,
	BACM_SPDIF_OriginalSamplingFrequency_e192000 = BACM_SPDIF_CHAN_STAT4_ORIG_FS_192000,
	BACM_SPDIF_OriginalSamplingFrequency_e12000 = BACM_SPDIF_CHAN_STAT4_ORIG_FS_12000,
	BACM_SPDIF_OriginalSamplingFrequency_e176400 = BACM_SPDIF_CHAN_STAT4_ORIG_FS_176400,
	BACM_SPDIF_OriginalSamplingFrequency_eReserved0 = BACM_SPDIF_CHAN_STAT4_ORIG_FS_Reserved0,
	BACM_SPDIF_OriginalSamplingFrequency_e96000 = BACM_SPDIF_CHAN_STAT4_ORIG_FS_96000,
	BACM_SPDIF_OriginalSamplingFrequency_e8000 = BACM_SPDIF_CHAN_STAT4_ORIG_FS_8000,
	BACM_SPDIF_OriginalSamplingFrequency_e88200 = BACM_SPDIF_CHAN_STAT4_ORIG_FS_88200,
	BACM_SPDIF_OriginalSamplingFrequency_e16000 = BACM_SPDIF_CHAN_STAT4_ORIG_FS_16000,
	BACM_SPDIF_OriginalSamplingFrequency_e24000 = BACM_SPDIF_CHAN_STAT4_ORIG_FS_24000,
	BACM_SPDIF_OriginalSamplingFrequency_e11025 = BACM_SPDIF_CHAN_STAT4_ORIG_FS_11025,
	BACM_SPDIF_OriginalSamplingFrequency_e22050 = BACM_SPDIF_CHAN_STAT4_ORIG_FS_22050,
	BACM_SPDIF_OriginalSamplingFrequency_e32000 = BACM_SPDIF_CHAN_STAT4_ORIG_FS_32000,
	BACM_SPDIF_OriginalSamplingFrequency_e48000 = BACM_SPDIF_CHAN_STAT4_ORIG_FS_48000,
	BACM_SPDIF_OriginalSamplingFrequency_eReserved1 = BACM_SPDIF_CHAN_STAT4_ORIG_FS_Reserved1,
	BACM_SPDIF_OriginalSamplingFrequency_e44100 = BACM_SPDIF_CHAN_STAT4_ORIG_FS_44100
} BACM_SPDIF_OriginalSamplingFrequency;

/***************************************************************************
Summary:
	The SPDIF channel status.  See IEC60958 for more information.

**************************************************************************/
typedef struct BACM_SPDIF_ChannelStatus
{
	BACM_SPDIF_Application eApplication;
	BACM_SPDIF_StreamType eStreamType;
	bool bCopyrighted;
	union BACM_SPDIF_AdditionalFormatInfo
	{
		BACM_SPDIF_PCM_AdditionalFormatInfo ePCM;
		BACM_SPDIF_Nonlinear_AdditionalFormatInfo eNonlinear;
	} uAdditionalFormatInfo;
	BACM_SPDIF_ChannelStatusMode eChannelStatusMode;
	uint8_t uiCategoryCode;
	BACM_SPDIF_SourceNumber eSourceNumber;
	BACM_SPDIF_ChannelNumber eChannelNumber;
	BACM_SPDIF_SamplingFrequency eSamplingFrequency;
	BACM_SPDIF_ClockAccuracy eClockAccuracy;
	BACM_SPDIF_WordLength eWordLength;
	BACM_SPDIF_OriginalSamplingFrequency eOriginalSamplingFrequency;
} BACM_SPDIF_ChannelStatus;

#define BACM_SPDIF_ChannelStatusMaxByteCount	24 /* 192 bits */

/***************************************************************************
Summary:
Takes the channel status byte array and converts it to a struct
**************************************************************************/
void BACM_SPDIF_InitChannelStatus_isrsafe(BACM_SPDIF_ChannelStatus * psChannelStatus);

/***************************************************************************
Summary:
Takes the channel status byte array and converts it to a struct
**************************************************************************/
void BACM_SPDIF_ParseChannelStatus_isrsafe(BACM_SPDIF_ChannelStatus * psChannelStatus, uint8_t uiChannelStatusSize, uint8_t auiChannelStatus[]);

/***************************************************************************
Summary:
	This enumeration defines the data type of SPDIF Preamble C.

***************************************************************************/
typedef enum BACM_SPDIF_DataType
{
	BACM_SPDIF_DataType_eNULL = BACM_SPDIF_PC_DATA_TYPE_Null,
	BACM_SPDIF_DataType_eAC3 = BACM_SPDIF_PC_DATA_TYPE_AC3,
	BACM_SPDIF_DataType_eReserved2 = BACM_SPDIF_PC_DATA_TYPE_Reserved2,
	BACM_SPDIF_DataType_ePause = BACM_SPDIF_PC_DATA_TYPE_Pause,
	BACM_SPDIF_DataType_eMpeg1L1 = BACM_SPDIF_PC_DATA_TYPE_Mpeg1L1,
	BACM_SPDIF_DataType_eMpeg1L23Mpeg2noext = BACM_SPDIF_PC_DATA_TYPE_Mpeg1L23Mpeg2noext,
	BACM_SPDIF_DataType_eMpeg2withext = BACM_SPDIF_PC_DATA_TYPE_Mpeg2withext,
	BACM_SPDIF_DataType_eAAC = BACM_SPDIF_PC_DATA_TYPE_AAC,
	BACM_SPDIF_DataType_eMpeg2L1LF = BACM_SPDIF_PC_DATA_TYPE_Mpeg2L1LF,
	BACM_SPDIF_DataType_eMpeg2L2LF = BACM_SPDIF_PC_DATA_TYPE_Mpeg2L2LF,
	BACM_SPDIF_DataType_eMpeg2L3LF = BACM_SPDIF_PC_DATA_TYPE_Mpeg2L3LF,
	BACM_SPDIF_DataType_eDTS1 = BACM_SPDIF_PC_DATA_TYPE_DTS1,
	BACM_SPDIF_DataType_eDTS2 = BACM_SPDIF_PC_DATA_TYPE_DTS2,
	BACM_SPDIF_DataType_eDTS3 = BACM_SPDIF_PC_DATA_TYPE_DTS3,
	BACM_SPDIF_DataType_eATRAC = BACM_SPDIF_PC_DATA_TYPE_ATRAC,
	BACM_SPDIF_DataType_eATRAC23 = BACM_SPDIF_PC_DATA_TYPE_ATRAC23,
	BACM_SPDIF_DataType_eExtended = BACM_SPDIF_PC_DATA_TYPE_Extended
} BACM_SPDIF_DataType;

/***************************************************************************
Summary:
	This structure defines the Burst Preamble C.

***************************************************************************/
typedef struct BACM_SPDIF_PreambleC
{
	bool bPayloadMayContainErrors;			/* true if the payload may contain errors, false otherwise */
	uint8_t uiBitstreamNumber;				/* the bitstream number, 0 - 7 */
	uint8_t uiDataTypeDependentInfo;			/* info that depends on the data type */
	BACM_SPDIF_DataType eDataType;			/* input data type */
} BACM_SPDIF_PreambleC;

/***************************************************************************
Summary:
Takes the channel status byte array and converts it to a struct
**************************************************************************/
#define BACM_SPDIF_ParsePreambleC_isr BACM_SPDIF_ParsePreambleC
void BACM_SPDIF_ParsePreambleC(BACM_SPDIF_PreambleC * psPreambleC, uint16_t uiPreambleC);

#endif /* BACM_SPDIF_COMMON_H__ */

