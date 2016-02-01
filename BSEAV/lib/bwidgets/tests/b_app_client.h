/******************************************************************************
 * (c) 2004-2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
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
 *
 *****************************************************************************/

#ifndef B_APP_CLIENT_H__
#define B_APP_CLIENT_H__

#include "bwin.h"
#include "nexus_types.h"
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "nexus_display.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************
Offscreen UI control
*****************************/

struct b_offscreen_status
{
    NEXUS_Rect update_rect;
    NEXUS_SurfaceHandle surface; /* drawing surface */
    NEXUS_Graphics2DHandle blitter;
    BKNI_EventHandle checkpoint_event;

    NEXUS_SurfaceHandle back_surface[2]; /* double buffered back buffer*/
    unsigned current_back_surface; /* index to back_surface[] */

    bwin_engine_t win_engine;
    bwin_font_t default_font;
    bwin_framebuffer_t win_framebuffer;
    bwin_t win_framebuffer_window;

    void (*sync)(void *context);
    void *context;

    /* linked if the server */
    struct b_server_status *server;
};

int b_init_offscreen(unsigned width, unsigned height, unsigned backwidth, unsigned backheight, struct b_offscreen_status *status);
void b_uninit_offscreen(const struct b_offscreen_status *status);
void checkpoint(const struct b_offscreen_status *status);

/****************************
Client/server messaging
*****************************/

/**
General message api
**/
typedef struct b_message {
    unsigned type;
    unsigned datasize;
#define B_MAX_MSG 1000
    uint8_t data[B_MAX_MSG];
} b_message;

void b_message_init(b_message *msg);
int b_send_message(int fd, const b_message *msg);
int b_recv_message(int fd, b_message *msg);

/**
Specific messages
**/
enum b_message_type {
    /* from client to server */
    b_message_type_report_offscreen, /* payload: b_message_report_offscreen_data, 1 or 2 surfaces */
    b_message_type_offscreen_ready,  /* payload: b_message_index, index of the surface that is ready */

    /* from server to client */
    b_message_type_offscreen_done,   /* the surface given at b_message_offscreen_ready has been blitted
                                        payload: b_message_index, index of the surface which completed */
    b_message_type_activate,         /* client is visible on the server. */
    b_message_type_deactivate,       /* client is not visible on the server. */
    b_message_type_user_input,       /* payload: b_message_user_input_data */
    b_message_type_focus,            /* gain focus */
    b_message_type_blur,             /* lose focus, but still visible */

    b_message_max
};

struct b_message_report_offscreen_data {
    unsigned offset[2]; /* offset[1]!=0 means double buffered. all other params must be the same for both surfaces */
    unsigned width, height, pitch;
    NEXUS_PixelFormat pixelFormat;
};

/* multi-purpose message */
struct b_message_index {
    unsigned index;
};

void b_get_surface_data(const struct b_offscreen_status *status, b_message *msg);
NEXUS_Error b_create_surfaces(const struct b_message_report_offscreen_data *data, NEXUS_SurfaceHandle *pSurface0, NEXUS_SurfaceHandle *pSurface1);

struct b_message_user_input_data {
    unsigned code;
};

/* returns fd */
int b_connect_to_server(void);
void b_disconnect_from_server(int fd);

#ifdef __cplusplus
}
#endif

#endif
