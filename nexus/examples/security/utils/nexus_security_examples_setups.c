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

#if NEXUS_HAS_SECURITY && NEXUS_HAS_PLAYBACK && NEXUS_HAS_RECORD && (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 2) && NEXUS_HAS_VIDEO_DECODER

#include <assert.h>
#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nexus_security_examples_setups.h"

/* The following functions setup the miscellaneous configurations for the security tests. */
/* They are not the focus of the security examples. It is better to not look at them, and focus on security configs. */

NEXUS_Error SecurityExampleInitPlatform ( NEXUS_ExampleSecuritySettings * pSettings )
{
    NEXUS_PlatformSettings platformSettings;

    if ( !pSettings )
    {
        return NEXUS_NOT_INITIALIZED;
    }

    BKNI_Memset ( ( void * ) pSettings, 0, sizeof ( NEXUS_ExampleSecuritySettings ) );

    /* Nexus platform initilisation. */
    NEXUS_Platform_GetDefaultSettings ( &platformSettings );
    platformSettings.openFrontend = false;

    if ( NEXUS_Platform_Init ( &platformSettings ) )
    {
        return NEXUS_NOT_INITIALIZED;
    }

    NEXUS_Platform_GetConfiguration ( &pSettings->platformConfig );

    return NEXUS_SUCCESS;
}

NEXUS_Error SecurityExampleShutdown ( NEXUS_ExampleSecuritySettings * pSettings )
{
    if ( pSettings->window )
        NEXUS_VideoWindow_Close ( pSettings->window );

    if ( pSettings->display )
        NEXUS_Display_Close ( pSettings->display );

    if ( pSettings->record )
    {
        NEXUS_Record_Stop ( pSettings->record );
        NEXUS_Record_RemoveAllPidChannels ( pSettings->record );
        NEXUS_Record_Destroy ( pSettings->record );
    }

    if ( pSettings->recpump )
        NEXUS_Recpump_Close ( pSettings->recpump );

    if ( pSettings->recordfile )
        NEXUS_FileRecord_Close ( pSettings->recordfile );

    if ( pSettings->playback )
    {
        if ( pSettings->videoPidChannel )
            NEXUS_Playback_ClosePidChannel ( pSettings->playback, pSettings->videoPidChannel );
        if ( pSettings->audioPidChannel )
            NEXUS_Playback_ClosePidChannel ( pSettings->playback, pSettings->audioPidChannel );
    }

    if ( pSettings->playback )
    {
        NEXUS_Playback_Stop ( pSettings->playback );
        NEXUS_Playback_Destroy ( pSettings->playback );
    }

    if ( pSettings->playpump )
    {
        NEXUS_Playpump_Stop ( pSettings->playpump );
        NEXUS_Playpump_Close ( pSettings->playpump );
    }

    if ( pSettings->videoDecoder )
    {
        NEXUS_VideoDecoder_Stop ( pSettings->videoDecoder );
        NEXUS_VideoDecoder_Close ( pSettings->videoDecoder );
    }

    if ( pSettings->audioDecoder )
    {
        NEXUS_AudioDecoder_Stop ( pSettings->audioDecoder );
        NEXUS_AudioDecoder_Close ( pSettings->audioDecoder );
    }

    if ( pSettings->stcChannel )
        NEXUS_StcChannel_Close ( pSettings->stcChannel );

    if ( pSettings->playfile )
        NEXUS_FilePlay_Close ( pSettings->playfile );

    /* Security free handlers: keyslots, vkl etc. */
    if ( pSettings->videoKeyHandle )
    {
        NEXUS_Security_RemovePidChannelFromKeySlot ( pSettings->videoKeyHandle, pSettings->videoPID );
        NEXUS_Security_FreeKeySlot ( pSettings->videoKeyHandle );
    }

    if ( pSettings->audioKeyHandle )
    {
        NEXUS_Security_RemovePidChannelFromKeySlot ( pSettings->audioKeyHandle, pSettings->audioPID );
        NEXUS_Security_FreeKeySlot ( pSettings->audioKeyHandle );
    }

	if ( pSettings->vklHandle)
	{
        NEXUS_Security_FreeVKL ( pSettings->vklHandle );
	}
    printf ( "Finish freeing display, decoders and keyslot resources.\n" );

    NEXUS_Platform_Uninit (  );

    BKNI_Memset ( ( void * ) pSettings, 0, sizeof ( NEXUS_ExampleSecuritySettings ) );

    return NEXUS_SUCCESS;
}

NEXUS_Error SecurityExampleSetupPlayback ( NEXUS_ExampleSecuritySettings * pSettings )
{
    NEXUS_PlaybackSettings playbackSettings;
    pSettings->playpump = NEXUS_Playpump_Open ( NEXUS_ANY_ID, NULL );
    BDBG_ASSERT ( pSettings->playpump );
    pSettings->playback = NEXUS_Playback_Create (  );
    BDBG_ASSERT ( pSettings->playback );

    printf ( "\nPlayback stream %s.\n", pSettings->playfname );
    pSettings->playfile = NEXUS_FilePlay_OpenPosix ( pSettings->playfname, NULL );
    if ( !pSettings->playfile )
    {
        fprintf ( stderr, "can't open file:%s\n", pSettings->playfname );
        return NEXUS_INVALID_PARAMETER;
    }

    NEXUS_Playback_GetSettings ( pSettings->playback, &playbackSettings );
    playbackSettings.playpump = pSettings->playpump;
    /* set a stream format, it could be any audio video transport type or file format, i.e NEXUS_TransportType_eMp4, NEXUS_TransportType_eAvi ... */
    playbackSettings.playpumpSettings.transportType = TRANSPORT_TYPE;
    playbackSettings.stcChannel = pSettings->stcChannel;
    NEXUS_Playback_SetSettings ( pSettings->playback, &playbackSettings );

    return NEXUS_SUCCESS;
}

NEXUS_Error SecurityExampleSetupPlaybackPidChannels ( NEXUS_ExampleSecuritySettings * pSettings )
{
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;

    /* Open the audio and video pid channels */
    NEXUS_Playback_GetDefaultPidChannelSettings ( &playbackPidSettings );
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = VIDEO_CODEC;
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = pSettings->videoDecoder;
    /* configure the video pid for indexing */
    pSettings->videoPidChannel = NEXUS_Playback_OpenPidChannel ( pSettings->playback, VIDEO_PID, &playbackPidSettings );

    NEXUS_Playback_GetDefaultPidChannelSettings ( &playbackPidSettings );
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.primary = pSettings->audioDecoder;
    pSettings->audioPidChannel = NEXUS_Playback_OpenPidChannel ( pSettings->playback, AUDIO_PID, &playbackPidSettings );

    printf ( "\nSetup Playback PidChannel for video PID 0x%x, audio PID 0x%x.\n", VIDEO_PID, AUDIO_PID );
    return 0;
}

NEXUS_Error SecurityExampleSetupRecord4Encrpytion ( NEXUS_ExampleSecuritySettings * pSettings )
{
    NEXUS_RecordPidChannelSettings pidSettings;
    NEXUS_RecordSettings recordCfg;
    NEXUS_RecpumpHandle recpump;
    NEXUS_RecpumpOpenSettings recpumpOpenSettings;

    NEXUS_Recpump_GetDefaultOpenSettings ( &recpumpOpenSettings );
    /* set threshold to 30%. with band hold enabled, it is a bandhold threshold. */
    recpumpOpenSettings.data.dataReadyThreshold = recpumpOpenSettings.data.bufferSize * 3 / 10;
    recpumpOpenSettings.index.dataReadyThreshold = recpumpOpenSettings.index.bufferSize * 3 / 10;
    recpump = NEXUS_Recpump_Open ( NEXUS_ANY_ID, &recpumpOpenSettings );
    pSettings->record = NEXUS_Record_Create (  );
    NEXUS_Record_GetSettings ( pSettings->record, &recordCfg );
    recordCfg.recpump = recpump;
    /* enable bandhold. required for record from playback. */
    recordCfg.recpumpSettings.bandHold = NEXUS_RecpumpFlowControl_eEnable;
    NEXUS_Record_SetSettings ( pSettings->record, &recordCfg );

    pSettings->recordfile = NEXUS_FileRecord_OpenPosix ( pSettings->recfname, NULL );
    if ( !pSettings->recordfile )
    {
        fprintf ( stderr, "can't open record file:%s.\n", pSettings->recfname);
        return -1;
    }

    NEXUS_Record_GetDefaultPidChannelSettings ( &pidSettings );
    pidSettings.recpumpSettings.pidType = NEXUS_PidType_eVideo;
    pidSettings.recpumpSettings.pidTypeSettings.video.index = true;
    pidSettings.recpumpSettings.pidTypeSettings.video.codec = VIDEO_CODEC;
    NEXUS_Record_AddPidChannel ( pSettings->record, pSettings->videoPidChannel, &pidSettings );

}

NEXUS_Error SecurityExampleStartPlayback ( NEXUS_ExampleSecuritySettings * pSettings )
{
    printf ( "\n\nStart playback. \n\n\n" );
    NEXUS_Playback_Start ( pSettings->playback, pSettings->playfile, NULL );
}

NEXUS_Error SecurityExampleStartRecord ( NEXUS_ExampleSecuritySettings * pSettings )
{
    printf ( "\n\n\nStart recording. \n\n\n" );
    NEXUS_Record_Start ( pSettings->record, pSettings->recordfile );
}

NEXUS_Error SecurityExampleSetupDecoders ( NEXUS_ExampleSecuritySettings * pSettings )
{
    pSettings->audioDecoder = NEXUS_AudioDecoder_Open ( 0, NULL );
    pSettings->videoDecoder = NEXUS_VideoDecoder_Open ( 0, NULL );      /* take default capabilities */
}

NEXUS_Error SecurityExampleStartDecoders ( NEXUS_ExampleSecuritySettings * pSettings )
{
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderStartSettings audioProgram;

    NEXUS_StcChannel_GetDefaultSettings ( 0, &pSettings->stcSettings );
    pSettings->stcSettings.timebase = NEXUS_Timebase_e0;
    pSettings->stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    pSettings->stcChannel = NEXUS_StcChannel_Open ( 0, &pSettings->stcSettings );

    NEXUS_VideoDecoder_GetDefaultStartSettings ( &videoProgram );
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = pSettings->videoPidChannel;
    videoProgram.stcChannel = pSettings->stcChannel;
    NEXUS_AudioDecoder_GetDefaultStartSettings ( &audioProgram );
    audioProgram.codec = AUDIO_CODEC;
    audioProgram.pidChannel = pSettings->audioPidChannel;
    audioProgram.stcChannel = pSettings->stcChannel;
    /* Start decoders */
    NEXUS_VideoDecoder_Start ( pSettings->videoDecoder, &videoProgram );
    NEXUS_AudioDecoder_Start ( pSettings->audioDecoder, &audioProgram );

}

NEXUS_Error SecurityExampleSetupDecodersDisplays ( NEXUS_ExampleSecuritySettings * pSettings )
{
#if NEXUS_NUM_HDMI_OUTPUTS
    /* Support HDMI output */
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
#endif
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_Error     rc = 0;

#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput ( NEXUS_AudioDac_GetConnector ( pSettings->platformConfig.outputs.audioDacs[0] ),
                                 NEXUS_AudioDecoder_GetConnector ( pSettings->audioDecoder,
                                                                   NEXUS_AudioDecoderConnectorType_eStereo ) );
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutput_AddInput ( NEXUS_SpdifOutput_GetConnector ( pSettings->platformConfig.outputs.spdif[0] ),
                                 NEXUS_AudioDecoder_GetConnector ( pSettings->audioDecoder,
                                                                   NEXUS_AudioDecoderConnectorType_eStereo ) );
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput ( NEXUS_HdmiOutput_GetAudioConnector ( pSettings->platformConfig.outputs.hdmi[0] ),
                                 NEXUS_AudioDecoder_GetConnector ( pSettings->audioDecoder,
                                                                   NEXUS_AudioDecoderConnectorType_eStereo ) );
#endif

    /* Bring up video display and outputs */
    pSettings->display = NEXUS_Display_Open ( 0, NULL );
    pSettings->window = NEXUS_VideoWindow_Open ( pSettings->display, 0 );

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput ( pSettings->display,
                              NEXUS_ComponentOutput_GetConnector ( pSettings->platformConfig.outputs.component[0] ) );
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput ( pSettings->display,
                              NEXUS_CompositeOutput_GetConnector ( pSettings->platformConfig.outputs.composite[0] ) );
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput ( pSettings->display,
                              NEXUS_HdmiOutput_GetVideoConnector ( pSettings->platformConfig.outputs.hdmi[0] ) );
    rc = NEXUS_HdmiOutput_GetStatus ( pSettings->platformConfig.outputs.hdmi[0], &hdmiStatus );
    if ( !rc && hdmiStatus.connected )
    {
        /* If current display format is not supported by monitor, switch to monitor's preferred format.
         * If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings ( pSettings->display, &displaySettings );
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] )
        {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings ( pSettings->display, &displaySettings );
        }
    }
#endif

    /* bring up decoder and connect to display */
    NEXUS_VideoWindow_AddInput ( pSettings->window, NEXUS_VideoDecoder_GetConnector ( pSettings->videoDecoder ) );

    return NEXUS_SUCCESS;
}

#endif
