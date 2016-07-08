/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#include "bwin_font.h"
#include "bwin_priv.h"
#include "bwin_transform.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
BDBG_MODULE(bwin_font);

#define VERT_SPACING 3

static bwin_font_glyph *bwin_p_get_glyph(bwin_font_t font, bwin_char ch);

int bwin_save_rendered_font(bwin_font_t font, const char *filename)
{
    int i;
    unsigned dummy = 0;
    FILE *f = fopen(filename, "wb+");
    if (!f) return -1;

    fwrite(&(font->version), sizeof(font->version), 1, f);
    fwrite(&(font->refcnt), sizeof(font->refcnt), 1, f);
    fwrite(&dummy, sizeof(dummy), 1, f);
    fwrite(&(font->size), sizeof(font->size), 1, f);
    fwrite(&(font->antialiased), sizeof(font->antialiased), 1, f);
    fwrite(&(font->height), sizeof(font->height), 1, f);
    fwrite(&dummy, sizeof(dummy), 1, f);
    fwrite(&(font->ascender), sizeof(font->ascender), 1, f);
    fwrite(&(font->descender), sizeof(font->descender), 1, f);
    fwrite(&(font->cache_size), sizeof(font->cache_size), 1, f);
    fwrite(&(font->cache_maxsize), sizeof(font->cache_maxsize), 1, f);
    fwrite(&dummy, sizeof(dummy), 1, f);
    fwrite(&dummy, sizeof(dummy), 1, f);
    for (i=0;i<font->cache_size;i++) {
        bwin_font_glyph *glyph = &font->glyph_cache[i];
        fwrite(&(glyph->top), sizeof(glyph->top), 1, f);
        fwrite(&(glyph->left), sizeof(glyph->left), 1, f);
        fwrite(&(glyph->width), sizeof(glyph->width), 1, f);
        fwrite(&(glyph->height), sizeof(glyph->height), 1, f);
        fwrite(&(glyph->pitch), sizeof(glyph->pitch), 1, f);
        fwrite(&(glyph->advance), sizeof(glyph->advance), 1, f);
        fwrite(&dummy, sizeof(dummy), 1, f);
    }

    fwrite(font->char_map, sizeof(bwin_char) * font->cache_size, 1, f);

    for (i=0;i<font->cache_size;i++) {
        bwin_font_glyph *glyph = &font->glyph_cache[i];
        int s = glyph->pitch * glyph->height;
        if (s)
            fwrite(glyph->mem, s, 1, f);
    }
    fclose(f);
    return 0;
}

static bwin_font_t bwin_p_open_rendered_font(bwin_engine_t win, const char *filename)
{
    bwin_font_t font;
    FILE *f;

    f = fopen(filename, "rb");
    if (!f) {
        BDBG_ERR(("Unable to open file %s", filename));
        return NULL;
    }

    font = bwin_open_rendered_font(win, (bwin_readfn_t)fread, f);

    fclose(f);

    if (font) {
        BDBG_MSG(("loaded rendered font: %s", filename));
        bwin_p_add_font_to_cache(filename, font);
    }
    else {
        BDBG_ERR(("unable to load rendered font: %s", filename));
    }

    return font;
}

bwin_font_t bwin_open_rendered_font(bwin_engine_t win, bwin_readfn_t readfn, void *context)
{
    int i, s;
    bwin_font_t font;
    unsigned dummy;

    font = malloc(sizeof(*font));
    if (!font) {
        return NULL;
    }

    if (readfn(&(font->version), sizeof(font->version), 1, context) != 1) {
        BDBG_ERR(("Font read error(2) version"));
        goto error;
    }
    if (font->version != BWIN_FONT_VERSION) {
        BDBG_ERR(("Incorrect font version %d. Code supports %d", font->version, BWIN_FONT_VERSION));
        goto error;
    }

    if (readfn(&(font->refcnt), sizeof(font->refcnt), 1, context) != 1) {
        BDBG_ERR(("Font read error(2) refcnt"));
        goto error;
    }
    if (readfn(&dummy, sizeof(dummy), 1, context) != 1) {
        BDBG_ERR(("Font read error(2) size"));
        goto error;
    }
    if (readfn(&(font->size), sizeof(font->size), 1, context) != 1) {
        BDBG_ERR(("Font read error(2) size"));
        goto error;
    }
    if (readfn(&(font->antialiased), sizeof(font->antialiased), 1, context) != 1) {
        BDBG_ERR(("Font read error(2) antialiased"));
        goto error;
    }
    if (readfn(&(font->height), sizeof(font->height), 1, context) != 1) {
        BDBG_ERR(("Font read error(2) height"));
        goto error;
    }
    if (readfn(&dummy, sizeof(dummy), 1, context) != 1) {
        BDBG_ERR(("Font read error(2) size"));
        goto error;
    }
    if (readfn(&(font->ascender), sizeof(font->ascender), 1, context) != 1) {
        BDBG_ERR(("Font read error(2) ascender"));
        goto error;
    }
    if (readfn(&(font->descender), sizeof(font->descender), 1, context) != 1) {
        BDBG_ERR(("Font read error(2) descender"));
        goto error;
    }
    if (readfn(&(font->cache_size), sizeof(font->cache_size), 1, context) != 1) {
        BDBG_ERR(("Font read error(2) cache_size"));
        goto error;
    }
    if (readfn(&(font->cache_maxsize), sizeof(font->cache_maxsize), 1, context) != 1) {
        BDBG_ERR(("Font read error(2) cache_maxsize"));
        goto error;
    }
    if (readfn(&dummy, sizeof(dummy), 1, context) != 1) {
        BDBG_ERR(("Font read error(2) size"));
        goto error;
    }
    if (readfn(&dummy, sizeof(dummy), 1, context) != 1) {
        BDBG_ERR(("Font read error(2) size"));
        goto error;
    }

    /* reconstruct some values in new context */
    font->win = win;
    font->cache_maxsize = font->cache_size;
    font->refcnt = 0;

    s = sizeof(bwin_font_glyph) * font->cache_size;
    font->glyph_cache = (bwin_font_glyph*)malloc(s);
    for (i=0;i<font->cache_size;i++) {
        bwin_font_glyph *glyph = &font->glyph_cache[i];
        if (readfn(&(glyph->top), sizeof(glyph->top), 1, context) != 1) {
            BDBG_ERR(("Font read error(2) top"));
            goto error;
        }
        if (readfn(&(glyph->left), sizeof(glyph->left), 1, context) != 1) {
            BDBG_ERR(("Font read error(2) left"));
            goto error;
        }
        if (readfn(&(glyph->width), sizeof(glyph->width), 1, context) != 1) {
            BDBG_ERR(("Font read error(2) width"));
            goto error;
        }
        if (readfn(&(glyph->height), sizeof(glyph->height), 1, context) != 1) {
            BDBG_ERR(("Font read error(2) height"));
            goto error;
        }
        if (readfn(&(glyph->pitch), sizeof(glyph->pitch), 1, context) != 1) {
            BDBG_ERR(("Font read error(2) pitch"));
            goto error;
        }
        if (readfn(&(glyph->advance), sizeof(glyph->advance), 1, context) != 1) {
            BDBG_ERR(("Font read error(2) advance"));
            goto error;
        }
        if (readfn(&dummy, sizeof(dummy), 1, context) != 1) {
            BDBG_ERR(("Font read error(2) size"));
            goto error;
        }
    }

    s = sizeof(bwin_char) * font->cache_size;
    font->char_map = (bwin_char*)malloc(s);
    if (readfn(font->char_map, s, 1, context) != 1) {
        BDBG_ERR(("Font read error(3)"));
        goto error;
    }

    /* now reconstruct the individual glyph->mem pointers */
    for (i=0;i<font->cache_size;i++) {
        bwin_font_glyph *glyph = &font->glyph_cache[i];
        int s = glyph->pitch * glyph->height;
        if (s) { /* space has no pixels */
            glyph->mem = malloc(s);
            if (readfn(glyph->mem, s, 1, context) != 1) {
                BDBG_ERR(("Font file is truncated."));
#if 0
                /* run with what we have. it's probably no good, but what the heck. */
                font->cache_size = i;
                break;
#else
                goto error;
#endif
            }
        }
        else
            glyph->mem = NULL;
    }

    return font;

error:
    BDBG_ERR(("%s Unable to load font", __FUNCTION__));
    free(font);
    return NULL;
}

#ifdef FREETYPE_SUPPORT
static bwin_font_t bwin_p_open_freetype_font(bwin_engine_t win, void *context,
                         int size, bool antialiased, unsigned len)
{
    int rc;
    bwin_font_t font;

    if (!win->freetype_init) {
        if (FT_Init_FreeType(&win->freetype)) {
            BDBG_ERR(("FT_Init_FreeType failed"));
            return NULL;
        }
        win->freetype_init = true;
    }

    font = (bwin_font_t)malloc(sizeof(*font));
    if (!font) {
        return NULL;
    }
    memset(font, 0, sizeof(*font));
    font->win = win;
    font->size = size;
    font->antialiased = antialiased;
    font->version = BWIN_FONT_VERSION;

    if(!len){
        rc = FT_New_Face(font->win->freetype, (char*)context, 0, &font->face);
    }
    else {
        rc = FT_New_Memory_Face(font->win->freetype, (FT_Byte*)context, len, 0, &font->face);
    }

    if (rc){
        BDBG_ERR(("FT_New_Face failed"));
        free(font);
        return NULL;
    }

    if (FT_Set_Pixel_Sizes(font->face, 0, font->size)) {
        BDBG_ERR(("FT_Set_Pixel_Sizes failed"));
        free(font);
        return NULL;
    }

    /* convert all metrics to units of pixels */
    font->height = size + VERT_SPACING; /* add a little for spacing */
    font->ascender = font->face->ascender * size / font->face->height;
    font->descender = font->face->descender * size / font->face->height;

    /* Add font to cache only if opening from file */
    if (font && !len) {
        BDBG_MSG(("loaded truetype font: %x", context));
        bwin_p_add_font_to_cache((char*)context, font);
    }

    return font;
}

static bwin_font_t bwin_p_open_freetype_font_italic(bwin_engine_t win, void *context,
                         int size, bool antialiased, unsigned len)
{
    int rc;
    bwin_font_t font;

    FT_Matrix  matrix;

    matrix.xx = 0x10000L;
    matrix.xy = 0.3 * 0x10000L;
    matrix.yx = 0;
    matrix.yy = 0x10000L;

    if (!win->freetype_init) {
        if (FT_Init_FreeType(&win->freetype)) {
            BDBG_ERR(("FT_Init_FreeType failed"));
            return NULL;
        }
        win->freetype_init = true;
    }

    font = (bwin_font_t)malloc(sizeof(*font));
    if (!font) {
        return NULL;
    }
    memset(font, 0, sizeof(*font));
    font->win = win;
    font->size = size;
    font->antialiased = antialiased;
    font->version = BWIN_FONT_VERSION;

    if(!len){
        rc = FT_New_Face(font->win->freetype, (char*)context, 0, &font->face);
    }
    else {
        rc = FT_New_Memory_Face(font->win->freetype, (FT_Byte*)context, len, 0, &font->face);
    }

    if (rc){
        BDBG_ERR(("FT_New_Face failed"));
        free(font);
        return NULL;
    }

    if (FT_Set_Pixel_Sizes(font->face, 0, font->size)) {
        BDBG_ERR(("FT_Set_Pixel_Sizes failed"));
        free(font);
        return NULL;
    }

    FT_Set_Transform(font->face, &matrix, 0 );

    /* convert all metrics to units of pixels */
    font->height = size + VERT_SPACING; /* add a little for spacing */
    font->ascender = font->face->ascender * size / font->face->height;
    font->descender = font->face->descender * size / font->face->height;

    /* Add font to cache only if opening from file */
    if (font && !len) {
        BDBG_MSG(("loaded truetype font: %x", context));
        bwin_p_add_font_to_cache((char*)context, font);
    }

    return font;
}
#endif

bwin_font_t bwin_open_font(bwin_engine_t win, const char *filename,
    int size, bool antialiased)
{
    bwin_font_t font=NULL;

    font = bwin_p_get_font_from_cache(win, filename, size, antialiased);
    if (font)
        return font;

    if (size == -1)
        return bwin_p_open_rendered_font(win, filename);

#ifdef FREETYPE_SUPPORT
    return bwin_p_open_freetype_font(win, (void*)filename, size, antialiased, 0);
#endif

    if (!font) { BDBG_ERR(("%s unable to load font: %s", __FUNCTION__, filename)); }

    return font;
}

bwin_font_t bwin_open_font_italic(bwin_engine_t win, const char *filename,
    int size, bool antialiased)
{
    bwin_font_t font=NULL;

    font = bwin_p_get_font_from_cache(win, filename, size, antialiased);
    if (font)
        return font;

    if (size == -1)
        return bwin_p_open_rendered_font(win, filename);

#ifdef FREETYPE_SUPPORT
    return bwin_p_open_freetype_font_italic(win, (void*)filename, size, antialiased, 0);
#endif

    if (!font) { BDBG_ERR(("%s unable to load font: %s", __FUNCTION__, filename)); }

    return font;
}

void bwin_get_default_font_settings(bwin_font_settings *settings)
{
    memset(settings, 0, sizeof(*settings));
}

bwin_font_t bwin_open_font_generic(bwin_engine_t win, bwin_font_settings *settings)
{
    if(settings->size == -1)
        return bwin_open_rendered_font(win, settings->readfn, settings->context);

#ifdef FREETYPE_SUPPORT
    return bwin_p_open_freetype_font(win, settings->context, settings->size, settings->antialiased, settings->buf_len);
#endif

    BDBG_ERR(("%s unable to load font", __FUNCTION__));

    return NULL;
}

void bwin_close_font(bwin_font_t font)
{
    int i;
    bwin_p_remove_font(font);
#ifdef FREETYPE_SUPPORT
    if (font->face)
        FT_Done_Face(font->face);
#endif
    for (i=0;i<font->cache_size;i++)
        free(font->glyph_cache[i].mem);
    free(font->glyph_cache);
    free(font->char_map);
    free(font);
}

void bwin_get_font_height(bwin_font_t font, unsigned *height)
{
    *height = font->height;
}

#define GET_BUFFER_PTR(FB, X, Y) \
    ((unsigned char *)(FB)->buffer + ((FB)->settings.pitch * (Y)) + ((X) * ((FB)->bpp/8)))

unsigned short antialias_rgb565_pixels(unsigned short bg_pixel, unsigned short fg_pixel,
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

unsigned short antialias_argb1555_pixels(unsigned short bg_pixel, unsigned short fg_pixel,
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
    BSTD_UNUSED(a1);
    BSTD_UNUSED(a2);

#if 0
    if (r1 != g1 || g1 != b1) {
        BDBG_ERR(("Bad antialiasing %x,%x,%x", r1, g1, b1));
    }
#endif

    return COMPOSE_ARGB1555(a1,r1,g1,b1);
}

unsigned int antialias_argb8888_pixels(unsigned int bg_pixel, unsigned int fg_pixel,
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

unsigned int antialias_palette8_pixels(unsigned int bg_pixel, unsigned int fg_pixel,
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
void bwin_p_fb_draw_font_bitmap(bwin_framebuffer_t fb, bwin_font_t font, bwin_font_glyph *glyph,
    int x, int y, uint32_t color, const bwin_rect *cliprect)
{
    unsigned char *b;
    int i, skip_x = 0, skip_y = 0;
    int width, height;
    unsigned char *src;

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
    b = GET_BUFFER_PTR(fb, x+skip_x, y+skip_y);

    switch (font->antialiased) {
    case 1:
        /* blend pixels. they are bytes with grayscale 0..255 */
        src = glyph->mem + glyph->pitch * skip_y;
        for (i=skip_y;i<height;i++) {
            unsigned char *keep = b;
            int j;
            for (j=skip_x;j<width;j++) {
                /* TODO: move this switch statement out of the loop for faster rendering */
                switch (fb->settings.pixel_format) {
                case bwin_pixel_format_r5_g6_b5:
                    *(unsigned short *)b = antialias_rgb565_pixels(*(unsigned short *)b, color, src[j]);
                    b += 2;
                    break;
                case bwin_pixel_format_a1_r5_g5_b5:
                    *(unsigned short *)b = antialias_argb1555_pixels(*(unsigned short *)b, color, src[j]);
                    b += 2;
                    break;
                case bwin_pixel_format_a8_r8_g8_b8:
                    *(unsigned int *)b = antialias_argb8888_pixels(*(unsigned int *)b, color, src[j]);
                    b += 4;
                    break;
                case bwin_pixel_format_palette8:
                    *(unsigned char *)b = antialias_palette8_pixels(*(unsigned int *)b, color, src[j]);
                    b += 1;
                    break;
                }
            }
            b = keep + fb->settings.pitch;
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
                switch (fb->settings.pixel_format) {
                case bwin_pixel_format_r5_g6_b5:
                    if (src[j] >= 128)
                        *(unsigned short *)b = color;
                    b += 2;
                    break;
                case bwin_pixel_format_a1_r5_g5_b5:
                    if (src[j] >= 128)
                        *(unsigned short *)b = color;
                    b += 2;
                    break;
                case bwin_pixel_format_a8_r8_g8_b8:
                    if (src[j] >= 128)
                        *(unsigned int *)b = color;
                    b += 4;
                    break;
                case bwin_pixel_format_palette8:
                    if (src[j] >= 128)
                        *(unsigned char *)b = color;
                    b += 1;
                    break;
                }
            }
            b = keep + fb->settings.pitch;
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
                switch (fb->settings.pixel_format) {
                case bwin_pixel_format_r5_g6_b5:
                    if (isset)
                        *(unsigned short *)b = color;
                    b += 2;
                    break;
                case bwin_pixel_format_a1_r5_g5_b5:
                    if (isset)
                        *(unsigned short *)b = color;
                    b += 2;
                    break;
                case bwin_pixel_format_a8_r8_g8_b8:
                    if (isset)
                        *(unsigned int *)b = color;
                    b += 4;
                    break;
                case bwin_pixel_format_palette8:
                    if (isset)
                        *(unsigned char *)b = color;
                    b += 1;
                    break;
                }
                if (++bit == 8) bit = 0;
            }
            b = keep + fb->settings.pitch;
            src += glyph->pitch;
            bit = skip_y % 8;
        }
        }
        break;
    }
}

void bwin_p_draw_font_bitmap(bwin_t window, bwin_font_t font, bwin_font_glyph *glyph, int x, int y, uint32_t color,
    const bwin_rect *cliprect)
{
    int gx,gy;
    bwin_rect gcliprect;

    /* convert x,y to global */
    bwin_p_convert_to_global(window, x, y, &gx, &gy);

    /* clip to the window */
    if (!cliprect) cliprect = &window->settings.rect;
    gcliprect = *cliprect;

    /* convert cliprect to global coordinates */
    bwin_p_convert_to_global(window, cliprect->x, cliprect->y, &gcliprect.x, &gcliprect.y);

    /* clip to the framebuffer */
    bwin_intersect_rect(&gcliprect, &gcliprect, &window->fb->rect);

/*  printf("fontcolor %06x ==> %06x\n", color, bwin_p_convert_color(window->fb, color)); */
    bwin_p_fb_draw_font_bitmap(window->fb, font, glyph, gx, gy,
        bwin_p_convert_color(window->fb, color), &gcliprect);
}

char bwin_p_get_char(const char *text, bwin_char *ch)
{
    char data = text[0];
    char bytes_per_char=0;

    while(data&0x80){
        data<<=1;
        bytes_per_char++;
    }

    if(!bytes_per_char)
        bytes_per_char=1;

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

    return bytes_per_char;

}

int bwin_draw_text(bwin_t window, bwin_font_t font,
    int x, int y, const char *text, int len, uint32_t color, const bwin_rect *cliprect)
{
    int i;
    int base_y;
    if (len == -1)
        len = strlen(text);

    BDBG_MSG(("bwin_draw_text '%s', %d,%d, 0x%08x", text, x, y, color));

    /* jump down to the baseline */
    base_y = y + VERT_SPACING + font->ascender;

    if (!cliprect) cliprect = &window->settings.rect;

    for (i=0;i<len;) {
        bwin_font_glyph *glyph;
    char bytes_per_char;
    bwin_char ch;

        /* If this char is already past the cliprect, be done */
        if (x >= (int)cliprect->width + cliprect->x ||
            y >= (int)cliprect->height + cliprect->y)
            break;

    bytes_per_char = bwin_p_get_char((text+i), &ch);

        glyph = bwin_p_get_glyph(font, ch);
        if (!glyph)
            return -1;

        /* don't allow kerning to push you outside the left/top of window */
        if (x + glyph->left < 0) {
            x -= glyph->left;
        }
        if (y + glyph->top < 0) {
            y -= glyph->top;
        }

        BDBG_MSG(("  draw %c,%d,%d", ch, x + glyph->left, base_y - glyph->top));
        bwin_p_draw_font_bitmap(window, font, glyph, x + glyph->left, base_y - glyph->top,
            color, cliprect);
        x += glyph->advance;
    i += bytes_per_char;
    }
    return 0;
}

int bwin_get_glyph_metrics(bwin_font_t font, const bwin_char ch, int *width, int *height, int *advance)
{
    bwin_font_glyph *glyph = bwin_p_get_glyph(font, ch);
    if (!glyph)
        return -1;

    *height = font->height;
    *advance = glyph->advance;
    *width = glyph->left + glyph->width;

    return 0;
}

int bwin_measure_text(bwin_font_t font, const char *text,
    int len, int *width, int *height, int *base)
{
    int i;
    if (len == -1)
        len = strlen(text);
    *width = 0;
    *base = 0; /* TODO: */
    *height = font->height;

    for (i=0;i<len;) {
        bwin_font_glyph *glyph;
        char bytes_per_char;
        bwin_char ch;

        bytes_per_char = bwin_p_get_char((text+i), &ch);

        glyph = bwin_p_get_glyph(font, ch);
        if (!glyph)
            return -1;
        *width += glyph->advance;
        i += bytes_per_char;
    }
    return 0;
}

/* TODO:
1. font-on-chromakey mode, apply alpha to alpha-per-pixel and use the whole
font color in every non-zero pixel. this will cause the font to be blended with the
video, not the chromakey color.
2. use 7038 blitter to blit directly from freetype memory to the frame buffer. then
we can blend to video or graphics, even with palettized color.
*/

static bwin_font_glyph *bwin_p_get_glyph(bwin_font_t font, bwin_char ch)
{
    int i;
/* TODO: do we need a hash here? */
    for (i=0;i<font->cache_size;i++)
      if (font->char_map[i] == ch){
            return &font->glyph_cache[i];
      }

#ifdef FREETYPE_SUPPORT
    {
    bwin_font_glyph *glyph;

    FT_UInt glyph_index = FT_Get_Char_Index(font->face, ch);
    FT_GlyphSlot slot = font->face->glyph;
    if (!glyph_index)
        return NULL;
    if (FT_Load_Glyph(font->face, glyph_index, FT_LOAD_DEFAULT))
        return NULL;
    if (FT_Render_Glyph(font->face->glyph,
        font->antialiased?FT_RENDER_MODE_NORMAL:FT_RENDER_MODE_MONO )) {

        if (FT_Render_Glyph(font->face->glyph,
            (!font->antialiased)?FT_RENDER_MODE_NORMAL:FT_RENDER_MODE_MONO )) {
            BDBG_ERR(("Unable to render"));
            return NULL;
        }
        else {
            if (font->antialiased) {
                /* can't antialias. have to use mono */
                BDBG_ERR(("Can't antialias. Using mono instead."));
                font->antialiased = 0;
            }
            else {
                /* switch into quasi-mono mode. see above for details. */
                font->antialiased = 2;
            }
        }
    }

    /* now save it to the cache */
    if (font->cache_size == font->cache_maxsize) {
        bwin_font_glyph *temp_glyph_cache;
        bwin_char *temp_char_map;

        /* extend the cache by 10. This doesn't have to be terribly efficient
        because production systems will run with prerendered fonts. */
        font->cache_maxsize += 10;

        temp_glyph_cache = (bwin_font_glyph *)malloc(font->cache_maxsize * sizeof(bwin_font_glyph));
        temp_char_map = (bwin_char *)malloc(font->cache_maxsize * sizeof(bwin_char));
        memcpy(temp_glyph_cache, font->glyph_cache, font->cache_size * sizeof(bwin_font_glyph));
        memcpy(temp_char_map, font->char_map, font->cache_size * sizeof(bwin_char));
        free(font->glyph_cache);
        free(font->char_map);
        font->glyph_cache = temp_glyph_cache;
        font->char_map = temp_char_map;
    }

    /* store the glyph and all its metrics */
    font->char_map[font->cache_size] = ch;
    glyph = &font->glyph_cache[font->cache_size];
    font->cache_size++;
    glyph->left = slot->bitmap_left;
    glyph->top = slot->bitmap_top;
    glyph->width = slot->bitmap.width;
    glyph->height = slot->bitmap.rows;
    glyph->pitch = slot->bitmap.pitch;
    glyph->advance = slot->advance.x >> 6;
    glyph->mem = malloc(slot->bitmap.rows * slot->bitmap.pitch);
    memcpy(glyph->mem, slot->bitmap.buffer, slot->bitmap.rows * slot->bitmap.pitch);

    BDBG_MSG(("font cache add: %c, %d", ch, font->cache_size));
    return glyph;
    }
#endif

    return NULL;
}

bwin_font_t bwin_p_get_font_from_cache(bwin_engine_t win, const char *filename, int size, bool antialiased)
{
    int i;
    for (i=0;i<MAX_FONT_CACHE;i++) {
        bwin_font_t cfont = win->fontcache[i].font;
        if (cfont &&
            !strcmp(filename, win->fontcache[i].filename) &&
            (size == -1 ||
             ((unsigned)size == cfont->size && (int)antialiased == cfont->antialiased)))
        {
            cfont->refcnt++;
            return cfont;
        }
    }
    return NULL;
}

void bwin_p_remove_font(bwin_font_t font)
{
    if (font->refcnt && --font->refcnt == 0) {
        int i;
        for (i=0;i<MAX_FONT_CACHE;i++) {
            if (font->win->fontcache[i].font == font) {
                font->win->fontcache[i].font = NULL;
                return;
            }
        }
    }
    /* this font wasn't cached. consider increasing MAX_FONT_CACHE */
}

void bwin_p_add_font_to_cache(const char *filename, bwin_font_t font)
{
    int i;
    for (i=0;i<MAX_FONT_CACHE;i++) {
        if (!font->win->fontcache[i].font) {
            font->win->fontcache[i].font = font;
            strncpy(font->win->fontcache[i].filename, filename, BWIN_MAX_FILENAME);
            font->refcnt++;
            return;
        }
    }
    BDBG_ERR(("Font cannot be cached and so it may be reloaded unnecessarily."));
    /* this font wasn't cached. consider increasing MAX_FONT_CACHE */
}
