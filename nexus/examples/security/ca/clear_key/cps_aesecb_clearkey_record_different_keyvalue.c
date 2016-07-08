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
/* Example record CPS AES-ECB encryption with clearkey - same key for m2m decryption playback */
/* This example makes a pair with  playback using "cps_aesecb_clearkey_playback.c " */
#if NEXUS_HAS_SECURITY  && (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 4) && NEXUS_HAS_VIDEO_DECODER

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
#include "nexus_recpump.h"
#include <stdio.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

#define ENABLE_CRYPTO
#define CPS_ONLY

#define NUM_STATIC_CHANNELS             4
#define MAX_PROGRAM_TITLE_LENGTH       128
#define NUM_AUDIO_STREAMS               4
#define NUM_VIDEO_STREAMS               4
#define MAX_STREAM_PATH				  4
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eMpeg
#define VIDEO_PID (2316 )
#define AUDIO_PID (2317 )

static BERR_Code playpump_setup (NEXUS_VideoDecoderHandle * videoDecoder,
                                 NEXUS_AudioDecoderHandle * audioDecoder,
                                 NEXUS_PidChannelHandle * pVideoPidChannel,
                                 NEXUS_PidChannelHandle * pAudioPidChannel,
                                 NEXUS_StcChannelHandle * pStcChannel,
                                 NEXUS_PlaybackHandle * pPlayback, NEXUS_PlaypumpHandle * pPlaypump);

#ifndef CPS_ONLY
static NEXUS_Error ConfigureKeySlotFor3DesCA (NEXUS_KeySlotHandle keyHandle,
                                              unsigned char *pKeyEven, unsigned char *pKeyOdd);
#endif

static NEXUS_Error ConfigureKeySlotForAesCPS (NEXUS_KeySlotHandle keyHandle,
                                              unsigned char *pKeyEven,
                                              unsigned char *pKeyOdd, NEXUS_SecurityAlgorithmScPolarity polarity);

int main (void)
{
#if (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 4)

#if NEXUS_HAS_RECORD
    NEXUS_PlatformSettings platformSettings;
    NEXUS_RecpumpHandle recpump;
    NEXUS_RecordHandle record;
    NEXUS_RecordSettings recordCfg;
    NEXUS_RecordPidChannelSettings pidSettings;
    NEXUS_RecpumpOpenSettings recpumpOpenSettings;
    NEXUS_FileRecordHandle recordfile;
    unsigned int    videoPID, audioPID;
    NEXUS_KeySlotHandle videoKeyHandle = NULL;
    NEXUS_KeySlotHandle audioKeyHandle = NULL;
    NEXUS_PidChannelStatus pidStatus;
    NEXUS_SecurityKeySlotSettings keySlotSettings;
    NEXUS_SecurityInvalidateKeySettings invSettings;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;
    NEXUS_FilePlayHandle playfile;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_AudioDecoderHandle audioDecoder;
    const char     *recfname = "/mnt/nfs/video/aesecb_clearKey_576i50_2min.mpg";
    const char     *recindex = "/mnt/nfs/video/aesecb_clearKey_576i50_2min.idx";
    const char     *playfname = "/mnt/nfs/video/576i50_2min.ts";

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
    unsigned char   VidEvenCpsControlWord[] =
        { 0x6e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7, 0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73 };
    unsigned char   VidOddCpsControlWord[] =
        { 0x6e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7, 0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73 };
    unsigned char   AudEvenCpsControlWord[] =
        { 0x6e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7, 0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73 };
    unsigned char   AudOddCpsControlWord[] =
        { 0x6e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7, 0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73 };

    NEXUS_Platform_GetDefaultSettings (&platformSettings);
    platformSettings.openFrontend = false;

    if (NEXUS_Platform_Init (&platformSettings))
    {
        return -1;
    }

    playfile = NEXUS_FilePlay_OpenPosix (playfname, NULL);
    if (!playfile)
    {
        fprintf (stderr, "can't open file:%s\n", playfname);
        return -1;
    }

    /* set up the source */
    if (playpump_setup
        (&videoDecoder, &audioDecoder, &videoPidChannel, &audioPidChannel, &stcChannel, &playback, &playpump))
    {
        return -1;
    }

    NEXUS_Recpump_GetDefaultOpenSettings (&recpumpOpenSettings);

    /* set threshold to 30%. with band hold enabled, it is a bandhold threshold. */
    recpumpOpenSettings.data.dataReadyThreshold = recpumpOpenSettings.data.bufferSize * 3 / 10;
    recpumpOpenSettings.index.dataReadyThreshold = recpumpOpenSettings.index.bufferSize * 3 / 10;
    recpump = NEXUS_Recpump_Open (NEXUS_ANY_ID, &recpumpOpenSettings);
    record = NEXUS_Record_Create ();
    NEXUS_Record_GetSettings (record, &recordCfg);
    recordCfg.recpump = recpump;
    /* enable bandhold. required for record from playback. */
    recordCfg.recpumpSettings.bandHold = NEXUS_RecpumpFlowControl_eEnable;
    NEXUS_Record_SetSettings (record, &recordCfg);

    recordfile = NEXUS_FileRecord_OpenPosix (recfname, recindex);
    if (!recordfile)
    {
        fprintf (stderr, "can't open file:%s %s\n", recfname, recindex);
        return -1;
    }

    NEXUS_Record_GetDefaultPidChannelSettings (&pidSettings);
    pidSettings.recpumpSettings.pidType = NEXUS_PidType_eVideo;
    pidSettings.recpumpSettings.pidTypeSettings.video.index = true;
    pidSettings.recpumpSettings.pidTypeSettings.video.codec = NEXUS_VideoCodec_eMpeg2;
    NEXUS_Record_AddPidChannel (record, videoPidChannel, &pidSettings);

    /***************************************************************************************
		Config CA descrambler 
	***************************************************************************************/
#if defined(ENABLE_CRYPTO)
    NEXUS_Security_GetDefaultKeySlotSettings (&keySlotSettings);
    keySlotSettings.keySlotEngine = NEXUS_SecurityEngine_eCaCp; /* eCa? */

    /* Allocate AV keyslots */
    videoKeyHandle = NEXUS_Security_AllocateKeySlot (&keySlotSettings);
    if (!videoKeyHandle)
    {
        printf ("\nAllocate CACP video keyslot failed \n");
        return 1;
    }
    audioKeyHandle = NEXUS_Security_AllocateKeySlot (&keySlotSettings);
    if (!audioKeyHandle)
    {
        printf ("\nAllocate CACP audio keyslot failed \n");
        return 1;
    }

    invSettings.keyDestEntryType = NEXUS_SecurityKeyType_eOddAndEven;
    invSettings.invalidateKeyType = NEXUS_SecurityInvalidateKeyFlag_eDestKeyOnly;
    invSettings.keyDestBlckType = NEXUS_SecurityAlgorithmConfigDestination_eCa;
    invSettings.virtualKeyLadderID = NEXUS_SecurityVirtualKeyladderID_eVKLDummy;
    NEXUS_Security_InvalidateKey (videoKeyHandle, &invSettings);
    NEXUS_Security_InvalidateKey (audioKeyHandle, &invSettings);

    invSettings.keyDestEntryType = NEXUS_SecurityKeyType_eOddAndEven;
    invSettings.invalidateKeyType = NEXUS_SecurityInvalidateKeyFlag_eDestKeyOnly;
    invSettings.keyDestBlckType = NEXUS_SecurityAlgorithmConfigDestination_eCps;
    invSettings.virtualKeyLadderID = NEXUS_SecurityVirtualKeyladderID_eVKLDummy;
    NEXUS_Security_InvalidateKey (videoKeyHandle, &invSettings);
    NEXUS_Security_InvalidateKey (audioKeyHandle, &invSettings);

    invSettings.keyDestEntryType = NEXUS_SecurityKeyType_eClear;
    invSettings.invalidateKeyType = NEXUS_SecurityInvalidateKeyFlag_eDestKeyOnly;
    invSettings.keyDestBlckType = NEXUS_SecurityAlgorithmConfigDestination_eCps;
    invSettings.virtualKeyLadderID = NEXUS_SecurityVirtualKeyladderID_eVKLDummy;
    NEXUS_Security_InvalidateKey (videoKeyHandle, &invSettings);
    NEXUS_Security_InvalidateKey (audioKeyHandle, &invSettings);

    invSettings.keyDestEntryType = NEXUS_SecurityKeyType_eOddAndEven;
    invSettings.invalidateKeyType = NEXUS_SecurityInvalidateKeyFlag_eDestKeyOnly;
    invSettings.keyDestBlckType = NEXUS_SecurityAlgorithmConfigDestination_eCpd;
    invSettings.virtualKeyLadderID = NEXUS_SecurityVirtualKeyladderID_eVKLDummy;
    NEXUS_Security_InvalidateKey (videoKeyHandle, &invSettings);
    NEXUS_Security_InvalidateKey (audioKeyHandle, &invSettings);

/* Just testing CPS clear stream input encryption even though opening CaCp engine */
#ifndef CPS_ONLY
    /* Config AV algorithms */
    if (ConfigureKeySlotFor3DesCA (videoKeyHandle, VidEvenControlWord, VidOddControlWord) != 0)
    {
        printf ("\nConfig video CA Algorithm failed for video keyslot 1 \n");
        return 1;
    }

    if (ConfigureKeySlotFor3DesCA (audioKeyHandle, AudEvenControlWord, AudOddControlWord) != 0)
    {
        printf ("\nConfig audio CA Algorithm failed for audio keyslot 1 \n");
        return 1;
    }

#endif
    printf ("\n CPS scrambler\n");

    /***************************************************************************************
		Config CPS (CP scrambler) 
	***************************************************************************************/

    if (ConfigureKeySlotForAesCPS (videoKeyHandle,
                                   VidEvenCpsControlWord,
                                   VidOddCpsControlWord, NEXUS_SecurityAlgorithmScPolarity_eEven) != 0)
    {
        printf ("\nConfig video CPS Algorithm failed for video keyslot 1 \n");
        return 1;
    }

    if (ConfigureKeySlotForAesCPS (videoKeyHandle,
                                   VidEvenCpsControlWord,
                                   VidOddCpsControlWord, NEXUS_SecurityAlgorithmScPolarity_eOdd) != 0)
    {
        printf ("\nConfig video CPS Algorithm failed for video keyslot 1 \n");
        return 1;
    }

    printf ("\nSecurity Config even CW OK\n");
    if (ConfigureKeySlotForAesCPS (audioKeyHandle,
                                   AudEvenCpsControlWord,
                                   AudOddCpsControlWord, NEXUS_SecurityAlgorithmScPolarity_eClear) != 0)
    {
        printf ("\nConfig audio CPS Algorithm failed for audio keyslot 1 \n");
        return 1;
    }

    printf ("\nSecurity Config odd CW OK\n");

    /* Start record */
    printf ("\n\n\n\n\n\nrecording \n\n\n");
    NEXUS_Record_Start (record, recordfile);

    /* Add video PID channel to keyslot */
    NEXUS_PidChannel_GetStatus (videoPidChannel, &pidStatus);
    videoPID = pidStatus.pidChannelIndex;
    NEXUS_Security_AddPidChannelToKeySlot (videoKeyHandle, videoPID);
    printf ("\nSecurity Add video Pid channel OK\n");
#if 1
    NEXUS_PidChannel_GetStatus (audioPidChannel, &pidStatus);
    audioPID = pidStatus.pidChannelIndex;
    NEXUS_Security_AddPidChannelToKeySlot (audioKeyHandle, audioPID);
#endif

    printf ("\nSecurity Config OK\n");

#endif

    /* Start playback */
    NEXUS_Playback_Start (playback, playfile, NULL);

    /* Wait for the two seconds */
    sleep (2);
    printf ("Record has been created with files stream.mpg and stream.nav. Use playback to inspect on HDMI.\n");

    /* Tear down the devices chain */
    NEXUS_Record_Stop (record);
    NEXUS_Record_RemoveAllPidChannels (record);

    NEXUS_FileRecord_Close (recordfile);
    NEXUS_Record_Destroy (record);
    NEXUS_Recpump_Close (recpump);

    NEXUS_Playback_Stop (playback);
    NEXUS_Playback_ClosePidChannel (playback, videoPidChannel);
    NEXUS_FilePlay_Close (playfile);
    NEXUS_Playback_Destroy (playback);
    NEXUS_Playpump_Close (playpump);

    NEXUS_VideoDecoder_Close (videoDecoder);
    NEXUS_AudioDecoder_Close (audioDecoder);
    NEXUS_StcChannel_Close (stcChannel);
#if 1
    NEXUS_Security_RemovePidChannelFromKeySlot (videoKeyHandle, videoPID);
    NEXUS_Security_RemovePidChannelFromKeySlot (audioKeyHandle, audioPID);
#endif
    NEXUS_Security_FreeKeySlot (videoKeyHandle);
    NEXUS_Security_FreeKeySlot (audioKeyHandle);
    printf ("finish free keyslot\n");

    NEXUS_Platform_Uninit ();

    /* loops forever */
#endif
#endif

    return 1;
}

#if (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 4)

    /* Some aspects of this configuration are specific to 40nm HSM */
#ifndef CPS_ONLY
static NEXUS_Error ConfigureKeySlotFor3DesCA (NEXUS_KeySlotHandle keyHandle,
                                              unsigned char *pKeyEven, unsigned char *pKeyOdd)
{
    NEXUS_SecurityAlgorithmSettings AlgConfig;
    NEXUS_SecurityClearKey key;

    NEXUS_Security_GetDefaultAlgorithmSettings (&AlgConfig);
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

    if (NEXUS_Security_ConfigAlgorithm (keyHandle, &AlgConfig) != 0)
    {
        printf ("\nConfig video CPS Algorithm failed \n");
        return 1;
    }

    /* Load AV keys */
    NEXUS_Security_GetDefaultClearKey (&key);
    key.keySize = 16;
    key.dest = NEXUS_SecurityAlgorithmConfigDestination_eCa;
    key.keyIVType = NEXUS_SecurityKeyIVType_eNoIV;

    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    memcpy (key.keyData, pKeyOdd, key.keySize);
    if (NEXUS_Security_LoadClearKey (keyHandle, &key) != 0)
    {
        printf ("\nLoad CA ODD key failed \n");
        return 1;
    }

    key.keyEntryType = NEXUS_SecurityKeyType_eEven;
    memcpy (key.keyData, pKeyEven, key.keySize);
    if (NEXUS_Security_LoadClearKey (keyHandle, &key) != 0)
    {
        printf ("\nLoad CA EVEN key failed \n");
        return 1;
    }

    return 0;

}
#endif

static NEXUS_Error ConfigureKeySlotForAesCPS (NEXUS_KeySlotHandle keyHandle,
                                              unsigned char *pKeyEven,
                                              unsigned char *pKeyOdd, NEXUS_SecurityAlgorithmScPolarity polarity)
{
    NEXUS_SecurityAlgorithmSettings AlgConfig;
    NEXUS_SecurityClearKey key;

    NEXUS_Security_GetDefaultAlgorithmSettings (&AlgConfig);
    AlgConfig.algorithm = NEXUS_SecurityAlgorithm_eAes128;
    AlgConfig.algorithmVar = NEXUS_SecurityAlgorithmVariant_eEcb;
    AlgConfig.terminationMode = NEXUS_SecurityTerminationMode_eCipherStealing;
    AlgConfig.ivMode = NEXUS_SecurityIVMode_eRegular;
    AlgConfig.solitarySelect = NEXUS_SecuritySolitarySelect_eClear;
    AlgConfig.caVendorID = 0x1234;
    AlgConfig.askmModuleID = NEXUS_SecurityAskmModuleID_eModuleID_4;
    AlgConfig.otpId = NEXUS_SecurityOtpId_eOtpVal;
    AlgConfig.key2Select = NEXUS_SecurityKey2Select_eReserved1;
    AlgConfig.dest = NEXUS_SecurityAlgorithmConfigDestination_eCps;
    AlgConfig.bRestrictEnable = false;
    AlgConfig.bEncryptBeforeRave = false;
    AlgConfig.operation = NEXUS_SecurityOperation_eEncrypt;
    AlgConfig.modifyScValue[NEXUS_SecurityPacketType_eRestricted] = true;
    AlgConfig.modifyScValue[NEXUS_SecurityPacketType_eGlobal] = true;
    AlgConfig.scValue[NEXUS_SecurityPacketType_eRestricted] = polarity;
    AlgConfig.scValue[NEXUS_SecurityPacketType_eGlobal] = polarity;

    if (NEXUS_Security_ConfigAlgorithm (keyHandle, &AlgConfig) != 0)
    {
        printf ("\nConfig video CPS Algorithm failed \n");
        return 1;
    }

    AlgConfig.keyDestEntryType = NEXUS_SecurityKeyType_eClear;
    if (NEXUS_Security_ConfigAlgorithm (keyHandle, &AlgConfig) != 0)
    {
        printf ("\nConfig video CPS Algorithm failed \n");
        return 1;
    }

    /* Load AV keys */
    NEXUS_Security_GetDefaultClearKey (&key);
    key.keySize = 16;
    key.dest = NEXUS_SecurityAlgorithmConfigDestination_eCps;
    key.keyIVType = NEXUS_SecurityKeyIVType_eNoIV;

    if (polarity == NEXUS_SecurityAlgorithmScPolarity_eEven)
    {
        key.keyEntryType = NEXUS_SecurityKeyType_eEven;
        memcpy (key.keyData, pKeyEven, key.keySize);
    }
    else                        /* clear and odd are made as odd. */
    {
        key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
        memcpy (key.keyData, pKeyOdd, key.keySize);
    }

    if (NEXUS_Security_LoadClearKey (keyHandle, &key) != 0)
    {
        printf ("\nLoad CPS ODD key failed \n");
        return 1;
    }
    return 0;

}

static BERR_Code playpump_setup (NEXUS_VideoDecoderHandle * videoDecoder,
                                 NEXUS_AudioDecoderHandle * audioDecoder,
                                 NEXUS_PidChannelHandle * pVideoPidChannel,
                                 NEXUS_PidChannelHandle * pAudioPidChannel,
                                 NEXUS_StcChannelHandle * pStcChannel,
                                 NEXUS_PlaybackHandle * pPlayback, NEXUS_PlaypumpHandle * pPlaypump)
{
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;

    NEXUS_StcChannelSettings stcSettings;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;

    /* set up the source */
    playpump = NEXUS_Playpump_Open (NEXUS_ANY_ID, NULL);
    assert (playpump);
    playback = NEXUS_Playback_Create ();
    assert (playback);
    /* Open the StcChannel to do lipsync with the PCR */
    NEXUS_StcChannel_GetDefaultSettings (0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open (0, &stcSettings);
    *pStcChannel = stcChannel;

    NEXUS_Playback_GetSettings (playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    playbackSettings.playpumpSettings.transportType = TRANSPORT_TYPE;

    playbackSettings.stcChannel = stcChannel;
    NEXUS_Playback_SetSettings (playback, &playbackSettings);

    *videoDecoder = NEXUS_VideoDecoder_Open (0, NULL);  /* take default capabilities */

    /* Open the audio and video pid channels */
    NEXUS_Playback_GetDefaultPidChannelSettings (&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = VIDEO_CODEC;
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = *videoDecoder;
    /* configure the video pid for indexing */
    videoPidChannel = NEXUS_Playback_OpenPidChannel (playback, VIDEO_PID, &playbackPidSettings);

    *audioDecoder = NEXUS_AudioDecoder_Open (0, NULL);

    NEXUS_Playback_GetDefaultPidChannelSettings (&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.primary = *audioDecoder;
    audioPidChannel = NEXUS_Playback_OpenPidChannel (playback, AUDIO_PID, &playbackPidSettings);

    *pVideoPidChannel = videoPidChannel;
    *pAudioPidChannel = audioPidChannel;

/*    NEXUS_Playback_Start (playback, playfile, NULL); */

    *pPlayback = playback;
    *pPlaypump = playpump;
    return 0;
}
#endif
#else /* NEXUS_HAS_SECURITY && NEXUS_HAS_FRONTEND */
#include <stdio.h>
int main (void)
{
    printf ("This application is not supported on this platform!\n");
    return -1;
}
#endif
