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
#ifndef CNEXUS_H
#define CNEXUS_H

#include "nxclient.h"
#include "bkni.h"
#include "nexus_platform_client.h"
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "nexus_surface_client.h"
#include "nexus_ir_input.h"
#include "nexus_display.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CNexus CNexus;
typedef struct _CNexusDisplayLayer CNexusDisplayLayer;
typedef struct _CNexusSurface CNexusSurface;
typedef struct _CNexusInputEvent CNexusInputEvent;

NEXUS_Error NexusCreate(CNexus **nexus);

struct _CNexus {
    void (*addRef)(CNexus* );
    void (*release)(CNexus*);
    NEXUS_Error (*createInputEvent)(CNexus *self, CNexusInputEvent** inputEventOut);
    NEXUS_Error (*createDisplayLayer)(CNexus *self, NEXUS_SurfaceCompositorClientId id, CNexusDisplayLayer **displayLayerOut);
    NEXUS_Error (*createSurface)(CNexus *self, NEXUS_PixelFormat format, uint16_t w, uint16_t h, CNexusSurface**surfaceOut);
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DSettings gfxSettings;

    BKNI_EventHandle checkpointEvent;
    BKNI_EventHandle packetSpaceAvailableEvent;
    int ref;
};

struct _CNexusDisplayLayer {
    void (*addRef)(CNexusDisplayLayer*);
    void (*release)(CNexusDisplayLayer*);
    NEXUS_Error (*enableCursor)(CNexusDisplayLayer *self, int);
    NEXUS_Error (*createSurface)(CNexusDisplayLayer *self, NEXUS_PixelFormat format, uint16_t w, uint16_t h, CNexusSurface**);

    CNexus *ctx;
    NEXUS_SurfaceClientHandle surfaceClient;
    NEXUS_SurfaceClientSettings surfaceClientSettings;
    CNexusSurface *screen;

    BKNI_EventHandle displayedEvent;
    int ref;
};

struct _CNexusSurface {
    void (*addRef)(CNexusSurface *self);
    void (*release)(CNexusSurface *self);
    void (*clear)(CNexusSurface *self, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    NEXUS_PixelFormat (*getPixelFormat)(CNexusSurface *self);
    int (*getWidth)(CNexusSurface *self);
    int (*getHeight)(CNexusSurface *self);
    void (*getRectangle)(CNexusSurface *self, NEXUS_Rect*);
    bool (*isAlphaPremultipliedSourceEnabled)(CNexusSurface *self);
    void (*blit)(CNexusSurface *self, CNexusSurface* , NEXUS_PorterDuffOp op, NEXUS_Rect*, int, int);
    void (*update)(CNexusSurface*, bool waitForSync);
    void (*move)(CNexusSurface*, int dx, int dy);
    NEXUS_Error (*sync)(CNexusSurface*);
    NEXUS_Error (*lock)(CNexusSurface*, void **, int*);
    void (*flush)(CNexusSurface*);
    void (*unlock)(CNexusSurface*);

    void (*clipRectangles)(CNexusSurface *self, NEXUS_Rect *r, int *nbRect);
    NEXUS_Error (*fillRectangles)(CNexusSurface *self, uint8_t colorComponents[4], NEXUS_PorterDuffOp op, NEXUS_Rect *rect, int nbRect);
    NEXUS_Pixel (*convertColor)(CNexusSurface *self, uint8_t colorComponents[4]);

    CNexus *ctx;
    CNexusDisplayLayer *layer;
    NEXUS_SurfaceHandle handle;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_Graphics2DFillSettings fillSettings;
    int ref;
};

struct _CNexusInputEvent {
    void (*addRef)(CNexusInputEvent *self);
    void (*release)(CNexusInputEvent *self);
    NEXUS_Error (*wait)(CNexusInputEvent *self);
    NEXUS_Error (*get)(CNexusInputEvent *self, NEXUS_IrInputEvent*);

    NEXUS_IrInputHandle irHandle;
    NEXUS_IrInputSettings irSettings;
    NEXUS_IrInputEvent irEvent;
    int ref;
};

#ifdef __cplusplus
}
#endif

#endif
