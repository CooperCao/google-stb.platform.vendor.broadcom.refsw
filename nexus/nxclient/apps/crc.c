/******************************************************************************
 *    (c)2014 Broadcom Corporation
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
#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_SIMPLE_DECODER
#include "nxclient.h"
#include "media_player.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

BDBG_MODULE(crc);

static void print_usage(void)
{
    printf(
        "Usage: crc OPTIONS\n"
        "-cmp DISPLAYINDEX       0=HD,1=SD,2=encode. ctrl-c to exit.\n"
        "-hdmi                   ctrl-c to exit.\n"
        "-avd FILE               play file once and exit\n"
        "-mfd FILE               play file once and exit\n"
        );
}

enum crc_type
{
    crc_type_none,
    crc_type_cmp,
    crc_type_hdmi,
    crc_type_avd,
    crc_type_mfd
};

int main(int argc, const char **argv)
{
    NxClient_JoinSettings joinSettings;
    int rc;
    enum crc_type crc_type = crc_type_none;
    int curarg = 1;
    const char *filename = NULL;
    media_player_t player = NULL;
    unsigned displayIndex;

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "--help") || !strcmp(argv[curarg], "-h")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-cmp") && curarg+1 < argc) {
            crc_type = crc_type_cmp;
            displayIndex = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-hdmi")) {
            crc_type = crc_type_hdmi;
        }
        else if (!strcmp(argv[curarg], "-avd") && curarg+1 < argc) {
            crc_type = crc_type_avd;
            filename = argv[++curarg];
        }
        else if (!strcmp(argv[curarg], "-mfd") && curarg+1 < argc) {
            crc_type = crc_type_mfd;
            filename = argv[++curarg];
        }
        else {
            print_usage();
            return -1;
        }
        curarg++;
    }
    if (crc_type == crc_type_none) {
        print_usage();
        return -1;
    }

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    if (filename) {
        media_player_start_settings start_settings;

        player = media_player_create(NULL);
        if (!player) return -1;

        media_player_get_default_start_settings(&start_settings);
        start_settings.stream_url = filename;
        start_settings.loopMode = NEXUS_PlaybackLoopMode_ePause; /* once */
        rc = media_player_start(player, &start_settings);
        if (rc) return rc;
    }

    switch (crc_type) {
    case crc_type_cmp:
        for (;;) {
            NxClient_DisplayCrcData data;
            rc = NxClient_Display_GetCrcData(displayIndex, &data);
            if (rc) { BDBG_ERR(("CRC on CMP %d not supported", displayIndex)); goto done; }
            if (!data.numEntries) {
                BKNI_Sleep(10);
            }
            else {
                unsigned i;
                for (i=0;i<data.numEntries;i++) {
                    printf("CMP CRC %x %x %x\n", data.data[i].cmp.luma, data.data[i].cmp.cb, data.data[i].cmp.cr);
                }
            }
        }
        break;
#if NEXUS_HAS_HDMI_OUTPUT
    case crc_type_hdmi:
        for (;;) {
            NxClient_HdmiOutputCrcData data;
            rc = NxClient_HdmiOutput_GetCrcData(&data);
            if (rc) { BDBG_ERR(("CRC on HDMI not supported")); goto done; }
            if (!data.numEntries) {
                BKNI_Sleep(10);
            }
            else {
                unsigned i;
                for (i=0;i<data.numEntries;i++) {
                    printf("HDMI CRC %x\n", data.data[i].crc);
                }
            }
        }
        break;
#endif
    case crc_type_avd:
        if (!player) {
            BERR_TRACE(NEXUS_NOT_SUPPORTED);
            break;
        }
        for (;;) {
            NEXUS_VideoDecoderCrc data[16];
            unsigned num;
            NEXUS_SimpleVideoDecoderHandle videoDecoder = media_player_get_video_decoder(player);

            rc = NEXUS_SimpleVideoDecoder_GetCrcData(videoDecoder, data, sizeof(data)/sizeof(data[0]), &num);
            BDBG_ASSERT(!rc);
            if (!num) {
                BKNI_Sleep(10);
            }
            else {
                unsigned i;
                for (i=0;i<num;i++) {
                    printf("AVD CRC %x %x %x; %x %x %x\n", data[i].top.luma, data[i].top.cr, data[i].top.cb, data[i].bottom.luma, data[i].bottom.cr, data[i].bottom.cb);
                }
            }
        }
        break;
    case crc_type_mfd:
        if (!player) {
            BERR_TRACE(NEXUS_NOT_SUPPORTED);
            break;
        }
        for (;;) {
            NEXUS_VideoInputCrcData data[16];
            unsigned num;
            NEXUS_SimpleVideoDecoderHandle videoDecoder = media_player_get_video_decoder(player);

            rc = NEXUS_SimpleVideoDecoder_GetVideoInputCrcData(videoDecoder, data, sizeof(data)/sizeof(data[0]), &num);
            BDBG_ASSERT(!rc);
            if (!num) {
                BKNI_Sleep(10);
            }
            else {
                unsigned i;
                for (i=0;i<num;i++) {
                    printf("MFD CRC %u,%u %u,%u right=%u,%u, field=%c\n", data[i].idrPictureId, data[i].pictureOrderCount,
                        data[i].crc[0], data[i].crc[1], data[i].crc[3], data[i].crc[4], data[i].isField?'y':'n');
                }
            }
        }
        break;
    default:
        break;
    }

done:
    if (filename) {
        media_player_stop(player);
        media_player_destroy(player);
    }
    NxClient_Uninit();
    return 0;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform (needs playback and simple_decoder)!\n");
    return 0;
}
#endif
