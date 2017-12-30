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
#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_SIMPLE_DECODER
#include "nxclient.h"
#include "media_player.h"
#include "nexus_simple_video_decoder.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "namevalue.h"
#include "nexus_core_utils.h"

BDBG_MODULE(crc);

static void print_usage(void)
{
    printf(
        "Usage: crc OPTIONS\n"
        "-cmp DISPLAYINDEX       0=HD,1=SD,2=encode. ctrl-c to exit.\n"
        "-hdmi                   ctrl-c to exit.\n"
        "-avd FILE[:PROGRAM] ...\n"
        "-mfd FILE[:PROGRAM] ...\n"
        "-only_mfd FILE[:PROGRAM] ...\n   like MFD, but don't set the decoder into CRC mode"
        "\n"
        "PROGRAM defaults to 0. If more than one FILE[:PROGRAM] is listed, mosaic decode will be used and CRC's will print to crc#.txt\n"
        );
    printf(
        "\n"
        "Bypass media probe:\n"
        "  -video PID\n");
    print_list_option(
        "  -video_type          ", g_videoCodecStrs);
    print_list_option(
        "  -mpeg_type           ", g_transportTypeStrs);
    printf(
        "  -colorDepth {8|10}\n"
        "  -max WIDTH,HEIGHT        max video decoder resolution\n"
        "  -dqt\n"
        "  -mdqt\n"
        "  -zero_delay_output_mode\n"
        "  -display_timestamp_mode\n"
        "  -iframe_as_rap\n"
        "  -ignore_dpb_output_delay\n"
        "  -ignore_num_reorder_frames\n"
    );
    printf(
        "  -video_cdb SIZE          use 'm' or 'k' suffix, decimal allows\n"
        "  -video_itb SIZE          use 'm' or 'k' suffix, decimal allows\n"
        "  -video_framerate HZ      default video frame rate if not in stream (for example 29.97, 30, 59.94, 60)\n"
    );
}

static unsigned b_parse_size(const char *parse)
{
    if (strchr(parse, 'M') || strchr(parse, 'm')) {
        return atof(parse)*1024*1024;
    }
    else if (strchr(parse, 'K') || strchr(parse, 'k')) {
        return atof(parse)*1024;
    }
    else {
        return strtoul(parse, NULL, 0);
    }
}

enum crc_type
{
    crc_type_none,
    crc_type_cmp,
    crc_type_hdmi,
    crc_type_avd,
    crc_type_mfd
};

#define MAX_DECODES 12

int main(int argc, const char **argv)
{
    NxClient_JoinSettings joinSettings;
    int rc;
    enum crc_type crc_type = crc_type_none;
    int curarg = 1;
    struct {
        const char *filename;
        bool done;
        FILE *file;
        unsigned program;
    } decoder[MAX_DECODES];
    media_player_t player[MAX_DECODES];
    unsigned num_decodes = 0;
    unsigned num_done = 0;
    unsigned displayIndex;
    unsigned i;
    NEXUS_VideoDecoderCrcMode crcMode = NEXUS_VideoDecoderCrcMode_eDefault;
    media_player_create_settings create_settings;
    media_player_start_settings start_settings;
    struct {
        bool dqt, mdqt, zero_delay_output_mode, iframe_as_rap, ignore_dpb_output_delay, ignore_num_reorder_frames;
    } extra_settings;
    float video_framerate = 0.0;

    memset(decoder, 0, sizeof(decoder));
    memset(&extra_settings, 0, sizeof(extra_settings));
    media_player_get_default_create_settings(&create_settings);
    media_player_get_default_start_settings(&start_settings);
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
        else if (!strcmp(argv[curarg], "-avd")) {
            crc_type = crc_type_avd;
        }
        else if (!strcmp(argv[curarg], "-mfd")) {
            crc_type = crc_type_mfd;
            crcMode = NEXUS_VideoDecoderCrcMode_eMfd;
        }
        else if (!strcmp(argv[curarg], "-only_mfd")) {
            crc_type = crc_type_mfd;
        }
        else if (!strcmp(argv[curarg], "-video") && argc>curarg+1) {
            start_settings.video.pid = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "-video_type") && argc>curarg+1) {
            start_settings.video.codec = lookup(g_videoCodecStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-mpeg_type") && argc>curarg+1) {
            start_settings.transportType = lookup(g_transportTypeStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-colorDepth") && curarg+1 < argc) {
            start_settings.video.colorDepth = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-dqt")) {
            extra_settings.dqt = true;
        }
        else if (!strcmp(argv[curarg], "-mdqt")) {
            extra_settings.mdqt = true;
        }
        else if (!strcmp(argv[curarg], "-zero_delay_output_mode")) {
            extra_settings.zero_delay_output_mode = true;
        }
        else if (!strcmp(argv[curarg], "-display_timestamp_mode")) {
            start_settings.video.timestampMode = NEXUS_VideoDecoderTimestampMode_eDisplay;
        }
        else if (!strcmp(argv[curarg], "-iframe_as_rap")) {
            extra_settings.iframe_as_rap = true;
        }
        else if (!strcmp(argv[curarg], "-ignore_dpb_output_delay")) {
            extra_settings.ignore_dpb_output_delay = true;
        }
        else if (!strcmp(argv[curarg], "-ignore_num_reorder_frames")) {
            extra_settings.ignore_num_reorder_frames = true;
        }
        else if (!strcmp(argv[curarg], "-video_cdb") && curarg+1 < argc) {
            start_settings.video.fifoSize = b_parse_size(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-video_itb") && curarg+1 < argc) {
            start_settings.video.itbFifoSize = b_parse_size(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-video_framerate") && curarg+1 < argc) {
            sscanf(argv[++curarg], "%f", &video_framerate);
        }
        else if (!strcmp(argv[curarg], "-max") && curarg+1 < argc) {
            int n = sscanf(argv[++curarg], "%u,%u", &create_settings.maxWidth, &create_settings.maxHeight);
            if (n != 2) {
                print_usage();
                return -1;
            }
        }
        else {
            if (num_decodes == MAX_DECODES) {
                BDBG_ERR(("too many mosaics"));
            }
            else {
                char *program = strchr(argv[curarg], ':');
                if (program) {
                    *program = 0; /* just modify argv storage */
                    decoder[num_decodes].program = atoi(++program);
                }
                decoder[num_decodes++].filename = argv[curarg];
            }
        }
        curarg++;
    }
    switch (crc_type) {
    case crc_type_none:
        print_usage();
        return -1;
    case crc_type_avd:
    case crc_type_mfd:
        if (num_decodes == 0) {
            print_usage();
            return -1;
        }
        break;
    default:
        if (num_decodes) {
            print_usage();
            return -1;
        }
        break;
    }

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    if (video_framerate) {
        NEXUS_LookupFrameRate(video_framerate * 1000, &start_settings.video.frameRate);
    }

    if (num_decodes) {
        if (num_decodes == 1) {
            player[0] = media_player_create(&create_settings);
            if (!player[0]) return -1;
            decoder[0].file = stdout;
        }
        else {
            rc = media_player_create_mosaics(player, num_decodes, &create_settings);
            if (rc) return -1;
        }

        for (i=0;i<num_decodes;i++) {
            NEXUS_SimpleVideoDecoderHandle videoDecoder = media_player_get_video_decoder(player[i]);

            {
                /* TEMP disable CC for mosaics */
                NEXUS_SimpleVideoDecoderClientSettings settings;
                NEXUS_SimpleVideoDecoder_GetClientSettings(videoDecoder, &settings);
                settings.closedCaptionRouting = false;
                NEXUS_SimpleVideoDecoder_SetClientSettings(videoDecoder, &settings);
            }

            if (extra_settings.dqt || extra_settings.mdqt) {
                NEXUS_VideoDecoderTrickState trickState;
                NEXUS_SimpleVideoDecoder_GetTrickState(videoDecoder, &trickState);
                if (extra_settings.dqt) {
                    trickState.dqtEnabled = NEXUS_VideoDecoderDqtMode_eSinglePass;
                }
                if (extra_settings.mdqt) {
                    trickState.dqtEnabled = NEXUS_VideoDecoderDqtMode_eMultiPass;
                }
                NEXUS_SimpleVideoDecoder_SetTrickState(videoDecoder, &trickState);
            }

            if (extra_settings.zero_delay_output_mode || extra_settings.iframe_as_rap ||
                extra_settings.ignore_dpb_output_delay || extra_settings.ignore_num_reorder_frames) {
                NEXUS_VideoDecoderExtendedSettings extendedSettings;
                NEXUS_SimpleVideoDecoder_GetExtendedSettings(videoDecoder, &extendedSettings);
                if (extra_settings.zero_delay_output_mode) {
                    extendedSettings.zeroDelayOutputMode = true;
                }
                if (extra_settings.iframe_as_rap) {
                    extendedSettings.treatIFrameAsRap = true;
                }
                if (extra_settings.ignore_dpb_output_delay) {
                    extendedSettings.ignoreDpbOutputDelaySyntax = true;
                }
                if (extra_settings.ignore_num_reorder_frames) {
                    extendedSettings.ignoreNumReorderFramesEqZero = true;
                }
                NEXUS_SimpleVideoDecoder_SetExtendedSettings(videoDecoder, &extendedSettings);
            }

            start_settings.stream_url = decoder[i].filename;
            start_settings.program = decoder[i].program;
            start_settings.loopMode = NEXUS_PlaybackLoopMode_ePause; /* once */
            start_settings.stcTrick = false;
            start_settings.crcMode = crcMode;
            start_settings.sync = NEXUS_SimpleStcChannelSyncMode_eOff;
            start_settings.audio.pid = 0; /* no audio */
            rc = media_player_start(player[i], &start_settings);
            if (rc) return rc;

            if (num_decodes > 1) {
                char buf[32];
                sprintf(buf, "crc%u.txt", i);
                decoder[i].file = fopen(buf, "w");
                if (!decoder[i].file) {
                    BDBG_ERR(("unable to open %s", buf));
                    return -1;
                }
                BDBG_LOG(("writing CRCs for %s, program %u to %s", decoder[i].filename, decoder[i].program, buf));
            }
        }
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
    case crc_type_mfd:
    case crc_type_avd:
        while (num_done < num_decodes) {
            unsigned j;
            bool any = false;
            for (j=0;j<num_decodes;j++) {
                NEXUS_PlaybackStatus status;
                NEXUS_SimpleVideoDecoderHandle videoDecoder;

                if (decoder[j].done) continue;
                videoDecoder = media_player_get_video_decoder(player[j]);

                if (crc_type == crc_type_avd) {
                    NEXUS_VideoDecoderCrc data[16];
                    unsigned num;
                    rc = NEXUS_SimpleVideoDecoder_GetCrcData(videoDecoder, data, sizeof(data)/sizeof(data[0]), &num);
                    BDBG_ASSERT(!rc);
                    if (num) {
                        unsigned i;
                        any = true;
                        for (i=0;i<num;i++) {
                            fprintf(decoder[j].file, "AVD CRC %x %x %x; %x %x %x\n", data[i].top.luma, data[i].top.cr, data[i].top.cb, data[i].bottom.luma, data[i].bottom.cr, data[i].bottom.cb);
                        }
                    }
                }
                else {
                    NEXUS_VideoInputCrcData data[16];
                    unsigned num;
                    rc = NEXUS_SimpleVideoDecoder_GetVideoInputCrcData(videoDecoder, data, sizeof(data)/sizeof(data[0]), &num);
                    BDBG_ASSERT(!rc);
                    if (num) {
                        unsigned i;
                        any = true;
                        for (i=0;i<num;i++) {
                            fprintf(decoder[j].file, "MFD CRC %u,%u %u,%u right=%u,%u, field=%c\n", data[i].idrPictureId, data[i].pictureOrderCount,
                                data[i].crc[0], data[i].crc[1], data[i].crc[3], data[i].crc[4], data[i].isField?'y':'n');
                        }
                    }
                }
                media_player_get_playback_status(player[j], &status);
                if (status.state == NEXUS_PlaybackState_ePaused) {
                    decoder[j].done = true;
                    num_done++;
                }
            }
            if (!any) BKNI_Sleep(10);
        }
        break;
    default:
        break;
    }

done:
    if (num_decodes) {
        for (i=0;i<num_decodes;i++) {
            media_player_stop(player[i]);
            if (num_decodes > 1) fclose(decoder[i].file);
        }
        if (num_decodes == 1) {
            media_player_destroy(player[0]);
        }
        else {
            media_player_destroy_mosaics(player, num_decodes);
        }
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
