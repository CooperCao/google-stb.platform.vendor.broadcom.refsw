/***************************************************************************
 *  Copyright (C) 2016-2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "priv/nexus_core.h"
#include "priv/nexus_display_priv.h"
#include "priv/nexus_surface_priv.h"
#include "nexus_client_resources.h"

BDBG_MODULE(nexus_surface_client);

#define BDBG_MSG_TRACE(X) BDBG_MSG(X)
const NEXUS_BlendEquation NEXUS_SurfaceCompositor_P_ColorCopySource = {
        NEXUS_BlendFactor_eSourceColor, NEXUS_BlendFactor_eOne, false, NEXUS_BlendFactor_eZero, NEXUS_BlendFactor_eZero, false, NEXUS_BlendFactor_eZero
};

const NEXUS_BlendEquation NEXUS_SurfaceCompositor_P_AlphaCopySource = {
        NEXUS_BlendFactor_eSourceAlpha, NEXUS_BlendFactor_eOne, false, NEXUS_BlendFactor_eZero, NEXUS_BlendFactor_eZero, false, NEXUS_BlendFactor_eZero
};

void nexus_p_surface_composition_init(NEXUS_SurfaceComposition *composition)
{
    BKNI_Memset(composition, 0, sizeof(*composition));
    composition->position.width = 1920;
    composition->position.height = 1080;
    composition->visible = true;
    composition->zorder = 0;
    composition->constantColor = 0xFF000000;
    composition->colorBlend = NEXUS_SurfaceCompositor_P_ColorCopySource;
    composition->alphaBlend = NEXUS_SurfaceCompositor_P_AlphaCopySource;
    composition->horizontalFilter = NEXUS_Graphics2DFilterCoeffs_eAnisotropic;
    composition->verticalFilter = NEXUS_Graphics2DFilterCoeffs_eAnisotropic;
    composition->alphaPremultiplySourceEnabled = false;
    composition->colorMatrixEnabled = false;
    composition->virtualDisplay.width = 1920;
    composition->virtualDisplay.height = 1080;
    composition->contentMode = NEXUS_VideoWindowContentMode_eBox;
    return;
}

static void nexus_p_surface_client_init(NEXUS_SurfaceClientHandle client)
{
    unsigned i;

    BLST_SQ_INIT(&client->queue.push);
    BLST_SQ_INIT(&client->queue.recycle);
    BLST_SQ_INIT(&client->queue.free);
    client->queue.compositing = false;
    client->queue.last = false;
    client->queue.firstCompositingValid = false;
    client->queue.frameFractionalTime = 0;
    client->queue.frameFractionalTimeHysteresis = 0;
    client->state.client_type = client_type_idle;
    client->state.update_flags = 0;
    NEXUS_SURFACECLIENT_P_SURFACE_INIT(&client->state.current);
    client->state.left.visible = false;
    client->state.left.hidden = true;
    client->state.right.visible = false;
    client->state.right.hidden = true;
    client->set.dirty = false;
    client->set.updating = false;
    client->pending_displayed_callback = false;
    client->process_pending_displayed_callback = false;
    for(i=0;i<NEXUS_P_SURFACECMP_MAX_PUSH_SURFACES;i++) {
        NEXUS_SurfaceCompositor_P_PushElement *node = client->queue.elements + i;
        BLST_SQ_INSERT_TAIL(&client->queue.free, node, link);
    }
    client->settings.orientation = NEXUS_VideoOrientation_e2D;
    nexus_p_surface_composition_init(&client->settings.composition);
    NEXUS_CallbackDesc_Init(&client->settings.displayed);
    NEXUS_CallbackDesc_Init(&client->settings.recycled);
    NEXUS_CallbackDesc_Init(&client->settings.displayStatusChanged);
    NEXUS_CallbackDesc_Init(&client->settings.windowMoved);
    NEXUS_CallbackDesc_Init(&client->settings.vsync);

    return ;
}

NEXUS_SurfaceClientHandle NEXUS_SurfaceCompositor_CreateClient( NEXUS_SurfaceCompositorHandle server, NEXUS_SurfaceCompositorClientId client_id )
{
    NEXUS_SurfaceClientHandle client;
    NEXUS_Error rc;
    unsigned i, j;

    BDBG_OBJECT_ASSERT(server, NEXUS_SurfaceCompositor);

    BDBG_MSG(("create client %d", client_id));
    client = BKNI_Malloc(sizeof(*client));
    if (!client) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_SurfaceClient, client);
    client->server = server;
    client->client_id = client_id;
    client->type = NEXUS_SurfaceClient_eTopLevel;
    nexus_p_surface_composition_init(&client->serverSettings.composition);
    nexus_surfacecmp_p_insert_client(server, client, false);
    /* for backward compat, default ids in order */
    for (i=0;i<NEXUS_SURFACE_COMPOSITOR_VIDEO_WINDOWS;i++) {
        for (j=0;j<NEXUS_MAX_DISPLAYS;j++) {
            client->serverSettings.display[j].window[i].id = i;
        }
    }
    nexus_p_surface_client_init(client);
    NEXUS_OBJECT_REGISTER(NEXUS_SurfaceClient, client, Create);

    client->displayedCallback = NEXUS_TaskCallback_Create(client, NULL);
    if (!client->displayedCallback) {
        goto error;
    }
    client->recycledCallback = NEXUS_TaskCallback_Create(client, NULL);
    if (!client->recycledCallback) {
        goto error;
    }
    client->displayStatusChangedCallback = NEXUS_TaskCallback_Create(client, NULL);
    if (!client->displayStatusChangedCallback) {
        goto error;
    }
    client->windowMovedCallback = NEXUS_TaskCallback_Create(client, NULL);
    if (!client->windowMovedCallback) {
        goto error;
    }
    client->vsyncCallback = NEXUS_TaskCallback_Create(client, NULL);
    if (!client->vsyncCallback) {
        goto error;
    }

    rc = nexus_surface_compositor_p_verify_tunnel(server);
    if (rc) {
        rc = BERR_TRACE(rc);
        goto error;
    }

    nexus_p_surface_compositor_update_client(client, NEXUS_P_SURFACECLIENT_UPDATE_CLIENT);
    return client;

error:
    NEXUS_SurfaceCompositor_DestroyClient(client);
    return NULL;
}

static void NEXUS_SurfaceClient_P_Release( NEXUS_SurfaceClientHandle client )
{
    NEXUS_OBJECT_ASSERT(NEXUS_SurfaceClient, client);
    NEXUS_OBJECT_UNREGISTER(NEXUS_SurfaceClient, client, Destroy);
}

static void NEXUS_SurfaceClient_P_ParentFinalizer( NEXUS_SurfaceClientHandle client )
{
    NEXUS_OBJECT_ASSERT(NEXUS_SurfaceClient, client);

    NEXUS_SurfaceClient_Clear(client);

    BLST_Q_REMOVE(&client->server->clients, client, link);

    if (client->displayedCallback) {
        NEXUS_TaskCallback_Destroy(client->displayedCallback);
    }
    if (client->recycledCallback) {
        NEXUS_TaskCallback_Destroy(client->recycledCallback);
    }
    if (client->displayStatusChangedCallback) {
        NEXUS_TaskCallback_Destroy(client->displayStatusChangedCallback);
    }
    if (client->windowMovedCallback) {
        NEXUS_TaskCallback_Destroy(client->windowMovedCallback);
    }
    if (client->vsyncCallback) {
        NEXUS_TaskCallback_Destroy(client->vsyncCallback);
    }

    NEXUS_OBJECT_DESTROY(NEXUS_SurfaceClient, client);
    BKNI_Free(client);
    return;
}

static void NEXUS_SurfaceClient_P_Finalizer( NEXUS_SurfaceClientHandle client )
{
    NEXUS_OBJECT_ASSERT(NEXUS_SurfaceClient, client);
    switch (client->type) {
    case NEXUS_SurfaceClient_eVideoWindow:
        NEXUS_SurfaceClient_P_VideoWindowFinalizer(client);
        break;
    /* TODO: case NEXUS_SurfaceClient_eChild: */
    default:
        NEXUS_SurfaceClient_P_ParentFinalizer( client);
        break;
    }
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_SurfaceClient, NEXUS_SurfaceCompositor_DestroyClient);

NEXUS_SurfaceClientHandle NEXUS_SurfaceClient_Acquire( NEXUS_SurfaceCompositorClientId client_id )
{
    NEXUS_SurfaceCompositorHandle server;
    int rc;

    rc = NEXUS_CLIENT_RESOURCES_ACQUIRE(surfaceClient,IdList,client_id);
    if (rc) { rc = BERR_TRACE(rc); return NULL; }

    for (server = BLST_S_FIRST(&g_nexus_surfacecompositor_list); server; server = BLST_S_NEXT(server, link)) {
        NEXUS_SurfaceClientHandle client;
        BDBG_OBJECT_ASSERT(server, NEXUS_SurfaceCompositor);
        for (client = BLST_Q_FIRST(&server->clients); client; client = BLST_Q_NEXT(client, link)) {
            BDBG_OBJECT_ASSERT(client, NEXUS_SurfaceClient);
            if (client->client_id == client_id) {
                if (!client->acquired) {
                    nexus_p_surface_compositor_update_client(client, NEXUS_P_SURFACECLIENT_UPDATE_CLIENT);
                    client->acquired = true;
                    return client;
                }
                else {
                    BDBG_ERR(("client_id %d already acquired", client_id));
                    BERR_TRACE(NEXUS_NOT_AVAILABLE);
                    break;
                }
            }
        }
    }

    BDBG_ERR(("client_id %d not created", client_id));
    NEXUS_CLIENT_RESOURCES_RELEASE(surfaceClient,IdList,client_id);
    BERR_TRACE(NEXUS_NOT_AVAILABLE);
    return NULL;
}

void NEXUS_SurfaceClient_Release( NEXUS_SurfaceClientHandle client )
{
    BDBG_MSG_TRACE(("NEXUS_SurfaceClient_Release:%p", (void *)client));
    BDBG_OBJECT_ASSERT(client, NEXUS_SurfaceClient);
    
    if (client->type == NEXUS_SurfaceClient_eChild) {
        NEXUS_SurfaceClient_DestroyChild(client);
        return;
    }
    if (client->type == NEXUS_SurfaceClient_eVideoWindow) {
        NEXUS_SurfaceClient_ReleaseVideoWindow(client);
        return;
    }
    
    NEXUS_TaskCallback_Set(client->displayedCallback, NULL);
    NEXUS_TaskCallback_Set(client->recycledCallback, NULL);
    NEXUS_TaskCallback_Set(client->displayStatusChangedCallback, NULL);
    NEXUS_TaskCallback_Set(client->windowMovedCallback, NULL);
    NEXUS_TaskCallback_Set(client->vsyncCallback, NULL);
    client->acquired = false;
    NEXUS_CLIENT_RESOURCES_RELEASE(surfaceClient,IdList,client->client_id);
    NEXUS_SurfaceClient_Clear(client);
    nexus_p_surface_client_init(client);
    return;
}

NEXUS_SurfaceClientHandle NEXUS_SurfaceClient_CreateChild( NEXUS_SurfaceClientHandle parent_handle )
{
    NEXUS_SurfaceClientHandle client;
    BDBG_OBJECT_ASSERT(parent_handle, NEXUS_SurfaceClient);
    if (parent_handle->type != NEXUS_SurfaceClient_eTopLevel) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }
    client = NULL; /* TODO */
    return client;
}

void NEXUS_SurfaceClient_DestroyChild( NEXUS_SurfaceClientHandle client )
{
    BDBG_OBJECT_ASSERT(client, NEXUS_SurfaceClient);
    if (client->type != NEXUS_SurfaceClient_eChild) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return;
    }
}

#if NEXUS_SURFACE_COMPOSITOR_P_CHECK_IMMUTABLE
/* use spares CRC to reduce computation complexity */
/* do rcr on 32 bytes and then skip X_STEP*32 bytes */
#define NEXUS_SURFACE_COMPOSITOR_P_CRC_X_STEP       8
/* do crc on every Y_STEP line */
#define NEXUS_SURFACE_COMPOSITOR_P_CRC_Y_STEP       8

BTRC_MODULE(surface_compositor_crc,ENABLE);

BSTD_INLINE unsigned
b_crc_update_uint8(unsigned seed, unsigned data)
{
    unsigned temp;
    unsigned result;

    /* x16 + x12 + x5 + 1 (CRC-16-CCITT) */
    temp = data ^ (seed >> 8);
    temp ^= (temp >> 4);
    result = (seed << 8) ^ temp ^ (temp << 5) ^ (temp << 12);
    return result&0xFFFF;
}

BSTD_INLINE unsigned
b_crc_update_uint16(unsigned seed, unsigned data)
{
    seed = b_crc_update_uint8(seed, data>>8);
    seed = b_crc_update_uint8(seed, data&0xFF);
    return seed;
}

BSTD_INLINE unsigned
b_crc_update_uint32(unsigned seed, unsigned data)
{
    seed = b_crc_update_uint16(seed, data>>16);
    seed = b_crc_update_uint16(seed, data&0xFFFF);
    return seed;
}

static unsigned NEXUS_SurfaceCompositor_P_SurfaceCrc(const NEXUS_Surface_P_ClientSurface *clientSurface,  NEXUS_SurfaceCreateSettings *createSettings)
{
    unsigned y;
    unsigned seed = 0xFFFF;
    unsigned xsteps;
    NEXUS_SurfaceMemory mem;
    NEXUS_SurfaceHandle surface = clientSurface->surface;

    BTRC_TRACE(surface_compositor_crc, START);

    NEXUS_Surface_GetMemory(surface, &mem);
    NEXUS_Surface_Flush(surface);
    xsteps = (clientSurface->bpp*createSettings->width+4*(1-NEXUS_SURFACE_COMPOSITOR_P_CRC_X_STEP))/(4*NEXUS_SURFACE_COMPOSITOR_P_CRC_X_STEP*8*sizeof(NEXUS_Pixel));
    for(y=0;y<createSettings->height;y+=NEXUS_SURFACE_COMPOSITOR_P_CRC_Y_STEP) {
        unsigned x;
        const NEXUS_Pixel *pixels = (NEXUS_Pixel *)((uint8_t *)mem.buffer + y*mem.pitch);

        for(x=0;x<xsteps;x++,pixels+=NEXUS_SURFACE_COMPOSITOR_P_CRC_X_STEP*4) {
            /* unroll loop to process one cache line at a time */
            seed = b_crc_update_uint32(seed, pixels[0]);
            seed = b_crc_update_uint32(seed, pixels[1]);
            seed = b_crc_update_uint32(seed, pixels[2]);
            seed = b_crc_update_uint32(seed, pixels[3]);
        }
    }
    BDBG_MSG_TRACE(("surface %p(%p) %ux%u crc %#x", clientSurface->surface, mem.buffer, createSettings->width, createSettings->height, seed));
    BTRC_TRACE(surface_compositor_crc, STOP);
    return seed;
}

static void NEXUS_SURFACECLIENT_P_ASSIGN_SURFACE(NEXUS_SurfaceClientHandle client, NEXUS_Surface_P_ClientSurface *clientSurface, NEXUS_SurfaceHandle appSurface)
{
    const NEXUS_PixelFormatConvertInfo *pixelInfo;
    NEXUS_SurfaceCreateSettings createSettings;
    BSTD_UNUSED(client);

    clientSurface->surface = appSurface;
    clientSurface->verify = true;
    NEXUS_Surface_GetCreateSettings(appSurface, &createSettings);
    pixelInfo = NEXUS_P_PixelFormat_GetConvertInfo_isrsafe(createSettings.pixelFormat);
    BDBG_ASSERT(pixelInfo);
    clientSurface->bpp = pixelInfo->info.bpp;
    clientSurface->crc = NEXUS_SurfaceCompositor_P_SurfaceCrc(clientSurface, &createSettings);
    BDBG_MSG_TRACE(("NEXUS_SURFACECLIENT_P_ASSIGN_SURFACE client (%p,%u) surface:%p crc:%#x", client, client->client_id, clientSurface->surface, clientSurface->crc));
    return;
}

void nexus_surfaceclient_p_verify_surface(NEXUS_SurfaceClientHandle client, const NEXUS_Surface_P_ClientSurface *clientSurface, const char *filename, unsigned lineno)
{
    BSTD_UNUSED(client);
    if(clientSurface->verify && clientSurface->surface) {
        NEXUS_SurfaceCreateSettings createSettings;
        unsigned crc;

        NEXUS_Surface_GetCreateSettings(clientSurface->surface, &createSettings);
        crc = NEXUS_SurfaceCompositor_P_SurfaceCrc(clientSurface, &createSettings);
        if(crc != clientSurface->crc) {
            BDBG_WRN(("NEXUS_SURFACECLIENT_P_SURFACE_VERIFY failed, client (%p,%u) surface:%p crc:%#x,%#x %s:%u", client, client->client_id, clientSurface->surface, crc, clientSurface->crc, filename, lineno));
        }
    }
    return;
}

#else
#define NEXUS_SURFACECLIENT_P_ASSIGN_SURFACE(client, clientSurface, appSurface) (clientSurface)->surface = appSurface
#endif

NEXUS_Error NEXUS_SurfaceClient_P_CopySetSurface( NEXUS_SurfaceClientHandle client, NEXUS_SurfaceHandle surface )
{
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_Surface_GetCreateSettings(surface, &createSettings);
    client->state.clientRegion.width = createSettings.width;
    client->state.clientRegion.height = createSettings.height;

    /* if different, destroy and recreate */
    if (client->state.client_type == client_type_set && BKNI_Memcmp(&client->set.serverCreateSettings, &createSettings, sizeof(createSettings))!=0) {
        BDBG_ASSERT(client->set.serverSurface);
        NEXUS_Surface_Destroy(client->set.serverSurface);
        client->set.serverSurface = NULL;
        client->state.client_type = client_type_idle;
    }
    if(client->set.serverSurface==NULL) {
        client->set.serverCreateSettings = createSettings;
        if (client->server->secureFramebuffer) {
            client->set.serverCreateSettings.heap = client->server->settings.display[0].framebuffer.heap;
        }
        client->set.serverSurface = NEXUS_Surface_Create(&client->set.serverCreateSettings);
        if (!client->set.serverSurface) {
            return BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
        }
        client->state.client_type = client_type_set;
    }
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_SurfaceClient_SetSurface( NEXUS_SurfaceClientHandle client, NEXUS_SurfaceHandle surface )
{
    NEXUS_Error rc;
    unsigned flags = NEXUS_P_SURFACECLIENT_UPDATE_SOURCE;

    BDBG_OBJECT_ASSERT(client, NEXUS_SurfaceClient);
    BDBG_ASSERT(surface);

    BDBG_MSG_TRACE(("SetSurface:%p %u %p", (void *)client, client->client_id, (void *)surface));
    if (client->type == NEXUS_SurfaceClient_eVideoWindow) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    if(client->state.client_type != client_type_set && client->state.client_type != client_type_idle) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
     /* if the previous blit is not complete (if client->set.serverSurface is being read by M2MC HW), we need to fail this call.
    app must wait for recycle event. */
    if (client->set.dirty) {
        BDBG_ERR(("app must wait for recycled callback to avoid tearing. NEXUS_SurfaceClient_SetSurface rejected."));
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    rc = NEXUS_SurfaceClient_P_CopySetSurface(client, surface);
    if (rc) return BERR_TRACE(rc);
    flags |= NEXUS_P_SURFACECLIENT_UPDATE_VISIBLE;

    BDBG_ASSERT(client->state.client_type == client_type_set);
    if(client->set.surface.surface!=surface) {
        if(client->set.surface.surface) {
            NEXUS_OBJECT_RELEASE(client, NEXUS_Surface, client->set.surface.surface);
        }
        NEXUS_OBJECT_ACQUIRE(client, NEXUS_Surface, surface);
        NEXUS_SURFACECLIENT_P_ASSIGN_SURFACE(client, &client->set.surface, surface);
    }
    client->update_info.updateRect.x = 0;
    client->update_info.updateRect.y = 0;
    client->update_info.updateRect.width = client->state.clientRegion.width;
    client->update_info.updateRect.height = client->state.clientRegion.height;
    nexus_p_surface_compositor_update_client(client, flags);
    client->set.cnt++;
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_SurfaceClient_UpdateSurface( NEXUS_SurfaceClientHandle client, const NEXUS_Rect *pUpdateRect )
{
    BDBG_OBJECT_ASSERT(client, NEXUS_SurfaceClient);
    BDBG_MSG_TRACE(("UpdateSurface:%p %u", (void *)client, client->client_id));
    if (client->type == NEXUS_SurfaceClient_eVideoWindow) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if(client->state.client_type != client_type_set) {
        BDBG_ERR(("must call NEXUS_SurfaceClient_SetSurface first"));
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    /* if the previous blit is not complete (if client->set.serverSurface is being read by M2MC HW), we need to fail this call.
    app must wait for recycle event. */
    if (client->set.dirty) {
        BDBG_ERR(("app must wait for recycle callback to avoid tearing. NEXUS_SurfaceClient_UpdateSurface rejected."));
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    if(pUpdateRect==NULL) {
        client->update_info.updateRect.x = 0;
        client->update_info.updateRect.y = 0;
        client->update_info.updateRect.width = client->state.clientRegion.width;
        client->update_info.updateRect.height = client->state.clientRegion.height;
    } else {
        if(pUpdateRect->x < 0 || pUpdateRect->y < 0 || (pUpdateRect->x + pUpdateRect->width) > client->state.clientRegion.width || (pUpdateRect->y + pUpdateRect->height) > client->state.clientRegion.height) {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        client->update_info.updateRect = *pUpdateRect;
    }
    NEXUS_SURFACECLIENT_P_ASSIGN_SURFACE(client, &client->set.surface, client->set.surface.surface);
    nexus_p_surface_compositor_update_client(client, NEXUS_P_SURFACECLIENT_UPDATE_SOURCE);
    client->set.cnt++;
    return NEXUS_SUCCESS;
}

static void NEXUS_SurfaceClient_P_ClearRecycleQueue(NEXUS_SurfaceClientHandle client)
{
    struct NEXUS_SurfaceCompositor_P_PushQueue *queue = &client->queue.recycle;
    NEXUS_SurfaceCompositor_P_PushElement *node;
    while ((node = BLST_SQ_FIRST(queue))) {
        BLST_SQ_REMOVE_HEAD(queue, link);
        BLST_SQ_INSERT_TAIL(&client->queue.free, node, link);
    }
    return;
}

static void NEXUS_SurfaceClient_P_ClearPushQueue(NEXUS_SurfaceClientHandle client)
{
    NEXUS_SurfaceCompositor_P_PushElement *node;
    struct NEXUS_SurfaceCompositor_P_PushQueue *queue = &client->queue.push;
    while ((node = BLST_SQ_FIRST(queue))) {
        NEXUS_OBJECT_RELEASE(client, NEXUS_Surface, node->surface.surface);
        BLST_SQ_REMOVE_HEAD(queue, link);
        BLST_SQ_INSERT_TAIL(&client->queue.free, node, link);
    }
    return;
}

static void NEXUS_SurfaceClient_P_RecycleOld(NEXUS_SurfaceClientHandle client, bool infront)
{
    NEXUS_SurfaceCompositor_P_PushElement *node;
    bool recycled = false;

#if (BCHP_CHIP == 11360)
    /* There is no FramebufferCallback for Cygnus, so queue.last is set here to enable recycling */
    client->queue.last = true;
#endif

    if(!infront) {
        if(client->queue.last && !client->queue.compositing) {
            NEXUS_SurfaceCompositor_P_PushElement *node = BLST_SQ_FIRST(&client->queue.push);
            if(node) {
                recycled = true;
                BLST_SQ_REMOVE_HEAD(&client->queue.push, link);
                BLST_SQ_INSERT_TAIL(&client->queue.recycle, node, link);
                NEXUS_OBJECT_RELEASE(client, NEXUS_Surface, node->surface.surface);
                BDBG_MSG_TRACE(("RecycleOld:%#lx recycle %#lx ...", (unsigned long)client, (unsigned long)node));
            }
        }
    } else {
        NEXUS_SurfaceCompositor_P_PushElement *first_node=NULL;
        if(client->queue.compositing) {
            first_node = BLST_SQ_FIRST(&client->queue.push);
            if(first_node) {
                BLST_SQ_REMOVE_HEAD(&client->queue.push, link);
            }
        }
        while ((node = BLST_SQ_FIRST(&client->queue.push))) {
            recycled = true;
            BLST_SQ_REMOVE_HEAD(&client->queue.push, link);
            BLST_SQ_INSERT_TAIL(&client->queue.recycle, node, link);
            NEXUS_OBJECT_RELEASE(client, NEXUS_Surface, node->surface.surface);
            BDBG_MSG_TRACE(("RecycleOld:%#lx recycle %#lx ...", (unsigned long)client, (unsigned long)node));
        }
        if(first_node) {
            BLST_SQ_INSERT_HEAD(&client->queue.push, first_node, link);
        }
    }
    if(recycled) {
        NEXUS_TaskCallback_Fire(client->recycledCallback);
    }
    return;
}

static void NEXUS_SurfaceClient_P_InsertPushQueue(NEXUS_SurfaceClientHandle client, NEXUS_SurfaceCompositor_P_PushElement *node, NEXUS_SurfaceHandle surface)
{
    NEXUS_OBJECT_ACQUIRE(client, NEXUS_Surface, surface);
    NEXUS_SURFACECLIENT_P_ASSIGN_SURFACE(client, &node->surface, surface);
    BLST_SQ_REMOVE_HEAD(&client->queue.free, link);
    BLST_SQ_INSERT_TAIL(&client->queue.push, node, link);
    client->queue.last = false;
    return;
}



NEXUS_Error NEXUS_SurfaceClient_PushSurface( NEXUS_SurfaceClientHandle client, NEXUS_SurfaceHandle surface, const NEXUS_Rect *pUpdateRect, bool infront )
{
    NEXUS_SurfaceCompositor_P_PushElement *node;
    NEXUS_SurfaceCreateSettings createSettings;
    unsigned flags = NEXUS_P_SURFACECLIENT_UPDATE_SOURCE;

    BDBG_OBJECT_ASSERT(client, NEXUS_SurfaceClient);
    if (client->type == NEXUS_SurfaceClient_eVideoWindow) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    if(client->state.client_type != client_type_push && client->state.client_type != client_type_idle) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    BDBG_MSG_TRACE(("PushSurface:%#lx %#lx %s", (unsigned long)client, (unsigned long)surface, infront?"infront":""));

    NEXUS_Surface_GetCreateSettings(surface, &createSettings);
    if(pUpdateRect) {
        if(pUpdateRect->x < 0 || pUpdateRect->y < 0 || (pUpdateRect->x + pUpdateRect->width) > createSettings.width || (pUpdateRect->y + pUpdateRect->height) > createSettings.height) {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        if(pUpdateRect->x > 0 || pUpdateRect->y > 0 || pUpdateRect->width < createSettings.width || pUpdateRect->height < createSettings.height) {
            /* Until we have partial scaled blit capability, push can't support update rect */
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
    }

    if (BLST_SQ_FIRST(&client->queue.push)==NULL) {
        flags |= NEXUS_P_SURFACECLIENT_UPDATE_VISIBLE;
    }
    NEXUS_SurfaceClient_P_RecycleOld(client, infront);

    node = BLST_SQ_FIRST(&client->queue.free);
    if (!node) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    client->state.client_type = client_type_push;
    NEXUS_SurfaceClient_P_InsertPushQueue(client, node, surface);

    client->state.clientRegion.width = createSettings.width;
    client->state.clientRegion.height = createSettings.height;
    if(pUpdateRect==NULL) {
        client->update_info.updateRect.x = 0;
        client->update_info.updateRect.y = 0;
        client->update_info.updateRect.width = createSettings.width;
        client->update_info.updateRect.height = createSettings.height;
    } else {
        client->update_info.updateRect = *pUpdateRect;
    }
    nexus_p_surface_compositor_update_client(client, flags);
    client->queue.cnt++;
    return NEXUS_SUCCESS;
}

static void NEXUS_SurfaceClient_P_RecycleFromQueue(NEXUS_SurfaceClientHandle client, NEXUS_SurfaceHandle *recycled, size_t num_entries, size_t *num_returned)
{
    unsigned i;
    NEXUS_SurfaceCompositor_P_PushElement *node;
    for (i=0;i<num_entries;i++) {
        node = BLST_SQ_FIRST(&client->queue.recycle);
        if (!node) break;
        NEXUS_SURFACECLIENT_P_SURFACE_VERIFY(client, &node->surface);
        recycled[i] = node->surface.surface;
        BLST_SQ_REMOVE_HEAD(&client->queue.recycle, link);
        NEXUS_SURFACECLIENT_P_SURFACE_INIT(&node->surface);
        BLST_SQ_INSERT_HEAD(&client->queue.free, node, link);
    }
    *num_returned = i;
    return;
}

NEXUS_Error NEXUS_SurfaceClient_RecycleSurface( NEXUS_SurfaceClientHandle client, NEXUS_SurfaceHandle *recycled, size_t num_entries, size_t *num_returned )
{
    BDBG_OBJECT_ASSERT(client, NEXUS_SurfaceClient);

    BDBG_MSG_TRACE(("NEXUS_SurfaceClient_RecycleSurface:%p", (void *)client));
    BDBG_ASSERT(num_returned);
    if (client->type == NEXUS_SurfaceClient_eVideoWindow) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    if(client->state.client_type!=client_type_push) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    NEXUS_SurfaceClient_P_RecycleFromQueue(client, recycled, num_entries, num_returned);
    BDBG_MSG_TRACE(("NEXUS_SurfaceClient_RecycleSurface:%p -> %u", (void *)client, (unsigned)*num_returned));
    return NEXUS_SUCCESS;
}

void NEXUS_SurfaceClient_Clear( NEXUS_SurfaceClientHandle client )
{
    BDBG_OBJECT_ASSERT(client, NEXUS_SurfaceClient);
    if (client->type == NEXUS_SurfaceClient_eVideoWindow) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return;
    }
    switch(client->state.client_type) {
    case client_type_set:
        BDBG_ASSERT(client->set.serverSurface);
        BDBG_ASSERT(client->set.surface.surface);
        NEXUS_OBJECT_RELEASE(client, NEXUS_Surface, client->set.surface.surface);
        NEXUS_Surface_Destroy(client->set.serverSurface);
        client->set.surface.surface = NULL;
        client->set.serverSurface = NULL;
        client->set.dirty = false;
        client->set.updating = false;
        break;
    case client_type_push:
        /* recycle multibuffering queue */
        NEXUS_SurfaceClient_P_ClearRecycleQueue(client);
        NEXUS_SurfaceClient_P_ClearPushQueue(client);
        client->queue.compositing = false;
        client->queue.last = false;
        if (client->server->bypass_compose.client == client) {
            client->server->bypass_compose.client = NULL;
            nexus_surface_compositor_p_realloc_framebuffers(client->server);
        }
        break;
    default:
        break;
    }

    client->state.client_type = client_type_idle;

    nexus_p_surface_compositor_update_client(client, NEXUS_P_SURFACECLIENT_UPDATE_VISIBLE);
    client->set.cnt = 0;
    client->queue.cnt = 0;

    return;
}

void NEXUS_SurfaceClient_GetSettings( NEXUS_SurfaceClientHandle client, NEXUS_SurfaceClientSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(client, NEXUS_SurfaceClient);
    *pSettings = client->settings;
}

NEXUS_Error NEXUS_SurfaceClient_SetSettings( NEXUS_SurfaceClientHandle client, const NEXUS_SurfaceClientSettings *pSettings )
{
    bool changeChildZorder = false;
    BDBG_OBJECT_ASSERT(client, NEXUS_SurfaceClient);
    BDBG_ASSERT(pSettings);
    if(client->state.client_type != client_type_idle) {
        /* certain configuration can't be changed when rendering is active */
        if(pSettings->orientation != client->settings.orientation) {
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
    }
    if (client->type == NEXUS_SurfaceClient_eTopLevel) {
        if (client->settings.composition.zorder != pSettings->composition.zorder ||
            client->settings.composition.position.x != pSettings->composition.position.x ||
            client->settings.composition.position.y != pSettings->composition.position.y ||
            client->settings.composition.position.width != pSettings->composition.position.width ||
            client->settings.composition.position.height != pSettings->composition.position.height ||
            client->settings.composition.visible != pSettings->composition.visible)
        {
            /* app should use NxClient_SetSurfaceClientComposition instead */
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
        NEXUS_TaskCallback_Set(client->displayedCallback, &pSettings->displayed);
        NEXUS_TaskCallback_Set(client->recycledCallback, &pSettings->recycled);
        NEXUS_TaskCallback_Set(client->displayStatusChangedCallback, &pSettings->displayStatusChanged);
        NEXUS_TaskCallback_Set(client->windowMovedCallback, &pSettings->windowMoved);
        NEXUS_TaskCallback_Set(client->vsyncCallback, &pSettings->vsync);
    }
    else {
        changeChildZorder = client->settings.composition.zorder != pSettings->composition.zorder;
    }
    if(client->state.client_type == client_type_tunnel) {
        /* can't change orientation of active tunneled client */
        if(pSettings->orientation != client->settings.orientation) { return BERR_TRACE(NEXUS_NOT_SUPPORTED); }
    }

    client->settings = *pSettings;

    if (changeChildZorder) {
        nexus_surfacecmp_p_insert_child(client->parent, client, true);
    }
    if (client->type == NEXUS_SurfaceClient_eVideoWindow) {
        nexus_surfaceclient_request_setvideo(client);
    }
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_SurfaceClient_GetStatus( NEXUS_SurfaceClientHandle client, NEXUS_SurfaceClientStatus *pStatus )
{
    const struct NEXUS_SurfaceCompositorDisplay *display;

    BDBG_OBJECT_ASSERT(client, NEXUS_SurfaceClient);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    pStatus->id = client->client_id;
    display = client->server->display[0];

    if(display->display) {
        pStatus->display.framebuffer = display->canvas;
        pStatus->display.screen = display->formatInfo.canvas;
        pStatus->display.enabled3d = display->formatInfo.orientation != NEXUS_VideoOrientation_e2D;
        pStatus->display.numFramebuffers = client->server->settings.display[0].framebuffer.number;
    }

    pStatus->display.enabled = client->server->settings.enabled;

    switch (client->type) {
    case NEXUS_SurfaceClient_eVideoWindow:
        /* window position in display coordinates, not framebuffer coordinates */
        if (client->window[0]) {
            NEXUS_VideoWindowSettings settings;
            NEXUS_VideoWindow_GetSettings(client->window[0], &settings);
            pStatus->position = settings.position;
        }
        pStatus->child = true;
        pStatus->parentId = client->parent->client_id;
        break;
    case NEXUS_SurfaceClient_eChild:
        pStatus->child = true;
        pStatus->parentId = client->parent->client_id;
        /* TODO: */
        break;
    default:
        pStatus->position = client->state.framebufferRect; /* not clipped */
        break;
    }

    return 0;
}

NEXUS_Error NEXUS_SurfaceClient_PublishSurface( NEXUS_SurfaceClientHandle client )
{
    NEXUS_SurfaceHandle surface = NULL;

    BDBG_OBJECT_ASSERT(client, NEXUS_SurfaceClient);
    if (client->type == NEXUS_SurfaceClient_eVideoWindow) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    /* determine which surface can be published */
    switch(client->state.client_type) {
    case client_type_idle:
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        break;
    case client_type_set:
        surface = client->set.surface.surface;
        break;
    case client_type_push:
        {
            NEXUS_SurfaceCompositor_P_PushElement *node;
            NEXUS_SurfaceCreateSettings createSettings;

            node = BLST_SQ_FIRST(&client->queue.push);
            if (node==NULL) {
                return BERR_TRACE(NEXUS_NOT_SUPPORTED);
            }
            surface = node->surface.surface;
            NEXUS_Surface_GetCreateSettings(surface, &createSettings);
            client->state.clientRegion.width = createSettings.width;
            client->state.clientRegion.height = createSettings.height;
            break;
        }
    default:
        break;
    }
    if(!surface) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    client->published.surface = surface;
    client->update_info.updateRect.x = 0;
    client->update_info.updateRect.y = 0;
    client->update_info.updateRect.width = client->state.clientRegion.width;
    client->update_info.updateRect.height = client->state.clientRegion.height;
    nexus_p_surface_compositor_update_client(client, NEXUS_P_SURFACECLIENT_UPDATE_PUBLISH);

    return NEXUS_SUCCESS;
}
