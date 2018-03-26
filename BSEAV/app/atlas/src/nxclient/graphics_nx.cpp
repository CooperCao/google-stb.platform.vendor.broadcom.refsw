/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "board.h"
#include "nxclient.h"
#include "graphics_nx.h"
#include "display.h"
#include "nexus_surface_client.h"
#include "nexus_surface_compositor.h"

BDBG_MODULE(graphics);

static void setEventCallback(
        void * context,
        int    param
        )
{
    BSTD_UNUSED(param);
    B_Event_Set((B_EventHandle)context);
}

CSurfaceClientNx::CSurfaceClientNx(
        const char *     name,
        const unsigned   number,
        CConfiguration * pCfg
        ) :
    CSurfaceClient(name, number, pCfg)
{
}

CSurfaceClientNx::~CSurfaceClientNx()
{
    CSurfaceClient::close();
}

/* if given a surfaceCompositor (nexus case) we will create a new surface client.
 * if NOT given a surfaceCompositor (nxclient case) we will acquire a surface client.
 */
eRet CSurfaceClientNx::open(NEXUS_SurfaceCompositorHandle surfaceCompositor)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    BDBG_ASSERT(NULL == _surfaceClient);

    _surfaceCompositor = surfaceCompositor;

    _recycledEvent = B_Event_Create(NULL);
    CHECK_PTR_ERROR_GOTO("graphics displayed event create failed", _recycledEvent, ret, eRet_ExternalError, error);

    _surfaceClient = NEXUS_SurfaceClient_Acquire(getNumber());
    CHECK_PTR_ERROR_GOTO("unable to acquire surface client", _surfaceClient, ret, eRet_NotAvailable, error);

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

void CGraphicsNx::setActive(bool bActive)
{
    eRet ret = eRet_Ok;

    if (_bActive == bActive)
    {
        return;
    }

    _bActive = bActive;

error:
    return;
} /* setActive */

eRet CSurfaceClientNx::setBlend(
        NEXUS_BlendEquation * pBlendEquationAlpha,
        NEXUS_BlendEquation * pBlendEquationColor
        )
{
    eRet                     ret    = eRet_Ok;
    NEXUS_Error              nerror = NEXUS_SUCCESS;
    NEXUS_SurfaceComposition settingsClientSurface;

    NxClient_GetSurfaceClientComposition(getNumber(), &settingsClientSurface);
    /* set blending equations for desktopClient surface */
    if (NULL != pBlendEquationAlpha)
    {
        settingsClientSurface.alphaBlend = *pBlendEquationAlpha;
    }
    if (NULL != pBlendEquationColor)
    {
        settingsClientSurface.colorBlend = *pBlendEquationColor;
    }
    nerror = NxClient_SetSurfaceClientComposition(getNumber(), &settingsClientSurface);
    CHECK_NEXUS_ERROR_GOTO("unable to set blend equations", ret, nerror, error);

error:
    return(ret);
} /* setBlend */

eRet CSurfaceClientNx::setZOrder(unsigned zOrder)
{
    eRet                     ret    = eRet_Ok;
    NEXUS_Error              nerror = NEXUS_SUCCESS;
    NEXUS_SurfaceComposition settingsClientSurface;

    NxClient_GetSurfaceClientComposition(getNumber(), &settingsClientSurface);
    settingsClientSurface.zorder = zOrder;
    nerror                       = NxClient_SetSurfaceClientComposition(getNumber(), &settingsClientSurface);
    CHECK_NEXUS_ERROR_GOTO("unable to set zorder", ret, nerror, error);

error:
    return(ret);
}

CGraphicsNx::CGraphicsNx(
        const char *     name,
        const unsigned   number,
        CConfiguration * pCfg
        ) :
    CGraphics(name, number, pCfg),
    _pSurfaceClientVideo(NULL)
{
}

CGraphicsNx::~CGraphicsNx()
{
    close();
}

eRet CGraphicsNx::initGraphics(
        unsigned width,
        unsigned height
        )
{
    eRet                         ret    = eRet_Ok;
    NEXUS_Error                  nerror = NEXUS_SUCCESS;
    NEXUS_Graphics2DSettings     graphicsSettings;
    NEXUS_SurfaceCreateSettings  surfaceSettings;
    NEXUS_Graphics2DFillSettings fillSettings;

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

    /* create framebuffer surface for main atlas */
    NEXUS_Surface_GetDefaultCreateSettings(&surfaceSettings);
    surfaceSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    surfaceSettings.width       = width;
    surfaceSettings.height      = height;
    /* createSettings.heap is NULL. proxy will populate. */
    _surface = NEXUS_Surface_Create(&surfaceSettings);
    BDBG_MSG(("Creating _surface w=%d h=%d", surfaceSettings.width, surfaceSettings.height));
    CHECK_PTR_ERROR_GOTO("surface create failed.", _surface, ret, eRet_ExternalError, error);

    /* fill surface with black */
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = _surface;
    fillSettings.color   = 0x0; /* set default to transparent for video window to show through */
    fillSettings.color   = COLOR_BLACK;
    nerror               = NEXUS_Graphics2D_Fill(_blitter, &fillSettings);
    CHECK_NEXUS_ERROR_GOTO("graphics fill error", ret, nerror, error);

    graphicsCheckpoint();

    /*
     * create desktopClient - atlas is a surface server but also draws like a client
     *      default settings make it fullscreen, zorder=0
     */
    {
        _pSurfaceClientDesktop = (CSurfaceClient *)_pBoardResources->checkoutResource(_pModel->getId(), eBoardResource_surfaceClient);
        CHECK_PTR_ERROR_GOTO("unable to checkout Desktop surface client resource", _pSurfaceClientDesktop, ret, eRet_NotAvailable, error);

        ret = _pSurfaceClientDesktop->open(_surfaceCompositor);
        CHECK_ERROR_GOTO("unable to open desktop surface client", ret, error);

        ret = _pSurfaceClientDesktop->setSurface(_surface);
        CHECK_ERROR_GOTO("unable to set surface with desktop surface client", ret, error);

        /*
         * coverity[stack_use_overflow]
         * coverity[stack_use_local_overflow]
         * BLENDING_TYPE_SRC_OVER_NON_PREMULTIPLIED
         */
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
        ret = _pSurfaceClientDesktop->setBlend(&alphaBlendEquation, &colorBlendEquation);
        CHECK_ERROR_GOTO("unable to set blend equations for desktop surface client", ret, error);
        ret = _pSurfaceClientDesktop->setZOrder(1); /* raise atlas surface z order above close caption */
        CHECK_ERROR_GOTO("unable to set z-order for desktop surface client", ret, error);
    }

    _graphicsWidth  = width;
    _graphicsHeight = height;

    ret = initBwin(_surface);
    CHECK_ERROR_GOTO("unable to initialize bwidgets/bwin", ret, error);

    _bActive = true;
    return(ret);

error:
    uninitGraphics();
    return(ret);
} /* initGraphics */

void CGraphicsNx::uninitGraphics()
{
    eRet ret = eRet_Ok;

    uninitBwin();

    if (NULL != _pSurfaceClientDesktop)
    {
        _pSurfaceClientDesktop->close();
        ret = _pBoardResources->checkinResource(_pSurfaceClientDesktop);
        CHECK_ERROR("unable to checkin Desktop surface client resource", ret);

        _pSurfaceClientDesktop = NULL;
    }

    if (NULL != _surface)
    {
        NEXUS_Surface_Destroy(_surface);
        _surface = NULL;
    }

    if (NULL != _inactiveEvent)
    {
        B_Event_Destroy(_inactiveEvent);
        _inactiveEvent = NULL;
    }

    if (NULL != _checkpointEvent)
    {
        B_Event_Destroy(_checkpointEvent);
        _checkpointEvent = NULL;
    }

    if (NULL != _blitter)
    {
        NEXUS_Graphics2D_Close(_blitter);
        _blitter = NULL;
    }
} /* uninitGraphics */