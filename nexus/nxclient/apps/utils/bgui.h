/******************************************************************************
 *    (c)2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 *****************************************************************************/
#ifndef BGUI_H__
#define BGUI_H__

#include "nexus_graphics2d.h"
#if NXCLIENT_SUPPORT
#include "nexus_surface_client.h"
#else
#include "nexus_display.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
NOTE: This API is only example code. It is subject to change and
is not supported as a standard reference software deliverable.
**/

/* boiler plate GUI code */

typedef struct bgui *bgui_t;

struct bgui_settings
{
    unsigned width, height;
#if !NXCLIENT_SUPPORT
    NEXUS_DisplayHandle display;
#endif
};

void bgui_get_default_settings(struct bgui_settings *psettings);
bgui_t bgui_create(const struct bgui_settings *psettings);
void bgui_destroy(bgui_t gui);

/* if not NXCLIENT_SUPPORT, bgui_submit will do a flip. bgui_surface() will retrieve the current offscreen surface. */
NEXUS_SurfaceHandle bgui_surface(bgui_t gui);
NEXUS_Graphics2DHandle bgui_blitter(bgui_t gui);

#if NXCLIENT_SUPPORT
NEXUS_SurfaceClientHandle bgui_surface_client(bgui_t gui);
unsigned bgui_surface_client_id(bgui_t gui);
#endif

/* common actions */
int bgui_fill(bgui_t gui, unsigned color);
int bgui_checkpoint(bgui_t gui);
int bgui_submit(bgui_t gui);

#if NXCLIENT_SUPPORT
/* picture in graphics test API */
struct b_pig_inc {
    int x, y, width;
};
void b_pig_init(NEXUS_SurfaceClientHandle video_sc);
void b_pig_move(NEXUS_SurfaceClientHandle video_sc, struct b_pig_inc *pig_inc);
int bgui_wait_for_move(bgui_t gui);
#endif

#ifdef __cplusplus
}
#endif

#endif
