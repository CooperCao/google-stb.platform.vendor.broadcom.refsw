/******************************************************************************
 *    (c)2011-2014 Broadcom Corporation
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
#include "nexus_types.h"
#include "nexus_platform.h"
#include "nxclient.h"
#include "nxserverlib.h"
#include "namevalue.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

/* print_capabilities */
#if NEXUS_HAS_VIDEO_DECODER
#include "nexus_video_decoder.h"
#endif
#include "nexus_display.h"

BDBG_MODULE(nxserver);

#if !defined(NEXUS_HAS_STREAM_MUX)
#undef NEXUS_NUM_VIDEO_ENCODERS
#endif
#define APP_MAX_CLIENTS 20
#define MB (1024*1024)

static struct {
    BKNI_MutexHandle lock;
    nxserver_t server;
    unsigned refcnt;
    struct {
        nxclient_t client;
        NxClient_JoinSettings joinSettings;
    } clients[APP_MAX_CLIENTS]; /* index provides id */
} g_app;

static int client_connect(nxclient_t client, const NxClient_JoinSettings *pJoinSettings, NEXUS_ClientSettings *pClientSettings)
{
    unsigned i;
    BSTD_UNUSED(pClientSettings);
    /* server app has opportunity to reject client altogether, or modify pClientSettings */
    for (i=0;i<APP_MAX_CLIENTS;i++) {
        if (!g_app.clients[i].client) {
            g_app.clients[i].client = client;
            g_app.clients[i].joinSettings = *pJoinSettings;
            break;
        }
    }
    return 0;
}

static void client_disconnect(nxclient_t client, const NxClient_JoinSettings *pJoinSettings)
{
    unsigned i;
    BSTD_UNUSED(pJoinSettings);
    for (i=0;i<APP_MAX_CLIENTS;i++) {
        if (g_app.clients[i].client == client) {
            g_app.clients[i].client = NULL;
            break;
        }
    }
}

static void print_prompt_usage(void)
{
    printf(
    "nxserver commands:\n"
    "?|help - print this help\n"
    "q      - quit\n"
    "heaps  - print heap status\n"
    "mem [HEAP] - print all device memory allocation [for a HEAP index]\n"
    "kni    - print all BKNI memory allocation\n"
    "clients - print list of clients\n"
    "cap     - print capabilities\n"
    "kill CLIENTID\n"
    "focus CLIENTID\n"
    );
}

static void print_capabilities(void)
{
#if NEXUS_HAS_VIDEO_DECODER && NEXUS_HAS_DISPLAY
    NEXUS_VideoDecoderCapabilities vcap;
    NEXUS_DisplayCapabilities dcap;
    unsigned i;

    NEXUS_GetVideoDecoderCapabilities(&vcap);
    for (i=0;i<NEXUS_MAX_VIDEO_DECODERS;i++) {
        if (!vcap.memory[i].used) continue;
        printf("videoDecoder[%d]: maxFormat %s, colorDepth %d, mosaic %d %dx%d, avc51 %d\n",
            i, lookup_name(g_videoFormatStrs, vcap.memory[i].maxFormat), vcap.memory[i].colorDepth,
            vcap.memory[i].mosaic.maxNumber, vcap.memory[i].mosaic.maxWidth, vcap.memory[i].mosaic.maxHeight,
            vcap.memory[i].avc51Supported);
    }
    NEXUS_GetDisplayCapabilities(&dcap);
    for (i=0;i<NEXUS_MAX_DISPLAYS;i++) {
        if (!dcap.display[i].numVideoWindows) continue;
        printf("display[%d]: %d video windows, graphics %dx%d\n",
            i, dcap.display[i].numVideoWindows,
            dcap.display[i].graphics.width, dcap.display[i].graphics.height);
    }
#endif
}

static bool g_exit = false;
static void sig_handler(int signum)
{
    if (signum == SIGTERM) {
        g_exit = true;
    }
}

static void nxserver_prompt(void)
{
    int rc;
    unsigned i;
    while (1) {
        char buf[64];
        printf("nxserver>");
        fflush(stdout);
        fgets(buf, sizeof(buf), stdin);
        buf[strlen(buf)-1] = 0;
        if (!strcmp(buf, "?") || !strcmp(buf, "help")) {
            print_prompt_usage();
        }
        else if (!strcmp(buf, "q")) {
            break;
        }
        else if (!strcmp(buf, "heaps")) {
            NEXUS_Memory_PrintHeaps();
            BKNI_Sleep(10); /* increase change that logger will print */
        }
        else if (!strncmp(buf, "mem", 3)) {
            NEXUS_PlatformConfiguration platformConfig;
            NEXUS_Platform_GetConfiguration(&platformConfig);
            if (buf[3] == ' ' && buf[4]) {
                i = atoi(&buf[4]);
            }
            else {
                i = NEXUS_MAX_HEAPS;
            }
            if (i < NEXUS_MAX_HEAPS && platformConfig.heap[i]) {
                NEXUS_Heap_Dump(platformConfig.heap[i]);
            }
            else {
                for (i=0;i<NEXUS_MAX_HEAPS;i++) {
                    if (platformConfig.heap[i]) {
                        NEXUS_Heap_Dump(platformConfig.heap[i]);
                    }
                }
            }
        }
        else if (!strcmp(buf, "kni")) {
            BKNI_DumpMallocs();
        }
        else if (!strcmp(buf, "clients")) {
            for (i=0;i<APP_MAX_CLIENTS;i++) {
                /* g_app.clients[] is populated by connect/disconnect callbacks.
                by storing in external array, the index number persists. internally, the linked list
                position is dynamic. */
                if (g_app.clients[i].client) {
                    printf("%3d: %s\n", i+1, g_app.clients[i].joinSettings.name);
                }
            }
        }
        else if (strstr(buf, "kill ") && buf[5]) {
            unsigned i = atoi(&buf[5])-1;
            if (i < APP_MAX_CLIENTS && g_app.clients[i].client) {
                BKNI_AcquireMutex(g_app.lock);
                /* must call nxserver_ipc so it can unwind first. */
                nxserver_ipc_close_client(g_app.clients[i].client);
                BKNI_ReleaseMutex(g_app.lock);
            }
        }
        else if (strstr(buf, "focus ") && buf[6]) {
            unsigned i = atoi(&buf[6])-1;
            if (i < APP_MAX_CLIENTS && g_app.clients[i].client) {
                /* the definition of "focus" is variable. this is one impl. */
                NxClient_ConnectList list;
                struct nxclient_status status;
                BKNI_AcquireMutex(g_app.lock);
                nxclient_get_status(g_app.clients[i].client, &status);
                rc = NxClient_P_Config_GetConnectList(g_app.clients[i].client, status.handle, &list);
                /* only refresh first connect */
                if (list.connectId[0]) {
                    NxClient_P_RefreshConnect(g_app.clients[i].client, list.connectId[0]);
                }
                /* focus first InputClient, blur all others */
                nxserver_p_focus_input_client(g_app.clients[i].client);
                /* bring first SurfaceClient to top of zorder */
                nxserver_p_focus_surface_client(g_app.clients[i].client);
                BKNI_ReleaseMutex(g_app.lock);
            }
        }
        else if (strstr(buf, "alpha ") && buf[6]) {
            char alpha[8];
            if (sscanf(&buf[6], "%u %s", &i, alpha) == 2) {
                i -= 1;
                if (i < APP_MAX_CLIENTS && g_app.clients[i].client) {
                    nxserverlib_set_server_alpha(g_app.clients[i].client, strtoul(alpha, NULL, 0));
                }
            }
        }
        else if (!strcmp(buf, "cap")) {
            print_capabilities();
        }
        else if (!buf[0]) {
        }
        else {
            print_prompt_usage();
        }
    }
}

/* If you want to embed nxserverlib in your own server application,
we do not recommend you call nxserver_init. Instead, copy this code and
customize nxserver_settings and platform settings without being limited by
the cmdline interface. */
nxserver_t nxserver_init(int argc, char **argv, bool blocking)
{
    NEXUS_Error rc;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_MemoryConfigurationSettings memConfigSettings;
    NEXUS_PlatformCapabilities platformCap;
    struct nxserver_settings settings;
    struct nxserver_cmdline_settings cmdline_settings;

    /* Allow nxserver_init to be called twice if second time from NxClient_Join. This allows a local app can pass argv around NxClient_Join.
    Only allow g_app.refcnt of 0, 1 or 2. nxclient_local has its own refcnt for nxclient. */
    if (!argv && g_app.refcnt == 1) {
        g_app.refcnt++;
        return g_app.server;
    }
    if (g_app.server) {
        fprintf(stderr,"nxserver already initialized\n");
        return NULL;
    }

    nxserver_get_default_settings(&settings);
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    NEXUS_GetDefaultMemoryConfigurationSettings(&memConfigSettings);
    NEXUS_GetPlatformCapabilities(&platformCap);

    /* display[1] will always be either SD or transcode */
    if (!platformCap.display[1].supported || platformCap.display[1].encoder) {
        unsigned i;
        for (i = 0 ; i < NXCLIENT_MAX_SESSIONS ; i++)
            settings.session[i].output.sd = false;
    }
    /* display[0] will always be either HD, or system is headless */
    if (!platformCap.display[0].supported || platformCap.display[0].encoder) {
        unsigned i;
        for (i = 0 ; i < NXCLIENT_MAX_SESSIONS ; i++)
            settings.session[i].output.hd = false;
    }

    /* optional: modify settings by cmdline options */
    rc = nxserver_parse_cmdline(argc, argv, &settings, &cmdline_settings);
    if (rc) return NULL;

    rc = nxserver_modify_platform_settings(&settings, &cmdline_settings, &platformSettings, &memConfigSettings);
    if (rc) return NULL;

    rc = NEXUS_Platform_MemConfigInit(&platformSettings, &memConfigSettings);
    if (rc) return NULL;

    BKNI_CreateMutex(&g_app.lock);
    settings.lock = g_app.lock;
    nxserver_set_client_heaps(&settings, &platformSettings);
    settings.client.connect = client_connect;
    settings.client.disconnect = client_disconnect;
    settings.memConfigSettings = memConfigSettings;
    g_app.server = nxserverlib_init(&settings);
    if (!g_app.server) return NULL;

    rc = nxserver_ipc_init(g_app.server, g_app.lock);
    if (rc) {BERR_TRACE(rc); return NULL;}

    if (!settings.prompt) {
        signal(SIGTERM, sig_handler);
    }

    if (settings.prompt) {
        nxserver_prompt();
    }
    else if (settings.timeout) {
        printf("nxserver is running for %d second(s).\n", settings.timeout);
        while (settings.timeout--) {
            BKNI_Sleep(1000);
            if (g_exit) break;
        }
    }
    else if (blocking) {
        printf("nxserver is running.\n");
        while (1) {
            BKNI_Sleep(1000);
            if (g_exit) break;
        }
    }
    /* else return immediately. */
    g_app.refcnt++;
    return g_app.server;
}

void nxserver_uninit(nxserver_t server)
{
    BDBG_ASSERT(server == g_app.server);
    if (--g_app.refcnt) return;

    nxserver_ipc_uninit();
    nxserverlib_uninit(server);
    BKNI_DestroyMutex(g_app.lock);
    NEXUS_Platform_Uninit();
}
