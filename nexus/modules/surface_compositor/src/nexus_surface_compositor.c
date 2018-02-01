/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 **************************************************************************/
#include "nexus_surface_compositor_module.h"
#include "nexus_surface_compositor_impl.h"
#include "priv/nexus_surface_priv.h"
#include "priv/nexus_core.h"
#include "priv/nexus_surface_compositor_standby_priv.h"

BDBG_MODULE(nexus_surface_compositor);
BTRC_MODULE(surface_compositor_composite,ENABLE);

struct nexus_surfacecompositor_list g_nexus_surfacecompositor_list;

#define BDBG_MSG_TRACE(X) BDBG_MSG(X)
#define BDBG_MSG_DISABLE(X)


void NEXUS_SurfaceCompositorModule_P_Print(void)
{
#if BDBG_DEBUG_BUILD
    NEXUS_SurfaceCompositorHandle server;
    for (server = BLST_S_FIRST(&g_nexus_surfacecompositor_list); server; server = BLST_S_NEXT(server, link)) {
        NEXUS_SurfaceClientHandle client;
        unsigned i;
        BDBG_LOG(("SurfaceCompositor %p", (void *)server));
        for (i=0;i<NEXUS_MAX_DISPLAYS;i++) {
            struct NEXUS_SurfaceCompositorDisplay *display = server->display[i];
            if (display && display->display) {
                BDBG_LOG(("display %d: %p, canvas %dx%d, %d compositions, %d fills", display->index, (void *)display->display, display->formatInfo.canvas.width, display->formatInfo.canvas.height,
                    display->stats.compose, display->stats.fill));
                if (display->formatInfo.orientation != NEXUS_VideoOrientation_e2D) {
                    BDBG_LOG(("        3D %s%s", display->formatInfo.native3D?"native ":"", display->formatInfo.orientation==NEXUS_VideoOrientation_e3D_OverUnder?"O/U":"L/R"));
                }
            }
        }
        for (client = BLST_Q_FIRST(&server->clients); client; client = BLST_Q_NEXT(client, link)) {
            NEXUS_SurfaceClientHandle child;
            BDBG_LOG(("client %p: %d,%d,%d,%d; visible %c; zorder %d", (void *)client,
                client->serverSettings.composition.position.x,
                client->serverSettings.composition.position.y,
                client->serverSettings.composition.position.width,
                client->serverSettings.composition.position.height,
                client->serverSettings.composition.visible?'y':'n',
                client->serverSettings.composition.zorder));
            if (client->settings.orientation != NEXUS_VideoOrientation_e2D) {
                BDBG_LOG(("  3D %s", client->settings.orientation==NEXUS_VideoOrientation_e3D_OverUnder?"O/U":"L/R"));
            }
            if (client == server->bypass_compose.client) {
                BDBG_LOG(("  bypass client"));
            }
            if (client->serverSettings.composition.clipRect.width || client->serverSettings.composition.clipRect.height) {
                BDBG_LOG(("  clip %d,%d,%d,%d(%d,%d)",
                    client->serverSettings.composition.clipRect.x,
                    client->serverSettings.composition.clipRect.y,
                    client->serverSettings.composition.clipRect.width,
                    client->serverSettings.composition.clipRect.height,
                    client->serverSettings.composition.clipBase.width?client->serverSettings.composition.clipBase.width:client->serverSettings.composition.virtualDisplay.width,
                    client->serverSettings.composition.clipBase.height?client->serverSettings.composition.clipBase.height:client->serverSettings.composition.virtualDisplay.height));
            }
            for (child = BLST_S_FIRST(&client->children); child; child = BLST_S_NEXT(child, child_link)) {
                BDBG_LOG(("  %s %p: id %d; %d,%d,%d,%d; virtual display %d,%d; visible %c; zorder %d",
                    (child->type == NEXUS_SurfaceClient_eVideoWindow)?"video":"child",
                    (void *)child,
                    child->client_id,
                    child->settings.composition.position.x,
                    child->settings.composition.position.y,
                    child->settings.composition.position.width,
                    child->settings.composition.position.height,
                    child->settings.composition.virtualDisplay.width,
                    child->settings.composition.virtualDisplay.height,
                    child->settings.composition.visible?'y':'n',
                    child->settings.composition.zorder));
                if (child->settings.composition.clipRect.width || child->settings.composition.clipRect.height) {
                    BDBG_LOG(("        clip %d,%d,%d,%d(%d,%d)",
                        child->settings.composition.clipRect.x,
                        child->settings.composition.clipRect.y,
                        child->settings.composition.clipRect.width,
                        child->settings.composition.clipRect.height,
                        child->settings.composition.clipBase.width?child->settings.composition.clipBase.width:child->settings.composition.virtualDisplay.width,
                        child->settings.composition.clipBase.height?child->settings.composition.clipBase.height:child->settings.composition.virtualDisplay.height));
                }
            }
            switch (client->state.client_type) {
            case client_type_set:
                BDBG_LOG(("  set %p, total %d", (void *)client->set.surface.surface, client->set.cnt));
                break;
            case client_type_push:
                {
                    unsigned push_count = 0, recycle_count = 0;
                    NEXUS_SurfaceCompositor_P_PushElement *e;
                    for (e = BLST_SQ_FIRST(&client->queue.push); e; e = BLST_SQ_NEXT(e, link)) push_count++;
                    for (e = BLST_SQ_FIRST(&client->queue.recycle); e; e = BLST_SQ_NEXT(e, link)) recycle_count++;
                    BDBG_LOG(("  pushed %d, recycled %d, total %d", push_count, recycle_count, client->queue.cnt));
                }
                break;
            case client_type_tunnel:
                BDBG_LOG(("  tunneled"));
                break;
            default:
                break;
            }
        }
    }
#endif
}

static void nexus_surface_compositor_p_dealloc_displays(NEXUS_SurfaceCompositorHandle server);
static void nexus_p_surface_compositor_update_all_clients(NEXUS_SurfaceCompositorHandle server, unsigned flags);
static void nexus_surface_compositor_p_update_dirty_client(NEXUS_SurfaceClientHandle client);

static void nexus_surface_compositor_p_initialize_compositor_display(struct NEXUS_SurfaceCompositorDisplay *cmpDisplay)
{
    unsigned i;

    cmpDisplay->compositing = NULL;
    cmpDisplay->composited = NULL;
    cmpDisplay->submitted = NULL;
    cmpDisplay->displaying = NULL;
    cmpDisplay->lastComposited = NULL;
    cmpDisplay->master_framebuffer = NULL;
    cmpDisplay->generation = 1;
    cmpDisplay->slave.dirty = false;
    BLST_SQ_INIT(&cmpDisplay->available);
    BLST_Q_INIT(&cmpDisplay->tunnel.submitted);
    BLST_Q_INIT(&cmpDisplay->tunnel.available);
    for (i=0;i<NEXUS_SURFACECMP_MAX_FRAMEBUFFERS;i++) {
        struct NEXUS_SurfaceCompositorFramebuffer *framebuffer = &cmpDisplay->framebuffer[i];
        /* framebuffer->surface points to allocated memory. do not clear. */
        framebuffer->full_render = true;
        framebuffer->scene.elements.count = 0;
        framebuffer->scene.dirty.x = 0;
        framebuffer->scene.dirty.y = 0;
        framebuffer->scene.dirty.width = 0;
        framebuffer->scene.dirty.height = 0;
        framebuffer->ref_cnt = 0;
        framebuffer->generation = 0;
        framebuffer->state = NEXUS_SurfaceCompositorFramebufferState_eAvailable;
        framebuffer->tunnel.acquired = false;
        framebuffer->tunnel.pristine = false;
        NEXUS_SURFACECLIENT_P_SURFACE_INIT(&framebuffer->tunnel.surface);
    }
    return;
}

static void nexus_surface_compositor_p_framebuffer_callback(void *context)
{
    struct NEXUS_SurfaceCompositorDisplay *display = context;
    nexus_surface_compositor_p_framebuffer_applied(display);
    return;
}

static struct NEXUS_SurfaceCompositorDisplay *nexus_surface_compositor_p_create_display(NEXUS_SurfaceCompositorHandle server)
{
    struct NEXUS_SurfaceCompositorDisplay *display;

    display = BKNI_Malloc(sizeof(*display));
    if(display==NULL) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    BKNI_Memset(display, 0, sizeof(*display));
    nexus_surface_compositor_p_initialize_compositor_display(display);
    display->canvas.width = 720;
    display->canvas.height = 480;

    NEXUS_CallbackHandler_Init(display->frameBufferCallback, nexus_surface_compositor_p_framebuffer_callback, display);
    NEXUS_CallbackHandler_Init(display->vsyncCallback, nexus_surface_compositor_p_vsync, server);
    display->server = server;
    return display;
}

static void nexus_surface_compositor_p_destroy_display(struct NEXUS_SurfaceCompositorDisplay *display)
{
    unsigned i;

    for (i=0;i<NEXUS_SURFACECMP_MAX_FRAMEBUFFERS;i++) {
        struct NEXUS_SurfaceCompositorFramebuffer *framebuffer = &display->framebuffer[i];
        /* framebuffer->surface points to allocated memory. do not clear. */
        NEXUS_P_SurfaceCompositorRenderElements_Shutdown(&framebuffer->scene.elements);
    }
    BKNI_Free(display);
}

static NEXUS_Error nexus_p_open_blitter(NEXUS_SurfaceCompositorHandle server, bool secure)
{
    NEXUS_Graphics2DOpenSettings settings;
    NEXUS_Graphics2DSettings gfxSettings;
    int rc;
    BDBG_ASSERT(!server->gfx);
    NEXUS_Graphics2D_GetDefaultOpenSettings(&settings);
    settings.secure = secure;
    settings.compatibleWithSurfaceCompaction = true;
    server->gfx = NEXUS_Graphics2D_Open(0, &settings);
    if (!server->gfx) return BERR_TRACE(NEXUS_UNKNOWN);
    NEXUS_Graphics2D_GetSettings(server->gfx, &gfxSettings);
    NEXUS_CallbackHandler_PrepareCallback(server->checkpointCallback, gfxSettings.checkpointCallback);
    NEXUS_CallbackHandler_PrepareCallback(server->packetSpaceAvailableCallback, gfxSettings.packetSpaceAvailable);
    rc = NEXUS_Graphics2D_SetSettings(server->gfx, &gfxSettings);
    if (rc) return BERR_TRACE(rc);
    return NEXUS_SUCCESS;
}

NEXUS_SurfaceCompositorHandle NEXUS_SurfaceCompositor_Create( unsigned server_id )
{
    NEXUS_SurfaceCompositorHandle server;
    NEXUS_Error rc;
    unsigned i;

    server = BKNI_Malloc(sizeof(*server));
    if (!server) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_SurfaceCompositor, server);
    for(i=0;i<sizeof(server->display)/sizeof(server->display[0]);i++) {
        server->display[i] = NULL;
    }
    server->server_id = server_id;
    BLST_Q_INIT(&server->clients);
    BLST_D_INIT(&server->cursors);
    server->settings.enabled = true;
    NEXUS_OBJECT_REGISTER(NEXUS_SurfaceCompositor, server, Create);

    BLST_S_INSERT_HEAD(&g_nexus_surfacecompositor_list, server, link);

    server->frameBufferCallback = NEXUS_TaskCallback_Create(server, NULL);
    if (!server->frameBufferCallback) {
        rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }
    server->inactiveCallback = NEXUS_TaskCallback_Create(server, NULL);
    if (!server->inactiveCallback) {
        rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }

    NEXUS_CallbackHandler_Init(server->packetSpaceAvailableCallback, nexus_surface_compositor_p_packetspaceavailable, server);
    NEXUS_CallbackHandler_Init(server->checkpointCallback, nexus_surface_compositor_p_checkpoint, server);
    rc = nexus_p_open_blitter(server, false);
    if (rc) {
        rc = BERR_TRACE(rc);
        goto error;
    }
    server->state.update_flags = 0;
    NEXUS_Graphics2D_GetCapabilities(server->gfx, &server->bounceBuffer.capabilities);

    for (i=0;i<NEXUS_MAX_DISPLAYS;i++) {
        /* default settings per display */
        server->settings.display[i].enabled = true;
        server->settings.display[i].framebuffer.number = 2;
        server->settings.display[i].framebuffer.width = 720;
        server->settings.display[i].framebuffer.height = 480;
        server->settings.display[i].framebuffer.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
        server->settings.display[i].framebuffer.backgroundColor = 0xFF000000;
        server->settings.display[i].display3DSettings.overrideOrientation = false;
        server->settings.display[i].display3DSettings.orientation = NEXUS_VideoOrientation_e2D;
    }
    server->settings.bounceBuffer.width = 256;
    server->settings.bounceBuffer.height = 256;
    NEXUS_CallbackDesc_Init(&server->settings.frameBufferCallback);
    NEXUS_CallbackDesc_Init(&server->settings.inactiveCallback);
    server->display[0] = nexus_surface_compositor_p_create_display(server);
    if(server->display[0]==NULL) {
        goto error;
    }

    return server;

error:
    NEXUS_SurfaceCompositor_Destroy(server);
    return NULL;
}

static void NEXUS_SurfaceCompositor_P_Release( NEXUS_SurfaceCompositorHandle server )
{
    NEXUS_OBJECT_ASSERT(NEXUS_SurfaceCompositor, server);
    NEXUS_OBJECT_UNREGISTER(NEXUS_SurfaceCompositor, server, Destroy);
}

static void NEXUS_SurfaceCompositor_P_Finalizer( NEXUS_SurfaceCompositorHandle server )
{
    NEXUS_SurfaceCursorHandle cursor;
    NEXUS_SurfaceClientHandle client;
    unsigned i;

    NEXUS_OBJECT_ASSERT(NEXUS_SurfaceCompositor, server);

    if (server->inactiveTimer) {
        NEXUS_CancelTimer(server->inactiveTimer);
        server->inactiveTimer = NULL;
    }

    BLST_S_REMOVE(&g_nexus_surfacecompositor_list, server, NEXUS_SurfaceCompositor, link);

    /* coverity[use_after_free] */
    while(NULL!=(cursor=BLST_D_FIRST(&server->cursors))) {
        BDBG_WRN(("%p:leaked cursor %p", (void *)server, (void *)cursor));
        NEXUS_OBJECT_UNREGISTER(NEXUS_SurfaceCursor, cursor, Close);
        NEXUS_SurfaceCursor_Destroy(cursor);
    }

    /* coverity[use_after_free] */
    while(NULL!=(client=BLST_Q_FIRST(&server->clients))) {
        BDBG_WRN(("%p:leaked client %p", (void *)server, (void *)client));
        NEXUS_SurfaceCompositor_DestroyClient(client);
    }

    /* we are immediately inactive, not by waiting, but by destroying */
    nexus_surface_compositor_p_dealloc_displays(server);

    if(server->bounceBuffer.buffer) {
        NEXUS_Surface_Destroy(server->bounceBuffer.buffer);
    }

    if(server->gfx) {
        NEXUS_Graphics2D_Close(server->gfx);
    }
    NEXUS_CallbackHandler_Shutdown(server->packetSpaceAvailableCallback);
    NEXUS_CallbackHandler_Shutdown(server->checkpointCallback);

    if (server->frameBufferCallback) {
        NEXUS_TaskCallback_Destroy(server->frameBufferCallback);
    }
    if (server->inactiveCallback) {
        NEXUS_TaskCallback_Destroy(server->inactiveCallback);
    }

    /* release held surfaces */
    nexus_surface_compositor_p_release_surfaces(server);

    for (i=0;i<NEXUS_MAX_DISPLAYS;i++) {
        struct NEXUS_SurfaceCompositorDisplay *cmpDisplay = server->display[i];
        if(cmpDisplay) {
            nexus_surface_compositor_p_destroy_display(cmpDisplay);
            server->display[i]=NULL;
        }
    }
    NEXUS_P_SurfaceCompositorRenderElements_Shutdown(&server->renderState.elements);

    NEXUS_OBJECT_DESTROY(NEXUS_SurfaceCompositor, server);
#if NEXUS_SURFACE_COMPOSITOR_P_CHECK_IMMUTABLE
    NEXUS_BTRC_REPORT(surface_compositor_crc);
#endif
    NEXUS_BTRC_REPORT(surface_compositor_composite);
    NEXUS_BTRC_REPORT(surface_compositor_render);
    BKNI_Free(server);
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_SurfaceCompositor, NEXUS_SurfaceCompositor_Destroy);

void NEXUS_SurfaceCompositor_GetSettings( NEXUS_SurfaceCompositorHandle server, NEXUS_SurfaceCompositorSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(server, NEXUS_SurfaceCompositor);
    *pSettings = server->settings;
}

static void nexus_surfacemp_p_release_display_3dframebuffers(struct NEXUS_SurfaceCompositorDisplay *cmpDisplay)
{
    unsigned j;

    for (j=0;j<NEXUS_SURFACECMP_MAX_FRAMEBUFFERS;j++) {
        struct NEXUS_SurfaceCompositorFramebuffer *framebuffer = &cmpDisplay->framebuffer[j];
        if(framebuffer->view3D.left) {
            NEXUS_Surface_Destroy(framebuffer->view3D.left);
        }
        if(framebuffer->view3D.right) {
            NEXUS_Surface_Destroy(framebuffer->view3D.right);
        }
        framebuffer->view3D.left = NULL;
        framebuffer->view3D.right = NULL;
    }
    return;
}

static void nexus_surfacemp_p_release_display_framebuffers(struct NEXUS_SurfaceCompositorDisplay *cmpDisplay)
{
    unsigned j;
    /* this function should only be called after it ensured that there is no any outstanding activity in compositor or display */

    nexus_surfacemp_p_release_display_3dframebuffers(cmpDisplay);
    for (j=0;j<NEXUS_SURFACECMP_MAX_FRAMEBUFFERS;j++) {
        struct NEXUS_SurfaceCompositorFramebuffer *framebuffer = &cmpDisplay->framebuffer[j];
        if (framebuffer->surface) {
            NEXUS_Surface_Destroy(framebuffer->surface);
        }
        framebuffer->surface = NULL;
    }
    return;
}

static void nexus_surface_compositor_p_dealloc_displays(NEXUS_SurfaceCompositorHandle server)
{
    unsigned i;
    /* it's only handled from the NEXUS_SurfaceCompositor_Destroy */

    NEXUS_SurfaceCursor_P_Clear(server);

    for (i=0;i<NEXUS_MAX_DISPLAYS;i++) {
        struct NEXUS_SurfaceCompositorDisplay *cmpDisplay = server->display[i];
        if(cmpDisplay==NULL) {
            continue;
        }
        if (cmpDisplay->display) {
            NEXUS_Error rc;
            NEXUS_GraphicsSettings graphicsSettings;
            /* disable graphics and stop callbacks */
            if (i == 0) {
                (void)NEXUS_Display_SetVsyncCallback(cmpDisplay->display, NULL);
            }
            NEXUS_Display_GetGraphicsSettings(cmpDisplay->display, &graphicsSettings);
            NEXUS_CallbackDesc_Init(&graphicsSettings.frameBufferCallback);
            graphicsSettings.enabled = false;
            rc = NEXUS_Display_SetGraphicsSettings(cmpDisplay->display, &graphicsSettings);
            if (rc) rc = BERR_TRACE(rc);
            NEXUS_StopCallbacks(cmpDisplay->display); /* clear callbacks that are 'in-flight' */
            NEXUS_CallbackHandler_Shutdown(cmpDisplay->frameBufferCallback);
            NEXUS_CallbackHandler_Shutdown(cmpDisplay->vsyncCallback);
            NEXUS_StartCallbacks(cmpDisplay->display); /* enable further use of callbacks from the display */
        }

        if(server->bounceBufferMasterFramebuffer.buffer) {
            NEXUS_Surface_Destroy(server->bounceBufferMasterFramebuffer.buffer);
            server->bounceBufferMasterFramebuffer.buffer = NULL;
        }
        nexus_surfacemp_p_release_display_framebuffers(cmpDisplay);
    }
    return;
}

/* disable without stopping callbacks */
static void nexus_surfacemp_p_disable_display(NEXUS_SurfaceCompositorHandle server, struct NEXUS_SurfaceCompositorDisplay *cmpDisplay)
{
    NEXUS_Error rc;
    NEXUS_GraphicsSettings graphicsSettings;
    BSTD_UNUSED(server);
    NEXUS_Display_GetGraphicsSettings(cmpDisplay->display, &graphicsSettings);
    graphicsSettings.enabled = false;
    rc = NEXUS_Display_SetGraphicsSettings(cmpDisplay->display, &graphicsSettings);
    if (rc) rc = BERR_TRACE(rc);
    return;
}

static NEXUS_Error nexus_surface_compositor_p_alloc_framebuffers(struct NEXUS_SurfaceCompositorDisplay *cmpDisplay, const NEXUS_SurfaceCompositorDisplaySettings *pSettings)
{
    NEXUS_SurfaceCreateSettings createSettings;
    unsigned j;
    NEXUS_Error rc = NEXUS_SUCCESS;

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    /* create new framebuffers if needed */
    for (j=0;j<cmpDisplay->num_framebuffers;j++) {
        struct NEXUS_SurfaceCompositorFramebuffer *framebuffer = &cmpDisplay->framebuffer[j];

        /* already initialized above */
        framebuffer->scene.dirty.width = pSettings->framebuffer.width;
        framebuffer->scene.dirty.height = pSettings->framebuffer.height;
        /* allocate framebuffer surface */
        if (framebuffer->surface==NULL) {
            BDBG_ASSERT(framebuffer->view3D.left==NULL);
            BDBG_ASSERT(framebuffer->view3D.right==NULL);
            createSettings.width = pSettings->framebuffer.width;
            createSettings.height = pSettings->framebuffer.height;
            if (!createSettings.width || !createSettings.height) {
                /* get accurate error for somewhat likely misconfig */
                rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
                goto create_error;
            }
            createSettings.heap = pSettings->framebuffer.heap;
            createSettings.pixelFormat = pSettings->framebuffer.pixelFormat;
            createSettings.pMemory = NULL;
            framebuffer->surface = NEXUS_Display_CreateFramebuffer(cmpDisplay->display, &createSettings);
            if (!framebuffer->surface) {
                rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
                goto create_error;
            }
        }
        BLST_SQ_INSERT_TAIL(&cmpDisplay->available, framebuffer, link);
    }
    /* create 3D buffers when needed */
    for (j=0;j<cmpDisplay->num_framebuffers;j++) {
        struct NEXUS_SurfaceCompositorFramebuffer *framebuffer = &cmpDisplay->framebuffer[j];
        if( (framebuffer->view3D.left == NULL || framebuffer->view3D.right == NULL) &&
            (cmpDisplay->formatInfo.orientation==NEXUS_VideoOrientation_e3D_LeftRight || cmpDisplay->formatInfo.orientation==NEXUS_VideoOrientation_e3D_OverUnder)) {
                NEXUS_SurfaceMemory memory;
                const NEXUS_PixelFormatConvertInfo *pixelInfo;

                BDBG_ASSERT(framebuffer->surface);
                NEXUS_Surface_GetMemory(framebuffer->surface, &memory);
                pixelInfo = NEXUS_P_PixelFormat_GetConvertInfo_isrsafe(createSettings.pixelFormat);
                createSettings.width = cmpDisplay->canvas.width;
                createSettings.height = cmpDisplay->canvas.height;
                createSettings.pitch = memory.pitch;
                createSettings.pMemory = memory.buffer;
                framebuffer->view3D.left = NEXUS_Display_CreateFramebuffer(cmpDisplay->display, &createSettings);
                if (!framebuffer->view3D.left) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto create_error; }
                createSettings.pMemory = (void *)((uint8_t *)memory.buffer + pixelInfo->info.bpp/8 * cmpDisplay->offset3DRight.x + memory.pitch * cmpDisplay->offset3DRight.y);
                framebuffer->view3D.right = NEXUS_Display_CreateFramebuffer(cmpDisplay->display, &createSettings);
                if (!framebuffer->view3D.right) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto create_error; }
        }
    }
    return rc;

create_error:
    /* let caller unwind */
    return rc;
}

/* In bypass mode, we dynamaically alloc/free the framebuffers. Bypass mode is usually for low bandwidth systems, which
are also memory size systems. */
NEXUS_Error nexus_surface_compositor_p_realloc_framebuffers(NEXUS_SurfaceCompositorHandle server)
{
    unsigned i;
    /* check for no change */
    if ((server->bypass_compose.client == NULL) == (server->display[0] && (server->display[0]->framebuffer[0].surface != NULL))) {
        return NEXUS_SUCCESS;
    }

    for (i=0;i<NEXUS_MAX_DISPLAYS;i++) {
        struct NEXUS_SurfaceCompositorDisplay *cmpDisplay = server->display[i];
        if (!cmpDisplay) continue;
        if (server->bypass_compose.client) {
            nexus_surfacemp_p_release_display_framebuffers(cmpDisplay);
        }
        else {
            NEXUS_Error rc;
            /* if we cannot alloc, system is in unusable state */
            rc = nexus_surface_compositor_p_alloc_framebuffers(cmpDisplay, &server->settings.display[i]);
            if (rc) return BERR_TRACE(rc);
        }
    }
    return NEXUS_SUCCESS;
}

#define COPY_STRUCT_FIELD(TO, FROM, FIELD) do {if ((TO)->FIELD != (FROM)->FIELD) { (TO)->FIELD = (FROM)->FIELD; change = true; }}while(0)

static NEXUS_Error nexus_surface_compositor_p_update_graphics_settings( NEXUS_SurfaceCompositorHandle server, const NEXUS_SurfaceCompositorSettings *pSettings )
{
    unsigned i;
    NEXUS_Error rc;
    BSTD_UNUSED(server);
    for (i=0;i<NEXUS_MAX_DISPLAYS;i++) {
        NEXUS_GraphicsSettings graphicsSettings;
        bool change = false;
        bool toogle_enable;
        if (!pSettings->display[i].display) continue;
        NEXUS_Display_GetGraphicsSettings(pSettings->display[i].display, &graphicsSettings);
        /* update only those members which NSC does not calculate */
        COPY_STRUCT_FIELD(&graphicsSettings, &pSettings->display[i].graphicsSettings, sourceBlendFactor);
        COPY_STRUCT_FIELD(&graphicsSettings, &pSettings->display[i].graphicsSettings, destBlendFactor);
        COPY_STRUCT_FIELD(&graphicsSettings, &pSettings->display[i].graphicsSettings, constantAlpha);
        COPY_STRUCT_FIELD(&graphicsSettings, &pSettings->display[i].graphicsSettings, horizontalFilter);
        COPY_STRUCT_FIELD(&graphicsSettings, &pSettings->display[i].graphicsSettings, verticalFilter);
        COPY_STRUCT_FIELD(&graphicsSettings, &pSettings->display[i].graphicsSettings, horizontalCoeffIndex);
        COPY_STRUCT_FIELD(&graphicsSettings, &pSettings->display[i].graphicsSettings, verticalCoeffIndex);
        COPY_STRUCT_FIELD(&graphicsSettings, &pSettings->display[i].graphicsSettings, alpha);
        COPY_STRUCT_FIELD(&graphicsSettings.sdrToHdr, &pSettings->display[i].graphicsSettings.sdrToHdr, y);
        COPY_STRUCT_FIELD(&graphicsSettings.sdrToHdr, &pSettings->display[i].graphicsSettings.sdrToHdr, cb);
        COPY_STRUCT_FIELD(&graphicsSettings.sdrToHdr, &pSettings->display[i].graphicsSettings.sdrToHdr, cr);
        toogle_enable = graphicsSettings.enabled != pSettings->display[i].enabled;
        COPY_STRUCT_FIELD(&graphicsSettings, &pSettings->display[i], enabled);
        if (change) {
            rc = NEXUS_Display_SetGraphicsSettings(pSettings->display[i].display, &graphicsSettings);
            if (rc) return BERR_TRACE(rc);
        }
        if(toogle_enable) {
            /* if enabled was changed we need to simulate that submitted framebuffers was applied */
            if(server->settings.display[i].display && server->display) {
                struct NEXUS_SurfaceCompositorDisplay *display =  server->display[i];
                if(display && display->submitted) {
                    nexus_surface_compositor_p_framebuffer_applied(display);
                }
            }
        }
    }
    return 0;
}

static bool nexus_p_is_secure_heap(NEXUS_HeapHandle heap)
{
    NEXUS_MemoryStatus status;
    NEXUS_Heap_GetStatus_driver(heap, &status);
    return status.memoryType & NEXUS_MEMORY_TYPE_SECURE && !(status.memoryType & NEXUS_MEMORY_TYPE_SECURE_OFF);
}

static void nexus_surfacemp_p_release_all_display_framebuffers(NEXUS_SurfaceCompositorHandle server)
{
    unsigned i;
    for (i=0;i<NEXUS_MAX_DISPLAYS;i++) {
        if (server->display[i]) {
            nexus_surfacemp_p_release_display_framebuffers(server->display[i]);
        }
    }
}

static NEXUS_Error nexus_p_switch_secure(NEXUS_SurfaceCompositorHandle server, bool secure)
{
    int rc;
    struct NEXUS_SurfaceCompositorDisplay *cmpDisplay = server->display[0];

    if (!cmpDisplay) return -1;

    BDBG_MSG(("GFD0 secure %u -> %u", !secure, secure));
    nexus_surfacemp_p_release_all_display_framebuffers(server);
    if (server->bounceBufferMasterFramebuffer.buffer) {
        NEXUS_Surface_Destroy(server->bounceBufferMasterFramebuffer.buffer);
        server->bounceBufferMasterFramebuffer.buffer = NULL;
    }
    NEXUS_Graphics2D_Close(server->gfx);
    server->gfx = NULL;
    server->state.blitter.active = false;
    server->state.blitter.delayed = false;
    rc = nexus_p_open_blitter(server, secure);
    if (rc) return BERR_TRACE(rc);
    return NEXUS_SUCCESS;
}

static void nexus_p_post_switch_secure(NEXUS_SurfaceCompositorHandle server)
{
    NEXUS_SurfaceClientHandle client;
    for (client=BLST_Q_FIRST(&server->clients); client; client = BLST_Q_NEXT(client, link)) {
        if (client->state.client_type == client_type_set) {
            /* force a new copy into secure or unsecure memory */
            BKNI_Memset(&client->set.serverCreateSettings, 0, sizeof(client->set.serverCreateSettings));
            NEXUS_SurfaceClient_P_CopySetSurface(client, client->set.surface.surface);
            client->set.updating = false;
        }
    }
}

static NEXUS_Error nexus_surface_compositor_p_update_display( NEXUS_SurfaceCompositorHandle server, const NEXUS_SurfaceCompositorSettings *pSettings )
{
    unsigned i;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_Error rc;
    bool secureFramebuffer;
    bool switch_secure;

    BDBG_ASSERT(!server->state.active);

    secureFramebuffer = nexus_p_is_secure_heap(pSettings->display[0].framebuffer.heap);
    switch_secure = secureFramebuffer != server->secureFramebuffer;

    if (switch_secure) {
        rc = nexus_p_switch_secure(server, secureFramebuffer);
        if (rc) {BERR_TRACE(rc); goto err_switch_secure;}
    }

    for (i=0;i<NEXUS_MAX_DISPLAYS;i++) {
        struct NEXUS_SurfaceCompositorDisplay *cmpDisplay = server->display[i];
        NEXUS_VideoFormatInfo formatInfo;
        NEXUS_DisplaySettings displaySettings;
        bool defaultClip = false;

        if(cmpDisplay==NULL) {
            if (pSettings->display[i].display==NULL) {
                continue;
            }
            /* try to create new display */
            cmpDisplay = nexus_surface_compositor_p_create_display(server);
            if(cmpDisplay==NULL) {
                rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
                goto err_create_display;
            }
            server->display[i] = cmpDisplay;
        }

        nexus_surface_compositor_p_initialize_compositor_display(cmpDisplay);

        if (pSettings->display[i].display) {
            NEXUS_VideoOrientation prevOrientation = cmpDisplay->formatInfo.orientation;
            NEXUS_SurfaceRegion prevCanvas = cmpDisplay->canvas;

            if (pSettings->display[i].framebuffer.number > NEXUS_SURFACECMP_MAX_FRAMEBUFFERS ||
                pSettings->display[i].framebuffer.number < 1)
            {
                rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
                goto err_create_display;
            }
            cmpDisplay->num_framebuffers = pSettings->display[i].framebuffer.number;

            if (server->settings.display[i].display) {
                /* this display already registered, so check if framebuffer must be recreated */
                if (pSettings->display[i].framebuffer.width != server->settings.display[i].framebuffer.width ||
                    pSettings->display[i].framebuffer.height != server->settings.display[i].framebuffer.height)
                {
                    nexus_surfacemp_p_release_display_framebuffers(cmpDisplay);
                }
            }

            NEXUS_Display_GetSettings(pSettings->display[i].display, &displaySettings);
            NEXUS_VideoFormat_GetInfo(displaySettings.format, &formatInfo);
            if(formatInfo.height==482) {
                formatInfo.height = 480; /* FIX for NTSC 482 vs 480 */
            }
            cmpDisplay->index = i;
            cmpDisplay->display = pSettings->display[i].display;
            cmpDisplay->backgroundColor = pSettings->display[i].framebuffer.backgroundColor;

            graphicsSettings = pSettings->display[i].graphicsSettings;
            graphicsSettings.enabled = pSettings->display[i].enabled;
            /* at this time only full screen graphics is supported */
            graphicsSettings.clip.x = graphicsSettings.clip.y = 0;
            if (!pSettings->display[i].manualPosition) {
                graphicsSettings.position.x = graphicsSettings.position.y = 0;
                graphicsSettings.position.width = formatInfo.width;
                graphicsSettings.position.height = formatInfo.height;
            }
            if (graphicsSettings.clip.width==0 || graphicsSettings.clip.width>pSettings->display[i].framebuffer.width||
                graphicsSettings.clip.height==0 || graphicsSettings.clip.height>pSettings->display[i].framebuffer.height) {
                graphicsSettings.clip.width = pSettings->display[i].framebuffer.width;
                graphicsSettings.clip.height = pSettings->display[i].framebuffer.height;
                defaultClip = true;
            }
            cmpDisplay->canvas.width = graphicsSettings.clip.width;
            cmpDisplay->canvas.height = graphicsSettings.clip.height;
            cmpDisplay->formatInfo.canvas.width = formatInfo.width;
            cmpDisplay->formatInfo.canvas.height = formatInfo.height;
            cmpDisplay->formatInfo.orientation = NEXUS_VideoOrientation_e2D;
            cmpDisplay->formatInfo.native3D = false;
            cmpDisplay->formatInfo.verticalFreq = formatInfo.verticalFreq;
            {
                NEXUS_DisplayStatus sDisplayStatus;
                long lTmp;
                unsigned vsyncIntervalUsec;
                NEXUS_Display_GetStatus(pSettings->display[i].display, &sDisplayStatus);
                vsyncIntervalUsec = (1000 * 1000 * 1000) / sDisplayStatus.refreshRate;
                lTmp = vsyncIntervalUsec * NEXUS_P_SURFACECMP_HYSTERESIS_THRESHOLD / 100;
                cmpDisplay->formatInfo.vsyncIntervalUsecHystLow  = ((long)vsyncIntervalUsec) - lTmp;
                cmpDisplay->formatInfo.vsyncIntervalUsecHystHigh = ((long)vsyncIntervalUsec) + lTmp;
            }
            cmpDisplay->offset3DRight.x = cmpDisplay->offset3DRight.y = 0;

            if(pSettings->display[i].display3DSettings.overrideOrientation || displaySettings.display3DSettings.overrideOrientation || formatInfo.isFullRes3d) {
                if(pSettings->display[i].display3DSettings.overrideOrientation) {
                    cmpDisplay->formatInfo.orientation = pSettings->display[i].display3DSettings.orientation;
                } else if (displaySettings.display3DSettings.overrideOrientation) {
                    cmpDisplay->formatInfo.orientation = displaySettings.display3DSettings.orientation;
                    cmpDisplay->formatInfo.native3D = cmpDisplay->formatInfo.orientation != NEXUS_VideoOrientation_e2D;
                } else {
                    BDBG_ASSERT(formatInfo.isFullRes3d);
                    defaultClip = true; /* have to divide canvas and clip */
                    cmpDisplay->formatInfo.orientation = NEXUS_VideoOrientation_e3D_OverUnder;
                    cmpDisplay->formatInfo.native3D = true;
                    cmpDisplay->formatInfo.canvas.height *= 2; /* it would get divided latter */
                    graphicsSettings.position.height *= 2;
                    /* cmpDisplay->canvas.height and graphicsSettings.clip.height left as is, and would get divided latter, effectively size of the framebuffer in pixels is controlled by an application */
                }
                switch(cmpDisplay->formatInfo.orientation) {
                case NEXUS_VideoOrientation_e3D_LeftRight:
                    cmpDisplay->formatInfo.canvas.width /= 2;
                    if(!cmpDisplay->formatInfo.native3D || defaultClip) {
                        if(cmpDisplay->canvas.width%2) {
                            BDBG_WRN(("%p:display:%u invalid framebuffer width:%u", (void *)server, i, cmpDisplay->canvas.width));
                        }
                        cmpDisplay->canvas.width /= 2; /* with native3D and  default clip, canvas has to be halved */
                    }
                    cmpDisplay->offset3DRight.x = cmpDisplay->canvas.width;
                    if(cmpDisplay->formatInfo.native3D) {
                        graphicsSettings.position.width /= 2;
                        if(defaultClip) {
                            graphicsSettings.clip.width /= 2;
                        }
                    }
                    break;
                case NEXUS_VideoOrientation_e3D_OverUnder:
                    cmpDisplay->formatInfo.canvas.height /= 2;
                    if(!cmpDisplay->formatInfo.native3D || defaultClip) {
                        if(cmpDisplay->canvas.height%2) {
                            BDBG_WRN(("%p:display:%d invalid framebuffer height%u", (void *)server, i, cmpDisplay->canvas.height));
                        }
                        cmpDisplay->canvas.height /= 2; /* with native3D and  default clip, canvas has to be halved */
                    }
                    cmpDisplay->offset3DRight.y = cmpDisplay->canvas.height;
                    if(cmpDisplay->formatInfo.native3D) {
                        graphicsSettings.position.height /= 2;
                        if(defaultClip) {
                            graphicsSettings.clip.height /= 2;
                        }
                    }
                    break;
                default:
                    break;
                }
            }

            BDBG_MSG_TRACE(("%p: display:%u format:%u(%ux%u) display.canvas:%ux%u framebuffer.canvas:%ux%u clip:%ux%u position:%ux%u %s%s", (void *)server, i, displaySettings.format, graphicsSettings.position.width, graphicsSettings.position.height, cmpDisplay->formatInfo.canvas.width, cmpDisplay->formatInfo.canvas.height,  cmpDisplay->canvas.width, cmpDisplay->canvas.height, graphicsSettings.clip.width, graphicsSettings.clip.height, graphicsSettings.position.width, graphicsSettings.position.height, cmpDisplay->formatInfo.native3D?"native ":"", cmpDisplay->formatInfo.orientation==NEXUS_VideoOrientation_e2D?"2D":"3D"));
            rc = NEXUS_SurfaceCursor_P_UpdateDisplay(server); /* update cursor state and verify if new configuration compatible with cursor configuration */
            if(rc!=NEXUS_SUCCESS) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error;}

            if(
               prevOrientation != cmpDisplay->formatInfo.orientation || /* change in 3D mode */
               (cmpDisplay->formatInfo.orientation != NEXUS_VideoOrientation_e2D && (cmpDisplay->canvas.width != prevCanvas.width || cmpDisplay->canvas.height != prevCanvas.height)) /* change in canvas */
               ) {
                nexus_surfacemp_p_release_display_3dframebuffers(cmpDisplay);
            }

            if(i==0 && pSettings->display[i].framebuffer.pixelFormat==NEXUS_PixelFormat_eCompressed_A8_R8_G8_B8 && !server->bounceBufferMasterFramebuffer.buffer) {
                NEXUS_SurfaceCreateSettings createSettings;
                NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
                createSettings.width = pSettings->display[i].framebuffer.width;
                createSettings.height = pSettings->display[i].framebuffer.height;
                createSettings.heap = pSettings->display[i].framebuffer.heap;
                createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
                createSettings.pMemory = NULL;
                server->bounceBufferMasterFramebuffer.buffer = NEXUS_Surface_Create(&createSettings);
                if (!server->bounceBufferMasterFramebuffer.buffer) {
                    rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
                    goto error;
                }
            }

            rc = nexus_surface_compositor_p_alloc_framebuffers(cmpDisplay, &pSettings->display[i]);
            if (rc) {
                rc = BERR_TRACE(rc);
                goto error;
            }

            NEXUS_CallbackHandler_PrepareCallback(cmpDisplay->frameBufferCallback, graphicsSettings.frameBufferCallback);
            if (i == 0) {
                graphicsSettings.secure = secureFramebuffer;
            }
            rc = NEXUS_Display_SetGraphicsSettings(pSettings->display[i].display, &graphicsSettings);
            if (rc) {
                rc = BERR_TRACE(rc);
                goto error;
            }

            if (i == 0) {
                NEXUS_CallbackDesc desc;
                NEXUS_CallbackHandler_PrepareCallback(cmpDisplay->vsyncCallback, desc);
                rc = NEXUS_Display_SetVsyncCallback(cmpDisplay->display, &desc);
                if (rc) {rc = BERR_TRACE(rc); goto error;}
            }
        }
        else {
            /* display being unregistered */
            if (cmpDisplay->display) {
                nexus_surfacemp_p_disable_display(server, cmpDisplay);
                cmpDisplay->display = NULL;
            }
            nexus_surfacemp_p_release_display_framebuffers(cmpDisplay);
        }
    }

    server->settings = *pSettings;
    server->secureFramebuffer = secureFramebuffer;

    rc = nexus_surface_compositor_p_verify_tunnel(server);
    if (rc) {
        rc = BERR_TRACE(rc);
        /* TODO: does not unwind server->settings. */
        goto err_verify_tunnel;
    }

    if (switch_secure) {
        nexus_p_post_switch_secure(server);
    }

    nexus_p_surface_compositor_update_all_clients(server, NEXUS_P_SURFACECLIENT_UPDATE_DISPLAY);

    return 0;

err_verify_tunnel:
error:
    nexus_surfacemp_p_release_all_display_framebuffers(server);
err_create_display:
    if (secureFramebuffer != server->secureFramebuffer) {
        nexus_p_switch_secure(server, server->secureFramebuffer);
    }
err_switch_secure:
    return rc;
}

static void nexus_surface_compositor_p_inactive_timer(void *context)
{
    NEXUS_SurfaceCompositorHandle server = context;
    NEXUS_SurfaceClientHandle client;
    bool fire_again = false;

    BDBG_MSG(("inactive_timer enabled=%d", server->settings.enabled));
    server->inactiveTimer = NULL;

    if (!server->settings.enabled) {
        /* fire displayed events to all clients who have submitted */
        for (client = BLST_Q_FIRST(&server->clients); client; client = BLST_Q_NEXT(client, link)) {
            if(client->state.client_type == client_type_set && client->set.dirty) {
                client->set.dirty = false;
                client->set.updating = false;
            }
            if (client->pending_displayed_callback) {
                client->pending_displayed_callback = false;
                client->process_pending_displayed_callback = false;
                NEXUS_TaskCallback_Fire(client->displayedCallback);
            }
        }
        fire_again = true;
    } else {
        unsigned i;
        for (i=0;i<NEXUS_MAX_DISPLAYS;i++) {
            if(server->settings.display[i].display && !server->settings.display[i].enabled) {
                struct NEXUS_SurfaceCompositorDisplay *display =  server->display[i];
                if(display && display->submitted) {
                    nexus_surface_compositor_p_framebuffer_applied(display);
                    fire_again = true;
                }
            }
        }
    }
    if(fire_again) {
        nexus_surface_compositor_p_schedule_inactive_timer(server);
    }
}

void nexus_surface_compositor_p_schedule_inactive_timer(NEXUS_SurfaceCompositorHandle server)
{
    if (!server->inactiveTimer) {
        server->inactiveTimer = NEXUS_ScheduleTimer(50, nexus_surface_compositor_p_inactive_timer, server);
    }
    return;
}

NEXUS_Error NEXUS_SurfaceCompositor_SetSettings( NEXUS_SurfaceCompositorHandle server, const NEXUS_SurfaceCompositorSettings *pSettings )
{
    NEXUS_Error rc = 0;
    bool enable_changed;
    unsigned i;

    BDBG_OBJECT_ASSERT(server, NEXUS_SurfaceCompositor);

    if (server->state.active) {
        for (i=0;i<NEXUS_MAX_DISPLAYS;i++) {
            if (pSettings->display[i].display != server->settings.display[i].display ||
                pSettings->display[i].framebuffer.number != server->settings.display[i].framebuffer.number ||
                pSettings->display[i].framebuffer.width != server->settings.display[i].framebuffer.width ||
                pSettings->display[i].framebuffer.height != server->settings.display[i].framebuffer.height ||
                pSettings->display[i].framebuffer.pixelFormat != server->settings.display[i].framebuffer.pixelFormat ||
                pSettings->display[i].framebuffer.heap != server->settings.display[i].framebuffer.heap ||
                pSettings->display[i].display3DSettings.overrideOrientation != server->settings.display[i].display3DSettings.overrideOrientation ||
                pSettings->display[i].display3DSettings.orientation != server->settings.display[i].display3DSettings.orientation ||
                pSettings->bounceBuffer.width != server->settings.bounceBuffer.width ||
                pSettings->bounceBuffer.height != server->settings.bounceBuffer.height ||
                pSettings->bounceBuffer.heap != server->settings.bounceBuffer.heap)
            {
                BDBG_ERR(("you must disable the surface compositor and wait for the inactiveCallback before changing display settings."));
                return BERR_TRACE(NEXUS_NOT_AVAILABLE);
            }
        }
    }

    /* these settings can be set regardless of enabled/active state */
    NEXUS_TaskCallback_Set(server->frameBufferCallback, &pSettings->frameBufferCallback);
    NEXUS_TaskCallback_Set(server->inactiveCallback, &pSettings->inactiveCallback);
    enable_changed = pSettings->enabled != server->settings.enabled;

    if (!server->state.active) {
        if (pSettings->bounceBuffer.width != server->settings.bounceBuffer.width ||
            pSettings->bounceBuffer.height != server->settings.bounceBuffer.height ||
            pSettings->bounceBuffer.heap != server->settings.bounceBuffer.heap) {
            if(server->bounceBuffer.buffer) {
                NEXUS_Surface_Destroy(server->bounceBuffer.buffer);
                server->bounceBuffer.buffer = NULL;
            }
        }

        rc = nexus_surface_compositor_p_update_display(server, pSettings);
        if (rc) return BERR_TRACE(rc);

        server->state.active = pSettings->enabled;
        if (server->state.active) {
            BDBG_MSG(("resume composition after enabling"));
            nexus_surface_compositor_p_compose(server);
        }
    }
    else {
        unsigned i;
        bool muteVideoChanged = false;
        for (i=0;i<NEXUS_MAX_DISPLAYS;i++) {
            if (pSettings->muteVideo[i] != server->settings.muteVideo[i]) {
                muteVideoChanged = true;
                break;
            }
        }
        rc = nexus_surface_compositor_p_update_graphics_settings(server, pSettings);
        if (rc) return BERR_TRACE(rc);

        /* if active, the only possible change is the enabled flag. */
        server->settings = *pSettings;

        if (enable_changed && !pSettings->enabled) {
            /* starting disabled -> !active process */
            nexus_p_surface_compositor_check_inactive(server);
            if (!server->inactiveTimer) {
                /* kick start timer that keeps clients alive if fmt change takes a long time */
                nexus_surface_compositor_p_schedule_inactive_timer(server);
            }
        }
        else if (muteVideoChanged) {
            nexus_p_surface_compositor_update_all_clients(server, NEXUS_P_SURFACECLIENT_UPDATE_DISPLAY);
        }
    }

    if (enable_changed) {
        /* we only have to check enabled flag for client displayStatusChanged because
        no display setting can change while enabled, and no display change matters while disabled */
        NEXUS_SurfaceClientHandle client;
        for (client = BLST_Q_FIRST(&server->clients); client; client = BLST_Q_NEXT(client, link)) {
            NEXUS_TaskCallback_Fire(client->displayStatusChangedCallback);
        }
    }

    return rc;
}

void NEXUS_SurfaceCompositor_GetClientSettings( NEXUS_SurfaceCompositorHandle server, NEXUS_SurfaceClientHandle client, NEXUS_SurfaceCompositorClientSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(server, NEXUS_SurfaceCompositor);
    BDBG_OBJECT_ASSERT(client, NEXUS_SurfaceClient);
    *pSettings = client->serverSettings;
}

#if 0
static bool nexus_rect_is_not_intersected(const NEXUS_Rect *a, const NEXUS_Rect *b)
{
    return
    (a->x + a->width <= b->x) ||  /* a completely to the left */
    (a->x >= b->x + b->width) ||  /* a completely to the right */
    (a->y + a->height <= b->y) || /* a completely to the top */
    (a->y >= b->y + b->height);    /* a completely to the bottom */
}
#endif

/* insert client to server's client list based on zorder. sorted in ascending order. 0 is on bottom.
if equal, last one inserted goes at the end (which is on top).
*/
void nexus_surfacecmp_p_insert_client(NEXUS_SurfaceCompositorHandle server, NEXUS_SurfaceClientHandle client, bool remove)
{
    NEXUS_SurfaceClientHandle temp, prev = NULL;
    const NEXUS_SurfaceComposition *pSettings = &client->serverSettings.composition;
    if (remove) {
        BLST_Q_REMOVE(&server->clients, client, link);
    }
    /* TODO: optimize search */
    for (temp = BLST_Q_FIRST(&server->clients); temp; temp = BLST_Q_NEXT(temp, link)) {
        BDBG_ASSERT(temp != client);
        /* insert after zorder matches so that last client set with same zorder goes on top */
        if (temp->serverSettings.composition.zorder > pSettings->zorder) {
            if (prev) {
                BLST_Q_INSERT_AFTER(&server->clients, prev, client, link);
            }
            else {
                BLST_Q_INSERT_HEAD(&server->clients, client, link);
            }
            break;
        }
        prev = temp;
    }
    if (!temp) {
        if (prev) {
            BLST_Q_INSERT_AFTER(&server->clients, prev, client, link);
        }
        else {
            BLST_Q_INSERT_HEAD(&server->clients, client, link);
        }
    }
}

/* insert child to client list based on zorder. sorted in ascending order. 0 is on bottom.
if equal, last one inserted goes at the end (which is on top).
*/
void nexus_surfacecmp_p_insert_child(NEXUS_SurfaceClientHandle parent, NEXUS_SurfaceClientHandle client, bool remove)
{
    struct nexus_client_children_t *head;
    NEXUS_SurfaceClientHandle curr;
    NEXUS_SurfaceClientHandle next;
    unsigned zorder;
    const NEXUS_SurfaceClientSettings *pSettings = &client->settings;

    if (remove) {
        BLST_S_REMOVE(&parent->children, client, NEXUS_SurfaceClient, child_link);
    }

    head = &parent->children;
    zorder = pSettings->composition.zorder;
    if (BLST_S_EMPTY(head)) {
        BLST_S_INSERT_HEAD(head, client, child_link);
    }
    else {
        curr = BLST_S_FIRST(head);
        if (curr->settings.composition.zorder > zorder) {
            BLST_S_INSERT_HEAD(head, client, child_link);
        }
        else {
            do {
                next = BLST_S_NEXT(curr, child_link);
                if (next == NULL) {
                    break;
                }
                if (next->settings.composition.zorder > zorder) {
                    break;
                }
                curr = next;
            } while (1);
            BLST_S_INSERT_AFTER(head, curr, client, child_link);
        }
    }
}

NEXUS_Error NEXUS_SurfaceCompositor_SetClientSettings( NEXUS_SurfaceCompositorHandle server, NEXUS_SurfaceClientHandle client, const NEXUS_SurfaceCompositorClientSettings *pSettings )
{
    bool resize, move;
    unsigned flags = 0;

    BDBG_OBJECT_ASSERT(server, NEXUS_SurfaceCompositor);
    BDBG_OBJECT_ASSERT(client, NEXUS_SurfaceClient);

    if (pSettings->tunnelCapable && server->tunnel.client && server->tunnel.client != client) {
        /* currently, there can only be one tunneled client. it's conceivable that we could support
        more than one, if they are non-overlapping, but it would be complicated. */
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if (pSettings->composition.displayCache) {
        /* This feature has been deprecated. Instead, the client should call NEXUS_SurfaceClient_GetStatus and resize what it submits. */
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    if (client->serverSettings.composition.zorder != pSettings->composition.zorder) {
        flags = NEXUS_P_SURFACECLIENT_UPDATE_ZORDER;
    }

    resize = pSettings->composition.position.width != client->serverSettings.composition.position.width ||
             pSettings->composition.position.height != client->serverSettings.composition.position.height;
    move =   pSettings->composition.position.x != client->serverSettings.composition.position.x ||
             pSettings->composition.position.y != client->serverSettings.composition.position.y;

    /* can't move or resize active tunneled client */
    if( client->state.client_type == client_type_tunnel && (resize || move)) { return BERR_TRACE(NEXUS_NOT_SUPPORTED); }

    flags |= client->serverSettings.composition.visible != pSettings->composition.visible ? NEXUS_P_SURFACECLIENT_UPDATE_VISIBLE : 0;
    if (BKNI_Memcmp(&client->serverSettings.composition, &pSettings->composition, sizeof(client->serverSettings.composition))) {
        flags |= NEXUS_P_SURFACECLIENT_UPDATE_CLIENT;
        resize = true;
        move = true;
    }

    /* change of video window */
    flags |= BKNI_Memcmp(&pSettings->display, &client->serverSettings.display, sizeof(pSettings->display)) ? NEXUS_P_SURFACECLIENT_UPDATE_CLIENT : 0;

    /* save settings */
    client->serverSettings = *pSettings;
    nexus_surfaceclient_p_setwindows(client);

    if (flags & NEXUS_P_SURFACECLIENT_UPDATE_ZORDER) {
        nexus_surfacecmp_p_insert_client(server, client, true);
    }
    nexus_p_surface_compositor_update_client(client, flags | (resize?NEXUS_P_SURFACECLIENT_UPDATE_SIZE:0)|(move?NEXUS_P_SURFACECLIENT_UPDATE_POSITION:0));

    return NEXUS_SUCCESS;
}

/*
pInputCoord = input coordinates
pOutputCoord = output coordinates
pRect = [in/out] param to be converted from inputCoordinates to outputCoordinates
*/
void nexus_surfacemp_p_convert_coord(const NEXUS_SurfaceRegion *pInputCoord, const NEXUS_SurfaceRegion *pOutputCoord, const NEXUS_Rect *pSrcRect, NEXUS_Rect *pDstRect)
{
    BDBG_ASSERT((const void *)pSrcRect != (const void *)pDstRect);
    if (pInputCoord->width && pInputCoord->height) {
        pDstRect->x = pSrcRect->x * (int)pOutputCoord->width / (int)pInputCoord->width;
        pDstRect->width = pSrcRect->width * pOutputCoord->width / pInputCoord->width;
        pDstRect->y = pSrcRect->y * (int)pOutputCoord->height / (int)pInputCoord->height;
        pDstRect->height = pSrcRect->height * pOutputCoord->height / pInputCoord->height;
    }
    else {
        pDstRect->x = pDstRect->y = pDstRect->width = pDstRect->height = 0;
    }
    return;
}

void nexus_surfacemp_p_clip_rect(const NEXUS_Rect *pBound, const NEXUS_Rect *pSrcRect, NEXUS_Rect *pDstRect)
{
    int inc;
    BDBG_ASSERT((const void *)pSrcRect != (const void *)pDstRect);

    if (pSrcRect->x >= pBound->x + pBound->width ||
        pSrcRect->y >= pBound->y + pBound->height ||
        pSrcRect->x + pSrcRect->width < pBound->x ||
        pSrcRect->y + pSrcRect->height < pBound->y)
    {
        pDstRect->y = pBound->y;
        pDstRect->x = pBound->x;
        pDstRect->width = pDstRect->height = 0;
        return;
    }

    inc = pSrcRect->x < pBound->x ? (pBound->x - pSrcRect->x) : 0;
    pDstRect->width = pSrcRect->width - inc;
    pDstRect->x = pSrcRect->x + inc;

    inc = pSrcRect->y < pBound->y ? (pBound->y - pSrcRect->y) : 0;
    pDstRect->height = pSrcRect->height - inc;
    pDstRect->y = pSrcRect->y + inc;

    if (pDstRect->x + pDstRect->width > pBound->x + pBound->width) {
        pDstRect->width = pBound->x + pBound->width - pDstRect->x;
    }
    if (pDstRect->y + pDstRect->height > pBound->y + pBound->height) {
        pDstRect->height = pBound->y + pBound->height - pDstRect->y;
    }
    return;
}

/* SrcClipRect is the clipped version of pSrcRect , finds pDstClipRect such that proportions between pDstClipRect and pSrcClipRect are the same as between pSrcRect and pDstRect */
void nexus_surfacemp_scale_clipped_rect(const NEXUS_Rect *pSrcRect, const NEXUS_Rect *pSrcClipRect, const NEXUS_Rect *pDstRect, NEXUS_Rect *pDstClipRect)
{
    int inc;
    BDBG_ASSERT((const void *)pDstRect != (const void *)pDstClipRect);

    /* pSrcClipRect must be within pSrcRect */
    BDBG_ASSERT(pSrcClipRect->x >= pSrcRect->x);
    BDBG_ASSERT(pSrcClipRect->x + pSrcClipRect->width <= pSrcRect->x + pSrcRect->width);
    BDBG_ASSERT(pSrcClipRect->y >= pSrcRect->y);
    BDBG_ASSERT(pSrcClipRect->y + pSrcClipRect->height <= pSrcRect->y + pSrcRect->height);

    if (pSrcRect->width) {
        inc = pSrcClipRect->x - pSrcRect->x;
        pDstClipRect->x = pDstRect->x + inc * pDstRect->width / pSrcRect->width;
        pDstClipRect->width = pSrcClipRect->width * pDstRect->width / pSrcRect->width;
    }
    else {
        pDstClipRect->x = pDstClipRect->width = 0;
    }

    if (pSrcRect->height) {
        inc = pSrcClipRect->y - pSrcRect->y;
        pDstClipRect->y = pDstRect->y + inc * pDstRect->height / pSrcRect->height;
        pDstClipRect->height = pSrcClipRect->height * pDstRect->height / pSrcRect->height;
    }
    else {
        pDstClipRect->y = pDstClipRect->height = 0;
    }

    return;
}

NEXUS_Error nexus_surface_compositor_p_verify_tunnel(NEXUS_SurfaceCompositorHandle server)
{
    BDBG_OBJECT_ASSERT(server, NEXUS_SurfaceCompositor);
    /* no-op */
    return NEXUS_SUCCESS;
}

static unsigned nexus_surface_compositor_p_limit_downscale_factor(unsigned src, unsigned dst, unsigned limit, unsigned scale_factor)
{
    unsigned result;
    if(src <= (dst * scale_factor)) { /* no scale limit */
        return dst <= limit ? dst : limit;
    }
    result = dst * scale_factor;
    result = result < dst ? dst : result;
    result = result > limit ? limit : result;
    return result;
}


static NEXUS_P_SurfaceCompositorRenderElement *nexus_surface_compositor_p_process_bounce(NEXUS_SurfaceCompositorHandle server, NEXUS_P_SurfaceCompositorRenderElements *elements, NEXUS_P_SurfaceCompositorRenderElement *data, const NEXUS_Rect *source, const NEXUS_Rect *dest)
{
    unsigned new_width, new_height;
    NEXUS_P_SurfaceCompositorRenderElement *new_data;


    data->sourceRect = *source;
    data->outputRect = *dest;
    data->useBounceBuffer = false;
    if( (server->settings.bounceBuffer.width==0 || server->settings.bounceBuffer.height==0) ||
       (source->width <= (server->bounceBuffer.capabilities.maxHorizontalDownScale * dest->width) &&
        source->height <= (server->bounceBuffer.capabilities.maxVerticalDownScale * dest->height))) {
        return data;
    }
    if(server->bounceBuffer.buffer == NULL) {
        NEXUS_SurfaceCreateSettings createSettings;
        NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
        createSettings.width = server->settings.bounceBuffer.width;
        createSettings.height = server->settings.bounceBuffer.height;
        createSettings.pixelFormat = server->settings.display[0].framebuffer.pixelFormat;
        createSettings.heap = server->settings.bounceBuffer.heap ? server->settings.bounceBuffer.heap : server->settings.display[0].framebuffer.heap;
        BDBG_WRN(("bounce:%p allocationg %ux%u surface", (void *)server, server->settings.bounceBuffer.width, server->settings.bounceBuffer.height));
        server->bounceBuffer.buffer = NEXUS_Surface_Create(&createSettings);
        if(server->bounceBuffer.buffer==NULL) { return data;}
    }

    new_width = nexus_surface_compositor_p_limit_downscale_factor(source->width, dest->width, server->settings.bounceBuffer.width, server->bounceBuffer.capabilities.maxHorizontalDownScale);
    new_height = nexus_surface_compositor_p_limit_downscale_factor(source->height, dest->height, server->settings.bounceBuffer.height, server->bounceBuffer.capabilities.maxVerticalDownScale);

    BDBG_MSG(("bounce:%p %ux%u -> %ux%u -> %ux%u", (void *)server, source->width, source->height, new_width, new_height, dest->width, dest->height));

    data->outputRect.x = 0;
    data->outputRect.y = 0;
    data->outputRect.width = new_width;
    data->outputRect.height = new_height;
    data->colorBlend = NEXUS_SurfaceCompositor_P_ColorCopySource;
    data->alphaBlend = NEXUS_SurfaceCompositor_P_AlphaCopySource;
    data->colorKey.source.enabled = false;
    data->colorKey.dest.enabled = false;
#if 0
    data->horizontalFilter = client->serverSettings.composition.horizontalFilter;
    data->verticalFilter = client->serverSettings.composition.verticalFilter;
#else
    data->horizontalFilter = NEXUS_Graphics2DFilterCoeffs_eAnisotropic;
    data->verticalFilter = NEXUS_Graphics2DFilterCoeffs_eAnisotropic;
#endif
    data->alphaPremultiplySourceEnabled = false;
    data->constantColor = 0xFF000000;
    data->colorMatrixEnabled = false;
    data->useBounceBuffer = true;
    new_data = NEXUS_P_SurfaceCompositorRenderElements_Next(elements);
    if(new_data==NULL) {
        data->useBounceBuffer = false;
        data->outputRect = *dest;
        return data;
    }
    new_data->sourceSurface = server->bounceBuffer.buffer;
    new_data->sourceRect.x = 0;
    new_data->sourceRect.y = 0;
    new_data->sourceRect.width = new_width;
    new_data->sourceRect.height = new_height;
    new_data->outputRect = *dest;
    new_data->useBounceBuffer = false;
    return new_data;
}

static void nexus_surface_compositor_add_render_element(NEXUS_SurfaceCompositorHandle server, NEXUS_P_SurfaceCompositorRenderElements *elements, NEXUS_SurfaceClientHandle client, bool left)
{
    NEXUS_P_SurfaceCompositorRenderElement *data;
    BDBG_MSG_TRACE(("add_render_element:%p [%u] client:%p surface:%p", (void *)server, elements->count, (void *)client, (void *)client->state.current.surface));
    data = NEXUS_P_SurfaceCompositorRenderElements_Next(elements);
    if(data) {
        const NEXUS_Rect *source;
        const NEXUS_Rect *dest;

        if(left) {
            source = &client->state.left.sourceRect;
            dest = &client->state.left.outputRect;
            if(client->state.client_type==client_type_push || client->state.client_type == client_type_tunnel_emulated) {
                NEXUS_SURFACECLIENT_P_SURFACE_VERIFY(client, &client->state.current);
            }
        } else {
            source = &client->state.right.sourceRect;
            dest = &client->state.right.outputRect;
        }
        data->sourceSurface = client->state.current.surface;
        data = nexus_surface_compositor_p_process_bounce(server, elements, data, source, dest);
        data->colorBlend = client->serverSettings.composition.colorBlend;
        data->alphaBlend = client->serverSettings.composition.alphaBlend;
        if (client->serverSettings.composition.colorKey.source.enabled || client->serverSettings.composition.colorKey.dest.enabled) {
            data->colorKey.source = client->serverSettings.composition.colorKey.source;
            data->colorKey.dest = client->serverSettings.composition.colorKey.dest;
        }
        else {
            data->colorKey.source.enabled = false;
            data->colorKey.dest.enabled = false;
        }
        data->horizontalFilter = client->serverSettings.composition.horizontalFilter;
        data->verticalFilter = client->serverSettings.composition.verticalFilter;
        data->alphaPremultiplySourceEnabled = client->serverSettings.composition.alphaPremultiplySourceEnabled;
        data->constantColor = client->serverSettings.composition.constantColor;
        data->colorMatrixEnabled = client->serverSettings.composition.colorMatrixEnabled;
        if (data->colorMatrixEnabled) {
            data->colorMatrix = client->serverSettings.composition.colorMatrix;
        }
    }
    return;
}

/* return time in usec */
#include "priv/nexus_display_priv.h"
unsigned nexus_surface_compositor_p_get_time(NEXUS_SurfaceCompositorHandle server)
{
    unsigned t;
    BKNI_EnterCriticalSection();
    t = NEXUS_Display_GetLastVsyncTime_isr(server->settings.display[0].display);
    BKNI_LeaveCriticalSection();
    return t;
}

unsigned nexus_surface_compositor_p_get_time_diff(NEXUS_SurfaceCompositorHandle server, unsigned future, unsigned past)
{
    BSTD_UNUSED(server);
    if (future >= past) {
        return future - past;
    }
    else {
        return (BTMR_ReadTimerMax() - past) + future;
    }
}

static void nexus_surface_compositor_p_compose_framebuffer(NEXUS_SurfaceCompositorHandle server, struct NEXUS_SurfaceCompositorDisplay *cmpDisplay, struct NEXUS_SurfaceCompositorFramebuffer *framebuffer)
{
    NEXUS_SurfaceClientHandle client;

    BDBG_ASSERT(cmpDisplay->display);

    /* if a surface is full screen and non-opaque, we can skip rendering anything below it, including the background.
    the general principle is that we can skip rendering any surface that is fully concealed. */

    BDBG_MSG_TRACE(("compose_framebuffer:%#lx %#lx", (unsigned long)server, (unsigned long)cmpDisplay));

    framebuffer->generation = cmpDisplay->generation;

    for (client = BLST_Q_FIRST(&server->clients); client; client = BLST_Q_NEXT(client, link)) {
        BDBG_MSG_TRACE(("compose:%#lx client:%#lx", (unsigned long)server, (unsigned long)client));
        BDBG_MSG_TRACE(("compose_framebuffer:%#lx %#lx client:%#lx:%u:%u %s (%d,%d,%u,%u)", (unsigned long)server, (unsigned long)cmpDisplay, (unsigned long)client, client->client_id, client->state.client_type, client->state.left.visible==0?"OFFSCREEN":"", client->state.left.outputRect.x, client->state.left.outputRect.y, client->state.left.outputRect.width, client->state.left.outputRect.height));
        client->process_pending_displayed_callback = client->pending_displayed_callback;

        if(client->state.client_type==client_type_push || client->state.client_type == client_type_tunnel_emulated) {
            client->queue.compositing = true;
        }
        if( client->settings.virtualRefreshRate && !client->queue.firstCompositingValid) {
            client->queue.firstCompositingValid = true;
            client->queue.firstCompositing = nexus_surface_compositor_p_get_time(server);
#if BDBG_DEBUG_BUILD
            client->queue.repeatCount = 0;
#endif
        }

        if(client->set.dirty) {
            /* even if hidden, we must blit */
            nexus_surface_compositor_p_update_dirty_client(client);
        }

        if(client->state.left.hidden && client->state.right.hidden) {
            continue;
        }
        if(client->state.client_type == client_type_tunnel) {
            BDBG_ASSERT(server->tunnel.client == client);
            if( ((client->state.current.surface == framebuffer->tunnel.surface.surface) /* don't composite to itself */ ||
               (framebuffer->tunnel.pristine && !server->tunnel.overlapped) /* already rendered */)) {
                BDBG_MSG_TRACE(("compose:%p reusing framebuffer:%p for tunnel client:%p", (void *)server, (void *)framebuffer, (void *)client));
                server->renderState.tunnel.active = true;
                server->renderState.tunnel.rect = client->state.left.sourceRect;
                    /* do nothing */
            } else {
                BDBG_MSG_TRACE(("compose:%p copying surface:%p to framebuffer:%p for tunnel client:%p ", (void *)server, (void *)client->state.current.surface, (void *)framebuffer, (void *)client));
                client->state.left.outputRect = client->state.framebufferRect;
                switch(client->settings.orientation) {
                default:
                case NEXUS_VideoOrientation_e2D:
                    break;
                case NEXUS_VideoOrientation_e3D_LeftRight:
                    client->state.left.outputRect.width *= 2;
                    break;
                case NEXUS_VideoOrientation_e3D_OverUnder:
                    client->state.left.outputRect.height *= 2;
                    break;
                }
                client->state.left.sourceRect.width = client->state.left.outputRect.width;
                client->state.left.sourceRect.height = client->state.left.outputRect.height;
                client->state.left.sourceRect.x = client->state.left.sourceRect.y = 0;
                nexus_surface_compositor_add_render_element(server, &server->renderState.elements, client, true);
            }
            continue;
        } else if(client->state.update_flags) { /* there were some changes, update current surface and sourceRect */
            NEXUS_Surface_P_ClientSurface surface;
            NEXUS_Rect sourceRect;
            unsigned right_x_offset = 0;
            unsigned right_y_offset = 0;

            BDBG_ASSERT(client->state.left.visible || client->state.right.visible);
            NEXUS_SURFACECLIENT_P_SURFACE_INIT(&surface);

            client->state.update_flags = 0;
            if(client->published.surface) {
                surface.surface = client->published.surface;
            } else {
                switch(client->state.client_type) {
                case client_type_set:
                    BDBG_ASSERT(client->set.serverSurface);
                    surface.surface = client->set.serverSurface;
                    break;
                case client_type_tunnel_emulated:
                case client_type_push:
                    {
                        NEXUS_SurfaceCreateSettings createSettings;
                        const NEXUS_SurfaceCompositor_P_PushElement *first_node = BLST_SQ_FIRST(&client->queue.push);

                        BDBG_ASSERT(first_node); /* for push clients, queue should be unempty, for tunnel_emulated clients, if queue empty then visible is false */
                        surface = first_node->surface;
                        NEXUS_Surface_GetCreateSettings(surface.surface, &createSettings);
                        client->state.clientRegion.width = createSettings.width;
                        client->state.clientRegion.height = createSettings.height;
                    }
                    break;
                case client_type_tunnel:
                default:
                    BDBG_ASSERT(0);
                    surface.surface = NULL;
                    break;
                }
            }
            BDBG_ASSERT(surface.surface);
            client->state.current = surface;
            sourceRect.x = sourceRect.y = 0;
            sourceRect.width = client->state.clientRegion.width;
            sourceRect.height = client->state.clientRegion.height;
            if (client->serverSettings.composition.clipRect.width && client->serverSettings.composition.clipRect.height) {
                NEXUS_Rect clippedSourceRect, clipBase = {0,0,0,0};
                clipBase.width = client->serverSettings.composition.clipBase.width;
                clipBase.height = client->serverSettings.composition.clipBase.height;
                if (!clipBase.width || !clipBase.height) {
                    clipBase.width = client->serverSettings.composition.virtualDisplay.width;
                    clipBase.height = client->serverSettings.composition.virtualDisplay.height;
                }
                nexus_surfacemp_scale_clipped_rect(&clipBase, &client->serverSettings.composition.clipRect, &sourceRect, &clippedSourceRect);
                sourceRect = clippedSourceRect;
            }
            switch(client->settings.orientation) {
            default:
            case NEXUS_VideoOrientation_e2D:
                break;
            case NEXUS_VideoOrientation_e3D_LeftRight:
                if(sourceRect.width%2) {
                    BDBG_WRN(("client:%p:%u unsupported 3D LeftRight width:%u", (void *)client, client->client_id, sourceRect.width));
                }
                sourceRect.width/=2;
                right_x_offset = sourceRect.width;
                break;
            case NEXUS_VideoOrientation_e3D_OverUnder:
                if(sourceRect.height%2) {
                    BDBG_WRN(("client:%p:%u unsupported 3D OverUnder height:%u", (void *)client, client->client_id, sourceRect.height));
                }
                sourceRect.height/=2;
                right_y_offset = sourceRect.height;
                break;
            }
            if(!client->state.left.hidden) {
                nexus_surfacemp_scale_clipped_rect(&client->state.framebufferRect, &client->state.left.outputRect, &sourceRect, &client->state.left.sourceRect);
                BDBG_MSG_TRACE(("%p: client:%p:%u,%u (%d,%d,%u,%u)[left]", (void *)server, (void *)client, client->client_id, client->state.client_type, client->state.left.sourceRect.x, client->state.left.sourceRect.y, client->state.left.sourceRect.width, client->state.left.sourceRect.height));
            }
            if(!client->state.right.hidden && (cmpDisplay->formatInfo.orientation==NEXUS_VideoOrientation_e3D_LeftRight || cmpDisplay->formatInfo.orientation==NEXUS_VideoOrientation_e3D_OverUnder)) {
                NEXUS_Rect outputRect; /* we need to shift outputRect to 0 */
                NEXUS_Rect framebufferRect; /* we need to account for rightViewOffset */
                BDBG_ASSERT(client->state.right.outputRect.x >= cmpDisplay->offset3DRight.x);
                BDBG_ASSERT(client->state.right.outputRect.y >= cmpDisplay->offset3DRight.y);
                outputRect.width = client->state.right.outputRect.width;
                outputRect.height = client->state.right.outputRect.height;
                outputRect.x = client->state.right.outputRect.x - cmpDisplay->offset3DRight.x;
                outputRect.y = client->state.right.outputRect.y - cmpDisplay->offset3DRight.y;
                framebufferRect.width = client->state.framebufferRect.width;
                framebufferRect.height = client->state.framebufferRect.height;
                framebufferRect.y = client->state.framebufferRect.y;
                framebufferRect.x = client->state.framebufferRect.x + client->state.rightViewOffset;
                nexus_surfacemp_scale_clipped_rect(&framebufferRect, &outputRect, &sourceRect, &client->state.right.sourceRect);
                /* and shift client rectangle */
                client->state.right.sourceRect.x += right_x_offset;
                client->state.right.sourceRect.y += right_y_offset;
                BDBG_MSG_TRACE(("%p:client:%p:%u,%u (%d,%d,%u,%u)[right]", (void *)server, (void *)client, client->client_id, client->state.client_type, client->state.left.sourceRect.x, client->state.right.sourceRect.y, client->state.right.sourceRect.width, client->state.right.sourceRect.height));
            }
        }
        BDBG_ASSERT(client->state.current.surface);
        if(!client->state.left.hidden) {
            nexus_surface_compositor_add_render_element(server, &server->renderState.elements, client, true);
        }
        if(!client->state.right.hidden) {
            nexus_surface_compositor_add_render_element(server, &server->renderState.elements, client, false);
        }
    }
    framebuffer->tunnel.pristine = !server->tunnel.overlapped;
    return;
}

/* return true if b is completely within a */
static bool nexus_p_surface_rect_is_bound(const NEXUS_Rect *a, const NEXUS_Rect *b)
{
    return
    (b->x >= a->x) && (b->x + b->width <= a->x + a->width) &&
    (b->y >= a->y) && (b->y + b->height <= a->y + a->height);
}


/* return true if client is opaque, e.g. it would override (not blend) anything that drawn behind it */
static bool nexus_p_surface_client_opaque(NEXUS_SurfaceClientHandle client)
{
    return nexus_p_surface_composition_opaque(&client->serverSettings.composition.colorBlend, &client->serverSettings.composition.alphaBlend);
}

static void nexus_surface_compositor_p_update_hidden_clients(NEXUS_SurfaceCompositorHandle server)
{
    NEXUS_SurfaceClientHandle client;
    const struct NEXUS_SurfaceCompositorDisplay *cmpDisplay = server->display[0];

    if(server->state.update_flags & (NEXUS_P_SURFACECLIENT_UPDATE_DISPLAY | NEXUS_P_SURFACECLIENT_UPDATE_SIZE | NEXUS_P_SURFACECLIENT_UPDATE_POSITION | NEXUS_P_SURFACECLIENT_UPDATE_VISIBLE | NEXUS_P_SURFACECLIENT_UPDATE_ZORDER | NEXUS_P_SURFACECLIENT_UPDATE_CLIENT)) {
        BDBG_MSG_TRACE(("update_hidden_clients:%p looking for hidden clients", (void *)server));
        /* update hidden flag for all clients */
        for (client=BLST_Q_LAST(&server->clients); client; client = BLST_Q_PREV(client, link)) {
            NEXUS_SurfaceClientHandle prev;

            client->state.left.hidden = true;
            client->state.right.hidden = true;
            if(client->state.left.visible) {
                client->state.left.hidden = false;
                /* then look for all clients that are on top and verify that they aren't completely masking current client */
                for(prev=BLST_Q_NEXT(client,link); prev ; prev=BLST_Q_NEXT(prev, link)) {
                    if(prev->state.left.hidden) {
                        continue;
                    }
                    if(nexus_p_surface_rect_is_bound(&prev->state.left.outputRect, &client->state.left.outputRect) && nexus_p_surface_client_opaque(prev)) {
                        BDBG_MSG_TRACE(("compose:%p client %p (%d,%d,%u,%u) hides client %p (%d,%d,%u,%u) left", (void *)server, (void *)prev, prev->state.left.outputRect.x, prev->state.left.outputRect.y, prev->state.left.outputRect.width, prev->state.left.outputRect.height, (void *)client, client->state.left.outputRect.x, client->state.left.outputRect.y, client->state.left.outputRect.width, client->state.left.outputRect.height));
                        client->state.left.hidden = true;
                        break;
                    }
                }
            }
            if(client->state.right.visible && (cmpDisplay && (cmpDisplay->formatInfo.orientation==NEXUS_VideoOrientation_e3D_LeftRight || cmpDisplay->formatInfo.orientation==NEXUS_VideoOrientation_e3D_OverUnder))) {
                client->state.right.hidden = false;
                /* then look for all clients that are on top and verify that they aren't completely masking current client */
                for(prev=BLST_Q_NEXT(client,link); prev ; prev=BLST_Q_NEXT(prev, link)) {
                    if(prev->state.right.hidden) {
                        continue;
                    }
                    if(nexus_p_surface_rect_is_bound(&prev->state.right.outputRect, &client->state.right.outputRect) && nexus_p_surface_client_opaque(prev)) {
                        BDBG_MSG_TRACE(("compose:%p client %p (%d,%d,%u,%u) hides client %p (%d,%d,%u,%u)", (void *)server, (void *)prev, prev->state.right.outputRect.x, prev->state.right.outputRect.y, prev->state.right.outputRect.width, prev->state.right.outputRect.height, (void *)client, client->state.right.outputRect.x, client->state.right.outputRect.y, client->state.right.outputRect.width, client->state.right.outputRect.height));
                        client->state.right.hidden = true;
                        break;
                    }
                }
            }
        }
        if(server->tunnel.client) {
            /* find first rendered client that is not tunnel */
            for (client=BLST_Q_LAST(&server->clients); client; client = BLST_Q_PREV(client, link)) {
                if(client == server->tunnel.client) {
                    continue;
                }
                if( !client->state.right.hidden || !client->state.left.hidden) {
                    break;
                }
            }
            server->tunnel.overlapped = client != NULL;
            if(server->tunnel.overlapped) {
                BDBG_MSG_TRACE(("update_hidden_clients:%p tunnel client:%p(%u) overlapped by client:%p(%u)", (void *)server, (void *)server->tunnel.client, server->tunnel.client->client_id, (void *)client, client->client_id));
            }
        }
    }
    return;
}

static int nexus_surface_compositor_p_check_bypass(NEXUS_SurfaceCompositorHandle server)
{
    int rc = 0;
    NEXUS_SurfaceClientHandle bypassClient = NULL;
    if (server->settings.allowCompositionBypass) {
        NEXUS_SurfaceClientHandle client = BLST_Q_LAST(&server->clients);
        /* must be single client in push mode */
        if (client && client->settings.allowCompositionBypass && client->state.client_type == client_type_push) {
            NEXUS_SurfaceCompositor_P_PushElement *e = BLST_SQ_FIRST(&client->queue.push);
            if (!e) goto done;

            if (server->bypass_compose.client && server->bypass_compose.client != client) {
                BDBG_ERR(("bypass client overlap %p %p", (void *)server->bypass_compose.client, (void *)client));
                goto done;
            }

            /* yes, we can bypass composition and send this surface directly to GFD0 */
            bypassClient = client;

            /* if we've already rendered, then there must be a second surface before going again */
            if (server->bypass_compose.set && !BLST_SQ_NEXT(e, link)) {
                return 1; /* no BERR_TRACE */
            }
        }
    }
done:
    if (server->bypass_compose.client != bypassClient) {
        BDBG_WRN(("switching %s bypass composition mode for SurfaceClient %p", bypassClient?"into":"out of", (void *)bypassClient));
        server->bypass_compose.client = bypassClient;
        if (!bypassClient) {
            rc = nexus_surface_compositor_p_realloc_framebuffers(server);
            if (rc) return BERR_TRACE(rc);
        }
    }
    return rc;
}

/* compose all framebuffers */
void nexus_surface_compositor_p_compose(NEXUS_SurfaceCompositorHandle server)
{
    struct NEXUS_SurfaceCompositorDisplay *cmpDisplay;

    BDBG_OBJECT_ASSERT(server, NEXUS_SurfaceCompositor);
    cmpDisplay = server->display[0];

    if (!server->settings.enabled) {
        /* short circuit composition triggered by client activity */
        return;
    }

    BDBG_MSG_TRACE(("compose:%#lx", (unsigned long)server));
    if(cmpDisplay==NULL || cmpDisplay->display==NULL || server->settings.display[0].display==NULL) {
        return;
    }
    if(cmpDisplay->num_framebuffers==1) { /* if there is a single framebuffer, then rendering to currently displayed buffer is allowed */
        nexus_surface_compositor_p_recycle_displayed_framebuffer(cmpDisplay);
    }

    if (nexus_surface_compositor_p_check_bypass(server)) {
        return;
    }

    /* 1. Check if all steps are completed */
    if (cmpDisplay->compositing || cmpDisplay->composited || cmpDisplay->submitted) {
        /* framebuffer callback is pending, so we can update then */
        BDBG_MSG_TRACE(("compose:%#lx -> compositing:%#lx composited:%#lx submitted:%#lx", (unsigned long)server, (unsigned long)cmpDisplay->compositing, (unsigned long)cmpDisplay->composited, (unsigned long)cmpDisplay->submitted));
        server->pending_update = true;
        return;
    }
    if(!nexus_surface_compositor_p_blitter_acquire(server, NEXUS_P_SURFACEBLITTER_COMPOSITOR)) {
        BDBG_MSG_TRACE(("compose:%p blitter busy", (void *)server));
        return;
    }

    server->pending_update = false;

    nexus_surface_compositor_p_update_video(server);

    if(server->state.update_flags) {
        nexus_surface_compositor_p_update_hidden_clients(server);
        server->state.update_flags = 0;
    }

    server->renderState.tunnel.rect.width = 0;
    server->renderState.tunnel.rect.height = 0;
    server->renderState.tunnel.active = false;
    server->renderState.tunnel.tunnelSource = NULL;

    /* 2.  select framebuffer to draw */
    {
        struct NEXUS_SurfaceCompositorFramebuffer *framebuffer;
        bool tunnelActive = server->tunnel.client && !server->tunnel.client->state.left.hidden;
        struct NEXUS_SurfaceCompositorFramebuffer *tunnelSource = NULL;

        tunnelSource = framebuffer = BLST_Q_FIRST(&cmpDisplay->tunnel.submitted);
        if(tunnelActive && framebuffer && !server->tunnel.overlapped) {
            /* render into client provided framebuffer */
            BDBG_MSG_TRACE(("compose:%#lx rendering into tunnel framebuffer:%#lx", (unsigned long)server, (unsigned long)framebuffer));
            BLST_Q_REMOVE_HEAD(&cmpDisplay->tunnel.submitted, tunnelLink); /* would be added  into available by nexus_surface_compositor_p_recycle_displayed_framebuffer */
        } else {
            framebuffer = BLST_SQ_FIRST(&cmpDisplay->available);
            if(framebuffer) {
                BDBG_MSG_TRACE(("compose:%p rendering into framebuffer:%p", (void *)server, (void *)framebuffer));
                BLST_SQ_REMOVE_HEAD(&cmpDisplay->available, link);
                if(tunnelActive) {
                    if(tunnelSource ) {
                        BLST_Q_REMOVE_HEAD(&cmpDisplay->tunnel.submitted, tunnelLink); /* would be added  into available by nexus_surface_compositor_compositing_completed */
                        server->renderState.tunnel.tunnelSource = tunnelSource;
                    } else {
                        if(tunnelActive && !server->tunnel.overlapped) { /* if no new tunnel buffer, just skip rendering */
                            BDBG_MSG_TRACE(("compose:%p skip rendering due to idle tunnel client %p", (void *)server, (void *)server->tunnel.client));
                            BLST_SQ_INSERT_HEAD(&cmpDisplay->available, framebuffer, link); /* push framebuffer back */
                            framebuffer = NULL;
                            goto framebuffer_done;
                        }
                        /* look in currently displayed framebuffer */
                        if(cmpDisplay->displaying && cmpDisplay->displaying->tunnel.pristine) {
                            tunnelSource = cmpDisplay->displaying;
                        } else {
                            /* and then in list of available surfaces */
                            for(tunnelSource=BLST_Q_FIRST(&cmpDisplay->tunnel.available);tunnelSource;tunnelSource=BLST_Q_NEXT(tunnelSource, tunnelLink)) {
                                if(tunnelSource->tunnel.pristine) {
                                    break;
                                }
                            }
                        }
                    }
                    BDBG_MSG_TRACE(("compose:%p rendering into framebuffer:%p from tunnel:%p", (void *)server, (void *)framebuffer, (void *)tunnelSource));
                }
            }
        }
framebuffer_done:
        if(framebuffer) {
            cmpDisplay->compositing = framebuffer;
            framebuffer->state = NEXUS_SurfaceCompositorFramebufferState_eCompositing;
        } else {
            /* undo */
            cmpDisplay->compositing = NULL;
            server->pending_update = true;
            nexus_surface_compositor_p_blitter_release(server, NEXUS_P_SURFACEBLITTER_COMPOSITOR);
            return;
        }
        if(server->tunnel.client) {
            BDBG_MSG_TRACE(("compose:%p: tunnelSource:%p(%p,%p) dest:%p", (void *)server, (void *)tunnelSource, (void *)BLST_Q_FIRST(&cmpDisplay->tunnel.submitted), (void *)server->renderState.tunnel.tunnelSource, (void *)framebuffer));
            NEXUS_SURFACECLIENT_P_SURFACE_INIT(&server->tunnel.client->state.current);
            if(tunnelSource) {
                NEXUS_SURFACECLIENT_P_SURFACE_VERIFY(server->tunnel.client, &tunnelSource->tunnel.surface);
                server->tunnel.client->state.current.surface = tunnelSource->tunnel.surface.surface;
            } else {
                server->tunnel.client->state.current.surface = NULL;
            }
        }
   }

    /* 3. Composite master display */
    BTRC_TRACE(surface_compositor_composite,START);
    {
        struct NEXUS_SurfaceCompositorFramebuffer *framebuffer = cmpDisplay->compositing;
        BDBG_ASSERT(framebuffer);
        framebuffer->state = NEXUS_SurfaceCompositorFramebufferState_eCompositing;
        server->renderState.elements.count = 0;
        server->renderState.step = taskInit;
        nexus_surface_compositor_p_compose_framebuffer(server, cmpDisplay, framebuffer);
        cmpDisplay->generation++;
        /* 4. render framebuffer for master display */
        nexus_surface_compositor_p_render_framebuffer(server, cmpDisplay, framebuffer);
    }
    BTRC_TRACE(surface_compositor_composite,STOP);
    return;
}


static void nexus_p_surface_compositor_update(NEXUS_SurfaceCompositorHandle server, unsigned flags)
{
    server->state.update_flags |= flags;
    nexus_surface_compositor_p_compose(server);
    return;
}

static void nexus_p_surface_compositor_get_display_canvas(const struct NEXUS_SurfaceCompositorDisplay *display, NEXUS_SurfaceRegion *canvas)
{
    if(display) {
        canvas->width = display->canvas.width;
        canvas->height = display->canvas.height;
    } else {
        canvas->width = 720;
        canvas->height = 480;
    }
    return;
}

void nexus_p_surface_compositor_update_virtual_display(const struct NEXUS_SurfaceCompositorDisplay *display, const NEXUS_SurfaceComposition *composition, NEXUS_SurfaceRegion *virtualDisplay)
{
    NEXUS_SurfaceRegion canvas;
    nexus_p_surface_compositor_get_display_canvas(display, &canvas);

    virtualDisplay->width = composition->virtualDisplay.width ? composition->virtualDisplay.width: canvas.width;
    virtualDisplay->height = composition->virtualDisplay.height ? composition->virtualDisplay.height : canvas.height;
    return;
}

static void nexus_surface_compositor_p_copy_palette(NEXUS_SurfaceHandle dst, NEXUS_SurfaceHandle src)
{
    NEXUS_SurfaceStatus status;
    NEXUS_Surface_GetStatus( src, &status );
    if (status.numPaletteEntries) {
        unsigned i;
        bool modified = false;
        uint32_t *srcPalette, *dstPalette;
        void *temp;
        int rc;
        NEXUS_SurfaceStatus dstStatus;

        NEXUS_Surface_GetStatus( dst, &dstStatus );
        if (status.numPaletteEntries != dstStatus.numPaletteEntries) {
            BERR_TRACE(NEXUS_UNKNOWN);
            return;
        }

        rc = NEXUS_Surface_LockPalette(src, &temp);
        if (rc) {BERR_TRACE(rc);return;}
        srcPalette = temp;
        rc = NEXUS_Surface_LockPalette(dst, &temp);
        if (rc) {BERR_TRACE(rc);NEXUS_Surface_UnlockPalette(src);return;}
        dstPalette = temp;

        for (i = 0; i < status.numPaletteEntries; i++) {
            if (dstPalette[i] != srcPalette[i]) {
                dstPalette[i] = srcPalette[i];
                modified = true;
            }
        }
        if ( modified ) {
            NEXUS_Surface_Flush( dst );
        }
        NEXUS_Surface_UnlockPalette(src);
        NEXUS_Surface_UnlockPalette(dst);
    }
}

static void nexus_surface_compositor_p_update_dirty_client(NEXUS_SurfaceClientHandle client)
{
    NEXUS_Error rc;
    NEXUS_Graphics2DBlitSettings *pBlitSettings = &client->server->renderState.blitSettings;
    BDBG_ASSERT(client->state.client_type == client_type_set);
    BDBG_ASSERT(client->set.dirty);
    BDBG_ASSERT(client->set.surface.surface);
    BDBG_ASSERT(client->set.serverSurface);
    BDBG_ASSERT(!client->set.updating); /* can't have another transaction in flight */

    nexus_surface_compositor_p_copy_palette(client->set.serverSurface, client->set.surface.surface);

    NEXUS_Graphics2D_GetDefaultBlitSettings(pBlitSettings);
    pBlitSettings->source.surface = client->set.surface.surface;
    pBlitSettings->output.surface = client->set.serverSurface;
    pBlitSettings->source.rect = client->set.dirtyRect;
    pBlitSettings->output.rect = client->set.dirtyRect;
    NEXUS_SURFACECLIENT_P_SURFACE_VERIFY(client, &client->set.surface);
    rc = NEXUS_Graphics2D_Blit(client->server->gfx, pBlitSettings);
    BSTD_UNUSED(rc);
    /* TODO: if (rc == NEXUS_GRAPHICS2D_QUEUE_FULL) {
        cannot block for packetSpaceAvailable.
        must keep track of which blits are completed.
        in the packetSpaceAvailable callback, resume work.
        }
   */
    client->set.updating = true;
    return;
}

void nexus_surface_compositor_p_update_dirty_clients(NEXUS_SurfaceCompositorHandle server, NEXUS_SurfaceClientHandle client)
{
    if(!nexus_surface_compositor_p_blitter_acquire(server, NEXUS_P_SURFACEBLITTER_CLIENT)) {
        return;
    }

    if(client!=NULL) {
        /* update this client */
        nexus_surface_compositor_p_update_dirty_client(client);
    } else {
        /* update all clients */
        for (client = BLST_Q_FIRST(&server->clients); client; client = BLST_Q_NEXT(client, link)) {
            if(client->state.client_type == client_type_set && client->set.dirty) {
                nexus_surface_compositor_p_update_dirty_client(client);
            }
        }
    }
    nexus_surface_compositor_p_blitter_start(server, NEXUS_P_SURFACEBLITTER_CLIENT);
    return;
}

void nexus_surface_compositor_p_update_dirty_clients_done(NEXUS_SurfaceCompositorHandle server)
{
    NEXUS_SurfaceClientHandle client;
    for (client = BLST_Q_FIRST(&server->clients); client; client = BLST_Q_NEXT(client, link)) {
        if(client->state.client_type == client_type_set && client->set.updating) {
            BDBG_ASSERT(client->set.dirty); /* dirty should be set */
            client->set.dirty = false;
            client->set.updating = false;
            NEXUS_TaskCallback_Fire(client->recycledCallback);
        }
    }
}

static void nexus_p_surface_compositor_update_single_client(NEXUS_SurfaceClientHandle client, unsigned flags)
{
    NEXUS_SurfaceCompositorHandle server = client->server;

    BDBG_MSG_TRACE(("update_client:%#lx:%u:%u %s%s%s%s%s%s%s%s",  (unsigned long)client, client->client_id, client->state.client_type, flags&NEXUS_P_SURFACECLIENT_UPDATE_ZORDER?"zorder ":"", flags&NEXUS_P_SURFACECLIENT_UPDATE_SIZE?"size ":"", flags&NEXUS_P_SURFACECLIENT_UPDATE_POSITION?"position ":"", flags&NEXUS_P_SURFACECLIENT_UPDATE_VISIBLE?"visible ":"", flags&NEXUS_P_SURFACECLIENT_UPDATE_SOURCE?"source ":"", flags&NEXUS_P_SURFACECLIENT_UPDATE_COMPOSITION?"composition ":"", flags&NEXUS_P_SURFACECLIENT_UPDATE_DISPLAY?"display ":"", flags&NEXUS_P_SURFACECLIENT_UPDATE_PUBLISH?"publish":""));
    client->state.update_flags |= flags;
    if(flags & (NEXUS_P_SURFACECLIENT_UPDATE_DISPLAY | NEXUS_P_SURFACECLIENT_UPDATE_CLIENT)) {
        nexus_p_surface_compositor_update_virtual_display(server->display[0], &client->serverSettings.composition, &client->state.virtualDisplay);
    }

    if(flags & (NEXUS_P_SURFACECLIENT_UPDATE_SIZE | NEXUS_P_SURFACECLIENT_UPDATE_DISPLAY | NEXUS_P_SURFACECLIENT_UPDATE_CLIENT)) {
    /* update client cache */
    }

    if(flags & (NEXUS_P_SURFACECLIENT_UPDATE_DISPLAY | NEXUS_P_SURFACECLIENT_UPDATE_SIZE | NEXUS_P_SURFACECLIENT_UPDATE_POSITION | NEXUS_P_SURFACECLIENT_UPDATE_CLIENT)) {
        NEXUS_Rect framebuffer;
        const struct NEXUS_SurfaceCompositorDisplay *mainDisplay = server->display[0];
        BDBG_ASSERT(mainDisplay);

        framebuffer.x = framebuffer.y = 0;
        framebuffer.width = mainDisplay->canvas.width;
        framebuffer.height = mainDisplay->canvas.height;
        nexus_surfacemp_p_convert_coord(&client->state.virtualDisplay, &mainDisplay->canvas, &client->serverSettings.composition.position, &client->state.framebufferRect);
        nexus_surfacemp_p_clip_rect(&framebuffer, &client->state.framebufferRect, &client->state.left.outputRect);
        BDBG_MSG_TRACE(("update_client:%#lx:%u:%u (%u,%u) (%d,%d,%u,%u) (%d,%d,%u,%u) left", (unsigned long)client, client->client_id, client->state.client_type, client->state.virtualDisplay.width, client->state.virtualDisplay.height, client->serverSettings.composition.position.x, client->serverSettings.composition.position.y, client->serverSettings.composition.position.width, client->serverSettings.composition.position.height, client->state.left.outputRect.x, client->state.left.outputRect.y, client->state.left.outputRect.width, client->state.left.outputRect.height));
        if(mainDisplay->formatInfo.orientation==NEXUS_VideoOrientation_e3D_LeftRight || mainDisplay->formatInfo.orientation==NEXUS_VideoOrientation_e3D_OverUnder) {
            NEXUS_Rect framebufferRect; /* we need to account for rightViewOffset */
            client->state.rightViewOffset = (client->serverSettings.composition.rightViewOffset * client->state.virtualDisplay.width) /  mainDisplay->canvas.width;
            /* moove framebufferRect to according to the rightViewOffset */
            framebufferRect.width = client->state.framebufferRect.width;
            framebufferRect.height = client->state.framebufferRect.height;
            framebufferRect.y = client->state.framebufferRect.y;
            framebufferRect.x = client->state.framebufferRect.x + client->state.rightViewOffset;
            /* clip it to the framebufferRect */
            nexus_surfacemp_p_clip_rect(&framebuffer, &framebufferRect, &client->state.right.outputRect);
            client->state.right.outputRect.x += mainDisplay->offset3DRight.x;
            client->state.right.outputRect.y += mainDisplay->offset3DRight.y;
            BDBG_MSG_TRACE(("update_client:%#lx:%u:%u (%u,%u) (%d,%d,%u,%u) (%d/%d) (%d,%d,%u,%u) right", (unsigned long)client, client->client_id, client->state.client_type, client->state.virtualDisplay.width, client->state.virtualDisplay.height, client->serverSettings.composition.position.x, client->serverSettings.composition.position.y, client->serverSettings.composition.position.width, client->serverSettings.composition.position.height, client->serverSettings.composition.rightViewOffset, client->state.rightViewOffset, client->state.right.outputRect.x, client->state.left.outputRect.y, client->state.right.outputRect.width, client->state.right.outputRect.height));
        } else {
            client->state.right.outputRect.width = client->state.right.outputRect.height = 0;
        }
    }

    if(flags & (NEXUS_P_SURFACECLIENT_UPDATE_DISPLAY | NEXUS_P_SURFACECLIENT_UPDATE_SIZE | NEXUS_P_SURFACECLIENT_UPDATE_POSITION | NEXUS_P_SURFACECLIENT_UPDATE_VISIBLE | NEXUS_P_SURFACECLIENT_UPDATE_ZORDER | NEXUS_P_SURFACECLIENT_UPDATE_CLIENT)) {
        NEXUS_SurfaceClientHandle child;

        /* verify if window has to be repositioned */
        for (child = BLST_S_FIRST(&client->children); child; child = BLST_S_NEXT(child, child_link)) {
            if (child->type == NEXUS_SurfaceClient_eVideoWindow) {
                nexus_surfaceclient_request_setvideo(child);
            }
        }

        if(!client->serverSettings.composition.visible || client->state.client_type == client_type_idle ||
                ((client->state.client_type == client_type_tunnel || client->state.client_type == client_type_tunnel_emulated)&& !client->tunnel.visible) ) {
            client->state.left.visible = false;
            client->state.right.visible = false;
        } else {
            client->state.left.visible = client->state.left.outputRect.width != 0 && client->state.left.outputRect.height != 0;
            client->state.right.visible = client->state.right.outputRect.width != 0 && client->state.right.outputRect.height != 0;
        }
        BDBG_MSG_TRACE(("visible:%p:%u %s %s", (void *)client, client->client_id, client->state.left.visible?"left":"", client->state.right.visible?"right":""));
        client->state.current.surface = NULL;
    }

    if(flags & NEXUS_P_SURFACECLIENT_UPDATE_SOURCE) {
        BDBG_ASSERT(!(flags & NEXUS_P_SURFACECLIENT_UPDATE_PUBLISH)); /* SOURCE and PUBLISH can't be set simultaneously */
        client->published.surface = NULL;
        client->pending_displayed_callback = true;
        switch(client->state.client_type) {
        case client_type_set:
            client->set.dirty = true;
            BDBG_ASSERT(!client->set.updating);
            client->set.dirtyRect = client->update_info.updateRect;
            nexus_surface_compositor_p_update_dirty_clients(client->server, client);
            break;
        case client_type_push:
        case client_type_tunnel:
        case client_type_tunnel_emulated:
        default:
            break;
        }
    }
    switch(client->state.client_type) {
    case client_type_push:
            BDBG_ASSERT(BLST_SQ_FIRST(&client->queue.push));
            break;
    case client_type_tunnel_emulated:
        if(client->state.left.visible || client->state.right.visible) {
            BDBG_ASSERT(BLST_SQ_FIRST(&client->queue.push));
        }
        break;
    default:
        break;
    }
    return;
}

void nexus_p_surface_compositor_update_client(NEXUS_SurfaceClientHandle client, unsigned flags)
{
    if(flags) {
        nexus_p_surface_compositor_update_single_client(client, flags);
        nexus_p_surface_compositor_update(client->server, flags);
        if(client->state.client_type == client_type_tunnel) {
            /* needed in order to set server->tunnel.overlapped which in turn used to allocate tunnel surfaces */
            nexus_surface_compositor_p_update_hidden_clients(client->server);
        }
    }
    return;
}

static void nexus_p_surface_compositor_update_all_clients(NEXUS_SurfaceCompositorHandle server, unsigned flags)
{
    NEXUS_SurfaceClientHandle client;
    for (client = BLST_Q_FIRST(&server->clients); client; client = BLST_Q_NEXT(client, link)) {
        nexus_p_surface_compositor_update_single_client(client, flags);
    }
    nexus_p_surface_compositor_update(server, flags);
    return;
}

NEXUS_Error NEXUS_SurfaceCompositor_GetStatus( NEXUS_SurfaceCompositorHandle server, NEXUS_SurfaceCompositorStatus *pStatus )
{
    BDBG_OBJECT_ASSERT(server, NEXUS_SurfaceCompositor);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    pStatus->active = server->state.active;

    if (server->tunnel.client) {
        unsigned i;
        for(i=0;i<sizeof(server->tunnel.client->tunnel.surfaces)/sizeof(server->tunnel.client->tunnel.surfaces[0]);i++) {
            if(server->tunnel.client->tunnel.surfaces[i].surface) {
                pStatus->numAcquiredTunneledSurfaces++;
            }
        }
    }

    return 0;
}

NEXUS_Error NEXUS_SurfaceCompositor_GetCurrentFramebuffer( NEXUS_SurfaceCompositorHandle server, NEXUS_SurfaceHandle *pSurface )
{
    if (server->display[0]) {
        unsigned i;
        for (i=0;i<server->display[0]->num_framebuffers;i++) {
            if ((&server->display[0]->framebuffer[i] == server->display[0]->displaying)) {
                *pSurface = server->display[0]->framebuffer[i].surface;
                return NEXUS_SUCCESS;
            }
        }
    }
    return BERR_TRACE(NEXUS_NOT_AVAILABLE);
}

void nexus_p_surface_compositor_check_inactive(NEXUS_SurfaceCompositorHandle server)
{
    unsigned i;
    NEXUS_Error rc;
    NEXUS_SurfaceCompositorStatus status;

    BDBG_ASSERT(server->state.active); /* if inactive, the condition should never arise to call this */
    BDBG_ASSERT(!server->settings.enabled); /* if enabled, it should not be called */

    for (i=0;i<NEXUS_MAX_DISPLAYS;i++) {
        if(server->display[i]==NULL) {
            continue;
        }
        BDBG_MSG(("check_inactive[%d]: %p %p", i, (void *)server->display[i]->compositing, (void *)server->display[i]->submitted));
        if (server->display[i]->compositing || server->display[i]->submitted) return;
    }

    rc = NEXUS_SurfaceCompositor_GetStatus(server, &status);
    if (rc || status.numAcquiredTunneledSurfaces) return;

    /* nothing is pending. we are inactive. */
    BDBG_MSG(("inactive"));
    nexus_surface_compositor_p_release_surfaces(server);
    /* last fb callbacks received, so disable displays */
    for (i=0;i<NEXUS_MAX_DISPLAYS;i++) {
        struct NEXUS_SurfaceCompositorDisplay *cmpDisplay = server->display[i];
        if (cmpDisplay && cmpDisplay->display) {
            nexus_surfacemp_p_disable_display(server, cmpDisplay);
        }
    }

    server->state.active = false;

    /* only allow clients to continue blitting */
    server->state.blitter.active &= NEXUS_P_SURFACEBLITTER_CLIENT;
    server->state.blitter.delayed &= NEXUS_P_SURFACEBLITTER_CLIENT;

    NEXUS_TaskCallback_Fire(server->inactiveCallback);
    return;
}

NEXUS_Error NEXUS_SurfaceCompositorModule_Standby_priv(bool enabled, const NEXUS_StandbySettings *pSettings)
{
#if NEXUS_POWER_MANAGEMENT
    NEXUS_SurfaceCompositorHandle server;
    NEXUS_SurfaceCompositorSettings *surface_compositor_settings=NULL;
    NEXUS_Error rc = NEXUS_SUCCESS;

    BSTD_UNUSED(pSettings);

    surface_compositor_settings = BKNI_Malloc(sizeof(NEXUS_SurfaceCompositorSettings));
    if(surface_compositor_settings==NULL) {
        rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); return rc;
    }

    for (server = BLST_S_FIRST(&g_nexus_surfacecompositor_list); server; server = BLST_S_NEXT(server, link)) {
        NEXUS_SurfaceCompositor_GetSettings(server, surface_compositor_settings);
        if(enabled) {
            if(surface_compositor_settings->enabled) {
                BDBG_MSG(("Disabling Surface Compositor %p", (void *)server));
                surface_compositor_settings->enabled = false;
                rc = NEXUS_SurfaceCompositor_SetSettings(server, surface_compositor_settings);
                if(rc) {
                    BERR_TRACE(NEXUS_NOT_AVAILABLE); goto done;
                }
                server->state.standbyDisabled = true;
            }
        } else {
            if(server->state.standbyDisabled) {
                BDBG_MSG(("Enabling Surface Compositor %p", (void *)server));
                surface_compositor_settings->enabled = true;
                rc = NEXUS_SurfaceCompositor_SetSettings(server, surface_compositor_settings);
                if(rc) {
                    BERR_TRACE(NEXUS_NOT_AVAILABLE); goto done;
                }
            }
        }
    }

done:
    if(surface_compositor_settings) {
        BKNI_Free(surface_compositor_settings);
    }

    return rc;
#else
    BSTD_UNUSED(enabled);
    BSTD_UNUSED(pSettings);
    return NEXUS_SUCCESS;
#endif
}
