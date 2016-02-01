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
#include "bwin_transform.h"

#include "bwin_image_debug.h"
BDBG_MODULE(bwin_image_transform);

/**
There are three basic modes.
1) No alpha in the src or dest. In this case, just transform the src and assign
    it into the dest.
2) Alpha in the dest only. In this case, transform the src and write to the
    dest while preserving the dest's alpha.
3) Alpha in the src. There may or may not be alpha in the dest, but in this case,
    the per-pixel operation is expensive already so just process the dest_alphamask
    anyway. In this case, transform the src, transform the dest, blend them
    together using the src's alpha, then writing the value to dest while preserving
    the dest's alpha.

We're making the following assumptions about src formats
1) 8 bits per color channel (R, G, B)
2) optional 8 bits of alpha

Therefore here are the elemental steps:
1) decompose dest pixel to bytes
2) blend each color channel for dest & src using src alpha
3) compose the new dest pixel from src bytes
4) add the alpha from the original dest pixel
5) assign to dest

Mode 1 uses steps 3 & 5.
Mode 2 uses steps 3-5.
Mode 3 uses steps 1-5.
**/

void bwin_image_p_render_line(bwin_framebuffer_t fb,
    const bwin_rect *destrect,
    const bwin_rect *clipdestrect,
    int destrow, /* row in fb */
    bwin_image_t image,
    const bwin_rect *srcrect,
    const bwin_image_transform_settings *ts)
{
    uint8_t *src_lineptr = (uint8_t *)image->srcrow_buffer;
    uint8_t *dest_ptr;
    unsigned dest_x;
    int bytes_per_dest_pixel = ts->dest_bpp/8;
    int bytes_per_src_pixel = ts->src_bpp/8;
    bool blend = image->render_mode != bwin_image_render_mode_single_load;

    dest_ptr = (unsigned char *)fb->buffer + (fb->settings.pitch * destrow) +
        clipdestrect->x * bytes_per_dest_pixel;

    /* This is the inner loop for scaling images. It should be as tight as
    possible, so this needs some work. */
    for (dest_x = 0; dest_x < clipdestrect->width; dest_x++) {
        unsigned src_x = srcrect->x + (dest_x + (clipdestrect->x - destrect->x)) * \
            srcrect->width / destrect->width;
        uint8_t *src_ptr = src_lineptr + (src_x * bytes_per_src_pixel);
        uint8_t alpha = 0xFF, red = 0, green = 0, blue = 0;

        if (ts->src_alpha && blend) {
            switch (ts->dest_format) {
            case bwin_pixel_format_r5_g6_b5:
                DECOMPOSE_RGB565(*(uint16_t*)dest_ptr, red, green, blue);
                break;
            case bwin_pixel_format_a8_r8_g8_b8:
                DECOMPOSE_ARGB8888(*(uint32_t*)dest_ptr,
                    alpha, red, green, blue);
                break;
            case bwin_pixel_format_a1_r5_g5_b5:
                DECOMPOSE_ARGB1555(*(uint16_t*)dest_ptr, alpha, red, green, blue);
                break;
            default: break;
            }

            /* blend with src */
            BLEND(red,src_ptr[ts->red_index],src_ptr[ts->alpha_index]);
            BLEND(green,src_ptr[ts->green_index],src_ptr[ts->alpha_index]);
            BLEND(blue,src_ptr[ts->blue_index],src_ptr[ts->alpha_index]);
        }
        else {
            /* assign src */
            if (ts->src_alpha) {
                alpha = src_ptr[ts->alpha_index];
            }
            red = src_ptr[ts->red_index];
            green = src_ptr[ts->green_index];
            blue = src_ptr[ts->blue_index];
        }

#if 0
        printf("src: %x,%x,%x (%x) ==> ", src_ptr[ts->red_index],
            src_ptr[ts->green_index], src_ptr[ts->blue_index], src_ptr[ts->alpha_index]);
        printf("dest: %x,%x,%x,%x\n", alpha, red, green, blue);
#endif

        switch (ts->dest_format) {
        case bwin_pixel_format_r5_g6_b5:
            *(uint16_t*)dest_ptr = COMPOSE_RGB565(red, green, blue);
            break;
        case bwin_pixel_format_a8_r8_g8_b8:
            *(uint32_t*)dest_ptr = COMPOSE_ARGB8888(alpha, red, green, blue);
            break;
        case bwin_pixel_format_a1_r5_g5_b5:
            *(uint16_t*)dest_ptr = COMPOSE_ARGB1555(alpha, red, green, blue);
            break;
        default: break;
        }

        dest_ptr += bytes_per_dest_pixel;
    }

#if 0
    /* temp */
    dest_ptr = fb->buffer + (fb->settings.pitch * destrow) +
        clipdestrect->x * bytes_per_dest_pixel;
    int x;
    for (x=0;x<destrect->width*bytes_per_dest_pixel;x++)
        printf("%02x ",((uint8_t*)dest_ptr)[x]);
    printf("\n");
#endif
}

void bwin_image_transform_settings_init(
    bwin_image_transform_settings* ts,
    bwin_pixel_format src_format,
    bwin_pixel_format dest_format)
{
    switch ((unsigned)src_format) {
    case bwin_pixel_format_r8_g8_b8:
        /* TODO: verify this on big endian systems for all image
        formats. is the src format in host endianness? */
        ts->red_index = 0;
        ts->green_index = 1;
        ts->blue_index = 2;
        ts->src_alpha = false;
        ts->src_bpp = 24;
        break;
    case bwin_pixel_format_b8_g8_r8:
        /* TODO: verify this on big endian systems for all image
        formats. is the src format in host endianness? */
        ts->red_index = 2;
        ts->green_index = 1;
        ts->blue_index = 0;
        ts->src_alpha = false;
        ts->src_bpp = 24;
        break;
    case bwin_pixel_format_a8_r8_g8_b8:
        ts->alpha_index = 0;
        ts->red_index = 1;
        ts->green_index = 2;
        ts->blue_index = 3;
        ts->src_alpha = true;
        ts->src_bpp = 32;
        break;
    default:
        /* internal unsupported format. something is wrong. */
        BDBG_ASSERT(false);
    }

    ts->dest_format = dest_format;

    switch (dest_format) {
    case bwin_pixel_format_r5_g6_b5:
    case bwin_pixel_format_a1_r5_g5_b5:
        ts->dest_bpp = 16;
        break;
    case bwin_pixel_format_a8_r8_g8_b8:
        ts->dest_bpp = 32;
        break;
    case bwin_pixel_format_palette8:
        /* should never have come here */
        return;
    }
}
