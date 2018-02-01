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


#ifndef BM2MC_PACKET_H__
#define BM2MC_PACKET_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
PACKET TYPE ENUMERATION
 ***************************************************************************/
typedef enum BM2MC_PACKET_PacketType
{
    BM2MC_PACKET_PacketType_eSourceFeeder = 1,           /* source plane and color */
    BM2MC_PACKET_PacketType_eSourceFeeders,              /* 2 source planes and color */
    BM2MC_PACKET_PacketType_eThreeSourceFeeders,         /* 3 source planes and color */
    BM2MC_PACKET_PacketType_eStripedSourceFeeders,       /* striped source planes and color */
    BM2MC_PACKET_PacketType_eSourceColor,                /* source color */
    BM2MC_PACKET_PacketType_eSourceNone,                 /* source none (default) */
    BM2MC_PACKET_PacketType_eSourceControl,              /* source control of zero_pad and chroma_filter */
    BM2MC_PACKET_PacketType_eDestinationFeeder,          /* destination plane and color */
    BM2MC_PACKET_PacketType_eDestinationColor,           /* destination color */
    BM2MC_PACKET_PacketType_eDestinationNone,            /* destination none (default) */
    BM2MC_PACKET_PacketType_eDestinationControl,         /* destination control of zero_pad and chroma_filter */
    BM2MC_PACKET_PacketType_eOutputFeeder,               /* output plane */
    BM2MC_PACKET_PacketType_eOutputControl,              /* output control of dither and chroma_filter */
    BM2MC_PACKET_PacketType_eMipmapControl,              /* set mipmap level */
    BM2MC_PACKET_PacketType_eBlend,                      /* color and alpha blend, and blend color */
    BM2MC_PACKET_PacketType_eBlendColor,                 /* blend color */
    BM2MC_PACKET_PacketType_eRop,                        /* ternary raster operation */
    BM2MC_PACKET_PacketType_eSourceColorkey,             /* source colorkeys, masks, and replacements */
    BM2MC_PACKET_PacketType_eSourceColorkeyEnable,       /* source colorkey enable */
    BM2MC_PACKET_PacketType_eDestinationColorkey,        /* destination colorkeys, masks, and replacements */
    BM2MC_PACKET_PacketType_eDestinationColorkeyEnable,  /* destination colorkey enable */
    BM2MC_PACKET_PacketType_eFilter,                     /* filter coefficient table */
    BM2MC_PACKET_PacketType_eFilterEnable,               /* filter enable */
    BM2MC_PACKET_PacketType_eFilterControl,              /* filter control of rounding, channel adjustment and ordering */
    BM2MC_PACKET_PacketType_eSourceColorMatrix,          /* source color matrix */
    BM2MC_PACKET_PacketType_eSourceColorMatrixEnable,    /* source color matrix enable */
    BM2MC_PACKET_PacketType_eSourcePalette,              /* source palette address */
    BM2MC_PACKET_PacketType_eAlphaPremultiply,           /* multiply source pixel color by pixel alpha before blending */
    BM2MC_PACKET_PacketType_eAlphaDemultiply,            /* divide source pixel color by pixel alpha after blending */
    BM2MC_PACKET_PacketType_eDestAlphaPremultiply,       /* multiply destination pixel color by pixel alpha before blending */
    BM2MC_PACKET_PacketType_eMirror,                     /* reverse read/write order for source, destination and output feeders */
    BM2MC_PACKET_PacketType_eFixedScale,                 /* Deprecated. */
    BM2MC_PACKET_PacketType_eDestripeFixedScale,         /* Deprecated. */
    BM2MC_PACKET_PacketType_eFillBlit,                   /* fill output rectangle */
    BM2MC_PACKET_PacketType_eCopyBlit,                   /* copy source rectangle to output point */
    BM2MC_PACKET_PacketType_eBlendBlit,                  /* blend/rop/colorkey source rectangle to output point */
    BM2MC_PACKET_PacketType_eScaleBlit,                  /* scale source rectangle to output rectangle */
    BM2MC_PACKET_PacketType_eScaleBlendBlit,             /* scale and blend/rop/colorkey source rectangle to output rectangle */
    BM2MC_PACKET_PacketType_eUpdateScaleBlit,            /* scale source rectangle to output rectangle with partial update to output */
    BM2MC_PACKET_PacketType_eDestripeBlit,               /* YCbCr420 destripe blit */
    BM2MC_PACKET_PacketType_eResetState,                 /* reset state to default */
    BM2MC_PACKET_PacketType_eSaveState,                  /* save state (state must be restored before sync/checkpoint) */
    BM2MC_PACKET_PacketType_eRestoreState,               /* restore state */
    BM2MC_PACKET_PacketType_eMax                         /* maximum enum value */
}
BM2MC_PACKET_PacketType;

/***************************************************************************
PACKET HEADER STRUCTURE
 ***************************************************************************/
typedef struct
{
    uint8_t type;                    /* BM2MC_PACKET_PacketType enum */
    uint8_t size;                    /* size of packet structure */
    bool execute;                    /* execute blit operation after packet */
    bool sync;                       /* reserved */
}
BM2MC_PACKET_Header;

/***************************************************************************
PACKET MACROS

Usage:  BM2MC_PACKET_INIT( packet, BlendColor, false );
        (write blend color packet)
        BM2MC_PACKET_TERM( packet );
        BM2MC_PACKET_INIT( packet, FillBlit, true );
        (write fill packet)
        BM2MC_PACKET_TERM( packet );
 ***************************************************************************/
#define BM2MC_PACKET_INIT( \
    PACKET,                         /* ptr to packet */ \
    TYPE,                           /* BM2MC_PACKET_PacketType enum */ \
    EXECUTE )                       /* execute blit operation after packet */ \
do { \
    BDBG_CASSERT((sizeof (*(PACKET)) % sizeof (uint32_t)) == 0); \
    BDBG_CASSERT((sizeof (*(PACKET)) == sizeof (BM2MC_PACKET_Packet##TYPE))); \
    (PACKET)->header.type = BM2MC_PACKET_PacketType_e##TYPE; \
    (PACKET)->header.size = sizeof (*(PACKET)) / sizeof (uint32_t); \
    (PACKET)->header.execute = EXECUTE; \
    (PACKET)->header.sync = 0; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_TERM( packet ) (packet)++

/***************************************************************************
PACKET STRUCTURES
 ***************************************************************************/
typedef enum BM2MC_PACKET_PixelFormat
{
    BM2MC_PACKET_PixelFormat_eUnknown = 0,

    BM2MC_PACKET_PixelFormat_eR5_G6_B5,              /* 16-bit RGB565, (constant alpha) */
    BM2MC_PACKET_PixelFormat_eB5_G6_R5,              /* 16-bit BGR565, (constant alpha) */

    BM2MC_PACKET_PixelFormat_eA1_R5_G5_B5,           /* 16-bit ARGB1555 */
    BM2MC_PACKET_PixelFormat_eX1_R5_G5_B5,           /* 16-bit XRGB1555 */
    BM2MC_PACKET_PixelFormat_eA1_B5_G5_R5,           /* 16-bit ABGR1555 */
    BM2MC_PACKET_PixelFormat_eX1_B5_G5_R5,           /* 16-bit XBGR1555 */
    BM2MC_PACKET_PixelFormat_eR5_G5_B5_A1,           /* 16-bit RGBA5551 */
    BM2MC_PACKET_PixelFormat_eR5_G5_B5_X1,           /* 16-bit RGBX5551 */
    BM2MC_PACKET_PixelFormat_eB5_G5_R5_A1,           /* 16-bit BGRA5551 */
    BM2MC_PACKET_PixelFormat_eB5_G5_R5_X1,           /* 16-bit BGRX5551 */

    BM2MC_PACKET_PixelFormat_eA4_R4_G4_B4,           /* 16-bit ARGB4444 */
    BM2MC_PACKET_PixelFormat_eX4_R4_G4_B4,           /* 16-bit XRGB4444 */
    BM2MC_PACKET_PixelFormat_eA4_B4_G4_R4,           /* 16-bit ABGR4444 */
    BM2MC_PACKET_PixelFormat_eX4_B4_G4_R4,           /* 16-bit XBGR4444 */
    BM2MC_PACKET_PixelFormat_eR4_G4_B4_A4,           /* 16-bit RGBA4444 */
    BM2MC_PACKET_PixelFormat_eR4_G4_B4_X4,           /* 16-bit RGBX4444 */
    BM2MC_PACKET_PixelFormat_eB4_G4_R4_A4,           /* 16-bit BGRA4444 */
    BM2MC_PACKET_PixelFormat_eB4_G4_R4_X4,           /* 16-bit BGRX4444 */

    BM2MC_PACKET_PixelFormat_eA8_R8_G8_B8,           /* 32-bit ARGB8888 */
    BM2MC_PACKET_PixelFormat_eX8_R8_G8_B8,           /* 32-bit XRGB8888 */
    BM2MC_PACKET_PixelFormat_eA8_B8_G8_R8,           /* 32-bit ABGR8888 */
    BM2MC_PACKET_PixelFormat_eX8_B8_G8_R8,           /* 32-bit XBGR8888 */
    BM2MC_PACKET_PixelFormat_eR8_G8_B8_A8,           /* 32-bit RGBA8888 */
    BM2MC_PACKET_PixelFormat_eR8_G8_B8_X8,           /* 32-bit RGBX8888 */
    BM2MC_PACKET_PixelFormat_eB8_G8_R8_A8,           /* 32-bit BGRA8888 */
    BM2MC_PACKET_PixelFormat_eB8_G8_R8_X8,           /* 32-bit BGRX8888 */

    BM2MC_PACKET_PixelFormat_eA8,                    /* 8-bit alpha-only surface (constant color) */
    BM2MC_PACKET_PixelFormat_eA4,                    /* 4-bit alpha-only surface (constant color) */
    BM2MC_PACKET_PixelFormat_eA2,                    /* 2-bit alpha-only surface (constant color) */
    BM2MC_PACKET_PixelFormat_eA1,                    /* 1-bit alpha-only surface (constant color) */

    BM2MC_PACKET_PixelFormat_eW1,                    /* 1-bit window alpha-only surface (constant color) */

    BM2MC_PACKET_PixelFormat_eA8_P8,                 /* 8-bit alpha and 8-bit palette index */
    BM2MC_PACKET_PixelFormat_eP8,                    /* 8-bit palette index */
    BM2MC_PACKET_PixelFormat_eP4,                    /* 4-bit palette index */
    BM2MC_PACKET_PixelFormat_eP2,                    /* 2-bit palette index */
    BM2MC_PACKET_PixelFormat_eP1,                    /* 1-bit palette index */

    BM2MC_PACKET_PixelFormat_eY8_P8,                 /* 8-bit luma and 8-bit palette index */
    BM2MC_PACKET_PixelFormat_eA8_Y8,                 /* 8-bit alpha and 8-bit luma */

    BM2MC_PACKET_PixelFormat_eCb8,                   /* 8-bit chroma-only (Cb) */
    BM2MC_PACKET_PixelFormat_eCr8,                   /* 8-bit chroma-only (Cr) */

    BM2MC_PACKET_PixelFormat_eY8,                    /* 8-bit luma-only, for YCbCr420 */
    BM2MC_PACKET_PixelFormat_eCb8_Cr8,               /* 8-bit Cb and 8 bit Cr, for YCbCr420 */
    BM2MC_PACKET_PixelFormat_eCr8_Cb8,               /* 8-bit Cr and 8 bit Cb, for YCbCr420 */

    BM2MC_PACKET_PixelFormat_eY10,                   /* 10-bit luma-only, for YCbCr420 */
    BM2MC_PACKET_PixelFormat_eCb10_Cr10,             /* 10-bit Cb and 10 bit Cr, for YCbCr420 */
    BM2MC_PACKET_PixelFormat_eCr10_Cb10,             /* 10-bit Cr and 10 bit Cb, for YCbCr420 */

    BM2MC_PACKET_PixelFormat_eY08_Cb8_Y18_Cr8,       /* 32-bit for 2 pixels, YCbCr422 w/ 8-bit values */
    BM2MC_PACKET_PixelFormat_eY08_Cr8_Y18_Cb8,       /* 32-bit for 2 pixels, YCbCr422 w/ 8-bit values */
    BM2MC_PACKET_PixelFormat_eY18_Cb8_Y08_Cr8,       /* 32-bit for 2 pixels, YCbCr422 w/ 8-bit values */
    BM2MC_PACKET_PixelFormat_eY18_Cr8_Y08_Cb8,       /* 32-bit for 2 pixels, YCbCr422 w/ 8-bit values */
    BM2MC_PACKET_PixelFormat_eCb8_Y08_Cr8_Y18,       /* 32-bit for 2 pixels, YCbCr422 w/ 8-bit values */
    BM2MC_PACKET_PixelFormat_eCb8_Y18_Cr8_Y08,       /* 32-bit for 2 pixels, YCbCr422 w/ 8-bit values */
    BM2MC_PACKET_PixelFormat_eCr8_Y18_Cb8_Y08,       /* 32-bit for 2 pixels, YCbCr422 w/ 8-bit values */
    BM2MC_PACKET_PixelFormat_eCr8_Y08_Cb8_Y18,       /* 32-bit for 2 pixels, YCbCr422 w/ 8-bit values */

    BM2MC_PACKET_PixelFormat_eX2_Cr10_Y10_Cb10,      /* 32-bit for 1 pixel,  YCbCr444 w/ 10-bit values */

    BM2MC_PACKET_PixelFormat_eA8_Y8_Cb8_Cr8,         /* YCbCr444 w/ 8-bit values */
    BM2MC_PACKET_PixelFormat_eA8_Cr8_Cb8_Y8,         /* YCbCr444 w/ 8-bit values */
    BM2MC_PACKET_PixelFormat_eCr8_Cb8_Y8_A8,         /* YCbCr444 w/ 8-bit values */
    BM2MC_PACKET_PixelFormat_eY8_Cb8_Cr8_A8,         /* YCbCr444 w/ 8-bit values */

    BM2MC_PACKET_PixelFormat_eY010_Cb10_Y110_Cr10,   /* YCbCr422 w/ 10-bit values */
    BM2MC_PACKET_PixelFormat_eY010_Cr10_Y110_Cb10,   /* YCbCr422 w/ 10-bit values */
    BM2MC_PACKET_PixelFormat_eY110_Cb10_Y010_Cr10,   /* YCbCr422 w/ 10-bit values */
    BM2MC_PACKET_PixelFormat_eY110_Cr10_Y010_Cb10,   /* YCbCr422 w/ 10-bit values */
    BM2MC_PACKET_PixelFormat_eCb10_Y010_Cr10_Y110,   /* YCbCr422 w/ 10-bit values */
    BM2MC_PACKET_PixelFormat_eCb10_Y110_Cr10_Y010,   /* YCbCr422 w/ 10-bit values */
    BM2MC_PACKET_PixelFormat_eCr10_Y110_Cb10_Y010,   /* YCbCr422 w/ 10-bit values */
    BM2MC_PACKET_PixelFormat_eCr10_Y010_Cb10_Y110,   /* YCbCr422 w/ 10-bit values */

    BM2MC_PACKET_PixelFormat_eL8,                    /* 8-bit luma */
    BM2MC_PACKET_PixelFormat_eL4_A4,                 /* 4-bit luma and 4-bit alpha */
    BM2MC_PACKET_PixelFormat_eL8_A8,                 /* 8-bit luma and 8-bit alpha */
    BM2MC_PACKET_PixelFormat_eL15_L05_A6,            /* 5-bit luma1, 5-bit luma0, 6-bit alpha */

    BM2MC_PACKET_PixelFormat_eCompressed_A8_R8_G8_B8,/* compressed ARGB_8888 */
    BM2MC_PACKET_PixelFormat_eUIF_R8_G8_B8_A8,       /* UIF [V3D texture layout] ARGB_8888 */

    /* The following pixel formats are not supported by NEXUS_Graphics2D */
    BM2MC_PACKET_PixelFormat_eR8_G8_B8,              /* 24-bit packet */
    BM2MC_PACKET_PixelFormat_eYCbCr422_10bit,

    BM2MC_PACKET_PixelFormat_eMax
}
BM2MC_PACKET_PixelFormat;

/***************************************************************************/
typedef struct BM2MC_PACKET_Plane
{
    uint64_t address;                /* physical memory offset of plane */
    uint16_t pitch;                  /* pitch of plane */
    uint8_t format;                  /* BM2MC_PACKET_PixelFormat enum */
    uint16_t width;                  /* surface width, used for bounds checking */
    uint16_t height;                 /* surface height, used for bounds checking */
}
BM2MC_PACKET_Plane;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketSourceFeeder
{
    BM2MC_PACKET_Header header;      /* packet header */
    BM2MC_PACKET_Plane plane;        /* source plane (defaults to none) */
    uint32_t color;                  /* 32-bit color (A, R/Y, G/Cb, B/Cr/P) (defaults to 0) */
}
BM2MC_PACKET_PacketSourceFeeder;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketSourceFeeders
{
    BM2MC_PACKET_Header header;      /* packet header */
    BM2MC_PACKET_Plane plane0;       /* primary source plane (defaults to none) */
    BM2MC_PACKET_Plane plane1;       /* secondary source plane (defaults to none) */
    uint32_t color;                  /* 32-bit color (A, R/Y, G/Cb, B/Cr/P) (defaults to 0) */
}
BM2MC_PACKET_PacketSourceFeeders;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketThreeSourceFeeders
{
    BM2MC_PACKET_Header header;      /* packet header */
    BM2MC_PACKET_Plane plane0;       /* primary source plane (defaults to none) */
    BM2MC_PACKET_Plane plane1;       /* secondary source plane (defaults to none) */
    BM2MC_PACKET_Plane plane2;       /* third source plane (defaults to none) */
    uint32_t color;                  /* 32-bit color (A, R/Y, G/Cb, B/Cr/P) (defaults to 0) */
}
BM2MC_PACKET_PacketThreeSourceFeeders;

/***************************************************************************
 * for striped 4:2:0 format, frame or fields
 * non zero bottom field address could be set only if the picture is truely progressive
 */
typedef struct BM2MC_PACKET_StripedPlane
{
    uint64_t luma_address;               /* physical memory offset of top field or frame luma */
    uint64_t chroma_address;             /* physical memory offset of top field or frame chroma */
    uint64_t bottom_field_luma_address;  /* physical memory offset of bottom field luma,
                                          * 0 if this is a frame or top field only picture */
    uint64_t bottom_field_chroma_address;/* physical memory offset of bottom field chroma,
                                          * ignored if luma_bot_field_address is 0 */
    uint16_t width;                      /* picture width, for size verification */
    uint16_t height;                     /* picture height, for size verification */
    uint16_t stripe_width;               /* bytes to read for a stripe line, must be 128 or 256 */
    uint16_t stripe_pitch;               /* bytes to offset to next stripe line, >= stripe_width */
    uint16_t luma_stripe_height;         /* field luma stripe height in pixels if it is a field or field pair */
    uint16_t chroma_stripe_height;       /* field chroma stripe height in pixels if it is a field or field pair */
    uint8_t luma_format;                 /* BM2MC_PACKET_PixelFormat enum, Y8 or Y10 */
    uint8_t chroma_format;               /* BM2MC_PACKET_PixelFormat enum, Cb8_Cr8, Cr8_Cb8, Cb10_Cr10, or Cr10_Cb10 */
    uint8_t reserved[2];                 /* align to 4-bytes */
}
BM2MC_PACKET_StripedPlane;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketStripedSourceFeeders
{
    BM2MC_PACKET_Header header;       /* packet header */
    BM2MC_PACKET_StripedPlane plane;  /* top/bottom luma/chroma planes, 4:2:0 format */
    uint32_t color;                   /* 32-bit color (A, R/Y, G/Cb, B/Cr/P) (defaults to 0) */
}
BM2MC_PACKET_PacketStripedSourceFeeders;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketSourceColor
{
    BM2MC_PACKET_Header header;      /* packet header */
    uint32_t color;                  /* 32-bit color (A, R/Y, G/Cb, B/Cr/P) (defaults to 0) */
}
BM2MC_PACKET_PacketSourceColor;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketSourceNone
{
    BM2MC_PACKET_Header header;      /* packet header */
}
BM2MC_PACKET_PacketSourceNone;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketSourceControl
{
    BM2MC_PACKET_Header header;      /* packet header */
    bool zero_pad;                   /* zero pad lower bits when expanding pixel channels */
                                     /* instead of replicating upper bits (defaults to false) */
    bool chroma_filter;              /* whether filter chroma's when converting YCbCr422/420 source */
                                     /* format into internal YCbCr444 format (defaults to true) */
    bool linear_destripe;            /* Deprecated */
    uint8_t reserved[1];             /* align to 4-bytes */
}
BM2MC_PACKET_PacketSourceControl;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketDestinationFeeder
{
    BM2MC_PACKET_Header header;      /* packet header */
    BM2MC_PACKET_Plane plane;        /* destination plane (defaults to none) */
    uint32_t color;                  /* 32-bit color (A, R/Y, G/Cb, B/Cr/P) (defaults to 0) */
}
BM2MC_PACKET_PacketDestinationFeeder;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketDestinationColor
{
    BM2MC_PACKET_Header header;      /* packet header */
    uint32_t color;                  /* 32-bit color (A, R/Y, G/Cb, B/Cr/P) (defaults to 0) */
}
BM2MC_PACKET_PacketDestinationColor;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketDestinationNone
{
    BM2MC_PACKET_Header header;      /* packet header */
}
BM2MC_PACKET_PacketDestinationNone;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketDestinationControl
{
    BM2MC_PACKET_Header header;      /* packet header */
    bool zero_pad;                   /* zero pad lower bits when expanding pixel channels */
                                     /* instead of replicating upper bits (defaults to false) */
    bool chroma_filter;              /* whether filter chroma's when converting YCbCr422/420 destination */
                                     /* format into internal YCbCr444 format (defaults to true) */
    uint8_t reserved[2];             /* align to 4-bytes */
}
BM2MC_PACKET_PacketDestinationControl;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketOutputFeeder
{
    BM2MC_PACKET_Header header;      /* packet header */
    BM2MC_PACKET_Plane plane;        /* output plane  (defaults to none) */
}
BM2MC_PACKET_PacketOutputFeeder;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketOutputControl
{
    BM2MC_PACKET_Header header;      /* packet header */
    bool dither;                     /* dither pixel channels less than 8-bits (defaults to false) */
    bool chroma_filter;              /* whether filter chroma's when converting internal YCbCr444 format */
                                     /* into output YCbCr422/420 format (defaults to true) */
    uint8_t reserved[2];             /* align to 4-bytes */
}BM2MC_PACKET_PacketOutputControl;
/***************************************************************************/
typedef struct BM2MC_PACKET_PacketMipmapControl
{
    BM2MC_PACKET_Header header;      /* packet header */
    uint32_t mipLevel;                /* 0-15, (defaults to 0, base image only) */
}BM2MC_PACKET_PacketMipmapControl;

/***************************************************************************/
typedef enum BM2MC_PACKET_BlendFactor
{
    BM2MC_PACKET_BlendFactor_eZero = 0,                /* Zero */
    BM2MC_PACKET_BlendFactor_eHalf,                    /* 1/2 */
    BM2MC_PACKET_BlendFactor_eOne,                     /* One */
    BM2MC_PACKET_BlendFactor_eSourceColor,             /* Color from source */
    BM2MC_PACKET_BlendFactor_eInverseSourceColor,      /* 1-color from source */
    BM2MC_PACKET_BlendFactor_eSourceAlpha,             /* Alpha from source */
    BM2MC_PACKET_BlendFactor_eInverseSourceAlpha,      /* 1-alpha from source */
    BM2MC_PACKET_BlendFactor_eDestinationColor,        /* Color from destination */
    BM2MC_PACKET_BlendFactor_eInverseDestinationColor, /* 1-color from destination */
    BM2MC_PACKET_BlendFactor_eDestinationAlpha,        /* Alpha from destination */
    BM2MC_PACKET_BlendFactor_eInverseDestinationAlpha, /* 1-alpha from destination */
    BM2MC_PACKET_BlendFactor_eConstantColor,           /* Color from blend color */
    BM2MC_PACKET_BlendFactor_eInverseConstantColor,    /* 1-color from blend color */
    BM2MC_PACKET_BlendFactor_eConstantAlpha,           /* Alpha from blend color */
    BM2MC_PACKET_BlendFactor_eInverseConstantAlpha     /* 1-alpha from blend color */
}
BM2MC_PACKET_BlendFactor;

/***************************************************************************/
/* The blend equation is "a*b +/- c*d +/- e" */
typedef struct BM2MC_PACKET_Blend
{
    uint8_t a;                       /* BM2MC_PACKET_BlendFactor */
    uint8_t b;                       /* BM2MC_PACKET_BlendFactor */
    bool sub_cd;                     /* add or subtract cd*/
    uint8_t c;                       /* BM2MC_PACKET_BlendFactor */
    uint8_t d;                       /* BM2MC_PACKET_BlendFactor */
    bool sub_e;                      /* add or subtract e*/
    uint8_t e;                       /* BM2MC_PACKET_BlendFactor */
}
BM2MC_PACKET_Blend;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketBlend
{
    BM2MC_PACKET_Header header;      /* packet header */
    BM2MC_PACKET_Blend color_blend;  /* color blend equation (defaults to eSourceColor) */
    BM2MC_PACKET_Blend alpha_blend;  /* alpha blend equation (defaults to eSourceAlpha)*/
    uint32_t color;                  /* 32-bit color (A, R/Y, G/Cb, B/Cr/P) (defaults to 0) */
}
BM2MC_PACKET_PacketBlend;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketBlendColor
{
    BM2MC_PACKET_Header header;      /* packet header */
    uint32_t color;                  /* 32-bit color (A, R/Y, G/Cb, B/Cr/P) (defaults to 0) */
}
BM2MC_PACKET_PacketBlendColor;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketRop
{
    BM2MC_PACKET_Header header;      /* packet header */
    uint32_t rop;                    /* ternary raster operation (defaults to 0) */
    uint32_t pattern0;               /* output aligned 1-bit pattern 0 (8x4) top (defaults to 0) */
    uint32_t pattern1;               /* output aligned 1-bit pattern 1 (8x4) bottom (defaults to 0) */
    uint32_t color0;                 /* 32-bit pattern color 0 (A, R/Y, G/Cb, B/Cr/P) (defaults to 0) */
    uint32_t color1;                 /* 32-bit pattern color 1 (A, R/Y, G/Cb, B/Cr/P) (defaults to 0) */
}
BM2MC_PACKET_PacketRop;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketSourceColorkey
{
    BM2MC_PACKET_Header header;      /* packet header */
    uint32_t high;                   /* 32-bit colorkey high range (defaults to 0) */
    uint32_t low;                    /* 32-bit colorkey low range (defaults to 0) */
    uint32_t mask;                   /* 32-bit mask, used an pixel before compare (defaults to 0) */
    uint32_t replacement;            /* 32-bit replacement color, when colorkeyed (defaults to 0) */
    uint32_t replacement_mask;       /* 32-bit replacement mask, when colorkeyed (defaults to 0) */
    bool exclusive;                  /* exclusive high compare (defaults to false) */
}
BM2MC_PACKET_PacketSourceColorkey;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketSourceColorkeyEnable
{
    BM2MC_PACKET_Header header;      /* packet header */
    bool enable;                     /* enable colorkey (defaults to false) */
    uint8_t reserved[3];             /* align to 4-bytes */
}
BM2MC_PACKET_PacketSourceColorkeyEnable;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketDestinationColorkey
{
    BM2MC_PACKET_Header header;      /* packet header */
    uint32_t high;                   /* 32-bit colorkey high range (defaults to 0) */
    uint32_t low;                    /* 32-bit colorkey low range (defaults to 0) */
    uint32_t mask;                   /* 32-bit mask, used an pixel before compare (defaults to 0) */
    uint32_t replacement;            /* 32-bit replacement color, when colorkeyed (defaults to 0) */
    uint32_t replacement_mask;       /* 32-bit replacement mask, when colorkeyed (defaults to 0) */
    bool exclusive;                  /* exclusive high compare (defaults to false) */
}
BM2MC_PACKET_PacketDestinationColorkey;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketDestinationColorkeyEnable
{
    BM2MC_PACKET_Header header;      /* packet header */
    bool enable;                     /* enable colorkey (defaults to false) */
    uint8_t reserved[3];             /* align to 4-bytes */
}
BM2MC_PACKET_PacketDestinationColorkeyEnable;

/***************************************************************************/
typedef struct BM2MC_PACKET_FilterCoeffs
{
    int16_t coeffs[2][3];            /* fixed point in S1.8 format */
}
BM2MC_PACKET_FilterCoeffs;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketFilter
{
    BM2MC_PACKET_Header header;      /* packet header */
    BM2MC_PACKET_FilterCoeffs hor;   /* horizontal filter coefficients (defaults to bilinear coeffs) */
    BM2MC_PACKET_FilterCoeffs ver;   /* vertical filter coefficients (defaults to bilinear coeffs) */
}
BM2MC_PACKET_PacketFilter;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketFilterEnable
{
    BM2MC_PACKET_Header header;      /* packet header */
    bool enable;                     /* enable filter (defaults to false) */
    uint8_t reserved[3];             /* align to 4-bytes */
}
BM2MC_PACKET_PacketFilterEnable;

/***************************************************************************/
/*
  process order of colorkey, filter/scale, and matrix
*/
typedef enum BM2MC_PACKET_eFilterOrder
{
    BM2MC_PACKET_eFilterOrder_FilterColorkeyMatrix = 0,
    BM2MC_PACKET_eFilterOrder_FilterMatrixColorkey,
    BM2MC_PACKET_eFilterOrder_ColorkeyMatrixFilter,
    BM2MC_PACKET_eFilterOrder_ColorkeyFilterMatrix,
    BM2MC_PACKET_eFilterOrder_MatrixFilterColorkey,
    BM2MC_PACKET_eFilterOrder_MatrixColorkeyFilter,
    BM2MC_PACKET_eFilterOrder_BypassAll
}
BM2MC_PACKET_eFilterOrder;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketFilterControl
{
    BM2MC_PACKET_Header header;      /* packet header */
    bool round;                      /* round pixel channels instead of truncating when filtering (defaults to true) */
    bool adjust_color;               /* subtract value from color channels before filtering (defaults to false) */
    bool sub_alpha;                  /* use alpha/2 as subtract value instead of 128 (defaults to false) */
    uint8_t order;                   /* BM2MC_PACKET_eFilterOrder (default ColorkeyMatrixFilter) */
}
BM2MC_PACKET_PacketFilterControl;

/***************************************************************************/
/* Color Matrix Formula:
    C2out = m[0][0]*C2in + m[0][1]*C1in + m[0][2]*C0in + m[0][3]*C3in + m[0][4]
    C1out = m[1][0]*C2in + m[1][1]*C1in + m[1][2]*C0in + m[1][3]*C3in + m[1][4]
    C0out = m[2][0]*C2in + m[2][1]*C1in + m[2][2]*C0in + m[2][3]*C3in + m[2][4]
    C3out = m[3][0]*C2in + m[3][1]*C1in + m[3][2]*C0in + m[3][3]*C3in + m[3][4]
*/
typedef struct BM2MC_PACKET_ColorMatrix
{
    int16_t m[4][5];        /* fixed point in S3.10 format for multipliers,
                               fixed point in S9.4 format for adders */
}
BM2MC_PACKET_ColorMatrix;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketSourceColorMatrix
{
    BM2MC_PACKET_Header header;      /* packet header */
    BM2MC_PACKET_ColorMatrix matrix; /* color matrix (defaults to all 0s) */
}
BM2MC_PACKET_PacketSourceColorMatrix;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketSourceColorMatrixEnable
{
    BM2MC_PACKET_Header header;      /* packet header */
    bool enable;                     /* enable color matrix (defaults to false) */
    uint8_t reserved[3];             /* align to 4-bytes */
}
BM2MC_PACKET_PacketSourceColorMatrixEnable;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketSourcePalette
{
    BM2MC_PACKET_Header header;      /* packet header */
    uint64_t address;                /* physical memory offset of palette (defaults to null) */
}
BM2MC_PACKET_PacketSourcePalette;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketAlphaPremultiply
{
    BM2MC_PACKET_Header header;      /* packet header */
    bool enable;                     /* multiply source pixel's color by its alpha before blending (defaults to false) */
    uint8_t reserved[3];             /* align to 4-bytes */
}
BM2MC_PACKET_PacketAlphaPremultiply;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketAlphaDemultiply
{
    BM2MC_PACKET_Header header;      /* packet header */
    bool enable;                     /* divide source pixel's color by its alpha after blending (defaults to false) */
    uint8_t reserved[3];             /* align to 4-bytes */
}
BM2MC_PACKET_PacketAlphaDemultiply;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketDestAlphaPremultiply
{
    BM2MC_PACKET_Header header;      /* packet header */
    bool enable;                     /* multiply destination pixel's color by its alpha (defaults to false) */
    uint8_t reserved[3];             /* align to 4-bytes */
}
BM2MC_PACKET_PacketDestAlphaPremultiply;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketMirror
{
    BM2MC_PACKET_Header header;      /* packet header */
    bool src_hor;                    /* reverse horizontal direction of src (defaults to false) */
    bool src_ver;                    /* reverse vertical direction of src (defaults to false) */
    bool dst_hor;                    /* reverse horizontal direction of dst (defaults to false) */
    bool dst_ver;                    /* reverse vertical direction of dst (defaults to false) */
    bool out_hor;                    /* reverse horizontal direction of out (defaults to false) */
    bool out_ver;                    /* reverse vertical direction of out (defaults to false) */
    uint8_t reserved[2];             /* align to 4-bytes */
}
BM2MC_PACKET_PacketMirror;

/***************************************************************************/
typedef struct BM2MC_PACKET_Point
{
    uint16_t x;                      /* x coordinate (0-8191) */
    uint16_t y;                      /* y coordinate (0-8191) */
}
BM2MC_PACKET_Point;

/***************************************************************************/
typedef struct BM2MC_PACKET_Size
{
    uint16_t width;                  /* width (1-8191) */
    uint16_t height;                 /* height (1-8191) */
}
BM2MC_PACKET_Size;

/***************************************************************************/
typedef struct BM2MC_PACKET_Rectangle
{
    uint16_t x;                      /* x coordinate (0-8191) */
    uint16_t y;                      /* y coordinate (0-8191) */
    uint16_t width;                  /* width (1-8191) */
    uint16_t height;                 /* height (1-8191) */
}
BM2MC_PACKET_Rectangle;

/***************************************************************************/
#define BM2MC_PACKET_FIXED_SCALE_STEP_ZERO  0x80000000  /* set hor_step or ver_step to this value to use 0 as fixed scale */

/***************************************************************************
 * Deprecated.
 */
typedef struct BM2MC_PACKET_PacketFixedScale
{
    BM2MC_PACKET_Header header;      /* packet header */
    int32_t hor_phase;               /* horizontal initial phase adjustment (0=ignore) */
    int32_t ver_phase;               /* vertical initial phase adjustment (0=ignore) */
    uint32_t hor_step;               /* horizontal fixed source step (0=ignore) */
    uint32_t ver_step;               /* vertical fixed source step (0=ignore) */
    uint32_t shift;                  /* amount of factional bits in values (max is 20) */
}
BM2MC_PACKET_PacketFixedScale;

/***************************************************************************
 * Deprecated.
 */
typedef struct BM2MC_PACKET_PacketDestripeFixedScale
{
    BM2MC_PACKET_Header header;         /* packet header */
    BM2MC_PACKET_Rectangle chroma_rect; /* source chroma rect */
    int32_t hor_luma_phase;             /* horizontal luma initial phase adjustment (0=ignore) */
    int32_t ver_luma_phase;             /* vertical luma initial phase adjustment (0=ignore) */
    int32_t hor_chroma_phase;           /* horizontal chroma initial phase adjustment (0=ignore) */
    int32_t ver_chroma_phase;           /* vertical chroma initial phase adjustment (0=ignore) */
    uint32_t hor_luma_step;             /* horizontal luma fixed source step (0=ignore) */
    uint32_t ver_luma_step;             /* vertical luma fixed source step (0=ignore) */
    uint32_t shift;                     /* amount of factional bits in values (max is 20) */
}
BM2MC_PACKET_PacketDestripeFixedScale;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketFillBlit
{
    BM2MC_PACKET_Header header;      /* packet header */
    BM2MC_PACKET_Rectangle rect;     /* output rect */
}
BM2MC_PACKET_PacketFillBlit;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketCopyBlit
{
    BM2MC_PACKET_Header header;      /* packet header */
    BM2MC_PACKET_Rectangle src_rect; /* source rect */
    BM2MC_PACKET_Point out_point;    /* output point */
}
BM2MC_PACKET_PacketCopyBlit;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketBlendBlit
{
    BM2MC_PACKET_Header header;      /* packet header */
    BM2MC_PACKET_Rectangle src_rect; /* source rect */
    BM2MC_PACKET_Point out_point;    /* output point */
    BM2MC_PACKET_Point dst_point;    /* destintation point */
}
BM2MC_PACKET_PacketBlendBlit;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketScaleBlit
{
    BM2MC_PACKET_Header header;      /* packet header */
    BM2MC_PACKET_Rectangle src_rect; /* source rect */
    BM2MC_PACKET_Rectangle out_rect; /* output rect */
}
BM2MC_PACKET_PacketScaleBlit;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketScaleBlendBlit
{
    BM2MC_PACKET_Header header;      /* packet header */
    BM2MC_PACKET_Rectangle src_rect; /* source rect */
    BM2MC_PACKET_Rectangle out_rect; /* output rect */
    BM2MC_PACKET_Point dst_point;    /* destintation point */
}
BM2MC_PACKET_PacketScaleBlendBlit;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketUpdateScaleBlit
{
    BM2MC_PACKET_Header header;         /* packet header */
    BM2MC_PACKET_Rectangle src_rect;    /* source rect */
    BM2MC_PACKET_Rectangle out_rect;    /* output rect */
    BM2MC_PACKET_Rectangle update_rect; /* update rect for output */
}
BM2MC_PACKET_PacketUpdateScaleBlit;

/***************************************************************************
 * Deprecated. Instead, use BM2MC_PACKET_PacketStripedSourceFeeders and regular
 * blit packet such as BM2MC_PACKET_PacketScaleBlit.
 * If BM2MC_PACKET_PacketStripedSourceFeeders and BM2MC_PACKET_PacketDestripeBlit
 * are used together, stripe parameters set in BM2MC_PACKET_PacketDestripeBlit
 * will override the stripe setting in BM2MC_PACKET_PacketStripedSourceFeeders.
 */
typedef struct BM2MC_PACKET_PacketDestripeBlit
{
    BM2MC_PACKET_Header header;      /* packet header */
    BM2MC_PACKET_Rectangle src_rect; /* source rect */
    BM2MC_PACKET_Rectangle out_rect; /* output rect */
    BM2MC_PACKET_Point dst_point;    /* destintation point */
    uint16_t source_stripe_width;    /* source stripe width in bytes */
    uint16_t luma_stripe_height;     /* luma (Y) stripe height */
    uint16_t chroma_stripe_height;   /* chroma (CbCr) stripe height */
    uint8_t reserved[2];             /* align to 4-bytes */
}
BM2MC_PACKET_PacketDestripeBlit;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketResetState
{
    BM2MC_PACKET_Header header;      /* packet header */
}
BM2MC_PACKET_PacketResetState;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketSaveState
{
    BM2MC_PACKET_Header header;      /* packet header */
}
BM2MC_PACKET_PacketSaveState;

/***************************************************************************/
typedef struct BM2MC_PACKET_PacketRestoreState
{
    BM2MC_PACKET_Header header;      /* packet header */
}
BM2MC_PACKET_PacketRestoreState;

/***************************************************************************
PACKET WRITE MACROS

The following macros are an optional method for writing packets and
incrementing the packet pointer. Please be aware that the BUFFER param is
an [in,out] param. Use as follows:

    current_buffer = first_buffer;
    BM2MC_PACKET_WRITE_SourceFeeder( current_buffer, plane, color, false );
    BM2MC_PACKET_WRITE_Blend( current_buffer, alphaBlend, colorBlend, color, true );
    size = current_buffer - first_buffer;
 ***************************************************************************/

#define BM2MC_PACKET_WRITE_SourceFeeder( BUFFER, PLANE, COLOR, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketSourceFeeder *packet = (BM2MC_PACKET_PacketSourceFeeder *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, SourceFeeder, EXECUTE ); \
    packet->plane = (PLANE); \
    packet->color = (COLOR); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_SourceFeeders( BUFFER, PLANE0, PLANE1, COLOR, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketSourceFeeders *packet = (BM2MC_PACKET_PacketSourceFeeders *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, SourceFeeders, EXECUTE ); \
    packet->plane0 = (PLANE0); \
    packet->plane1 = (PLANE1); \
    packet->color = (COLOR); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_ThreeSourceFeeders( BUFFER, PLANE0, PLANE1, PLANE2,COLOR, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketThreeSourceFeeders *packet = (BM2MC_PACKET_PacketThreeSourceFeeders *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, ThreeSourceFeeders, EXECUTE ); \
    packet->plane0 = (PLANE0); \
    packet->plane1 = (PLANE1); \
    packet->plane2 = (PLANE2); \
    packet->color = (COLOR); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_SourceColor( BUFFER, COLOR, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketSourceColor *packet = (BM2MC_PACKET_PacketSourceColor *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, SourceColor, EXECUTE ); \
    packet->color = (COLOR); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_SourceNone( BUFFER, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketSourceNone *packet = (BM2MC_PACKET_PacketSourceNone *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, SourceNone, EXECUTE ); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_StripedSourceFeeders( BUFFER, PLANE, COLOR, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketStripedSourceFeeders *packet = (BM2MC_PACKET_PacketStripedSourceFeeders *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, StripedSourceFeeders, EXECUTE ); \
    packet->plane = (PLANE); \
    packet->color = (COLOR); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_SourceControl( BUFFER, ZERO_PAD, CHROMA_FILTER, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketSourceControl *packet = (BM2MC_PACKET_PacketSourceControl *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, SourceControl, EXECUTE ); \
    packet->zero_pad = (ZERO_PAD); \
    packet->chroma_filter = (CHROMA_FILTER); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_DestinationFeeder( BUFFER, PLANE, COLOR, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketDestinationFeeder *packet = (BM2MC_PACKET_PacketDestinationFeeder *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, DestinationFeeder, EXECUTE ); \
    packet->plane = (PLANE); \
    packet->color = (COLOR); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_DestinationColor( BUFFER, COLOR, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketDestinationColor *packet = (BM2MC_PACKET_PacketDestinationColor *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, DestinationColor, EXECUTE ); \
    packet->color = (COLOR); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_DestinationNone( BUFFER, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketDestinationNone *packet = (BM2MC_PACKET_PacketDestinationNone *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, DestinationNone, EXECUTE ); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_DestinationControl( BUFFER, ZERO_PAD, CHROMA_FILTER, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketDestinationControl *packet = (BM2MC_PACKET_PacketDestinationControl *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, DestinationControl, EXECUTE ); \
    packet->zero_pad = (ZERO_PAD); \
    packet->chroma_filter = (CHROMA_FILTER); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_OutputFeeder( BUFFER, PLANE, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketOutputFeeder *packet = (BM2MC_PACKET_PacketOutputFeeder *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, OutputFeeder, EXECUTE ); \
    packet->plane = (PLANE); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_OutputControl( BUFFER, DITHER, CHROMA_FILTER, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketOutputControl *packet = (BM2MC_PACKET_PacketOutputControl *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, OutputControl, EXECUTE ); \
    packet->dither = (DITHER); \
    packet->chroma_filter = (CHROMA_FILTER); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_MipmapControl( BUFFER, MIPLEVEL, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketMipmapControl *packet = (BM2MC_PACKET_PacketMipmapControl *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, MipmapControl, EXECUTE ); \
    packet->mipLevel = (MIPLEVEL); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)


/***************************************************************************/
#define BM2MC_PACKET_WRITE_Blend( BUFFER, COLOR_BLEND, ALPHA_BLEND, COLOR, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketBlend *packet = (BM2MC_PACKET_PacketBlend *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, Blend, EXECUTE ); \
    packet->color_blend = (COLOR_BLEND); \
    packet->alpha_blend = (ALPHA_BLEND); \
    packet->color = (COLOR); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_BlendColor( BUFFER, COLOR, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketBlendColor *packet = (BM2MC_PACKET_PacketBlendColor *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, BlendColor, EXECUTE ); \
    packet->color = (COLOR); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_Rop( BUFFER, ROP, PATTERN0, PATTERN1, COLOR0, COLOR1, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketRop *packet = (BM2MC_PACKET_PacketRop *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, Rop, EXECUTE ); \
    packet->rop = (ROP); \
    packet->pattern0 = (PATTERN0); \
    packet->pattern1 = (PATTERN1); \
    packet->color0 = (COLOR0); \
    packet->color1 = (COLOR1); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_SourceColorkey( BUFFER, HIGH, LOW, MASK, REPLACEMENT, REPLACEMENT_MASK, EXCLUSIVE, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketSourceColorkey *packet = (BM2MC_PACKET_PacketSourceColorkey *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, SourceColorkey, EXECUTE ); \
    packet->high = (HIGH); \
    packet->low = (LOW); \
    packet->mask = (MASK); \
    packet->replacement = (REPLACEMENT); \
    packet->replacement_mask = (REPLACEMENT_MASK); \
    packet->exclusive = (EXCLUSIVE); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_SourceColorkeyEnable( BUFFER, ENABLE, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketSourceColorkeyEnable *packet = (BM2MC_PACKET_PacketSourceColorkeyEnable *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, SourceColorkeyEnable, EXECUTE ); \
    packet->enable = (ENABLE); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_DestinationColorkey( BUFFER, HIGH, LOW, MASK, REPLACEMENT, REPLACEMENT_MASK, EXCLUSIVE, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketDestinationColorkey *packet = (BM2MC_PACKET_PacketDestinationColorkey *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, DestinationColorkey, EXECUTE ); \
    packet->high = (HIGH); \
    packet->low = (LOW); \
    packet->mask = (MASK); \
    packet->replacement = (REPLACEMENT); \
    packet->replacement_mask = (REPLACEMENT_MASK); \
    packet->exclusive = (EXCLUSIVE); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_DestinationColorkeyEnable( BUFFER, ENABLE, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketDestinationColorkeyEnable *packet = (BM2MC_PACKET_PacketDestinationColorkeyEnable *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, DestinationColorkeyEnable, EXECUTE ); \
    packet->enable = (ENABLE); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_Filter( BUFFER, HOR, VER, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketFilter *packet = (BM2MC_PACKET_PacketFilter *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, Filter, EXECUTE ); \
    packet->hor = (HOR); \
    packet->ver = (VER); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_FilterEnable( BUFFER, ENABLE, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketFilterEnable *packet = (BM2MC_PACKET_PacketFilterEnable *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, FilterEnable, EXECUTE ); \
    packet->enable = (ENABLE); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_FilterControl( BUFFER, ROUND, ADJUST_COLOR, SUB_ALPHA, ORDER, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketFilterControl *packet = (BM2MC_PACKET_PacketFilterControl *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, FilterControl, EXECUTE ); \
    packet->round = (ROUND); \
    packet->adjust_color = (ADJUST_COLOR); \
    packet->sub_alpha = (SUB_ALPHA); \
    packet->order = (ORDER); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_SourceColorMatrix( BUFFER, MATRIX, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketSourceColorMatrix *packet = (BM2MC_PACKET_PacketSourceColorMatrix *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, SourceColorMatrix, EXECUTE ); \
    packet->matrix = (MATRIX); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_SourceColorMatrixEnable( BUFFER, ENABLE, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketSourceColorMatrixEnable *packet = (BM2MC_PACKET_PacketSourceColorMatrixEnable *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, SourceColorMatrixEnable, EXECUTE ); \
    packet->enable = (ENABLE); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_SourcePalette( BUFFER, ADDRESS, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketSourcePalette *packet = (BM2MC_PACKET_PacketSourcePalette *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, SourcePalette, EXECUTE ); \
    packet->address = (ADDRESS); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_AlphaPremultiply( BUFFER, ENABLE, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketAlphaPremultiply *packet = (BM2MC_PACKET_PacketAlphaPremultiply *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, AlphaPremultiply, EXECUTE ); \
    packet->enable = (ENABLE); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_Mirror( BUFFER, SRC_HOR, SRC_VER, DST_HOR, DST_VER, OUT_HOR, OUT_VER, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketMirror *packet = (BM2MC_PACKET_PacketMirror *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, Mirror, EXECUTE ); \
    packet->src_hor = (SRC_HOR); \
    packet->src_ver = (SRC_VER); \
    packet->dst_hor = (DST_HOR); \
    packet->dst_ver = (DST_VER); \
    packet->out_hor = (OUT_HOR); \
    packet->out_ver = (OUT_VER); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_FixedScale( BUFFER, HPHASE, VPHASE, HSTEP, VSTEP, SHIFT, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketFixedScale *packet = (BM2MC_PACKET_PacketFixedScale *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, FixedScale, EXECUTE ); \
    packet->hor_phase = (HPHASE); \
    packet->ver_phase = (VPHASE); \
    packet->hor_step =  (HSTEP); \
    packet->ver_step =  (VSTEP); \
    packet->shift =     (SHIFT); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_FillBlit( BUFFER, RECT, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketFillBlit *packet = (BM2MC_PACKET_PacketFillBlit *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, FillBlit, EXECUTE ); \
    packet->rect = (RECT); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_CopyBlit( BUFFER, SRC_RECT, OUT_POINT, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketCopyBlit *packet = (BM2MC_PACKET_PacketCopyBlit *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, CopyBlit, EXECUTE ); \
    packet->src_rect = (SRC_RECT); \
    packet->out_point = (OUT_POINT); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_BlendBlit( BUFFER, SRC_RECT, OUT_POINT, DST_POINT, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketBlendBlit *packet = (BM2MC_PACKET_PacketBlendBlit *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, BlendBlit, EXECUTE ); \
    packet->src_rect = (SRC_RECT); \
    packet->out_point = (OUT_POINT); \
    packet->dst_point = (DST_POINT); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_ScaleBlit( BUFFER, SRC_RECT, OUT_RECT, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketScaleBlit *packet = (BM2MC_PACKET_PacketScaleBlit *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, ScaleBlit, EXECUTE ); \
    packet->src_rect = (SRC_RECT); \
    packet->out_rect = (OUT_RECT); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_ScaleBlendBlit( BUFFER, SRC_RECT, OUT_RECT, DST_POINT, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketScaleBlendBlit *packet = (BM2MC_PACKET_PacketScaleBlendBlit *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, ScaleBlendBlit, EXECUTE ); \
    packet->src_rect = (SRC_RECT); \
    packet->out_rect = (OUT_RECT); \
    packet->dst_point = (DST_POINT); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_UpdateScaleBlit( BUFFER, SRC_RECT, OUT_RECT, UPDATE_RECT, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketUpdateScaleBlit *packet = (BM2MC_PACKET_PacketUpdateScaleBlit *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, UpdateScaleBlit, EXECUTE ); \
    packet->src_rect = (SRC_RECT); \
    packet->out_rect = (OUT_RECT); \
    packet->update_rect = (UPDATE_RECT); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_DestripeBlit( BUFFER, SRC_RECT, OUT_RECT, DST_POINT, SOURCE_STRIPE_WIDTH, LUMA_STRIPE_HEIGHT, CHROMA_STRIPE_HEIGHT, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketDestripeBlit *packet = (BM2MC_PACKET_PacketDestripeBlit *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, DestripeBlit, EXECUTE ); \
    packet->src_rect = (SRC_RECT); \
    packet->out_rect = (OUT_RECT); \
    packet->dst_point = (DST_POINT); \
    packet->source_stripe_width = (SOURCE_STRIPE_WIDTH); \
    packet->luma_stripe_height = (LUMA_STRIPE_HEIGHT); \
    packet->chroma_stripe_height = (CHROMA_STRIPE_HEIGHT); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_ResetState( BUFFER, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketResetState *packet = (BM2MC_PACKET_PacketResetState *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, ResetState, EXECUTE ); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_SaveState( BUFFER, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketSaveState *packet = (BM2MC_PACKET_PacketSaveState *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, SaveState, EXECUTE ); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/
#define BM2MC_PACKET_WRITE_RestoreState( BUFFER, EXECUTE ) do \
{ \
    BM2MC_PACKET_PacketRestoreState *packet = (BM2MC_PACKET_PacketRestoreState *) (BUFFER); \
    BM2MC_PACKET_INIT( packet, RestoreState, EXECUTE ); \
    BM2MC_PACKET_TERM( packet ); \
    (BUFFER) = (void *) packet; \
} \
while(0)

/***************************************************************************/

/***************************************************************************
Virtual PACKET to combine all packets
 ***************************************************************************/
typedef struct BM2MC_PACKET_PacketAll {
    union {
        BM2MC_PACKET_PacketSourceFeeder             packetSourceFeeder;
        BM2MC_PACKET_PacketSourceFeeders            packetSourceFeeders;
        BM2MC_PACKET_PacketThreeSourceFeeders       packetSourceThreeFeeders;
        BM2MC_PACKET_PacketStripedSourceFeeders     packetStripedSourceFeeders;
        BM2MC_PACKET_PacketSourceColor              packetSourceColor;
        BM2MC_PACKET_PacketSourceNone               packetSourceNone;
        BM2MC_PACKET_PacketSourceControl            packetSourceControl;
        BM2MC_PACKET_PacketDestinationFeeder        packetDestinationFeeder;
        BM2MC_PACKET_PacketDestinationColor         packetDestinationColor;
        BM2MC_PACKET_PacketDestinationNone          packetDestinationNone;
        BM2MC_PACKET_PacketDestinationControl       packetDestinationControl;
        BM2MC_PACKET_PacketOutputFeeder             packetOutputFeeder;
        BM2MC_PACKET_PacketOutputControl            packetOutputControl;
        BM2MC_PACKET_PacketBlend                    packetBlend;
        BM2MC_PACKET_PacketBlendColor               packetBlendColor;
        BM2MC_PACKET_PacketRop                      packetRop;
        BM2MC_PACKET_PacketSourceColorkey           packetSourceColorkey;
        BM2MC_PACKET_PacketSourceColorkeyEnable     packetSourceColorkeyEnable;
        BM2MC_PACKET_PacketDestinationColorkey      packetDestinationColorkey;
        BM2MC_PACKET_PacketDestinationColorkeyEnable packetDestinationColorkeyEnable;
        BM2MC_PACKET_PacketFilter                   packetFilter;
        BM2MC_PACKET_PacketFilterEnable             packetFilterEnable;
        BM2MC_PACKET_PacketFilterControl            packetFilterControl;
        BM2MC_PACKET_PacketSourceColorMatrix        packetSourceColorMatrix;
        BM2MC_PACKET_PacketSourceColorMatrixEnable  packetSourceColorMatrixEnable;
        BM2MC_PACKET_PacketSourcePalette            packetSourcePalette;
        BM2MC_PACKET_PacketAlphaPremultiply         packetAlphaPremultiply;
        BM2MC_PACKET_PacketAlphaDemultiply          packetAlphaDemultiply;
        BM2MC_PACKET_PacketDestAlphaPremultiply     packetDestAlphaPremultiply;
        BM2MC_PACKET_PacketMirror                   packetMirror;
        BM2MC_PACKET_PacketFixedScale               packetFixedScale;
        BM2MC_PACKET_PacketDestripeFixedScale       packetDestripeFixedScale;
        BM2MC_PACKET_PacketFillBlit                 packetFillBlit;
        BM2MC_PACKET_PacketCopyBlit                 packetCopyBlit;
        BM2MC_PACKET_PacketBlendBlit                packetBlendBlit;
        BM2MC_PACKET_PacketScaleBlit                packetScaleBlit;
        BM2MC_PACKET_PacketScaleBlendBlit           packetScaleBlendBlit;
        BM2MC_PACKET_PacketUpdateScaleBlit          packetUpdateScaleBlit;
        BM2MC_PACKET_PacketDestripeBlit             packetDestripeBlit;
        BM2MC_PACKET_PacketResetState               packetResetState;
        BM2MC_PACKET_PacketSaveState                packetSaveState;
        BM2MC_PACKET_PacketRestoreState             packetRestoreState;
    } data;
} BM2MC_PACKET_PacketAll;

#ifdef __cplusplus
}
#endif

#endif /* BM2MC_PACKET_H__ */

/* end of file */
