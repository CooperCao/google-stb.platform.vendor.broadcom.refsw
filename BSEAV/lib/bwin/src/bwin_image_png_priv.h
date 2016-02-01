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


#ifndef BWIN_IMAGE_PNG_PRIV_H__
#define BWIN_IMAGE_PNG_PRIV_H__

#include <png.h>

#include "bwin_image.h"

/**
Summary:
The private image data structure for PNG-format graphics.
**/
struct bwin_image_data
{
    png_structp png_ptr;
    png_infop info_ptr;
    int bit_depth;
    int color_type;
    png_color_8p sig_bit;
    png_color_16p bgcolor;
    int row_bytes;
    png_bytepp row_pointers;
    unsigned char *uncompressed_buffer;         /* see image->pitch for pitch */

/* DECOMPRESS_ONCE is a hack for platforms that don't have the cpu guts to
decompress the image quickly. Instead, chew up memory by storing the
decompress image after the first render. */
#if 0
#define DECOMPRESS_ONCE
#endif

#ifdef DECOMPRESS_ONCE
    bwin_pixel_format stored_pixel_format;
#endif
};

/**
Summary:
Set up the PNG library interface.
Returns:
Zero if the setup succeeded, non-zero otherwise.
**/
bwin_result bwin_image_png_setup(bwin_image_t image);

/**
Summary:
Tear down the PNG library interface.
**/
void bwin_image_png_teardown(bwin_image_t image);

/**
Summary:
Read the info chunk from a PNG-format image.
**/
void bwin_image_png_read_info(bwin_image_t image);

/**
Summary:
Set the transformations we want libpng to perform during the
read of the image.
**/
void bwin_image_png_set_read_transforms(bwin_image_t image,
    bwin_pixel_format *pixel_format);

/**
Summary:
Create the PNG library interface structures.
Returns:
Zero if the allocation succeeded, non-zero otherwise.
**/
bwin_result bwin_image_png_create_struct(bwin_image_t image);

/**
Summary:
Destroy the PNG library interface structures.
**/
void bwin_image_png_destroy_struct(bwin_image_t image);

/**
Summary:
Create the buffer for the specified PNG image.
Returns:
Zero if the allocation succeeded, non-zero otherwise.
**/
bwin_result bwin_image_png_create_buffer(bwin_image_t image);

/**
Summary:
Destroy the buffer for the specified PNG image.
**/
void bwin_image_png_destroy_buffer(bwin_image_t image);

#endif /* BWIN_IMAGE_PNG_PRIV_H__ */
