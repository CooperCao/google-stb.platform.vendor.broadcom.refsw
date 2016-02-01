/******************************************************************************
 *    (c)2008-2013 Broadcom Corporation
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
******************************************************************************/
/*
 * Nexus example app: single live video decode from an input band, with low
 * latency settings applied.  Mostly the same low latency settings from this app
 * could be used regardless of the source.  However, for IP video sources, since
 * jitter is usually high, you will need to use the eAverage mode and you may
 * need to increase the latency setting itself to avoid passing the jitter
 * directly to the output.  For gaming clients, the eFast mode is usually best,
 * as low latency trumps all other concerns (like smooth video).
 *
 * There are a few other tricks to play at the head-end, depending on the type
 * of content sent to the client.  Our video decoder requires that the start
 * code of the next syntax element be available before processing on the current
 * syntax element can begin (as, usually, no explicit size is given for video
 * elements).  Thus, if the head end is producing a single frame followed by a
 * gap in content in time, followed by another single frame, and so on, then to
 * get each frame to display when it is transmitted (and not when the next frame
 * arrives) requires transmitting additional start codes after each frame.
 * Specifically, for H264 content, send an AUD NAL (0x00,0x00,0x01,0x09) after
 * every frame.
 *
 * Because our transport parsing system is pipelined, and it, too, is looking
 * for the next start code to delimit the ending of the current syntax element,
 * an additional syntax element transmittal is necessary.  This means 2x AUD NAL
 * elements should be transmitted after every frame.
 *
 * Last, our transport packetizer (which is activated when the source is not
 * already an MPEG2 TS) must wait for an entire transport packet's payload
 * length of data to arrive before it will release a packet of data.  This means
 * that an additional padding of 188 zeroes also needs to be transmitted after
 * the frame itself and the two NALs.
 *
 * In summary, for frame-by-frame slide-show-like systems, in order to achieve
 * 1 frame in / 1 frame out timing, one must append 2 AUD NALs and 188 bytes of
 * zero to each frame.
 *
 * The other settings for low latency are described below, each where the 
 * setting is applied to the module it affects.  Some notable restrictions are 
 * that video windows must be the same size as the display (i.e., full-screen), 
 * and that no pictures that rely on future pictures (in display order) for 
 * prediction are allowed in the stream (for MPEG, that means Is and Ps only).
 */

#include "nexus_platform.h"
#include <stdio.h>
#if NEXUS_HAS_DISPLAY
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_video_decoder_extra.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_adj.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "bstd.h"
#include "bkni.h"

/* the following define the input and its characteristics -- these will vary by input */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define VIDEO_PID 0x11

int main(void)
{
    NEXUS_VideoDecoderHandle vdecode;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_VideoDecoderExtendedSettings vidExtSettings;
    NEXUS_ParserBand parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoWindowSettings winSettings;
    NEXUS_VideoWindowMadSettings madSettings;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_DisplaySettings displaySettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_Error rc;
#endif

    /* Bring up all modules for a platform in a default configuraiton for this 
     * platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* Map input band and parser band. This could be eliminated because a 
     * straight mapping and TS config is a good default. */
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    NEXUS_Platform_GetStreamerInputBand(0, &parserBandSettings.sourceTypeSettings.inputBand);
    parserBandSettings.transportType = TRANSPORT_TYPE;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    /* Open the pid channels */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = NEXUS_PidChannel_Open(parserBand, VIDEO_PID, NULL);

    /* By default, StcChannel will configure NEXUS_Timebase with the info it 
     * has */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = videoProgram.pidChannel; /* PCR on video pid */
    videoProgram.stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /* bring up display */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.displayType = NEXUS_DisplayType_eAuto;
    displaySettings.format = NEXUS_VideoFormat_eNtsc;
    displaySettings.aspectRatio = NEXUS_DisplayAspectRatio_e4x3;
    display = NEXUS_Display_Open(0, &displaySettings);
    window = NEXUS_VideoWindow_Open(display, 0);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( !rc && hdmiStatus.connected )
    {
        /* If current display format is not supported by monitor, switch to 
         * monitor's preferred format. If other connected outputs do not support
         * the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(display, &displaySettings);
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
		}
    }
#endif

    /* bring up decoder and connect to display */
    vdecode = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(vdecode));

    /*
     * Disable forced capture.  By default, BVN will capture video from the
     * decoder into a buffer and then play it back out.  This introduces
     * roughly 1 frame of latency.  Please ensure that your video is full
     * screen while capture is off.
     */
    NEXUS_VideoWindow_GetSettings(window, &winSettings);
    winSettings.forceCapture = false;
    NEXUS_VideoWindow_SetSettings(window, &winSettings);

    /*
     * Disable temporal deinterlacing.  By default, the deinterlacer will
     * buffer up 3 fields in order to perform temporal deinterlacing. This
     * removes that 3-field latency.
     */
    NEXUS_VideoWindow_GetMadSettings(window, &madSettings);
    madSettings.gameMode = NEXUS_VideoWindowGameMode_e5Fields_ForceSpatial;
    NEXUS_VideoWindow_SetMadSettings(window, &madSettings);

    /*
     * The following are latency settings related to the decoder and
     * the display manager.
     */
    NEXUS_VideoDecoder_GetExtendedSettings(vdecode, &vidExtSettings);
    /*
     * Early picture delivery mode. Enabling this will permit AVD FW to deliver
     * pictures to the display queue before they have completed decoding.
     */
    vidExtSettings.earlyPictureDeliveryMode = true;
    /*
     * Zero delay output mode.  Enabling this will cause the decoder to deliver
     * a picture to the display queue as soon as it's decoded. This eliminates
     * the pipeline used for picture reordering. This requires the pictures to
     * be encoded in display order. For example, in an MPEG stream only I's and
     * P's are allowed; no B's. This feature is currently only implemented for
     * MPEG video.
     */
    vidExtSettings.zeroDelayOutputMode = true;
    /*
     * The low latency settings apply mostly to the display queue management.
     * Using the eFast setting is best for games, and will achieve the given
     * latency target very quickly.  However, it passes input temporal jitter
     * directly through to the display.  As such, it is no good for most video
     * streams.  Therefore, for video, use the eAverage mode, which introduces
     * a low pass filter over the latency control algorithm.  This will result
     * in smoother video in the presence of input jitter, but will have periods
     * of higher latency while the algorithm converges on the target.
     */
    vidExtSettings.lowLatencySettings.mode = NEXUS_VideoDecoderLowLatencyMode_eFast;
    /*
     * This is the target latency in milliseconds.  It is rounded up to an
     * integer multiple of the source picture time internally.  Setting this
     * to zero does not guarantee zero latency, and may cause oscillation.  It
     * is best to set this to somewhere between 0 and 1 frame's time for the
     * lowest latency setting (eFast mode), and 100 ms or so for the best jitter
     * tolerance (eAverage mode, which translates into smoother video). However,
     * no one number is good for everyone, as much depends on the streaming
     * environment.  You may have to play with this number to compromise
     * between low latency and temporal quality.
     */
    vidExtSettings.lowLatencySettings.latency = 5;
    NEXUS_VideoDecoder_SetExtendedSettings(vdecode, &vidExtSettings);

    /* Start Decoders */
    NEXUS_VideoDecoder_Start(vdecode, &videoProgram);

    for (;;) {
        NEXUS_VideoDecoderStatus status;
        NEXUS_VideoDecoderFifoStatus fifoStatus;
        uint32_t stc;

        NEXUS_VideoDecoder_GetStatus(vdecode, &status);
        NEXUS_VideoDecoder_GetFifoStatus(vdecode, &fifoStatus);
        NEXUS_StcChannel_GetStc(videoProgram.stcChannel, &stc);
        printf("decode %dx%d, pts %#x, stc %#x (diff %d), queueDepth %u pix, bufferLatency %u ms\n",
            status.source.width, status.source.height, status.pts, stc,
            status.ptsStcDifference, status.queueDepth, fifoStatus.bufferLatency);
        BKNI_Sleep(1000);
    }

    getchar();
    return 0;
}
#else
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
