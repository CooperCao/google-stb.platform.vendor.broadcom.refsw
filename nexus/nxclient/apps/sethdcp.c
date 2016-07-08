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
#include "nexus_platform_client.h"
#include "nxclient.h"
#include "namevalue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bstd.h"
#include "bkni.h"

BDBG_MODULE(sethdcp);

static void print_usage(void)
{
    printf(
        "Usage: sethdcp OPTIONS\n"
        "\n"
        "OPTIONS:\n"
        "  --help or -h for help\n"
        "  -o           HDCP authentication is optional (video not muted). Default is mandatory (video will be muted on HDCP failure).\n"
        "  -version {auto|follow|hdcp1x|hdcp22}\n"
        "               Force new version select policy\n"
        "  -hdcp2x_keys BINFILE \tload Hdcp2.x bin file\n"
        "  -hdcp1x_keys BINFILE \tload Hdcp1.x bin file\n"
        "  -timeout     SECONDS \tbefore exiting (otherwise, wait for ctrl-C)\n"
        );
}

int main(int argc, char **argv)  {
    NxClient_JoinSettings joinSettings;
    NxClient_DisplaySettings displaySettings;
    int curarg = 1;
    int rc;
    const char *hdcp_keys_binfile[NxClient_HdcpType_eMax] = {NULL,NULL};
    bool load_keys = false;
    int timeout = -1;
    unsigned i;

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    NxClient_GetDisplaySettings(&displaySettings);

    displaySettings.hdmiPreferences.hdcp = NxClient_HdcpLevel_eMandatory;

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-o")) {
            displaySettings.hdmiPreferences.hdcp = NxClient_HdcpLevel_eOptional;
        }
        else if (!strcmp(argv[curarg], "-version") && argc>curarg+1) {
            curarg++;
            if (!strcmp(argv[curarg], "auto"))   displaySettings.hdmiPreferences.version = NxClient_HdcpVersion_eAuto;
            if (!strcmp(argv[curarg], "follow")) displaySettings.hdmiPreferences.version = NxClient_HdcpVersion_eFollow;
            if (!strcmp(argv[curarg], "hdcp1x")) displaySettings.hdmiPreferences.version = NxClient_HdcpVersion_eHdcp1x;
            if (!strcmp(argv[curarg], "hdcp22")) displaySettings.hdmiPreferences.version = NxClient_HdcpVersion_eHdcp22;
        }
        else if (!strcmp(argv[curarg], "-hdcp1x_keys") && curarg+1<argc) {
            hdcp_keys_binfile[NxClient_HdcpType_1x] = argv[++curarg];
            load_keys = true;
        }
        else if (!strcmp(argv[curarg], "-hdcp2x_keys") && curarg+1<argc) {
            hdcp_keys_binfile[NxClient_HdcpType_2x] = argv[++curarg];
            load_keys = true;
        }
        else if (!strcmp(argv[curarg], "-timeout") && argc>curarg+1) {
            timeout = strtoul(argv[++curarg], NULL, 0);
        }
        else {
            print_usage();
            return -1;
        }
        curarg++;
    }

    if (load_keys) {
        int size = 32*1024;
        void *ptr;
        NEXUS_MemoryBlockHandle block;

        block = NEXUS_MemoryBlock_Allocate(NULL, size, 0, NULL);
        if (!block) return BERR_TRACE(-1);
        rc = NEXUS_MemoryBlock_Lock(block, &ptr);
        if (rc) return BERR_TRACE(rc);

        for (i=0;i<NxClient_HdcpType_eMax;i++) {
            FILE *f;
            if (!hdcp_keys_binfile[i]) continue;
            f = fopen(hdcp_keys_binfile[i], "rb");
            if (!f)  {
                BERR_TRACE(-1);
            }
            else {
                int n = fread(ptr, 1, size, f);
                if (n <= 0) {
                    BDBG_ERR(("unable to read keys: %d", n));
                }
                else {
                    rc = NxClient_LoadHdcpKeys(i, block, 0, n);
                    if (rc) BERR_TRACE(rc);
                }
                fclose(f);
            }
        }
        NEXUS_MemoryBlock_Free(block);
    }
    else {
        rc = NxClient_SetDisplaySettings(&displaySettings);
        if (rc) BERR_TRACE(rc);

        if (timeout < 0) {
            BDBG_WRN(("Press Ctrl-C to exit sethdcp and clear HDCP %s state",
                displaySettings.hdmiPreferences.hdcp == NxClient_HdcpLevel_eMandatory?"mandatory":"optional"));
            while (1) sleep(1);
        }
        else {
            sleep(timeout);
            BDBG_WRN(("exiting sethdcp and clear HDCP %s state",
                displaySettings.hdmiPreferences.hdcp == NxClient_HdcpLevel_eMandatory?"mandatory":"optional"));
        }
    }

    return 0;
}
