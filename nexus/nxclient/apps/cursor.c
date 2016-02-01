/***************************************************************************
 *     (c)2011-2013 Broadcom Corporation
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
#include "nxclient.h"
#include "nexus_surface.h"
#include "nexus_surface_client.h"
#if NEXUS_HAS_INPUT_ROUTER
#include "nexus_input_client.h"
#endif
#include "linux/input.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

BDBG_MODULE(cursor);

/* TODO: use evdev directly for better performance */

struct {
    unsigned surfaceClientId;
    NEXUS_Rect rect;
} g_app;

static const unsigned g_cursor[8][8] = {
    {0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00},
    {0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00},
    {0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00},
    {0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00},
    {0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00},
};

#if NEXUS_HAS_INPUT_ROUTER
static void input_client_callback(void *context, int param)
{
    NEXUS_InputClientHandle inputClient = context;
    NEXUS_SurfaceComposition comp;
    BSTD_UNUSED(param);
    NxClient_GetSurfaceClientComposition(g_app.surfaceClientId, &comp);
    while (1) {
        NEXUS_InputRouterCode code[10];
        unsigned i;
        int rc;
        unsigned num;
        rc = NEXUS_InputClient_GetCodes(inputClient, code, 10, &num);
        if (rc || !num) {
            break;
        }
        for (i=0;i<10;i++) {
            if (code[i].deviceType == NEXUS_InputRouterDevice_eEvdev) {
                switch (code[i].data.evdev.type) {
                case EV_REL:
                    if (code[i].data.evdev.code == REL_X) {
                        g_app.rect.x += code[i].data.evdev.value;
                    }
                    else if (code[i].data.evdev.code == REL_Y) {
                        g_app.rect.y += code[i].data.evdev.value;
                    }
                    break;
                case EV_SYN:
                case EV_MSC:
                    break;
                case EV_KEY:
                    if (code[i].data.evdev.code == BTN_LEFT) {
                        BDBG_WRN(("left click at %dx%d", g_app.rect.x, g_app.rect.y));
                        break;
                    }
                    else if (code[i].data.evdev.code == BTN_RIGHT) {
                        BDBG_WRN(("right click at %dx%d", g_app.rect.x, g_app.rect.y));
                        break;
                    }
                    /* else fall through */
                default:
                    BDBG_WRN(("unknown: %d %d %d", code[i].data.evdev.type, code[i].data.evdev.code, code[i].data.evdev.value));
                    break;
                }
            }
        }
        comp.position = g_app.rect;
        NxClient_SetSurfaceClientComposition(g_app.surfaceClientId, &comp);
    }
}
#endif

static void print_usage(void)
{
    printf(
    "Usage: cursor OPTIONS\n"
    "\n"
    "OPTIONS:\n"
    "  --help or -h for help\n"
    "  -alpha XX\n"
    );
}

int main(int argc, char **argv)
{
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NEXUS_SurfaceHandle surface;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_SurfaceClientHandle blit_client;
    NEXUS_SurfaceMemory mem;
#if NEXUS_HAS_INPUT_ROUTER
    NEXUS_InputClientHandle inputClient;
#endif
    NEXUS_Error rc;
    unsigned x, y;
    int curarg = 1;
    unsigned alpha = 0x80;

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-alpha") && argc>curarg+1) {
            alpha = strtoul(argv[++curarg], NULL, 0);
        }
        curarg++;
    }

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.surfaceClient = 1;
    allocSettings.inputClient = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) return BERR_TRACE(rc);

    blit_client = NEXUS_SurfaceClient_Acquire(allocResults.surfaceClient[0].id);
    if (!blit_client) {
        BDBG_ERR(("NEXUS_SurfaceClient_Acquire failed"));
        return -1;
    }

#if NEXUS_HAS_INPUT_ROUTER
    if (allocResults.inputClient[0].id) {
        inputClient = NEXUS_InputClient_Acquire(allocResults.inputClient[0].id);
        if (inputClient) {
            NEXUS_InputClientSettings settings;
            NEXUS_InputClient_GetSettings(inputClient, &settings);
            settings.filterMask = 1<<NEXUS_InputRouterDevice_eEvdev; /* only evdev */
            settings.codeAvailable.callback = input_client_callback;
            settings.codeAvailable.context = inputClient;
            NEXUS_InputClient_SetSettings(inputClient, &settings);
        }
    }
#endif

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = 8;
    createSettings.height = 8;
    surface = NEXUS_Surface_Create(&createSettings);
    NEXUS_Surface_GetMemory(surface, &mem);
    /* draw blue cursor */
    for (y=0;y<createSettings.height;y++) {
        uint32_t *ptr = (uint32_t *)(((uint8_t*)mem.buffer) + y * mem.pitch);
        for (x=0;x<createSettings.width;x++) {
            ptr[x] = g_cursor[y][x] ? ((alpha << 24) | g_cursor[y][x]) : 0x00;
        }
    }
    NEXUS_Surface_Flush(surface);

    g_app.surfaceClientId = allocResults.surfaceClient[0].id;
    g_app.rect.x = 1920/2;
    g_app.rect.y = 1080/2;
    g_app.rect.width = createSettings.width*3;
    g_app.rect.height = createSettings.height*3;

    {
        static const NEXUS_BlendEquation g_colorBlend = {NEXUS_BlendFactor_eSourceAlpha, NEXUS_BlendFactor_eSourceColor, false,
            NEXUS_BlendFactor_eInverseSourceAlpha, NEXUS_BlendFactor_eDestinationColor, false, NEXUS_BlendFactor_eZero};
        static const NEXUS_BlendEquation g_alphaBlend = {NEXUS_BlendFactor_eDestinationAlpha, NEXUS_BlendFactor_eOne, false,
            NEXUS_BlendFactor_eZero, NEXUS_BlendFactor_eZero, false, NEXUS_BlendFactor_eZero};
        NEXUS_SurfaceComposition comp;
        NxClient_GetSurfaceClientComposition(g_app.surfaceClientId, &comp);
        comp.colorBlend = g_colorBlend;
        comp.alphaBlend = g_alphaBlend;
        comp.position = g_app.rect;
        NxClient_SetSurfaceClientComposition(g_app.surfaceClientId, &comp);
    }

    rc = NEXUS_SurfaceClient_SetSurface(blit_client, surface);
    BDBG_ASSERT(!rc);

    while (1) BKNI_Sleep(100);

    NEXUS_SurfaceClient_Release(blit_client);
    NxClient_Free(&allocResults);
    NxClient_Uninit();
    return 0;
}
