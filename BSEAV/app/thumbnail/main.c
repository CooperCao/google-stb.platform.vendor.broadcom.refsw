/******************************************************************************
 *    (c)2016 Broadcom Corporation
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
#include "nexus_platform.h"
#include "nexus_surface.h"
#include "thumbnail.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

BDBG_MODULE(thumbnail);

static void print_usage(void)
{
    printf(
    "Usage: nexus thumbnail [OPTIONS] DATAFILE [INDEXFILE]\n"
    "  DATAFILE = stream data\n"
    "  INDEXFILE = stream index (use ""same"" if the DATAFILE contains the index, ""none"" if no index)\n"
    "\n"
    "OPTIONS:\n"
    "  -h or --help\n"
    "  -spacing SECONDS (default 5)\n"
    );
}

int main(int argc, char **argv)
{
    const char *filename = NULL;
    const char *indexfile = NULL;
    int curarg = 1;
    int rc;
    NEXUS_PlatformSettings platformSettings;

    g_data.spacing = 5;
    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-spacing") && curarg+1 < argc) {
            g_data.spacing = atoi(argv[++curarg]);
        }
        else if (!filename) {
            filename = argv[curarg];
        }
        else if (!indexfile) {
            indexfile = argv[curarg];
        }
        else {
            print_usage();
            return -1;
        }
        curarg++;
    }
    if (!filename) {
        print_usage();
        return -1;
    }

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);

    g_data.datafilename = filename;
    g_data.indexfilename = indexfile;
    if (g_data.indexfilename)  {
        if (!strcasecmp(g_data.indexfilename, "same")) {
            g_data.indexfilename = g_data.datafilename;
        }
        else if (!strcasecmp(g_data.indexfilename, "none")) {
            g_data.indexfilename = NULL;
        }
    }

    rc = probe_media(filename, &g_data.probe_results);
    if (rc) return BERR_TRACE(-1);

    thumbnail_demo_init();

    thumbnail_demo_run();

    thumbnail_demo_uninit();

    NEXUS_Platform_Uninit();

    return 0;
}
