/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
/* Example CA descramble playback for with clearkey for Even and Odd  on 7425 reference */
/* This example makes a pair with  recording using "cps_aesecb_clearkey_record_different_keyvalue.c " */

#if NEXUS_HAS_SECURITY && (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 4) && NEXUS_HAS_VIDEO_DECODER
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
#if NEXUS_HAS_PLAYBACK
#include "nexus_playback.h"
#include "nexus_file.h"
#endif
#if NEXUS_DTV_PLATFORM
#include "nexus_platform_boardcfg.h"
#endif
#include "nexus_security.h"
#include "string.h"

#include <stdio.h>
#include <assert.h>
#include "bstd.h"
#include "bkni.h"

#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eMpeg
#define VIDEO_PID (2316 )
#define AUDIO_PID (2317 )

    /* Some aspects of this configuration are specific to 40nm HSM */

static NEXUS_Error ConfigureKeySlotForAesCA (NEXUS_KeySlotHandle keyHandle,
                                             unsigned char *pkeyEven, unsigned char *pKeyOdd)
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
    AlgConfig.askmModuleID = NEXUS_SecurityAskmModuleID_eModuleID_8;
    AlgConfig.otpId = NEXUS_SecurityOtpId_eOtpVal;
    AlgConfig.key2Select = NEXUS_SecurityKey2Select_eReserved1;
    AlgConfig.modifyScValue[NEXUS_SecurityPacketType_eRestricted] = false;
    AlgConfig.modifyScValue[NEXUS_SecurityPacketType_eGlobal] = false;
    AlgConfig.operation = NEXUS_SecurityOperation_eDecrypt;
    AlgConfig.dest = NEXUS_SecurityAlgorithmConfigDestination_eCa;
    if (NEXUS_Security_ConfigAlgorithm (keyHandle, &AlgConfig) != 0)
    {
        printf ("\nConfig video CPS Algorithm failed \n");
        return 1;
    }

    /* Load AV keys */
    NEXUS_Security_GetDefaultClearKey (&key);
    key.keySize = 16;
    key.keyEntryType = NEXUS_SecurityKeyType_eEven;
    key.dest = NEXUS_SecurityAlgorithmConfigDestination_eCa;
    key.keyIVType = NEXUS_SecurityKeyIVType_eNoIV;

    memcpy (key.keyData, pkeyEven, key.keySize);        /* Even Key */
    if (NEXUS_Security_LoadClearKey (keyHandle, &key) != 0)
    {
        printf ("\nLoad CA EVEN key failed \n");
        return 1;
    }

    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    memcpy (key.keyData, pKeyOdd, key.keySize); /* Odd Key */
    if (NEXUS_Security_LoadClearKey (keyHandle, &key) != 0)
    {
        printf ("\nLoad CA ODD key failed \n");
        return 1;
    }

    return 0;

}

int main (void)
{
#if NEXUS_HAS_PLAYBACK
    int             command;
    NEXUS_PlaybackTrickModeSettings trick;
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
    NEXUS_SecurityInvalidateKeySettings invSettings;
    const char     *fname = "/mnt/nfs/video/aesecb_clearKey_576i50_2min.mpg";
    const char     *index = "/mnt/nfs/video/aesecb_clearKey_576i50_2min.idx";

    unsigned char   VidEvenControlWord[] =
        { 0x2e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7, 0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73 };
    unsigned char   VidOddControlWord[] =
        { 0x6e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7, 0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73 };
    unsigned char   AudEvenControlWord[] =
        { 0x8e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7, 0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73 };
    unsigned char   AudOddControlWord[] =
        { 0x0e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7, 0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73 };

    NEXUS_KeySlotHandle videoKeyHandle = NULL;
    NEXUS_KeySlotHandle audioKeyHandle = NULL;
    NEXUS_SecurityKeySlotSettings keySlotSettings;

    NEXUS_Platform_GetDefaultSettings (&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init (&platformSettings);
    NEXUS_Platform_GetConfiguration (&platformConfig);

    playpump = NEXUS_Playpump_Open (0, NULL);
    assert (playpump);
    playback = NEXUS_Playback_Create ();
    assert (playback);

    file = NEXUS_FilePlay_OpenPosix (fname, index /*NULL*/);
    if (!file)
    {
        fprintf (stderr, "can't open file:%s\n", fname);
        return -1;
    }

    NEXUS_StcChannel_GetDefaultSettings (0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open (0, &stcSettings);

    NEXUS_Playback_GetSettings (playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    /* set a stream format, it could be any audio video transport type or file format, i.e NEXUS_TransportType_eMp4, NEXUS_TransportType_eAvi ... */
    playbackSettings.playpumpSettings.transportType = NEXUS_TransportType_eTs;
    playbackSettings.stcChannel = stcChannel;
    NEXUS_Playback_SetSettings (playback, &playbackSettings);

    /* Bring up audio decoders and outputs */
    audioDecoder = NEXUS_AudioDecoder_Open (0, NULL);

    NEXUS_AudioOutput_AddInput (NEXUS_HdmiOutput_GetAudioConnector (platformConfig.outputs.hdmi[0]),
                                NEXUS_AudioDecoder_GetConnector (audioDecoder,
                                                                 NEXUS_AudioDecoderConnectorType_eStereo));

    /* Bring up video display and outputs */
    display = NEXUS_Display_Open (0, NULL);
#if NEXUS_DTV_PLATFORM
    NEXUS_Display_AddOutput (display, NEXUS_PanelOutput_GetConnector (platformConfig.outputs.panel[0]));
    NEXUS_BoardCfg_ConfigurePanel (true, true, true);
#else
#if NEXUS_NUM_COMPONENT_OUTPUTS
    if (platformConfig.outputs.component[0])
    {
        NEXUS_Display_AddOutput (display, NEXUS_ComponentOutput_GetConnector (platformConfig.outputs.component[0]));
    }
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput (display, NEXUS_CompositeOutput_GetConnector (platformConfig.outputs.composite[0]));
#endif
#endif

    NEXUS_Display_AddOutput (display, NEXUS_HdmiOutput_GetVideoConnector (platformConfig.outputs.hdmi[0]));

    window = NEXUS_VideoWindow_Open (display, 0);

    /* bring up decoder and connect to display */
    videoDecoder = NEXUS_VideoDecoder_Open (0, NULL);   /* take default capabilities */
    NEXUS_VideoWindow_AddInput (window, NEXUS_VideoDecoder_GetConnector (videoDecoder));

    /* Open the audio and video pid channels */
    NEXUS_Playback_GetDefaultPidChannelSettings (&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = NEXUS_VideoCodec_eMpeg2;  /* must be told codec for correct handling */
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
    videoPidChannel = NEXUS_Playback_OpenPidChannel (playback, VIDEO_PID, &playbackPidSettings);

    NEXUS_Playback_GetDefaultPidChannelSettings (&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.primary = audioDecoder;
    audioPidChannel = NEXUS_Playback_OpenPidChannel (playback, AUDIO_PID, &playbackPidSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up
     * the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings (&videoProgram);
    videoProgram.codec = NEXUS_VideoCodec_eMpeg2;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
    NEXUS_AudioDecoder_GetDefaultStartSettings (&audioProgram);
    audioProgram.codec = NEXUS_AudioCodec_eAc3;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;

    /***************************************************************************************
        Config CA descrambler 
    ***************************************************************************************/

    NEXUS_Security_GetDefaultKeySlotSettings (&keySlotSettings);
    keySlotSettings.keySlotEngine = NEXUS_SecurityEngine_eCa;

    /* Allocate AV keyslots */
    videoKeyHandle = NEXUS_Security_AllocateKeySlot (&keySlotSettings);
    if (!videoKeyHandle)
    {
        printf ("\nAllocate CA video keyslot failed \n");
        return 1;
    }
    audioKeyHandle = NEXUS_Security_AllocateKeySlot (&keySlotSettings);
    if (!audioKeyHandle)
    {
        printf ("\nAllocate CA audio keyslot failed \n");
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

    invSettings.keyDestEntryType = NEXUS_SecurityKeyType_eOddAndEven;
    invSettings.invalidateKeyType = NEXUS_SecurityInvalidateKeyFlag_eDestKeyOnly;
    invSettings.keyDestBlckType = NEXUS_SecurityAlgorithmConfigDestination_eCpd;
    invSettings.virtualKeyLadderID = NEXUS_SecurityVirtualKeyladderID_eVKLDummy;
    NEXUS_Security_InvalidateKey (videoKeyHandle, &invSettings);
    NEXUS_Security_InvalidateKey (audioKeyHandle, &invSettings);

    /* Config AV algorithms */
    if (ConfigureKeySlotForAesCA (videoKeyHandle, VidEvenControlWord, VidOddControlWord) != 0)
    {
        printf ("\nConfig video CA Algorithm failed for video keyslot\n");
        return 1;
    }

    if (ConfigureKeySlotForAesCA (audioKeyHandle, AudEvenControlWord, AudOddControlWord) != 0)
    {
        printf ("\nConfig audio CA Algorithm failed for audio keyslot\n");
        return 1;
    }

    /* Add video PID channel to keyslot */
    NEXUS_KeySlot_AddPidChannel ( videoKeyHandle, videoPidChannel );

    /* Add video PID channel to keyslot */
    NEXUS_KeySlot_AddPidChannel ( audioKeyHandle, audioPidChannel );

    printf ("\nSecurity Config OK\n");

    /* Start decoders */
    NEXUS_VideoDecoder_Start (videoDecoder, &videoProgram);
    NEXUS_AudioDecoder_Start (audioDecoder, &audioProgram);

    /* Start playback */
    NEXUS_Playback_Start (playback, file, NULL);

    fflush (stdin);
    do
    {
        printf ("[1] 1x Rewind\n");
        printf ("[2] 2x Fast Forward\n");
        printf ("[3] Pause\n");
        printf ("[4] Resume\n");
        printf ("[0] quit\n ");

        scanf ("%d", &command);
        switch (command)
        {
            case 1:
                printf ("\n1x Rewind\n");
                NEXUS_Playback_GetDefaultTrickModeSettings (&trick);
                trick.rate = -2 * NEXUS_NORMAL_PLAY_SPEED;
                NEXUS_Playback_TrickMode (playback, &trick);
                break;
            case 2:
                printf ("\n2x FF\n");
                NEXUS_Playback_GetDefaultTrickModeSettings (&trick);
                trick.rate = 2 * NEXUS_NORMAL_PLAY_SPEED;
                NEXUS_Playback_TrickMode (playback, &trick);
                break;
            case 3:
                printf ("\nPause\n");
                NEXUS_Playback_Pause (playback);
                break;
            case 4:
                printf ("\n Resume\n");
                NEXUS_Playback_Play (playback);
                break;

            case 99:
                printf ("Quit \n");
                break;
        }
    }
    while (0 != command);

    /* Playback state machine is driven from inside Nexus. */
    printf ("Press ENTER to quit\n");
    getchar ();

    /* Bring down system */
    NEXUS_VideoDecoder_Stop (videoDecoder);
    NEXUS_AudioDecoder_Stop (audioDecoder);
    NEXUS_Playback_Stop (playback);
    NEXUS_FilePlay_Close (file);
    NEXUS_Playback_Destroy (playback);
    NEXUS_Playpump_Close (playpump);
    NEXUS_VideoInput_Shutdown (NEXUS_VideoDecoder_GetConnector (videoDecoder));
    NEXUS_VideoDecoder_Close (videoDecoder);
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_RemoveAllInputs (NEXUS_AudioDac_GetConnector (platformConfig.outputs.audioDacs[0]));
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutput_RemoveAllInputs (NEXUS_SpdifOutput_GetConnector (platformConfig.outputs.spdif[0]));
#endif

    NEXUS_AudioInput_Shutdown (NEXUS_AudioDecoder_GetConnector (audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    NEXUS_AudioDecoder_Close (audioDecoder);
    NEXUS_Display_Close (display);
    NEXUS_StcChannel_Close (stcChannel);
    NEXUS_Platform_Uninit ();

#endif
    return 0;
}


#else /* NEXUS_HAS_SECURITY && (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 4) && NEXUS_HAS_VIDEO_DECODER */
#include <stdio.h>
int main ( void )
{
    printf ( "This application is not supported on this platform!\n" );
    return -1;
}
#endif
