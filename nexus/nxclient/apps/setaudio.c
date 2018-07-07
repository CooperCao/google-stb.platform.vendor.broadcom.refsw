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
 *
 * Module Description:
 *
 *****************************************************************************/
#if NEXUS_HAS_AUDIO
#include "nexus_platform_client.h"
#include "nxclient.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "bstd.h"
#include "bkni.h"
#include "namevalue.h"

BDBG_MODULE(setaudio);

#define NUM_VOL_STEPS 10
#define MIN_VOL 9000 /* Value is in 1/100th of a dB */
#define MAX_DOLBY_ENHANCER 16
#define MAX_DOLBY_LEVELER 10

const namevalue_t g_nxclientAudioOutputModeStrs[] = {
    {"none",         NxClient_AudioOutputMode_eNone},
    {"auto",         NxClient_AudioOutputMode_eAuto},
    {"pcm",          NxClient_AudioOutputMode_ePcm},
    {"multichannel", NxClient_AudioOutputMode_eMultichannelPcm},
    {"passthrough",  NxClient_AudioOutputMode_ePassthrough},
    {"transcode",    NxClient_AudioOutputMode_eTranscode},
    {NULL, 0}
};

static void print_usage(void)
{
    printf(
        "Usage: setaudio OPTIONS\n"
        "  -mute {on|off}\n"
        );
    print_list_option("hdmi", g_nxclientAudioOutputModeStrs);
    print_list_option("hdmi_channel_mode", g_audioChannelModeStrs);
    print_list_option("hdmi_transcode_codec", g_audioDdreCodecs);
    printf(
        "  -hdmi_delay MSEC\n"
        "  -hdmi_compressed_codec_default [codec]\n"
        "  -hdmi_compressed_codec_enable [codec]\n"
        "  -hdmi_compressed_codec_disable [codec]\n"
        );
    print_list_option("spdif", g_nxclientAudioOutputModeStrs);
    print_list_option("spdif_channel_mode", g_audioChannelModeStrs);
    printf(
        "  -spdif_delay MSEC\n"
        "  -spdif_compressed_codec_default [codec]\n"
        "  -spdif_compressed_codec_enable [codec]\n"
        "  -spdif_compressed_codec_disable [codec]\n"
        );
    print_list_option("dac_channel_mode", g_audioChannelModeStrs);
    printf(
        "  -dac_delay MSEC\n"
        );
    print_list_option("rfm_channel_mode", g_audioChannelModeStrs);
    printf(
        "  -rfm_delay MSEC\n"
        );
    printf(
        "  -vol {0..%d}\n",
        NUM_VOL_STEPS
        );
    printf(
        "  -avl {on|off}  \tauto volume level control\n"
        "  -truVolume {on|off}  \tenable SRS TruVolume stage\n"
        "  -dv258 {on|off}  \tenable DolbyVolume258 stage (requires nxserver -ms11)\n"
        );
    printf(
        "  -dv {on|off}  \tenable MS12 Dolby Volume Leveler (requires nxserver -ms12)\n"
        "  -dvIntelligentLoudness {on|off}  \tenable Dolby Volume Intelligent Loudness (requires Dolby Volume Leveler enabled)\n"
        "  -dvAmount {0..%d}  \t Controls the Amount of Volume Leveling (requires Dolby Volume Leveler enabled)\n",
        MAX_DOLBY_LEVELER
        );
    print_list_option("advancedTsmMode", g_audioAdvancedTsmModeStrs);
    printf(
        "  -dde {on|off}  \tenable MS12 Dolby Dialog Enhancer (requires nxserver -ms12)\n"
        "  -ddeEnhancerLevel {0..%d}  \t Controls the Amount of Content Enhancing (requires Dolby Dialog Enhancer enabled)\n"
        "  -ddeSuppressionLevel {0..%d}  \t Controls the Amount of Content Supressing (requires Dolby Volume Leveler enabled)\n",
        MAX_DOLBY_ENHANCER,
        MAX_DOLBY_ENHANCER
        );
    printf(
        "  -ms12EncoderFixedOutput {on|off}  \tenable MS12 Dolby Encoder Fixed Output Mode (requires nxserver -ms12)\n"
        "  -ms12EncoderFixedAtmos {on|off}  \tenable MS12 Dolby Encoder Fixed Atmos (requires nxserver -ms12)\n"
        );
}

#define CONVERT_TO_USER_VOL(vol,steps) (vol + MIN_VOL) * steps / MIN_VOL
#define CONVERT_TO_NEXUS_DECIBEL_VOL(vol,steps) -MIN_VOL + (vol * MIN_VOL / steps) /* Range of 0dB to -90dB */
static void print_settings(const NxClient_AudioSettings *pSettings, NxClient_AudioProcessingSettings *pAudioProcessingSettings)
{
    printf(
        "NxClient Audio Settings:\n"
        "dac   %s, %s, %d msec delay\n"
        "i2s0  %s, %s, %d msec delay\n"
        "i2s1  %s, %s, %d msec delay\n"
        "hdmi  %s, %s, %d msec delay\n"
        "spdif %s, %s, %d msec delay\n"
        "rfm   %s, %s, %d msec delay\n"
        "vol   %s%d/%d %s\n",

        lookup_name(g_nxclientAudioOutputModeStrs, pSettings->dac.outputMode),
        lookup_name(g_audioChannelModeStrs, pSettings->dac.channelMode),
        pSettings->dac.additionalDelay,

        lookup_name(g_nxclientAudioOutputModeStrs, pSettings->i2s[0].outputMode),
        lookup_name(g_audioChannelModeStrs, pSettings->i2s[0].channelMode),
        pSettings->i2s[0].additionalDelay,

        lookup_name(g_nxclientAudioOutputModeStrs, pSettings->i2s[1].outputMode),
        lookup_name(g_audioChannelModeStrs, pSettings->i2s[1].channelMode),
        pSettings->i2s[1].additionalDelay,

        lookup_name(g_nxclientAudioOutputModeStrs, pSettings->hdmi.outputMode),
        lookup_name(g_audioChannelModeStrs, pSettings->hdmi.channelMode),
        pSettings->hdmi.additionalDelay,

        lookup_name(g_nxclientAudioOutputModeStrs, pSettings->spdif.outputMode),
        lookup_name(g_audioChannelModeStrs, pSettings->spdif.channelMode),
        pSettings->spdif.additionalDelay,

        lookup_name(g_nxclientAudioOutputModeStrs, pSettings->rfm.outputMode),
        lookup_name(g_audioChannelModeStrs, pSettings->rfm.channelMode),
        pSettings->rfm.additionalDelay,

        pSettings->muted?"muted ":"",
        CONVERT_TO_USER_VOL(pSettings->leftVolume, NUM_VOL_STEPS),
        CONVERT_TO_USER_VOL(pSettings->rightVolume, NUM_VOL_STEPS),
        pSettings->volumeType == NEXUS_AudioVolumeType_eLinear?"linear":"decibel");
    printf(
        "avl                    %s\n"
        "truVolume              %s\n"
        "advancedTsmMode        %s\n"
        "dv258                  %s\n"
        "dv                     %s\n"
        "dvIntelligentLoudness  %s\n"
        "dvLevelAmount          %u\n"
        "dde                    %s\n"
        "ddeEnhancerLevel       %u\n"
        "ddeSuppressionLevel    %u\n"
        "ms12EncoderFixedOutput %s\n"
        "ms12EncoderFixedAtmos  %s\n",
        pAudioProcessingSettings->avl.enabled?"on":"off",
        pAudioProcessingSettings->truVolume.enabled?"on":"off",
        lookup_name(g_audioAdvancedTsmModeStrs, pAudioProcessingSettings->advancedTsm.mode),
        pAudioProcessingSettings->dolby.dolbyVolume258.enabled?"on":"off",
        pAudioProcessingSettings->dolby.dolbySettings.volumeLimiter.enableVolumeLimiting?"on":"off",
        pAudioProcessingSettings->dolby.dolbySettings.volumeLimiter.enableIntelligentLoudness?"on":"off",
        pAudioProcessingSettings->dolby.dolbySettings.volumeLimiter.volumeLimiterAmount,
        pAudioProcessingSettings->dolby.dolbySettings.dialogEnhancer.enableDialogEnhancer?"on":"off",
        pAudioProcessingSettings->dolby.dolbySettings.dialogEnhancer.dialogEnhancerLevel,
        pAudioProcessingSettings->dolby.dolbySettings.dialogEnhancer.contentSuppressionLevel,
        pAudioProcessingSettings->dolby.ddre.fixedEncoderFormat?"on":"off",
        pAudioProcessingSettings->dolby.ddre.fixedAtmosOutput?"on":"off");
}

static void print_status(void)
{
    NxClient_AudioStatus status;
    if (NxClient_GetAudioStatus(&status)) return;
    printf(
        "NxClient Audio Status:\n"
        "dac   %s, %s\n"
        "i2s0  %s, %s\n"
        "i2s1  %s, %s\n"
        "hdmi  %s, %s\n"
        "spdif %s, %s\n"
        "rfm   %s, %s\n",

        lookup_name(g_nxclientAudioOutputModeStrs, status.dac.outputMode),
        lookup_name(g_audioCodecStrs, status.dac.outputCodec),

        lookup_name(g_nxclientAudioOutputModeStrs, status.i2s[0].outputMode),
        lookup_name(g_audioCodecStrs, status.i2s[0].outputCodec),

        lookup_name(g_nxclientAudioOutputModeStrs, status.i2s[1].outputMode),
        lookup_name(g_audioCodecStrs, status.i2s[1].outputCodec),

        lookup_name(g_nxclientAudioOutputModeStrs, status.hdmi.outputMode),
        lookup_name(g_audioCodecStrs, status.hdmi.outputCodec),

        lookup_name(g_nxclientAudioOutputModeStrs, status.spdif.outputMode),
        lookup_name(g_audioCodecStrs, status.spdif.outputCodec),

        lookup_name(g_nxclientAudioOutputModeStrs, status.rfm.outputMode),
        lookup_name(g_audioCodecStrs, status.rfm.outputCodec)
        );
}

int main(int argc, char **argv)  {
    NxClient_JoinSettings joinSettings;
    NxClient_AudioSettings audioSettings;
    NxClient_AudioProcessingSettings audioProcessingSettings;
    bool change = false;
    bool processing_change = false;
    int curarg = 1;
    int rc;
    NEXUS_AudioCodec codec;

    NxClient_GetDefaultJoinSettings(&joinSettings);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;
    
    NxClient_GetAudioSettings(&audioSettings);
    NxClient_GetAudioProcessingSettings(&audioProcessingSettings);
    
    while (argc > curarg) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-mute") && argc>curarg+1) {
            change = true;
            audioSettings.muted = !strcmp(argv[++curarg], "on");
        }
        else if (!strcmp(argv[curarg], "-hdmi") && argc>curarg+1) {
            change = true;
            audioSettings.hdmi.outputMode = lookup(g_nxclientAudioOutputModeStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-hdmi_channel_mode") && curarg+1<argc) {
            change = true;
            audioSettings.hdmi.channelMode = lookup(g_audioChannelModeStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-hdmi_delay") && curarg+1<argc) {
            change = true;
            audioSettings.hdmi.additionalDelay = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-hdmi_transcode_codec") && curarg+1<argc) {
            change = true;
            audioSettings.hdmi.transcodeCodec = lookup(g_audioDdreCodecs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-hdmi_compressed_codec_default") && curarg+1<argc) {
            change = true;
            codec = lookup(g_audioCodecStrs, argv[++curarg]);
            audioSettings.hdmi.compressedOverride[codec] = NxClientAudioCodecSupport_eDefault;
        }
        else if (!strcmp(argv[curarg], "-hdmi_compressed_codec_enable") && curarg+1<argc) {
            change = true;
            codec = lookup(g_audioCodecStrs, argv[++curarg]);
            audioSettings.hdmi.compressedOverride[codec] = NxClientAudioCodecSupport_eEnabled;
        }
        else if (!strcmp(argv[curarg], "-hdmi_compressed_codec_disable") && curarg+1<argc) {
            change = true;
            codec = lookup(g_audioCodecStrs, argv[++curarg]);
            audioSettings.hdmi.compressedOverride[codec] = NxClientAudioCodecSupport_eDisabled;
        }
        else if (!strcmp(argv[curarg], "-spdif") && argc>curarg+1) {
            change = true;
            audioSettings.spdif.outputMode = lookup(g_nxclientAudioOutputModeStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-spdif_channel_mode") && curarg+1<argc) {
            change = true;
            audioSettings.spdif.channelMode = lookup(g_audioChannelModeStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-spdif_delay") && curarg+1<argc) {
            change = true;
            audioSettings.spdif.additionalDelay = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-spdif_compressed_codec_default") && curarg+1<argc) {
            change = true;
            codec = lookup(g_audioCodecStrs, argv[++curarg]);
            audioSettings.spdif.compressedOverride[codec] = NxClientAudioCodecSupport_eDefault;
        }
        else if (!strcmp(argv[curarg], "-spdif_compressed_codec_enable") && curarg+1<argc) {
            change = true;
            codec = lookup(g_audioCodecStrs, argv[++curarg]);
            audioSettings.spdif.compressedOverride[codec] = NxClientAudioCodecSupport_eEnabled;
        }
        else if (!strcmp(argv[curarg], "-spdif_compressed_codec_disable") && curarg+1<argc) {
            change = true;
            codec = lookup(g_audioCodecStrs, argv[++curarg]);
            audioSettings.spdif.compressedOverride[codec] = NxClientAudioCodecSupport_eDisabled;
        }
        else if (!strcmp(argv[curarg], "-dac_channel_mode") && curarg+1<argc) {
            change = true;
            audioSettings.dac.channelMode = lookup(g_audioChannelModeStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-dac_delay") && curarg+1<argc) {
            change = true;
            audioSettings.dac.additionalDelay = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-i2s0_channel_mode") && curarg+1<argc) {
            change = true;
            audioSettings.i2s[0].channelMode = lookup(g_audioChannelModeStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-i2s0_delay") && curarg+1<argc) {
            change = true;
            audioSettings.i2s[0].additionalDelay = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-i2s1_channel_mode") && curarg+1<argc) {
            change = true;
            audioSettings.i2s[1].channelMode = lookup(g_audioChannelModeStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-i2s1_delay") && curarg+1<argc) {
            change = true;
            audioSettings.i2s[1].additionalDelay = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-rfm_channel_mode") && curarg+1<argc) {
            change = true;
            audioSettings.rfm.channelMode = lookup(g_audioChannelModeStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-rfm_delay") && curarg+1<argc) {
            change = true;
            audioSettings.rfm.additionalDelay = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-vol") && curarg+1<argc) {
            unsigned vol;
            change = true;
            vol = atoi(argv[++curarg]);
            if (vol > NUM_VOL_STEPS) vol = NUM_VOL_STEPS;
            audioSettings.volumeType = NEXUS_AudioVolumeType_eDecibel;
            audioSettings.rightVolume = audioSettings.leftVolume = CONVERT_TO_NEXUS_DECIBEL_VOL(vol, NUM_VOL_STEPS);
        }
        else if (!strcmp(argv[curarg], "-avl") && curarg+1<argc) {
            processing_change = true;
            audioProcessingSettings.avl.enabled = !strcmp(argv[++curarg], "on");
        }
        else if (!strcmp(argv[curarg], "-truVolume") && curarg+1<argc) {
            processing_change = true;
            audioProcessingSettings.truVolume.enabled = !strcmp(argv[++curarg], "on");
        }
        else if (!strcmp(argv[curarg], "-advancedTsmMode") && curarg+1<argc) {
            processing_change = true;
            audioProcessingSettings.advancedTsm.mode = lookup(g_audioAdvancedTsmModeStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-dv258") && curarg+1<argc) {
            processing_change = true;
            audioProcessingSettings.dolby.dolbyVolume258.enabled = !strcmp(argv[++curarg], "on");
        }
        else if (!strcmp(argv[curarg], "-dv") && curarg+1<argc) {
            processing_change = true;
            audioProcessingSettings.dolby.dolbySettings.volumeLimiter.enableVolumeLimiting = !strcmp(argv[++curarg], "on");
            audioProcessingSettings.dolby.dolbySettings.enablePostProcessing = audioProcessingSettings.dolby.dolbySettings.volumeLimiter.enableVolumeLimiting ||
                audioProcessingSettings.dolby.dolbySettings.dialogEnhancer.enableDialogEnhancer;
        }
        else if (!strcmp(argv[curarg], "-dvIntelligentLoudness") && curarg+1<argc) {
            processing_change = true;
            audioProcessingSettings.dolby.dolbySettings.volumeLimiter.volumeLimiterAmount = !strcmp(argv[++curarg], "on");
        }
        else if (!strcmp(argv[curarg], "-dvLevelAmount") && curarg+1<argc) {
            unsigned amount;
            processing_change = true;
            amount = atoi(argv[++curarg]);
            if (amount > MAX_DOLBY_LEVELER) amount = MAX_DOLBY_LEVELER;
            audioProcessingSettings.dolby.dolbySettings.volumeLimiter.volumeLimiterAmount = amount;
        }
        else if (!strcmp(argv[curarg], "-dde") && curarg+1<argc) {
            processing_change = true;
            audioProcessingSettings.dolby.dolbySettings.dialogEnhancer.enableDialogEnhancer = !strcmp(argv[++curarg], "on");
            audioProcessingSettings.dolby.dolbySettings.enablePostProcessing = audioProcessingSettings.dolby.dolbySettings.dialogEnhancer.enableDialogEnhancer ||
                audioProcessingSettings.dolby.dolbySettings.volumeLimiter.enableVolumeLimiting;
        }
        else if (!strcmp(argv[curarg], "-ddeEnhancerLevel") && curarg+1<argc) {
            unsigned amount;
            processing_change = true;
            amount = atoi(argv[++curarg]);
            if (amount > MAX_DOLBY_ENHANCER) amount = MAX_DOLBY_ENHANCER;
            audioProcessingSettings.dolby.dolbySettings.dialogEnhancer.dialogEnhancerLevel = amount;
        }
        else if (!strcmp(argv[curarg], "-ddeSuppressionLevel") && curarg+1<argc) {
            unsigned amount;
            processing_change = true;
            amount = atoi(argv[++curarg]);
            if (amount > MAX_DOLBY_ENHANCER) amount = MAX_DOLBY_ENHANCER;
            audioProcessingSettings.dolby.dolbySettings.dialogEnhancer.contentSuppressionLevel = amount;
        }
        else if (!strcmp(argv[curarg], "-ms12EncoderFixedOutput") && argc>curarg+1) {
            processing_change = true;
            audioProcessingSettings.dolby.ddre.fixedEncoderFormat = !strcmp(argv[++curarg], "on");
        }
        else if (!strcmp(argv[curarg], "-ms12EncoderFixedAtmos") && argc>curarg+1) {
            processing_change = true;
            audioProcessingSettings.dolby.ddre.fixedAtmosOutput = !strcmp(argv[++curarg], "on");
        }
        else {
            print_usage();
            return -1;
        }
        curarg++;
    }

    if (change) {
        rc = NxClient_SetAudioSettings(&audioSettings);
        if (rc) {BERR_TRACE(rc); goto done;}
    }
    if (processing_change) {
        rc = NxClient_SetAudioProcessingSettings(&audioProcessingSettings);
        if (rc) {BERR_TRACE(rc); goto done;}
    }
    print_settings(&audioSettings, &audioProcessingSettings);
    printf("\n");
    print_status();
    
done:
    NxClient_Uninit();
    return 0;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform (needs audio)!\n");
    return 0;
}
#endif
