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
 *****************************************************************************/
#include "nexus_platform.h"
#include "nxclient.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bstd.h"

BDBG_MODULE(power_status);

static void print_each_power_status(const NEXUS_PowerStatus *resource, const char *str)
{
    if (resource->clock)
        printf("    %s Clock : %s\n", str, resource->clock==NEXUS_PowerState_eOn?"ON":"OFF");
    if (resource->sram)
        printf("    %s Sram  : %s\n", str, resource->sram==NEXUS_PowerState_eOn?"ON":"OFF");
    if (resource->phy)
        printf("    %s Phy   : %s\n", str, resource->phy==NEXUS_PowerState_eOn?"ON":"OFF");
    if (resource->frequency)
        printf("    %s Freq  : %u\n", str, resource->frequency);
}

static void print_power_status(void)
{
    NEXUS_Error rc;
    NEXUS_PlatformStatus platformStatus;
    unsigned i;
    char buf[64];

    rc = NEXUS_Platform_GetStatus(&platformStatus);
    BDBG_ASSERT(!rc);

    printf("\n");

#ifndef NEXUS_POWER_MANAGEMENT
    printf("NEXUS POWER MANAGEMENT IS DISABLED!!!\n");
#endif
#if NEXUS_HAS_DISPLAY
    printf("\nDisplay\n");
    print_each_power_status(&platformStatus.displayModuleStatus.power.bvn, "BVN");
    for (i=0; i<NEXUS_MAX_DISPLAYS; i++) {
        BKNI_Snprintf(buf, sizeof(buf), "%s %d", "VEC", i);
        print_each_power_status(&platformStatus.displayModuleStatus.power.vec[i], buf);
    }
    for (i=0; i<NEXUS_MAX_VIDEO_DACS; i++) {
        BKNI_Snprintf(buf, sizeof(buf), "%s %d", "Video Dac", i);
        print_each_power_status(&platformStatus.displayModuleStatus.power.dacs[i], buf);
    }
    print_each_power_status(&platformStatus.displayModuleStatus.power.ccir656Output, "656");
#endif
#if NEXUS_HAS_RFM
    print_each_power_status(&platformStatus.rfmModuleStatus.power.core, "Rfm");
#endif
#if NEXUS_HAS_VIDEO_DECODER
    printf("\nVideo Decoder\n");
    for (i=0; i<NEXUS_MAX_VIDEO_DECODERS; i++) {
        BKNI_Snprintf(buf, sizeof(buf), "%s %d", "Video Decoder", i);
        print_each_power_status(&platformStatus.videoDecoderModuleStatus.power.core[i], buf);
    }
#endif
#if NEXUS_HAS_PICTURE_DECODER
    printf("\nPicture Decoder\n");
    print_each_power_status(&platformStatus.pictureDecoderModuleStatus.power.core, "Picture Decoder");
#endif

#if NEXUS_HAS_VIDEO_ENCODER
    printf("\nVideo Encoder\n");
    for (i=0; i<NEXUS_MAX_VIDEO_ENCODERS; i++) {
        BKNI_Snprintf(buf, sizeof(buf), "%s %d", "Video Encoder", i);
        print_each_power_status(&platformStatus.videoEncoderModuleStatus.power.core[i], buf);
    }
#endif
#if NEXUS_HAS_AUDIO
    printf("\nAudio\n");
    print_each_power_status(&platformStatus.audioModuleStatus.power.aio, "Aio");
    for (i=0; i<NEXUS_MAX_AUDIO_PLLS; i++) {
        BKNI_Snprintf(buf, sizeof(buf), "%s %d", "Audio Pll", i);
        print_each_power_status(&platformStatus.audioModuleStatus.power.pll[i], buf);
    }
    for (i=0; i<NEXUS_MAX_AUDIO_DECODERS; i++) {
        BKNI_Snprintf(buf, sizeof(buf), "%s %d", "Audio Decoder", i);
        print_each_power_status(&platformStatus.audioModuleStatus.power.decoder[i], buf);
    }
    for (i=0; i<NEXUS_MAX_AUDIO_DAC_OUTPUTS; i++) {
        BKNI_Snprintf(buf, sizeof(buf), "%s %d", "Audio Dac", i);
        print_each_power_status(&platformStatus.audioModuleStatus.power.dacs[i], buf);
    }
#endif
#if NEXUS_HAS_GRAPHICS2D
    printf("\nGraphics\n");
    for (i=0; i<NEXUS_MAX_GRAPHICS2D_CORES; i++) {
        BKNI_Snprintf(buf, sizeof(buf), "%s %d", "Graphics2D", i);
        print_each_power_status(&platformStatus.graphics2DModuleStatus.power.core[i], buf);
    }
#endif
#if NEXUS_HAS_GRAPHICSV3D
    print_each_power_status(&platformStatus.graphicsv3dModuleStatus.power.core, "Graphics3D");
#endif

    printf("\nHdmi\n");
#if NEXUS_HAS_HDMI_INPUT
    print_each_power_status(&platformStatus.hdmiInputModuleStatus.power.core, "Hdmi Input");
#endif
#if NEXUS_HAS_HDMI_OUTPUT
    for (i=0; i<NEXUS_MAX_HDMI_OUTPUTS; i++) {
        BKNI_Snprintf(buf, sizeof(buf), "%s %d", "Hdmi Output", i);
        print_each_power_status(&platformStatus.hdmiOutputModuleStatus.power.core[i], buf);
    }
#endif
#if NEXUS_HAS_SMARTCARD
    printf("\nSmartcard\n");
    for (i=0; i<NEXUS_MAX_SMARTCARD_CHANNELS; i++) {
        BKNI_Snprintf(buf, sizeof(buf), "%s %d", "Smartcard", i);
        print_each_power_status(&platformStatus.smartcardModuleStatus.power.core[i], buf);
    }
#endif
#if NEXUS_HAS_TRANSPORT
    printf("\nTransport\n");
    print_each_power_status(&platformStatus.transportModuleStatus.power.transport, "Transport");
    print_each_power_status(&platformStatus.transportModuleStatus.power.remux, "Remux");
    print_each_power_status(&platformStatus.transportModuleStatus.power.tsio, "Tsio");
#endif
    printf("\n");
}

int main(int argc, char **argv)
{
    NxClient_JoinSettings joinSettings;
    NEXUS_Error rc;

    BSTD_UNUSED(argc);

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    joinSettings.mode = NEXUS_ClientMode_eVerified;
    rc = NxClient_Join(&joinSettings);
    if (rc) { printf("Failed to join\n"); return -1; }

    print_power_status();

    NxClient_Uninit();

    return 0;
}
