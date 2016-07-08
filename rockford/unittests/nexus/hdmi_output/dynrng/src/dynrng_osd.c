/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *
 * Module Description:
 *
******************************************************************************/

#include "nexus_platform.h"
#include "nexus_hdmi_output.h"
#include "nexus_video_decoder.h"
#include "nexus_playback.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_graphics2d.h"
#include "nexus_surface.h"
#include "nexus_core_utils.h"
#include "bstd.h"
#include "bkni.h"
#include "blst_queue.h"
#include "dynrng_osd_priv.h"
#include "dynrng_utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/*#include <ctype.h>*/


/******************************************************************************
 *  string tables
 *****************************************************************************/

static const char * const osdModeStrings[] =
{
    "off",
    "on",
    "timer",
    NULL
};

void OSD_CheckpointComplete(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

void OSD_MoveIcon(OSD_IconHandle icon, int x, int y)
{
    icon->rect.x = x;
    icon->rect.y = y;
}

void OSD_DestroyIcon(OSD_IconHandle icon)
{
    if (icon)
    {
        if (icon->surface)
        {
            NEXUS_Surface_Destroy(icon->surface);
        }
        BKNI_Free(icon);
    }
}

OSD_IconHandle OSD_CreateIcon(OSD_ImageId imageId)
{
    OSD_IconHandle icon = NULL;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_SurfaceMemory mem;
    OSD_ImageHandle image = NULL;

    icon = BKNI_Malloc(sizeof(struct OSD_Icon));
    if (!icon) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error; }

    BKNI_Memset(icon, 0, sizeof(struct OSD_Icon));

    image = OSD_GetImageById(imageId);
    if (!image) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error; }

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    switch (image->bytes_per_pixel)
    {
        case 2:
            createSettings.pixelFormat = NEXUS_PixelFormat_eR5_G6_B5;
            break;
        case 4:
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE
            createSettings.pixelFormat = NEXUS_PixelFormat_eA8_B8_G8_R8;
#else
            createSettings.pixelFormat = NEXUS_PixelFormat_eR8_G8_B8_A8;
#endif
            break;
        default:
            fprintf(stderr, "Image %u bpp is not supported\n", image->bytes_per_pixel * 8);
            rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
            goto error;
            break;
    }
    icon->rect.width = createSettings.width = image->width;
    icon->rect.height = createSettings.height = image->height;
    createSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    icon->surface = NEXUS_Surface_Create(&createSettings);

    rc = NEXUS_Surface_GetMemory(icon->surface, &mem);
    if (rc) { rc = BERR_TRACE(rc); goto error; }
    BKNI_Memcpy(mem.buffer, image->pixel_data, image->width * image->height * image->bytes_per_pixel);

    return icon;

error:
    if (icon)
    {
        OSD_DestroyIcon(icon);
    }
    return NULL;
}

void OSD_DestroyScreen(OSD_ScreenHandle screen)
{
    if (screen)
    {
        if (screen->tv)
        {
            OSD_DestroyIcon(screen->tv);
            screen->tv = NULL;
        }
        if (screen->output)
        {
            OSD_DestroyIcon(screen->output);
            screen->output = NULL;
        }
        if (screen->input)
        {
            OSD_DestroyIcon(screen->input);
            screen->input = NULL;
        }

        NEXUS_Surface_Destroy(screen->surface);
        screen->surface = NULL;

        BKNI_Free(screen);
    }
}

OSD_ScreenHandle OSD_CreateScreen(unsigned width, unsigned height, unsigned statusHeight)
{
    OSD_ScreenHandle screen = NULL;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_SurfaceMemory mem;
    int x;
    int y;

    screen = BKNI_Malloc(sizeof(struct OSD_Screen));
    if (!screen) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error; }

    BKNI_Memset(screen, 0, sizeof(struct OSD_Screen));

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    screen->rect.width = createSettings.width = width;
    screen->rect.height = createSettings.height = height;
    createSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    screen->surface = NEXUS_Surface_Create(&createSettings);

    /* fill screen with black */
    rc = NEXUS_Surface_GetMemory(screen->surface, &mem);
    if (rc) { rc = BERR_TRACE(rc); goto error; }
    BKNI_Memset(mem.buffer, 0, screen->rect.height * mem.pitch);

    screen->tv = OSD_CreateIcon(OSD_ImageId_eTv);
    if (!screen->tv) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error; }
    screen->output = OSD_CreateIcon(OSD_ImageId_eOutput);
    if (!screen->output) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error; }
    screen->input = OSD_CreateIcon(OSD_ImageId_eInput);
    if (!screen->input) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error; }

    y = screen->rect.height;
    y -= screen->output->rect.height;
    y -= statusHeight;
    y -= 20; /* for overscan */
    x = screen->rect.width;
    x -= screen->tv->rect.width + 10;
    x -= screen->output->rect.width + 10;
    x -= screen->input->rect.width + 10;
    x -= 20; /* for overscan */
    OSD_MoveIcon(screen->input, x, y);
    x += screen->input->rect.width + 10;
    OSD_MoveIcon(screen->output, x, y);
    x += screen->output->rect.width + 10;
    OSD_MoveIcon(screen->tv, x, y);

    return screen;

error:
    if (screen)
    {
        OSD_DestroyScreen(screen);
    }
    return NULL;
}

int OSD_DrawIcon(OSD_OsdHandle osd, OSD_IconHandle icon, int x, int y)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_Graphics2DBlitSettings blitSettings;

    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    blitSettings.source.surface = icon->surface;
    blitSettings.output.surface = osd->screen->surface;
    blitSettings.output.rect.x = x;
    blitSettings.output.rect.y = y;
    blitSettings.output.rect.width = icon->rect.width;
    blitSettings.output.rect.height = icon->rect.height;
    rc = NEXUS_Graphics2D_Blit(osd->graphics, &blitSettings);
    if (rc) { fprintf(stderr, "Error blitting status\n"); rc = BERR_TRACE(rc); goto end; }

    rc = NEXUS_Graphics2D_Checkpoint(osd->graphics, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(osd->checkpointEvent, BKNI_INFINITE);
    }

end:
    return rc;
}

int OSD_DrawBlank(OSD_OsdHandle osd, OSD_ScreenHandle screen)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_SurfaceMemory mem;

    BSTD_UNUSED(osd);

    rc = NEXUS_Surface_GetMemory(screen->surface, &mem);
    if (rc) { rc = BERR_TRACE(rc); goto end; }

    /* fill screen with black */
    BKNI_Memset(mem.buffer, 0, screen->rect.height * mem.pitch);

    /* flush cached memory */
    NEXUS_Surface_Flush(screen->surface);

end:
    return rc;
}

int OSD_PopulateEotfIcon(OSD_OsdHandle osd, OSD_IconHandle icon, OSD_Eotf eotf)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    rc = OSD_DrawIcon(osd, icon, icon->rect.x, icon->rect.y);
    if (rc) { rc = BERR_TRACE(rc); goto end; }

    switch (eotf)
    {
        case OSD_Eotf_eUnknown:
            rc = OSD_DrawIcon(osd, osd->unknown, icon->rect.x, icon->rect.y + icon->rect.height);
            if (rc) { rc = BERR_TRACE(rc); goto end; }
            break;
        case OSD_Eotf_eSdr:
            rc = OSD_DrawIcon(osd, osd->sdr, icon->rect.x, icon->rect.y + icon->rect.height);
            if (rc) { rc = BERR_TRACE(rc); goto end; }
            break;
        case OSD_Eotf_eHdr10:
            rc = OSD_DrawIcon(osd, osd->hdr10, icon->rect.x, icon->rect.y + icon->rect.height);
            if (rc) { rc = BERR_TRACE(rc); goto end; }
            break;
        case OSD_Eotf_eHlg:
            rc = OSD_DrawIcon(osd, osd->hlg, icon->rect.x, icon->rect.y + icon->rect.height);
            if (rc) { rc = BERR_TRACE(rc); goto end; }
            break;
        default:
            break;
    }

end:
    return rc;
}

int OSD_PopulateEotfSupportIcon(OSD_OsdHandle osd, OSD_IconHandle icon, OSD_EotfSupport support)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    rc = OSD_DrawIcon(osd, icon, icon->rect.x, icon->rect.y);
    if (rc) { rc = BERR_TRACE(rc); goto end; }

    switch (support)
    {
        case OSD_EotfSupport_eUnknown:
            rc = OSD_DrawIcon(osd, osd->unknown, icon->rect.x, icon->rect.y + icon->rect.height);
            if (rc) { rc = BERR_TRACE(rc); goto end; }
            break;
        case OSD_EotfSupport_eNo:
            rc = OSD_DrawIcon(osd, osd->no, icon->rect.x, icon->rect.y + icon->rect.height);
            if (rc) { rc = BERR_TRACE(rc); goto end; }
            break;
        case OSD_EotfSupport_eYes:
            rc = OSD_DrawIcon(osd, osd->yes, icon->rect.x, icon->rect.y + icon->rect.height);
            if (rc) { rc = BERR_TRACE(rc); goto end; }
            break;
        default:
            break;
    }

end:
    return rc;
}

int OSD_DrawScreen(OSD_OsdHandle osd, OSD_ScreenHandle screen)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    rc = OSD_PopulateEotfIcon(osd, screen->input, osd->model.input);
    if (rc) { rc = BERR_TRACE(rc); goto end; }

    rc = OSD_PopulateEotfIcon(osd, screen->output, osd->model.output);
    if (rc) { rc = BERR_TRACE(rc); goto end; }

    rc = OSD_PopulateEotfSupportIcon(osd, screen->tv, osd->model.tv);
    if (rc) { rc = BERR_TRACE(rc); goto end; }

end:
    return rc;
}

void OSD_Destroy(OSD_OsdHandle osd)
{
    if (osd)
    {
        if (osd->unknown)
        {
            OSD_DestroyIcon(osd->unknown);
            osd->unknown = NULL;
        }
        if (osd->sdr)
        {
            OSD_DestroyIcon(osd->sdr);
            osd->sdr = NULL;
        }
        if (osd->hdr10)
        {
            OSD_DestroyIcon(osd->hdr10);
            osd->hdr10 = NULL;
        }
        if (osd->hlg)
        {
            OSD_DestroyIcon(osd->hlg);
            osd->hlg = NULL;
        }
        if (osd->screen)
        {
            OSD_DestroyScreen(osd->screen);
            osd->screen = NULL;
        }
        if (osd->graphics)
        {
            NEXUS_Graphics2D_Close(osd->graphics);
            osd->graphics = NULL;
        }

        if (osd->checkpointEvent)
        {
            BKNI_DestroyEvent(osd->checkpointEvent);
            osd->checkpointEvent = NULL;
        }

        BKNI_Free(osd);
    }
}

OSD_OsdHandle OSD_Create(unsigned width, unsigned height)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    OSD_OsdHandle osd = NULL;
    NEXUS_Graphics2DOpenSettings graphicsOpenSettings;
    NEXUS_Graphics2DSettings graphicsSettings;

    osd = BKNI_Malloc(sizeof(struct OSD_Osd));
    if (!osd) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error; }

    BKNI_Memset(osd, 0, sizeof(struct OSD_Osd));

    rc = BKNI_CreateEvent(&osd->checkpointEvent);
    BDBG_ASSERT(!rc);

    NEXUS_Graphics2D_GetDefaultOpenSettings(&graphicsOpenSettings);
    graphicsOpenSettings.packetFifoSize = 1024; /* only single blit queued */
    osd->graphics = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, &graphicsOpenSettings);
    if (!osd->graphics) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error; }

    NEXUS_Graphics2D_GetSettings(osd->graphics, &graphicsSettings);
    graphicsSettings.checkpointCallback.callback = &OSD_CheckpointComplete;
    graphicsSettings.checkpointCallback.context = osd->checkpointEvent;
    rc = NEXUS_Graphics2D_SetSettings(osd->graphics, &graphicsSettings);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

    osd->hlg = OSD_CreateIcon(OSD_ImageId_eHlg);
    if (!osd->hlg) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error; }
    osd->hdr10 = OSD_CreateIcon(OSD_ImageId_eHdr10);
    if (!osd->hdr10) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error; }
    osd->sdr = OSD_CreateIcon(OSD_ImageId_eSdr);
    if (!osd->sdr) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error; }
    osd->unknown = OSD_CreateIcon(OSD_ImageId_eUnknown);
    if (!osd->unknown) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error; }
    osd->yes = OSD_CreateIcon(OSD_ImageId_eYes);
    if (!osd->yes) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error; }
    osd->no = OSD_CreateIcon(OSD_ImageId_eNo);
    if (!osd->no) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error; }

    osd->screen = OSD_CreateScreen(width, height, osd->sdr->rect.height);
    if (!osd->screen) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error; }

    return osd;

error:
    if (osd)
    {
        OSD_Destroy(osd);
    }
    return NULL;
}

NEXUS_SurfaceHandle OSD_GetFrameBuffer(OSD_OsdHandle osd)
{
    if (osd && osd->screen)
    {
        return osd->screen->surface;
    }
    else
    {
        return NULL;
    }
}

void OSD_GetSettings(OSD_OsdHandle osd, OSD_OsdSettings * pSettings)
{
    if (osd && pSettings)
    {
        *pSettings = osd->settings;
    }
}

int OSD_SetSettings(OSD_OsdHandle osd, const OSD_OsdSettings * pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    if (osd && pSettings)
    {
        osd->settings = *pSettings;
        fprintf(stdout, "OSD: %s\n", osdModeStrings[osd->settings.mode]);
        rc = OSD_Draw(osd);
        if (rc) { rc = BERR_TRACE(rc); goto end; }
    }

end:
    return rc;
}

void OSD_GetModel(OSD_OsdHandle osd, OSD_OsdModel * pModel)
{
    if (osd && pModel)
    {
        *pModel = osd->model;
    }
}

int OSD_SetModel(OSD_OsdHandle osd, const OSD_OsdModel * pModel)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    if (osd && pModel)
    {
        osd->model = *pModel;
        rc = OSD_Draw(osd);
        if (rc) { rc = BERR_TRACE(rc); goto end; }
    }

end:
    return rc;
}

int OSD_Draw(OSD_OsdHandle osd)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    if (osd->settings.mode == OSD_OsdMode_eOn || osd->settings.mode == OSD_OsdMode_eTimer)
    {
        rc = OSD_DrawScreen(osd, osd->screen);
        if (rc) { rc = BERR_TRACE(rc); goto end; }
    }
    else
    {
        rc = OSD_DrawBlank(osd, osd->screen);
        if (rc) { rc = BERR_TRACE(rc); goto end; }
    }
    /* schedule timer to blank screen
    if (osd->settings.mode == OsdMode_eTimer)
    {

    }
    */
end:
    return rc;
}

static const UTILS_StringIntMapEntry osdModeNames[] =
{
    { "off", OSD_OsdMode_eOff },
    { "on", OSD_OsdMode_eOn },
    { "timer", OSD_OsdMode_eTimer },
    { NULL, OSD_OsdMode_eMax }
};

const char * OSD_GetModeName(OSD_OsdMode mode)
{
    return UTILS_GetTableName(osdModeNames, mode);
}

static const UTILS_StringIntMapEntry modeAliases[] =
{
    { "off", OSD_OsdMode_eOff },
    { "0", OSD_OsdMode_eOff },
    { "on", OSD_OsdMode_eOn },
    { "1", OSD_OsdMode_eOn },
    { "timer", OSD_OsdMode_eTimer },
    { "tmr", OSD_OsdMode_eTimer },
    { NULL, OSD_OsdMode_eMax }
};

OSD_OsdMode OSD_ParseOsdMode(const char * osdStr)
{
    return (OSD_OsdMode)UTILS_ParseTableAlias(modeAliases, osdStr);
}

void OSD_PrintModel(const OSD_OsdModel * pModel)
{
    fprintf(stdout, "# OSD model\n");
    fprintf(stdout, "input = %s\n", OSD_GetEotfName(pModel->input));
    fprintf(stdout, "output = %s\n", OSD_GetEotfName(pModel->output));
    fprintf(stdout, "tv support = %s\n", OSD_GetEotfSupportName(pModel->tv));
}

static const UTILS_StringIntMapEntry eotfNames[] =
{
    { "unknown", OSD_Eotf_eUnknown },
    { "sdr", OSD_Eotf_eSdr },
    { "hdr10", OSD_Eotf_eHdr10 },
    { "hlg", OSD_Eotf_eHlg },
    { NULL, OSD_Eotf_eMax }
};

const char * OSD_GetEotfName(OSD_Eotf eotf)
{
    return UTILS_GetTableName(eotfNames, eotf);
}

static const UTILS_StringIntMapEntry eotfSupportNames[] =
{
    { "unknown", OSD_EotfSupport_eUnknown },
    { "no", OSD_EotfSupport_eNo },
    { "yes", OSD_EotfSupport_eYes },
    { NULL, OSD_EotfSupport_eMax }
};

const char * OSD_GetEotfSupportName(OSD_EotfSupport support)
{
    return UTILS_GetTableName(eotfSupportNames, support);
}

void OSD_PrintIcon(const char * tag, OSD_IconHandle icon)
{
    fprintf(stdout, "# %s icon\n", tag);
    fprintf(stdout, "%ux%u at (%u, %u)\n", icon->rect.width, icon->rect.height, icon->rect.x, icon->rect.y);
}

void OSD_PrintScreen(const char * tag, OSD_ScreenHandle screen)
{
    fprintf(stdout, "# %s screen\n", tag);
    fprintf(stdout, "%ux%u at (%u, %u)\n", screen->rect.width, screen->rect.height, screen->rect.x, screen->rect.y);
    fprintf(stdout, "# screen icons:\n");
    OSD_PrintIcon("input", screen->input);
    OSD_PrintIcon("output", screen->output);
    OSD_PrintIcon("tv", screen->tv);
}

void OSD_Print(OSD_OsdHandle osd)
{
    fprintf(stdout, "# OSD settings\n");
    fprintf(stdout, "mode = %s\n", OSD_GetModeName(osd->settings.mode));
    fprintf(stdout, "timeout = %u\n", osd->settings.timeout);
    OSD_PrintModel(&osd->model);
    OSD_PrintScreen("main", osd->screen);
    fprintf(stdout, "# state icons:\n");
    OSD_PrintIcon("hdr10", osd->hdr10);
    OSD_PrintIcon("hlg", osd->hlg);
    OSD_PrintIcon("sdr", osd->sdr);
    OSD_PrintIcon("unknown", osd->unknown);
    OSD_PrintIcon("yes", osd->yes);
    OSD_PrintIcon("no", osd->no);
}

const NEXUS_Rect * OSD_GetFrameBufferDimensions(OSD_OsdHandle osd)
{
    return &osd->screen->rect;
}
