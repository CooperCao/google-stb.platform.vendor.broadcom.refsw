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
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "bfont.h"

BDBG_MODULE(osd);

static void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static void print_usage(void)
{
    printf(
    "Usage: osd OPTIONS\n"
    "  --help or -h for help\n"
    "  -rect x,y,width,height   position in default 1920x1080 coordinates\n"
    "  -timeout MSEC            time between pages (default 1000 msec)\n"
    "  -eof_timeout MSEC        time to continue display after end of file (default 0, which is forever)\n"
    "  -zorder #\n"
    );
}

int main(int argc, const char **argv)
{
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NEXUS_SurfaceHandle surface;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_SurfaceMemory mem;
    struct bfont_surface_desc desc;
    BKNI_EventHandle displayedEvent;
    NEXUS_SurfaceClientHandle blit_client;
    NEXUS_SurfaceClientSettings client_settings;
    NEXUS_Error rc;
    int curarg = 1;
    NEXUS_Rect rect = {0,0,0,0};
    unsigned zorder = 0;
    bfont_t font;
    unsigned font_height;
    const char *fontname = "nxclient/arial_18_aa.bwin_font";
    unsigned timeout = 1000;
    unsigned eof_timeout = 0;
    
    while (argc > curarg) {
        if (!strcmp(argv[curarg], "--help") || !strcmp(argv[curarg], "-h")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-rect") && argc>curarg+1) {
            unsigned x, y, width, height;
            if (sscanf(argv[++curarg], "%u,%u,%u,%u", &x,&y,&width,&height) == 4) {
                rect.x = x;
                rect.y = y;
                rect.width = width;
                rect.height = height;
            }
        }
        else if (!strcmp(argv[curarg], "-zorder") && argc>curarg+1) {
            zorder = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-timeout") && argc>curarg+1) {
            timeout = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-eof_timeout") && argc>curarg+1) {
            eof_timeout = atoi(argv[++curarg]);
        }
        else {
            print_usage();
            return 1;
        }
        curarg++;
    }

    /* connect to server and nexus */
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    BKNI_CreateEvent(&displayedEvent);

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.surfaceClient = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) return BERR_TRACE(rc);
    
    /* No NxClient_Connect needed for SurfaceClient */
    
    blit_client = NEXUS_SurfaceClient_Acquire(allocResults.surfaceClient[0].id);
    if (!blit_client) {
        BDBG_ERR(("NEXUS_SurfaceClient_Acquire failed"));
        return -1;
    }
    
    NEXUS_SurfaceClient_GetSettings(blit_client, &client_settings);
    client_settings.displayed.callback = complete;
    client_settings.displayed.context = displayedEvent;
    rc = NEXUS_SurfaceClient_SetSettings(blit_client, &client_settings);
    BDBG_ASSERT(!rc);
    
    if (rect.width || zorder) {
        NEXUS_SurfaceComposition comp;   
        NxClient_GetSurfaceClientComposition(allocResults.surfaceClient[0].id, &comp);
        if (rect.width) {
            comp.position = rect;
        }
        comp.zorder = zorder;
        NxClient_SetSurfaceClientComposition(allocResults.surfaceClient[0].id, &comp);
    }

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = 1280;
    createSettings.height = 720;
    surface = NEXUS_Surface_Create(&createSettings);
    NEXUS_Surface_GetMemory(surface, &mem);
    bfont_get_surface_desc(surface, &desc);
    
    {
    int flags;
    flags = fcntl(fileno(stdin), F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(fileno(stdin), F_SETFL, flags);
    }
    
    font = bfont_open(fontname);
    if (!font) {
        BDBG_WRN(("unable to load font %s", fontname));
        return -1;
    }
    bfont_get_height(font, &font_height);
    
    rect.x = 0;
    while (!feof(stdin)) {
#define BORDER 40
#define LINESPACE 3
        unsigned lines = 0;

        if (!rect.x) {
            rect.x = BORDER;
            rect.y = BORDER;
            rect.height = font_height;
            rect.width = createSettings.width - BORDER*2;
            memset(mem.buffer, 0, createSettings.height * mem.pitch);
        }
        
        while (!feof(stdin)) {
            char buf[256];
            char *find;
            
            if (!fgets(buf, 256, stdin)) {
                break;
            }
            find = strchr(buf, '\n');
            if (find) *find = 0;
            
            /* printf("%s\n", buf); */
            bfont_draw_aligned_text(&desc, font, &rect, buf, -1, 0xFFCCCCCC, bfont_valign_top, bfont_halign_left);
            rect.y += rect.height + LINESPACE;
            lines++;

            if (rect.y >= createSettings.height - BORDER*2) {
                rect.x = 0;
                break;
            }
        }
        if (!lines) {
            BKNI_Sleep(10);
            continue;
        }
        NEXUS_Surface_Flush(surface);
        
        /* tell server to blit */
        rc = NEXUS_SurfaceClient_SetSurface(blit_client, surface);
        BDBG_ASSERT(!rc);
        rc = BKNI_WaitForEvent(displayedEvent, 5000);
        BDBG_ASSERT(!rc);
        
        if (!feof(stdin)) {
            BKNI_Sleep(timeout);
        }
    }

    if (eof_timeout) {
        BKNI_Sleep(eof_timeout);
    }
    else {
        /* no termination because graphics will disappear */
        while (1) BKNI_Sleep(1000);
    }
    
    return 0;
}
