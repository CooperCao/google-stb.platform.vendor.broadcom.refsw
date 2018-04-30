/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#define USE_BGUI_SUBMIT 0

static void platform_graphics_p_recycle_complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static NEXUS_SurfaceHandle platform_graphics_get_surface(PlatformGraphicsHandle gfx)
{
    BDBG_ASSERT(gfx);
#if USE_BGUI_SUBMIT
    return bgui_surface(gfx->gui);
#else
    return gfx->surfaceClient.surfaces[gfx->surfaceClient.submitIndex].handle;
#endif
}

PlatformGraphicsHandle platform_graphics_open(PlatformHandle platform, const char * fontPath, unsigned fbWidth, unsigned fbHeight)
{
    PlatformGraphicsHandle gfx;
    unsigned i;

    gfx = BKNI_Malloc(sizeof(*gfx));
    BDBG_ASSERT(gfx);
    BKNI_Memset(gfx, 0, sizeof(*gfx));
    bgui_get_default_settings(&gfx->guiSettings);
    gfx->guiSettings.width = fbWidth;
    gfx->guiSettings.height = fbHeight;
    gfx->gui = bgui_create(&gfx->guiSettings);
    BDBG_ASSERT(gfx->gui);
    gfx->surfaceClient.surfaces[0].handle = bgui_surface(gfx->gui);
    NEXUS_Surface_GetCreateSettings(gfx->surfaceClient.surfaces[0].handle, &gfx->surfaceClient.surfaceCreateSettings);
    gfx->fbRect.width = gfx->surfaceClient.surfaceCreateSettings.width;
    gfx->fbRect.height = gfx->surfaceClient.surfaceCreateSettings.height;
#if !USE_BGUI_SUBMIT
    BKNI_CreateEvent(&gfx->surfaceClient.recycledEvent);
#endif

#if !USE_BGUI_SUBMIT
    for (i = 1; i < MAX_SURFACES; i++)
    {
        gfx->surfaceClient.surfaces[i].handle = NEXUS_Surface_Create(&gfx->surfaceClient.surfaceCreateSettings);
    }
#endif

    gfx->font = bfont_open(fontPath);
    if (!gfx->font)
    {
        BDBG_WRN(("unable to load font %s", fontPath));
    }
    else
    {
        bfont_get_height(gfx->font, &gfx->textHeight);
    }

    gfx->surfaceClient.handle = bgui_surface_client(gfx->gui);
    gfx->surfaceClient.id = bgui_surface_client_id(gfx->gui);
    for (i=0; i<platform->media.maxStreams; i++) {
        gfx->surfaceClient.video[i] = NEXUS_SurfaceClient_AcquireVideoWindow(gfx->surfaceClient.handle, i);
        BDBG_ASSERT(gfx->surfaceClient.video[i]);
        {
            NEXUS_SurfaceClientSettings scSettings;
            NEXUS_SurfaceClient_GetSettings(gfx->surfaceClient.video[i], &scSettings);
            scSettings.composition.contentMode = NEXUS_VideoWindowContentMode_eFull;
#if !USE_BGUI_SUBMIT
            scSettings.recycled.callback = platform_graphics_p_recycle_complete;
            scSettings.recycled.context = gfx->surfaceClient.recycledEvent;
#endif
            NEXUS_SurfaceClient_SetSettings(gfx->surfaceClient.video[i], &scSettings);
        }
    }

    platform->gfx = gfx;
    gfx->platform = platform;

    /* gui is on top of video, so need to punch a hole by filling with 0 */
    bgui_fill(gfx->gui, 0);
    bgui_checkpoint(gfx->gui);
#if !USE_BGUI_SUBMIT
    platform_graphics_submit(gfx);
#else
    bgui_submit(gfx->gui);
#endif

    return gfx;
}

void platform_graphics_close(PlatformGraphicsHandle gfx)
{
    unsigned i;

    if (!gfx) return;

    NEXUS_SurfaceClient_Clear(gfx->surfaceClient.handle);

#if !USE_BGUI_SUBMIT
    for (i = 1; i < MAX_SURFACES; i++)
    {
        NEXUS_Surface_Destroy(gfx->surfaceClient.surfaces[i].handle);
        gfx->surfaceClient.surfaces[i].handle = NULL;
    }

    BKNI_DestroyEvent(gfx->surfaceClient.recycledEvent);
    gfx->surfaceClient.recycledEvent = NULL;
#endif

    for(i = 0; i < MAX_STREAMS; i++)
    {
        if (gfx->surfaceClient.video[i]) { NEXUS_SurfaceClient_Release(gfx->surfaceClient.video[i]); }
    }
    if (gfx->font) bfont_close(gfx->font);
    if (gfx->gui) {
        bgui_checkpoint(gfx->gui);
        bgui_destroy(gfx->gui);
    }
    BKNI_Free(gfx);
}

static void platform_graphics_p_blit(PlatformGraphicsHandle gfx, NEXUS_SurfaceHandle surface, const PlatformRect * pRect)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_Graphics2DBlitSettings blitSettings;

    BDBG_ASSERT(gfx);
    BDBG_ASSERT(gfx->gui);
    BDBG_ASSERT(surface);
    BDBG_ASSERT(pRect);

    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    blitSettings.source.surface = surface;
#if USE_BGUI_SUBMIT
    blitSettings.output.surface = bgui_surface(gfx->gui);
#else
    blitSettings.output.surface = platform_graphics_get_surface(gfx);
#endif
    blitSettings.output.rect.x = pRect->x;
    blitSettings.output.rect.y = pRect->y;
    blitSettings.output.rect.width = pRect->width;
    blitSettings.output.rect.height = pRect->height;
    rc = NEXUS_Graphics2D_Blit(bgui_blitter(gfx->gui), &blitSettings);
    BDBG_ASSERT(!rc);
    bgui_checkpoint(gfx->gui);
}

void platform_graphics_render_picture(PlatformGraphicsHandle gfx, PlatformPictureHandle pic, const PlatformRect * pRect)
{
    platform_graphics_p_blit(gfx, pic->nxSurface, pRect);
}

void platform_graphics_fill(PlatformGraphicsHandle gfx, const PlatformRect * pRect, unsigned color)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_Graphics2DFillSettings fillSettings;

    BDBG_ASSERT(gfx);
    BDBG_ASSERT(gfx->gui);
    BDBG_ASSERT(pRect);

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
#if USE_BGUI_SUBMIT
    fillSettings.surface = bgui_surface(gfx->gui);
#else
    fillSettings.surface = platform_graphics_get_surface(gfx);
#endif
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
#if USE_BGUI_SUBMIT
    fillSettings.surface = bgui_surface(gfx->gui);
#else
    fillSettings.surface = platform_graphics_get_surface(gfx);
#endif
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
#if USE_BGUI_SUBMIT
    fillSettings.surface = bgui_surface(gfx->gui);
#else
    fillSettings.surface = platform_graphics_get_surface(gfx);
#endif
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

#if USE_BGUI_SUBMIT
    surface = bgui_surface(gfx->gui);
#else
    surface = platform_graphics_get_surface(gfx);
#endif
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

void platform_graphics_render_video(PlatformGraphicsHandle gfx, const PlatformRect * pRect, unsigned id)
{
    BDBG_ASSERT(gfx);
    BDBG_ASSERT(gfx->gui);
    BDBG_ASSERT(pRect);
    platform_graphics_clear(gfx, pRect);

    if (gfx->platform->media.streams[id].player && id == platform_get_pip_stream_id(gfx->platform))
    {
        platform_media_player_p_capture_video(gfx->platform->media.streams[id].player);
        if (gfx->platform->media.streams[id].validCaptures)
        {
            platform_graphics_p_blit(gfx, gfx->platform->media.streams[id].captures[gfx->platform->media.streams[id].validCaptures-1], pRect);
            gfx->platform->media.streams[id].performance.frames++;
            gfx->platform->media.streams[id].lastValidCaptures = gfx->platform->media.streams[id].validCaptures;
        }
        else
        {
            if (gfx->platform->media.streams[id].lastValidCaptures)
            {
                platform_graphics_p_blit(gfx, gfx->platform->media.streams[id].captures[gfx->platform->media.streams[id].lastValidCaptures-1], pRect);
            }
        }
        platform_media_player_p_recycle_video(gfx->platform->media.streams[id].player);
    }
}

void platform_graphics_scale_video(PlatformGraphicsHandle gfx, const PlatformRect * pRect, unsigned id)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_SurfaceClientSettings settings;

    BDBG_ASSERT(gfx);
    BDBG_ASSERT(gfx->gui);
    BDBG_ASSERT(pRect);

    NEXUS_SurfaceClient_GetSettings(gfx->surfaceClient.video[id], &settings);
    settings.composition.virtualDisplay.width = gfx->guiSettings.width;
    settings.composition.virtualDisplay.height = gfx->guiSettings.height;
    settings.composition.position.x = pRect->x;
    settings.composition.position.y = pRect->y;
    settings.composition.position.width = pRect->width;
    settings.composition.position.height = pRect->height;
    rc = NEXUS_SurfaceClient_SetSettings(gfx->surfaceClient.video[id], &settings);
    if (rc) rc = BERR_TRACE(rc);
}

void platform_graphics_move_video(PlatformGraphicsHandle gfx, const PlatformRect * pRect, unsigned id)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_SurfaceClientSettings settings;

    BDBG_ASSERT(gfx);
    BDBG_ASSERT(gfx->gui);
    BDBG_ASSERT(pRect);

    NEXUS_SurfaceClient_GetSettings(gfx->surfaceClient.video[id], &settings);
    settings.composition.virtualDisplay.width = gfx->guiSettings.width;
    settings.composition.virtualDisplay.height = gfx->guiSettings.height;
    settings.composition.position.x = pRect->x;
    settings.composition.position.y = pRect->y;
    settings.composition.position.width = pRect->width;
    settings.composition.position.height = pRect->height;
    rc = NEXUS_SurfaceClient_SetSettings(gfx->surfaceClient.video[id], &settings);
    if (rc) rc = BERR_TRACE(rc);
}

#if !USE_BGUI_SUBMIT
static void platform_graphics_p_recycle_next(PlatformGraphicsHandle gfx)
{
    size_t num;
    do {
#define MAX_RECYCLE 2
        NEXUS_SurfaceHandle surface[MAX_RECYCLE];
        NEXUS_Error rc;
        unsigned i;

        rc = NEXUS_SurfaceClient_RecycleSurface(gfx->surfaceClient.handle, surface, MAX_RECYCLE, &num);
        BDBG_ASSERT(!rc);
        for (i = 0; i < num; i++)
        {
            unsigned j;
            /* if submitted infront, they may return out of order */
            for (j = 0; j < MAX_SURFACES; j++)
            {
                if (gfx->surfaceClient.surfaces[j].handle == surface[i]) {
                    BDBG_ASSERT(gfx->surfaceClient.surfaces[j].submitted);
                    gfx->surfaceClient.surfaces[j].submitted = false;
                    gfx->surfaceClient.queued--;
                }
            }
        }
    } while (num >= MAX_RECYCLE);
}
#endif

bool platform_graphics_recycle(PlatformGraphicsHandle gfx)
{
    BERR_Code rc = BERR_SUCCESS;

#if !USE_BGUI_SUBMIT
    if (gfx->surfaceClient.surfaces[gfx->surfaceClient.submitIndex].submitted) {
        platform_graphics_p_recycle_next(gfx);
        if (gfx->surfaceClient.surfaces[gfx->surfaceClient.submitIndex].submitted) {
            rc = BKNI_WaitForEvent(gfx->surfaceClient.recycledEvent, 2000);
            if (rc) BERR_TRACE(rc);
            return false;
        }
    }
#endif

    return true;
}

void platform_graphics_submit(PlatformGraphicsHandle gfx)
{
#if !USE_BGUI_SUBMIT
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned pip;
    if (!gfx) return;

    rc = NEXUS_SurfaceClient_PushSurface(gfx->surfaceClient.handle, gfx->surfaceClient.surfaces[gfx->surfaceClient.submitIndex].handle, NULL, true);
    BDBG_ASSERT(!rc);
    gfx->surfaceClient.surfaces[gfx->surfaceClient.submitIndex].submitted = true;
    if (++gfx->surfaceClient.submitIndex == MAX_SURFACES) {
        gfx->surfaceClient.submitIndex = 0;
    }
    gfx->surfaceClient.pushed++;
    gfx->surfaceClient.queued++;
    pip = platform_get_pip_stream_id(gfx->platform);
    gfx->platform->media.streams[pip].performance.frames++;
    gfx->platform->media.streams[pip].performance.now = platform_p_get_time();
#if 1
    if (gfx->platform->media.streams[pip].performance.now-gfx->platform->media.streams[pip].performance.startTime > 1000)
#else
    if (gfx->surfaceClient.pushed % 1000 == 0)
#endif
    {
        BDBG_MSG(("%u pushed, %u queued, %u fps", gfx->surfaceClient.pushed, gfx->surfaceClient.queued, 1000*gfx->platform->media.streams[pip].performance.frames/(gfx->platform->media.streams[pip].performance.now-gfx->platform->media.streams[pip].performance.startTime)));
        gfx->platform->media.streams[pip].performance.frames = 0;
        gfx->platform->media.streams[pip].performance.startTime = gfx->platform->media.streams[pip].performance.now;
    }
    platform_graphics_p_recycle_next(gfx);
#else
    bgui_submit(gfx->gui);
#endif
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
