/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 ******************************************************************************/
#include "nexus_base.h"
#include "nexus_display_module.h"
#include "priv/nexus_surface_priv.h"
#if NEXUS_HAS_SAGE
#include "priv/nexus_sage_priv.h"
#endif

BDBG_MODULE(nexus_display_graphics);

static void NEXUS_Display_P_DestroyGraphics(NEXUS_DisplayHandle display);

static void
NEXUS_Display_P_GraphicsNext_isr(void  *disp, int unused, BAVC_Polarity  polarity, BAVC_SourceState  state, void **picture)
{
    NEXUS_DisplayHandle display = disp;
    struct NEXUS_DisplayGraphics *graphics = &display->graphics;

    BSTD_UNUSED(unused);
    BSTD_UNUSED(state);
    BSTD_UNUSED(polarity);
    BDBG_OBJECT_ASSERT(display, NEXUS_Display);
    BDBG_ASSERT(picture);
    *picture = NULL; /* clear picture info */
    if(graphics->queuedPlane) {
        BAVC_Gfx_Picture pic;
        BERR_Code rc;
        rc = BVDC_Source_GetSurface_isr(graphics->source, &pic);
        if (!rc) {
            if (pic.pSurface == graphics->queuedPlane) {
                graphics->queuedPlane = NULL;
                NEXUS_IsrCallback_Fire_isr(graphics->frameBufferCallback);
            }
        }
    }
    return;
}

static NEXUS_Error
NEXUS_Display_P_SetGraphicsChromaKey(const struct NEXUS_DisplayGraphics *graphics, const NEXUS_GraphicsSettings *cfg)
{
    BERR_Code rc;

    if(cfg->chromakeyEnabled) {
        uint32_t min,max,mask;
        switch(graphics->frameBufferPixelFormat) {
        default:
            min = cfg->lowerChromakey;
            max = cfg->upperChromakey;
            mask = BPXL_MAKE_PIXEL(BPXL_eA8_R8_G8_B8, 0xFF, 0xFF, 0xFF, 0xFF);
            break;
        case NEXUS_PixelFormat_eY08_Cb8_Y18_Cr8:
        case NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08:
            {
            unsigned t_min, t_max;

            BPXL_ConvertPixel_RGBtoYCbCr(BPXL_eA8_Y8_Cb8_Cr8, BPXL_eA8_R8_G8_B8, cfg->lowerChromakey, &t_min);
            BPXL_ConvertPixel_RGBtoYCbCr(BPXL_eA8_Y8_Cb8_Cr8, BPXL_eA8_R8_G8_B8, cfg->upperChromakey, &t_max);
            min = t_min;
            max = t_max;
            mask = BPXL_MAKE_PIXEL(BPXL_eY08_Cr8_Y18_Cb8, 0xFF, 0xFF, 0xFF, 0xFF);
            break;
            }
        }
        rc = BVDC_Source_EnableColorKey(graphics->source, min, max, mask, 0 /* alpha 0 - remove it */);
        if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}
    } else {
        rc = BVDC_Source_DisableColorKey(graphics->source);
        if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}
    }
    return NEXUS_SUCCESS;
}

static NEXUS_Error
NEXUS_Display_P_SetSdrGfxToHdrApproximationAdjust(const struct NEXUS_DisplayGraphics *graphics, const NEXUS_GraphicsSettings *cfg)
{
    BERR_Code rc;

    rc = BVDC_Source_SetSdrGfxToHdrApproximationAdjust(graphics->source, (BVDC_Source_SdrGfxToHdrApproximationAdjust *)&cfg->sdrToHdr);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}
    return NEXUS_SUCCESS;
}

static BERR_Code
NEXUS_Display_P_SetGraphicsSettings(NEXUS_DisplayHandle display, const NEXUS_GraphicsSettings *cfg, bool force)
{
    BERR_Code rc;
    struct NEXUS_DisplayGraphics *graphics = &display->graphics;
    const NEXUS_DisplayModule_State *video= &g_NEXUS_DisplayModule_State;

    force = force || video->lastUpdateFailed;

    if(force || graphics->cfg.horizontalFilter != cfg->horizontalFilter
             || graphics->cfg.verticalFilter != cfg->verticalFilter)
    {
        BDBG_CASSERT(BVDC_FilterCoeffs_eSharp == (BVDC_FilterCoeffs)NEXUS_GraphicsFilterCoeffs_eSharp);
        rc = BVDC_Source_SetScaleCoeffs(graphics->source, cfg->horizontalFilter, cfg->verticalFilter);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_source_cfg;}
    }

    if (force ||
        graphics->cfg.horizontalCoeffIndex != cfg->horizontalCoeffIndex ||
        graphics->cfg.verticalCoeffIndex != cfg->verticalCoeffIndex)
    {
        BVDC_CoefficientIndex coeffSettings;
        BKNI_Memset(&coeffSettings, 0, sizeof(coeffSettings)); /* no GetSettings */
        coeffSettings.ulSclVertLuma = cfg->verticalCoeffIndex;
        coeffSettings.ulSclHorzLuma = cfg->horizontalCoeffIndex;
        rc = BVDC_Window_SetCoefficientIndex(graphics->windowVdc, &coeffSettings);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_window_cfg;}
    }

    /* Set the graphics framebuffer to be full screen for its display. */
    rc = BVDC_Window_SetScalerOutput( graphics->windowVdc, 0, 0, cfg->position.width, cfg->position.height);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_window_cfg;}

    rc = BVDC_Window_SetDstRect( graphics->windowVdc, cfg->position.x, cfg->position.y, cfg->position.width, cfg->position.height);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_window_cfg;}

    if(graphics->frameBufferWidth<(cfg->clip.width + cfg->clip.x) || graphics->frameBufferHeight < (cfg->clip.height+cfg->clip.y)) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto err_window_cfg;
    }
    /* Clip the framebuffer to match the display size */
    rc = BVDC_Window_SetSrcClip( graphics->windowVdc, cfg->clip.x, graphics->frameBufferWidth - (cfg->clip.width+cfg->clip.x), cfg->clip.y, graphics->frameBufferHeight - (cfg->clip.height+cfg->clip.y));
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_window_cfg;}

    if(force || graphics->cfg.zorder != cfg->zorder) {
        rc = BVDC_Window_SetZOrder( graphics->windowVdc, cfg->zorder);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_window_cfg;}
    }

    if(force || graphics->cfg.alpha != cfg->alpha ||
                graphics->cfg.sourceBlendFactor != cfg->sourceBlendFactor ||
                graphics->cfg.destBlendFactor != cfg->destBlendFactor ||
                graphics->cfg.constantAlpha != cfg->constantAlpha)
    {
        rc = BVDC_Window_SetAlpha(graphics->windowVdc, cfg->alpha);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_window_cfg;}

        rc = BVDC_Window_SetBlendFactor( graphics->windowVdc, cfg->sourceBlendFactor, cfg->destBlendFactor, cfg->constantAlpha);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_window_cfg;}
    }

    if(force || (graphics->cfg.chromakeyEnabled!=cfg->chromakeyEnabled || graphics->cfg.lowerChromakey!=cfg->lowerChromakey || graphics->cfg.upperChromakey!=cfg->upperChromakey)) {
        rc = NEXUS_Display_P_SetGraphicsChromaKey(graphics, cfg);
        if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto err_source_cfg;}
    }

    if(force || (graphics->cfg.sdrToHdr.y!=cfg->sdrToHdr.y ||
                 graphics->cfg.sdrToHdr.cb!=cfg->sdrToHdr.cb ||
                 graphics->cfg.sdrToHdr.cr!=cfg->sdrToHdr.cr)) {
        rc = NEXUS_Display_P_SetSdrGfxToHdrApproximationAdjust(graphics, cfg);
        if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto err_source_cfg;}
    }

    if(force || (graphics->cfg.visible != cfg->visible)) {
        rc = BVDC_Window_SetVisibility(graphics->windowVdc, cfg->visible);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_window_cfg;}
    }

    if(force || (graphics->cfg.graphics3DSettings.rightViewOffset != cfg->graphics3DSettings.rightViewOffset)) {
        rc = BVDC_Window_SetDstRightRect(graphics->windowVdc, cfg->graphics3DSettings.rightViewOffset);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_window_cfg;}
    }

    rc = BVDC_Window_SetContrast(graphics->windowVdc, graphics->colorSettings.contrast);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_window_cfg;}

    rc = BVDC_Window_SetSaturation(graphics->windowVdc, graphics->colorSettings.saturation);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_window_cfg;}

    rc = BVDC_Window_SetHue(graphics->windowVdc, graphics->colorSettings.hue);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_window_cfg;}

    rc = BVDC_Window_SetBrightness(graphics->windowVdc, graphics->colorSettings.brightness);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_window_cfg;}

    if (display->graphics.colorMatrixSet) {
        rc = NEXUS_Display_SetGraphicsColorMatrix(display, &display->graphics.colorMatrix);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_window_cfg;}
    }

    return rc;
err_window_cfg:
err_source_cfg:
    return rc;
}

static NEXUS_Error
NEXUS_Display_P_SetGraphicsSource(NEXUS_DisplayHandle display, const NEXUS_GraphicsFramebuffer3D *frameBuffer3D, const BPXL_Plane *main)
{
    BERR_Code rc;
    BAVC_Gfx_Picture pic;
    const NEXUS_DisplayModule_State *video= &g_NEXUS_DisplayModule_State;

    BKNI_Memset(&pic, 0, sizeof(pic));
    pic.pSurface = main;
    pic.eInOrientation = NEXUS_P_VideoOrientation_ToMagnum_isrsafe(frameBuffer3D->orientation);
    if(frameBuffer3D->alpha || frameBuffer3D->right) {
        NEXUS_Module_Lock(video->modules.surface);
        if(frameBuffer3D->right) {
            pic.pRSurface = NEXUS_Surface_GetPixelPlane_priv(frameBuffer3D->right);
        }
        NEXUS_Module_Unlock(video->modules.surface);
    }
    rc = BVDC_Source_SetSurface( display->graphics.source, &pic);
    if (rc!=BERR_SUCCESS) { return BERR_TRACE(rc);}

    return NEXUS_SUCCESS;
}

static void NEXUS_Display_P_SetSecureGraphics(struct NEXUS_DisplayGraphics *graphics, bool secure)
{
    if (graphics->secure != secure) {
#if NEXUS_HAS_SAGE
        BAVC_CoreList coreList;
        BKNI_Memset(&coreList, 0, sizeof(coreList));
        coreList.aeCores[BAVC_CoreId_eGFD_0] = true;
        if (secure) {
            int rc = NEXUS_Sage_AddSecureCores(&coreList);
            if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);return;}
        }
        else {
            NEXUS_Sage_RemoveSecureCores(&coreList);
        }
        graphics->secure = secure;
#else
        BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
    }
}

static BERR_Code
NEXUS_Display_P_CreateGraphics(NEXUS_DisplayHandle display, const NEXUS_GraphicsSettings *cfg)
{
    BERR_Code rc;
    BERR_Code cleanup_rc; /* keep original error code */
    const NEXUS_DisplayModule_State *video= &g_NEXUS_DisplayModule_State;
    struct NEXUS_DisplayGraphics *graphics = &display->graphics;
    static const BAVC_SourceId gfx_ids[]={BAVC_SourceId_eGfx0,BAVC_SourceId_eGfx1, BAVC_SourceId_eGfx2,BAVC_SourceId_eGfx3,BAVC_SourceId_eGfx4,BAVC_SourceId_eGfx5, BAVC_SourceId_eGfx6};
    const BPXL_Plane *surface;
    NEXUS_SurfaceCreateSettings surfaceCfg;
    BVDC_Window_Settings windowCfg;

    BDBG_MSG((">graphics: %ux%u video=%p display=%p graphics=%p", cfg->position.width, cfg->position.height, (void*)video, (void*)display, (void*)graphics));
    if (display->index >= sizeof(gfx_ids)/sizeof(*gfx_ids)) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    NEXUS_Display_P_SetSecureGraphics(graphics, cfg->secure);

    BDBG_ASSERT(graphics->frameBuffer3D.main);
    NEXUS_Module_Lock(video->modules.surface);
    surface = NEXUS_Surface_GetPixelPlane_priv(graphics->frameBuffer3D.main);
    BDBG_ASSERT(surface);
    NEXUS_Module_Unlock(video->modules.surface);
    NEXUS_Surface_GetCreateSettings(graphics->frameBuffer3D.main, &surfaceCfg);
    graphics->queuedPlane = surface;
    graphics->frameBufferHeight = surfaceCfg.height;
    graphics->frameBufferWidth = surfaceCfg.width;
    graphics->frameBufferPixelFormat = surfaceCfg.pixelFormat;

    rc = BVDC_Source_Create( video->vdc, &graphics->source, gfx_ids[display->index], NULL);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_source;}
    rc = NEXUS_Display_P_SetGraphicsSource(display, &graphics->frameBuffer3D, surface);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_source_cfg;}

    rc = BVDC_Source_InstallPictureCallback(graphics->source, NEXUS_Display_P_GraphicsNext_isr, display, 0);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_source_cfg;}

    BVDC_Window_GetDefaultSettings(BVDC_WindowId_eAuto, &windowCfg);
    windowCfg.bBypassVideoProcessings = display->graphics.colorMatrix.bypass;
    rc = BVDC_Window_Create( display->compositor, &graphics->windowVdc, BVDC_WindowId_eAuto, graphics->source, &windowCfg);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_window;}

    rc = NEXUS_Display_P_SetGraphicsSettings(display, cfg, true);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_graphics_cfg;}

    if(video->updateMode != NEXUS_DisplayUpdateMode_eAuto) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);}
    rc = BVDC_ApplyChanges(video->vdc);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_apply_changes;}

    BDBG_MSG(("<graphics:%p", (void *)graphics->windowVdc));

    return BERR_SUCCESS;

err_apply_changes:
err_graphics_cfg:
    cleanup_rc = BVDC_AbortChanges(video->vdc);
    if (cleanup_rc!=BERR_SUCCESS) { cleanup_rc = BERR_TRACE(cleanup_rc);}
    cleanup_rc = BVDC_Window_Destroy(graphics->windowVdc);
    if (cleanup_rc!=BERR_SUCCESS) { cleanup_rc = BERR_TRACE(cleanup_rc);}
err_window:
err_source_cfg:
    cleanup_rc = BVDC_Source_Destroy(graphics->source);
    if (cleanup_rc!=BERR_SUCCESS) { cleanup_rc = BERR_TRACE(cleanup_rc);}
err_source:
    return rc;

}

/* destroy graphics and reset NEXUS_GraphicsSettings based on the current display format */
void
NEXUS_Display_P_ResetGraphics(NEXUS_DisplayHandle display)
{
    /* if graphics has not been created, we should reset the display dimensions for when it is */
    NEXUS_Display_P_DestroyGraphics(display);

    display->graphics.cfg.position = display->displayRect;
    display->graphics.cfg.clip = display->displayRect;
    display->graphics.validCount = 0;
    return;
}

void
NEXUS_Display_P_DestroyGraphicsSource(NEXUS_DisplayHandle display)
{
    BERR_Code rc;
    struct NEXUS_DisplayGraphics *graphics = &display->graphics;
    const NEXUS_DisplayModule_State *video= &g_NEXUS_DisplayModule_State;

    if (!graphics->windowVdc) {
        return;
    }

    BKNI_EnterCriticalSection();
    if (graphics->queuedPlane) {
        graphics->queuedPlane = NULL;
        NEXUS_IsrCallback_Fire_isr(graphics->frameBufferCallback);
    }
    BKNI_LeaveCriticalSection();
    rc = BVDC_Window_Destroy(graphics->windowVdc);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);}
    rc = BVDC_Source_Destroy(graphics->source);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);}
    /* If apply changes fails now, we're in an unrecoverable state, so stop here. */
    if(video->updateMode != NEXUS_DisplayUpdateMode_eAuto) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);}
    rc = BVDC_ApplyChanges(video->vdc);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);}
    NEXUS_Display_P_SetSecureGraphics(graphics, false);
    graphics->source = NULL;
    graphics->windowVdc = NULL;

}

static void
NEXUS_Display_P_DestroyGraphics(NEXUS_DisplayHandle display)
{
    struct NEXUS_DisplayGraphics *graphics = &display->graphics;

    if (!graphics->windowVdc) {
        return;
    }

    BDBG_MSG((">graphics: destroy %p", (void *)graphics));

    NEXUS_Display_P_DestroyGraphicsSource(display);

    NEXUS_Graphics_GetDefaultFramebuffer3D(&graphics->frameBuffer3D);

    BDBG_MSG(("<graphics: destroy %p", (void *)graphics));

    return;
}

void
NEXUS_Display_GetGraphicsSettings(NEXUS_DisplayHandle display, NEXUS_GraphicsSettings *settings)
{
    BDBG_OBJECT_ASSERT(display, NEXUS_Display);
    BDBG_ASSERT(settings);
    *settings = display->graphics.cfg;
    return;
}

NEXUS_Error
NEXUS_Display_SetGraphicsSettings(NEXUS_DisplayHandle display,const NEXUS_GraphicsSettings *settings)
{
    struct NEXUS_DisplayGraphics *graphics = &display->graphics;
    const NEXUS_DisplayModule_State *video= &g_NEXUS_DisplayModule_State;
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(display, NEXUS_Display);
    BDBG_ASSERT(settings);
    if(graphics->windowVdc && !settings->enabled) {
        NEXUS_Display_P_DestroyGraphics(display);
    }
    if(!settings->enabled) {
        NEXUS_Graphics_GetDefaultFramebuffer3D(&graphics->frameBuffer3D);
    }
    if(graphics->frameBuffer3D.main && settings->enabled) {
        if(graphics->windowVdc) {  /* update existing */
            rc = NEXUS_Display_P_SetGraphicsSettings(display, settings, false);
            if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_graphics_cfg;}
            rc = NEXUS_Display_P_ApplyChanges();
            if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_apply_changes;}
        } else { /* create new graphics */
            rc = NEXUS_Display_P_CreateGraphics(display, settings);
            if(rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);goto err_create;}
        }
    } /* if no frame bufeer or graphics not enabled delay activation of graphics */
    graphics->cfg = *settings;
    NEXUS_IsrCallback_Set(graphics->frameBufferCallback, &graphics->cfg.frameBufferCallback);
    return BERR_SUCCESS;

err_apply_changes:
err_graphics_cfg:
    {
        BERR_Code rc = BVDC_AbortChanges(video->vdc);
        if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); }
    }
err_create:
    return rc;
}

NEXUS_Error
NEXUS_Display_SetGraphicsFramebuffer(NEXUS_DisplayHandle display, NEXUS_SurfaceHandle frameBuffer)
{
    NEXUS_Error rc;
    NEXUS_GraphicsFramebuffer3D framebuffer3D;

    NEXUS_Graphics_GetDefaultFramebuffer3D(&framebuffer3D);
    framebuffer3D.orientation = NEXUS_VideoOrientation_e2D;
    framebuffer3D.main = frameBuffer;
    rc = NEXUS_Display_SetGraphicsFramebuffer3D(display, &framebuffer3D);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}

    return rc;
}


NEXUS_Error
NEXUS_Display_SetGraphicsFramebuffer3D(NEXUS_DisplayHandle display, const NEXUS_GraphicsFramebuffer3D *frameBuffer3D)
{
    struct NEXUS_DisplayGraphics *graphics = &display->graphics;
    const NEXUS_DisplayModule_State *video= &g_NEXUS_DisplayModule_State;

    BERR_Code rc;
    BDBG_OBJECT_ASSERT(display, NEXUS_Display);
    BDBG_ASSERT(frameBuffer3D);


    if(frameBuffer3D->main==NULL) { return BERR_TRACE(NEXUS_INVALID_PARAMETER); }

    graphics->frameBuffer3D = *frameBuffer3D;
    if(graphics->windowVdc) {
        const BPXL_Plane *surface;
        NEXUS_SurfaceCreateSettings surfaceCfg;
        bool frameBufferChanged;

        BDBG_ASSERT(graphics->source);
        NEXUS_Module_Lock(video->modules.surface);
        surface = NEXUS_Surface_GetPixelPlane_priv(frameBuffer3D->main);
        BDBG_ASSERT(surface);
        NEXUS_Module_Unlock(video->modules.surface);
        NEXUS_Surface_GetCreateSettings(graphics->frameBuffer3D.main, &surfaceCfg);
        if(graphics->queuedPlane) {
            BDBG_MSG(("NEXUS_Display_SetGraphicsFramebuffer: %p setting duplicated framebuffer %p %s", (void *)display, (void *)frameBuffer3D->main, graphics->queuedPlane==surface?"without sync":""));
        }
        graphics->queuedPlane = surface;
        frameBufferChanged = (graphics->frameBufferHeight != surfaceCfg.height || graphics->frameBufferWidth != surfaceCfg.width || graphics->frameBufferPixelFormat != surfaceCfg.pixelFormat);
        graphics->frameBufferHeight = surfaceCfg.height;
        graphics->frameBufferWidth = surfaceCfg.width;
        graphics->frameBufferPixelFormat = surfaceCfg.pixelFormat;
        rc = NEXUS_Display_P_SetGraphicsSource(display, frameBuffer3D, surface);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_set_source;}
        if(frameBufferChanged) {
            rc = NEXUS_Display_P_SetGraphicsSettings(display, &graphics->cfg, true);
            if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_graphics_cfg;}
        }
        rc = NEXUS_Display_P_ApplyChanges();
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_apply_changes;}
    } else {
        if(graphics->cfg.enabled) {
            rc = NEXUS_Display_P_CreateGraphics(display, &graphics->cfg);
            if(rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc); goto err_create;}
        }
    }
    return BERR_SUCCESS;
err_apply_changes:
err_graphics_cfg:
    {
        BERR_Code rc = BVDC_AbortChanges(video->vdc);
        if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); }
    }
err_set_source:
err_create:
    return rc;
}

/* NEXUS_Display_P_InitGraphics is only called from NEXUS_Display_Open */
BERR_Code
NEXUS_Display_P_InitGraphics(NEXUS_DisplayHandle display)
{
    BERR_Code rc;
    struct NEXUS_DisplayGraphics *graphics = &display->graphics;

    graphics->source = NULL;
    graphics->windowVdc = NULL;
    graphics->frameBufferWidth = 0;
    graphics->frameBufferHeight = 0;
    graphics->frameBufferPixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    graphics->queuedPlane = NULL;
    graphics->validCount = 0;
    NEXUS_Graphics_GetDefaultFramebuffer3D(&graphics->frameBuffer3D);
    BKNI_Memset(&graphics->cfg, 0, sizeof(graphics->cfg));
    BKNI_Memset(&graphics->colorMatrix, 0, sizeof(graphics->colorMatrix));
    graphics->cfg.enabled = true;
    graphics->cfg.visible = true;
    /* cfg.position and cfg.clip are set from NEXUS_Display_P_ResetGraphics when the format changes */
    graphics->cfg.alpha = 0xFF;
    graphics->cfg.zorder = 2;
    graphics->cfg.sourceBlendFactor = NEXUS_CompositorBlendFactor_eSourceAlpha;
    graphics->cfg.destBlendFactor = NEXUS_CompositorBlendFactor_eInverseSourceAlpha;
    graphics->cfg.constantAlpha = 0xFF;
    graphics->cfg.horizontalFilter = NEXUS_GraphicsFilterCoeffs_eAuto;
    NEXUS_CallbackDesc_Init(&graphics->cfg.frameBufferCallback);
    graphics->frameBufferCallback = NEXUS_IsrCallback_Create(display, NULL);
    if(!graphics->frameBufferCallback) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_callback;
    }
    NEXUS_IsrCallback_Set(graphics->frameBufferCallback, &graphics->cfg.frameBufferCallback);
    return BERR_SUCCESS;
err_callback:
    return rc;
}

/* NEXUS_Display_P_UninitGraphics is only called from NEXUS_Display_Close */
void
NEXUS_Display_P_UninitGraphics(NEXUS_DisplayHandle display)
{
    struct NEXUS_DisplayGraphics *graphics = &display->graphics;

    BDBG_OBJECT_ASSERT(display, NEXUS_Display);

    NEXUS_Display_P_DestroyGraphics(display);

    NEXUS_IsrCallback_Destroy(graphics->frameBufferCallback);
    return;
}

void NEXUS_Display_GetGraphicsColorMatrix( NEXUS_DisplayHandle display, NEXUS_ColorMatrix *pColorMatrix )
{
    BDBG_OBJECT_ASSERT(display, NEXUS_Display);
    *pColorMatrix = display->graphics.colorMatrix;
}

NEXUS_Error NEXUS_Display_SetGraphicsColorMatrix( NEXUS_DisplayHandle display, const NEXUS_ColorMatrix *pColorMatrix )
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(display, NEXUS_Display);

    display->graphics.colorMatrixSet = (pColorMatrix != NULL);
    if (pColorMatrix && &display->graphics.colorMatrix != pColorMatrix) {
        display->graphics.colorMatrix = *pColorMatrix;
    }

    if (display->graphics.windowVdc) {
        if (pColorMatrix) {
            rc = BVDC_Window_SetColorMatrix(display->graphics.windowVdc, true, pColorMatrix->coeffMatrix, pColorMatrix->shift);
            if (rc) return BERR_TRACE(rc);
        }
        else {
            rc = BVDC_Window_SetColorMatrix(display->graphics.windowVdc, false, NULL, 0);
            if (rc) return BERR_TRACE(rc);
        }

        rc = NEXUS_Display_P_ApplyChanges();
        if (rc) return BERR_TRACE(rc);
    }
    return 0;
}

void NEXUS_Display_GetGraphicsColorSettings( NEXUS_DisplayHandle display, NEXUS_GraphicsColorSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(display, NEXUS_Display);
    *pSettings = display->graphics.colorSettings;
}

NEXUS_Error NEXUS_Display_SetGraphicsColorSettings( NEXUS_DisplayHandle display, const NEXUS_GraphicsColorSettings *pSettings )
{
    BERR_Code rc;
    BDBG_OBJECT_ASSERT(display, NEXUS_Display);
    display->graphics.colorSettings = *pSettings;
    if (display->graphics.windowVdc) {
        rc = BVDC_Window_SetContrast(display->graphics.windowVdc, pSettings->contrast);
        if (rc) return BERR_TRACE(rc);
        rc = BVDC_Window_SetSaturation(display->graphics.windowVdc, pSettings->saturation);
        if (rc) return BERR_TRACE(rc);
        rc = BVDC_Window_SetHue(display->graphics.windowVdc, pSettings->hue);
        if (rc) return BERR_TRACE(rc);
        rc = BVDC_Window_SetBrightness(display->graphics.windowVdc, pSettings->brightness);
        if (rc) return BERR_TRACE(rc);
        rc = NEXUS_Display_P_ApplyChanges();
        if (rc) return BERR_TRACE(rc);
    }
    return 0;
}

void NEXUS_Graphics_GetDefaultFramebuffer3D(NEXUS_GraphicsFramebuffer3D *pFramebuffer3D)
{
    BDBG_ASSERT(pFramebuffer3D);
    BKNI_Memset(pFramebuffer3D, 0, sizeof(*pFramebuffer3D));
    pFramebuffer3D->orientation = NEXUS_VideoOrientation_e2D;
    pFramebuffer3D->main = NULL;
    return;
}

