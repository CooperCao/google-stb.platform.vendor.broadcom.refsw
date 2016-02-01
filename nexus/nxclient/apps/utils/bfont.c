/******************************************************************************
 *    (c)2013-2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *****************************************************************************/
#include "bfont.h"
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

BDBG_MODULE(bfont);

/*********************************
*
* this code was copied from BSEAV/lib/bwin/src/bwin_font.c, bwin_transform.h, etc.
*
**********************************/

/*********************************
*
* begin bwin_font binary format
* if these structures change, increment the BWIN_FONT_VERSION number
*
**********************************/
#define BWIN_FONT_VERSION 1
typedef struct {
    int top;
    int left;
    unsigned width; /* in pixels */
    unsigned height; /* in pixels */
    unsigned pitch; /* linesize, in bytes */
    unsigned advance; /* in pixels, x axis */
    unsigned char *mem; /* pointer into bwin_font.glyph_mem */
} bwin_font_glyph;

struct bfont {
    int version;
    int refcnt; /* for the bwin_engine cache */
    void *win; /* included for binary compat */
    unsigned size;
    int antialiased; /* 0 is mono, 1 is antialiased,
        2 is quasi-mono. see bwin_font.c for an explanation of this. */
    unsigned height; /* max height, in pixels */
    void *face; /* included for binary compat */
    int ascender; /* in pixels */
    int descender; /* in pixels */
    unsigned cache_size; /* number of glyphs and chars in glyph_cache and char_map */
    int cache_maxsize; /* maximum number of entries in glyph_cache and char_map. corresponds to mem allocated */
    bwin_font_glyph *glyph_cache; /* points to array of size cache_size */
    bfont_char *char_map; /* points to array of size cache_size */
};
/*********************************
*
* end bwin_font binary format
*
**********************************/

#define VERT_SPACING 3

#undef min
#define min(A,B) ((A)<(B)?(A):(B))
#undef max
#define max(A,B) ((A)>(B)?(A):(B))

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

static bwin_font_glyph *bwin_p_get_glyph(bfont_t font, bfont_char ch);

#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
static void swap(void *buffer, size_t size)
{
    unsigned i;
    uint32_t *ptr = buffer;
    for (i=0;i<size/4;i++) {
        ptr[i] = (ptr[i] >> 24) | ((ptr[i] >> 8)&0xFF00) | ((ptr[i] << 8)&0xFF0000) | (ptr[i] << 24);
    }
}
#else
#define swap(buffer, size)
#endif

bfont_t bfont_open(const char *filename)
{
    bfont_t font;
    FILE *f;

    f = fopen(filename, "rb");
    if (!f) {
        BDBG_ERR(("Unable to open file %s", filename));
        return NULL;
    }

    font = bfont_callback_open((bfont_readfn_t)fread, f);

    fclose(f);

    return font;
}

bfont_t bfont_callback_open(bfont_readfn_t readfn, void *context)
{
    unsigned i, s;
    bfont_t font;

    font = BKNI_Malloc(sizeof(*font));
    if (!font) {
        return NULL;
    }

    if (readfn(font, sizeof(*font), 1, context) != 1) {
        BDBG_ERR(("Font read error(1)"));
        goto error;
    }
    swap(font, sizeof(*font));

    if (font->version != BWIN_FONT_VERSION) {
        BDBG_ERR(("Incorrect font version %d. Code supports %d", font->version, BWIN_FONT_VERSION));
        goto error;
    }

    /* reconstruct some values in new context */
    font->cache_maxsize = font->cache_size;
    font->refcnt = 0;

    s = sizeof(bwin_font_glyph) * font->cache_size;
    font->glyph_cache = (bwin_font_glyph*)BKNI_Malloc(s);
    if (readfn(font->glyph_cache, s, 1, context) != 1) {
        BDBG_ERR(("Font read error(2)"));
        goto error;
    }
    swap(font->glyph_cache, s);

    s = sizeof(bfont_char) * font->cache_size;
    font->char_map = (bfont_char*)BKNI_Malloc(s);
    if (readfn(font->char_map, s, 1, context) != 1) {
        BDBG_ERR(("Font read error(3)"));
        goto error;
    }
    swap(font->char_map, s);

    /* now reconstruct the individual glyph->mem pointers */
    for (i=0;i<font->cache_size;i++) {
        bwin_font_glyph *glyph = &font->glyph_cache[i];
        int s = glyph->pitch * glyph->height;
        if (s) { /* space has no pixels */
            glyph->mem = BKNI_Malloc(s);
            if (readfn(glyph->mem, s, 1, context) != 1) {
                BDBG_ERR(("Font file is truncated."));
                goto error;
            }
        }
        else {
            glyph->mem = NULL;
        }
    }

    /* sort char_map and glyph_cache for binary search in bwin_p_get_glyph */
    for (i=0;i<font->cache_size;i++) {
        unsigned j;
        for (j=i+1;j<font->cache_size;j++) {
            if (font->char_map[j] < font->char_map[i]) {
                bfont_char temp_char = font->char_map[i];
                bwin_font_glyph temp_glyph = font->glyph_cache[i];
                font->char_map[i] = font->char_map[j];
                font->glyph_cache[i] = font->glyph_cache[j];
                font->char_map[j] = temp_char;
                font->glyph_cache[j] = temp_glyph;
            }
        }
    }

    return font;

error:
    BDBG_ERR(("Unable to load font"));
    BKNI_Free(font);
    return NULL;
}

void bfont_close(bfont_t font)
{
    unsigned i;
    for (i=0;i<font->cache_size;i++) {
        if (font->glyph_cache[i].mem) {
            BKNI_Free(font->glyph_cache[i].mem);
        }
    }
    BKNI_Free(font->glyph_cache);
    BKNI_Free(font->char_map);
    BKNI_Free(font);
}

void bfont_get_height(bfont_t font, unsigned *height)
{
    *height = font->height;
}

static unsigned short antialias_rgb565_pixels(unsigned short bg_pixel, unsigned short fg_pixel,
    int alpha)
{
    int r1,g1,b1;
    int r2,g2,b2;
    unsigned short result;
    int temp;

    /* no change */
    if (alpha <= 0)
        return bg_pixel;
    else if (alpha >= 255)
        return fg_pixel;

    DECOMPOSE_RGB565(bg_pixel,r1,g1,b1);
    DECOMPOSE_RGB565(fg_pixel,r2,g2,b2);

#if 0
    BDBG_MSG(("alpha %d", alpha));
    BDBG_MSG(("%d, %d, %d",r1,g1,b1));
    BDBG_MSG(("%d, %d, %d",r2,g2,b2));
#endif

/* TODO: this rounding code should be moved into a macro so it can be used for
other pixel formats. Always handle alpha in a per-pixel_format manner. */
    /* blend them */
    temp = (r2-r1) * alpha;
    r1 = (temp/255) + r1;
    if (temp % 255 > 127) /* rounding */
        r1++;

    temp = (g2-g1) * alpha;
    g1 = (temp/255) + g1;
    if (temp % 255 > 127) /* rounding */
        g1++;

    temp = (b2-b1) * alpha;
    b1 = (temp/255) + b1;
    if (temp % 255 > 127) /* rounding */
        b1++;

    result = COMPOSE_RGB565(r1,g1,b1);

#if 0
    BDBG_MSG(("%d, %d, %d ==> %06x",r1,g1,b1, result));
#endif

    return result;
}

static unsigned short antialias_argb1555_pixels(unsigned short bg_pixel, unsigned short fg_pixel,
    int alpha)
{
    int a1,r1,g1,b1;
    int a2,r2,g2,b2;

    /* no change */
    if (alpha <= 0)
        return bg_pixel;
    else if (alpha >= 255)
        return fg_pixel;

    DECOMPOSE_ARGB1555(bg_pixel,a1,r1,g1,b1);
    DECOMPOSE_ARGB1555(fg_pixel,a2,r2,g2,b2);

    /* TODO: rounding like rgb565 */

    /* blend them */
    BLEND(r1,r2,alpha);
    BLEND(g1,g2,alpha);
    BLEND(b1,b2,alpha);
    /* don't change a */

#if 0
    if (r1 != g1 || g1 != b1) {
        BDBG_ERR(("Bad antialiasing %x,%x,%x", r1, g1, b1));
    }
#endif

    return COMPOSE_ARGB1555(a1,r1,g1,b1);
}

static unsigned long antialias_argb8888_pixels(unsigned long bg_pixel, unsigned long fg_pixel,
    int alpha)
{
    int a1,r1,g1,b1;
    int a2,r2,g2,b2;

    /* no change */
    if (alpha <= 0)
        return bg_pixel;
    else if (alpha >= 255)
        return fg_pixel;

    DECOMPOSE_ARGB8888(bg_pixel,a1,r1,g1,b1);
    DECOMPOSE_ARGB8888(fg_pixel,a2,r2,g2,b2);

    /* blend them */
    BLEND(r1,r2,alpha);
    BLEND(g1,g2,alpha);
    BLEND(b1,b2,alpha);
    BLEND(a1,a2,alpha); /* TODO: is this right? */

    return COMPOSE_ARGB8888(a1,r1,g1,b1);
}

static unsigned long antialias_palette8_pixels(unsigned long bg_pixel, unsigned long fg_pixel,
    int alpha)
{
#if 1
    /* simple monochrome, non-antialiased decision */
    if (alpha <= 100)
        return bg_pixel;
    else
        return fg_pixel;
#else
    /* antialias with 4 pixel values, starting with fg_pixel. see note in
    header file documentation for bwin_draw_text. */
    BSTD_UNUSED(bg_pixel);
    /* guarantee values of 0..3 by limiting the value of alpha to 0..255. */
    if (alpha < 0)
        alpha = 0;
    else if (alpha > 255)
        alpha = 255;
    return fg_pixel + (alpha/64);
#endif
}

/* global coordinates, converted color, correct cliprect */
static void bwin_p_fb_draw_font_bitmap(struct bfont_surface_desc *pSurfaceDesc, bfont_t font, bwin_font_glyph *glyph,
    int x, int y, uint32_t color, const NEXUS_Rect *cliprect)
{
    unsigned char *b;
    int i, skip_x = 0, skip_y = 0;
    int width, height;
    unsigned char *src;
    unsigned bytesPerPixel;
    
    switch (pSurfaceDesc->pixelFormat) {
    case NEXUS_PixelFormat_eR5_G6_B5: 
    case NEXUS_PixelFormat_eA1_R5_G5_B5:
        bytesPerPixel = 2; 
        break;
    case NEXUS_PixelFormat_eA8_R8_G8_B8:
        bytesPerPixel = 4; 
        break;
    case NEXUS_PixelFormat_ePalette8:
        bytesPerPixel = 1; 
        break;
    default:
        return;
    }    

    /* perform clipping */
    if (x < cliprect->x)
        skip_x = cliprect->x - x;
    else
        skip_x = 0;
    if (y < cliprect->y)
        skip_y = cliprect->y - y;
    else
        skip_y = 0;
    width = min((int)glyph->width,(int)(cliprect->x+cliprect->width-x));
    height = min((int)glyph->height,(int)(cliprect->y+cliprect->height-y));
    if (width <= 0 || height <= 0)
        return;

    /* get starting pointers */
    b = (unsigned char *)pSurfaceDesc->buffer + (pSurfaceDesc->pitch * (y+skip_y)) + (bytesPerPixel*(x+skip_x));

    switch (font->antialiased) {
    case 1:
        /* blend pixels. they are bytes with grayscale 0..255 */
        src = glyph->mem + glyph->pitch * skip_y;
        for (i=skip_y;i<height;i++) {
            unsigned char *keep = b;
            int j;
            for (j=skip_x;j<width;j++) {
                /* TODO: move this switch statement out of the loop for faster rendering */
                switch (pSurfaceDesc->pixelFormat) {
                case NEXUS_PixelFormat_eR5_G6_B5:
                    *(unsigned short *)b = antialias_rgb565_pixels(*(unsigned short *)b, color, src[j]);
                    b += 2;
                    break;
                case NEXUS_PixelFormat_eA1_R5_G5_B5:
                    *(unsigned short *)b = antialias_argb1555_pixels(*(unsigned short *)b, color, src[j]);
                    b += 2;
                    break;
                case NEXUS_PixelFormat_eA8_R8_G8_B8:
                    *(unsigned long *)b = antialias_argb8888_pixels(*(unsigned long *)b, color, src[j]);
                    b += 4;
                    break;
                case NEXUS_PixelFormat_ePalette8:
                    *(unsigned char *)b = antialias_palette8_pixels(*(unsigned long *)b, color, src[j]);
                    b += 1;
                    break;
                default:
                    return;
                }
            }
            b = keep + pSurfaceDesc->pitch;
            src += glyph->pitch;
        }
        break;
    case 2:
        /* quasi-mono mode. We couldn't open in mono mode, but we can for antialias.
        In this case, use the midpoint for mono. */
        src = glyph->mem + glyph->pitch * skip_y;
        for (i=skip_y;i<height;i++) {
            unsigned char *keep = b;
            int j;
            for (j=skip_x;j<width;j++) {
                /* TODO: move this switch statement out of the loop for faster rendering */
                switch (pSurfaceDesc->pixelFormat) {
                case NEXUS_PixelFormat_eR5_G6_B5:
                    if (src[j] >= 128)
                        *(unsigned short *)b = color;
                    b += 2;
                    break;
                case NEXUS_PixelFormat_eA1_R5_G5_B5:
                    if (src[j] >= 128)
                        *(unsigned short *)b = color;
                    b += 2;
                    break;
                case NEXUS_PixelFormat_eA8_R8_G8_B8:
                    if (src[j] >= 128)
                        *(unsigned long *)b = color;
                    b += 4;
                    break;
                case NEXUS_PixelFormat_ePalette8:
                    if (src[j] >= 128)
                        *(unsigned char *)b = color;
                    b += 1;
                    break;
                default:
                    return;
                }
            }
            b = keep + pSurfaceDesc->pitch;
            src += glyph->pitch;
        }
        break;
    case 0:
        {
        /* draw pixels. we're reading bits. */
        int bit = skip_y % 8;
        src = glyph->mem + glyph->pitch * skip_y/8;
        /* bit is a bit index from MSB->LSB */
        for (i=skip_y;i<height;i++) {
            unsigned char *keep = b;
            int j;
            for (j=skip_x;j<width;j++) {
                /* read the bit for this pixel */
                bool isset = src[j/8] & (0x1 << (7-bit));
                /* TODO: move this switch statement out of the loop for faster rendering */
                switch (pSurfaceDesc->pixelFormat) {
                case NEXUS_PixelFormat_eR5_G6_B5:
                    if (isset)
                        *(unsigned short *)b = color;
                    b += 2;
                    break;
                case NEXUS_PixelFormat_eA1_R5_G5_B5:
                    if (isset)
                        *(unsigned short *)b = color;
                    b += 2;
                    break;
                case NEXUS_PixelFormat_eA8_R8_G8_B8:
                    if (isset)
                        *(unsigned long *)b = color;
                    b += 4;
                    break;
                case NEXUS_PixelFormat_ePalette8:
                    if (isset)
                        *(unsigned char *)b = color;
                    b += 1;
                    break;
                default:
                    return;
                }
                if (++bit == 8) bit = 0;
            }
            b = keep + pSurfaceDesc->pitch;
            src += glyph->pitch;
            bit = skip_y % 8;
        }
        }
        break;
    }
}

static unsigned bwin_p_get_char(const char *text, unsigned size, bfont_char *ch, bfont_text_type text_type)
{
    unsigned bytes_per_char = 0;
    switch (text_type) {
    case bfont_text_type_ascii:
        if (size < 1) break;
        *ch = text[0];
        bytes_per_char = 1;
        break;
    case bfont_text_type_utf32:
        if (size < 4) break;
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
        *ch = (bfont_char)text[3] | (bfont_char)text[2]<<8 | (bfont_char)text[1]<<16 | (bfont_char)text[0]<<24;
#else
        *ch = (bfont_char)text[0] | (bfont_char)text[1]<<8 | (bfont_char)text[2]<<16 | (bfont_char)text[3]<<24;
#endif
        bytes_per_char = 4;
        break;
    case bfont_text_type_utf8:
        {
        unsigned char data = text[0];

        while(data&0x80){
            data<<=1;
            bytes_per_char++;
        }
        if (!bytes_per_char) {
            bytes_per_char=1;
        }
        if (size < bytes_per_char) {
            bytes_per_char = 0;
            break;
        }

        switch(bytes_per_char){
        default:
        case 1:
            *ch = (uint32_t)text[0];
            break;
        case 2:
            *ch = ((uint32_t)text[0]&0x1F)<<6 | ((uint32_t)text[1]&0x3F);
            break;
        case 3:
            *ch = ((uint32_t)text[0]&0xF)<<12 | ((uint32_t)text[1]&0x3F)<<6 | ((uint32_t)text[2]&0x3F);
            break;
        case 4:
            *ch = ((uint32_t)text[0]&0x3)<<18 | ((uint32_t)text[1]&0x3F)<<12 | ((uint32_t)text[1]&0x3F)<<6 | ((uint32_t)text[3]&0x3F);
            break;
        }
        }
        break;
    default:
        break;
    }

    return bytes_per_char;
    
}

static int bfont_p_draw_text(struct bfont_surface_desc *pSurfaceDesc, bfont_t font,
    int x, int y, const char *text, int len, uint32_t color, const NEXUS_Rect *cliprect, bfont_text_type text_type)
{
    int i;
    int base_y;
    NEXUS_Rect defaultClipRect;
    
    if (len == -1) {
        if (text_type == bfont_text_type_utf32) return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        len = strlen(text);
    }
    if (text_type == bfont_text_type_utf32) {
        len *= 4;
    }

    BDBG_MSG(("bwin_draw_text '%s', %d,%d, 0x%08x", text, x, y, color));

    /* jump down to the baseline */
    base_y = y + VERT_SPACING + font->ascender;

    if (!cliprect || !cliprect->width || !cliprect->height) {
        defaultClipRect.x = 0;
        defaultClipRect.y = 0;
        defaultClipRect.width = pSurfaceDesc->width;
        defaultClipRect.height = pSurfaceDesc->height;
        cliprect = &defaultClipRect;
    }

    for (i=0;i<len;) {
        bwin_font_glyph *glyph;
        unsigned bytes_per_char;
        bfont_char ch;

        /* If this char is already past the cliprect, be done */
        if (x >= (int)cliprect->width + cliprect->x ||
            y >= (int)cliprect->height + cliprect->y)
            break;
           
        bytes_per_char = bwin_p_get_char((text+i), len-i, &ch, text_type);
        if (!bytes_per_char) {
            BDBG_MSG(("bwin_p_get_char failed: %d, %d", i, len-i));
            return -1;
        }
    
        glyph = bwin_p_get_glyph(font, ch);
        if (!glyph) {
            BDBG_MSG(("bwin_p_get_glyph failed: %c %c(%u) bytes_per_char=%d", *(text+i), (unsigned char)ch, ch, bytes_per_char));
            return -1;
        }

        /* don't allow kerning to push you outside the left/top of window */
        if (x + glyph->left < 0) {
            x -= glyph->left;
        }
        if (y + glyph->top < 0) {
            y -= glyph->top;
        }

        BDBG_MSG(("  draw %c(%u),%d,%d", (unsigned char)ch, ch, x + glyph->left, base_y - glyph->top));
        bwin_p_fb_draw_font_bitmap(pSurfaceDesc, font, glyph, x + glyph->left, base_y - glyph->top,
            color, cliprect);
        x += glyph->advance;
        i += bytes_per_char;
    }
    return 0;
}

int bfont_draw_text(struct bfont_surface_desc *pSurfaceDesc, bfont_t font,
    int x, int y, const char *text, int len, uint32_t color, const NEXUS_Rect *cliprect)
{
    return bfont_p_draw_text(pSurfaceDesc, font, x, y, text, len, color, cliprect, bfont_text_type_utf8);
}

int bfont_get_glyph_metrics(bfont_t font, bfont_char ch, int *width, int *height, int *advance)
{
    bwin_font_glyph *glyph = bwin_p_get_glyph(font, ch);
    if (!glyph)
        return -1;

    *height = font->height;
    *advance = glyph->advance;
    *width = glyph->left + glyph->width;

    return 0;
}

static int bfont_p_measure_text(bfont_t font, const char *text,
    int len, int *width, int *height, int *base, bfont_text_type text_type)
{
    int i;
    if (len == -1) {
        if (text_type == bfont_text_type_utf32) return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        len = strlen(text);
    }
    if (text_type == bfont_text_type_utf32) {
        len *= 4;
    }

    *width = 0;
    *base = 0; /* TODO: */
    *height = font->height;
  
    for (i=0;i<len;) {
        bwin_font_glyph *glyph;
        char bytes_per_char;
        bfont_char ch;

        bytes_per_char = bwin_p_get_char((text+i), len-i, &ch, text_type);

        glyph = bwin_p_get_glyph(font, ch);
        if (!glyph)
            return -1;
        *width += glyph->advance;
        i += bytes_per_char;
    }
    return 0;
}

int bfont_measure_text(bfont_t font, const char *text,
    int len, int *width, int *height, int *base)
{
    return bfont_p_measure_text(font, text, len, width, height, base, bfont_text_type_utf8);
}

static bwin_font_glyph *bwin_p_get_glyph(bfont_t font, bfont_char ch)
{
    /* binary search */
    unsigned low = 0; /* inclusive */
    unsigned high = font->cache_size; /* exclusive */
    while (high > low) {
        unsigned mid = low + (high-low)/2;
        if (font->char_map[mid] == ch) {
            return &font->glyph_cache[mid];
        }
        else if (font->char_map[mid] < ch) {
            low = mid+1;
        }
        else {
            high = mid;
        }
    }
    return NULL;
}

void bfont_get_surface_desc(NEXUS_SurfaceHandle surface, struct bfont_surface_desc *pDesc)
{
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_SurfaceMemory mem;
    
    memset(pDesc, 0, sizeof(*pDesc));
    NEXUS_Surface_GetCreateSettings(surface, &createSettings);
    NEXUS_Surface_GetMemory(surface, &mem);
    pDesc->buffer = mem.buffer;
    pDesc->pitch = mem.pitch;
    pDesc->width = createSettings.width;
    pDesc->height = createSettings.height;
    pDesc->pixelFormat = createSettings.pixelFormat;
}

void bfont_get_default_draw_text_settings(bfont_draw_text_settings *psettings)
{
    memset(psettings, 0, sizeof(*psettings));
}

int bfont_draw_aligned_text(struct bfont_surface_desc *desc, bfont_t font, const NEXUS_Rect *pRect, const char *text, int len, uint32_t color,
    bfont_valign valign, bfont_halign halign)
{
    bfont_draw_text_settings settings;
    bfont_get_default_draw_text_settings(&settings);
    settings.valign = valign;
    settings.halign = halign;
    return bfont_draw_text_ex(desc, font, pRect, text, len, color, &settings);
}

int bfont_draw_text_ex(struct bfont_surface_desc *desc, bfont_t font, const NEXUS_Rect *pRect, const char *text, int len, uint32_t color, const bfont_draw_text_settings *psettings )
{
    unsigned x, y;
    int width, height, base;
    int rc;
    
    rc = bfont_p_measure_text(font, text, len, &width, &height, &base, psettings->text_type);
    if (rc) {
        return bfont_p_draw_text(desc, font, 0, 0, text, len, color, pRect, psettings->text_type);
    }
    if (pRect->height) {
        switch (psettings->valign) {
        case bfont_valign_top: y = pRect->y; break;
        case bfont_valign_center: y = pRect->y + ((int)pRect->height - height)/2; break;
        case bfont_valign_bottom: y = pRect->y + (int)pRect->height - height; break;
        default: return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
    }
    else {
        y = pRect->y;
    }
    if (pRect->width) {
        switch (psettings->halign) {
        case bfont_halign_left: x = pRect->x; break;
        case bfont_halign_center: x = pRect->x + ((int)pRect->width - width)/2; break;
        case bfont_halign_right: x = pRect->x + (int)pRect->width - width; break;
        default: return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
    }
    else {
        x = pRect->x;
    }
    
    return bfont_p_draw_text(desc, font, x, y, text, len, color, pRect, psettings->text_type);
}
