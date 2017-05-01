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
 *
 * Module Description:
 *
 *****************************************************************************/
#include "nexus_platform_client.h"
#include "nxclient.h"
#include "platform.h"
#include "platform_priv.h"
#include "platform_graphics_priv.h"
#include "platform_picture_priv.h"
#include "nexus_types.h"
#include "bkni.h"
#include "bdbg.h"

BDBG_MODULE(platform_graphics);

PlatformGraphicsHandle platform_graphics_open(PlatformHandle platform, const char * fontPath, unsigned fbWidth, unsigned fbHeight)
{
    PlatformGraphicsHandle gfx;
    NEXUS_SurfaceHandle fb;
    NEXUS_SurfaceCreateSettings fbCreateSettings;
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned i, j, windowCnt=0;

    gfx = BKNI_Malloc(sizeof(*gfx));
    BDBG_ASSERT(gfx);
    BKNI_Memset(gfx, 0, sizeof(*gfx));
    bgui_get_default_settings(&gfx->guiSettings);
    gfx->guiSettings.width = fbWidth;
    gfx->guiSettings.height = fbHeight;
    gfx->gui = bgui_create(&gfx->guiSettings);
    BDBG_ASSERT(gfx->gui);
    /* gui is on top of video, so need to punch a hole by filling with 0 */
    bgui_fill(gfx->gui, 0);
    bgui_checkpoint(gfx->gui);
    bgui_submit(gfx->gui);
    fb = bgui_surface(gfx->gui);
    NEXUS_Surface_GetCreateSettings(fb, &fbCreateSettings);
    gfx->fbRect.width = fbCreateSettings.width;
    gfx->fbRect.height = fbCreateSettings.height;

    gfx->font = bfont_open(fontPath);
    if (!gfx->font)
    {
        BDBG_WRN(("unable to load font %s", fontPath));
    }
    else
    {
        bfont_get_height(gfx->font, &gfx->textHeight);
    }

    for (i=0; i<NEXUS_MAX_VIDEO_DECODERS; i++) {
        if (!platform->videoCaps.memory[i].mosaic.maxNumber) break;
        gfx->sc[i].numWindows = platform->videoCaps.memory[i].mosaic.maxNumber;
        gfx->maxMosaics += gfx->sc[i].numWindows;
        if (gfx->maxMosaics >= MAX_MOSAICS) break;
    }
    if (gfx->maxMosaics) {
        if (gfx->maxMosaics < MAX_MOSAICS) {
            BDBG_ASSERT((MAX_MOSAICS - gfx->maxMosaics) == 1); /* Can we have more than 1 pip for mosaic?? */
            gfx->sc[i].numWindows = MAX_MOSAICS - gfx->maxMosaics ;
        }
    } else {
        BDBG_ASSERT(gfx->sc[0].numWindows == 0);
        gfx->sc[0].numWindows = 1;
        BDBG_WRN(("No Mosaic Support!!"));
    }


    for (i=0; i<(sizeof(gfx->sc)/sizeof(gfx->sc[0])); i++) {
        if (!gfx->sc[i].numWindows) continue;
        if (i == 0) {
            gfx->sc[i].surfaceClient = bgui_surface_client(gfx->gui);
            gfx->sc[i].id = bgui_surface_client_id(gfx->gui);
        } else {
            NxClient_AllocSettings allocSettings;
            NxClient_AllocResults allocResults;
            NxClient_GetDefaultAllocSettings(&allocSettings);
            allocSettings.surfaceClient = 1; /* surface client required for video window */
            rc = NxClient_Alloc(&allocSettings, &allocResults);
            if (rc) BERR_TRACE(rc);

            gfx->sc[i].surfaceClient = NEXUS_SurfaceClient_Acquire(allocResults.surfaceClient[0].id);
            gfx->sc[i].id = allocResults.surfaceClient[0].id;
        }

        for (j=0; j<gfx->sc[i].numWindows; j++) {
            gfx->video[windowCnt] = NEXUS_SurfaceClient_AcquireVideoWindow(gfx->sc[i].surfaceClient, j);
            if(j == 0) gfx->nonMosaicWindowId = windowCnt; /* First window on the last surface client */
            windowCnt++;
        }
    }

    if (gfx->maxMosaics) BDBG_ASSERT(windowCnt == MAX_MOSAICS);

    platform->gfx = gfx;
    gfx->platform = platform;

    return gfx;
}

void platform_graphics_close(PlatformGraphicsHandle gfx)
{
    unsigned i;

    if (!gfx) return;
    for(i=0; i<MAX_MOSAICS; i++) {
        if (gfx->video[i]) { NEXUS_SurfaceClient_Release(gfx->video[i]); }
    }
    if (gfx->font) bfont_close(gfx->font);
    if (gfx->gui) bgui_destroy(gfx->gui);
    BKNI_Free(gfx);
}

void platform_graphics_render_picture(PlatformGraphicsHandle gfx, PlatformPictureHandle pic, const PlatformRect * pRect)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_Graphics2DBlitSettings blitSettings;

    BDBG_ASSERT(gfx);
    BDBG_ASSERT(gfx->gui);
    BDBG_ASSERT(pic);
    BDBG_ASSERT(pRect);

    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    blitSettings.source.surface = pic->nxSurface;
    blitSettings.output.surface = bgui_surface(gfx->gui);
    blitSettings.output.rect.x = pRect->x;
    blitSettings.output.rect.y = pRect->y;
    blitSettings.output.rect.width = pRect->width;
    blitSettings.output.rect.height = pRect->height;
    rc = NEXUS_Graphics2D_Blit(bgui_blitter(gfx->gui), &blitSettings);
    BDBG_ASSERT(!rc);
    bgui_checkpoint(gfx->gui);
}

void platform_graphics_fill(PlatformGraphicsHandle gfx, const PlatformRect * pRect, unsigned color)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_Graphics2DFillSettings fillSettings;

    BDBG_ASSERT(gfx);
    BDBG_ASSERT(gfx->gui);
    BDBG_ASSERT(pRect);

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = bgui_surface(gfx->gui);
    fillSettings.rect.x = pRect->x;
    fillSettings.rect.y = pRect->y;
    fillSettings.rect.width = pRect->width;
    fillSettings.rect.height = pRect->height;
    fillSettings.color = color;
    fillSettings.colorOp = NEXUS_FillOp_eCopy;
    fillSettings.alphaOp = NEXUS_FillOp_eCopy;
    rc = NEXUS_Graphics2D_Fill(bgui_blitter(gfx->gui), &fillSettings);
    BDBG_ASSERT(!rc);
    bgui_checkpoint(gfx->gui);
}

void platform_graphics_blend(PlatformGraphicsHandle gfx, const PlatformRect * pRect, unsigned color)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_Graphics2DFillSettings fillSettings;

    BDBG_ASSERT(gfx);
    BDBG_ASSERT(gfx->gui);
    BDBG_ASSERT(pRect);

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = bgui_surface(gfx->gui);
    fillSettings.rect.x = pRect->x;
    fillSettings.rect.y = pRect->y;
    fillSettings.rect.width = pRect->width;
    fillSettings.rect.height = pRect->height;
    fillSettings.color = color;
    fillSettings.colorOp = NEXUS_FillOp_eBlend;
    fillSettings.alphaOp = NEXUS_FillOp_eIgnore;
    rc = NEXUS_Graphics2D_Fill(bgui_blitter(gfx->gui), &fillSettings);
    BDBG_ASSERT(!rc);
    bgui_checkpoint(gfx->gui);
}

void platform_graphics_clear(PlatformGraphicsHandle gfx, const PlatformRect * pRect)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_Graphics2DFillSettings fillSettings;

    BDBG_ASSERT(gfx);
    BDBG_ASSERT(gfx->gui);
    BDBG_ASSERT(pRect);

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = bgui_surface(gfx->gui);
    fillSettings.rect.x = pRect->x;
    fillSettings.rect.y = pRect->y;
    fillSettings.rect.width = pRect->width;
    fillSettings.rect.height = pRect->height;
    fillSettings.color = 0;
    fillSettings.colorOp = NEXUS_FillOp_eCopy;
    fillSettings.alphaOp = NEXUS_FillOp_eCopy;
    rc = NEXUS_Graphics2D_Fill(bgui_blitter(gfx->gui), &fillSettings);
    BDBG_ASSERT(!rc);
    bgui_checkpoint(gfx->gui);
}

void platform_graphics_get_default_text_rendering_settings(PlatformTextRenderingSettings * pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

void platform_graphics_render_text(PlatformGraphicsHandle gfx, const char * text, const PlatformTextRenderingSettings * pSettings)
{
    struct bfont_surface_desc desc;
    struct bfont_draw_text_settings fontsettings;
    NEXUS_Rect rect;
    NEXUS_SurfaceHandle surface;
    int rc;

    BDBG_ASSERT(gfx->gui);
    BDBG_ASSERT(gfx->font);

    BDBG_CASSERT(bfont_halign_left == (bfont_halign)PlatformHorizontalAlignment_eLeft);
    BDBG_CASSERT(bfont_halign_center == (bfont_halign)PlatformHorizontalAlignment_eCenter);
    BDBG_CASSERT(bfont_halign_right == (bfont_halign)PlatformHorizontalAlignment_eRight);

    surface = bgui_surface(gfx->gui);
    bfont_get_surface_desc(surface, &desc);
    bfont_get_default_draw_text_settings(&fontsettings);
    fontsettings.halign = (bfont_halign)pSettings->halign;
    fontsettings.valign = (bfont_valign)pSettings->valign;
    rect.x = pSettings->rect.x;
    rect.y = pSettings->rect.y;
    rect.width = pSettings->rect.width;
    rect.height = pSettings->rect.height;
    rc = bfont_draw_text_ex(&desc, gfx->font, &rect, text, -1, pSettings->color, &fontsettings);
    if (rc) rc = BERR_TRACE(rc);
    NEXUS_Surface_Flush(surface);
}

void platform_graphics_render_video(PlatformGraphicsHandle gfx, const PlatformRect * pRect)
{
    BDBG_ASSERT(gfx);
    BDBG_ASSERT(gfx->gui);
    BDBG_ASSERT(pRect);
    platform_graphics_clear(gfx, pRect);
}

void platform_graphics_scale_video(PlatformGraphicsHandle gfx, const PlatformRect * pRect, unsigned id)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_SurfaceClientSettings settings;

    BDBG_ASSERT(gfx);
    BDBG_ASSERT(gfx->gui);
    BDBG_ASSERT(pRect);

    NEXUS_SurfaceClient_GetSettings(gfx->video[id], &settings);
    settings.composition.virtualDisplay.width = gfx->guiSettings.width;
    settings.composition.virtualDisplay.height = gfx->guiSettings.height;
    settings.composition.position.x = pRect->x;
    settings.composition.position.y = pRect->y;
    settings.composition.position.width = pRect->width;
    settings.composition.position.height = pRect->height;
    rc = NEXUS_SurfaceClient_SetSettings(gfx->video[id], &settings);
    if (rc) rc = BERR_TRACE(rc);
}

void platform_graphics_move_video(PlatformGraphicsHandle gfx, const PlatformRect * pRect, unsigned id)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_SurfaceClientSettings settings;

    BDBG_ASSERT(gfx);
    BDBG_ASSERT(gfx->gui);
    BDBG_ASSERT(pRect);

    NEXUS_SurfaceClient_GetSettings(gfx->video[id], &settings);
    settings.composition.virtualDisplay.width = gfx->guiSettings.width;
    settings.composition.virtualDisplay.height = gfx->guiSettings.height;
    settings.composition.position.x = pRect->x;
    settings.composition.position.y = pRect->y;
    settings.composition.position.width = pRect->width;
    settings.composition.position.height = pRect->height;
    rc = NEXUS_SurfaceClient_SetSettings(gfx->video[id], &settings);
    if (rc) rc = BERR_TRACE(rc);
}

void platform_graphics_submit(PlatformGraphicsHandle gfx)
{
    if (!gfx) return;
    bgui_submit(gfx->gui);
}

unsigned platform_graphics_get_text_height(PlatformGraphicsHandle gfx)
{
    BDBG_ASSERT(gfx);
    return gfx->textHeight;
}

unsigned platform_graphics_get_text_width(PlatformGraphicsHandle gfx, const char * text)
{
    int width;
    int height;
    int base;

    bfont_measure_text(gfx->font, text, -1, &width, &height, &base);
    return width;
}

const PlatformRect * platform_graphics_get_fb_rect(PlatformGraphicsHandle gfx)
{
    BDBG_ASSERT(gfx);
    return &gfx->fbRect;
}

unsigned platform_graphics_get_non_mosaic_window_id(PlatformGraphicsHandle gfx)
{
    BDBG_ASSERT(gfx);
    return gfx->nonMosaicWindowId;

}
