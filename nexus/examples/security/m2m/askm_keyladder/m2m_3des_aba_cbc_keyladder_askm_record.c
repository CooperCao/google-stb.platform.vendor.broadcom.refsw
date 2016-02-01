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

/* Test with video only, since the audio security setups is very similar to video's. */
#if NEXUS_HAS_SECURITY && (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 4) && NEXUS_HAS_VIDEO_DECODER

#include "nexus_platform.h"
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_recpump.h"
#if NEXUS_HAS_RECORD
#include "nexus_record.h"
#endif

#if NEXUS_HAS_PLAYBACK
#include "nexus_playback.h"
#include "nexus_file.h"
#endif

#include "nexus_dma.h"
#include "nexus_security.h"
#include <stdio.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <assert.h>
#include <string.h>

#include "nexus_keyladder.h"

/* A clear stream, or stream with a clear program to be played and recorded to an encrpted stream. */
#define PLAY_FILE     "/mnt/nfs/video/576i50_2min.ts"

/* Any location that can be accessed by the STB
 * to record the encrypted stream with a clear program in the above stream. */
#define RECORDED_FILE "/mnt/nfs/video/recorded_stream_3des_askm.mpg"

/* The video pid value and parameters that match a clear pid in PLAY_FILE. */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eMpeg

/* The audio pid of the clear program to be recorded and encrypted in PLAY_FILE. */
#define AUDIO_PID (2317 )
/* The video pid of the clear program to be recorded and encrypted in PLAY_FILE. */
#define VIDEO_PID (2316 )

static BERR_Code playpump_setup ( NEXUS_VideoDecoderHandle * videoDecoder,
                                  NEXUS_AudioDecoderHandle * audioDecoder,
                                  NEXUS_PidChannelHandle * pVideoPidChannel,
                                  NEXUS_StcChannelHandle * pStcChannel,
                                  NEXUS_PlaybackHandle * pPlayback, NEXUS_PlaypumpHandle * pPlaypump );

static int      setup_ca_keylader_askm ( NEXUS_PidChannelHandle videoPidChannel, NEXUS_KeySlotHandle * pVideoKeyHandle,
                                         NEXUS_VirtualKeyLadderHandle * vklHandle, NEXUS_SecurityOperation operation );

static NEXUS_VirtualKeyLadderHandle Security_AllocateVkl ( NEXUS_SecurityVirtualKeyladderID * vkl )
{
    NEXUS_SecurityVKLSettings vklSettings;
    NEXUS_VirtualKeyLadderHandle vklHandle = 0;
    NEXUS_VirtualKeyLadderInfo vklInfo;

    BDBG_ASSERT ( vkl );

    NEXUS_Security_GetDefaultVKLSettings ( &vklSettings );

    /* For pre Zeus 3.0, please set vklSettings.custSubMode */

    vklHandle = NEXUS_Security_AllocateVKL ( &vklSettings );

    if ( !vklHandle )
    {
        printf ( "\nAllocate VKL failed \n" );
        return 0;
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

    return vklHandle;
}
int main ( void )
{
#if (NEXUS_HAS_RECORD  && NEXUS_HAS_PLAYBACK)
    NEXUS_KeySlotHandle keyHandle;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_FileRecordHandle recordfile;
    NEXUS_FilePlayHandle playfile;
    NEXUS_RecpumpOpenSettings recpumpOpenSettings;
    NEXUS_RecpumpHandle recpump;
    NEXUS_RecordSettings recordCfg;
    NEXUS_RecordHandle record;
    NEXUS_VirtualKeyLadderHandle vklHandle;

    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_AudioDecoderHandle audioDecoder;

    NEXUS_StcChannelHandle stcChannel;

    const char     *recfname = RECORDED_FILE;
    const char     *playfname = PLAY_FILE;

    NEXUS_PlaypumpHandle playpump;

    NEXUS_PlaybackHandle playback;
    NEXUS_PidChannelHandle videoPidChannel;

    NEXUS_RecordPidChannelSettings pidSettings;

    /* No front and AV output are used in the test. */
    NEXUS_Platform_GetDefaultSettings ( &platformSettings );
    platformSettings.openFrontend = false;
    platformSettings.openOutputs = false;
    platformSettings.openCec = false;
    NEXUS_Platform_Init ( &platformSettings );

    playfile = NEXUS_FilePlay_OpenPosix ( playfname, NULL );
    if ( !playfile )
    {
        fprintf ( stderr, "can't open file:%s\n", playfname );
        return -1;
    }

    /* set up the source */
    playpump_setup ( &videoDecoder, &audioDecoder, &videoPidChannel, &stcChannel, &playback, &playpump );

    if ( !videoPidChannel )
    {
        printf ( "\nError: null video pid channel.\n" );
        return -1;
    }

    /* Setup the security to encrypt the recorded stream. */
    setup_ca_keylader_askm ( videoPidChannel, &keyHandle, &vklHandle, NEXUS_SecurityOperation_eEncrypt );

    /* Setup the record */
    NEXUS_Recpump_GetDefaultOpenSettings ( &recpumpOpenSettings );

    /* set threshold to 30%. with band hold enabled, it is a bandhold threshold. */
    recpumpOpenSettings.data.dataReadyThreshold = recpumpOpenSettings.data.bufferSize * 3 / 10;
    recpumpOpenSettings.index.dataReadyThreshold = recpumpOpenSettings.index.bufferSize * 3 / 10;
    recpump = NEXUS_Recpump_Open ( NEXUS_ANY_ID, &recpumpOpenSettings );

    record = NEXUS_Record_Create (  );
    NEXUS_Record_GetSettings ( record, &recordCfg );
    recordCfg.recpump = recpump;

    /* enable bandhold. required for record from playback. */
    recordCfg.recpumpSettings.bandHold = NEXUS_RecpumpFlowControl_eEnable;
    NEXUS_Record_SetSettings ( record, &recordCfg );

    recordfile = NEXUS_FileRecord_OpenPosix ( recfname, NULL );
    if ( !recordfile )
    {
        fprintf ( stderr, "can't open file:%s \n", recfname );
        return -1;
    }

    NEXUS_Record_GetDefaultPidChannelSettings ( &pidSettings );
    pidSettings.recpumpSettings.pidType = NEXUS_PidType_eVideo;
    pidSettings.recpumpSettings.pidTypeSettings.video.index = true;
    pidSettings.recpumpSettings.pidTypeSettings.video.codec = NEXUS_VideoCodec_eMpeg2;
    NEXUS_Record_AddPidChannel ( record, videoPidChannel, &pidSettings );

    NEXUS_Record_Start ( record, recordfile );

    /* Start playback */
    NEXUS_Playback_Start ( playback, playfile, NULL );

    /* Wait for the two seconds for the record of the short stream completes. */
    sleep ( 2 );

    printf
        ( "Record has been created with files %s. Use cps_3desecb_keyladder_askm_playback to inspect the encrypted stream on HDMI.\n",
          RECORDED_FILE );

    /* Tear down the devices chain */
    NEXUS_Record_Stop ( record );
    NEXUS_Record_RemoveAllPidChannels ( record );

    NEXUS_FileRecord_Close ( recordfile );
    NEXUS_Record_Destroy ( record );
    NEXUS_Recpump_Close ( recpump );

    NEXUS_Playback_Stop ( playback );
    NEXUS_Playback_ClosePidChannel ( playback, videoPidChannel );
    NEXUS_FilePlay_Close ( playfile );
    NEXUS_Playback_Destroy ( playback );
    NEXUS_Playpump_Close ( playpump );

    NEXUS_VideoDecoder_Close ( videoDecoder );
    NEXUS_AudioDecoder_Close ( audioDecoder );
    NEXUS_StcChannel_Close ( stcChannel );

    NEXUS_Security_FreeKeySlot ( keyHandle );

    NEXUS_Security_FreeVKL ( vklHandle );

    NEXUS_Platform_Uninit (  );

#else
    printf ( "This application is not supported on this platform!\n" );
#endif
    return 0;
}

static BERR_Code playpump_setup ( NEXUS_VideoDecoderHandle * videoDecoder,
                                  NEXUS_AudioDecoderHandle * audioDecoder,
                                  NEXUS_PidChannelHandle * pVideoPidChannel,
                                  NEXUS_StcChannelHandle * pStcChannel,
                                  NEXUS_PlaybackHandle * pPlayback, NEXUS_PlaypumpHandle * pPlaypump )
{
    NEXUS_PidChannelHandle videoPidChannel,
                    audioPidChannel;

    NEXUS_StcChannelSettings stcSettings;
    NEXUS_StcChannelHandle stcChannel;

    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;

    /* set up the source */
    playpump = NEXUS_Playpump_Open ( NEXUS_ANY_ID, NULL );
    assert ( playpump );
    playback = NEXUS_Playback_Create (  );
    assert ( playback );

    NEXUS_StcChannel_GetDefaultSettings ( 0, &stcSettings );
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open ( 0, &stcSettings );
    *pStcChannel = stcChannel;

    NEXUS_Playback_GetSettings ( playback, &playbackSettings );
    playbackSettings.playpump = playpump;
    playbackSettings.playpumpSettings.transportType = TRANSPORT_TYPE;

    playbackSettings.stcChannel = stcChannel;
    NEXUS_Playback_SetSettings ( playback, &playbackSettings );

    *videoDecoder = NEXUS_VideoDecoder_Open ( 0, NULL );        /* take default capabilities */

    /* Open the audio and video pid channels */
    NEXUS_Playback_GetDefaultPidChannelSettings ( &playbackPidSettings );
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = VIDEO_CODEC;
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = *videoDecoder;

    /* configure the video pid for indexing */
    videoPidChannel = NEXUS_Playback_OpenPidChannel ( playback, VIDEO_PID, &playbackPidSettings );

    *audioDecoder = NEXUS_AudioDecoder_Open ( 0, NULL );

    NEXUS_Playback_GetDefaultPidChannelSettings ( &playbackPidSettings );
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.primary = *audioDecoder;
    audioPidChannel = NEXUS_Playback_OpenPidChannel ( playback, AUDIO_PID, &playbackPidSettings );

    *pVideoPidChannel = videoPidChannel;

    *pPlayback = playback;
    *pPlaypump = playpump;
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

    /* request for an VKL to use */
    *vklHandle = Security_AllocateVkl ( &vklId );
    if ( !( *vklHandle ) )
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
