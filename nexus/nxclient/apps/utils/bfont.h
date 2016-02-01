/******************************************************************************
 *    (c)2010-2013 Broadcom Corporation
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
 *****************************************************************************/
#ifndef BFONT_H__
#define BFONT_H__

#include "nexus_types.h"
#include "nexus_surface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
NOTE: This API is only example code. It is subject to change and 
is not supported as a standard reference software deliverable.
**/

typedef struct bfont *bfont_t;
typedef size_t (*bfont_readfn_t)(void *buffer, size_t size, size_t number, void *handle); /* fread compatible */
struct bfont_surface_desc
{
    void *buffer;
    unsigned pitch;
    unsigned width;
    unsigned height;
    NEXUS_PixelFormat pixelFormat;
};

/* load bwin prerendered font */
bfont_t bfont_open(const char *filename);

/* allows loading fonts from memory */
bfont_t bfont_callback_open(bfont_readfn_t readfn, void *context);

void bfont_close(bfont_t handle);

/* populate bfont_surface_desc from NEXUS_SurfaceHandle. This allows bfont to work with memory not
referenced by NEXUS_SurfaceHandle. It also allows metadata to be fetched only once for better performance. */
void bfont_get_surface_desc(NEXUS_SurfaceHandle surface, struct bfont_surface_desc *pSurfaceDesc);

void bfont_get_height(bfont_t font, unsigned *height);

int bfont_measure_text(bfont_t font,
    const char *text, /* UTF8 */
    int len,
    int *width,
    int *height,
    int *base
    );

typedef uint32_t bfont_char; /* unicode character */
int bfont_get_glyph_metrics(bfont_t font, bfont_char ch, int *width, int *height, int *advance);
    
/* basic draw function. use bfont_draw_text_ex for more features */
int bfont_draw_text(struct bfont_surface_desc *desc, bfont_t font, 
    int x, /* assumes halign left */
    int y, /* assumes valign top */
    const char *text, /* UTF8 */
    int len, /* -1 will search for null terminator */
    uint32_t color, /* ARGB8888 */
    const NEXUS_Rect *cliprect
    );

typedef enum bfont_valign
{
    bfont_valign_top,
    bfont_valign_center,
    bfont_valign_bottom,
    bfont_valign_max
} bfont_valign;

typedef enum bfont_halign
{
    bfont_halign_left,
    bfont_halign_center,
    bfont_halign_right,
    bfont_halign_max
} bfont_halign;

typedef enum bfont_text_type
{
    bfont_text_type_utf8, /* default */
    bfont_text_type_utf32, /* "const char *" points of array of uint32_t in host endianness. 'len' is number of characters, not bytes. */
    bfont_text_type_ascii,
    bfont_text_type_max
} bfont_text_type;

/* extensible draw function */
typedef struct bfont_draw_text_settings
{
    bfont_valign valign;
    bfont_halign halign;
    enum bfont_text_type text_type;
} bfont_draw_text_settings;
void bfont_get_default_draw_text_settings(bfont_draw_text_settings *psettings);
int bfont_draw_text_ex(struct bfont_surface_desc *desc, bfont_t font,
    const NEXUS_Rect *pRect, /* if width is 0, force bfont_halign_left; if height is 0, force bfont_valign_top */
    const char *text, /* see bfont_text_type for variations of format */
    int len, /* number of characters (not bytes). -1 for utf8/ascii will search for null terminator. required for utf32. */
    uint32_t color, /* ARGB8888 */
    const bfont_draw_text_settings *psettings /* optional */
    );

/* deprecated. use bfont_draw_text_ex. */
int bfont_draw_aligned_text(struct bfont_surface_desc *desc, bfont_t font, const NEXUS_Rect *pRect, const char *text, int len, uint32_t color,
    bfont_valign valign, bfont_halign halign);

#ifdef __cplusplus
}
#endif

#endif /* BFONT_H__ */
