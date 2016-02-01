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


#include <stdio.h>
#include <string.h>

#include "bwin.h"
#include "bwin_image.h"
#include "bwin_image_priv.h"
#include "bwin_image_debug.h"
#include "bwin_transform.h"
#ifdef EXIF_SUPPORT
#include "exif-data.h"
#endif
BDBG_MODULE(bwin_image);

void bwin_image_settings_init(bwin_image_settings *settings)
{
    memset(settings, 0, sizeof(*settings));
}

#ifdef PNG_SUPPORT
extern int bwin_image_p_is_png(bwin_image_t image);
#endif
#ifdef JPEG_SUPPORT
extern int bwin_image_p_is_jpeg(bwin_image_t image);
#endif
extern int bwin_image_p_is_bmp(bwin_image_t image);

static int bwin_image_p_discover_type(bwin_image_t image)
{
    /* if extension type id failed */
#ifdef PNG_SUPPORT
    if (!bwin_image_p_is_png(image)) {
        BDBG_MSG(("File '%s': image is png", image->filename));
        goto reset;
    }
#endif
#ifdef JPEG_SUPPORT
    if (!bwin_image_p_is_jpeg(image)) {
        BDBG_MSG(("File '%s': image is jpeg", image->filename));
        goto reset;
    }
#endif
    if (!bwin_image_p_is_bmp(image)) {
        BDBG_MSG(("File '%s': image is bmp", image->filename));
        goto reset;
    }

    /* We can always try using the file extension, but I'm not sure if
    it's worth it at this point. */
    BDBG_ERR(("File '%s': unable to determine image type", image->filename));
    return -1;

reset:
    /* we've read some data, but now we need to back up for a normal read */
    image->pos = 0;
    return 0;
}

static bwin_image_t bwin_image_p_init(bwin_engine_t engine, bwin_image_t image)
{
    image->pos = 0;
    image->srcrow = -1; /* srcrow_buffer is invalid */

    /* determine the type */
    if (bwin_image_p_discover_type(image))
        goto error;

    /* initialize the image data structure */
    if (image->init(image))
        goto error;

    image->settings.thumb_width  = image->settings.width;
    image->settings.thumb_height = image->settings.height;

#ifndef EXIF_SUPPORT
    (void)engine; /* touch it so we don't get warning */
#else
    {
        ExifData * pEXIF = NULL;

        /* look up true thumbnail width/height
         * we will unload the thumb after getting the width/height to save memory.
         * note that the thumbnail generation will be loaded on demand (when
         * rendering).  this will save memory (thumbnails are technically not
         * limited to any particular size) and since thumbnails are
         * typically very small, will not significantly hamper rendering
         * performance.
         */
        if (image->fpImage)
            pEXIF = exif_data_new_from_file(image->filename);
        else if ((image->buffer) && (image->size > 0))
            pEXIF = exif_data_new_from_data(image->buffer, image->size);

        if ((pEXIF) && (pEXIF->size > 0))
        {
            bwin_image_t tmpImage = NULL;
            bwin_image_settings tmpSettings;

            tmpImage = bwin_image_load_memory(engine, (void *)pEXIF->data, (size_t)pEXIF->size);
            bwin_image_get_settings(tmpImage, &tmpSettings);

            image->settings.thumb_width  = tmpSettings.width;
            image->settings.thumb_height = tmpSettings.height;

            bwin_image_close(tmpImage);
        }
        if (pEXIF)
        {
            exif_data_free(pEXIF);
        }
    }
#endif

    return image;

error:
    bwin_image_close(image);
    return NULL;
}

#if defined(__unix__)
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#elif defined(__vxworks)
#include  <sys/stat.h>
#elif defined(_WIN32_WCE)
#include <windows.h>
#include <Winbase.h>
#endif

static int b_filesize(const char *filename)
{
#ifndef _WIN32_WCE
    struct stat st;
    if (stat((char*)filename, &st))
        return -1;
    return st.st_size;
#else
    WIN32_FILE_ATTRIBUTE_DATA   fileData;
    TCHAR*     wchName;
    BOOL    bRslt;
    int str_size = strlen(filename);

    wchName = (TCHAR*)malloc(sizeof(TCHAR) *(str_size + 1));
    mbstowcs(wchName, filename, str_size+1);
    bRslt = GetFileAttributesEx(wchName, GetFileExInfoStandard, &fileData);
    free(wchName);
    if( bRslt)
        return (int)fileData.nFileSizeLow;
    else
        return -1;
#endif /* _WIN32_WCE */
}

/* This opens the file but doesn't read it in (yet) */
bwin_image_t bwin_image_file_io(bwin_engine_t win, const char *filename)
{
    bwin_image_t image;
    FILE *file;

    int size = b_filesize(filename);

    BSTD_UNUSED(win); /* TODO: Use win, store in struct? */

    if (size == -1) {
        BDBG_ERR(("Unable to get filesize for file '%s'", filename));
        return NULL;
    }
    BDBG_MSG(("getting %d bytes from file '%s'", size, filename));

    image = (bwin_image_t)BKNI_Malloc(sizeof(*image));
    if (!image)
        return NULL;
    memset(image, 0, sizeof(*image));

    image->size = size;

    file = fopen(filename, "rb");
    if (!file)
        goto error;
    image->fpImage = file;
    /* SW7335-1354, Coverity: 33636 */
    strncpy(image->filename, filename, sizeof(image->filename)-1);

    return bwin_image_p_init(win, image);

error:
    bwin_image_close(image);
    return NULL;
}

/* This opens the file, reads it into a buffer, and closes the file */
bwin_image_t bwin_image_load(bwin_engine_t win, const char *filename)
{
    bwin_image_t image;
    FILE *file;

    int size = b_filesize(filename);

    BSTD_UNUSED(win); /* TODO: Use win, store in struct? */

    if (size == -1) {
        BDBG_ERR(("Unable to read filesize of file '%s'", filename));
        return NULL;
    }
    BDBG_MSG(("reading %d bytes from file '%s'", size, filename));

    image = (bwin_image_t)BKNI_Malloc(sizeof(*image));
    if (!image)
        return NULL;
    memset(image, 0, sizeof(*image));

    image->size = size;

    image->buffer = BKNI_Malloc(size);
    if (!image->buffer)
        goto error;
    image->allocated = true;

    file = fopen(filename, "rb");
    if (!file)
        goto error;
    image->fpImage = file;
    /* SW7335-1354, Coverity: 33637 */
    strncpy(image->filename, filename, sizeof(image->filename)-1);

    /* read the entire file into memory */
    if ((size_t)fread((void*)image->buffer, 1, size, file) != (size_t)size)
        goto error;
    fclose(file);
    image->fpImage = NULL;

    return bwin_image_p_init(win, image);

error:
    bwin_image_close(image);
    return NULL;
}

bwin_image_t bwin_image_load_memory(bwin_engine_t win, const void *mem, unsigned length)
{
    bwin_image_t image;
    BSTD_UNUSED(win); /* TODO: Use win, store in struct? */

    image = (bwin_image_t)BKNI_Malloc(sizeof(*image));
    if (!image)
        return NULL;
    memset(image, 0, sizeof(*image));

    image->buffer = BKNI_Malloc(length);
    if (!image->buffer) {
        BKNI_Free(image);
        return NULL;
    }

    BKNI_Memcpy((void *)image->buffer, mem, length);
    image->allocated = true;
    image->size = length;

    return bwin_image_p_init(win, image);
}

void bwin_image_get_settings(bwin_image_t image, bwin_image_settings *settings)
{
    *settings = image->settings;
}

void bwin_image_close(bwin_image_t image)
{
    if (!image) return;

    BDBG_MSG(("closing image %p (buffer=%p, size=%d, file='%s')",
              image, image->buffer, image->size, image->fpImage?image->filename:"none"));

    if (image->finalize)
        image->finalize(image);
    if (image->allocated)
        BKNI_Free((void*)image->buffer);
    if (image->fpImage)
    {
        fclose(image->fpImage);
        image->filename[0]='\0';
    }

    BKNI_Free(image);
}

int bwin_image_render_thumb(bwin_t window, bwin_image_t image,
                             bwin_image_render_mode mode,
                             const bwin_rect *destrect,
                             const bwin_rect *cliprect)
{
    int bRet = 0;

    if (cliprect && cliprect->width == 0 && cliprect->height == 0)
    {
        /* nothing to draw so don't bother rendering */
        BDBG_WRN(("bwin_image_render_thumb: cancelling render request because cliprect area is 0"));
        return bRet;
    }

    if (!window)
    {
        BDBG_WRN(("bwin_image_render_thumb: cancelling render request because given window is invalid"));
        return bRet;
    }

    if (!image)
    {
        BDBG_WRN(("bwin_image_render_thumb: cancelling render request because given image is invalid"));
        return bRet;
    }

#ifdef EXIF_SUPPORT
    {
        /* extract thumb if possible */
        ExifData * pEXIF = NULL;

        if (image->fpImage)
            pEXIF = exif_data_new_from_file(image->filename);
        else if ((image->buffer) && (image->size > 0))
            pEXIF = exif_data_new_from_data(image->buffer, image->size);

        if ((pEXIF) && (pEXIF->size > 0))
        {
            bwin_image_t tmpImage = NULL;
            tmpImage = bwin_image_load_memory(window->fb->win_engine, pEXIF->data, pEXIF->size);
            bwin_image_render(window, tmpImage, mode, destrect, NULL, cliprect);
            bwin_image_close(tmpImage);
            exif_data_free(pEXIF);
            bRet = 1;
        }
    }
#else
    /* exif not supported so just render full sized image */
    bwin_image_render(window, image, mode, destrect, NULL, cliprect);
    bRet = 1;
#endif

    return bRet;
}

void bwin_image_render(bwin_t window, bwin_image_t image,
    bwin_image_render_mode mode,
    const bwin_rect *destrect, const bwin_rect *srcrect,
    const bwin_rect *cliprect)
{
    /* default to whole src -> whole window */
    bwin_rect actual_srcrect = {0,0,0,0};
    bwin_rect actual_destrect;
    bwin_rect gcliprect;

    if (!window)
    {
        BDBG_WRN(("bwin_image_render: cancelling render request because given window is invalid"));
        return;
    }

    if (!image)
    {
        BDBG_WRN(("bwin_image_render: cancelling render request because given image is invalid"));
        return;
    }

    if (cliprect && cliprect->width == 0 && cliprect->height == 0)
    {
        /* nothing to draw so don't bother rendering */
        BDBG_WRN(("bwin_image_render: cancelling render request because cliprect area is 0"));
        return;
    }

    actual_destrect = window->settings.rect;

    actual_srcrect.width = image->settings.width;
    actual_srcrect.height = image->settings.height;

    if (window->fb->settings.pixel_format == bwin_pixel_format_palette8) {
        BDBG_ERR(("Cannot render image to palettized surface"));
        return;
    }

    /* src coordiates are always relative to the image only */
    if (srcrect)
        bwin_intersect_rect(&actual_srcrect, &actual_srcrect, srcrect);

    /* dest coordinates are relative to the window and must be clipped and converted */
    if (destrect)
        bwin_intersect_rect(&actual_destrect, &actual_destrect, destrect);

    bwin_p_convert_to_global(window, actual_destrect.x, actual_destrect.y,
        &actual_destrect.x, &actual_destrect.y);

    /* compute global clipping rect */
    if (!cliprect)
        cliprect = &window->settings.rect;
    gcliprect = *cliprect;
    bwin_p_convert_to_global(window, cliprect->x, cliprect->y, &gcliprect.x, &gcliprect.y);
    /* clip to the framebuffer */
    bwin_intersect_rect(&gcliprect, &gcliprect, &window->fb->rect);

    image->render_mode = mode;
    switch (mode) {
    case bwin_image_render_mode_single:
    case bwin_image_render_mode_single_load:
        /* intersect the dest and cliprect now to get the correct final region */
        bwin_intersect_rect(&gcliprect, &gcliprect, &actual_destrect);
        /* then set the dest size to be the same as the src size */
        actual_destrect.width = actual_srcrect.width;
        actual_destrect.height = actual_srcrect.height;
        /* fall through */
    case bwin_image_render_mode_stretch:
    case bwin_image_render_mode_maximize:
    case bwin_image_render_mode_maximize_down:
        {
        bwin_rect clipdestrect;
        bwin_intersect_rect(&clipdestrect, &gcliprect, &actual_destrect);
        BDBG_MSG(("image->render %d,%d,%d,%d (%d,%d,%d,%d)",
            actual_destrect.x, actual_destrect.y, actual_destrect.width, actual_destrect.height,
            clipdestrect.x, clipdestrect.y, clipdestrect.width, clipdestrect.height));
        image->render(window->fb, image, &actual_destrect, &actual_srcrect, &clipdestrect);
        }
        break;
    case bwin_image_render_mode_tile:
        {
        unsigned x,y;
        for (x=0; x<actual_destrect.width; x+=actual_srcrect.width)
            for (y=0; y<actual_destrect.height; y+=actual_srcrect.height) {
                bwin_rect tiled_destrect;
                bwin_rect clipdestrect;

                tiled_destrect.x = actual_destrect.x+x;
                tiled_destrect.y = actual_destrect.y+y;
                tiled_destrect.width = actual_srcrect.width;
                tiled_destrect.height = actual_srcrect.height;

                bwin_intersect_rect(&clipdestrect, &gcliprect, &tiled_destrect);
                image->render(window->fb, image, &tiled_destrect, &actual_srcrect, &clipdestrect);
            }
        }
        break;
    }
}

/**
This function is called from the image-specific render function
in order to render all the lines. It requires a callback to the image-specific
module in order to get each source line.
**/
void bwin_image_p_render_lines(
    bwin_framebuffer_t fb,
    bwin_image_t image,
    const bwin_rect *destrect,
    const bwin_rect *srcrect,
    const bwin_rect *clipdestrect,
    const bwin_image_transform_settings *transform_settings,
    bwin_image_get_src_line_func get_src_line)
{
    unsigned destrow;

    /* invalidate the buffer before call the get_src_line callback */
    image->srcrow = -1;

    /* step through each line of the final destination */
    for (destrow=0;destrow<clipdestrect->height;destrow++) {
        /* compute src row for this destrow */
        int srcrow = srcrect->y + (destrow + (clipdestrect->y - destrect->y)) * srcrect->height / destrect->height;
/*        BDBG_MSG(("row %d ==> %d", srcrow, destrow)); */

        /* request the line from the image-specific module */
        if (get_src_line(image, srcrow))
            break;

#if 0
        BDBG_MSG(("line %x %x %x %x",
            (uint8_t*)image->srcrow_buffer[0],
            (uint8_t*)image->srcrow_buffer[1],
            (uint8_t*)image->srcrow_buffer[2],
            (uint8_t*)image->srcrow_buffer[3]));
#endif

        /* now render the line */
        bwin_image_p_render_line(fb, destrect, clipdestrect,
            destrow+clipdestrect->y, image, srcrect, transform_settings);
    }
}
