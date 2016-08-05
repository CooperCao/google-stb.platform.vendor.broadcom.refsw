/***************************************************************************
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
 **************************************************************************/

/**
A simple, modal client launcher
The desktop reads a "desktop.cfg" file which determines what clients it can launch.
User can select and launch a client with the IR remote.
Control returns to the desktop when the client exits.
**/
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "nexus_platform.h"
#include "nexus_ir_input.h"
#if NEXUS_HAS_FRONTEND
#include "nexus_frontend_qam.h"
#endif
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "bsu_picdecoder.h"
#include "bsu_bfont.h"
#include "bsu.h"
#include "bsu_filesys.h"
#include "bsu-api.h"
#include "bsu-api2.h"
#include "nexus_parser_band.h"
#include "bkni_event_group.h"

BDBG_MODULE(desktop);

#define DEFAULT_SD_VIDEOFORMAT NEXUS_VideoFormat_eNtsc
#define DEFAULT_HD_VIDEOFORMAT NEXUS_VideoFormat_e720p /*NEXUS_VideoFormat_e1080i*/

#ifdef MIPS_SDE
    #define do_commands(buffer) cfe_docommands(buffer)
#else
    #define do_commands(buffer) bolt_docommands(buffer)
#endif

/* UEI Remote Control */
#define UP_BUTTON 0x9034
#define DOWN_BUTTON 0x8035
#define RIGHT_BUTTON 0x6037
#define LEFT_BUTTON 0x7036
#define EXIT_BUTTON 0xd012
#define SELECT_BUTTON 0xe011

/* QAM */
#define FREQ 765
#define MODE NEXUS_FrontendQamMode_e64;

static void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

struct appcontext {
    NEXUS_Graphics2DHandle gfx;
    BKNI_EventHandle checkpointEvent, displayedEvent, inputEvent;
    NEXUS_SurfaceHandle surface;
    NEXUS_SurfaceHandle background;
#define MAX_ITEMS 14
    struct {
        char name[256];
        char cmdline[256];
        /* TODO: add image */
    } launch[MAX_ITEMS];
    unsigned button_pressed;
    unsigned total_buttons, focused_button;
    unsigned secondary_total_buttons, secondary_focused_button;
    bool show_secondary;
    bfont_t font;
};

static void render_ui(struct appcontext *pContext);
static void render_message(struct appcontext *pContext, char *text);
/*static*/ fileio_ctx_t *fsctx_usb0;

static void irCallback(void *pParam, int iParam)
{
    BSTD_UNUSED(iParam);
    BKNI_SetEvent((BKNI_EventHandle)pParam);
}

NEXUS_FrontendHandle frontend;

#ifdef BSU_QAM_SUPPORT
static void lock_changed_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void async_status_ready_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}
#endif

int bsu_gui(void)
{
    NEXUS_DisplayHandle display0 = NULL;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_IrInputSettings irSettings;
    NEXUS_IrInputHandle irHandle;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_Graphics2DSettings gfxSettings;
    NEXUS_Error rc;
    struct appcontext *pContext;
    FILE *launchfile;
    const char *launchfilename = "desktop.cfg";
#if NEXUS_HAS_PICTURE_DECODER
    const char *background = "desktop_background3.png";
#endif
    const char *fontname = "arial_18_aa.bwin_font";
    bool done = false;
    bool end_of_file = false;
    BERR_Code errCode;
    BKNI_EventGroupHandle eventGroup;
    BKNI_EventHandle wakeEvent;
#ifdef BSU_QAM_SUPPORT
    BKNI_EventHandle lockChangedEvent;
#endif
    unsigned wakeEventCount;

    errCode = BKNI_CreateEventGroup(&eventGroup);
    if ( errCode ) return BERR_TRACE(errCode);

    pContext = malloc(sizeof(struct appcontext));
    memset(pContext, 0, sizeof(*pContext));

    if (bsu_fs_init("fat",&fsctx_usb0,"usbdisk0") == 0) {
        printf("usb initialized with fat file system\n");
    }

    launchfile = bsu_fs_open(fsctx_usb0, (char *)launchfilename, FILE_MODE_READ);
    if (!launchfile) {
        printf("error from bsu_fs_open\n");
        return 1;
    }
    while (!end_of_file && pContext->total_buttons<MAX_ITEMS) {
        char buf[256];
        char *s;

        {
            int cnt = 0;
            unsigned char ch;
            memset(buf,0,sizeof(buf));
            while (cnt < 256) {
                if (!bsu_fs_read(fsctx_usb0, &ch, 1, 1, launchfile)) {
                    buf[cnt] = '\n';
                    end_of_file = true;
                    break;
                }
                buf[cnt++] = ch;
                if (ch == '\n') {
                    break;
                }
            }
        }

        s = buf;
        s += strspn(s, " \t");
        if (*s && *s != '#') {
            char *find;
            find = strchr(s, '\n');
            if (find) *find = 0;
            find = strchr(s, ':');
            if (find) {
                strncpy(pContext->launch[pContext->total_buttons].name, s, find-s);
                strcpy(pContext->launch[pContext->total_buttons].cmdline, ++find);
                pContext->total_buttons++;
            }
        }
    }
    pContext->secondary_total_buttons=3;

    if (bsu_fs_close(fsctx_usb0, launchfile) < 0) {
        printf("error from bsu_fs_close\n");
    }

    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* HD display */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = DEFAULT_HD_VIDEOFORMAT;
    display0 = NEXUS_Display_Open(0, &displaySettings);
    #if NEXUS_NUM_COMPONENT_OUTPUTS
        NEXUS_Display_AddOutput(display0, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
    #endif
    #if NEXUS_NUM_HDMI_OUTPUTS
        NEXUS_Display_AddOutput(display0, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    #endif

    BKNI_CreateEvent(&pContext->checkpointEvent);
    BKNI_CreateEvent(&pContext->displayedEvent);
    BKNI_CreateEvent(&pContext->inputEvent);

    errCode = BKNI_AddEventGroup(eventGroup, pContext->inputEvent);
    if ( errCode ) return BERR_TRACE(errCode);

    pContext->gfx = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, NULL);
    NEXUS_Graphics2D_GetSettings(pContext->gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = pContext->checkpointEvent;
    rc = NEXUS_Graphics2D_SetSettings(pContext->gfx, &gfxSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = 1280;
    createSettings.height = 720;
    createSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    pContext->surface = NEXUS_Surface_Create(&createSettings);

    pContext->font = bfont_open(fontname);
    if (!pContext->font) {
        BDBG_WRN(("unable to load font %s", fontname));
    }

#if NEXUS_HAS_PICTURE_DECODER
    if (background) {
        picdecoder_t handle;
        handle = picdecoder_open();
        if (handle) {
            pContext->background = NEXUS_Surface_Create(&createSettings);
            if (picdecoder_decode(handle, background, pContext->background, picdecoder_aspect_ratio_full)) {
                NEXUS_Surface_Destroy(pContext->background);
                pContext->background = NULL;
            }
            picdecoder_close(handle);
        }
    }
#endif

    render_ui(pContext);

    NEXUS_Display_GetGraphicsSettings(display0, &graphicsSettings);
    graphicsSettings.enabled = true;
    NEXUS_Display_SetGraphicsSettings(display0, &graphicsSettings);
    NEXUS_Display_SetGraphicsFramebuffer(display0, pContext->surface);

    NEXUS_IrInput_GetDefaultSettings(&irSettings);
    irSettings.mode = NEXUS_IrInputMode_eRemoteA;
    irSettings.dataReady.callback = irCallback;
    irSettings.dataReady.context = pContext->inputEvent;
    irHandle = NEXUS_IrInput_Open(0, &irSettings);

    #ifdef BSU_QAM_SUPPORT
    {
        NEXUS_FrontendUserParameters userParams;
        BKNI_EventHandle statusEvent;
        NEXUS_FrontendQamSettings qamSettings;
        NEXUS_ParserBand parserBand;
        NEXUS_ParserBandSettings parserBandSettings;
        NEXUS_FrontendAcquireSettings settings;
        NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
        settings.capabilities.qam = true;
        frontend = NEXUS_Frontend_Acquire(&settings);
        if (!frontend) {
            BDBG_ERR(("Unable to find QAM-capable frontend"));
            return -1;
        }

        BKNI_CreateEvent(&statusEvent);
        BKNI_CreateEvent(&lockChangedEvent);

        errCode = BKNI_AddEventGroup(eventGroup, lockChangedEvent);
        if ( errCode ) return BERR_TRACE(errCode);

        NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
        qamSettings.frequency = FREQ * 1000000;
        qamSettings.mode = MODE;
        qamSettings.symbolRate = 5056900;
        qamSettings.annex = NEXUS_FrontendQamAnnex_eB;
        qamSettings.bandwidth = NEXUS_FrontendQamBandwidth_e6Mhz;
        qamSettings.lockCallback.callback = lock_changed_callback;
        qamSettings.lockCallback.context = lockChangedEvent;
        qamSettings.asyncStatusReadyCallback.callback = async_status_ready_callback;
        qamSettings.asyncStatusReadyCallback.context = statusEvent;

        NEXUS_Frontend_GetUserParameters(frontend, &userParams);

        /* Map a parser band to the demod's input band. */
        parserBand = NEXUS_ParserBand_e0;
        NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
        if (userParams.isMtsif) {
            parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
            parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(frontend); /* NEXUS_Frontend_TuneXyz() will connect this frontend to this parser band */
        }
        else {
            parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
            parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
        }
        parserBandSettings.transportType = NEXUS_TransportType_eTs;
        NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

        BKNI_ResetEvent(lockChangedEvent);

        BDBG_WRN(("tuning %d MHz...", FREQ));
        rc = NEXUS_Frontend_TuneQam(frontend, &qamSettings);
        if(rc) return rc;
    }
    #endif

    while (!done) {
        BKNI_WaitForGroup(eventGroup, BKNI_INFINITE, &wakeEvent, 1, &wakeEventCount);
        if (wakeEvent == pContext->inputEvent) {
            size_t numEvents = 2;
            NEXUS_Error rc = 0;
            bool overflow;
            while ((numEvents > 1) && !rc) {
                NEXUS_IrInputEvent irEvent;
                rc = NEXUS_IrInput_GetEvents(irHandle,&irEvent,1,&numEvents,&overflow);

                if (numEvents) {
                    printf("irCallback: rc: %d, code: %08x, repeat: %s, numEvents=%d\n", rc, irEvent.code, irEvent.repeat ? "true" : "false", numEvents);

                    pContext->button_pressed = irEvent.code;
                    switch (irEvent.code) {
                        case UP_BUTTON:
                            if (pContext->show_secondary) {
                                if (pContext->secondary_focused_button) {
                                    pContext->secondary_focused_button--;
                                }
                                else {
                                    pContext->secondary_focused_button = pContext->secondary_total_buttons-1;
                                }
                            } else {
                                if (pContext->focused_button) {
                                    pContext->focused_button--;
                                }
                                else {
                                    pContext->focused_button = pContext->total_buttons-1;
                                }
                            }
                            render_ui(pContext);
                            break;

                        case DOWN_BUTTON:
                            if (pContext->show_secondary) {
                                if (++pContext->secondary_focused_button == pContext->secondary_total_buttons) {
                                    pContext->secondary_focused_button = 0;
                                }
                            } else {
                                if (++pContext->focused_button == pContext->total_buttons) {
                                    pContext->focused_button = 0;
                                }
                            }
                            render_ui(pContext);
                            break;

                        case RIGHT_BUTTON:
                            pContext->show_secondary = true;
                            pContext->secondary_focused_button = 0; /* Start off at the top */
                            render_ui(pContext);
                            break;

                        case LEFT_BUTTON:
                            pContext->show_secondary = false;
                            render_ui(pContext);
                            break;

                        case EXIT_BUTTON:
                            pContext->show_secondary = false;
                            render_ui(pContext);
                            break;

                        case SELECT_BUTTON:
                            /* If secondary was not shown, treat this like pressing "right button" */
                            {
                                char buffer[256];
                                int status;

                                switch (pContext->focused_button) {
                                case 0:

                                    if (pContext->show_secondary == false) {
                                        pContext->show_secondary = true;
                                        pContext->secondary_focused_button = 0; /* Start off at the top */
                                        render_ui(pContext);
                                    } else {
                                        switch (pContext->secondary_focused_button) {
                                            case 0:
                                                strcpy(buffer, "flash -noheader stb-irva-09:/agin/src/git/refsw/myrepo/CFE/build/7584/cfe.bin flash0.cfe");
                                                render_message(pContext, "Updating CFE from stb-irva-09:/agin/src/git/refsw/myrepo/CFE/build/7584/cfe.bin... Please wait...");
                                                status = do_commands(buffer);
                                                if (status != CMD_ERR_BLANK) {
                                                    printf("bsu command status = %d\n", status);
                                                }
                                                pContext->show_secondary = false;
                                                render_ui(pContext);
                                                break;
                                            default:
                                                render_message(pContext, "not currently supported");
                                        }
                                    }
                                    break;
                                case 1:
                                    if (pContext->show_secondary == false) {
                                        pContext->show_secondary = true;
                                        pContext->secondary_focused_button = 0; /* Start off at the top */
                                        render_ui(pContext);
                                    } else {
                                        switch (pContext->secondary_focused_button) {
                                            case 0:
                                                strcpy(buffer, "flash -noheader stb-irva-01:33-3.3/vmlinuz-7584a0 flash0.kernel");
                                                render_message(pContext, "Updating linux from stb-irva-01:33-3.3/vmlinuz-7584a0... Please wait...");
                                                status = do_commands(buffer);
                                                if (status != CMD_ERR_BLANK) {
                                                    printf("bsu command status = %d\n", status);
                                                }
                                                pContext->show_secondary = false;
                                                render_ui(pContext);
                                                break;
                                            default:
                                                render_message(pContext, "not currently supported");
                                        }
                                    }
                                    break;
                                case 2:
                                    if (pContext->show_secondary == false) {
                                        pContext->show_secondary = true;
                                        pContext->secondary_focused_button = 0; /* Start off at the top */
                                        render_ui(pContext);
                                    } else {
                                        switch (pContext->secondary_focused_button) {
                                            case 0:
                                                strcpy(buffer, "flash -noheader stb-irva-09:/agin/src/git/refsw/myrepo/obj.97584/BSEAV/bin/bsu.bin.gz flash0.kreserv");
                                                render_message(pContext, "Updating BSU from stb-irva-09:/agin/src/git/refsw/myrepo/obj.97584/BSEAV/bin/bsu.bin.gz... Please wait...");
                                                status = do_commands(buffer);
                                                if (status != CMD_ERR_BLANK) {
                                                    printf("bsu command status = %d\n", status);
                                                }
                                                pContext->show_secondary = false;
                                                render_ui(pContext);
                                                break;
                                            default:
                                                render_message(pContext, "not currently supported");
                                        }
                                    }
                                    break;
                                case 3:
#ifdef BSU_OSD_ACROSS_BOOT
                                    NEXUS_Platform_UninitInterrupts();
#else
                                    NEXUS_Platform_Uninit();
#endif
#ifndef MIPS_SDE
                                    interrupts_uninit();
#endif
                                    strcpy(buffer, "boot -z -elf flash0.kernel: 'bmem=192M@64M bmem=512M@512M ubiroot'");
                                    status = do_commands(buffer);
                                    if (status != CMD_ERR_BLANK) {
                                        printf("bsu command status = %d\n", status);
                                    }
                                    break;
                                case 4:
                                    render_message(pContext, "not currently supported");
                                    break;
                                case 5:
                                    strcpy(buffer, "reboot");
                                    status = do_commands(buffer);
                                    if (status != CMD_ERR_BLANK) {
                                        printf("bsu command status = %d\n", status);
                                    }
                                    break;
                                }
                            }
                            break;

                        default:
                            break;
                    }
                }
            }
        }
#ifdef BSU_QAM_SUPPORT
        else if (wakeEvent == lockChangedEvent) {
            render_ui(pContext);
        }
#endif
    }

    NEXUS_IrInput_Close(irHandle);

    if (pContext->font) {
        bfont_close(pContext->font);
    }
    BKNI_DestroyEvent(pContext->displayedEvent);
    BKNI_DestroyEvent(pContext->checkpointEvent);
    BKNI_DestroyEvent(pContext->inputEvent);
    NEXUS_Surface_Destroy(pContext->surface);
    NEXUS_Graphics2D_Close(pContext->gfx);

    if (fsctx_usb0) {
        if (bsu_fs_uninit(fsctx_usb0) != 0) printf("error from bsu_fs_uninit\n");
        fsctx_usb0 = 0;
        printf("usb file system un-initialized\n");
    }

    free(pContext);

    return 0;
}

static void render_ui(struct appcontext *pContext)
{
    NEXUS_Graphics2DFillSettings fillSettings;
    int rc;
    unsigned i;

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = pContext->surface;

    if (pContext->background) {
        NEXUS_Graphics2DBlitSettings blitSettings;
        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.source.surface = pContext->background;
        blitSettings.output.surface = pContext->surface;
        rc = NEXUS_Graphics2D_Blit(pContext->gfx, &blitSettings);
        BDBG_ASSERT(!rc);
    }
    else {
        fillSettings.color = 0;
        rc = NEXUS_Graphics2D_Fill(pContext->gfx, &fillSettings);
        BDBG_ASSERT(!rc);
    }

    for (i=0;i<pContext->total_buttons;i++) {
        const char *name = pContext->launch[i].name;
        bool focused = i == pContext->focused_button;

        fillSettings.rect.x = 50;
        fillSettings.rect.y = 90 + i*50;
        fillSettings.rect.width = 450;
        fillSettings.rect.height = 40;
        fillSettings.color = (i == pContext->focused_button) ? 0xFF00FF00 : 0xFF008888;
        rc = NEXUS_Graphics2D_Fill(pContext->gfx, &fillSettings);
        BDBG_ASSERT(!rc);

        rc = NEXUS_Graphics2D_Checkpoint(pContext->gfx, NULL); /* require to execute queue */
        if (rc == NEXUS_GRAPHICS2D_QUEUED) {
            rc = BKNI_WaitForEvent(pContext->checkpointEvent, BKNI_INFINITE);
        }
        BDBG_ASSERT(!rc);

        if (pContext->font) {
            struct bfont_surface_desc desc;
            bfont_get_surface_desc(pContext->surface, &desc);
            bfont_draw_aligned_text(&desc, pContext->font, &fillSettings.rect, name, -1, focused?0xFF333333:0xFFCCCCCC, bfont_valign_center, bfont_halign_center);
            NEXUS_Surface_Flush(pContext->surface);
        }

        if (focused) {
            printf("'%s' will launch '%s'\n", name, pContext->launch[i].cmdline);
        }
    }

    /* Show secondary column of buttons */
    if (pContext->show_secondary) {
        unsigned int j;
        int k = pContext->focused_button;
        switch (pContext->focused_button) {
            case 0:
            case 1:
            case 2:
                for (j=0; j<pContext->secondary_total_buttons; j++) {
                    bool focused2 = pContext->secondary_focused_button == j;
                    fillSettings.rect.x = 50 + 450 + 5;
                    fillSettings.rect.y = 90 + (j+k)*50;
                    fillSettings.rect.width = 450;
                    fillSettings.rect.height = 40;
                    fillSettings.color = (j == pContext->secondary_focused_button) ? 0xFF00FF00 : 0xFF008888;
                    rc = NEXUS_Graphics2D_Fill(pContext->gfx, &fillSettings);
                    BDBG_ASSERT(!rc);

                    rc = NEXUS_Graphics2D_Checkpoint(pContext->gfx, NULL); /* require to execute queue */
                    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
                        rc = BKNI_WaitForEvent(pContext->checkpointEvent, BKNI_INFINITE);
                    }
                    BDBG_ASSERT(!rc);

                    if (pContext->font) {
                        struct bfont_surface_desc desc;
                        bfont_get_surface_desc(pContext->surface, &desc);
                        bfont_draw_aligned_text(&desc, pContext->font, &fillSettings.rect, j==0 ? "from ethernet" : (j==1 ? "from USB" : "from frontend"), -1, focused2?0xFF333333:0xFFCCCCCC, bfont_valign_center, bfont_halign_center);
                        NEXUS_Surface_Flush(pContext->surface);
                    }
                }
                break;
        }
    }

    {
        struct bfont_surface_desc desc;
        char buffer[256];
        NEXUS_Rect rect;            /* Area of surface to fill. width,height of 0,0 fills the entire surface */
        rect.x = 50;
        rect.y = 620;
        rect.width = 1450;
        rect.height = 40;
        bfont_get_surface_desc(pContext->surface, &desc);
        sprintf(buffer, "BCM%d, CHIP=%d", NEXUS_PLATFORM, BCHP_CHIP);
#if BCHP_VER == BCHP_VER_A0
    strcat(buffer, "A0");
#elif BCHP_VER == BCHP_VER_A1
    strcat(buffer, "A1");
#elif BCHP_VER == BCHP_VER_A2
    strcat(buffer, "A2");
#elif BCHP_VER == BCHP_VER_A3
    strcat(buffer, "A3");
#elif BCHP_VER == BCHP_VER_A4
    strcat(buffer, "A4");
#elif BCHP_VER == BCHP_VER_A5
    strcat(buffer, "A5");
#elif BCHP_VER == BCHP_VER_B0
    strcat(buffer, "B0");
#elif BCHP_VER == BCHP_VER_B1
    strcat(buffer, "B1");
#elif BCHP_VER == BCHP_VER_B2
    strcat(buffer, "B2");
#elif BCHP_VER == BCHP_VER_B3
    strcat(buffer, "B3");
#elif BCHP_VER == BCHP_VER_B4
    strcat(buffer, "B4");
#elif BCHP_VER == BCHP_VER_B5
    strcat(buffer, "B5");
#elif BCHP_VER == BCHP_VER_C0
    strcat(buffer, "C0");
#elif BCHP_VER == BCHP_VER_C1
    strcat(buffer, "C1");
#elif BCHP_VER == BCHP_VER_C2
    strcat(buffer, "C2");
#elif BCHP_VER == BCHP_VER_C3
    strcat(buffer, "C3");
#elif BCHP_VER == BCHP_VER_C4
    strcat(buffer, "C4");
#elif BCHP_VER == BCHP_VER_C5
    strcat(buffer, "C5");
#elif BCHP_VER == BCHP_VER_D0
    strcat(buffer, "D0");
#elif BCHP_VER == BCHP_VER_D1
    strcat(buffer, "D1");
#elif BCHP_VER == BCHP_VER_D2
    strcat(buffer, "D2");
#elif BCHP_VER == BCHP_VER_D3
    strcat(buffer, "D3");
#elif BCHP_VER == BCHP_VER_D4
    strcat(buffer, "D4");
#elif BCHP_VER == BCHP_VER_D5
    strcat(buffer, "D5");
#elif BCHP_VER == BCHP_VER_E0
    strcat(buffer, "E0");
#elif BCHP_VER == BCHP_VER_E1
    strcat(buffer, "E1");
#elif BCHP_VER == BCHP_VER_E2
    strcat(buffer, "E2");
#elif BCHP_VER == BCHP_VER_E3
    strcat(buffer, "E3");
#elif BCHP_VER == BCHP_VER_E4
    strcat(buffer, "E4");
#elif BCHP_VER == BCHP_VER_E5
    strcat(buffer, "E5");
#else
    #error
#endif
        bfont_draw_aligned_text(&desc, pContext->font, &rect, buffer, -1, 0xFFCCCCCC, bfont_valign_center, bfont_halign_left);
        NEXUS_Surface_Flush(pContext->surface);
    }

#ifdef BSU_QAM_SUPPORT
    /* Show lock status */
    if (frontend) {
        NEXUS_FrontendQamStatus qamStatus;
        struct bfont_surface_desc desc;
        char buffer[256];
        NEXUS_Rect rect;            /* Area of surface to fill. width,height of 0,0 fills the entire surface */
        rect.x = 50;
        rect.y = 650;
        rect.width = 1450;
        rect.height = 40;
        bfont_get_surface_desc(pContext->surface, &desc);
        rc = NEXUS_Frontend_GetQamStatus(frontend, &qamStatus);
        sprintf(buffer, "QAM status:  freq=%d MHz, mode=%s, receiver lock=%d, fec lock=%d", qamStatus.settings.frequency/1000000, (qamStatus.settings.mode==NEXUS_FrontendQamMode_e64)?"64Q":"Unknown", qamStatus.receiverLock, qamStatus.fecLock);
        bfont_draw_aligned_text(&desc, pContext->font, &rect, buffer, -1, 0xFFCCCCCC, bfont_valign_center, bfont_halign_left);
        NEXUS_Surface_Flush(pContext->surface);
    }
#endif
}

static void render_message(struct appcontext *pContext, char *text)
{
    struct bfont_surface_desc desc;
    NEXUS_Rect rect;            /* Area of surface to fill. width,height of 0,0 fills the entire surface */
    rect.x = 50;
    rect.y = 590;
    rect.width = 1450;
    rect.height = 40;
    bfont_get_surface_desc(pContext->surface, &desc);
    bfont_draw_aligned_text(&desc, pContext->font, &rect, text, -1, 0xFF333333, bfont_valign_center, bfont_halign_left);
    NEXUS_Surface_Flush(pContext->surface);
}
