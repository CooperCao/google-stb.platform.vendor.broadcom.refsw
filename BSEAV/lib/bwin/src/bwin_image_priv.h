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


#ifndef BWIN_IMAGE_PRIV_H__
#define BWIN_IMAGE_PRIV_H__

#include "bwin.h"
#include "bwin_priv.h"
#include "bwin_image.h"
#include "bwin_transform.h"

/* Memory scheme
1. bwin_image_load_file loads compressed image into memory, then calls bwin_image_load_memory
2. bwin_image_load_memory reads format information
3. bwin_image_render handles all clipping, tiling/stretch modes, and conversion to global
   coordinates. Then it calls the image-specific render function.
4. The image-specific render function uncompressed the image line-by-line
    and calls the line renderer.
3. close will free memory
*/

/**
Summary:
Opaque type-specific data pointer.  Each implementation of an image format
will have its own private struct defining this.
**/
typedef struct bwin_image_data *bwin_image_data_t;

/**
Summary:
A private implementation of the image structure for bwin.
**/
struct bwin_image
{
    bwin_engine_t win;                          /* pointer to bwin engine */
    bwin_image_settings settings;               /* image settings */

    const unsigned char *buffer;                /* compressed buffer */
    int size;                                   /* size of buffer */
    int pos;                                    /* current read position in buffer */
    bool allocated;                             /* if true, then we must free buffer */

    const unsigned char *srcrow_buffer;         /* current uncompressed row */
    int srcrow;                                 /* index for srcrow_buffer, if -1,
                        then srcrow_buffer is invalid. */
    int pitch;                                  /* pitch of uncompressed row */

    bwin_image_data_t data;                     /* type-specific data pointer */

    FILE * fpImage;                             /* if valid, renderer is to access image file directly */
    char filename[256];                         /* only valid if fpImage is valid */
    bwin_image_render_mode render_mode;         /* current render mode */

    /* loads settings */
    bwin_result (*init)(bwin_image_t image);

    /* image-specific renderer */
    void (*render)(bwin_framebuffer_t fb,
        bwin_image_t image,
        const bwin_rect *destrect,
        const bwin_rect *srcrect,
        const bwin_rect *clipdestrect);

    /* clean up after init and possible render */
    void (*finalize)(bwin_image_t image);
};

/**
Callback from bwin_image_p_render_lines for retrieving a line of src
**/
typedef int (*bwin_image_get_src_line_func)(
    bwin_image_t image,
    int srcrow /* the line to retrieve */
    );

/**
 A generic line-based rendering algorithm. This must be called from
each modules render rountine.
**/
void bwin_image_p_render_lines(
    bwin_framebuffer_t fb,
    bwin_image_t image,
    const bwin_rect *destrect,
    const bwin_rect *srcrect,
    const bwin_rect *clipdestrect,
    const bwin_image_transform_settings *transform_settings,
    bwin_image_get_src_line_func get_src_line
    );

#endif /* BWIN_IMAGE_PRIV_H__ */
