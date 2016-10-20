/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "atlas.h"
#include "config.h"
#include "graphics.h"
#include "display.h"
#include "nexus_surface_client.h"
#include "nexus_surface_compositor.h"

BDBG_MODULE(atlas_graphics);

static void setEventCallback(
        void * context,
        int    param
        )
{
    BSTD_UNUSED(param);
    B_Event_Set((B_EventHandle)context);
}

CSurfaceClient::CSurfaceClient(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    CResource(name, number, eBoardResource_surfaceClient, pCfg),
    _surfaceCompositor(NULL),
    _surfaceClient(NULL),
    _recycledEvent(NULL)
{
}

CSurfaceClient::~CSurfaceClient()
{
    close();
}

/* if given a surfaceCompositor (nexus case) we will create a new surface client.
 * if NOT given a surfaceCompositor (nxclient case) we will acquire a surface client.
 */
eRet CSurfaceClient::open(NEXUS_SurfaceCompositorHandle surfaceCompositor)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    BDBG_ASSERT(NULL == _surfaceClient);
    BDBG_ASSERT(NULL != surfaceCompositor);

    _surfaceCompositor = surfaceCompositor;

    _recycledEvent = B_Event_Create(NULL);
    CHECK_PTR_ERROR_GOTO("graphics displayed event create failed", _recycledEvent, ret, eRet_ExternalError, error);

    _surfaceClient = NEXUS_SurfaceCompositor_CreateClient(_surfaceCompositor, getNumber());
    CHECK_PTR_ERROR_GOTO("unable to create surface client", _surfaceClient, ret, eRet_OutOfMemory, error);

    {
        NEXUS_SurfaceClientSettings settingsClient;

        NEXUS_SurfaceClient_GetSettings(_surfaceClient, &settingsClient);
        settingsClient.recycled.callback = setEventCallback;
        settingsClient.recycled.context  = _recycledEvent;
        nerror                           = NEXUS_SurfaceClient_SetSettings(_surfaceClient, &settingsClient);
        CHECK_NEXUS_ERROR_GOTO("surface client set settings failed", ret, nerror, error);
    }

error:
    return(ret);
} /* open */

void CSurfaceClient::close()
{
    if (NULL != _surfaceClient)
    {
        NEXUS_SurfaceClient_Clear(_surfaceClient);
        if (NULL != _surfaceCompositor)
        {
            NEXUS_SurfaceCompositor_DestroyClient(_surfaceClient);
        }
        else
        {
            NEXUS_SurfaceClient_Release(_surfaceClient);
        }
        _surfaceClient = NULL;
    }

    if (NULL != _recycledEvent)
    {
        B_Event_Destroy(_recycledEvent);
        _recycledEvent = NULL;
    }
} /* close */

eRet CSurfaceClient::setSurface(NEXUS_SurfaceHandle surface)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    BDBG_ASSERT(NULL != surface);
    BDBG_ASSERT(NULL != getSurfaceClient());

    nerror = NEXUS_SurfaceClient_SetSurface(getSurfaceClient(), surface);
    CHECK_NEXUS_ERROR_GOTO("surface compositor set surface error", ret, nerror, error);
    {
        B_Error berror = B_Event_Wait(_recycledEvent, 1000);
        CHECK_BOS_ERROR_GOTO("Error waiting for surface client recycled event.", ret, berror, error);
        B_Event_Reset(_recycledEvent);
    }

error:
    return(ret);
} /* setSurface */

eRet CSurfaceClient::updateSurface(MRect * pRectUpdate)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    NEXUS_Rect * pRect = NULL;
    NEXUS_Rect   nrect;

    BDBG_ASSERT(NULL != getSurfaceClient());

    if (NULL != pRectUpdate)
    {
        nrect.x      = pRectUpdate->x();
        nrect.y      = pRectUpdate->y();
        nrect.width  = pRectUpdate->width();
        nrect.height = pRectUpdate->height();

        pRect = &nrect;
    }

    nerror = NEXUS_SurfaceClient_UpdateSurface(getSurfaceClient(), pRect);
    CHECK_NEXUS_ERROR_GOTO("surface compositor set surface error", ret, nerror, error);
    {
        B_Error berror = B_Event_Wait(_recycledEvent, 1000);
        CHECK_BOS_ERROR_GOTO("Error waiting for surface client recycled event.", ret, berror, error);
        B_Event_Reset(_recycledEvent);
    }

error:
    return(ret);
} /* updateSurface */

eRet CSurfaceClient::setBlend(
        NEXUS_BlendEquation * pBlendEquationAlpha,
        NEXUS_BlendEquation * pBlendEquationColor
        )
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;
    /* coverity[stack_use_local_overflow] */
    NEXUS_SurfaceCompositorClientSettings settingsClientSurface;

    NEXUS_SurfaceCompositor_GetClientSettings(_surfaceCompositor, _surfaceClient, &settingsClientSurface);
    /* set blending equations for desktopClient surface */
    if (NULL != pBlendEquationAlpha)
    {
        settingsClientSurface.composition.alphaBlend = *pBlendEquationAlpha;
    }
    if (NULL != pBlendEquationColor)
    {
        settingsClientSurface.composition.colorBlend = *pBlendEquationColor;
    }
    nerror = NEXUS_SurfaceCompositor_SetClientSettings(_surfaceCompositor, _surfaceClient, &settingsClientSurface);
    CHECK_NEXUS_ERROR_GOTO("unable to set blend equations", ret, nerror, error);

error:
    return(ret);
} /* setBlend */

eRet CSurfaceClient::setZOrder(uint16_t zOrder)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;
    /* coverity[stack_use_local_overflow] */
    NEXUS_SurfaceCompositorClientSettings settingsClientSurface;

    NEXUS_SurfaceCompositor_GetClientSettings(_surfaceCompositor, _surfaceClient, &settingsClientSurface);
    settingsClientSurface.composition.zorder = zOrder;
    nerror = NEXUS_SurfaceCompositor_SetClientSettings(_surfaceCompositor, _surfaceClient, &settingsClientSurface);
    CHECK_NEXUS_ERROR_GOTO("unable to set zorder", ret, nerror, error);

error:
    return(ret);
}

CGraphics::CGraphics(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    CResource(name, number, eBoardResource_graphics, pCfg),
    _pModel(NULL),
    _pBoardResources(NULL),
    _pSurfaceClientDesktop(NULL),
    _surfaceCompositor(NULL),
    _pDisplayPrimary(NULL),
    _pDisplaySecondary(NULL),
    _graphicsWidth(0),
    _graphicsHeight(0),
    _blitter(NULL),
    _checkpointEvent(NULL),
    _inactiveEvent(NULL),
    _surface(NULL),
    _bActive(false),
    _bwinEngine(NULL),
    _font10(NULL),
    _font12(NULL),
    _font14(NULL),
    _font17(NULL),
    _framebuffer(NULL),
    _winFramebuffer(NULL)
{
}

eRet CGraphics::open(CConfig * pConfig)
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NULL != pConfig);

    _pBoardResources = pConfig->getBoardResources();

    return(ret);
}

void CGraphics::close()
{
    uninitGraphics();
}

void CGraphics::graphicsCheckpoint()
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    BDBG_ASSERT(NULL != _blitter);
    nerror = NEXUS_Graphics2D_Checkpoint(_blitter, NULL);

    if (nerror == NEXUS_GRAPHICS2D_QUEUED)
    {
        B_Error berr = B_ERROR_SUCCESS;
        berr = B_Event_Wait(_checkpointEvent, 5555 /*B_WAIT_FOREVER*/);
        CHECK_BOS_ERROR_GOTO("timeout waiting for graphics checkpoint", ret, berr, error);
    }

error:
    return;
} /* graphicsCheckpoint */

void CGraphics::flush(NEXUS_SurfaceHandle surface)
{
    if (NULL == surface)
    {
        surface = _surface;
    }

    BDBG_ASSERT(NULL != surface);
    NEXUS_Surface_Flush(surface);
}

void CGraphics::setActive(bool bActive)
{
    eRet ret = eRet_Ok;
    NEXUS_SurfaceCompositorSettings settings;

    BDBG_ASSERT(NULL != _surfaceCompositor);

    if (_bActive == bActive)
    {
        return;
    }

    if (false == bActive)
    {
        B_Event_Reset(_inactiveEvent);
    }

    NEXUS_SurfaceCompositor_GetSettings(_surfaceCompositor, &settings);
    settings.enabled = bActive;
    NEXUS_SurfaceCompositor_SetSettings(_surfaceCompositor, &settings);

    if (false == bActive)
    {
        B_Error berr = B_ERROR_SUCCESS;
        berr = B_Event_Wait(_inactiveEvent, 5555 /*B_WAIT_FOREVER*/);
        CHECK_BOS_ERROR_GOTO("timeout waiting for graphics activate/deactivate", ret, berr, error);
    }

    _bActive = bActive;

error:
    return;
} /* setActive */

static void do_sync(bwin_framebuffer_t fb)
{
    CGraphicsData *           pGraphicsData = NULL;
    bwin_framebuffer_settings settings;

    BDBG_MSG(("********** DO_SYNC **********"));

    bwin_get_framebuffer_settings(fb, &settings);
    pGraphicsData = (CGraphicsData *)settings.data;

    BDBG_ASSERT((NULL != pGraphicsData) && (NULL != pGraphicsData->_pGraphics));
    pGraphicsData->_pGraphics->sync(fb);
}

static void do_copy_rect(
        bwin_framebuffer_t destfb,
        const bwin_rect *  destrect,
        bwin_framebuffer_t srcfb,
        const bwin_rect *  srcrect
        )
{
    CGraphics *               pGraphics         = NULL;
    CGraphicsData *           pGraphicsDataSrc  = NULL;
    CGraphicsData *           pGraphicsDataDest = NULL;
    bwin_framebuffer_settings fbSettingsSrc;
    bwin_framebuffer_settings fbSettingsDest;

    BDBG_ASSERT(NULL != destfb);
    BDBG_ASSERT(NULL != srcfb);

    bwin_get_framebuffer_settings(srcfb, &fbSettingsSrc);
    bwin_get_framebuffer_settings(destfb, &fbSettingsDest);

    pGraphicsDataSrc  = (CGraphicsData *)fbSettingsSrc.data;
    pGraphicsDataDest = (CGraphicsData *)fbSettingsDest.data;
    BDBG_ASSERT((NULL != pGraphicsDataSrc) && (NULL != pGraphicsDataDest));
    BDBG_ASSERT((12346 == pGraphicsDataSrc->getMagic()) && (12346 == pGraphicsDataDest->getMagic()));

    /* get CGraphics object from dest or src */
    pGraphics = pGraphicsDataDest->_pGraphics ? pGraphicsDataDest->_pGraphics : pGraphicsDataSrc->_pGraphics;
    BDBG_ASSERT(NULL != pGraphics);

    if (false == pGraphics->isActive())
    {
        BDBG_WRN(("unable to copy rect, graphics are NOT active"));
        return;
    }

    NEXUS_Rect nsrcrect  = bwinToNexusRect(*srcrect);
    NEXUS_Rect ndestrect = bwinToNexusRect(*destrect);

    BDBG_WRN(("do_copy_rect() src surface:%p x:%d y:%d w:%d h:%d  dest surface:%p x:%d y:%d w:%d h:%d",
              (void *)pGraphicsDataSrc->_surface, nsrcrect.x, nsrcrect.y, nsrcrect.width, nsrcrect.height,
              (void *)pGraphicsDataDest->_surface, ndestrect.x, ndestrect.y, ndestrect.width, ndestrect.height));

    BDBG_ASSERT((NULL != pGraphicsDataSrc->_surface) && (NULL != pGraphicsDataDest->_surface));
    {
        pGraphics->flush();

        NEXUS_Graphics2DBlitSettings blitSettings;
        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.colorOp        = NEXUS_BlitColorOp_eCopySource;
        blitSettings.alphaOp        = NEXUS_BlitAlphaOp_eCopyConstant; /* YCrCb has no alpha, so we must set 0xFF */
        blitSettings.constantColor  = COLOR_BLACK;                     /* alpha is opaque */
        blitSettings.source.surface = pGraphicsDataSrc->_surface;
        blitSettings.source.rect    = nsrcrect;
        blitSettings.output.surface = pGraphicsDataDest->_surface;
        blitSettings.output.rect    = ndestrect;
        NEXUS_Graphics2D_Blit(pGraphics->getBlitter(), &blitSettings);

        pGraphics->graphicsCheckpoint();
        pGraphics->flush();
    }
} /* do_copy_rect */

void CGraphics::sync(bwin_framebuffer_t framebuffer)
{
    eRet ret = eRet_Ok;

    BSTD_UNUSED(framebuffer);

    if (false == isActive())
    {
        BDBG_WRN(("unable to sync, graphics are NOT active"));
        goto error;
    }

    graphicsCheckpoint();
    flush();

    ret = getSurfaceClientDesktop()->updateSurface();
    CHECK_ERROR_GOTO("unable update surface", ret, error);

error:
    return;
} /* sync */

eRet CGraphics::initGraphics(
        uint16_t width,
        uint16_t height
        )
{
    eRet                         ret    = eRet_Ok;
    NEXUS_Error                  nerror = NEXUS_SUCCESS;
    NEXUS_Graphics2DSettings     graphicsSettings;
    NEXUS_SurfaceCreateSettings  surfaceSettings;
    NEXUS_Graphics2DFillSettings fillSettings;

    BDBG_ASSERT(NULL != _pModel);

    /* headless box modes will not have any available displays, so we
     * will not open any graphics.  note that we will still need bwin
     * to handle notifications and the main loop! */
    if ((NULL != getDisplay(0)) || (NULL != getDisplay(1)))
    {
        _blitter = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, NULL);

        _checkpointEvent = B_Event_Create(NULL);
        CHECK_PTR_ERROR_GOTO("graphics checkpoint event create failed", _checkpointEvent, ret, eRet_ExternalError, error);
        _inactiveEvent = B_Event_Create(NULL);
        CHECK_PTR_ERROR_GOTO("graphics inactive event create failed", _inactiveEvent, ret, eRet_ExternalError, error);

        NEXUS_Graphics2D_GetSettings(_blitter, &graphicsSettings);
        graphicsSettings.checkpointCallback.callback = setEventCallback;
        graphicsSettings.checkpointCallback.context  = _checkpointEvent;
        nerror = NEXUS_Graphics2D_SetSettings(_blitter, &graphicsSettings);
        CHECK_NEXUS_ERROR_GOTO("unable to set graphics2D settings", ret, nerror, error);

        {
            MRect rectMaxGraphicsHD = _pDisplayPrimary->getMaxGraphicsGeometry();

            if (rectMaxGraphicsHD.width() < width)
            {
                /* use nexus reported max graphics size instead of given size */
                width  = rectMaxGraphicsHD.width();
                height = rectMaxGraphicsHD.height();
                BDBG_WRN(("Nexus reported max graphics size differs from given size.  Using width:%d height:%d", width, height));
            }
        }

        /* create framebuffer surface for main atlas */
        NEXUS_Surface_GetDefaultCreateSettings(&surfaceSettings);
        surfaceSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
        surfaceSettings.width       = width;
        surfaceSettings.height      = height;
        /* surfaceSettings.heap        = NEXUS_Platform_GetFramebufferHeap(0); */
        _surface = NEXUS_Surface_Create(&surfaceSettings);
        BDBG_MSG(("Creating _surface w=%d h=%d", surfaceSettings.width, surfaceSettings.height));
        CHECK_PTR_ERROR_GOTO("surface create for desktopClient failed.", _surface, ret, eRet_ExternalError, error);

        /* fill main atlas surface with transparent */
        NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
        fillSettings.surface = _surface;
        fillSettings.color   = 0x0; /* set default to transparent for video window to show through */
        nerror               = NEXUS_Graphics2D_Fill(_blitter, &fillSettings);
        CHECK_NEXUS_ERROR_GOTO("graphics fill for _surface error", ret, nerror, error);

        graphicsCheckpoint();

        /* create surface compositor */
        {
            NEXUS_SurfaceCompositorSettings settings;

            _surfaceCompositor = NEXUS_SurfaceCompositor_Create(0);
            NEXUS_SurfaceCompositor_GetSettings(_surfaceCompositor, &settings);
            settings.inactiveCallback.callback = setEventCallback;
            settings.inactiveCallback.context  = _inactiveEvent;

            MRect rectMaxGraphicsHD = _pDisplayPrimary->getMaxGraphicsGeometry();

            NEXUS_Display_GetGraphicsSettings(_pDisplayPrimary->getDisplay(), &settings.display[0].graphicsSettings);
            settings.display[0].graphicsSettings.enabled    = true;
            settings.display[0].display                     = _pDisplayPrimary->getDisplay();
            settings.display[0].framebuffer.number          = 2;
            settings.display[0].framebuffer.width           = rectMaxGraphicsHD.width();
            settings.display[0].framebuffer.height          = rectMaxGraphicsHD.height();
            settings.display[0].framebuffer.backgroundColor = 0x0; /* transparent background for video window to show through*/
            settings.display[0].framebuffer.heap            = NEXUS_Platform_GetFramebufferHeap(0);
            BDBG_WRN(("HD framebuffer w:%d h:%d", settings.display[0].framebuffer.width, settings.display[0].framebuffer.height));

            if (NULL != _pDisplaySecondary)
            {
                MRect rectMaxGraphicsSD = _pDisplaySecondary->getMaxGraphicsGeometry();

                NEXUS_Display_GetGraphicsSettings(_pDisplaySecondary->getDisplay(), &settings.display[1].graphicsSettings);
                settings.display[1].graphicsSettings.enabled    = true;
                settings.display[1].display                     = _pDisplaySecondary->getDisplay();
                settings.display[1].framebuffer.number          = 2;
                settings.display[1].framebuffer.width           = rectMaxGraphicsSD.width();
                settings.display[1].framebuffer.height          = rectMaxGraphicsSD.height();
                settings.display[1].framebuffer.backgroundColor = 0x0; /*transparent background for video window to show through*/
                settings.display[1].framebuffer.heap            = NEXUS_Platform_GetFramebufferHeap(1);
                BDBG_WRN(("SD framebuffer w:%d h:%d", settings.display[1].framebuffer.width, settings.display[1].framebuffer.height));
            }

            /*
             * settings.frameBufferCallback.callback = framebuffer_callback;
             * settings.frameBufferCallback.context  = _surfaceCompositor;
             */
            nerror = NEXUS_SurfaceCompositor_SetSettings(_surfaceCompositor, &settings);
            CHECK_NEXUS_ERROR_GOTO("surface compositor set settings error", ret, nerror, error);
        }

        /*
         * create desktopClient - atlas is a surface server but also draws like a client
         *      default settings make it fullscreen, zorder=0
         */
        {
            /* BLENDING_TYPE_SRC_OVER_NON_PREMULTIPLIED */
            NEXUS_BlendEquation alphaBlendEquation = {
                NEXUS_BlendFactor_eSourceAlpha,
                NEXUS_BlendFactor_eOne,
                false,
                NEXUS_BlendFactor_eDestinationAlpha,
                NEXUS_BlendFactor_eInverseSourceAlpha,
                false,
                NEXUS_BlendFactor_eZero
            };
            /* BLENDING_TYPE_SRC_OVER_NON_PREMULTIPLIED */
            NEXUS_BlendEquation colorBlendEquation = {
                NEXUS_BlendFactor_eSourceColor,
                NEXUS_BlendFactor_eSourceAlpha,
                false,
                NEXUS_BlendFactor_eDestinationColor,
                NEXUS_BlendFactor_eInverseSourceAlpha,
                false,
                NEXUS_BlendFactor_eZero
            };

            _pSurfaceClientDesktop = (CSurfaceClient *)_pBoardResources->checkoutResource(_pModel->getId(), eBoardResource_surfaceClient);
            CHECK_PTR_ERROR_GOTO("unable to checkout Desktop surface client resource", _pSurfaceClientDesktop, ret, eRet_NotAvailable, error);
            ret = _pSurfaceClientDesktop->open(_surfaceCompositor);
            CHECK_ERROR_GOTO("unable to open desktop surface client", ret, error);
            ret = _pSurfaceClientDesktop->setBlend(&alphaBlendEquation, &colorBlendEquation);
            CHECK_ERROR_GOTO("unable to set blend equations for desktop surface client", ret, error);
            ret = _pSurfaceClientDesktop->setZOrder(1); /* raise atlas surface z order above close caption */
            CHECK_ERROR_GOTO("unable to set z-order for desktop surface client", ret, error);
            ret = _pSurfaceClientDesktop->setSurface(_surface);
            CHECK_ERROR_GOTO("unable to set surface with desktop surface client", ret, error);
        }

        _graphicsWidth  = width;
        _graphicsHeight = height;
    }

    ret = initBwin(_surface);
    CHECK_ERROR_GOTO("unable to initialize bwidgets/bwin", ret, error);

    _bActive = true;
    return(ret);

error:
    uninitGraphics();
    return(ret);
} /* initGraphics */

void CGraphics::uninitGraphics()
{
    eRet ret = eRet_Ok;

    _bActive = false;

    _graphicsHeight = 0;
    _graphicsWidth  = 0;

    uninitBwin();

    if (NULL != _pSurfaceClientDesktop)
    {
        _pSurfaceClientDesktop->close();
        ret = _pBoardResources->checkinResource(_pSurfaceClientDesktop);
        CHECK_ERROR_GOTO("unable to checkin Desktop surface client resource", ret, error);

        _pSurfaceClientDesktop = NULL;
    }

    if (NULL != _surfaceCompositor)
    {
        NEXUS_SurfaceCompositor_Destroy(_surfaceCompositor);
        _surfaceCompositor = NULL;
    }

    if (NULL != _surface)
    {
        NEXUS_Surface_Destroy(_surface);
        _surface = NULL;
    }

    if (NULL != _blitter)
    {
        NEXUS_Graphics2D_Close(_blitter);
        _blitter = NULL;
    }

    if (NULL != _checkpointEvent)
    {
        B_Event_Destroy(_checkpointEvent);
        _checkpointEvent = NULL;
    }

    if (NULL != _inactiveEvent)
    {
        B_Event_Destroy(_inactiveEvent);
        _inactiveEvent = NULL;
    }

error:
    return;
} /* uninitGraphics */

CGraphics::~CGraphics()
{
    close();
}

void CGraphics::dump()
{
}

bwin_font_t CGraphics::getFont(uint8_t size)
{
    bwin_font_t font = NULL;

    switch (size)
    {
    case 10:
        font = _font10;
        break;
    case 12:
        font = _font12;
        break;
    case 14:
        font = _font14;
        break;
    case 17:
        font = _font17;
        break;
    default:
        BDBG_ERR(("invalid font size given"));
        break;
    } /* switch */

    return(font);
} /* getFont */

/* start bwin and bwidgets */
eRet CGraphics::initBwin(NEXUS_SurfaceHandle surface)
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NULL != _bwinEngine);

    if (NULL == surface)
    {
        /* no graphics */
        goto done;
    }

    _font10 = bwin_open_font(_bwinEngine, "fonts/verdana_10.bwin_font", -1, true);
    CHECK_PTR_ERROR_GOTO("Font creation failed - 'make install' and untar the tarball before starting Atlas.", _font10, ret, eRet_ExternalError, error);
    _font12 = bwin_open_font(_bwinEngine, "fonts/verdana_12.bwin_font", -1, true);
    CHECK_PTR_ERROR_GOTO("Font creation failed", _font12, ret, eRet_ExternalError, error);
    _font14 = bwin_open_font(_bwinEngine, "fonts/verdana_14.bwin_font", -1, true);
    CHECK_PTR_ERROR_GOTO("Font creation failed", _font14, ret, eRet_ExternalError, error);
    _font17 = bwin_open_font(_bwinEngine, "fonts/verdana_17.bwin_font", -1, true);
    CHECK_PTR_ERROR_GOTO("Font creation failed", _font17, ret, eRet_ExternalError, error);

    _framebuffer = createBwinFramebuffer(surface);
    CHECK_PTR_ERROR_GOTO("unable to create bwin framebuffer", _framebuffer, ret, eRet_OutOfMemory, error);
    {
        bwin_framebuffer_settings fbSettings;
        bwin_get_framebuffer_settings(_framebuffer, &fbSettings);
        _winFramebuffer = fbSettings.window;
    }
    goto done;
error:
    if (NULL != _framebuffer)
    {
        destroyBwinFramebuffer(_framebuffer);
        _framebuffer = NULL;
    }
done:
    return(ret);
} /* initBwin */

void CGraphics::uninitBwin()
{
    _winFramebuffer = NULL;

    if (NULL != _framebuffer)
    {
        destroyBwinFramebuffer(_framebuffer);
        _framebuffer = NULL;
    }

    if (NULL != _font10)
    {
        bwin_close_font(_font10);
        _font10 = NULL;
    }

    if (NULL != _font12)
    {
        bwin_close_font(_font12);
        _font12 = NULL;
    }

    if (NULL != _font14)
    {
        bwin_close_font(_font14);
        _font14 = NULL;
    }

    if (NULL != _font17)
    {
        bwin_close_font(_font17);
        _font17 = NULL;
    }

    setBwin(NULL);
} /* uninitBwin */

eRet CGraphics::getFramebufferSettings(bwin_framebuffer_settings * pSettings)
{
    eRet ret = eRet_NotAvailable;

    if (NULL != _framebuffer)
    {
        bwin_get_framebuffer_settings(_framebuffer, pSettings);
        ret = eRet_Ok;
    }

    return(ret);
} /* getFramebufferSettings */

eRet CGraphics::setFramebufferSize(CDisplay * pDisplay)
{
    NEXUS_SurfaceCompositorSettings settings;
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    if (NULL == pDisplay)
    {
        /* given display does not exist */
        return(ret);
    }

    BDBG_ASSERT(NULL != _surfaceCompositor);

    NEXUS_SurfaceCompositor_GetSettings(_surfaceCompositor, &settings);
    /* BDBG_ASSERT(false == settings.enabled); */

    {
        MRect                 rectMaxFramebuffer = pDisplay->getMaxGraphicsGeometry();
        MRect                 rectClip;
        NEXUS_VideoFormatInfo formatInfo;

        NEXUS_VideoFormat_GetInfo(pDisplay->getFormat(), &formatInfo);
        /* assume video format is our framebuffer clipping size */
        rectClip.setWidth(formatInfo.width);
        rectClip.setHeight(formatInfo.height);

        if ((rectClip.width() > rectMaxFramebuffer.width()) ||
            (rectClip.height() > rectMaxFramebuffer.height()))
        {
            /* video format dimensions are bigger than our framebuffer so keep clip rect at the max allowed */
            rectClip.setWidth(rectMaxFramebuffer.width());
            rectClip.setHeight(rectMaxFramebuffer.height());
        }

        BDBG_MSG(("set display%d format:%s framebuffer size w:%d h:%d",
                  pDisplay->getNumber(), videoFormatToString(pDisplay->getFormat()).s(), rectClip.width(), rectClip.height()));

        /* set simple compositor framebuffer size */
        settings.display[pDisplay->getNumber()].graphicsSettings.clip.width  = rectClip.width();
        settings.display[pDisplay->getNumber()].graphicsSettings.clip.height = rectClip.height();
        nerror = NEXUS_SurfaceCompositor_SetSettings(_surfaceCompositor, &settings);
        CHECK_NEXUS_ERROR_GOTO("unable to set simple compositor framebuffer size", ret, nerror, error);
    }

error:
    return(ret);
} /* setFramebufferSize */

eRet CGraphics::destripeToSurface(
        NEXUS_StripedSurfaceHandle stripedSurface,
        NEXUS_SurfaceHandle        stillSurface,
        MRect                      rectStill
        )
{
    NEXUS_Error nerror = NEXUS_SUCCESS;
    eRet        ret    = eRet_Ok;
    NEXUS_Rect  rectTarget;

    BDBG_ASSERT(NULL != stripedSurface);
    BDBG_ASSERT(NULL != stillSurface);

    if (false == isActive())
    {
        BDBG_WRN(("unable to destripe to surface, graphics are NOT active"));
        goto error;
    }

    rectTarget.x      = rectStill.x();
    rectTarget.y      = rectStill.y();
    rectTarget.width  = rectStill.width();
    rectTarget.height = rectStill.height();

    nerror = NEXUS_Graphics2D_DestripeToSurface(getBlitter(), stripedSurface, stillSurface, &rectTarget);
    CHECK_NEXUS_ERROR_GOTO("unable to destripe given surface", ret, nerror, error);
    graphicsCheckpoint();
    flush(stillSurface);

error:
    return(ret);
} /* destripeToSurface */

/* bwin framebuffers created with this method must call destroyBwinFramebuffer() to destroy them
 * or it will memory leak the CGraphicsData object.  Also, destruction of the given nexus surface remains
 * the resonsibility of the calling code. */
bwin_framebuffer_t CGraphics::createBwinFramebuffer(
        NEXUS_SurfaceHandle surface,
        uint16_t            width,
        uint16_t            height
        )
{
    eRet                        ret             = eRet_Ok;
    NEXUS_Error                 nerror          = NEXUS_SUCCESS;
    bwin_framebuffer_t          bwinFramebuffer = NULL;
    CGraphicsData *             pGraphicsData   = NULL;
    NEXUS_SurfaceCreateSettings surfaceSettings;
    NEXUS_SurfaceMemory         surfaceMem;
    int16_t                     newWidth  = width;
    int16_t                     newHeight = height;

    BDBG_ASSERT(NULL != surface);
    NEXUS_Surface_GetCreateSettings(surface, &surfaceSettings);

    if (0 == width)
    {
        newWidth = surfaceSettings.width;
    }
    if (0 == height)
    {
        newHeight = surfaceSettings.height;
    }

    nerror = NEXUS_Surface_GetMemory(surface, &surfaceMem);
    CHECK_NEXUS_ERROR_GOTO("unable to get thumb surface memory", ret, nerror, error);

    pGraphicsData = new CGraphicsData(this, surface);
    CHECK_PTR_ERROR_GOTO("unable to allocate graphics data", pGraphicsData, ret, eRet_OutOfMemory, error);

    BDBG_MSG(("createBwinFramebuffer() w:%d h:%d", surfaceSettings.width, surfaceSettings.height));
    {
        bwin_framebuffer_settings fbSettings;

        bwin_framebuffer_settings_init(&fbSettings);
        fbSettings.buffer        = surfaceMem.buffer;
        fbSettings.pitch         = surfaceMem.pitch;
        fbSettings.height        = newHeight;
        fbSettings.width         = newWidth;
        fbSettings.second_buffer = NULL;
        fbSettings.pixel_format  = bwin_pixel_format_a8_r8_g8_b8;
        fbSettings.drawops.sync  = do_sync;
        BSTD_UNUSED(do_copy_rect); /*TTTTTTTTT fbSettings.drawops.copy_rect = do_copy_rect; */
        fbSettings.data = pGraphicsData;
        bwinFramebuffer = bwin_open_framebuffer(getWinEngine(), &fbSettings);
        CHECK_PTR_ERROR_GOTO("unable to open framebuffer", bwinFramebuffer, ret, eRet_ExternalError, error);
    }

    goto done;
error:
    if (NULL != bwinFramebuffer)
    {
        /*
         * we will keep this error handling even tho it is dead code at this point.
         * coverity[dead_error_begin]
         */
        bwin_close_framebuffer(bwinFramebuffer);
        bwinFramebuffer = NULL;
    }

    if (NULL != pGraphicsData)
    {
        delete pGraphicsData;
        pGraphicsData = NULL;
    }
done:
    return(bwinFramebuffer);
} /* createBwinFramebuffer */

/* destroy bwin framebuffers created with createBwinFramebuffer() method.  This is done to ensure the
 * attached CGraphicsData object is properly destroyed.  Note that the nexus surface given in
 * createBwinFramebuffer() is NOT destroyed - responsibility for that remains with the calling code. */
eRet CGraphics::destroyBwinFramebuffer(bwin_framebuffer_t framebuffer)
{
    eRet ret = eRet_Ok;
    bwin_framebuffer_settings fbSettings;

    bwin_get_framebuffer_settings(framebuffer, &fbSettings);

    CGraphicsData * pGraphicsData = (CGraphicsData *)fbSettings.data;
    if (NULL != pGraphicsData)
    {
        delete pGraphicsData;
        pGraphicsData = NULL;
    }

    bwin_close_framebuffer(framebuffer);
    framebuffer = NULL;

    return(ret);
} /* destroyBwinFramebuffer */