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
#if NEXUS_HAS_TRANSPORT && NEXUS_HAS_RECORD
#include "brecord_gui.h"
#include "bfont.h"
#include "nexus_surface_client.h"
#include "nexus_graphics2d.h"
#include "nxclient.h"
#include "nxclient_config.h"
#include "bkni_multi.h"
#include <stdio.h>

#define BDBG_MSG_TRACE(X)

BDBG_MODULE(brecord_gui);

struct brecord_gui
{
    struct brecord_gui_settings settings;
#define TOTAL_COLORS 4
    unsigned color[TOTAL_COLORS];
    NEXUS_SurfaceClientHandle surfaceClient;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_SurfaceHandle surface;
    BKNI_EventHandle displayedEvent, checkpointEvent;
    NEXUS_Rect last_cursor;
    NEXUS_Rect rect; /* client rect; x = y = 0. */
    bfont_t font;
    unsigned font_height;
    struct bfont_surface_desc desc;
};

void brecord_gui_get_default_settings(struct brecord_gui_settings *psettings)
{
    BKNI_Memset(psettings, 0, sizeof(*psettings));
    psettings->fontname = "nxclient/arial_18_aa.bwin_font";
    psettings->color = 0xFF0000FF;
    psettings->borderColor = 0xFF888888;
    psettings->zorder = 2;
}

static void complete2(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

brecord_gui_t brecord_gui_create(const struct brecord_gui_settings *psettings)
{
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NEXUS_Graphics2DSettings gfxSettings;
    NEXUS_SurfaceCreateSettings createSettings;
    int rc;
    brecord_gui_t handle;
    NEXUS_SurfaceComposition newcomp;
    NEXUS_SurfaceClientSettings settings;
    unsigned i;

    if (!psettings->recpump && !psettings->record) {
        return NULL;
    }

    handle = BKNI_Malloc(sizeof(*handle));
    if (!handle) return NULL;
    BKNI_Memset(handle, 0, sizeof(*handle));
    handle->settings = *psettings;

    rc = NxClient_Join(NULL);
    if (rc) return NULL;

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.surfaceClient = 1; /* surface client required for video window */
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) return NULL;

    BKNI_CreateEvent(&handle->checkpointEvent);
    BKNI_CreateEvent(&handle->displayedEvent);

    handle->surfaceClient = NEXUS_SurfaceClient_Acquire(allocResults.surfaceClient[0].id);
    BDBG_ASSERT(handle->surfaceClient);

    if (!handle->settings.recpump) {
        NEXUS_RecordSettings settings;
        NEXUS_Record_GetSettings(handle->settings.record, &settings);
        handle->settings.recpump = settings.recpump;
    }

    if (!handle->settings.rect.width || !handle->settings.rect.height) {
        NEXUS_RecpumpStatus status;
        rc = NEXUS_Recpump_GetStatus(handle->settings.recpump, &status);
        BDBG_ASSERT(!rc);
#define DISPLAY_WIDTH 1280
#define DISPLAY_HEIGHT 720
#define WIDTH 350
#define HEIGHT 50
#define SPACING 10
#define NUM_PER_COLUMN ((DISPLAY_HEIGHT-VIEWABLE_AREA_BORDER) / (HEIGHT+SPACING))
#define VIEWABLE_AREA_BORDER 50
        handle->settings.virtualDisplay.width = DISPLAY_WIDTH;
        handle->settings.virtualDisplay.height = DISPLAY_HEIGHT;
        handle->settings.rect.x = VIEWABLE_AREA_BORDER + (status.rave.index/NUM_PER_COLUMN) * (WIDTH+SPACING);
        handle->settings.rect.y = VIEWABLE_AREA_BORDER + (status.rave.index%NUM_PER_COLUMN) * (HEIGHT+SPACING);
        handle->settings.rect.width = WIDTH;
        handle->settings.rect.height = HEIGHT;
    }
    handle->rect.width = handle->settings.rect.width;
    handle->rect.height = handle->settings.rect.height;

    NxClient_Config_GetSurfaceClientComposition(NULL, handle->surfaceClient, &newcomp);
    newcomp.zorder = handle->settings.zorder;
    newcomp.position = handle->settings.rect;
    newcomp.virtualDisplay = handle->settings.virtualDisplay;
    NxClient_Config_SetSurfaceClientComposition(NULL, handle->surfaceClient, &newcomp);
    NEXUS_SurfaceClient_GetSettings(handle->surfaceClient, &settings);
    settings.displayed.callback = complete2;
    settings.displayed.context = handle->displayedEvent;
    NEXUS_SurfaceClient_SetSettings(handle->surfaceClient, &settings);

    handle->gfx = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, NULL);
    NEXUS_Graphics2D_GetSettings(handle->gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete2;
    gfxSettings.checkpointCallback.context = handle->checkpointEvent;
    rc = NEXUS_Graphics2D_SetSettings(handle->gfx, &gfxSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = WIDTH;
    createSettings.height = HEIGHT;
    handle->surface = NEXUS_Surface_Create(&createSettings);

    handle->font = bfont_open(handle->settings.fontname);
    if (!handle->font) {
        BDBG_WRN(("unable to load font %s", handle->settings.fontname));
    }
    else {
        bfont_get_height(handle->font, &handle->font_height);
        bfont_get_surface_desc(handle->surface, &handle->desc);
    }

    handle->color[0] = handle->settings.color;
    for (i=1;i<TOTAL_COLORS;i++) {
        handle->color[i] = ((handle->color[i-1] >> 1) & 0x7F7F7F) | 0xFF000000;
    }
    brecord_gui_update(handle);

    return handle;
}

void brecord_gui_update(brecord_gui_t handle)
{
    int rc;
    NEXUS_RecpumpStatus status;
    NEXUS_RecordStatus recordStatus;
    NEXUS_Graphics2DFillSettings fillSettings;
    unsigned colorIndex;

    if (handle->settings.record) {
        NEXUS_Record_GetStatus(handle->settings.record, &recordStatus);
        status = recordStatus.recpumpStatus;
    }
    else {
        NEXUS_Recpump_GetStatus(handle->settings.recpump, &status);
    }

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = handle->surface;
    fillSettings.color = handle->settings.borderColor;
    rc = NEXUS_Graphics2D_Fill(handle->gfx, &fillSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = handle->surface;
    fillSettings.rect = handle->rect;
#define BORDER 3
    fillSettings.rect.x += BORDER;
    fillSettings.rect.y += BORDER;
    fillSettings.rect.width -= BORDER*2;
    fillSettings.rect.height -= BORDER*2;
    colorIndex = ((status.data.bytesRecorded/50000) / fillSettings.rect.width) % TOTAL_COLORS;
    fillSettings.rect.width = (status.data.bytesRecorded/50000) % fillSettings.rect.width;
    BDBG_MSG_TRACE(("%s fifo %d/%d (%d%%)", handle->settings.destName, status.data.fifoDepth, status.data.fifoSize, status.data.fifoDepth * 100 / status.data.fifoSize));
    if (fillSettings.rect.width) {
        fillSettings.color = handle->color[colorIndex];
        rc = NEXUS_Graphics2D_Fill(handle->gfx, &fillSettings);
        BDBG_ASSERT(!rc);
    }

    fillSettings.color = handle->color[colorIndex?colorIndex-1:TOTAL_COLORS-1];
    fillSettings.rect.x += fillSettings.rect.width;
    fillSettings.rect.width = handle->rect.width - fillSettings.rect.width - BORDER*2;
    if (fillSettings.rect.width) {
        rc = NEXUS_Graphics2D_Fill(handle->gfx, &fillSettings);
        BDBG_ASSERT(!rc);
    }

    rc = NEXUS_Graphics2D_Checkpoint(handle->gfx, NULL); /* require to execute queue */
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(handle->checkpointEvent, BKNI_INFINITE);
    }
    BDBG_ASSERT(!rc);

#define MB (1024*1024)
    if (handle->font) {
        NEXUS_Rect textrect = handle->rect;
        int width, height, base;
        char duration[32];

        textrect.height = handle->font_height;

        textrect.x = BORDER;
        textrect.y = BORDER;
        textrect.width = handle->rect.width - BORDER*2;
        bfont_draw_aligned_text(&handle->desc, handle->font, &textrect, handle->settings.sourceName, -1, 0xFFCCCCCC, bfont_valign_center, bfont_halign_left);

        snprintf(duration, sizeof(duration), "%u MB", (unsigned)(status.data.bytesRecorded / MB));
        bfont_measure_text(handle->font, duration, -1, &width, &height, &base);
        textrect.x = handle->rect.width - width - BORDER;
        textrect.y = BORDER;
        textrect.width = width;
        bfont_draw_aligned_text(&handle->desc, handle->font, &textrect, duration, -1, 0xFFCCCCCC, bfont_valign_center, bfont_halign_left);

        textrect.x = BORDER;
        textrect.y = handle->rect.height - handle->font_height - BORDER;
        textrect.width = handle->rect.width - BORDER*2;
        bfont_draw_aligned_text(&handle->desc, handle->font, &textrect, handle->settings.destName, -1, 0xFFCCCCCC, bfont_valign_center, bfont_halign_left);

        if (handle->settings.record) {
            snprintf(duration, sizeof(duration), "%lu:%02lu", recordStatus.lastTimestamp / 1000 / 60, (recordStatus.lastTimestamp / 1000) % 60);
        }
        else {
            unsigned bitrate = status.bitrate * 10 / MB;
            snprintf(duration, sizeof(duration), "%u.%01u Mbps", bitrate / 10, bitrate % 10);
        }
        bfont_measure_text(handle->font, duration, -1, &width, &height, &base);
        textrect.x = textrect.x + textrect.width - width - BORDER;
        textrect.y = handle->rect.height - handle->font_height - BORDER;
        textrect.width = width;
        bfont_draw_aligned_text(&handle->desc, handle->font, &textrect, duration, -1, 0xFFCCCCCC, bfont_valign_center, bfont_halign_left);

        NEXUS_Surface_Flush(handle->surface);
    }

    rc = NEXUS_SurfaceClient_SetSurface(handle->surfaceClient, handle->surface);
    BDBG_ASSERT(!rc);

    rc = BKNI_WaitForEvent(handle->displayedEvent, 5000);
    BDBG_ASSERT(!rc);
}

void brecord_gui_destroy(brecord_gui_t handle)
{
    NEXUS_Graphics2D_Close(handle->gfx);
    NEXUS_Surface_Destroy(handle->surface);
    BKNI_DestroyEvent(handle->checkpointEvent);
    BKNI_DestroyEvent(handle->displayedEvent);
    if (handle->font) {
        bfont_close(handle->font);
    }
    NEXUS_SurfaceClient_Release(handle->surfaceClient);
    NxClient_Uninit();
    BKNI_Free(handle);
}
#else
typedef unsigned unused;
#endif
