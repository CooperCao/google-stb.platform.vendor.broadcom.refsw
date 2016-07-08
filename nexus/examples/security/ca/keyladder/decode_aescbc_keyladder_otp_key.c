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
/* Test stream is spiderman_aescbc_block.ts */

/* 
 CA using keyladder and OTP key as root key.  The OTP keyA shall be 11112222 12345678 33445511 00112233
 for the test to pass.

 The corresponding clear key is  
 {0x79, 0x47, 0xa7, 0x0b, 0xb8, 0xa8, 0x9d, 0xcc, 0x12, 0x75, 0xc5, 0x68, 0xe5, 0x0e, 0x72, 0xe3 };
 */
#if NEXUS_HAS_SECURITY  && (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 1) && NEXUS_HAS_VIDEO_DECODER

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
#include "nexus_keyladder.h"

#define USE_CLEAR_KEY

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
    NEXUS_Error rc;
#endif
	NEXUS_SecurityClearKey key;

#ifdef USE_CLEAR_KEY
	unsigned char clearKey[16] = {0x79, 0x47, 0xa7, 0x0b, 0xb8, 0xa8, 0x9d, 0xcc, 0x12, 0x75, 0xc5, 0x68, 0xe5, 0x0e, 0x72, 0xe3 };
#else
	NEXUS_SecurityEncryptedSessionKey encryptedSessionkey;
	NEXUS_SecurityEncryptedControlWord encrytedCW;
	unsigned char ucProcInForKey3[16] = { 		
		0x0f, 0x09, 0xa2, 0x06,  /*high key */  
		0x19, 0x88, 0xb6, 0x89,	    
		0x28, 0xeb, 0x90, 0x2e,   /*low key  use twice */
		0xb2, 0x36, 0x18, 0x88};

	unsigned char ucProcInKey4[16] = { 0xe4, 0x62, 0x75, 0x1b, 0x5d, 0xd4, 0xbc, 0x02, 0x27, 0x9e, 0xc9, 0x6c, 0xc8, 0x66, 0xec, 0x10 };
#endif                                           
    unsigned char iv[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	unsigned int videoPID, audioPID;
	NEXUS_KeySlotHandle keyHandle = NULL;
	NEXUS_SecurityAlgorithmSettings AlgConfig;
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
    keySlotSettings.keySlotEngine = NEXUS_SecurityEngine_eCa;
	/* Allocate AV keyslots */
    keyHandle = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
	if ( !keyHandle)
	{
		printf("\nAllocate CA keyslot failed \n");
		return 1;
	}

	/* Config AV algorithms */
    NEXUS_Security_GetDefaultAlgorithmSettings(&AlgConfig);
	AlgConfig.algorithm = NEXUS_SecurityAlgorithm_eAes;
	AlgConfig.algorithmVar = NEXUS_SecurityAlgorithmVariant_eCbc;
	AlgConfig.terminationMode = NEXUS_SecurityTerminationMode_eBlock;
	if ( NEXUS_Security_ConfigAlgorithm (keyHandle, &AlgConfig) != 0 )
	{
		printf("\nConfig CA Algorithm failed \n");
		return 1;
	}

#ifdef USE_CLEAR_KEY
	key.keySize = sizeof(clearKey); 
    key.keyEntryType = NEXUS_SecurityKeyType_eEven;
    memcpy(key.keyData,clearKey,key.keySize);
	if ( NEXUS_Security_LoadClearKey (keyHandle, &key) != 0 )
	{
		printf("\nLoad EVEN key failed \n");
		return 1;
	}
    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
	if ( NEXUS_Security_LoadClearKey (keyHandle, &key) != 0 )
	{
		printf("\nLoad ODD key failed \n");
		return 1;
	}
#else
	/* Load AV keys using keyladder */
	/* Generate session key */
	encryptedSessionkey.keyladderType 	= NEXUS_SecurityKeyladderType_e3Des;
	encryptedSessionkey.swizzleType		= NEXUS_SecuritySwizzleType_eNone;
	encryptedSessionkey.cusKeyL 		= 0x00; 
	encryptedSessionkey.cusKeyH 		= 0x00; 
	encryptedSessionkey.cusKeyVarL 		= 0x00; 
	encryptedSessionkey.cusKeyVarH 		= 0xFF; 
    encryptedSessionkey.rootKeySrc 		= NEXUS_SecurityRootKeySrc_eOtpKeyB;
	encryptedSessionkey.bRouteKey 		= false;
	encryptedSessionkey.operation 		= NEXUS_SecurityOperation_eDecrypt;
	encryptedSessionkey.operationKey2 	= NEXUS_SecurityOperation_eEncrypt;
	encryptedSessionkey.keyEntryType 	= NEXUS_SecurityKeyType_eEven;
	/*encryptedSessionkey.pKeyData 		= ucProcInForKey3;*/
	BKNI_Memcpy(encryptedSessionkey.keyData, ucProcInForKey3, sizeof(ucProcInForKey3));

	if (NEXUS_Security_GenerateSessionKey(keyHandle, &encryptedSessionkey) !=0)
	{
		printf("\nLoading session key failed for CA\n");
		return 1;
	}

	/* Load CW for auido and video */
    encrytedCW.keyladderType = NEXUS_SecurityKeyladderType_e3Des;
    encrytedCW.keySize = sizeof(ucProcInKey4); 
    encrytedCW.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    /*encrytedCW.pKeyData = ucProcInKey4;*/
 	BKNI_Memcpy(encrytedCW.keyData, ucProcInKey4, encrytedCW.keySize);
    encrytedCW.operation = NEXUS_SecurityOperation_eDecrypt;     
    encrytedCW.bRouteKey = true;
	if (NEXUS_Security_GenerateControlWord(keyHandle, &encrytedCW))
	{
		printf("\nLoading session key failed for ODD key\n");
		return 1;
	}
    encrytedCW.keyEntryType = NEXUS_SecurityKeyType_eEven;
	if (NEXUS_Security_GenerateControlWord(keyHandle, &encrytedCW))
	{
		printf("\nLoading session key failed for EVEN key\n");
		return 1;
	}
#endif

	/* Load IV */
	key.keySize = sizeof(iv); 
    key.keyEntryType = NEXUS_SecurityKeyType_eIv;
    memcpy(key.keyData,iv,key.keySize);
	if ( NEXUS_Security_LoadClearKey (keyHandle, &key) != 0 )
	{
		printf("\nLoad IV failed \n");
		return 1;
	}

	/* Add video PID channel to keyslot */
	NEXUS_PidChannel_GetStatus (videoProgram.pidChannel, &pidStatus);
	videoPID = pidStatus.pidChannelIndex;
	if ( NEXUS_Security_AddPidChannelToKeySlot(keyHandle, videoPID)!= 0 )
	{
		printf("\nConfigPIDPointerTable failed \n");
		return 1;
	}

	/* Add video PID channel to keyslot */
	NEXUS_PidChannel_GetStatus (audioProgram.pidChannel, &pidStatus);
	audioPID = pidStatus.pidChannelIndex;
	NEXUS_Security_AddPidChannelToKeySlot(keyHandle, audioPID); 

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

#if 1
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
	NEXUS_Security_RemovePidChannelFromKeySlot(keyHandle, videoPID);
	NEXUS_Security_RemovePidChannelFromKeySlot(keyHandle, audioPID);
	NEXUS_Security_FreeKeySlot(keyHandle);

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

#else /* NEXUS_HAS_SECURITY && (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 1) && NEXUS_HAS_VIDEO_DECODER */
#include <stdio.h>
int main ( void )
{
    printf ( "This application is not supported on this platform!\n" );
    return -1;
}
#endif
