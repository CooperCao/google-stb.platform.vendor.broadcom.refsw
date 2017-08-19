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
#include "nexus_audio_capture.h"
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

typedef struct capture_handles
{
    NEXUS_AudioCaptureHandle capture;
    FILE* capFile;
} capture_handles;

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

enum eCaptureVerifyState
{
    eCaptureVerfiyStateUnlocked,         /* Totally lost... looking for the first sample that matches the reference. */
    eCaptureVerfiyStateLocking,          /* Found at least one match with the reference, looking for more.  */
    eCaptureVerfiyStateLocked,           /* Found enough consecutive matches to decide that we're in sync with the reference. */
    eCaptureVerfiyStateUnlocking         /* Found at least one mismatch while in Locked state... too many will take us to Unlocked. */
};

char *captureVerifyStateName[] =         /* String descriptions for each of the eState values. */
{
    "Unlocked",
    "Locking",
    "Locked",
    "Unlocking"
};

typedef struct CaptureVerifyContext
{
    enum eCaptureVerifyState    state;          /* Unlocked, Locking, Locked, Unlocking  */
    unsigned                    refIndex;       /* Index into reference buffer of last sample. */

    unsigned                    matches;        /* Counts consecutive matches during Locking state. */
    unsigned                    mismatches;     /* Counts consecutive mismatches during Unlocking state. */

    unsigned long               totalMatches;
    unsigned long               totalMismatches;
    unsigned long               totalSamples;   /* Total number of sampled processed */

    unsigned long               sizeCapture;    /* size of capture minus the begining padding */
    unsigned long               samplesSinceLastLock;
    unsigned long               mismatchesSinceLastLock;

    unsigned long               samplesInLongestLockPeriod;
    unsigned long               mismatchesInLongestLockPeriod;

    FILE* verifyFile;
    FILE* captureFile;
} CaptureVerifyContext;

static void print_usage(void);
static void default_dap_config(dapConfiguration *dapConfig);
static NEXUS_Error apply_dap_config(dapConfiguration *dapConfig);
static const char *dolby_ms12_cert_decoder_name(const decoderIdx idx);
static void endOfStreamCallback(void *context, int param);
static void capture_callback(void *pParam, int param);
static void verifyCapture(CaptureVerifyContext *pCtx);


int main(int argc, const char **argv) {
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
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned i;
    bool videoDisabled = false;
    bool done = false;
    bool once = false;
    bool audioDescription = false;
    bool single_pid = false;
    bool requiresDyanmic = false;
    NEXUS_AudioDecoderDolbyDrcMode stereoDrc = NEXUS_AudioDecoderDolbyDrcMode_eMax;
    NEXUS_AudioDecoderDolbyDrcMode multiDrc = NEXUS_AudioDecoderDolbyDrcMode_eMax;
    config configs[decoderIdx_max];
    bool preferrLanguageOverType = false;
    char preferredLangauge[8];
    NEXUS_AudioAc4AssociateType ac4associateType = NEXUS_AudioAc4AssociateType_eNotSpecified;
    bool ac4mixing = false;
    dapConfiguration dapConfig;
    int ac4DE = 0;
    BKNI_EventHandle endOfStreamEvent;
    bool enableCapture = false;
    char captureFileName[250];
    unsigned captureId;
    NEXUS_AudioCaptureOpenSettings openSettings;
    NEXUS_AudioCaptureStartSettings startSettings;
    NxClient_AudioCaptureType captureType = NxClient_AudioCaptureType_e16BitStereo;
    NEXUS_AudioMultichannelFormat multichCaptureFormat = NEXUS_AudioMultichannelFormat_eStereo;
    capture_handles capHandles = {NULL,NULL};
    bool enableVerify = false;
    const char * verifyFileName;
    CaptureVerifyContext    vfyCtx;

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
        else if (!strcmp(argv[curarg], "-disable_video")) {
            videoDisabled = true;
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
        else if (!strcmp(argv[curarg], "-single_pid")) {
            single_pid = true;
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
        else if (!strcmp(argv[curarg], "-once")) {
            once = true;
        }
        else if (!strcmp(argv[curarg], "-capture") && curarg+1 < argc) {
            ++curarg;
            if (!strcmp("stereo", argv[curarg])) {
                enableCapture = true;
                captureType = NxClient_AudioCaptureType_e24BitStereo;
            }
            else if (!strcmp("multichannel", argv[curarg])) {
                enableCapture = true;
                captureType = NxClient_AudioCaptureType_e24Bit5_1;
                multichCaptureFormat = NEXUS_AudioMultichannelFormat_e5_1;
            }
            else if (!strcmp("compressed", argv[curarg])) {
                enableCapture = true;
                captureType = NxClient_AudioCaptureType_eCompressed;
            }
            else if (!strcmp("compressed4x", argv[curarg])) {
                enableCapture = true;
                captureType = NxClient_AudioCaptureType_eCompressed4x;
            }
            else {
                print_usage();
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-verify") && argc>curarg+1) {
            verifyFileName = argv[++curarg];
            enableVerify = true;
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
    if (rc) {BERR_TRACE(rc); goto cleanup;}

    rc = apply_dap_config(&dapConfig);
    if (rc) {BERR_TRACE(rc); goto cleanup;}

    for (i = 0; i < decoderIdx_max; i++) {
        if (configs[i].filename)
        {
            struct probe_results probe_results;
            struct probe_request probe_request;

            probe_media_get_default_request(&probe_request);
            probe_request.streamname = configs[i].filename;
            printf("Probing %s\n", dolby_ms12_cert_decoder_name(i));
            rc = probe_media_request(&probe_request, &probe_results);
            if (rc) {
                BDBG_ERR(("media probe can't parse '%s'", configs[i].filename));
                goto cleanup;
            }

            if (i == decoderIdx_pri && enableCapture) {
                char base[200];
                char * fileName;
                char * ext;
                fileName = strrchr(configs[i].filename, '/');
                fileName++;
                ext = strchr(fileName, '.');
                strncpy(base, fileName, ext-fileName);

                switch (captureType) {
                case NxClient_AudioCaptureType_e16BitStereo:
                case NxClient_AudioCaptureType_e24BitStereo:
                    snprintf(captureFileName, sizeof(captureFileName), "./%s_capture_stereo.bin", base);
                    break;
                case NxClient_AudioCaptureType_e24Bit5_1:
                    snprintf(captureFileName, sizeof(captureFileName), "./%s_capture_multi.bin", base);
                    break;
                case NxClient_AudioCaptureType_eCompressed:
                    snprintf(captureFileName, sizeof(captureFileName), "./%s_capture_compressed.bin", base);
                    break;
                case NxClient_AudioCaptureType_eCompressed4x:
                    snprintf(captureFileName, sizeof(captureFileName), "./%s_capture_compressed_4x.bin", base);
                    break;
                default:
                    BDBG_WRN(("Invalid capture type, disable capture"));
                    enableCapture = false;
                    break;
                }
            }

            /* override probe */
            if (i == decoderIdx_pri) {
                if (videoCodec == NEXUS_VideoCodec_eMax) {
                    videoCodec = probe_results.video[0].codec;
                }
                if (videoPid == 0x1fff) {
                    videoPid = probe_results.video[0].pid;
                }

                if ((probe_results.num_audio > 1 || single_pid) && configs[decoderIdx_sec].filename != NULL &&
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

                if (i == decoderIdx_sec && single_pid) {
                    configs[i].audioPid = configs[0].audioPid;
                }
                else if (i == decoderIdx_sec && audioDescription) {
                    configs[i].audioPid = probe_results.audio[1].pid;
                }
            }

            if (configs[i].transportType == NEXUS_TransportType_eMax) {
                configs[i].transportType = probe_results.transportType;
            }

            NxClient_GetDefaultAllocSettings(&allocSettings);
            if (i == decoderIdx_pri) {
                if (probe_results.num_video > 0 && !videoDisabled)
                {
                    allocSettings.simpleVideoDecoder = 1;
                    allocSettings.surfaceClient = 1;
                }
                if (enableCapture) {
                    allocSettings.audioCapture = 1;
                    allocSettings.audioCaptureType.type = captureType;
                }
            }
            allocSettings.simpleAudioDecoder = 1;
            rc = NxClient_Alloc(&allocSettings, &configs[i].allocResults);
            if (rc) {BERR_TRACE(rc); goto cleanup;}

            if (configs[i].allocResults.simpleAudioDecoder.id) {
                configs[i].audioDecoder = NEXUS_SimpleAudioDecoder_Acquire(configs[i].allocResults.simpleAudioDecoder.id);
                if (!configs[i].audioDecoder) {BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto cleanup;}
            }

            if (i == decoderIdx_pri && enableCapture && configs[i].allocResults.audioCapture.id) {
                captureId = configs[i].allocResults.audioCapture.id;
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
            if (rc) {BERR_TRACE(rc); goto cleanup;}

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
                    BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto cleanup;
                }

                configs[i].stcChannel = NEXUS_SimpleStcChannel_Create(NULL);
                if (!configs[i].stcChannel) {BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto cleanup;}

                NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
                configs[i].playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpenSettings);
                if (!configs[i].playpump) {BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto cleanup;}
                configs[i].playback = NEXUS_Playback_Create();
                if (!configs[i].playback) {BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto cleanup;}

                NEXUS_Playback_GetSettings(configs[i].playback, &playbackSettings);
                playbackSettings.playpumpSettings.transportType = configs[i].transportType;
                playbackSettings.playpump = configs[i].playpump;
                playbackSettings.simpleStcChannel = configs[i].stcChannel;
                if (once) {
                    playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePause;
                    if (i == decoderIdx_pri) {
                        BKNI_CreateEvent(&endOfStreamEvent);
                        playbackSettings.endOfStreamCallback.callback = endOfStreamCallback;
                        playbackSettings.endOfStreamCallback.context = endOfStreamEvent;
                    }
                }
                rc = NEXUS_Playback_SetSettings(configs[i].playback, &playbackSettings);
                if (rc) {BERR_TRACE(rc); goto cleanup;}
            }

            if (i == decoderIdx_pri && videoDecoder) {
                NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
                playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
                playbackPidSettings.pidTypeSettings.video.codec = videoCodec;
                playbackPidSettings.pidTypeSettings.video.index = true;
                playbackPidSettings.pidTypeSettings.video.simpleDecoder = videoDecoder;
                videoPidChannel = NEXUS_Playback_OpenPidChannel(configs[0].playback, videoPid, &playbackPidSettings);
                if (!videoPidChannel) {BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto cleanup;}
                rc = NEXUS_SimpleVideoDecoder_SetStcChannel(videoDecoder, configs[0].stcChannel);
                if (rc) {BERR_TRACE(rc); goto cleanup;}
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
                    rc = NEXUS_SimpleAudioDecoder_SetStcChannel(configs[i].audioDecoder, configs[i].stcChannel);
                    if (rc) {BERR_TRACE(rc); goto cleanup;}
                }
                else {BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto cleanup;}
            }
            else {
                configs[i].audioProgram.primary.pidChannel = NEXUS_Playback_OpenPidChannel(configs[0].playback, configs[i].audioPid, &playbackPidSettings);
                if (configs[i].audioProgram.primary.pidChannel) {
                    rc = NEXUS_SimpleAudioDecoder_SetStcChannel(configs[i].audioDecoder, configs[0].stcChannel);
                    if (rc) {BERR_TRACE(rc); goto cleanup;}
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
                configs[i].audioProgram.primary.secondaryDecoder = true;
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
                ac4DE = codecSettings.codecSettings.ac4.dialogEnhancerAmount;
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
                if (single_pid && i == decoderIdx_sec) {
                    codecSettings.codecSettings.ac3Plus.substreamId = 1;
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
            rc = NEXUS_SimpleAudioDecoder_SetCodecSettings(configs[i].audioDecoder, NEXUS_SimpleAudioDecoderSelector_ePrimary, &codecSettings);
            if (rc) {BERR_TRACE(rc); goto cleanup;}
        }
    }

    if (enableCapture) {
        capHandles.capFile = fopen(captureFileName, "w+");
        if (!capHandles.capFile) {
            fprintf(stderr, "### unable to open %s\n", captureFileName);
            BERR_TRACE(rc); goto cleanup;
        }

        NEXUS_AudioCapture_GetDefaultOpenSettings(&openSettings);
        openSettings.multichannelFormat = multichCaptureFormat;
        /*BDBG_ERR(("Open Capture"));*/
        /* allocate our capture at the end of the array, so we don't try to use the same resources as NxServer */
        capHandles.capture = NEXUS_AudioCapture_Open(captureId, &openSettings);
        if (!capHandles.capture) {
            BDBG_ERR(("  unable to open capture"));
            BERR_TRACE(rc); goto cleanup;
        }

        NEXUS_AudioCapture_GetDefaultStartSettings(&startSettings);
        startSettings.dataCallback.callback = capture_callback;
        startSettings.dataCallback.context = &capHandles;
        /*BDBG_ERR(("Start Capture"));*/
        rc = NEXUS_AudioCapture_Start(capHandles.capture, &startSettings);
        if (rc) {BERR_TRACE(rc); goto cleanup;}
    }


    if (!once) {
        if (configs[0].file && configs[0].playback)
        {
            if (videoDecoder) {
                NEXUS_SimpleVideoDecoder_Start(videoDecoder, &videoProgram);
            }
            if (configs[0].audioDecoder) {
                rc = NEXUS_SimpleAudioDecoder_Start(configs[0].audioDecoder, &configs[0].audioProgram);
                if (rc) {BERR_TRACE(rc); goto cleanup;}
            }

            if (audioDescription && configs[1].audioDecoder) {
                rc = NEXUS_SimpleAudioDecoder_Start(configs[1].audioDecoder, &configs[1].audioProgram);
                if (rc) {BERR_TRACE(rc); goto cleanup;}
            }

            rc = NEXUS_Playback_Start(configs[0].playback, configs[0].file, NULL);
            if (rc) {BERR_TRACE(rc); goto cleanup;}
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
            printf(" 10) Increase AC4 Dialogue Enhancement (%d)\n", ac4DE);
            printf(" 11) Decrease AC4 Dialogue Enhancement (%d)\n", ac4DE);
            printf(" 12) %s AutoPilot Profile\n", dapConfig.autoPilotEnabled ? "DISABLE" : "ENABLE");
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
                                rc = NEXUS_SimpleAudioDecoder_Start(configs[decoderIdx_sec].audioDecoder, &configs[decoderIdx_sec].audioProgram);
                                if (rc) {BERR_TRACE(rc); goto cleanup;}
                                configs[decoderIdx_sec].started = true;
                            }
                        }
                        if (configs[tmp-1].audioDecoder) {
                            rc = NEXUS_SimpleAudioDecoder_Start(configs[tmp-1].audioDecoder, &configs[tmp-1].audioProgram);
                            if (rc) {BERR_TRACE(rc); goto cleanup;}
                        }
                        if (configs[tmp-1].playback) {
                            rc = NEXUS_Playback_Start(configs[tmp-1].playback, configs[tmp-1].file, NULL);
                            if (rc) {BERR_TRACE(rc); goto cleanup;}
                        }
                        configs[tmp-1].started = true;
                    }
                }
                else if (tmp == 2) {
                    if (configs[tmp-1].started) {
                        if (configs[tmp-1].audioDecoder) {
                            NEXUS_SimpleAudioDecoder_Stop(configs[tmp-1].audioDecoder);
                        }
                        configs[tmp-1].started = false;
                    }
                    else {
                        if (configs[0].started) {
                            rc = NEXUS_SimpleAudioDecoder_Start(configs[tmp-1].audioDecoder, &configs[tmp-1].audioProgram);
                            if (rc) {BERR_TRACE(rc); goto cleanup;}
                            configs[tmp-1].started = true;
                        }
                    }
                }
                else
                {
                    printf("%s does not have a file associated with it to start\n", dolby_ms12_cert_decoder_name(tmp-1));
                }
                break;
            case 5:
                dapConfig.dapEnabled = !dapConfig.dapEnabled;
                rc = apply_dap_config(&dapConfig);
                if (rc) {BERR_TRACE(rc); goto cleanup;}
            case 6:
                dapConfig.volumeLevelerEnabled = !dapConfig.volumeLevelerEnabled;
                rc = apply_dap_config(&dapConfig);
                if (rc) {BERR_TRACE(rc); goto cleanup;}
                break;
            case 7:
                dapConfig.dialogEnhancerEnabled = !dapConfig.dialogEnhancerEnabled;
                rc = apply_dap_config(&dapConfig);
                if (rc) {BERR_TRACE(rc); goto cleanup;}
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
                rc = apply_dap_config(&dapConfig);
                if (rc) {BERR_TRACE(rc); goto cleanup;}
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
                        rc = NEXUS_SimpleAudioDecoder_SetSettings(configs[0].audioDecoder, &simpleAudioSettings);
                        if (rc) {BERR_TRACE(rc); goto cleanup;}
                    }
                    else {
                        BDBG_WRN(("Primary is not Dual Mono"));
                    }
                }
                break;
            case 10:
                if (configs[0].audioCodec == NEXUS_AudioCodec_eAc4){
                    ac4DE = (ac4DE == 12 ? 12 : ac4DE + 1);
                    NEXUS_SimpleAudioDecoder_GetCodecSettings(configs[0].audioDecoder, NEXUS_SimpleAudioDecoderSelector_ePrimary, configs[0].audioProgram.primary.codec, &codecSettings);
                    codecSettings.codecSettings.ac4.dialogEnhancerAmount = ac4DE;
                    rc = NEXUS_SimpleAudioDecoder_SetCodecSettings(configs[0].audioDecoder, NEXUS_SimpleAudioDecoderSelector_ePrimary, &codecSettings);
                    if (rc) {BERR_TRACE(rc); goto cleanup;}
                }
                break;
            case 11:
                if (configs[0].audioCodec == NEXUS_AudioCodec_eAc4){
                    ac4DE = (ac4DE == -12 ? -12 : ac4DE - 1);
                    NEXUS_SimpleAudioDecoder_GetCodecSettings(configs[0].audioDecoder, NEXUS_SimpleAudioDecoderSelector_ePrimary, configs[0].audioProgram.primary.codec, &codecSettings);
                    codecSettings.codecSettings.ac4.dialogEnhancerAmount = ac4DE;
                    rc = NEXUS_SimpleAudioDecoder_SetCodecSettings(configs[0].audioDecoder, NEXUS_SimpleAudioDecoderSelector_ePrimary, &codecSettings);
                    if (rc) {BERR_TRACE(rc); goto cleanup;}
                }
                break;
            case 12:
                dapConfig.autoPilotEnabled = !dapConfig.autoPilotEnabled;
                rc = apply_dap_config(&dapConfig);
                if (rc) {BERR_TRACE(rc); goto cleanup;}
                break;
            default:
                break;

            }
        } while(!done);

    }
    else {
        for (i = 0; i < decoderIdx_max; i++) {
            if (configs[i].file && configs[i].playback) {
                if (videoDecoder && i == decoderIdx_pri) {
                    NEXUS_SimpleVideoDecoder_Start(videoDecoder, &videoProgram);
                }
                if (configs[i].audioDecoder) {
                    rc = NEXUS_SimpleAudioDecoder_Start(configs[i].audioDecoder, &configs[i].audioProgram);
                    if (rc) {BERR_TRACE(rc); goto cleanup;}
                }

                if (audioDescription && configs[1].audioDecoder && i == decoderIdx_pri) {
                    rc = NEXUS_SimpleAudioDecoder_Start(configs[1].audioDecoder, &configs[1].audioProgram);
                    if (rc) {BERR_TRACE(rc); goto cleanup;}
                }

                rc = NEXUS_Playback_Start(configs[i].playback, configs[i].file, NULL);
                if (rc) {BERR_TRACE(rc); goto cleanup;}
                configs[i].started = true;
                if (audioDescription && configs[1].audioDecoder && i == decoderIdx_pri) {
                    configs[1].started = true;
                }
            }
        }
        /* Wait for Primary to finish */
        BKNI_WaitForEvent(endOfStreamEvent, BKNI_INFINITE);
    }
    rc = NEXUS_SUCCESS;

cleanup:
    if ( rc != NEXUS_SUCCESS ) {
        enableVerify = false;
    }

    for ( i = 0; i < decoderIdx_max; i++ ) {
        if (configs[i].file && configs[i].playback && configs[i].started) {
            if (videoDecoder && i == decoderIdx_pri) {
                NEXUS_SimpleVideoDecoder_Stop(videoDecoder);
            }
            if (configs[i].audioDecoder) {
                NEXUS_SimpleAudioDecoder_Stop(configs[i].audioDecoder);
            }

            if (audioDescription && configs[1].audioDecoder && i == decoderIdx_pri && configs[1].started) {
                NEXUS_SimpleAudioDecoder_Stop(configs[1].audioDecoder);
            }

            NEXUS_Playback_Stop(configs[i].playback);
            configs[i].started = false;
            if (audioDescription && configs[1].audioDecoder && i == decoderIdx_pri) {
                configs[1].started = false;
            }
        }
    }

    if (enableCapture) {
        bool testFailed = false;
        bool wePassed;
        unsigned long longestLockMismatchesMax = 1;
        NEXUS_AudioCapture_Stop(capHandles.capture);
        NEXUS_AudioCapture_Close(capHandles.capture);
        if ( capHandles.capFile ) {
            fclose(capHandles.capFile);
        }
        if (enableVerify) {
            BKNI_Memset(&vfyCtx, 0, sizeof(vfyCtx));     /* Reset the verify context buffer. */
            vfyCtx.captureFile = fopen(captureFileName, "r+");
            if (!vfyCtx.captureFile) {
                fprintf(stderr, "### unable to open %s\n", captureFileName);
                return -1;
            }

            vfyCtx.verifyFile = fopen(verifyFileName, "r+");
            if (!vfyCtx.verifyFile) {
                fprintf(stderr, "### unable to open %s\n", verifyFileName);
                return -1;
            }
            verifyCapture(&vfyCtx);
            printf("------------------------------------------------------------------------\n");
            printf("Capture Verification Summary:\n");

            wePassed = ((vfyCtx.samplesInLongestLockPeriod) >= (vfyCtx.sizeCapture / 10 * 9)) ? true : false;
            if (!wePassed)  {
                testFailed = true;
                printf("    Samples In Longest Lock Period:    %8lu    (Min:%8lu)    %s\n",  vfyCtx.samplesInLongestLockPeriod, (vfyCtx.sizeCapture / 10 * 9), wePassed?"PASS":"***FAIL***");
            }


            wePassed = (vfyCtx.mismatchesInLongestLockPeriod <= longestLockMismatchesMax) ? true : false;
            if (!wePassed) {
                testFailed = true;
                printf("    Mismatches In Longest Lock Period: %8lu    (Max:%8lu)    %s\n",  vfyCtx.mismatchesInLongestLockPeriod, longestLockMismatchesMax, wePassed?"PASS":"***FAIL***");
            }


            if (testFailed) {
                printf(" Test FAILED!!!\n");
                return 1;
            }
            else {
                printf(" Test PASSED!!!\n");
            }
            printf("------------------------------------------------------------------------\n");

        }
    }

    return rc;
}

void default_dap_config(dapConfiguration *dapConfig)
{
    memset(dapConfig, 0, sizeof(dapConfiguration));
    dapConfig->ieqPreset = ieqProfile_max;
}

NEXUS_Error apply_dap_config(dapConfiguration *dapConfig)
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
        audioProcessingSettings.dolby.dolbySettings.intelligentEqualizer.numBands = 20;
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

    return rc;
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

void endOfStreamCallback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
    return;
}

void capture_callback(void *pParam, int param)
{
    capture_handles * capHandles = (capture_handles*)pParam;
    NEXUS_AudioCaptureHandle capture;
    FILE *pFile;
    NEXUS_Error errCode;

    BSTD_UNUSED(param);

    BDBG_ASSERT(capHandles);
    capture = capHandles->capture;
    BDBG_ASSERT(capture);
    pFile = (FILE*) capHandles->capFile;

    for ( ;; )
    {
        void *pBuffer;
        size_t bufferSize;

        /* Check available buffer space */
        errCode = NEXUS_AudioCapture_GetBuffer(capture, (void **)&pBuffer, &bufferSize);
        if ( errCode )
        {
            fprintf(stderr, "Error getting capture buffer\n");
            NEXUS_AudioCapture_Stop(capture);
            return;
        }

        if ( bufferSize > 0 )
        {
            /*printf("Capture received %lu bytes.", (unsigned long)bufferSize);*/
            if ( pFile )
            {
                /* Write samples to disk */
                /*printf("  writing %lu bytes to file.", (unsigned long)bufferSize);*/
                if ( 1 != fwrite(pBuffer, bufferSize, 1, pFile) )
                {
                    fprintf(stderr, "Error writing to disk\n");
                    NEXUS_AudioCapture_Stop(capture);
                    return;
                }
            }
            /*printf("\n");*/

            /*fprintf(stderr, "Data callback - Wrote %d bytes\n", (int)bufferSize);*/
            errCode = NEXUS_AudioCapture_ReadComplete(capture, bufferSize);
            if ( errCode )
            {
                fprintf(stderr, "Error committing capture buffer\n");
                NEXUS_AudioCapture_Stop(capture);
                return;
            }
        }
        else
        {
            break;
        }
    }
}

#define MAX_BUFFER_SIZE     (uint32_t)(256*1024)
void verifyCapture(CaptureVerifyContext *pCtx)
{
    uint32_t *captureBuffer;
    uint32_t *verifyBuffer;
    unsigned captureSampleIndex = 0;
    unsigned verifySampleIndex = 0;
    const unsigned  lockSampleCount   = 10;                      /* Need this many consecutive matches to lock */
    const unsigned  unlockSampleCount = 10;                      /* Need this many consecutive mismatches to unlock */
    const unsigned  totalMismatchLimit = 100;
    unsigned captureBufferSize;
    unsigned long captureFileSize = 0;
    unsigned verifyBufferSize;
    unsigned tempIndex=0;
    bool done = false;
    bool startCaptureFound = false;
    bool startVerifyFound = false;

    captureBuffer = BKNI_Malloc(MAX_BUFFER_SIZE*sizeof(uint32_t));
    verifyBuffer = BKNI_Malloc(MAX_BUFFER_SIZE*sizeof(uint32_t));

    /* find the first non-zero sample */
    fseek(pCtx->captureFile, 0, SEEK_END);
    captureFileSize = ftell(pCtx->captureFile);
    fseek(pCtx->captureFile, 0, SEEK_SET);

    while (!startCaptureFound) {
        captureBufferSize = fread(captureBuffer,  sizeof(uint32_t), MAX_BUFFER_SIZE, pCtx->captureFile);
        if (captureBufferSize == 0) {
            BDBG_ERR(("Unable to find non-zero byte in capture file."));
            goto done;
        }
        else {
            tempIndex += captureBufferSize;
        }
        for (captureSampleIndex = 0; captureSampleIndex < captureBufferSize; captureSampleIndex++) {
            uint32_t  mySample;
            mySample = captureBuffer[captureSampleIndex];

            if (mySample != 0x0) {
                startCaptureFound = true;
                /*printf("---capture begining found at %d %x\n", captureSampleIndex, mySample);*/
                pCtx->sizeCapture = ((captureFileSize - (tempIndex - captureBufferSize + captureSampleIndex)) / 4);
                break;
            }

        }
    }

    while (!startVerifyFound) {
        verifyBufferSize = fread(verifyBuffer,  sizeof(uint32_t), MAX_BUFFER_SIZE, pCtx->verifyFile);
        if (verifyBufferSize == 0) {
            BDBG_ERR(("Unable to find non-zero byte in verify file."));
            goto done;
        }
        for (verifySampleIndex = 0; verifySampleIndex < verifyBufferSize; verifySampleIndex++) {
            uint32_t  mySample;
            mySample = verifyBuffer[verifySampleIndex];
            if (mySample != 0x0) {
                startVerifyFound = true;
                pCtx->refIndex += verifySampleIndex;
                /*printf("---verify begining found at %d %x\n", verifySampleIndex, mySample);*/
                break;
            }

        }
        if (!startVerifyFound) {
            pCtx->refIndex += verifyBufferSize;
        }
    }

    while (!done) {
        for (; captureSampleIndex < captureBufferSize && verifySampleIndex < verifyBufferSize; captureSampleIndex++)
        {
            uint32_t  myCaptureSample;

            if (pCtx->totalMismatches == totalMismatchLimit) {
                printf("Reached mismatch limit of %lu... Giving up!\n", pCtx->totalMismatches);
                pCtx->totalMismatches++;         /* This is so we don't keep printing this message. */
            }

            if (pCtx->totalMismatches >= totalMismatchLimit) break;

            myCaptureSample = captureBuffer[captureSampleIndex];
            pCtx->totalSamples++;
            pCtx->samplesSinceLastLock++;

            /* Unlocked State: Look through the reference samples for a match with the current sample. */
            if (pCtx->state == eCaptureVerfiyStateUnlocked) {
                if (myCaptureSample != 0x0) {            /* Ignore null samples when we're unlocked */
                    tempIndex = 0;
                    while (pCtx->state == eCaptureVerfiyStateUnlocked) {
                        if (verifySampleIndex == verifyBufferSize) {
                            verifyBufferSize = fread(verifyBuffer,  sizeof(uint32_t), MAX_BUFFER_SIZE, pCtx->verifyFile);
                            if (verifyBufferSize == 0) {
                                /*printf("No more Data in Verify File\n");*/
                                done = true;
                                break;
                            }
                            verifySampleIndex = 0;
                        }

                        for (; verifySampleIndex < verifyBufferSize; verifySampleIndex++, tempIndex++) {
                            if (verifyBuffer[verifySampleIndex] == myCaptureSample) {
                                /*printf("At sample %lu %s -> %s: Found match at refIndex:%d\n", pCtx->totalSamples, captureVerifyStateName[pCtx->state], captureVerifyStateName[eCaptureVerfiyStateLocking], pCtx->refIndex + tempIndex);*/
                                pCtx->refIndex += tempIndex;
                                tempIndex = 0;
                                pCtx->state = eCaptureVerfiyStateLocking;
                                pCtx->matches = 0;
                                verifySampleIndex++;
                                break;
                            }
                        }
                    }
                }
            }
            /* Partially Locked States (Locking, Locked, Unlocking) */
            else {
                bool match = true;

                /* Check this sample's value against the reference array. */
                if (myCaptureSample != verifyBuffer[verifySampleIndex]) {
                    /*printf("Value mismatch: expected: %d  got %d\n", verifyBuffer[verifySampleIndex], myCaptureSample);*/
                    match = false;
                }

                /* Handle Locking state... */
                if (pCtx->state == eCaptureVerfiyStateLocking) {
                    if (match) {
                        pCtx->matches++;
                        tempIndex++;
                        verifySampleIndex++;

                        if (pCtx->matches > lockSampleCount) {
                            /*printf("At sample %lu %s -> %s: Found match at refIndex:%d\n", pCtx->totalSamples, captureVerifyStateName[pCtx->state], captureVerifyStateName[eCaptureVerfiyStateLocked], pCtx->refIndex + tempIndex);*/
                            pCtx->state = eCaptureVerfiyStateLocked;
                            pCtx->refIndex += tempIndex;
                            tempIndex = 0;
                            pCtx->mismatchesSinceLastLock = 0;
                            pCtx->samplesSinceLastLock = 0;
                        }
                    }
                    else {
                        /*printf("At sample %lu %s -> %s: Found mismatch at refIndex:%d\n",
                                pCtx->totalSamples, captureVerifyStateName[pCtx->state], captureVerifyStateName[eCaptureVerfiyStateUnlocked], pCtx->refIndex + tempIndex); */
                        pCtx->state = eCaptureVerfiyStateUnlocked;
                    }
                }

                /* Handle Locked state... */
                else if (pCtx->state == eCaptureVerfiyStateLocked) {
                    if (match) {
                        pCtx->totalMatches++;
                        tempIndex++;
                        verifySampleIndex++;
                    }
                    else {
                        /*printf("At sample %lu %s -> %s: Found mismatch at refIndex:%d\n", pCtx->totalSamples, captureVerifyStateName[pCtx->state], captureVerifyStateName[eCaptureVerfiyStateUnlocking], pCtx->refIndex + tempIndex);*/
                        pCtx->state = eCaptureVerfiyStateUnlocking;
                        pCtx->mismatches = 0;
                    }
                }

                /* Handle Unlocking state... */
                else if (pCtx->state == eCaptureVerfiyStateUnlocking) {
                    if (match) {
                        pCtx->totalMatches++;
                        tempIndex++;
                        verifySampleIndex++;
                        /*printf("At sample %lu %s -> %s: Found match at refIndex:%d\n",
                                pCtx->totalSamples, captureVerifyStateName[pCtx->state], captureVerifyStateName[eCaptureVerfiyStateLocked], pCtx->refIndex + tempIndex);*/

                        pCtx->totalMismatches += pCtx->mismatches;  /* Only count mismatches if we're staying locked. */
                        pCtx->mismatchesSinceLastLock += pCtx->mismatches;
                        pCtx->refIndex += tempIndex;
                        tempIndex = 0;

                        pCtx->state = eCaptureVerfiyStateLocked;
                    }
                    else {
                        pCtx->mismatches++;
                        if (pCtx->mismatches > unlockSampleCount) {
                            /*printf("At sample %lu %s -> %s: Found mismatch at refIndex:%d\n", pCtx->totalSamples, captureVerifyStateName[pCtx->state], captureVerifyStateName[eCaptureVerfiyStateUnlocked], pCtx->refIndex + tempIndex);*/
                            pCtx->state = eCaptureVerfiyStateUnlocked;
                        }
                    }
                }
            }

            if (pCtx->state == eCaptureVerfiyStateLocked || pCtx->state == eCaptureVerfiyStateUnlocking) {
                if (pCtx->samplesSinceLastLock > pCtx->samplesInLongestLockPeriod) {
                    pCtx->samplesInLongestLockPeriod    = pCtx->samplesSinceLastLock;
                    pCtx->mismatchesInLongestLockPeriod = pCtx->mismatchesSinceLastLock;
                }
            }
            if (pCtx->totalSamples % 100000 == 0) {
                if (pCtx->state == eCaptureVerfiyStateLocked || pCtx->state == eCaptureVerfiyStateUnlocking) {
                    if (pCtx->samplesSinceLastLock == 0) pCtx->samplesSinceLastLock++;  /* Prevent divide-by-zero */

                    #if 0
                    printf("Sample %luK  State: %s   Since last lock at %luK samples: total:%luK mismatches:%lu (%lu PPM)\n",
                           pCtx->totalSamples/1000,                               /* Sample %luK */
                           captureVerifyStateName[pCtx->state],                   /* State:%s */
                           (pCtx->totalSamples - pCtx->samplesSinceLastLock)/1000,/* Since last lock at %luK samples: */
                           pCtx->samplesSinceLastLock/1000,                       /* total:%luK */
                           pCtx->mismatchesSinceLastLock,                         /* mismatches:%lu */
                           ((pCtx->mismatchesSinceLastLock * 1000000) + (pCtx->samplesSinceLastLock/2)) / pCtx->samplesSinceLastLock );
                    #endif
                }
                else {
                    #if 0
                    printf("Sample %luK  State: %s Since last lock at %luK samples\n",
                           pCtx->totalSamples/1000,                               /* Sample %luK */
                           captureVerifyStateName[pCtx->state] ,
                           (pCtx->totalSamples - pCtx->samplesSinceLastLock)/1000);                 /* State:%s */
                    #endif
                }
            }

        }

        if (captureSampleIndex == captureBufferSize) {
            captureBufferSize = fread(captureBuffer,  sizeof(uint32_t), MAX_BUFFER_SIZE, pCtx->captureFile);
            if (captureBufferSize == 0) {
                /*printf("No more Data in Capture File\n");*/
                done = true;
                break;
            }
            captureSampleIndex = 0;
        }

        if (verifySampleIndex == verifyBufferSize) {
            verifyBufferSize = fread(verifyBuffer,  sizeof(uint32_t), MAX_BUFFER_SIZE, pCtx->verifyFile);
            if (verifyBufferSize == 0) {
                /*printf("No more Data in Verify File\n");*/
                done = true;
                break;
            }
            verifySampleIndex = 0;
        }
    }

done:
    BKNI_Free(captureBuffer);
    BKNI_Free(verifyBuffer);

    return;
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
    printf("  -single_pid secondary audio uses the same pid but different substream as primary\n");
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
    printf("  -once     Exits application when primary completes playback\n");
    printf("\n");
}
