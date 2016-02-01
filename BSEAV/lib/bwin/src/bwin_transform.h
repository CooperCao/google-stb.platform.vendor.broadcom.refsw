/******************************************************************************
 * (c) 2004-2014 Broadcom Corporation
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


#ifndef BWIN_TRANSFORM_H__
#define BWIN_TRANSFORM_H__

/**
Summary:
This is a continuation of the public bwin_pixel_format for internal
use only.
**/
typedef enum
{
    bwin_pixel_format_r8_g8_b8 = 100, /* used by JPEG and non-alpha PNG's */
    bwin_pixel_format_b8_g8_r8 = 101 /* used by BMP */
    /* In the future, there might be more */
} bwin_pixel_format_extension;

typedef struct bwin_image_transform_settings bwin_image_transform_settings;

void bwin_image_p_render_line(bwin_framebuffer_t fb,
    const bwin_rect *destrect,
    const bwin_rect *clipdestrect,
    int destrow, /* row in fb */
    bwin_image_t image,
    const bwin_rect *srcrect,
    const bwin_image_transform_settings *transform_settings);

/**
Summary: Transformation settings for the software scaler and blter
**/
struct bwin_image_transform_settings
{
    int red_index, green_index, blue_index, alpha_index;
    bool src_alpha;
    bwin_pixel_format dest_format;
    int src_bpp, dest_bpp;
};

/**
Summary: A function for initializing the transform settings
**/
void bwin_image_transform_settings_init
(
    bwin_image_transform_settings* p_transform_settings,
    bwin_pixel_format src_format,
    bwin_pixel_format dest_format
);

/**
The following DECOMPOSE/COMPOSE macros convert packed pixel
formats to and from one byte per color channel.
**/

#define DECOMPOSE_RGB565(pixel,r,g,b) do {\
    r = ((pixel) & 0xF800) >> 8; \
    g = ((pixel) & 0x07E0) >> 3; \
    b = ((pixel) & 0x001F) << 3; \
    } while(0)

#define COMPOSE_RGB565(r,g,b) (\
    ((unsigned short)(r) & 0xF8) << 8 | \
    ((unsigned short)(g) & 0xFC) << 3 | \
    ((unsigned short)(b) & 0xF8) >> 3 \
    )

#define DECOMPOSE_ARGB1555(pixel,a,r,g,b) do {\
    a = ((pixel) & 0x8000)?0xFF:00; \
    r = ((pixel) & 0x7C00) >> 7; \
    g = ((pixel) & 0x03E0) >> 2; \
    b = ((pixel) & 0x001F) << 3; \
    } while(0)

#define COMPOSE_ARGB1555(a,r,g,b) (\
    ((a)?0x8000:0x0000) | \
    ((unsigned short)(r) & 0xF8) << 7 | \
    ((unsigned short)(g) & 0xF8) << 2 | \
    ((unsigned short)(b) & 0xF8) >> 3 \
    )

#define DECOMPOSE_ARGB8888(pixel,a,r,g,b) do {\
    a = ((pixel) & 0xFF000000) >> 24; \
    r = ((pixel) & 0x00FF0000) >> 16; \
    g = ((pixel) & 0x0000FF00) >> 8; \
    b = ((pixel) & 0x000000FF); \
    } while(0)

#define COMPOSE_ARGB8888(a,r,g,b) (\
    ((unsigned long)(a) & 0xFF) << 24 | \
    ((unsigned long)(r) & 0xFF) << 16 | \
    ((unsigned long)(g) & 0xFF) << 8 | \
    ((unsigned long)(b) & 0xFF) \
    )

#define BLEND(DEST,SRC,ALPHA) \
    (DEST) = (((SRC)-(DEST))*(ALPHA)/255) + (DEST)

#endif /* BWIN_TRANSFORM_H__ */
