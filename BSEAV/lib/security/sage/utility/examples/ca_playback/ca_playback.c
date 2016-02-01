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
 *****************************************************************************/
/* Nexus example app: playback and decode CA protected stream. */

#ifndef NEXUS_HAS_SAGE
#error please recompile Nexus and the application with SAGE_SUPPORT=y
#endif

#ifndef NEXUS_HAS_PLAYBACK
#error please recompile Nexus and the application with PLAYBACK support
#endif

#ifndef NEXUS_HAS_SECURITY
#error please recompile Nexus and the application with SECURITY support
#endif

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
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_playback.h"
#include "nexus_file.h"
#include "nexus_security.h"

#include <ctype.h>

#include <stdio.h>
#include <assert.h>
#include "bstd.h"
#include "bkni.h"

#include "bsagelib_types.h"
#include "utility_ids.h"
#include "key_loader_tl.h"
#include "sage_srai.h"

/* the following define the input file and its characteristics -- these will vary by input file */
#define FILE_NAME "spiderman_aes.ts"
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3
#define VIDEO_PID 0x11
#define AUDIO_PID 0x14

static KeyLoaderTlSettings keyLoaderModuleSettings;
static BERR_Code Utility_LoadKey(KeyLoaderTl_Handle hKeyLoader);

/* CA keyladder information */
uint8_t ca_procInForKey3[] = {0xca, 0x1b, 0x36, 0x2c, 0x43, 0x75, 0xf8, 0x82, 0x7a, 0x03, 0xf5, 0xd8, 0x1d, 0x2f, 0xe9, 0xa4};
uint8_t ca_procInForKey4[] = {0xf5, 0x31, 0x33, 0x94, 0xd0, 0x9a, 0x3f, 0x3e, 0x18, 0x2a, 0x0b, 0x1f, 0x0e, 0x93, 0xc0, 0x3c};
uint8_t ca_procInForKey5[] = {0x6c, 0x8e, 0xfa, 0xa4, 0xe0, 0x7b, 0x89, 0x51, 0x2a, 0x3c, 0x10, 0xf6, 0x50, 0x93, 0xaf, 0x4f};

#define VID_INDEX   0
#define AUD_INDEX   1

NEXUS_KeySlotHandle gHandle[2] = {NULL, NULL};

int main(void)
{
    int status = 0;

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
    NEXUS_AudioDecoderOpenSettings audioDecoderOpenSettings;
    NEXUS_VideoDecoderOpenSettings videoDecoderOpenSettings;
    KeyLoaderTl_Handle hKeyLoader = NULL;

#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_Error rc;
#endif
    const char *fname = FILE_NAME;

    unsigned int videoPID, audioPID;
    int action;

    NEXUS_PidChannelStatus pidStatus;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /*-----------------------------------------------------------------------------
     *              Initialize SAGE platform and module
     *-----------------------------------------------------------------------------*/
    KeyLoaderTl_GetDefaultSettings(&keyLoaderModuleSettings);
    if (KeyLoaderTl_Init(&hKeyLoader, &keyLoaderModuleSettings) != BERR_SUCCESS) {
        printf("Error initializing Key Loader module\n");
        status = -1;
        goto handle_error;
    }

    playpump = NEXUS_Playpump_Open(0, NULL);
    assert(playpump);
    playback = NEXUS_Playback_Create();
    assert(playback);

    file = NEXUS_FilePlay_OpenPosix(fname, NULL);
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", fname);
        status = -1;
        goto handle_error;
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

    NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioDecoderOpenSettings);
    audioDecoderOpenSettings.cdbHeap = platformConfig.heap[NEXUS_VIDEO_SECURE_HEAP];
    audioDecoder = NEXUS_AudioDecoder_Open(0, &audioDecoderOpenSettings);
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
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
    NEXUS_VideoDecoder_GetDefaultOpenSettings(&videoDecoderOpenSettings);
    videoDecoderOpenSettings.cdbHeap = platformConfig.heap[NEXUS_VIDEO_SECURE_HEAP];
    videoDecoder = NEXUS_VideoDecoder_Open(0, &videoDecoderOpenSettings);
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

    /*-----------------------------------------------------------------------------
     *              set keys
     *-----------------------------------------------------------------------------*/
    if(Utility_LoadKey(hKeyLoader) != BERR_SUCCESS){
        printf("Error loading key Loader\n");
        status = -1;
        goto handle_error;
    }

    /* Add video PID channel to keyslot */
    NEXUS_PidChannel_GetStatus (videoProgram.pidChannel, &pidStatus);
    videoPID = pidStatus.pidChannelIndex;
    if (NEXUS_SUCCESS != NEXUS_KeySlot_AddPidChannel(gHandle[VID_INDEX], videoProgram.pidChannel) )
    {
        printf("\nConfig PID channel failed \n");
        status = -1;
        goto handle_error;
    }

    /* Add video PID channel to keyslot */
    NEXUS_PidChannel_GetStatus (audioProgram.pidChannel, &pidStatus);
    audioPID = pidStatus.pidChannelIndex;
    if(NEXUS_SUCCESS != NEXUS_KeySlot_AddPidChannel(gHandle[AUD_INDEX], audioProgram.pidChannel) )
    {
        printf("\nConfig PID channel failed \n");
        status = -1;
        goto handle_error;
    }

    printf ("\nSecurity Config OK\n");

    /* Start decoders */
    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
    NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);

    /* Start playback */
    NEXUS_Playback_Start(playback, file, NULL);

    /* Playback state machine is driven from inside Nexus. */
    printf("Press S to show the decoder status, Q to quit\n");
    do
    {
        action = toupper(getchar());

        if('S' == action)
        {
            NEXUS_VideoDecoderStatus status;
            NEXUS_AudioDecoderStatus audioStatus;

            NEXUS_VideoDecoder_GetStatus(videoDecoder, &status);
            printf("Main - VIDEO - numDecoded = '%u'   numDecodeErrors = '%u'   ptsErrorCount = '%u'\n", status.numDecoded, status.numDecodeErrors, status.ptsErrorCount);

            NEXUS_AudioDecoder_GetStatus(audioDecoder, &audioStatus);
            printf("Main - AUDIO - framesDecoded = '%u'   frameErrors = '%u'   dummyFrames = '%u'\n", audioStatus.framesDecoded, audioStatus.frameErrors, audioStatus.dummyFrames);

        }
    } while('Q' != action);


handle_error:

    /*-----------------------------------------------------------------------------
     *              close crypto
     *-----------------------------------------------------------------------------*/

    if(gHandle[VID_INDEX] != NULL)
    {
        NEXUS_KeySlot_RemovePidChannel(gHandle[VID_INDEX], videoProgram.pidChannel);
        KeyLoader_FreeKeySlot(hKeyLoader, gHandle[VID_INDEX]);
        gHandle[VID_INDEX] = NULL;
    }
    if(gHandle[AUD_INDEX] != NULL)
    {
        NEXUS_KeySlot_RemovePidChannel(gHandle[AUD_INDEX], audioProgram.pidChannel);
        KeyLoader_FreeKeySlot(hKeyLoader, gHandle[AUD_INDEX]);
        gHandle[AUD_INDEX] = NULL;
    }

    if(status != 0)
    {
        BKNI_Sleep(500);
    }

    /* Bring down system */
    NEXUS_VideoDecoder_Stop(videoDecoder);
    NEXUS_AudioDecoder_Stop(audioDecoder);
    NEXUS_Playback_Stop(playback);
    NEXUS_FilePlay_Close(file);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);
    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(videoDecoder));
    NEXUS_VideoDecoder_Close(videoDecoder);
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]));
#endif
#if NEXUS_NUM_SPDIF_INPUTS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]));
#endif

    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    NEXUS_AudioDecoder_Close(audioDecoder);
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    NEXUS_StcChannel_Close(stcChannel);

    KeyLoaderTl_Uninit(hKeyLoader);

    NEXUS_Platform_Uninit();

    return status;
}

static BERR_Code Utility_LoadKey(KeyLoaderTl_Handle hKeyLoader)
{
	BERR_Code rc = BERR_SUCCESS;
    KeyLoader_KeySlotConfigSettings keyslotConfigSettings;
    KeyLoader_WrappedKeySettings wrappedKeySettings;

    printf("%s - Entered function\n", __FUNCTION__);
    KeyLoader_GetDefaultConfigKeySlotSettings(&keyslotConfigSettings);

    keyslotConfigSettings.engine          = BSAGElib_Crypto_Engine_eCa;
    keyslotConfigSettings.algorithm       = BSAGElib_Crypto_Algorithm_eAes;
    keyslotConfigSettings.algorithmVar    = BSAGElib_Crypto_AlgorithmVariant_eCbc;
    keyslotConfigSettings.terminationMode = BSAGElib_Crypto_TerminationMode_eBlock;
    keyslotConfigSettings.solitarySelect  = BSAGElib_Crypto_SolitaryMode_eClear;
    keyslotConfigSettings.operation       = BSAGElib_Crypto_Operation_eDecrypt;
    keyslotConfigSettings.profileIndex    = 0;
    keyslotConfigSettings.custSubMode     = BSAGElib_Crypto_CustomerSubMode_eGeneric_CA_128_5;
    keyslotConfigSettings.keyType         = BSAGElib_Crypto_KeyType_eOddAndEven;

    rc = KeyLoader_AllocAndConfigKeySlot(hKeyLoader, &gHandle[VID_INDEX], &keyslotConfigSettings);
    if (rc != BERR_SUCCESS)
    {
        printf("%s - KeyLoader_AllocAndConfigKeySlot FAILED (ODD and Even)\n", __FUNCTION__);
        goto handle_error;
    }

    rc = KeyLoader_AllocAndConfigKeySlot(hKeyLoader, &gHandle[AUD_INDEX], &keyslotConfigSettings);
    if (rc != BERR_SUCCESS)
    {
        printf("%s - KeyLoader_AllocAndConfigKeySlot FAILED (ODD and Even)\n", __FUNCTION__);
        goto handle_error;
    }

    KeyLoader_GetDefaultWrappedKeySettings(&wrappedKeySettings);

    wrappedKeySettings.keyladderAlg = BSAGElib_Crypto_Algorithm_eAes;
    wrappedKeySettings.keyladderDepth = BSAGElib_Crypto_KeyLadderLevel_eKey5;
    wrappedKeySettings.keyType = BSAGElib_Crypto_KeyType_eEven;
    BKNI_Memcpy(wrappedKeySettings.procInForKey3, ca_procInForKey3, 16);
    BKNI_Memcpy(wrappedKeySettings.procInForKey4, ca_procInForKey4, 16);
    BKNI_Memcpy(wrappedKeySettings.procInForKey5, ca_procInForKey5, 16);
    wrappedKeySettings.keyLength = 16;
    wrappedKeySettings.ivLength = 16;

    rc = KeyLoader_LoadWrappedKey(hKeyLoader, gHandle[VID_INDEX], &wrappedKeySettings);
    if (rc != BERR_SUCCESS)
    {
        printf("%s - KeyLoader_LoadWrappedKey FAILED for CA EVEN (video)\n", __FUNCTION__);
        goto handle_error;
    }

    wrappedKeySettings.keyType = BSAGElib_Crypto_KeyType_eOdd;
    rc = KeyLoader_LoadWrappedKey(hKeyLoader, gHandle[VID_INDEX], &wrappedKeySettings);
    if (rc != BERR_SUCCESS)
    {
        printf("%s - KeyLoader_LoadWrappedKey FAILED for CA ODD (video)\n", __FUNCTION__);
        goto handle_error;
    }

    wrappedKeySettings.keyType = BSAGElib_Crypto_KeyType_eEven;
    rc = KeyLoader_LoadWrappedKey(hKeyLoader, gHandle[AUD_INDEX], &wrappedKeySettings);
    if (rc != BERR_SUCCESS)
    {
        printf("%s - KeyLoader_LoadWrappedKey FAILED for CA Even (audio)\n", __FUNCTION__);
        goto handle_error;
    }

    wrappedKeySettings.keyType = BSAGElib_Crypto_KeyType_eOdd;
    rc = KeyLoader_LoadWrappedKey(hKeyLoader, gHandle[AUD_INDEX], &wrappedKeySettings);
    if (rc != BERR_SUCCESS)
    {
        printf("%s - KeyLoader_LoadWrappedKey FAILED for CA ODD (audio)\n", __FUNCTION__);
        goto handle_error;
    }
handle_error:

    printf("%s - Exiting function\n", __FUNCTION__);
    return rc;
}
