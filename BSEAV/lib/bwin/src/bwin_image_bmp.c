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


#include "bwin.h"
#include "bwin_image.h"
#include "bwin_image_priv.h"

/* set the debug to a higher level for more debug output */
#include "bwin_image_debug.h"
BDBG_MODULE(bwin_image_bmp);

typedef struct {
    unsigned short int type;                 /* Magic identifier            */
    unsigned int size;                       /* File size in bytes          */
    unsigned short int reserved1, reserved2;
    unsigned int offset;                     /* Offset to image data, bytes */
} BMP_HEADER;

typedef struct {
    unsigned int size;               /* Header size in bytes      */
    int width,height;                /* Width and height of image */
    unsigned short int planes;       /* Number of colour planes   */
    unsigned short int bits;         /* Bits per pixel            */
    unsigned int compression;        /* Compression type          */
    unsigned int imagesize;          /* Image size in bytes       */
    int xresolution,yresolution;     /* Pixels per meter          */
    unsigned int ncolours;           /* Number of colours         */
    unsigned int importantcolours;   /* Important colours         */
} BMP_INFOHEADER;

struct bwin_image_data {
    BMP_HEADER header;
    BMP_INFOHEADER info;
};

/* Access the data by bytes so we don't do un-aligned accesses */
#define B_GET_U16(x) (x[1]<<8  | x[0]<<0)
#define B_GET_U32(x) (x[3]<<24 | x[2]<<16 | x[1]<<8 | x[0]<<0)

static bwin_result bwin_image_bmp_init(bwin_image_t image)
{
    struct bwin_image_data *bmp;
    const unsigned char *ptr;

    bmp = (struct bwin_image_data *)BKNI_Malloc(sizeof(struct bwin_image_data));
    if (!bmp)
        return bwin_result_allocation_error;
    BKNI_Memset(bmp, 0, sizeof(struct bwin_image_data));

    /* BMP header format is little endian, access the data appropriate for the platform. */
    ptr = image->buffer;
    bmp->header.type      = B_GET_U16(ptr); ptr += 2;
    bmp->header.size      = B_GET_U32(ptr); ptr += 4;
    bmp->header.reserved1 = B_GET_U16(ptr); ptr += 2;
    bmp->header.reserved2 = B_GET_U16(ptr); ptr += 2;
    bmp->header.offset    = B_GET_U32(ptr); ptr += 4;

    bmp->info.size        = B_GET_U32(ptr); ptr += 4;
    bmp->info.width       = B_GET_U32(ptr); ptr += 4;
    bmp->info.height      = B_GET_U32(ptr); ptr += 4;
    bmp->info.planes      = B_GET_U16(ptr); ptr += 2;
    bmp->info.bits        = B_GET_U16(ptr); ptr += 2;
    bmp->info.compression = B_GET_U32(ptr); ptr += 4;
    bmp->info.imagesize   = B_GET_U32(ptr); ptr += 4;
    bmp->info.xresolution = B_GET_U32(ptr); ptr += 4;
    bmp->info.yresolution = B_GET_U32(ptr); ptr += 4;
    bmp->info.ncolours    = B_GET_U32(ptr); ptr += 4;
    bmp->info.importantcolours = B_GET_U32(ptr); ptr += 4;

    image->data = bmp;

    BDBG_MSG(("image %x, size=%d, offset=%d",
        image->data->header.type, image->data->header.size, image->data->header.offset));
    BDBG_MSG(("info %d, %dx%d",
        image->data->info.size, image->data->info.width, image->data->info.height));
#if 0
    BDBG_MSG(("planes=%d, bits=%d, compression=%d, imagesize=%d, xres=%d, yres=%d, #colors=%d, important colors=%d",
              bmp->info.planes, bmp->info.bits, bmp->info.compression, bmp->info.imagesize,
              bmp->info.xresolution, bmp->info.yresolution, bmp->info.ncolours, bmp->info.importantcolours));
#endif

    /* Here's where all of our limitations come in. */
    if (image->data->info.compression) {
        BDBG_ERR(("file '%s': bmp decompression %d not supported.", image->filename, image->data->info.compression));
        goto error;
    }
    if (image->data->info.ncolours) {
        BDBG_ERR(("file '%s': palettized bmp's not supported. (ncolors=%d)", image->filename, image->data->info.ncolours));
        goto error;
    }
    if (image->data->info.bits != 24) {
        BDBG_ERR(("file '%s': %d bpp not supported.", image->filename, image->data->info.bits));
        goto error;
    }

    image->settings.width = image->data->info.width;
    image->settings.height = image->data->info.height;
    image->pitch = image->data->info.width * 3;
    if (image->pitch % 4)
        image->pitch += 4 - (image->pitch % 4);
    return 0;

  error:
    BKNI_Free(image->data);
    image->data = 0;
    return -1;
}

static void bwin_image_bmp_finalize(bwin_image_t image)
{
    if(image->data!=NULL) {
    BKNI_Free(image->data);
    image->data = NULL;
    }
}

static int get_bmp_src_line(bwin_image_t image, int srcrow)
{
    if (image->srcrow != srcrow) {
        image->srcrow = srcrow;
        /* the bmp buffer starts with the bottom row. */
        image->srcrow_buffer = image->buffer + image->data->header.offset +
            (image->pitch * (image->settings.height - image->srcrow - 1));

        /* ensure here that we aren't going to overrun because of bad metadata */
        if (image->srcrow_buffer + image->pitch - image->buffer > image->size) {
            image->srcrow_buffer = image->buffer + image->data->header.offset;
        }
    }
    return 0;
}

static void bwin_image_bmp_render(
    bwin_framebuffer_t fb,
    bwin_image_t image,
    const bwin_rect *destrect,
    const bwin_rect *srcrect,
    const bwin_rect *clipdestrect /* guaranteed to be inside destrect */
    )
{
    bwin_image_transform_settings transform_settings;
    BKNI_Memset(&transform_settings, 0, sizeof(bwin_image_transform_settings));

    bwin_image_transform_settings_init(&transform_settings,
        bwin_pixel_format_b8_g8_r8, fb->settings.pixel_format);

    bwin_image_p_render_lines(fb, image, destrect, srcrect, clipdestrect,
        &transform_settings, get_bmp_src_line);
}

int bwin_image_p_is_bmp(bwin_image_t image)
{
    int rc;
    if ((unsigned)image->size < sizeof(BMP_HEADER) + sizeof(BMP_INFOHEADER))
        return -1;

    rc = BKNI_Memcmp(image->buffer, "BM", 2);
    if (!rc) {
        image->init = bwin_image_bmp_init;
        image->render = bwin_image_bmp_render;
        image->finalize = bwin_image_bmp_finalize;
    }
    return rc;
}
