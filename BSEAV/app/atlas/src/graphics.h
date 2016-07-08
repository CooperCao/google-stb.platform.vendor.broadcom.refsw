/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

#ifndef GRAPHICS_H__
#define GRAPHICS_H__

#include "output.h"
#include "bwidgets.h"
#include "mgeom.h"

#define UINT32_C(c)  c ## U
#include "nexus_graphics2d.h"
#include "nexus_core_utils.h"
#include "nexus_surface_compositor.h"

#ifdef __cplusplus
extern "C" {
#endif

class CGraphics;
class CDisplay;
class CConfig;
class CModel;

/* data saved in bwin framebuffer data section for nexus/bwidgets/bwin integration */
class CGraphicsData
{
public:
    CGraphicsData(
            CGraphics *         pGraphics,
            NEXUS_SurfaceHandle surface
            ) :
        _pGraphics(pGraphics),
        _surface(surface) {}

    uint32_t getMagic(void) { return(12346); }

public:
    CGraphics *         _pGraphics;
    NEXUS_SurfaceHandle _surface;
};

class CSurfaceClient : public CResource
{
public:
    CSurfaceClient(
            const char *     name,
            const uint16_t   number,
            CConfiguration * pCfg
            );
    ~CSurfaceClient(void);

    virtual eRet open(NEXUS_SurfaceCompositorHandle surfaceCompositor = NULL);
    virtual void close(void);
    virtual eRet setBlend(NEXUS_BlendEquation * pBlendEquationAlpha, NEXUS_BlendEquation * pBlendEquationColor);
    virtual eRet setZOrder(uint16_t zOrder);

    eRet setSurface(NEXUS_SurfaceHandle surface);
    eRet updateSurface(MRect * pRectUpdate = NULL);

    NEXUS_SurfaceClientHandle     getSurfaceClient(void)     { return(_surfaceClient); }
    NEXUS_SurfaceCompositorHandle getSurfaceCompositor(void) { return(_surfaceCompositor); }

protected:
    NEXUS_SurfaceCompositorHandle _surfaceCompositor;
    NEXUS_SurfaceClientHandle     _surfaceClient;
    B_EventHandle                 _recycledEvent;
};

class CGraphics : public CResource
{
public:
    CGraphics(
            const char *     name,
            const uint16_t   number,
            CConfiguration * pCfg
            );
    ~CGraphics(void);

    virtual eRet open(CConfig * pConfig);
    virtual void close(void);
    virtual eRet initGraphics(uint16_t width, uint16_t height);
    virtual void uninitGraphics(void);
    virtual void dump();

    void                          setModel(CModel * pModel) { _pModel = pModel; }
    CModel *                      getModel(void)            { return(_pModel); }
    eRet                          initBwin(NEXUS_SurfaceHandle surface);
    void                          uninitBwin(void);
    bwin_engine_t                 getWinEngine(void)            { return(_bwinEngine); }
    NEXUS_Graphics2DHandle        getBlitter(void)              { return(_blitter); }
    NEXUS_SurfaceCompositorHandle getSurfaceCompositor(void)    { return(_surfaceCompositor); }
    CSurfaceClient *              getSurfaceClientDesktop(void) { return(_pSurfaceClientDesktop); }
    NEXUS_SurfaceHandle           getSurface(void)              { return(_surface); }
    bwin_t                        getWinFramebuffer(void)       { return(_winFramebuffer); }
    bwin_font_t                   getFont(uint8_t size = 12);
    void                          setActive(bool bActive);
    bool                          isActive(void) { return(_bActive); }
    eRet                          destripeToSurface(NEXUS_StripedSurfaceHandle stripedSurface, NEXUS_SurfaceHandle stillSurface, MRect rectStill);
    bwin_framebuffer_t            createBwinFramebuffer(NEXUS_SurfaceHandle surface, uint16_t width = 0, uint16_t height = 0);
    eRet                          destroyBwinFramebuffer(bwin_framebuffer_t framebuffer);
    eRet                          setFramebufferSize(CDisplay * pDisplay);
    eRet                          getFramebufferSettings(bwin_framebuffer_settings * pSettings);
    uint16_t                      getWidth(void)  { return(_graphicsWidth); }
    uint16_t                      getHeight(void) { return(_graphicsHeight); }
    void                          graphicsCheckpoint(void);
    void                          flush(NEXUS_SurfaceHandle surface = NULL);
    void                          sync(bwin_framebuffer_t framebuffer);
    eRet                          updateClientSurface(NEXUS_SurfaceClientHandle hClient, B_EventHandle hRecycleEvent, MRect * pRectUpdate = NULL);

    CDisplay * getDisplay(uint8_t num = 0) { return((0 == num) ? _pDisplayPrimary : _pDisplaySecondary); }
    void       setDisplays(
            CDisplay * pDisplayPrimary,
            CDisplay * pDisplaySecondary
            )
    {
        _pDisplayPrimary = pDisplayPrimary; _pDisplaySecondary = pDisplaySecondary;
    }

protected:
    CModel *                      _pModel;
    CBoardResources *             _pBoardResources;
    CSurfaceClient *              _pSurfaceClientDesktop;
    NEXUS_SurfaceCompositorHandle _surfaceCompositor;
    CDisplay *                    _pDisplayPrimary;
    CDisplay *                    _pDisplaySecondary;
    uint16_t                      _graphicsWidth;
    uint16_t                      _graphicsHeight;
    NEXUS_Graphics2DHandle        _blitter;
    B_EventHandle                 _checkpointEvent;
    B_EventHandle                 _inactiveEvent;
    NEXUS_SurfaceHandle           _surface;
    bool _bActive;

    bwin_engine_t      _bwinEngine;
    bwin_font_t        _font10;
    bwin_font_t        _font12;
    bwin_font_t        _font14;
    bwin_font_t        _font17;
    bwin_framebuffer_t _framebuffer;
    bwin_t             _winFramebuffer;
};

#ifdef __cplusplus
}
#endif

#endif /* GRAPHICS_H__ */