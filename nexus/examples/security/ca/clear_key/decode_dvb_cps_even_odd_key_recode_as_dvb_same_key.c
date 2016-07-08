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

/* Nexus example app: single live a/v descrambling and decode from an input band */
/* Test stream is superscrambler_spider.ts */
#if NEXUS_HAS_SECURITY  && defined(HSM_TEST_TO_BE_INTEGRATED)

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
#include "nexus_recpump.h"

#include <stdio.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

/* the following define the input file and its characteristics -- these will vary by input file */
#define DATA_FILE_NAME "videos/stream_dvb_cps_same.mpg"
#define INDEX_FILE_NAME "videos/stream_dvb_cps_same.idx"
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3
#define VIDEO_PID 0x11
#define AUDIO_PID 0x14




static void dataready_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void overflow_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    printf("overflow %s\n", (const char *)context);
}

static NEXUS_Error ConfigureKeySlotForDvbCA (NEXUS_KeySlotHandle keyHandle, 
											 unsigned char * pkeyEven, 
											 unsigned char * pKeyOdd,
											 NEXUS_SecurityAlgorithmScPolarity polarity);
static NEXUS_Error ConfigureKeySlotForDVBSameKeyCPS (NEXUS_KeySlotHandle keyHandle, 
											 unsigned char * pkeyEven, 
											 unsigned char * pKeyOdd);

int main(void)
{
    FILE *data_file;
    FILE *index_file;
    const char data_fname[] = DATA_FILE_NAME;
    const char index_fname[] = INDEX_FILE_NAME;
    BKNI_EventHandle event;

	
    NEXUS_PlatformConfiguration platformConfig;

    NEXUS_RecpumpSettings recpumpSettings;
    NEXUS_RecpumpAddPidChannelSettings addPidChannelSettings;
    NEXUS_RecpumpHandle recpump;
    NEXUS_ParserBand parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;
 
	unsigned char VidEvenControlWord[] = { 0xbe, 0xf9, 0xb0, 0x67,0x13, 0xb8, 0xbc, 0x87 };  
    unsigned char VidOddControlWord[] = { 0xbc, 0xfb, 0xb2, 0x69,0x13, 0xba, 0xbe, 0x8b };   
 	unsigned char AudEvenControlWord[] = { 0xd3, 0x94, 0xdd, 0x44,0x13, 0xd5, 0xd1, 0xb9 }; 
    unsigned char AudOddControlWord[] = { 0xd6, 0x91, 0xd8, 0x3f,0x13, 0xd0, 0xd4, 0xb7 };

	/*
                                           
	unsigned char   VidEvenCpsControlWord[] = { 
		                                    0x2e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7, 
											0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73  };   
    unsigned char VidOddCpsControlWord[] = { 
		                                    0x6e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7, 
											0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73  };   
 	unsigned char AudEvenCpsControlWord[] = { 
		                                    0x8e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7, 
											0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73  };   
   unsigned char AudOddCpsControlWord[] = {
		                                    0x0e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7, 
											0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73  };   
*/

	unsigned char VidEvenCpsControlWord[] = { 0xbc, 0x58, 0x4a, 0x91, 0x75, 0x54, 0x1a, 0x1f };  
    unsigned char VidOddCpsControlWord[] = { 0xbc, 0x58, 0x4a, 0x91, 0x75, 0x54, 0x1a, 0x1f };   
 	unsigned char AudEvenCpsControlWord[] = { 0xbc, 0x58, 0x4a, 0x91, 0x75, 0x54, 0x1a, 0x1f }; 
    unsigned char AudOddCpsControlWord[] = { 0xbc, 0x58, 0x4a, 0x91, 0x75, 0x54, 0x1a, 0x1f };

	unsigned int videoPID, audioPID, counter = 0;
	NEXUS_KeySlotHandle videoKeyHandle = NULL;
	NEXUS_KeySlotHandle audioKeyHandle = NULL;
	NEXUS_KeySlotHandle videoKeyHandle2 = NULL;
	NEXUS_KeySlotHandle audioKeyHandle2 = NULL;
	NEXUS_PidChannelStatus pidStatus;
	NEXUS_SecurityKeySlotSettings keySlotSettings;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle vdecode;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle pcmDecoder, compressedDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_Error rc;
#endif

	/* Bring up all modules for a platform in a default configuration for this platform */


#if 0	
		NEXUS_PlatformSettings platformSettings; 
		
   		NEXUS_Platform_GetDefaultSettings(&platformSettings);
    	platformSettings.openFrontend = false;
    	NEXUS_Platform_Init(&platformSettings);
#endif
	

	

	NEXUS_Platform_Init(NULL);  
	

    NEXUS_Platform_GetConfiguration(&platformConfig);
	

    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    NEXUS_Platform_GetStreamerInputBand(0, &parserBandSettings.sourceTypeSettings.inputBand);
    parserBandSettings.transportType = TRANSPORT_TYPE;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    printf("press ENTER to start record\n");
    getchar();
    
    BKNI_CreateEvent(&event);
	

    recpump = NEXUS_Recpump_Open(0, NULL);
    
    data_file = fopen(data_fname, "wb");
    if (!data_file) {
        fprintf(stderr, "can't open data file:%s\n", data_fname);
        goto error;
    }
    index_file = fopen(index_fname, "wb");
    if (!index_file) {
        fprintf(stderr, "can't open index file:%s\n", index_fname);
        goto error;
    }
	

    NEXUS_Recpump_GetSettings(recpump, &recpumpSettings);
    recpumpSettings.data.dataReady.callback = dataready_callback;
    recpumpSettings.data.dataReady.context = event;
    recpumpSettings.index.dataReady.callback = dataready_callback; /* same callback */
    recpumpSettings.index.dataReady.context = event; /* same event */
    recpumpSettings.data.overflow.callback = overflow_callback;
    recpumpSettings.data.overflow.context = "data";
    recpumpSettings.index.overflow.callback = overflow_callback;
    recpumpSettings.index.overflow.context = "index";
    NEXUS_Recpump_SetSettings(recpump, &recpumpSettings);
        
    NEXUS_Recpump_GetDefaultAddPidChannelSettings(&addPidChannelSettings);
    addPidChannelSettings.pidType = NEXUS_PidType_eVideo;
    addPidChannelSettings.pidTypeSettings.video.index = true;
    addPidChannelSettings.pidTypeSettings.video.codec = VIDEO_CODEC;

    videoPidChannel = NEXUS_PidChannel_Open(parserBand, VIDEO_PID, NULL);
    NEXUS_Recpump_AddPidChannel(recpump, videoPidChannel, &addPidChannelSettings);
    audioPidChannel = NEXUS_PidChannel_Open(parserBand, AUDIO_PID, NULL);
    NEXUS_Recpump_AddPidChannel(recpump, audioPidChannel, NULL);

	

    /* Open the StcChannel to do lipsync with the PCR */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = videoPidChannel; /* PCR happens to be on video pid */
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = AUDIO_CODEC;
    audioProgram.pidChannel = audioPidChannel;
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
    keySlotSettings.keySlotEngine = NEXUS_SecurityEngine_eCaCp;
	
	/* Allocate AV keyslots */
    videoKeyHandle = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
	if ( !videoKeyHandle)
	{
		printf("\nAllocate CACP video keyslot failed \n");
		return 1;
	}
    audioKeyHandle = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
	if ( !audioKeyHandle)
	{
		printf("\nAllocate CACP audio keyslot failed \n");
		return 1;
	}

    videoKeyHandle2 = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
	if ( !videoKeyHandle2)
	{
		printf("\nAllocate CACP video keyslot 2 failed \n");
		return 1;
	}
    audioKeyHandle2 = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
	if ( !audioKeyHandle2)
	{
		printf("\nAllocate CACP audio keyslot 2 failed \n");
		return 1;
	}
	printf("\n\n\n After\n");

	/* Config AV algorithms */
	if ( ConfigureKeySlotForDvbCA (videoKeyHandle, 
									VidEvenControlWord, 
									VidOddControlWord, 
									NEXUS_SecurityAlgorithmScPolarity_eEven) != 0 )
	{
		printf("\nConfig video CA Algorithm failed for video keyslot 1 \n");
		return 1;
	}
	printf("\n\n\n After\n");

	if ( ConfigureKeySlotForDvbCA (videoKeyHandle2, 
									VidEvenControlWord, 
									VidOddControlWord, 
									NEXUS_SecurityAlgorithmScPolarity_eOdd) != 0 )
	{
		printf("\nConfig video CA Algorithm failed for video keyslot 2 \n");
		return 1;
	}
   
	if ( ConfigureKeySlotForDvbCA (audioKeyHandle, 
									AudEvenControlWord, 
									AudOddControlWord, 
									NEXUS_SecurityAlgorithmScPolarity_eEven) != 0 )
	{
		printf("\nConfig audio CA Algorithm failed for audio keyslot 1 \n");
		return 1;
	}

	if ( ConfigureKeySlotForDvbCA (audioKeyHandle2, 
									AudEvenControlWord, 
									AudOddControlWord, 
									NEXUS_SecurityAlgorithmScPolarity_eOdd) != 0 )
	{
		printf("\nConfig audio CA Algorithm failed for audio keyslot 2 \n");
		return 1;
	}

	/***************************************************************************************
		Config CPS (CP scrambler) 
	***************************************************************************************/
	if ( ConfigureKeySlotForDVBSameKeyCPS (videoKeyHandle, 
									VidEvenCpsControlWord, 
									VidOddCpsControlWord) != 0 )
	{
		printf("\nConfig video CPS Algorithm failed for video keyslot 1 \n");
		return 1;
	}

	if ( ConfigureKeySlotForDVBSameKeyCPS (videoKeyHandle2, 
									VidEvenCpsControlWord, 
									VidOddCpsControlWord) != 0 )
	{
		printf("\nConfig video CPS Algorithm failed for video keyslot 2 \n");
		return 1;
	}
   
	if ( ConfigureKeySlotForDVBSameKeyCPS (audioKeyHandle, 
									AudEvenCpsControlWord, 
									AudOddCpsControlWord) != 0 )
	{
		printf("\nConfig audio CPS Algorithm failed for audio keyslot 1 \n");
		return 1;
	}

	if ( ConfigureKeySlotForDVBSameKeyCPS (audioKeyHandle2, 
									AudEvenCpsControlWord, 
									AudOddCpsControlWord) != 0 )
	{
		printf("\nConfig audio CPS Algorithm failed for audio keyslot 2 \n");
		return 1;
	}

	/* Add video PID channel to keyslot */
	NEXUS_PidChannel_GetStatus (videoPidChannel, &pidStatus);
	videoPID = pidStatus.pidChannelIndex;
	if ( NEXUS_Security_AddPidChannelToKeySlot(videoKeyHandle, videoPID)!= 0 )
	{
		printf("\nConfigPIDPointerTable failed \n");
		return 1;
	}

	NEXUS_PidChannel_GetStatus (audioPidChannel, &pidStatus);
	audioPID = pidStatus.pidChannelIndex;
	NEXUS_Security_AddPidChannelToKeySlot(audioKeyHandle, audioPID); 

	printf ("\nSecurity Config OK\n");

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

    NEXUS_Recpump_Start(recpump);

    while (1) {
        const void *data_buffer, *index_buffer;
        size_t data_buffer_size, index_buffer_size;
        int n;

        if (NEXUS_Recpump_GetDataBuffer(recpump, &data_buffer, &data_buffer_size))
            break;
        if (NEXUS_Recpump_GetIndexBuffer(recpump, &index_buffer, &index_buffer_size))
            break;
        if (data_buffer_size == 0 && index_buffer_size == 0) {
            BKNI_WaitForEvent(event, BKNI_INFINITE);
            continue;
		}
	       
			if (data_buffer_size) {
				n = fwrite(data_buffer, 1, data_buffer_size, data_file);
				if (n < 0) goto error;
				NEXUS_Recpump_DataReadComplete(recpump, n);
		}
			if (index_buffer_size) {
				n = fwrite(index_buffer, 1, index_buffer_size, index_file);
				if (n < 0) goto error;
				NEXUS_Recpump_IndexReadComplete(recpump, n);
		}

		/* Switch SC polarity for output packets */
		counter++;
		if ( (counter & 1) == 0 )
		{
			NEXUS_Security_AddPidChannelToKeySlot(videoKeyHandle, videoPID);
			NEXUS_Security_AddPidChannelToKeySlot(audioKeyHandle, audioPID); 
		}
		else
		{
			NEXUS_Security_AddPidChannelToKeySlot(videoKeyHandle2, videoPID);
			NEXUS_Security_AddPidChannelToKeySlot(audioKeyHandle2, audioPID); 
		}

	}

	
	
    /* loops forever */
error:
    return 1;
}

static NEXUS_Error ConfigureKeySlotForDvbCA (NEXUS_KeySlotHandle keyHandle, 
											 unsigned char * pkeyEven, 
											 unsigned char * pKeyOdd,
											 NEXUS_SecurityAlgorithmScPolarity polarity)
{
	NEXUS_SecurityAlgorithmSettings AlgConfig;
	NEXUS_SecurityClearKey key;

    NEXUS_Security_GetDefaultAlgorithmSettings(&AlgConfig);
	AlgConfig.algorithm = NEXUS_SecurityAlgorithm_eDvb;
	AlgConfig.dvbScramLevel = NEXUS_SecurityDvbScrambleLevel_eTs;
	AlgConfig.dest = NEXUS_SecurityAlgorithmConfigDestination_eCa;
	AlgConfig.modifyScValue[NEXUS_SecurityPacketType_eGlobal] = true; 
	AlgConfig.scValue[NEXUS_SecurityPacketType_eGlobal] = polarity; 
	if ( NEXUS_Security_ConfigAlgorithm (keyHandle, &AlgConfig) != 0 )
	{
		printf("\nConfig video CPS Algorithm failed \n");
		return 1;
	}
       
	/* Load AV keys */
	NEXUS_Security_GetDefaultClearKey(&key);
	key.keySize = 8; 
    key.keyEntryType = NEXUS_SecurityKeyType_eEven;
 	key.dest =NEXUS_SecurityAlgorithmConfigDestination_eCa;
    memcpy(key.keyData,pkeyEven,key.keySize);
	if ( NEXUS_Security_LoadClearKey (keyHandle, &key) != 0 )
	{
		printf("\nLoad CA EVEN key failed \n");
		return 1;
	}
    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    memcpy(key.keyData,pKeyOdd,key.keySize);
	if ( NEXUS_Security_LoadClearKey (keyHandle, &key) != 0 )
	{
		printf("\nLoad CA ODD key failed \n");
		return 1;
	}
	return 0;

}

static NEXUS_Error ConfigureKeySlotForDVBSameKeyCPS (NEXUS_KeySlotHandle keyHandle, 
											 unsigned char * pkeyEven, 
											 unsigned char * pKeyOdd)
{
	NEXUS_SecurityAlgorithmSettings AlgConfig;
	NEXUS_SecurityClearKey key;


    NEXUS_Security_GetDefaultAlgorithmSettings(&AlgConfig);
	AlgConfig.algorithm = NEXUS_SecurityAlgorithm_eDvb;
	AlgConfig.algorithmVar = NEXUS_SecurityDvbScrambleLevel_eTs;
	AlgConfig.terminationMode = NEXUS_SecurityTerminationMode_eCipherStealing;
	AlgConfig.dest = NEXUS_SecurityAlgorithmConfigDestination_eCps;
	/* Since the SC bits were modified by CA. No need to modify it by CPS */
	AlgConfig.modifyScValue[NEXUS_SecurityPacketType_eGlobal] = false; 
	if ( NEXUS_Security_ConfigAlgorithm (keyHandle, &AlgConfig) != 0 )
	{
		printf("\nConfig video CPS Algorithm failed \n");
		return 1;
	}
       
	/* Load AV keys */
	NEXUS_Security_GetDefaultClearKey(&key);
	key.keySize = 8; 
    key.keyEntryType = NEXUS_SecurityKeyType_eEven;
	key.dest =NEXUS_SecurityAlgorithmConfigDestination_eCps;
    memcpy(key.keyData,pkeyEven,key.keySize);
	if ( NEXUS_Security_LoadClearKey (keyHandle, &key) != 0 )
	{
		printf("\nLoad CPS EVEN key failed \n");
		return 1;
	}
    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    memcpy(key.keyData,pKeyOdd,key.keySize);
	if ( NEXUS_Security_LoadClearKey (keyHandle, &key) != 0 )
	{
		printf("\nLoad CPS ODD key failed \n");
		return 1;
	}
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
