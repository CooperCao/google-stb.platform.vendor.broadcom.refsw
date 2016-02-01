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
 /* Example m2m AES-ECB encryption record with clearkey
  * This example makes a pair with playback using "m2m_clearkey_playback.c"
  *
  * The app play a very short clear stream, and then record it to another encrypted stream.
  * This remove the need to setup the frontend and channel information etc.
  */

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

/* A clear stream, or stream with a clear program to be played and recorded to an encrpted stream. */
#define PLAY_FILE     "/mnt/nfs/video/576i50_2min.ts"

/* Any location that can be accessed by the STB
 * to record the encrypted stream with a clear program in the above stream. */
#define RECORDED_FILE "/mnt/nfs/video/recorded_stream.mpg"

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

static BERR_Code load_m2m_clear_keys ( NEXUS_PidChannelHandle videoPidChannel, NEXUS_KeySlotHandle * keyHandle );

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

    /* Setup the security */
    load_m2m_clear_keys ( videoPidChannel, &keyHandle );

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
        ( "Record has been created with files %s. Use m2m_clearkey_playback to inspect the encrypted stream on HDMI.\n",
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
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;

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

static BERR_Code load_m2m_clear_keys ( NEXUS_PidChannelHandle videoPidChannel, NEXUS_KeySlotHandle * pkeyHandle )
{
    NEXUS_KeySlotHandle keyHandle;
    NEXUS_SecurityAlgorithmSettings algConfig;
    NEXUS_SecurityClearKey key;
    NEXUS_SecurityKeySlotSettings keySettings;
    NEXUS_SecurityInvalidateKeySettings invSettings;
    NEXUS_SecurityKeySlotInfo keyslotInfo;

    const uint8_t   keys[16] =
        { 0x6e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7, 0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73 };

    /* Allocate AV keyslots */
    NEXUS_Security_GetDefaultKeySlotSettings ( &keySettings );
    keySettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
	/* For transport stream, use type0 or 2.
	 * By default, type 3 is used, which is also fine for M2M engine. */
    keyHandle = NEXUS_Security_AllocateKeySlot ( &keySettings );
    if ( !keyHandle )
    {
        printf ( "\nAllocate enc keyslot failed \n" );
        return 1;
    }

	/* Sanity check. */
    NEXUS_KeySlot_GetInfo(keyHandle, &keyslotInfo);
    if (keySettings.keySlotType != keyslotInfo.keySlotType && keySettings.keySlotType != NEXUS_SecurityKeySlotType_eAuto)
    {
	printf("Warning: nexus keyslot type mismatches as expected type %d got key type %d\n", keySettings.keySlotType, keyslotInfo.keySlotType);
    }
    else
    {
	printf("Got expected nexus keyslot type %d from key slot handler key type %d\n", keySettings.keySlotType, keyslotInfo.keySlotType);
    }

    /* Invalidate all the keys. */
    NEXUS_Security_GetDefaultInvalidateKeySettings ( &invSettings );
    invSettings.invalidateAllEntries = true;
    invSettings.invalidateKeyType = NEXUS_SecurityInvalidateKeyFlag_eDestKeyOnly;
    NEXUS_Security_InvalidateKey ( keyHandle, &invSettings );

    NEXUS_Security_GetDefaultAlgorithmSettings ( &algConfig );
    algConfig.algorithm = NEXUS_SecurityAlgorithm_eAes128;
    algConfig.algorithmVar = NEXUS_SecurityAlgorithmVariant_eEcb;
    algConfig.terminationMode = NEXUS_SecurityTerminationMode_eCipherStealing;
    algConfig.operation = NEXUS_SecurityOperation_eEncrypt;
    algConfig.dest = NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem;

    algConfig.keyDestEntryType = NEXUS_SecurityKeyType_eClear;
    NEXUS_Security_ConfigAlgorithm ( keyHandle, &algConfig );
    algConfig.keyDestEntryType = NEXUS_SecurityKeyType_eEven;
    NEXUS_Security_ConfigAlgorithm ( keyHandle, &algConfig );

    algConfig.keyDestEntryType = NEXUS_SecurityKeyType_eOdd;
    NEXUS_Security_ConfigAlgorithm ( keyHandle, &algConfig );

    /* Load AV keys */
    NEXUS_Security_GetDefaultClearKey ( &key );
    key.dest = NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem;
    key.keyIVType = NEXUS_SecurityKeyIVType_eNoIV;
    key.keySize = sizeof ( keys );
    BKNI_Memcpy ( key.keyData, keys, sizeof ( keys ) );

    key.keyEntryType = NEXUS_SecurityKeyType_eClear;
    NEXUS_Security_LoadClearKey ( keyHandle, &key );

    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    NEXUS_Security_LoadClearKey ( keyHandle, &key );

    key.keyEntryType = NEXUS_SecurityKeyType_eEven;
    NEXUS_Security_LoadClearKey ( keyHandle, &key );

    NEXUS_KeySlot_AddPidChannel ( keyHandle, videoPidChannel );

    *pkeyHandle = keyHandle;

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
