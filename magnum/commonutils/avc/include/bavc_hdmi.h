/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.

 ******************************************************************************/
#ifndef BAVC_HDMI_H__
#define BAVC_HDMI_H__

#include "bstd.h"
#include "bchp_common.h"

/* HDMI Rx Chips */
#if (BCHP_CHIP == 7422)  || (BCHP_CHIP == 7425)  \
 || (BCHP_CHIP == 7640)  || (BCHP_CHIP == 7429)  \
 || (BCHP_CHIP == 7435)  || (BCHP_CHIP == 7445)  \
 || (BCHP_CHIP == 7251)  || (BCHP_CHIP == 74371) \
 || (BCHP_CHIP == 74295) || (BCHP_HDMI_RX_0_REG_START)

#define BAVC_HDMI_RECEIVER 1
#endif

/* HDMI 1.3 Support */
       /* TX CHIPS */                                                     \
#if  (BCHP_CHIP == 7601)  || (BCHP_CHIP == 7635)  || (BCHP_CHIP == 7630)  \
  || (BCHP_CHIP == 7420)  || (BCHP_CHIP == 7125)  || (BCHP_CHIP == 7340)  \
  || (BCHP_CHIP == 7342)  || (BCHP_CHIP == 7550)  || (BCHP_CHIP == 7408)  \
  || (BCHP_CHIP == 7468)  || (BCHP_CHIP == 7208)  || (BCHP_CHIP == 7360)  \
  || (BCHP_CHIP == 7584)  || (BCHP_CHIP == 75845)                         \
                /* 40nm TX CHIPS */                                       \
  || (BCHP_CHIP == 7358)  || (BCHP_CHIP == 7344)  || (BCHP_CHIP == 7346)  \
  || (BCHP_CHIP == 7231)  || (BCHP_CHIP == 7552)  || (BCHP_CHIP == 7360)  \
  || (BCHP_CHIP == 7563)  || (BCHP_CHIP == 7543)  || (BCHP_CHIP == 7362)  \
  || (BCHP_CHIP == 7228)  || (BCHP_CHIP == 75635) || (BCHP_CHIP == 73625) \
  || (BCHP_CHIP == 73465) || (BCHP_CHIP == 75525) \
       /* COMBO RX/TX CHIPS */                                            \
  || (BCHP_CHIP == 7422)  || (BCHP_CHIP == 7425)  || (BCHP_CHIP == 7640)  \
  || (BCHP_CHIP == 7429)  || (BCHP_CHIP == 74295) || (BCHP_CHIP == 7435)  \
          /* COMBO RX/TX - 28nm */                                        \
  || (BCHP_CHIP == 7445)  || (BCHP_CHIP == 7145)  || (BCHP_CHIP == 7366)  \
  || (BCHP_CHIP == 7439)                                                  \
  || (BCHP_CHIP == 7251)  || (BCHP_CHIP == 7364)  || (BCHP_CHIP == 74371) \
  || BHDM_HAS_HDMI_20_SUPPORT                                             \
  || BAVC_HDMI_RECEIVER
#define BAVC_HDMI_1_3_SUPPORT 1
#endif


/* Check for HDMI 2.0 Support */
#if BCHP_HDMI_TX_AUTO_I2C_REG_START
#define BAVC_HDMI_20_SUPPORT 1
#endif


#define BAVC_HDMI_HDCP_N_PRIVATE_KEYS 40
#define BAVC_HDMI_HDCP_KSV_LENGTH 5
#define BAVC_HDMI_HDCP_AN_LENGTH 8


/******************************************************************************
Summary:
I2C Rx HDCP Registers
*******************************************************************************/
/* Rx Bksv value (Read Only) */
#define BAVC_HDMI_HDCP_RX_BKSV0                 0x00
#define BAVC_HDMI_HDCP_RX_BKSV1                 0x01
#define BAVC_HDMI_HDCP_RX_BKSV2                 0x02
#define BAVC_HDMI_HDCP_RX_BKSV3                 0x03
#define BAVC_HDMI_HDCP_RX_BKSV4                 0x04

/* Rx Link verification value (Read Only) */
#define BAVC_HDMI_HDCP_RX_RI0               0x08
#define BAVC_HDMI_HDCP_RX_RI1               0x09

/*  Rx Enhanced Link Verification Response (Read Only) */
#define BAVC_HDMI_HDCP_RX_PJ             0x0A


/* Tx Aksv value (Write Only) */
#define BAVC_HDMI_HDCP_RX_AKSV0                 0x10
#define BAVC_HDMI_HDCP_RX_AKSV1                 0x11
#define BAVC_HDMI_HDCP_RX_AKSV2                 0x12
#define BAVC_HDMI_HDCP_RX_AKSV3                 0x13
#define BAVC_HDMI_HDCP_RX_AKSV4                 0x14

/*  Rx HDCP Enable HDCP 1.1 features (Write Only) */
#define BAVC_HDMI_HDCP_RX_AINFO             0x15
#define BAVC_HDMI_HDCP_RX_ENABLE_1_1_FEATURES 0x02

/* Session Random Number (An) value generated by the Tx (Write Only) */
#define BAVC_HDMI_HDCP_RX_AN0                   0x18
#define BAVC_HDMI_HDCP_RX_AN1                   0x19
#define BAVC_HDMI_HDCP_RX_AN2                   0x1a
#define BAVC_HDMI_HDCP_RX_AN3                   0x1b
#define BAVC_HDMI_HDCP_RX_AN4                   0x1c
#define BAVC_HDMI_HDCP_RX_AN5                   0x1d
#define BAVC_HDMI_HDCP_RX_AN6                   0x1e
#define BAVC_HDMI_HDCP_RX_AN7                   0x1f

/* HDCP Repeater SHA-1 Hash value V' */
#define BAVC_HDMI_HDCP_REPEATER_SHA1_V_H0 0x20
#define BAVC_HDMI_HDCP_REPEATER_SHA1_V_H1 0x24
#define BAVC_HDMI_HDCP_REPEATER_SHA1_V_H2 0x28
#define BAVC_HDMI_HDCP_REPEATER_SHA1_V_H3 0x2c
#define BAVC_HDMI_HDCP_REPEATER_SHA1_V_H4 0x30

/* Rx Capabilities Register (Read Only) */
#define BAVC_HDMI_HDCP_RX_BCAPS          0x40
#define BAVC_HDMI_HDCP_RX_BCAPS_KSV_FIFO_RDY 0x20

/* Rx Status Registers (Read Only) */
#define BAVC_HDMI_HDCP_RX_BSTATUS        0x41 /* 2 Bytes */
#define BAVC_HDMI_HDCP_RX_BSTATUS_DEPTH        0x0700
#define BAVC_HDMI_HDCP_RX_BSTATUS_DEVICE_COUNT 0x007F

#define BAVC_HDMI_HDCP_REPEATER_KSV_FIFO 0x43 /* 2 Bytes */

/* HDCP Repeater Registers */
#define BAVC_HDMI_HDCP_REPEATER_MAX_DEVICE_COUNT 127
#define BAVC_HDMI_HDCP_REPEATER_MAX_DEPTH 7



/******************************************************************************
Summary:
Enumerated Type specifying the current status iof the attached receiver.

Description:
The enumeration types listed can be compared with the value of the returned
BStatus register from the Receiver

See Also:
        BAVC_HDMI_HDCP_GetRxStatus

*******************************************************************************/
typedef enum BAVC_HDMI_HDCP_RxStatus
{
   BAVC_HDMI_HDCP_RxStatus_eHdmiMode             = 0x1000, /* Rx in HDMI mode        */
   BAVC_HDMI_HDCP_RxStatus_eMaxRepeatersExceeded = 0x0800, /* Rx has too many repeaters */
   BAVC_HDMI_HDCP_RxStatus_eMaxDevicesExceeded   = 0x0080  /* Rx has too many devices */
} BAVC_HDMI_HDCP_RxStatus ;



 /******************************************************************************
Summary:
Constants used in association with the Infofromes

Description:
None

See Also:
        o BAVC_HDMI_AviInfoFrame
        o BAVC_HDMI_AudioInfoFrame
        o BAVC_HDMI_SPDInfoFrame
        o BAVC_HDMI_ACP

*******************************************************************************/
#define BAVC_HDMI_PACKET_DATA_LENGTH 28
/* Packet Byte (PB) Length */
#define BAVC_HDMI_PB_LENGTH 28
#define BAVC_HDMI_MAX_VIDEO_ID_CODES 35




typedef uint8_t BAVC_HDMI_HDCP_KSV[BAVC_HDMI_HDCP_KSV_LENGTH] ;


/******************************************************************************
Summary:
Enumerated Type of the status of a particular packet

Description:
The different status of packets at the HDMI Rx


See Also:

*******************************************************************************/
typedef enum
{
        BAVC_HDMI_PacketStatus_eNotReceived = 0, /* initial state */
        BAVC_HDMI_PacketStatus_eStopped,
        BAVC_HDMI_PacketStatus_eUpdated
} BAVC_HDMI_PacketStatus ;


/***************************************************************************
Summary:
Structure containing the HDMI Packet data: Type, Version, Length and Packet Data bytes

****************************************************************************/
typedef struct
{
        uint8_t Type ;
        uint8_t Version ;
        uint8_t Length ;
        uint8_t DataBytes[BAVC_HDMI_PB_LENGTH] ;
} BAVC_HDMI_Packet ;


/******************************************************************************
Summary:
Enumerated Type of the number of bits per pixel

Description:
The enumeration can be used to set/read the number of bits per pixel in the HDMI General
Control Packets

*******************************************************************************/
typedef enum
{
    BAVC_HDMI_BitsPerPixel_e24bit = 0,
    BAVC_HDMI_BitsPerPixel_e30bit,
    BAVC_HDMI_BitsPerPixel_e36bit,
    BAVC_HDMI_BitsPerPixel_e48bit,
    BAVC_HDMI_BitsPerPixel_ePacked /* MHL's packed 16-bit YCbCr */
} BAVC_HDMI_BitsPerPixel;


/******************************************************************************
Summary:
Enumerated Type of pixel repetition setting.

Description:
The enumeration can be used to set/read the number of pixel repetition in the AVI Infoframe
Packets

*******************************************************************************/
typedef enum
{
        BAVC_HDMI_PixelRepetition_eNone = 0,
        BAVC_HDMI_PixelRepetition_e1x,
        BAVC_HDMI_PixelRepetition_e3x,
        BAVC_HDMI_PixelRepetition_e4x,
        BAVC_HDMI_PixelRepetition_e5x,
        BAVC_HDMI_PixelRepetition_e6x,
        BAVC_HDMI_PixelRepetition_e7x,
        BAVC_HDMI_PixelRepetition_e8x,
        BAVC_HDMI_PixelRepetition_e9x,
        BAVC_HDMI_PixelRepetition_e10x
} BAVC_HDMI_PixelRepetition ;




 /******************************************************************************
Summary:
Enumerated Type of the type of Colorspace (Pixel Encoding)  in HDMI AVI Infoframe Packets

Description:
The enumeration can be used to set/read the Pixel Encoding Colorspace in the HDMI AVI
Infoframe Packets.

See Also:
        o BAVC_HDMI_AviInfoFrame

*******************************************************************************/
typedef enum
{
        BAVC_HDMI_AviInfoFrame_Colorspace_eRGB = 0,
        BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr422,
        BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444,
        BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr420
} BAVC_HDMI_AviInfoFrame_Colorspace ;


 /******************************************************************************
Summary:
Enumerated Type of the Active Info bit (valid or invalid) transmited to an HDMI
Receiver indicating if the Active Info fields contain valid data.

Description:
The enumeration can be used to set the Colorspace transmitted in the HDMI AVI
Packets.

See Also:
        o BAVC_HDMI_AviInfoFrame

*******************************************************************************/
typedef enum
{
        BAVC_HDMI_AviInfoFrame_ActiveInfo_eInvalid = 0,
        BAVC_HDMI_AviInfoFrame_ActiveInfo_eValid
} BAVC_HDMI_AviInfoFrame_ActiveInfo ;


/******************************************************************************
Summary:
Enumerated Type for information on Horizonatal and Vertical Bar

Description:
The enumeration can be used to configure the HDMI AVI packet for sending to the
HDMI Rx

See Also:
        o BAVC_HDMI_AviInfoFrame
        o BAVC_HDMI_AviInfoFrame_Colorspace
        o BAVC_HDMI_AviInfoFrame_ScanInfo
*******************************************************************************/
typedef enum
{
        BAVC_HDMI_AviInfoFrame_BarInfo_eInvalid,
        BAVC_HDMI_AviInfoFrame_BarInfo_eVerticalValid,
        BAVC_HDMI_AviInfoFrame_BarInfo_eHorizValid,
        BAVC_HDMI_AviInfoFrame_BarInfo_eVertHorizValid
} BAVC_HDMI_AviInfoFrame_BarInfo ;


/******************************************************************************
Summary:
Enumerated Type for information on Scan Information

Description:
The enumeration can be used to configure the HDMI AVI packet for sending to the
HDMI Rx

See Also:
        o BAVC_HDMI_AviInfoFrame
        o BAVC_HDMI_AviInfoFrame_Colorspace
        o BAVC_HDMI_AviInfoFrame_BarInfo
*******************************************************************************/
typedef enum
{
        BAVC_HDMI_AviInfoFrame_ScanInfo_eNoData,
        BAVC_HDMI_AviInfoFrame_ScanInfo_eOverScanned,
        BAVC_HDMI_AviInfoFrame_ScanInfo_eUnderScanned
} BAVC_HDMI_AviInfoFrame_ScanInfo ;


/******************************************************************************
Summary:
Enumerated Type of the Colorimetry stored in the AVI Info Frame

Description:
The enumeration can be used to parse/configure the HDMI AVI packet
received/sent

See Also:
        o BAVC_HDMI_AviInfoFrame
        o BAVC_HDMI_AviInfoFrame_Colorspace
*******************************************************************************/
typedef enum
{
        BAVC_HDMI_AviInfoFrame_Colorimetry_eNoData =  0,
        BAVC_HDMI_AviInfoFrame_Colorimetry_eSmpte170,
        BAVC_HDMI_AviInfoFrame_Colorimetry_eItu709,
        BAVC_HDMI_AviInfoFrame_Colorimetry_eFuture,
        BAVC_HDMI_AviInfoFrame_Colorimetry_eExtended = BAVC_HDMI_AviInfoFrame_Colorimetry_eFuture

} BAVC_HDMI_AviInfoFrame_Colorimetry ;



/******************************************************************************
Summary:
Enumerated Type of the Picture Aspect Ratio stored in the AVI Info Frame

Description:
The enumeration can be used to parse/configure the HDMI AVI packet
received/sent

See Also:
        o BAVC_HDMI_AviInfoFrame
        o BAVC_HDMI_AviInfoFrame_ActiveFormatAspectRatio
*******************************************************************************/
typedef enum
{
        BAVC_HDMI_AviInfoFrame_PictureAspectRatio_eNoData = 0,
        BAVC_HDMI_AviInfoFrame_PictureAspectRatio_e4_3,
        BAVC_HDMI_AviInfoFrame_PictureAspectRatio_e16_9,
        BAVC_HDMI_AviInfoFrame_PictureAspectRatio_eFuture
} BAVC_HDMI_AviInfoFrame_PictureAspectRatio ;


/******************************************************************************
Summary:
Enumerated Type of the Active Format Aspect Ratio stored in the AVI Info Frame

Description:
The enumeration can be used to parse/configure the HDMI AVI packet
received/sent

See Also:
        o BAVC_HDMI_AviInfoFrame
        o BAVC_HDMI_AviInfoFrame_PictureAspectRatio
*******************************************************************************/
typedef enum
{
        BAVC_HDMI_AviInfoFrame_ActiveFormatAspectRatio_ePicture    =  8,
        BAVC_HDMI_AviInfoFrame_ActiveFormatAspectRatio_e4_3Center  =  9,
        BAVC_HDMI_AviInfoFrame_ActiveFormatAspectRatio_e16_9Center = 10,
        BAVC_HDMI_AviInfoFrame_ActiveFormatAspectRatio_e14_9Center = 11,

        BAVC_HDMI_AviInfoFrame_ActiveFormatAspectRatio_e4_3_Alt14_9Center = 13,
        BAVC_HDMI_AviInfoFrame_ActiveFormatAspectRatio_e16_9_Alt14_9Center = 14,
        BAVC_HDMI_AviInfoFrame_ActiveFormatAspectRatio_e16_9_Alt4_3Center = 15,
        BAVC_HDMI_AviInfoFrame_ActiveFormatAspectRatio_eOther      =  0
} BAVC_HDMI_AviInfoFrame_ActiveFormatAspectRatio ;
                                                   /* see CEA 861 Spec */


/******************************************************************************
Summary:
Enumerated Type of the Picture Scaling used to parse/configure the HDMI AVI
packet received/sent

Description:
The enumeration can be used to configure the HDMI AVI packet for sending to the
HDMI Rx

See Also:
        o BAVC_HDMI_AviInfoFrame
*******************************************************************************/
typedef enum
{
        BAVC_HDMI_AviInfoFrame_Scaling_eNoScaling = 0,
        BAVC_HDMI_AviInfoFrame_Scaling_eHScaling,
        BAVC_HDMI_AviInfoFrame_Scaling_eVScaling,
        BAVC_HDMI_AviInfoFrame_Scaling_eHVScaling
} BAVC_HDMI_AviInfoFrame_Scaling ;



/******************************************************************************
Summary:
Enumerated Type for Extended Colorimetry (IED 61966-2-4) See CEA-861-D Table 11
used to parse/configure the HDMI AVI packet received/sent

Description:
The enumeration can be used to configure/parse the HDMI AVI packet for sending/receiving

See Also:
        o BAVC_HDMI_AviInfoFrame
*******************************************************************************/
typedef enum
{
        BAVC_HDMI_AviInfoFrame_ExtendedColorimetry_exvYCC601 = 0,
        BAVC_HDMI_AviInfoFrame_ExtendedColorimetry_exvYCC709,
        BAVC_HDMI_AviInfoFrame_ExtendedColorimetry_esYCC601,
        BAVC_HDMI_AviInfoFrame_ExtendedColorimetry_eAdobeYCC601,
        BAVC_HDMI_AviInfoFrame_ExtendedColorimetry_eAdobeRGB,
        BAVC_HDMI_AviInfoFrame_ExtendedColorimetry_eItuRBt2020YcCbcCrc,
        BAVC_HDMI_AviInfoFrame_ExtendedColorimetry_eItuRBt2020RGB,
        BAVC_HDMI_AviInfoFrame_ExtendedColorimetry_eItuRBt2020YCbCr = BAVC_HDMI_AviInfoFrame_ExtendedColorimetry_eItuRBt2020RGB,
        BAVC_HDMI_AviInfoFrame_ExtendedColorimetry_eReserved,
        BAVC_HDMI_AviInfoFrame_ExtendedColorimetry_eMax         = BAVC_HDMI_AviInfoFrame_ExtendedColorimetry_eReserved
} BAVC_HDMI_AviInfoFrame_ExtendedColorimetry ;

/******************************************************************************
Summary:
Enumerated Type for Quantization Range See CEA-861-D Table 11
used to parse/configure the HDMI AVI packet received/sent

Description:
The enumeration can be used to configure/parse the HDMI AVI packet for sending/receiving

See Also:
        o BAVC_HDMI_AviInfoFrame
*******************************************************************************/
typedef enum
{
        BAVC_HDMI_AviInfoFrame_RGBQuantizationRange_eDefault    =  0,
        BAVC_HDMI_AviInfoFrame_RGBQuantizationRange_eLimitedRange  =  1,
        BAVC_HDMI_AviInfoFrame_RGBQuantizationRange_eFullRange = 2,
        BAVC_HDMI_AviInfoFrame_RGBQuantizationRange_eReserved = 3
} BAVC_HDMI_AviInfoFrame_RGBQuantizationRange ;
                                                   /* see CEA 861-D Spec */

/******************************************************************************
Summary:
Enumerated Type for IT Content See CEA-861-D Table 11
used to parse/configure the HDMI AVI packet received/sent

Description:
The enumeration can be used to configure/parse the HDMI AVI packet for sending/receiving

See Also:
        o BAVC_HDMI_AviInfoFrame
*******************************************************************************/
typedef enum
{
        BAVC_HDMI_AviInfoFrame_ITContent_eNoData    =  0,
        BAVC_HDMI_AviInfoFrame_ITContent_eITContent  =  1
} BAVC_HDMI_AviInfoFrame_ITContent ;
                                                   /* see CEA 861-D Spec */

typedef enum
{
        BAVC_HDMI_AviInfoFrame_VideoFormat_eIT = 0,
        BAVC_HDMI_AviInfoFrame_VideoFormat_eSD,
        BAVC_HDMI_AviInfoFrame_VideoFormat_eHD
} BAVC_HDMI_AviInfoFrame_VideoFormat ;


 /******************************************************************************
Summary:
Enumerated Type of the type of IT Content Type (CN1 CN0)in HDMI AVI Infoframe Packets

Description:
The enumeration can be used to set/read the Content Type in the HDMI AVI  Infoframe Packets.
See CEA-861-E

See Also:
        o BAVC_HDMI_AviInfoFrame

*******************************************************************************/
typedef enum
{
        BAVC_HDMI_AviInfoFrame_ContentType_eGraphics = 0,
        BAVC_HDMI_AviInfoFrame_ContentType_ePhoto,
        BAVC_HDMI_AviInfoFrame_ContentType_eCinema,
        BAVC_HDMI_AviInfoFrame_ContentType_eGame
} BAVC_HDMI_AviInfoFrame_ContentType ;



 /******************************************************************************
Summary:
Enumerated Type of the type of YCC Quantization Range in HDMI AVI Infoframe Packets

Description:
The enumeration can be used to set/read the YCC Quantization Range (YQ1 YQ0) in the HDMI AVI  Infoframe Packets.
See CEA-861-E

See Also:
        o BAVC_HDMI_AviInfoFrame

*******************************************************************************/
typedef enum
{
        BAVC_HDMI_AviInfoFrame_YccQuantizationRange_eLimited = 0,
        BAVC_HDMI_AviInfoFrame_YccQuantizationRange_eFull,
        BAVC_HDMI_AviInfoFrame_YccQuantizationRange_eReserved
} BAVC_HDMI_AviInfoFrame_YccQuantizationRange ;



/***************************************************************************
Summary:
Structure containing the contents of the HDMI AVI Info Frame

Note:
For HDMI Rx (i.e. BCM3560), all AVI field values are parsed from the received
AVI frame.

For HDMI Tx (i,e, BCM7038), some AVI field values are populated based on
other magnum settings i.e BFMT_VideoFmt etc.

****************************************************************************/
typedef struct _HDMI_AVI_INFOFRAME_
{
        /*
        --NOTE: modifications to this HDMI_AVI_INFOFRAME  structure must be
        -- accounted for in the BHDM_P_HdmiSettingsChange function
        */

        /* Generate internally for HDMI Tx platforms */
        BAVC_HDMI_AviInfoFrame_Colorspace ePixelEncoding ;      /* [Y2]Y1Y0 - Rx Only*/

        bool bOverrideDefaults ;

        /* Pixel Encoding (RGB44 or YCbCr444) based on Colorimetry  */
        BAVC_HDMI_AviInfoFrame_ActiveInfo eActiveInfo ; /* A0 */
        BAVC_HDMI_AviInfoFrame_BarInfo    eBarInfo ;    /* B1B0 */
        BAVC_HDMI_AviInfoFrame_ScanInfo   eScanInfo ;   /* S1S0 */

        /* Generate internally from BAVC_MatrixCoefficients for HDMI Tx platforms */
        BAVC_HDMI_AviInfoFrame_Colorimetry eColorimetry ;  /* C1C0 - Rx Only*/

        /* for Colorimetry (C1C0) requires conversion to BCM magnum values */

        /* By default, Picture AR (M1M0) is generated from BFMT_AspectRatio which
           requires conversion to HDMI AVI values.
           Otherwise Picture AR can be overridden if bOverrideDefaults is set */
        BAVC_HDMI_AviInfoFrame_PictureAspectRatio ePictureAspectRatio ; /* M1M0 */

        BAVC_HDMI_AviInfoFrame_ActiveFormatAspectRatio eActiveFormatAspectRatio ; /* R3R0 */

        BAVC_HDMI_AviInfoFrame_Scaling eScaling ; /* SC1SC0 */

        uint8_t VideoIdCode ; /* VICn */

        /* Generate internally from eVideoFmt for HDMI Tx platforms */
        uint8_t PixelRepeat ;


#if BAVC_HDMI_1_3_SUPPORT
        BAVC_HDMI_AviInfoFrame_ITContent eITContent ; /*ITC */

        BAVC_HDMI_AviInfoFrame_ExtendedColorimetry eExtendedColorimetry ; /* EC2EC1EC0 */

        BAVC_HDMI_AviInfoFrame_RGBQuantizationRange eRGBQuantizationRange ; /* Q1Q0 */

        BAVC_HDMI_AviInfoFrame_YccQuantizationRange eYccQuantizationRange ; /* YQ1YQ0 */

        BAVC_HDMI_AviInfoFrame_ContentType eContentType ;  /* CN1CN0 */
#endif


        /* bar info */
        uint16_t TopBarEndLineNumber ;
        uint16_t BottomBarStartLineNumber ;

        uint16_t LeftBarEndPixelNumber ;
        uint16_t RightBarEndPixelNumber ;

#if BAVC_HDMI_RECEIVER
        BAVC_HDMI_PacketStatus ePacketStatus ;
        BAVC_HDMI_Packet stPacket ;
#endif

} BAVC_HDMI_AviInfoFrame ;




/*******************************************************************************
Summary:
Enumerated Type for Coding Type. See CEA-861-D
********************************************************************************/
typedef enum
{
        BAVC_HDMI_AudioInfoFrame_CodingType_eReferToStream,
        BAVC_HDMI_AudioInfoFrame_CodingType_ePCM,
        BAVC_HDMI_AudioInfoFrame_CodingType_eAC3,
        BAVC_HDMI_AudioInfoFrame_CodingType_eMPEG1,
        BAVC_HDMI_AudioInfoFrame_CodingType_eMP3,
        BAVC_HDMI_AudioInfoFrame_CodingType_eMPEG2,
        BAVC_HDMI_AudioInfoFrame_CodingType_eAACLC,
        BAVC_HDMI_AudioInfoFrame_CodingType_eDTS,
        BAVC_HDMI_AudioInfoFrame_CodingType_eATRAC,
        BAVC_HDMI_AudioInfoFrame_CodingType_eDSD,
        BAVC_HDMI_AudioInfoFrame_CodingType_eEAC3,
        BAVC_HDMI_AudioInfoFrame_CodingType_eDTSHD,
        BAVC_HDMI_AudioInfoFrame_CodingType_eMLP,
        BAVC_HDMI_AudioInfoFrame_CodingType_eDST,
        BAVC_HDMI_AudioInfoFrame_CodingType_eWMAPro,
        BAVC_HDMI_AudioInfoFrame_CodingType_eUnknown
} BAVC_HDMI_AudioInfoFrame_CodingType ;


/******************************************************************************
Summary:
Enumerated Type for Channel Count. See CEA-861-D
*******************************************************************************/
typedef enum
{
        BAVC_HDMI_AudioInfoFrame_ChannelCount_eReferToStreamHeader,
        BAVC_HDMI_AudioInfoFrame_ChannelCount_e2Channels,
        BAVC_HDMI_AudioInfoFrame_ChannelCount_e3Channels,
        BAVC_HDMI_AudioInfoFrame_ChannelCount_e4Channels,
        BAVC_HDMI_AudioInfoFrame_ChannelCount_e5Channels,
        BAVC_HDMI_AudioInfoFrame_ChannelCount_e6Channels,
        BAVC_HDMI_AudioInfoFrame_ChannelCount_e7Channels,
        BAVC_HDMI_AudioInfoFrame_ChannelCount_e8Channels
} BAVC_HDMI_AudioInfoFrame_ChannelCount ;


/*******************************************************************************
Summary:
Enumerated Type for Sampling Frequency. See CEA-861-D
*******************************************************************************/
typedef enum
{
        BAVC_HDMI_AudioInfoFrame_SampleFrequency_eReferToStreamHeader,
        BAVC_HDMI_AudioInfoFrame_SampleFrequency_e32,
        BAVC_HDMI_AudioInfoFrame_SampleFrequency_e44_1,
        BAVC_HDMI_AudioInfoFrame_SampleFrequency_e48,
        BAVC_HDMI_AudioInfoFrame_SampleFrequency_e88_2,
        BAVC_HDMI_AudioInfoFrame_SampleFrequency_e96,
        BAVC_HDMI_AudioInfoFrame_SampleFrequency_e176_4,
        BAVC_HDMI_AudioInfoFrame_SampleFrequency_e192
} BAVC_HDMI_AudioInfoFrame_SampleFrequency ;


/******************************************************************************
Summary:
Enumerated Type for Sample Size. See CEA-861-D
*******************************************************************************/
typedef enum
{
        BAVC_HDMI_AudioInfoFrame_SampleSize_eReferToStreamHeader,
        BAVC_HDMI_AudioInfoFrame_SampleSize_e16,
        BAVC_HDMI_AudioInfoFrame_SampleSize_e20,
        BAVC_HDMI_AudioInfoFrame_SampleSize_e24
} BAVC_HDMI_AudioInfoFrame_SampleSize ;


/******************************************************************************
Summary:
Enumerated Type for Level Shift Value. See CEA-861-D
*******************************************************************************/
typedef enum
{
        BAVC_HDMI_AudioInfoFrame_LevelShift_e0db,
        BAVC_HDMI_AudioInfoFrame_LevelShift_e1db,
        BAVC_HDMI_AudioInfoFrame_LevelShift_e2db,
        BAVC_HDMI_AudioInfoFrame_LevelShift_e3db,
        BAVC_HDMI_AudioInfoFrame_LevelShift_e4db,
        BAVC_HDMI_AudioInfoFrame_LevelShift_e5db,
        BAVC_HDMI_AudioInfoFrame_LevelShift_e6db,
        BAVC_HDMI_AudioInfoFrame_LevelShift_e7db,
        BAVC_HDMI_AudioInfoFrame_LevelShift_e8db,
        BAVC_HDMI_AudioInfoFrame_LevelShift_e9db,
        BAVC_HDMI_AudioInfoFrame_LevelShift_e10db,
        BAVC_HDMI_AudioInfoFrame_LevelShift_e11db,
        BAVC_HDMI_AudioInfoFrame_LevelShift_e12db,
        BAVC_HDMI_AudioInfoFrame_LevelShift_e13db,
        BAVC_HDMI_AudioInfoFrame_LevelShift_e14db,
        BAVC_HDMI_AudioInfoFrame_LevelShift_e15db
} BAVC_HDMI_AudioInfoFrame_LevelShift ;


/******************************************************************************
Summary:
Enumerated Type for Down Mix. See CEA-861-D
******************************************************************************/
typedef enum
{
        BAVC_HDMI_AudioInfoFrame_DownMixInhibit_ePermitted,
        BAVC_HDMI_AudioInfoFrame_DownMixInhibit_eProhibited
} BAVC_HDMI_AudioInfoFrame_DownMixInhibit ;


typedef enum
{
        BAVC_HDMI_SpdInfoFrame_SourceType_eUnknown,
        BAVC_HDMI_SpdInfoFrame_SourceType_eDigitalSTB,
        BAVC_HDMI_SpdInfoFrame_SourceType_eDVDPlayer,
        BAVC_HDMI_SpdInfoFrame_SourceType_eDVHS,
        BAVC_HDMI_SpdInfoFrame_SourceType_eHDDVideoRecorder,
        BAVC_HDMI_SpdInfoFrame_SourceType_eDVC,
        BAVC_HDMI_SpdInfoFrame_SourceType_eDSC,
        BAVC_HDMI_SpdInfoFrame_SourceType_eVideoCD,
        BAVC_HDMI_SpdInfoFrame_SourceType_eGame,
        BAVC_HDMI_SpdInfoFrame_SourceType_ePCGeneral,
        BAVC_HDMI_SpdInfoFrame_SourceType_eBluRayDisc,
        BAVC_HDMI_SpdInfoFrame_SourceType_eSuperAudioCD,
        BAVC_HDMI_SpdInfoFrame_SourceType_eHD_DVD,
        BAVC_HDMI_SpdInfoFrame_SourceType_ePMP
} BAVC_HDMI_SpdInfoFrame_SourceType ;


/******************************************************************************
Summary:
Enumerated Type for Vendor-Specific Infoframe HDMI Video Format
******************************************************************************/
typedef enum
{
        BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eNone,
        BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eExtendedResolution,
        BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_e3DFormat,
        BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eMax
} BAVC_HDMI_VSInfoFrame_HDMIVideoFormat ;


/******************************************************************************
Summary:
Enumerated Type for Vendor-Specific Infoframe HDMI VIC
******************************************************************************/
typedef enum
{
        BAVC_HDMI_VSInfoFrame_HDMIVIC_eReserved,
        BAVC_HDMI_VSInfoFrame_HDMIVIC_e4Kx2K_2997_30Hz,
        BAVC_HDMI_VSInfoFrame_HDMIVIC_e4Kx2K_25Hz,
        BAVC_HDMI_VSInfoFrame_HDMIVIC_e4Kx2K_2398_24Hz,
        BAVC_HDMI_VSInfoFrame_HDMIVIC_e4Kx2K_SMPTE_24Hz,
        BAVC_HDMI_VSInfoFrame_HDMIVIC_eMax
} BAVC_HDMI_VSInfoFrame_HDMIVIC ;


/******************************************************************************
Summary:
Enumerated Type for Vendor-Specific Infoframe 3D Structure
******************************************************************************/
typedef enum
{
        BAVC_HDMI_VSInfoFrame_3DStructure_eFramePacking,
        BAVC_HDMI_VSInfoFrame_3DStructure_eFieldAlternative,
        BAVC_HDMI_VSInfoFrame_3DStructure_eLineAlternative,
        BAVC_HDMI_VSInfoFrame_3DStructure_eSidexSideFull,
        BAVC_HDMI_VSInfoFrame_3DStructure_eLDepth,
        BAVC_HDMI_VSInfoFrame_3DStructure_eLDepthGraphics,
        BAVC_HDMI_VSInfoFrame_3DStructure_eTopAndBottom,
        BAVC_HDMI_VSInfoFrame_3DStructure_eReserved,
        BAVC_HDMI_VSInfoFrame_3DStructure_eSidexSideHalf, /* 0x8 1000 */
        BAVC_HDMI_VSInfoFrame_3DStructure_eMax
} BAVC_HDMI_VSInfoFrame_3DStructure ;


/******************************************************************************
Summary:
Enumerated Type for Vendor-Specific Infoframe 3D Ext Data
******************************************************************************/
typedef enum
{
        BAVC_HDMI_VSInfoFrame_3DExtData_eNone,
        BAVC_HDMI_VSInfoFrame_3DExtData_eHorzOLOR = 0 ,
        BAVC_HDMI_VSInfoFrame_3DExtData_eHorzOLER,
        BAVC_HDMI_VSInfoFrame_3DExtData_eHorzELOR,
        BAVC_HDMI_VSInfoFrame_3DExtData_eHorzELER,
        BAVC_HDMI_VSInfoFrame_3DExtData_eQuinOLOR,
        BAVC_HDMI_VSInfoFrame_3DExtData_eQuinOLER,
        BAVC_HDMI_VSInfoFrame_3DExtData_eQuinELOR,
        BAVC_HDMI_VSInfoFrame_3DExtData_eQuinELER,
        BAVC_HDMI_VSInfoFrame_3DExtData_eMax
} BAVC_HDMI_VSInfoFrame_3DExtData ;



#define BAVC_HDMI_INFOFRAME_PACKET_TYPE 0x80

/******************************************************************************
Summary:
Enumerated Type of the different HDMI RAM Packets

Description:
The enumeration can be used to configure the HDMI RAM packets for sending and/or
receiving


See Also:
*******************************************************************************/
typedef enum
{
        BAVC_HDMI_PacketType_eNull                   = 0x00,
        BAVC_HDMI_PacketType_eAudioClockRegeneration = 0x01,
        BAVC_HDMI_PacketType_eAudioSample            = 0x02,
        BAVC_HDMI_PacketType_eGeneralControl         = 0x03,
        BAVC_HDMI_PacketType_eAudioContentProtection = 0x04,
        BAVC_HDMI_PacketType_eISRC1                  = 0x05,
        BAVC_HDMI_PacketType_eISRC2                  = 0x06,
        BAVC_HDMI_PacketType_eDirectStream           = 0x07,
#if 0
        BAVC_HDMI_PacketType_eDstAudioPacket       = 0x08,
#endif
        BAVC_HDMI_PacketType_eHbrAudioPacket      = 0x09,

#if BAVC_HDMI_1_3_SUPPORT
        BAVC_HDMI_PacketType_eGamutMetadataPacket   = 0xA,
#endif

        BAVC_HDMI_PacketType_eVendorSpecificInfoFrame         = 0x81,
        /* backwards compatibility */
        BAVC_HDMI_PacketType_eVendorSpecificInfoframe         = BAVC_HDMI_PacketType_eVendorSpecificInfoFrame,
        BAVC_HDMI_PacketType_eVendorSpecific = BAVC_HDMI_PacketType_eVendorSpecificInfoFrame,

        BAVC_HDMI_PacketType_eAviInfoFrame           = 0x82, /* AVI - Auxillary Video Information */
        BAVC_HDMI_PacketType_eSpdInfoFrame           = 0x83, /* SPD - Source Product Description */
        BAVC_HDMI_PacketType_eAudioInfoFrame         = 0x84,
        BAVC_HDMI_PacketType_eMpegInfoFrame          = 0x85, /* MPEG - Moving Picture Experts Group */
        BAVC_HDMI_PacketType_eDrmInfoFrame           = 0x87, /* DRM - Dynamic Range and Mastering */

        BAVC_HDMI_PacketType_eUnused                 = 0xA0
} BAVC_HDMI_PacketType ;


/******************************************************************************
Summary:
Version number of each InfoFrame Packet Type

Description:
The values are used to indicate the supported version of each HDMI RAM packet


See Also:
        o BAVC_HDMI_PacketType
*******************************************************************************/
#define BAVC_HDMI_PacketType_VendorSpecificInfoFrameVersion  0x01
#define BAVC_HDMI_PacketType_AviInfoFrameVersion    0x02
#define BAVC_HDMI_PacketType_SpdInfoFrameVersion    0x01
#define BAVC_HDMI_PacketType_AudioInfoFrameVersion 0x01
#define BAVC_HDMI_PacketType_DrmInfoFrameVersion   0x01


typedef enum
{
        BAVC_HDMI_GamutFormat_eVerticesFacets,
        BAVC_HDMI_GamutFormat_eRange
} BAVC_HDMI_GamutFormat ;


typedef enum
{
        BAVC_HDMI_GamutColorPrecision_e8Bit,
        BAVC_HDMI_GamutColorPrecision_e10Bit,
        BAVC_HDMI_GamutColorPrecision_e12Bit,
        BAVC_HDMI_GamutColorPrecision_eUnknown
} BAVC_HDMI_GamutColorPrecision ;


typedef enum
{
        BAVC_HDMI_GamutColorspace_eItu709RGB,
        BAVC_HDMI_GamutColorspace_exvYCC601SD,
        BAVC_HDMI_GamutColorspace_exvYCC709HD,
        BAVC_HDMI_GamutColorspace_eXZY /* not supported */
} BAVC_HDMI_GamutColorspace ;


/***************************************************************************
Summary:
Structure containing the contents of the HDMI Audio Info Frame

****************************************************************************/
typedef struct BAVC_HDMI_AudioInfoFrame
{
        /*
        --NOTE: modifications to this HDMI_AUDIO_INFOFRAME  structure must be
        -- accounted for in the BHDM_P_HdmiSettingsChange function
        */

        bool bOverrideDefaults;
        BAVC_HDMI_AudioInfoFrame_ChannelCount ChannelCount;

        BAVC_HDMI_AudioInfoFrame_CodingType CodingType ;
        BAVC_HDMI_AudioInfoFrame_SampleSize SampleSize ;
        BAVC_HDMI_AudioInfoFrame_SampleFrequency SampleFrequency ;

        uint8_t SpeakerAllocation;
        BAVC_HDMI_AudioInfoFrame_DownMixInhibit DownMixInhibit;
        BAVC_HDMI_AudioInfoFrame_LevelShift LevelShift;

#if BAVC_HDMI_RECEIVER
        BAVC_HDMI_PacketStatus ePacketStatus ;
        BAVC_HDMI_Packet stPacket ;
#endif
} BAVC_HDMI_AudioInfoFrame ;

/***************************************************************************
Summary:
Structure containing the contents of the Vendor Specific Info Frame

****************************************************************************/

#define BAVC_HDMI_VS_IEEE_REGID_OFFSET 1
#define BAVC_HDMI_IEEE_REGID_LEN 3
#define BAVC_HDMI_VS_PAYLOAD_OFFSET 4

typedef struct _HDMI_VENDOR_SPECIFIC_INFOFRAME_
{
        /*
        --NOTE: modifications to this HDMI_VENDOR_SPECIFIC_INFOFRAME  structure must be
        -- accounted for in the BHDM_P_HdmiSettingsChange function
        */


        uint8_t uIEEE_RegId[BAVC_HDMI_IEEE_REGID_LEN] ;

        BAVC_HDMI_VSInfoFrame_HDMIVideoFormat  eHdmiVideoFormat ;

        BAVC_HDMI_VSInfoFrame_HDMIVIC eHdmiVic ;
        /* or */
        BAVC_HDMI_VSInfoFrame_3DStructure e3DStructure ;

        BAVC_HDMI_VSInfoFrame_3DExtData e3DExtData ;


#if BAVC_HDMI_RECEIVER
        BAVC_HDMI_PacketStatus ePacketStatus ;
        BAVC_HDMI_Packet stPacket ;
#endif
} BAVC_HDMI_VendorSpecificInfoFrame ;


/******************************************************************************
Summary
Structure containing the contents of the Audio Clock Regeneration Frame
*******************************************************************************/
typedef struct  _HDMI_AudioClockRegeneration_PACKET_
{
        uint32_t CTS;
        uint32_t N;
#if BAVC_HDMI_RECEIVER
        BAVC_HDMI_PacketStatus ePacketStatus ;
        BAVC_HDMI_Packet stPacket ;
#endif
}BAVC_HDMI_AudioClockRegenerationPacket;


/******************************************************************************
Summary
Enumeration type describing HDMI Color Depth
*******************************************************************************/
typedef enum _BAVC_HDMI_GCP_ColorDepth_
{
        BAVC_HDMI_GCP_ColorDepth_eDepthNotIndicated,
        BAVC_HDMI_GCP_ColorDepth_e24bpp = 4,
        BAVC_HDMI_GCP_ColorDepth_e30bpp = 5,
        BAVC_HDMI_GCP_ColorDepth_e36bpp = 6,
        BAVC_HDMI_GCP_ColorDepth_e48bpp = 7,
        BAVC_HDMI_GCP_ColorDepth_eUnknown
} BAVC_HDMI_GCP_ColorDepth ;


/******************************************************************************
Summary
Enumeration type describing HDMI Pixel Packing
*******************************************************************************/
typedef enum _BAVC_HDMI_GCP_PixelPacking_
{
        BAVC_HDMI_GCP_PixelPacking_ePhase4_10P4,
        BAVC_HDMI_GCP_PixelPacking_ePhase1_10P1_12P1_16P1,
        BAVC_HDMI_GCP_PixelPacking_ePhase2_10P2_12P2,
        BAVC_HDMI_GCP_PixelPacking_ePhase3_10P3
} BAVC_HDMI_GCP_PixelPacking ;


/***************************************************************************
Summary:
Structure containing the contents of the HDMI General Control Packet (GCP) Data

****************************************************************************/
typedef struct _HDMI_GCP_DATA_
{
        uint32_t uiMuteStatus ;
        uint32_t SetAvMute ;
        uint32_t ClearAvMute ;
        BAVC_HDMI_GCP_PixelPacking ePixelPacking ;
        BAVC_HDMI_GCP_ColorDepth eColorDepth ;
        uint32_t DefaultPhase ;
#if BAVC_HDMI_RECEIVER
        BAVC_HDMI_PacketStatus ePacketStatus ;
        BAVC_HDMI_Packet stPacket ;
#endif
} BAVC_HDMI_GcpData ;


/***************************************************************************
Summary:
Structure containing the contents of the HDMI Gamut Boundary Data (GDB) Packer

****************************************************************************/

#define BAVC_HDMI_GDB_VerticeBlack      0
#define BAVC_HDMI_GDB_VerticeRedIndex   1
#define BAVC_HDMI_GDB_VerticeGreenIndex 2
#define BAVC_HDMI_GDB_VerticeBlueIndex  3



typedef struct _HDMI_GAMUT_PACKET_
{
        uint8_t Type ;
        uint8_t NextField ;
        uint8_t Profile ;  /* update to use enum */
        uint8_t AffectedGamutSeqNumber ;
        uint8_t NoCurrentGamut ;
        uint8_t PacketSeq ;
        uint8_t CurrentGamutSeqNumber ;
        uint8_t DataBytes[BAVC_HDMI_PB_LENGTH] ;

        BAVC_HDMI_GamutFormat Format ;
        bool bFacets ;
        BAVC_HDMI_GamutColorPrecision eColorPrecision;
        BAVC_HDMI_GamutColorspace eColorSpace  ;
        uint16_t uiNumberVertices ;
        uint16_t uiNumberFacets ;

        /* for Gamut Vertice Data */
        uint16_t uiY[4] ;
        uint16_t uiCb[4] ;
        uint16_t uiCr[4] ;

        /* for Gamut Range Data */
        uint16_t uiMinRedData ;
        uint16_t uiMaxRedData ;
        uint16_t uiMinGreenData ;
        uint16_t uiMaxGreenData ;
        uint16_t uiMinBlueData ;
        uint16_t uiMaxBlueData ;

#if 0
        uint16_t uiBlackY ;
        uint16_t uiBlackCb ;
        uint16_t uiBlackCr ;

        uint16_t uiRedY ;
        uint16_t uiRedCb ;
        uint16_t uiRedCr ;

        uint16_t uiGreenY ;
        uint16_t uiGreenCb ;
        uint16_t uiGreenCr ;

        uint16_t uiBlueY ;
        uint16_t uiBlueCb ;
        uint16_t uiBlueCr ;
#endif
        BAVC_HDMI_PacketStatus ePacketStatus ;

} BAVC_HDMI_GamutPacket ;





/***************************************************************************
Summary:
Structure containing the contents of the HDMI  SPD Info Frame

****************************************************************************/

#define BAVC_HDMI_IF_DATA_OFFSET                       3

#define BAVC_HDMI_SPD_IF_VENDOR_OFFSET   1
#define BAVC_HDMI_SPD_IF_VENDOR_LEN         8
#define BAVC_HDMI_SPD_IF_DESC_OFFSET       9
#define BAVC_HDMI_SPD_IF_DESC_LEN            16
#define BAVC_HDMI_SPD_IF_DEVICE_INFO_OFFSET      25

typedef struct _HDMI_SPD_INFOFRAME_
{
        /*
        --NOTE: modifications to this HDMI_SPD_INFOFRAME  structure must be
        -- accounted for in the BHDM_P_HdmiSettingsChange function
        */

        uint8_t VendorName[BAVC_HDMI_SPD_IF_VENDOR_LEN+1] ;  /* add space for NULL */
        uint8_t ProductDescription[BAVC_HDMI_SPD_IF_DESC_LEN+1] ;
        BAVC_HDMI_SpdInfoFrame_SourceType SourceDeviceInfo;
#if BAVC_HDMI_RECEIVER
        BAVC_HDMI_PacketStatus ePacketStatus ;
        BAVC_HDMI_Packet stPacket ;
#endif
}BAVC_HDMI_SPDInfoFrame;



/***************************************************************************
Summary:
Structure containing the contents of the HDMI Audio Content Protection
****************************************************************************/
typedef struct _HDMI_AUDIO_CONTENT_PROTECTION
{
        uint8_t         ACP_Type;
        char    ACP_Type_Dependent[BAVC_HDMI_PB_LENGTH];

#if BAVC_HDMI_RECEIVER
        BAVC_HDMI_PacketStatus ePacketStatus ;
        BAVC_HDMI_Packet stPacket ;
#endif
} BAVC_HDMI_ACP ;


#define BAVC_HDMI_ISRC_PACKET_BYTES 16
#define BAVC_HDMI_IRSC2_PACKET_OFFSET 16
#define BAVC_HDMI_ISRC_LENGTH 32
/***************************************************************************
Summary:
Structure containing the contents of ISRC Packet.  The contents of the ISRC packet *can*
actually be sent over two different packets (ISRC1 and ISRC2)
****************************************************************************/
typedef struct _HDMI_ISRC
{
        bool    ISRC1_PacketReceived ;

        /* fields from ISRC Packet */

        bool    ISRC_Cont ;  /* ISRC Continued in next Packet */

        /* data located in ISRC_Status field and ISRC_UPC_EAN_ISRC_xx fields are valid */
        bool    ISRC_Valid ;
        uint8_t         ISRC_Status ;

        /* contains concatenated data bytes from both ISRC1 and ISRC2 */
        uint8_t ISRC_UPC_EAN[BAVC_HDMI_ISRC_LENGTH];


#if BAVC_HDMI_RECEIVER
        BAVC_HDMI_Packet stISRC1_Packet ;
        BAVC_HDMI_Packet stISRC2_Packet ;

        BAVC_HDMI_PacketStatus ePacketStatus ;
#endif
} BAVC_HDMI_ISRC ;


/***************************************************************************
Summary:
enunumeration of Electro-Optical Transfer Function
****************************************************************************/
typedef enum BAVC_HDMI_DRM_EOTF
{
    BAVC_HDMI_DRM_EOTF_eSDR,
    BAVC_HDMI_DRM_EOTF_eHDR,
    BAVC_HDMI_DRM_EOTF_eSMPTE_ST_2084,
    BAVC_HDMI_DRM_EOTF_eFuture,
    BAVC_HDMI_DRM_EOTF_eMax
} BAVC_HDMI_DRM_EOTF ;


/***************************************************************************
Summary:
enunumeration of Static Metadata Decscriptor IDs
****************************************************************************/
typedef enum BAVC_HDMI_DRM_DescriptorId
{
    BAVC_HDMI_DRM_DescriptorId_eType1,
    BAVC_HDMI_DRM_DescriptorId_eMax
} BAVC_HDMI_DRM_DescriptorId ;


/***************************************************************************
Summary:
Structure containing the contents of DRM Packet.
****************************************************************************/
typedef struct BAVC_HDMI_DRMInfoFrame
{
    bool    DRM_PacketReceived ;

    /* fields from DRM Packet */
    BAVC_HDMI_DRM_EOTF eEOTF ;  /* Electro-Optical Transfer Function */
    BAVC_HDMI_DRM_DescriptorId eDescriptorId ;
    struct {
        struct {
            uint16_t X ;
            uint16_t Y ;
        } DisplayPrimaries[3] ;

        struct {
            uint16_t X ;
            uint16_t Y ;
        } WhitePoint ;

        struct {
            uint16_t Max ;
            uint16_t Min ;
        } DisplayMasteringLuminance ;

        uint16_t MaxContentLightLevel ;
        uint16_t MaxFrameAverageLightLevel ;
    } Type1 ;

} BAVC_HDMI_DRMInfoFrame ;


#define BAVC_HDMI_CEC_MAX_MSGLENGTH        32
#define BAVC_HDMI_CEC_MAX_XMIT_LENGTH      15  /* +1 for CEC Header Length */

#define BAVC_HDMI_CEC_BROADCAST_ADDR       0x0F

/********************************* Typedefs ***********************************/
/******************************************************************************
Summary:
Enumeration Type of available logical addresses for attached HDMI devices.

Description:
Each Device in an HDMI chain is assigned a logical address;  This enumeration
table details all available logical addresses.

See Also:
        o BAVC_HDMI_CEC_RecDevices
        o BAVC_HDMI_CEC_StbDevices
        o BAVC_HDMI_CEC_DvdDevices

*******************************************************************************/
typedef enum BAVC_HDMI_CEC_AllDevices
{
        BAVC_HDMI_CEC_AllDevices_eTV = 0,
        BAVC_HDMI_CEC_AllDevices_eRecDevice1,        /* Addr for 1st Recording Device */
        BAVC_HDMI_CEC_AllDevices_eRecDevice2,           /* Addr for 2nd Recording Device */
        BAVC_HDMI_CEC_AllDevices_eSTB1,                         /* Addr for 1st SetTop Box Device */
        BAVC_HDMI_CEC_AllDevices_eDVD1,                         /* Addr for 1st DVD Device */
        BAVC_HDMI_CEC_AllDevices_eAudioSystem,          /* Addr for Audio Device */
        BAVC_HDMI_CEC_AllDevices_eSTB2,                         /* Addr for 2nd SetTop Box Device */
        BAVC_HDMI_CEC_AllDevices_eSTB3,                         /* Addr for 3rd SetTop Box Device */
        BAVC_HDMI_CEC_AllDevices_eDVD2,                         /* Addr for 2nd DVD Device */
        BAVC_HDMI_CEC_AllDevices_eRecDevice3,        /* Addr for 3rd Recording Device */
        BAVC_HDMI_CEC_AllDevices_eSTB4,             /* Reserved and cannot be used */
        BAVC_HDMI_CEC_AllDevices_eDVD3,                         /* Reserved and cannot be used */
        BAVC_HDMI_CEC_AllDevices_eRsvd3,                                /* Reserved and cannot be used */
        BAVC_HDMI_CEC_AllDevices_eRsvd4,                                /* Reserved and cannot be used */
        BAVC_HDMI_CEC_AllDevices_eFreeUse,                      /* Free Addr, use for any device */
        BAVC_HDMI_CEC_AllDevices_eUnRegistered = 15  /* UnRegistered Devices */
} BAVC_HDMI_CEC_AllDevices ;


/******************************************************************************
Summary:
Enumeration Type of available logical addresses for attached HDMI devices that
have Recording capability.

Description:
Each Device in an HDMI chain is assigned a logical address;  This enumeration
table details all available Recording Device logical addresses.


See Also:
        o BAVC_HDMI_CEC_AllDevices
        o BAVC_HDMI_CEC_StbDevices
        o BAVC_HDMI_CEC_DvdDevices

*******************************************************************************/
typedef enum BAVC_HDMI_CEC_RecDevices
{
        BAVC_HDMI_CEC_RecDevices_eRecDevice1 =  1,      /* Addr for 1st Recording Device */
        BAVC_HDMI_CEC_RecDevices_eRecDevice2 =  2,      /* Addr for 2nd Recording Device */
        BAVC_HDMI_CEC_RecDevices_eRecDevice3 =  9,      /* Addr for 3rd Recording Device */
        BAVC_HDMI_CEC_RecDevices_eFreeUse    = 14,      /* Addr for 4th Recording Device */
        BAVC_HDMI_CEC_RecDevices_eUnRegistered = 15     /* Addr for 5th Recording Device */
} BAVC_HDMI_CEC_RecDevices ;



/******************************************************************************
Summary:
Enumeration Type of available logical addresses for attached HDMI devices that
have STB capability.

Description:
Each Device in an HDMI chain is assigned a logical address;  This enumeration
table details all available STB logical addresses.


See Also:
        o BAVC_HDMI_CEC_AllDevices
        o BAVC_HDMI_CEC_RecDevices
        o BAVC_HDMI_CEC_DvdDevices

*******************************************************************************/
typedef enum BAVC_HDMI_CEC_StbDevices
{
        BAVC_HDMI_CEC_StbDevices_eSTB1         =  3,  /* Addr for 1st SetTop Box Device */
        BAVC_HDMI_CEC_StbDevices_eSTB2         =  6,     /* Addr for 2nd SetTop Box Device */
        BAVC_HDMI_CEC_StbDevices_eSTB3         =  7,     /* Addr for 3rd SetTop Box Device */
        BAVC_HDMI_CEC_StbDevices_eSTB4         =  10,    /* Addr for 3rd SetTop Box Device */
        BAVC_HDMI_CEC_StbDevices_eFreeUse      = 14,     /* Addr for 4th SetTop Box Device */
        BAVC_HDMI_CEC_StbDevices_eUnRegistered = 15      /* Addr for 5th SetTop Box Device */
} BAVC_HDMI_CEC_StbDevices ;



/******************************************************************************
Summary:
Enumeration Type of available logical addresses for attached HDMI devices that
have DVD capability.

Description:
Each Device in an HDMI chain is assigned a logical address;  This enumeration
table details all available DVD logical addresses.


See Also:
        o BAVC_HDMI_CEC_AllDevices
        o BAVC_HDMI_CEC_RecDevices
        o BAVC_HDMI_CEC_StbDevices

*******************************************************************************/
typedef enum BAVC_HDMI_CEC_DvdDevices
{
        BAVC_HDMI_CEC_DvdDevices_eDVD1        =  4, /* Available Logical DVD 1 */
        BAVC_HDMI_CEC_DvdDevices_eDVD2           =  8, /* Available Logical DVD 2 */
        BAVC_HDMI_CEC_DvdDevices_eDVD3           =  11, /* Available Logical DVD 2 */
        BAVC_HDMI_CEC_DvdDevices_eFREE_USE       = 14, /* Use FreeUse for Logical DVD */
        BAVC_HDMI_CEC_DvdDevices_eUNREISTERED = 15  /* Use UnRegistered for Logical DVD */
} BAVC_HDMI_CEC_DvdDevices ;


/******************************************************************************
Summary:
Enumeration Type of available CEC interrupts

Description:


See Also:
        o BAVC_HDMI_CEC_GetMsgType
        o BAVC_HDMI_CEC_RecDevices
        o BAVC_HDMI_CEC_StbDevices

*******************************************************************************/
typedef enum BAVC_HDMI_CEC_IntMessageType
{
        BAVC_HDMI_CEC_IntMessageType_eTransmit = 0,
        BAVC_HDMI_CEC_IntMessageType_eReceive = 1
} BAVC_HDMI_CEC_IntMessageType ;


/******************************************************************************
Summary:
HDMI CEC logical address state machine
*******************************************************************************/
typedef enum BAVC_HDMI_CEC_SM
{
        BAVC_HDMI_CEC_SM_eInitAddrSearch,
        BAVC_HDMI_CEC_SM_eNextAddrSearch,
        BAVC_HDMI_CEC_SM_eReady
} BAVC_HDMI_CEC_SM ;


/******************************************************************************
Summary:
Data structure containing information of a CEC message
*******************************************************************************/
typedef struct BAVC_HDMI_CEC_MessageData
{
        uint8_t initiatorAddr ;
        uint8_t destinationAddr ;
        uint8_t messageBuffer[16] ;
        uint8_t messageLength ;

} BAVC_HDMI_CEC_MessageData ;


typedef struct  _HDMI_VideoFormat{
        uint8_t uHsyncPolarity;/*Hsync Polarity*/
        uint16_t uHsyncPulsePixels;/*The number of pixels in the horizontal sync pulse*/
        uint16_t uHsyncBackporchPixels;/*The number of pixels in the horizontal back porch*/


        uint16_t uHorizontalActivePixels; /*The number of active pixels on a horizontal line*/
        uint16_t uHorizontalFrontporchPixels;/*The number of pixels in the horizontal front porch*/

        uint8_t uDelayedFieldVersion;/*Delayed version of UUT_FIELD. Updates on the first active line following a vertical blank*/
        uint8_t uProgressive;/*This bit indicates that the format has been determined to be a progressive format*/
        uint8_t uInterlaced; /*This bit indicates that the format has been determined to be an interlaced format*/
        uint8_t uVsyncPolarity;/*The polarity of the vertical sync*/
        uint8_t uField;/*This denotes the currently active field, It is updated every leading edge of the vertical sync*/
        uint8_t uVerticalDe;/*vertical de*/
        uint16_t uVerticalFrontporchLinesField1;/*The integer number of lines in the vertical front porch of field 1*/
        uint16_t uVerticalSyncpulseLinesField1;/*The number of lines in the vertical sync pulse of field 1*/

        uint16_t uVerticalBackporchLinesField1; /*The number of lines in the vertical back porch of field 1*/
        uint16_t uVerticalSyncPulsePixelsField1;/*The number of pixels the leading edge of the vertical sync pulse is offset from the horizontal sync pulse in field 1*/


        uint16_t uActiveLinesField2;/*The number of active lines of video in field 2*/
        uint16_t uVerticalFrontPorchlinesField2; /*The integer number of lines in the vertical front porch of field 2*/


        uint16_t uVerticalBackporchLinesField2;/*The number of lines in the vertical back porch of field 2*/
        uint16_t uVerticalSyncPulsePixelsField2;/*The number of pixels the leading edge of the vertical sync pulse is offset from the horizontal sync pulse in field 2*/

        uint16_t uVdeCopy;/*Copy of UUT_VDE*/
        uint16_t uCurrentReveivedVideoLine;/*The current line of video being recieved, Line 1 is the first line of active video, The count continues into the blanking period*/


        uint16_t uActivelinesField1;/*The number of active lines of video in field 1*/
        uint16_t uVerticalSyncPulseLinesField2;/*The number of lines in the vertical sync pulse of field 2*/


}BAVC_HDMI_VideoFormat;


/******************************************************************************
Summary:
Data structure containing pointer to EDID and the number of EDID bytes
*******************************************************************************/
typedef struct
{
    uint8_t *data ;
    uint16_t numBytes ;
} BAVC_HDMI_EDID ;


/******************************************************************************
Summary:
Get Default EDID
*******************************************************************************/
void BAVC_HDMI_GetDefaultEDID(BAVC_HDMI_EDID *stEdid) ;


/**** AVI InfoFrame Conversions ****/

const char * BAVC_HDMI_AviInfoFrame_ColorspaceToStr_isrsafe(
        BAVC_HDMI_AviInfoFrame_Colorspace uiY1Y0) ;

const char * BAVC_HDMI_AviInfoFrame_ActiveFormatToStr(
        BAVC_HDMI_AviInfoFrame_ActiveInfo uiA0) ;

const char * BAVC_HDMI_AviInfoFrame_BarInfoToStr(
        BAVC_HDMI_AviInfoFrame_BarInfo uiB1B0) ;

const char * BAVC_HDMI_AviInfoFrame_ScanInfoToStr(
        BAVC_HDMI_AviInfoFrame_ScanInfo uiS1S0) ;

const char * BAVC_HDMI_AviInfoFrame_ColorimetryToStr(
        BAVC_HDMI_AviInfoFrame_Colorimetry uiC1C0) ;

const char * BAVC_HDMI_AviInfoFrame_PictureAspectRatioToStr(
        BAVC_HDMI_AviInfoFrame_PictureAspectRatio uiM1M0) ;

const char * BAVC_HDMI_AviInfoFrame_ActiveFormatAspectRatioToStr(
        BAVC_HDMI_AviInfoFrame_ActiveFormatAspectRatio uiR3_R0) ;

const char * BAVC_HDMI_AviInfoFrame_ITContentToStr(
        BAVC_HDMI_AviInfoFrame_ITContent uiITC) ;

const char * BAVC_HDMI_AviInfoFrame_ExtendedColorimetryToStr(
        BAVC_HDMI_AviInfoFrame_ExtendedColorimetry uiEC2_EC0) ;

const char * BAVC_HDMI_AviInfoFrame_RGBQuantizationRangeToStr(
        BAVC_HDMI_AviInfoFrame_RGBQuantizationRange uiQ1Q0) ;

const char * BAVC_HDMI_AviInfoFrame_ScalingToStr(
        BAVC_HDMI_AviInfoFrame_Scaling uiSC1SC0) ;

const char * BAVC_HDMI_AviInfoFrame_VideoIdCodeToStr_isrsafe(
        BAVC_HDMI_AviInfoFrame_Scaling uiVIC) ;
/* backwards compatibility */
#define BAVC_HDMI_AviInfoFrame_VideoIdCodeToStr BAVC_HDMI_AviInfoFrame_VideoIdCodeToStr_isrsafe

const char * BAVC_HDMI_AviInfoFrame_ContentTypeToStr(
        BAVC_HDMI_AviInfoFrame_ContentType eContentType) ;

const char * BAVC_HDMI_AviInfoFrame_YccQuantizationRangeToStr(
        BAVC_HDMI_AviInfoFrame_ContentType eYccQuantizationRange) ;


/**** Audio InfoFrame Conversions ****/

const char * BAVC_HDMI_AudioInfoFrame_CodingTypeToStr(
        BAVC_HDMI_AudioInfoFrame_CodingType uiCT3_CT0) ;

const char * BAVC_HDMI_AudioInfoFrame_ChannelCountToStr(
        BAVC_HDMI_AudioInfoFrame_ChannelCount uiCC2_CC0) ;

const char * BAVC_HDMI_AudioInfoFrame_SampleFrequencyToStr(
        BAVC_HDMI_AudioInfoFrame_SampleFrequency uiSF2_SF0) ;

const char * BAVC_HDMI_AudioInfoFrame_SampleSizeToStr(
        BAVC_HDMI_AudioInfoFrame_SampleSize uiSS1SS0) ;

const char * BAVC_HDMI_AudioInfoFrame_LevelShiftToStr(
        BAVC_HDMI_AudioInfoFrame_LevelShift uiLSV3_LSV0);

const char * BAVC_HDMI_AudioInfoFrame_DownMixInhibitToStr(
        BAVC_HDMI_AudioInfoFrame_DownMixInhibit uiDM) ;


/**** Source Product Description (SPD) InfoFrame Conversions ****/

const char * BAVC_HDMI_SpdInfoFrame_SourceTypeToStr(
        BAVC_HDMI_SpdInfoFrame_SourceType eSourceType) ;


/**** Vendor Specific (VS) InfoFrame Conversions ****/

const char * BAVC_HDMI_VsInfoFrame_HdmiVicToStr(
        BAVC_HDMI_VSInfoFrame_HDMIVIC  eHdmiVic) ;

const char * BAVC_HDMI_VsInfoFrame_3DStructureToStr(
        BAVC_HDMI_VSInfoFrame_3DStructure e3DStructType) ;

const char * BAVC_HDMI_VsInfoFrame_3DExtDataToStr(
        BAVC_HDMI_VSInfoFrame_3DExtData e3DExtType) ;

const char * BAVC_HDMI_VsInfoFrame_HdmiVideoFormatToStr_isrsafe(
        BAVC_HDMI_VSInfoFrame_HDMIVideoFormat eFormatType) ;
/* backwards compatibility */
#define BAVC_HDMI_VsInfoFrame_HdmiVideoFormatToStr BAVC_HDMI_VsInfoFrame_HdmiVideoFormatToStr_isrsafe


/**** Dynamic Range and Mastering InfoFrame (HDR) Conversions ****/
const char * BAVC_HDMI_DRMInfoFrame_EOTFToStr(
    BAVC_HDMI_DRM_EOTF eEOTF) ;

const char * BAVC_HDMI_DRMInfoFrame_DescriptorIdToStr(
    BAVC_HDMI_DRM_DescriptorId eDescriptorId) ;



const char * BAVC_HDMI_PacketTypeToStr_isrsafe(
        BAVC_HDMI_PacketType ePacketType) ;


const char * BAVC_HDMI_BitsPerPixelToStr(
        BAVC_HDMI_BitsPerPixel eBitsPerPixel) ;


unsigned int BAVC_HDMI_HDCP_NumberOfSetBits(const unsigned char *bytes, int nbytes) ;

#endif
