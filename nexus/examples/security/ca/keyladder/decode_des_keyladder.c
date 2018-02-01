/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

/* Nexus example app: single live a/v descrambling and decode from an input band */
/* Test stream is liar40sec_Enc.ts  */

#if NEXUS_HAS_SECURITY && (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 4) && NEXUS_HAS_VIDEO_DECODER

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
#include <string.h>
#if NEXUS_DTV_PLATFORM
#include "nexus_platform_boardcfg.h"
#endif
#include "nexus_security.h"
#include "nexus_keyladder.h"


static int Security_AllocateVkl ( NEXUS_SecurityCustomerSubMode custSubMode, NEXUS_SecurityVirtualKeyladderID * vkl )
{
    NEXUS_SecurityVKLSettings vklSettings;
    NEXUS_VirtualKeyLadderHandle vklHandle;
    NEXUS_VirtualKeyLadderInfo vklInfo;

    BDBG_ASSERT ( vkl );

    NEXUS_Security_GetDefaultVKLSettings ( &vklSettings );

    /* For pre Zeus 3.0, please set vklSettings.custSubMode */
	vklSettings.custSubMode = custSubMode;

    vklHandle = NEXUS_Security_AllocateVKL ( &vklSettings );

    if ( !vklHandle )
    {
        printf ( "\nAllocate VKL failed \n" );
        return 1;
    }

    NEXUS_Security_GetVKLInfo ( vklHandle, &vklInfo );
    printf ( "\nVKL Handle %p is allocated for VKL#%d\n", ( void * ) vklHandle, vklInfo.vkl & 0x7F );

    /* For Zeus 4.2 or later
     * if ((vklInfo.vkl & 0x7F ) >= NEXUS_SecurityVirtualKeyLadderID_eMax)
     * {
     * printf ( "\nAllocate VKL failed with invalid VKL Id.\n" );
     * return 1;
     * }
     */

    *vkl = vklInfo.vkl;

    return 0;
}

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
    NEXUS_SecurityVirtualKeyladderID vklId;

	unsigned char ucProcInForKey3[16] = {
		0xCB, 0x9E, 0x69, 0x79, 0xED, 0x74, 0x39, 0x1A,
		0xE6, 0x5D, 0xFF, 0xAD, 0x80, 0x49, 0xDE, 0xF5
		                                };

	unsigned char ucTempKey[16];

	/* DES only requires 8 bytes procins for key4 generation since the DES key size is 8 bytes */
	/* For AES128 and 3DES-ABA, the procinsize shall be 16 bytes */
	unsigned char ucProcInForVidOddKey4[16] = { 0xB5, 0x34, 0xD4, 0xCF, 0xB1, 0x9C, 0x7D, 0xA2, 0xB5, 0x34, 0xD4, 0xCF, 0xB1, 0x9C, 0x7D, 0xA2 };
	unsigned char ucProcInForVidEvenKey4[16] = { 0xB8, 0x04, 0xE2, 0x9D, 0x13, 0xD1, 0x91, 0xAE, 0xB8, 0x04, 0xE2, 0x9D, 0x13, 0xD1, 0x91, 0xAE };
	unsigned char ucProcInForAudOddKey4[16] = { 0xB5, 0x34, 0xD4, 0xCF, 0xB1, 0x9C, 0x7D, 0xA2, 0xB5, 0x34, 0xD4, 0xCF, 0xB1, 0x9C, 0x7D, 0xA2 };
	unsigned char ucProcInForAudEvenKey4[16] = { 0xB8, 0x04, 0xE2, 0x9D, 0x13, 0xD1, 0x91, 0xAE, 0xB8, 0x04, 0xE2, 0x9D, 0x13, 0xD1, 0x91, 0xAE };



	unsigned int videoPID, audioPID, i;
	NEXUS_KeySlotHandle videoKeyHandle = NULL;
	NEXUS_KeySlotHandle audioKeyHandle = NULL;
	NEXUS_SecurityAlgorithmSettings AlgConfig;
	NEXUS_PidChannelStatus pidStatus;
	NEXUS_SecurityKeySlotSettings keySlotSettings;
	NEXUS_SecurityEncryptedSessionKey encryptedSessionkey;
	NEXUS_SecurityEncryptedControlWord encrytedCW;


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
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    /* Open the audio and video pid channels */
    videoPidChannel = NEXUS_PidChannel_Open(parserBand, 0x1E1, NULL);
    audioPidChannel = NEXUS_PidChannel_Open(parserBand, 0x1E2, NULL);

    /* Open the StcChannel to do lipsync with the PCR */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = videoPidChannel; /* PCR happens to be on video pid */
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

	/***************************************************************************************
		Config CA descrambler
	***************************************************************************************/
	/* Request for an VKL to use */
	if ( Security_AllocateVkl ( NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4, &vklId ) )
	{
		printf ( "\nAllocate VKL failed.\n" );
		return 1;
	}

    NEXUS_Security_GetDefaultKeySlotSettings(&keySlotSettings);
    keySlotSettings.keySlotEngine = NEXUS_SecurityEngine_eCa;
	/* Allocate AV keyslots */
    videoKeyHandle = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
	if ( !videoKeyHandle)
	{
		printf("\nAllocate CA video keyslot failed \n");
		return 1;
	}
    audioKeyHandle = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
	if ( !audioKeyHandle)
	{
		printf("\nAllocate CA audio keyslot failed \n");
		return 1;
	}

	/* Config AV algorithms */
    NEXUS_Security_GetDefaultAlgorithmSettings(&AlgConfig);
	AlgConfig.algorithm 		  = NEXUS_SecurityAlgorithm_eDes;
	AlgConfig.algorithmVar		  = NEXUS_SecurityAlgorithmVariant_eEcb;

	/* ++++++++ */
	AlgConfig.terminationMode	  = NEXUS_SecurityTerminationMode_eCipherStealing;
	AlgConfig.ivMode              = NEXUS_SecurityIVMode_eRegular;
	AlgConfig.solitarySelect      = NEXUS_SecuritySolitarySelect_eClear;
	AlgConfig.caVendorID          = 0x1234;
	AlgConfig.askmModuleID        = NEXUS_SecurityAskmModuleID_eModuleID_3;
	AlgConfig.otpId               = NEXUS_SecurityOtpId_eOtpVal;
	AlgConfig.key2Select		  = NEXUS_SecurityKey2Select_eReserved1;
	/* ++++++++ */

	if ( NEXUS_Security_ConfigAlgorithm (videoKeyHandle, &AlgConfig) != 0 )
	{
		printf("\nConfig video CA Algorithm failed \n");
		return 1;
	}
 	if ( NEXUS_Security_ConfigAlgorithm (audioKeyHandle, &AlgConfig) != 0 )
	{
		printf("\nConfig video CA Algorithm failed \n");
		return 1;
	}

	/* Load AV keys using keyladder */
	/* Generate session key */

	NEXUS_Security_GetDefaultSessionKeySettings(&encryptedSessionkey);
	encryptedSessionkey.keyladderType 	= NEXUS_SecurityKeyladderType_e3Des;
	encryptedSessionkey.swizzleType		= NEXUS_SecuritySwizzleType_eNone;
	encryptedSessionkey.cusKeyL 		= 0x00;
	encryptedSessionkey.cusKeyH 		= 0x00;
	encryptedSessionkey.cusKeyVarL 		= 0x00;
	encryptedSessionkey.cusKeyVarH 		= 0xFF;
	encryptedSessionkey.keyGenCmdID 	= NEXUS_SecurityKeyGenCmdID_eKeyGen;
	encryptedSessionkey.sessionKeyOp	= NEXUS_SecuritySessionKeyOp_eNoProcess;
	encryptedSessionkey.bASKMMode		= false;
	encryptedSessionkey.rootKeySrc		= NEXUS_SecurityRootKeySrc_eOtpKeyA;
	encryptedSessionkey.bSwapAESKey 	= false;
    encryptedSessionkey.keyDestIVType   = NEXUS_SecurityKeyIVType_eNoIV;
	encryptedSessionkey.bRouteKey 		= false;
	encryptedSessionkey.operation 		= NEXUS_SecurityOperation_eDecrypt;
	encryptedSessionkey.operationKey2 	= NEXUS_SecurityOperation_eEncrypt;
	encryptedSessionkey.keyEntryType 	= NEXUS_SecurityKeyType_eEven;

	/* ++++++++ */
	/*encryptedSessionkey.custSubMode        = NEXUS_SecurityCustomerSubMode_eGeneric_CA_64_4; */
	encryptedSessionkey.custSubMode        = NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4;
	encryptedSessionkey.virtualKeyLadderID = vklId;
	encryptedSessionkey.keyMode            = NEXUS_SecurityKeyMode_eRegular;
	/* ++++++++ */

	/* 8 bytes swapping needed since BCM is using BA key mode instead of AB */
	for (i=0 ; i < 8; i++)
	{

		ucTempKey[i]     = ucProcInForKey3[i + 8];
		ucTempKey[8 + i] = ucProcInForKey3[i];

	}
	/*BKNI_Memcpy(ucTempKey, ucProcInForKey3, sizeof(ucTempKey));*/

	BKNI_Memcpy(encryptedSessionkey.keyData, ucTempKey, sizeof(ucTempKey));

	if (NEXUS_Security_GenerateSessionKey(videoKeyHandle, &encryptedSessionkey) !=0)
	{
		printf("\nLoading session key failed for video\n");
		return 1;
	}

	/* Load CW for auido and video */
	NEXUS_Security_GetDefaultControlWordSettings(&encrytedCW);
    encrytedCW.keyladderType = NEXUS_SecurityKeyladderType_e3Des;
    encrytedCW.keySize = sizeof(ucProcInForVidOddKey4);
    encrytedCW.keyEntryType = NEXUS_SecurityKeyType_eOdd;

	 /* ++++++++ */
	 encrytedCW.custSubMode		= NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4;
	 encrytedCW.virtualKeyLadderID = vklId;
	 encrytedCW.keyMode			= NEXUS_SecurityKeyMode_eRegular;
	 /* ++++++++ */

     /*encrytedCW.pKeyData = ucProcInForVidOddKey4;*/
 	BKNI_Memcpy(encrytedCW.keyData, ucProcInForVidOddKey4, encrytedCW.keySize );
    encrytedCW.operation = NEXUS_SecurityOperation_eDecrypt;
    encrytedCW.bRouteKey = true;
	encrytedCW.keyDestIVType = NEXUS_SecurityKeyIVType_eNoIV;
	encrytedCW.keyGenCmdID	 = NEXUS_SecurityKeyGenCmdID_eKeyGen;
	encrytedCW.bSwapAESKey	 = false;

	if (NEXUS_Security_GenerateControlWord(videoKeyHandle, &encrytedCW))
	{
		printf("\nLoading session key failed for video ODD key\n");
		return 1;
	}
    encrytedCW.keyEntryType = NEXUS_SecurityKeyType_eEven;
    /*encrytedCW.pKeyData = ucProcInForVidEvenKey4;*/
 	BKNI_Memcpy(encrytedCW.keyData, ucProcInForVidEvenKey4, encrytedCW.keySize);
	if (NEXUS_Security_GenerateControlWord(videoKeyHandle, &encrytedCW))
	{
		printf("\nLoading session key failed for video EVEN key\n");
		return 1;
	}

	if (NEXUS_Security_GenerateSessionKey(audioKeyHandle, &encryptedSessionkey) !=0)
	{
		printf("\nLoading session key failed for audio\n");
		return 1;
	}

    encrytedCW.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    /*encrytedCW.pKeyData = ucProcInForAudOddKey4;*/
 	BKNI_Memcpy(encrytedCW.keyData, ucProcInForAudOddKey4, encrytedCW.keySize);
	if (NEXUS_Security_GenerateControlWord(audioKeyHandle, &encrytedCW))
	{
		printf("\nLoading session key failed for audio ODD key\n");
		return 1;
	}
    encrytedCW.keyEntryType = NEXUS_SecurityKeyType_eEven;
    /*encrytedCW.pKeyData = ucProcInForAudEvenKey4;*/
 	BKNI_Memcpy(encrytedCW.keyData, ucProcInForAudEvenKey4, encrytedCW.keySize);
	if (NEXUS_Security_GenerateControlWord(audioKeyHandle, &encrytedCW))
	{
		printf("\nLoading session key failed for audio EVEN key\n");
		return 1;
	}


    /* Add video PID channel to keyslot */
    NEXUS_KeySlot_AddPidChannel ( videoKeyHandle, videoProgram.pidChannel);

    /* Add video PID channel to keyslot */
    NEXUS_KeySlot_AddPidChannel ( audioKeyHandle, audioProgram.pidChannel);

	printf ("\nSecurity Config OK\n");


    /* Bring up video display and outputs */
    NEXUS_Display_GetDefaultSettings(&displaySettings);

    display = NEXUS_Display_Open(0, &displaySettings);
    window = NEXUS_VideoWindow_Open(display, 0);


    #if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
    #endif
    #if NEXUS_NUM_SCART_INPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ScartInput_GetVideoOutputConnector(platformConfig.inputs.scart[0]));
    #if NEXUS_NUM_SCART_INPUTS>1
    NEXUS_Display_AddOutput(display, NEXUS_ScartInput_GetVideoOutputConnector(platformConfig.inputs.scart[1]));
    #endif
    #endif
    #if NEXUS_NUM_COMPOSITE_OUTPUTS
    if ( platformConfig.outputs.composite[0] ) {
        NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
    }
    #endif

	/* Add HDMI display */
	NEXUS_Display_GetDefaultSettings(&displaySettings);
	displaySettings.format = NEXUS_VideoFormat_e1080i;
	displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eHdmiDvo;
	NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));

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

#if 1
    /* Print status while decoding */
    for (;;) {
        NEXUS_VideoDecoderStatus status;
        NEXUS_AudioDecoderStatus audioStatus;
        uint32_t stc;

        NEXUS_VideoDecoder_GetStatus(vdecode, &status);
        NEXUS_StcChannel_GetStc(videoProgram.stcChannel, &stc);
        printf("decode %.4dx%.4d, pts %#x, stc %#x (diff %d) fifo=%d%%\n",
            status.source.width, status.source.height, status.pts, stc, status.pts - stc, status.fifoSize?(status.fifoDepth*100)/status.fifoSize:0);
        NEXUS_AudioDecoder_GetStatus(pcmDecoder, &audioStatus);
        printf("audio0            pts %#x, stc %#x (diff %d) fifo=%d%%\n",
            audioStatus.pts, stc, audioStatus.pts - stc, audioStatus.fifoSize?(audioStatus.fifoDepth*100)/audioStatus.fifoSize:0);
        NEXUS_AudioDecoder_GetStatus(compressedDecoder, &audioStatus);
        if ( audioStatus.started )
        {
            printf("audio1            pts %#x, stc %#x (diff %d) fifo=%d%%\n",
                audioStatus.pts, stc, audioStatus.pts - stc, audioStatus.fifoSize?(audioStatus.fifoDepth*100)/audioStatus.fifoSize:0);
        }
        BKNI_Sleep(1000);
    }
#else
    printf("Press ENTER to stop decode\n");
    getchar();

    /* example shutdown */
	NEXUS_Security_RemovePidChannelFromKeySlot(videoKeyHandle, videoPID);
	NEXUS_Security_RemovePidChannelFromKeySlot(audioKeyHandle, audioPID);
	NEXUS_Security_FreeKeySlot(videoKeyHandle);
	NEXUS_Security_FreeKeySlot(audioKeyHandle);

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

#else /* NEXUS_HAS_SECURITY && ... */
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform!\n");
    return -1;
}
#endif
