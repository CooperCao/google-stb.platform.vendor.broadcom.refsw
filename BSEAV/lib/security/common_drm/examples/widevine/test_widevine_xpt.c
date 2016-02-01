/******************************************************************************
 *    (c)2012 Broadcom Corporation
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *****************************************************************************/
/* Nexus example app: playback and decode CA protected stream. */

#include "nexus_platform.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_input.h"
#include "nexus_audio_output.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#include "nexus_hdmi_output.h"
#include "nexus_playback.h"
#include "nexus_file.h"
#include "nexus_security.h"
#include <string.h> /* for memcpy */

#include "drm_widevine.h"

#include <stdio.h>
#include <assert.h>
#include "bstd.h"
#include "bkni.h"

/* the following define the input file and its characteristics -- these will vary by input file */
/*#define TEST_UNENCRYPTED*/
/*#define TEST_SPIDERMAN*/
#define TEST_COMMON_DRM

#ifdef TEST_UNENCRYPTED
#define FILE_NAME "cnnticker.mpg"
#define VIDEO_PID 0x21
#define AUDIO_PID 0x22
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#else

#ifdef TEST_SPIDERMAN
#define FILE_NAME "spiderman_aes.ts"
#define VIDEO_PID 0x11
#define AUDIO_PID 0x14
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#else
#define FILE_NAME "outfile.mpg"
#define VIDEO_PID 0x120
#define AUDIO_PID 0x131
#define VIDEO_CODEC NEXUS_VideoCodec_eH264
#endif /* TEST_SPIDERMAN */
#endif

#define TEST_PLAT 
/* or don't, i.e. TEST_CARD - remove the prev #define */

#ifdef TEST_PLAT
#define PLAT_DRM_BIN_FILE_NAME "WIDEVINE_SETTOP_KEYBOX_00000000.bin"
#endif

#define TRANSPORT_TYPE NEXUS_TransportType_eTs

#define AUDIO_CODEC NEXUS_AudioCodec_eMpeg


int main(int argc, char* argv[])
{
	DRM_Widevine_Init_t inputWvInitStruct;
	DrmRC drm_rc = Drm_Success;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_Error rc;
#endif
    const char *fname = FILE_NAME;
/*#ifdef TEST_SPIDERMAN*/
    unsigned char ControlWord[16] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
            						0x12, 0x34, 0xfe, 0xed, 0xba, 0xbe, 0xbe, 0xef  };

/*#else*/

    uint8_t ecm_odd[32] = {0x18, 0x99, 0x35, 0x79, 0x07, 0x83, 0x40, 0x42,
    						0x2f, 0x6e, 0xe3, 0x9e, 0xf8, 0x3b, 0x88, 0xca,
							0x28, 0x20, 0x31, 0x26, 0x03, 0x73, 0x49, 0x2a,
							0xe5, 0x0c, 0x08, 0xbe, 0xa8, 0xf8, 0x5f, 0xe9 };

    uint8_t id[16] = {0x00};

    uint8_t common_key[16] = {0x31, 0x9f, 0xf5, 0xa5, 0xb2, 0x74, 0x6d, 0x9b, 0x74, 0xa6, 0x34, 0x3c, 0xaa, 0xf0, 0x1e, 0xf1};

/*#endif*/
    unsigned int videoPID = VIDEO_PID;
    unsigned int audioPID = AUDIO_PID;
    NEXUS_PidChannelStatus pidStatus;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    playpump = NEXUS_Playpump_Open(0, NULL);
    assert(playpump);
    playback = NEXUS_Playback_Create();
    assert(playback);

    file = NEXUS_FilePlay_OpenPosix(fname, NULL);
    printf("\n\t File name is %s\n\n", fname);
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", fname);
        return -1;
    }

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    /* set a stream format, it could be any audio video transport type or file format, i.e NEXUS_TransportType_eMp4, NEXUS_TransportType_eAvi ... */
    playbackSettings.playpumpSettings.transportType = TRANSPORT_TYPE;
    playbackSettings.stcChannel = stcChannel;
    NEXUS_Playback_SetSettings(playback, &playbackSettings);

    /* Bring up audio decoders and outputs */
    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    NEXUS_AudioOutput_AddInput(
        NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#if NEXUS_NUM_HDMI_OUTPUTS
    printf("\tMAIN - NEXUS_AudioOutput_AddInput(hdmi)\n");
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif

    /* Bring up video display and outputs */
    display = NEXUS_Display_Open(0, NULL);
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
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    /* Open the audio and video pid channels */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = VIDEO_CODEC; /* must be told codec for correct handling */
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
    videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, VIDEO_PID, &playbackPidSettings);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.primary = audioDecoder;
    audioPidChannel = NEXUS_Playback_OpenPidChannel(playback, AUDIO_PID, &playbackPidSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up
    the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = AUDIO_CODEC;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;

    /* Start decoders */
    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
    NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);

    /***************************************************************************************
        Config CA descrambler 
    ***************************************************************************************/
#ifndef TEST_UNENCRYPTED

#ifdef TEST_COMMON_DRM
    inputWvInitStruct.key_file = 
#ifdef TEST_PLAT
                                 PLAT_DRM_BIN_FILE_NAME;
#else
                                 NULL;
#endif 

    printf("\n\tMAIN - executable use case 1:   ./nexus test_widevine_xpt <mpeg file>\n");
    printf("\tMAIN - executable use case 2:   ./nexus test_widevine_xpt <DRM bin file> <mpeg file>\n");
    printf("\t MAIN - inputWvInitStruct.key_file: %s\n", inputWvInitStruct.key_file);

    if(argc > 2){
	    inputWvInitStruct.key_file = argv[1];
        printf("\n\tMAIN - set key file path to '%s'\n", argv[1]);
    }

    inputWvInitStruct.mode = DRM_WidevineDecryptMode_eTs_Iptv;
    DRM_Widevine_Initialize(inputWvInitStruct);

    drm_rc = DRM_Widevine_LoadCwFixed(common_key, ecm_odd, id, DrmSecurityKeyType_eEven);
    if(drm_rc != Drm_Success)
	{
        printf("\tMAIN - DRM_Widevine_LoadCw (even) failed, rc %d\n", drm_rc);
        goto ErrorExit;
    }
	else
	{
		printf("\n\tMAIN - CW ID: 0x%02x 0x%02x 0x%02x 0x%02x\n\n", id[0], id[1], id[2], id[3]);
	}

    drm_rc = DRM_Widevine_LoadCwFixed(common_key, ecm_odd, id, DrmSecurityKeyType_eOdd);
    if(drm_rc != Drm_Success)
	{
        printf("\tMAIN - DRM_Widevine_LoadCw (odd) failed, rc %d\n", drm_rc);
        goto ErrorExit;
    }
	else
	{
		printf("\n\tMAIN - CW ID: 0x%02x 0x%02x 0x%02x 0x%02x\n\n", id[0], id[1], id[2], id[3]);
	}

    NEXUS_PidChannel_GetStatus (videoProgram.pidChannel, &pidStatus);
     videoPID = pidStatus.pidChannelIndex;
     NEXUS_PidChannel_GetStatus (audioProgram.pidChannel, &pidStatus);
     audioPID = pidStatus.pidChannelIndex;
     DRM_Widevine_AttachPidChannel(audioPID);
     DRM_Widevine_AttachPidChannel(videoPID);

#else
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
    AlgConfig.dest = NEXUS_SecurityAlgorithmConfigDestination_eCa;
    AlgConfig.algorithm = NEXUS_SecurityAlgorithm_eAes;
    AlgConfig.algorithmVar = NEXUS_SecurityAlgorithmVariant_eCbc;
#ifdef TEST_SPIDERMAN
    AlgConfig.terminationMode = NEXUS_SecurityTerminationMode_eBlock;
#else
    AlgConfig.terminationMode = NEXUS_SecurityTerminationMode_eCtsCpcm;
#endif /* TEST_SPIDERMAN*/
    AlgConfig.key2Select		    = NEXUS_SecurityKey2Select_eReserved1;
    AlgConfig.caVendorID          = 0x1234;/* Verify vendor ID with security team */
    AlgConfig.askmModuleID        = NEXUS_SecurityAskmModuleID_eModuleID_4;

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

    NEXUS_Security_GetDefaultClearKey(&key);

    /* Load Video Odd/Even keys */
    key.keySize = sizeof(ControlWord);
    key.keyEntryType = NEXUS_SecurityKeyType_eEven;
    key.keyIVType	 = NEXUS_SecurityKeyIVType_eNoIV;
    memcpy(key.keyData, ControlWord,sizeof(ControlWord));
    if ( NEXUS_Security_LoadClearKey (videoKeyHandle, &key) != 0 )
    {
        printf("\nLoad video EVEN key failed \n");
        return 1;
    }
    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    key.keyIVType	 = NEXUS_SecurityKeyIVType_eNoIV;
    memcpy(key.keyData, ControlWord,sizeof(ControlWord));
    if ( NEXUS_Security_LoadClearKey (videoKeyHandle, &key) != 0 )
    {
        printf("\nLoad video ODD key failed \n");
        return 1;
    }

    /* Load audio Odd/Even */
    key.keySize = sizeof(ControlWord);
    key.keyEntryType = NEXUS_SecurityKeyType_eEven;
    key.keyIVType	 = NEXUS_SecurityKeyIVType_eNoIV;
    memcpy(key.keyData, ControlWord,sizeof(ControlWord));
    if ( NEXUS_Security_LoadClearKey (audioKeyHandle, &key) != 0 )
    {
        printf("\nLoad audio EVEN key failed \n");
        return 1;
    }

    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    key.keyIVType	 = NEXUS_SecurityKeyIVType_eNoIV;
    memcpy(key.keyData, ControlWord,sizeof(ControlWord));
    if ( NEXUS_Security_LoadClearKey (audioKeyHandle, &key) != 0 )
    {
        printf("\nLoad audio ODD key failed \n");
        return 1;
    }

    /* IV for audio/video */
    key.keyIVType	 = NEXUS_SecurityKeyIVType_eIV;
    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    memcpy(key.keyData, iv0,sizeof(iv0));
    if ( NEXUS_Security_LoadClearKey (audioKeyHandle, &key) != 0 )
    {
        printf("\nLoad audio audio IV failed \n");
        return 1;
    }

    key.keyIVType	 = NEXUS_SecurityKeyIVType_eIV;
    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    memcpy(key.keyData, iv0, sizeof(iv0));
    if ( NEXUS_Security_LoadClearKey (videoKeyHandle, &key) != 0 )
    {
        printf("\nLoad audio Video IV failed \n");
        return 1;
    }

    key.keyIVType	 = NEXUS_SecurityKeyIVType_eIV;
    key.keyEntryType = NEXUS_SecurityKeyType_eEven;
    memcpy(key.keyData, iv0,sizeof(iv0));
    if ( NEXUS_Security_LoadClearKey (audioKeyHandle, &key) != 0 )
    {
        printf("\nLoad audio audio IV failed \n");
        return 1;
    }

    key.keyIVType	 = NEXUS_SecurityKeyIVType_eIV;
    key.keyEntryType = NEXUS_SecurityKeyType_eEven;
    memcpy(key.keyData, iv0, sizeof(iv0));
    if ( NEXUS_Security_LoadClearKey (videoKeyHandle, &key) != 0 )
    {
        printf("\nLoad audio Video IV failed \n");
        return 1;
    }

    /* Add video PID channel to keyslot */
    NEXUS_PidChannel_GetStatus (videoProgram.pidChannel, &pidStatus);
    videoPID = pidStatus.pidChannelIndex;
    if ( NEXUS_Security_AddPidChannelToKeySlot(videoKeyHandle, videoPID)!= 0 )
    {
        printf("\nConfigPIDPointerTable failed \n");
        return 1;
    }

    /* Add video PID channel to keyslot */
    NEXUS_PidChannel_GetStatus (audioProgram.pidChannel, &pidStatus);
    audioPID = pidStatus.pidChannelIndex;
    NEXUS_Security_AddPidChannelToKeySlot(audioKeyHandle, audioPID); 
#endif

    printf ("\nSecurity Config OK\n");
#endif /* TEST_UNENCRYPTED */

    /* Start playback */
    NEXUS_Playback_Start(playback, file, NULL);

#if 1
    /* Print status while decoding */
    for (;;) {
        NEXUS_VideoDecoderStatus status;
        NEXUS_AudioDecoderStatus audioStatus;
        uint32_t stc;

        NEXUS_VideoDecoder_GetStatus(videoDecoder, &status);
        NEXUS_StcChannel_GetStc(videoProgram.stcChannel, &stc);
        printf("decode %.4dx%.4d, pts %#x, stc %#x (diff %d) fifo=%d%%\n",
            status.source.width, status.source.height, status.pts, stc, status.pts - stc, status.fifoSize?(status.fifoDepth*100)/status.fifoSize:0);
        NEXUS_AudioDecoder_GetStatus(audioDecoder, &audioStatus);
        printf("audio0            pts %#x, stc %#x (diff %d) fifo=%d%%\n",
            audioStatus.pts, stc, audioStatus.pts - stc, audioStatus.fifoSize?(audioStatus.fifoDepth*100)/audioStatus.fifoSize:0);
        BKNI_Sleep(1000);
    }
#endif

    /* Playback state machine is driven from inside Nexus. */
    printf("Press ENTER to quit\n");
    getchar();

ErrorExit:

	DRM_Widevine_Finalize();

	/*if testing CARD mode */
	/*DRM_Module_UnInit();*/

    /* Bring down system */
    NEXUS_VideoDecoder_Stop(videoDecoder);
    NEXUS_AudioDecoder_Stop(audioDecoder);
    NEXUS_Playback_Stop(playback);
    NEXUS_FilePlay_Close(file);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);
    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(videoDecoder));
    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]));
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]));
    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    NEXUS_AudioDecoder_Close(audioDecoder);
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    NEXUS_StcChannel_Close(stcChannel);
    NEXUS_Platform_Uninit();

    return 0;
}

