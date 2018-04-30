/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
* API Description:
*
***************************************************************************/

#include "CNexus.h"
#include "bkni_multi.h"
#include <stdlib.h>
#include <stdio.h>

/* forward declaration of member functions*/

static NEXUS_Error _nexusCreateInputEvent(CNexus* self, CNexusInputEvent** ev);
static NEXUS_Error _nexusCreateDisplayLayer(CNexus* self, NEXUS_SurfaceCompositorClientId id, CNexusDisplayLayer **displayLayerOut);
static void _nexusAddRef(CNexus *self);
static void _nexusRelease(CNexus *self);
static NEXUS_Error _nexusDisplayEnableCursor(CNexusDisplayLayer*, int);
static void _nexusDisplayLayerAddRef(CNexusDisplayLayer*);
static void _nexusDisplayLayerRelease(CNexusDisplayLayer*);
static void _nexusSurfaceAddRef(CNexusSurface *self);
static void _nexusSurfaceRelease(CNexusSurface *self);
static NEXUS_PixelFormat _nexusSurfaceGetPixelFormat(CNexusSurface *self);
static int _nexusSurfaceGetWidth(CNexusSurface *self);
static int _nexusSurfaceGetHeight(CNexusSurface *self);
static void _nexusSurfaceGetRectangle(CNexusSurface *self, NEXUS_Rect *rect);
static bool _nexusSurfaceIsAlphaPremultipliedSourceEnabled(CNexusSurface*);
static void _nexusSurfaceUpdate(CNexusSurface *self, bool waitForSync);
static void _nexusSurfaceMove(CNexusSurface *self, int dx, int dy);
static void _nexusSurfaceBlit(CNexusSurface *self, CNexusSurface *source, NEXUS_PorterDuffOp op, NEXUS_Rect*, int, int);
static NEXUS_Error _nexusSurfaceSync(CNexusSurface *self);
static NEXUS_Error _nexusSurfaceLock(CNexusSurface *self, void **data, int *pitch);
static void _nexusSurfaceFlush(CNexusSurface *self);
static void _nexusSurfaceUnlock(CNexusSurface *self);
static NEXUS_Pixel _nexusSurfaceConvertColor(CNexusSurface *self, uint8_t colorComponents[4]);
static void _nexusSurfaceClear(CNexusSurface *self, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
static void _nexusSurfaceClipRectangles(CNexusSurface *self, NEXUS_Rect *r, int *nbRect);
static NEXUS_Error _nexusSurfaceFillRectangles(CNexusSurface *self, uint8_t colorComponents[4], NEXUS_PorterDuffOp op, NEXUS_Rect *rect, int nbRect);
static NEXUS_Error _nexusSurfaceFillPorterDuffRectangle(CNexusSurface *self, uint8_t colorComponents[4], NEXUS_PorterDuffOp op, NEXUS_Rect *rect);
static NEXUS_Error _nexusCreateSurface(CNexus *self, NEXUS_PixelFormat format, uint16_t width, uint16_t height, CNexusSurface **s);
static NEXUS_Error _nexusCreateDisplaySurface(CNexusDisplayLayer *self, NEXUS_PixelFormat format, uint16_t width, uint16_t height, CNexusSurface **s);
static void _nexusDisplayLayerCallback(void *data, int ev);
static void _nexusSurfaceCallback(void *data, int ev);

static NEXUS_Error _nexusWaitEvent(CNexusInputEvent *self);
static NEXUS_Error _nexusGetEvent(CNexusInputEvent *self, NEXUS_IrInputEvent *ev);
static void irCallback(void *pParam, int iParam);

static void _nexusDisplayLayerCallback(void *data, int ev) {
    BSTD_UNUSED(ev);
    BKNI_SetEvent((BKNI_EventHandle)data);

}

NEXUS_Error NexusCreate(CNexus **nexusOut)
{
    NEXUS_Error rc;
    CNexus *nexus;
    *nexusOut = NULL;
    nexus = (CNexus *)calloc(1, sizeof(*nexus));
    if (!nexus) {
        rc = NEXUS_OUT_OF_DEVICE_MEMORY;
        goto error;
    }

    nexus->addRef = _nexusAddRef;
    nexus->release = _nexusRelease;
    nexus->createInputEvent = _nexusCreateInputEvent;
    nexus->createDisplayLayer = _nexusCreateDisplayLayer;
    nexus->createSurface = _nexusCreateSurface;

    BKNI_CreateEvent(&nexus->checkpointEvent);
    BKNI_CreateEvent(&nexus->packetSpaceAvailableEvent);
    nexus->gfx = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, NULL);
    NEXUS_Graphics2D_GetSettings(nexus->gfx, &nexus->gfxSettings);
    nexus->gfxSettings.colorFormatType = NEXUS_ColorFormatType_eStandardFormat;
    nexus->gfxSettings.checkpointCallback.callback = _nexusSurfaceCallback;
    nexus->gfxSettings.checkpointCallback.context = nexus->checkpointEvent;
    nexus->gfxSettings.packetSpaceAvailable.callback = _nexusSurfaceCallback;
    nexus->gfxSettings.packetSpaceAvailable.context = nexus->packetSpaceAvailableEvent;
    rc = NEXUS_Graphics2D_SetSettings(nexus->gfx, &nexus->gfxSettings);
    if (rc != NEXUS_SUCCESS) {
        goto error;
    }

    nexus->ref = 1;
    *nexusOut = nexus;
    return NEXUS_SUCCESS;

error:
    if (nexus) {
        free(nexus);
    }
    return rc;
}

static void _nexusAddRef(CNexus *self) {

}
static void _nexusRelease(CNexus *self) {
}

static NEXUS_Error _nexusCreateDisplayLayer(CNexus *self, NEXUS_SurfaceCompositorClientId id, CNexusDisplayLayer **displayLayerOut) {
    NEXUS_Error rc;
    CNexusDisplayLayer *layer;
    *displayLayerOut = NULL;
    layer = (CNexusDisplayLayer *)calloc(1, sizeof(*layer));
    if (!layer) {
        rc = NEXUS_OUT_OF_DEVICE_MEMORY;
        goto error;
    }

    layer->ctx = self;
    layer->addRef = _nexusDisplayLayerAddRef;
    layer->release = _nexusDisplayLayerRelease;
    layer->enableCursor = _nexusDisplayEnableCursor;
    layer->createSurface = _nexusCreateDisplaySurface;

    layer->surfaceClient = NEXUS_SurfaceClient_Acquire(id);
    if (!layer->surfaceClient) {
        rc = NEXUS_OUT_OF_DEVICE_MEMORY;
        goto error;
    }

    BKNI_CreateEvent(&layer->displayedEvent);

    NEXUS_SurfaceClient_GetSettings(layer->surfaceClient, &layer->surfaceClientSettings);
    layer->surfaceClientSettings.displayed.callback = _nexusDisplayLayerCallback;
    layer->surfaceClientSettings.displayed.context = layer->displayedEvent;
    rc = NEXUS_SurfaceClient_SetSettings(layer->surfaceClient, &layer->surfaceClientSettings);
    if (rc != NEXUS_SUCCESS) {
        goto error;
    }

    layer->ref = 1;
    *displayLayerOut = layer;
    return NEXUS_SUCCESS;

error:
    if (layer) {
        free(layer);
    }
    return rc;
}

static NEXUS_Error _nexusDisplayEnableCursor(CNexusDisplayLayer* self, int enable) {
    return NEXUS_SUCCESS;
}

static void _nexusSurfaceCallback(void *data, int ev) {
    BSTD_UNUSED(ev);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static void _nexusSurfaceClear(CNexusSurface* self, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    NEXUS_Rect rect;
    uint8_t colors[] = { a, r, g, b };
    self->getRectangle(self, &rect);
    _nexusSurfaceFillPorterDuffRectangle(self, colors, NEXUS_PorterDuffOp_eSrc, &rect);
}

static NEXUS_Error _nexusCreateSurface(CNexus *self, NEXUS_PixelFormat format, uint16_t width, uint16_t height, CNexusSurface **surfaceOut)
{
    CNexusSurface *surface;
    NEXUS_Error rc;
    *surfaceOut = NULL;
    surface = (CNexusSurface *)calloc(1, sizeof(*surface));
    if (!surface) {
        rc = NEXUS_OUT_OF_DEVICE_MEMORY;
        goto error;
    }

    surface->ctx = self;
    surface->layer = NULL;
    surface->addRef = _nexusSurfaceAddRef;
    surface->release = _nexusSurfaceRelease;
    surface->clear = _nexusSurfaceClear;
    surface->getPixelFormat = _nexusSurfaceGetPixelFormat;
    surface->getWidth = _nexusSurfaceGetWidth;
    surface->getHeight = _nexusSurfaceGetHeight;
    surface->getRectangle = _nexusSurfaceGetRectangle;
    surface->isAlphaPremultipliedSourceEnabled = _nexusSurfaceIsAlphaPremultipliedSourceEnabled;
    surface->clipRectangles = _nexusSurfaceClipRectangles;
    surface->fillRectangles = _nexusSurfaceFillRectangles;
    surface->convertColor = _nexusSurfaceConvertColor;
    surface->blit = _nexusSurfaceBlit;
    surface->update = _nexusSurfaceUpdate;
    surface->move = _nexusSurfaceMove;
    surface->sync = _nexusSurfaceSync;
    surface->lock = _nexusSurfaceLock;
    surface->flush = _nexusSurfaceFlush;
    surface->unlock = _nexusSurfaceUnlock;
    NEXUS_Surface_GetDefaultCreateSettings(&surface->createSettings);
    surface->createSettings.pixelFormat = format;
    surface->createSettings.width = width;
    surface->createSettings.height = height;
    surface->handle = NEXUS_Surface_Create(&surface->createSettings);
    if (surface->handle == NULL) {
        rc = NEXUS_OUT_OF_DEVICE_MEMORY;
        goto error;
    }

    _nexusSurfaceClear(surface, 0x00, 0x00, 0x00, 0x40);
    surface->ref = 1;
    *surfaceOut = surface;
    return rc;

error:
    if (surface) {
        free(surface);
    }
    return rc;
}

static NEXUS_Error _nexusCreateDisplaySurface(CNexusDisplayLayer *self, NEXUS_PixelFormat format, uint16_t width, uint16_t height, CNexusSurface **surfaceOut)
{
    _nexusCreateSurface(self->ctx, format, width, height, surfaceOut);
    self->screen = *surfaceOut;
    _nexusSurfaceSync(self->screen);
    self->screen->layer = self;
    NEXUS_Error rc = NEXUS_SurfaceClient_SetSurface(self->surfaceClient, self->screen->handle);
    if (rc != NEXUS_SUCCESS) {
        self->screen->release(self->screen);
        *surfaceOut = NULL;
        return NEXUS_OUT_OF_DEVICE_MEMORY;
    }
    return rc;
}

static void _nexusSurfaceAddRef(CNexusSurface* self) {
    self->ref++;
}

static void _nexusSurfaceRelease(CNexusSurface* self) {
    self->ref--;
    if (self->ref < 1) {
        NEXUS_Surface_Destroy(self->handle);
        free(self);
    }
}

static void _nexusDisplayLayerAddRef(CNexusDisplayLayer* self) {
    self->ref++;
}

//TODO: implement cleaning on ref == 0
static void _nexusDisplayLayerRelease(CNexusDisplayLayer* self) {
    self->ref--;
}

static NEXUS_PixelFormat _nexusSurfaceGetPixelFormat(CNexusSurface *self) {
    return self->createSettings.pixelFormat;
}

static int _nexusSurfaceGetWidth(CNexusSurface *self) {
    return self->createSettings.width;
}

static int _nexusSurfaceGetHeight(CNexusSurface *self) {
    return self->createSettings.height;
}

static void _nexusSurfaceGetRectangle(CNexusSurface *self, NEXUS_Rect* rect){
    if (rect) {
        rect->x = 0;
        rect->y = 0;
        rect->width = self->getWidth(self);
        rect->height = self->getHeight(self);
    }
}

static bool _nexusSurfaceIsAlphaPremultipliedSourceEnabled(CNexusSurface* self) {
    NEXUS_Graphics2DBlitSettings settings;
    NEXUS_Graphics2D_GetDefaultBlitSettings(&settings);
    return settings.alphaPremultiplySourceEnabled;
}

static void _nexusSurfaceClipRectangles(CNexusSurface *self, NEXUS_Rect *r, int *nbRect) {
    NEXUS_Rect tmp;
    NEXUS_Rect *current;
    uint16_t width;
    uint16_t height;
    int i,j;
    width = self->createSettings.width;
    height = self->createSettings.height;

    for (i = 0, j= 0; i < *nbRect; ++i) {
        current = &r[i];
        tmp = *current;
        if (current->x > width) {
            //remove that rectangle
            continue;
        }
        if (current->x < 0) {
            tmp.x = 0;
            tmp.width = current->width + current->x;
        }
        if (current->y > height ) {
            //remove that rectangle
            continue;
        }
        if (current->y < 0) {
            tmp.y = 0;
            tmp.height = current->height + current->y;
        }

        if ((tmp.x + tmp.width) > width) {
            tmp.width = tmp.width - (tmp.x + tmp.width) - width;
        }
        if ((tmp.y + tmp.height) > height) {
            tmp.height = tmp.height - (tmp.y + tmp.height) - height;
        }

        else if (current->x + current->width > width) {
            tmp.width = width - current->x;
        }

        r[j++] = tmp;
    }
    *nbRect = j;
}

static NEXUS_Pixel _nexusSurfaceConvertColor(CNexusSurface *self, uint8_t colorComponents[4]) {
    return ((colorComponents[0] & 0xff) << 24 | (colorComponents[1] & 0xff) << 16 | (colorComponents[2] & 0xff) << 8 | (colorComponents[3] & 0xff));
}

static NEXUS_Error _nexusSurfaceFillRectangles(CNexusSurface* self, uint8_t colorComponents[4], NEXUS_PorterDuffOp op, NEXUS_Rect *rect, int nbRect) {
    NEXUS_Error rc;
    NEXUS_Rect *current;
    int i;
    _nexusSurfaceFlush(self);
    self->clipRectangles(self, rect, &nbRect);
    for (i = 0; i < nbRect; ++i) {
        current = &rect[i];
        rc = _nexusSurfaceFillPorterDuffRectangle(self, colorComponents, op, current);
    }
    return NEXUS_SUCCESS;
}

static NEXUS_Error _nexusSurfaceFillPorterDuffRectangle(CNexusSurface* self, uint8_t colorComponents[4], NEXUS_PorterDuffOp op, NEXUS_Rect *rect) {
    NEXUS_Error rc;
    NEXUS_Graphics2DPorterDuffFillSettings settings;
    NEXUS_Graphics2D_GetDefaultPorterDuffFillSettings(&settings);
    settings.surface = self->handle;
    settings.operation = op;
    settings.color = _nexusSurfaceConvertColor(self, colorComponents);
    settings.rect = *rect;
    while (1) {
        rc = NEXUS_Graphics2D_PorterDuffFill(self->ctx->gfx, &settings);
        if (rc == NEXUS_GRAPHICS2D_QUEUE_FULL) {
            BKNI_WaitForEvent(self->ctx->packetSpaceAvailableEvent, BKNI_INFINITE);
            /* retry */
        }
        else {
            break;
        }
    }
    return rc;
}

static void _nexusSurfaceBlit(CNexusSurface *self, CNexusSurface *source, NEXUS_PorterDuffOp op, NEXUS_Rect *source_rect, int x, int y) {
    NEXUS_Error rc;
    NEXUS_Rect rect;
    int nbRect = 1;
    NEXUS_Graphics2DPorterDuffBlitSettings settings;
    _nexusSurfaceFlush(self);
    _nexusSurfaceFlush(source);
    NEXUS_Graphics2D_GetDefaultPorterDuffBlitSettings(&settings);
    if (source_rect == NULL) {
        source->getRectangle(source, &rect);
        source_rect = &rect;
    }
    self->clipRectangles(self, source_rect, &nbRect);
    if (nbRect) {
        settings.sourceSurface = source->handle;
        settings.sourceRect.x = source_rect->x;
        settings.sourceRect.y = source_rect->y;
        settings.sourceRect.width = source_rect->width;
        settings.sourceRect.height = source_rect->height;
        settings.destSurface = self->handle;
        settings.destRect.width = source_rect->width;
        settings.destRect.height = source_rect->height;
        settings.destRect.x =  x;
        settings.destRect.y =  y;
        settings.outSurface = self->handle;
        settings.outRect.x =  x;
        settings.outRect.y =  y;
        settings.outRect.width = source_rect->width;
        settings.outRect.height = source_rect->height;
    }
    settings.operation = op;

    while (1) {
           rc = NEXUS_Graphics2D_PorterDuffBlit(self->ctx->gfx, &settings);
        if (rc == NEXUS_GRAPHICS2D_QUEUE_FULL) {
            /* blit can fail because an internal queue is full. wait for space to open up,
            then resubmit the blit. */
            BKNI_WaitForEvent(self->ctx->packetSpaceAvailableEvent, BKNI_INFINITE);
            /* retry */
        }
        else {
            break;
        }
    }
}

/*
 * Flip/Update surface buffers.
 *
 * If no region is specified the whole surface is flipped,
 * otherwise blitting is used to update the region.
 * If surface capabilities don't include DSCAPS_FLIPPING,
 * this method has the effect to make visible changes
 * made to the surface contents.
 */

//TODO: implement flip
static void _nexusSurfaceUpdate(CNexusSurface *self, bool waitForSync) {
    _nexusSurfaceFlush(self);
    _nexusSurfaceSync(self);
    if (self->layer) {
        NEXUS_Error rc = NEXUS_SurfaceClient_UpdateSurface(self->layer->surfaceClient, NULL);
        BDBG_ASSERT(!rc);
        rc = BKNI_WaitForEvent(self->layer->displayedEvent, 5000);
        BDBG_ASSERT(!rc);
    }
}

static void _nexusSurfaceMove(CNexusSurface *self, int dx, int dy) {
}

static NEXUS_Error _nexusSurfaceSync(CNexusSurface* self) {
    NEXUS_Error rc;
    rc = NEXUS_Graphics2D_Checkpoint(self->ctx->gfx, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(self->ctx->checkpointEvent, BKNI_INFINITE);
    }
    return rc;
}

static NEXUS_Error _nexusSurfaceLock(CNexusSurface* self, void **data, int* pitch) {
    NEXUS_SurfaceStatus surfaceStatus;
    _nexusSurfaceSync(self);
    NEXUS_Surface_GetStatus(self->handle, &surfaceStatus);
    *pitch = surfaceStatus.pitch;
    NEXUS_Surface_Lock(self->handle, data);
    return NEXUS_SUCCESS;
}

static void _nexusSurfaceFlush(CNexusSurface *self) {
    NEXUS_Surface_Flush(self->handle);
}

static void _nexusSurfaceUnlock(CNexusSurface *self) {
    _nexusSurfaceFlush(self);
    NEXUS_Surface_Unlock(self->handle);
}

/* input event : must be generalize for keyboard, ir, mouse events already exist in trellis?...
 *
 */
static NEXUS_Error _nexusCreateInputEvent(CNexus* self, CNexusInputEvent** ev) {
    CNexusInputEvent *event = (CNexusInputEvent *)calloc(1, sizeof(*self));
    *ev = NULL;
    if (!event) {
        return NEXUS_OUT_OF_DEVICE_MEMORY;
    }
    event->wait = _nexusWaitEvent;
    event->get = _nexusGetEvent;
    NEXUS_IrInput_GetDefaultSettings(&event->irSettings);
    event->irSettings.mode = NEXUS_IrInputMode_eRemoteA;
    event->irSettings.dataReady.callback = irCallback;
    event->irSettings.dataReady.context = self;
    event->irHandle = NEXUS_IrInput_Open(0, &event->irSettings);
    *ev = event;
    return NEXUS_SUCCESS;
}

static NEXUS_Error _nexusWaitEvent(CNexusInputEvent*self) {
    //TODO: sem.take();
    return NEXUS_SUCCESS;
}

static NEXUS_Error _nexusGetEvent(CNexusInputEvent*self, NEXUS_IrInputEvent*ev) {
    *ev = self->irEvent;
    return NEXUS_SUCCESS;
}

static void irCallback(void *pParam, int iParam)
{
    size_t numEvents = 1;
    NEXUS_Error rc = 0;
    bool overflow;
    CNexusInputEvent* self = (CNexusInputEvent *)pParam;
    BSTD_UNUSED(iParam);
    while (numEvents && !rc) {
        rc = NEXUS_IrInput_GetEvents(self->irHandle,&self->irEvent,1,&numEvents,&overflow);
        if (numEvents) {
            printf("irCallback: rc: %d, code: %08x, repeat: %s\n", rc, self->irEvent.code, self->irEvent.repeat ? "true" : "false");
            // TODO: sem.give();
        }
    }
}
