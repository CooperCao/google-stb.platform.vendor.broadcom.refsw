/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/

#include "standby.h"
#include "cmdline.h"

extern B_StandbyNexusHandles g_StandbyNexusHandles;
extern B_DeviceState g_DeviceState;
B_CmdOptions g_cmd_options;

extern int activeStandbyMode(void);
extern int passiveStandbyMode(void);
extern int deepStandbyMode(void);
extern int onMode(void);

struct {
    char *name; /* string for nexus enum */
    NEXUS_VideoFormat value; /* nexus enum */
} g_videoFormatStrs[] = {
    {"480i",      NEXUS_VideoFormat_eNtsc},
    {"576i",      NEXUS_VideoFormat_ePal},
    {"1080i",     NEXUS_VideoFormat_e1080i},
    {"1080i50",   NEXUS_VideoFormat_e1080i50hz},
    {"720p",      NEXUS_VideoFormat_e720p},
    {"720p24",    NEXUS_VideoFormat_e720p24hz},
    {"720p25",    NEXUS_VideoFormat_e720p25hz},
    {"720p30",    NEXUS_VideoFormat_e720p30hz},
    {"480p",      NEXUS_VideoFormat_e480p},
    {"576p",      NEXUS_VideoFormat_e576p},
    {"1080p",     NEXUS_VideoFormat_e1080p},
    {"1080p24",   NEXUS_VideoFormat_e1080p24hz},
    {"1080p25",   NEXUS_VideoFormat_e1080p25hz},
    {"1080p30",   NEXUS_VideoFormat_e1080p30hz},
    {"1080p50",   NEXUS_VideoFormat_e1080p50hz},
    {"3840x2160p24", NEXUS_VideoFormat_e3840x2160p24hz},
    {"3840x2160p25", NEXUS_VideoFormat_e3840x2160p25hz},
    {"3840x2160p30", NEXUS_VideoFormat_e3840x2160p30hz},
    {"3840x2160p50", NEXUS_VideoFormat_e3840x2160p50hz},
    {"3840x2160p60", NEXUS_VideoFormat_e3840x2160p60hz},
    {"4096x2160p24", NEXUS_VideoFormat_e4096x2160p24hz},
    {"4096x2160p25", NEXUS_VideoFormat_e4096x2160p25hz},
    {"4096x2160p30", NEXUS_VideoFormat_e4096x2160p30hz},
    {"4096x2160p50", NEXUS_VideoFormat_e4096x2160p50hz},
    {"4096x2160p60", NEXUS_VideoFormat_e4096x2160p60hz},
    {NULL, 0}
};

void *commands_thread(void *context)
{
    fd_set rfds;

    BSTD_UNUSED(context);

    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO, &rfds);

    for (g_DeviceState.exit_app=false;!g_DeviceState.exit_app;)
    {
        char buffer[256];
        char *buf;
        int retval;
        unsigned id;

        fflush(stdout); printf("standby>"); fflush(stdout);

        retval = select(1, &rfds, NULL, NULL, NULL);
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
            if (!strcmp(buf, "?") || !strcmp(buf, "h") || !strcmp(buf, "help")) {
                printf("Commands:\n");
                printf("  s1               :  S0 -> S1 -> S0 transition\n");
                printf("  s2               :  S0 -> S2 -> S0 transition\n");
                printf("  s3               :  S0 -> S3 -> S0 transition\n");
                printf("  s5               :  S0 -> S5 (never returns, reboots on wakeup)\n");
                printf("  sx               :  S0 -> S1 -> S2 -> S3 -> S0 transition\n");
                printf("  qam tune/untune  :  Tune or Untune Qam frontend\n");
                printf("  sat tune/untune  :  Tune or Untune Sat frontend\n");
                printf("  ofdm tune/untune :  Tune or Untune Ofdm frontend\n");
                printf("  strm start/stop  :  Start or Stop Streamer\n");
                printf("  play start/stop  :  Start or Stop playback\n");
                printf("  rec start/stop   :  Start or Stop record\n");
                printf("  enc start/stop   :  Start or Stop encode\n");
                printf("  pic start/stop   :  Run picture decode test\n");
                printf("  gfx start/stop   :  Run graphics 2D test\n");
                printf("  format           :  Set display format\n");
                printf("  hdmi on/off      :  Hdmi Output On/Off\n");
                printf("  component on/off :  Component Output On/Off\n");
                printf("  composite on/off :  Composite Output On/Off\n");
                printf("  rfm on/off       :  Rfm Output On/Off\n");
                printf("  dac on/off       :  Audio Dac On/Off\n");
                printf("  spdif on/off     :  Spdif On/Off\n");
                printf("  uhf on/off       :  Uhf Input On/Off\n");
                printf("  wakeups          :  Set wakeup devices\n");
                printf("  q                :  Quit\n");
            } else if (!strcmp(buf, "s1")) {
                g_DeviceState.power_mode = ePowerModeS1;
                g_DeviceState.timer_wake=true;
                BKNI_SetEvent(g_StandbyNexusHandles.event);
            } else if (!strcmp(buf, "s2")) {
                g_DeviceState.power_mode = ePowerModeS2;
                g_DeviceState.timer_wake=true;
                BKNI_SetEvent(g_StandbyNexusHandles.event);
            } else if (!strcmp(buf, "s3")) {
                g_DeviceState.power_mode = ePowerModeS3;
                g_DeviceState.timer_wake=true;
                BKNI_SetEvent(g_StandbyNexusHandles.event);
            } else if (!strcmp(buf, "s5")) {
                g_DeviceState.power_mode = ePowerModeS5;
                g_DeviceState.timer_wake=true;
                BKNI_SetEvent(g_StandbyNexusHandles.event);
            } else if (!strcmp(buf, "sx")) {
                g_cmd_options._auto = true;
                g_DeviceState.power_mode = ePowerModeS1;
                g_DeviceState.timer_wake=true;
                activeStandbyMode();
                g_DeviceState.power_mode = ePowerModeS2;
                g_DeviceState.timer_wake=true;
                passiveStandbyMode();
                g_DeviceState.power_mode = ePowerModeS3;
                g_DeviceState.timer_wake=true;
                deepStandbyMode();
                g_DeviceState.power_mode = ePowerModeS0;
                onMode();
                g_cmd_options._auto = false;
            } else if (!strncmp(buf, "qam", 3)) {
                buf+=4;
                printf("  Select Context 0 or 1 : "); scanf("%u", &id);
                if(id < g_DeviceState.num_contexts) {
                    if (!strcmp(buf, "tune")) {
                       if(!g_DeviceState.frontend_tuned[id]) {
                            printf("  Select Qam Mode 64/256/1024 : ");
                            scanf("%u", &g_DeviceState.frontend[id].qammode);
                            if (g_DeviceState.frontend[id].qammode == 64 || g_DeviceState.frontend[id].qammode == 256 || g_DeviceState.frontend[id].qammode == 1024) {
                                printf("  Enter Frequency  : ");
                                scanf("%u", &g_DeviceState.frontend[id].freq);
                                g_DeviceState.source[id] = eInputSourceQam;
                                start_live_context(id);
                            } else {
                                printf("Invalid QAM mode\n");
                            }
                        } else {
                            printf("Context %d already started\n", id);
                        }
                    } else if (!strcmp(buf, "untune")) {
                        if(g_DeviceState.frontend_tuned[id]) {
                            stop_live_context(id);
                        } else {
                            printf("Context %d not started\n", id);
                        }
                    } else
                        printf("Unknown command\n");
                } else
                    printf("Unsupported Context %d\n", id);
            } else if (!strncmp(buf, "sat", 3)) {
                buf+=4;
                printf("Select Context 0 or 1 : "); scanf("%u", &id);
                if(id < g_DeviceState.num_contexts) {
                    if (!strcmp(buf, "tune")) {
                        if(!g_DeviceState.frontend_tuned[id]) {
                            printf("  Enter Frequency  : ");
                            scanf("%u", &g_DeviceState.frontend[id].freq);
                            g_DeviceState.source[id] = eInputSourceSat;
                            start_live_context(id);
                        } else {
                            printf("Context %d already started\n", id);
                        }
                    } else if (!strcmp(buf, "untune")) {
                        if(g_DeviceState.frontend_tuned[id]) {
                            stop_live_context(id);
                        } else {
                            printf("Context %d not started\n", id);
                        }
                    } else
                        printf("Unknown command\n");
                } else
                    printf("Unsupported Context %d\n", id);
            } else if (!strncmp(buf, "ofdm", 4)) {
                buf+=5;
                printf("Select Context 0 or 1 : "); scanf("%u", &id);
                if(id < g_DeviceState.num_contexts) {
                    if (!strcmp(buf, "tune")) {
                        if(!g_DeviceState.frontend_tuned[id]) {
                            printf("  Select Ofdm Mode\n"
                                   "  0. Dvbt\n"
                                   "  1. Dvbt2\n"
                                   "  2. Dvbc2\n"
                                   "  3. Isdbt\n");
                            scanf("%u", &g_DeviceState.frontend[id].ofdmmode);
                            if (g_DeviceState.frontend[id].ofdmmode <= 3) {
                                printf("  Enter Frequency  : ");
                                scanf("%u", &g_DeviceState.frontend[id].freq);
                                g_DeviceState.source[id] = eInputSourceOfdm;
                                start_live_context(id);
                            } else {
                                printf("Invalid OFDM mode\n");
                            }
                        } else {
                            printf("Context %d already started\n", id);
                        }
                    } else if (!strcmp(buf, "untune")) {
                        if(g_DeviceState.frontend_tuned[id]) {
                            stop_live_context(id);
                        } else {
                            printf("Context %d not started\n", id);
                        }
                    } else
                        printf("Unknown command\n");
                } else
                    printf("Unsupported Context %d\n", id);
            } else if (!strncmp(buf, "strm", 4)) {
                buf+=5;
                printf("  Select Context 0 or 1 : "); scanf("%u", &id);
                if(id < g_DeviceState.num_contexts) {
                    if (!strcmp(buf, "start")) {
                        if(!g_DeviceState.decode_started[id]) {
                            g_DeviceState.source[id] = eInputSourceStreamer;
                            start_live_context(id);
                        } else {
                            printf("Context %d already started\n", id);
                        }
                    } else if (!strcmp(buf, "stop")) {
                        if(g_DeviceState.decode_started[id]) {
                            stop_live_context(id);
                        } else {
                            printf("Context %d not started\n", id);
                        }
                    } else
                        printf("Unknown command\n");
                } else
                    printf("Unsupported Context %d\n", id);
            } else if (!strncmp(buf, "play", 4)) {
                buf+=5;
                printf("  Select Context 0 or 1 : "); scanf("%u", &id);
                if(id < g_DeviceState.num_contexts) {
                    if (!strcmp(buf, "start")) {
                        if(!g_DeviceState.playback_started[id]) {
                            char filename[256];
                            printf("  Enter File name :  ");
                            scanf("%s", filename);
                            g_DeviceState.source[id] = eInputSourceFile;
                            g_DeviceState.playfile[id] = filename;
                            start_play_context(id);
                        } else
                            printf("Context %d already started\n", id);
                    } else if (!strcmp(buf, "stop")) {
                        if(g_DeviceState.playback_started[id]) {
                            stop_play_context(id);
                        } else {
                            printf("Context %d not started\n", id);
                        }
                    }
                    else
                        printf("Unknown command\n");
                } else
                    printf("Unsupported Context %d\n", id);
            } else if (!strncmp(buf, "rec", 3)) {
                buf+=4;
                printf("  Select Context 0 or 1 : "); scanf("%u", &id);
                if(id < g_DeviceState.num_contexts) {
                    if (!strcmp(buf, "start")) {
                        if(!g_DeviceState.record_started[id])
                            record_start(id);
                        else
                            printf("Context %d already started\n", id);
                    } else if (!strcmp(buf, "stop")) {
                        if(g_DeviceState.record_started[id])
                            record_stop(id);
                        else
                            printf("Context %d not started\n", id);
                    } else
                        printf("Unknown command\n");
                } else
                    printf("Unsupported Context %d\n", id);
            } else if (!strncmp(buf, "enc", 3)) {
                buf+=4;
                if (!strcmp(buf, "start")) {
                    if(!g_DeviceState.encode_started) {
                        encoder_open(0);
                        encode_start(0);
                    } else
                        printf("Encode already started\n");
                } else if (!strcmp(buf, "stop")) {
                    if(g_DeviceState.encode_started) {
                        encode_stop(0);
                        encoder_close(0);
                    } else
                        printf("Encode not started\n");
                } else
                    printf("Unknown command\n");
            } else if (!strncmp(buf, "pic", 3)) {
                buf+=4;
                if (!strcmp(buf, "start")) {
                    char filename[256];
                    printf("  Enter File name :  ");
                    scanf("%s", filename);
                    g_DeviceState.picfile = filename;
                    picture_decoder_open();
                    picture_decode_start();
                    graphics2d_open();
                    picture_decode_display();
                } else if (!strcmp(buf, "stop")) {
                    picture_decode_stop();
                    picture_decoder_close();
                } else
                    printf("Unknown command\n");
            } else if (!strncmp(buf, "gfx", 3)) {
                buf+=4;
                if (!strcmp(buf, "start")) {
                    if(!g_DeviceState.graphics2d_started) {
                        graphics2d_open();
                        graphics2d_setup();
                        graphics2d_start();
                    } else
                        printf("Graphics already started\n");
                } else if (!strcmp(buf, "stop")) {
                    if(g_DeviceState.graphics2d_started) {
                        graphics2d_stop();
                        graphics2d_destroy();
                        graphics2d_close();
                    } else
                        printf("Graphics not started\n");
                } else
                    printf("Unknown command\n");
            } else if (!strncmp(buf, "format", 6)) {
                NEXUS_DisplaySettings displaySettings;
                char format[32];
                NEXUS_Display_GetSettings(g_StandbyNexusHandles.displayHD, &displaySettings);
                for (id=0;g_videoFormatStrs[id].name;id++) {
                    if(g_videoFormatStrs[id].value == displaySettings.format)
                        strcpy(format, g_videoFormatStrs[id].name);
                    printf("  %s\n", g_videoFormatStrs[id].name);
                }
                printf("  Current display format    : %s\n", format);
                printf("  Select new display format : ");
                scanf("%s", format);
                printf("  Changing display format to %s\n", format);
                for (id=0;g_videoFormatStrs[id].name;id++) {
                    if(!strcmp(format, g_videoFormatStrs[id].name)) {
                        displaySettings.format = g_videoFormatStrs[id].value;
                        break;
                    }
                }
                NEXUS_Display_SetSettings(g_StandbyNexusHandles.displayHD, &displaySettings);
            } else if (!strncmp(buf, "hdmi", 4)) {
                buf+=5;
                if(!strcmp(buf, "on"))
                    add_hdmi_output();
                else if (!strcmp(buf, "off"))
                    remove_hdmi_output();
                else
                    printf("Unknown command\n");
            } else if (!strncmp(buf, "component", 9)) {
                buf+=10;
                if(!strcmp(buf, "on"))
                    add_component_output();
                else if (!strcmp(buf, "off"))
                    remove_component_output();
                else
                    printf("Unknown command\n");
            } else if (!strncmp(buf, "composite", 9)) {
                buf+=10;
                if(!strcmp(buf, "on"))
                    add_composite_output();
                else if (!strcmp(buf, "off"))
                    remove_composite_output();
                else
                    printf("Unknown command\n");
            } else if (!strncmp(buf, "rfm", 3)) {
                buf+=4;
                if(!strcmp(buf, "on"))
                    add_rfm_output();
                else if (!strcmp(buf, "off"))
                    remove_rfm_output();
                else
                    printf("Unknown command\n");
            } else if (!strncmp(buf, "dac", 3)) {
                buf+=4;
                printf("  Select Context 0 or 1 : "); scanf("%u", &id);
                if(!strcmp(buf, "on"))
                    add_dac_output(id);
                else if (!strcmp(buf, "off"))
                    remove_dac_output(id);
                else
                    printf("Unknown command\n");
            } else if (!strncmp(buf, "spdif", 5)) {
                buf+=6;
                printf("  Select Context 0 or 1 : "); scanf("%u", &id);
                if(!strcmp(buf, "on"))
                    add_spdif_output(id);
                else if (!strcmp(buf, "off"))
                    remove_spdif_output(id);
                else
                    printf("Unknown command\n");
            } else if (!strncmp(buf, "uhf", 3)) {
                buf+=4;
                if(!strcmp(buf, "on"))
                    uhf_open();
                else if (!strcmp(buf, "off"))
                    uhf_close();
                else
                    printf("Unknown command\n");
            } else if (!strncmp(buf, "kpd", 3)) {
                buf+=4;
                if(!strcmp(buf, "on"))
                    keypad_open();
                else if (!strcmp(buf, "off"))
                    keypad_close();
                else
                    printf("Unknown command\n");
            } else if (!strncmp(buf, "wakeups", 7)) {
                do {
                    printf("  1. IR       %s\n", g_cmd_options.ir_wakeup?"On":"Off");
                    printf("  2. UHF      %s\n", g_cmd_options.uhf_wakeup?"On":"Off");
                    printf("  3. XPT      %s\n", g_cmd_options.xpt_wakeup?"On":"Off");
                    printf("  4. CEC      %s\n", g_cmd_options.cec_wakeup?"On":"Off");
                    printf("  5. GPIO     %s\n", g_cmd_options.gpio_wakeup?"On":"Off");
                    printf("  6. KPD      %s\n", g_cmd_options.kpd_wakeup?"On":"Off");
                    printf("  7. ETH WOL  %s\n", g_cmd_options.eth_wol_wakeup?"On":"Off");
                    printf("  8. MOCA WOL %s\n", g_cmd_options.moca_wol_wakeup?"On":"Off");
                    printf("  9. Timeout  %u\n", g_cmd_options.timeout);
                    printf("  Enter Wakeup id to toggle. '0' to exit : ");
                    scanf("%u", &id);
                    if(id == 1)
                        g_cmd_options.ir_wakeup ^= 1;
                    if(id == 2)
                        g_cmd_options.uhf_wakeup ^= 1;
                    if(id == 3)
                        g_cmd_options.xpt_wakeup ^= 1;
                    if(id == 4)
                        g_cmd_options.cec_wakeup ^= 1;
                    if(id == 5)
                        g_cmd_options.gpio_wakeup ^= 1;
                    if(id == 6)
                        g_cmd_options.kpd_wakeup ^= 1;
                    if(id == 7)
                        g_cmd_options.eth_wol_wakeup ^= 1;
                    if(id == 8)
                        g_cmd_options.moca_wol_wakeup ^= 1;
                    if(id == 9) {
                        unsigned timeout;
                        printf("  Enter Timeout : ");
                        scanf("%u", &timeout);
                        g_cmd_options.timeout = timeout;
                    }
                } while (id);
            } else if (!strcmp(buf, "q")) {
                g_DeviceState.exit_app = true;
                BKNI_SetEvent(g_StandbyNexusHandles.event);
                break;
            } else {
                printf("unknown command: '%s' (use '?' for list)\n", buf);
            }
        } while ((buf = strtok(NULL, ";")));
    }

    return NULL;
}

static void print_usage(void)
{
    printf("Usage: nexus standby [-timeout SECONDS]\n");
    printf("\n");
    printf("-timeout SECONDS   sets the standby timeout. 0 means no timeout.\n"  );
    printf("-ir                1 : enable IR wake up;  0 : disable IR wakeup; default is 1.\n");
    printf("-uhf               1 : enable UHF wake up; 0 : disable UHF wakeup; default is 1.\n");
    printf("-xpt               1 : enable XPT wake up; 0 : disable XPT wakeup; default is 1.\n");
    printf("-cec               1 : enable CEC wake up; 0 : disable CEC wakeup; default is 1.\n");
    printf("-gpio              1 : enable GPIO wake up; 0 : disable GPIO wakeup; default is 1.\n");
    printf("-kpd               1 : enable Keypad wake up; 0 : disable Keypad wakeup; default is 1.\n");
    printf("-ethwol            1 : enable ETH WOL wake up; 0 : disable ETH WOL wakeup; default is 1.\n");
    printf("-mocawol           1 : enable MOCA WOL wake up; 0 : disable MOCA WOL wakeup; default is 1.\n");
    printf("-ethoff            1 : Eth  is powered down in S1; 0 : Eth  is powered up in S1; default is 0.\n");
    printf("-mocaoff           1 : Moca is powered down in S1; 0 : Moca is powered up in S1; default is 0.\n");
    printf("-sataoff           1 : SATA is powered down in S1; 0 : SATA is powered up in S1; default is 1.\n");
    printf("-qam               Use qam input. Default is playback.\n");
    printf("-qammode           Qam mode to use eg; 64, 256 or 1024\n");
    printf("-ofdm              Use ofdm input. Default is playback.\n");
    printf("-ofdmmode          Ofdm mode to use eg; DVBT(0), DVBT2(1), DVBC2(2) or ISDBT(3)\n");
    printf("-sat               Use satellite input. Default is playback.\n");
    printf("-freq              Qam, Ofdm or Sat frequency to tune in MHz\n");
    printf("-play              Specify the file to playback. Default is cnnticker.mpg\n");
    printf("-streamer          Use streamer input. Default is playback.\n");
    printf("-auto              Automatically transition through all power states.\n");
    printf("-autostate         Power state to transtion in auto mode. Default is all states. 1 for S1 only, 2 for S2 only, 3 for S3 only. 0 for all states\n");
    printf("-flags             Standby Flags. Used for debug. e.g. -flags 0x1\n");
}

int parse_cmdline_args(int argc, char **argv)
{
    int curarg = 0;

    while (++curarg < argc) {
        if (!strcmp(argv[curarg], "--help") || !strcmp(argv[curarg], "-h")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-timeout") && curarg+1 < argc) {
            g_cmd_options.timeout = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-ir") && curarg+1 < argc) {
            g_cmd_options.ir_wakeup = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-uhf") && curarg+1 < argc) {
            g_cmd_options.uhf_wakeup = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-xpt") && curarg+1 < argc) {
            g_cmd_options.xpt_wakeup = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-cec") && curarg+1 < argc) {
            g_cmd_options.cec_wakeup = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-gpio") && curarg+1 < argc) {
            g_cmd_options.gpio_wakeup = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-kpd") && curarg+1 < argc) {
            g_cmd_options.kpd_wakeup = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-ethwol") && curarg+1 < argc) {
            g_cmd_options.eth_wol_wakeup = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-mocawol") && curarg+1 < argc) {
            g_cmd_options.moca_wol_wakeup = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-ethoff") && curarg+1 < argc) {
            g_cmd_options.ethoff = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-mocaoff") && curarg+1 < argc) {
            g_cmd_options.mocaoff = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-sataoff") && curarg+1 < argc) {
            g_cmd_options.sataoff = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-flags") && curarg+1 < argc) {
            g_cmd_options.standby_flags = strtol(argv[++curarg], NULL, 0);
        }
        else if(!strcmp(argv[curarg], "-qam")){
            g_DeviceState.source[0] = eInputSourceQam;
        }
        else if(!strcmp(argv[curarg], "-sat")){
            g_DeviceState.source[0] = eInputSourceSat;
        }
        else if(!strcmp(argv[curarg], "-ofdm")){
            g_DeviceState.source[0] = eInputSourceOfdm;
        }
        else if (!strcmp(argv[curarg], "-freq") && curarg+1 < argc) {
            g_DeviceState.frontend[0].freq = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-qammode") && curarg+1 < argc) {
            g_DeviceState.frontend[0].qammode = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-satmode") && curarg+1 < argc) {
            g_DeviceState.frontend[0].satmode = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-ofdmmode") && curarg+1 < argc) {
            g_DeviceState.frontend[0].ofdmmode = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-adc") && curarg+1 < argc) {
            g_DeviceState.frontend[0].adc = atoi(argv[++curarg]);
        }
        else if(!strcmp(argv[curarg], "-streamer")){
            g_DeviceState.source[0] = eInputSourceStreamer;
        }
        else if(!strcmp(argv[curarg], "-play") && curarg+1 < argc){
            g_DeviceState.source[0] = eInputSourceFile;
            g_DeviceState.playfile[0] = argv[++curarg];
        }
        else if(!strcmp(argv[curarg], "-openfe")){
            g_DeviceState.openfe = true;
        }
        else if (!strcmp(argv[curarg], "-auto")) {
            g_cmd_options._auto = true;
        }
        else if(!strcmp(argv[curarg], "-autostate") && curarg+1 < argc){
            g_cmd_options._autostate = atoi(argv[++curarg]);
        }
    }

    return 1;
}
