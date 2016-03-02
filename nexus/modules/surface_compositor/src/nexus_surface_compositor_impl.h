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
   3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
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
#ifndef NEXUS_SURFACECMP_IMPL_H__
#define NEXUS_SURFACECMP_IMPL_H__

#include "nexus_surface_compositor_module.h"
#include "nexus_graphics2d.h"
#include "nexus_core_utils.h"
#include "blst_slist.h"
#include "blst_queue.h"
#include "blst_squeue.h"
#include "blst_list.h"
#include "priv/nexus_core.h"

#ifdef __cplusplus
#error
#endif

/* uncoment line bellow to enable runt-time verificatio that surface didn't
 * change while it was submitted  to the surface compositioon */
/* #define NEXUS_SURFACE_COMPOSITOR_P_CHECK_IMMUTABLE 1 */

#define NEXUS_SURFACECMP_MAX_FRAMEBUFFERS 8

#define NEXUS_P_SURFACECMP_MAX_PUSH_SURFACES   64
#define NEXUS_P_SURFACECMP_HYSTERESIS_THRESHOLD 10  /* percentage of the vsync interval */



BTRC_MODULE_DECLARE(surface_compositor_render);

typedef struct NEXUS_P_SurfaceCompositorRenderElement {
    NEXUS_SurfaceHandle sourceSurface;
    NEXUS_BlendEquation colorBlend;  /* Specify how to compose the color component */
    NEXUS_BlendEquation alphaBlend;   /* Specify how to compose the alpha component */
    struct {
        NEXUS_Graphics2DColorKey source, dest;
    } colorKey;
    NEXUS_Graphics2DFilterCoeffs horizontalFilter;
    NEXUS_Graphics2DFilterCoeffs verticalFilter;
    NEXUS_Pixel constantColor;       /*  Constant alpha and color. */
    bool alphaPremultiplySourceEnabled;
    bool colorMatrixEnabled;
    bool useBounceBuffer;
    NEXUS_Graphics2DColorMatrix colorMatrix;
    NEXUS_Rect outputRect;
    NEXUS_Rect sourceRect;
} NEXUS_P_SurfaceCompositorRenderElement;

typedef struct NEXUS_P_SurfaceCompositorRenderElements {
    unsigned count;
    NEXUS_P_SurfaceCompositorRenderElement *data;
    unsigned size;
    unsigned prev_size;
} NEXUS_P_SurfaceCompositorRenderElements;

void NEXUS_P_SurfaceCompositorRenderElements_Shutdown(NEXUS_P_SurfaceCompositorRenderElements *renderElements);
NEXUS_P_SurfaceCompositorRenderElement *NEXUS_P_SurfaceCompositorRenderElements_Next(NEXUS_P_SurfaceCompositorRenderElements *renderElements);

typedef struct NEXUS_P_SurfaceCompositoaRenderScene {
    NEXUS_Rect dirty; /* dirty rectangle not counted in previous elements */
    NEXUS_P_SurfaceCompositorRenderElements elements;
} NEXUS_P_SurfaceCompositorFramebufferRenderScene; 

typedef struct NEXUS_P_SurfaceCompositorRenderState {
    enum {taskInit, taskFill, taskBlit, taskCompleted} step;
    unsigned current;
    struct {
        bool active; /* set if composited into the tunnel surface */
        NEXUS_Rect rect;  /* valid if active != NULL */
        struct NEXUS_SurfaceCompositorFramebuffer *tunnelSource; /* set if composited using tunnel surface */
    } tunnel;
    NEXUS_P_SurfaceCompositorRenderElements elements;
    NEXUS_Graphics2DBlitSettings blitSettings; /* avoid stack blowout */
    NEXUS_Graphics2DFillSettings fillSettings; /* avoid stack blowout */
} NEXUS_P_SurfaceCompositorRenderState;

typedef struct NEXUS_Surface_P_ClientSurface {
    NEXUS_SurfaceHandle surface; 
#if NEXUS_SURFACE_COMPOSITOR_P_CHECK_IMMUTABLE 
    uint16_t crc;
    uint8_t bpp;
    bool verify;
#endif
} NEXUS_Surface_P_ClientSurface;

#if NEXUS_SURFACE_COMPOSITOR_P_CHECK_IMMUTABLE
BTRC_MODULE_DECLARE(surface_compositor_crc);
void nexus_surfaceclient_p_verify_surface(NEXUS_SurfaceClientHandle client, const NEXUS_Surface_P_ClientSurface *clientSurface, const char *filename, unsigned lineno);
#define NEXUS_SURFACECLIENT_P_SURFACE_VERIFY(client, clientSurface) nexus_surfaceclient_p_verify_surface(client, clientSurface, BSTD_FILE,BSTD_LINE)
#define NEXUS_SURFACECLIENT_P_SURFACE_INIT(s) do {(s)->surface = NULL;(s)->crc=0;(s)->bpp=0;(s)->verify=false;}while(0)
#else
#define NEXUS_SURFACECLIENT_P_SURFACE_INIT(s) do {(s)->surface = NULL;}while(0)
#define NEXUS_SURFACECLIENT_P_SURFACE_VERIFY(client, clientSurface)
#endif

typedef enum NEXUS_SurfaceCompositorramebufferState {
    NEXUS_SurfaceCompositorFramebufferState_eAvailable, /* frame buffer unused, it could be either in avaliable (if tunnel.acquired==false) or tunnel.avaliable lists */
    NEXUS_SurfaceCompositorFramebufferState_eCompositing, /* frame buffer currently being composed into */
    NEXUS_SurfaceCompositorFramebufferState_eComposited, /* frame buffer composed but not submitted */
    NEXUS_SurfaceCompositorFramebufferState_eSubmitted, /* frame buffer submitted to display */
    NEXUS_SurfaceCompositorFramebufferState_eDisplaying, /* frame buffer currently displayed */
    NEXUS_SurfaceCompositorFramebufferState_eTunnelSubmitted /* frame buffer was submitted and is in the tunnel.submitted list */
} NEXUS_SurfaceCompositorFramebufferState ;

struct NEXUS_SurfaceCompositorFramebuffer {
    BLST_SQ_ENTRY(NEXUS_SurfaceCompositorFramebuffer) link; /* link used for compositing/display lifecycle */
    BLST_Q_ENTRY(NEXUS_SurfaceCompositorFramebuffer) tunnelLink; /* link used for tunnel management */
    NEXUS_SurfaceCompositorFramebufferState state;
    NEXUS_P_SurfaceCompositorFramebufferRenderScene scene;
    NEXUS_SurfaceHandle surface;
    struct { /* if formatInfo.orientation is 3D then this surfaces would point to left and right views, otherwise they are NULL */
        NEXUS_SurfaceHandle left;
        NEXUS_SurfaceHandle right;
    } view3D;
    struct {
        NEXUS_Surface_P_ClientSurface surface; /* surface used by the client */
        NEXUS_Rect rect; /* rectangle in framebuffer coordinates */
        bool acquired; 
        bool pristine; /* if this framebuffer - even if not 'acquired' contains exact copy of last rendered tunnel surface */
        bool composited; /* true if this framebuffer was recyled back to the application */
    } tunnel;
    unsigned generation; /* copy of display.generation done when rendering is initiated */
    uint16_t ref_cnt; /* reference counter used when framebuffer on 'master' display is used to render into the 'slave' display */
    bool full_render; /* must rerender entire framebuffer */
};

BLST_SQ_HEAD(NEXUS_SurfaceCompositorFramebufferQueue, NEXUS_SurfaceCompositorFramebuffer);

/* internal per-display storage */
struct NEXUS_SurfaceCompositorDisplay {
    struct NEXUS_SurfaceCompositor *server;
    
    NEXUS_DisplayHandle display;
    unsigned index;
    unsigned generation; /*  running counter for the scenes, used to track data rendered into the framebuffers */
    NEXUS_SurfaceRegion canvas; /* in-use framebuffer coordinates. maybe less than framebuffer size because of clipping and/or 3D modes. And different from formatInfo.canvas because of graphics scaling */
    struct { /* for 3D we are using split framebuffers where left eye view at coordinates 0,0, and below offsets are used to shift to right view, typically only x or y are not zero */
        uint16_t x;
        uint16_t y;
    } offset3DRight;
    struct {
        NEXUS_SurfaceRegion canvas; /* display canvas size, may be different from entire display size for 3D modes */
        NEXUS_VideoOrientation orientation;
        bool native3D; /* use native3D API for graphics */
        unsigned verticalFreq;
        long vsyncIntervalUsecHystLow;
        long vsyncIntervalUsecHystHigh;
    } formatInfo;
    uint32_t backgroundColor;
    
    NEXUS_CallbackHandler frameBufferCallback, vsyncCallback;
    /* 
     * Non overlapped tunnel rendering of newly submitted framebuffer goes through tunnel.submitted->compositing->composited->submitted->displaying->tunnel.available
     * Everything else goes through available->compositing->composited->submitted->displaying->available 
     *
     */

    struct NEXUS_SurfaceCompositorFramebufferQueue available;
    struct NEXUS_SurfaceCompositorFramebuffer *compositing;
    struct NEXUS_SurfaceCompositorFramebuffer *composited;
    struct NEXUS_SurfaceCompositorFramebuffer *submitted;
    struct NEXUS_SurfaceCompositorFramebuffer *displaying;
    struct {
        BLST_Q_HEAD(NEXUS_SurfaceCompositorFramebufferTunnelList, NEXUS_SurfaceCompositorFramebuffer) submitted, available;
    } tunnel;

    unsigned num_framebuffers;
    struct NEXUS_SurfaceCompositorFramebuffer *master_framebuffer;
    struct NEXUS_SurfaceCompositorFramebuffer framebuffer[NEXUS_SURFACECMP_MAX_FRAMEBUFFERS];
    struct {
        bool dirty;
    } slave;
    struct {
        unsigned compose, fill;
    } stats;
};



typedef struct NEXUS_SurfaceCompositor_P_PushElement
{
    BLST_SQ_ENTRY(NEXUS_SurfaceCompositor_P_PushElement) link;
    NEXUS_Surface_P_ClientSurface surface;
} NEXUS_SurfaceCompositor_P_PushElement;

BLST_SQ_HEAD(NEXUS_SurfaceCompositor_P_PushQueue, NEXUS_SurfaceCompositor_P_PushElement);
        
/* state per frame buffer */
typedef struct NEXUS_Surface_P_CursorFramebuffer {
    NEXUS_SurfaceHandle buffer;  /* saved pristince copy from NULL means it was not allocated, and cursor wouldn't get drawn */
    NEXUS_Rect left;
    NEXUS_Rect right;
    const struct NEXUS_SurfaceCompositorFramebuffer *framebuffer;  /* if not NULL, then rect has a coordinates of framebuffer rectange copied into the buffer, it copied to buffer from 0,0 and with matching height and width */
} NEXUS_Surface_P_CursorFramebuffer;

/* per display state */
typedef struct NEXUS_Surface_P_CursorDisplay {
    NEXUS_Surface_P_CursorFramebuffer framebuffers[NEXUS_SURFACECMP_MAX_FRAMEBUFFERS];
    struct { /* for 3D we are using split buffers where left eye view at coordinates 0,0, and below offsets are used to shift to right view, typically only x or y are not zero */
        uint16_t x;
        uint16_t y;
    } offset3DRight;
} NEXUS_Surface_P_CursorDisplay;

struct NEXUS_SurfaceCursor {
    NEXUS_OBJECT(NEXUS_SurfaceCursor);
    BLST_D_ENTRY(NEXUS_SurfaceCursor) link;
    NEXUS_SurfaceCompositorHandle compositor;
    NEXUS_Surface_P_CursorDisplay display[NEXUS_MAX_DISPLAYS]; 
    NEXUS_SurfaceCursorCreateSettings createSettings;
    NEXUS_SurfaceCursorSettings settings;
    NEXUS_Rect size;
    NEXUS_SurfaceRegion virtualDisplay; /* copy of serverSettings.composition.virtualDisplay updated with current display information */
};


BLST_Q_HEAD(NEXUS_P_SurfaceClientList, NEXUS_SurfaceClient);

/* users of the single blitter engine */
#define NEXUS_P_SURFACEBLITTER_COMPOSITOR (1<<0)
/* used to render cursors on top of framebuffers */
#define NEXUS_P_SURFACEBLITTER_CURSOR     (1<<1)
/* used to update client side state */
#define NEXUS_P_SURFACEBLITTER_CLIENT     (1<<2)

struct NEXUS_SurfaceCompositor
{
    NEXUS_OBJECT(NEXUS_SurfaceCompositor);
    BLST_S_ENTRY(NEXUS_SurfaceCompositor) link; /* for g_nexus_surfacecompositor_list */
    struct NEXUS_P_SurfaceClientList clients; /* master list of clients */
    BLST_D_HEAD(NEXUS_Surface_P_CursurList, NEXUS_SurfaceCursor) cursors; 
    unsigned server_id;
    bool pending_update; /* an update of framebuffers is pending */
    struct {
        bool set; /* one client needs nexus_surfaceclient_p_setvideo called */
        unsigned windowMoved; /* # of vsyncs requires before at least one client needs windowMoved callback fired */
    } set_video_pending;
    NEXUS_TaskCallbackHandle frameBufferCallback, inactiveCallback;
    NEXUS_TimerHandle inactiveTimer; /* provide timer for displayed callbacks which inactive */
    NEXUS_Graphics2DHandle gfx;
    NEXUS_CallbackHandler packetSpaceAvailableCallback, checkpointCallback;

    struct {
        NEXUS_SurfaceClientHandle client; /* tunneled client that has allocated framebuffer surfaces, it may or may not be visible/rendered */
        bool overlapped; /* true, if additional clients should be rendered into the tunnel framebuffer */
    } tunnel;
    struct {
        NEXUS_SurfaceClientHandle client;
        NEXUS_SurfaceHandle set, completing;
    } bypass_compose;

    struct {
        unsigned update_flags;
        bool active; /* true if enabled or if inactive callback has not fired */
        struct {
            unsigned active; /* operation currently in progress, bits of NEXUS_P_SURFACEBLITTER_XXX, only one bit could be set */
            unsigned delayed; /* operations that were delayed */
        } blitter;
        bool standbyDisabled;   /* Disabled during standby, needs to be re-enabled after resume */
    } state;

    struct {
        NEXUS_Graphics2DCapabilities capabilities;
        NEXUS_SurfaceHandle buffer; /* NULL if not enabled */
    } bounceBuffer;

    struct { /* buffer used to support compressed framebuffer */
        NEXUS_SurfaceHandle buffer; /* NULL if not enabled */
    } bounceBufferMasterFramebuffer;

    NEXUS_P_SurfaceCompositorRenderState renderState;
    NEXUS_SurfaceCompositorSettings settings;
    struct NEXUS_SurfaceCompositorDisplay *display[NEXUS_MAX_DISPLAYS];
    NEXUS_VideoWindowSettings windowSettings, prevWindowSettings; /* avoid stack blowout */
};


typedef struct NEXUS_SurfaceClient_TunnelSurface {
    bool acquired;
    struct NEXUS_SurfaceCompositorFramebuffer *framebuffer; /* frame buffer or NULL if allocated not from a framebuffer */
    NEXUS_SurfaceHandle surface;
}  NEXUS_SurfaceClient_TunnelSurface;

typedef enum NEXUS_SurfaceClientType {
    NEXUS_SurfaceClient_eTopLevel,
    NEXUS_SurfaceClient_eChild,
    NEXUS_SurfaceClient_eVideoWindow,
    NEXUS_SurfaceClient_eMax
} NEXUS_SurfaceClientType;

/* changes in value of zorder */
#define NEXUS_P_SURFACECLIENT_UPDATE_ZORDER    (1<<0)
/* width & height of destination rectangle */
#define NEXUS_P_SURFACECLIENT_UPDATE_SIZE      (1<<1)
#define NEXUS_P_SURFACECLIENT_UPDATE_POSITION  (1<<2)
#define NEXUS_P_SURFACECLIENT_UPDATE_VISIBLE   (1<<3)
#define NEXUS_P_SURFACECLIENT_UPDATE_SOURCE   (1<<4)
#define NEXUS_P_SURFACECLIENT_UPDATE_PUBLISH    (1<<5)
/* changes in the other composition - blend equation */
#define NEXUS_P_SURFACECLIENT_UPDATE_COMPOSITION  (1<<5)
#define NEXUS_P_SURFACECLIENT_UPDATE_DISPLAY (1<<6)
/* new client was created */
#define NEXUS_P_SURFACECLIENT_UPDATE_CLIENT (1<<7)
/* new client was added or removed */
#define NEXUS_P_SURFACECLIENT_UPDATE_ALL    (0xFFFF)

typedef struct NEXUS_Surface_P_Composition {
    bool visible;  /* true if any portion of client view on the screen */
    bool hidden; /* true if entire visible portion of client is hidden by another client or client is invisible */
    NEXUS_Rect outputRect; /* entire visible rectangle in the framebuffer pixels */
    NEXUS_Rect sourceRect; /* entire visible rectangle in the source pixels */
} NEXUS_Surface_P_Composition;

struct NEXUS_SurfaceClient
{
    NEXUS_OBJECT(NEXUS_SurfaceClient);
    BLST_Q_ENTRY(NEXUS_SurfaceClient) link; /* for server->clients */
    BLST_S_ENTRY(NEXUS_SurfaceClient) child_link; /* for parent->children */
    BLST_S_HEAD(nexus_client_children_t, NEXUS_SurfaceClient) children; /* for eChild or eVideoWindow clients */
    NEXUS_SurfaceCompositorClientId client_id;
    NEXUS_SurfaceCompositorHandle server;
    bool acquired;
    bool calcblit_required; /* set to true if calcblit should be called when composing */
    NEXUS_SurfaceClientSettings settings;
    NEXUS_SurfaceCompositorClientSettings serverSettings;
    NEXUS_TaskCallbackHandle displayedCallback, recycledCallback, displayStatusChangedCallback, windowMovedCallback, vsyncCallback;
    bool pending_displayed_callback; /* set when client adds new source */
    bool process_pending_displayed_callback; /* set when new source used for compositing */
    NEXUS_SurfaceClientType type;
    NEXUS_SurfaceClientHandle parent; /* if eChild or eVideoWindow */
    
    struct {
        bool set; /* this video window client needs nexus_surfaceclient_p_setvideo called */
        bool windowMoved; /* this top-level client needs windowMoved callback fired */
    } set_video_pending;
    NEXUS_VideoWindowHandle window[NEXUS_MAX_DISPLAYS];
    
    struct {
        NEXUS_SurfaceHandle surface; /* copy of set.surface, BLST_SQ_FIRST(&push_entries) or tunnel.get_surface */
    } published; 
    
    /* incremental mode storage (set/update) */
    struct {
        NEXUS_Surface_P_ClientSurface surface; /* last surface passed to SetSurface */
        NEXUS_SurfaceHandle serverSurface; /* server's copy of last SetSurface */
        NEXUS_SurfaceCreateSettings serverCreateSettings;
        NEXUS_Rect dirtyRect;
        bool dirty; /* server's copy should be updated */
        bool updating; /* server's copy currently being updated */
        unsigned cnt;
    } set;
    
    /* multibuffering mode storage (push/recycle) */
    struct {
        struct NEXUS_SurfaceCompositor_P_PushQueue push;
        struct NEXUS_SurfaceCompositor_P_PushQueue recycle;
        struct NEXUS_SurfaceCompositor_P_PushQueue free;
        bool compositing; /* signals that first element from the push queue currently composited  */
        bool last; /* signals that first element from the push queue can't be recycled, because it's the only element left */
        bool firstCompositingValid; /* set when firstCompositing have proper timestamp, resets when client queue gets empty */
        unsigned firstCompositing; /* time when current surface was first composited, used to implement NEXUS_SurfaceClientSettings.virtualRefreshRate */
        long frameFractionalTime;
        long frameFractionalTimeHysteresis;
#if BDBG_DEBUG_BUILD
        unsigned repeatCount;
#endif
        NEXUS_SurfaceCompositor_P_PushElement elements[NEXUS_P_SURFACECMP_MAX_PUSH_SURFACES];
        unsigned cnt;
    } queue;

    /* tunnel mode storage */    
    struct {
        bool visible; /* set to true when either client_type_tunnel_emulated orient_type_tunnel client pushed a first surface, set to false when last pushed surface is released */
        NEXUS_SurfaceClient_TunnelSurface surfaces[NEXUS_SURFACECMP_MAX_FRAMEBUFFERS];
    } tunnel;
    
    struct {
        NEXUS_Rect updateRect;
    } update_info;

    struct {
        enum {
            client_type_idle,
            client_type_set,
            client_type_push,
            client_type_tunnel,
            client_type_tunnel_emulated
        } client_type;
        unsigned update_flags;
        int rightViewOffset; /* right view offset in framebuffer pixels */
        NEXUS_SurfaceRegion virtualDisplay; /* copy of serverSettings.composition.virtualDisplay updated with current display information */
        NEXUS_SurfaceRegion clientRegion; /* width/height = dimensions of last surface submitted  */
        NEXUS_Rect framebufferRect; /* entire (unclipped) client rectangle in framebuffer coordinates */
        NEXUS_Surface_P_ClientSurface current; /* current surface that should be composited */
        NEXUS_Surface_P_Composition left; /* left eye view or 2D view, managed and used all the time */
        NEXUS_Surface_P_Composition right; /* managed and used only if server.formatInfo.orientation != 3D */
        struct {
            NEXUS_SurfaceHandle surface; /* created if NEXUS_SurfaceComposition.displayCache is true */
            bool needs_update; /* need a blit from serverSurface to cache.surface */
            uint16_t width;
            uint16_t height;
            NEXUS_PixelFormat format;
        } cache;
    } state;
};

extern BLST_S_HEAD(nexus_surfacecompositor_list, NEXUS_SurfaceCompositor) g_nexus_surfacecompositor_list;

void nexus_surface_compositor_p_compose(NEXUS_SurfaceCompositorHandle server);

/* display cache */
NEXUS_Error nexus_surface_compositor_p_verify_tunnel(NEXUS_SurfaceCompositorHandle server);

void nexus_surfacemp_p_convert_coord(const NEXUS_SurfaceRegion *pInputCoord, const NEXUS_SurfaceRegion *pOutputCoord, const NEXUS_Rect *pSrcRect, NEXUS_Rect *pDstRect);
void nexus_surfacemp_p_clip_rect(const NEXUS_Rect *pBound, const NEXUS_Rect *pSrcRect, NEXUS_Rect *pDstRect);
void nexus_surfacemp_scale_clipped_rect(const NEXUS_Rect *pSrcRect, const NEXUS_Rect *pSrcClipRect, const NEXUS_Rect *pDstRect, NEXUS_Rect *pDstClipRect);
    
void nexus_surface_compositor_p_packetspaceavailable(void *context);
void nexus_surface_compositor_p_checkpoint(void *context);
void nexus_surface_compositor_p_framebuffer(void *context);
void nexus_surface_compositor_p_vsync(void *context);
void nexus_surface_compositor_p_render_framebuffer(NEXUS_SurfaceCompositorHandle server, struct NEXUS_SurfaceCompositorDisplay *cmpDisplay, struct NEXUS_SurfaceCompositorFramebuffer *framebuffer);
NEXUS_Error nexus_surface_compositor_p_realloc_framebuffers(NEXUS_SurfaceCompositorHandle server);

void nexus_p_surface_compositor_update_client(NEXUS_SurfaceClientHandle client, unsigned flags);
void nexus_p_surface_compositor_check_inactive(NEXUS_SurfaceCompositorHandle server);

void nexus_surface_compositor_p_release_surfaces(NEXUS_SurfaceCompositorHandle server);

void nexus_p_surface_composition_init(NEXUS_SurfaceComposition *composition);
void nexus_surfacecmp_p_insert_client(NEXUS_SurfaceCompositorHandle server, NEXUS_SurfaceClientHandle client, bool remove);
void nexus_surfacecmp_p_insert_child(NEXUS_SurfaceClientHandle parent, NEXUS_SurfaceClientHandle client, bool remove);


/* clear all cursors from a framebuffer */
void NEXUS_SurfaceCursor_P_ClearCursors(NEXUS_SurfaceCompositorHandle compositor, const struct NEXUS_SurfaceCompositorDisplay *compositorDisplay, const struct NEXUS_SurfaceCompositorFramebuffer *framebuffer);
/* render all cursors on a framebuffer */
void NEXUS_SurfaceCursor_P_RenderCursors(NEXUS_SurfaceCompositorHandle compositor, const struct NEXUS_SurfaceCompositorDisplay *compositorDisplay, const struct NEXUS_SurfaceCompositorFramebuffer *framebuffer);
/* release link between cursor and framebuffer, used when entire framebuffer wiped away */
void NEXUS_SurfaceCursor_P_ReleaseCursors(NEXUS_SurfaceCompositorHandle compositor, const struct NEXUS_SurfaceCompositorDisplay *compositorDisplay, const struct NEXUS_SurfaceCompositorFramebuffer *framebuffer);
/* update cursor in already composed framebuffer, it finds best suitable framebuffer and re-draws all cursors */
void NEXUS_SurfaceCursor_P_UpdateCursor(NEXUS_SurfaceCompositorHandle compositor);

/* clears all references to framebuffers */
void NEXUS_SurfaceCursor_P_Clear(NEXUS_SurfaceCompositorHandle compositor);

/* verifies and updates cursor state for new display  configuration */
NEXUS_Error NEXUS_SurfaceCursor_P_UpdateDisplay(NEXUS_SurfaceCompositorHandle compositor);


/* from 'soft' virtualDispaly in composition produces 'hard' virtualDisplay, that resolves '0' in the 'soft' virtualDisplay using current state of display */
void nexus_p_surface_compositor_update_virtual_display(const struct NEXUS_SurfaceCompositorDisplay *display, const NEXUS_SurfaceComposition *composition, NEXUS_SurfaceRegion *virtualDisplay);


/* function that serialize access to blitter (NEXUS_Graphics2DHandle) */
bool nexus_surface_compositor_p_blitter_acquire(NEXUS_SurfaceCompositorHandle compositor, unsigned client);
void nexus_surface_compositor_p_blitter_release(NEXUS_SurfaceCompositorHandle compositor, unsigned client);
/* start blitter, should be used instead of NEXUS_Graphics2D_Checkpoint */
void nexus_surface_compositor_p_blitter_start(NEXUS_SurfaceCompositorHandle compositor, unsigned client);

void nexus_surface_compositor_p_recycle_displayed_framebuffer(struct NEXUS_SurfaceCompositorDisplay *display);

/* nexus_surface_client_video functions */
void nexus_surfaceclient_request_setvideo( NEXUS_SurfaceClientHandle client );
void nexus_surfaceclient_p_setwindows(NEXUS_SurfaceClientHandle client);
void nexus_surface_compositor_p_update_video(NEXUS_SurfaceCompositorHandle server);

/* manages transfer of client surface to the server surface */
void nexus_surface_compositor_p_update_dirty_clients(NEXUS_SurfaceCompositorHandle server, NEXUS_SurfaceClientHandle client);
void nexus_surface_compositor_p_update_dirty_clients_done(NEXUS_SurfaceCompositorHandle server);

/* return true if client is opaque, e.g. it would override (not blend) anything that drawn behind it */
bool nexus_p_surface_composition_opaque(const NEXUS_BlendEquation *colorBlend, const NEXUS_BlendEquation *alphaBlend);

void NEXUS_SurfaceClient_P_VideoWindowFinalizer( NEXUS_SurfaceClientHandle client );
unsigned nexus_surface_compositor_p_get_time(NEXUS_SurfaceCompositorHandle server);
unsigned nexus_surface_compositor_p_get_time_diff(NEXUS_SurfaceCompositorHandle server, unsigned future, unsigned past);
extern const NEXUS_BlendEquation NEXUS_SurfaceCompositor_P_ColorCopySource;
extern const NEXUS_BlendEquation NEXUS_SurfaceCompositor_P_AlphaCopySource;

#endif /* NEXUS_SURFACECMP_IMPL_H__ */
