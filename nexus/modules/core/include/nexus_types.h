/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 **************************************************************************/
#ifndef NEXUS_TYPES_H__
#define NEXUS_TYPES_H__

#include "nexus_base_types.h"
#include "nexus_audio_types.h"
#include "nexus_video_types.h"
#include "nexus_security_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
An InputBand is a digital input to the transport block.
It takes data from outside the chip and routes it to a parser band.

Description:
See Also:
NEXUS_InputBand_SetSettings
**/
typedef unsigned long NEXUS_InputBand;
#define NEXUS_InputBand_e0 (0)
#define NEXUS_InputBand_e1 (1)
#define NEXUS_InputBand_e2 (2)
#define NEXUS_InputBand_e3 (3)
#define NEXUS_InputBand_e4 (4)
#define NEXUS_InputBand_e5 (5)
#define NEXUS_InputBand_e6 (6)
#define NEXUS_InputBand_e7 (7)
#define NEXUS_InputBand_e8 (8)
#define NEXUS_InputBand_e9 (9)
#define NEXUS_InputBand_e10 (10)
#define NEXUS_InputBand_e11 (11)
#define NEXUS_InputBand_e12 (12)
#define NEXUS_InputBand_e13 (13)
#define NEXUS_InputBand_e14 (14)
#define NEXUS_InputBand_e15 (15)
#define NEXUS_InputBand_eMax NEXUS_NUM_INPUT_BANDS

/**
Summary:
An ParserBand is a digital input to the transport block
which takes data from an input band and routes it to various PidChannels.

Description:
Nexus supports playback parser bands through the Playpump and Playback interfaces.

See Also:
NEXUS_ParserBand_SetSettings
**/
typedef unsigned long NEXUS_ParserBand;
#define NEXUS_ParserBand_eInvalid  ((NEXUS_ParserBand)(-1))
#define NEXUS_ParserBand_e0 (0)
#define NEXUS_ParserBand_e1 (1)
#define NEXUS_ParserBand_e2 (2)
#define NEXUS_ParserBand_e3 (3)
#define NEXUS_ParserBand_e4 (4)
#define NEXUS_ParserBand_e5 (5)
#define NEXUS_ParserBand_e6 (6)
#define NEXUS_ParserBand_e7 (7)
#define NEXUS_ParserBand_e8 (8)
#define NEXUS_ParserBand_e9 (9)
#define NEXUS_ParserBand_e10 (10)
#define NEXUS_ParserBand_e11 (11)
#define NEXUS_ParserBand_e12 (12)
#define NEXUS_ParserBand_e13 (13)
#define NEXUS_ParserBand_e14 (14)
#define NEXUS_ParserBand_e15 (15)
#define NEXUS_ParserBand_e16 (16)
#define NEXUS_ParserBand_e17 (17)
#define NEXUS_ParserBand_e18 (18)
#define NEXUS_ParserBand_e19 (19)
#define NEXUS_ParserBand_e20 (20)
#define NEXUS_ParserBand_e21 (21)
#define NEXUS_ParserBand_e22 (22)
#define NEXUS_ParserBand_e23 (23)
#define NEXUS_ParserBand_eMax (24)

/**
Summary:
The transport format, or stream type, of digital data.

Description:
This includes a variety of container classes and standard stream muxes.

See Also:
NEXUS_PidChannelSettings
NEXUS_StcChannelSettings
**/
typedef enum NEXUS_TransportType {
    NEXUS_TransportType_eUnknown,   /* Unknown stream format */
    NEXUS_TransportType_eEs,        /* Elementary stream. No container or muxing. */
    NEXUS_TransportType_eTs,        /* MPEG2 transport stream */
    NEXUS_TransportType_eMpeg2Pes,  /* MPEG2 packetized elementary stream, this includes MPEG2 Program Stream  streams */
    NEXUS_TransportType_eVob,       /* DVD VOB, this is subset of MPEG2 Program Stream, special processing is applied for VOB streams */
    NEXUS_TransportType_eMpeg1Ps,   /* MPEG1 program stream */
    NEXUS_TransportType_eDssEs,     /* DSS with ES payload (used for SD) */
    NEXUS_TransportType_eDssPes,    /* DSS with PES payload (used for HD) */
    NEXUS_TransportType_eAsf,       /* Advanced Systems Format */
    NEXUS_TransportType_eAvi,       /* Audio Video Interleave */
    NEXUS_TransportType_eMp4,       /* MP4 (MPEG-4 Part12) container */
    NEXUS_TransportType_eFlv,       /* Flash video container */
    NEXUS_TransportType_eMkv,       /* Matroska container */
    NEXUS_TransportType_eWav,       /* WAVE audio container */
    NEXUS_TransportType_eMp4Fragment,  /* separate 'moof' boxes from the MP4 (MPEG-4 Part12) container */
    NEXUS_TransportType_eRmff,       /* RMFF container */
    NEXUS_TransportType_eOgg,        /* OGG container */
    NEXUS_TransportType_eFlac,       /* FLAC encapsulation */
    NEXUS_TransportType_eAiff,       /* AIFF (Audio Interchange File Format) container */
    NEXUS_TransportType_eApe,        /* Monkey's Audio container */
    NEXUS_TransportType_eAmr,        /* AMR audio ES format - RFC4867 */
    NEXUS_TransportType_eBulk,       /* Unformatted data for security use. No container or muxing. */
    NEXUS_TransportType_eMax
} NEXUS_TransportType;

/**
Summary:
A transport timestamp is a value prepended to every transport packet for pacing control.

Description:
See Also:
NEXUS_ParserBandSettings
NEXUS_PlaypumpSettings
**/
typedef enum NEXUS_TransportTimestampType {
    NEXUS_TransportTimestampType_eNone,   /* No timestamp is prepended to the transport packets. */

    NEXUS_TransportTimestampType_e30_2U_Mod300, /* 30 bit timestamp, mod-300 value, 2 user bits. no parity bits.
                                             A 4-byte 27MHz timestamp is prepended to every transport packet.
                                             Used for MPEG2TS streams (188 byte packets).
                                             The 27Mhz timestamp is divided by 300 to convert to a 90Hz timestamp before use
                                             because MPEG2TS PTS/PCR units are 90Khz. */
    NEXUS_TransportTimestampType_eMod300 = NEXUS_TransportTimestampType_e30_2U_Mod300, /* alias */

    NEXUS_TransportTimestampType_e30_2U_Binary, /* 30 bit timestamp, binary value, 2 user bits. no parity bits.
                                             A 4-byte 27MHz timestamp is prepended to every transport packet.
                                             Used for DSS streams (130 byte packets).
                                             The 27Mz timestamp is used unmodified because DSS PTS/PCR units are 27Mhz. */
    NEXUS_TransportTimestampType_eBinary = NEXUS_TransportTimestampType_e30_2U_Binary, /* alias */

    NEXUS_TransportTimestampType_e32_Mod300, /* 32 bit timestamp, mod-300 value, no user bits. no parity bits. */
    NEXUS_TransportTimestampType_e32_Binary, /* 32 bit timestamp, binary value, no user bits. no parity bits. */
    /* e28_4P_Binary is not supported */
    NEXUS_TransportTimestampType_e28_4P_Mod300, /* 28 bit timestamp, 4 bit parity, mod-300 value */

    NEXUS_TransportTimestampType_eMax
} NEXUS_TransportTimestampType;

/**
Summary:
Maximum number of MTSIF channels, for either backend or frontend.

Description:
This is the larger of the number of MTSIF channels on either the backend or frontend chip.

It was moved here because it is used both by the transport module and the frontend module.
**/
#define NEXUS_MAX_MTSIF        4
#define NEXUS_MAX_HDMI_OUTPUTS 2

/**
Summary:
A Timebase provides a clock for various cores like decoders, display and mixers.

Description:
See Also:
NEXUS_Timebase_SetSettings
**/
typedef unsigned long NEXUS_Timebase;
#define NEXUS_Timebase_eInvalid ((NEXUS_Timebase)(-1))
#define NEXUS_Timebase_e0 (0)
#define NEXUS_Timebase_e1 (1)
#define NEXUS_Timebase_e2 (2)
#define NEXUS_Timebase_e3 (3)
#define NEXUS_Timebase_e4 (4)
#define NEXUS_Timebase_e5 (5)
#define NEXUS_Timebase_e6 (6)
#define NEXUS_Timebase_e7 (7)
#define NEXUS_Timebase_e8 (8)
#define NEXUS_Timebase_e9 (9)
#define NEXUS_Timebase_e10 (10)
#define NEXUS_Timebase_e11 (11)
#define NEXUS_Timebase_e12 (12)
#define NEXUS_Timebase_e13 (13)
#define NEXUS_Timebase_eMax (14)

/**
Summary:
Handle for a frontend connection
**/
typedef struct NEXUS_FrontendConnector *NEXUS_FrontendConnectorHandle;

/*
Summary:
Handle which represents a remux channel.
*/
typedef struct NEXUS_Remux *NEXUS_RemuxHandle;

/**
Summary:
Cable card type
**/
typedef enum NEXUS_CableCardType
{
    NEXUS_CableCardType_eNone,
    NEXUS_CableCardType_eSCard,
    NEXUS_CableCardType_eMCard
} NEXUS_CableCardType;

/**
Summary:
Describes the origin of a PTS value.

Description:
See Also:
NEXUS_VideoDecoderStatus
NEXUS_AudioDecoderStatus
**/
typedef enum NEXUS_PtsType
{
    NEXUS_PtsType_eCoded,                     /* PTS value came from the stream. */
    NEXUS_PtsType_eInterpolatedFromValidPTS,  /* PTS has been interpolated over time from a valid (i.e. coded) PTS from the stream. */
    NEXUS_PtsType_eHostProgrammedPTS,         /* PTS was set by the application and did not come from the stream. */
    NEXUS_PtsType_eInterpolatedFromInvalidPTS /* PTS has been interpolated over time from an invalid value. */
} NEXUS_PtsType;

/**
Summary:
Describes type of data in a PID.

Description:
See Also:
NEXUS_RecpumpPidChannelSettings
NEXUS_PlaypumpAddPidChannelSettings
**/
typedef enum NEXUS_PidType {
    NEXUS_PidType_eVideo,            /* Used to indicate video stream */
    NEXUS_PidType_eAudio,            /* Used to indicate audio stream */
    NEXUS_PidType_eOther,            /* Used to indicate stream type different from above. Useful for PCR, PSI or other non-audio/video data. */
    NEXUS_PidType_eUnknown = NEXUS_PidType_eOther /* Unknown pid type */
} NEXUS_PidType;

/*
Summary:
For non-palettized surfaces, NEXUS_Pixel is an ARGB8888 pixel.
When used, it will be converted into the desired pixel for a surface.
For palettized surfaces, this is the palette index plus possible alpha value.

Description:
For palette indexes, values should be packed into the least significant bits
of NEXUS_Pixel. For instance, a8_palette8 uses byte[0] for the palette8 index,
byte[1] for alpha8 value and ignores byte[2] and byte[3].
palette2 uses bits 0 and 1 of byte[0] and ignores the rest.

Used in NEXUS_SurfaceMemory, NEXUS_SurfaceSettings, NEXUS_DisplaySettings, and NEXUS_GraphicsSettings.
*/
typedef uint32_t NEXUS_Pixel;

/**
Summary:
Base rate for normal decode used to specify trick mode rates.

Description:
This is used for both audio and video trick modes.
See NEXUS_VideoDecoderTrickState and NEXUS_AudioDecoderTrickState.
See comments in NEXUS_VideoDecoderTrickState.rate for how to use a multiple or fraction of this value to specify trick mode rates.
**/
#define NEXUS_NORMAL_DECODE_RATE 1000

/*
Summary:
Pixel format of a graphics surface or video buffer

Description:
The enum name describes the pixel format:

o A = alpha channel
o X = block out alpha channel
o R = red component of color
o G = green component of color
o B = blue component of color

YCbCr444 means each sample has a Y, a Cb, and a Cr value, so Y:Cb:Cr = 1:1:1 in stream.
YCbCr422 means for every two horizontal Y samples, there is one Cb and Cr sample, so Y:Cb:Cr = 2:1:1 in stream.
10-bit means each Y/Cb/Cr value is 10 bits, 8-bit means each Y/Cb/Cr value is 8 bits.
For example,
YCbCr444 10bit, each sample has Y(10bit)+Cb(10bit)+Cr(10bit) = 30 bits
YCbCr422 8bit, each sample has either Y(8bit)+Cb(8bit)=16bit or Y(8bit)+Cr(8bit) = 16 bits

R8 G8 B8 means 8 bits of red component (R), 8 bits of green component (G) and 8 bits of blue component (B) that are 24-bit packed.
X8 R8 G8 B8 means 8 bits of nothing (X), 8 bits of red component (R), 8 bits of green component (G) and 8 bits of blue component (B)
that occupy a 32-bit word.

There is no pixel format for 4:2:0. The video decoder outputs 4:2:0 using separate luma (eY8) and chroma buffers (eCb8_Cr8), which also happen to
be in a striped format. See NEXUS_StripedSurfaceHandle.

These values match BM2MC_PACKET_PixelFormat. They do not match BPXL_Format.

See Also:
NEXUS_SurfaceCreateSettings
*/
typedef enum NEXUS_PixelFormat
{
    NEXUS_PixelFormat_eUnknown = 0,

    NEXUS_PixelFormat_eR5_G6_B5,         /* 16-bit, no per-pixel alpha */
    NEXUS_PixelFormat_eB5_G6_R5,         /* 16-bit, no per-pixel alpha */

    NEXUS_PixelFormat_eA1_R5_G5_B5,      /* 16-bit */
    NEXUS_PixelFormat_eX1_R5_G5_B5,      /* 16-bit */
    NEXUS_PixelFormat_eA1_B5_G5_R5,      /* 16-bit */
    NEXUS_PixelFormat_eX1_B5_G5_R5,      /* 16-bit */
    NEXUS_PixelFormat_eR5_G5_B5_A1,      /* 16-bit */
    NEXUS_PixelFormat_eR5_G5_B5_X1,      /* 16-bit */
    NEXUS_PixelFormat_eB5_G5_R5_A1,      /* 16-bit */
    NEXUS_PixelFormat_eB5_G5_R5_X1,      /* 16-bit */

    NEXUS_PixelFormat_eA4_R4_G4_B4,      /* 16-bit */
    NEXUS_PixelFormat_eX4_R4_G4_B4,      /* 16-bit */
    NEXUS_PixelFormat_eA4_B4_G4_R4,      /* 16-bit */
    NEXUS_PixelFormat_eX4_B4_G4_R4,      /* 16-bit */
    NEXUS_PixelFormat_eR4_G4_B4_A4,      /* 16-bit */
    NEXUS_PixelFormat_eR4_G4_B4_X4,      /* 16-bit */
    NEXUS_PixelFormat_eB4_G4_R4_A4,      /* 16-bit */
    NEXUS_PixelFormat_eB4_G4_R4_X4,      /* 16-bit */

    NEXUS_PixelFormat_eA8_R8_G8_B8,      /* 32-bit */
    NEXUS_PixelFormat_eX8_R8_G8_B8,      /* 32-bit */
    NEXUS_PixelFormat_eA8_B8_G8_R8,      /* 32-bit */
    NEXUS_PixelFormat_eX8_B8_G8_R8,      /* 32-bit */
    NEXUS_PixelFormat_eR8_G8_B8_A8,      /* 32-bit */
    NEXUS_PixelFormat_eR8_G8_B8_X8,      /* 32-bit */
    NEXUS_PixelFormat_eB8_G8_R8_A8,      /* 32-bit */
    NEXUS_PixelFormat_eB8_G8_R8_X8,      /* 32-bit */

    NEXUS_PixelFormat_eA8,               /* 8-bit alpha-only surface (constant color) */
    NEXUS_PixelFormat_eA4,               /* 4-bit alpha-only surface (constant color) */
    NEXUS_PixelFormat_eA2,               /* 2-bit alpha-only surface (constant color) */
    NEXUS_PixelFormat_eA1,               /* 1-bit alpha-only surface (constant color) */

    NEXUS_PixelFormat_eW1,

    NEXUS_PixelFormat_eA8_Palette8,      /* A8 and 8-bit Palette index */
    NEXUS_PixelFormat_ePalette8,         /* 8-bit Palette index */
    NEXUS_PixelFormat_ePalette4,         /* 4-bit palette index */
    NEXUS_PixelFormat_ePalette2,         /* 2-bit palette index */
    NEXUS_PixelFormat_ePalette1,         /* 1-bit palette index */

    NEXUS_PixelFormat_eY8_Palette8,      /* Y8 and 8-bit Palette index */
    NEXUS_PixelFormat_eA8_Y8,            /* 8-bit alpha and 8-bit luma */

    NEXUS_PixelFormat_eCb8,              /* 8-bit chroma-only (Cb) */
    NEXUS_PixelFormat_eCr8,              /* 8-bit chroma-only (Cr) */

    NEXUS_PixelFormat_eY8,               /* Y8, typically for 4:2:0 */
    NEXUS_PixelFormat_eCb8_Cr8,          /* chroma w/ 8-bit values, typically for 4:2:0 */
    NEXUS_PixelFormat_eCr8_Cb8,          /* chroma w/ 8-bit values, typically for 4:2:0 */

    NEXUS_PixelFormat_eY10,              /* Y10, typically for 4:2:0 */
    NEXUS_PixelFormat_eCb10_Cr10,        /* chroma w/ 10-bit values, typically for 4:2:0 */
    NEXUS_PixelFormat_eCr10_Cb10,        /* chroma w/ 10-bit values, typically for 4:2:0 */

    NEXUS_PixelFormat_eY08_Cb8_Y18_Cr8,  /* 32-bit for 2 pixels, YCbCr422 w/ 8-bit values */
    NEXUS_PixelFormat_eY08_Cr8_Y18_Cb8,  /* 32-bit for 2 pixels, YCbCr422 w/ 8-bit values */
    NEXUS_PixelFormat_eY18_Cb8_Y08_Cr8,  /* 32-bit for 2 pixels, YCbCr422 w/ 8-bit values */
    NEXUS_PixelFormat_eY18_Cr8_Y08_Cb8,  /* 32-bit for 2 pixels, YCbCr422 w/ 8-bit values */
    NEXUS_PixelFormat_eCb8_Y08_Cr8_Y18,  /* 32-bit for 2 pixels, YCbCr422 w/ 8-bit values */
    NEXUS_PixelFormat_eCb8_Y18_Cr8_Y08,  /* 32-bit for 2 pixels, YCbCr422 w/ 8-bit values */
    NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08,  /* 32-bit for 2 pixels, YCbCr422 w/ 8-bit values */
    NEXUS_PixelFormat_eCr8_Y08_Cb8_Y18,  /* 32-bit for 2 pixels, YCbCr422 w/ 8-bit values */

    NEXUS_PixelFormat_eX2_Cr10_Y10_Cb10, /* 32-bit for 1 pixel,  YCbCr444 w/ 10-bit values */

    NEXUS_PixelFormat_eA8_Y8_Cb8_Cr8, /* YCbCr444 w/ 8-bit values */
    NEXUS_PixelFormat_eA8_Cr8_Cb8_Y8, /* YCbCr444 w/ 8-bit values */
    NEXUS_PixelFormat_eCr8_Cb8_Y8_A8, /* YCbCr444 w/ 8-bit values */
    NEXUS_PixelFormat_eY8_Cb8_Cr8_A8, /* YCbCr444 w/ 8-bit values */

    NEXUS_PixelFormat_eY010_Cb10_Y110_Cr10,   /* YCbCr422 w/ 10-bit values */
    NEXUS_PixelFormat_eY010_Cr10_Y110_Cb10,   /* YCbCr422 w/ 10-bit values */
    NEXUS_PixelFormat_eY110_Cb10_Y010_Cr10,   /* YCbCr422 w/ 10-bit values */
    NEXUS_PixelFormat_eY110_Cr10_Y010_Cb10,   /* YCbCr422 w/ 10-bit values */
    NEXUS_PixelFormat_eCb10_Y010_Cr10_Y110,   /* YCbCr422 w/ 10-bit values */
    NEXUS_PixelFormat_eCb10_Y110_Cr10_Y010,   /* YCbCr422 w/ 10-bit values */
    NEXUS_PixelFormat_eCr10_Y110_Cb10_Y010,   /* YCbCr422 w/ 10-bit values */
    NEXUS_PixelFormat_eCr10_Y010_Cb10_Y110,   /* YCbCr422 w/ 10-bit values */

    NEXUS_PixelFormat_eL8,                    /* 8-bit luma */
    NEXUS_PixelFormat_eL4_A4,                 /* 4-bit luma and 4-bit alpha */
    NEXUS_PixelFormat_eL8_A8,                 /* 8-bit luma and 8-bit alpha */
    NEXUS_PixelFormat_eL15_L05_A6,            /* 5-bit luma1, 5-bit luma0, 6-bit alpha */

    NEXUS_PixelFormat_eCompressed_A8_R8_G8_B8, /* compressed ARGB_8888 */
    NEXUS_PixelFormat_eUIF_R8_G8_B8_A8,        /* UIF [V3D texture layout] RGBA_8888 */

    /* The following pixel formats are not supported by NEXUS_Graphics2D */
    NEXUS_PixelFormat_eR8_G8_B8,         /* 24-bit packet */
    NEXUS_PixelFormat_eYCbCr422_10bit,

    NEXUS_PixelFormat_eMax
} NEXUS_PixelFormat;

/**
Summary:
Palette test macro for NEXUS_PixelFormat type
**/
#define NEXUS_PIXEL_FORMAT_IS_PALETTE(pixelformat) \
     ((pixelformat) >= NEXUS_PixelFormat_eA8_Palette8 && (pixelformat) <= NEXUS_PixelFormat_eY8_Palette8)

/**
Summary:
YCrCb test macro for NEXUS_PixelFormat type
**/
#define NEXUS_PIXEL_FORMAT_IS_YCRCB(pixelformat) \
    (((pixelformat) >= NEXUS_PixelFormat_eA8_Y8 && (pixelformat) <= NEXUS_PixelFormat_eCr10_Y010_Cb10_Y110) || \
     ((pixelformat)== NEXUS_PixelFormat_eYCbCr422_10bit))

/**
Summary:
422 format test macro for NEXUS_PixelFormat type
**/
#define NEXUS_PIXEL_FORMAT_IS_422(pixelFormat) \
    ((NEXUS_PixelFormat_eY08_Cb8_Y18_Cr8 <= pixelFormat && \
      NEXUS_PixelFormat_eCr8_Y08_Cb8_Y18 >= pixelFormat) || \
     (NEXUS_PixelFormat_eY010_Cb10_Y110_Cr10 <= pixelFormat && \
      NEXUS_PixelFormat_eCr10_Y010_Cb10_Y110 >= pixelFormat) || \
     (NEXUS_PixelFormat_eYCbCr422_10bit == pixelFormat))

/**
Summary:
444 format test macro for NEXUS_PixelFormat type
**/
#define NEXUS_PIXEL_FORMAT_IS_444(pixelFormat) \
    (NEXUS_PixelFormat_eX2_Cr10_Y10_Cb10 <= pixelFormat && \
     NEXUS_PixelFormat_eY8_Cb8_Cr8_A8    >= pixelFormat)

/***************************************************************************
Summary:
Buffer count and pixel types per buffer type (fullHd, hd or sd) used in NEXUS_DisplayHeapSettings and NEXUS_DisplayModuleSettings
***************************************************************************/
typedef struct NEXUS_DisplayBufferTypeSettings
{
    unsigned count;                 /* Number of full-sized buffers to allocate */
    unsigned pipCount;              /* Number of PIP-sized buffers to allocate */
    NEXUS_PixelFormat pixelFormat;  /* Pixel format of the buffers */
    NEXUS_VideoFormat format;       /* Max-sized format of the buffer (e.g. 1080i or 1080p? 480 or 576?) */
    unsigned additionalLines;       /* deprecated and unused */
} NEXUS_DisplayBufferTypeSettings;

/***************************************************************************
Summary:
Additional properties for Display module heap allocation

Description:
When the Display module does video buffer allocation, it needs to manage those allocations
in fullHD (i.e. 2HD), HD and SD sizes. This is needed to prevent fragmentation for highly dynamic
buffer allocation schemes.

If a heap is not used by the Display module, these settings are not used.

If a heap is used by the Display module in either NEXUS_VideoWindowSettings.heapIndex or NEXUS_VideoInputSettings.heapIndex,
the heap cannot also be used by any other part of Nexus.
The Display module will overlay its internal display heap over the whole heap.
***************************************************************************/
typedef struct NEXUS_DisplayHeapSettings
{
    NEXUS_DisplayBufferTypeSettings quadHdBuffers; /* quad-sized HD buffers used for 2160p output */
    NEXUS_DisplayBufferTypeSettings fullHdBuffers; /* double-sized HD buffers used for 1080p output */
    NEXUS_DisplayBufferTypeSettings hdBuffers;     /* HD buffers used for 1080i or 720p output */
    NEXUS_DisplayBufferTypeSettings sdBuffers;     /* SD buffers used for 480i/576i output */
} NEXUS_DisplayHeapSettings;

/**
Summary:
General purpose point
**/
typedef struct NEXUS_Point
{
    int x;
    int y;
} NEXUS_Point;

/**
Summary:
General purpose rectangle
**/
typedef struct NEXUS_Rect
{
    int16_t x;
    int16_t y;
    uint16_t width;
    uint16_t height;
} NEXUS_Rect;

/**
Summary:
Relative clip rectangle

Description:
Values are relative, specified in units of 1/100%.  For example, if you want 5%
clipping on all sides, use the value 500 in all fields.

Structure can be used with other units (e.g. pixels), but it should be explicitly
commented so where used and not here.
**/
typedef struct NEXUS_ClipRect
{
    unsigned left;
    unsigned right;
    unsigned top;
    unsigned bottom;
} NEXUS_ClipRect;

/**
Summary:
VBI data type enum

Description:
NEXUS_VbiDataType is general purpose and could be used in a variety of ways in different Nexus. It should not be modified for chip-specific use.

See NEXUS_ClosedCaptionData in this file as well as nexus_video_input_vbi.h and nexus_display_vbi.h for
other VBI-related API's.
**/
typedef enum NEXUS_VbiDataType {
    NEXUS_VbiDataType_eClosedCaption,
    NEXUS_VbiDataType_eSubtitle,
    NEXUS_VbiDataType_eTeletext,
    NEXUS_VbiDataType_eVps,
    NEXUS_VbiDataType_eGemStar,
    NEXUS_VbiDataType_eWss,
    NEXUS_VbiDataType_eCgms,
    NEXUS_VbiDataType_eMax
} NEXUS_VbiDataType;

/**
Summary:
One field of EIA-608 or EIA-708 closed caption data

Description:
See NEXUS_VideoInput_ReadClosedCaption and NEXUS_Display_WriteClosedCaption
**/
typedef struct NEXUS_ClosedCaptionData
{
    NEXUS_VbiDataType type; /* Can be eClosedCaption or eSubtitle. eSubtitle is only supported for DSS subtitles.
                               If eSubtitle, then 'field' is 'languageType'. */
    uint32_t pts;   /* only set if data comes from a digital source */
    uint16_t field; /* If 0/1, this is EIA-608. 0 = top field, 1 = bottom field.
                       If 2/3, this is EIA-708, 2 = DTVCC_PACKET_DATA cc_type, 3 = DTVCC_PACKET_START cc_type */
    bool noData;    /* If true, data[] contains no valid data.
                       For 608 sources, this is reported by the VBI decoder.
                       For 708 sources, noData is the negation of userdata's cc_valid field. Therefore, do not discard an entry
                         if noData=true. Instead, set your parser's cc_valid=!noData and pass the data along. */
    bool parityError;  /* If true, data[] contains a parity error. Parity is the 8th bit of each data[] element and can be computed using the lower 7 bits of data. */
    uint8_t data[2];
} NEXUS_ClosedCaptionData;


/* NEXUS_MEMORY_TYPE macros are used to control memory mapping.
Memory mapping requirements depend on the type of access in the driver and application (for kernel mode,
the driver is in the kernel and the application is in user space). */
#define NEXUS_MEMORY_TYPE_DRIVER_UNCACHED       0x0001
#define NEXUS_MEMORY_TYPE_DRIVER_CACHED         0x0002
#define NEXUS_MEMORY_TYPE_APPLICATION_CACHED    0x0004
#define NEXUS_MEMORY_TYPE_SECURE                0x0008

/* RESERVED = nexus heap is placeholder. no underlying BMEM heap created, so no allocations allowed. */
#define NEXUS_MEMORY_TYPE_RESERVED              0x0010


/* MANAGED = nexus heap is in managed mode, all allocations should be done using BMMA_Alloc/Free */
#define NEXUS_MEMORY_TYPE_MANAGED               0x0020

/* NOT_MAPPED = heap is not mapped, and no virtual address space allocated to the heap, must be used in conjunction with NEXUS_MEMORY_TYPE_MANAGED */
#define NEXUS_MEMORY_TYPE_NOT_MAPPED            0x0040

/* ONDEMAND_MAPPED = heap is not mapped, and virtual address space allocated when requested per allocation, must be used in conjunction with NEXUS_MEMORY_TYPE_MANAGED */
#define NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED       0x0080

/* HIGH MEMORY = heap could be allocated from memory that is outside of 4GB window, must be used in conjunction with NEXUS_MEMORY_TYPE_MANAGED */
#define NEXUS_MEMORY_TYPE_HIGH_MEMORY           0x0100

/* DYNAMIC = heap could dynamically change it's size based, acquiring and releasing memory to OS, must be used in conjunction with NEXUS_MEMORY_TYPE_NOT_MAPPED or NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED */
#define NEXUS_MEMORY_TYPE_DYNAMIC               0x0200

/* NEXUS_MEMORY_TYPE_SECURE heap has been toggled off with NEXUS_Platform_SetRuntimeSettings */
#define NEXUS_MEMORY_TYPE_SECURE_OFF            0x0400

/* NEXUS_MemoryType macros are used to assemble commonly used NEXUS_MEMORY_TYPE combinations */

/* cached and uncached CPU access from the driver only.
note that some driver code may only require cached or only required uncached.
the minimal memory mapping is preferred if it is known. */
#define NEXUS_MemoryType_eDriver        (NEXUS_MEMORY_TYPE_DRIVER_UNCACHED|NEXUS_MEMORY_TYPE_DRIVER_CACHED)

/* cached CPU access in application (user space). no CPU access in the driver. no uncached access in application.
can be used for graphics surfaces, record buffers, etc.
HW access is still allowed with an eApplication heap. for instance, you can allocate a graphics framebuffer from
an eApplication heap because only the app needs CPU access; the driver simply needs to program the offset to the GFD. */
#define NEXUS_MemoryType_eApplication   (NEXUS_MEMORY_TYPE_APPLICATION_CACHED)

/* full CPU access from driver and application */
#define NEXUS_MemoryType_eFull          (NEXUS_MEMORY_TYPE_DRIVER_UNCACHED|NEXUS_MEMORY_TYPE_DRIVER_CACHED|NEXUS_MEMORY_TYPE_APPLICATION_CACHED)

/* accessible by HW devices only. no CPU access in driver or application; therefore no memory
mapping required. */
#define NEXUS_MemoryType_eDeviceOnly    (0x0)

/* restricted access heap for secure processor. limited device access and no CPU access. */
#define NEXUS_MemoryType_eSecure        (NEXUS_MEMORY_TYPE_SECURE)

/* NEXUS_MemoryType_eDevice is deprecated */
#define NEXUS_MemoryType_eDevice NEXUS_MemoryType_eFull

typedef unsigned NEXUS_MemoryType;

/* NEXUS_HEAP_TYPE_XXX macros are used in NEXUS_Core_MemoryRegion.heapType and NEXUS_MemoryStatus.heapType */
#define NEXUS_HEAP_TYPE_MAIN                         0x00001
#define NEXUS_HEAP_TYPE_PICTURE_BUFFERS              0x00002
#define NEXUS_HEAP_TYPE_SAGE_RESTRICTED_REGION       0x00004
#define NEXUS_HEAP_TYPE_COMPRESSED_RESTRICTED_REGION 0x00008
/* GRAPHICS is used for off-screen graphics; not to be confused with GFD RTS */
#define NEXUS_HEAP_TYPE_GRAPHICS                     0x00010
/* SECONDARY_GRAPHICS is used for off-screen graphics which are not accessible by VC4; not to be confused with GFD RTS */
#define NEXUS_HEAP_TYPE_SECONDARY_GRAPHICS           0x00020
#define NEXUS_HEAP_TYPE_EXPORT_REGION                0x00040
#define NEXUS_HEAP_TYPE_SECURE_GRAPHICS              0x00080
#define NEXUS_HEAP_TYPE_PICTURE_BUFFER_EXT           0x00100
#define NEXUS_HEAP_TYPE_DTU                          0x00200


/***************************************************************************
Summary:
Callback function prototype used in NEXUS_CoreInterruptInterface.pConnectInterrupt
****************************************************************************/
typedef void (*NEXUS_Core_InterruptFunction)(void *, int);

/***************************************************************************
Summary:
VCXO-PLL Index

Description:
VCXO-PLLs connect to a NEXUS_Timebase and provide a reference clock to
NEXUS_AudioOutputPll objects as well as CCIR-656 outputs.

See Also:
NEXUS_Vcxo_SetSettings
****************************************************************************/
typedef enum NEXUS_Vcxo
{
    NEXUS_Vcxo_eInvalid=-1,
    NEXUS_Vcxo_e0=0,
    NEXUS_Vcxo_e1,
    NEXUS_Vcxo_e2,
    NEXUS_Vcxo_e3,
    NEXUS_Vcxo_eMax
} NEXUS_Vcxo;

typedef enum NEXUS_ModuleStandbyLevel {
    NEXUS_ModuleStandbyLevel_eAll,         /* Module is disabled in all standby modes */
    NEXUS_ModuleStandbyLevel_eActive,      /* Module is not disabled in Active Standby mode */
    NEXUS_ModuleStandbyLevel_eAlwaysOn,    /* Module is Always On in all modes */
    NEXUS_ModuleStandbyLevel_eMax
} NEXUS_ModuleStandbyLevel;

/***************************************************************************
Summary:
Settings common to all nexus modules

Description:
Should be added with name 'common'.
****************************************************************************/
typedef struct NEXUS_CommonModuleSettings
{
    NEXUS_ModuleStandbyLevel standbyLevel; /* Minimum level of standby for a given module */
} NEXUS_CommonModuleSettings;

/***************************************************************************
Summary:
Specification of which end of the soft-boiled egg should be cracked open in
Lilliput and Blefuscu. Also applied to computer memory.
****************************************************************************/
typedef enum NEXUS_EndianMode
{
    NEXUS_EndianMode_eLittle, /* least significant byte stored in the smallest address */
    NEXUS_EndianMode_eBig,    /* most significant byte stored in the smallest address */
    NEXUS_EndianMode_eMax
} NEXUS_EndianMode;

/**
Summary:
Handle that represents the allocated memory block
**/
typedef struct NEXUS_MemoryBlock *NEXUS_MemoryBlockHandle;

/***************************************************************************
Summary:
Information that is required to create instance of the BDBG_FifoReader
****************************************************************************/
typedef struct NEXUS_DebugFifoInfo {
    size_t elementSize; /* size of the single entry in the BDBG_Fifo */
    NEXUS_MemoryBlockHandle buffer; /* Memory Block containing internal BDBG_Fifo */
    unsigned offset; /* Offset from start of memory block where BDBG_Fifo begins */
} NEXUS_DebugFifoInfo;

/**
Summary:
Information about the amount of light contained in pixels found throughout the stream.
**/
typedef struct NEXUS_ContentLightLevel
{
    unsigned max; /* 1 cd / m^2. This is the max light level used in any pixel across the entire stream */
    unsigned maxFrameAverage; /* 1 cd / m^2. Averaging the light level spatially per picture as frmAvg,
                                 this is the max value of frmAvg reached across the entire stream */
} NEXUS_ContentLightLevel;

/**
Summary:
Information about the color properties of the display that was used to
master the content this information accompanies.
**/
typedef struct NEXUS_MasteringDisplayColorVolume
{
    NEXUS_Point redPrimary; /* red primary color chromaticity coordinates used by the mastering display.
                               X and Y values range from 0 to 50000 and are fixed point representations
                               of floating point values between 0.0 and 1.0, with a step of 0.00002 per tick. */
    NEXUS_Point greenPrimary; /* green primary color chromaticity coordinates used by the mastering display.
                                 see description for red primary above for unit details */
    NEXUS_Point bluePrimary; /* blue primary color chromaticity coordinates used by the mastering display.
                                see description for red primary above for unit details */
    NEXUS_Point whitePoint; /* white point chromaticity coordinate used by the mastering display.
                                see description for red primary above for unit details */
    struct
    {
        unsigned max; /* 1 cd / m^2 */
        unsigned min; /* 0.0001 cd / m^2 */
    } luminance; /* luminance range of the mastering display */
} NEXUS_MasteringDisplayColorVolume;

/***************************************************************************
Summary:
Standby mode used in NEXUS_PlatformStandbySettings

Description:
See "Power Management" section of nexus/docs/Nexus_Usage.pdf.

When nexus is implementing ePassive and eActive standby modes, our goal is to leave
as much code resident as possible. This allows applications to leave handles open and minimizes
change in SW state.

However, this is not possible in all cases. If a module must be shut down, the application
is responsible for closing handles before calling NEXUS_Platform_SetStandbySettings.
If you do not, an error will be issued to the console and NEXUS_Platform_SetStandbySettings will fail.
***************************************************************************/
typedef enum NEXUS_StandbyMode
{
    NEXUS_StandbyMode_eOn,          /* Normal mode of operation. Also known as S0 mode. */
    NEXUS_StandbyMode_eActive,      /* Frontend and transport modules are running. All other modules are put to sleep.
                       The same wakeup devices as ePassive are available.
                       The application cannot put the CPU to sleep in this mode. Also known as S1 mode*/
    NEXUS_StandbyMode_ePassive,     /* Lowest power setting while code remains resident.
                       IrInput, UhfInput, HdmiOutput (CEC), Gpio and Keypad are available to be configured as wakeup devices.
                       Application must call OS to put the CPU to sleep. Also known as S2 mode. */
    NEXUS_StandbyMode_eDeepSleep,   /* All cores are power gated except for AON block. Achieves minimum power state.
                       Gpio and Keypad are available to be configured as wakeup devices.
                       Application must call OS to put the CPU to sleep. Also known as S3 mode. */
    NEXUS_StandbyMode_eMax
} NEXUS_StandbyMode;

/***************************************************************************
Summary:
Settings used for module standby api
***************************************************************************/
typedef struct NEXUS_StandbySettings
{
    NEXUS_StandbyMode mode;
    struct {
        bool ir;
        bool uhf;
        bool keypad;
        bool gpio;
        bool nmi;
        bool cec;
        bool transport;
        unsigned timeout; /* in seconds */
    } wakeupSettings;
    bool openFrontend; /* If true, NEXUS_Platform_SetStandbySettings will initialize the frontend. */
    unsigned timeout; /* time (in milliseconds) for nexus to wait its internal activity to wind down */
} NEXUS_StandbySettings;

#ifdef __cplusplus
}
#endif

#endif
