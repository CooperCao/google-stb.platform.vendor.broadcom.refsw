/******************************************************************************
 *    (c)2007-2015 Broadcom Corporation
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
 *****************************************************************************/
/* Nexus example app: single live a/v decode from an input band */
#if NEXUS_HAS_SECURITY  && defined(HSM_TEST_TO_BE_INTEGRATED)

#include "nexus_platform.h"
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
/* #include "nexus_keyladder.h"*/

int Prompt(void);
void Descrmbler_Multi2Ecb_SwKey(void);
void Descrmbler_Multi2Cbc_SwKey(void);

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

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    platformSettings.securitySettings.numMulti2KeySlots = 8;
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
    videoPidChannel = NEXUS_PidChannel_Open(parserBand, 0x11, NULL);
    audioPidChannel = NEXUS_PidChannel_Open(parserBand, 0x14, NULL);

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

    /* Bring up display and outputs */
#if NEXUS_DTV_PLATFORM
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    display = NEXUS_Display_Open(0, &displaySettings);
    NEXUS_Display_AddOutput(display, NEXUS_PanelOutput_GetConnector(platformConfig.outputs.panel[0]));
    NEXUS_BoardCfg_ConfigurePanel(true, true, true);
    fprintf(stderr, "Panel output ready\n");
#else
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_eNtsc;
    display = NEXUS_Display_Open(0, &displaySettings);
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
    NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#if NEXUS_NUM_SVIDEO_OUTPUTS    
    if (platformConfig.outputs.svideo[0]) {
        NEXUS_Display_AddOutput(display, NEXUS_SvideoOutput_GetConnector(platformConfig.outputs.svideo[0]));
    }
#endif    
#endif
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

{
	unsigned int command_id;
	
	while(1) {
		BKNI_Printf("\nSelect Descrambler Test:\n");
		BKNI_Printf("1) Multi2-ECB SW Key( play spiderman_multi2_ecb_ct_round32.ts)\n");
		BKNI_Printf("2) Multi2-CBC SW Key( play spiderman_cbc_rb.ts )\n");
		BKNI_Printf("0) Exit\n");

		command_id = Prompt();

		switch (command_id) {
			case 0:  /* Exit */
				return 0;				

			case 1:
				Descrmbler_Multi2Ecb_SwKey();
				break;	
				
			case 2:
				Descrmbler_Multi2Cbc_SwKey();
				break;		

			default:
				BKNI_Printf("\nInvalid Choice!, command_id = %d\n\n", command_id);
				break;
		}
	}
}	

    printf("Press ENTER to stop decode\n");
    getchar();

    /* example shutdown */
    NEXUS_AudioDecoder_Stop(pcmDecoder);
    if ( audioProgram.codec == NEXUS_AudioCodec_eAc3 )
    {
        NEXUS_AudioDecoder_Stop(compressedDecoder);
    }
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]));
#endif

#if NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]));
#endif
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
#endif

    return 0;
}


int Prompt()
{
   char choice[10];
   unsigned char input_char;
   unsigned int command_id;

   input_char = 0;
   
   BKNI_Printf("\nChoice: ");

   while (input_char == 0) {
      input_char = getchar(); 
	if (input_char == '\n')
		input_char = 0;
   }

   choice[0] = input_char;
   choice[1] = '\0';

   if ((choice[0] == 'y')  ||  (choice[0] == 'Y')  ||
       (choice[0] == 'n')  ||  (choice[0] == 'N'))
      return choice[0];

   sscanf(choice,"%x\n",&command_id);
   return command_id;
}

/*
file 	\\brcm-irv\dfs\projects\bseswss\STREAM\Multi2_stream\spiderman_multi2_ecb_ct_round32.ts
algorithm		Multi2_ECB
termination   	clear
PID			0x11
scrambling_ctrl	2				// output transport_scrambling_control bits
key1			00 00 00 01 00 02 00 03 00 04 00 05 00 06 00 07  // for Multi2, MSBs of system key
key2			00 08 00 09 00 0a 00 0b 00 0c 00 0d 00 0e 00 0f  // for Multi2, LSBs of system key
key3			00 00 00 00 00 00 00 00 01 23 45 67 89 ab cd ef  // for Multi2, 64 LSBs are the data key
round_number	see file name				// used only in Multi2 modes

*** Note that current H/W does not support 128 rounds ***

For 32-round stream
For H/W register programming:
MULTI2 SYSTEM KEY 0 00 00 00 01
MULTI2 SYSTEM KEY 1 00 02 00 03
MULTI2 SYSTEM KEY 2 00 04 00 05
MULTI2 SYSTEM KEY 3 00 06 00 07
MULTI2 SYSTEM KEY 4 00 08 00 09
MULTI2 SYSTEM KEY 5 00 0a 00 0b
MULTI2 SYSTEM KEY 6 00 0c 00 0d
MULTI2 SYSTEM KEY 7 00 0e 00 0f
MULTI2_RC (Multi2 round count divided by 8) = 4 (default)
*/

void Descrmbler_Multi2Ecb_SwKey(void)
{
    NEXUS_KeySlotHandle videoKeyHandle = NULL;
    
    NEXUS_SecurityAlgorithmSettings AlgConfig;
    NEXUS_SecurityMulti2Settings Multi2Config;
    NEXUS_SecurityClearKey key;
/*    NEXUS_PidChannelStatus pidStatus;
	NEXUS_KeySlotHandle audioKeyHandle = NULL;
*/
    NEXUS_SecurityKeySlotSettings keySlotSettings;

	unsigned char cwVideoOdd[8]      =  { 
						0x01 ,0x23 ,0x45 ,0x67 ,0x89 ,0xab ,0xcd ,0xef 
					};	

	unsigned char cwVideoEven[8]      = { 
						0x01 ,0x23 ,0x45 ,0x67 ,0x89 ,0xab ,0xcd ,0xef 
					};

	unsigned char multi2_sys_key[32] = {
		0x00 ,0x00 ,0x00 ,0x01 ,0x00 ,0x02 ,0x00 ,0x03 ,0x00 ,0x04 ,0x00 ,0x05 ,0x00 ,0x06 ,0x00 ,0x07,
		0x00 ,0x08 ,0x00 ,0x09 ,0x00 ,0x0a ,0x00 ,0x0b ,0x00 ,0x0c ,0x00 ,0x0d ,0x00 ,0x0e ,0x00 ,0x0f	
	};
	
	
	
    /***************************************************************************************
        Config CA descrambler (Multi2) 
    ***************************************************************************************/
printf("Allocate Key Slot \n");
	
    NEXUS_Security_GetDefaultKeySlotSettings(&keySlotSettings);
    keySlotSettings.keySlotEngine = NEXUS_SecurityEngine_eCa;
    /* Allocate AV keyslots */
    videoKeyHandle = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
    if ( !videoKeyHandle)
    {
        printf("\nAllocate CA video keyslot failed \n");
        return ;
    }

printf("Link Video Pid Channel with key slot \n"); 
    /* Add video PID channel to keyslot  : Now Assume Video Pid channel Num = 0 */ 
/*
    NEXUS_PidChannel_GetStatus (videoProgram.pidChannel, &pidStatus);
   
    if ( NEXUS_Security_AddPidChannelToKeySlot(videoKeyHandle, pidStatus.pidChannelIndex)!= 0 )
*/    

    if ( NEXUS_Security_AddPidChannelToKeySlot(videoKeyHandle, 0 )!= 0 )
   {
        printf("\nConfigPIDPointerTable failed \n");
        return ;
    }


printf("Configure Algorithm for both Odd and Even Key Slot \n");

    /* Config AV algorithms */
    NEXUS_Security_GetDefaultAlgorithmSettings(&AlgConfig);
    AlgConfig.algorithm = NEXUS_SecurityAlgorithm_eMulti2;
    AlgConfig.algorithmVar = NEXUS_SecurityAlgorithmVariant_eEcb;
    AlgConfig.multi2KeySelect = 0;
    /* Default for captured streams is probably CbcResidual, so it is recorded here for future use */
    /* AlgConfig.terminationMode = NEXUS_SecurityTerminationMode_eCbcResidual; */
    /* Our test stream, however, uses the clear termination mode */
	
#if 1
    AlgConfig.terminationMode = NEXUS_SecurityTerminationMode_eClear; 
#else
    AlgConfig.terminationMode = NEXUS_SecurityTerminationMode_eCbcResidual ;
#endif

    if ( NEXUS_Security_ConfigAlgorithm (videoKeyHandle, &AlgConfig) != 0 )
    {
        printf("\nConfig video CA Algorithm Even failed \n");
        return ;
    }
	
printf("Configure Multi2 Setting \n");
    /* Multi2 step */
    NEXUS_Security_GetDefaultMulti2Settings(&Multi2Config);
    Multi2Config.multi2KeySelect = 0;
    Multi2Config.multi2Rounds = 4;
    memcpy(Multi2Config.multi2SysKey, multi2_sys_key, 32);
    if ( NEXUS_Security_ConfigMulti2(videoKeyHandle, &Multi2Config) != 0 )
    {
        printf("\nConfig video CA Multi2 parameters failed \n");
        return ;
    }

printf("Load Odd Key \n");

    /* Load AV keys */
	NEXUS_Security_GetDefaultClearKey(&key);
    key.keySize = 8; 
    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
	/*--------*/
	key.keyIVType	 = NEXUS_SecurityKeyIVType_eNoIV;
	/*--------*/
    memcpy(key.keyData,cwVideoOdd,sizeof(cwVideoOdd));
    if ( NEXUS_Security_LoadClearKey (videoKeyHandle, &key) != 0 )
    {
        printf("\nLoad video EVEN key failed \n");
        return ;
    }

printf("Load Even Key \n");

    key.keyEntryType = NEXUS_SecurityKeyType_eEven;
    memcpy(key.keyData,cwVideoEven,sizeof(cwVideoEven));
    if ( NEXUS_Security_LoadClearKey (videoKeyHandle, &key) != 0 )
    {
        printf("\nLoad video ODD key failed \n");
        return ;
    }

    printf ("\nSecurity Config Done\n");

}

/*
spiderman_cbc_rb.ts has the following parameters:

Multi-2 CBC encrypted with residual block termination (OFB)
PID			0x11
scrambling_ctrl	3
key1			36 31 04 66 4B 17 EA 5C 32 DF 9C F5 C4 C3 6C 1B  // for Multi2, MSBs of system key
key2			EC 99 39 21 68 9D 4B B7 B7 4E 40 84 0D 2E 7D 98  // for Multi2, LSBs of system key
key3			00 00 00 00 00 00 00 00 01 23 45 67 89 ab cd ef  // for Multi2, 64 LSBs are the data key
iv1			FE 27 19 99 19 69 09 11		// initial vector for CBC modes (except IPTV) or initial counter for AES_COUNTER_HW mode
iv2			27 19 99 19 69 09 11 FE		// initial vector for short solitary block
round_number	32 // note that this is the actual number of rounds.  register definition may be different
*/

void Descrmbler_Multi2Cbc_SwKey(void)
{
    NEXUS_KeySlotHandle videoKeyHandle = NULL;
    NEXUS_SecurityAlgorithmSettings AlgConfig;
    NEXUS_SecurityMulti2Settings Multi2Config;
    NEXUS_SecurityClearKey key;
/*    NEXUS_PidChannelStatus pidStatus;  
	NEXUS_KeySlotHandle audioKeyHandle = NULL;
*/
    NEXUS_SecurityKeySlotSettings keySlotSettings;

    unsigned char cwVideoOdd[]  = {0x01 ,0x23 ,0x45 ,0x67 ,0x89 ,0xab ,0xcd ,0xef};
    unsigned char cwVideoEven[] = {0x01 ,0x23 ,0x45 ,0x67 ,0x89 ,0xab ,0xcd ,0xef};

    uint8_t multi2_sys_key[] = {
		0x36 ,0x31 ,0x04 ,0x66 ,0x4B ,0x17 ,0xEA ,0x5C ,0x32 ,0xDF ,0x9C ,0xF5 ,0xC4 ,0xC3 ,0x6C ,0x1B,
		0xEC ,0x99 ,0x39 ,0x21 ,0x68 ,0x9D ,0x4B ,0xB7 ,0xB7 ,0x4E ,0x40 ,0x84 ,0x0D ,0x2E ,0x7D ,0x98
            };


    uint8_t iv[] = {
 		0x27 ,0x19 ,0x99 ,0x19 ,0x69 ,0x09 ,0x11 ,0xFE,
   		0xFE ,0x27 ,0x19 ,0x99 ,0x19 ,0x69 ,0x09 ,0x11,
            };
	
	
	
    /***************************************************************************************
        Config CA descrambler (Multi2) 
    ***************************************************************************************/
printf("Allocate Key Slot \n");
	
    NEXUS_Security_GetDefaultKeySlotSettings(&keySlotSettings);
    keySlotSettings.keySlotEngine = NEXUS_SecurityEngine_eCa;
    /* Allocate AV keyslots */
    videoKeyHandle = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
    if ( !videoKeyHandle)
    {
        printf("\nAllocate CA video keyslot failed \n");
        return ;
    }

printf("Link Video Pid Channel with key slot \n"); 
    /* Add video PID channel to keyslot  : Now Assume Video Pid channel Num = 0 */ 
/*
    NEXUS_PidChannel_GetStatus (videoProgram.pidChannel, &pidStatus);
   
    if ( NEXUS_Security_AddPidChannelToKeySlot(videoKeyHandle, pidStatus.pidChannelIndex)!= 0 )
*/    

    if ( NEXUS_Security_AddPidChannelToKeySlot(videoKeyHandle, 0 )!= 0 )
   {
        printf("\nConfigPIDPointerTable failed \n");
        return ;
    }


printf("Configure Algorithm for both Odd and Even Key Slot \n");

    /* Config AV algorithms */
    NEXUS_Security_GetDefaultAlgorithmSettings(&AlgConfig);
    AlgConfig.algorithm = NEXUS_SecurityAlgorithm_eMulti2;
    AlgConfig.algorithmVar = NEXUS_SecurityAlgorithmVariant_eCbc;
/*    AlgConfig.dvbScramLevel = NEXUS_SecurityDvbScrambleLevel_eTs;*/
    AlgConfig.multi2KeySelect = 0;
    /* Default for captured streams is probably CbcResidual, so it is recorded here for future use */
    /* AlgConfig.terminationMode = NEXUS_SecurityTerminationMode_eCbcResidual; */
    /* Our test stream, however, uses the clear termination mode */
	
#if 0
    AlgConfig.terminationMode = NEXUS_SecurityTerminationMode_eClear; 
#else
    AlgConfig.terminationMode = NEXUS_SecurityTerminationMode_eCbcResidual ;
#endif

    if ( NEXUS_Security_ConfigAlgorithm (videoKeyHandle, &AlgConfig) != 0 )
    {
        printf("\nConfig video CA Algorithm Even failed \n");
        return ;
    }
	
printf("Configure Multi2 Setting \n");
    /* Multi2 step */
    NEXUS_Security_GetDefaultMulti2Settings(&Multi2Config);
    Multi2Config.multi2KeySelect = 0;
    Multi2Config.multi2Rounds = 4;
    memcpy(Multi2Config.multi2SysKey, multi2_sys_key, 32);
    if ( NEXUS_Security_ConfigMulti2(videoKeyHandle, &Multi2Config) != 0 )
    {
        printf("\nConfig video CA Multi2 parameters failed \n");
        return ;
    }

printf("Load Odd Key \n");

    /* Load AV keys */
	NEXUS_Security_GetDefaultClearKey(&key);
    key.keySize = 8; 
    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
	/*--------*/
	key.keyIVType	 = NEXUS_SecurityKeyIVType_eNoIV;
	/*--------*/
    memcpy(key.keyData,cwVideoOdd,sizeof(cwVideoOdd));
    if ( NEXUS_Security_LoadClearKey (videoKeyHandle, &key) != 0 )
    {
        printf("\nLoad video EVEN key failed \n");
        return ;
    }

printf("Load Even Key \n");

    key.keyEntryType = NEXUS_SecurityKeyType_eEven;
    memcpy(key.keyData,cwVideoEven,sizeof(cwVideoEven));
    if ( NEXUS_Security_LoadClearKey (videoKeyHandle, &key) != 0 )
    {
        printf("\nLoad video ODD key failed \n");
        return ;
    }

printf("Load IV \n");
	/*--------*/
	key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
	key.keyIVType	 = NEXUS_SecurityKeyIVType_eIV;
	/*--------*/
    key.keySize = 16;
    BKNI_Memcpy(key.keyData,iv,16);
    if (NEXUS_Security_LoadClearKey(videoKeyHandle, &key) != 0) {
        printf("\nLoad video IV Odd failed \n");
        return ;
    }
	key.keyEntryType = NEXUS_SecurityKeyType_eEven;
	key.keyIVType	 = NEXUS_SecurityKeyIVType_eIV;

	memcpy(key.keyData,iv,sizeof(iv));
	if ( NEXUS_Security_LoadClearKey (videoKeyHandle, &key) != 0 )
	{
		printf("\nLoad video IV Even failed \n");
		return ;
	}


    printf ("\nSecurity Config Done\n");

}


#else /* NEXUS_HAS_SECURITY */
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform!\n");
    return -1;
}
#endif
