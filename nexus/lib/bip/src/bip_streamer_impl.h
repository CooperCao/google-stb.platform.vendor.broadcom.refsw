/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

#ifndef BIP_STREAMER_IMPL_H
#define BIP_STREAMER_IMPL_H

#include "bip_priv.h"
#include "b_playback_ip_lib.h"

/*
 * States for Streamer Object (main server object),
 *  states reflecting Streamer APIs visiable to Apps: Create, Start, Stop of Server
 */
typedef enum BIP_StreamerInputState
{
    BIP_StreamerInputState_eNotSet,                 /* Streamer Input is not set or cleared. */
    BIP_StreamerInputState_eSet,                    /* Streamer Input is set. */
    BIP_StreamerInputState_eMax
} BIP_StreamerInputState;

typedef enum BIP_StreamerOutputState
{
    BIP_StreamerOutputState_eNotSet,                /* Streamer Output is not set or cleared. */
    BIP_StreamerOutputState_eSet,                   /* Streamer Output is set. */
    BIP_StreamerOutputState_eMax
} BIP_StreamerOutputState;

typedef enum BIP_StreamerState
{
    BIP_StreamerState_eUninitialized = 0,           /* Initial state: object is just created but not fully initialzed. */
    BIP_StreamerState_eIdle,                        /* Idle state: streamer object is either just created or is just stopped. */
    BIP_StreamerState_ePrepared,                    /* Prepared for Streaming. */
    BIP_StreamerState_eStreaming,                   /* Streamer is now streaming the media. */
    /* NOTE: following two states will only be applicable once we add native streaming to BIP_Streamer. Currently, derived classes (Http & Udp streamers) use PBIP for this. */
    BIP_StreamerState_eStreamingDone,               /* Streamer is now done streaming the media (reached EOF or met an n/w error) and letting us know via the callback. */
    BIP_StreamerState_eWaitingForStopApi,           /* Streamer reached either EOF or an Error, notified endOfStreaming Callback to app and now waiting for Stop() or Resume() */
    BIP_StreamerState_eMax
} BIP_StreamerState;

typedef struct BIP_StreamerTrackListEntry
{
    BIP_StreamerTrackInfo                           streamerTrackInfo;
    BIP_StreamerTrackSettings                       trackSettings;
    NEXUS_PidChannelHandle                          hPidChannel;
    bool                                            pidChannelAddedToRecpump;
    BIP_StringHandle                                hMediaNavFileAbsolutePathName;
    bool                                            duplicateTrack; /* A Track already is in the list w/ same trackId but with the different type: e.g. PCR & Vidoe types using same PID. */
    BLST_Q_ENTRY(BIP_StreamerTrackListEntry)    trackListNext;
} BIP_StreamerTrackListEntry;
typedef BIP_StreamerTrackListEntry *BIP_StreamerTrackListEntryHandle;

typedef struct BIP_StreamerTranscodeProfileListEntry
{
    BIP_TranscodeProfile transcodeProfile;
    unsigned profileIndex;
    BLST_Q_ENTRY(BIP_StreamerTranscodeProfileListEntry)    transcodeProfileListNext;
} BIP_StreamerTranscodeProfileListEntry;

typedef struct BIP_Streamer
{
    BDBG_OBJECT( BIP_Streamer )

    B_MutexHandle                                   hStateMutex;                            /* Mutex to synchronize a API invocation with callback invocation */
    BIP_Status                                      completionStatus;

    BIP_StreamerCreateSettings                      createSettings;
    BIP_StreamerSettings                            settings;
    BIP_StreamerStartSettings                       startSettings;
    BIP_StreamerPrepareSettings                     prepareSettings;

    BIP_StreamerState                               state;                                  /* Main Streamer state. */

    struct
    {
        BIP_StreamerOutputState                     profileState;                           /* Streamer sub-state for transcode profile. */
        BIP_StreamerOutputState                     handlesState;                           /* Streamer sub-state for transcode handles. */
        BIP_TranscodeHandle                         hTranscode;
        unsigned                                    numProfiles;                            /* count of profiles. */
        BIP_TranscodeProfile                        profile;                                /* current profile settings. */
        BLST_Q_HEAD( transcodeProfileListHead, BIP_StreamerTranscodeProfileListEntry ) profileListHead;       /* list of ServerSocket objects: object is added to this list when it is created! */
    } transcode;
    struct
    {
        BLST_Q_HEAD( trackListHead, BIP_StreamerTrackListEntry ) listHead;                  /* list of tracks that needs to be streamed out. */
        BIP_StreamerInputState                      inputState;                             /* Streamer sub-state for addTrack input. */
    } track;
    BIP_StreamerInputType                           inputType;                              /* type of input source */
    struct
    {
        BIP_StreamerInputState                      inputState;                             /* Streamer sub-state for file input. */
        bool                                        feedUsingPlaybackChannel;               /* Set when either streaming a single program from MPTS or enableHwPacing is set. */
        BIP_StringHandle                            hMediaFileAbsolutePathName;             /* Media input source: Name of media file */
        BIP_StringHandle                            hPlaySpeed;                             /* Playspeed string reflecting the speed at which to stream the media out. */
        BIP_StreamerFileInputSettings               inputSettings;
    } file;
    struct
    {
        NEXUS_ParserBand                            hParserBand;
        BIP_StreamerTunerInputSettings              inputSettings;
        BIP_StreamerInputState                      inputState;                             /* Streamer sub-state for tuner input. */
    } tuner;
    struct
    {
        BIP_PlayerHandle                            hPlayer;
        BIP_StreamerIpInputSettings                 inputSettings;
        BIP_StreamerInputState                      inputState;                             /* Streamer sub-state for tuner input. */
    } ip;
    struct
    {
        BIP_StreamerInputState                      inputState;                             /* Streamer sub-state for recpump input. */
        BIP_StreamerRecpumpInputSettings            inputSettings;
    } recpump;
    struct
    {
        BIP_StreamerProtocol                        streamerProtocol;
        BIP_StreamerOutputSettings                  settings;
        BIP_StreamerOutputState                     state;                                  /* Streamer sub-state for its output. */
    } output;

    /* State common to different inputs. */
    BIP_StreamerStreamInfo                          streamerStreamInfo;

    NEXUS_RecpumpHandle                             hRecpump;
    bool                                            openedRecpump;
    NEXUS_PlaypumpHandle                            hPlaypump;
    bool                                            openedPlaypump;
    NEXUS_PlaybackHandle                            hPlayback;
    NEXUS_FilePlayHandle                            hFilePlay;

    NEXUS_Timebase                                  pacingTimebase;

    /* One Arb per API */
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_StreamerSettings      *pSettings;
    } getSettingsApi;
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_StreamerSettings      *pSettings;
    } setSettingsApi;
    struct
    {
        BIP_ArbHandle               hArb;
        const char                    *pMediaFileAbsolutePathName;
        BIP_StreamerStreamInfo        *pStreamerStreamInfo;
        BIP_StreamerFileInputSettings *pFileInputSettings;
    } fileInputSettingsApi;
    struct
    {
        BIP_ArbHandle               hArb;
        NEXUS_ParserBand            hParserBand;
        BIP_StreamerStreamInfo         *pStreamerStreamInfo;
        BIP_StreamerTunerInputSettings *pTunerInputSettings;
    } tunerInputSettingsApi;
    struct
    {
        BIP_ArbHandle                   hArb;
        BIP_PlayerHandle                hPlayer;
        BIP_StreamerStreamInfo          *pStreamerStreamInfo;
        BIP_StreamerIpInputSettings     *pIpInputSettings;
    } ipInputSettingsApi;
    struct
    {
        BIP_ArbHandle               hArb;
        NEXUS_RecpumpHandle         hRecpump;
        BIP_StreamerRecpumpInputSettings *pRecpumpInputSettings;
    } recpumpInputSettingsApi;
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_StreamerTrackInfo       *pStreamerTrackInfo;
        BIP_StreamerTrackSettings   *pTrackSettings;
    } addTrackApi;
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_StreamerProtocol        streamerProtocol;
        BIP_StreamerOutputSettings  *pOutputSettings;
    } outputSettingsApi;
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_TranscodeProfile        *pTranscodeProfile;
    } addTranscodeProfileApi;
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_TranscodeNexusHandles   transcodeNexusHandles;
    } setTranscodeHandlesApi;
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_StreamerStartSettings *pSettings;
    } startApi;
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_StreamerPrepareSettings *pSettings;
    } prepareApi;
    struct
    {
        BIP_ArbHandle               hArb;
    } stopApi;
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_StreamerStatus          *pStatus;
    } getStatusApi;
    struct
    {
        BIP_ArbHandle               hArb;
    } printStatusApi;
    struct
    {
        BIP_ArbHandle               hArb;
    } destroyApi;
    BIP_StreamerStats stats;
} BIP_Streamer;


#define BIP_STREAMER_PRINTF_FMT  \
    "[hStreamer=%p State=%s Protocol=%s Xcode=%s InputType=%s File=%s]"

#define BIP_STREAMER_PRINTF_ARG(pObj)                                                                   \
    (void *)(pObj),                                                                                             \
    (pObj)->state==BIP_StreamerState_eIdle                  ? "Idle"                                :   \
    (pObj)->state==BIP_StreamerState_ePrepared              ? "Prepared"                            :   \
    (pObj)->state==BIP_StreamerState_eStreaming             ? "Streaming"                           :   \
    (pObj)->state==BIP_StreamerState_eStreamingDone         ? "StreamingDone"                       :   \
    (pObj)->state==BIP_StreamerState_eWaitingForStopApi     ? "WaitingForStopApi"           :           \
                                                                  "StateNotMapped",                     \
                                                                                                        \
                                                                                                        \
    (pObj)->output.streamerProtocol==BIP_StreamerProtocol_eTcp          ? "TCP"         :               \
    (pObj)->output.streamerProtocol==BIP_StreamerProtocol_ePlainUdp     ? "PlainUDP"    :               \
    (pObj)->output.streamerProtocol==BIP_StreamerProtocol_eRtp          ? "RTP"         :               \
                                                                          "<undefined>",                \
    (pObj)->transcode.profileState==BIP_StreamerOutputState_eSet   ?   "Y" : "N",                        \
    (pObj)->inputType==BIP_StreamerInputType_eFile   ? "File"    :                                      \
    (pObj)->inputType==BIP_StreamerInputType_eBfile  ? "Bfile"   :                                      \
    (pObj)->inputType==BIP_StreamerInputType_eTuner  ? "Tuner"   :                                      \
    (pObj)->inputType==BIP_StreamerInputType_eRecpump? "Recpump" :                                      \
    (pObj)->inputType==BIP_StreamerInputType_eIp     ? "Ip"      :                                      \
                                                       "<undefined>",                                   \
    (pObj)->file.hMediaFileAbsolutePathName ? BIP_String_GetString((pObj)->file.hMediaFileAbsolutePathName) : "NULL"

#endif /* BIP_STREAMER_IMPL_H */
