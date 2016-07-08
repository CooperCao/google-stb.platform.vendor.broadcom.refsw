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
/* Nexus example app: playback and decode the short stream recorded by m2m_clearkey_record. */

#if NEXUS_HAS_SECURITY  &&     (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 4) && NEXUS_HAS_VIDEO_DECODER

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
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#if NEXUS_HAS_PLAYBACK
#include "nexus_playback.h"
#include "nexus_file.h"
#endif

#include <stdio.h>
#include "bstd.h"
#include "bkni.h"

#include "nexus_dma.h"
#include "nexus_security.h"

/* This can be any stream file recorded stream from test app m2m_clearkey_record recording and encrypting. */
#define RECORDED_FILE "/mnt/nfs/video/recorded_stream.mpg"
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eMpeg

/* The video pid of clear program in stream RECORDED_FILE. */
#define VIDEO_PID 2316
#define AUDIO_PID 2317

static BERR_Code load_cpd_clear_keys ( NEXUS_PidChannelHandle videoPidChannel );

int main ( void )
{
#if NEXUS_HAS_PLAYBACK
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
#endif
    const char     *fname = RECORDED_FILE;
    NEXUS_Error     rc;

    NEXUS_Platform_GetDefaultSettings ( &platformSettings );
    platformSettings.openFrontend = false;
    rc = NEXUS_Platform_Init ( &platformSettings );
    if ( rc )
        return -1;
    NEXUS_Platform_GetConfiguration ( &platformConfig );

    playpump = NEXUS_Playpump_Open ( NEXUS_ANY_ID, NULL );
    BDBG_ASSERT ( playpump );
    playback = NEXUS_Playback_Create (  );
    BDBG_ASSERT ( playback );

    file = NEXUS_FilePlay_OpenPosix ( fname, NULL );
    if ( !file )
    {
        fprintf ( stderr, "can't open file:%s\n", fname );
        return -1;
    }

    NEXUS_StcChannel_GetDefaultSettings ( 0, &stcSettings );
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open ( 0, &stcSettings );

    NEXUS_Playback_GetSettings ( playback, &playbackSettings );
    playbackSettings.playpump = playpump;
    /* set a stream format, it could be any audio video transport type or file format, i.e NEXUS_TransportType_eMp4, NEXUS_TransportType_eAvi ... */
    playbackSettings.playpumpSettings.transportType = TRANSPORT_TYPE;
    playbackSettings.stcChannel = stcChannel;
    NEXUS_Playback_SetSettings ( playback, &playbackSettings );

    /* Bring up audio decoders and outputs */
    audioDecoder = NEXUS_AudioDecoder_Open ( 0, NULL );
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput ( NEXUS_AudioDac_GetConnector ( platformConfig.outputs.audioDacs[0] ),
                                 NEXUS_AudioDecoder_GetConnector ( audioDecoder,
                                                                   NEXUS_AudioDecoderConnectorType_eStereo ) );
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutput_AddInput ( NEXUS_SpdifOutput_GetConnector ( platformConfig.outputs.spdif[0] ),
                                 NEXUS_AudioDecoder_GetConnector ( audioDecoder,
                                                                   NEXUS_AudioDecoderConnectorType_eStereo ) );
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput ( NEXUS_HdmiOutput_GetAudioConnector ( platformConfig.outputs.hdmi[0] ),
                                 NEXUS_AudioDecoder_GetConnector ( audioDecoder,
                                                                   NEXUS_AudioDecoderConnectorType_eStereo ) );
#endif

    /* Bring up video display and outputs */
    display = NEXUS_Display_Open ( 0, NULL );
    window = NEXUS_VideoWindow_Open ( display, 0 );

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput ( display, NEXUS_ComponentOutput_GetConnector ( platformConfig.outputs.component[0] ) );
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput ( display, NEXUS_CompositeOutput_GetConnector ( platformConfig.outputs.composite[0] ) );
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput ( display, NEXUS_HdmiOutput_GetVideoConnector ( platformConfig.outputs.hdmi[0] ) );
    rc = NEXUS_HdmiOutput_GetStatus ( platformConfig.outputs.hdmi[0], &hdmiStatus );
    if ( !rc && hdmiStatus.connected )
    {
        /* If current display format is not supported by monitor, switch to monitor's preferred format.
         * If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings ( display, &displaySettings );
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] )
        {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings ( display, &displaySettings );
        }
    }
#endif

    /* bring up decoder and connect to display */
    videoDecoder = NEXUS_VideoDecoder_Open ( 0, NULL ); /* take default capabilities */
    NEXUS_VideoWindow_AddInput ( window, NEXUS_VideoDecoder_GetConnector ( videoDecoder ) );

    /* Open the audio and video pid channels */
    NEXUS_Playback_GetDefaultPidChannelSettings ( &playbackPidSettings );
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = VIDEO_CODEC;
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
    videoPidChannel = NEXUS_Playback_OpenPidChannel ( playback, VIDEO_PID, &playbackPidSettings );

    NEXUS_Playback_GetDefaultPidChannelSettings ( &playbackPidSettings );
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.primary = audioDecoder;
    audioPidChannel = NEXUS_Playback_OpenPidChannel ( playback, AUDIO_PID, &playbackPidSettings );

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings ( &videoProgram );
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
    NEXUS_AudioDecoder_GetDefaultStartSettings ( &audioProgram );
    audioProgram.codec = AUDIO_CODEC;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;

    /* Setup the security */
    load_cpd_clear_keys ( videoPidChannel );

    /* Start decoders */
    NEXUS_VideoDecoder_Start ( videoDecoder, &videoProgram );
    NEXUS_AudioDecoder_Start ( audioDecoder, &audioProgram );

    /* Start playback */
    NEXUS_Playback_Start ( playback, file, NULL );

    /* Playback state machine is driven from inside Nexus. */
    printf ( "Press ENTER to quit\n" );
    getchar (  );

    /* Bring down system */
    NEXUS_VideoDecoder_Stop ( videoDecoder );
    NEXUS_AudioDecoder_Stop ( audioDecoder );
    NEXUS_Playback_Stop ( playback );
    NEXUS_FilePlay_Close ( file );
    NEXUS_Playback_Destroy ( playback );
    NEXUS_Playpump_Close ( playpump );
    NEXUS_VideoDecoder_Close ( videoDecoder );
    NEXUS_AudioDecoder_Close ( audioDecoder );
    NEXUS_VideoWindow_Close ( window );
    NEXUS_Display_Close ( display );
    NEXUS_StcChannel_Close ( stcChannel );
    NEXUS_Platform_Uninit (  );

#else
    printf ( "This application is not supported on this platform!\n" );
#endif
    return 0;
}

static BERR_Code load_cpd_clear_keys ( NEXUS_PidChannelHandle videoPidChannel )
{
    NEXUS_KeySlotHandle keyHandle;
    NEXUS_SecurityAlgorithmSettings algConfig;
    NEXUS_SecurityClearKey key;
    NEXUS_SecurityKeySlotSettings keySettings;

    const uint8_t   keys[16] =
        { 0x6e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7, 0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73 };

    /* Allocate AV keyslots */
    NEXUS_Security_GetDefaultKeySlotSettings ( &keySettings );
    keySettings.keySlotEngine = NEXUS_SecurityEngine_eCaCp;
    keyHandle = NEXUS_Security_AllocateKeySlot ( &keySettings );
    if ( !keyHandle )
    {
        printf ( "\nAllocate decrypt keyslot failed \n" );
        return 1;
    }

    NEXUS_Security_GetDefaultAlgorithmSettings ( &algConfig );
    algConfig.algorithm = NEXUS_SecurityAlgorithm_eAes128;
    algConfig.algorithmVar = NEXUS_SecurityAlgorithmVariant_eEcb;
    algConfig.terminationMode = NEXUS_SecurityTerminationMode_eCipherStealing;
    algConfig.keyDestEntryType = NEXUS_SecurityKeyType_eClear;
    algConfig.operation = NEXUS_SecurityOperation_eDecrypt;
    algConfig.dest = NEXUS_SecurityAlgorithmConfigDestination_eCpd;
    NEXUS_Security_ConfigAlgorithm ( keyHandle, &algConfig );

    /* Load AV keys */
    NEXUS_Security_GetDefaultClearKey ( &key );
    key.keyEntryType = NEXUS_SecurityKeyType_eClear;
    key.dest = NEXUS_SecurityAlgorithmConfigDestination_eCpd;
    key.keyIVType = NEXUS_SecurityKeyIVType_eNoIV;
    key.keySize = sizeof ( keys );
    BKNI_Memcpy ( key.keyData, keys, sizeof ( keys ) );

    if ( NEXUS_Security_LoadClearKey ( keyHandle, &key ) != 0 )
    {
        printf ( "\nLoad encryption even key failed \n" );
        return 1;
    }

    if ( NEXUS_SUCCESS != NEXUS_KeySlot_AddPidChannel ( keyHandle, videoPidChannel ) )
    {
        printf ( "\nError: NEXUS_KeySlot_AddPidChannel() failed. " );
        return -1;
    }

    return 0;
}

#else /* NEXUS_HAS_SECURITY */
#include <stdio.h>
int main ( void )
{
    printf ( "This application is not supported on this platform!\n" );
    return -1;
}
#endif
