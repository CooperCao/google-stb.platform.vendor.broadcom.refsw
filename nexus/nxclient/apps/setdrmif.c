/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/
#include "nexus_platform_client.h"
#include "nxclient.h"
#include "namevalue.h"
#include "nxapps_cmdline.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "bstd.h"
#include "bkni.h"

BDBG_MODULE(setdrmif);

static void print_usage(void)
{
    printf(
        "Usage: setdrmif OPTIONS\n"
        );
    print_list_option("eotf",g_videoEotfStrs);
    printf(
        "  -cll MAX_CLL         max content light level, units 1 cd/m^2, -1 means from input\n"
        "  -fal MAX_FAL         max frame average light level, units 1 cd/m^2, -1 means from input\n"
    );
    printf(
        "  -mdcv.red X,Y        red chromaticity coordinate, range 0 to 50000, maps to 0.0 to 1.0, -1 means from input\n"
        "  -mdcv.green X,Y      green chromaticity coordinate, range 0 to 50000, maps to 0.0 to 1.0, -1 means from input\n"
        "  -mdcv.blue X,Y       blue chromaticity coordinate, range 0 to 50000, maps to 0.0 to 1.0, -1 means from input\n"
        "  -mdcv.white X,Y      chromaticity white point, range 0 to 50000, maps to 0.0 to 1.0, -1 means from input\n"
    );
    printf(
        "  -mdcv.luma.max MAX   units 1 cd / m^2, -1 means from input\n"
        "  -mdcv.luma.min MIN   units 0.0001 cd / m^2, -1 means from input\n"
    );
}

static void print_settings(const NxClient_DisplaySettings *pSettings)
{
    char buf[256];
    unsigned n;

    n = 0;
    if (pSettings->hdmiPreferences.enabled) {
        n += snprintf(&buf[n], sizeof(buf)-n, "DRMIF Settings:\n\teotf: %s\n\tmdcv:\n\t\tred=(%d,%d)\n\t\tgreen=(%d,%d)\n\t\tblue=(%d,%d)\n\t\twhite=(%d,%d)\n\t\tluma=(%d,%d)\n\tcll:\n\t\tmaxCLL=%d\n\t\tmaxFAL=%d\n",
        lookup_name(g_videoEotfStrs, pSettings->hdmiPreferences.drmInfoFrame.eotf),
        pSettings->hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.contentLightLevel.max,
        pSettings->hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.contentLightLevel.maxFrameAverage,
        pSettings->hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.redPrimary.x,
        pSettings->hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.redPrimary.y,
        pSettings->hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.greenPrimary.x,
        pSettings->hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.greenPrimary.y,
        pSettings->hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.bluePrimary.x,
        pSettings->hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.bluePrimary.y,
        pSettings->hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.whitePoint.x,
        pSettings->hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.whitePoint.y,
        pSettings->hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.luminance.max,
        pSettings->hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.luminance.min);

    }
    printf("%s\n", buf);
}

int main(int argc, char **argv)  {
    NxClient_JoinSettings joinSettings;
    NxClient_DisplaySettings displaySettings;
    bool change = false;
    int curarg = 1;
    int rc;

    NxClient_GetDefaultJoinSettings(&joinSettings);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    NxClient_GetDisplaySettings(&displaySettings);

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-eotf") && argc>curarg+1) {
            change = true;
            displaySettings.hdmiPreferences.drmInfoFrame.eotf = lookup(g_videoEotfStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-mdcv.red") && argc>curarg+1) {
            int x,y;
            if (sscanf(argv[++curarg], "%d,%d", &x, &y) == 2) {
                displaySettings.hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.redPrimary.x = x;
                displaySettings.hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.redPrimary.y = y;
                change = true;
            }
        }
        else if (!strcmp(argv[curarg], "-mdcv.green") && argc>curarg+1) {
            int x,y;
            if (sscanf(argv[++curarg], "%d,%d", &x, &y) == 2) {
                displaySettings.hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.greenPrimary.x = x;
                displaySettings.hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.greenPrimary.y = y;
                change = true;
            }
        }
        else if (!strcmp(argv[curarg], "-mdcv.blue") && argc>curarg+1) {
            int x,y;
            if (sscanf(argv[++curarg], "%d,%d", &x, &y) == 2) {
                displaySettings.hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.bluePrimary.x = x;
                displaySettings.hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.bluePrimary.y = y;
                change = true;
            }
        }
        else if (!strcmp(argv[curarg], "-mdcv.white") && argc>curarg+1) {
            int x,y;
            if (sscanf(argv[++curarg], "%d,%d", &x, &y) == 2) {
                displaySettings.hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.whitePoint.x = x;
                displaySettings.hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.whitePoint.y = y;
                change = true;
            }
        }
        else if (!strcmp(argv[curarg], "-mdcv.luma.max") && argc>curarg+1) {
            change = true;
            displaySettings.hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.luminance.max = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-mdcv.luma.min") && argc>curarg+1) {
            change = true;
            displaySettings.hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.luminance.min = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-cll") && argc>curarg+1) {
            change = true;
            displaySettings.hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.contentLightLevel.max = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-fal") && argc>curarg+1) {
            change = true;
            displaySettings.hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.contentLightLevel.maxFrameAverage = atoi(argv[++curarg]);
        }
        else {
            print_usage();
            return -1;
        }
        curarg++;
    }

    if (change) {
        rc = NxClient_SetDisplaySettings(&displaySettings);
        if (rc) BERR_TRACE(rc);
        print_settings(&displaySettings);
    }
    else {
        print_settings(&displaySettings);
    }

    NxClient_Uninit();
    return 0;
}
