/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/


#include <png.h>

#include "bwin.h"
#include "bwin_priv.h"
#include "bwin_image.h"
#include "bwin_image_priv.h"
#include "bwin_image_png_priv.h"

#include "bwin_image_debug.h"

#include <string.h>

BDBG_MODULE(bwin_image_png);

static bwin_result bwin_image_png_init(bwin_image_t image)
{
    struct bwin_image_data *png;
    bwin_result rc = 0;

    DBG_ENTER(bwin_image_png_init);

    /* allocate png-specific data structure memory */
    png = (struct bwin_image_data *)BKNI_Malloc(sizeof(struct bwin_image_data));
    if (!png)
        return bwin_result_allocation_error;
    memset(png, 0, sizeof(struct bwin_image_data));
    image->data = png;

    /* need these here for getting image info before we render, without
    loading the image data itself due to memory constraints */

    /* setup png stuff */
    rc = bwin_image_png_setup(image);
    if (!rc) {
        /* read the info and then discard everything */
        bwin_image_png_read_info(image);
        bwin_image_png_teardown(image);
    }

    DBG_LEAVE(bwin_image_png_init);
    return rc;
}

static void bwin_image_png_finalize(bwin_image_t image)
{
    BKNI_Free(image->data);
    image->data = NULL;
}

static bwin_result bwin_image_png_load(bwin_image_t image,
    bwin_pixel_format *pixel_format)
{
    bwin_result rc = 0;

    /* set up the data structures and source pointers */
    rc = bwin_image_png_setup(image);
    if (rc) return rc;

    /* read the image info */
    bwin_image_png_read_info(image);

    /* set up read transformations   */
    bwin_image_png_set_read_transforms(image, pixel_format);

    /* create a buffer to store the image */
    rc = bwin_image_png_create_buffer(image);

    /* check if we got a buffer or ran out of heap space */
    if (!rc) /* no problem with buffer */
    {
        /* read the image into the buffer */
        DBG_CALL(png_read_image);
        png_read_image(image->data->png_ptr, image->data->row_pointers);

        /* finish reading, don't care about end data */
        DBG_CALL(png_read_end);
        png_read_end(image->data->png_ptr, NULL);
    }
    else /* problem with allocation */
    {
        /* make sure the image doesn't try to render */
        image->render = NULL;
    }

    /* kill the data structures and source pointers */
    /* since we don't need them after we've copied the image */
    /* to the buffer */
    bwin_image_png_teardown(image);

    DBG_LEAVE(bwin_image_png_load);
    return rc;
}

static bwin_result bwin_image_png_unload(bwin_image_t image)
{
    bwin_image_png_destroy_buffer(image);
    return 0;
}


static int get_png_src_line(bwin_image_t image, int srcrow)
{
    if (image->srcrow != srcrow) {
        image->srcrow = srcrow;
        image->srcrow_buffer = image->data->uncompressed_buffer + (image->pitch * image->srcrow);

        /* ensure here that we aren't going to overrun because of bad metadata */
        if (image->srcrow_buffer + image->pitch - image->data->uncompressed_buffer > (int)(image->pitch * image->settings.height)) {
            image->srcrow_buffer = image->data->uncompressed_buffer;
        }
    }
    return 0;
}

/* render from the compressed buffer to the framebuffer */
static void bwin_image_png_render(
    bwin_framebuffer_t fb,
    bwin_image_t image,
    const bwin_rect *destrect,
    const bwin_rect *srcrect,
    const bwin_rect *clipdestrect /* guaranteed to be inside destrect */
    )
{
    bwin_image_transform_settings transform_settings;
    bwin_pixel_format pixel_format;

    /* This loads the entire image into an uncompressed buffer. We could do it line-based,
    but that would require we handle interlacing. I don't know how many interlaced
    png's are out there, but that's the issue. */
#ifdef DECOMPRESS_ONCE
    if (image->data->row_pointers) {
        pixel_format = image->data->stored_pixel_format;
    }
    else {
        if (bwin_image_png_load(image, &pixel_format)) goto error;
        image->data->stored_pixel_format = pixel_format;
    }
#else
    if (bwin_image_png_load(image, &pixel_format)) goto error;
#endif

    bwin_image_transform_settings_init(&transform_settings,
        pixel_format, fb->settings.pixel_format);

    bwin_image_p_render_lines(fb, image, destrect, srcrect, clipdestrect,
        &transform_settings, get_png_src_line);

error:
#ifndef DECOMPRESS_ONCE
    bwin_image_png_unload(image);
#endif
}

int bwin_image_p_is_png(bwin_image_t image)
{
    int rc = 0;
#define PNG_BYTES_TO_CHECK 4
    if (image->size < PNG_BYTES_TO_CHECK)
        return -1;

    if (image->fpImage)
    {
        int numRead = 0;
        /* decompressor will read image directly from file
           so use file handle to determine if it is a jpeg */
        char buf[PNG_BYTES_TO_CHECK];
        fpos_t posOrig;
        BKNI_Memset(buf, 0, sizeof(buf));
        fgetpos(image->fpImage, &posOrig);
        rewind(image->fpImage);
        numRead = fread(buf, sizeof(*buf), sizeof(buf), image->fpImage);
        if (0 < numRead)
        {
            rc = fsetpos(image->fpImage, &posOrig);
            rc = png_sig_cmp((void*)buf, 0, PNG_BYTES_TO_CHECK);
        }
    }
    else
    if (image->buffer)
    {
        /* decompressor will read image from memory
           so use buffer to determine if it is a jpeg */
        rc = png_sig_cmp((void*)image->buffer, 0, PNG_BYTES_TO_CHECK);
    }

    if (!rc) {
        image->init = bwin_image_png_init;
        image->render = bwin_image_png_render;
        image->finalize = bwin_image_png_finalize;
    }
    return rc;
}
