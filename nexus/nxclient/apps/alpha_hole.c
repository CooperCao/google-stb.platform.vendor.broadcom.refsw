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
#include "nexus_platform_client.h"
#include "bgui.h"
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

BDBG_MODULE(alpha_hole);

/* Punching an alpha hole through SurfaceCompositor is very simple: just submit a surface with all zeroes. If it is not blended
with other surfaces below, and if not covered by surfaces above, the result will be alpha 0 all the way through. Video shows.
For example:

nexus nxserver &
. nxclient.sh
play videos/cnnticker.mpg &
sleep 1
animation_client &
sleep 1
alpha_hole -rect 100,100,400,300
*/

static void print_usage(void)
{
    printf(
    "Usage: alpha_hole OPTIONS\n"
    "  --help or -h for help\n"
    "  -rect x,y,width,height   position in default 1920x1080 coordinates\n"
    "  -zorder #\n"
    );
}

int main(int argc, const char **argv)
{
    NxClient_JoinSettings joinSettings;
    bgui_t gui;
    struct bgui_settings gui_settings;
    NEXUS_Error rc;
    int curarg = 1;
    NEXUS_Rect rect = {0,0,0,0};
    unsigned zorder = 0;

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

    bgui_get_default_settings(&gui_settings);
    gui_settings.width = 10;
    gui_settings.height = 10;
    gui = bgui_create(&gui_settings);

    if (rect.width || zorder) {
        NEXUS_SurfaceComposition comp;
        NxClient_GetSurfaceClientComposition(bgui_surface_client_id(gui), &comp);
        if (rect.width) {
            comp.position = rect;
        }
        comp.zorder = zorder;
        NxClient_SetSurfaceClientComposition(bgui_surface_client_id(gui), &comp);
    }

    bgui_fill(gui, 0);
    bgui_checkpoint(gui);
    bgui_submit(gui);

    while (1) BKNI_Sleep(1000);

    return 0;
}
