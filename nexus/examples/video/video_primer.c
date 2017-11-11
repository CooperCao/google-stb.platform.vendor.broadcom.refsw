/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "nexus_video_decoder_primer.h"
#include "nexus_video_decoder_extra.h"
#include "nexus_stc_channel.h"
#include "nexus_timebase.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "bstd.h"
#include "bkni.h"
#include <string.h>
#include <stdlib.h>

BDBG_MODULE(video_primer);

/* the following define the input and its characteristics -- these will vary by input */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2

/* reduce to 2 so we can see toggle */
#define TOTAL_PRIMERS 2
unsigned short g_pid[TOTAL_PRIMERS] = {0x11, 0x31};
#define FCC_PTS_OFFSET (1000*45)

static int tune_qam(NEXUS_ParserBand parserBand, unsigned freq);
static int tune_qam_reapply(void);

static void first_pts_passed(void *context, int param)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
    BDBG_WRN(("first pts passed"));
}

int main(int argc, char **argv)
{
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    NEXUS_VideoDecoderPrimerHandle primer[TOTAL_PRIMERS];
    NEXUS_VideoDecoderOpenSettings openSettings;
    NEXUS_VideoDecoderStartSettings videoProgram[TOTAL_PRIMERS];
    NEXUS_ParserBand parserBand[TOTAL_PRIMERS];
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_Timebase timebase = NEXUS_Timebase_e0;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
    NEXUS_Error rc;
    unsigned index = 0;
    unsigned i;
    int curarg = 1;
    unsigned qam_freq = 0;

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-qam") && curarg+1 < argc) {
            qam_freq = atoi(argv[++curarg]);
        }
        curarg++;
    }

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = qam_freq;
    rc = NEXUS_Platform_Init(&platformSettings);
    if (rc) return rc;
    NEXUS_Platform_GetConfiguration(&platformConfig);

    parserBand[0] = NEXUS_ParserBand_Open(NEXUS_ANY_ID);
    if (qam_freq) {
        rc = tune_qam(parserBand[0], qam_freq);
        if (rc) return rc;
    }
    else {
        NEXUS_ParserBand_GetSettings(parserBand[0], &parserBandSettings);
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        NEXUS_Platform_GetStreamerInputBand(0, &parserBandSettings.sourceTypeSettings.inputBand);
        parserBandSettings.transportType = TRANSPORT_TYPE;
        NEXUS_ParserBand_SetSettings(parserBand[0], &parserBandSettings);
    }

    /* Open the pid channels */
    for (i=0;i<TOTAL_PRIMERS;i++) {
        NEXUS_StcChannelSettings stcSettings;

        if (i > 0) {
            /* one parser band per program ensures ample bandwidth */
            parserBand[i] = NEXUS_ParserBand_Open(NEXUS_ANY_ID);
            NEXUS_ParserBand_GetSettings(parserBand[0], &parserBandSettings);
            NEXUS_ParserBand_SetSettings(parserBand[i], &parserBandSettings);
        }
    
        NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram[i]);
        videoProgram[i].codec = VIDEO_CODEC;
        videoProgram[i].pidChannel = NEXUS_PidChannel_Open(parserBand[i], g_pid[i], NULL);

        NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
        stcSettings.autoConfigTimebase = false; /* must do it manually */
        stcSettings.timebase = timebase;
        stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
        stcSettings.modeSettings.pcr.pidChannel = videoProgram[i].pidChannel;
        stcSettings.modeSettings.pcr.offsetThreshold = 0xFF;
        stcSettings.stcIndex = 0;
        videoProgram[i].stcChannel = NEXUS_StcChannel_Open(i, &stcSettings);
    }

    if (qam_freq) {
        tune_qam_reapply();
    }

    /* bring up display */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
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
        /* If current display format is not supported by monitor, switch to monitor's preferred format.
           If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(display, &displaySettings);
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
        }
    }
#endif

    /* bring up decoder and connect to display */
    NEXUS_VideoDecoder_GetDefaultOpenSettings(&openSettings);
    openSettings.fifoSize = 8 * 1024 * 1024;
    videoDecoder = NEXUS_VideoDecoder_Open(0, &openSettings);
    NEXUS_VideoDecoder_GetSettings(videoDecoder, &videoDecoderSettings);
    videoDecoderSettings.ptsOffset = FCC_PTS_OFFSET;
    videoDecoderSettings.firstPtsPassed.callback = first_pts_passed;
    rc = NEXUS_VideoDecoder_SetSettings(videoDecoder, &videoDecoderSettings);
    BDBG_ASSERT(!rc);
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

/* you can turn off the primer to compare channel change with no acceleration */
#define USE_PRIMER 1

#if USE_PRIMER
    for (i=0;i<TOTAL_PRIMERS;i++) {
        NEXUS_VideoDecoderPrimerSettings primerSettings;
        primer[i] = NEXUS_VideoDecoderPrimer_Create(NULL);

        NEXUS_VideoDecoderPrimer_GetSettings(primer[i], &primerSettings);
        primerSettings.ptsOffset = FCC_PTS_OFFSET; /* provide a default pts offset for faster change */
        primerSettings.ptsStcDiffCorrectionEnabled = true;
        NEXUS_VideoDecoderPrimer_SetSettings(primer[i], &primerSettings);

        NEXUS_VideoDecoderPrimer_Start(primer[i], &videoProgram[i]);
    }
#endif

    while (1) {
        NEXUS_TimebaseSettings timebaseSettings;

        NEXUS_Timebase_GetSettings(timebase, &timebaseSettings);
        timebaseSettings.sourceType = NEXUS_TimebaseSourceType_ePcr;
        timebaseSettings.sourceSettings.pcr.pidChannel = videoProgram[index].pidChannel;
        timebaseSettings.sourceSettings.pcr.maxPcrError = 0xff;
        timebaseSettings.sourceSettings.pcr.trackRange = NEXUS_TimebaseTrackRange_e61ppm;
        NEXUS_Timebase_SetSettings(timebase, &timebaseSettings);

        BDBG_WRN(("start decode. press ENTER to change channels."));
#if USE_PRIMER
        NEXUS_VideoDecoderPrimer_StopPrimerAndStartDecode(primer[index], videoDecoder);
#else
        NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram[index]);
#endif

        getchar();

#if USE_PRIMER
        NEXUS_VideoDecoderPrimer_StopDecodeAndStartPrimer(primer[index], videoDecoder);
#else
        NEXUS_VideoDecoder_Stop(videoDecoder);
#endif

        if (++index == TOTAL_PRIMERS) index = 0;
    }

    return 0;
}

#if NEXUS_HAS_FRONTEND
static NEXUS_FrontendHandle frontend;
static int tune_qam(NEXUS_ParserBand parserBand, unsigned freq)
{
    NEXUS_FrontendAcquireSettings frontendAcquireSettings;
    NEXUS_FrontendQamSettings qamSettings;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_ParserBandSettings parserBandSettings;
    int rc;

    NEXUS_Frontend_GetDefaultAcquireSettings(&frontendAcquireSettings);
    frontendAcquireSettings.capabilities.qam = true;
    frontend = NEXUS_Frontend_Acquire(&frontendAcquireSettings);
    if (!frontend) {
        BDBG_WRN(("Unable to find QAM-capable frontend. Run nxserver with -frontend option."));
        return -1;
    }

    NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
    qamSettings.frequency = freq * 1000000;
    qamSettings.mode = NEXUS_FrontendQamMode_e64;
    qamSettings.symbolRate = 5056900;
    NEXUS_Frontend_GetUserParameters(frontend, &userParams);
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    if (userParams.isMtsif) {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
        parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(frontend); /* NEXUS_Frontend_TuneXyz() will connect this frontend to this parser band */
    }
    else {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
    }
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    rc = NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Frontend_TuneQam(frontend, &qamSettings);
    BDBG_ASSERT(!rc);

    return 0;
}
static int tune_qam_reapply(void)
{
    return NEXUS_Frontend_ReapplyTransportSettings(frontend);
}
#else
static int tune_qam(NEXUS_ParserBand parserBand, unsigned freq)
{
    BSTD_UNUSED(parserBand);
    BSTD_UNUSED(freq);
    return 0;
}
static int tune_qam_reapply(void)
{
    return 0;
}
#endif
#else
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
