/******************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/

/* Nexus example app: single live a/v decode from an input band routed through a remux (ie. input band -> remux -> parser band) */
/* We are using loopback to test the 1394 output. For real time scenarios, please disable the loopback and configure the
   REAL_REMUXP_INPUT
 */

#include "nexus_platform.h"
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_spdif_output.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include <stdlib.h>

#undef  REAL_REMUXP_INPUT  /* enable for real SPOD case*/
static void remux_init_muxpins(void);

#ifndef USE_STREAMER
#define USE_STREAMER 0
#endif

int main(void)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_InputBand inputBand;
    NEXUS_RemuxHandle remux;
    NEXUS_RemuxSettings remuxSettings;
    NEXUS_ParserBand parserBand, parserBandRmx;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel, pcrPidChannel;
    NEXUS_PidChannelHandle allPassToRemux;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle vdecode;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle pcmDecoder, compressedDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;


	NEXUS_FrontendUserParameters userParams;
	NEXUS_FrontendHandle frontend;
	NEXUS_FrontendQamSettings qamSettings;
	int i;


    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
/*    platformSettings.openFrontend = false; */ /* For using QAM */
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);
#if USE_STREAMER
		/* Get the streamer input band from Platform. Platform has already configured the FPGA with a default streamer routing */
		NEXUS_Platform_GetStreamerInputBand(0, &inputBand);
#else
		for ( i = 0; i < NEXUS_MAX_FRONTENDS; i++ )
		{
			NEXUS_FrontendCapabilities capabilities;
			frontend = platformConfig.frontend[i];
			if (frontend) {
				NEXUS_Frontend_GetCapabilities(frontend, &capabilities);
				/* Does this frontend support qam? */
				if ( capabilities.qam )
				{
					break;
				}
			}
		}

		if (NULL == frontend )
		{
			fprintf(stderr, "Unable to find QAM-capable frontend\n");
			return 0;
		}

		NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
		qamSettings.frequency = 441 * 1000000;
		qamSettings.mode = NEXUS_FrontendQamMode_e64;
		qamSettings.annex = NEXUS_FrontendQamAnnex_eB;
		qamSettings.lockCallback.callback = NULL;
		qamSettings.lockCallback.context = frontend;

		NEXUS_Frontend_GetUserParameters(frontend, &userParams);
		NEXUS_Frontend_TuneQam(frontend, &qamSettings);
#endif

    parserBandRmx = NEXUS_ParserBand_e0;
    NEXUS_ParserBand_GetSettings(parserBandRmx, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
#if USE_STREAMER
		parserBandSettings.sourceTypeSettings.inputBand = inputBand;
#else
		parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
#endif

#ifdef REAL_REMUXP_INPUT
	remux_init_muxpins();
#endif

	parserBandSettings.allPass = true;
    NEXUS_ParserBand_SetSettings(parserBandRmx, &parserBandSettings);
    /* this is used to feed all data to the remux. in allpass mode, it doesn't matter what pid is specified here */
    allPassToRemux = NEXUS_PidChannel_Open(parserBandRmx, 0, NULL);
    /* this is used for clock recovery. we can't use a PCR pid from a remux->parser band loop around */
    pcrPidChannel = NEXUS_PidChannel_Open(parserBandRmx, 0x11, NULL);

    /* Configure remux */
    NEXUS_Remux_GetDefaultSettings(&remuxSettings);
    remuxSettings.allPass = true;
	remuxSettings.parallelOutput = true;
    remux = NEXUS_Remux_Open(0, &remuxSettings);

    NEXUS_Remux_AddPidChannel(remux, allPassToRemux);

    NEXUS_Remux_Start(remux);

#ifndef REAL_REMUXP_INPUT
    /* Map a parser band to loopback route from remux output Using Parser band 4 */
	/* This is the loopback from Remux to the Parser Band 4 */
    parserBand = NEXUS_ParserBand_e4;
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eRemux;
    parserBandSettings.sourceTypeSettings.remux = remux;
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);
#endif

	/* Open the audio and video pid channels */
    videoPidChannel = NEXUS_PidChannel_Open(parserBand, 0x11, NULL);
    audioPidChannel = NEXUS_PidChannel_Open(parserBand, 0x14, NULL);

    /* Open the StcChannel to do lipsync with the PCR */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = pcrPidChannel;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up
    the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = NEXUS_VideoCodec_eMpeg2;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = NEXUS_AudioCodec_eAc3;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;

    /* Bring up audio decoders and outputs */
    pcmDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    compressedDecoder = NEXUS_AudioDecoder_Open(1, NULL);
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    if ( audioProgram.codec == NEXUS_AudioCodec_eAc3 )
    {
        /* Only pass through AC3 */
        NEXUS_AudioOutput_AddInput(
            NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
            NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
    }
    else
    {
        NEXUS_AudioOutput_AddInput(
            NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
            NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    }

    /* Bring up display and outputs */

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_eNtsc;
    display = NEXUS_Display_Open(0, &displaySettings);
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
    NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
    if (platformConfig.outputs.svideo[0]) {
        NEXUS_Display_AddOutput(display, NEXUS_SvideoOutput_GetConnector(platformConfig.outputs.svideo[0]));
    }

    window = NEXUS_VideoWindow_Open(display, 0);

    /* bring up decoder and connect to display */
    vdecode = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(vdecode));

    /* Start Decoders */
    NEXUS_VideoDecoder_Start(vdecode, &videoProgram);
    NEXUS_AudioDecoder_Start(pcmDecoder, &audioProgram);
    if ( audioProgram.codec == NEXUS_AudioCodec_eAc3 )
    {
        /* Only pass through AC3 */
        NEXUS_AudioDecoder_Start(compressedDecoder, &audioProgram);
    }

	printf("Decoding from the Loopback Source \n");
    printf("Press ENTER to stop decode\n");
    getchar();

    /* example shutdown */
	NEXUS_AudioDecoder_Stop(pcmDecoder);
	   if ( audioProgram.codec == NEXUS_AudioCodec_eAc3 )
	   {
		   NEXUS_AudioDecoder_Stop(compressedDecoder);
	   }
	   NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]));
	   NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]));
	   NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
	   NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
	   NEXUS_AudioDecoder_Close(pcmDecoder);
	   NEXUS_AudioDecoder_Close(compressedDecoder);
	   NEXUS_VideoDecoder_Stop(vdecode);
	   NEXUS_VideoWindow_RemoveAllInputs(window);
	   NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(vdecode));
	   NEXUS_VideoWindow_Close(window);
	   NEXUS_Display_Close(display);
	   NEXUS_VideoDecoder_Close(vdecode);
	   NEXUS_StcChannel_Close(stcChannel);
	   NEXUS_PidChannel_Close(videoPidChannel);
	   NEXUS_PidChannel_Close(audioPidChannel);
	   NEXUS_Platform_Uninit();
    return 0;
}

static void remux_init_muxpins(void)
{
#if BCHP_CHIP == 7401
	/*
	 * Bcm7401 to POD MDI:
	 *
	 * POD_RMXP_VALID, POD_RMXP_CLK, POD_RMXP_SYNC POD_RMXP_DATA[0..6]: GPIO[16..25]
	 * POD_RMXP_DATA7: GPIO_37
	 */

	/*
	 * POD MDO to Bcm7401
	 *
	 * GP_PPKT_SYNC/GP_PPKT_ERROR/GP_PPKT_CLK: GPIO_30/31/33
	 * GP_PPKT_DATA[0..7]: GPIO[26..29]/34/35/51/54
	 * GP_PPKT_SYNC: GPIO_52
	 */

#elif BCHP_CHIP == 7403
	/*
	 * Bcm7403 to POD MDI:
	 *
	 * POD_RMXP_VALID, POD_RMXP_CLK, POD_RMXP_SYNC POD_RMXP_DATA[0..6]: GPIO[16..25]
	 * POD_RMXP_DATA7: GPIO_37
	 */

	/*
	 * POD MDO to Bcm7403
	 *
	 * GP_PPKT_SYNC/GP_PPKT_ERROR/GP_PPKT_CLK: GPIO_30/31/33
	 * GP_PPKT_DATA[0..7]: GPIO[26..29]/34/35/51/54
	 * GP_PPKT_SYNC: GPIO_52
	 */
#elif BCHP_CHIP ==7400

/*POD  ==> Chip

MOSTRT			GPIO_029/POD2CHIP_MISTRT(1)			BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5/6
MOCLK			GPIO_039/POD2CHIP_MICLK
MDO[0..7]		GPIO_[031..038]/POD2CHIP_MDI[0..7]

Chip ==> POD

MISTRT			GPIO_041/CHIP2POD_MOSTRT			BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6/7
MICLK			GPIO_50/CHIP2POD_MOCLK
MDI[0..7]			GPIO_[042..049]/CHIP2POD_MDO[0..7]
*/

#endif

}
