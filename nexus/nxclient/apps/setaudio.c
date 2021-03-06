/******************************************************************************
 *    (c)2010-2013 Broadcom Corporation
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

#define MAX_VOLUME 10

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
        );
    print_list_option("spdif", g_nxclientAudioOutputModeStrs);
    print_list_option("spdif_channel_mode", g_audioChannelModeStrs);
    printf(
        "  -spdif_delay MSEC\n"
        );
    print_list_option("dac_channel_mode", g_audioChannelModeStrs);
    printf(
        "  -dac_delay MSEC\n"
        );
    printf(
        "  -vol {0..%d}\n",
        MAX_VOLUME
        );
    printf(
        "  -avl {on|off}  \tauto volume level control\n"
        "  -truVolume {on|off}  \tenable SRS TruVolume stage\n"
        "  -dv258 {on|off}  \tenable DolbyVolume258 stage (requires nxserver -ms11)\n"
        "  -dv {on|off}  \tenable MS12 Dolby Volume Levelor (requires nxserver -ms12)\n"
        "  -dde {on|off}  \tenable MS12 Dolby Dialog Enhancer (requires nxserver -ms12)\n"
        );
}

#define CONVERT_TO_USER_VOL(vol,max_vol) (((vol) - NEXUS_AUDIO_VOLUME_LINEAR_MIN + 1) * (max_vol) / (NEXUS_AUDIO_VOLUME_LINEAR_NORMAL - NEXUS_AUDIO_VOLUME_LINEAR_MIN))
#define CONVERT_TO_NEXUS_LINEAR_VOL(vol,max_vol) (((vol) * (NEXUS_AUDIO_VOLUME_LINEAR_NORMAL - NEXUS_AUDIO_VOLUME_LINEAR_MIN) / (max_vol)) + NEXUS_AUDIO_VOLUME_LINEAR_MIN)

static void print_settings(const NxClient_AudioSettings *pSettings, NxClient_AudioProcessingSettings *pAudioProcessingSettings)
{
    printf(
        "NxClient Audio Settings:\n"
        "dac   %s, %s, %d msec delay\n"
        "hdmi  %s, %s, %d msec delay\n"
        "spdif %s, %s, %d msec delay\n"
        "vol   %s%d/%d %s\n",

        lookup_name(g_nxclientAudioOutputModeStrs, pSettings->dac.outputMode),
        lookup_name(g_audioChannelModeStrs, pSettings->dac.channelMode),
        pSettings->dac.additionalDelay,

        lookup_name(g_nxclientAudioOutputModeStrs, pSettings->hdmi.outputMode),
        lookup_name(g_audioChannelModeStrs, pSettings->hdmi.channelMode),
        pSettings->hdmi.additionalDelay,

        lookup_name(g_nxclientAudioOutputModeStrs, pSettings->spdif.outputMode),
        lookup_name(g_audioChannelModeStrs, pSettings->spdif.channelMode),
        pSettings->spdif.additionalDelay,

        pSettings->muted?"muted ":"",
        CONVERT_TO_USER_VOL(pSettings->leftVolume, MAX_VOLUME),
        CONVERT_TO_USER_VOL(pSettings->rightVolume, MAX_VOLUME),
        pSettings->volumeType == NEXUS_AudioVolumeType_eLinear?"linear":"decibel");
    printf(
        "avl       %s\n"
        "truVolume %s\n"
        "dv258     %s\n"
        "dv        %s\n"
        "dde       %s\n",
        pAudioProcessingSettings->avl.enabled?"on":"off",
        pAudioProcessingSettings->truVolume.enabled?"on":"off",
        pAudioProcessingSettings->dolby.dolbyVolume258.enabled?"on":"off",
        pAudioProcessingSettings->dolby.dolbySettings.volumeLimiter.enableVolumeLimiting?"on":"off",
        pAudioProcessingSettings->dolby.dolbySettings.dialogEnhancer.enableDialogEnhancer?"on":"off");
}

static void print_status(void)
{
    NxClient_AudioStatus status;
    if (NxClient_GetAudioStatus(&status)) return;
    printf(
        "NxClient Audio Status:\n"
        "dac   %s, %s\n"
        "hdmi  %s, %s\n"
        "spdif %s, %s\n",

        lookup_name(g_nxclientAudioOutputModeStrs, status.dac.outputMode),
        lookup_name(g_audioCodecStrs, status.dac.outputCodec),

        lookup_name(g_nxclientAudioOutputModeStrs, status.hdmi.outputMode),
        lookup_name(g_audioCodecStrs, status.hdmi.outputCodec),

        lookup_name(g_nxclientAudioOutputModeStrs, status.spdif.outputMode),
        lookup_name(g_audioCodecStrs, status.spdif.outputCodec)
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
        else if (!strcmp(argv[curarg], "-dac_channel_mode") && curarg+1<argc) {
            change = true;
            audioSettings.dac.channelMode = lookup(g_audioChannelModeStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-dac_delay") && curarg+1<argc) {
            change = true;
            audioSettings.dac.additionalDelay = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-vol") && curarg+1<argc) {
            unsigned vol;
            change = true;
            vol = atoi(argv[++curarg]);
            if (vol > MAX_VOLUME) vol = MAX_VOLUME;
            audioSettings.volumeType = NEXUS_AudioVolumeType_eLinear;
            audioSettings.rightVolume = audioSettings.leftVolume = CONVERT_TO_NEXUS_LINEAR_VOL(vol, MAX_VOLUME);
        }
        else if (!strcmp(argv[curarg], "-avl") && curarg+1<argc) {
            processing_change = true;
            audioProcessingSettings.avl.enabled = !strcmp(argv[++curarg], "on");
        }
        else if (!strcmp(argv[curarg], "-truVolume") && curarg+1<argc) {
            processing_change = true;
            audioProcessingSettings.truVolume.enabled = !strcmp(argv[++curarg], "on");
        }
        else if (!strcmp(argv[curarg], "-dv258") && curarg+1<argc) {
            processing_change = true;
            audioProcessingSettings.dolby.dolbyVolume258.enabled = !strcmp(argv[++curarg], "on");
        }
        else if (!strcmp(argv[curarg], "-dv") && curarg+1<argc) {
            processing_change = true;
            audioProcessingSettings.dolby.dolbySettings.volumeLimiter.enableVolumeLimiting = !strcmp(argv[++curarg], "on");
            audioProcessingSettings.dolby.dolbySettings.enablePostProcessing |= audioProcessingSettings.dolby.dolbySettings.volumeLimiter.enableVolumeLimiting ? true : false;
        }
        else if (!strcmp(argv[curarg], "-dde") && curarg+1<argc) {
            processing_change = true;
            audioProcessingSettings.dolby.dolbySettings.dialogEnhancer.enableDialogEnhancer = !strcmp(argv[++curarg], "on");
            audioProcessingSettings.dolby.dolbySettings.enablePostProcessing |= audioProcessingSettings.dolby.dolbySettings.dialogEnhancer.enableDialogEnhancer ? true : false;
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
