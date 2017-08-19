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
******************************************************************************/

#include "nexus_platform.h"
#include <stdio.h>
#if NEXUS_HAS_DISPLAY
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "bstd.h"
#include "bkni.h"

BDBG_MODULE(app_display_mgmt);

/* the following define the input and its characteristics -- these will vary by input */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define VIDEO_PID 0x31

static unsigned fifo_depth(NEXUS_VideoDecoderHandle vdecode)
{
    NEXUS_VideoDecoderStatus status;
    NEXUS_VideoDecoder_GetStatus(vdecode, &status);
    return status.fifoSize ? status.fifoDepth*100/status.fifoSize : 0;
}

int main(void)
{
    NEXUS_VideoDecoderHandle vdecode;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_VideoDecoderSettings settings;
    NEXUS_ParserBand parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_DisplaySettings displaySettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_Error rc;
#endif
    NEXUS_VideoDecoderReturnFrameSettings returnFrame;
    unsigned count = 0;
    struct {
        NEXUS_StcChannelHandle handle;
        bool valid;
    } stcChannel;

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* Map input band and parser band. This could be eliminated because a straight mapping and TS config is a good default. */
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    NEXUS_Platform_GetStreamerInputBand(0, &parserBandSettings.sourceTypeSettings.inputBand);
    parserBandSettings.transportType = TRANSPORT_TYPE;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    /* Open the pid channels */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = NEXUS_PidChannel_Open(parserBand, VIDEO_PID, NULL);

    /* We need a freerun STC in playback mode (no offset).
    Instead of DPCR clock recovery, we could monitor fifo depth to skew the freerun clock. */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel.handle = NEXUS_StcChannel_Open(0, &stcSettings);
    stcChannel.valid = false;

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e1080p;
    display = NEXUS_Display_Open(0, &displaySettings);
    window = NEXUS_VideoWindow_Open(display, 0);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
#endif

    /* bring up decoder and connect to display */
    vdecode = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(vdecode));

    NEXUS_VideoDecoder_GetSettings(vdecode, &settings);
    settings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilFirstPicture;
    NEXUS_VideoDecoder_SetSettings(vdecode, &settings);

    /* Start Decoders */
    videoProgram.appDisplayManagement = true;
    NEXUS_VideoDecoder_Start(vdecode, &videoProgram);

    NEXUS_VideoDecoder_GetDefaultReturnFrameSettings(&returnFrame);
    for (;;) {
        NEXUS_VideoDecoderFrameStatus frameStatus;
        unsigned num;
        uint32_t stc;
        int diff;

        /* Create a Nexus striped surface for the most recent picture reported by VideoDecoder */
        rc = NEXUS_VideoDecoder_GetDecodedFrames(vdecode, &frameStatus, 1, &num);
        BDBG_ASSERT(!rc);

        if (!num) {
            BKNI_Sleep(1);
            continue;
        }

        if (!stcChannel.valid) {
            if (fifo_depth(vdecode) < 25) {
                BKNI_Sleep(1);
                continue;
            }
            stcChannel.valid = true;
            NEXUS_StcChannel_SetStc(stcChannel.handle, frameStatus.pts);
        }
        NEXUS_StcChannel_GetStc(stcChannel.handle, &stc);

        /* simplistic TSM algo */
        diff = frameStatus.pts > stc ? frameStatus.pts - stc : -1 * (stc - frameStatus.pts);
        if (frameStatus.pts == 0 && !frameStatus.ptsValid) {
            /* unknown */
            returnFrame.display = false;
        }
        else if (diff > 45*2000 || diff < -45*10000) {
            /* reset STC if very early or very late */
            BDBG_WRN(("PTS error (%#x %#x %d). Reset STC.", frameStatus.pts, stc, diff));
            NEXUS_StcChannel_SetStc(stcChannel.handle, frameStatus.pts);
            returnFrame.display = true;
        }
        else if (diff < -4*45) {
            /* drop if a little late */
            BDBG_WRN(("drop picture %d", diff));
            returnFrame.display = false;
        }
        else {
            /* the TSM lock window is -4..20 msec, which is 0..750 with a 4 msec margin on either side. */
            if (diff > 750 + 4*45) {
                /* wait if a little early */
                BKNI_Sleep(diff / 45 - 4);
            }
            returnFrame.display = true;
            BDBG_MSG(("pts %x, stc %x = %d", frameStatus.pts, stc, diff));
        }

        rc = NEXUS_VideoDecoder_ReturnDecodedFrames(vdecode, &returnFrame, 1);
        BDBG_ASSERT(!rc);

        if (++count % 30 == 0) {
            BDBG_WRN(("decode %d pictures, fifo %u%%", count, fifo_depth(vdecode)));
        }
    }
    return 0;
}
#else
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
