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

#ifndef BWIN_IMAGE_H__
#define BWIN_IMAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <bstd.h>
#include <bkni.h>

/**
Summary:
An opaque handle for an image.
**/
typedef struct bwin_image *bwin_image_t;

/**
Summary:
Load an image from a file.
Description:
This loads the entire compressed file into memory until bwin_image_close
is called.
**/
bwin_image_t bwin_image_load(
    bwin_engine_t win,
    const char *filename);      /* the name of the file to load */

/**
Summary:
Load an image from a file.
Description:
This opens the image file and saves the file pointer.  Unlike bwin_image_load(),
the image is not read into memory.  This function is useful when a
particular image decompressor accesses the file directly.
**/
bwin_image_t bwin_image_file_io(
    bwin_engine_t win,
    const char *filename);      /* the name of the file to load */

/**
Summary:
Load an image from memory.
Description:
This does not copy the memory but assumes it is available until bwin_image_close
is called.
**/
bwin_image_t bwin_image_load_memory(
    bwin_engine_t win,
    const void *mem,            /* a pointer to the memory from stores the
                                compressed image */
    unsigned length);           /* the length of the image in memory */


/**
Summary:
Close an image and invalidate the handle.
Description:
This frees any memory allocated by bwin_image_load.
**/
void bwin_image_close(bwin_image_t image);

/* Rendering modes:
   Single: draw a single image in the dest rect.
   Stretch: stretch a single image to fit the dest rect
   Tile: tile a single image multiple times horizontally and vertically
    to fit the dest rect. */
typedef enum
{
    bwin_image_render_mode_single, /* no stretch, but blend using alpha-per-pixel */
    bwin_image_render_mode_stretch,
    bwin_image_render_mode_tile,
    bwin_image_render_mode_maximize,
    bwin_image_render_mode_maximize_down,
    bwin_image_render_mode_single_load /* like single, but don't blend using alpha-per-pixel */
} bwin_image_render_mode;

/**
Summary:
Draw an image into a window.
**/
void bwin_image_render(
    bwin_t window,
    bwin_image_t image,
    bwin_image_render_mode mode,/* rendering mode: single, stretch, tile */
    const bwin_rect *destrect,  /* Optional window-coordinates for the image.
                                If NULL, draw at 0,0 using the whole window. */
    const bwin_rect *srcrect,   /* Optional image-coordinates for the source.
                                If NULL, draw the entire image. */
    const bwin_rect *cliprect   /* Optional clip rect */
    );

/**
Summary:
Draw a thumbnail image into a window.
 returns 1 on success, 0 otherwise
**/
int bwin_image_render_thumb(
    bwin_t window, bwin_image_t image,
    bwin_image_render_mode mode,/* rendering mode: single, stretch, tile */
    const bwin_rect *destrect,  /* Optional window-coordinates for the image.
                                If NULL, draw at 0,0 using the whole window. */
    const bwin_rect *cliprect   /* Optional clip rect */
    );

/**
Summary:
A data type for retrieving information about the image
**/
typedef struct
{
    unsigned width; /* width of uncompressed, unscaled image */
    unsigned height; /* height of uncompressed, unscaled image */
    unsigned thumb_width; /* width of uncompress, unscaled thumbnail image */
    unsigned thumb_height; /* height of uncompressed, unscaled thumbnail image */
} bwin_image_settings;

/**
Summary:
Get the image settings from the image.
**/
void bwin_image_get_settings(bwin_image_t image,
    bwin_image_settings *settings);

#ifdef __cplusplus
}
#endif

#endif /* BWIN_IMAGE_H__ */
