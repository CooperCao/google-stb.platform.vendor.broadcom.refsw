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

#include "bwin.h"
#include "bwin_priv.h"
#include "bwin_image.h"
#include "bwin_image_priv.h"
#include "jpeglib.h"
#include "bwin_image_debug.h"
BDBG_MODULE(bwin_image_jpeg);

struct bwin_image_data {
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPARRAY linebuffer;      /* Output row buffer */
};

static void b_init_source(j_decompress_ptr cinfo)
{
    BSTD_UNUSED(cinfo);
    /* nothing */
}
static boolean b_fill_input_buffer(j_decompress_ptr cinfo)
{
    BSTD_UNUSED(cinfo);
    /* already filled */
    return true;
}
static void b_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
    cinfo->src->bytes_in_buffer -= num_bytes;
    cinfo->src->next_input_byte += num_bytes;
}
static void b_term_source(j_decompress_ptr cinfo)
{
    BSTD_UNUSED(cinfo);
    /* nothing */
}

static void b_jpeg_mem_src(j_decompress_ptr cinfo, const unsigned char * buffer, int length)
{
    if (cinfo->src == NULL) {   /* first time for this JPEG object? */
        cinfo->src = (struct jpeg_source_mgr *)
        (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                    sizeof(struct jpeg_source_mgr));
        /* TODO: is this automatically dealloced? */
    }

    cinfo->src->init_source = b_init_source;
    cinfo->src->fill_input_buffer = b_fill_input_buffer;
    cinfo->src->skip_input_data = b_skip_input_data;
    cinfo->src->resync_to_restart = jpeg_resync_to_restart; /* use default*/
    cinfo->src->term_source = b_term_source;
    cinfo->src->bytes_in_buffer = length;
    cinfo->src->next_input_byte = buffer;
}

static void b_jpeg_start(bwin_image_t image)
{
    struct bwin_image_data *jpeg = image->data;
    jpeg->cinfo.err = jpeg_std_error(&jpeg->jerr);
    jpeg_create_decompress(&jpeg->cinfo);

    if (image->fpImage)
    {
        /* image resides in file */
        rewind(image->fpImage);
        jpeg_stdio_src(&jpeg->cinfo, image->fpImage);
    }
    else
    {
        /* image resides in memory buffer */
        b_jpeg_mem_src(&jpeg->cinfo, image->buffer, image->size);
    }

    jpeg_read_header(&jpeg->cinfo, TRUE);
}

static void b_jpeg_stop(bwin_image_t image)
{
    struct bwin_image_data *jpeg = image->data;
    jpeg_destroy_decompress(&jpeg->cinfo);
}

static bwin_result bwin_image_jpeg_init(bwin_image_t image)
{
    struct bwin_image_data *jpeg;

    jpeg = (struct bwin_image_data *)BKNI_Malloc(sizeof(struct bwin_image_data));
    if (!jpeg)
        return bwin_result_allocation_error;
    BKNI_Memset(jpeg, 0, sizeof(struct bwin_image_data));
    image->data = (void*)jpeg;

    b_jpeg_start(image);
    image->settings.width = jpeg->cinfo.image_width;
    image->settings.height = jpeg->cinfo.image_height;
    b_jpeg_stop(image);
    return 0;
}

static void bwin_image_jpeg_finalize(bwin_image_t image)
{
    BKNI_Free(image->data);
    image->data = NULL;
}

static int get_jpeg_src_line(bwin_image_t image, int srcrow)
{
    struct bwin_image_data *jpeg = image->data;
    while (srcrow > image->srcrow) {
        jpeg_read_scanlines(&jpeg->cinfo, jpeg->linebuffer, 1);
        image->srcrow++;
        image->srcrow_buffer = jpeg->linebuffer[0];
    }
    return 0;
}

static void bwin_image_jpeg_render(bwin_framebuffer_t fb, bwin_image_t image,
    const bwin_rect *destrect,const bwin_rect *srcrect,const bwin_rect *clipdestrect)
{
    struct bwin_image_data *jpeg = image->data;
    bwin_image_transform_settings transform_settings;
    int scale;

    bwin_rect rectScaled = *srcrect;
    bwin_framebuffer_settings   fbSettingsMain;

    BDBG_MSG(("jpeg image render begin"));
    bwin_image_transform_settings_init(&transform_settings,
        bwin_pixel_format_r8_g8_b8, fb->settings.pixel_format);

    b_jpeg_start(image);

    BDBG_MSG(("src size %dx%d", image->settings.width, image->settings.height));
    BDBG_MSG(("destrect %dx%d", destrect->width, destrect->height));
    /* calc amount of dct downscaling (use simple integer based rounding) */
    scale = max(((image->settings.width * 10 / destrect->width + 5) / 10),
                ((image->settings.height * 10 / destrect->height + 5) / 10));

    jpeg->cinfo.scale_num = 1;

    if (scale >= 8)
        jpeg->cinfo.scale_denom = 8;
    else if (scale >= 4)
        jpeg->cinfo.scale_denom = 4;
    else if (scale >= 2)
        jpeg->cinfo.scale_denom = 2;
    else
        jpeg->cinfo.scale_denom = 1;

    BDBG_MSG(("JPEG DCT scale 1/%d", jpeg->cinfo.scale_denom));

    jpeg_start_decompress(&jpeg->cinfo);
    BDBG_MSG(("jpeg_render %dx%d", jpeg->cinfo.output_width, jpeg->cinfo.output_height));
    jpeg->linebuffer = (*jpeg->cinfo.mem->alloc_sarray)
        ((j_common_ptr) &jpeg->cinfo, JPOOL_IMAGE,
        jpeg->cinfo.output_width * jpeg->cinfo.output_components, 1);
    BDBG_MSG(("linebuffer size:%d", jpeg->cinfo.output_width * jpeg->cinfo.output_components));

    rectScaled.width  = jpeg->cinfo.output_width;
    rectScaled.height = jpeg->cinfo.output_height;

    bwin_get_framebuffer_settings(fb, &fbSettingsMain);

    if (fbSettingsMain.drawops.copy_rect != bwin_p_copy_rect)
    {
        /*
         * the copy rect function has been overridden (probably to include
         * hardware acceleration) so we'll take advantage of that for
         * jpeg images.  note that we are assuming that this copy
         * rect function can handle scaling.
         *
         * so we will use an offscreen buffer to decompress our image
         * into, and then rely of the overridden copy rect function to
         * to do scaling and blitting to the screen.
         *
         * we draw into the offscreen buffer here since we only want this to
         * apply for jpeg images.  jpeg images are unique in that they allow
         * for dct scaling during decompression...which reduces the size of
         * the offscreen buffer we have to allocate.  since this is known
         * only during decompression, we must allocate the buffer here.
         *
         * 1. create offscreen surface (handle and ptr to memory)
         *
         * 2. use jpeg_read_scanlines() to decompress jpeg line by line and
         *    save to offscreen surface
         *
         * 3. use bwin_copy_rect() to copy/scale/colorcorrect to display
         *
         * 4. destroy offscreen surface
         *
         */

        bwin_framebuffer_t          fbTmp = NULL;
        bwin_framebuffer_settings   fbSettingsTmp;
        int        srcBytesPerPixel = 0;
        int        dstBytesPerPixel = 0;
        uint32_t   srcwidth         = 0;
        uint32_t   destwidth        = 0;
        uint8_t  * pSrc             = NULL;
        uint8_t  * pDest            = NULL;

        BDBG_MSG(("drawing jpeg image with HW scaling"));
        bwin_framebuffer_settings_init(&fbSettingsTmp);
        fbSettingsTmp.data         = fbSettingsMain.data;
        fbSettingsTmp.drawops      = fbSettingsMain.drawops;
        fbSettingsTmp.width        = jpeg->cinfo.output_width;
        fbSettingsTmp.height       = jpeg->cinfo.output_height;
        fbSettingsTmp.pixel_format = bwin_pixel_format_a8_r8_g8_b8;
        fbSettingsTmp.pitch        = bwin_get_bpp(fbSettingsTmp.pixel_format)/8 *
                                                  fbSettingsTmp.width;
        /* null buffer setting will make bwin_open_framebuffer()
         * allocate/deallocate buffer memory for us.
         * it will attempt to use graphics memory if possible in anticipation
         * of bwin_copy_rect() hardware acceleration.
         */
        fbSettingsTmp.buffer       = NULL;

        BDBG_MSG(("temp fb size:%d", fbSettingsTmp.height * fbSettingsTmp.pitch));
        fbTmp = bwin_open_framebuffer(fb->win_engine, &fbSettingsTmp);
        bwin_get_framebuffer_settings(fbTmp, &fbSettingsTmp);

        srcBytesPerPixel = jpeg->cinfo.output_components;
        dstBytesPerPixel = bwin_get_bpp(fbSettingsTmp.pixel_format)/8;
        srcwidth         = jpeg->cinfo.output_width * srcBytesPerPixel;
        destwidth        = fbSettingsTmp.pitch;
        pSrc             = (uint8_t *)jpeg->linebuffer[0];
        pDest            = (uint8_t *)fbSettingsTmp.buffer;

        while (jpeg->cinfo.output_scanline < jpeg->cinfo.output_height)
        {
            uint32_t srcIdx = 0;
            uint32_t dstIdx = 0;

            typedef struct {
                unsigned char r;
                unsigned char g;
                unsigned char b;
            } jpeg_rgb;
#if BSTD_CPU_ENDIAN==BSTD_ENDIAN_LITTLE
            typedef struct {
                unsigned char b;
                unsigned char g;
                unsigned char r;
                unsigned char a;
            } jpeg_argb;
#else
            typedef struct {
                unsigned char a;
                unsigned char r;
                unsigned char g;
                unsigned char b;
            } jpeg_argb;
#endif
            jpeg_rgb  * pSrcColorFmt;
            jpeg_argb * pDstColorFmt;

            jpeg_read_scanlines(&jpeg->cinfo, jpeg->linebuffer, 1);

            /* copy decoded line to our temp bwin framebuffer while
             * adjusting to a compatible color format and endianness */
            while (srcIdx < srcwidth)
            {
                pSrcColorFmt  = (jpeg_rgb *)&(pSrc[srcIdx]);
                pDstColorFmt  = (jpeg_argb *)&(pDest[dstIdx]);

                pDstColorFmt->r = pSrcColorFmt->r;
                pDstColorFmt->g = pSrcColorFmt->g;
                pDstColorFmt->b = pSrcColorFmt->b;
                pDstColorFmt->a = 0xFF;

                srcIdx += srcBytesPerPixel;
                dstIdx += dstBytesPerPixel;
            }

            pDest += destwidth;
        }

        bwin_copy_rect(fb->settings.window, destrect,
                       fbTmp->settings.window, &rectScaled,
                       clipdestrect);

        bwin_close_framebuffer(fbTmp);
    }
    else
    {
        BDBG_MSG(("drawing jpeg image with SW scaling"));
        bwin_image_p_render_lines(fb, image, destrect, &rectScaled, clipdestrect,
            &transform_settings, get_jpeg_src_line);
    }

    jpeg_destroy_decompress(&jpeg->cinfo);

    /* no need to read the unread lines, call jpeg_finish_decompress() */
    /* or jpeg_abort_decompress() because b_jpeg_stop() calls          */
    /* jpeg_destroy_decompress() which does all the clean up for us -  */
    /* see jpeglib.h for more details                                  */
    b_jpeg_stop(image);
    BDBG_MSG(("jpeg image render complete"));
}

int bwin_image_p_is_jpeg(bwin_image_t image)
{
    int rc = -1;

    if (image->size < 11)
        return -1;

    /* TODO: what's the real method? */
    /* adding support for EXchangeable Imga format for Digital Still Camera */

    if (image->fpImage)
    {
        int numRead = 0;
        /* decompressor will read image directly from file
           so use file handle to determine if it is a jpeg */
        char buf[11];
        fpos_t posOrig;
        BKNI_Memset(buf, 0, sizeof(buf));
        fgetpos(image->fpImage, &posOrig);
        rewind(image->fpImage);
        numRead = fread(buf, sizeof(*buf), sizeof(buf), image->fpImage);
        if (0 < numRead)
        {
            rc = fsetpos(image->fpImage, &posOrig);
            rc = (BKNI_Memcmp(&buf[6], "JFIF", 5) && BKNI_Memcmp(&buf[6], "Exif", 4));
        }
    }
    else
    if (image->buffer)
    {
        /* decompressor will read image from memory
           so use buffer to determine if it is a jpeg */
        unsigned char soi[] = { 0xff, 0xd8 };
        rc = (BKNI_Memcmp(image->buffer, soi, 2) &&
              BKNI_Memcmp(&image->buffer[6], "JFIF", 5) && BKNI_Memcmp(&image->buffer[6], "Exif", 4));
    }

    if (!rc) {
        image->init = bwin_image_jpeg_init;
        image->render = bwin_image_jpeg_render;
        image->finalize = bwin_image_jpeg_finalize;
    }

    return rc;
}
