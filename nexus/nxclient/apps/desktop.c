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
 *****************************************************************************/
#if NEXUS_HAS_INPUT_ROUTER && NEXUS_HAS_PICTURE_DECODER && NEXUS_HAS_PLAYBACK && NEXUS_HAS_SIMPLE_DECODER
/**
A simple, modal client launcher
The desktop reads a "desktop.cfg" file which determines what clients it can launch.
User can select and launch a client with the IR remote.
Control returns to the desktop when the client exits.
**/
#include "nxclient.h"
#include "nexus_platform_client.h"
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "nexus_surface_client.h"
#include "bstd.h"
#include "bkni.h"
#include "blst_queue.h"
#include "bkni_multi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "media_player.h"
#include "binput.h"
#include "picdecoder.h"
#include "bfont.h"
#include "bgui.h"

BDBG_MODULE(desktop);

#define DEFAULT_FONTNAME  "nxclient/arial_18_aa.bwin_font"

static void print_usage(void)
{
    printf(
    "'desktop' is a multiprocess app launcher.\n"
    "\n"
    "Options:\n"
    "  --help or -h for help\n"
    "  -cfg CONFIGFILE         list of files to launch. default is nxclient/desktop.cfg\n"
    "  -background IMAGE       default is nxclient/desktop_background.png\n"
    "  -f FONTNAME             <FONTNAME>font file to use in menu\n"
    );
}

struct desktop_item
{
    BLST_Q_ENTRY(desktop_item) link;
    char name[256];
    char cmdline[256];
};

struct appcontext {
    NEXUS_SurfaceHandle background;
    binput_t input;
    BLST_Q_HEAD(desktop_item_list, desktop_item) launch;
    unsigned total_buttons, focused_button;
    bfont_t font;
    bgui_t gui;
};

static void render_ui(struct appcontext *pContext);
static void launch_client(struct appcontext *pContext);

int main(int argc, const char **argv)
{
    NxClient_JoinSettings joinSettings;
    NEXUS_ClientConfiguration clientConfig;
    NEXUS_Error rc;
    int curarg = 1;
    struct appcontext appcontext, *pContext = &appcontext;
    FILE *launchfile;
    const char *launchfilename = "nxclient/desktop.cfg";
    const char *background = "nxclient/desktop_background.png";
    char fontname[100];
    bool done = false;
    struct desktop_item *item;

    memset(pContext, 0, sizeof(*pContext));
    strcpy(fontname, DEFAULT_FONTNAME);

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "--help") || !strcmp(argv[curarg], "-h")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-background") && argc>curarg+1) {
            background = argv[++curarg];
        }
        else if (!strcmp(argv[curarg], "-cfg") && argc>curarg+1) {
            launchfilename = argv[++curarg];
        }
#ifdef FREETYPE_SUPPORT
        else if (!strcmp(argv[curarg], "-f") && argc>curarg+1) {
            strcpy(fontname, argv[++curarg]);
        }
#endif
        else {
            print_usage();
            return 1;
        }
        curarg++;
    }

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    launchfile = fopen(launchfilename, "r");
    if (!launchfile) {
        printf("ERROR: unable to open %s\n", launchfilename);
        return 1;
    }
    while (!feof(launchfile)) {
        char buf[256];
        char *s;
        fgets(buf, 256, launchfile);
        
        s = buf;
        s += strspn(s, " \t");
        if (*s && *s != '#') {
            char *find;
            find = strchr(s, '\n');
            if (find) *find = 0;
            find = strchr(s, ':');
            if (find) {
                item = BKNI_Malloc(sizeof(*item));
                BKNI_Memset(item, 0, sizeof(*item));
                strncpy(item->name, s, find-s);
                strcpy(item->cmdline, ++find);
                BLST_Q_INSERT_TAIL(&pContext->launch, item, link);
                pContext->total_buttons++;
            }
        }
    }

    NEXUS_Platform_GetClientConfiguration(&clientConfig);

    pContext->input = binput_open(NULL);
    pContext->gui = bgui_create(NULL);

#ifdef FREETYPE_SUPPORT
    {
        struct bfont_open_freetype_settings settings;
        bfont_get_default_open_freetype_settings(&settings);
        settings.filename = fontname;
        settings.size = 20;
        pContext->font = bfont_open_freetype(&settings);
    }
#else
    pContext->font = bfont_open(fontname);
#endif
    if (!pContext->font) {
        BDBG_WRN(("unable to load font %s", fontname));
    }

    if (background) {
        picdecoder_t handle;
        handle = picdecoder_open();
        if (handle) {
            pContext->background = picdecoder_decode(handle, background);
            picdecoder_close(handle);
        }
    }
    
    render_ui(pContext);
    while (!done) {
        b_remote_key key;
        if (binput_read_no_repeat(pContext->input, &key)) {
            binput_wait(pContext->input, 1000);
            continue;
        }
        switch (key) {
        case b_remote_key_up:
            if (pContext->focused_button) {
                pContext->focused_button--;
            }
            else {
                pContext->focused_button = pContext->total_buttons-1;
            }
            render_ui(pContext);
            break;
        case b_remote_key_down:
            if (++pContext->focused_button == pContext->total_buttons) {
                pContext->focused_button = 0;
            }
            render_ui(pContext);
            break;
        case b_remote_key_select:
            launch_client(pContext);
            render_ui(pContext);
            break;
#if 0
/* desktop is top-level so don't allow user input to exit */
        case b_remote_key_stop:
        case b_remote_key_back:
        case b_remote_key_clear:
            done = true;
            break;
#endif
        default:
            break;
        }
    }

    if (pContext->font) {
        bfont_close(pContext->font);
    }
    bgui_destroy(pContext->gui);
    binput_close(pContext->input);
    if (pContext->background) {
        NEXUS_Surface_Destroy(pContext->background);
    }
    while ((item=BLST_Q_FIRST(&pContext->launch))) {
        BLST_Q_REMOVE_HEAD(&pContext->launch, link);
        BKNI_Free(item);
    }
    NxClient_Uninit();
    return 0;
}

static void render_ui(struct appcontext *pContext)
{
    NEXUS_Graphics2DFillSettings fillSettings;
    int rc;
    unsigned i;
    struct desktop_item *item;
    NEXUS_SurfaceHandle surface = bgui_surface(pContext->gui);

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = surface;
    
    if (pContext->background) {
        NEXUS_Graphics2DBlitSettings blitSettings;
        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.source.surface = pContext->background;
        blitSettings.output.surface = surface;
        rc = NEXUS_Graphics2D_Blit(bgui_blitter(pContext->gui), &blitSettings);
        BDBG_ASSERT(!rc);
    }
    else {
        fillSettings.color = 0;
        rc = NEXUS_Graphics2D_Fill(bgui_blitter(pContext->gui), &fillSettings);
        BDBG_ASSERT(!rc);
    }

    for (i=0,item=BLST_Q_FIRST(&pContext->launch);item;i++,item=BLST_Q_NEXT(item,link)) {
        bool focused = i == pContext->focused_button;
        
        fillSettings.rect.x = 50;
        fillSettings.rect.y = 50 + i*50;
        fillSettings.rect.width = 200;
        fillSettings.rect.height = 40;
        if (focused) {
            fillSettings.color = 0xFF00FF00;
            fillSettings.colorOp = NEXUS_FillOp_eCopy;
        }
        else {
            NEXUS_BlendEquation g_colorBlend = {NEXUS_BlendFactor_eDestinationColor, NEXUS_BlendFactor_eConstantAlpha, false,
                NEXUS_BlendFactor_eConstantColor, NEXUS_BlendFactor_eInverseConstantAlpha, false, NEXUS_BlendFactor_eZero};
            NEXUS_BlendEquation g_alphaBlend = {NEXUS_BlendFactor_eOne, NEXUS_BlendFactor_eOne, false,
                NEXUS_BlendFactor_eZero, NEXUS_BlendFactor_eZero, false, NEXUS_BlendFactor_eZero};
            fillSettings.colorOp = NEXUS_FillOp_eUseBlendEquation;
            fillSettings.colorBlend = g_colorBlend;
            fillSettings.alphaOp = NEXUS_FillOp_eUseBlendEquation;
            fillSettings.alphaBlend = g_alphaBlend;
            fillSettings.color = 0xD0FFFFFF;
        }
        rc = NEXUS_Graphics2D_Fill(bgui_blitter(pContext->gui), &fillSettings);
        BDBG_ASSERT(!rc);

        bgui_checkpoint(pContext->gui);
    
        if (pContext->font) {
            struct bfont_surface_desc desc;
            bfont_get_surface_desc(surface, &desc);
            bfont_draw_aligned_text(&desc, pContext->font, &fillSettings.rect, item->name, -1, focused?0xFF333333:0xFFCCCCCC, bfont_valign_center, bfont_halign_center);
            NEXUS_Surface_Flush(surface);
        }
        
        if (focused) {
            printf("'%s' will launch '%s'\n", item->name, item->cmdline);
        }
    }
    
    bgui_submit(pContext->gui);
}

/**
server masks user input, hides itself, launched client, waits for it to exit, then unmasks
**/
static void launch_client(struct appcontext *pContext)
{
    unsigned pid;
    struct desktop_item *item;
    unsigned i;
    int rc;
    bool background;

    for (item=BLST_Q_FIRST(&pContext->launch),i=0;item && i<pContext->focused_button;item=BLST_Q_NEXT(item,link),i++);
    if (!item) return;
    
    background = strstr(item->cmdline, "&");
    if (!background) {
        binput_set_mask(pContext->input, 0); /* nothing */
        NEXUS_SurfaceClient_Clear(bgui_surface_client(pContext->gui));
    }
    
    pid = fork();
    if (!pid) {
        rc = system(item->cmdline);
        if (rc == -1) {
            printf("unable to launch %s: %d\n", item->cmdline, errno);
        }
        exit(0);
    }
    
    if (!background) {
        waitpid(pid, 0, 0);
        binput_set_mask(pContext->input, 0xFFFFFFFF); /* everything */
    }
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform (needs input_router, picture_decoder, playback and simple_decoder)!\n");
    return 0;
}
#endif
