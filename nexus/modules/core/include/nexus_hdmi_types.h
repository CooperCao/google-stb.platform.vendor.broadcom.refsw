/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 *****************************************************************************/
#ifndef NEXUS_HDMI_TYPES_H__
#define NEXUS_HDMI_TYPES_H__

#include "nexus_types.h"
#include "nexus_video_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
Summary:
Enumerated Type for Vendor-Specific Infoframe HDMI Video Format
******************************************************************************/
typedef enum NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat
{
    NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat_eNone,
    NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat_eExtendedResolution,
    NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat_e3DFormat,
    NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat_eMax
} NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat;


/******************************************************************************
Summary:
Enumerated Type for Vendor-Specific Infoframe HDMI VIC
******************************************************************************/
typedef enum NEXUS_HdmiVendorSpecificInfoFrame_HDMIVIC
{
    NEXUS_HdmiVendorSpecificInfoFrame_HDMIVIC_eReserved,
    NEXUS_HdmiVendorSpecificInfoFrame_HDMIVIC_e4Kx2K_2997_30Hz,
    NEXUS_HdmiVendorSpecificInfoFrame_HDMIVIC_e4Kx2K_25Hz,
    NEXUS_HdmiVendorSpecificInfoFrame_HDMIVIC_e4Kx2K_2398_24Hz,
    NEXUS_HdmiVendorSpecificInfoFrame_HDMIVIC_e4Kx2K_SMPTE_24Hz,
    NEXUS_HdmiVendorSpecificInfoFrame_HDMIVIC_eMax
} NEXUS_HdmiVendorSpecificInfoFrame_HDMIVIC;


/******************************************************************************
Summary:
Enumerated Type for Vendor-Specific Infoframe 3D Structure
******************************************************************************/
#define NEXUS_HdmiVendorSpecificInfoFrame_3DStructure                   NEXUS_Video3DStructure
#define NEXUS_HdmiVendorSpecificInfoFrame_3DStructure_eFramePacking     NEXUS_Video3DStructure_eFramePacking
#define NEXUS_HdmiVendorSpecificInfoFrame_3DStructure_eFieldAlternative NEXUS_Video3DStructure_eFieldAlternative
#define NEXUS_HdmiVendorSpecificInfoFrame_3DStructure_eLineAlternative  NEXUS_Video3DStructure_eLineAlternative
#define NEXUS_HdmiVendorSpecificInfoFrame_3DStructure_eSidexSideFull    NEXUS_Video3DStructure_eSidexSideFull
#define NEXUS_HdmiVendorSpecificInfoFrame_3DStructure_eLDepth           NEXUS_Video3DStructure_eLDepth
#define NEXUS_HdmiVendorSpecificInfoFrame_3DStructure_eLDepthGraphics   NEXUS_Video3DStructure_eLDepthGraphics
#define NEXUS_HdmiVendorSpecificInfoFrame_3DStructure_eTopAndBottom     NEXUS_Video3DStructure_eTopAndBottom
#define NEXUS_HdmiVendorSpecificInfoFrame_3DStructure_eReserved         NEXUS_Video3DStructure_eReserved
#define NEXUS_HdmiVendorSpecificInfoFrame_3DStructure_eSidexSideHalf    NEXUS_Video3DStructure_eSidexSideHalf
#define NEXUS_HdmiVendorSpecificInfoFrame_3DStructure_eMax              NEXUS_Video3DStructure_eMax


/******************************************************************************
Summary:
Enumerated Type for Vendor-Specific Infoframe 3D Ext Data
******************************************************************************/
#define NEXUS_HdmiVendorSpecificInfoFrame_3DExtData                     NEXUS_Video3DSubSample
#define NEXUS_HdmiVendorSpecificInfoFrame_3DExtData_eNone               NEXUS_Video3DSubSample_eNone
#define NEXUS_HdmiVendorSpecificInfoFrame_3DExtData_eHorzOLOR           NEXUS_Video3DSubSample_eHorzOLOR
#define NEXUS_HdmiVendorSpecificInfoFrame_3DExtData_eHorzOLER           NEXUS_Video3DSubSample_eHorzOLER
#define NEXUS_HdmiVendorSpecificInfoFrame_3DExtData_eHorzELOR           NEXUS_Video3DSubSample_eHorzELOR
#define NEXUS_HdmiVendorSpecificInfoFrame_3DExtData_eHorzELER           NEXUS_Video3DSubSample_eHorzELER
#define NEXUS_HdmiVendorSpecificInfoFrame_3DExtData_eQuinOLOR           NEXUS_Video3DSubSample_eQuinOLOR
#define NEXUS_HdmiVendorSpecificInfoFrame_3DExtData_eQuinOLER           NEXUS_Video3DSubSample_eQuinOLER
#define NEXUS_HdmiVendorSpecificInfoFrame_3DExtData_eQuinELOR           NEXUS_Video3DSubSample_eQuinELOR
#define NEXUS_HdmiVendorSpecificInfoFrame_3DExtData_eQuinELER           NEXUS_Video3DSubSample_eQuinELER
#define NEXUS_HdmiVendorSpecificInfoFrame_3DExtData_eMax                NEXUS_Video3DSubSample_eMax


/******************************************************************************
Summary:
defines for HDMI HDCP
******************************************************************************/
#define NEXUS_HDMI_HDCP_KSV_LENGTH 5
#define NEXUS_HDMI_HDCP_AN_LENGTH 8
#define NEXUS_HDMI_HDCP_NUM_KEYS 40

/******************************************************************************
Summary:
Enumerated Type of the type of Colorspace (Pixel Encoding) in HDMI AVI Infoframe Packets
*******************************************************************************/
typedef enum NEXUS_HdmiAviInfoFrame_Colorspace
{
    NEXUS_HdmiAviInfoFrame_Colorspace_eRGB = 0,
    NEXUS_HdmiAviInfoFrame_Colorspace_eYCbCr422,
    NEXUS_HdmiAviInfoFrame_Colorspace_eYCbCr444,
    NEXUS_HdmiAviInfoFrame_Colorspace_eYCbCr420,
    NEXUS_HdmiAviInfoFrame_Colorspace_eMax
} NEXUS_HdmiAviInfoFrame_Colorspace;

/******************************************************************************
Summary:
Enumerated Type of the Active Info bit (valid or invalid) for HDMI AVI InfoFrame
*******************************************************************************/
typedef enum NEXUS_HdmiAviInfoFrame_ActiveInfo
{
    NEXUS_HdmiAviInfoFrame_ActiveInfo_eInvalid = 0,
    NEXUS_HdmiAviInfoFrame_ActiveInfo_eValid
} NEXUS_HdmiAviInfoFrame_ActiveInfo;


/******************************************************************************
Summary:
Enumerated Type for information on Horizonatal and Vertical Bar for HDMI AVI InfoFrame
*******************************************************************************/
typedef enum NEXUS_HdmiAviInfoFrame_BarInfo
{
    NEXUS_HdmiAviInfoFrame_BarInfo_eInvalid,
    NEXUS_HdmiAviInfoFrame_BarInfo_eVerticalValid,
    NEXUS_HdmiAviInfoFrame_BarInfo_eHorizValid,
    NEXUS_HdmiAviInfoFrame_BarInfo_eVertHorizValid
} NEXUS_HdmiAviInfoFrame_BarInfo;


/******************************************************************************
Summary:
Enumerated Type for information on Scan Information for HDMI AVI InfoFrame
*******************************************************************************/
typedef enum NEXUS_HdmiAviInfoFrame_ScanInfo
{
    NEXUS_HdmiAviInfoFrame_ScanInfo_eNoData,
    NEXUS_HdmiAviInfoFrame_ScanInfo_eOverScanned,
    NEXUS_HdmiAviInfoFrame_ScanInfo_eUnderScanned
} NEXUS_HdmiAviInfoFrame_ScanInfo;


/******************************************************************************
Summary:
Enumerated Type of the Colorimetry for HDMI AVI InfoFrame
*******************************************************************************/
typedef enum NEXUS_HdmiAviInfoFrame_Colorimetry
{
    NEXUS_HdmiAviInfoFrame_Colorimetry_eNoData =  0,
    NEXUS_HdmiAviInfoFrame_Colorimetry_eSmpte170,
    NEXUS_HdmiAviInfoFrame_Colorimetry_eItu709,
    NEXUS_HdmiAviInfoFrame_Colorimetry_eFuture,
    NEXUS_HdmiAviInfoFrame_Colorimetry_eExtended = NEXUS_HdmiAviInfoFrame_Colorimetry_eFuture
} NEXUS_HdmiAviInfoFrame_Colorimetry;


/******************************************************************************
Summary:
Enumerated Type of the Picture Aspect Ratio for HDMI AVI InfoFrame
*******************************************************************************/
typedef enum NEXUS_HdmiAviInfoFrame_PictureAspectRatio
{
    NEXUS_HdmiAviInfoFrame_PictureAspectRatio_eNoData = 0,
    NEXUS_HdmiAviInfoFrame_PictureAspectRatio_e4_3,
    NEXUS_HdmiAviInfoFrame_PictureAspectRatio_e16_9,
    NEXUS_HdmiAviInfoFrame_PictureAspectRatio_eFuture
} NEXUS_HdmiAviInfoFrame_PictureAspectRatio;


/******************************************************************************
Summary:
Enumerated Type of the Active Format Aspect Ratio for HDMI AVI InfoFrame
*******************************************************************************/
typedef enum NEXUS_HdmiAviInfoFrame_ActiveFormatAspectRatio
{
    NEXUS_HdmiAviInfoFrame_ActiveFormatAspectRatio_ePicture    =  8,
    NEXUS_HdmiAviInfoFrame_ActiveFormatAspectRatio_e4_3Center  =  9,
    NEXUS_HdmiAviInfoFrame_ActiveFormatAspectRatio_e16_9Center = 10,
    NEXUS_HdmiAviInfoFrame_ActiveFormatAspectRatio_e14_9Center = 11,

    NEXUS_HdmiAviInfoFrame_ActiveFormatAspectRatio_e4_3_Alt14_9Center = 13,
    NEXUS_HdmiAviInfoFrame_ActiveFormatAspectRatio_e16_9_Alt14_9Center = 14,
    NEXUS_HdmiAviInfoFrame_ActiveFormatAspectRatio_e16_9_Alt4_3Center = 15,

    NEXUS_HdmiAviInfoFrame_ActiveFormatAspectRatio_eOther      =  0
} NEXUS_HdmiAviInfoFrame_ActiveFormatAspectRatio;


/******************************************************************************
Summary:
Enumerated Type of the Picture Scaling for HDMI AVI InfoFrame
*******************************************************************************/
typedef enum NEXUS_HdmiAviInfoFrame_Scaling
{
    NEXUS_HdmiAviInfoFrame_Scaling_eNoScaling = 0,
    NEXUS_HdmiAviInfoFrame_Scaling_eHScaling,
    NEXUS_HdmiAviInfoFrame_Scaling_eVScaling,
    NEXUS_HdmiAviInfoFrame_Scaling_eHVScaling
} NEXUS_HdmiAviInfoFrame_Scaling;


/******************************************************************************
Summary:
Enumerated Type for Extended Colorimetry (IED 61966-2-4) for HDMI AVI InfoFrame
*******************************************************************************/
typedef enum NEXUS_HdmiAviInfoFrame_ExtendedColorimetry
{
    NEXUS_HdmiAviInfoFrame_ExtendedColorimetry_exvYCC601 = 0,
    NEXUS_HdmiAviInfoFrame_ExtendedColorimetry_exvYCC709,
    NEXUS_HdmiAviInfoFrame_ExtendedColorimetry_esYCC601,
    NEXUS_HdmiAviInfoFrame_ExtendedColorimetry_eAdobeYCC601,
    NEXUS_HdmiAviInfoFrame_ExtendedColorimetry_eAdobeRGB,
    NEXUS_HdmiAviInfoFrame_ExtendedColorimetry_eItuRBt2020YcCbcCrc,
    NEXUS_HdmiAviInfoFrame_ExtendedColorimetry_eItuRBt2020RGB,
    NEXUS_HdmiAviInfoFrame_ExtendedColorimetry_eItuRBt2020YCbCr = NEXUS_HdmiAviInfoFrame_ExtendedColorimetry_eItuRBt2020RGB,
    NEXUS_HdmiAviInfoFrame_ExtendedColorimetry_eReserved,
    NEXUS_HdmiAviInfoFrame_ExtendedColorimetry_eMax
} NEXUS_HdmiAviInfoFrame_ExtendedColorimetry;




/******************************************************************************
Summary:
Enumerated Type for Quantization Range  for HDMI AVI InfoFrame
*******************************************************************************/
typedef enum NEXUS_HdmiAviInfoFrame_RGBQuantizationRange
{
    NEXUS_HdmiAviInfoFrame_RGBQuantizationRange_eDefault    =  0,
    NEXUS_HdmiAviInfoFrame_RGBQuantizationRange_eLimitedRange  =  1,
    NEXUS_HdmiAviInfoFrame_RGBQuantizationRange_eFullRange = 2,
    NEXUS_HdmiAviInfoFrame_RGBQuantizationRange_eReserved = 3
} NEXUS_HdmiAviInfoFrame_RGBQuantizationRange;


/******************************************************************************
Summary:
Enumerated Type for IT Content for HDMI AVI InfoFrame
*******************************************************************************/
typedef enum NEXUS_HdmiAviInfoFrame_ITContent
{
    NEXUS_HdmiAviInfoFrame_ITContent_eNoData    =  0,
    NEXUS_HdmiAviInfoFrame_ITContent_eITContent  =  1
} NEXUS_HdmiAviInfoFrame_ITContent;


/******************************************************************************
Summary:
Enumerated Type for IT Content Type for HDMI AVI InfoFrame
*******************************************************************************/
typedef enum NEXUS_HdmiAviInfoFrame_ContentType
{
    NEXUS_HdmiAviInfoFrame_ContentType_eGraphics = 0,
    NEXUS_HdmiAviInfoFrame_ContentType_ePhoto,
    NEXUS_HdmiAviInfoFrame_ContentType_eCinema,
    NEXUS_HdmiAviInfoFrame_ContentType_eGame,
    NEXUS_HdmiAviInfoFrame_ContentType_eMax
} NEXUS_HdmiAviInfoFrame_ContentType;


/******************************************************************************
Summary:
Enumerated Type for YCC Quantization for HDMI AVI InfoFrame
*******************************************************************************/
typedef enum NEXUS_HdmiAviInfoFrame_YccQuantizationRange
{
    NEXUS_HdmiAviInfoFrame_YccQuantizationRange_eLimited = 0,
    NEXUS_HdmiAviInfoFrame_YccQuantizationRange_eFull,
    NEXUS_HdmiAviInfoFrame_YccQuantizationRange_eReserved,
    NEXUS_HdmiAviInfoFrame_YccQuantizationRange_eMax
} NEXUS_HdmiAviInfoFrame_YccQuantizationRange;


/******************************************************************************
Summary:
Enumerated Type for Gamut Metadata Profile
*******************************************************************************/
typedef enum NEXUS_HdmiGamutPacket_MetadataProfile
{
    NEXUS_HdmiGamutPacket_MetadataProfile_eMD0,
    NEXUS_HdmiGamutPacket_MetadataProfile_eMD1,
    NEXUS_HdmiGamutPacket_MetadataProfile_eMD2,
    NEXUS_HdmiGamutPacket_MetadataProfile_eMD3,
    NEXUS_HdmiGamutPacket_MetadataProfile_eMax
} NEXUS_HdmiGamutPacket_MetadataProfile ;


#define NEXUS_HDMI_PACKET_DATA_LENGTH 28
/**************************************************************
Summary:
HDMI packet structure returned by NEXUS_HdmiInput_GetRawPacketData.

Description:
This structure is designed according to HDMI standard 1.3
**************************************************************/
typedef struct NEXUS_HdmiPacket
{
    uint8_t type;            /* packet type */
    uint8_t version;         /* packet version     */
    uint8_t length;          /* packet length      */
    uint8_t dataBytes[NEXUS_HDMI_PACKET_DATA_LENGTH]; /* packet data */
} NEXUS_HdmiPacket;


/**************************************************************
Summary:
Status of the received HDMI packet
**************************************************************/
typedef enum NEXUS_HdmiPacketStatus
{
    NEXUS_HdmiPacketStatus_eNotReceived = 0, /* initial state */
    NEXUS_HdmiPacketStatus_eStopped,
    NEXUS_HdmiPacketStatus_eUpdated,
    NEXUS_HdmiPacketStatus_eMax
} NEXUS_HdmiPacketStatus;


/******************************************************
Summary:
Auxiliary Video Information (AVI) Infoframe for HDMI output/input
*******************************************************/
typedef struct NEXUS_HdmiAviInfoFrame
{
    /* Generate internally for HDMI Tx platforms unless request to override */
    NEXUS_HdmiAviInfoFrame_Colorspace eColorSpace; /* [Y2]Y1Y0 */

    /*************************************
    * If set to true, all settings will be used to set HDMI AVI InfoFrame for hdmi output.
    * Otherwise, only partial of the settings will be applied. The following settings will be
    * generated: colorimetry, colorSpace, pictureAspectRatio, extendedColorimetry,
    * and videoIDCode
    *************************************/
    bool bOverrideDefaults;

    NEXUS_HdmiAviInfoFrame_ActiveInfo eActiveInfo; /* A0 */
    NEXUS_HdmiAviInfoFrame_BarInfo    eBarInfo;    /* B1B0 */
    NEXUS_HdmiAviInfoFrame_ScanInfo   eScanInfo;   /* S1S0 */

    /* Generate internally for HDMI Tx platforms unless request to override */
    NEXUS_HdmiAviInfoFrame_Colorimetry eColorimetry;  /* C1C0 */

    /* Generate internally for HDMI Tx platforms unless request to override */
    NEXUS_HdmiAviInfoFrame_PictureAspectRatio ePictureAspectRatio; /* M1M0 */

    NEXUS_HdmiAviInfoFrame_ActiveFormatAspectRatio eActiveFormatAspectRatio; /* R3R0 */
    NEXUS_HdmiAviInfoFrame_Scaling eScaling; /* SC1SC0 */
    uint8_t videoIdCode;

    /* Generate internally for HDMI Tx platforms unless request to override */
    uint8_t pixelRepeat;

    NEXUS_HdmiAviInfoFrame_ITContent eITContent; /*ITC */
    NEXUS_HdmiAviInfoFrame_ExtendedColorimetry eExtendedColorimetry; /* EC2EC1EC0 */
    NEXUS_HdmiAviInfoFrame_RGBQuantizationRange eRGBQuantizationRange; /* Q1Q0 */
    NEXUS_HdmiAviInfoFrame_ContentType eContentType; /* CN1CN0 */
    NEXUS_HdmiAviInfoFrame_YccQuantizationRange eYccQuantizationRange; /* YQ1YQ0 */

    /* bar info */
    uint16_t topBarEndLineNumber;
    uint16_t bottomBarStartLineNumber;

    uint16_t leftBarEndPixelNumber;
    uint16_t rightBarEndPixelNumber;

    /* These fields are only applicable/use for HDMI Rx platforms */
    NEXUS_HdmiPacketStatus packetStatus;
    NEXUS_HdmiPacket packet;
} NEXUS_HdmiAviInfoFrame;


/******************************************************
Summary:
Enumerated Type for Coding Type in HDMI Audio InfoFrame
*******************************************************/
typedef enum NEXUS_HdmiAudioInfoFrame_CodingType
{
    NEXUS_HdmiAudioInfoFrame_CodingType_eReferToStream,
    NEXUS_HdmiAudioInfoFrame_CodingType_ePCM,
    NEXUS_HdmiAudioInfoFrame_CodingType_eAC3,
    NEXUS_HdmiAudioInfoFrame_CodingType_eMPEG1,
    NEXUS_HdmiAudioInfoFrame_CodingType_eMP3,
    NEXUS_HdmiAudioInfoFrame_CodingType_eMPEG2,
    NEXUS_HdmiAudioInfoFrame_CodingType_eAACLC,
    NEXUS_HdmiAudioInfoFrame_CodingType_eDTS,
    NEXUS_HdmiAudioInfoFrame_CodingType_eATRAC,
    NEXUS_HdmiAudioInfoFrame_CodingType_eDSD,
    NEXUS_HdmiAudioInfoFrame_CodingType_eEAC3,
    NEXUS_HdmiAudioInfoFrame_CodingType_eDTSHD,
    NEXUS_HdmiAudioInfoFrame_CodingType_eMLP,
    NEXUS_HdmiAudioInfoFrame_CodingType_eDST,
    NEXUS_HdmiAudioInfoFrame_CodingType_eWMAPro,
    NEXUS_HdmiAudioInfoFrame_CodingType_eUnknown
} NEXUS_HdmiAudioInfoFrame_CodingType;


/******************************************************************************
Summary:
Enumerated Type for Channel Count in HDMI Audio InfoFrame
*******************************************************************************/
typedef enum NEXUS_HdmiAudioInfoFrame_ChannelCount
{
    NEXUS_HdmiAudioInfoFrame_ChannelCount_eReferToStreamHeader,
    NEXUS_HdmiAudioInfoFrame_ChannelCount_e2Channels,
    NEXUS_HdmiAudioInfoFrame_ChannelCount_e3Channels,
    NEXUS_HdmiAudioInfoFrame_ChannelCount_e4Channels,
    NEXUS_HdmiAudioInfoFrame_ChannelCount_e5Channels,
    NEXUS_HdmiAudioInfoFrame_ChannelCount_e6Channels,
    NEXUS_HdmiAudioInfoFrame_ChannelCount_e7Channels,
    NEXUS_HdmiAudioInfoFrame_ChannelCount_e8Channels
} NEXUS_HdmiAudioInfoFrame_ChannelCount;


/*******************************************************************************
Summary:
Enumerated Type for Sampling Frequency  in HDMI Audio InfoFrame
*******************************************************************************/
typedef enum NEXUS_HdmiAudioInfoFrame_SampleFrequency
{
    NEXUS_HdmiAudioInfoFrame_SampleFrequency_eReferToStreamHeader,
    NEXUS_HdmiAudioInfoFrame_SampleFrequency_e32,
    NEXUS_HdmiAudioInfoFrame_SampleFrequency_e44_1,
    NEXUS_HdmiAudioInfoFrame_SampleFrequency_e48,
    NEXUS_HdmiAudioInfoFrame_SampleFrequency_e88_2,
    NEXUS_HdmiAudioInfoFrame_SampleFrequency_e96,
    NEXUS_HdmiAudioInfoFrame_SampleFrequency_e176_4,
    NEXUS_HdmiAudioInfoFrame_SampleFrequency_e192
} NEXUS_HdmiAudioInfoFrame_SampleFrequency;


/******************************************************************************
Summary:
Enumerated Type for Sample Size in HDMI Audio InfoFrame
*******************************************************************************/
typedef enum NEXUS_HdmiAudioInfoFrame_SampleSize
{
    NEXUS_HdmiAudioInfoFrame_SampleSize_eReferToStreamHeader,
    NEXUS_HdmiAudioInfoFrame_SampleSize_e16,
    NEXUS_HdmiAudioInfoFrame_SampleSize_e20,
    NEXUS_HdmiAudioInfoFrame_SampleSize_e24
} NEXUS_HdmiAudioInfoFrame_SampleSize;


/******************************************************************************
Summary:
Enumerated Type for Level Shift Value in HDMI Audio InfoFrame
*******************************************************************************/
typedef enum NEXUS_HdmiAudioInfoFrame_LevelShift
{
    NEXUS_HdmiAudioInfoFrame_LevelShift_e0db,
    NEXUS_HdmiAudioInfoFrame_LevelShift_e1db,
    NEXUS_HdmiAudioInfoFrame_LevelShift_e2db,
    NEXUS_HdmiAudioInfoFrame_LevelShift_e3db,
    NEXUS_HdmiAudioInfoFrame_LevelShift_e4db,
    NEXUS_HdmiAudioInfoFrame_LevelShift_e5db,
    NEXUS_HdmiAudioInfoFrame_LevelShift_e6db,
    NEXUS_HdmiAudioInfoFrame_LevelShift_e7db,
    NEXUS_HdmiAudioInfoFrame_LevelShift_e8db,
    NEXUS_HdmiAudioInfoFrame_LevelShift_e9db,
    NEXUS_HdmiAudioInfoFrame_LevelShift_e10db,
    NEXUS_HdmiAudioInfoFrame_LevelShift_e11db,
    NEXUS_HdmiAudioInfoFrame_LevelShift_e12db,
    NEXUS_HdmiAudioInfoFrame_LevelShift_e13db,
    NEXUS_HdmiAudioInfoFrame_LevelShift_e14db,
    NEXUS_HdmiAudioInfoFrame_LevelShift_e15db
} NEXUS_HdmiAudioInfoFrame_LevelShift;


/******************************************************************************
Summary:
Enumerated Type for Down Mix in HDMI Audio InfoFrame
******************************************************************************/
typedef enum NEXUS_HdmiAudioInfoFrame_DownMixInhibit
{
    NEXUS_HdmiAudioInfoFrame_DownMixInhibit_ePermitted,
    NEXUS_HdmiAudioInfoFrame_DownMixInhibit_eProhibited
} NEXUS_HdmiAudioInfoFrame_DownMixInhibit;


/***************************************
Summary:
Audio Infoframe for HDMI input/output
****************************************/
typedef struct NEXUS_HdmiAudioInfoFrame
{
    /*************************************
    * If set to true, all settings will be used to set HDMI AudioInfoFrame for hdmi output.
    * Otherwise, only partial of the settings will be applied. The following settings will be
    * obtain from stream header: codingType, sampleSize and sampleFrequency
    *************************************/
    bool bOverrideDefaults;
    NEXUS_HdmiAudioInfoFrame_ChannelCount eChannelCount;
    NEXUS_HdmiAudioInfoFrame_CodingType eCodingType;
    NEXUS_HdmiAudioInfoFrame_SampleSize eSampleSize;
    NEXUS_HdmiAudioInfoFrame_SampleFrequency eSampleFrequency;
    uint8_t speakerAllocation;
    NEXUS_HdmiAudioInfoFrame_DownMixInhibit eDownMixInhibit;
    NEXUS_HdmiAudioInfoFrame_LevelShift eLevelShift;

    /* These fields are only applicable/use in HDMI Rx platforms */
    NEXUS_HdmiPacketStatus packetStatus;
    NEXUS_HdmiPacket packet;
} NEXUS_HdmiAudioInfoFrame;


#define NEXUS_HDMI_SPD_VENDOR_NAME_MAX 8
#define NEXUS_HDMI_SPD_DESCRIPTION_MAX 16

/************************************************************
Summary:
Enumeration for SPD (Source Product Description) Device Types
************************************************************/
typedef enum NEXUS_HdmiSpdSourceDeviceType
{
    NEXUS_HdmiSpdSourceDeviceType_eUnknown,
    NEXUS_HdmiSpdSourceDeviceType_eDigitalStb,
    NEXUS_HdmiSpdSourceDeviceType_eDvd,
    NEXUS_HdmiSpdSourceDeviceType_eDvhs,
    NEXUS_HdmiSpdSourceDeviceType_eHddVideo,
    NEXUS_HdmiSpdSourceDeviceType_eDvc,
    NEXUS_HdmiSpdSourceDeviceType_eDsc,
    NEXUS_HdmiSpdSourceDeviceType_eVideoCd,
    NEXUS_HdmiSpdSourceDeviceType_eGame,
    NEXUS_HdmiSpdSourceDeviceType_ePc,
    NEXUS_HdmiSpdSourceDeviceType_eBluRayDisc,
    NEXUS_HdmiSpdSourceDeviceType_eSuperAudioCd,
    NEXUS_HdmiSpdSourceDeviceType_eHdDvd,
    NEXUS_HdmiSpdSourceDeviceType_ePmp,
    NEXUS_HdmiSpdSourceDeviceType_eMax
} NEXUS_HdmiSpdSourceDeviceType;


/***************************************
Summary:
SPD (Source Product Description) Infoframe for HDMI input/output
****************************************/
typedef struct NEXUS_HdmiSpdInfoFrame
{
    uint8_t vendorName[NEXUS_HDMI_SPD_VENDOR_NAME_MAX+1];
    uint8_t description[NEXUS_HDMI_SPD_DESCRIPTION_MAX+1];
    NEXUS_HdmiSpdSourceDeviceType deviceType;

    /* These fields are only applicable/use for HDMI Rx platforms */
    NEXUS_HdmiPacketStatus packetStatus;
    NEXUS_HdmiPacket packet;
} NEXUS_HdmiSpdInfoFrame;


/***************************************
Summary:
Vendor Specific Infoframe for HDMI input/output
****************************************/
typedef struct NEXUS_HdmiVendorSpecificInfoFrame
{
    uint8_t ieeeRegId[3];
    NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat hdmiVideoFormat;
    NEXUS_HdmiVendorSpecificInfoFrame_HDMIVIC  hdmiVIC;
    NEXUS_HdmiVendorSpecificInfoFrame_3DStructure hdmi3DStructure;
    NEXUS_HdmiVendorSpecificInfoFrame_3DExtData hdmi3DExtData;

    /* These fields are only applicable/use in HDMI Rx platforms */
    NEXUS_HdmiPacketStatus packetStatus;
    NEXUS_HdmiPacket packet;
} NEXUS_HdmiVendorSpecificInfoFrame;

/**
Summary:
Static metadata types defined by CEA861 for high dynamic range output modes
**/
typedef enum NEXUS_HdmiDynamicRangeMasteringStaticMetadataType
{
    NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_e1,
    NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_eMax
} NEXUS_HdmiDynamicRangeMasteringStaticMetadataType;

#define NEXUS_HdmiType1DynamicRangeMasteringStaticMetadata NEXUS_StaticHdrMetadata

/**
Summary:
Static metadata defined by CEA861 for high dynamic range output modes
**/
typedef struct NEXUS_HdmiDynamicRangeMasteringStaticMetadata
{
    NEXUS_HdmiDynamicRangeMasteringStaticMetadataType type; /* the type of this metadata */
    struct
    {
        NEXUS_StaticHdrMetadata type1; /* type "1" metadata */
    } typeSettings; /* type-specific metadata settings */
} NEXUS_HdmiDynamicRangeMasteringStaticMetadata;

/**
Summary:
Dynamic Range and Mastering InfoFrame as defined by CEA861
**/
typedef struct NEXUS_HdmiDynamicRangeMasteringInfoFrame
{
    NEXUS_VideoEotf eotf; /* eotf of HDMI transmitted video */
    NEXUS_HdmiDynamicRangeMasteringStaticMetadata metadata; /* static metadata of HDMI transmitted video */
} NEXUS_HdmiDynamicRangeMasteringInfoFrame;

/**
Summary:
Structure for an HDCP KSV key
**/
typedef struct NEXUS_HdmiHdcpKsv
{
    uint8_t data[NEXUS_HDMI_HDCP_KSV_LENGTH] ;
} NEXUS_HdmiHdcpKsv ;

typedef struct NEXUS_HdmiHdcpDownStreamInfo
{
    uint8_t depth;               /* number of levels of devices */
    bool maxDepthExceeded;

    uint8_t devices;             /* number of devices  */
    bool maxDevicesExceeded;

    bool isRepeater;
    NEXUS_HdmiHdcpKsv repeaterKsv;
} NEXUS_HdmiHdcpDownStreamInfo;


/**
Summary:
Structure represent HDCP2.x receiver Id List Data.
**/
typedef struct NEXUS_Hdcp2xReceiverIdListData
{
    unsigned deviceCount;
    unsigned depth;
    bool maxDevsExceeded;
    bool maxCascadeExceeded;
    bool hdcp2xLegacyDeviceDownstream;
    bool hdcp1DeviceDownstream;
    uint8_t rxIdList[155];
} NEXUS_Hdcp2xReceiverIdListData;


/**
Summary:
HDCP Version
**/
typedef enum NEXUS_HdcpVersion
{
    NEXUS_HdcpVersion_e1x,
    NEXUS_HdcpVersion_e2x,
    NEXUS_HdcpVersion_eMax
} NEXUS_HdcpVersion;


/******************************************************************************
Summary:
    Enum for the Hdcp2.x Content Stream Type

Description:
    Type 0 - Content might be transmitted by HDCP Repeaters to all HDCP Devices
    Type 1 - Content must not be transmitted by HDCP Repeater to HDCP 1.x Devices.

*******************************************************************************/
typedef enum NEXUS_Hdcp2xContentStream
{
    NEXUS_Hdcp2xContentStream_eType0, /* Content may be transmitted to all HDCP downstream devices */
    NEXUS_Hdcp2xContentStream_eType1,     /* Content must not be transmitted to HDCP 1.x downstream devices */
    NEXUS_Hdcp2xContentStream_eMax

} NEXUS_Hdcp2xContentStream;


typedef enum NEXUS_HdmiEdidColorimetryDbSupport
{
    NEXUS_HdmiEdidColorimetryDbSupport_exvYCC601,
    NEXUS_HdmiEdidColorimetryDbSupport_exvYCC709,
    NEXUS_HdmiEdidColorimetryDbSupport_esYCC601,
    NEXUS_HdmiEdidColorimetryDbSupport_eAdobeYCC601,
    NEXUS_HdmiEdidColorimetryDbSupport_eAdobeRGB,
    NEXUS_HdmiEdidColorimetryDbSupport_eBT2020cYCC,
    NEXUS_HdmiEdidColorimetryDbSupport_eBT2020YCC,
    NEXUS_HdmiEdidColorimetryDbSupport_eBT2020RGB,
    NEXUS_HdmiEdidColorimetryDbSupport_eMax
} NEXUS_HdmiEdidColorimetryDbSupport;

/******************************************************************************
Summary:
Enumerated Type for Supported Gamut Metadata Profile
*******************************************************************************/
typedef enum NEXUS_HdmiEdidColorimetryDbMetadataProfile
{
    NEXUS_HdmiEdidColorimetryDbMetadataProfile_e0,
    NEXUS_HdmiEdidColorimetryDbMetadataProfile_e1,
    NEXUS_HdmiEdidColorimetryDbMetadataProfile_e2,
    NEXUS_HdmiEdidColorimetryDbMetadataProfile_e3,
    NEXUS_HdmiEdidColorimetryDbMetadataProfile_eMax
} NEXUS_HdmiEdidColorimetryDbMetadataProfile ;


#ifdef __cplusplus
}
#endif

#endif
