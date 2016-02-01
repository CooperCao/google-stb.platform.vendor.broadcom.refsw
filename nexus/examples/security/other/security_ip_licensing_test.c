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

/* Nexus example app: set the ipLicensing tokens to BSP. */

/***************************************************************************
 * The related Otp bit must be programmed to enable IP License.
 *
 * The marketId must be programmed, and the IP license data shall include the
 * programmed market Id value.
 *
 ******************************************************************************/

#if NEXUS_HAS_SECURITY && (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 1) && NEXUS_HAS_VIDEO_DECODER && NEXUS_SECURITY_IPLICENSING
#include "nexus_platform.h"

/* Audio and video decoder and playback head files. */
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define RECORDED_FILE "/mnt/nfs/video/576i50_2min.ts"
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eMpeg

/* The video pid of clear program in stream RECORDED_FILE. */
#define VIDEO_PID 2316
#define AUDIO_PID 2317

/* The encrypted data. */
#include "ip_license.h"

static void     print_usage ( void );

int main ( int argc, char **argv )
{

    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;

    /* Playback parameters. */
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel,
                    audioPidChannel;
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

    /* ipLicensing data. */
    unsigned char  *pEncryptedTokens = ipLicense;
    /* App input parameters. */
    int             curarg = 1;

    while ( curarg < argc )
    {
        if ( !strcmp ( argv[curarg], "--help" ) || !strcmp ( argv[curarg], "-h" ) )
        {
            print_usage (  );
            return 0;
        }
        else if ( !strcmp ( argv[curarg], "-disable" ) )
        {
            printf ( "\nDisable all IPs. Audio and video decoding shall stop working." );
            pEncryptedTokens = ipLicenseAllDisabled;
        }
        else if ( !strcmp ( argv[curarg], "-enable" ) )
        {
            printf ( "\nEnable all IPs. Audio and video decoding shall work normally." );
            pEncryptedTokens = ipLicenseAllEnabled;
        }
        else
        {
            printf ( "\nIPs tokens are in 0x11112222...CCCCDDDD pattern." );
        }
        curarg++;
    }

    /* Platform init */
    NEXUS_Platform_GetDefaultSettings ( &platformSettings );
    platformSettings.openFrontend = false;
    platformSettings.openOutputs = true;
    platformSettings.openCec = true;
    platformSettings.openI2c = true;

    /* IP License check is enabled. */
    platformSettings.securitySettings.ipLicense.valid = true;

    /* Initilise platform with the IP licensing tokens, so that the video FW etc can read them when FWs being loaded. */
    BKNI_Memcpy ( &platformSettings.securitySettings.ipLicense.data[0], pEncryptedTokens,
                  NEXUS_SECURITY_IP_LICENCE_SIZE );
    NEXUS_Platform_Init ( &platformSettings );
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

    return rc;
}

static void print_usage ( void )
{
    printf ( "\nusage: nexus security_IP_licensing OPTIONS\n"
             "options are:\n"
             "  --help or -h      print this help screen\n"
             "  -disable       disable all the IPs: video and audio decoding will report error after this configuration.\n"
             "  -enable        enable all the IPs: video and audio decoding shall be normal after this configuration.\n" );
}
#else /* NEXUS_HAS_SECURITY ... */
#include <stdio.h>
int main ( void )
{
    printf ( "This application is not supported on this platform!\n" );
    return -1;
}
#endif
