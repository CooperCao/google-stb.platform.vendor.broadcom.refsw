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

/* Example record CPS AES-ECB encryption with clearkey - same key for m2m decryption playback */
/* This example makes a pair with record app using "cps_aesecb_clearkey_record.c " */

#if NEXUS_HAS_SECURITY && NEXUS_HAS_PLAYBACK && (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 4) && NEXUS_HAS_VIDEO_DECODER

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
#include "nexus_record.h"

#include <assert.h>
#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if NEXUS_DTV_PLATFORM
#include "nexus_platform_boardcfg.h"
#endif
#include "nexus_security.h"
#include <stdio.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

#define CPS_ONLY

#define NUM_STATIC_CHANNELS             4
#define MAX_PROGRAM_TITLE_LENGTH        128
#define NUM_AUDIO_STREAMS               4
#define NUM_VIDEO_STREAMS               4
#define MAX_STREAM_PATH                 4
#define VIDEO_PID                       2316
#define AUDIO_PID                       2317

#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eMpeg

#ifndef CPS_ONLY
static NEXUS_Error ConfigureKeySlotFor3DesCA ( NEXUS_KeySlotHandle keyHandle,
                                               unsigned char *pKeyEven, unsigned char *pKeyOdd );
#endif

static NEXUS_Error ConfigureKeySlotForAesCPD ( NEXUS_KeySlotHandle keyHandle, unsigned char *pKeyOdd );

int main ( void )
{

    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
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
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_FilePlayHandle playfile;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
#if NEXUS_NUM_HDMI_OUTPUTS
/* Support HDMI output */
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
#endif
    unsigned int    videoPID,
                    audioPID;
    NEXUS_SecurityKeySlotSettings keySlotSettings;
    NEXUS_KeySlotHandle videoKeyHandle = NULL;
    NEXUS_KeySlotHandle audioKeyHandle = NULL;
    NEXUS_SecurityInvalidateKeySettings invSettings;
    NEXUS_PidChannelStatus pidStatus;

    NEXUS_Error     rc;
    const char     *playfname = "/mnt/nfs/video/aesecb_clearKey_576i50_2min.mpg";

#ifndef CPS_ONLY

    unsigned char   VidEvenControlWord[] = {
        0x2e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7,
        0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73
    };
    unsigned char   VidOddControlWord[] = {
        0x6e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7,
        0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73
    };
    unsigned char   AudEvenControlWord[] = {
        0x8e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7,
        0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73
    };
    unsigned char   AudOddControlWord[] = {
        0x0e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7,
        0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73
    };

#endif
    unsigned char   VidClearCpsControlWord[] =
        { 0x6e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7, 0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73 };
    unsigned char   AudOddCpsControlWord[] =
        { 0x6e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7, 0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73 };

    NEXUS_Platform_GetDefaultSettings ( &platformSettings );
    platformSettings.openFrontend = false;

    if ( NEXUS_Platform_Init ( &platformSettings ) )
    {
        return -1;
    }

    NEXUS_Platform_GetConfiguration ( &platformConfig );

    playpump = NEXUS_Playpump_Open ( NEXUS_ANY_ID, NULL );
    BDBG_ASSERT ( playpump );
    playback = NEXUS_Playback_Create (  );
    BDBG_ASSERT ( playback );

    printf ( "\n -- playback stream %s.\n", playfname );
    playfile = NEXUS_FilePlay_OpenPosix ( playfname, NULL );
    if ( !playfile )
    {
        fprintf ( stderr, "can't open file:%s\n", playfname );
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
    playbackPidSettings.pidTypeSettings.video.codec = VIDEO_CODEC;      /* must be told codec for correct handling */
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

    /***************************************************************************************
		Config CA descrambler
	***************************************************************************************/
    NEXUS_Security_GetDefaultKeySlotSettings ( &keySlotSettings );
    keySlotSettings.keySlotEngine = NEXUS_SecurityEngine_eCa;   /* For decryption the stream. */

    /* Allocate AV keyslots */
    videoKeyHandle = NEXUS_Security_AllocateKeySlot ( &keySlotSettings );
    if ( !videoKeyHandle )
    {
        printf ( "\nAllocate CACP video keyslot failed \n" );
        return 1;
    }
    audioKeyHandle = NEXUS_Security_AllocateKeySlot ( &keySlotSettings );
    if ( !audioKeyHandle )
    {
        printf ( "\nAllocate CACP audio keyslot failed \n" );
        return 1;
    }

    /* Invalidate all the keys. */
    NEXUS_Security_GetDefaultInvalidateKeySettings ( &invSettings );
    invSettings.invalidateAllEntries = true;
    invSettings.invalidateKeyType = NEXUS_SecurityInvalidateKeyFlag_eDestKeyOnly;
    NEXUS_Security_InvalidateKey ( videoKeyHandle, &invSettings );

/* Just testing CPS clear stream input encryption even though opening CaCp engine */
#ifndef CPS_ONLY
    /* Config AV algorithms */
    if ( ConfigureKeySlotFor3DesCA ( videoKeyHandle, VidEvenControlWord, VidOddControlWord ) != 0 )
    {
        printf ( "\nConfig video CA Algorithm failed for video keyslot 1 \n" );
        return 1;
    }

    if ( ConfigureKeySlotFor3DesCA ( audioKeyHandle, AudEvenControlWord, AudOddControlWord ) != 0 )
    {
        printf ( "\nConfig audio CA Algorithm failed for audio keyslot 1 \n" );
        return 1;
    }

#endif
    printf ( "\n CPS descrambler settings.\n" );

        /***************************************************************************************
	***************************************************************************************/

    if ( ConfigureKeySlotForAesCPD ( videoKeyHandle, VidClearCpsControlWord ) != 0 )
    {
        printf ( "\nConfig video CPD Algorithm failed for video keyslot 1 \n" );
        return 1;
    }

    printf ( "\nSecurity Config even CW OK\n" );
    if ( ConfigureKeySlotForAesCPD ( audioKeyHandle, AudOddCpsControlWord ) != 0 )
    {
        printf ( "\nConfig audio CPD Algorithm failed for audio keyslot 1 \n" );
        return 1;
    }

    printf ( "\nSecurity Config audio odd CW OK\n" );

    /* Add video PID channel to keyslot */
    NEXUS_PidChannel_GetStatus ( videoPidChannel, &pidStatus );
    videoPID = pidStatus.pidChannelIndex;
    NEXUS_Security_AddPidChannelToKeySlot ( videoKeyHandle, videoPID );
    printf ( "\nSecurity Add video Pid channel OK\n" );
    NEXUS_PidChannel_GetStatus ( audioPidChannel, &pidStatus );
    audioPID = pidStatus.pidChannelIndex;
    NEXUS_Security_AddPidChannelToKeySlot ( audioKeyHandle, audioPID );

    printf ( "\nSecurity Config OK\n" );

    /* Start playback */
    NEXUS_Playback_Start ( playback, playfile, NULL );

    /* Playback state machine is driven from inside Nexus. */
    printf ( "Press ENTER to quit\n" );
    getchar (  );
    /* Bring down system */
    NEXUS_VideoWindow_Close ( window );
    NEXUS_Display_Close ( display );

    NEXUS_Playback_Stop ( playback );
    NEXUS_Playback_ClosePidChannel ( playback, videoPidChannel );
    NEXUS_FilePlay_Close ( playfile );
    NEXUS_Playback_Destroy ( playback );
    NEXUS_Playpump_Close ( playpump );

    NEXUS_VideoDecoder_Close ( videoDecoder );
    NEXUS_AudioDecoder_Close ( audioDecoder );
    NEXUS_StcChannel_Close ( stcChannel );

    NEXUS_Security_RemovePidChannelFromKeySlot ( videoKeyHandle, videoPID );
    NEXUS_Security_RemovePidChannelFromKeySlot ( audioKeyHandle, audioPID );
    NEXUS_Security_FreeKeySlot ( videoKeyHandle );
    NEXUS_Security_FreeKeySlot ( audioKeyHandle );
    printf ( "finish free keyslot\n" );

    NEXUS_Platform_Uninit (  );

    /* loops forever */

    return 1;

}

    /* Some aspects of this configuration are specific to 40nm HSM */
#ifndef CPS_ONLY
static NEXUS_Error ConfigureKeySlotFor3DesCA ( NEXUS_KeySlotHandle keyHandle,
                                               unsigned char *pKeyEven, unsigned char *pKeyOdd )
{
    NEXUS_SecurityAlgorithmSettings AlgConfig;
    NEXUS_SecurityClearKey key;

    NEXUS_Security_GetDefaultAlgorithmSettings ( &AlgConfig );
    AlgConfig.algorithm = NEXUS_SecurityAlgorithm_e3DesAba;
    AlgConfig.terminationMode = NEXUS_SecurityTerminationMode_eCipherStealing;
    AlgConfig.ivMode = NEXUS_SecurityIVMode_eRegular;
    AlgConfig.solitarySelect = NEXUS_SecuritySolitarySelect_eClear;
    AlgConfig.caVendorID = 0x1234;
    AlgConfig.askmModuleID = NEXUS_SecurityAskmModuleID_eModuleID_4;
    AlgConfig.otpId = NEXUS_SecurityOtpId_eOtpVal;
    AlgConfig.key2Select = NEXUS_SecurityKey2Select_eReserved1;
    AlgConfig.dest = NEXUS_SecurityAlgorithmConfigDestination_eCa;
    AlgConfig.operation = NEXUS_SecurityOperation_eDecrypt;

    AlgConfig.modifyScValue[NEXUS_SecurityPacketType_eRestricted] = false;
    AlgConfig.modifyScValue[NEXUS_SecurityPacketType_eGlobal] = false;

    if ( NEXUS_Security_ConfigAlgorithm ( keyHandle, &AlgConfig ) != 0 )
    {
        printf ( "\nConfig video CPS Algorithm failed \n" );
        return 1;
    }

    /* Load AV keys */
    NEXUS_Security_GetDefaultClearKey ( &key );
    key.keySize = 16;
    key.dest = NEXUS_SecurityAlgorithmConfigDestination_eCa;
    key.keyIVType = NEXUS_SecurityKeyIVType_eNoIV;

    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    memcpy ( key.keyData, pKeyOdd, key.keySize );
    if ( NEXUS_Security_LoadClearKey ( keyHandle, &key ) != 0 )
    {
        printf ( "\nLoad CA ODD key failed \n" );
        return 1;
    }

    key.keyEntryType = NEXUS_SecurityKeyType_eEven;
    memcpy ( key.keyData, pKeyEven, key.keySize );
    if ( NEXUS_Security_LoadClearKey ( keyHandle, &key ) != 0 )
    {
        printf ( "\nLoad CA EVEN key failed \n" );
        return 1;
    }

    return 0;

}
#endif

static NEXUS_Error ConfigureKeySlotForAesCPD ( NEXUS_KeySlotHandle keyHandle, unsigned char *pKeyOdd )
{
    NEXUS_SecurityAlgorithmSettings AlgConfig;
    NEXUS_SecurityClearKey key;

    NEXUS_Security_GetDefaultAlgorithmSettings ( &AlgConfig );
    AlgConfig.algorithm = NEXUS_SecurityAlgorithm_eAes128;
    AlgConfig.algorithmVar = NEXUS_SecurityAlgorithmVariant_eEcb;
    AlgConfig.terminationMode = NEXUS_SecurityTerminationMode_eCipherStealing;
    AlgConfig.dest = NEXUS_SecurityAlgorithmConfigDestination_eCpd;
    AlgConfig.operation = NEXUS_SecurityOperation_eDecrypt;

    /* The recorded stream has SC bits of Odd. */
    AlgConfig.keyDestEntryType = NEXUS_SecurityKeyType_eOdd;
    NEXUS_Security_ConfigAlgorithm ( keyHandle, &AlgConfig );

    /* Load AV keys */
    NEXUS_Security_GetDefaultClearKey ( &key );
    key.keySize = 16;

    key.dest = NEXUS_SecurityAlgorithmConfigDestination_eCpd;
    key.keyIVType = NEXUS_SecurityKeyIVType_eNoIV;
    memcpy ( key.keyData, pKeyOdd, key.keySize );

    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    NEXUS_Security_LoadClearKey ( keyHandle, &key );
    return 0;
}

#else /* #if NEXUS_HAS_SECURITY && NEXUS_HAS_PLAYBACK && (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 4) && NEXUS_HAS_VIDEO_DECODER */

#include <stdio.h>
int main ( void )
{
    printf ( "This application is not supported on this platform!\n" );
    return -1;
}
#endif
