/***************************************************************************
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
 **************************************************************************/
#include "nxclient.h"
#include "nexus_playback.h"
#include "nexus_simple_audio_decoder.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "media_probe.h"
#include "namevalue.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

BDBG_MODULE(dolby_ms12_cert);

typedef enum decoderIdx{
    decoderIdx_pri,
    decoderIdx_sec,
    decoderIdx_app,
    decoderIdx_sfx,
    decoderIdx_max
} decoderIdx;

typedef enum ieqProfile{
    ieqProfile_open,
    ieqProfile_rich,
    ieqProfile_focused,
    ieqProfile_max
} ieqProfile;

typedef struct config {
    NEXUS_FilePlayHandle file;
    const char * filename;
    NEXUS_AudioCodec audioCodec;
    unsigned audioPid;
    NEXUS_TransportType transportType;
    bool started;
    NEXUS_SimpleAudioDecoderStartSettings audioProgram;
    NEXUS_SimpleAudioDecoderHandle audioDecoder;
    NEXUS_SimpleStcChannelHandle stcChannel;
    NEXUS_PidChannelHandle audioPidChannel;
    unsigned connectId;
    NxClient_AllocResults allocResults;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
} config;

typedef struct dapConfiguration {
    bool autoPilotEnabled; /* overides other settings */
    bool dapEnabled;
    bool volumeLevelerEnabled;
    unsigned volumeLevelerAmount;
    bool dialogEnhancerEnabled;
    unsigned dialogEnhancerAmount;
    bool intelligentEqualizerEnabled;
    ieqProfile ieqPreset;
    int multiStreamBalance;
} dapConfiguration;

typedef struct intelligentEqProfile {
    unsigned frequency;         /* Center frequency of each eq band, specified in Hz. Valid values [20 - 20,000] */
    int gain;                   /* Gain in 0.0625 dB steps, achieving +/- 30dB of gain. Valid values [-480 - 480] */
} intelligentEqProfile;

intelligentEqProfile openProfile[20] = {
    { 65, 117 },
    { 136, 133 },
    { 223, 188 },
    { 332, 176 },
    { 467, 141 },
    { 634, 149 },
    { 841, 175 },
    { 1098, 185 },
    { 1416, 185 },
    { 1812, 200 },
    { 2302, 236 },
    { 2909, 242 },
    { 3663, 228 },
    { 4598, 213 },
    { 5756, 182 },
    { 7194, 132 },
    { 8976, 110 },
    { 11186, 68 },
    { 13927, -27 },
    { 17326, -240 }
};

intelligentEqProfile focusedProfile[20] = {
    { 65, -419 },
    { 136, -112 },
    { 223, 75 },
    { 332, 116 },
    { 467, 113 },
    { 634, 160 },
    { 841, 165 },
    { 1098, 80 },
    { 1416, 61 },
    { 1812, 79 },
    { 2302, 98 },
    { 2909, 121 },
    { 3663, 64 },
    { 4598, 70 },
    { 5756, 44 },
    { 7194, -71 },
    { 8976, -33 },
    { 11186, -100 },
    { 13927, -238 },
    { 17326, -411 }
};

intelligentEqProfile richProfile[20] = {
    { 65, 67 },
    { 136, 95 },
    { 223, 172 },
    { 332, 163 },
    { 467, 168 },
    { 634, 201 },
    { 841, 189 },
    { 1098, 242 },
    { 1416, 196 },
    { 1812, 221 },
    { 2302, 192 },
    { 2909, 186 },
    { 3663, 168 },
    { 4598, 139 },
    { 5756, 102 },
    { 7194, 57 },
    { 8976, 35 },
    { 11186, 9 },
    { 13927, -55 },
    { 17326, -235 }
};

static void print_usage(void);
static void default_dap_config(dapConfiguration *dapConfig);
static void apply_dap_config(dapConfiguration *dapConfig);
static const char *dolby_ms12_cert_decoder_name(const decoderIdx idx);


int main(int argc, const char **argv)  {
    int curarg = 1;
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings allocSettings;
    NEXUS_SimpleVideoDecoderHandle videoDecoder;
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
    NEXUS_VideoCodec videoCodec = NEXUS_VideoCodec_eMax;
    unsigned videoPid = 0x1fff;
    NEXUS_PidChannelHandle videoPidChannel=NULL;
    NxClient_ConnectSettings connectSettings;
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_AudioDecoderStereoDownmixMode downmixMode = NEXUS_AudioDecoderStereoDownmixMode_eAuto;
    NEXUS_AudioDecoderCodecSettings codecSettings;
    NEXUS_Error rc;
    unsigned i;
    bool done = false;
    bool audioDescription = false;
    bool requiresDyanmic = false;
    NEXUS_AudioDecoderDolbyDrcMode stereoDrc = NEXUS_AudioDecoderDolbyDrcMode_eMax;
    NEXUS_AudioDecoderDolbyDrcMode multiDrc = NEXUS_AudioDecoderDolbyDrcMode_eMax;
    config configs[decoderIdx_max];
    bool preferrLanguageOverType = false;
    char preferredLangauge[8];
    NEXUS_AudioAc4AssociateType ac4associateType = NEXUS_AudioAc4AssociateType_eNotSpecified;
    bool ac4mixing = false;
    dapConfiguration dapConfig;

    /* Prep config file */
    memset(configs, 0, sizeof(configs));
    configs[decoderIdx_pri].audioPid = configs[decoderIdx_sec].audioPid =
        configs[decoderIdx_app].audioPid = configs[decoderIdx_sfx].audioPid = 0x1fff;

    configs[decoderIdx_pri].audioCodec = configs[decoderIdx_sec].audioCodec =
        configs[decoderIdx_app].audioCodec = configs[decoderIdx_sfx].audioCodec = NEXUS_AudioCodec_eMax;

    configs[decoderIdx_pri].transportType = configs[decoderIdx_sec].transportType =
        configs[decoderIdx_app].transportType = configs[decoderIdx_sfx].transportType = NEXUS_TransportType_eMax;

    default_dap_config(&dapConfig);


    while (curarg < argc) {
        if (!strcmp("--help", argv[curarg]) ||
            !strcmp("-h", argv[curarg]) ||
            !strcmp("-?", argv[curarg])) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-primary") && argc>curarg+1) {
            configs[decoderIdx_pri].filename = argv[++curarg];
        }
        else if (!strcmp(argv[curarg], "-secondary") && argc>curarg+1) {
            configs[decoderIdx_sec].filename = argv[++curarg];
        }
        else if (!strcmp(argv[curarg], "-app") && argc>curarg+1) {
            configs[decoderIdx_app].filename = argv[++curarg];
        }
        else if (!strcmp(argv[curarg], "-sfx") && argc>curarg+1) {
            configs[decoderIdx_sfx].filename = argv[++curarg];
        }
        else if (!strcmp(argv[curarg], "-video") && argc>curarg+1) {
            videoPid = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "-video_type") && argc>curarg+1) {
            videoCodec = lookup(g_videoCodecStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-pri_audio") && argc>curarg+1) {
            configs[decoderIdx_pri].audioPid = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "-pri_audio_type") && argc>curarg+1) {
            configs[decoderIdx_pri].audioCodec = lookup(g_audioCodecStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-pri_mpeg_type") && argc>curarg+1) {
            configs[decoderIdx_pri].transportType = lookup(g_transportTypeStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-sec_audio") && argc>curarg+1) {
            configs[decoderIdx_sec].audioPid = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "-sec_audio_type") && argc>curarg+1) {
            configs[decoderIdx_sec].audioCodec = lookup(g_audioCodecStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-sec_mpeg_type") && argc>curarg+1) {
            configs[decoderIdx_sec].transportType = lookup(g_transportTypeStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-stereo") && curarg+1 < argc) {
            ++curarg;
            if (!strcmp("loro", argv[curarg])) {
                downmixMode = NEXUS_AudioDecoderStereoDownmixMode_eLoRo;
            }
            else if (!strcmp("ltrt", argv[curarg])) {
                downmixMode = NEXUS_AudioDecoderStereoDownmixMode_eLtRt;
            }
            else {
                print_usage();
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-autoPilot")) {
            dapConfig.autoPilotEnabled = true;
        }
        else if (!strcmp(argv[curarg], "-dap")) {
            dapConfig.dapEnabled = true;
        }
        else if (!strcmp(argv[curarg], "-volume_leveler") && argc>curarg+1) {
            dapConfig.volumeLevelerAmount = strtoul(argv[++curarg], NULL, 10);
            if (dapConfig.volumeLevelerAmount > 10) {
                BDBG_ERR(("Invalid volume_leveler amount only 0-10 supported"));
                print_usage();
                return -1;
            }
            dapConfig.volumeLevelerEnabled = true;
        }
        else if (!strcmp(argv[curarg], "-dialog_enhancer") && argc>curarg+1) {
            dapConfig.dialogEnhancerAmount = strtoul(argv[++curarg], NULL, 10);
            if (dapConfig.dialogEnhancerAmount > 16) {
                BDBG_ERR(("Invalid dialog_enhancer amount only 0-16 supported"));
                print_usage();
                return -1;
            }
            dapConfig.dialogEnhancerEnabled = true;
        }
        else if (!strcmp(argv[curarg], "-ieq") && curarg+1 < argc) {
            ++curarg;
            if (!strcmp("open", argv[curarg])) {
                dapConfig.ieqPreset = ieqProfile_open;
            }
            else if (!strcmp("rich", argv[curarg])) {
                dapConfig.ieqPreset = ieqProfile_rich;
            }
            else if (!strcmp("focused", argv[curarg])) {
                dapConfig.ieqPreset = ieqProfile_focused;
            }
            else {
                print_usage();
                return -1;
            }
            dapConfig.intelligentEqualizerEnabled = true;
        }
        else if (!strcmp(argv[curarg], "-dynamic")) {
            requiresDyanmic = true;
        }
        else if (!strcmp(argv[curarg], "-stereoDRC") && curarg+1 < argc) {
            ++curarg;
            if (!strcmp("line", argv[curarg])) {
                stereoDrc = NEXUS_AudioDecoderDolbyDrcMode_eLine;
            }
            else if (!strcmp("rf", argv[curarg])) {
                stereoDrc = NEXUS_AudioDecoderDolbyDrcMode_eRf;
            }
            else {
                print_usage();
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-multiDRC") && curarg+1 < argc) {
            ++curarg;
            if (!strcmp("line", argv[curarg])) {
                multiDrc = NEXUS_AudioDecoderDolbyDrcMode_eLine;
            }
            else if (!strcmp("rf", argv[curarg])) {
                multiDrc = NEXUS_AudioDecoderDolbyDrcMode_eRf;
            }
            else {
                print_usage();
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-ac4Language") && curarg+1 < argc) {
            ++curarg;
            strcpy(preferredLangauge, argv[curarg]);
            preferrLanguageOverType = true;
        }
        else if (!strcmp(argv[curarg], "-ac4AssociateMixing") && curarg+1 < argc) {
            ++curarg;
            ac4mixing = true;
            if (!strcmp("notSpecified", argv[curarg])) {
                ac4associateType = NEXUS_AudioAc4AssociateType_eNotSpecified;
            }
            else if (!strcmp("visual", argv[curarg])) {
                ac4associateType = NEXUS_AudioAc4AssociateType_eVisuallyImpaired;
            }
            else if (!strcmp("hearing", argv[curarg])) {
                ac4associateType = NEXUS_AudioAc4AssociateType_eHearingImpaired;
            }
            else if (!strcmp("commentary", argv[curarg])) {
                ac4associateType = NEXUS_AudioAc4AssociateType_eCommentary;
            }
            else {
                print_usage();
                return -1;
            }
        }
        else
        {
            print_usage();
            return -1;
        }
        curarg++;
    }

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    apply_dap_config(&dapConfig);

    for (i = 0; i < decoderIdx_max; i++) {
        if (configs[i].filename) {
            struct probe_results probe_results;
            struct probe_request probe_request;

            probe_media_get_default_request(&probe_request);
            probe_request.streamname = configs[i].filename;
            printf("Probing %s\n", dolby_ms12_cert_decoder_name(i));
            rc = probe_media_request(&probe_request, &probe_results);
            if (rc) {
                BDBG_ERR(("media probe can't parse '%s'", configs[i].filename));
                return -1;
            }
            /* override probe */
            if (i == decoderIdx_pri) {
                if (videoCodec == NEXUS_VideoCodec_eMax) {
                    videoCodec = probe_results.video[0].codec;
                }
                if (videoPid == 0x1fff) {
                    videoPid = probe_results.video[0].pid;
                }

                if (probe_results.num_audio > 1 && configs[decoderIdx_sec].filename != NULL &&
                    strcmp(configs[decoderIdx_pri].filename, configs[decoderIdx_sec].filename) == 0) {
                    audioDescription = true;
                }
            }

            if (configs[i].audioCodec == NEXUS_AudioCodec_eMax) {
                configs[i].audioCodec = probe_results.audio[0].codec;

                if (i == decoderIdx_sec && audioDescription) {
                    configs[i].audioCodec = probe_results.audio[1].codec;
                }
            }

            if (configs[i].audioPid == 0x1fff) {
                configs[i].audioPid = probe_results.audio[0].pid;

                if (i == decoderIdx_sec && audioDescription) {
                    configs[i].audioPid = probe_results.audio[1].pid;
                }
            }

            if (configs[i].transportType == NEXUS_TransportType_eMax) {
                configs[i].transportType = probe_results.transportType;
            }

            NxClient_GetDefaultAllocSettings(&allocSettings);
            if (i == decoderIdx_pri) {
                allocSettings.simpleVideoDecoder = 1;
                allocSettings.surfaceClient = 1;
            }
            allocSettings.simpleAudioDecoder = 1;
            rc = NxClient_Alloc(&allocSettings, &configs[i].allocResults);
            if (rc) return BERR_TRACE(rc);

            if (configs[i].allocResults.simpleAudioDecoder.id) {
                configs[i].audioDecoder = NEXUS_SimpleAudioDecoder_Acquire(configs[i].allocResults.simpleAudioDecoder.id);
            }

            NxClient_GetDefaultConnectSettings(&connectSettings);
            if (i == decoderIdx_pri && configs[i].allocResults.simpleVideoDecoder[0].id) {
                videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(configs[i].allocResults.simpleVideoDecoder[0].id);
                connectSettings.simpleVideoDecoder[0].id = configs[i].allocResults.simpleVideoDecoder[0].id;
                connectSettings.simpleVideoDecoder[0].surfaceClientId = configs[i].allocResults.surfaceClient[0].id;
                connectSettings.simpleVideoDecoder[0].windowId = 0;
                connectSettings.simpleVideoDecoder[0].decoderCapabilities.supportedCodecs[videoCodec] = true;
                connectSettings.simpleVideoDecoder[0].windowCapabilities.type = NxClient_VideoWindowType_eMain;
                connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxWidth = probe_results.video[0].width;
                connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxHeight = probe_results.video[0].height;
            }
            if (requiresDyanmic && i == decoderIdx_pri) {
                connectSettings.simpleAudioDecoder.decoderCapabilities.type = NxClient_AudioDecoderType_eDynamic;
            }
            else {
                connectSettings.simpleAudioDecoder.decoderCapabilities.type = NxClient_AudioDecoderType_ePersistent;
            }
            connectSettings.simpleAudioDecoder.id = configs[i].allocResults.simpleAudioDecoder.id;
            rc = NxClient_Connect(&connectSettings, &configs[i].connectId);
            if (rc) return BERR_TRACE(rc);

            if (i != decoderIdx_sec || (i == decoderIdx_sec && !audioDescription)) {

                if (configs[i].transportType == NEXUS_TransportType_eMp4) {
                    configs[i].file = NEXUS_FilePlay_OpenPosix(configs[i].filename, configs[i].filename);
                }
                else
                {
                    configs[i].file = NEXUS_FilePlay_OpenPosix(configs[i].filename, NULL);
                }
                if ( !configs[i].file )
                {
                    BDBG_ERR(("Unable to open file %s", configs[i].filename));
                    return -1;
                }

                configs[i].stcChannel = NEXUS_SimpleStcChannel_Create(NULL);
                BDBG_ASSERT(configs[i].stcChannel);

                NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
                configs[i].playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpenSettings);
                configs[i].playback = NEXUS_Playback_Create();
                BDBG_ASSERT(configs[i].playback);

                NEXUS_Playback_GetSettings(configs[i].playback, &playbackSettings);
                playbackSettings.playpumpSettings.transportType = configs[i].transportType;
                playbackSettings.playpump = configs[i].playpump;
                playbackSettings.simpleStcChannel = configs[i].stcChannel;
                rc = NEXUS_Playback_SetSettings(configs[i].playback, &playbackSettings);
                BDBG_ASSERT(!rc);
            }

            if (i == decoderIdx_pri && videoDecoder) {
                NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
                playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
                playbackPidSettings.pidTypeSettings.video.codec = videoCodec;
                playbackPidSettings.pidTypeSettings.video.index = true;
                playbackPidSettings.pidTypeSettings.video.simpleDecoder = videoDecoder;
                videoPidChannel = NEXUS_Playback_OpenPidChannel(configs[0].playback, videoPid, &playbackPidSettings);
                NEXUS_SimpleVideoDecoder_SetStcChannel(videoDecoder, configs[0].stcChannel);
                NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);
                videoProgram.settings.codec = videoCodec;
                videoProgram.settings.pidChannel = videoPidChannel;
                videoProgram.maxHeight = probe_results.video[0].height;
                videoProgram.maxWidth = probe_results.video[0].width;
                videoProgram.settings.frameRate = probe_results.video[0].frameRate;
                videoProgram.settings.aspectRatio = probe_results.video[0].aspectRatio;
                videoProgram.settings.sampleAspectRatio.x = probe_results.video[0].sampleAspectRatio.x;
                videoProgram.settings.sampleAspectRatio.y = probe_results.video[0].sampleAspectRatio.y;
            }

            NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&configs[i].audioProgram);

            NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
            playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
            playbackPidSettings.pidTypeSettings.audio.simpleDecoder = configs[i].audioDecoder;
            if (i != decoderIdx_sec || (i == decoderIdx_sec && !audioDescription)) {
                configs[i].audioProgram.primary.pidChannel = NEXUS_Playback_OpenPidChannel(configs[i].playback, configs[i].audioPid, &playbackPidSettings);
                if (configs[i].audioProgram.primary.pidChannel) {
                    NEXUS_SimpleAudioDecoder_SetStcChannel(configs[i].audioDecoder, configs[i].stcChannel);
                }
            }
            else {
                configs[i].audioProgram.primary.pidChannel = NEXUS_Playback_OpenPidChannel(configs[0].playback, configs[i].audioPid, &playbackPidSettings);
                if (configs[i].audioProgram.primary.pidChannel) {
                    NEXUS_SimpleAudioDecoder_SetStcChannel(configs[i].audioDecoder, configs[0].stcChannel);
                }
            }

            configs[i].audioProgram.primary.codec = configs[i].audioCodec;

            switch (i) {
            case 0:
                configs[i].audioProgram.master = true;
                if (configs[i].audioCodec == NEXUS_AudioCodec_eAc4){
                    configs[i].audioProgram.primary.mixingMode = NEXUS_AudioDecoderMixingMode_eStandalone;
                }
                else {
                    configs[i].audioProgram.primary.mixingMode = NEXUS_AudioDecoderMixingMode_eDescription;
                }
                break;
            case 1:
                configs[i].audioProgram.primary.mixingMode = NEXUS_AudioDecoderMixingMode_eDescription;
                break;
            case 2:
                configs[i].audioProgram.primary.mixingMode = NEXUS_AudioDecoderMixingMode_eApplicationAudio;
                break;
            case 3:
                configs[i].audioProgram.primary.mixingMode = NEXUS_AudioDecoderMixingMode_eSoundEffects;
                break;
            }

            /* Codec Specific Settings*/
            NEXUS_SimpleAudioDecoder_GetCodecSettings(configs[i].audioDecoder, NEXUS_SimpleAudioDecoderSelector_ePrimary, configs[i].audioProgram.primary.codec, &codecSettings);
            switch (configs[i].audioCodec) {
            case NEXUS_AudioCodec_eAc4:
                if (downmixMode != NEXUS_AudioDecoderStereoDownmixMode_eAuto) {
                    codecSettings.codecSettings.ac4.stereoMode = downmixMode;
                }
                if (stereoDrc != NEXUS_AudioDecoderDolbyDrcMode_eMax) {
                    codecSettings.codecSettings.ac4.drcModeDownmix = stereoDrc;
                }
                if (multiDrc != NEXUS_AudioDecoderDolbyDrcMode_eMax) {
                    codecSettings.codecSettings.ac4.drcMode = multiDrc;
                }
                if (preferrLanguageOverType) {
                    codecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].preferLanguageOverAssociateType = true;
                    codecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].preferLanguageOverAssociateType = true;
                    strcpy(codecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].languagePreference[0].selection, preferredLangauge);
                    strcpy(codecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].languagePreference[0].selection, preferredLangauge);
                }
                codecSettings.codecSettings.ac4.enableAssociateMixing = ac4mixing;
                codecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].preferredAssociateType = ac4associateType;
                codecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].preferredAssociateType = ac4associateType;
                break;
            case NEXUS_AudioCodec_eAc3:
                if (downmixMode != NEXUS_AudioDecoderStereoDownmixMode_eAuto) {
                    codecSettings.codecSettings.ac3.stereoDownmixMode =
                        (downmixMode == NEXUS_AudioDecoderStereoDownmixMode_eLoRo ? NEXUS_AudioDecoderDolbyStereoDownmixMode_eStandard : NEXUS_AudioDecoderDolbyStereoDownmixMode_eDolbySurroundCompatible);
                }
                if (stereoDrc != NEXUS_AudioDecoderDolbyDrcMode_eMax) {
                    codecSettings.codecSettings.ac3.drcModeDownmix = stereoDrc;
                }
                if (multiDrc != NEXUS_AudioDecoderDolbyDrcMode_eMax) {
                    codecSettings.codecSettings.ac3.drcMode = multiDrc;
                }
                break;
            case NEXUS_AudioCodec_eAc3Plus:
                if (downmixMode != NEXUS_AudioDecoderStereoDownmixMode_eAuto) {
                    codecSettings.codecSettings.ac3Plus.stereoDownmixMode =
                    (downmixMode == NEXUS_AudioDecoderStereoDownmixMode_eLoRo ? NEXUS_AudioDecoderDolbyStereoDownmixMode_eStandard : NEXUS_AudioDecoderDolbyStereoDownmixMode_eDolbySurroundCompatible);
                }
                if (i == decoderIdx_pri){
                    codecSettings.codecSettings.ac3Plus.enableAtmosProcessing = true;
                }
                if (stereoDrc != NEXUS_AudioDecoderDolbyDrcMode_eMax) {
                    codecSettings.codecSettings.ac3Plus.drcModeDownmix = stereoDrc;
                }
                if (multiDrc != NEXUS_AudioDecoderDolbyDrcMode_eMax) {
                    codecSettings.codecSettings.ac3Plus.drcMode = multiDrc;
                }
                break;
            case NEXUS_AudioCodec_eAacAdts:
            case NEXUS_AudioCodec_eAacLoas:
                if (downmixMode != NEXUS_AudioDecoderStereoDownmixMode_eAuto) {
                    codecSettings.codecSettings.aac.downmixMode =
                        (downmixMode == NEXUS_AudioDecoderStereoDownmixMode_eLoRo ? NEXUS_AudioDecoderAacDownmixMode_eLoRo : NEXUS_AudioDecoderAacDownmixMode_eLtRt);
                }
                break;
            case NEXUS_AudioCodec_eAacPlusAdts:
            case NEXUS_AudioCodec_eAacPlusLoas:
                if (downmixMode != NEXUS_AudioDecoderStereoDownmixMode_eAuto) {
                    codecSettings.codecSettings.aacPlus.downmixMode =
                        (downmixMode == NEXUS_AudioDecoderStereoDownmixMode_eLoRo ? NEXUS_AudioDecoderAacDownmixMode_eLoRo : NEXUS_AudioDecoderAacDownmixMode_eLtRt);
                }
                break;
            default:
                break;
            }
            NEXUS_SimpleAudioDecoder_SetCodecSettings(configs[i].audioDecoder, NEXUS_SimpleAudioDecoderSelector_ePrimary, &codecSettings);
        }
    }


    if (configs[0].file && configs[0].playback) {
        if (videoDecoder) {
            NEXUS_SimpleVideoDecoder_Start(videoDecoder, &videoProgram);
        }
        if (configs[0].audioDecoder) {
            NEXUS_SimpleAudioDecoder_Start(configs[0].audioDecoder, &configs[0].audioProgram);
        }

        if (audioDescription && configs[1].audioDecoder) {
            NEXUS_SimpleAudioDecoder_Start(configs[1].audioDecoder, &configs[1].audioProgram);
        }

        rc = NEXUS_Playback_Start(configs[0].playback, configs[0].file, NULL);
        BDBG_ASSERT(!rc);
        configs[0].started = true;
        if (audioDescription && configs[1].audioDecoder) {
            configs[1].started = true;
        }
    }

    do
    {
        int tmp;

        printf("Main Menu\n");
        printf(" 0) Exit\n");
        printf(" 1) %s Primary %s \n", configs[0].started ? "STOP":"START", configs[0].filename);
        printf(" 2) %s Secondary %s\n", configs[1].started ? "STOP":"START", configs[1].filename);
        printf(" 3) %s Application Audio %s\n", configs[2].started ? "STOP":"START",configs[2].filename);
        printf(" 4) %s Sound Effects %s\n", configs[3].started ? "STOP":"START",configs[3].filename);
        printf(" 5) %s DAP Processing\n", dapConfig.dapEnabled ? "DISABLE" : "ENABLE");
        printf(" 6) %s Volume Leveler\n", dapConfig.volumeLevelerEnabled ? "DISABLE" : "ENABLE");
        printf(" 7) %s Dialog Enhancer\n", dapConfig.dialogEnhancerEnabled ? "DISABLE" : "ENABLE");
        printf(" 8) Adjust multistream balance\n");
        printf(" 9) Change Dual Mono Mode\n");
        printf(" 10) %s AutoPilot Profile\n", dapConfig.autoPilotEnabled ? "DISABLE" : "ENABLE");
        printf("Enter Selection: \n");
        scanf("%d", &tmp);
        switch (tmp) {
        case 0:
            done = true;
            break;
        case 1:
        case 2:
        case 3:
        case 4:
            if (configs[tmp-1].file) {
                if (configs[tmp-1].started) {
                    if (configs[tmp-1].playback) {
                        NEXUS_Playback_Stop(configs[tmp-1].playback);
                    }
                    if (configs[tmp-1].audioDecoder) {
                        NEXUS_SimpleAudioDecoder_Stop(configs[tmp-1].audioDecoder);
                    }
                    if (tmp == 1) {
                        if (videoDecoder) {
                            NEXUS_SimpleVideoDecoder_Stop(videoDecoder);
                        }
                        if (audioDescription && configs[decoderIdx_sec].started) {
                            NEXUS_SimpleAudioDecoder_Stop(configs[decoderIdx_sec].audioDecoder);
                            configs[decoderIdx_sec].started = false;
                        }
                    }
                    configs[tmp-1].started = false;
                }
                else {
                    if (tmp == 1) {
                        if (videoDecoder) {
                            NEXUS_SimpleVideoDecoder_Start(videoDecoder, &videoProgram);
                        }
                        if (audioDescription && !configs[decoderIdx_sec].started) {
                            NEXUS_SimpleAudioDecoder_Start(configs[decoderIdx_sec].audioDecoder, &configs[decoderIdx_sec].audioProgram);
                            configs[decoderIdx_sec].started = true;
                        }
                    }
                    if (configs[tmp-1].audioDecoder) {
                        NEXUS_SimpleAudioDecoder_Start(configs[tmp-1].audioDecoder, &configs[tmp-1].audioProgram);
                    }
                    if (configs[tmp-1].playback) {
                        rc = NEXUS_Playback_Start(configs[tmp-1].playback, configs[tmp-1].file, NULL);
                        BDBG_ASSERT(!rc);
                    }
                    configs[tmp-1].started = true;
                }
            }
            else {
                printf("%s does not have a file associated with it to start\n", dolby_ms12_cert_decoder_name(tmp-1));
            }
            break;
        case 5:
            dapConfig.dapEnabled = !dapConfig.dapEnabled;
            apply_dap_config(&dapConfig);
        case 6:
            dapConfig.volumeLevelerEnabled = !dapConfig.volumeLevelerEnabled;
            apply_dap_config(&dapConfig);
            break;
        case 7:
            dapConfig.dialogEnhancerEnabled = !dapConfig.dialogEnhancerEnabled;
            apply_dap_config(&dapConfig);
            break;
        case 8:
            switch (dapConfig.multiStreamBalance) {
            case 0:
                dapConfig.multiStreamBalance = -32;
                break;
            case -32:
                dapConfig.multiStreamBalance = 32;
                break;
            default:
            case 32:
                dapConfig.multiStreamBalance = 0;
                break;
            }
            apply_dap_config(&dapConfig);
            break;
        case 9:
            if (configs[0].audioDecoder) {
                bool dualMono = false;
                NEXUS_AudioDecoderStatus audioStatus;
                NEXUS_SimpleAudioDecoder_GetStatus(configs[0].audioDecoder, &audioStatus);
                switch (audioStatus.codec) {
                case NEXUS_AudioCodec_eAc3Plus:
                    dualMono = (audioStatus.codecStatus.ac3.acmod == NEXUS_AudioAc3Acmod_eTwoMono_1_ch1_ch2);
                    break;
                default:
                    break;
                }
                if (dualMono) {
                    NEXUS_SimpleAudioDecoderSettings simpleAudioSettings;
                    NEXUS_SimpleAudioDecoder_GetSettings(configs[0].audioDecoder, &simpleAudioSettings);
                    switch (simpleAudioSettings.primary.dualMonoMode) {
                    case NEXUS_AudioDecoderDualMonoMode_eStereo: /*->Left*/
                        simpleAudioSettings.primary.dualMonoMode = NEXUS_AudioDecoderDualMonoMode_eLeft;
                        break;
                    case NEXUS_AudioDecoderDualMonoMode_eLeft: /*->Right*/
                        simpleAudioSettings.primary.dualMonoMode = NEXUS_AudioDecoderDualMonoMode_eRight;
                        break;
                    default:
                    case NEXUS_AudioDecoderDualMonoMode_eRight: /*->Stereo*/
                        simpleAudioSettings.primary.dualMonoMode = NEXUS_AudioDecoderDualMonoMode_eStereo;
                        break;
                    }
                    NEXUS_SimpleAudioDecoder_SetSettings(configs[0].audioDecoder, &simpleAudioSettings);
                }
                else {
                    BDBG_WRN(("Primary is not Dual Mono"));
                }
            }
            break;
        case 10:
            dapConfig.autoPilotEnabled = !dapConfig.autoPilotEnabled;
            apply_dap_config(&dapConfig);
            break;
        default:
            break;

        }
    } while(!done);

    /* I need to clean up after myself */

    return 0;
}

void default_dap_config(dapConfiguration *dapConfig)
{
    memset(dapConfig, 0, sizeof(dapConfiguration));
    dapConfig->ieqPreset = ieqProfile_max;
}

void apply_dap_config(dapConfiguration *dapConfig)
{
    NEXUS_Error rc;
    NxClient_AudioProcessingSettings audioProcessingSettings;

    NxClient_GetAudioProcessingSettings(&audioProcessingSettings);
    if (dapConfig->autoPilotEnabled) {
        audioProcessingSettings.dolby.dolbySettings.enablePostProcessing = true;
        audioProcessingSettings.dolby.dolbySettings.volumeLimiter.enableVolumeLimiting = true;
        audioProcessingSettings.dolby.dolbySettings.volumeLimiter.enableIntelligentLoudness = true;
        audioProcessingSettings.dolby.dolbySettings.volumeLimiter.volumeLimiterAmount = 7;
        audioProcessingSettings.dolby.dolbySettings.dialogEnhancer.enableDialogEnhancer = true;
        audioProcessingSettings.dolby.dolbySettings.dialogEnhancer.dialogEnhancerLevel = 5;
        audioProcessingSettings.dolby.dolbySettings.intelligentEqualizer.enabled = true;
        BKNI_Memcpy(&audioProcessingSettings.dolby.dolbySettings.intelligentEqualizer.band, &openProfile, sizeof(openProfile));

    }
    else {
        audioProcessingSettings.dolby.dolbySettings.enablePostProcessing = dapConfig->dapEnabled;
        audioProcessingSettings.dolby.dolbySettings.volumeLimiter.enableVolumeLimiting = dapConfig->volumeLevelerEnabled;
        audioProcessingSettings.dolby.dolbySettings.volumeLimiter.enableIntelligentLoudness = dapConfig->volumeLevelerEnabled;
        audioProcessingSettings.dolby.dolbySettings.volumeLimiter.volumeLimiterAmount = dapConfig->volumeLevelerAmount;
        audioProcessingSettings.dolby.dolbySettings.dialogEnhancer.enableDialogEnhancer = dapConfig->dialogEnhancerEnabled;
        audioProcessingSettings.dolby.dolbySettings.dialogEnhancer.dialogEnhancerLevel = dapConfig->dialogEnhancerAmount;
        audioProcessingSettings.dolby.dolbySettings.intelligentEqualizer.enabled = dapConfig->intelligentEqualizerEnabled;
        switch (dapConfig->ieqPreset) {
        case ieqProfile_open:
            BKNI_Memcpy(&audioProcessingSettings.dolby.dolbySettings.intelligentEqualizer.band, &openProfile, sizeof(openProfile));
            audioProcessingSettings.dolby.dolbySettings.intelligentEqualizer.numBands = 20;
            break;
        case ieqProfile_rich:
            BKNI_Memcpy(&audioProcessingSettings.dolby.dolbySettings.intelligentEqualizer.band, &richProfile, sizeof(richProfile));
            audioProcessingSettings.dolby.dolbySettings.intelligentEqualizer.numBands = 20;
            break;
        case ieqProfile_focused:
            BKNI_Memcpy(&audioProcessingSettings.dolby.dolbySettings.intelligentEqualizer.band, &focusedProfile, sizeof(focusedProfile));
            audioProcessingSettings.dolby.dolbySettings.intelligentEqualizer.numBands = 20;
            break;
        default:
            audioProcessingSettings.dolby.dolbySettings.intelligentEqualizer.numBands = 0;
            break;
        }
    }
    audioProcessingSettings.dolby.dolbySettings.multiStreamBalance = dapConfig->multiStreamBalance;
    rc = NxClient_SetAudioProcessingSettings(&audioProcessingSettings);
    if (rc) {
        BERR_TRACE(rc);
    }
}

const char *dolby_ms12_cert_decoder_name(const decoderIdx idx)
{
    switch (idx) {
    case decoderIdx_pri: return "Primary";
    case decoderIdx_sec: return "Secondary";
    case decoderIdx_app: return "Application Audio";
    case decoderIdx_sfx: return "Sound Effects";
    default: return "Invalid";
    }
}


void print_usage(void) {
    printf("Usage:\n");
    printf("play OPTIONS\n");
    printf("  -primary <file location> - File to use for primary video/audio decoder\n");
    printf("  -secondary <file location> - File to use for secondary audio decoder\n");
    printf("  -app <file location> - File to use for application audio decoder\n");
    printf("  -sfx <file location> - File to use for sound effects audio decoder\n");
    printf("  -video PID               override media probe. use 0 for no video.\n");
    printf("  -pri_audio PID / -sec_audio PID   override media probe for audio. use 0 for no audio.\n");
    print_list_option(
    "  -video_type              override media probe", g_videoCodecStrs);
        print_list_option(
    "  -pri_audio_type / -sec_audio_type    override media probe", g_audioCodecStrs);
    print_list_option(
    "  -pri_mpeg_type -sec_mpeg_type    override media probe", g_transportTypeStrs);
    printf("  -stereo [loro|ltrt]   Sets stereo downmix mode [Automatic is default]\n");
    printf("  -autoPilot    Enables AutoPilot Profile\n");
    printf("  -dap  Set DAP to enabled at start time\n");
    printf("  -volume_leveler <0-10>  Set Volume Leveler to enabled at start time \n");
    printf("  -dialog_enhancer <0-16)  Set Dialog Enhancer to enabled at start time \n");
    printf("  -ieq [open|focused|rich] Set Intelligent Equializer preset\n)");
    printf("  -dynamic  Uses a dyanmic decoder for primary.  This is required for passthrough testing. \n");
    printf("  -stereoDRC [line|rf]   Sets stereo DRC configuration\n");
    printf("  -multiDRC [line|rf]   Sets multichannel DRC configuration\n");
    printf("  -ac4AssociateMixing [notSpecified|visual|hearing|commentary]   Enables AC4 Associate Mixing\n");
    printf("  -ac4Language <language> Sets prefrence for specified language\n");
    printf("\n");
}
