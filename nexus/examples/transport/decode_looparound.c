/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#if NEXUS_HAS_VIDEO_DECODER
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#include "nexus_playpump.h"
#include "nexus_recpump.h"
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>
#include <pthread.h>

struct {
    unsigned videoPid, audioPid;
} g_channels[] = {
    {0x31, 0x34},
    {0x11, 0x14},
};

/**
The example shows three methods of routing live data through transport:
METHOD 1 - no loop around
METHOD 2 - remux loop around
METHOD 3 - record/playback loop around

Loop around is a standard method to allow transport to take a second pass at data.
**/
#define METHOD 3

struct b_pid_channels {
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel, pcrPidChannel;
#if METHOD == 2
    struct {
        NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;
        NEXUS_ParserBand parserBand;
        NEXUS_RemuxHandle remux;
    } remux;
#elif METHOD == 3
    struct {
        NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;
        NEXUS_PlaypumpHandle playpump;
        NEXUS_RecpumpHandle recpump;
        pthread_t thread;
        BKNI_EventHandle event;
        bool done;
    } recplay;
#endif
};

void config_transport(NEXUS_ParserBand parserBand, unsigned channel, struct b_pid_channels *pid_channels);
void unconfig_transport(struct b_pid_channels *pid_channels);

int main(void)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_InputBand inputBand;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_Error rc;
    unsigned i;
    unsigned channel = 0;

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* Get the streamer input band from Platform. Platform has already configured the FPGA with a default streamer routing */
    NEXUS_Platform_GetStreamerInputBand(0, &inputBand);
    parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    parserBandSettings.sourceTypeSettings.inputBand = inputBand;
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    stcChannel = NEXUS_StcChannel_Open(0, NULL);

    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    display = NEXUS_Display_Open(0, &displaySettings);
#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
#endif

    window = NEXUS_VideoWindow_Open(display, 0);
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL);
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    /* channel change loop */
    while (1) {
        struct b_pid_channels pid_channels;

        /* route live parserBand through transport to decodeable pid channels */
        config_transport(parserBand, channel, &pid_channels);

        NEXUS_StcChannel_GetSettings(stcChannel, &stcSettings);
        stcSettings.timebase = NEXUS_Timebase_e0;
        stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
        stcSettings.modeSettings.pcr.pidChannel = pid_channels.pcrPidChannel;
        rc = NEXUS_StcChannel_SetSettings(stcChannel, &stcSettings);
        BDBG_ASSERT(!rc);

        /* Set up decoder Start structures now. We need to know the audio codec to properly set up
        the audio outputs. */
        NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
        videoProgram.codec = NEXUS_VideoCodec_eMpeg2;
        videoProgram.pidChannel = pid_channels.videoPidChannel;
        videoProgram.stcChannel = stcChannel;
        NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
        audioProgram.codec = NEXUS_AudioCodec_eAc3;
        audioProgram.pidChannel = pid_channels.audioPidChannel;
        audioProgram.stcChannel = stcChannel;

        /* Start Decoders */
        NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
        NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);

        /* a few seconds of status */
        for (i=0;i<10;i++) {
            NEXUS_VideoDecoderStatus status;
            NEXUS_AudioDecoderStatus audioStatus;
            uint32_t stc;

            NEXUS_VideoDecoder_GetStatus(videoDecoder, &status);
            NEXUS_StcChannel_GetStc(videoProgram.stcChannel, &stc);
            printf("decode %.4dx%.4d, pts %#x, stc %#x (diff %d) fifo=%d%%\n",
                status.source.width, status.source.height, status.pts, stc, status.ptsStcDifference, status.fifoSize?(status.fifoDepth*100)/status.fifoSize:0);
            NEXUS_AudioDecoder_GetStatus(audioDecoder, &audioStatus);
            printf("audio             pts %#x, stc %#x (diff %d) fifo=%d%%\n",
                audioStatus.pts, stc, audioStatus.ptsStcDifference, audioStatus.fifoSize?(audioStatus.fifoDepth*100)/audioStatus.fifoSize:0);
            BKNI_Sleep(1000);
        }

        printf("changing channels...\n");
        NEXUS_VideoDecoder_Stop(videoDecoder);
        NEXUS_AudioDecoder_Stop(audioDecoder);
        unconfig_transport(&pid_channels);

        channel = 1 - channel; /* toggle */
    }

    return 0;
}

#if METHOD == 3
static void *b_pump_data(void *context)
{
    struct b_pid_channels *pid_channels = context;
    while (!pid_channels->recplay.done) {
        const void *rec_buffer;
        NEXUS_PlaypumpScatterGatherDescriptor desc;
        size_t rec_size;
        size_t num;
        NEXUS_Error rc;

        rc = NEXUS_Recpump_GetDataBuffer(pid_channels->recplay.recpump, &rec_buffer, &rec_size);
        BDBG_ASSERT(!rc);

        if (!rec_size) {
            BKNI_WaitForEvent(pid_channels->recplay.event, BKNI_INFINITE);
            continue;
        }

        /* use scatter/gather to go from record buffer directly to playback HW */
        desc.addr = (void*)rec_buffer;
        desc.length = rec_size;

        rc = NEXUS_Playpump_SubmitScatterGatherDescriptor(pid_channels->recplay.playpump, &desc, 1, &num);
        BDBG_ASSERT(!rc);

        if (!num) {
            BKNI_WaitForEvent(pid_channels->recplay.event, BKNI_INFINITE);
            continue;
        }

        /* calling NEXUS_Recpump_DataReadComplete before the XPT HW has actually read the data. this
        is ok because the recpump buffer will generally be empty (wrap around unlikely) */
        rc = NEXUS_Recpump_DataReadComplete(pid_channels->recplay.recpump, rec_size);
        BDBG_ASSERT(!rc);
    }
    return NULL;
}

static void dataready_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}
#endif

void config_transport(NEXUS_ParserBand parserBand, unsigned channel, struct b_pid_channels *pid_channels)
{
    NEXUS_Error rc;
#if METHOD == 1
    pid_channels->videoPidChannel = NEXUS_PidChannel_Open(parserBand, g_channels[channel].videoPid, NULL);
    pid_channels->audioPidChannel = NEXUS_PidChannel_Open(parserBand, g_channels[channel].audioPid, NULL);
    pid_channels->pcrPidChannel = pid_channels->videoPidChannel;
#elif METHOD == 2
    NEXUS_RemuxSettings remuxSettings;
    NEXUS_ParserBandSettings parserBandSettings;

    pid_channels->remux.videoPidChannel = NEXUS_PidChannel_Open(parserBand, g_channels[channel].videoPid, NULL);
    pid_channels->remux.audioPidChannel = NEXUS_PidChannel_Open(parserBand, g_channels[channel].audioPid, NULL);

    NEXUS_Remux_GetDefaultSettings(&remuxSettings);
    remuxSettings.allPass = false; /* we will route specific pid channels */
    pid_channels->remux.remux = NEXUS_Remux_Open(0, &remuxSettings);
    rc = NEXUS_Remux_AddPidChannel(pid_channels->remux.remux, pid_channels->remux.videoPidChannel);
    BDBG_ASSERT(!rc);
    rc = NEXUS_Remux_AddPidChannel(pid_channels->remux.remux, pid_channels->remux.audioPidChannel);
    BDBG_ASSERT(!rc);
    rc = NEXUS_Remux_Start(pid_channels->remux.remux);
    BDBG_ASSERT(!rc);

    pid_channels->remux.parserBand = NEXUS_ParserBand_e1;
    NEXUS_ParserBand_GetSettings(pid_channels->remux.parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eRemux;
    parserBandSettings.sourceTypeSettings.remux = pid_channels->remux.remux;
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    rc = NEXUS_ParserBand_SetSettings(pid_channels->remux.parserBand, &parserBandSettings);
    BDBG_ASSERT(!rc);

    pid_channels->videoPidChannel = NEXUS_PidChannel_Open(pid_channels->remux.parserBand, g_channels[channel].videoPid, NULL);
    pid_channels->audioPidChannel = NEXUS_PidChannel_Open(pid_channels->remux.parserBand, g_channels[channel].audioPid, NULL);
    pid_channels->pcrPidChannel = pid_channels->remux.videoPidChannel; /* PCR comes from un-remuxed PCR pid */

#elif METHOD == 3
    NEXUS_RecpumpSettings recpumpSettings;
    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;

    pid_channels->recplay.videoPidChannel = NEXUS_PidChannel_Open(parserBand, g_channels[channel].videoPid, NULL);
    pid_channels->recplay.audioPidChannel = NEXUS_PidChannel_Open(parserBand, g_channels[channel].audioPid, NULL);

    BKNI_CreateEvent(&pid_channels->recplay.event);
    pid_channels->recplay.recpump = NEXUS_Recpump_Open(0, NULL);
    NEXUS_Recpump_GetSettings(pid_channels->recplay.recpump, &recpumpSettings);
    recpumpSettings.data.dataReady.callback = dataready_callback;
    recpumpSettings.data.dataReady.context = pid_channels->recplay.event;
    rc = NEXUS_Recpump_SetSettings(pid_channels->recplay.recpump, &recpumpSettings);
    BDBG_ASSERT(!rc);
    NEXUS_Recpump_AddPidChannel(pid_channels->recplay.recpump, pid_channels->recplay.videoPidChannel, NULL);
    NEXUS_Recpump_AddPidChannel(pid_channels->recplay.recpump, pid_channels->recplay.audioPidChannel, NULL);
    rc = NEXUS_Recpump_Start(pid_channels->recplay.recpump);
    BDBG_ASSERT(!rc);

    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
    playpumpOpenSettings.fifoSize = 1024; /* only playing descriptors */
    pid_channels->recplay.playpump = NEXUS_Playpump_Open(0, &playpumpOpenSettings);
    NEXUS_Playpump_GetSettings(pid_channels->recplay.playpump, &playpumpSettings);
    playpumpSettings.dataCallback.callback = dataready_callback;
    playpumpSettings.dataCallback.context = pid_channels->recplay.event;
    NEXUS_Playpump_SetSettings(pid_channels->recplay.playpump, &playpumpSettings);
    rc = NEXUS_Playpump_Start(pid_channels->recplay.playpump);
    BDBG_ASSERT(!rc);

    pid_channels->videoPidChannel = NEXUS_Playpump_OpenPidChannel(pid_channels->recplay.playpump, g_channels[channel].videoPid, NULL);
    pid_channels->audioPidChannel = NEXUS_Playpump_OpenPidChannel(pid_channels->recplay.playpump, g_channels[channel].audioPid, NULL);
    pid_channels->pcrPidChannel = pid_channels->recplay.videoPidChannel; /* PCR comes from un-remuxed PCR pid */

    /* start dedicated thread for pumping data */
    pid_channels->recplay.done = false;
    rc = pthread_create(&pid_channels->recplay.thread, NULL, b_pump_data, pid_channels);
    BDBG_ASSERT(!rc);
#endif
}

void unconfig_transport(struct b_pid_channels *pid_channels)
{
#if METHOD == 1
    NEXUS_PidChannel_Close(pid_channels->videoPidChannel);
    NEXUS_PidChannel_Close(pid_channels->audioPidChannel);
#elif METHOD == 2
    NEXUS_PidChannel_Close(pid_channels->videoPidChannel);
    NEXUS_PidChannel_Close(pid_channels->audioPidChannel);
    NEXUS_Remux_Stop(pid_channels->remux.remux);
    NEXUS_Remux_RemoveAllPidChannels(pid_channels->remux.remux);
    NEXUS_PidChannel_Close(pid_channels->remux.videoPidChannel);
    NEXUS_PidChannel_Close(pid_channels->remux.audioPidChannel);
    NEXUS_Remux_Close(pid_channels->remux.remux);
#elif METHOD == 3
    pid_channels->recplay.done = true;
    BKNI_SetEvent(pid_channels->recplay.event);
    pthread_join(pid_channels->recplay.thread, NULL);

    NEXUS_Recpump_Stop(pid_channels->recplay.recpump);
    NEXUS_Playpump_ClosePidChannel(pid_channels->recplay.playpump, pid_channels->videoPidChannel);
    NEXUS_Playpump_ClosePidChannel(pid_channels->recplay.playpump, pid_channels->audioPidChannel);

    NEXUS_Recpump_Stop(pid_channels->recplay.recpump);
    NEXUS_Recpump_RemoveAllPidChannels(pid_channels->recplay.recpump);

    NEXUS_PidChannel_Close(pid_channels->recplay.videoPidChannel);
    NEXUS_PidChannel_Close(pid_channels->recplay.audioPidChannel);
    NEXUS_Recpump_Close(pid_channels->recplay.recpump);
    NEXUS_Playpump_Close(pid_channels->recplay.playpump);
    BKNI_DestroyEvent(pid_channels->recplay.event);
#endif
}
#else
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
