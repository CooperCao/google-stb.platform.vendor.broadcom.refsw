/***************************************************************************
 *     (c)2011-2013 Broadcom Corporation
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
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
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
#ifndef NEXUS_SURFACE_CLIENT_H__
#define NEXUS_SURFACE_CLIENT_H__

#include "bstd.h"
#include "nexus_types.h"
#include "nexus_surface.h"
#include "nexus_surface_compositor_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Acquire a client handle from the server

Description:
Only the server can create clients and control their top-level composition.
If the client wants to request this, it must be done with app-level IPC.
**/
NEXUS_SurfaceClientHandle NEXUS_SurfaceClient_Acquire( /* attr{release=NEXUS_SurfaceClient_Release} */
    NEXUS_SurfaceCompositorClientId client_id
    );

/**
Summary:
Release a client that was acquired
**/
void NEXUS_SurfaceClient_Release(
    NEXUS_SurfaceClientHandle client
    );

/**
Summary:
TODO: Create a child window within an acquired parent window
**/
NEXUS_SurfaceClientHandle NEXUS_SurfaceClient_CreateChild(
    NEXUS_SurfaceClientHandle parent_handle
    );

/**
Summary:
Destroy a child window
**/
void NEXUS_SurfaceClient_DestroyChild(
    NEXUS_SurfaceClientHandle child_handle
    );

/**
Summary:
Acquire a video window as a child of an acquired parent window

Description:
this allows the client to resize and reposition the window along with its GUI changes.
you cannot submit surfaces to a video window NEXUS_SurfaceClientHandle.
you can call NEXUS_SurfaceClient_SetSettings to control composition settings.
**/
NEXUS_SurfaceClientHandle NEXUS_SurfaceClient_AcquireVideoWindow(
    NEXUS_SurfaceClientHandle parent_handle,
    unsigned window_id /* any id, unique to the parent */
    );

/**
Summary:
**/
void NEXUS_SurfaceClient_ReleaseVideoWindow(
    NEXUS_SurfaceClientHandle window_handle
    );

/**
Summary:
Client settings

Description:
There are three different kinds of clients. The following settings apply in this way:
1) top-level clients - all settings exception composition apply
2) child video window - only composition applies
3) child surfaces - not implemented
**/
typedef struct NEXUS_SurfaceClientSettings
{
    NEXUS_SurfaceComposition composition; /* This currently applies only to video child windows, not graphics SurfaceClients.
        Setting NEXUS_SurfaceComposition for graphics clients requires NEXUS_SurfaceCompositor_SetClientSettings,
        which is a server-only call. If you are a client and want to change your top-level composition, see NxClient_SetSurfaceClientComposition. */
    NEXUS_CallbackDesc recycled; /* one or more surfaces have been recycled. They are no longer in use by the server or hardware.
        client should call RecycleSurface to retrieve the surface(s).
        recycling happens after PushSurface if the new surface causes an old surface to become unused.
        recycling also happens after SetSurface/UpdateSurface when the client's surface has been copied to the server's memory. */
    NEXUS_CallbackDesc displayed; /* called when the contents of the last SetSurface/UpdateSurface/PushSurface has made it to the framebuffer.
        you should wait on this callback only if you want your client to be paced by the display's frame rate. 
        it will result in slower updates than just using the recycled callback. */
    NEXUS_CallbackDesc displayStatusChanged; /* NEXUS_SurfaceClientStatus.display has changed */
    NEXUS_CallbackDesc windowMoved; /* video window has been moved and another call can be accepted without blocking */

    NEXUS_VideoOrientation orientation; /* orientation of the client's surface.
        if display is 3D:
            if e3D_LeftRight, then the left half of the surface is used for left eye, right half for right eye.
            if e3D_OverUnder, then the top half of the surface is used for left eye, bottom half for right eye.
            if e2D, then the whole surface is used for both eyes.
        if display is 2D:
            if e3D_LeftRight, then only left half will be used.
            if e3D_OverUnder, then only top half will be used.
            if 2D, then the whole surface is used. */

    unsigned virtualRefreshRate; /* Refresh rate of multibuffered surfaces in units of 1/1000 Hz. For example, 24000
                        causes frames to be consumed from multibuffered FIFO at 24 Hz, independent of the display refresh rate.
                        Value of zero (default) means frames are consumed based on the display refresh rate.
                        Only applicable for PushSurface API. */
    bool allowCompositionBypass; /* see NEXUS_SurfaceCompositorSettings.allowCompositionBypass */
} NEXUS_SurfaceClientSettings;

/**
Summary:
set a new surface (incremental mode)

Example scenario:
1) client creates a surface
2) client renders the whole surface
3) client calls NEXUS_SurfaceClient_SetSurface.
4) client must wait for the recycled or displayed callback before making an update to the surface.
5) client can update a region of the surface and call NEXUS_SurfaceClient_UpdateSurface.
6) client must wait for the recycled callback before making another update to the surface.
**/
NEXUS_Error NEXUS_SurfaceClient_SetSurface(
    NEXUS_SurfaceClientHandle handle,
    NEXUS_SurfaceHandle surface
    );

/**
Summary:
update a surface that was set with NEXUS_SurfaceClient_SetSurface

Description:
Client is responsible for waiting for the recycled or displayed callback before calling UpdateSurface after a previous SetSurface or UpdateSurface.
If it is called prematurely, the function will fail.
**/
NEXUS_Error NEXUS_SurfaceClient_UpdateSurface(
    NEXUS_SurfaceClientHandle handle,
    const NEXUS_Rect *pUpdateRect /* attr{null_allowed=y} NULL for whole surface */
    );

/**
Summary:
push a new surface (multibuffered mode)

Example scenario:
1) client creates surface[0] and [1].
2) client renders surface[0] then calls NEXUS_SurfaceClient_PushSurface. The function is non-blocking and no copy will be made.
   This surface will be composited and displayed by the server.
3) client renders into surface[1] then calls NEXUS_SurfaceClient_PushSurface.
4) pushing surface[1] will cause surface[0] to eventually become unused. when that occurs, the recycled callback will fire.
5) the client should wait for the recycled or displayed callback, then call RecycleSurface to retrieve the surface.
6) once the surface has been retrieved, the client can redraw into it or destroy it. it is no longer used by the surface compositor.
**/
NEXUS_Error NEXUS_SurfaceClient_PushSurface(
    NEXUS_SurfaceClientHandle handle,
    NEXUS_SurfaceHandle surface,
    const NEXUS_Rect *pUpdateRect, /* attr{null_allowed=y} NULL for whole surface */
    bool infront /* in front is used to push surface in front of any other queued surfaces, if there are any other surfaces queued they would get immediately recycled */
    );

/**
Summary:
recycle a surface that was previously pushed with NEXUS_SurfaceClient_PushSurface

Description:
after receiving a recycled callback, you should call NEXUS_SurfaceClient_RecycleSurface to retrieve the surfaces from the surface compositor.
more than one surface may be recycled for a single callback. you should continue to call RecycleSurface until num_returned < num_entries.
**/
NEXUS_Error NEXUS_SurfaceClient_RecycleSurface(
    NEXUS_SurfaceClientHandle handle,
    NEXUS_SurfaceHandle *recycled, /* attr{nelem=num_entries;nelem_out=num_returned} */
    size_t num_entries,
    size_t *num_returned
    );

/**
Summary:
Remove all surfaces from the surface compositor immediately

Description:
This clears surfaces in all modes.
This does not cause surfaces to be recycled. They are immediately available to the client, either to reuse or destroy.

NEXUS_SurfaceClient_Clear does not clear in-flight callbacks. So, a pending recycled callback or a surface that is in the recycle queue because of previous
actions will still have to be recycled.
**/
void NEXUS_SurfaceClient_Clear(
    NEXUS_SurfaceClientHandle handle
    );

/**
Summary:
Get client settings
**/
void NEXUS_SurfaceClient_GetSettings(
    NEXUS_SurfaceClientHandle handle,
    NEXUS_SurfaceClientSettings *pSettings /* [out] */
    );

/**
Summary:
Set client settings

Description:
See NEXUS_SurfaceClientSettings for which settings apply to the client

Note that NEXUS_SurfaceClientSettings.composition only works for video clients. See NEXUS_SurfaceClientSettings for more details.
**/
NEXUS_Error NEXUS_SurfaceClient_SetSettings(
    NEXUS_SurfaceClientHandle handle,
    const NEXUS_SurfaceClientSettings *pSettings
    );

/**
Summary:
Server and display status for the client
**/
typedef struct NEXUS_SurfaceClientStatus
{
    unsigned id;
    bool child; /* true for any client created with NEXUS_SurfaceClient_CreateChild or NEXUS_SurfaceClient_AcquireVideoWindow */
    unsigned parentId; /* only applies if 'child' is true */
    NEXUS_Rect position; /* client position and size in framebuffer coordinates */
    struct {
        bool enabled; /* display is enabled. */
        bool enabled3d; /* if display is in a 3D mode */
        NEXUS_SurfaceRegion framebuffer; /* framebuffer size in pixels. */
        NEXUS_SurfaceRegion screen; /* display size */
        unsigned numFramebuffers;
    } display;    
} NEXUS_SurfaceClientStatus;

/**
Summary:
Get server and display status
**/
NEXUS_Error NEXUS_SurfaceClient_GetStatus(
    NEXUS_SurfaceClientHandle handle,
    NEXUS_SurfaceClientStatus *pStatus /* [out] */
    );

/**
Deprecated
**/
NEXUS_Error NEXUS_SurfaceClient_AcquireTunneledSurface(
    NEXUS_SurfaceClientHandle handle,
    NEXUS_SurfaceHandle *pSurface /* [out] */
    );

/**
Deprecated
**/
void NEXUS_SurfaceClient_ReleaseTunneledSurface(
    NEXUS_SurfaceClientHandle handle,
    NEXUS_SurfaceHandle pSurface
    );

/**
Deprecated
**/
NEXUS_Error NEXUS_SurfaceClient_PushTunneledSurface(
    NEXUS_SurfaceClientHandle handle,
    NEXUS_SurfaceHandle pSurface,
    const NEXUS_Rect *pUpdateRect, /* attr{null_allowed=y} NULL for whole surface */
    bool infront /* in front is used to submit surface in front of any other submitted surfaces, if there are any other surfaces submitted they would get immediately recycled */
    );

/**
Deprecated
*/
NEXUS_Error NEXUS_SurfaceClient_RecycleTunneledSurface(
    NEXUS_SurfaceClientHandle handle,
    NEXUS_SurfaceHandle *recycled, /* attr{nelem=num_entries;nelem_out=num_returned} */
    size_t num_entries,
    size_t *num_returned
    );


/**
Summary:
tell surface_compositor that for whatever surface is current (either last SetSurface or last PushSurface)
the compositor should re-render that surface on every vsync without any further notification from the client.

Description:
the feature cannot be implemented without possible tearing, so it is not recommended.
it is useful for porting libraries that expect direct access to the framebuffer.
To release the published surface, you must either call SetSurface or PushSurface (with another surface, same surface or NULL).
**/
NEXUS_Error NEXUS_SurfaceClient_PublishSurface(
    NEXUS_SurfaceClientHandle handle
    );

#ifdef __cplusplus
}
#endif

#endif
