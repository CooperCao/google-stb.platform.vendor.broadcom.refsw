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

/* Nexus example app: playback and decode the short stream recorded by test app
 * cps_3desecb_keyladder_askm_record.c */

#if NEXUS_HAS_SECURITY  &&     (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 4)

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
#include <assert.h>
#include <string.h>

#include "nexus_dma.h"
#include "nexus_security.h"

#include "nexus_keyladder.h"

#undef CUSS_KEY

/* This can be any stream file recorded stream from test app cps_3desecb_keyladder_askm_record with encryption. */

#define RECORDED_FILE "/mnt/nfs/video/recorded_stream_3des_askm.mpg"
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eMpeg

/* The video pid of clear program in stream RECORDED_FILE. */
#define VIDEO_PID 2316
#define AUDIO_PID 2317

static int      setup_ca_keylader_askm ( NEXUS_PidChannelHandle videoPidChannel,
                                         NEXUS_KeySlotHandle * pVideoKeyHandle, NEXUS_VirtualKeyLadderHandle * vkl,
                                         NEXUS_SecurityOperation operation );
static int Security_AllocateVkl ( NEXUS_SecurityCustomerSubMode custSubMode, NEXUS_SecurityVirtualKeyladderID * vkl,
                                  NEXUS_VirtualKeyLadderHandle * vklHandle )
{
    NEXUS_SecurityVKLSettings vklSettings;
    NEXUS_VirtualKeyLadderInfo vklInfo;

    BDBG_ASSERT ( vkl );
    BDBG_ASSERT ( vklHandle );

    NEXUS_Security_GetDefaultVKLSettings ( &vklSettings );

    /* For pre Zeus 3.0, please set vklSettings.custSubMode */
    vklSettings.custSubMode = custSubMode;

    *vklHandle = NEXUS_Security_AllocateVKL ( &vklSettings );

    if ( !( *vklHandle ) )
    {
        printf ( "\nAllocate VKL failed \n" );
        return 1;
    }

    NEXUS_Security_GetVKLInfo ( *vklHandle, &vklInfo );
    printf ( "\nVKL Handle %p is allocated for VKL#%d\n", ( void * ) *vklHandle, vklInfo.vkl & 0x7F );

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
    NEXUS_KeySlotHandle keyHandle;
    NEXUS_VirtualKeyLadderHandle vklHandle;
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

    printf ( "\n -- playing stream file %s.\n", fname );

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
    setup_ca_keylader_askm ( videoPidChannel, &keyHandle, &vklHandle, NEXUS_SecurityOperation_eDecrypt );

    /* Start decoders */
    NEXUS_VideoDecoder_Start ( videoDecoder, &videoProgram );
    NEXUS_AudioDecoder_Start ( audioDecoder, &audioProgram );

    printf ( "Press q to quit, other key to check the decoder status. \n" );
    /* Start playback */
    NEXUS_Playback_Start ( playback, file, NULL );
    /* Print status while decoding */
    while ( getchar (  ) != 'q' )
    {
        NEXUS_VideoDecoderStatus status;
        uint32_t        stc;

        NEXUS_VideoDecoder_GetStatus ( videoDecoder, &status );
        NEXUS_StcChannel_GetStc ( videoProgram.stcChannel, &stc );
        printf ( "decode %.4dx%.4d, pts %#x, stc %#x (diff %d) fifo=%d%%\n",
                 status.source.width, status.source.height, status.pts, stc, status.ptsStcDifference,
                 status.fifoSize ? ( status.fifoDepth * 100 ) / status.fifoSize : 0 );
        BKNI_Sleep ( 1000 );
    }
    /* Playback state machine is driven from inside Nexus. */

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
    NEXUS_Security_FreeKeySlot ( keyHandle );
    NEXUS_Security_FreeVKL ( vklHandle );
    NEXUS_Platform_Uninit (  );

#else
    printf ( "This application is not supported on this platform!\n" );
#endif
    return 0;
}

static int setup_ca_keylader_askm ( NEXUS_PidChannelHandle videoPidChannel, NEXUS_KeySlotHandle * pVideoKeyHandle,
                                    NEXUS_VirtualKeyLadderHandle * vklHandle, NEXUS_SecurityOperation operation )
{
    NEXUS_SecurityEncryptedSessionKey encryptedSessionkey;
    NEXUS_SecurityEncryptedControlWord encrytedCW;
    NEXUS_SecurityAlgorithmSettings NexusConfig;
    NEXUS_SecurityKeySlotSettings keySlotSettings;
    NEXUS_KeySlotHandle videoKeyHandle;
    NEXUS_SecurityVirtualKeyladderID vklId;

    /* The encryption keys are the same for Audio/Video EVEN/ODD keys in this example.  Those
     * Keys can be different in a real application */

    unsigned char   ucProcInForKey3[16] = {
        0xEB, 0x05, 0x5B, 0x0E, /*high key */
        0xAC, 0xc0, 0x94, 0x31,
        0xEB, 0x05, 0x5B, 0x0E, /*low key  use twice */
        0xAC, 0xc0, 0x94, 0x30
    };

    unsigned char   ucProcInKey4[16] = {
        0xc1, 0x2c, 0x19, 0x24,
        0xd4, 0x84, 0x66, 0xa1,
        0xb4, 0x05, 0x3c, 0xac,
        0xe8, 0x0c, 0x95, 0x0e,
    };

    /* Request for an VKL to use */
    if ( Security_AllocateVkl ( NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4, &vklId, vklHandle ) )
    {
        printf ( "\nAllocate VKL failed.\n" );
        return 1;
    }

    NEXUS_Security_GetDefaultKeySlotSettings ( &keySlotSettings );
    /* keySlotSettings.keySlotEngine = NEXUS_SecurityEngine_eCaCp; */
    keySlotSettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
    /* Allocate AV keyslots */
    videoKeyHandle = NEXUS_Security_AllocateKeySlot ( &keySlotSettings );
    if ( !videoKeyHandle )
    {
        printf ( "\nAllocate CA video keyslot failed \n" );
        return 1;
    }

    /* Set up key for clear key */
    NEXUS_Security_GetDefaultAlgorithmSettings ( &NexusConfig );
    NexusConfig.algorithm = NEXUS_SecurityAlgorithm_e3DesAba;
    NexusConfig.algorithmVar = NEXUS_SecurityAlgorithmVariant_eEcb;

    NexusConfig.terminationMode = NEXUS_SecurityTerminationMode_eClear;
    NexusConfig.ivMode = NEXUS_SecurityIVMode_eRegular;
    NexusConfig.solitarySelect = NEXUS_SecuritySolitarySelect_eClear;
    NexusConfig.caVendorID = 0x1234;
    NexusConfig.askmModuleID = NEXUS_SecurityAskmModuleID_eModuleID_8;
    NexusConfig.otpId = NEXUS_SecurityOtpId_eOneVal;
    NexusConfig.key2Select = NEXUS_SecurityKey2Select_eFixedKey;

    NexusConfig.keyDestEntryType = NEXUS_SecurityKeyType_eClear;

    /* Just test encrypt a stream with ASKM and record it to another stream
     * and descrpyt the recorded stream. */
    NexusConfig.operation = operation;
    NexusConfig.dest =
        ( operation ==
          NEXUS_SecurityOperation_eEncrypt ) ? NEXUS_SecurityAlgorithmConfigDestination_eCps :
        NEXUS_SecurityAlgorithmConfigDestination_eCpd;

    /* configure the key slot */
    NEXUS_Security_ConfigAlgorithm ( videoKeyHandle, &NexusConfig );

    memset ( &encryptedSessionkey, 0, sizeof ( NEXUS_SecurityEncryptedSessionKey ) );
    memset ( &encrytedCW, 0, sizeof ( NEXUS_SecurityEncryptedControlWord ) );
    /* Load session key - key3 */
    NEXUS_Security_GetDefaultSessionKeySettings ( &encryptedSessionkey );
    encryptedSessionkey.keyladderType = NEXUS_SecurityKeyladderType_e3Des;
    encryptedSessionkey.swizzleType = NEXUS_SecuritySwizzleType_eNone;
    encryptedSessionkey.cusKeyL = 0x00;
    encryptedSessionkey.cusKeyH = 0x00;
    encryptedSessionkey.cusKeyVarL = 0x00;
    encryptedSessionkey.cusKeyVarH = 0xFF;
    encryptedSessionkey.keyGenCmdID = NEXUS_SecurityKeyGenCmdID_eKeyGen;
    encryptedSessionkey.sessionKeyOp = NEXUS_SecuritySessionKeyOp_eNoProcess;
    encryptedSessionkey.bASKMMode = true;
    encryptedSessionkey.rootKeySrc = NEXUS_SecurityRootKeySrc_eOtpKeyA;
    encryptedSessionkey.bSwapAESKey = false;
    encryptedSessionkey.keyDestIVType = NEXUS_SecurityKeyIVType_eNoIV;
    encryptedSessionkey.bRouteKey = false;
    encryptedSessionkey.operation = NEXUS_SecurityOperation_eDecrypt;
    encryptedSessionkey.operationKey2 = NEXUS_SecurityOperation_eEncrypt;
    encryptedSessionkey.keyEntryType = NEXUS_SecurityKeyType_eClear;

    encryptedSessionkey.custSubMode = NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4;
    encryptedSessionkey.virtualKeyLadderID = vklId;
    encryptedSessionkey.keyMode = NEXUS_SecurityKeyMode_eRegular;

    BKNI_Memcpy ( encryptedSessionkey.keyData, ucProcInForKey3, sizeof ( ucProcInForKey3 ) );

    NEXUS_Security_GenerateSessionKey ( videoKeyHandle, &encryptedSessionkey );

    /* Load CW - key4 --- routed out */
    NEXUS_Security_GetDefaultControlWordSettings ( &encrytedCW );
    encrytedCW.keyladderType = NEXUS_SecurityKeyladderType_e3Des;
    encrytedCW.keySize = sizeof ( ucProcInKey4 );
    encrytedCW.keyEntryType = NEXUS_SecurityKeyType_eClear;

    encrytedCW.custSubMode = NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4;
    encrytedCW.virtualKeyLadderID = vklId;
    encrytedCW.keyMode = NEXUS_SecurityKeyMode_eRegular;

    BKNI_Memcpy ( encrytedCW.keyData, ucProcInKey4, encrytedCW.keySize );
    encrytedCW.operation = NEXUS_SecurityOperation_eDecrypt;
    encrytedCW.bRouteKey = true;
    encrytedCW.keyDestIVType = NEXUS_SecurityKeyIVType_eNoIV;
    encrytedCW.keyGenCmdID = NEXUS_SecurityKeyGenCmdID_eKeyGen;
    encrytedCW.bSwapAESKey = false;

    printf ( "\n\n\n\nGenerate control word\n" );

    NEXUS_Security_GenerateControlWord ( videoKeyHandle, &encrytedCW );

    /* Add video PID channel to keyslot */
    NEXUS_KeySlot_AddPidChannel ( videoKeyHandle, videoPidChannel );

    *pVideoKeyHandle = videoKeyHandle;

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
