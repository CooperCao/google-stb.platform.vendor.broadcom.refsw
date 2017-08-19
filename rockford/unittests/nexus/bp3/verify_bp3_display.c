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
 *****************************************************************************/

#include "nexus_platform.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_hdmi_output.h"
#include "nexus_hdmi_output_extra.h"
#include "nexus_playback.h"
#include "nexus_file.h"

#include <stdio.h>
#include <stdlib.h>
#include "bstd.h"
#include "bkni.h"

struct {
    NEXUS_LicensedFeature feature;
    const char *fname;
    NEXUS_VideoCodec codec;
    unsigned videoPid;
    bool useHdrEotf;
} testcases[] = {
    { NEXUS_LicensedFeature_eDolbyVision, "videos/volcanos_U30000_mz.ts",     NEXUS_VideoCodec_eH265, 0x21,   true },
    { NEXUS_LicensedFeature_eTchPrime,    "videos/ZombieClip1mn_2020_ldr.ts", NEXUS_VideoCodec_eH265, 0x1001, true }, /* SDR -> HDR */
    { NEXUS_LicensedFeature_eTchPrime,    "videos/ZombieClip1mn_2020_hdr.ts", NEXUS_VideoCodec_eH265, 0x1001, false }, /* HDR -> SDR */
};

#define NUM_TESTCASES (sizeof(testcases) / sizeof(*testcases))

#include "bchp_hdr_cmp_0.h"

bool test_hw_use(NEXUS_LicensedFeature feature)
{
    uint32_t reg, val;
    unsigned i;
    if (feature==NEXUS_LicensedFeature_eDolbyVision) {
        NEXUS_Platform_ReadRegister(BCHP_HDR_CMP_0_CMP_HDR_V0_CTRL, &reg);
        val = BCHP_GET_FIELD_DATA(reg, HDR_CMP_0_CMP_HDR_V0_CTRL, DLBV_CVM_EN);
        BKNI_Printf("DLBV_CVM_EN = %u\n", val);
        if (val) return true;

        NEXUS_Platform_ReadRegister(BCHP_HDR_CMP_0_CMP_HDR_V1_CTRL, &reg);
        val = BCHP_GET_FIELD_DATA(reg, HDR_CMP_0_CMP_HDR_V1_CTRL, DLBV_EL_EN);
        BKNI_Printf("DLBV_EL_EN  = %u\n", val);
        if (val) return true;

        for (i=0; i<BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_ARRAY_END; i++) {
            NEXUS_Platform_ReadRegister(BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_ARRAY_BASE + i*4, &reg);
            val = BCHP_GET_FIELD_DATA(reg, HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, DLBV_COMP_SEL);
            BKNI_Printf("[%2u]DLBV_COMP_SEL = %u\n", i, val);
            if (val!=BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_DLBV_COMP_SEL_DISABLE) return true;
        }
    }
    else if (feature==NEXUS_LicensedFeature_eTchPrime) {
        NEXUS_Platform_ReadRegister(BCHP_HDR_CMP_0_CMP_HDR_V0_CTRL, &reg);
        val = BCHP_GET_FIELD_DATA(reg, HDR_CMP_0_CMP_HDR_V0_CTRL, TP_TONE_MAP_EN);
        BKNI_Printf("TP_TONE_MAP_EN = %u\n", val);
        if (val) return true;
    }
    return false;
}

int main(int argc, const char* argv[])
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_HdmiOutputHandle hdmi;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_HdmiOutputExtraSettings hdmiSettings;
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_Error rc;
    bool supported, hwUsed;
    unsigned t = 0;

    if (argc > 1) {
        t = atoi(argv[1]);
        if (t >= NUM_TESTCASES) {
            fprintf(stderr, "Testcase %u does not exist\n", t);
            return -1;
        }
    }

    if (testcases[t].useHdrEotf) {
        NEXUS_SetEnv("hdmi_crc_test", "y");
    }
    else {
        NEXUS_SetEnv("hdmi_crc_test", NULL);
    }

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);

    NEXUS_Platform_GetConfiguration(&platformConfig);
    hdmi = platformConfig.outputs.hdmi[0];
    BDBG_ASSERT(hdmi);

    playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
    BDBG_ASSERT(playpump);
    playback = NEXUS_Playback_Create();
    BDBG_ASSERT(playback);

    file = NEXUS_FilePlay_OpenPosix(testcases[t].fname, NULL);
    if (!file) {
        fprintf(stderr, "Can't open file:%s\n", testcases[t].fname);
        return -1;
    }

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    playbackSettings.playpumpSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_Playback_SetSettings(playback, &playbackSettings);

    /* Bring up video display and outputs */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e3840x2160p30hz;
    display = NEXUS_Display_Open(0, &displaySettings);
    window = NEXUS_VideoWindow_Open(display, 0);

    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(hdmi));
    rc = NEXUS_HdmiOutput_GetStatus(hdmi, &hdmiStatus);
    if ( !rc && hdmiStatus.connected )
    {
        /* If current display format is not supported by monitor, switch to monitor's preferred format.
           If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(display, &displaySettings);
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
        }
    }

    /* bring up decoder and connect to display */
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoDecoder_GetSettings(videoDecoder,&videoDecoderSettings);
    videoDecoderSettings.maxWidth = 3840;
    videoDecoderSettings.maxHeight = 2160;
    rc = NEXUS_VideoDecoder_SetSettings(videoDecoder,&videoDecoderSettings);
    BDBG_ASSERT(!rc);

    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    NEXUS_HdmiOutput_GetExtraSettings(hdmi, &hdmiSettings);
    if (testcases[t].useHdrEotf) {
        switch (testcases[t].feature) {
            case NEXUS_LicensedFeature_eDolbyVision:
                hdmiSettings.dolbyVision.outputMode = NEXUS_HdmiOutputDolbyVisionMode_eEnabled;
                break;
            case NEXUS_LicensedFeature_eTchPrime:
                hdmiSettings.overrideDynamicRangeMasteringInfoFrame = true;
                hdmiSettings.dynamicRangeMasteringInfoFrame.eotf = NEXUS_VideoEotf_eHdr10;
                break;
            default:
                fprintf(stderr, "Not supported\n");
                return -1;
        }
    }
    NEXUS_HdmiOutput_SetExtraSettings(hdmi, &hdmiSettings);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = testcases[t].codec;
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
    videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, testcases[t].videoPid, &playbackPidSettings);

    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = testcases[t].codec;
    videoProgram.pidChannel = videoPidChannel;

    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
    NEXUS_Playback_Start(playback, file, NULL);

    NEXUS_Platform_IsLicensedFeatureSupported(testcases[t].feature, &supported);
    BKNI_Printf("Stream: %s\n", testcases[t].fname);
    BKNI_Printf("Feature %u, HasLicense %u\n", testcases[t].feature, supported);

    /* wait for decode and display to start */
    BKNI_Sleep(3000);
    hwUsed = test_hw_use(testcases[t].feature);
    BKNI_Printf("HW in use: %u\n", hwUsed);
    if (supported && hwUsed) {
        BKNI_Printf("Feature supported and HW being exercised. Test returning success\n");
        rc = 0;
    }
    else if (!supported && !hwUsed) {
        BKNI_Printf("Feature not supported and HW not being exercised. Test returning success\n");
        rc = 0;
    }
    else {
        BKNI_Printf("Mismatch between feature support and actual HW usage. Test retuning failure\n");
        rc = 1;
    }

    /* Bring down system */
    NEXUS_VideoDecoder_Stop(videoDecoder);
    NEXUS_Playback_Stop(playback);
    NEXUS_FilePlay_Close(file);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);
    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    NEXUS_Platform_Uninit();
    return rc;
}
