/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
 *****************************************************************************/

#include "b_os_lib.h"
#include "bip_priv.h"
#include "bip_streamer_impl.h"
#include "b_playback_ip_lib.h"
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

BDBG_MODULE( bip_streamer );
BDBG_OBJECT_ID( BIP_Streamer );
BIP_SETTINGS_ID(BIP_StreamerFileInputSettings);
BIP_SETTINGS_ID(BIP_StreamerCreateSettings);
BIP_SETTINGS_ID(BIP_StreamerTunerInputSettings);
BIP_SETTINGS_ID(BIP_StreamerIpInputSettings);
BIP_SETTINGS_ID(BIP_StreamerRecpumpInputSettings);
BIP_SETTINGS_ID(BIP_StreamerTrackSettings);
BIP_SETTINGS_ID(BIP_StreamerOutputSettings);
BIP_SETTINGS_ID(BIP_StreamerPrepareSettings);
BIP_SETTINGS_ID(BIP_StreamerStartSettings);
BIP_SETTINGS_ID_DECLARE(BIP_TranscodeProfile);
BIP_SETTINGS_ID_DECLARE(BIP_TranscodeNexusHandles);


struct BIP_StreamerStateNames
{
    BIP_StreamerState state;
    char *pStateName;
}gStreamerState[] = {
    {BIP_StreamerState_eUninitialized,      "UnInitialized"},
    {BIP_StreamerState_eIdle,               "Idle"},                /* Idle state. */
    {BIP_StreamerState_ePrepared,           "Prepared"},            /* Prepared for Streaming. */
    {BIP_StreamerState_eStreaming,          "Streaming"},           /* Media is being streamed. */
    {BIP_StreamerState_eStreamingDone,      "StreamingDone"},       /* Streaming is done: either reached EOF or met an error during streaming. */
    {BIP_StreamerState_eWaitingForStopApi,  "BIP_StreamerState_eWaitingForStopApi"},   /* Waiting for Stop() from the caller. */
    {BIP_StreamerState_eMax,                "MaxState"}
};
#define BIP_STREAMER_STATE(state) \
    gStreamerState[state].pStateName

void processStreamerState( void *jObject, int value, BIP_Arb_ThreadOrigin threadOrigin );

static void printStreamerStatus(
    BIP_StreamerHandle hStreamer
    )
{
    NEXUS_Error nrc;

    if ( hStreamer == NULL ) return;

    /* Playback Stats. */
    if ( hStreamer->hPlayback )
    {
        NEXUS_PlaybackStatus pbStatus;
        NEXUS_PlaypumpStatus ppStatus;

        nrc = NEXUS_Playback_GetStatus( hStreamer->hPlayback, &pbStatus );
        BIP_CHECK_GOTO((nrc==NEXUS_SUCCESS), ( "NEXUS_Playback_GetStatus() failed" ), error_play, nrc, nrc );

        BDBG_WRN(( "PB: state=%s fifo=%u/%u played=%ju dataErrors=%d indexErrors=%d posFirst=%d posLast=%d posRead=%d posCurrent=%d",
                    pbStatus.state == NEXUS_PlaybackState_eStopped ? "Stopped":
                    pbStatus.state == NEXUS_PlaybackState_ePlaying ? "Playing":
                    pbStatus.state == NEXUS_PlaybackState_ePaused ? "Paused":
                    pbStatus.state == NEXUS_PlaybackState_eTrickMode ? "Trickmode":
                    pbStatus.state == NEXUS_PlaybackState_eAborted ? "Aborted": "Unknown",
                    pbStatus.fifoDepth, pbStatus.fifoSize,
                    (uintmax_t)pbStatus.bytesPlayed, pbStatus.dataErrors, pbStatus.indexErrors,
                    (int)pbStatus.first, (int)pbStatus.last, (int)pbStatus.readPosition, (int)pbStatus.position
                 ));
        nrc = NEXUS_Playpump_GetStatus( hStreamer->hPlaypump, &ppStatus );
        BIP_CHECK_GOTO((nrc==NEXUS_SUCCESS), ( "NEXUS_Playpump_GetStatus() failed" ), error_play, nrc, nrc );

        BDBG_WRN(( "PP: state=%s syncErrors=%u pacingTsRangeError=%u streamErrors=%d",
                    ppStatus.started?"Started":"Stopped", ppStatus.syncErrors, ppStatus.pacingTsRangeError, ppStatus.streamErrors));
error_play:
        ; /* empty statement for label */
    }

    /* Recpump Stats. */
    if (hStreamer->hRecpump)
    {
        NEXUS_RecpumpStatus recStatus;

        nrc = NEXUS_Recpump_GetStatus( hStreamer->hRecpump, &recStatus );
        BIP_CHECK_GOTO((nrc==NEXUS_SUCCESS), ( "NEXUS_Recpump_GetStatus() failed" ), error_rec, nrc, nrc );

        BDBG_WRN(( "RP: state=%s dataFifo=%zu / %zu indexFifo=%zu/%zu hasIdx=%s raveIdx=%d",
                    recStatus.started?"Started":"Stopped",
                    recStatus.data.fifoDepth, recStatus.data.fifoSize,
                    recStatus.index.fifoDepth, recStatus.index.fifoSize,
                    recStatus.hasIndex?"Y":"N", recStatus.rave.index
                    ));
error_rec:
        ; /* empty statement for label */
    }

    /* PidChannel Stats. */
    {
        NEXUS_PidChannelStatus pidStatus;

        BIP_StreamerTrackListEntryHandle hTrackEntry;

        for (
                hTrackEntry = BLST_Q_FIRST( &hStreamer->track.listHead);
                hTrackEntry;
                hTrackEntry = BLST_Q_NEXT( hTrackEntry, trackListNext )
            )
        {
            if ( hTrackEntry->hPidChannel == NULL) continue;
            nrc = NEXUS_PidChannel_GetStatus( hTrackEntry->hPidChannel, &pidStatus);
            BIP_CHECK_GOTO((nrc==NEXUS_SUCCESS), ( "NEXUS_PidChannel_GetStatus() failed" ), error_pid, nrc, nrc );

            BDBG_ASSERT( nrc == NEXUS_SUCCESS );
            BDBG_WRN(("PidCh: pid=0x%x hwIndex=%u transportType=%u enabled=%s playback=%s pbIdx=%u parserBandIdx=%u ccErrs=%u",
                        pidStatus.pid, pidStatus.pidChannelIndex,
                        (unsigned)pidStatus.transportType, pidStatus.enabled?"Y":"N",
                        pidStatus.playback?"Y":"N", pidStatus.playbackIndex,pidStatus.parserBand,
                        pidStatus.continuityCountErrors
                        ));
            BDBG_WRN(("PidChRave: ccErrs=%u cdbOverflowErrors=%u itbOverflowErrors=%u emulationByteRemovalErrors=%u teiErrors=%u pusiErrors=%u xcBufOverflowErrors=%u",
                        pidStatus.raveStatus.continuityCountErrors,
                        pidStatus.raveStatus.cdbOverflowErrors,
                        pidStatus.raveStatus.itbOverflowErrors,
                        pidStatus.raveStatus.emulationByteRemovalErrors,
                        pidStatus.raveStatus.teiErrors,
                        pidStatus.raveStatus.pusiErrors,
                        pidStatus.xcBufferStatus.overflowErrors
                     ));
        }
error_pid:
        ; /* empty statement for label */
    }
}

static void removeAllTracksFromList(
    BIP_StreamerHandle hStreamer
    )
{
    BIP_StreamerTrackListEntryHandle hTrackEntry;

    for (
         hTrackEntry = BLST_Q_FIRST( &hStreamer->track.listHead);
         hTrackEntry;
         hTrackEntry = BLST_Q_FIRST( &hStreamer->track.listHead)
        )
    {
        BLST_Q_REMOVE( &hStreamer->track.listHead, hTrackEntry, trackListNext );

        /* Note: PidChannel handle should either be not yet opened or already be freed. */
        BDBG_ASSERT( hTrackEntry->hPidChannel == NULL );
        if ( hTrackEntry->hMediaNavFileAbsolutePathName ) BIP_String_Destroy( hTrackEntry->hMediaNavFileAbsolutePathName );
        B_Os_Free(hTrackEntry);
    }

    hStreamer->track.inputState = BIP_StreamerInputState_eNotSet;
    return;
} /* removeAllTracksFromList */

static BIP_Status addTrackToList(
    BIP_StreamerHandle hStreamer,
    BIP_StreamerTrackInfo *pStreamerTrackInfo,
    BIP_StreamerTrackSettings *pTrackSettings
    )
{
    BIP_Status bipStatus = BIP_ERR_INTERNAL;
    BIP_StreamerTrackListEntryHandle hTrackEntry;
    bool duplicateTrack = false;

    BDBG_ASSERT( pTrackSettings );

    /* Check if new track matches either both Id & type or just one of them of an existing track and take the correct action. */
    for (
         hTrackEntry = BLST_Q_FIRST( &hStreamer->track.listHead);
         hTrackEntry;
         hTrackEntry = BLST_Q_NEXT( hTrackEntry, trackListNext), duplicateTrack = false
        )
    {
        if ( hTrackEntry->streamerTrackInfo.trackId == pStreamerTrackInfo->trackId &&  hTrackEntry->streamerTrackInfo.type == pStreamerTrackInfo->type )
        {
            /* Duplicate track, we overwrite the existing one w/ the new one! */
            hTrackEntry->trackSettings = *pTrackSettings;
            hTrackEntry->streamerTrackInfo = *pStreamerTrackInfo;
            hTrackEntry->hPidChannel = NULL;    /* allocated during Start API. */
            BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: Duplicate entry: (trackId 0x%x, type %d) for inputType %d is replaced!"
                        BIP_MSG_PRE_ARG, (void *)hStreamer, pStreamerTrackInfo->trackId, pStreamerTrackInfo->type, hStreamer->inputType));
            bipStatus = BIP_SUCCESS;
            break;
        }
        else if ( hTrackEntry->streamerTrackInfo.trackId == pStreamerTrackInfo->trackId )
        {
            /* Only trackId matches and thus track types are different. */
            if ( hTrackEntry->streamerTrackInfo.type == BIP_MediaInfoTrackType_ePcr && pStreamerTrackInfo->type == BIP_MediaInfoTrackType_eVideo )
            {
                /* PCR track is already there and the new one being added is of type Video. */
                /* We mark PCR track as the duplicate one as PidChannel is not opened for the duplicate track */
                /* and we want to open the PidChannel for Video track. */
                hTrackEntry->duplicateTrack = true;
                hTrackEntry = false;    /* so that we can create a new track entry but NOT mark it as duplicate. */
                duplicateTrack = false;
                break;
            }
            else if ( hTrackEntry->streamerTrackInfo.type == BIP_MediaInfoTrackType_eVideo && pStreamerTrackInfo->type == BIP_MediaInfoTrackType_ePcr )
            {
                /* Video track is already there and the new one being added is of type PCR. We will mark new one as duplicate. */
                hTrackEntry = false;    /* so that we can create a new track entry but mark it as duplicate. */
                duplicateTrack = true;
                break;
            }
            else
            {
                /* Track w/ same trackId but different type is marked as duplicate track. */
                duplicateTrack = true;
                hTrackEntry = false;    /* so that we can create a new track entry but mark it as duplicate. */
                break;
            }
        }
        /* else case: where trackTypes will be same but trackId will be different, we add such tracks as individual tracks (e.g. multiple audio tracks). */
    }

    /* If new track is not exact duplicate of the tracks in the current list, create a new entry and store its settings. */
    if ( hTrackEntry == NULL )
    {
        hTrackEntry = B_Os_Calloc( 1, sizeof(BIP_StreamerTrackListEntry));
        BIP_CHECK_GOTO(( hTrackEntry != NULL ), ( "Failed to allocate memory (%zu bytes) for TrackEntry Object", sizeof(BIP_StreamerTrackListEntry) ),
                error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

        hTrackEntry->trackSettings = *pTrackSettings;
        hTrackEntry->streamerTrackInfo = *pStreamerTrackInfo;
        hTrackEntry->hPidChannel = NULL;    /* allocated during Start API. */
        hTrackEntry->duplicateTrack = duplicateTrack; /* if marked duplicate, it is not added to the h/w pid parsers. */
        if ( pStreamerTrackInfo->type == BIP_MediaInfoTrackType_eVideo && pStreamerTrackInfo->info.video.pMediaNavFileAbsolutePathName )
        {
            hTrackEntry->hMediaNavFileAbsolutePathName = BIP_String_CreateFromChar( pStreamerTrackInfo->info.video.pMediaNavFileAbsolutePathName );
            BIP_CHECK_GOTO(( hTrackEntry->hMediaNavFileAbsolutePathName ), ( "BIP_String_CreateFromChar Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
        }
        BLST_Q_INSERT_TAIL( &hStreamer->track.listHead, hTrackEntry, trackListNext );

        hStreamer->stats.numTracksAdded++;
        hStreamer->track.inputState = BIP_StreamerInputState_eSet;
        BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: (trackId 0x%x, type %d) for inputType %d is added, track.inputState %s, duplicateTrack %s, hTrackEntry %p"
                    BIP_MSG_PRE_ARG, (void *)hStreamer, pStreamerTrackInfo->trackId, pStreamerTrackInfo->type, hStreamer->inputType,
                    hStreamer->track.inputState == BIP_StreamerInputState_eSet ? "Set":"NotSet",
                    duplicateTrack ? "Y":"N", (void *)hTrackEntry
                    ));

        bipStatus = BIP_SUCCESS;
    }

error:
    if ( bipStatus != BIP_SUCCESS )
    {
        if ( hTrackEntry ) B_Os_Free( hTrackEntry );
    }
    return bipStatus;
} /* addTrackToList */

bool BIP_Streamer_GetTrackEntry_priv(
    BIP_StreamerHandle hStreamer,
    BIP_MediaInfoTrackType trackType,
    BIP_StreamerTrackListEntryHandle *phTrackEntry
    )
{
    BIP_Status bipStatus = BIP_ERR_NOT_AVAILABLE;
    BIP_StreamerTrackListEntryHandle hTrackEntry;

    for (
         hTrackEntry = BLST_Q_FIRST( &hStreamer->track.listHead);
         hTrackEntry;
         hTrackEntry = BLST_Q_NEXT( hTrackEntry, trackListNext)
        )
    {
        if (hTrackEntry->streamerTrackInfo.type == trackType)
        {
            bipStatus = BIP_SUCCESS;
            *phTrackEntry = hTrackEntry;
            break;
        }
    }
    BDBG_MSG(( BIP_MSG_PRE_FMT "Streamer %p: track type %d is present %s, hTrackEntry %p" BIP_MSG_PRE_ARG, (void *)hStreamer, trackType, hTrackEntry?"Y":"N", (void *)hTrackEntry ));
    return ( bipStatus );
} /* BIP_Streamer_GetTrackEntry_priv */

static void removeAllTranscodeProfilesFromList(
    BIP_StreamerHandle hStreamer
    )
{
    BIP_StreamerTranscodeProfileListEntry *pTranscodeProfileEntry;

    for (
         pTranscodeProfileEntry = BLST_Q_FIRST( &hStreamer->transcode.profileListHead);
         pTranscodeProfileEntry;
         pTranscodeProfileEntry = BLST_Q_FIRST( &hStreamer->transcode.profileListHead)
        )
    {
        BLST_Q_REMOVE( &hStreamer->transcode.profileListHead, pTranscodeProfileEntry, transcodeProfileListNext );
        B_Os_Free(pTranscodeProfileEntry);
    }

    hStreamer->transcode.profileState = BIP_StreamerOutputState_eNotSet;
    return;
} /* removeAllTranscodeProfilesFromList */

static BIP_Status addTranscodeProfileToList(
    BIP_StreamerHandle hStreamer,
    BIP_TranscodeProfile *pTranscodeProfile
    )
{
    BIP_Status bipStatus = BIP_ERR_INTERNAL;
#if NEXUS_HAS_VIDEO_ENCODER
    BIP_StreamerTranscodeProfileListEntry *pTranscodeProfileEntry, *pCurTranscodeProfileEntry;
    bool before = false;

    pTranscodeProfileEntry = B_Os_Calloc( 1, sizeof(BIP_StreamerTranscodeProfileListEntry));
    BIP_CHECK_GOTO(( pTranscodeProfileEntry != NULL ), ( "Failed to allocate memory (%zu bytes) for TranscodeProfileEntry Object", sizeof(BIP_StreamerTranscodeProfileListEntry) ),
            error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    pTranscodeProfileEntry->transcodeProfile = *pTranscodeProfile;
    pTranscodeProfileEntry->profileIndex = hStreamer->transcode.numProfiles++;

    /* Keep the list sorted in the increasing order of the bitrate. */
    for (
         pCurTranscodeProfileEntry = BLST_Q_FIRST( &hStreamer->transcode.profileListHead);
         pCurTranscodeProfileEntry;
         pCurTranscodeProfileEntry = BLST_Q_NEXT( pCurTranscodeProfileEntry, transcodeProfileListNext )
        )
    {
        if ( pTranscodeProfileEntry->transcodeProfile.video.settings.bitrateMax <= pCurTranscodeProfileEntry->transcodeProfile.video.settings.bitrateMax )
        {
            /* The new entry has bitrate smaller than the current one, so insert the new entry before this current one! */
            break;
        }
    }

    if ( pCurTranscodeProfileEntry )
    {
        BLST_Q_INSERT_BEFORE( &hStreamer->transcode.profileListHead, pCurTranscodeProfileEntry, pTranscodeProfileEntry, transcodeProfileListNext );
        before = true;
    }
    else
    {
        BLST_Q_INSERT_TAIL( &hStreamer->transcode.profileListHead, pTranscodeProfileEntry, transcodeProfileListNext );
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: transcodeProfile added %s" BIP_MSG_PRE_ARG, (void *)hStreamer, before?"before the current entry":"at the tail of list" ));

    bipStatus = BIP_SUCCESS;
    return bipStatus;

error:
    removeAllTranscodeProfilesFromList( hStreamer );
#else
    BSTD_UNUSED( hStreamer );
    BSTD_UNUSED( pTranscodeProfile );
#endif
    return bipStatus;
} /* addTranscodeProfileToList */

static BIP_Status removeAndClosePidChannels(
    BIP_StreamerHandle hStreamer
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_StreamerTrackListEntryHandle hTrackEntry;

    BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer: %p" BIP_MSG_PRE_ARG, (void *)hStreamer ));
    for (
         hTrackEntry = BLST_Q_FIRST( &hStreamer->track.listHead);
         hTrackEntry;
         hTrackEntry = BLST_Q_NEXT( hTrackEntry, trackListNext)
        )
    {
        if ( hTrackEntry->duplicateTrack ) continue;
        if ( !hTrackEntry->hPidChannel ) continue;
        if ( hTrackEntry->pidChannelAddedToRecpump ) NEXUS_Recpump_RemovePidChannel( hStreamer->hRecpump, hTrackEntry->hPidChannel);
        if ( hStreamer->inputType == BIP_StreamerInputType_eFile )
        {
            NEXUS_Playback_ClosePidChannel( hStreamer->hPlayback, hTrackEntry->hPidChannel );
        }
        if ( hStreamer->inputType == BIP_StreamerInputType_eTuner )
        {
            NEXUS_PidChannel_Close(hTrackEntry->hPidChannel);
        }
        hTrackEntry->hPidChannel = NULL;
    }
    return bipStatus;
} /* removeAndClosePidChannels */

static BIP_Status openPidChannels(
    BIP_StreamerHandle hStreamer
    )
{
    BIP_Status bipStatus = BIP_ERR_INTERNAL;
    NEXUS_PidChannelHandle hPidChannel = NULL;
    BIP_StreamerTrackListEntryHandle hTrackEntry;

    for (
         hTrackEntry = BLST_Q_FIRST( &hStreamer->track.listHead);
         hTrackEntry;
         hTrackEntry = BLST_Q_NEXT( hTrackEntry, trackListNext)
        )
    {
        if ( hTrackEntry->duplicateTrack )
        {
            /* Skip the duplicateTrack as we will open PidChannels for its matching track. */
            continue;
        }
        switch ( hStreamer->inputType )
        {
            case BIP_StreamerInputType_eIp:
                {
                    BIP_PlayerOpenPidChannelSettings  settings;

                    BIP_Player_GetDefaultOpenPidChannelSettings(&settings);
                    bipStatus = BIP_Player_OpenPidChannel(hStreamer->ip.hPlayer, hTrackEntry->streamerTrackInfo.trackId, &settings, &hPidChannel);
                    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS && hPidChannel ), ( "hStreamer %p, state %s: BIP_Player_OpenPidChannel() Failed",
                                (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state) ), error, BIP_ERR_NOT_AVAILABLE, bipStatus );

                    break;
                }
            case BIP_StreamerInputType_eTuner:
                {
                    hPidChannel = NEXUS_PidChannel_Open( hStreamer->tuner.hParserBand, hTrackEntry->streamerTrackInfo.trackId, &hTrackEntry->trackSettings.pidChannelSettings );
                    BIP_CHECK_GOTO(( hPidChannel ), ( "hStreamer %p, state %s: NEXUS_PidChannel_Open() Failed",
                                (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state) ), error, BIP_ERR_NOT_AVAILABLE, bipStatus );

                    break;
                }
            case BIP_StreamerInputType_eFile:
                {
                    NEXUS_PlaybackPidChannelSettings playbackPidSettings;

                    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
                    playbackPidSettings.pidSettings.pidSettings = hTrackEntry->trackSettings.pidChannelSettings;
                    if (hTrackEntry->streamerTrackInfo.type == BIP_MediaInfoTrackType_eVideo)
                    {
                        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
                        playbackPidSettings.pidTypeSettings.video.codec = hTrackEntry->streamerTrackInfo.info.video.codec;
                        playbackPidSettings.pidTypeSettings.video.index = false;
                    }
                    else if (hTrackEntry->streamerTrackInfo.type == BIP_MediaInfoTrackType_eAudio)
                    {
                        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
                    }
                    hPidChannel = NEXUS_Playback_OpenPidChannel(hStreamer->hPlayback, hTrackEntry->streamerTrackInfo.trackId, &playbackPidSettings);
                    BIP_CHECK_GOTO(( hPidChannel ), ( "hStreamer %p, state %s: NEXUS_PlaybackOpen_PidChannel() Failed",
                                (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state) ), error, BIP_ERR_NOT_AVAILABLE, bipStatus );
                    break;
                }
            default:
                {
                    bipStatus = BIP_ERR_NOT_AVAILABLE;
                    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "hStreamer %p, state %s: PidChannel from inputType %d is not yet supported!",
                                (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state), hStreamer->inputType ), error, bipStatus, bipStatus );
                }
        } /* switch */

        /* Now update the trackEntry with this associated pidChannel handle. */
        if ( hPidChannel )
        {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: PidChannel=%p Opened: (trackId 0x%x, type %d) for inputType %d!, hTrackEntry=%p"
                        BIP_MSG_PRE_ARG, (void *)hStreamer, (void *)hPidChannel, hTrackEntry->streamerTrackInfo.trackId, hTrackEntry->streamerTrackInfo.type, hStreamer->inputType, (void *)hTrackEntry ));

            hTrackEntry->hPidChannel = hPidChannel;
            bipStatus = BIP_SUCCESS;
        }
    }
    return (bipStatus);

error:
    if (hPidChannel)
    {
        if (hStreamer->inputType == BIP_StreamerInputType_eFile)
        {
            NEXUS_Playback_ClosePidChannel(hStreamer->hPlayback, hPidChannel);
        }
        else
        {
            NEXUS_PidChannel_Close(hPidChannel);
        }
    }
    return bipStatus;
} /* openPidChannels */

static BIP_Status openAllpassPidChannel(
    BIP_StreamerHandle hStreamer
    )
{
    BIP_Status bipStatus = BIP_ERR_INTERNAL;
    NEXUS_PidChannelHandle hPidChannel = NULL;
    BIP_StreamerTrackListEntryHandle hTrackEntry;

    {
        BIP_StreamerTrackInfo       streamerTrackInfo;
        BIP_StreamerTrackSettings   trackSettings;

        BIP_Streamer_GetDefaultTrackSettings( &trackSettings );
        streamerTrackInfo.trackId = 0;
        streamerTrackInfo.type = BIP_MediaInfoTrackType_eOther;
        bipStatus = addTrackToList( hStreamer, &streamerTrackInfo , &trackSettings);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "hStreamer %p, addTrackToList failed state=%s",
                    (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state) ), error, BIP_ERR_NOT_AVAILABLE, bipStatus );
    }

    switch ( hStreamer->inputType )
    {
        case BIP_StreamerInputType_eIp:
            {
                BIP_PlayerOpenPidChannelSettings  settings;

                BIP_Player_GetDefaultOpenPidChannelSettings(&settings);
                bipStatus = BIP_Player_OpenPidChannel(hStreamer->ip.hPlayer, 0/* NA for allPass case*/, &settings, &hPidChannel);
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS && hPidChannel ), ( "hStreamer %p, state %s: BIP_Player_OpenPidChannel() Failed",
                            (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state) ), error, BIP_ERR_NOT_AVAILABLE, bipStatus );

                break;
            }
#if 0
            /* TODO: need to add all pass support for other inputs */
        case BIP_StreamerInputType_eTuner:
            {
                hPidChannel = NEXUS_PidChannel_Open( hStreamer->tuner.hParserBand, hTrackEntry->streamerTrackInfo.trackId, &hTrackEntry->trackSettings.pidChannelSettings );
                BIP_CHECK_GOTO(( hPidChannel ), ( "hStreamer %p, state %s: NEXUS_PidChannel_Open() Failed",
                            hStreamer, BIP_STREAMER_STATE(hStreamer->state) ), error, BIP_ERR_NOT_AVAILABLE, bipStatus );

                break;
            }
#endif
        case BIP_StreamerInputType_eFile:
            {
                NEXUS_PlaybackPidChannelSettings playbackPidSettings;

                NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
            NEXUS_Playpump_GetAllPassPidChannelIndex(hStreamer->hPlaypump, &playbackPidSettings.pidSettings.pidSettings.pidChannelIndex );
                hPidChannel = NEXUS_Playback_OpenPidChannel(hStreamer->hPlayback, 0x00, &playbackPidSettings);
                BIP_CHECK_GOTO(( hPidChannel ), ( "hStreamer %p, state %s: NEXUS_PlaybackOpen_PidChannel() Failed",
                            (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state) ), error, BIP_ERR_NOT_AVAILABLE, bipStatus );
                break;
            }
        default:
            {
                bipStatus = BIP_ERR_NOT_AVAILABLE;
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "hStreamer %p, state %s: PidChannel from inputType %d is not yet supported!",
                            (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state), hStreamer->inputType ), error, bipStatus, bipStatus );
            }
    } /* switch */

    /* Now update the trackEntry with this associated pidChannel handle. */
    if ( hPidChannel )
    {
        hTrackEntry = BLST_Q_FIRST( &hStreamer->track.listHead);
        hTrackEntry->hPidChannel = hPidChannel;
        BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: AllPass PidChannel=%p Opened for inputType %d!, hTrackEntry=%p" BIP_MSG_PRE_ARG, (void *)hStreamer, (void *)hPidChannel, hStreamer->inputType, (void *)hTrackEntry ));
        bipStatus = BIP_SUCCESS;
    }
    return (bipStatus);

error:
    if (hPidChannel)
    {
        if (hStreamer->inputType == BIP_StreamerInputType_eFile)
        {
            NEXUS_Playback_ClosePidChannel(hStreamer->hPlayback, hPidChannel);
        }
        else
        {
            NEXUS_PidChannel_Close(hPidChannel);
        }
    }
    return bipStatus;
} /* openAllpassPidChannel */

static BIP_Status addPidChannelToRecpump(
    BIP_StreamerHandle hStreamer
    )
{
    BIP_Status bipStatus = BIP_ERR_INTERNAL;
    BIP_StreamerTrackListEntryHandle hTrackEntry;
    NEXUS_Error nrc;

    for (
         hTrackEntry = BLST_Q_FIRST( &hStreamer->track.listHead);
         hTrackEntry;
         hTrackEntry = BLST_Q_NEXT( hTrackEntry, trackListNext)
        )
    {
        if ( hTrackEntry->duplicateTrack || !hTrackEntry->hPidChannel )
        {
            /* Skip the duplicateTrack as we will open PidChannels for its matching track. */
            continue;
        }
        nrc = NEXUS_Recpump_AddPidChannel( hStreamer->hRecpump, hTrackEntry->hPidChannel, &hTrackEntry->trackSettings.recpumpPidChannelSettings );
        BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "hStreamer %p, state %s: NEXUS_Recpump_AddPidChannel() Failed, nrc %d",
                    (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state), nrc ), error, BIP_ERR_INTERNAL, bipStatus );
        hTrackEntry->pidChannelAddedToRecpump = true;

        BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: PidChannel=%p Added: (trackId 0x%x, type %d) for inputType %d!, hTrackEntry=%p"
                    BIP_MSG_PRE_ARG, (void *)hStreamer, (void *)hTrackEntry->hPidChannel, hTrackEntry->streamerTrackInfo.trackId, hTrackEntry->streamerTrackInfo.type, hStreamer->inputType, (void *)hTrackEntry ));

        bipStatus = BIP_SUCCESS;
    }

error:
    return bipStatus;
} /* addPidChannelToRecpump */

static void closeNexusRecpump(
    BIP_StreamerHandle hStreamer
    )
{
    if ( hStreamer->openedRecpump == true )
    {
        if ( hStreamer->hRecpump )
            NEXUS_Recpump_Close( hStreamer->hRecpump );
        hStreamer->openedRecpump = false;
    }
    hStreamer->hRecpump = NULL;
    BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: Done" BIP_MSG_PRE_ARG, (void *)hStreamer ));
} /* closeNexusRecpump */

static BIP_Status openNexusRecpump(
    BIP_StreamerHandle hStreamer
    )
{
    BIP_Status bipStatus = BIP_ERR_INVALID_PARAMETER;

    if ( hStreamer->recpump.inputState == BIP_StreamerInputState_eSet )
    {
        /* If app is directly configuring the recpump, then it must have already provided us recpump handle. */
        /* Api side has verified it, so just use it. */
        BDBG_ASSERT( hStreamer->hRecpump );
        BIP_CHECK_GOTO(( hStreamer->hRecpump ), ( "App must provide RecpumpHandle for Recpump input case!" ), error, BIP_ERR_NEXUS, bipStatus );
    }
    else if ( hStreamer->tuner.inputSettings.hRecpump )
    {
        /* Caller has provided a recpumpHandle, we use that. */
        /* Note: this may be an issue we need to size the Recpump fifo as that can only be done at the open time. */
        hStreamer->hRecpump = hStreamer->tuner.inputSettings.hRecpump;
    }
    else if ( hStreamer->file.inputSettings.hRecpump )
    {
        /* Caller has provided a recpumpHandle, we use that. */
        /* Note: this may be an issue we need to size the Recpump fifo as that can only be done at the open time. */
        hStreamer->hRecpump = hStreamer->file.inputSettings.hRecpump;
    }
    else
    {
        /* Note: recpumpOpenSettings get initialised in the BIP_Streamer_GetDefaultPrepareSettings. */
        BDBG_MSG((BIP_MSG_PRE_FMT "atomSize %d, dataReadyThreshold %d" BIP_MSG_PRE_ARG,
                    hStreamer->prepareSettings.recpumpOpenSettings.data.atomSize,
                    hStreamer->prepareSettings.recpumpOpenSettings.data.dataReadyThreshold));

        hStreamer->hRecpump = NEXUS_Recpump_Open( NEXUS_ANY_ID, &hStreamer->prepareSettings.recpumpOpenSettings );
        BIP_CHECK_GOTO(( hStreamer->hRecpump ), ( "NEXUS_Recpump_Open Failed!" ), error, BIP_ERR_NEXUS, bipStatus );

        hStreamer->openedRecpump = true;
    }
    bipStatus = BIP_SUCCESS;

error:
    return ( bipStatus );
} /* openNexusRecpump */

static void closeNexusPlayback(
    BIP_StreamerHandle hStreamer
    )
{
    if ( hStreamer->hPlayback ) NEXUS_Playback_Destroy( hStreamer->hPlayback );
    hStreamer->hPlayback = NULL;

    if ( hStreamer->openedPlaypump && hStreamer->hPlaypump ) NEXUS_Playpump_Close( hStreamer->hPlaypump );
    hStreamer->hPlaypump = NULL;
    hStreamer->openedPlaypump = NULL;

    if ( hStreamer->hFilePlay ) NEXUS_FilePlay_Close( hStreamer->hFilePlay );
    hStreamer->hFilePlay = NULL;

    BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: Done" BIP_MSG_PRE_ARG, (void *)hStreamer ));
} /* closeNexusPlayback */

static BIP_Status openNexusPlayback(
    BIP_StreamerHandle hStreamer
    )
{
    BIP_Status bipStatus;
    const char *navFile = NULL;
    BIP_StreamerTrackListEntryHandle hTrackEntry;

    if ( hStreamer->file.inputSettings.hPlaypump )
    {
        hStreamer->hPlaypump = hStreamer->file.inputSettings.hPlaypump;
    }
    else
    {
        NEXUS_PlaypumpOpenSettings playpumpOpenSettings;

        NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
        hStreamer->hPlaypump = NEXUS_Playpump_Open( NEXUS_ANY_ID, &playpumpOpenSettings );
        BIP_CHECK_GOTO(( hStreamer->hPlaypump ), ( "NEXUS_Playpump_Open Failed!" ), error, BIP_ERR_NEXUS, bipStatus );
        hStreamer->openedPlaypump = true;
    }

    /* Create Nexus Playback context, check if Video track specifies the NAV file and if so, use it in the NEXUS_FilePlay_OpenPosix() */
    {
        if ( BIP_Streamer_GetTrackEntry_priv( hStreamer, BIP_MediaInfoTrackType_eVideo, &hTrackEntry ) == BIP_SUCCESS &&
                hTrackEntry->hMediaNavFileAbsolutePathName )
        {
            navFile = BIP_String_GetString( hTrackEntry->hMediaNavFileAbsolutePathName );
            BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: nag file: %s" BIP_MSG_PRE_ARG, (void *)hStreamer, navFile ));
        }

        hStreamer->hPlayback = NEXUS_Playback_Create();
        BIP_CHECK_GOTO(( hStreamer->hPlayback ), ( "NEXUS_Playback_Create Failed!" ), error, BIP_ERR_NEXUS, bipStatus );

        hStreamer->hFilePlay = NEXUS_FilePlay_OpenPosix(
                BIP_String_GetString(hStreamer->file.hMediaFileAbsolutePathName),
                navFile ? navFile : BIP_String_GetString( hStreamer->file.hMediaFileAbsolutePathName )
                );
        BIP_CHECK_GOTO(( hStreamer->hFilePlay ), ( "NEXUS_FilePlay_OpenPosix Failed!" ), error, BIP_ERR_NEXUS, bipStatus );
    }

#if 0
    /* PCR based pacing doesn't seem to require a separate h/w timebase for free-running it. Once this is confirmed, I will take this block out! */
    {
        NEXUS_Error nrc;

        if (0)
        {
            NEXUS_TimebaseSettings timebaseSettings;

            /* Must use the timebase after Video decoders & Video Encodres */
            hStreamer->pacingTimebase = NEXUS_Timebase_Open(NEXUS_ANY_ID);
            BIP_CHECK_GOTO(( hStreamer->pacingTimebase ), ( "NEXUS_Timebase_Open Failed!" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

            NEXUS_Timebase_GetSettings(hStreamer->pacingTimebase, &timebaseSettings);
            timebaseSettings.sourceType = NEXUS_TimebaseSourceType_eFreeRun;
            timebaseSettings.freeze = true;
            timebaseSettings.sourceSettings.pcr.trackRange = NEXUS_TimebaseTrackRange_e122ppm;
            nrc = NEXUS_Timebase_SetSettings(hStreamer->pacingTimebase, &timebaseSettings);
            BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEXUS_Timebase_SetSettings Failed!" ), error, BIP_ERR_INTERNAL, bipStatus );
        }
    }
#endif

    /* Now associate Playpump w/ the Playback */
    {
        NEXUS_Error nrc;
        NEXUS_PlaybackSettings playbackSettings;

        NEXUS_Playback_GetSettings( hStreamer->hPlayback, &playbackSettings );
        playbackSettings.endOfStreamAction = hStreamer->file.inputSettings.enableContinousPlay ? NEXUS_PlaybackLoopMode_eLoop : NEXUS_PlaybackLoopMode_ePause;
        playbackSettings.playpump = hStreamer->hPlaypump;
        playbackSettings.playpumpSettings.transportType = hStreamer->streamerStreamInfo.transportType;
        if (!hStreamer->offloadStreamerToAsp)
        {
            /* TODO: restricting b/w for non-ASP case. Restricting it here for ASP case has some issue w/ client's starving for data. */
            playbackSettings.playpumpSettings.maxDataRate = hStreamer->file.inputSettings.maxDataRate;
        }
        if ( hStreamer->file.inputSettings.enableAllPass )
        {
            playbackSettings.playpumpSettings.allPass = true;
            playbackSettings.playpumpSettings.acceptNullPackets = true;
        }
        nrc = NEXUS_Playback_SetSettings( hStreamer->hPlayback, &playbackSettings );
        BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEUS_Playback_SetSettings Failed!" ), error, BIP_ERR_NEXUS, bipStatus );
        bipStatus = BIP_SUCCESS;
    }
    BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: Nexus Playpump, Playback are Setup for transportType %d, input file: %s" BIP_MSG_PRE_ARG,
                (void *)hStreamer, hStreamer->streamerStreamInfo.transportType, BIP_String_GetString(hStreamer->file.hMediaFileAbsolutePathName) ));
    return bipStatus;

error:
    closeNexusPlayback( hStreamer );
    return ( bipStatus );
} /* openNexusPlayback */

static BIP_Status openNexusPlaybackAndRecpump(
    BIP_StreamerHandle hStreamer
    )
{
    BIP_Status bipStatus;

    bipStatus = openNexusRecpump( hStreamer );
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "openNexusRecpump Failed!" ), error, bipStatus, bipStatus );

    bipStatus = openNexusPlayback( hStreamer );
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "openNexusPlayback Failed!" ), error, bipStatus, bipStatus );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: Done" BIP_MSG_PRE_ARG, (void *)hStreamer ));
    return BIP_SUCCESS;

error:
    closeNexusRecpump( hStreamer );
    closeNexusPlayback( hStreamer );

    return bipStatus;
} /* openNexusPlaybackAndRecpump */

static void resetStreamerFileInputState(
    BIP_StreamerHandle hStreamer
    )
{
    if (hStreamer->file.inputState == BIP_StreamerInputState_eSet)
    {
        if (hStreamer->file.hMediaFileAbsolutePathName) BIP_String_Destroy(hStreamer->file.hMediaFileAbsolutePathName);
        hStreamer->file.hMediaFileAbsolutePathName = NULL;

        if (hStreamer->file.hPlaySpeed) BIP_String_Destroy(hStreamer->file.hPlaySpeed);
        hStreamer->file.hPlaySpeed = NULL;

        hStreamer->file.feedUsingPlaybackChannel = false;
        hStreamer->file.inputState = BIP_StreamerInputState_eNotSet;
    }
} /* resetStreamerFileInputState */

static void resetStreamerTunerInputState(
    BIP_StreamerHandle hStreamer
    )
{
    hStreamer->tuner.inputState = BIP_StreamerInputState_eNotSet;
} /* resetStreamerTunerInputState */

static void resetStreamerRecpumpInputState(
    BIP_StreamerHandle hStreamer
    )
{
    hStreamer->recpump.inputState = BIP_StreamerInputState_eNotSet;
} /* resetStreamerRecpumpInputState */

static void resetStreamerState(
    BIP_StreamerHandle hStreamer
    )
{
    BDBG_ASSERT(hStreamer);

    hStreamer->inputType = BIP_StreamerInputType_eMax;
    hStreamer->transcode.handlesState = BIP_StreamerOutputState_eNotSet;
    hStreamer->transcode.profileState = BIP_StreamerOutputState_eNotSet;
    hStreamer->offloadStreamerToAsp = false;

    hStreamer->stats.numTracksAdded = 0;

    removeAllTracksFromList( hStreamer );

    removeAllTranscodeProfilesFromList( hStreamer );

    resetStreamerRecpumpInputState(hStreamer);

    resetStreamerTunerInputState(hStreamer);

    resetStreamerFileInputState(hStreamer);

    BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: Done" BIP_MSG_PRE_ARG, (void *)hStreamer ));
} /* resetStreamerState */

static void closeNexusResources(
    BIP_StreamerHandle hStreamer
    )
{
    /* Now remove the PidChannels corresponding to any tracks. */
    removeAndClosePidChannels( hStreamer );

    closeNexusPlayback(hStreamer);
    closeNexusRecpump(hStreamer);
}

static BIP_Status stopAndUnPrepare(
    BIP_StreamerHandle hStreamer
    )
{
    BIP_Status bipStatus = BIP_ERR_INTERNAL;

    /* Stop Producer first & then the consumer */
    if ( hStreamer->ip.hPlayer )
    {
        BIP_Player_Stop(hStreamer->ip.hPlayer);
    }

    if ( hStreamer->hPlayback )
    {
        NEXUS_Playback_Stop( hStreamer->hPlayback );
    }

    if ( hStreamer->hRecpump )
    {
        NEXUS_Recpump_Stop( hStreamer->hRecpump );
    }

    if ( hStreamer->transcode.profileState == BIP_StreamerOutputState_eSet && hStreamer->transcode.hTranscode )
    {
        BIP_Transcode_Stop( hStreamer->transcode.hTranscode );
        BIP_Transcode_Destroy( hStreamer->transcode.hTranscode );
        hStreamer->transcode.hTranscode = NULL;
    }

    hStreamer->offloadStreamerToAsp = false;
    closeNexusResources( hStreamer );

    bipStatus = BIP_SUCCESS;

    BDBG_MSG((BIP_MSG_PRE_FMT "hStreamer %p: Done!" BIP_MSG_PRE_ARG, (void *)hStreamer ));
    return (bipStatus);

} /* stopAndUnPrepare */

static int convertPlaySpeedToInt(
    const char *playSpeed )
{
    int playSpeedInt = 1;
    if (playSpeed && *playSpeed != '\0')
    {
        /*
         * Note: even though app may have specified playSpeed string to
         * contain numerator & denominator (for slow-fwd or slow rwd) cases,
         * we just use the atoi to convert this to an int. This is becacause
         * we always send these slow speeds as 1x and let the client use its
         * STC to achieve the desired slow speed.
         */
        playSpeedInt = atoi(playSpeed);
        if (playSpeedInt == 0) /* something went wrong in the coversion, treat it as 1x */
        {
            BDBG_MSG(( BIP_MSG_PRE_FMT "app provided playSpeed: %s, atoi was 0, treating it as 1x" BIP_MSG_PRE_ARG, playSpeed ));
            playSpeedInt = 1;
        }
    }
    else
    {
        playSpeedInt = 1;
    }

    return (playSpeedInt);
} /* convertPlaySpeedToInt */

static void dataReadyCallbackFromRecpump(
    void *context,
    int   param
    )
{
    BIP_StreamerHandle hStreamer = (BIP_StreamerHandle)context;

    BSTD_UNUSED( param );
    BDBG_ASSERT(hStreamer);
    BDBG_OBJECT_ASSERT( hStreamer, BIP_Streamer);

} /* dataReadyCallbackFromRecpump */

static BIP_Status startNexusRecpump(
    BIP_StreamerHandle hStreamer
    )
{
    BIP_Status bipStatus = BIP_ERR_NEXUS;
    NEXUS_Error nrc = NEXUS_SUCCESS;
    NEXUS_RecpumpSettings recpumpSettings;

    /* Configure and start Recpump */
    NEXUS_Recpump_GetSettings( hStreamer->hRecpump, &recpumpSettings );
    recpumpSettings.timestampType = hStreamer->output.settings.mpeg2Ts.enableTransportTimestamp?
        NEXUS_TransportTimestampType_e32_Binary:NEXUS_TransportTimestampType_eNone;
    if ( hStreamer->inputType != BIP_StreamerInputType_eTuner )
    {
        recpumpSettings.bandHold = true;
    }
    recpumpSettings.data.dataReady.callback = dataReadyCallbackFromRecpump;
    recpumpSettings.data.dataReady.context  = hStreamer;
    nrc = NEXUS_Recpump_SetSettings( hStreamer->hRecpump, &recpumpSettings );
#if 0
    nrc = 1; /* TODO : test code */
#endif
    BIP_CHECK_GOTO(( !nrc ), ( "NEXUS_Recpump_SetSettings Failed!" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    nrc = NEXUS_Recpump_Start( hStreamer->hRecpump);
    BIP_CHECK_GOTO(( !nrc ), ( "NEXUS_Recpump_Start Failed!" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: NEXUS_Recpump is started!" BIP_MSG_PRE_ARG, (void *)hStreamer));
    bipStatus = BIP_SUCCESS;

error:
    return ( bipStatus );
}

static BIP_Status startNexusPlayback(
    BIP_StreamerHandle  hStreamer,
    bool                seekPositionValid,
    unsigned            seekPositionInMs
    )
{
    BIP_Status bipStatus = BIP_ERR_NEXUS;
    NEXUS_Error nrc;
    int playSpeed;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackStartSettings playbackStartSettings;
    BIP_StreamerTrackListEntryHandle hTrackEntry = NULL;

    /* Start Playback so that it can feed file in for file case. */

    playSpeed = convertPlaySpeedToInt(BIP_String_GetString(hStreamer->file.hPlaySpeed));
    NEXUS_Playback_GetSettings( hStreamer->hPlayback, &playbackSettings );
    playbackSettings.playpumpSettings.transportType = hStreamer->streamerStreamInfo.transportType;

    if (hStreamer->streamerStreamInfo.transportType == NEXUS_TransportType_eTs && playSpeed == 1 && hStreamer->transcode.profileState == BIP_StreamerOutputState_eNotSet &&
            (BIP_Streamer_GetTrackEntry_priv( hStreamer, BIP_MediaInfoTrackType_ePcr, &hTrackEntry ) == BIP_SUCCESS) && hStreamer->file.inputSettings.enableHwPacing == true)
    {
        /* For TS streams being played via Playback path w/o transcode enabled, turn on PCR Pacing if PCR track is present in the stream. */
        playbackSettings.playpumpSettings.timestamp.pacing = true;
        playbackSettings.playpumpSettings.timestamp.pcrPacingPid = hTrackEntry->streamerTrackInfo.trackId;
        playbackSettings.playpumpSettings.timestamp.pacingMaxError = 0xffff;
#if 0
        /*TODO: Doesn't seem like we have to set the specific timebase for PCR pacing but need to confirm this! */
        playbackSettings.playpumpSettings.timestamp.type = NEXUS_TransportTimestampType_eNone;
        playbackSettings.playpumpSettings.timestamp.timebase = hStreamer->pacingTimebase;
        playbackSettings.playpumpSettings.timestamp.pacingOffsetAdjustDisable = true;
        playbackSettings.playpumpSettings.timestamp.resetPacing = true;
        playbackSettings.playpumpSettings.timestamp.parityCheckDisable = true;
        playbackSettings.playpumpSettings.maxDataRate = (16*1024*1024);
#endif
        BDBG_MSG((BIP_MSG_PRE_FMT "hStreamer %p: PlaybackSettings: pacingMaxError %d, PCR# %d, PlaySpeed %d" BIP_MSG_PRE_ARG, (void *)hStreamer,
                    playbackSettings.playpumpSettings.timestamp.pacingMaxError,
                    playbackSettings.playpumpSettings.timestamp.pcrPacingPid, playSpeed));
    }
    else if ( (hStreamer->streamerStreamInfo.transportType == NEXUS_TransportType_eTs && playSpeed != 1) || seekPositionValid )
    {
        /* For non-1x playback, we start playback in the paused state and call the trickmode api to feed the scaled data! */
        /* Likewise, if seekPosition is set, then we start paused and then call Playback Seek API. */
        playbackSettings.startPaused = true;
    }

    nrc = NEXUS_Playback_SetSettings( hStreamer->hPlayback, &playbackSettings );
    BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEXUS_Playback_SetSettings Failed!" ), error, BIP_ERR_INTERNAL, bipStatus );

    NEXUS_Playback_GetDefaultStartSettings( &playbackStartSettings );
    nrc = NEXUS_Playback_Start( hStreamer->hPlayback, hStreamer->hFilePlay, &playbackStartSettings);
    BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEUS_Playback_Start Failed!" ), error, BIP_ERR_INTERNAL, bipStatus );

    /* TODO: add logic here for the file streaming byte/time seek case. */
    if (playSpeed != 1)
    {
        /* TODO: may need to refine some of these settings! */
        NEXUS_PlaybackTrickModeSettings playbackTrickModeSettings;
        NEXUS_Playback_GetDefaultTrickModeSettings(&playbackTrickModeSettings);
        playbackTrickModeSettings.rate = playSpeed * NEXUS_NORMAL_PLAY_SPEED;
        playbackTrickModeSettings.mode = NEXUS_PlaybackHostTrickMode_ePlayI;
        playbackTrickModeSettings.rateControl = NEXUS_PlaybackRateControl_eStream;
        playbackTrickModeSettings.skipControl = NEXUS_PlaybackSkipControl_eHost;
        nrc = NEXUS_Playback_TrickMode( hStreamer->hPlayback, &playbackTrickModeSettings );
        BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEUS_Playback_TrickMode Failed!" ), error, BIP_ERR_INTERNAL, bipStatus );
    }
    else if ( seekPositionValid )
    {
        nrc = NEXUS_Playback_Seek( hStreamer->hPlayback, seekPositionInMs );
        BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEUS_Playback_Seek Failed!" ), error, BIP_ERR_INTERNAL, bipStatus );
        nrc = NEXUS_Playback_Play( hStreamer->hPlayback );
        BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEUS_Playback_Play Failed!" ), error, BIP_ERR_INTERNAL, bipStatus );
    }

    bipStatus = BIP_SUCCESS;
    BDBG_WRN(( BIP_MSG_PRE_FMT "hStreamer %p: Playback from inputType %d Started: playSpeed=%d seekPositionInMs=%u" BIP_MSG_PRE_ARG, (void *)hStreamer, hStreamer->inputType, playSpeed, seekPositionInMs ));

error:
    return ( bipStatus );
} /* startNexusPlayback */

#if NEXUS_HAS_VIDEO_ENCODER
static void populateTranscodeTrackInfoFromStreamerTrackInfo(
    BIP_StreamerTrackInfo  *pStreamerTrack,
    BIP_TranscodeTrackInfo *pTranscodeTrack
    )
{
    BKNI_Memset(pTranscodeTrack, 0, sizeof(BIP_TranscodeTrackInfo));

    pTranscodeTrack->trackId = pStreamerTrack->trackId;
    pTranscodeTrack->type = pStreamerTrack->type;

    if( pStreamerTrack->type == BIP_MediaInfoTrackType_eVideo  )
    {
        pTranscodeTrack->info.video.codec = pStreamerTrack->info.video.codec;
        pTranscodeTrack->info.video.colorDepth = pStreamerTrack->info.video.colorDepth;
        pTranscodeTrack->info.video.height = pStreamerTrack->info.video.height;
        pTranscodeTrack->info.video.width = pStreamerTrack->info.video.width;
        pTranscodeTrack->info.video.pMediaNavFileAbsolutePathName = NULL;
    }
    else if( pStreamerTrack->type == BIP_MediaInfoTrackType_eAudio  )
    {
        pTranscodeTrack->info.audio.codec = pStreamerTrack->info.audio.codec;
    }
}

static void populateTranscodeStreamInfoFromStreamerStreamInfo(
    BIP_StreamerStreamInfo *pStreamerStreamInfo,
    BIP_TranscodeStreamInfo *pTranscodeStreamInfo
    )
{
    BKNI_Memset(pTranscodeStreamInfo, 0, sizeof(BIP_TranscodeStreamInfo));

    pTranscodeStreamInfo->contentLength = pStreamerStreamInfo->contentLength;
    pTranscodeStreamInfo->durationInMs = pStreamerStreamInfo->durationInMs;
    pTranscodeStreamInfo->numberOfTrackGroups = pStreamerStreamInfo->numberOfTrackGroups;
    pTranscodeStreamInfo->numberOfTracks = pStreamerStreamInfo->numberOfTracks;
    pTranscodeStreamInfo->transportTimeStampEnabled = pStreamerStreamInfo->transportTimeStampEnabled;
    pTranscodeStreamInfo->transportType = pStreamerStreamInfo->transportType;
}

static BIP_Status prepareTranscode(
        BIP_StreamerHandle hStreamer,
        BIP_StreamerInputType inputType,
        BIP_TranscodeProfile *pTranscodeProfile
        )
{
    BIP_Status  bipStatus = BIP_SUCCESS;
    bool        nonRealTime = true;
    BIP_TranscodeStreamInfo          transcodeStreamInfo;

    hStreamer->transcode.hTranscode = BIP_Transcode_Create( NULL );
    if ( hStreamer->transcode.hTranscode )
    {
        bool videoEnabled = false;
        BIP_StreamerTrackListEntryHandle hTrackEntry;
        BIP_TranscodePrepareSettings xcodePrepareSettings;

        BIP_Transcode_GetDefaultPrepareSettings( &xcodePrepareSettings );

        /* Fill-up Video PidChannel & Track Info if Video is enabled in the caller's transcode profile. */
        if ( !pTranscodeProfile->disableVideo )
        {
            bipStatus = BIP_Streamer_GetTrackEntry_priv( hStreamer, BIP_MediaInfoTrackType_eVideo, &hTrackEntry);
            if ( bipStatus == BIP_SUCCESS )
            {
                xcodePrepareSettings.hVideoPidChannel = hTrackEntry->hPidChannel;
                populateTranscodeTrackInfoFromStreamerTrackInfo(
                            &(hTrackEntry->streamerTrackInfo),
                            &(xcodePrepareSettings.videoTrack)
                            );
                videoEnabled = true;
            }
            else
            {
                BDBG_ERR(( BIP_MSG_PRE_FMT "hStreamer %p: Video Type Track is not found, its needed transcode profile enables the video output. " BIP_MSG_PRE_ARG, (void *)hStreamer ));
            }
        }

        /* Fill-up Audio PidChannel & Track Info if Audio is enabled in the caller's transcode profile. */
        if ( bipStatus == BIP_SUCCESS && !pTranscodeProfile->disableAudio )
        {
            bipStatus = BIP_Streamer_GetTrackEntry_priv( hStreamer, BIP_MediaInfoTrackType_eAudio, &hTrackEntry);
            if ( bipStatus == BIP_SUCCESS )
            {
                xcodePrepareSettings.hAudioPidChannel = hTrackEntry->hPidChannel;
                populateTranscodeTrackInfoFromStreamerTrackInfo(
                            &(hTrackEntry->streamerTrackInfo),
                            &(xcodePrepareSettings.audioTrack)
                            );
            }
            else
            {
                if ( videoEnabled )
                {
                    BDBG_WRN(( BIP_MSG_PRE_FMT "hStreamer %p: Audio Track is not found, even thought transcode profile enables audio, ignoring it!! " BIP_MSG_PRE_ARG, (void *)hStreamer ));
                    pTranscodeProfile->disableAudio = true;
                    bipStatus = BIP_SUCCESS;
                }
                else
                {
                    BDBG_ERR(( BIP_MSG_PRE_FMT "hStreamer %p: Audio Type Track is not found, its needed transcode profile enables the audio output. " BIP_MSG_PRE_ARG, (void *)hStreamer ));
                }
            }
        }

        /* Fill-in the PCR PidChannel & Track Info for Tuner inputs as transcode will need to be setup in the realtime mode. */
        if ( bipStatus == BIP_SUCCESS && inputType == BIP_StreamerInputType_eTuner )
        {
            bipStatus = BIP_Streamer_GetTrackEntry_priv( hStreamer, BIP_MediaInfoTrackType_ePcr, &hTrackEntry );
            if ( bipStatus == BIP_SUCCESS )
            {
                populateTranscodeTrackInfoFromStreamerTrackInfo(
                            &(hTrackEntry->streamerTrackInfo),
                            &(xcodePrepareSettings.pcrTrack)
                            );
                if ( hTrackEntry->hPidChannel == NULL )
                {
                    /* Since PCR track doesn't have its pidChannel handle set, it will happen when Pcr PID is same a the Vidoe PID. */
                    /* So we will use the pidChannel assigned to the VideoPid. */
                    bipStatus = BIP_Streamer_GetTrackEntry_priv( hStreamer, BIP_MediaInfoTrackType_eVideo, &hTrackEntry );
                    BDBG_ASSERT( bipStatus == BIP_SUCCESS );
                    xcodePrepareSettings.hPcrPidChannel = hTrackEntry->hPidChannel;
                }
                else
                {
                    xcodePrepareSettings.hPcrPidChannel = hTrackEntry->hPidChannel;
                }
                nonRealTime = false;
            }
            else
            {
                BDBG_ERR(( BIP_MSG_PRE_FMT "hStreamer %p: PCR Type Track is not found, its needed for enabling transcode from Tuner input. " BIP_MSG_PRE_ARG, (void *)hStreamer ));
            }
        }

        if ( bipStatus == BIP_SUCCESS )
        {
            xcodePrepareSettings.hPlayback = hStreamer->hPlayback;
            if (hStreamer->transcode.handlesState == BIP_StreamerOutputState_eSet )
            {
                xcodePrepareSettings.pNexusHandles = &hStreamer->setTranscodeHandlesApi.transcodeNexusHandles;
            }
            xcodePrepareSettings.enableRaiIndex = hStreamer->prepareSettings.enableRaiIndex;
            pTranscodeProfile->video.startSettings.nonRealTime = nonRealTime;
            populateTranscodeStreamInfoFromStreamerStreamInfo(
                                            &hStreamer->streamerStreamInfo,
                                            &transcodeStreamInfo
                                            );

            bipStatus = BIP_Transcode_Prepare(
                    hStreamer->transcode.hTranscode,
                    &transcodeStreamInfo,
                    pTranscodeProfile,
                    hStreamer->hRecpump,
                    nonRealTime,
                    &xcodePrepareSettings
                    );
        }
    }
    else
    {
        bipStatus = BIP_ERR_OUT_OF_SYSTEM_MEMORY;
    }

    if ( bipStatus == BIP_SUCCESS )
    {
        /* Success case, cache the currently used TranscodeProfile. */
        hStreamer->transcode.profile = *pTranscodeProfile;
    }
    else
    {
        if ( hStreamer->transcode.hTranscode ) BIP_Transcode_Destroy( hStreamer->transcode.hTranscode );
        hStreamer->transcode.hTranscode = NULL;
    }
    BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: state %s: BIP_Transcode_Prepare is completed, transcode.hTranscode=%p bipStatus = %s"
                BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state), (void *)hStreamer->transcode.hTranscode, BIP_StatusGetText(bipStatus) ));
    return ( bipStatus );
} /* prepareTranscode */

static BIP_Status updateStreamerSettings(
    BIP_StreamerHandle hStreamer,
    BIP_StreamerSettings *pSettings
    )
{
    BIP_Status  bipStatus = BIP_SUCCESS;
    bool        updateTranscodeProfile = false;

    if ( pSettings->pTranscodeProfile )
    {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer=%p state=%s: videoBitRate from profiles: cur=%u new=%u, cmpSize=%zu"
                    BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state), hStreamer->transcode.profile.video.settings.bitrateMax, pSettings->pTranscodeProfile->video.settings.bitrateMax, sizeof(BIP_TranscodeProfile) ));
        if ( BKNI_Memcmp( pSettings->pTranscodeProfile, &hStreamer->transcode.profile, sizeof(BIP_TranscodeProfile) ) != 0 )
        {
            /* Provided transcode profiles are different, so use this one. */
            hStreamer->transcode.profile = *pSettings->pTranscodeProfile;
            updateTranscodeProfile = true;
            BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer=%p: transcodeProfile updated!" BIP_MSG_PRE_ARG, (void *)hStreamer ));
        }
    }

    if ( pSettings->seekPositionValid )
    {
        BIP_TranscodeSettings transcodeSettings;

        /* Seek requires us to flush the pipe. We Stop & Start the Playback & Recpump here. */
        /* and then notify the Transcode object to flush the Decoders & Encoder via the SetSettings. */
        BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer=%p: seekPositionInMs=%u" BIP_MSG_PRE_ARG, (void *)hStreamer, pSettings->seekPositionInMs ));

        /* Stop the Producer & Consumer. */
        /* Note: little risk about stopping Recpump before Encoders, so may need to consider moving this to the Transcode object. */
        {
            NEXUS_Playback_Stop( hStreamer->hPlayback );
#if 0
            /* Note: we are letting transcode module do the Recpump_Stop as we can't stop a downstream consumer */
            /* until decoders & encoders are stopped. There may still be some data in the pipe. */
            NEXUS_Recpump_Stop( hStreamer->hRecpump );
#endif
            BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer=%p: Playback & Recpump are stopped!" BIP_MSG_PRE_ARG, (void *)hStreamer ));
        }

        /* Notify Transcode object to flush its pipe and restart. */
        {
            BIP_TranscodeProfile transcodeProfile;
            transcodeSettings.pTranscodeProfile = &transcodeProfile;
            BIP_Transcode_GetSettings( hStreamer->transcode.hTranscode, &transcodeSettings );
            transcodeSettings.flush = true;
            if ( updateTranscodeProfile )
            {
                transcodeSettings.pTranscodeProfile = &hStreamer->transcode.profile;
            }
            else
            {
                transcodeSettings.pTranscodeProfile = NULL;
            }
            bipStatus = BIP_Transcode_SetSettings( hStreamer->transcode.hTranscode, &transcodeSettings );
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Transcode_SetSettings Failed!" ), error, bipStatus, bipStatus );
        }

        /* Start Recpump. */
        {
            bipStatus = startNexusRecpump( hStreamer );
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "startNexusRecpump Failed!" ), error, bipStatus, bipStatus );
        }

        /* Start Playback from new position. */
        {
            bipStatus = startNexusPlayback( hStreamer, true /* seekPositionValid */, pSettings->seekPositionInMs );
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "startNexusPlayback Failed!" ), error, bipStatus, bipStatus );
        }
    }
    else if ( updateTranscodeProfile )
    {
        BIP_TranscodeSettings transcodeSettings;

        BDBG_ERR(( BIP_MSG_PRE_FMT "hStreamer=%p: Update transcodeProfile!" BIP_MSG_PRE_ARG, (void *)hStreamer ));

        transcodeSettings.pTranscodeProfile = NULL;
        BIP_Transcode_GetSettings( hStreamer->transcode.hTranscode, &transcodeSettings );

        transcodeSettings.flush = false;
        transcodeSettings.pTranscodeProfile = &hStreamer->transcode.profile;
        bipStatus = BIP_Transcode_SetSettings( hStreamer->transcode.hTranscode, &transcodeSettings );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Transcode_SetSettings Failed!" ), error, bipStatus, bipStatus );
    }
    else
    {
        /* No Changes to settings, save the new settings. */
        hStreamer->settings = *pSettings;
    }

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer=%p: StreamerSettings are updated: bipStatus=%s" BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_StatusGetText( bipStatus ) ));

    return ( bipStatus );
} /* updateStreamerSettings */

#else
static BIP_Status prepareTranscode(
        BIP_StreamerHandle hStreamer,
        BIP_StreamerInputType inputType,
        BIP_TranscodeProfile *pTranscodeProfile
        )
{
    BSTD_UNUSED(hStreamer);
    BSTD_UNUSED(inputType);
    BSTD_UNUSED(pTranscodeProfile);
    BDBG_ERR(( BIP_MSG_PRE_FMT "NEXUS Video Transcode Feature is not supported for this platform unless you are not compiling with VIDEO_ENCODER_SUPPORT flag. " BIP_MSG_PRE_ARG ));
    return (BIP_ERR_NOT_AVAILABLE);
}
static BIP_Status updateStreamerSettings(
    BIP_StreamerHandle hStreamer,
    BIP_StreamerSettings *pSettings
    )
{
    BSTD_UNUSED(hStreamer);
    BSTD_UNUSED(pSettings);
    BDBG_ERR(( BIP_MSG_PRE_FMT "NEXUS Video Transcode Feature is not supported for this platform unless you are not compiling with VIDEO_ENCODER_SUPPORT flag. " BIP_MSG_PRE_ARG ));
    return (BIP_ERR_NOT_AVAILABLE);
}
#endif /* NEXUS_HAS_VIDEO_ENCODER */

static BIP_Status prepareStreamerForRecpumpInput(
    BIP_StreamerHandle hStreamer
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;

    /*
     * Since App completely sets up how data gets to the Recpump, BIP will use following path for streaming:
     *  - Recpump -> Network
     */

    /* Start Recpump. */
    bipStatus = startNexusRecpump( hStreamer );
    if (bipStatus == BIP_SUCCESS)
    {
        hStreamer->state = BIP_StreamerState_ePrepared;
        BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: state %s: Nexus Resources are successfully prepared. "
                    BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state) ));
    }

    return ( bipStatus );
} /* prepareStreamerForRecpumpInput */

static BIP_Status prepareStreamerForTunerInput(
    BIP_StreamerHandle hStreamer
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;

    /*
     * Tuner Input Setup:
     *
     * For non-xcode case, BIP will use following path for streaming:
     *  -ParserBand -> Recpump -> Network
     *
     * For xcode case, BIP will use following path for streaming:
     *  -ParserBand -> AV Decoders -> Simple Transcode -> Recpump -> Network
     */

    /* Open Nexus Recpump as its common for both Xcode & Non-Xcode paths. */
    bipStatus = openNexusRecpump( hStreamer );
    if ( bipStatus == BIP_SUCCESS )
    {
        /* Now open the PidChannels. */
        bipStatus = openPidChannels( hStreamer );
    }
    if ( bipStatus == BIP_SUCCESS )
    {
        BIP_TranscodeProfile *pTranscodeProfile;
        /*
         * Then connect the PidChannels to either AV Decoders for Xcode Path or to Recpump for non-Xcode case.
         * Either Configure the Transcode Path: Simple Decoders, Simple Transcode.
         */
        if ( hStreamer->transcode.profileState == BIP_StreamerOutputState_eSet )
        {
            /* Xcode Path: PidChannels are connected to AV Decoders. */
            if ( hStreamer->prepareSettings.pTranscodeProfile )
            {
                /* Use the caller provided specific transcode profile. */
                pTranscodeProfile = hStreamer->prepareSettings.pTranscodeProfile;
            }
            else
            {
                /* Pick the first one from our list. */
                BIP_StreamerTranscodeProfileListEntry *pTranscodeProfileEntry;

                pTranscodeProfileEntry = BLST_Q_FIRST( &hStreamer->transcode.profileListHead);
                pTranscodeProfile = &pTranscodeProfileEntry->transcodeProfile;
            }
            bipStatus = prepareTranscode( hStreamer, hStreamer->inputType, pTranscodeProfile );
            if ( bipStatus == BIP_SUCCESS )
            {
                /*
                 * Note:
                 * We start the transcode path before the Streamer_Start() as it takes time to prime the whole transcode path.
                 * Usually there is initial latency of ~1sec in this setup.
                 */
                bipStatus = BIP_Transcode_Start( hStreamer->transcode.hTranscode, NULL );
            }
        }
        else
        {
            /* Non-Xcode path: Connect PidChannels directly to the Recpump. */
            bipStatus = addPidChannelToRecpump( hStreamer );
        }
        /* Both Xcode & Non-Xcode Paths are setup in terms of PidChannels & Xcode resources. */
        if ( bipStatus == BIP_SUCCESS )
        {
            /* Start Recpump. */
            bipStatus = startNexusRecpump( hStreamer );
        }
    /*
     * Note:
     * we DONT setup the PBIP or other streamer sub-modules (like ASP) as
     * we dont know the Socket information until _Streamer_Start().
     */
    }

    /* Check if resources are successfully acquired and then continue w/ the remaining streaming related setup. */
    if (bipStatus == BIP_SUCCESS)
    {
        hStreamer->state = BIP_StreamerState_ePrepared;
        BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: state %s: Nexus Resources are successfully prepared. "
                    BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state) ));
    }
    else
    {
        if ( hStreamer->transcode.profileState == BIP_StreamerOutputState_eSet && hStreamer->transcode.hTranscode )
        {
            BIP_Transcode_Stop( hStreamer->transcode.hTranscode );
            BIP_Transcode_Destroy( hStreamer->transcode.hTranscode );
            hStreamer->transcode.hTranscode = NULL;
        }

        /* Note: these functions only close the resources if they were valid & successfully acquired. */
        closeNexusResources( hStreamer );
    }

    return ( bipStatus );
} /* prepareStreamerForTunerInput */

static BIP_Status prepareStreamerForIpInput(
    BIP_StreamerHandle hStreamer
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;

    /*
     * Ip Input Setup:
     *
     * For non-xcode case, BIP will use following path for streaming:
     *  -BIP Player -> Playpump -> Recpump -> Network
     *
     * For xcode case, BIP will use following path for streaming:
     *  -BIP Player -> Playpump -> AV Decoders -> Simple Transcode -> Recpump -> Network
     */

    /* Open Nexus Recpump as its common for both Xcode & Non-Xcode paths. */
    bipStatus = openNexusRecpump( hStreamer );

    /* Prepare the BIP Player first so that it will allow us to open the Pid Channels that we will connect to the recpump. */
    if ( bipStatus == BIP_SUCCESS )
    {
        BIP_PlayerSettings          playerSettings;

        BIP_Player_GetDefaultSettings(&playerSettings);
        if (hStreamer->ip.inputSettings.enableAllPass)
        {
            playerSettings.playbackSettings.playpumpSettings.allPass = true;
            playerSettings.playbackSettings.playpumpSettings.acceptNullPackets = hStreamer->ip.inputSettings.dropNullPackets ? false : true;
        }
        bipStatus = BIP_Player_Prepare(hStreamer->ip.hPlayer, NULL /*&prepareSettings*/, &playerSettings, NULL/*&probeSettings*/, NULL/*&streamInfo*/, NULL /*&prepareStatus*/);
    }

    /* Now open the PidChannels. */
    if ( bipStatus == BIP_SUCCESS )
    {
        if (hStreamer->ip.inputSettings.enableAllPass)
        {
            bipStatus = openAllpassPidChannel( hStreamer );
        }
        else
        {
            bipStatus = openPidChannels( hStreamer );
        }
    }

    if ( bipStatus == BIP_SUCCESS )
    {
        BIP_TranscodeProfile *pTranscodeProfile;
        /*
         * Then connect the PidChannels to either AV Decoders for Xcode Path or to Recpump for non-Xcode case.
         * Either Configure the Transcode Path: Simple Decoders, Simple Transcode.
         */
        if ( hStreamer->transcode.profileState == BIP_StreamerOutputState_eSet )
        {
            /* Xcode Path: PidChannels are connected to AV Decoders. */
            if ( hStreamer->prepareSettings.pTranscodeProfile )
            {
                /* Use the caller provided specific transcode profile. */
                pTranscodeProfile = hStreamer->prepareSettings.pTranscodeProfile;
            }
            else
            {
                /* Pick the first one from our list. */
                BIP_StreamerTranscodeProfileListEntry *pTranscodeProfileEntry;

                pTranscodeProfileEntry = BLST_Q_FIRST( &hStreamer->transcode.profileListHead);
                pTranscodeProfile = &pTranscodeProfileEntry->transcodeProfile;
            }
            bipStatus = prepareTranscode( hStreamer, hStreamer->inputType, pTranscodeProfile );
            if ( bipStatus == BIP_SUCCESS )
            {
                /*
                 * Note:
                 * We start the transcode path before the Streamer_Start() as it takes time to prime the whole transcode path.
                 * Usually there is initial latency of ~1sec in this setup.
                 */
                bipStatus = BIP_Transcode_Start( hStreamer->transcode.hTranscode, NULL );
            }
        }
        else
        {
            /* Non-Xcode path: Connect PidChannels directly to the Recpump. */
            bipStatus = addPidChannelToRecpump( hStreamer );
        }
        /* Both Xcode & Non-Xcode Paths are setup in terms of PidChannels & Xcode resources. */
        if ( bipStatus == BIP_SUCCESS )
        {
            /* Start Recpump. */
            bipStatus = startNexusRecpump( hStreamer );
        }
        if ( bipStatus == BIP_SUCCESS )
        {
            bipStatus = BIP_Player_Start(hStreamer->ip.hPlayer, NULL);
        }
    }

    /* Check if resources are successfully acquired and then continue w/ the remaining streaming related setup. */
    if (bipStatus == BIP_SUCCESS)
    {
        hStreamer->state = BIP_StreamerState_ePrepared;
        BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: state %s: Nexus Resources are successfully prepared. "
                    BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state) ));
    }
    else
    {
        BDBG_ERR(( BIP_MSG_PRE_FMT "hStreamer %p: state %s: Failed to prepare BIP & Nexus Resources."
                    BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state) ));
        if ( hStreamer->transcode.profileState == BIP_StreamerOutputState_eSet && hStreamer->transcode.hTranscode )
        {
            BIP_Transcode_Stop( hStreamer->transcode.hTranscode );
            BIP_Transcode_Destroy( hStreamer->transcode.hTranscode );
            hStreamer->transcode.hTranscode = NULL;
        }

        /* Note: these functions only close the resources if they were valid & successfully acquired. */
        closeNexusResources( hStreamer );
    }

    return ( bipStatus );
} /* prepareStreamerForIpInput */

static BIP_Status prepareStreamerForFileInput(
    BIP_StreamerHandle hStreamer
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;

    /* Determine whether Streamer should use Playback or Direct Network path for Streaming from File Input. */
    {
        bool feedUsingPlaybackChannel = false;

        if ( hStreamer->transcode.profileState == BIP_StreamerOutputState_eSet )
        {
            /* App has set the transcodeProfile, so it wants us to encode the media file before we stream it. */
            /* This will require us to decode & re-encode the media file. And thus we use the playback path for this flow. */
            feedUsingPlaybackChannel = true;
        }
        else if (hStreamer->output.settings.enableStreamingUsingPlaybackCh)
        {
            feedUsingPlaybackChannel = true;
        }
        else
        {
            /*
             * For non-xcode cases: determine if we should feed the file stream using the transport playback path vs direct network path.
             * All non-TS & non-PES formats can't be fed thru the Playback path (as h/w doesn't support containers other than TS/PES/ES).
             */

            /* if Pacing is enabled for TS format, use Playback path. */
            if ( hStreamer->file.inputSettings.enableHwPacing && hStreamer->streamerStreamInfo.transportType == NEXUS_TransportType_eTs &&
                    hStreamer->track.inputState == BIP_StreamerInputState_eSet
               )
            {
                feedUsingPlaybackChannel = true;
            }
            /* if we have to dropNull packets for TS format, use Playback path. */
            else if ( hStreamer->file.inputSettings.dropNullPackets == true && hStreamer->streamerStreamInfo.transportType == NEXUS_TransportType_eTs &&
                    hStreamer->track.inputState == BIP_StreamerInputState_eSet
                    )
            {
                feedUsingPlaybackChannel = true;
            }
            /* Also use Playback path for MPTS stream when app provided individual tracks, & hasn't set allPass flag. */
            else if ( hStreamer->streamerStreamInfo.numberOfTrackGroups > 1  &&
                    hStreamer->track.inputState == BIP_StreamerInputState_eSet &&
                    hStreamer->file.inputSettings.enableAllPass == false &&
                    (hStreamer->streamerStreamInfo.transportType == NEXUS_TransportType_eTs ||
                     hStreamer->streamerStreamInfo.transportType == NEXUS_TransportType_eMpeg2Pes) )
            {
                feedUsingPlaybackChannel = true;
            }
            else if (hStreamer->offloadStreamerToAsp)
            {
                feedUsingPlaybackChannel = true;
            }
            /* TODO: */
            /* Also use Playback path if input stream has timestamps & output doesn't as Playback path will remove it. */
            /* Also use Playback path if input stream doesn't timestamps & output settings has it, as Playback path will add/replace it. */
            else
            {
                /* Otherwise, we either can't or dont need to use the h/w Playback path. */
                feedUsingPlaybackChannel = false;
            }
        }
        BDBG_MSG(( BIP_MSG_PRE_FMT " FeedUsingPlaybackPath: %s, enableHwPacing %d, format %d, dropNullPackets %d tracksAdded %d, numberOfPrograms %d, enableAllPass %d"
                    BIP_MSG_PRE_ARG,
                    (feedUsingPlaybackChannel == true) ? "Y":"N",
                    hStreamer->file.inputSettings.enableHwPacing, hStreamer->streamerStreamInfo.transportType,
                    hStreamer->file.inputSettings.dropNullPackets, hStreamer->track.inputState,
                    hStreamer->streamerStreamInfo.numberOfTrackGroups, hStreamer->file.inputSettings.enableAllPass
                 ));
        hStreamer->file.feedUsingPlaybackChannel = feedUsingPlaybackChannel;
    }

    if ( hStreamer->file.feedUsingPlaybackChannel == false )
    {
        /*
         * If logic above determined that we dont need to or can't use the Playback path for file input,
         * then, we will setup the Direct network Streaming path: BIP will use File -> BSD Sockets for streaming.
         *
         * Note: NO setup is needed in the Nexus h/w pipe for direct network streaming.
         */
        bipStatus = BIP_SUCCESS;
    }
    else
    {
        /*
         * Playback Path Setup:
         *
         * For non-xcode case, BIP will use following path for streaming:
         *  -File -> Nexus Playback -> Playpump -> Recpump -> Network
         *
         * For xcode case, BIP will use following path for streaming:
         *  -File -> Nexus Playback -> Playpump -> Decoders -> Transcodes -> Mux -> Playback -> Recpump -> Network
         */

        /* Open Nexus Playback & Recpump as its common for both Xcode & Non-Xcode paths. */
        bipStatus = openNexusPlaybackAndRecpump( hStreamer );
        if ( bipStatus == BIP_SUCCESS )
        {
            /* Now open the PidChannels. */
            if (hStreamer->file.inputSettings.enableAllPass)
            {
                bipStatus = openAllpassPidChannel( hStreamer );
            }
            else
            {
                bipStatus = openPidChannels( hStreamer );
            }
            if ( bipStatus == BIP_SUCCESS )
            {
                BIP_TranscodeProfile *pTranscodeProfile;
                /*
                 * Then connect the PidChannels to either AV Decoders for Xcode Path or to Recpump for non-Xcode case.
                 * Either Configure the Transcode Path: Simple Decoders, Simple Transcode.
                 */
                if ( hStreamer->transcode.profileState == BIP_StreamerOutputState_eSet )
                {
                    /* Xcode Path: PidChannels are successfully opened. Now pick a transcode profile. */
                    if ( hStreamer->prepareSettings.pTranscodeProfile )
                    {
                        /* Use the caller provided specific transcode profile. */
                        pTranscodeProfile = hStreamer->prepareSettings.pTranscodeProfile;
                    }
                    else
                    {
                        /* Pick the first one from our list. */
                        BIP_StreamerTranscodeProfileListEntry *pTranscodeProfileEntry;

                        pTranscodeProfileEntry = BLST_Q_FIRST( &hStreamer->transcode.profileListHead);
                        pTranscodeProfile = &pTranscodeProfileEntry->transcodeProfile;
                    }
                    bipStatus = prepareTranscode( hStreamer, hStreamer->inputType, pTranscodeProfile );
                    if ( bipStatus == BIP_SUCCESS )
                    {
                        /*
                         * Note:
                         * We start the transcode path before the Streamer_Start() as it takes time to prime the whole transcode path.
                         * Usually there is initial latency of ~1sec in this setup.
                         */
                        bipStatus = BIP_Transcode_Start( hStreamer->transcode.hTranscode, NULL );
                    }
                }
                else
                {
                    /* Non-Xcode path: Connect PidChannels directly to the Recpump. */
                    bipStatus = addPidChannelToRecpump( hStreamer );
                }
                /* Both Xcode & Non-Xcode Paths are setup in terms of PidChannels & Xcode resources. */
                if ( bipStatus == BIP_SUCCESS )
                {
                    /* Start the Playback & Recpump. */
                    bipStatus = startNexusRecpump( hStreamer );
                    if ( bipStatus == BIP_SUCCESS )
                    {
                        bipStatus = startNexusPlayback( hStreamer, hStreamer->prepareApi.pSettings->seekPositionValid, hStreamer->prepareApi.pSettings->seekPositionInMs );
                    }
                }
            }
            /*
             * Note:
             * we DONT setup the PBIP or other streamer sub-modules (like ASP) as
             * we dont know the Socket information until _Streamer_Start().
             */
        }
    } /* Playback Path setup case. */

        /* Check if resources are successfully acquired and then continue w/ the remaining streaming related setup. */
    if (bipStatus == BIP_SUCCESS)
    {
        hStreamer->state = BIP_StreamerState_ePrepared;
        BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: state %s: Nexus Resources are successfully prepared. "
                    BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state) ));
    }
    else
    {
        if ( hStreamer->transcode.profileState == BIP_StreamerOutputState_eSet && hStreamer->transcode.hTranscode )
        {
            BIP_Transcode_Stop( hStreamer->transcode.hTranscode );
            BIP_Transcode_Destroy( hStreamer->transcode.hTranscode );
            hStreamer->transcode.hTranscode = NULL;
        }

        /* Note: these functions only close the resources if they were valid & successfully acquired. */
        closeNexusResources( hStreamer );
    }

    return ( bipStatus );
} /* prepareStreamerForFileInput */

void processStreamerState(
    void *hObject,
    int value,
    BIP_Arb_ThreadOrigin threadOrigin
    )
{
    BIP_StreamerHandle hStreamer = hObject;               /* Streamer object handle */
    BIP_ArbHandle           hArb;
    BIP_Status              brc = BIP_ERR_INTERNAL;
    BIP_Status              completionStatus = BIP_ERR_INTERNAL;

    BSTD_UNUSED(value);

    BDBG_ASSERT(hStreamer);
    BDBG_OBJECT_ASSERT( hStreamer, BIP_Streamer);

    /*
     ***************************************************************************************************************
     * Streamer State Machine Processing:
     *
     * Note: Streamer Settings related APIs are required to be called before the _Streamer_Start().
     * These are _Set*Input, _SetOutput, _AddTracks, _SetProgram, _SetResponseHeaders, etc.
     * In these APIs, we will just cache the caller provided settings but not acquire any
     * Nexus Resources needed for streaming. Once _Start() is called, then we will acquire & setup
     * the required resources for streaming from a input to a particular output method.
     *
     ***************************************************************************************************************
     */

    B_Mutex_Lock( hStreamer->hStateMutex );
    BDBG_MSG(( BIP_MSG_PRE_FMT "ENTRY ---> hStreamer %p: state %s"
                BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state) ));

    if (BIP_Arb_IsNew(hArb = hStreamer->getSettingsApi.hArb))
    {
        /* App is request current Streamer settings. */
        BIP_Arb_AcceptRequest(hArb);

        /* Return the current cached settings. */
        if ( hStreamer->getSettingsApi.pSettings->pTranscodeProfile )
            *hStreamer->getSettingsApi.pSettings->pTranscodeProfile = hStreamer->transcode.profile;
        hStreamer->getSettingsApi.pSettings->seekPositionValid = false;

        /* We are done this API Arb, so set its completion status. */
        hStreamer->completionStatus = BIP_SUCCESS;
        BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: GetSettings Arb request is complete: state %s!"
                    BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state) ));
        BIP_Arb_CompleteRequest( hArb, hStreamer->completionStatus);
    }
    else if (BIP_Arb_IsNew(hArb = hStreamer->getStatusApi.hArb))
    {
        /* App is request current Streamer settings. */
        BIP_Arb_AcceptRequest(hArb);

        /* Return the current status. */
        hStreamer->getStatusApi.pStatus->stats = hStreamer->stats;

        /* We are done this API Arb, so set its completion status. */
        hStreamer->completionStatus = BIP_SUCCESS;
        BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: GetStatus Arb request is complete: state %s!"
                    BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state) ));
        BIP_Arb_CompleteRequest( hArb, hStreamer->completionStatus);
    }
    else if (BIP_Arb_IsNew(hArb = hStreamer->printStatusApi.hArb))
    {
        /* App is request to print Streamer stats. */
        BIP_Arb_AcceptRequest(hArb);

        printStreamerStatus( hStreamer );

        if ( hStreamer->transcode.hTranscode ) BIP_Transcode_PrintStatus( hStreamer->transcode.hTranscode );

        hStreamer->completionStatus = BIP_SUCCESS;
        BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: printStatus Arb request is complete: state %s!"
                    BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state) ));
        BIP_Arb_CompleteRequest( hArb, hStreamer->completionStatus);
    }
    else if (BIP_Arb_IsNew(hArb = hStreamer->setSettingsApi.hArb))
    {
        BIP_Arb_AcceptRequest(hArb);

        hStreamer->completionStatus = updateStreamerSettings( hStreamer, hStreamer->setSettingsApi.pSettings );

        BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: SetSettings Arb request is complete : state %s, status=%s"
                    BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state), BIP_StatusGetText( hStreamer->completionStatus ) ));
        BIP_Arb_CompleteRequest( hArb, hStreamer->completionStatus);
    }
    else if (BIP_Arb_IsNew(hArb = hStreamer->fileInputSettingsApi.hArb))
    {
        /* We only allow streamer sub-state changes in the Idle state. */
        BIP_StreamerFileInputSettings *pSettings = hStreamer->fileInputSettingsApi.pFileInputSettings;

        if (hStreamer->state != BIP_StreamerState_eIdle)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hStreamer %p: Calling BIP_Arb_RejectRequest(): BIP_Streamer_SetFileInputSettings not allowed in this state: %s, Streamer must be in the Idle state"
                        BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state)));

            hStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hStreamer->completionStatus);
        }
        /* Do detailed fileInputSettings validation. */
        else if ( pSettings->endByteOffset > 0 && pSettings->endByteOffset <= pSettings->beginByteOffset )
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hStreamer %p: Incorrect byte offset values: begin=%"PRId64 " end=%"PRId64
                        BIP_MSG_PRE_ARG, (void *)hStreamer, pSettings->beginByteOffset, pSettings->endByteOffset ));
            hStreamer->completionStatus = BIP_ERR_INVALID_PARAMETER;
            BIP_Arb_RejectRequest(hArb, hStreamer->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);
            /* We have valid input, reset any previously_set the input settings. */
            /* This can happen if app had set the input settings and then didn't start the streamer due to */
            /* some app error. We would cleanup the streamer resources associated with the older settings.  */
            resetStreamerState(hStreamer);

            hStreamer->file.hMediaFileAbsolutePathName = BIP_String_CreateFromChar( hStreamer->fileInputSettingsApi.pMediaFileAbsolutePathName );
            hStreamer->file.hPlaySpeed = BIP_String_CreateFromChar( hStreamer->fileInputSettingsApi.pFileInputSettings->playSpeed );

            /* If we successfully allocated & cached the stringHandles for needed input settings, then cache the remaining settings. */
            if ( hStreamer->file.hMediaFileAbsolutePathName && hStreamer->file.hPlaySpeed )
            {
                hStreamer->file.inputSettings = *hStreamer->fileInputSettingsApi.pFileInputSettings;
                hStreamer->inputType = BIP_StreamerInputType_eFile;
                hStreamer->streamerStreamInfo = *hStreamer->fileInputSettingsApi.pStreamerStreamInfo;

                hStreamer->file.inputState = BIP_StreamerInputState_eSet;
                hStreamer->completionStatus = BIP_SUCCESS;

                BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: state %s: FileInputSettings: filePath %s, transportType %d, pacing %s, speed %s"
                            BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state),
                            BIP_String_GetString( hStreamer->file.hMediaFileAbsolutePathName ),
                            hStreamer->streamerStreamInfo.transportType,
                            hStreamer->file.inputSettings.enableHwPacing ? "Y" : "N",
                            BIP_String_GetString( hStreamer->file.hPlaySpeed )
                            ));
            }
            else
            {
                /* BIP_String_Create* would have failed above, so set the status to indicate that. */
                hStreamer->completionStatus = BIP_ERR_OUT_OF_SYSTEM_MEMORY;
                BDBG_ERR(( BIP_MSG_PRE_FMT "hStreamer %p: state %s: BIP_String_Create Failed for FileInput params"
                            BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state)));
                resetStreamerFileInputState( hStreamer );
            }
            BIP_Arb_CompleteRequest( hArb, hStreamer->completionStatus );
        }
    }
    else if (BIP_Arb_IsNew(hArb = hStreamer->tunerInputSettingsApi.hArb))
    {
        if (hStreamer->state != BIP_StreamerState_eIdle)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hStreamer %p: Calling BIP_Arb_RejectRequest(): BIP_Streamer_SetTunerInputSettings not allowed in this state: %s, Streamer must be in the Idle state"
                        BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state)));
            hStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hStreamer->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);
            hStreamer->completionStatus = BIP_ERR_OUT_OF_SYSTEM_MEMORY;

            /* Reset any previously set inputs. */
            resetStreamerState(hStreamer);

            /* Now save the remaining settings */
            hStreamer->tuner.inputSettings = *hStreamer->tunerInputSettingsApi.pTunerInputSettings;
            hStreamer->tuner.hParserBand = hStreamer->tunerInputSettingsApi.hParserBand;
            hStreamer->streamerStreamInfo = *hStreamer->tunerInputSettingsApi.pStreamerStreamInfo;
            hStreamer->inputType = BIP_StreamerInputType_eTuner;

            /* Update the Streamer state to InputSet */
            hStreamer->tuner.inputState = BIP_StreamerInputState_eSet;
            hStreamer->completionStatus = BIP_SUCCESS;

            BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: state %s: pTunerInputSettings: parserBand %p"
                        BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state), (void *)hStreamer->tuner.hParserBand
                     ));
            BIP_Arb_CompleteRequest( hArb, hStreamer->completionStatus );
        }
    }
    else if (BIP_Arb_IsNew(hArb = hStreamer->ipInputSettingsApi.hArb))
    {
        if (hStreamer->state != BIP_StreamerState_eIdle)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hStreamer %p: Calling BIP_Arb_RejectRequest(): BIP_Streamer_SetIpInputSettings not allowed in this state: %s, Streamer must be in the Idle state"
                        BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state)));
            hStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hStreamer->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);
            hStreamer->completionStatus = BIP_ERR_OUT_OF_SYSTEM_MEMORY;

            /* Reset any previously set inputs. */
            resetStreamerState(hStreamer);

            /* Now save the remaining settings */
            hStreamer->ip.inputSettings = *hStreamer->ipInputSettingsApi.pIpInputSettings;
            hStreamer->ip.hPlayer = hStreamer->ipInputSettingsApi.hPlayer;
            hStreamer->streamerStreamInfo = *hStreamer->ipInputSettingsApi.pStreamerStreamInfo;
            hStreamer->inputType = BIP_StreamerInputType_eIp;

            /* Update the Streamer state to InputSet */
            hStreamer->ip.inputState = BIP_StreamerInputState_eSet;
            hStreamer->completionStatus = BIP_SUCCESS;

            BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: state %s: pIpInputSettings: hPlayer=%p"
                        BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state), (void *)hStreamer->ip.hPlayer
                     ));
            BIP_Arb_CompleteRequest( hArb, hStreamer->completionStatus );
        }
    }
    else if (BIP_Arb_IsNew(hArb = hStreamer->recpumpInputSettingsApi.hArb))
    {
        if (hStreamer->state != BIP_StreamerState_eIdle)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hStreamer %p: Calling BIP_Arb_RejectRequest(): BIP_Streamer_SetRecpumpInputSettings not allowed in this state: %s, Streamer must be in the Idle state"
                        BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state)));
            hStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hStreamer->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);

            /* Reset any previously set inputs. */
            resetStreamerState(hStreamer);

            /* We cache the settings into streamer object.  API side code has already verified the Settings. */
            hStreamer->hRecpump = hStreamer->recpumpInputSettingsApi.hRecpump;
            hStreamer->recpump.inputSettings = *hStreamer->recpumpInputSettingsApi.pRecpumpInputSettings;
            hStreamer->inputType = BIP_StreamerInputType_eRecpump;
            hStreamer->recpump.inputState = BIP_StreamerInputState_eSet;

            hStreamer->completionStatus = BIP_SUCCESS;
            BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: SetRecpumpInputSettings Arb request is complete : state %s!"
                        BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state) ));
            BIP_Arb_CompleteRequest( hArb, hStreamer->completionStatus );
        }
    }
    else if (BIP_Arb_IsNew(hArb = hStreamer->outputSettingsApi.hArb))
    {
        /*
         * We cache the settings into streamer object.
         * Note: API side code has already verified the requied Setting parameters!
         * Input & Output Settings can be set in any order.
         */
        if (hStreamer->state != BIP_StreamerState_eIdle)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hStreamer %p: BIP_Streamer_SetOutputSettings not allowed in this state: %s"
                        BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state)));
            hStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hStreamer->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);

            /* Save the output settings */
            hStreamer->output.settings = *hStreamer->outputSettingsApi.pOutputSettings;
            hStreamer->output.streamerProtocol = hStreamer->outputSettingsApi.streamerProtocol;
            hStreamer->output.state = BIP_StreamerOutputState_eSet;
            hStreamer->completionStatus = BIP_SUCCESS;
            BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: output.settings are cached: timestamp=%s, protocol=%d"
                        BIP_MSG_PRE_ARG, (void *)hStreamer, hStreamer->output.settings.mpeg2Ts.enableTransportTimestamp?"Y":"N",
                        hStreamer->output.streamerProtocol
                        ));
            BIP_Arb_CompleteRequest( hArb, hStreamer->completionStatus);
        }
    }
    else if (BIP_Arb_IsNew(hArb = hStreamer->addTrackApi.hArb))
    {
        if (hStreamer->state != BIP_StreamerState_eIdle)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hStreamer %p: BIP_Streamer_AddTrack() is only allowed in Idle state, current state: %s"
                        BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state)));
            hStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hStreamer->completionStatus);
        }
        else if ( hStreamer->recpump.inputState == BIP_StreamerInputState_eSet )
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hStreamer %p: BIP_Streamer_AddTrack() is not allowed for RecpumpInput type!." BIP_MSG_PRE_ARG, (void *)hStreamer ));
            hStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hStreamer->completionStatus);
        }
        else if (
                (hStreamer->inputType == BIP_StreamerInputType_eIp && hStreamer->ip.inputSettings.enableAllPass) ||
                (hStreamer->inputType == BIP_StreamerInputType_eFile && hStreamer->file.inputSettings.enableAllPass)
                )
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hStreamer %p: BIP_Streamer_AddTrack() is not allowed if enableAllPass is set!." BIP_MSG_PRE_ARG, (void *)hStreamer ));
            hStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hStreamer->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);

            /* Add the Track settings to a list of tracks. */
            hStreamer->completionStatus = addTrackToList( hStreamer, hStreamer->addTrackApi.pStreamerTrackInfo , hStreamer->addTrackApi.pTrackSettings );
            if ( hStreamer->completionStatus == BIP_SUCCESS )
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: BIP_Streamer_AddTrack(): trackId %d, type %d is successful : state %s!"
                            BIP_MSG_PRE_ARG, (void *)hStreamer,
                            hStreamer->addTrackApi.pStreamerTrackInfo->trackId,
                            hStreamer->addTrackApi.pStreamerTrackInfo->type,
                            BIP_STREAMER_STATE(hStreamer->state)
                            ));
            }
            else
            {
                /* Failed to add this track, return error back. Also, remove any tracks that were previously successfully added. */
                removeAllTracksFromList( hStreamer );
                BDBG_ERR(( BIP_MSG_PRE_FMT "hStreamer %p: state %s: BIP_Streamer_AddTrack Failed"
                            BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state)));
            }
            BIP_Arb_CompleteRequest( hArb, hStreamer->completionStatus);
        }
    }
    else if (BIP_Arb_IsNew(hArb = hStreamer->addTranscodeProfileApi.hArb))
    {
        BIP_TranscodeProfile *pTranscodeProfile = hStreamer->addTranscodeProfileApi.pTranscodeProfile;

        if (hStreamer->state != BIP_StreamerState_eIdle)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hStreamer %p: BIP_Streamer_AddTranscodeProfile() is only allowed in Idle state, current state: %s"
                        BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state)));
            hStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hStreamer->completionStatus);
        }
        else if ( hStreamer->recpump.inputState == BIP_StreamerInputState_eSet )
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hStreamer %p: BIP_Streamer_AddTranscodeProfile() is not allowed for RecpumpInput type!." BIP_MSG_PRE_ARG, (void *)hStreamer ));
            hStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hStreamer->completionStatus);
        }
        /* Reject if output settings are not set as we need to know streamerProtocol & based on that allow multiple AddTranscodeProfiles call (for HLS & MPEG DASH). */
        else if ( hStreamer->output.state == BIP_StreamerOutputState_eNotSet )
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hStreamer %p: BIP_Streamer_AddTranscodeProfile() is not allowed when output.settings are not set!." BIP_MSG_PRE_ARG, (void *)hStreamer ));
            hStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hStreamer->completionStatus);
        }
        /* Reject for non-TS transcode container formats until we support them. */
        else if ( pTranscodeProfile->containerType != NEXUS_TransportType_eTs )
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hStreamer %p: BIP_Streamer_AddTranscodeProfile(): encoding of streaming output is not YET allowed for non MPEG TS container types: %d"
                        BIP_MSG_PRE_ARG, (void *)hStreamer, hStreamer->addTranscodeProfileApi.pTranscodeProfile->containerType ));
            hStreamer->completionStatus = BIP_ERR_INVALID_PARAMETER;
            BIP_Arb_RejectRequest(hArb, hStreamer->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);

            /* Add transcodeProfile settings to a list of profiles. */
            hStreamer->completionStatus = addTranscodeProfileToList( hStreamer, pTranscodeProfile );
            if ( hStreamer->completionStatus == BIP_SUCCESS )
            {
                hStreamer->transcode.profileState = BIP_StreamerOutputState_eSet;
                BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: BIP_Streamer_AddTranscodeProfile() for output containerType %d is successful" BIP_MSG_PRE_ARG, (void *)hStreamer, pTranscodeProfile->containerType ));
            }
            else
            {
                /* Failed to add transcodeProfile, return error back. Also, remove any transcodeProfiles that were previously successfully added. */
                removeAllTranscodeProfilesFromList( hStreamer );
                BDBG_ERR(( BIP_MSG_PRE_FMT "hStreamer %p: state %s: BIP_Streamer_AddTranscodeProfile Failed"
                            BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state)));
            }
            BIP_Arb_CompleteRequest( hArb, hStreamer->completionStatus);
        }
    }
    else if (BIP_Arb_IsNew(hArb = hStreamer->setTranscodeHandlesApi.hArb))
    {
        if (hStreamer->state != BIP_StreamerState_eIdle)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hStreamer %p: BIP_Streamer_AddTranscodeProfile() is only allowed in Idle state, current state: %s"
                        BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state)));
            hStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hStreamer->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);

            /* We just cache the app provided handles and use them during the Transcode_Prepare call. */
            hStreamer->completionStatus = BIP_SUCCESS;
            hStreamer->transcode.handlesState = BIP_StreamerOutputState_eSet;
            BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: BIP_Streamer_AddTranscodeProfile() for output containerType %d is successful" BIP_MSG_PRE_ARG, (void *)hStreamer, hStreamer->transcode.profile.containerType));
            BIP_Arb_CompleteRequest( hArb, hStreamer->completionStatus);
        }
    }
    else if (BIP_Arb_IsNew(hArb = hStreamer->prepareApi.hArb))
    {
        /* App is starting the Server, make sure we are in the correct state to do so! */
        if (hStreamer->state != BIP_StreamerState_eIdle)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hStreamer %p: BIP_Streamer_Prepare() is only allowed in Idle state, current state: %s"
                        BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state)));
            hStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hStreamer->completionStatus);
        }
        /* Make sure required settings for streaming are provided, can't start streamer otherwise! */
        else if (hStreamer->file.inputState == BIP_StreamerInputState_eNotSet &&
                 hStreamer->tuner.inputState == BIP_StreamerInputState_eNotSet &&
                 hStreamer->recpump.inputState == BIP_StreamerInputState_eNotSet &&
                 hStreamer->output.state == BIP_StreamerOutputState_eNotSet
                )
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hStreamer %p: BIP_Streamer_Prepare() is not allowed when Input or Output are not set!." BIP_MSG_PRE_ARG, (void *)hStreamer ));
            hStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hStreamer->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);
            hStreamer->completionStatus = BIP_INF_IN_PROGRESS;

            /* We have confirmed that all streaming related states are valid and thus their settings are in place. */
            /* Note: we dont acquire & setup any Nexus streaming resources until caller invokes the _ProcessRequest(). */
            hStreamer->prepareSettings = *hStreamer->prepareApi.pSettings;

            /* Determine if we can offload this streamer to ASP. */
            if (
                    hStreamer->output.settings.enableHwOffload &&                               /* App has enabled it, & */
                    hStreamer->streamerStreamInfo.transportType == NEXUS_TransportType_eTs &&   /* its a format ASP can stream out, & */
                    hStreamer->transcode.profileState == BIP_StreamerOutputState_eNotSet )      /* transcode is not enabled (TODO: only HLS can't be streamed out by ASP, but for now disable all!) */
            {
                hStreamer->offloadStreamerToAsp = true;
            }

            if ( hStreamer->file.inputState == BIP_StreamerInputState_eSet )
            {
                hStreamer->completionStatus = prepareStreamerForFileInput( hStreamer );
            }
            else if ( hStreamer->tuner.inputState == BIP_StreamerInputState_eSet )
            {
                hStreamer->completionStatus = prepareStreamerForTunerInput( hStreamer );
            }
            else if ( hStreamer->ip.inputState == BIP_StreamerInputState_eSet )
            {
                hStreamer->completionStatus = prepareStreamerForIpInput( hStreamer );
            }
            else if ( hStreamer->recpump.inputState == BIP_StreamerInputState_eSet )
            {
                hStreamer->completionStatus = prepareStreamerForRecpumpInput( hStreamer );
            }
            if ( hStreamer->completionStatus == BIP_SUCCESS )
            {
                hStreamer->state = BIP_StreamerState_ePrepared;
                BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: BIP_Streamer_Prepare successful: state %s!" BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state) ));
            }
            else
            {
                BDBG_ERR(( BIP_MSG_PRE_FMT "hStreamer %p: BIP_Streamer_Prepare() Failed: streamer input is not supported, current state: %s" BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state)));
            }
            BIP_Arb_CompleteRequest(hArb, hStreamer->completionStatus);
        }
    }
    else if (BIP_Arb_IsNew(hArb = hStreamer->startApi.hArb))
    {
        /* Caller is starting streamer, we must be in the Prepared state. */
        if (hStreamer->state != BIP_StreamerState_ePrepared)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hStreamer %p: BIP_Streamer_Start() is only allowed in Prepared state, current state: %s" BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state)));
            hStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hStreamer->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);
            hStreamer->completionStatus = BIP_INF_IN_PROGRESS;

            /* We have confirmed that all streaming related states are valid and thus their settings are in place. */
            /* All streaming resources are already setup in the prepared state, so lets start streamer. */
            hStreamer->startSettings = *hStreamer->startApi.pSettings;

            hStreamer->completionStatus = BIP_SUCCESS;
            if ( hStreamer->completionStatus == BIP_SUCCESS )
            {
                hStreamer->state = BIP_StreamerState_eStreaming;
                BIP_MSG_SUM(( BIP_MSG_PRE_FMT "StreamerStarted: " BIP_STREAMER_PRINTF_FMT
                            BIP_MSG_PRE_ARG, BIP_STREAMER_PRINTF_ARG(hStreamer)));
            }
            else
            {
                BDBG_ERR(( BIP_MSG_PRE_FMT "hStreamer %p: BIP_Streamer_Start() Failed: current state: %s" BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state)));
            }
            BIP_Arb_CompleteRequest(hArb, hStreamer->completionStatus);
            /* NOTE: we change to a temporary SetupComplete state (w/o completing the ARB) */
            /* This way we let the per streaming protocol state logic below do any streaming start related processing. */
            hStreamer->state = BIP_StreamerState_eStreaming;
        }
    }
    else if (BIP_Arb_IsNew(hArb = hStreamer->stopApi.hArb))
    {
        {
            BIP_Arb_AcceptRequest(hArb);
            hStreamer->completionStatus = BIP_INF_IN_PROGRESS;

            /* Note: we also Close Nexus resources during Streamer_Stop() as these resources were allocated during Streamer_Prepare(). */
            stopAndUnPrepare( hStreamer );
            hStreamer->completionStatus = BIP_SUCCESS;

            hStreamer->state = BIP_StreamerState_eIdle;
            BIP_Arb_CompleteRequest(hArb, hStreamer->completionStatus);
            BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: BIP_Streamer_Stop complete: state %s!"
                        BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state) ));

        }
    }
    else if (BIP_Arb_IsNew(hArb = hStreamer->destroyApi.hArb))
    {
        BIP_Arb_AcceptRequest(hArb);
        hStreamer->completionStatus = BIP_INF_IN_PROGRESS;

        BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: Accepted _Destroy Arb: state %s!"
                    BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state) ));

        if ( hStreamer->state != BIP_StreamerState_eIdle )
        {
            /* Streamer_Stop() wasn't called. */
            stopAndUnPrepare( hStreamer );
        }
        BIP_Arb_CompleteRequest(hArb, BIP_SUCCESS);
    }

    /*
     * Done with state processing. We have to unlock state machine before asking Arb to do any deferred callbacks!
     */
    B_Mutex_Unlock( hStreamer->hStateMutex );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Finished Processing  State for hStreamer %p: state %s, before issuing the callbacks with completionStatus 0x%x"
            BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state), completionStatus ));

    /* Tell ARB to do any deferred work. */
    brc = BIP_Arb_DoDeferred( hStreamer->startApi.hArb, threadOrigin );
    BDBG_ASSERT( brc == BIP_SUCCESS );


    BDBG_MSG(( BIP_MSG_PRE_FMT "EXIT <--- hStreamer %p: state %s"
                BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_STREAMER_STATE(hStreamer->state)));
    return;
} /* processStreamerState */

void processStreamerStateFromArb(
    void *hObject,
    int value
    )
{
    processStreamerState( (BIP_StreamerHandle) hObject, value, BIP_Arb_ThreadOrigin_eArb);
}

void BIP_Streamer_GetDefaultSettings(
    BIP_StreamerSettings *pSettings
    )
{
    BKNI_Memset( pSettings, 0, sizeof( BIP_StreamerSettings ));
}

static void streamerDestroy(
    BIP_StreamerHandle hStreamer
    )
{
    if (!hStreamer) return;
    BDBG_MSG(( BIP_MSG_PRE_FMT "Destroying hStreamer %p" BIP_MSG_PRE_ARG, (void *)hStreamer ));

    resetStreamerState( hStreamer );

    if (hStreamer->getStatusApi.hArb) BIP_Arb_Destroy(hStreamer->getStatusApi.hArb);
    if (hStreamer->printStatusApi.hArb) BIP_Arb_Destroy(hStreamer->printStatusApi.hArb);
    if (hStreamer->setSettingsApi.hArb) BIP_Arb_Destroy(hStreamer->setSettingsApi.hArb);
    if (hStreamer->getSettingsApi.hArb) BIP_Arb_Destroy(hStreamer->getSettingsApi.hArb);
    if (hStreamer->fileInputSettingsApi.hArb) BIP_Arb_Destroy(hStreamer->fileInputSettingsApi.hArb);
    if (hStreamer->tunerInputSettingsApi.hArb) BIP_Arb_Destroy(hStreamer->tunerInputSettingsApi.hArb);
    if (hStreamer->ipInputSettingsApi.hArb) BIP_Arb_Destroy(hStreamer->ipInputSettingsApi.hArb);
    if (hStreamer->recpumpInputSettingsApi.hArb) BIP_Arb_Destroy(hStreamer->recpumpInputSettingsApi.hArb);
    if (hStreamer->outputSettingsApi.hArb) BIP_Arb_Destroy(hStreamer->outputSettingsApi.hArb);
    if (hStreamer->addTrackApi.hArb) BIP_Arb_Destroy(hStreamer->addTrackApi.hArb);
    if (hStreamer->addTranscodeProfileApi.hArb) BIP_Arb_Destroy(hStreamer->addTranscodeProfileApi.hArb);
    if (hStreamer->setTranscodeHandlesApi.hArb) BIP_Arb_Destroy(hStreamer->setTranscodeHandlesApi.hArb);
    if (hStreamer->startApi.hArb) BIP_Arb_Destroy(hStreamer->startApi.hArb);
    if (hStreamer->prepareApi.hArb) BIP_Arb_Destroy(hStreamer->prepareApi.hArb);
    if (hStreamer->stopApi.hArb) BIP_Arb_Destroy(hStreamer->stopApi.hArb);
    if (hStreamer->destroyApi.hArb) BIP_Arb_Destroy(hStreamer->destroyApi.hArb);

    if (hStreamer->file.hMediaFileAbsolutePathName) BIP_String_Destroy( hStreamer->file.hMediaFileAbsolutePathName );
    if (hStreamer->file.hPlaySpeed) BIP_String_Destroy(hStreamer->file.hPlaySpeed);

    if (hStreamer->hStateMutex) B_Mutex_Destroy( hStreamer->hStateMutex );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: Destroyed" BIP_MSG_PRE_ARG, (void *)hStreamer ));
    BDBG_OBJECT_DESTROY( hStreamer, BIP_Streamer );

    B_Os_Free( hStreamer );

} /* streamerDestroy */

const BIP_Streamer * BIP_Streamer_GetObject_priv(
    BIP_StreamerHandle  hStreamer
    )
{
    return ( hStreamer );
}


/**
Summary:
Populate a BIP_StreamerStreamInfo structure using data from a BIP_MediaInfoStream structure
*/
void BIP_Streamer_GetStreamerStreamInfoFromMediaInfo(
    const BIP_MediaInfoStream *pMediaInfoStream,
    BIP_StreamerStreamInfo    *pStreamerStreamInfo
    )
{
    BKNI_Memset( pStreamerStreamInfo, 0, sizeof(BIP_StreamerStreamInfo) );

    pStreamerStreamInfo->contentLength = pMediaInfoStream->contentLength;
    pStreamerStreamInfo->durationInMs = pMediaInfoStream->durationInMs;
    pStreamerStreamInfo->numberOfTrackGroups = pMediaInfoStream->numberOfTrackGroups;
    pStreamerStreamInfo->numberOfTracks = pMediaInfoStream->numberOfTracks;
    pStreamerStreamInfo->transportTimeStampEnabled = pMediaInfoStream->transportTimeStampEnabled;
    pStreamerStreamInfo->transportType = pMediaInfoStream->transportType;
}


/**
Summary:
Populate a BIP_StreamerTrackInfo structure using data from a BIP_MediaInfoTrack structure
*/
void BIP_Streamer_GetStreamerTrackInfoFromMediaInfo(
    const BIP_MediaInfoTrack  *pMediaInfoTrack,
    BIP_StreamerTrackInfo     *pStreamerTrack
    )
{
    BKNI_Memset( pStreamerTrack, 0, sizeof(BIP_StreamerTrackInfo) );

    pStreamerTrack->trackId =pMediaInfoTrack->trackId;
    pStreamerTrack->type = pMediaInfoTrack->trackType;

    if(pMediaInfoTrack->trackType == BIP_MediaInfoTrackType_eVideo)
    {
        pStreamerTrack->info.video.codec = pMediaInfoTrack->info.video.codec;
        pStreamerTrack->info.video.colorDepth = pMediaInfoTrack->info.video.colorDepth;
        pStreamerTrack->info.video.height = pMediaInfoTrack->info.video.height;
        pStreamerTrack->info.video.width =  pMediaInfoTrack->info.video.width;
        pStreamerTrack->info.video.pMediaNavFileAbsolutePathName =  NULL;
    }
    else if(pMediaInfoTrack->trackType == BIP_MediaInfoTrackType_eAudio)
    {
        pStreamerTrack->info.audio.codec = pMediaInfoTrack->info.audio.codec;
    }
}


BIP_StreamerHandle BIP_Streamer_Create(
    const BIP_StreamerCreateSettings *pCreateSettings
    )
{
    BIP_Status                  bipStatus;
    BIP_StreamerHandle          hStreamer = NULL;
    BIP_StreamerCreateSettings  defaultSettings;

    BIP_SETTINGS_ASSERT(pCreateSettings, BIP_StreamerCreateSettings);

    /* Create the streamer object */
    hStreamer = B_Os_Calloc( 1, sizeof( BIP_Streamer ));
    BIP_CHECK_GOTO(( hStreamer != NULL ), ( "Failed to allocate memory (%zu bytes) for Streamer Object", sizeof(BIP_Streamer) ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    BDBG_OBJECT_SET( hStreamer, BIP_Streamer );

    /* Create mutex to synchronize state machine from being run via callbacks (BIP_Socket or timer) & Caller calling APIs. */
    hStreamer->hStateMutex = B_Mutex_Create(NULL);
    BIP_CHECK_GOTO(( hStreamer->hStateMutex ), ( "B_Mutex_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    if (NULL == pCreateSettings)
    {
        BIP_Streamer_GetDefaultCreateSettings( &defaultSettings );
        pCreateSettings = &defaultSettings;
    }
    hStreamer->createSettings = *pCreateSettings;

    BIP_Streamer_GetDefaultSettings(&hStreamer->settings);

    /* Create API ARBs: one per API */
    hStreamer->getSettingsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hStreamer->getSettingsApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hStreamer->setSettingsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hStreamer->setSettingsApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hStreamer->getStatusApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hStreamer->getStatusApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hStreamer->printStatusApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hStreamer->printStatusApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hStreamer->fileInputSettingsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hStreamer->fileInputSettingsApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hStreamer->tunerInputSettingsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hStreamer->tunerInputSettingsApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hStreamer->ipInputSettingsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hStreamer->ipInputSettingsApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hStreamer->recpumpInputSettingsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hStreamer->recpumpInputSettingsApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hStreamer->outputSettingsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hStreamer->outputSettingsApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hStreamer->addTrackApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hStreamer->addTrackApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hStreamer->addTranscodeProfileApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hStreamer->addTranscodeProfileApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hStreamer->setTranscodeHandlesApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hStreamer->setTranscodeHandlesApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hStreamer->startApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hStreamer->startApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hStreamer->prepareApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hStreamer->prepareApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hStreamer->stopApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hStreamer->stopApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hStreamer->destroyApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hStreamer->destroyApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hStreamer->state = BIP_StreamerState_eIdle;
    BDBG_MSG(( BIP_MSG_PRE_FMT "Created hStreamer %p: state %d" BIP_MSG_PRE_ARG, (void *)hStreamer, hStreamer->state));

    bipStatus = BIP_SUCCESS;

    BDBG_MSG((    BIP_MSG_PRE_FMT "Created: " BIP_STREAMER_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_STREAMER_PRINTF_ARG(hStreamer)));
    BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Created: " BIP_STREAMER_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_STREAMER_PRINTF_ARG(hStreamer)));

    return ( hStreamer );

error:
    return ( NULL );
} /* BIP_Streamer_Create */

/**
 * Summary:
 * Destroy http socket
 *
 * Description:
 **/
void BIP_Streamer_Destroy(
    BIP_StreamerHandle hStreamer
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hStreamer, BIP_Streamer );

    BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Destroying: " BIP_STREAMER_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_STREAMER_PRINTF_ARG(hStreamer)));

    /* Serialize access to Settings state among another thread calling the same _GetSettings API. */
    hArb = hStreamer->destroyApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hStreamer;
    arbSettings.arbProcessor = processStreamerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_Streamer_Destroy" ), error, bipStatus, bipStatus );

error:
    streamerDestroy( hStreamer );

} /* BIP_Streamer_Destroy */

void BIP_Streamer_GetSettings(
        BIP_StreamerHandle    hStreamer,
        BIP_StreamerSettings *pSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hStreamer, BIP_Streamer );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hStreamer));

    BIP_CHECK_GOTO(( hStreamer ), ( "hStreamer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pSettings ), ( "pSettings can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    /* Serialize access to Settings state among another thread calling the same _GetSettings API. */
    hArb = hStreamer->getSettingsApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hStreamer->getSettingsApi.pSettings = pSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hStreamer;
    arbSettings.arbProcessor = processStreamerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_Streamer_GetSettings" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hStreamer, bipStatus ));

    return;
}

BIP_Status BIP_Streamer_SetSettings(
    BIP_StreamerHandle    hStreamer,
    BIP_StreamerSettings  *pSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hStreamer, BIP_Streamer );
    BDBG_ASSERT( pSettings );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hStreamer));

    BIP_CHECK_GOTO(( hStreamer ), ( "hStreamer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pSettings ), ( "pSettings can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hStreamer->setSettingsApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hStreamer->setSettingsApi.pSettings = pSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hStreamer;
    arbSettings.arbProcessor = processStreamerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_Streamer_SetSettings" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hStreamer, bipStatus ));

    return( bipStatus );
} /* BIP_Streamer_SetSettings */


BIP_Status BIP_Streamer_SetFileInputSettings(
    BIP_StreamerHandle              hStreamer,
    const char                      *pMediaFileAbsolutePathName,
    BIP_StreamerStreamInfo          *pStreamerStreamInfo,
    BIP_StreamerFileInputSettings   *pFileInputSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_StreamerFileInputSettings defaultFileInputSettings;

    BDBG_OBJECT_ASSERT( hStreamer, BIP_Streamer );
    BIP_SETTINGS_ASSERT(pFileInputSettings, BIP_StreamerFileInputSettings);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hStreamer));

    BIP_CHECK_GOTO(( hStreamer ), ( "hStreamer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pMediaFileAbsolutePathName ), ( "pMediaFileAbsolutePathName can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pStreamerStreamInfo), ( "pStreamerStreamInfo can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pFileInputSettings && !pFileInputSettings->dropNullPackets), ( "dropNullPackets settings is not yet supported!" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    /* Make sure file to be streamed exists & has non-zero size. */
    {
        int fd = -1;
        struct stat fileStats;
        if ( ( (fd = open( pMediaFileAbsolutePathName, O_RDONLY )) < 0 ) || (( fstat( fd, &fileStats ) == 0 ) && ( fileStats.st_size <= 0 )) )
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "pMediaFileAbsolutePathName %s file doesn't exist or has 0 size" BIP_MSG_PRE_ARG, pMediaFileAbsolutePathName ));
            if ( fd >= 0 ) close( fd );
            bipStatus = BIP_ERR_INVALID_PARAMETER;
            goto error;
        }
        if ( fd >= 0 ) close( fd );
    }

    /* Serialize access to Settings state among another thread calling the same API. */
    hArb = hStreamer->fileInputSettingsApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    if ( pFileInputSettings == NULL )
    {
        BIP_Streamer_GetDefaultFileInputSettings( &defaultFileInputSettings );
        pFileInputSettings = &defaultFileInputSettings;
    }
    /* Move the API arguments into it's argument list so the state machine can find them. */
    hStreamer->fileInputSettingsApi.pStreamerStreamInfo = pStreamerStreamInfo;
    hStreamer->fileInputSettingsApi.pMediaFileAbsolutePathName = pMediaFileAbsolutePathName;
    hStreamer->fileInputSettingsApi.pFileInputSettings = pFileInputSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hStreamer;
    arbSettings.arbProcessor = processStreamerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_Streamer_SetFileInputSettings" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hStreamer, bipStatus ));

    return( bipStatus );
} /* BIP_Streamer_SetFileInputSettings */

BIP_Status BIP_Streamer_SetTunerInputSettings(
    BIP_StreamerHandle              hStreamer,
    NEXUS_ParserBand                hParserBand,
    BIP_StreamerStreamInfo          *pStreamerStreamInfo,
    BIP_StreamerTunerInputSettings  *pTunerInputSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_StreamerTunerInputSettings  defaultTunerInputSettings;

    BDBG_OBJECT_ASSERT( hStreamer, BIP_Streamer );
    BIP_SETTINGS_ASSERT(pTunerInputSettings, BIP_StreamerTunerInputSettings);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hStreamer));

    BIP_CHECK_GOTO(( hStreamer ), ( "hStreamer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pStreamerStreamInfo), ( "pStreamerStreamInfo can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pStreamerStreamInfo->transportType == NEXUS_TransportType_eTs || pStreamerStreamInfo->transportType == NEXUS_TransportType_eMpeg2Pes ),
            ( "This containerType %d can't be supported via TunerInput", pStreamerStreamInfo->transportType ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    /* Validate the parserBand handle. */
    {
        NEXUS_Error nrc;
        NEXUS_ParserBandStatus parserBandStatus;

        nrc = NEXUS_ParserBand_GetStatus( hParserBand, &parserBandStatus );
        BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "Invalid ParserBand" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    }

    /* Validate the recpump handle. */
    if ( pTunerInputSettings && pTunerInputSettings->hRecpump )
    {
        NEXUS_Error nrc;
        NEXUS_RecpumpStatus recpumpStatus;

        nrc = NEXUS_Recpump_GetStatus( pTunerInputSettings->hRecpump, &recpumpStatus );
        BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "Invalid Recpump" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

        BIP_CHECK_GOTO( (recpumpStatus.started == false), ( "Recpump should not be started by App!" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    }

    if ( pTunerInputSettings == NULL )
    {
        BIP_Streamer_GetDefaultTunerInputSettings( &defaultTunerInputSettings );
        pTunerInputSettings = &defaultTunerInputSettings;
    }

    /* Serialize access to Settings state among another thread calling the same API. */
    hArb = hStreamer->tunerInputSettingsApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hStreamer->tunerInputSettingsApi.hParserBand = hParserBand;
    hStreamer->tunerInputSettingsApi.pStreamerStreamInfo = pStreamerStreamInfo;
    hStreamer->tunerInputSettingsApi.pTunerInputSettings = pTunerInputSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hStreamer;
    arbSettings.arbProcessor = processStreamerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_Streamer_SetTunerInputSettings" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hStreamer, bipStatus ));

    return( bipStatus );
} /* BIP_Streamer_SetTunerInputSettings */

BIP_Status BIP_Streamer_SetIpInputSettings(
    BIP_StreamerHandle              hStreamer,
    BIP_PlayerHandle                hPlayer,
    BIP_StreamerStreamInfo          *pStreamerStreamInfo,
    BIP_StreamerIpInputSettings     *pIpInputSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_StreamerIpInputSettings  defaultIpInputSettings;

    BDBG_OBJECT_ASSERT( hStreamer, BIP_Streamer );
    BIP_SETTINGS_ASSERT(pIpInputSettings, BIP_StreamerIpInputSettings);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hStreamer));

    BIP_CHECK_GOTO(( hStreamer ), ( "hStreamer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( hPlayer ), ( "hPlayer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pStreamerStreamInfo), ( "pStreamerStreamInfo can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pStreamerStreamInfo->transportType == NEXUS_TransportType_eTs || pStreamerStreamInfo->transportType == NEXUS_TransportType_eMpeg2Pes ),
            ( "This containerType %d can't be supported via IpInput", pStreamerStreamInfo->transportType ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    if ( pIpInputSettings == NULL )
    {
        BIP_Streamer_GetDefaultIpInputSettings( &defaultIpInputSettings );
        pIpInputSettings = &defaultIpInputSettings;
    }

    /* Validate the recpump handle. */
    if ( pIpInputSettings && pIpInputSettings->hRecpump )
    {
        NEXUS_Error nrc;
        NEXUS_RecpumpStatus recpumpStatus;

        nrc = NEXUS_Recpump_GetStatus( pIpInputSettings->hRecpump, &recpumpStatus );
        BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "Invalid Recpump" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

        BIP_CHECK_GOTO( (recpumpStatus.started == false), ( "Recpump should not be started by App!" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    }

    /* Serialize access to Settings state among another thread calling the same API. */
    hArb = hStreamer->ipInputSettingsApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hStreamer->ipInputSettingsApi.hPlayer = hPlayer;
    hStreamer->ipInputSettingsApi.pStreamerStreamInfo = pStreamerStreamInfo;
    hStreamer->ipInputSettingsApi.pIpInputSettings = pIpInputSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hStreamer;
    arbSettings.arbProcessor = processStreamerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_Streamer_SetIpInputSettings" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hStreamer, bipStatus ));

    return( bipStatus );
} /* BIP_Streamer_SetIpInputSettings */

BIP_Status BIP_Streamer_SetRecpumpInputSettings(
    BIP_StreamerHandle                  hStreamer,
    NEXUS_RecpumpHandle                 hRecpump,
    BIP_StreamerRecpumpInputSettings    *pRecpumpInputSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_StreamerRecpumpInputSettings defaultSettings;

    BDBG_OBJECT_ASSERT( hStreamer, BIP_Streamer );
    BDBG_ASSERT( pRecpumpInputSettings );
    BIP_SETTINGS_ASSERT(pRecpumpInputSettings, BIP_StreamerRecpumpInputSettings);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hStreamer));

    BIP_CHECK_GOTO(( hStreamer ), ( "hStreamer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( hRecpump ), ( "hRecpump can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    /* Validate Recpump is not yet started! */
    {
        NEXUS_Error nrc;
        NEXUS_RecpumpStatus recpumpStatus;

        nrc = NEXUS_Recpump_GetStatus( hRecpump, &recpumpStatus );
        BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "Failed to Get RecpumpStatus: Invalid handle, nexus error 0x%x ", nrc ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
        BIP_CHECK_GOTO(( recpumpStatus.started == false ), ( "Recpump handle %p shouldn't be started  by App", (void *)hRecpump ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    }

    /* Serialize access to Settings state among another thread calling the same API. */
    hArb = hStreamer->recpumpInputSettingsApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    if ( pRecpumpInputSettings == NULL )
    {
        BIP_Streamer_GetDefaultRecpumpInputSettings( &defaultSettings );
        pRecpumpInputSettings = &defaultSettings;
    }

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hStreamer->recpumpInputSettingsApi.pRecpumpInputSettings = pRecpumpInputSettings;
    hStreamer->recpumpInputSettingsApi.hRecpump = hRecpump;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hStreamer;
    arbSettings.arbProcessor = processStreamerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_Streamer_SetRecpumpInputSettings" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hStreamer, bipStatus ));

    return( bipStatus );
} /* BIP_Streamer_SetRecpumpInputSettings */

BIP_Status BIP_Streamer_AddTrack(
    BIP_StreamerHandle hStreamer,
    BIP_StreamerTrackInfo *pStreamerTrackInfo,
    BIP_StreamerTrackSettings *pTrackSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_StreamerTrackSettings defaultTrackSettings;

    BDBG_OBJECT_ASSERT( hStreamer, BIP_Streamer );
    BIP_SETTINGS_ASSERT(pTrackSettings, BIP_StreamerTrackSettings);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hStreamer));

    BIP_CHECK_GOTO(( hStreamer ), ( "hStreamer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pStreamerTrackInfo ), ( "pStreamerTrackInfo can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    if ( pStreamerTrackInfo->type == BIP_MediaInfoTrackType_eVideo && pStreamerTrackInfo->info.video.pMediaNavFileAbsolutePathName )
    {
        int fd = -1;
        struct stat fileStats;
        if ( ( (fd = open( pStreamerTrackInfo->info.video.pMediaNavFileAbsolutePathName , O_RDONLY )) < 0 ) || (( fstat( fd, &fileStats ) == 0 ) && ( fileStats.st_size <= 0 )) )
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "pMediaFileAbsolutePathName %s file doesn't exist or has 0 size" BIP_MSG_PRE_ARG, pStreamerTrackInfo->info.video.pMediaNavFileAbsolutePathName ));
            if ( fd >= 0 ) close( fd );
            bipStatus = BIP_ERR_INVALID_PARAMETER;
            goto error;
        }
        if ( fd >= 0 ) close( fd );
    }

    if ( pTrackSettings == NULL )
    {
        BIP_Streamer_GetDefaultTrackSettings( &defaultTrackSettings );
        pTrackSettings = &defaultTrackSettings;
    }

    hArb = hStreamer->addTrackApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hStreamer->addTrackApi.pStreamerTrackInfo = pStreamerTrackInfo;
    hStreamer->addTrackApi.pTrackSettings = pTrackSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hStreamer;
    arbSettings.arbProcessor = processStreamerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_Streamer_AddTrack" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hStreamer, bipStatus ));

    return( bipStatus );
}

BIP_Status BIP_Streamer_SetOutputSettings(
    BIP_StreamerHandle          hStreamer,
    BIP_StreamerProtocol        streamerProtocol,
    BIP_StreamerOutputSettings  *pOutputSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_StreamerOutputSettings defaultSettings;

    BDBG_OBJECT_ASSERT( hStreamer, BIP_Streamer );
    BIP_SETTINGS_ASSERT(pOutputSettings, BIP_StreamerOutputSettings);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hStreamer));

    BIP_CHECK_GOTO(( hStreamer ), ( "hStreamer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( streamerProtocol < BIP_StreamerProtocol_eMax ), ( "streamerProtocol %d is not valid", streamerProtocol ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    if ( pOutputSettings == NULL )
    {
        BIP_Streamer_GetDefaultOutputSettings( &defaultSettings );
        pOutputSettings = &defaultSettings;
    }
    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hStreamer->outputSettingsApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hStreamer->outputSettingsApi.pOutputSettings = pOutputSettings;
    hStreamer->outputSettingsApi.streamerProtocol = streamerProtocol;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hStreamer;
    arbSettings.arbProcessor = processStreamerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_Streamer_SetOutputSettings" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hStreamer, bipStatus ));

    return( bipStatus );
} /* BIP_Streamer_SetOutputSettings */

BIP_Status BIP_Streamer_AddTranscodeProfile(
    BIP_StreamerHandle      hStreamer,
    BIP_TranscodeProfile    *pTranscodeProfile
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BIP_CHECK_GOTO(( hStreamer ), ( "hStreamer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pTranscodeProfile ), ( "pTranscodeProfile can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    BDBG_OBJECT_ASSERT( hStreamer, BIP_Streamer );
    BIP_SETTINGS_ASSERT(pTranscodeProfile, BIP_TranscodeProfile);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hStreamer));

    /* Note: Rest of parameter validation happens in the BIP_Streamer class. */

    hArb = hStreamer->addTranscodeProfileApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hStreamer->addTranscodeProfileApi.pTranscodeProfile = pTranscodeProfile;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hStreamer;
    arbSettings.arbProcessor = processStreamerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_Streamer_AddTranscodeProfile" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hStreamer, bipStatus ));

    return( bipStatus );
}

BIP_Status BIP_Streamer_SetTranscodeHandles(
    BIP_StreamerHandle          hStreamer,
    BIP_TranscodeNexusHandles   *pTranscodeNexusHandles
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BIP_CHECK_GOTO(( hStreamer ), ( "hStreamer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pTranscodeNexusHandles ), ( "pTranscodeNexusHandles can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pTranscodeNexusHandles->useSimpleHandles ), ( "pTranscodeNexusHandles->useSimpleHandles must be set" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    BDBG_OBJECT_ASSERT( hStreamer, BIP_Streamer );
    BIP_SETTINGS_ASSERT(pTranscodeNexusHandles, BIP_TranscodeNexusHandles);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hStreamer));

    hArb = hStreamer->setTranscodeHandlesApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hStreamer->setTranscodeHandlesApi.transcodeNexusHandles = *pTranscodeNexusHandles;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hStreamer;
    arbSettings.arbProcessor = processStreamerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_Streamer_AddTranscodeProfile" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hStreamer, bipStatus ));

    return( bipStatus );
}
BIP_Status BIP_Streamer_Prepare(
    BIP_StreamerHandle          hStreamer,
    BIP_StreamerPrepareSettings *pSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_StreamerPrepareSettings defaultPrepareSettings;

    BDBG_OBJECT_ASSERT( hStreamer, BIP_Streamer );
    BIP_SETTINGS_ASSERT(pSettings, BIP_StreamerPrepareSettings);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hStreamer));

    BIP_CHECK_GOTO(( hStreamer ), ( "hStreamer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    if ( pSettings == NULL )
    {
        BIP_Streamer_GetDefaultPrepareSettings( &defaultPrepareSettings );
        pSettings = &defaultPrepareSettings;
    }

    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hStreamer->prepareApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hStreamer->prepareApi.pSettings = pSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hStreamer;
    arbSettings.arbProcessor = processStreamerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_Streamer_Prepare" ), error, bipStatus, bipStatus );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: Streamer Prepareed: completionStatus: %s" BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_StatusGetText(bipStatus) ));

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hStreamer, bipStatus ));

    return ( bipStatus );
} /* BIP_Streamer_Prepare */

BIP_Status BIP_Streamer_Start(
    BIP_StreamerHandle          hStreamer,
    BIP_StreamerStartSettings   *pSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_StreamerStartSettings defaultSettings;

    BDBG_OBJECT_ASSERT( hStreamer, BIP_Streamer );
    BIP_SETTINGS_ASSERT(pSettings, BIP_StreamerStartSettings);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hStreamer));

    BIP_CHECK_GOTO(( hStreamer ), ( "hStreamer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    if ( pSettings == NULL )
    {
        BIP_Streamer_GetDefaultStartSettings( &defaultSettings );
        pSettings = &defaultSettings;
    }
    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hStreamer->startApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hStreamer->startApi.pSettings = pSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hStreamer;
    arbSettings.arbProcessor = processStreamerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_Streamer_Start" ), error, bipStatus, bipStatus );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hStreamer %p: Streamer Started: completionStatus: %s" BIP_MSG_PRE_ARG, (void *)hStreamer, BIP_StatusGetText(bipStatus) ));

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hStreamer, bipStatus ));

    return ( bipStatus );
} /* BIP_Streamer_Start */

BIP_Status BIP_Streamer_Stop(
    BIP_StreamerHandle    hStreamer
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hStreamer, BIP_Streamer );

    BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Stopping: " BIP_STREAMER_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_STREAMER_PRINTF_ARG(hStreamer)));
    BDBG_MSG((    BIP_MSG_PRE_FMT "Stopping: " BIP_STREAMER_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_STREAMER_PRINTF_ARG(hStreamer)));

    BIP_CHECK_GOTO(( hStreamer ), ( "hStreamer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hStreamer->stopApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hStreamer;
    arbSettings.arbProcessor = processStreamerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_Streamer_Stop" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hStreamer, bipStatus ));

    return( bipStatus );
} /* BIP_Streamer_Stop */

BIP_Status  BIP_Streamer_GetStatus(
        BIP_StreamerHandle  hStreamer,
        BIP_StreamerStatus  *pStatus
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hStreamer, BIP_Streamer );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hStreamer));

    BIP_CHECK_GOTO(( hStreamer ), ( "hStreamer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pStatus ), ( "pStatus can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    /* Serialize access to Settings state among another thread calling the same _GetStatus API. */
    hArb = hStreamer->getStatusApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hStreamer->getStatusApi.pStatus = pStatus;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hStreamer;
    arbSettings.arbProcessor = processStreamerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_Streamer_GetStatus" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hStreamer, bipStatus ));

    return ( bipStatus );
}

void BIP_Streamer_PrintStatus(
    BIP_StreamerHandle hStreamer
    )
{
    BIP_Status bipStatus;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_ASSERT( hStreamer );
    BDBG_OBJECT_ASSERT( hStreamer, BIP_Streamer );

    if ( !hStreamer ) return;

    /* Serialize access to Settings state among another thread calling the same _GetSettings API. */
    hArb = hStreamer->printStatusApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hStreamer;
    arbSettings.arbProcessor = processStreamerState;
    arbSettings.waitIfBusy = true;;

    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed" ), error, bipStatus, bipStatus );

error:
    return;
} /* BIP_Streamer_PrintStats */
