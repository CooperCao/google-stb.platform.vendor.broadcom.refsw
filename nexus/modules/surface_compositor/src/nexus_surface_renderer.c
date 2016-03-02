/***************************************************************************
 *     (c)2011-2014 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 **************************************************************************/
#include "nexus_surface_compositor_module.h"
#include "nexus_surface_compositor_impl.h"
#include "priv/nexus_surface_priv.h"

#define BDBG_MSG_TRACE(X) BDBG_MSG(X)

BDBG_MODULE(nexus_surface_renderer);
BTRC_MODULE(surface_compositor_render,ENABLE);

static bool nexus_surface_compositor_p_try_submitframebuffer(struct NEXUS_SurfaceCompositorDisplay *display);


static void nexus_surface_compositor_p_submitframebuffer(struct NEXUS_SurfaceCompositorDisplay *display, struct NEXUS_SurfaceCompositorFramebuffer *framebuffer)
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(display->server, NEXUS_SurfaceCompositor);
    BDBG_ASSERT(display->display);
    BDBG_MSG_TRACE(("setfb display[%u] fb[%p->%p]", display->index, (void *)display->displaying, (void *)framebuffer));
    framebuffer->state = NEXUS_SurfaceCompositorFramebufferState_eSubmitted;
    display->submitted = framebuffer;
    if(!display->formatInfo.native3D) {
        NEXUS_SurfaceHandle surface;

        if (display->server->bypass_compose.client) {
            NEXUS_SurfaceCompositor_P_PushElement *e = BLST_SQ_FIRST(&display->server->bypass_compose.client->queue.push);
            if (!e) {
                BDBG_ERR(("invalid bypass client: %p", (void *)display->server->bypass_compose.client));
                return;
            }
            if (display->server->bypass_compose.set) {
                /* if one already set, we submit the next if available */
                NEXUS_SurfaceCompositor_P_PushElement *current = e;
                e = BLST_SQ_NEXT(e, link);
                if (!e) {
                    e = current;
                }
            }
            BDBG_MSG_TRACE(("bypass: %p %p %p %p", (void *)display->server->bypass_compose.client, (void *)e->surface.surface,
                (void *)display->server->bypass_compose.set, (void *)display->server->bypass_compose.completing));
            surface = e->surface.surface;
            NEXUS_OBJECT_ACQUIRE(display->server, NEXUS_Surface, surface);
            display->server->bypass_compose.completing = display->server->bypass_compose.set;
            display->server->bypass_compose.set = surface;
        }
        else {
            surface = framebuffer->surface;
        }
        rc = NEXUS_Display_SetGraphicsFramebuffer(display->display, surface);
        if (rc) BERR_TRACE(rc); /* fall through */
    } else {
        NEXUS_GraphicsFramebuffer3D  framebuffer3D;
        BDBG_ASSERT(framebuffer->view3D.left);
        BDBG_ASSERT(framebuffer->view3D.right);
        NEXUS_Graphics_GetDefaultFramebuffer3D(&framebuffer3D);
        framebuffer3D.main = framebuffer->view3D.left;
        framebuffer3D.right = framebuffer->view3D.right;
        framebuffer3D.orientation = display->formatInfo.orientation; /* XXX unused */
        BDBG_MSG_TRACE(("setfb display[%u] fb[%p->%p] left:%p right:%p orientation:%u", display->index, (void *)display->displaying, (void *)framebuffer, (void *)framebuffer3D.main, (void *)framebuffer3D.right, framebuffer3D.orientation));
        rc = NEXUS_Display_SetGraphicsFramebuffer3D(display->display, &framebuffer3D);
        if (rc) BERR_TRACE(rc); /* fall through */
    }
    display->stats.compose++;

    return;
}

static void nexus_surface_compositor_slave_compositing_completed(struct NEXUS_SurfaceCompositorDisplay *slaveDisplay)
{
    unsigned new_ref_cnt;

    BDBG_MSG_TRACE(("slave_compositing_completed:%#lx %#lx:%#lx", (unsigned long)slaveDisplay, (unsigned long)slaveDisplay->compositing, (unsigned long)slaveDisplay->master_framebuffer));
    BDBG_ASSERT(slaveDisplay->master_framebuffer);
    BDBG_ASSERT(slaveDisplay->master_framebuffer->ref_cnt>0);
    BDBG_ASSERT(slaveDisplay->compositing);

    slaveDisplay->master_framebuffer->ref_cnt--;
    new_ref_cnt = slaveDisplay->master_framebuffer->ref_cnt;
    slaveDisplay->master_framebuffer = NULL;
    slaveDisplay->composited =  slaveDisplay->compositing;
    slaveDisplay->compositing = NULL;
    nexus_surface_compositor_p_try_submitframebuffer(slaveDisplay);

    return;
}

static void nexus_surface_compositor_render_slave(NEXUS_SurfaceCompositorHandle server, const struct NEXUS_SurfaceCompositorDisplay *masterDisplay, struct NEXUS_SurfaceCompositorDisplay *slaveDisplay,struct NEXUS_SurfaceCompositorFramebuffer *slaveFramebuffer, struct NEXUS_SurfaceCompositorFramebuffer *masterFramebuffer)
{
    NEXUS_Error rc;

    NEXUS_Graphics2D_GetDefaultBlitSettings(&server->renderState.blitSettings);
    BDBG_MSG_TRACE(("render_slave:%#lx  %#lx->%#lx", (unsigned long)slaveDisplay, (unsigned long)masterFramebuffer, (unsigned long)slaveFramebuffer));

    if(masterDisplay->formatInfo.orientation == slaveDisplay->formatInfo.orientation) {
        server->renderState.blitSettings.output.rect.x = 0;
        server->renderState.blitSettings.output.rect.y = 0;
        server->renderState.blitSettings.output.rect.width  = slaveDisplay->canvas.width;
        server->renderState.blitSettings.output.rect.height = slaveDisplay->canvas.height;
        server->renderState.blitSettings.source.rect.x = 0;
        server->renderState.blitSettings.source.rect.y = 0;
        server->renderState.blitSettings.source.rect.width = masterDisplay->canvas.width;
        server->renderState.blitSettings.source.rect.height = masterDisplay->canvas.height;
        server->renderState.blitSettings.output.surface = slaveFramebuffer->surface;
        if(server->bounceBufferMasterFramebuffer.buffer==NULL) {
            server->renderState.blitSettings.source.surface = masterFramebuffer->surface;
        } else {
            server->renderState.blitSettings.source.surface = server->bounceBufferMasterFramebuffer.buffer;
        }
    } else if(masterDisplay->formatInfo.orientation == NEXUS_VideoOrientation_e3D_LeftRight || masterDisplay->formatInfo.orientation == NEXUS_VideoOrientation_e3D_OverUnder) {
        BDBG_ASSERT(masterFramebuffer->view3D.right);
        BDBG_ASSERT(masterFramebuffer->view3D.left);
        if(slaveDisplay->formatInfo.orientation == NEXUS_VideoOrientation_e2D) {
            /* 3D -> 2D */
            server->renderState.blitSettings.source.surface = masterFramebuffer->view3D.left;
            server->renderState.blitSettings.output.rect.x = 0;
            server->renderState.blitSettings.output.rect.y = 0;
            server->renderState.blitSettings.output.rect.width  = slaveDisplay->canvas.width;
            server->renderState.blitSettings.output.rect.height = slaveDisplay->canvas.height;
            server->renderState.blitSettings.output.surface = slaveFramebuffer->surface;
        } else {
            /* 3D -> 3D */
            BDBG_ASSERT(slaveFramebuffer->view3D.left);
            BDBG_ASSERT(slaveFramebuffer->view3D.right);
            server->renderState.blitSettings.source.surface = masterFramebuffer->view3D.right;
            server->renderState.blitSettings.output.surface = slaveFramebuffer->view3D.right;
            rc = NEXUS_Graphics2D_Blit(server->gfx, &server->renderState.blitSettings);
            BERR_TRACE(rc);
            server->renderState.blitSettings.source.surface = masterFramebuffer->view3D.left;
            server->renderState.blitSettings.output.surface = slaveFramebuffer->view3D.left;
        }
    } else if(slaveDisplay->formatInfo.orientation == NEXUS_VideoOrientation_e3D_LeftRight || slaveDisplay->formatInfo.orientation == NEXUS_VideoOrientation_e3D_OverUnder) {
        /* 2D -> 3D */
        server->renderState.blitSettings.source.rect.x = 0;
        server->renderState.blitSettings.source.rect.y = 0;
        server->renderState.blitSettings.source.rect.width = masterDisplay->canvas.width;
        server->renderState.blitSettings.source.rect.height = masterDisplay->canvas.height;
        server->renderState.blitSettings.source.surface = masterFramebuffer->surface;
        server->renderState.blitSettings.output.surface = slaveFramebuffer->view3D.right;
        rc = NEXUS_Graphics2D_Blit(server->gfx, &server->renderState.blitSettings);
        BERR_TRACE(rc);
        server->renderState.blitSettings.output.surface = slaveFramebuffer->view3D.left;
    } else {
        BDBG_ASSERT(0);
    }
    slaveFramebuffer->generation = masterFramebuffer->generation;
    slaveDisplay->master_framebuffer = masterFramebuffer;
    slaveDisplay->compositing = slaveFramebuffer;
    masterFramebuffer->ref_cnt++;
    slaveFramebuffer->generation = masterFramebuffer->generation;

    rc = NEXUS_Graphics2D_Blit(server->gfx, &server->renderState.blitSettings);
    BERR_TRACE(rc);

    NEXUS_SurfaceCursor_P_ReleaseCursors(server, slaveDisplay, slaveFramebuffer);
    NEXUS_SurfaceCursor_P_RenderCursors(server, slaveDisplay, slaveFramebuffer);
    return;
}

static void nexus_surface_compositor_update_slave(NEXUS_SurfaceCompositorHandle server, struct NEXUS_SurfaceCompositorDisplay *masterDisplay, struct NEXUS_SurfaceCompositorFramebuffer *masterFramebuffer, struct NEXUS_SurfaceCompositorDisplay *slaveDisplay)
{
    BDBG_MSG_TRACE(("update_slave:%#lx %#lx:%#lx -> %#lx(%#lx,%#lx,%#lx)", (unsigned long)server, (unsigned long)masterDisplay, (unsigned long)masterFramebuffer, (unsigned long)slaveDisplay, (unsigned long)slaveDisplay->compositing, (unsigned long)slaveDisplay->composited, (unsigned long)slaveDisplay->submitted));
    if(slaveDisplay->compositing==NULL && slaveDisplay->composited==NULL && slaveDisplay->submitted==NULL) {
        struct NEXUS_SurfaceCompositorFramebuffer *framebuffer = slaveDisplay->displaying;
        if(framebuffer) {
            BDBG_MSG_TRACE(("update_slave:%#lx %#lx:%#lx -> %#lx:%#lx (%u,%u)", (unsigned long)server, (unsigned long)masterDisplay, (unsigned long)masterFramebuffer, (unsigned long)slaveDisplay, (unsigned long)framebuffer, framebuffer->generation, masterFramebuffer->generation));
            if( ((int)(framebuffer->generation - masterFramebuffer->generation))>=0) {
                goto done;/* newer frame already displayed */
            }
        }
        if(slaveDisplay->num_framebuffers==1) {
            nexus_surface_compositor_p_recycle_displayed_framebuffer(slaveDisplay);
        }
        framebuffer = BLST_SQ_FIRST(&slaveDisplay->available);
        BDBG_MSG_TRACE(("update_slave:%#lx %#lx:%#lx -> %#lx:%#lx", (unsigned long)server, (unsigned long)masterDisplay, (unsigned long)masterFramebuffer, (unsigned long)slaveDisplay, (unsigned long)framebuffer));
        if(framebuffer) {
            slaveDisplay->slave.dirty = false;
            BLST_SQ_REMOVE_HEAD(&slaveDisplay->available, link);
            nexus_surface_compositor_render_slave(server, masterDisplay, slaveDisplay, framebuffer, masterFramebuffer);
        }
    }
done:
    return;
}

static void nexus_surface_compositor_update_slaves(NEXUS_SurfaceCompositorHandle server, struct NEXUS_SurfaceCompositorDisplay *masterDisplay, struct NEXUS_SurfaceCompositorFramebuffer *masterFramebuffer)
{
    unsigned i;
    for(i=0;i<NEXUS_MAX_DISPLAYS;i++) {
        struct NEXUS_SurfaceCompositorDisplay *slaveDisplay = server->display[i];
        if(masterDisplay->index==i) {
            continue;
        }
        if(slaveDisplay==NULL || slaveDisplay->display==NULL) {
            continue;
        }
        slaveDisplay->slave.dirty = true;
        nexus_surface_compositor_update_slave(server, masterDisplay, masterFramebuffer, slaveDisplay);
    }
    return;
}

void nexus_surface_compositor_p_release_surfaces(NEXUS_SurfaceCompositorHandle server)
{
    unsigned i;

    for(i=0;i<server->renderState.elements.count;i++) {
        const NEXUS_P_SurfaceCompositorRenderElement *data = server->renderState.elements.data+i;
        NEXUS_OBJECT_RELEASE(server, NEXUS_Surface, data->sourceSurface);
    }
    /* clear state, so it's safe to call this function multiple times */
    server->renderState.elements.count = 0;
    server->renderState.tunnel.active = false;
    server->renderState.tunnel.tunnelSource = NULL;
    return;
}

static void nexus_surface_compositor_p_return_tunnel_framebuffer(struct NEXUS_SurfaceCompositorDisplay *display, struct NEXUS_SurfaceCompositorFramebuffer *framebuffer)
{
    if(!framebuffer->tunnel.acquired) {
        BDBG_MSG_TRACE(("return_tunnel_framebuffer:%p %p to display", (void *)display, (void *)framebuffer));
        framebuffer->state = NEXUS_SurfaceCompositorFramebufferState_eAvailable;
        BLST_SQ_INSERT_TAIL(&display->available, framebuffer, link);
    } else {
        BDBG_MSG_TRACE(("return_tunnel_framebuffer:%p %p to tunnel", (void *)display, (void *)framebuffer));
        framebuffer->tunnel.composited = true;
        framebuffer->state = NEXUS_SurfaceCompositorFramebufferState_eAvailable;
        BLST_Q_INSERT_TAIL(&display->tunnel.available, framebuffer , tunnelLink);
        BDBG_ASSERT(display->server->tunnel.client);
        NEXUS_TaskCallback_Fire(display->server->tunnel.client->recycledCallback);
    }
    return;
}

static void nexus_surface_compositor_p_recycle_push(NEXUS_SurfaceClientHandle client)
{
    NEXUS_SurfaceCompositor_P_PushElement *first_node = BLST_SQ_FIRST(&client->queue.push);
    BDBG_MSG_TRACE(("nexus_surface_compositor_p_recycle_push:%p queue recycle %p", (void *)client, (void *)first_node->surface.surface));
    NEXUS_OBJECT_RELEASE(client, NEXUS_Surface, first_node->surface.surface);
    BLST_SQ_REMOVE_HEAD(&client->queue.push, link);
    BLST_SQ_INSERT_TAIL(&client->queue.recycle, first_node, link);
    NEXUS_TaskCallback_Fire(client->recycledCallback);
    client->state.update_flags |= NEXUS_P_SURFACECLIENT_UPDATE_SOURCE;
}

static void nexus_surface_compositor_p_compose_release_clients(NEXUS_SurfaceCompositorHandle server)
{
    NEXUS_SurfaceClientHandle client;

    /* after compositing completed, buffers for push type clients could be immediately recycled */
    for (client = BLST_Q_FIRST(&server->clients); client; client = BLST_Q_NEXT(client, link)) {
        if(client->state.client_type==client_type_push || client->state.client_type == client_type_tunnel_emulated) {
            if(client->queue.compositing) {
                NEXUS_SurfaceCompositor_P_PushElement *first_node = BLST_SQ_FIRST(&client->queue.push);
                client->queue.compositing = false;
                if(first_node) {
                    NEXUS_SurfaceCompositor_P_PushElement *next_node = BLST_SQ_NEXT(first_node, link);
                    unsigned targetRepeatRate = client->settings.virtualRefreshRate ? 10*server->display[0]->formatInfo.verticalFreq / client->settings.virtualRefreshRate : 0;
                    client->queue.last = next_node == NULL;
                    if(client->settings.virtualRefreshRate && client->queue.firstCompositingValid && targetRepeatRate) {
                        /* test whether surface has to be rendered again */
                        /* CHANGE 1: use microsecond time from TMR and do microsecond calculations to minimize rounding error */
                        unsigned currentTime = nexus_surface_compositor_p_get_time(server);
                        long diff, target; /* usec */
                        bool repeat;
                        diff = nexus_surface_compositor_p_get_time_diff(server, currentTime, client->queue.firstCompositing);
                        diff += client->queue.frameFractionalTime;
                        target = (1000 * 1000 * 1000)/client->settings.virtualRefreshRate;
                        BDBG_MSG_TRACE(("nexus_surface_compositor_p_compose_release_clients:%p virtualRefreshRate %ld(%ld)(%ld) node:%p %s", (void *)client, diff, target, client->queue.frameFractionalTime, (void *)first_node, diff<target?"repeat":""));
#if BDBG_DEBUG_BUILD
                        ++client->queue.repeatCount;
#endif
                        repeat = diff < target;
#if BDBG_DEBUG_BUILD
                        if (!repeat && client->queue.repeatCount != targetRepeatRate) {
                            BDBG_MSG(("virtualRefreshRate:%p %ld+%ld<%ld [%ld] node:%p cnt=%d[%d] %s", (void *)client,
                                diff-client->queue.frameFractionalTime, client->queue.frameFractionalTime, target,
                                client->queue.frameFractionalTimeHysteresis,
                                (void *)first_node, client->queue.repeatCount, targetRepeatRate, repeat?"":"last"));
                        }
#endif
                        if(repeat) {
                            continue;
                        } else {
                            long lFTTmp;
#if BDBG_DEBUG_BUILD
                            client->queue.repeatCount = 0;
#endif
                            client->queue.frameFractionalTime = diff - target;
                            client->queue.firstCompositing = currentTime;

                            /* hysteresis filter...
                               gate propagation of frameFractionalTime when it's within +/- 10% of the vsync interval */
                            lFTTmp = client->queue.frameFractionalTime + client->queue.frameFractionalTimeHysteresis;
                            if( (lFTTmp >= server->display[0]->formatInfo.vsyncIntervalUsecHystLow) &&
                                (lFTTmp <= server->display[0]->formatInfo.vsyncIntervalUsecHystHigh) )
                            {
                                client->queue.frameFractionalTimeHysteresis = (lFTTmp - server->display[0]->formatInfo.vsyncIntervalUsecHystLow);
                                client->queue.frameFractionalTime = server->display[0]->formatInfo.vsyncIntervalUsecHystLow;
                            }
                            else
                            {
                                client->queue.frameFractionalTime = lFTTmp;
                                client->queue.frameFractionalTimeHysteresis = 0;
                            }
                        }
                    }
                    if(next_node) {
                        if (client != server->bypass_compose.client) {
                            nexus_surface_compositor_p_recycle_push(client);
                        }
                    } else {
                        client->queue.frameFractionalTime = 0;
                        client->queue.firstCompositingValid = false;
                    }
                }
            }
        }
    }
    return;
}


static void nexus_surface_compositor_compositing_completed(NEXUS_SurfaceCompositorHandle server)
{
    struct NEXUS_SurfaceCompositorDisplay *display = server->display[0];
    unsigned i;

    BDBG_ASSERT(display);
    BDBG_MSG_TRACE(("compositing_completed:%p %p", (void *)server, (void *)display->compositing));

    nexus_surface_compositor_p_blitter_release(server, NEXUS_P_SURFACEBLITTER_COMPOSITOR);
    nexus_surface_compositor_p_update_dirty_clients_done(server); /* check if for set clients server side copy was updated */

    if(display->compositing) {
        struct NEXUS_SurfaceCompositorFramebuffer *framebuffer = display->compositing;
        display->compositing = NULL;
        server->renderState.step = taskCompleted;
        server->renderState.current = 0;
        framebuffer->scene.elements.count = 0;
        for(i=0;i<server->renderState.elements.count;i++) {
            const NEXUS_P_SurfaceCompositorRenderElement *data = server->renderState.elements.data+i;
            NEXUS_P_SurfaceCompositorRenderElement *dest_data = NEXUS_P_SurfaceCompositorRenderElements_Next(&framebuffer->scene.elements);
            if(dest_data==NULL) {
                break;
            }
            *dest_data = *data;
        }
        framebuffer->scene.dirty = server->renderState.tunnel.rect;
        display->composited = framebuffer;
        framebuffer->state = NEXUS_SurfaceCompositorFramebufferState_eComposited;
        if(server->renderState.tunnel.tunnelSource && server->renderState.tunnel.tunnelSource->state == NEXUS_SurfaceCompositorFramebufferState_eTunnelSubmitted) {
            nexus_surface_compositor_p_return_tunnel_framebuffer(display,server->renderState.tunnel.tunnelSource);
        }
        nexus_surface_compositor_p_release_surfaces(server);
        nexus_surface_compositor_p_compose_release_clients(server);
        nexus_surface_compositor_p_try_submitframebuffer(display);
    }

    for(i=0;i<NEXUS_MAX_DISPLAYS;i++) {
        struct NEXUS_SurfaceCompositorDisplay *slaveDisplay = server->display[i];
        if(slaveDisplay==NULL) {
            continue;
        }
        if(display->index==i) {
            continue;
        }
        if(slaveDisplay->compositing) {
            nexus_surface_compositor_slave_compositing_completed(slaveDisplay);
        }
    }
    BTRC_TRACE(surface_compositor_render,STOP);
    return;
}

static void nexus_surface_compositor_p_blitter_complete(NEXUS_SurfaceCompositorHandle compositor)
{
    BDBG_MSG_TRACE(("%p:blitter_complete %#x", (void *)compositor, compositor->state.blitter.active));
    switch(compositor->state.blitter.active) {
    case NEXUS_P_SURFACEBLITTER_COMPOSITOR:
        nexus_surface_compositor_compositing_completed(compositor);
        break;
    case NEXUS_P_SURFACEBLITTER_CURSOR:
        nexus_surface_compositor_p_blitter_release(compositor, NEXUS_P_SURFACEBLITTER_CURSOR);
        break;
    case NEXUS_P_SURFACEBLITTER_CLIENT:
        nexus_surface_compositor_p_blitter_release(compositor, NEXUS_P_SURFACEBLITTER_CLIENT);
        nexus_surface_compositor_p_update_dirty_clients_done(compositor);
        break;
    default:
        BDBG_ASSERT(0);
    }
    BDBG_MSG_TRACE(("%p:blitter_complete done %#x", (void *)compositor, compositor->state.blitter.active));
    BDBG_ASSERT(compositor->state.blitter.active==0);
    return;
}

void nexus_surface_compositor_p_checkpoint(void *context)
{
    NEXUS_SurfaceCompositorHandle compositor = context;

    BDBG_OBJECT_ASSERT(compositor, NEXUS_SurfaceCompositor);
  
    if (!compositor->settings.enabled && compositor->state.active) {
        unsigned i;
        for (i=0;i<NEXUS_MAX_DISPLAYS;i++) {
            struct NEXUS_SurfaceCompositorDisplay *display = compositor->display[i];
            if(display==NULL) {
                continue;
            }
            display->compositing = NULL;
        }
        nexus_p_surface_compositor_check_inactive(compositor);
        if (!compositor->state.active) return;
    }
    nexus_surface_compositor_p_blitter_complete(compositor);

    if(compositor->state.blitter.delayed & NEXUS_P_SURFACEBLITTER_COMPOSITOR) {
        compositor->state.blitter.delayed &= ~(NEXUS_P_SURFACEBLITTER_CURSOR | NEXUS_P_SURFACEBLITTER_COMPOSITOR | NEXUS_P_SURFACEBLITTER_CLIENT);
        if (compositor->settings.enabled) {
            nexus_surface_compositor_p_compose(compositor); /* compose would render cursors at new framebuffer */
        }
    } else if(compositor->state.blitter.delayed & NEXUS_P_SURFACEBLITTER_CURSOR) {
        compositor->state.blitter.delayed &= ~(NEXUS_P_SURFACEBLITTER_CURSOR);
        if (compositor->settings.enabled) {
            NEXUS_SurfaceCursor_P_UpdateCursor(compositor);
        }
    } else if(compositor->state.blitter.delayed & NEXUS_P_SURFACEBLITTER_CLIENT) {
        compositor->state.blitter.delayed &= ~(NEXUS_P_SURFACEBLITTER_CLIENT);
        nexus_surface_compositor_p_update_dirty_clients(compositor, NULL /* update all clients */);
    }
    return;
}




void nexus_surface_compositor_p_packetspaceavailable(void *context)
{
    NEXUS_SurfaceCompositorHandle server = context;
    /* TODO */
    BSTD_UNUSED(server);
}

static bool nexus_surface_compositor_p_try_submitframebuffer(struct NEXUS_SurfaceCompositorDisplay *display)
{
    BDBG_MSG_TRACE(("try_sumbitframebuffer:%p %p[%u] submitted:%p composited:%p", (void *)display->server, (void *)display, display->index, (void *)display->submitted, (void *)display->composited));

    if(display->submitted==NULL && display->composited) {
        struct NEXUS_SurfaceCompositorFramebuffer *framebuffer = display->composited;
        display->composited = NULL;
        nexus_surface_compositor_p_submitframebuffer(display, framebuffer);
        return true;
    } else {
        return false;
    }
}

void nexus_surface_compositor_p_recycle_displayed_framebuffer(struct NEXUS_SurfaceCompositorDisplay *display)
{
    struct NEXUS_SurfaceCompositorFramebuffer *framebuffer;
    framebuffer = display->displaying;
    if(framebuffer) {
        /* BDBG_ASSERT(framebuffer->state == NEXUS_SurfaceCompositorFramebufferState_eDisplaying); */
        display->displaying = NULL;
        BDBG_MSG_TRACE(("recycle_displayed_framebuffer:%#lx recycling:%#lx(%u) %s %s", (unsigned long)display->server, (unsigned long)framebuffer, framebuffer->ref_cnt, display->index?"master":"slave", framebuffer->tunnel.acquired?"tunnel":""));
        nexus_surface_compositor_p_return_tunnel_framebuffer(display,framebuffer);
    }
    return;
}


static void nexus_surface_compositor_p_slave_framebuffer_applied(struct NEXUS_SurfaceCompositorDisplay *display)
{
    struct NEXUS_SurfaceCompositorFramebuffer *framebuffer;
    NEXUS_SurfaceCompositorHandle server = display->server;

    BDBG_OBJECT_ASSERT(server, NEXUS_SurfaceCompositor);

    framebuffer = display->displaying;
    BDBG_MSG_TRACE(("slave_framebuffer:%#lx %#lx<->%#lx", (unsigned long)server, (unsigned long)framebuffer, (unsigned long)display->submitted));
    nexus_surface_compositor_p_recycle_displayed_framebuffer(display);
    BDBG_ASSERT(display->submitted);
    framebuffer = display->submitted;
    display->submitted = NULL;
    display->displaying = framebuffer;
    framebuffer->state = NEXUS_SurfaceCompositorFramebufferState_eDisplaying;
    nexus_surface_compositor_p_try_submitframebuffer(display);
    /* slave displays could be only copied when clean master framebuffer available, which is at the end of nexus_surface_compositor_p_render_framebuffer */
    return;
}

static void nexus_surface_compositor_p_master_framebuffer_applied(struct NEXUS_SurfaceCompositorDisplay *display)
{
    NEXUS_SurfaceCompositorHandle server = display->server;
    struct NEXUS_SurfaceCompositorFramebuffer *framebuffer;

    BDBG_OBJECT_ASSERT(server, NEXUS_SurfaceCompositor);

    nexus_surface_compositor_p_recycle_displayed_framebuffer(display);

    BDBG_ASSERT(display->submitted);
    framebuffer = display->submitted;
    display->submitted = NULL;
    display->displaying = framebuffer;
    framebuffer->state = NEXUS_SurfaceCompositorFramebufferState_eDisplaying;
    BDBG_MSG_TRACE(("master_framebuffer_applied:%p %p[%u] displaying:%p", (void *)server, (void *)display, display->index, (void *)framebuffer));

    nexus_surface_compositor_p_try_submitframebuffer(display);

    {
        NEXUS_SurfaceClientHandle client;
        bool recompose = server->pending_update;


        /* framebuffers have been set. */
        NEXUS_TaskCallback_Fire(server->frameBufferCallback);
        for (client = BLST_Q_FIRST(&server->clients); client; client = BLST_Q_NEXT(client, link)) {
            if (client->process_pending_displayed_callback) {
                client->process_pending_displayed_callback = false;
                client->pending_displayed_callback = false;
                BDBG_MSG_TRACE(("master_framebuffer_callback:%p client %p(%u) displayed", (void *)server, (void *)client, client->client_id));
                NEXUS_TaskCallback_Fire(client->displayedCallback);
            }
            if(!recompose) {
                if(client->published.surface) { /* if one client is published, we must rerender for the next vsync */
                    recompose = true;
                } else if(client->state.client_type==client_type_push || client->state.client_type == client_type_tunnel_emulated) {
                    if (!client->queue.last) {
                        recompose = true; /* need to compose with new surface */
                        BDBG_MSG_TRACE(("master_framebuffer_callback:%p client %p recompose", (void *)server, (void *)client));
                    }
                }
            }
        }
        if(!recompose) {
            unsigned i;
            /* if there are any slave displays that weren't rendered we need to re-render master display to get clean copy of the framebuffer */
            for(i=0;i<NEXUS_MAX_DISPLAYS;i++) {
                struct NEXUS_SurfaceCompositorDisplay *slaveDisplay = server->display[i];
                if(display->index==i) {
                    continue;
                }
                if(slaveDisplay==NULL || slaveDisplay->display==NULL) {
                    continue;
                }
                if(slaveDisplay->slave.dirty) {
                    recompose = true;
                }
            }
        }

        if (recompose) {
            BDBG_MSG_TRACE(("master_framebuffer_callback:%p recompose", (void *)server));
            nexus_surface_compositor_p_compose(server);
        }
        else {
            nexus_surface_compositor_p_update_video(server);
        }
    }
    return;
}

void nexus_surface_compositor_p_framebuffer(void *context)
{
    struct NEXUS_SurfaceCompositorDisplay *display = context;
    BDBG_MSG_TRACE(("framebuffer_callback:%p %p[%u] new:%p old:%p", (void *)display->server, (void *)display, display->index, (void *)display->submitted, (void *)display->displaying));

    if (!display->server->settings.enabled) {
        display->submitted = NULL;
        nexus_p_surface_compositor_check_inactive(display->server);
        return;
    }

    if (display->server->bypass_compose.completing) {
        BDBG_MSG_TRACE(("bypass recycle: %p %p %p", (void *)display->server->bypass_compose.client, (void *)display->server->bypass_compose.set,
            (void *)display->server->bypass_compose.completing));
        NEXUS_OBJECT_RELEASE(display->server, NEXUS_Surface, display->server->bypass_compose.completing);
        display->server->bypass_compose.completing = NULL;
        if (!display->server->bypass_compose.client) {
            display->server->bypass_compose.completing = display->server->bypass_compose.set;
            display->server->bypass_compose.set = NULL;
        }
        else {
            nexus_surface_compositor_p_recycle_push(display->server->bypass_compose.client);
            nexus_surface_compositor_p_realloc_framebuffers(display->server);
        }
    }

    /* may get stale callback after standby */
    if (!display->submitted) return;

    if(display->index==0) {
        nexus_surface_compositor_p_master_framebuffer_applied(display);
    } else {
        nexus_surface_compositor_p_slave_framebuffer_applied(display);
    }
}

void nexus_surface_compositor_p_vsync(void *context)
{
    NEXUS_SurfaceCompositorHandle server = context;
    if (server->set_video_pending.windowMoved) {
        NEXUS_SurfaceClientHandle client;
        for (client = BLST_Q_FIRST(&server->clients); client; client = BLST_Q_NEXT(client, link)) {
            if (client->set_video_pending.windowMoved) {
                client->set_video_pending.windowMoved = false;
                NEXUS_TaskCallback_Fire(client->windowMovedCallback);
            }
        }
        if (--server->set_video_pending.windowMoved == 0) {
            nexus_surface_compositor_p_update_video(server);
        }
    }
    {
       NEXUS_SurfaceClientHandle client;
       for (client = BLST_Q_FIRST(&server->clients); client; client = BLST_Q_NEXT(client, link)) {
           NEXUS_TaskCallback_Fire(client->vsyncCallback);
       }
    }
}

/* return true if composition is opaque, e.g. it would override (not blend) anything that drawn behind it */
bool nexus_p_surface_composition_opaque(const NEXUS_BlendEquation *colorBlend, const NEXUS_BlendEquation *alphaBlend)
{
    BSTD_UNUSED(alphaBlend);
    return colorBlend->c == NEXUS_BlendFactor_eZero && colorBlend->d == NEXUS_BlendFactor_eZero;
}

/* skip blend if lowest level client is on black background */
static bool nexus_p_reducible_blend(struct NEXUS_SurfaceCompositorDisplay *cmpDisplay, unsigned i)
{
    return i == 0 && cmpDisplay->backgroundColor == 0;
}

/* if we know the dest surface is all 0, we can use blend factors zero/one directly */
static unsigned nexus_p_switch_dest_term(NEXUS_BlendFactor *f) {
    switch (*f) {
    case NEXUS_BlendFactor_eDestinationColor:
    case NEXUS_BlendFactor_eDestinationAlpha:
        *f = NEXUS_BlendFactor_eZero;
        return 1;
    case NEXUS_BlendFactor_eInverseDestinationColor:
    case NEXUS_BlendFactor_eInverseDestinationAlpha:
        *f = NEXUS_BlendFactor_eOne;
        return 1;
    default:
        return 0;
    }
}

/* only use if nexus_p_reducable_blend() is true */
static void
nexus_p_surface_composition_reduce_equation(NEXUS_BlendEquation *cb, NEXUS_BlendEquation *ab)
{
    nexus_p_switch_dest_term(&cb->a);
    nexus_p_switch_dest_term(&cb->b);
    nexus_p_switch_dest_term(&cb->c);
    nexus_p_switch_dest_term(&cb->d);
    nexus_p_switch_dest_term(&cb->e);
    nexus_p_switch_dest_term(&ab->a);
    nexus_p_switch_dest_term(&ab->b);
    nexus_p_switch_dest_term(&ab->c);
    nexus_p_switch_dest_term(&ab->d);
    nexus_p_switch_dest_term(&ab->e);
}

void nexus_surface_compositor_p_render_framebuffer(NEXUS_SurfaceCompositorHandle server, struct NEXUS_SurfaceCompositorDisplay *cmpDisplay, struct NEXUS_SurfaceCompositorFramebuffer *framebuffer)
{
    unsigned i;
    NEXUS_Error rc; const NEXUS_P_SurfaceCompositorRenderElement *data;

    BDBG_MSG_TRACE(("render_framebuffer:%p: %p:%p[%u]", (void *)server, (void *)framebuffer, (void *)framebuffer->surface, framebuffer->ref_cnt));
    /* this function could be interrupted at any call to Graphics2D call, at which point it would save it's state and be resumed (by calling function over) some later time */
    if(server->renderState.step == taskInit) {
        BTRC_TRACE(surface_compositor_render,START);
        for(i=0;i<server->renderState.elements.count;i++) {
            data = server->renderState.elements.data + i;
            NEXUS_OBJECT_ACQUIRE(server, NEXUS_Surface, data->sourceSurface);
        }
        if (server->bypass_compose.client) {
            server->renderState.step = taskCompleted;
            nexus_surface_compositor_compositing_completed(server);
            return;
        }
        server->renderState.current = 0;
        server->renderState.step = taskFill;
    }
    if(server->renderState.step == taskFill) {
        if(server->renderState.current==0) {
            /* verify whether there are any 'full screen' elements thus making fill step unnecessary */
            for(i=0;i<server->renderState.elements.count;i++) {
                data = server->renderState.elements.data + i;
                if(data->outputRect.width == cmpDisplay->canvas.width && data->outputRect.height == cmpDisplay->canvas.height &&
                   (nexus_p_surface_composition_opaque(&data->colorBlend, &data->alphaBlend) || nexus_p_reducible_blend(cmpDisplay, i))) {
                    NEXUS_SurfaceCursor_P_ReleaseCursors(server, cmpDisplay, framebuffer);
                    goto blit_step;
                }
            }
        }
        NEXUS_SurfaceCursor_P_ClearCursors(server, cmpDisplay, framebuffer);
        NEXUS_Graphics2D_GetDefaultFillSettings(&server->renderState.fillSettings);

        server->renderState.fillSettings.color = cmpDisplay->backgroundColor;
        if(server->bounceBufferMasterFramebuffer.buffer==NULL) {
            server->renderState.fillSettings.surface = framebuffer->surface;
        } else {
            server->renderState.fillSettings.surface = server->bounceBufferMasterFramebuffer.buffer;
        }
        if(!server->renderState.tunnel.active) { /* we don't support incremental fill of tunneled client */
            if(framebuffer->scene.dirty.width && framebuffer->scene.dirty.height) {
                server->renderState.fillSettings.rect = framebuffer->scene.dirty; 
                BDBG_MSG_TRACE(("clear dirty:%#lx:%#lx [%u,%u %ux%u]", (unsigned long)server, (unsigned long)cmpDisplay, server->renderState.fillSettings.rect.x, server->renderState.fillSettings.rect.y, server->renderState.fillSettings.rect.width, server->renderState.fillSettings.rect.height));
                rc = NEXUS_Graphics2D_Fill(server->gfx, &server->renderState.fillSettings);
                /* TODO: if (rc == NEXUS_GRAPHICS2D_QUEUE_FULL) */
                if (rc) rc = BERR_TRACE(rc);
                cmpDisplay->stats.fill++;
            }
        }

        for(;server->renderState.current<framebuffer->scene.elements.count;server->renderState.current++) {
            data = framebuffer->scene.elements.data + server->renderState.current;
            server->renderState.fillSettings.rect = data->outputRect;
            BDBG_MSG_TRACE(("clear prev:%#lx :%#lx:%#lx [%u,%u %ux%u]", (unsigned long)server, (unsigned long)cmpDisplay, (unsigned long)data->sourceSurface, server->renderState.fillSettings.rect.x, server->renderState.fillSettings.rect.y, server->renderState.fillSettings.rect.width, server->renderState.fillSettings.rect.height));
            rc = NEXUS_Graphics2D_Fill(server->gfx, &server->renderState.fillSettings);
            /* TODO: if (rc == NEXUS_GRAPHICS2D_QUEUE_FULL) */
            if (rc) rc = BERR_TRACE(rc);
            cmpDisplay->stats.fill++;
        }
blit_step:
        server->renderState.step = taskBlit;
        server->renderState.current = 0;
    }

    if(server->renderState.step == taskBlit) {
        NEXUS_Graphics2D_GetDefaultBlitSettings(&server->renderState.blitSettings);
        server->renderState.blitSettings.colorOp = NEXUS_BlitColorOp_eUseBlendEquation;
        server->renderState.blitSettings.alphaOp = NEXUS_BlitAlphaOp_eUseBlendEquation;
        for(;server->renderState.current<server->renderState.elements.count;server->renderState.current++) {
            data = server->renderState.elements.data + server->renderState.current;
            if(!data->useBounceBuffer) {
                if(server->bounceBufferMasterFramebuffer.buffer==NULL) {
                    server->renderState.blitSettings.output.surface = framebuffer->surface;
                    server->renderState.blitSettings.dest.surface = framebuffer->surface;
                } else {
                    server->renderState.blitSettings.output.surface = server->bounceBufferMasterFramebuffer.buffer;
                    server->renderState.blitSettings.dest.surface = server->bounceBufferMasterFramebuffer.buffer;
                }
            } else {
                server->renderState.blitSettings.output.surface = server->bounceBuffer.buffer;
                server->renderState.blitSettings.dest.surface = server->bounceBuffer.buffer;
            }
            server->renderState.blitSettings.source.surface = data->sourceSurface;
            server->renderState.blitSettings.colorBlend = data->colorBlend;
            server->renderState.blitSettings.alphaBlend = data->alphaBlend;
            if (nexus_p_reducible_blend(cmpDisplay, server->renderState.current)) {
                nexus_p_surface_composition_reduce_equation(&server->renderState.blitSettings.colorBlend, &server->renderState.blitSettings.alphaBlend);
            }
            if (data->colorKey.source.enabled || data->colorKey.dest.enabled) {
                server->renderState.blitSettings.colorKey.source = data->colorKey.source;
                server->renderState.blitSettings.colorKey.dest = data->colorKey.dest;
            }
            else {
                server->renderState.blitSettings.colorKey.source.enabled = false;
                server->renderState.blitSettings.colorKey.dest.enabled = false;
            }
            server->renderState.blitSettings.horizontalFilter = data->horizontalFilter;
            server->renderState.blitSettings.verticalFilter = data->verticalFilter;
            server->renderState.blitSettings.alphaPremultiplySourceEnabled = data->alphaPremultiplySourceEnabled;
            server->renderState.blitSettings.constantColor = data->constantColor;
            server->renderState.blitSettings.output.rect = data->outputRect;
            server->renderState.blitSettings.dest.rect = data->outputRect;
            server->renderState.blitSettings.source.rect = data->sourceRect;
            server->renderState.blitSettings.conversionMatrixEnabled = data->colorMatrixEnabled;
            if (data->colorMatrixEnabled) {
                server->renderState.blitSettings.conversionMatrix = data->colorMatrix;
            }
            rc = NEXUS_Graphics2D_Blit(server->gfx, &server->renderState.blitSettings);
            BDBG_MSG_TRACE(("blit [%d:%p] [%u,%u %ux%u]", cmpDisplay->index, (void *)server->renderState.blitSettings.source.surface,
            server->renderState.blitSettings.output.rect.x, server->renderState.blitSettings.output.rect.y,
            server->renderState.blitSettings.output.rect.width, server->renderState.blitSettings.output.rect.height));
            /* TODO: if (rc == NEXUS_GRAPHICS2D_QUEUE_FULL) */
            if (rc) rc = BERR_TRACE(rc);
        }
        server->renderState.step = taskCompleted;
        if(server->bounceBufferMasterFramebuffer.buffer) {
            NEXUS_Graphics2D_GetDefaultBlitSettings(&server->renderState.blitSettings);

            server->renderState.blitSettings.output.rect.x = 0;
            server->renderState.blitSettings.output.rect.y = 0;
            server->renderState.blitSettings.output.rect.width  = cmpDisplay->canvas.width;
            server->renderState.blitSettings.output.rect.height = cmpDisplay->canvas.height;
            server->renderState.blitSettings.source.rect.x = 0;
            server->renderState.blitSettings.source.rect.y = 0;
            server->renderState.blitSettings.source.rect.width = cmpDisplay->canvas.width;
            server->renderState.blitSettings.source.rect.height = cmpDisplay->canvas.height;
            server->renderState.blitSettings.output.surface = framebuffer->surface;
            server->renderState.blitSettings.source.surface = server->bounceBufferMasterFramebuffer.buffer;
            rc = NEXUS_Graphics2D_Blit(server->gfx, &server->renderState.blitSettings);
            BERR_TRACE(rc);
        }

        nexus_surface_compositor_update_slaves(server, cmpDisplay, framebuffer); /* render slaves without cursor */
        NEXUS_SurfaceCursor_P_RenderCursors(server, cmpDisplay, framebuffer);

        nexus_surface_compositor_p_blitter_start(server, NEXUS_P_SURFACEBLITTER_COMPOSITOR);
    }

    return;
}


bool nexus_surface_compositor_p_blitter_acquire(NEXUS_SurfaceCompositorHandle compositor, unsigned client)
{
    BDBG_ASSERT(client==NEXUS_P_SURFACEBLITTER_COMPOSITOR || client==NEXUS_P_SURFACEBLITTER_CURSOR || client==NEXUS_P_SURFACEBLITTER_CLIENT);
    if(compositor->state.blitter.active==0) {
        BDBG_MSG_TRACE(("%p:blitter_acquire %#x ", (void *)compositor, client));
        compositor->state.blitter.active = client;
        return true;
    } else {
        BDBG_MSG_TRACE(("%p:blitter busy %#x delayed (%#x,%#x)", (void *)compositor, compositor->state.blitter.active, client, compositor->state.blitter.delayed));
        compositor->state.blitter.delayed |= client;
        return false;
    }
}


void nexus_surface_compositor_p_blitter_release(NEXUS_SurfaceCompositorHandle compositor, unsigned client)
{
    BDBG_ASSERT(compositor->state.blitter.active == client);
    BDBG_MSG_TRACE(("%p:blitter_release %#x", (void *)compositor, client));
    BSTD_UNUSED(client);
    compositor->state.blitter.active = 0;
    return;
}

void nexus_surface_compositor_p_blitter_start(NEXUS_SurfaceCompositorHandle compositor, unsigned client)
{
    NEXUS_Error rc;
    BDBG_ASSERT(compositor->state.blitter.active == client);
    BSTD_UNUSED(client);
    rc = NEXUS_Graphics2D_Checkpoint(compositor->gfx, NULL);
    BDBG_MSG_TRACE(("%p:blitter_start %#x->%u", (void *)compositor, client, rc));
    if(rc==NEXUS_SUCCESS) {
        nexus_surface_compositor_p_blitter_complete(compositor);
        BDBG_ASSERT(compositor->state.blitter.delayed==0); /* XXX we don't support delayed without checkpoint, they can't be called directly since it could cause an endless recursion, and if it has to be handled then, perhaps via 0 time callout */
    } else if (rc == NEXUS_GRAPHICS2D_QUEUED) {
       /* nexus_surface_compositor_p_checkpoint would call nexus_surface_compositor_p_blitter_complete */
    } else {
        rc = BERR_TRACE(rc);
    }
    return;

}
