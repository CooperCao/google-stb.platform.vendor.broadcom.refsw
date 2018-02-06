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
#include "priv/nexus_display_priv.h"
#include "priv/nexus_video_window_priv.h"

BDBG_MODULE(nexus_surface_client_video);

static void nexus_surfaceclient_p_resetvideo( NEXUS_SurfaceClientHandle client );

NEXUS_SurfaceClientHandle NEXUS_SurfaceClient_AcquireVideoWindow( NEXUS_SurfaceClientHandle parent_handle, unsigned window_id )
{
    NEXUS_SurfaceClientHandle client;

    BDBG_OBJECT_ASSERT(parent_handle, NEXUS_SurfaceClient);
    for (client = BLST_S_FIRST(&parent_handle->children);client;client=BLST_S_NEXT(client,child_link)) {
        if (client->client_id == window_id) {
            BDBG_WRN(("video window %d already used", window_id));
            return NULL;
        }
    }

    client = BKNI_Malloc(sizeof(*client));
    if (!client) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_SurfaceClient, client);
    client->server = parent_handle->server;
    client->type = NEXUS_SurfaceClient_eVideoWindow;
    client->client_id = window_id;
    client->parent = parent_handle;
    nexus_p_surface_composition_init(&client->settings.composition);
    NEXUS_OBJECT_REGISTER(NEXUS_SurfaceClient, client, Create);

    client->acquired = true;
    client->parent = parent_handle;
    nexus_surfacecmp_p_insert_child(client->parent, client, false);
    NEXUS_OBJECT_ACQUIRE(client, NEXUS_SurfaceClient, client->parent);
    nexus_surfaceclient_p_setwindows(client);

    nexus_p_surface_compositor_update_client(client->parent, NEXUS_P_SURFACECLIENT_UPDATE_CLIENT);

    return client;
}

void NEXUS_SurfaceClient_ReleaseVideoWindow( NEXUS_SurfaceClientHandle client )
{
    NEXUS_SurfaceCompositor_DestroyClient(client);
    return;
}

void NEXUS_SurfaceClient_P_VideoWindowFinalizer( NEXUS_SurfaceClientHandle client )
{
    NEXUS_OBJECT_ASSERT(NEXUS_SurfaceClient, client);
    NEXUS_OBJECT_ASSERT(NEXUS_SurfaceClient, client->parent);
    BDBG_ASSERT(client->type == NEXUS_SurfaceClient_eVideoWindow);
    client->acquired = false;
    nexus_surfaceclient_p_resetvideo(client);
    BLST_S_REMOVE(&client->parent->children, client, NEXUS_SurfaceClient, child_link);
    NEXUS_OBJECT_RELEASE(client, NEXUS_SurfaceClient, client->parent);
    client->parent = NULL;

    NEXUS_OBJECT_DESTROY(NEXUS_SurfaceClient, client);
    BKNI_Free(client);
}

NEXUS_Error NEXUS_SurfaceCompositor_SwapWindows(NEXUS_SurfaceCompositorHandle handle,
        NEXUS_SurfaceClientHandle client0, unsigned windowId0,
        NEXUS_SurfaceClientHandle client1, unsigned windowId1)
{
    unsigned i;
    NEXUS_SurfaceClientHandle child0, child1;
    BSTD_UNUSED(handle);
    for (child0 = BLST_S_FIRST(&client0->children); child0; child0 = BLST_S_NEXT(child0, child_link)) {
        if (child0->client_id == windowId0) break;
    }
    if (!child0) return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    for (child1 = BLST_S_FIRST(&client1->children); child1; child1 = BLST_S_NEXT(child1, child_link)) {
        if (child1->client_id == windowId1) break;
    }
    if (!child1) return BERR_TRACE(NEXUS_INVALID_PARAMETER);

    for (i=0;i<NEXUS_MAX_DISPLAYS;i++) {
        NEXUS_VideoWindowHandle temp;
        temp = client0->serverSettings.display[i].window[windowId0].window;
        client0->serverSettings.display[i].window[windowId0].window = client1->serverSettings.display[i].window[windowId1].window;
        client1->serverSettings.display[i].window[windowId1].window = temp;
    }

    /* remove and re-insert for correct zorder */
    if (client0 == client1) {
        NEXUS_SurfaceClientSettings temp = child0->settings;
        child0->settings = child1->settings;
        child1->settings = temp;
        nexus_surfacecmp_p_insert_child(client0, child0, true);
    }
    else {
        NEXUS_SurfaceComposition temp = client0->serverSettings.composition;
        client0->serverSettings.composition = client1->serverSettings.composition;
        client1->serverSettings.composition = temp;
        nexus_surfacecmp_p_insert_client(client0->server, client0, true);
    }

    return NEXUS_SUCCESS;
}

/* copy window handles to VideoWindow client for fast access */
void nexus_surfaceclient_p_setwindows(NEXUS_SurfaceClientHandle client)
{
    NEXUS_OBJECT_ASSERT(NEXUS_SurfaceClient, client);
    if (!client->parent) {
        NEXUS_SurfaceClientHandle child;
        for (child = BLST_S_FIRST(&client->children); child; child = BLST_S_NEXT(child, child_link)) {
            if (child->type == NEXUS_SurfaceClient_eVideoWindow) {
                nexus_surfaceclient_p_setwindows(child);
            }
        }
    }
    else {
        unsigned i, display;
        NEXUS_OBJECT_ASSERT(NEXUS_SurfaceClient, client->parent);
        BKNI_Memset(client->window, 0, sizeof(client->window));
        for (display=0;display<NEXUS_MAX_DISPLAYS;display++) {
            for (i=0;i<NEXUS_SURFACE_COMPOSITOR_VIDEO_WINDOWS;i++) {
                if (client->parent->serverSettings.display[display].window[i].window && i == client->client_id) {
                    client->window[display] = client->parent->serverSettings.display[display].window[i].window;
                    break;
                }
            }
        }
    }
}

static void nexus_surfaceclient_p_resetvideo( NEXUS_SurfaceClientHandle client )
{
    unsigned i;
    NEXUS_DisplayUpdateMode updateMode;

    /* if no video window, nothing to do */
    if (!client->window[0]) return;

    if (!g_NEXUS_SurfaceCompositorModuleSettings.modules.display) return;
    NEXUS_Module_Lock(g_NEXUS_SurfaceCompositorModuleSettings.modules.display);
    NEXUS_DisplayModule_SetUpdateMode_priv(NEXUS_DisplayUpdateMode_eManual, &updateMode);

    for (i=0;i<NEXUS_MAX_DISPLAYS;i++) {
        NEXUS_VideoWindowSettings *pWindowSettings;
        NEXUS_VideoWindowHandle window;
        int rc;
        const struct NEXUS_SurfaceCompositorDisplay *cmpDisplay = client->server->display[i];

        if(cmpDisplay==NULL) {
            continue;
        }
        window = client->window[i];
        if (!window) continue;

        pWindowSettings = &client->server->windowSettings;
        NEXUS_VideoWindow_GetSettings_priv(window, pWindowSettings);
        /* don't reset geometry, may hit RTS limits */
        pWindowSettings->visible = false;
        pWindowSettings->clipRect.x =
        pWindowSettings->clipRect.y =
        pWindowSettings->clipRect.width =
        pWindowSettings->clipRect.height = 0;
        pWindowSettings->clipBase = pWindowSettings->clipRect;
        pWindowSettings->contentMode = NEXUS_VideoWindowContentMode_eBox;
        rc = NEXUS_VideoWindow_SetSettings_priv(window, pWindowSettings);
        if (rc) {rc = BERR_TRACE(rc); goto done;}
    }
done:
    NEXUS_DisplayModule_SetUpdateMode_priv(NEXUS_DisplayUpdateMode_eAuto, NULL);
    NEXUS_Module_Unlock(g_NEXUS_SurfaceCompositorModuleSettings.modules.display);
}

static void nexus_surfaceclient_p_combine_clipping(const NEXUS_Rect *clipBase1, const NEXUS_Rect *clipRect1, const NEXUS_SurfaceRegion *clipBase2, const NEXUS_Rect *clipRect2, NEXUS_Rect *clipRect)
{
    clipRect->width = (clipRect1->width * clipRect2->width) / clipBase2->width;
    clipRect->x = ((clipRect2->x*clipBase1->width) + (clipRect1->x*clipRect2->width))/clipBase2->width;
    clipRect->height = (clipRect1->height * clipRect2->height)/ clipBase2->height;
    clipRect->y = ((clipRect2->y*clipBase1->height) + (clipRect1->y*clipRect2->height))/clipBase2->height;
    BDBG_MSG(("combine_clipping: %u,%u,%u,%u -> %u,%u,%u,%u", clipRect1->x, clipRect1->y, clipRect1->width, clipRect1->height, clipRect->x, clipRect->y, clipRect->width, clipRect->height));
    return;
}

#if 0
#define PRINT_REGION(NAME, PREGION) BDBG_WRN(("line %d: %s: %d,%d",  __LINE__, NAME, (PREGION)->width, (PREGION)->height))
#define PRINT_RECT(NAME, PRECT) BDBG_WRN(("line %d: %s: %d,%d,%d,%d", __LINE__, NAME, (PRECT)->x, (PRECT)->y, (PRECT)->width, (PRECT)->height))
#else
#define PRINT_REGION(NAME, PREGION)
#define PRINT_RECT(NAME, PRECT)
#endif

/* returns non-zero if window actually changed */
static int nexus_surfaceclient_p_setvideo( NEXUS_SurfaceClientHandle client )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned i;
    const NEXUS_SurfaceClientSettings *pSettings = &client->settings;
    bool changed = false;

    /* if no video window, nothing to do */
    if (!client->window[0]) return 0;

    for (i=0;i<NEXUS_MAX_DISPLAYS;i++) {
        NEXUS_VideoWindowSettings *pWindowSettings;
        NEXUS_VideoWindowHandle window;
        bool visible = client->settings.composition.visible && client->parent->serverSettings.composition.visible && !client->server->settings.muteVideo[i];
        /* XXX also verify that parent is on the top */
        const struct NEXUS_SurfaceCompositorDisplay *cmpDisplay = client->server->display[i];

        if(cmpDisplay==NULL) {
            continue;
        }

        window = client->window[i];
        if (!window) continue;

        pWindowSettings = &client->server->windowSettings;
        NEXUS_VideoWindow_GetSettings_priv(window, pWindowSettings);
        BKNI_Memcpy(&client->server->prevWindowSettings, pWindowSettings, sizeof(*pWindowSettings)); /* use memcpy to match memcmp later on */

        /* unlike graphics, video window's target is actual display coordinates.
        this algorithm will set window position and source clipping.
        two passes are required:
        1) scale and clip for the client
        2) scale and clip for the display
        the source clip must be done after both stages are complete.
        */

        if(visible) {
            NEXUS_Rect windowScreenRect; /* unclipped client rectangle */
            NEXUS_SurfaceRegion displayScale;
            NEXUS_SurfaceRegion clientScale;
            NEXUS_Rect display;
            NEXUS_Rect temp, temp2;
            const NEXUS_SurfaceRegion *pInputCoord;

            /* set artificial source coordinates */
            pWindowSettings->clipBase.x = 0;
            pWindowSettings->clipBase.y = 0;
            pWindowSettings->clipBase.width = 720;
            pWindowSettings->clipBase.height = 480;

            PRINT_RECT("parent pos in virtual display coord", &client->parent->serverSettings.composition.position);

            /* convert pSettings->composition.position into client's top-level coordiantes */
            if (pSettings->composition.virtualDisplay.width && pSettings->composition.virtualDisplay.height) {
                /* from given virtual display coordinates */
                pInputCoord = &pSettings->composition.virtualDisplay;
            }
            else {
                /* from parent's dimensions */
                pInputCoord = &client->parent->state.clientRegion;
            }
            clientScale.width = client->parent->serverSettings.composition.position.width;
            clientScale.height = client->parent->serverSettings.composition.position.height;
            nexus_surfacemp_p_convert_coord(
                pInputCoord,
                &clientScale, /* to the client's top-level position, in virtual display coordinates */
                &pSettings->composition.position,
                &temp
            );

            /* apply origin */
            temp.x += client->parent->serverSettings.composition.position.x;
            temp.y += client->parent->serverSettings.composition.position.y;
            PRINT_REGION("video window coord", pInputCoord);
            PRINT_REGION("virtual display coord", &clientScale);
            PRINT_RECT("video pos in video window coord", &pSettings->composition.position);
            PRINT_RECT("video pos in virtual display coord", &temp);

            /* clip in client box */
            nexus_surfacemp_p_clip_rect(&client->parent->serverSettings.composition.position, &temp, &pWindowSettings->position);
            PRINT_RECT("clipped video pos in virtual display coord", &pWindowSettings->position);

            displayScale.width = display.width = cmpDisplay->formatInfo.canvas.width;
            displayScale.height = display.height = cmpDisplay->formatInfo.canvas.height;
            display.x = display.y = 0;

            nexus_surfacemp_p_convert_coord(&client->parent->state.virtualDisplay, &displayScale, &temp, &windowScreenRect); /* unclipped window */
            PRINT_REGION("virtual display coord", &client->parent->state.virtualDisplay);
            PRINT_REGION("actual display coord", &displayScale);
            PRINT_RECT("video pos in virtual display coord", &temp);
            PRINT_RECT("video pos in actual display coord", &windowScreenRect);
            nexus_surfacemp_p_convert_coord(&client->parent->state.virtualDisplay, &displayScale, &pWindowSettings->position, &temp); /* clipped window */
            nexus_surfacemp_p_clip_rect(&display, &temp, &temp2); /* must clip because rounding could be different for the two scales */
            /* clip into the screen box */
            nexus_surfacemp_p_clip_rect(&display, &temp, &pWindowSettings->position);

            /* find clipped source */
            if (temp2.width && temp2.height) {
                NEXUS_Rect windowScreenClipRect;
                nexus_surfacemp_p_clip_rect(&windowScreenRect, &temp2, &windowScreenClipRect);
                nexus_surfacemp_scale_clipped_rect(&windowScreenRect, &windowScreenClipRect, &pWindowSettings->clipBase, &pWindowSettings->clipRect);
            }
            else {
                /* no clip */
                pWindowSettings->clipRect = pWindowSettings->clipBase;
            }

            if(pSettings->composition.clipRect.x || pSettings->composition.clipRect.y || pSettings->composition.clipRect.width || pSettings->composition.clipRect.height) {
                /* add user requested clipping, using either clipBase or virtualDisplay */
                NEXUS_SurfaceRegion clipBase = pSettings->composition.clipBase;
                if (!clipBase.width || !clipBase.height) {
                    clipBase = pSettings->composition.virtualDisplay;
                }
                if (clipBase.width && clipBase.height) {
                    nexus_surfaceclient_p_combine_clipping(&pWindowSettings->clipBase, &pWindowSettings->clipRect, &clipBase, &pSettings->composition.clipRect,  &temp);
                    pWindowSettings->clipRect = temp;
                }
            }

            /* 16x10 is VDC's enforced minimum */
            if(pWindowSettings->position.width < 16 || pWindowSettings->position.height < 10) {
                visible = false;
                /* SW7445-1796: position should be a don't care if invisible, but there's a race condition which causes a full-screen flash.
                just keep the dimensions from falling below a minimum. */
                pWindowSettings->position.width = 16;
                pWindowSettings->position.height = 10;
                if (pWindowSettings->position.x + pWindowSettings->position.width > display.width) {
                    pWindowSettings->position.x = display.width - pWindowSettings->position.width;
                }
                if (pWindowSettings->position.y + pWindowSettings->position.height > display.height) {
                    pWindowSettings->position.y = display.height - pWindowSettings->position.height;
                }
                pWindowSettings->clipRect = pWindowSettings->clipBase;
            }
            BDBG_MSG(("setvideo %d,%d,%d,%d => from:%dx%d to:%dx%d ==> %d,%d,%d,%d (%d,%d,%d,%d) %s",
                /* specified position */
                pSettings->composition.position.x, pSettings->composition.position.y, pSettings->composition.position.width, pSettings->composition.position.height,
                /* transform */
                pInputCoord->width, pInputCoord->height,
                display.width,
                display.height,
                /* actual position */
                pWindowSettings->position.x, pWindowSettings->position.y, pWindowSettings->position.width, pWindowSettings->position.height,
                pWindowSettings->clipRect.x, pWindowSettings->clipRect.y, pWindowSettings->clipRect.width, pWindowSettings->clipRect.height,
                visible?"visible":"hidden"));
        }

        pWindowSettings->visible = visible;
        if ((pWindowSettings->clipRect.width || pWindowSettings->clipRect.height) &&
            (pWindowSettings->clipRect.width < pWindowSettings->clipBase.width ||
            pWindowSettings->clipRect.height < pWindowSettings->clipBase.height))
        {
            /* source clipping requires no A/R correction */
            pWindowSettings->contentMode = NEXUS_VideoWindowContentMode_eFull;
        }
        else {
            pWindowSettings->contentMode = pSettings->composition.contentMode;
        }
        pWindowSettings->alpha = pSettings->composition.constantColor >> 24;

        /* set if there's any change */
        if (BKNI_Memcmp(&client->server->prevWindowSettings, pWindowSettings, sizeof(*pWindowSettings))) {
            changed = true;
            rc = NEXUS_VideoWindow_SetSettings_priv(window, pWindowSettings);
            if (rc) {rc = BERR_TRACE(rc); goto done;}
        }
    }

    /* set zorder */
    for (i=0;i<NEXUS_MAX_DISPLAYS;i++) {
        NEXUS_SurfaceClientHandle c, child;
        unsigned parentIndex = NEXUS_NUM_VIDEO_WINDOWS; /* wait to learn first parent, which could be 0 or 1 */
        unsigned parentZorder = 0; /* first parent is zorder 0 */
        unsigned surfaceClientZorder = 0;
        for (c = BLST_Q_FIRST(&client->server->clients); c; c = BLST_Q_NEXT(c, link)) {
            for (child = BLST_S_FIRST(&c->children); child; child = BLST_S_NEXT(child, child_link)) {
                NEXUS_VideoWindowSettings *pWindowSettings;
                NEXUS_VideoWindowHandle window;
                unsigned parent, setzorder;
                bool isMosaic;
                window = child->window[i];
                if (!window) continue;

                NEXUS_VideoWindow_GetParentIndex_isrsafe(window, &parent, &isMosaic);
                if (parentIndex != NEXUS_NUM_VIDEO_WINDOWS && parent != parentIndex) {
                    /* a change in parentIndex bumps parentZorder, but don't unsupported interleave to
                    cause collision with graphics zorder (which is NEXUS_NUM_VIDEO_WINDOWS, aka "above video"). */
                    if (parentZorder+1 == NEXUS_NUM_VIDEO_WINDOWS) {
                        /* unsupported interleave happens if pip is placed between main mosaics or vice versa. */
                        BDBG_ERR(("unsupported window zorder interleaving: %d %d %d", parent, parentIndex, parentZorder));
                    }
                    else {
                        ++parentZorder;
                    }
                }
                parentIndex = parent;
                /* for mosaics, passthrough SurfaceClient zorder and let NEXUS_Display and VDC resolve.
                for non-mosaics, resolve to only 0 or 1 so that video is always under graphics. */
                if (isMosaic) {
                    setzorder = surfaceClientZorder;
                }
                else {
                    setzorder = parentZorder;
                }

                pWindowSettings = &client->server->windowSettings;
                NEXUS_VideoWindow_GetSettings_priv(window, pWindowSettings);
                if (pWindowSettings->zorder != setzorder) {
                    BDBG_MSG(("client %p, child %p, display%d: zorder %d -> %d", (void *)c, (void *)child, i, pWindowSettings->zorder, setzorder));
                    changed = true;
                    pWindowSettings->zorder = setzorder;
                    rc = NEXUS_VideoWindow_SetSettings_priv(window, pWindowSettings);
                    if (rc) {rc = BERR_TRACE(rc); goto done;}
                }
                surfaceClientZorder++;
            }
        }
    }

done:
    return changed?1:0;
}

void nexus_surfaceclient_request_setvideo( NEXUS_SurfaceClientHandle client )
{
    BDBG_ASSERT(client->parent);
    client->set_video_pending.set = true;
    client->server->set_video_pending.set = true;
    nexus_surface_compositor_p_update_video(client->server);
}

void nexus_surface_compositor_p_update_video(NEXUS_SurfaceCompositorHandle server)
{
    NEXUS_DisplayUpdateMode updateMode;
    NEXUS_SurfaceClientHandle client;
    NEXUS_Time begin, end;

    /* at least one needs to be set, and none have to be in process. */
    if (!server->state.active) return;

    if (!g_NEXUS_SurfaceCompositorModuleSettings.modules.display) return;
    NEXUS_Module_Lock(g_NEXUS_SurfaceCompositorModuleSettings.modules.display);
    NEXUS_DisplayModule_SetUpdateMode_priv(NEXUS_DisplayUpdateMode_eManual, &updateMode);

    for (client = BLST_Q_FIRST(&server->clients); client; client = BLST_Q_NEXT(client, link)) {
        NEXUS_SurfaceClientHandle child;
        BDBG_ASSERT(!client->set_video_pending.set); /* parent never has video */
        for (child = BLST_S_FIRST(&client->children); child; child = BLST_S_NEXT(child, child_link)) {
            if (child->set_video_pending.set) {
                nexus_surfaceclient_p_setvideo(child);
                /* must fire windowMoved, even if nexus_surfaceclient_p_setvideo did not move it. app may be waiting. */
                client->set_video_pending.windowMoved = true; /* callback handled from top-level client, not video window child */
                server->set_video_pending.windowMoved = 1;    /* callback handled from top-level client, not video window child */
                child->set_video_pending.set = false;
            }
        }
    }
    server->set_video_pending.set = false;

    NEXUS_Time_Get(&begin);
    NEXUS_DisplayModule_SetUpdateMode_priv(NEXUS_DisplayUpdateMode_eAuto, NULL);
    NEXUS_Time_Get(&end);
    /* if this BVDC_ApplyChanges is blocking, we need to wait 2 vsyncs to get back to non-blocking behavior. */
    if (NEXUS_Time_Diff(&end, &begin) > 10) {
        server->set_video_pending.windowMoved++;
    }
    NEXUS_Module_Unlock(g_NEXUS_SurfaceCompositorModuleSettings.modules.display);
}
