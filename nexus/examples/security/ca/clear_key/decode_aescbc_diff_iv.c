/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
#if NEXUS_HAS_SECURITY  && defined(HSM_TEST_TO_BE_INTEGRATED)

/* Nexus example app: single live a/v descrambling and decode from an input band */
/* IVs are different between EVEN/ODD key */
/* EVEN packets are decrypted inband */
/* ODD packets are decrypted by RMX loopback */
/* Input band -> Parser(0) -> CA(EVEN) -> RMX -> parser(1) -> CA(ODD) -> Decode -> Display */
/* Test stream is spiderman_aescbc_diff_iv.ts */

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
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nexus_security.h"

/* the following define the input and its characteristics -- these will vary by input */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3
#define VIDEO_PID 0x11
#define AUDIO_PID 0x14

int main(void)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_InputBand inputBand;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle vdecode;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle pcmDecoder, compressedDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif

	NEXUS_RemuxHandle remux;
    NEXUS_RemuxSettings remuxSettings;
    NEXUS_PidChannelHandle videoPidChannel_loopback, audioPidChannel_loopback;
    NEXUS_Error rc;

	/* The encryption keys are the same for Audio/Video EVEN/ODD keys in this example.  Those
       Keys can be different in a real application */
	unsigned char EvenControlWord[] = { 
		0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 
		0x12, 0x34, 0xfe, 0xed, 0xba, 0xbe, 0xbe, 0xef  };   
    unsigned char OddControlWord[] = { 
		0x12, 0x34, 0xfe, 0xed, 0xba, 0xbe, 0xbe, 0xef,
		0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, };

    unsigned char iv_even[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    unsigned char iv_odd[] = {0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1};

	unsigned int videoPID, audioPID;
	NEXUS_KeySlotHandle videoKeyHandle = NULL;
	NEXUS_KeySlotHandle audioKeyHandle = NULL;
	NEXUS_KeySlotHandle videoKeyHandle_loopback = NULL;
	NEXUS_KeySlotHandle audioKeyHandle_loopback = NULL;
	NEXUS_SecurityAlgorithmSettings AlgConfig;
	NEXUS_SecurityClearKey key;
	NEXUS_PidChannelStatus pidStatus;
	NEXUS_SecurityKeySlotSettings keySlotSettings;

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

	

    /* Get the streamer input band from Platform. Platform has already configured the FPGA with a default streamer routing */
    NEXUS_Platform_GetStreamerInputBand(0, &inputBand);

	

    /* Map a parser band to the streamer input band. */
    parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    parserBandSettings.sourceTypeSettings.inputBand = inputBand;
    parserBandSettings.transportType = TRANSPORT_TYPE;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);
	

    /* Open the audio and video pid channels */
    videoPidChannel = NEXUS_PidChannel_Open(parserBand, VIDEO_PID, NULL);
    audioPidChannel = NEXUS_PidChannel_Open(parserBand, AUDIO_PID, NULL);

	

    /* Configure remux. ie. Map input => remux */
    NEXUS_Remux_GetDefaultSettings(&remuxSettings);
    remuxSettings.allPass = true;
    remux = NEXUS_Remux_Open(0, &remuxSettings);

    rc = NEXUS_Remux_AddPidChannel(remux, videoPidChannel);
    BDBG_ASSERT(!rc);
    rc = NEXUS_Remux_AddPidChannel(remux, audioPidChannel);
    BDBG_ASSERT(!rc);
    rc = NEXUS_Remux_Start(remux); 
    BDBG_ASSERT(!rc);
	

    /* Map remux output to a parser band via loopback route */
    parserBand = NEXUS_ParserBand_e1;
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eRemux;
    parserBandSettings.sourceTypeSettings.remux = remux;
    parserBandSettings.transportType = TRANSPORT_TYPE;
    /*parserBandSettings.allPass = true;*/
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    /* Open a pid channel from parser band */
    videoPidChannel_loopback = NEXUS_PidChannel_Open(parserBand, VIDEO_PID, NULL); 
    audioPidChannel_loopback = NEXUS_PidChannel_Open(parserBand, AUDIO_PID, NULL); 

    /* Open the StcChannel to do lipsync with the PCR */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = videoPidChannel; /* PCR happens to be on video pid */
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = videoPidChannel_loopback;
    videoProgram.stcChannel = stcChannel;
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = AUDIO_CODEC;
    audioProgram.pidChannel = audioPidChannel_loopback;
    audioProgram.stcChannel = stcChannel;

	

    /* Bring up audio decoders and outputs */
    pcmDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    compressedDecoder = NEXUS_AudioDecoder_Open(1, NULL);

#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif

#if NEXUS_NUM_SPDIF_OUTPUTS
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
#endif

#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif

	/***************************************************************************************
		Config CA descrambler 
	***************************************************************************************/
	
    NEXUS_Security_GetDefaultKeySlotSettings(&keySlotSettings);
    keySlotSettings.keySlotEngine = NEXUS_SecurityEngine_eCa;

	/* Allocate AV keyslots */
    videoKeyHandle = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
	if ( !videoKeyHandle)
	{
		printf("\nAllocate CA video keyslot failed \n");
	}
    audioKeyHandle = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
	if ( !audioKeyHandle)
	{
		printf("\nAllocate CA audio keyslot failed \n");
	}
    videoKeyHandle_loopback = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
	if ( !videoKeyHandle_loopback)
	{
		printf("\nAllocate CA video keyslot failed \n");
	}
    audioKeyHandle_loopback = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
	if ( !audioKeyHandle_loopback)
	{
		printf("\nAllocate CA audio keyslot failed \n");
	}

	/* Config AV algorithms */
    NEXUS_Security_GetDefaultAlgorithmSettings(&AlgConfig);
	AlgConfig.algorithm = NEXUS_SecurityAlgorithm_eAes;
	AlgConfig.algorithmVar = NEXUS_SecurityAlgorithmVariant_eCbc;
	AlgConfig.terminationMode = NEXUS_SecurityTerminationMode_eClear;
	AlgConfig.modifyScValue[NEXUS_SecurityPacketType_eGlobal] = false;
	if ( NEXUS_Security_ConfigAlgorithm (videoKeyHandle, &AlgConfig) != 0 )
	{
		printf("\nConfig video CA Algorithm failed \n");
	}
 	if ( NEXUS_Security_ConfigAlgorithm (audioKeyHandle, &AlgConfig) != 0 )
	{
		printf("\nConfig video CA Algorithm failed \n");
	}
	

	AlgConfig.modifyScValue[NEXUS_SecurityPacketType_eGlobal] = true;
	if ( NEXUS_Security_ConfigAlgorithm (videoKeyHandle_loopback, &AlgConfig) != 0 )
	{
		printf("\nConfig video CA Algorithm failed \n");
	}
 	if ( NEXUS_Security_ConfigAlgorithm (audioKeyHandle_loopback, &AlgConfig) != 0 )
	{
		printf("\nConfig video CA Algorithm failed \n");
	}

	
	/* Load AV keys */
	NEXUS_Security_GetDefaultClearKey(&key);
	key.keySize = sizeof(EvenControlWord); 
    key.keyEntryType = NEXUS_SecurityKeyType_eEven;
    memcpy(key.keyData,EvenControlWord,sizeof(EvenControlWord));
	if ( NEXUS_Security_LoadClearKey (videoKeyHandle, &key) != 0 )
	{
		printf("\nLoad video EVEN key failed \n");
	}
	if ( NEXUS_Security_LoadClearKey (audioKeyHandle, &key) != 0 )
	{
		printf("\nLoad audio EVEN key failed \n");
	}

    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    memcpy(key.keyData,OddControlWord,sizeof(OddControlWord));
	if ( NEXUS_Security_LoadClearKey (videoKeyHandle_loopback, &key) != 0 )
	{
		printf("\nLoad video ODD key failed \n");
	}
	if ( NEXUS_Security_LoadClearKey (audioKeyHandle_loopback, &key) != 0 )
	{
		printf("\nLoad audio ODD key failed \n");
	}

    key.keyEntryType = NEXUS_SecurityKeyType_eIv;
    memcpy(key.keyData,iv_even,sizeof(iv_even));
	if ( NEXUS_Security_LoadClearKey (videoKeyHandle, &key) != 0 )
	{
		printf("\nLoad video EVEN IV failed \n");
	}
	if ( NEXUS_Security_LoadClearKey (audioKeyHandle, &key) != 0 )
	{
		printf("\nLoad audio EVEN IV failed \n");
	}

    memcpy(key.keyData,iv_odd,sizeof(iv_odd));
	if ( NEXUS_Security_LoadClearKey (videoKeyHandle_loopback, &key) != 0 )
	{
		printf("\nLoad video ODD IV failed \n");
	}
	if ( NEXUS_Security_LoadClearKey (audioKeyHandle_loopback, &key) != 0 )
	{
		printf("\nLoad audio ODD IV failed \n");
	}

	/* Add video PID channel to ODD keyslot */
	NEXUS_PidChannel_GetStatus (videoProgram.pidChannel, &pidStatus);
	videoPID = pidStatus.pidChannelIndex;
	if ( NEXUS_Security_AddPidChannelToKeySlot(videoKeyHandle_loopback, videoPID)!= 0 )
	{
		printf("\nConfigPIDPointerTable failed \n");
	}

	/* Add audio PID channel to ODD keyslot */
	NEXUS_PidChannel_GetStatus (audioProgram.pidChannel, &pidStatus);
	audioPID = pidStatus.pidChannelIndex;
	if ( NEXUS_Security_AddPidChannelToKeySlot(audioKeyHandle_loopback, audioPID)!= 0 )
	{
		printf("\nConfigPIDPointerTable failed \n");
	} 

	/* Add video PID channel to EVEN keyslot */
	NEXUS_PidChannel_GetStatus (videoPidChannel, &pidStatus);
	videoPID = pidStatus.pidChannelIndex;
	if ( NEXUS_Security_AddPidChannelToKeySlot(videoKeyHandle, videoPID)!= 0 )
	{
		printf("\nConfigPIDPointerTable failed \n");
	}

	/* Add audio PID channel to EVEN keyslot */
	NEXUS_PidChannel_GetStatus (audioPidChannel, &pidStatus);
	audioPID = pidStatus.pidChannelIndex;
	if ( NEXUS_Security_AddPidChannelToKeySlot(audioKeyHandle, audioPID)!= 0 )
	{
		printf("\nConfigPIDPointerTable failed \n");
	} 

	printf ("\nSecurity Config Done\n");

    /* Bring up video display and outputs */
    NEXUS_Display_GetDefaultSettings(&displaySettings);

    display = NEXUS_Display_Open(0, &displaySettings);
    window = NEXUS_VideoWindow_Open(display, 0);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#if NEXUS_NUM_SCART_INPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ScartInput_GetVideoOutputConnector(platformConfig.inputs.scart[0]));
#if NEXUS_NUM_SCART_INPUTS>1
    NEXUS_Display_AddOutput(display, NEXUS_ScartInput_GetVideoOutputConnector(platformConfig.inputs.scart[1]));
#endif
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
    vdecode = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(vdecode));

    /* Start Decoders */
    NEXUS_VideoDecoder_Start(vdecode, &videoProgram);
    NEXUS_AudioDecoder_Start(pcmDecoder, &audioProgram);
    if ( audioProgram.codec == NEXUS_AudioCodec_eAc3 )
    {
        NEXUS_AudioDecoder_Start(compressedDecoder, &audioProgram);
    }

#if 0
    /* Print status while decoding */
    for (;;) {
        NEXUS_VideoDecoderStatus status;
        NEXUS_AudioDecoderStatus audioStatus;
        uint32_t stc;

        NEXUS_VideoDecoder_GetStatus(vdecode, &status);
        NEXUS_StcChannel_GetStc(videoProgram.stcChannel, &stc);
        printf("decode %.4dx%.4d, pts %#x, stc %#x (diff %d) fifo=%d%%\n",
            status.source.width, status.source.height, status.pts, stc, status.ptsStcDifference, status.fifoSize?(status.fifoDepth*100)/status.fifoSize:0);
        NEXUS_AudioDecoder_GetStatus(pcmDecoder, &audioStatus);
        printf("audio0            pts %#x, stc %#x (diff %d) fifo=%d%%\n",
            audioStatus.pts, stc, audioStatus.ptsStcDifference, audioStatus.fifoSize?(audioStatus.fifoDepth*100)/audioStatus.fifoSize:0);
        NEXUS_AudioDecoder_GetStatus(compressedDecoder, &audioStatus);
        if ( audioStatus.started )
        {
            printf("audio1            pts %#x, stc %#x (diff %d) fifo=%d%%\n",
                audioStatus.pts, stc, audioStatus.ptsStcDifference, audioStatus.fifoSize?(audioStatus.fifoDepth*100)/audioStatus.fifoSize:0);
        }
        BKNI_Sleep(1000);
    }
#else
    printf("Press ENTER to stop decode\n");
    getchar();

    /* example shutdown */
	NEXUS_Security_RemovePidChannelFromKeySlot(videoKeyHandle, videoPID);
	NEXUS_Security_RemovePidChannelFromKeySlot(audioKeyHandle, audioPID);
	NEXUS_Security_RemovePidChannelFromKeySlot(videoKeyHandle_loopback, videoPID);
	NEXUS_Security_RemovePidChannelFromKeySlot(audioKeyHandle, audioPID);
	NEXUS_Security_FreeKeySlot(videoKeyHandle);
	NEXUS_Security_FreeKeySlot(audioKeyHandle);
	NEXUS_Security_FreeKeySlot(videoKeyHandle_loopback);
	NEXUS_Security_FreeKeySlot(audioKeyHandle_loopback);

    NEXUS_Remux_Stop(remux);
    NEXUS_AudioDecoder_Close(pcmDecoder);
    NEXUS_AudioDecoder_Close(compressedDecoder);
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    NEXUS_VideoDecoder_Close(vdecode);
    NEXUS_StcChannel_Close(stcChannel);
    NEXUS_PidChannel_Close(videoPidChannel);
    NEXUS_PidChannel_Close(audioPidChannel);
    NEXUS_Platform_Uninit();
#endif

    return 0;
}


#else /* NEXUS_HAS_SECURITY */
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform!\n");
    return -1;
}
#endif
