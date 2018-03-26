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
 * Module Description:
 *
 **************************************************************************/
#include "nexus_surface_compositor_module.h"
#include "nexus_surface_compositor_impl.h"

BDBG_MODULE(nexus_surface_cursor);

#define BDBG_MSG_TRACE(X) BDBG_MSG(X)
static const NEXUS_BlendEquation NEXUS_SurfaceCompositor_P_ColorUseSourceAlpha = {
        NEXUS_BlendFactor_eSourceColor, NEXUS_BlendFactor_eSourceAlpha, false, NEXUS_BlendFactor_eDestinationColor, NEXUS_BlendFactor_eInverseSourceAlpha, false, NEXUS_BlendFactor_eZero
};

static const NEXUS_BlendEquation NEXUS_SurfaceCompositor_P_AlphaAddSourceAndDestination = {
        NEXUS_BlendFactor_eDestinationAlpha, NEXUS_BlendFactor_eOne, false, NEXUS_BlendFactor_eZero, NEXUS_BlendFactor_eZero, false, NEXUS_BlendFactor_eSourceAlpha
};

void NEXUS_SurfaceCursor_GetDefaultCreateSettings(NEXUS_SurfaceCursorCreateSettings *createSettings)
{
    unsigned i;
    BDBG_ASSERT(createSettings);
    BKNI_Memset(createSettings, 0, sizeof(*createSettings));
    for(i=0;i<NEXUS_MAX_DISPLAYS;i++) {
        createSettings->display[i].maxSize.width = 64;
        createSettings->display[i].maxSize.height = 64;
    }
    return;
}

NEXUS_SurfaceCursorHandle NEXUS_SurfaceCursor_Create(NEXUS_SurfaceCompositorHandle surfaceCompositor, const NEXUS_SurfaceCursorCreateSettings *createSettings)
{
    unsigned i,j;
    NEXUS_SurfaceCursorHandle cursor;
    NEXUS_SurfaceCreateSettings surfaceSettings; 

    BDBG_OBJECT_ASSERT(surfaceCompositor, NEXUS_SurfaceCompositor);
    if(createSettings == NULL  || createSettings->surface == NULL || createSettings->display[0].maxSize.width == 0 || createSettings->display[0].maxSize.height == 0) {
        (void)BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto err_cfg;
    }
    cursor = BKNI_Malloc(sizeof(*cursor));
    if(cursor==NULL) {(void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto err_alloc;}
    NEXUS_OBJECT_INIT(NEXUS_SurfaceCursor, cursor);
    nexus_p_surface_composition_init(&cursor->settings.composition);
    cursor->settings.composition.colorBlend = NEXUS_SurfaceCompositor_P_ColorUseSourceAlpha;
    cursor->settings.composition.alphaBlend = NEXUS_SurfaceCompositor_P_AlphaAddSourceAndDestination;
    cursor->createSettings = *createSettings;
    cursor->compositor = surfaceCompositor;
    for(i=0;i<NEXUS_MAX_DISPLAYS;i++) {
        NEXUS_Surface_P_CursorDisplay *display = &cursor->display[i];
        display->offset3DRight.x = 0;
        display->offset3DRight.y = 0;
        for(j=0;j<NEXUS_SURFACECMP_MAX_FRAMEBUFFERS;j++) {
            NEXUS_Surface_P_CursorFramebuffer *cursorFramebuffer = &display->framebuffers[j];
            cursorFramebuffer->framebuffer = NULL;
            cursorFramebuffer->buffer = NULL;
        }
    }
    NEXUS_Surface_GetCreateSettings(cursor->createSettings.surface, &surfaceSettings);
    cursor->size.width = surfaceSettings.width;
    cursor->size.height = surfaceSettings.height;
    cursor->size.x = 0;
    cursor->size.y = 0;
    BLST_D_INSERT_HEAD(&surfaceCompositor->cursors, cursor, link);

    NEXUS_Surface_GetDefaultCreateSettings(&surfaceSettings);
    for(i=0;i<NEXUS_MAX_DISPLAYS;i++) {
        NEXUS_Surface_P_CursorDisplay *display = &cursor->display[i];
        if(cursor->createSettings.display[i].maxSize.width==0 || cursor->createSettings.display[i].maxSize.height==0) {
            continue;
        }
        surfaceSettings.pixelFormat = surfaceCompositor->settings.display[i].framebuffer.pixelFormat;
        surfaceSettings.width = cursor->createSettings.display[i].maxSize.width;
        surfaceSettings.height = cursor->createSettings.display[i].maxSize.height;
        surfaceSettings.heap = surfaceCompositor->settings.display[i].framebuffer.heap;
        for(j=0;j<NEXUS_SURFACECMP_MAX_FRAMEBUFFERS;j++) {
            NEXUS_Surface_P_CursorFramebuffer *cursorFramebuffer = &display->framebuffers[j];
            cursorFramebuffer->buffer = NEXUS_Surface_Create(&surfaceSettings);
            if(!cursorFramebuffer->buffer) {(void)BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);goto err_buffer;}
        }
    }

    return cursor;

err_buffer:
    NEXUS_SurfaceCursor_Destroy(cursor);
err_alloc:
err_cfg:
    return NULL;
}

static void NEXUS_SurfaceCursor_P_Finalizer(NEXUS_SurfaceCursorHandle  cursor)
{
    unsigned i;

    NEXUS_OBJECT_ASSERT(NEXUS_SurfaceCursor, cursor);
    for(i=0;i<NEXUS_MAX_DISPLAYS;i++) {
        unsigned j;
        NEXUS_Surface_P_CursorDisplay *display = &cursor->display[i];

        for(j=0;j<NEXUS_SURFACECMP_MAX_FRAMEBUFFERS;j++) {
            NEXUS_Surface_P_CursorFramebuffer *cursorFramebuffer = &display->framebuffers[j];
            if(cursorFramebuffer->buffer) {
                NEXUS_Surface_Destroy(cursorFramebuffer->buffer);
            }
        }
    }
    BLST_D_REMOVE(&cursor->compositor->cursors, cursor, link);
    NEXUS_OBJECT_DESTROY(NEXUS_SurfaceCursor, cursor);
    BKNI_Free(cursor);
    return;
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_SurfaceCursor, NEXUS_SurfaceCursor_Destroy);

void NEXUS_SurfaceCursor_GetSettings (NEXUS_SurfaceCursorHandle  cursor, NEXUS_SurfaceCursorSettings *settings)
{
    BDBG_OBJECT_ASSERT(cursor, NEXUS_SurfaceCursor);
    BDBG_ASSERT(settings);

    *settings = cursor->settings;
    return;
}

static NEXUS_Error NEXUS_SurfaceCursor_P_VerifySize(NEXUS_SurfaceCompositorHandle compositor, NEXUS_SurfaceCursorHandle cursor, const NEXUS_SurfaceCursorSettings *settings, const NEXUS_SurfaceRegion *virtualDisplay)
{
    unsigned i;

    if(!settings->composition.visible) {return NEXUS_SUCCESS;}

    for(i=0;i<NEXUS_MAX_DISPLAYS;i++) {
        NEXUS_Rect framebufferRect;
        const NEXUS_SurfaceRegion *maxSize = &cursor->createSettings.display[i].maxSize;

        if(maxSize->width == 0 || maxSize->height==0) {
            continue; /* cursor not rendered on that display */
        }
        if(compositor->display[0] && compositor->display[0]->display==NULL) {
            continue; /* this display is not activated */
        }
        nexus_surfacemp_p_convert_coord(virtualDisplay, &compositor->display[0]->canvas, &settings->composition.position, &framebufferRect);
        if(framebufferRect.width > maxSize->width || framebufferRect.height > maxSize->height) {
            BDBG_ERR(("Visible size of cursor:%p on display:%u extends reserved size %u,%u > %u,%u", (void *)cursor, i, framebufferRect.width, framebufferRect.height, maxSize->width, maxSize->height));
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
    }
    return NEXUS_SUCCESS;
}


NEXUS_Error NEXUS_SurfaceCursor_P_UpdateDisplay(NEXUS_SurfaceCompositorHandle compositor)
{
    NEXUS_SurfaceCursorHandle cursor;
    for(cursor=BLST_D_FIRST(&compositor->cursors);cursor;cursor=BLST_D_NEXT(cursor,link)) {
        NEXUS_SurfaceRegion virtualDisplay;
        NEXUS_Error rc;
        nexus_p_surface_compositor_update_virtual_display(compositor->display[0], &cursor->settings.composition, &virtualDisplay);
        rc = NEXUS_SurfaceCursor_P_VerifySize(cursor->compositor, cursor, &cursor->settings, &cursor->virtualDisplay);
        if(rc!=NEXUS_SUCCESS) { return BERR_TRACE(rc); }
        cursor->virtualDisplay = virtualDisplay;
        /* XXX stereo only for the first display */
        cursor->display[0].offset3DRight.x = 0;
        cursor->display[0].offset3DRight.y = 0;
        switch(compositor->display[0]->formatInfo.orientation) {
        case NEXUS_VideoOrientation_e3D_LeftRight:
            if(cursor->createSettings.display[0].maxSize.width%2!=0) {
                return BERR_TRACE(NEXUS_INVALID_PARAMETER);
            }
            cursor->display[0].offset3DRight.x = cursor->createSettings.display[0].maxSize.width/2;
            break;
        case NEXUS_VideoOrientation_e3D_OverUnder:
            if(cursor->createSettings.display[0].maxSize.height%2!=0) {
                return BERR_TRACE(NEXUS_INVALID_PARAMETER);
            }
            cursor->display[0].offset3DRight.y = cursor->createSettings.display[0].maxSize.height/2;
            break;
        case NEXUS_VideoOrientation_e2D:
        default:
            break;
        }
    }
    return NEXUS_SUCCESS;
}


/* clears all references to framebuffers */
void NEXUS_SurfaceCursor_P_Clear(NEXUS_SurfaceCompositorHandle compositor)
{
    NEXUS_SurfaceCursorHandle cursor;
    for(cursor=BLST_D_FIRST(&compositor->cursors);cursor;cursor=BLST_D_NEXT(cursor,link)) {
        unsigned i;
        unsigned j;

        for(i=0;i<NEXUS_MAX_DISPLAYS;i++) {
            NEXUS_Surface_P_CursorDisplay *cursorDisplay = &cursor->display[i];
            for(j=0;j<NEXUS_SURFACECMP_MAX_FRAMEBUFFERS;j++) {
                NEXUS_Surface_P_CursorFramebuffer *cursorFramebuffer = &cursorDisplay->framebuffers[j];

                cursorFramebuffer->framebuffer = NULL;
                cursorFramebuffer->left.width = 0;
                cursorFramebuffer->right.width = 0;
            }
        }
    }
    return;
}

void NEXUS_SurfaceCursor_P_ReleaseCursors(NEXUS_SurfaceCompositorHandle compositor, const struct NEXUS_SurfaceCompositorDisplay *compositorDisplay, const struct NEXUS_SurfaceCompositorFramebuffer *framebuffer)
{
    NEXUS_SurfaceCursorHandle cursor;

    BDBG_MSG_TRACE(("NEXUS_SurfaceCursor_P_ReleaseCursors:%p %p(%u) %p", (void *)compositor, (void *)compositorDisplay, compositorDisplay->index, (void *)framebuffer));

    for(cursor=BLST_D_FIRST(&compositor->cursors);cursor;cursor=BLST_D_NEXT(cursor,link)) {
        NEXUS_Surface_P_CursorDisplay *cursorDisplay = &cursor->display[compositorDisplay->index];
        unsigned i;

        for(i=0;i<NEXUS_SURFACECMP_MAX_FRAMEBUFFERS;i++) {
            NEXUS_Surface_P_CursorFramebuffer *cursorFramebuffer = &cursorDisplay->framebuffers[i];

            if(cursorFramebuffer->framebuffer==framebuffer) {
                cursorFramebuffer->framebuffer=NULL;
            }
        }
    }
    return;
}

void NEXUS_SurfaceCursor_P_ClearCursors(NEXUS_SurfaceCompositorHandle compositor, const struct NEXUS_SurfaceCompositorDisplay *compositorDisplay, const struct NEXUS_SurfaceCompositorFramebuffer *framebuffer)
{
    NEXUS_SurfaceCursorHandle cursor;
    NEXUS_Graphics2DBlitSettings blitSettings;

    BDBG_MSG_TRACE(("NEXUS_SurfaceCursor_P_ClearCursors:%p %p(%u) %p", (void *)compositor, (void *)compositorDisplay, compositorDisplay->index, (void *)framebuffer));

    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    for(cursor=BLST_D_FIRST(&compositor->cursors);cursor;cursor=BLST_D_NEXT(cursor,link)) {
        NEXUS_Surface_P_CursorDisplay *cursorDisplay = &cursor->display[compositorDisplay->index];
        unsigned i;

        for(i=0;i<NEXUS_SURFACECMP_MAX_FRAMEBUFFERS;i++) {
            NEXUS_Surface_P_CursorFramebuffer *cursorFramebuffer = &cursorDisplay->framebuffers[i];
            NEXUS_Error rc;

            if(cursorFramebuffer->framebuffer!=framebuffer) {
                continue;
            }
            cursorFramebuffer->framebuffer = NULL;
            if(cursorFramebuffer->left.width) {
                BDBG_ASSERT(cursorFramebuffer->left.height); /* height shouled be the same for left and right rectangles, and not equal to zero */
                blitSettings.output.surface = framebuffer->surface;
                blitSettings.output.rect = cursorFramebuffer->left;
                blitSettings.source.surface = cursorFramebuffer->buffer;
                blitSettings.source.rect.x = blitSettings.source.rect.y = 0;
                blitSettings.source.rect.width = cursorFramebuffer->left.width;
                blitSettings.source.rect.height = cursorFramebuffer->left.height;
                rc = NEXUS_Graphics2D_Blit(compositor->gfx, &blitSettings);
                BERR_TRACE(rc);
            }
            if(cursorFramebuffer->right.width) {
                BDBG_ASSERT(cursorFramebuffer->left.height); /* height shouled be the same for left and right rectangles, and not equal to zero */
                BDBG_MSG_TRACE(("NEXUS_SurfaceCursor_P_ClearCursors:%p restore %p (%d,%d,%u,%u) right", (void *)compositor, (void *)cursor, cursorFramebuffer->left.x, cursorFramebuffer->left.y, cursorFramebuffer->left.width, cursorFramebuffer->left.height));
                blitSettings.output.surface = framebuffer->surface;
                blitSettings.output.rect = cursorFramebuffer->right;
                blitSettings.output.rect.x += compositorDisplay->offset3DRight.x;
                blitSettings.output.rect.y += compositorDisplay->offset3DRight.y;
                blitSettings.source.surface = cursorFramebuffer->buffer;
                blitSettings.source.rect.x = cursorDisplay->offset3DRight.x;
                blitSettings.source.rect.y = cursorDisplay->offset3DRight.y;
                blitSettings.source.rect.width = cursorFramebuffer->right.width;
                blitSettings.source.rect.height = cursorFramebuffer->right.height;
                rc = NEXUS_Graphics2D_Blit(compositor->gfx, &blitSettings);
                BERR_TRACE(rc);
            }
            break;
        }
    }
    return;
}


static void NEXUS_SurfaceCursor_P_RenderCursor_Eye(NEXUS_SurfaceCompositorHandle compositor, const struct NEXUS_SurfaceCompositorDisplay *compositorDisplay, const struct NEXUS_SurfaceCompositorFramebuffer *framebuffer, NEXUS_SurfaceCursorHandle cursor, const NEXUS_Surface_P_CursorFramebuffer *cursorFramebuffer, NEXUS_Surface_P_CursorDisplay *cursorDisplay, const NEXUS_Rect *outputRect, const NEXUS_Rect *sourceRect, bool left)
{
    NEXUS_Graphics2DBlitSettings blitSettings;
    NEXUS_Error rc;

    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    BDBG_MSG_TRACE(("NEXUS_SurfaceCursor_P_RenderCursor_Eye:%p save %p (%d,%d,%u,%u) %s", (void *)compositor, (void *)cursor, outputRect->x, outputRect->y, outputRect->width, outputRect->height, left?"left":"right"));
    /* 1. copy original data into the safe buffer */
    blitSettings.output.surface = cursorFramebuffer->buffer;
    blitSettings.source.surface = framebuffer->surface;
    blitSettings.output.rect.x = 0;
    blitSettings.output.rect.y = 0;
    blitSettings.output.rect.width = outputRect->width;
    blitSettings.output.rect.height = outputRect->height;
    blitSettings.source.rect.x = outputRect->x;
    blitSettings.source.rect.y = outputRect->y;
    blitSettings.source.rect.width = outputRect->width;
    blitSettings.source.rect.height = outputRect->height;
    if(!left) {
        blitSettings.source.rect.x += compositorDisplay->offset3DRight.x;
        blitSettings.source.rect.y += compositorDisplay->offset3DRight.y;
        blitSettings.output.rect.x += cursorDisplay->offset3DRight.x;
        blitSettings.output.rect.y += cursorDisplay->offset3DRight.y;
    }


    BDBG_ASSERT(blitSettings.output.rect.width + blitSettings.output.rect.x <= cursor->createSettings.display[compositorDisplay->index].maxSize.width);
    BDBG_ASSERT(blitSettings.output.rect.height + blitSettings.output.rect.y <= cursor->createSettings.display[compositorDisplay->index].maxSize.height);
    rc = NEXUS_Graphics2D_Blit(compositor->gfx, &blitSettings);
    BERR_TRACE(rc);
    /* 2. copy cursor to framebuffer */
    blitSettings.output.surface = framebuffer->surface;
    blitSettings.source.surface = cursor->createSettings.surface;
    blitSettings.dest.surface = framebuffer->surface;
    blitSettings.dest.rect = blitSettings.source.rect; /* copy from previous step's source */
    blitSettings.output.rect = blitSettings.source.rect; /* copy from previous step's source */
    blitSettings.source.rect = *sourceRect;
    blitSettings.colorBlend = cursor->settings.composition.colorBlend;
    blitSettings.alphaBlend = cursor->settings.composition.alphaBlend;
    blitSettings.constantColor = cursor->settings.composition.constantColor;
    blitSettings.colorOp = NEXUS_BlitColorOp_eUseBlendEquation;
    blitSettings.alphaOp = NEXUS_BlitAlphaOp_eUseBlendEquation;
    BDBG_MSG_TRACE(("NEXUS_SurfaceCursor_P_RenderCursor_Eye:%p copy %p (%d,%d,%u,%u) -> (%d,%d,%u,%u) %s", (void *)compositor, (void *)cursor, sourceRect->x, sourceRect->y, sourceRect->width, sourceRect->height, outputRect->x, outputRect->y, outputRect->width, outputRect->height, left?"left":"right"));
    rc = NEXUS_Graphics2D_Blit(compositor->gfx, &blitSettings);
    BERR_TRACE(rc);
    return;
}



void NEXUS_SurfaceCursor_P_RenderCursors(NEXUS_SurfaceCompositorHandle compositor, const struct NEXUS_SurfaceCompositorDisplay *compositorDisplay, const struct NEXUS_SurfaceCompositorFramebuffer *framebuffer)
{
    NEXUS_SurfaceCursorHandle cursor;
    NEXUS_Rect framebufferRegion;

    BDBG_MSG_TRACE(("NEXUS_SurfaceCursor_P_RenderCursors:%p %p(%u) %p", (void *)compositor, (void *)compositorDisplay, compositorDisplay->index, (void *)framebuffer));

    framebufferRegion.x = framebufferRegion.y = 0;
    framebufferRegion.width = compositorDisplay->canvas.width;
    framebufferRegion.height = compositorDisplay->canvas.height;


    for(cursor=BLST_D_FIRST(&compositor->cursors);cursor;cursor=BLST_D_NEXT(cursor,link)) {
        NEXUS_Rect outputRectLeft;
        NEXUS_Rect sourceRectLeft;
        NEXUS_Rect framebufferRect; 
        NEXUS_Rect outputRectRight;
        NEXUS_Rect sourceRectRight;
        NEXUS_Surface_P_CursorDisplay *cursorDisplay = &cursor->display[compositorDisplay->index];
        unsigned i;
    
        if(!cursor->settings.composition.visible) {
            continue;
        }
        sourceRectRight.width=0;
        sourceRectLeft.width=0;

        nexus_surfacemp_p_convert_coord(&cursor->virtualDisplay, &compositorDisplay->canvas, &cursor->settings.composition.position, &framebufferRect);

        nexus_surfacemp_p_clip_rect(&framebufferRegion, &framebufferRect, &outputRectLeft);
        if(outputRectLeft.height && outputRectLeft.width) {
            nexus_surfacemp_scale_clipped_rect(&framebufferRect, &outputRectLeft, &cursor->size, &sourceRectLeft);
            if(sourceRectLeft.height==0) {
                sourceRectLeft.width=0;
            }
        }

        if(compositorDisplay->formatInfo.orientation==NEXUS_VideoOrientation_e3D_LeftRight || compositorDisplay->formatInfo.orientation==NEXUS_VideoOrientation_e3D_OverUnder) {
            unsigned rightViewOffset = (cursor->settings.composition.rightViewOffset * cursor->virtualDisplay.width) /  compositorDisplay->canvas.width;
            framebufferRect.x += rightViewOffset;

            nexus_surfacemp_p_clip_rect(&framebufferRegion, &framebufferRect, &outputRectRight);
            if(outputRectRight.height && outputRectRight.width) {
                nexus_surfacemp_scale_clipped_rect(&framebufferRect, &outputRectRight, &cursor->size, &sourceRectRight);
                if(sourceRectRight.height==0) {
                    sourceRectRight.width=0;
                }
            }
        }

        if(sourceRectLeft.width == 0 && sourceRectRight.width == 0) {
            continue;
        }

        for(i=0;i<NEXUS_SURFACECMP_MAX_FRAMEBUFFERS;i++) {
            NEXUS_Surface_P_CursorFramebuffer *cursorFramebuffer = &cursorDisplay->framebuffers[i];
            if(cursorFramebuffer->framebuffer!=NULL) {
                /* composed could be only used for single framebuffer */
                continue;
            }
            cursorFramebuffer->left.width = 0;
            cursorFramebuffer->right.width = 0;

            if(sourceRectLeft.width) {
                NEXUS_SurfaceCursor_P_RenderCursor_Eye(compositor, compositorDisplay, framebuffer, cursor, cursorFramebuffer, cursorDisplay, &outputRectLeft, &sourceRectLeft, true);
                cursorFramebuffer->left = outputRectLeft;
            }

            if(sourceRectRight.width) {
                NEXUS_SurfaceCursor_P_RenderCursor_Eye(compositor, compositorDisplay, framebuffer, cursor, cursorFramebuffer, cursorDisplay, &outputRectRight, &sourceRectRight, false);
                cursorFramebuffer->right = outputRectRight;
            }
            /* mark it as composited */
            cursorFramebuffer->framebuffer=framebuffer;
            break;
        }
    }
    return;
}

/* update cursor in already composed framebuffer */
void NEXUS_SurfaceCursor_P_UpdateCursor(NEXUS_SurfaceCompositorHandle compositor)
{
    unsigned i;

    if(!nexus_surface_compositor_p_blitter_acquire(compositor, NEXUS_P_SURFACEBLITTER_CURSOR)) {
        return;
    }
    for(i=0;i<NEXUS_MAX_DISPLAYS;i++) {
        const struct NEXUS_SurfaceCompositorDisplay *compositorDisplay = compositor->display[i];
        const struct NEXUS_SurfaceCompositorFramebuffer *framebuffer = NULL;
        if(compositorDisplay==NULL || compositorDisplay->display==NULL) {
            continue;
        }
        /* find most recently composed framebuffer */
        if(compositorDisplay->submitted) { /* if framebufeer was submitted to display, then render in it */
            framebuffer = compositorDisplay->submitted;
        } else if(compositorDisplay->displaying) {
            framebuffer = compositorDisplay->displaying;
        } else if(compositorDisplay->composited) {
            framebuffer = compositorDisplay->composited;
        } else {
            continue;
        }
        BDBG_MSG_TRACE(("updating cursors:%p display:%p(%u) framebuffer:%p", (void *)compositor, (void *)compositorDisplay, i, (void *)framebuffer));
        NEXUS_SurfaceCursor_P_ClearCursors(compositor, compositorDisplay, framebuffer);
        NEXUS_SurfaceCursor_P_RenderCursors(compositor, compositorDisplay, framebuffer);
    }
    nexus_surface_compositor_p_blitter_start(compositor, NEXUS_P_SURFACEBLITTER_CURSOR);

    return;
}

NEXUS_Error NEXUS_SurfaceCursor_SetSettings ( NEXUS_SurfaceCursorHandle  cursor, const NEXUS_SurfaceCursorSettings *settings)
{
    NEXUS_SurfaceRegion virtualDisplay;
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(cursor, NEXUS_SurfaceCursor);
    BDBG_ASSERT(settings);

    nexus_p_surface_compositor_update_virtual_display(cursor->compositor->display[0], &settings->composition, &virtualDisplay);
    rc = NEXUS_SurfaceCursor_P_VerifySize(cursor->compositor, cursor, settings, &virtualDisplay);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
    cursor->virtualDisplay = virtualDisplay;
    cursor->settings = *settings;
    NEXUS_SurfaceCursor_P_UpdateCursor(cursor->compositor);
    return NEXUS_SUCCESS;
}

