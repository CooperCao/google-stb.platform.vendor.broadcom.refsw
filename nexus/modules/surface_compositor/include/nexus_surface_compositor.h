/***************************************************************************
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
 **************************************************************************/
#ifndef NEXUS_SURFACE_COMPOSITOR_H__
#define NEXUS_SURFACE_COMPOSITOR_H__

#include "bstd.h"
#include "nexus_types.h"
#include "nexus_surface.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_surface_client.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
This API is semi-private. It is called by nxserver, not client applications, and
so is subject to non-backward compatible API changes.
**/

/**
Summary:
**/
typedef struct NEXUS_SurfaceCompositor *NEXUS_SurfaceCompositorHandle;

/**
Summary:
**/
NEXUS_SurfaceCompositorHandle NEXUS_SurfaceCompositor_Create( /* attr{destructor=NEXUS_SurfaceCompositor_Destroy} */
    unsigned server_id
    );

/**
Summary:
**/
void NEXUS_SurfaceCompositor_Destroy(
    NEXUS_SurfaceCompositorHandle handle
    );

/**
Summary:
**/
NEXUS_SurfaceClientHandle NEXUS_SurfaceCompositor_CreateClient( /* attr{destructor=NEXUS_SurfaceCompositor_DestroyClient} */
    NEXUS_SurfaceCompositorHandle handle,
    NEXUS_SurfaceCompositorClientId client_id
    );

/**
Summary:
**/
void NEXUS_SurfaceCompositor_DestroyClient(
    NEXUS_SurfaceClientHandle client
    );

/* must support regular and mosaic windows */
#define NEXUS_SURFACE_COMPOSITOR_VIDEO_WINDOWS 16

/**
Summary:
per-display settings used in NEXUS_SurfaceCompositorSettings

Description:
When you pass a NEXUS_DisplayHandle, you give up your ability to call certain nexus functions. They are:
- NEXUS_Display_Close
- NEXUS_Display_SetGraphicsSettings
- NEXUS_Display_SetGraphicsFramebuffer
- NEXUS_Display_SetSettings - you cannot change 'format' while SurfaceCompositor is enabled. See NEXUS_SurfaceCompositorSettings.enabled and NEXUS_SurfaceCompositorStatus.active.
- NEXUS_Display_SetVsyncCallback - but only if NEXUS_SurfaceClientSettings.windowMoved is requested

When you pass a NEXUS_VideoWindowHandle, you give up your ability to call certain nexus functions. They are:
- NEXUS_VideoWindow_Close
- NEXUS_VideoWindow_SetSettings

You cannot change a display handle or framebuffer settings while the surface compositor is active.

Typical values for framebuffer.number are:
1: Single buffering results in maximum performance, but there will be tearing on the screen.
2: Double buffering is the default and standard mode.
*/
typedef struct NEXUS_SurfaceCompositorDisplaySettings {
    NEXUS_GraphicsSettings graphicsSettings; /* user can apply new graphics settings while surface compositor is active. However, the following
        settings will be overridden: enabled, clip, position, frameBufferCallback */
    bool manualPosition; /* user has set graphicsSettings.position relative to display format. Used to position graphics within TV viewable area. */
    NEXUS_DisplayHandle display;
    struct {
        unsigned number; /* number of framebuffers. Default is 2. */
        unsigned width, height; /* desired framebuffer dimensions. If not equal to graphicsSettings.position.width/height GFD upscale is required. If zero, defaults will be used. */
        NEXUS_Pixel backgroundColor; /* used to fill background if there is no fullscreen surface */
        NEXUS_PixelFormat pixelFormat; /* pixel format of the framebuffer surfaces */
        NEXUS_HeapHandle heap;  /* set with NEXUS_Platform_GetFramebufferHeap() */
    } framebuffer;
    struct {
       bool overrideOrientation;           /* used to override orientation of 2D display formats */
       NEXUS_VideoOrientation orientation; /* orientation of the 3D display */
    } display3DSettings;
} NEXUS_SurfaceCompositorDisplaySettings;

/**
Summary:
server-side compositor settings
**/
typedef struct NEXUS_SurfaceCompositorSettings
{
    bool enabled; /* set to false and wait for inactiveCallback before changing display settings */
    NEXUS_CallbackDesc inactiveCallback; /* fired when SurfaceCompositor finishes going from enabled -> disabled.
                                            After this has fired, you can change display settings. */
    NEXUS_SurfaceCompositorDisplaySettings display[NEXUS_MAX_DISPLAYS]; /* can only be changed if status.active is false. */
    NEXUS_CallbackDesc frameBufferCallback; /* called after all framebuffers have been updated */
    bool muteVideo[NEXUS_MAX_DISPLAYS];
    struct {
        /* Bounce buffer used to handle large downscale factors, by splitting single HW transaction to two operations.
           Bounce buffer would be used as destination buffer for the first operation and as the source for the second. */
        unsigned width;
        unsigned height;
        NEXUS_HeapHandle heap; /* if NULL would use the master framebuffer heap  */
    } bounceBuffer;
    bool allowCompositionBypass; /* If top-level client in push mode and NEXUS_SurfaceClientSettings.allowCompositionBypass is true,
                                    send surface directly to GFD0, bypassing composition and saving a blit.
                                    Only recommended for single display, 2D mode, no cursor.
                                    Client must allocate framebuffer-sized surface from same MEMC as GFD0. */
} NEXUS_SurfaceCompositorSettings;

/**
Summary:
**/
void NEXUS_SurfaceCompositor_GetSettings(
    NEXUS_SurfaceCompositorHandle handle,
    NEXUS_SurfaceCompositorSettings *pSettings /* [out] */
    );

/**
Summary:
set surface compositor settings
**/
NEXUS_Error NEXUS_SurfaceCompositor_SetSettings(
    NEXUS_SurfaceCompositorHandle handle,
    const NEXUS_SurfaceCompositorSettings *pSettings
    );
    
typedef struct NEXUS_SurfaceCompositorClientDisplaySettings {
    struct {
        unsigned id; /* deprecated. same as window[] index. */
        NEXUS_VideoWindowHandle window; /* see above for nexus API's consumed by this interface */
    } window[NEXUS_SURFACE_COMPOSITOR_VIDEO_WINDOWS]; /* this index corresponds to window_id of NEXUS_SurfaceClient_AcquireVideoWindow */
} NEXUS_SurfaceCompositorClientDisplaySettings;

typedef struct NEXUS_SurfaceCompositorClientSettings
{
    NEXUS_SurfaceComposition composition;
    bool tunnelCapable; /* deprecated */
    NEXUS_SurfaceCompositorClientDisplaySettings display[NEXUS_MAX_DISPLAYS];
} NEXUS_SurfaceCompositorClientSettings;

void NEXUS_SurfaceCompositor_GetClientSettings(
    NEXUS_SurfaceCompositorHandle handle,
    NEXUS_SurfaceClientHandle client,
    NEXUS_SurfaceCompositorClientSettings *pSettings /* [out] */
    );

/**
set parent surface settings
the client can set its child surface and video window settings, but it cannot set parent surface settings.
**/
NEXUS_Error NEXUS_SurfaceCompositor_SetClientSettings(
    NEXUS_SurfaceCompositorHandle handle,
    NEXUS_SurfaceClientHandle client,
    const NEXUS_SurfaceCompositorClientSettings *pSettings
    );
    
NEXUS_Error NEXUS_SurfaceCompositor_SwapWindows(
    NEXUS_SurfaceCompositorHandle handle,
    NEXUS_SurfaceClientHandle surfaceClient0, /* top-level graphics SurfaceClient */
    unsigned windowId0,        /* video window SurfaceClient acquire ID */
    NEXUS_SurfaceClientHandle surfaceClient1, /* top-level graphics SurfaceClient */
    unsigned windowId1         /* video window SurfaceClient acquire ID */
    );

typedef struct NEXUS_SurfaceCompositorStatus
{
    bool active; /* this goes to false only after enabled is set to false and the inactiveCallback has fired. 
                    if enabled is true or if the inactiveCallback has not fired, active is true and display
                    settings cannot be changed. */
    unsigned numAcquiredTunneledSurfaces;
} NEXUS_SurfaceCompositorStatus;

NEXUS_Error NEXUS_SurfaceCompositor_GetStatus(
    NEXUS_SurfaceCompositorHandle handle,
    NEXUS_SurfaceCompositorStatus *pStatus /* [out] */
    );
    
/* Used for graphics screenshots. Must be copied immediately to avoid tearing.
NEXUS_SurfaceHandle may become invalid after NEXUS_SurfaceCompositor_SetSettings is called. */
NEXUS_Error NEXUS_SurfaceCompositor_GetCurrentFramebuffer(
    NEXUS_SurfaceCompositorHandle handle,
    NEXUS_SurfaceHandle *pSurface
    );

#ifdef __cplusplus
}
#endif

#endif

