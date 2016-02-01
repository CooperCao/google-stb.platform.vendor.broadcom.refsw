/******************************************************************************
 *    (c)2013 Broadcom Corporation
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
 *****************************************************************************/
#include "nexus_platform_client.h"
#include "nexus_core_utils.h"
#include "nxclient.h"
#include "bgui.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bfont.h"
#include "binput.h"
#include "namevalue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#ifndef NXSERVER_PMLIB_SUPPORT
#include "pmlib.h"
#endif

BDBG_MODULE(standby);


struct appcontext {
    bgui_t gui;
    binput_t input;
    unsigned total_buttons, focused_button;
    pthread_t standby_thread_id, remote_thread_id, cmdline_thread_id;
    bfont_t font;
    bool done;
    bool timer;
    unsigned timeout;
    bool prompt;
    BKNI_EventHandle event, wakeupEvent;
#ifndef NXSERVER_PMLIB_SUPPORT
    void *pm_ctx;
#endif
    bool standbyTransition;
    NEXUS_PlatformStandbyMode mode;
} g_context;

static const char *g_standby_state[3] = {
    "ACTIVE STANDBY (S1)",
    "PASSIVE STANDBY (S2)",
    "DEEP SLEEP (S3)"};

static void render_ui(struct appcontext *pContext)
{
    NxClient_StandbyStatus standbyStatus;
    NEXUS_Graphics2DFillSettings fillSettings;
    int rc;
    unsigned i;

    NxClient_GetStandbyStatus(&standbyStatus);
    if(standbyStatus.settings.mode != NEXUS_PlatformStandbyMode_eOn)
        return;

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = bgui_surface(pContext->gui);
    fillSettings.color = 0;
    rc = NEXUS_Graphics2D_Fill(bgui_blitter(pContext->gui), &fillSettings);
    BDBG_ASSERT(!rc);

    for (i=0;i<pContext->total_buttons;i++) {
        bool focused = i == pContext->focused_button;

        fillSettings.rect.x = 0;
        fillSettings.rect.y = i*50;
        fillSettings.rect.width = 250;
        fillSettings.rect.height = 50;
        fillSettings.color = (i == pContext->focused_button) ? 0xFF00FF00 : 0xFF008888;
        rc = NEXUS_Graphics2D_Fill(bgui_blitter(pContext->gui), &fillSettings);
        BDBG_ASSERT(!rc);

        bgui_checkpoint(pContext->gui);

        if (pContext->font) {
            struct bfont_surface_desc desc;
            bfont_get_surface_desc(bgui_surface(pContext->gui), &desc);
            bfont_draw_aligned_text(&desc, pContext->font, &fillSettings.rect, g_standby_state[i], -1, focused?0xFF333333:0xFFCCCCCC, bfont_valign_center, bfont_halign_center);
            NEXUS_Surface_Flush(bgui_surface(pContext->gui));
        }
    }

    bgui_submit(pContext->gui);
}

static void print_wakeup(struct appcontext *pContext)
{
    NxClient_StandbyStatus standbyStatus;
    NEXUS_Error rc;

    if(pContext->focused_button == 0)
        return;

    rc = NxClient_GetStandbyStatus(&standbyStatus);
    if (rc) {BERR_TRACE(rc); return;}

    BDBG_WRN(("Wake up Status:\n"
           "IR      : %d\n"
           "UHF     : %d\n"
           "XPT     : %d\n"
           "CEC     : %d\n"
           "GPIO    : %d\n"
           "Timeout : %d\n"
           "\n",
           standbyStatus.status.wakeupStatus.ir,
           standbyStatus.status.wakeupStatus.uhf,
           standbyStatus.status.wakeupStatus.transport,
           standbyStatus.status.wakeupStatus.cec,
           standbyStatus.status.wakeupStatus.gpio,
           standbyStatus.status.wakeupStatus.timeout));
}

static void standby(struct appcontext *pContext)
{
    NxClient_StandbySettings standbySettings;
    NEXUS_Error rc;

    NxClient_GetDefaultStandbySettings(&standbySettings);
    standbySettings.settings.mode = pContext->mode;
    standbySettings.settings.wakeupSettings.ir = true;
    standbySettings.settings.wakeupSettings.timeout = pContext->timer?pContext->timeout:0;
    rc = NxClient_SetStandbySettings(&standbySettings);

#ifndef NXSERVER_PMLIB_SUPPORT
    while(!pContext->done) {
        NxClient_StandbyStatus standbyStatus;
        rc = NxClient_GetStandbyStatus(&standbyStatus);

        if(standbyStatus.standbyTransition)
            break;
    }
    if(standbySettings.settings.mode == NEXUS_PlatformStandbyMode_ePassive) {
        brcm_pm_suspend(pContext->pm_ctx, BRCM_PM_STANDBY);
    } else if(standbySettings.settings.mode == NEXUS_PlatformStandbyMode_eDeepSleep) {
        brcm_pm_suspend(pContext->pm_ctx, BRCM_PM_SUSPEND);
    } else
#endif
    if(standbySettings.settings.mode == NEXUS_PlatformStandbyMode_eActive) {
        BKNI_WaitForEvent(pContext->wakeupEvent, pContext->timer?(int)pContext->timeout*1000:BKNI_INFINITE);
    }

    pContext->standbyTransition = true;
}

static void resume(struct appcontext *pContext)
{
    NxClient_StandbySettings standbySettings;
    NEXUS_Error rc;

    BDBG_ASSERT(pContext->mode == NEXUS_PlatformStandbyMode_eOn);

    NxClient_GetDefaultStandbySettings(&standbySettings);
    standbySettings.settings.mode = pContext->mode;
    rc = NxClient_SetStandbySettings(&standbySettings);
    if (rc) BERR_TRACE(rc);
}

static void *standby_monitor(void *context)
{
    struct appcontext *pContext = context;
    NEXUS_Error rc;
    NxClient_StandbyStatus standbyStatus, prevStatus;

    BSTD_UNUSED(context);

    NxClient_GetStandbyStatus(&standbyStatus);
    prevStatus = standbyStatus;

    while(!pContext->done) {
        rc = NxClient_GetStandbyStatus(&standbyStatus);
#ifdef NXSERVER_PMLIB_SUPPORT
        if(standbyStatus.standbyTransition && pContext->standbyTransition) {
#else
        if(pContext->standbyTransition) {
#endif
            pContext->standbyTransition = false;
            pContext->mode = NEXUS_PlatformStandbyMode_eOn;
            BKNI_SetEvent(pContext->event);
        }

        if(standbyStatus.settings.mode != prevStatus.settings.mode) {
            printf("'standby' acknowledges standby state: %s\n", lookup_name(g_platformStandbyModeStrs, standbyStatus.settings.mode));
            printf("NOTE: Not all clients may acknowledge standby. Server may need to timeout.\n");
            NxClient_AcknowledgeStandby(true);
        }
        prevStatus = standbyStatus;
        BKNI_Sleep(100);
    }

    return NULL;
}

static void *remote_key_monitor(void *context)
{
    struct appcontext *pContext = context;

    while(!pContext->done) {
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
            pContext->timer = false;
            pContext->mode = pContext->focused_button+1;
            BKNI_SetEvent(pContext->event);
            break;
        case b_remote_key_stop:
        case b_remote_key_clear:
            pContext->done = true;
            BKNI_SetEvent(pContext->event);
            break;
        case b_remote_key_power:
            pContext->mode = NEXUS_PlatformStandbyMode_eOn;
            /* Set event only for S1 wakeup */
            if(!pContext->focused_button) BKNI_SetEvent(pContext->wakeupEvent);
            break;
        default:
            break;
        }
    }

    return NULL;
}

static void *cmdline_monitor(void *context)
{
    struct appcontext *pContext = context;
    int retval = 1;

    while(!pContext->done) {
        fd_set rfds;
        struct timeval timeout;
        char buffer[256];
        char *buf;

        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 250000;

        if(retval) {
            fflush(stdout); printf("standby>"); fflush(stdout);
        }

        retval = select(1, &rfds, NULL, NULL, &timeout);
        if(retval == -1) {
            printf("Stdin Error\n");
            return NULL;
        } else if (!retval) {
            continue;
        }


        fgets(buffer, 256, stdin);
        if (feof(stdin)) break;
        buffer[strlen(buffer)-1] = 0; /* chop off \n */

        buf = strtok(buffer, ";");
        if (!buf) continue;

        do {
            if(!strncmp(buf, "s", 1)) {
                buf++;
                if (!strncmp(buf, "0", 1)) {
                    pContext->mode = NEXUS_PlatformStandbyMode_eOn;
                } else if (!strncmp(buf, "1", 1)) {
                    pContext->mode = NEXUS_PlatformStandbyMode_eActive;
                } else if (!strncmp(buf, "2", 1)) {
                    pContext->mode = NEXUS_PlatformStandbyMode_ePassive;
                } else if (!strncmp(buf, "3", 1)) {
                    pContext->mode = NEXUS_PlatformStandbyMode_eDeepSleep;
                } else {
                    printf("Unknown standby mode\n");
                    break;
                }

                if(pContext->mode == NEXUS_PlatformStandbyMode_eOn) {
                    /* Set event only for S1 wakeup */
                    if(!pContext->focused_button) BKNI_SetEvent(pContext->wakeupEvent);
                } else {
                    pContext->focused_button = pContext->mode-1;
                    BDBG_ASSERT(pContext->focused_button<pContext->total_buttons);
                    render_ui(pContext);
                    pContext->timer = true;
                    BKNI_SetEvent(pContext->event);
                }
            } else if (!strncmp(buf, "q", 1)) {
                pContext->done = true;
                BKNI_SetEvent(pContext->event);
            } else {
                printf("Unknown command\n");
            }
        } while ((buf = strtok(NULL, ";")));
    }

    return NULL;
}

static void print_usage(void)
{
    printf(
    "Usage: nexus.client standby OPTIONS\n"
    "\n"
    "OPTIONS:\n"
    "  --help or -h for help\n"
    "  -timeout X       timeout in seconds\n"
    "  -prompt  off     disable user prompt.\n"
    );
}

int main(int argc, const char **argv)
{
    NEXUS_Error rc;
    struct appcontext *pContext = &g_context;
    NxClient_JoinSettings joinSettings;
    int curarg = 1;

    BKNI_Memset(pContext, 0, sizeof(struct appcontext));
    pContext->timeout = 5;
    pContext->prompt = true;

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-timeout") && argc>curarg+1) {
            pContext->timeout = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "-prompt") && argc>curarg+1) {
            if (!strcmp(argv[++curarg], "off")) {
                pContext->prompt = false;
            }
        }
        curarg++;
    }

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    joinSettings.standbyClient = true;
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    /* Server maybe already be in standby. Wakeup first */
    pContext->mode= NEXUS_PlatformStandbyMode_eOn;
    resume(pContext);

    pContext->total_buttons = NEXUS_PlatformStandbyMode_eMax-1;
    pContext->done = false;

    pContext->gui = bgui_create(250, 150);
    pContext->input = binput_open(NULL);

    {
        NEXUS_SurfaceComposition comp;
        NxClient_GetSurfaceClientComposition(bgui_surface_client_id(pContext->gui), &comp);
        comp.position.x = 50;
        comp.position.y = 50;
        comp.position.width = 250;
        comp.position.height = 150;
        comp.zorder = 100; /* always on top */
        NxClient_SetSurfaceClientComposition(bgui_surface_client_id(pContext->gui), &comp);
    }

    BKNI_CreateEvent(&pContext->event);
    BKNI_CreateEvent(&pContext->wakeupEvent);

    pContext->font = bfont_open("nxclient/arial_18_aa.bwin_font");

#ifndef NXSERVER_PMLIB_SUPPORT
    pContext->pm_ctx = brcm_pm_init();
#endif

    render_ui(pContext);

    rc = pthread_create(&pContext->standby_thread_id, NULL, standby_monitor, pContext);
    if (rc) return -1;

    rc = pthread_create(&pContext->remote_thread_id, NULL, remote_key_monitor, pContext);
    if (rc) return -1;

    if(pContext->prompt) {
        rc = pthread_create(&pContext->cmdline_thread_id, NULL, cmdline_monitor, pContext);
        if (rc) return -1;
    }

    while(1) {
        BKNI_WaitForEvent(pContext->event, BKNI_INFINITE);

        if(pContext->done)
            break;

        if(pContext->mode == NEXUS_PlatformStandbyMode_eOn) {
            resume(pContext);
            print_wakeup(pContext);
        } else {
            standby(pContext);
        }
    }

    if(pContext->prompt) {
        pthread_join(pContext->cmdline_thread_id, NULL);
    }
    pthread_join(pContext->remote_thread_id, NULL);
    pthread_join(pContext->standby_thread_id, NULL);

#ifndef NXSERVER_PMLIB_SUPPORT
    brcm_pm_close(pContext->pm_ctx);
#endif

    if (pContext->font) {
        bfont_close(pContext->font);
    }
    BKNI_DestroyEvent(pContext->event);
    BKNI_DestroyEvent(pContext->wakeupEvent);
    bgui_destroy(pContext->gui);
    binput_close(pContext->input);
    NxClient_Uninit();

    return 0;
}
