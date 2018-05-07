/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_platform_client.h"
#include "nxclient.h"
#include "namevalue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
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
        "  -version {auto|hdcp1x|hdcp22type0|hdcp22}\n"
        "  -hdcp2x_keys BINFILE \tload Hdcp2.x bin file\n"
        "  -hdcp1x_keys BINFILE \tload Hdcp1.x bin file\n"
        "  -timeout     SECONDS \tbefore exiting (otherwise, wait for ctrl-C)\n"
        "  -p           prompt\n"
        );

    printf (
        "  -revokedList BINFILE \tload HDCP revocation list bin file\n"
                        /* HDCP revocation list is a binary file content the list of appending revoked KSV/ReceiverID.
                                Each ReceiverID/KSV is 5 bytes long store in big endian format.
                                */
        );
}

#include "hdmi_output_status.inc"

int main(int argc, char **argv)  {
    NxClient_JoinSettings joinSettings;
    NxClient_DisplaySettings displaySettings;
    int curarg = 1;
    int rc;
    const char *hdcp_keys_binfile[NxClient_HdcpType_eMax] = {NULL,NULL};
    bool load_keys = false;
    int timeout = -1;
    unsigned i;
    char *revocationListFile = NULL;
    bool prompt = false;

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
            else if (!strcmp(argv[curarg], "hdcp1x")) displaySettings.hdmiPreferences.version = NxClient_HdcpVersion_eHdcp1x;
            else if (!strcmp(argv[curarg], "hdcp22type0"))   displaySettings.hdmiPreferences.version = NxClient_HdcpVersion_eAutoHdcp22Type0;
            else if (!strcmp(argv[curarg], "hdcp22")) displaySettings.hdmiPreferences.version = NxClient_HdcpVersion_eHdcp22;
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
        else if (!strcmp(argv[curarg], "-revokedList") && curarg+1<argc) {
            revocationListFile = (char *)argv[++curarg];
        }
        else if (!strcmp(argv[curarg], "-p")) {
            prompt = true;
        }
        else {
            print_usage();
            return -1;
        }
        curarg++;
    }

    if (revocationListFile)
    {
        NEXUS_HdmiOutputHandle hdmiOutput;
        int fd, n;
        off_t seekPos;
        size_t fileSize;
        uint8_t *buffer = NULL;

        hdmiOutput = NEXUS_HdmiOutput_Open(0 + NEXUS_ALIAS_ID, NULL);
        if (!hdmiOutput) {
            BDBG_WRN(("Can't get hdmi output alias\n"));
            return -1;
        }

        fd = open(revocationListFile, O_RDONLY);
        if (fd < 0) {
            if (revocationListFile) {
                BDBG_ERR(("Cannot open revocation bin file %s", revocationListFile));
            }
            rc = NEXUS_OS_ERROR;
            goto done;
        }

        seekPos = lseek(fd, 0, SEEK_END);
        if (seekPos < 0) {
            rc = BERR_TRACE(NEXUS_OS_ERROR);
            goto done;
        }
        fileSize = (size_t) (seekPos - (seekPos % 5));
        BDBG_LOG(("loading %u bytes of revoked KSV/ReceiverId List from '%s', dropping %d bytes off the list",
                        (unsigned) fileSize, revocationListFile, (uint16_t) (seekPos % 5))) ;
        if (lseek(fd, 0, SEEK_SET) < 0) {
            rc = BERR_TRACE(NEXUS_OS_ERROR);
            goto done;
        }

        buffer = BKNI_Malloc(fileSize);
        if (!buffer) {
            rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            goto done;
        }

        n = read(fd, buffer, fileSize);
        if (n != (int)fileSize) {
            rc = BERR_TRACE(NEXUS_OS_ERROR);
            goto done;
        }

        close(fd);

        /* install list of revoked KSVs from SRMs (System Renewability Message) if available */
        BDBG_LOG(("Read Revoked KSV/ReceiverID list from file"));
        NEXUS_HdmiOutput_SetHdcpRevokedKsvs(hdmiOutput, (NEXUS_HdmiOutputHdcpKsv *)buffer, (uint16_t) (fileSize/5)) ;

        BKNI_Free(buffer);
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
        if (prompt) {
            while (1) {
                char buf[64];

                NxClient_GetDisplaySettings(&displaySettings);
                printf("sethdcp>");
                fflush(stdout);
                fgets(buf, sizeof(buf), stdin);
                if (feof(stdin)) break;
                buf[strlen(buf)-1] = 0; /* chop off \n */
                if (!strcmp(buf, "?")) {
                    printf(
                    "none\n"
                    "auto\n"
                    "hdcp1x\n"
                    "hdcp22type0\n"
                    "hdcp22\n"
                    "st\n"
                    "q\n"
                    );
                }
                else if (!strcmp(buf, "none")) {
                    displaySettings.hdmiPreferences.hdcp = NxClient_HdcpLevel_eNone;
                }
                else if (!strcmp(buf, "auto")) {
                    displaySettings.hdmiPreferences.hdcp = NxClient_HdcpLevel_eMandatory;
                    displaySettings.hdmiPreferences.version = NxClient_HdcpVersion_eAuto;
                }
                else if (!strcmp(buf, "hdcp1x")) {
                    displaySettings.hdmiPreferences.hdcp = NxClient_HdcpLevel_eMandatory;
                    displaySettings.hdmiPreferences.version = NxClient_HdcpVersion_eHdcp1x;
                }
                else if (!strcmp(buf, "hdcp22type0")) {
                    displaySettings.hdmiPreferences.hdcp = NxClient_HdcpLevel_eMandatory;
                    displaySettings.hdmiPreferences.version = NxClient_HdcpVersion_eAutoHdcp22Type0;
                }
                else if (!strcmp(buf, "hdcp22")) {
                    displaySettings.hdmiPreferences.hdcp = NxClient_HdcpLevel_eMandatory;
                    displaySettings.hdmiPreferences.version = NxClient_HdcpVersion_eHdcp22;
                }
                else if (!strcmp(buf, "st") || !buf[0]) {
                    print_hdmi_status();
                    continue;
                }
                else if (!strcmp(buf, "q")) {
                    break;
                }
                else {
                    printf("unknown command: %s\n", buf);
                    continue;
                }

                rc = NxClient_SetDisplaySettings(&displaySettings);
                if (rc) BERR_TRACE(rc);
            }
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
    }

done:
    return 0;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
