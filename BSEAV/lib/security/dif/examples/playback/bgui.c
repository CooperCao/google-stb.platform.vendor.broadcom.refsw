/******************************************************************************
 *    (c)2015 Broadcom Corporation
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
 *****************************************************************************/
#include "bgui.h"
#include "bkni_multi.h"
#include "nxclient.h"

BDBG_MODULE(bgui);

struct bgui
{
    NxClient_AllocResults allocResults;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_SurfaceHandle surface;
    NEXUS_SurfaceClientHandle surfaceClient;
    BKNI_EventHandle displayedEvent, checkpointEvent, windowMovedEvent;
};

static void complete2(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

bgui_t bgui_create(unsigned width, unsigned height)
{
    NxClient_AllocSettings allocSettings;
    bgui_t gui;
    int rc;
    NEXUS_SurfaceCreateSettings surfaceCreateSettings;
    NEXUS_Graphics2DSettings gfxSettings;
    NEXUS_SurfaceClientSettings surfaceClientSettings;

    gui = BKNI_Malloc(sizeof(*gui));
    if (!gui) return NULL;
    BKNI_Memset(gui, 0, sizeof(*gui));

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.surfaceClient = 1; /* surface client required for video windows */
    rc = NxClient_Alloc(&allocSettings, &gui->allocResults);
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    rc = BKNI_CreateEvent(&gui->checkpointEvent);
    if (rc) {rc = BERR_TRACE(rc); goto error;}
    rc = BKNI_CreateEvent(&gui->displayedEvent);
    if (rc) {rc = BERR_TRACE(rc); goto error;}
    rc = BKNI_CreateEvent(&gui->windowMovedEvent);
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    gui->surfaceClient = NEXUS_SurfaceClient_Acquire(gui->allocResults.surfaceClient[0].id);
    if (!gui->surfaceClient) {BERR_TRACE(-1); goto error;}

    NEXUS_SurfaceClient_GetSettings(gui->surfaceClient, &surfaceClientSettings);
    surfaceClientSettings.displayed.callback = complete2;
    surfaceClientSettings.displayed.context = gui->displayedEvent;
    surfaceClientSettings.windowMoved.callback = complete2;
    surfaceClientSettings.windowMoved.context = gui->windowMovedEvent;
    rc = NEXUS_SurfaceClient_SetSettings(gui->surfaceClient, &surfaceClientSettings);
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    NEXUS_Surface_GetDefaultCreateSettings(&surfaceCreateSettings);
    surfaceCreateSettings.width = width;
    surfaceCreateSettings.height = height;
    gui->surface = NEXUS_Surface_Create(&surfaceCreateSettings);
    if (!gui->surface) {BERR_TRACE(-1); goto error;}

    gui->gfx = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, NULL);
    NEXUS_Graphics2D_GetSettings(gui->gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete2;
    gfxSettings.checkpointCallback.context = gui->checkpointEvent;
    rc = NEXUS_Graphics2D_SetSettings(gui->gfx, &gfxSettings);
    if (rc) {BERR_TRACE(rc); goto error;}

    return gui;

error:
    bgui_destroy(gui);
    return NULL;
}

void bgui_destroy(bgui_t gui)
{
    if (gui->gfx) {
        NEXUS_Graphics2D_Close(gui->gfx);
    }
    if (gui->checkpointEvent) {
        BKNI_DestroyEvent(gui->checkpointEvent);
    }
    if (gui->displayedEvent) {
        BKNI_DestroyEvent(gui->displayedEvent);
    }
    if (gui->windowMovedEvent) {
        BKNI_DestroyEvent(gui->windowMovedEvent);
    }
    if (gui->surfaceClient) {
        NEXUS_SurfaceClient_Release(gui->surfaceClient);
    }
    if (gui->allocResults.surfaceClient[0].id) {
        NxClient_Free(&gui->allocResults);
    }
    BKNI_Free(gui);
}

NEXUS_SurfaceHandle bgui_surface(bgui_t gui)
{
    return gui->surface;
}

NEXUS_SurfaceClientHandle bgui_surface_client(bgui_t gui)
{
    return gui->surfaceClient;
}

unsigned bgui_surface_client_id(bgui_t gui)
{
    return gui->allocResults.surfaceClient[0].id;
}

NEXUS_Graphics2DHandle bgui_blitter(bgui_t gui)
{
    return gui->gfx;
}

int bgui_fill(bgui_t gui, unsigned color)
{
    int rc;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = gui->surface;
    fillSettings.color = color;
    rc = NEXUS_Graphics2D_Fill(gui->gfx, &fillSettings);
    if (rc) return BERR_TRACE(rc);
    return 0;
}

int bgui_checkpoint(bgui_t gui)
{
    int rc;
    rc = NEXUS_Graphics2D_Checkpoint(gui->gfx, NULL); /* require to execute queue */
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(gui->checkpointEvent, BKNI_INFINITE);
    }
    if (rc) return BERR_TRACE(rc);
    return 0;
}

int bgui_submit(bgui_t gui)
{
    int rc;
    rc = NEXUS_SurfaceClient_SetSurface(gui->surfaceClient, gui->surface);
    if (rc) return BERR_TRACE(rc);
    rc = BKNI_WaitForEvent(gui->displayedEvent, 5000);
    if (rc) return BERR_TRACE(rc);
    return 0;
}

int bgui_wait_for_move(bgui_t gui)
{
    int rc;
    rc = BKNI_WaitForEvent(gui->windowMovedEvent, 5000);
    if (rc) return BERR_TRACE(rc);
    return 0;
}

void b_pig_init(NEXUS_SurfaceClientHandle video_sc)
{
    NEXUS_SurfaceClientSettings settings;
    NEXUS_SurfaceClient_GetSettings(video_sc, &settings);
    settings.composition.position.x = 100;
    settings.composition.position.y = 100;
    settings.composition.position.width = 640;
    settings.composition.position.height = 360;
    settings.composition.virtualDisplay.width = 1280;
    settings.composition.virtualDisplay.height = 720;
    NEXUS_SurfaceClient_SetSettings(video_sc, &settings);
}

void b_pig_move(NEXUS_SurfaceClientHandle video_sc, struct b_pig_inc *pig_inc)
{
    NEXUS_SurfaceClientSettings settings;
    NEXUS_SurfaceClient_GetSettings(video_sc, &settings);
    settings.composition.virtualDisplay.width = 1280;
    settings.composition.virtualDisplay.height = 720;
    settings.composition.position.width += pig_inc->width;
    settings.composition.position.height = settings.composition.position.width * 9 / 16; /* keep 16:9 AR */
    if (settings.composition.position.x + settings.composition.position.width + pig_inc->x > settings.composition.virtualDisplay.width ||
        settings.composition.position.x + pig_inc->x < 0) {
        pig_inc->x *= -1;
    }
    if (settings.composition.position.y + settings.composition.position.height + pig_inc->y > settings.composition.virtualDisplay.height ||
        settings.composition.position.y + pig_inc->y < 0) {
        pig_inc->y *= -1;
    }
    settings.composition.position.x += pig_inc->x;
    settings.composition.position.y += pig_inc->y;
    if (settings.composition.position.x < 0 ||
        settings.composition.position.y < 0 ||
        settings.composition.position.x + settings.composition.position.width > settings.composition.virtualDisplay.width ||
        settings.composition.position.y + settings.composition.position.height > settings.composition.virtualDisplay.height)
    {
        settings.composition.position.x = 0;
        settings.composition.position.y = 0;
        /* go less than full screen to avoid clipping on the bounce */
        settings.composition.position.width = settings.composition.virtualDisplay.width - 4;
        settings.composition.position.height = settings.composition.virtualDisplay.height - 4;
        pig_inc->width *= -1;
    }
    else if (!settings.composition.position.width || !settings.composition.position.height) {
        pig_inc->width *= -1;
    }

    NEXUS_SurfaceClient_SetSettings(video_sc, &settings);
    BDBG_MSG(("video %d,%d,%d,%d with %d,%d,%d,%d",
        settings.composition.position.x, settings.composition.position.y, settings.composition.position.width, settings.composition.position.height,
        pig_inc->x, pig_inc->y, pig_inc->width, pig_inc->width));
}
