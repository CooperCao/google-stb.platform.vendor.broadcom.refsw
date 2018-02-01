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
/* Nexus example app: playback and decode */
#if NEXUS_HAS_SECURITY && NEXUS_HAS_PLAYBACK && (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 4) && NEXUS_HAS_VIDEO_DECODER

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
#include "nexus_security.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_PLAYBACK
#include "nexus_playback.h"
#include "nexus_file.h"
#endif
#if NEXUS_DTV_PLATFORM
#include "nexus_platform_boardcfg.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include "bstd.h"
#include "bkni.h"

#if    (BCHP_CHIP == 7422) || (BCHP_CHIP == 7346) || (BCHP_CHIP == 7344) || (BCHP_CHIP == 7435) || (BCHP_CHIP == 7425) || (BCHP_CHIP == 7231) || (BCHP_CHIP == 7230) \
    || (BCHP_CHIP == 7358) || (BCHP_CHIP == 7552) || (BCHP_CHIP == 7445)
#define HSM_IS_40NM_PLUS    1
#endif

int main ( void )
{
#if NEXUS_HAS_PLAYBACK
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
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    /*const char *fname = "videos/spiderman_aes_cbc_short.ts"; */
    const char     *fname = "/mnt/nfs/spiderman_aes.ts";

    /* The encryption keys are the same for Audio/Video EVEN/ODD keys in this example.  Those
     * Keys can be different in a real application */
    unsigned char   VidEvenControlWord[] = {
        0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
        0x12, 0x34, 0xfe, 0xed, 0xba, 0xbe, 0xbe, 0xef
    };
    unsigned char   VidOddControlWord[] = {
        0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
        0x12, 0x34, 0xfe, 0xed, 0xba, 0xbe, 0xbe, 0xef
    };
    unsigned char   AudEvenControlWord[] = {
        0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
        0x12, 0x34, 0xfe, 0xed, 0xba, 0xbe, 0xbe, 0xef
    };
    unsigned char   AudOddControlWord[] = {
        0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
        0x12, 0x34, 0xfe, 0xed, 0xba, 0xbe, 0xbe, 0xef
    };

    unsigned char   iv[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    NEXUS_KeySlotHandle videoKeyHandle = NULL;
    NEXUS_KeySlotHandle audioKeyHandle = NULL;
    NEXUS_SecurityAlgorithmSettings AlgConfig;
    NEXUS_SecurityClearKey key;
    NEXUS_SecurityKeySlotSettings keySlotSettings;

    NEXUS_Platform_GetDefaultSettings ( &platformSettings );
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init ( &platformSettings );
    NEXUS_Platform_GetConfiguration ( &platformConfig );

    playpump = NEXUS_Playpump_Open ( 0, NULL );
    assert ( playpump );
    playback = NEXUS_Playback_Create (  );
    assert ( playback );

    file = NEXUS_FilePlay_OpenPosix ( fname, NULL );
    if ( !file )
    {
        fprintf ( stderr, "can't open file:%s\n", fname );
        return -1;
    }
    else
    {
        fprintf ( stderr, "file exists:%s\n", fname );
    }

    NEXUS_StcChannel_GetDefaultSettings ( 0, &stcSettings );
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open ( 0, &stcSettings );

    NEXUS_Playback_GetSettings ( playback, &playbackSettings );
    playbackSettings.playpump = playpump;
    /* set a stream format, it could be any audio video transport type or file format, i.e NEXUS_TransportType_eMp4, NEXUS_TransportType_eAvi ... */
    playbackSettings.playpumpSettings.transportType = NEXUS_TransportType_eTs;
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

    /* Bring up video display and outputs */
    display = NEXUS_Display_Open ( 0, NULL );
#if NEXUS_DTV_PLATFORM
    NEXUS_Display_AddOutput ( display, NEXUS_PanelOutput_GetConnector ( platformConfig.outputs.panel[0] ) );
    NEXUS_BoardCfg_ConfigurePanel ( true, true, true );
#else
#if NEXUS_NUM_COMPONENT_OUTPUTS
    if ( platformConfig.outputs.component[0] )
    {
        NEXUS_Display_AddOutput ( display, NEXUS_ComponentOutput_GetConnector ( platformConfig.outputs.component[0] ) );
    }
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput ( display, NEXUS_CompositeOutput_GetConnector ( platformConfig.outputs.composite[0] ) );
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput ( display, NEXUS_HdmiOutput_GetVideoConnector ( platformConfig.outputs.hdmi[0] ) );
#endif
#endif
    window = NEXUS_VideoWindow_Open ( display, 0 );

    /* bring up decoder and connect to display */
    videoDecoder = NEXUS_VideoDecoder_Open ( 0, NULL ); /* take default capabilities */
    NEXUS_VideoWindow_AddInput ( window, NEXUS_VideoDecoder_GetConnector ( videoDecoder ) );

    /* Open the audio and video pid channels */
    NEXUS_Playback_GetDefaultPidChannelSettings ( &playbackPidSettings );
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = NEXUS_VideoCodec_eMpeg2;  /* must be told codec for correct handling */
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
    videoPidChannel = NEXUS_Playback_OpenPidChannel ( playback, 0x11, &playbackPidSettings );

    NEXUS_Playback_GetDefaultPidChannelSettings ( &playbackPidSettings );
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.primary = audioDecoder;
    audioPidChannel = NEXUS_Playback_OpenPidChannel ( playback, 0x14, &playbackPidSettings );

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings ( &videoProgram );
    videoProgram.codec = NEXUS_VideoCodec_eMpeg2;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
    NEXUS_AudioDecoder_GetDefaultStartSettings ( &audioProgram );
    audioProgram.codec = NEXUS_AudioCodec_eAc3;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;

    /****************************************************************************************/
    /* Config CA descrambler                                                                                                                    */
    /****************************************************************************************/

    NEXUS_Security_GetDefaultKeySlotSettings ( &keySlotSettings );
    keySlotSettings.keySlotEngine = NEXUS_SecurityEngine_eCa;
    /* Allocate AV keyslots */
    videoKeyHandle = NEXUS_Security_AllocateKeySlot ( &keySlotSettings );
    if ( !videoKeyHandle )
    {
        printf ( "\nAllocate CA video keyslot failed \n" );
        return 1;
    }
    audioKeyHandle = NEXUS_Security_AllocateKeySlot ( &keySlotSettings );
    if ( !audioKeyHandle )
    {
        printf ( "\nAllocate CA audio keyslot failed \n" );
        return 1;
    }

    /* Config AV algorithms */
    NEXUS_Security_GetDefaultAlgorithmSettings ( &AlgConfig );
    AlgConfig.algorithm = NEXUS_SecurityAlgorithm_eAes;
    AlgConfig.algorithmVar = NEXUS_SecurityAlgorithmVariant_eCbc;
    AlgConfig.terminationMode = NEXUS_SecurityTerminationMode_eBlock;

#if  !HSM_IS_40NM_PLUS
    printf ( " ==========HSM_IS_NOT_40NM============\n" );
    /* ++++++++ */
    AlgConfig.ivMode = NEXUS_SecurityIVMode_eRegular;
#endif

    if ( NEXUS_Security_ConfigAlgorithm ( videoKeyHandle, &AlgConfig ) != 0 )
    {
        printf ( "\nConfig video CA Algorithm failed \n" );
        return 1;
    }
    if ( NEXUS_Security_ConfigAlgorithm ( audioKeyHandle, &AlgConfig ) != 0 )
    {
        printf ( "\nConfig video CA Algorithm failed \n" );
        return 1;
    }

    /* Load AV keys */
    key.keySize = sizeof ( VidEvenControlWord );
    key.keyEntryType = NEXUS_SecurityKeyType_eEven;
    /*--------*/
#if HSM_IS_40NM_PLUS
    key.keyIVType = NEXUS_SecurityKeyIVType_eNoIV;
#endif
    /*--------*/
    memcpy ( key.keyData, VidEvenControlWord, sizeof ( VidEvenControlWord ) );
    if ( NEXUS_Security_LoadClearKey ( videoKeyHandle, &key ) != 0 )
    {
        printf ( "\nLoad video EVEN key failed \n" );
        return 1;
    }
    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    memcpy ( key.keyData, VidOddControlWord, sizeof ( VidOddControlWord ) );
    if ( NEXUS_Security_LoadClearKey ( videoKeyHandle, &key ) != 0 )
    {
        printf ( "\nLoad video ODD key failed \n" );
        return 1;
    }

    key.keyEntryType = NEXUS_SecurityKeyType_eEven;
    memcpy ( key.keyData, AudEvenControlWord, sizeof ( AudEvenControlWord ) );
    if ( NEXUS_Security_LoadClearKey ( audioKeyHandle, &key ) != 0 )
    {
        printf ( "\nLoad audio EVEN key failed \n" );
        return 1;
    }
    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    memcpy ( key.keyData, AudOddControlWord, sizeof ( AudOddControlWord ) );
    if ( NEXUS_Security_LoadClearKey ( audioKeyHandle, &key ) != 0 )
    {
        printf ( "\nLoad audio ODD key failed \n" );
        return 1;
    }

    /* Loading of IV values */
    key.keySize = sizeof ( iv );

    /*--------*/
#if HSM_IS_40NM_PLUS
    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    key.keyIVType = NEXUS_SecurityKeyIVType_eIV;
#else
    key.keyEntryType = NEXUS_SecurityKeyType_eIv;
#endif
    /*--------*/

    memcpy ( key.keyData, iv, sizeof ( iv ) );
    if ( NEXUS_Security_LoadClearKey ( videoKeyHandle, &key ) != 0 )
    {
        printf ( "\nLoad video IV failed \n" );
        return 1;
    }
    memcpy ( key.keyData, iv, sizeof ( iv ) );
    if ( NEXUS_Security_LoadClearKey ( audioKeyHandle, &key ) != 0 )
    {
        printf ( "\nLoad audio IV failed \n" );
        return 1;
    }

#if HSM_IS_40NM_PLUS
    key.keyEntryType = NEXUS_SecurityKeyType_eEven;
    key.keyIVType = NEXUS_SecurityKeyIVType_eIV;

    memcpy ( key.keyData, iv, sizeof ( iv ) );
    if ( NEXUS_Security_LoadClearKey ( videoKeyHandle, &key ) != 0 )
    {
        printf ( "\nLoad video IV failed \n" );
        return 1;
    }
    memcpy ( key.keyData, iv, sizeof ( iv ) );
    if ( NEXUS_Security_LoadClearKey ( audioKeyHandle, &key ) != 0 )
    {
        printf ( "\nLoad audio IV failed \n" );
        return 1;
    }
#endif

    /* Add video PID channel to keyslot */
    NEXUS_KeySlot_AddPidChannel ( videoKeyHandle, videoProgram.pidChannel );

    /* Add audio PID channel to keyslot */
    NEXUS_KeySlot_AddPidChannel ( audioKeyHandle, audioProgram.pidChannel );

    printf ( "\nSecurity Config OK\n" );

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
    NEXUS_VideoInput_Shutdown ( NEXUS_VideoDecoder_GetConnector ( videoDecoder ) );
    NEXUS_VideoDecoder_Close ( videoDecoder );
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_RemoveAllInputs ( NEXUS_AudioDac_GetConnector ( platformConfig.outputs.audioDacs[0] ) );
#endif

#if NEXUS_NUM_SPDIF_INPUTS
    NEXUS_AudioOutput_RemoveAllInputs ( NEXUS_SpdifOutput_GetConnector ( platformConfig.outputs.spdif[0] ) );
#endif
    NEXUS_AudioInput_Shutdown ( NEXUS_AudioDecoder_GetConnector
                                ( audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo ) );
    NEXUS_AudioDecoder_Close ( audioDecoder );
    NEXUS_Display_Close ( display );
    NEXUS_StcChannel_Close ( stcChannel );
    NEXUS_Platform_Uninit (  );

#endif
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
