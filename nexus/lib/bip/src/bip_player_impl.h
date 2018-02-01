/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bip_priv.h"
#if NXCLIENT_SUPPORT
#include "nxclient.h"
#endif

/* Protocols supported by BIP Player as mapped from the URL scheme. */
typedef enum BIP_PlayerProtocol
{
    BIP_PlayerProtocol_eInvalid,
    BIP_PlayerProtocol_eSimpleHttp,
    BIP_PlayerProtocol_eSimpleUdp,
    BIP_PlayerProtocol_eRtp,
    BIP_PlayerProtocol_eRtsp,
    BIP_PlayerProtocol_eMax
} BIP_PlayerProtocol;

/* TODO: these Playing states can be merged w/ the Player's subStates? */
typedef enum BIP_PlayerPlayingState
{
    BIP_PlayerPlayingState_eNormal,     /* Playing at: rate == 1x */
    BIP_PlayerPlayingState_eSlowFwd,    /* Playing at:  0  < rate < 1x */
    BIP_PlayerPlayingState_eFastFwd,    /* Playing at:  1x < rate      */
    BIP_PlayerPlayingState_eSlowRwd,    /* Playing at: -1x < rate < 0  */
    BIP_PlayerPlayingState_eFastRwd,    /* Playing at:  rate < -1x     */
    BIP_PlayerPlayingState_eMax
} BIP_PlayerPlayingState;

typedef enum BIP_PlayerStateInput
{
    BIP_PlayerStateInput_eNone,             /* No input. */
    BIP_PlayerStateInput_eDisconnect,       /* Either BIP_Player_Disconnect() or BIP_Player_Destroy() has been called. */
    BIP_PlayerStateInput_ePause,            /* Either BIP_Player_Pause() has been called or Player has an internal need to Pause (e.g. startPause flag is set. ) */
    BIP_PlayerStateInput_ePlay,             /* BIP_Player_Play() is called. */
    BIP_PlayerStateInput_eSeek,             /* BIP_Player_Seek() has been called. */
    BIP_PlayerStateInput_eStop,             /* BIP_Player_Stop() has been called. */
    BIP_PlayerStateInput_eMax
} BIP_PlayerStateInput;

typedef enum BIP_PlayerApiState
{
    BIP_PlayerApiState_eIdle,               /* Idle */
    BIP_PlayerApiState_eNew,                /* Initiate New API related work. */
    BIP_PlayerApiState_eInProgress,         /* API is in progress and is waiting for its related operation to finish. */
    BIP_PlayerApiState_eDone,               /* API is done. */
    BIP_PlayerApiState_eMax
} BIP_PlayerApiState;

typedef struct BIP_PlayerTrackListEntry
{
    unsigned                                    trackId;                    /* Virtual trackId assigned by the BIP_MediaInfo. Can be different from the realTrackId when multiple tracks contain the same trackId. */
                                                                            /* e.g. for HLS container formats, audio may not be muxed into the main stream and is accessed via a separate URL. In that case, multiple audio tracks may be on different URLs & thus can have same real trackId. */
    BIP_PlayerOpenPidChannelSettings            settings;
    NEXUS_PidChannelHandle                      hPidChannel;
    bool                                        appOwnedDecoders;
#if NXCLIENT_SUPPORT
    NxClient_AllocResults                       allocResults;
#endif
    unsigned                                    connectId;
    bool                                        connected;
    bool                                        alloced;
    NEXUS_PlaypumpHandle                        hPlaypump;                  /* Some AV tracks may not be muxed in the main stream and thus may have same trackId as ones in the main stream, */
                                                                            /*so we would need a separate playpump to feed such tracks. */

    BLST_Q_ENTRY(BIP_PlayerTrackListEntry)      trackListNext;
} BIP_PlayerTrackListEntry;
typedef BIP_PlayerTrackListEntry *BIP_PlayerTrackListEntryHandle;

typedef struct BIP_Player
{
    BDBG_OBJECT( BIP_Player )

    B_MutexHandle                   hStateMutex;                            /* Mutex to synchronize a API invocation with callback invocation */
    BIP_PlayerState                 state;
    BIP_PlayerSubState              subState;
    bool                            disconnecting;
    BIP_PlayerStateInput            stateInput;
    bool                            reRunState;
    BIP_PlayerPlayingState          playingState;
    BIP_PlayerSubState              cachedProbingSubState;
    BIP_Status                      dtcpIpAkeCompletionStatus;
    bool                            dtcpIpEnabled;
    bool                            sslEnabled;
    bool                            dynamicTrackSelectionEnabled;           /* Set if BIP_PlayerSettings.enableDynamicTrackSelection is set & either */
                                                                            /* PlayerSettings.trackSelectionCallback is set or explicit trackIds were not specified!. */
    bool                            newProgramFound;
    int                             socketFd;
#ifdef NEXUS_HAS_ASP
    B_AspChannelHandle              hAspChannel;
    bool                            gotAspLibDataReadyCallback;
    bool                            gotAspLibStateChangedCallback;
#endif

    BIP_DtcpIpClientFactoryAkeEntryHandle hAkeEntry;
    BIP_PlayerDataAvailabilityModel dataAvailabilityModel;

    BLST_Q_HEAD( trackListHead, BIP_PlayerTrackListEntry ) trackListHead;        /* list of tracks opened via the BIP_Player_OpenPidChannel() for recording or streaming out purpose. */

    BIP_PlayerMode                  mode;
    bool                            trickmodeActive;
    int                             playRate;
    int                             newPlayRate;                            /* play rate in NEXUS_NORMAL_PLAY_SPEED */

    struct
    {
        BIP_PlayerApiState          apiState;
        B_PlaybackIpError           pbipStatus;
        BIP_TimerHandle             hApiTimer;
        B_Time                      apiStartTime;
        bool                        apiTimerActive;
        BIP_PlayerSubState          prevPlayerSubState;
        BIP_Status                  completionStatus;
        bool                        waitingForServerStatus;
        bool                        waitingForPlayCompletion;
    } playState;

    struct
    {
        BIP_PlayerApiState          apiState;
        B_PlaybackIpError           pbipStatus;
        BIP_TimerHandle             hApiTimer;
        B_Time                      apiStartTime;
        bool                        apiTimerActive;
        BIP_PlayerSubState          prevPlayerSubState;
        B_PlaybackIpTrickModesSettings pbipSettings;
        BIP_Status                  completionStatus;
        bool                        waitingForServerStatus;
        bool                        waitingForTrickmodeCompletion;
    } playAtRateState;

    struct
    {
        BIP_PlayerApiState          apiState;
        B_PlaybackIpError           pbipStatus;
        BIP_TimerHandle             hApiTimer;
        B_Time                      apiStartTime;
        bool                        apiTimerActive;
        BIP_PlayerSubState          prevPlayerSubState;
        B_PlaybackIpTrickModesSettings pbipSettings;
        BIP_Status                  completionStatus;
        bool                        waitingForServerStatus;
        bool                        waitingForTrickmodeCompletion;
    } playByFrameState;

    struct
    {
        BIP_PlayerApiState          apiState;
        B_PlaybackIpError           pbipStatus;
        BIP_TimerHandle             hApiTimer;
        B_Time                      apiStartTime;
        bool                        apiTimerActive;
        BIP_PlayerSubState          prevPlayerSubState;
        BIP_Status                  completionStatus;
        B_Time                      pauseStartTime;
        BIP_TimerHandle             hPauseTimer;
        bool                        pauseTimerActive;
        int                         pauseTimeoutInMs;
    } pauseState;

    struct
    {
        BIP_PlayerApiState          apiState;
        B_PlaybackIpError           pbipStatus;
        BIP_TimerHandle             hApiTimer;
        B_Time                      apiStartTime;
        bool                        apiTimerActive;
        BIP_PlayerSubState          prevPlayerSubState;
        B_PlaybackIpTrickModesSettings pbipSettings;
        BIP_Status                  completionStatus;
        bool                        waitingForServerStatus;
        bool                        waitingForSeekCompletion;
    } seekState;

    struct
    {
        BIP_PlayerApiState          apiState;
        BIP_Status                  completionStatus;
    } stopState;

    struct
    {
        BIP_PlayerApiState                      apiState;
        B_PlaybackIpHandle                      hPlaybackIp;
        BIP_Status                              completionStatus;
        B_PlaybackIpSessionOpenStatus           ipSessionOpenStatus;
        B_PlaybackIpSessionOpenSettings         ipSessionOpenSettings;
        B_PlaybackIpError                       status;
        BIP_StringHandle                        hUrl;
        BIP_CallbackDesc                        pbipCallback;
        B_Time                                  lastUpdateTime;
        bool                                    lastUpdateTimeSet;
        BIP_PlayerStatusFromServer              serverStatus;
        BIP_StringHandle                        hAdditionalHeaders;
    } getStatusFromServerState;

    BIP_HttpResponseHandle          hHttpResponse;
    BIP_UrlHandle                   hUrl;

    void                            *pSslInitCtx;
    void                            *pDtcpIpLibAkeCtx;
    void                            *dtcpInitCtx;

    struct
    {
        BIP_StringHandle                    hUrl;
        BIP_CallbackDesc                    pbipCallback;
        B_PlaybackIpHandle                  hPlaybackIp;
        B_PlaybackIpPsiInfo                 psi;
        B_PlaybackIpProtocol                protocol;
        B_PlaybackIpError                   status;
        BIP_CallbackDesc                    eventCallback;
        B_PlaybackIpSessionOpenStatus       ipSessionOpenStatus;
        B_PlaybackIpSessionOpenSettings     ipSessionOpenSettings;
        B_PlaybackIpSessionSetupSettings    ipSessionSetupSettings;
        B_PlaybackIpSessionSetupStatus      ipSessionSetupStatus;
        B_PlaybackIpSessionStartSettings    ipStartSettings;
        B_PlaybackIpSessionStartStatus      ipStartStatus;
        B_PlaybackIpTrickModesSettings      ipTrickModeSettings;
        B_PlaybackIpSettings                settings;
    } pbipState;

    struct
    {
        bool                        timeBasedSeekSupported;
        bool                        byteBasedSeekSupported;
        bool                        limitedOpFlagsSupported;
        bool                        senderPacingSupported;
        bool                        linkProtectionSupported;
        bool                        pauseUsingConnxStallingSupported;
        bool                        s0Increasing;
        bool                        sNIncreasing;
        bool                        playSpeedSupported;
        bool                        availableSeekRangeSupported;
        BIP_StringHandle            hDtcpIpHostIp;
        BIP_StringHandle            hDtcpIpPort;
    } dlnaFlags;

    BIP_PlayerProtocol              playerProtocol;
    bool                            mediaProbeComplete;
    BIP_MediaInfoHandle             hMediaInfo;
    BIP_MediaInfoHandle             hMediaInfoForApp;       /* last media info object that is given to App via GetStatus function. */
    BIP_StringHandle                hNetworkInterfaceName;
    BIP_StringHandle                hAdditionalHeaders;
    BIP_StringHandle                hPreferredAudioLanguage;

    NEXUS_DmaHandle                 hDma;
    NEXUS_FilePlayHandle            hFile; /* NOTE: this is currently created PBIP during SesionSetup. If App skips _GetStreamInfo, then we will need to create this here. */
    NEXUS_PlaybackHandle            hPlayback;
    NEXUS_PlaypumpHandle            hPlaypump;
    NEXUS_PlaypumpHandle            hPlaypump2;

    unsigned                        videoTrackId;
    NEXUS_PidChannelHandle          hVideoPidChannel;           /* PidChannel handle associated with currently played Video Track. */
    NEXUS_VideoDecoderHandle        hVideoDecoder;
    NEXUS_SimpleVideoDecoderHandle  hSimpleVideoDecoder;
    NEXUS_DisplayHandle             hDisplay;
    NEXUS_VideoWindowHandle         hWindow;

    unsigned                        audioTrackId;
    BIP_MediaInfoTrack              audioTrackInfo;             /* Cached audio track that is currently being played. */
    NEXUS_PidChannelHandle          hPrimaryAudioPidChannel;    /* PidChannel handle associated with currently played Audio Track. */
    NEXUS_AudioDecoderHandle        hPrimaryAudioDecoder;
    NEXUS_SimpleAudioDecoderHandle  hSimplePrimaryAudioDecoder;

    NEXUS_Timebase                  lockedTimebase;
    NEXUS_Timebase                  freeRunTimebase;
    NEXUS_PidChannelHandle          hPcrPidChannel;
    NEXUS_StcChannelHandle          hStcChannel;
    NEXUS_SimpleStcChannelHandle    hSimpleStcChannel;

    BIP_PlayerCreateSettings        createSettings;
    BIP_PlayerConnectSettings       connectSettings;
    BIP_PlayerProbeMediaInfoSettings probeSettings;
    BIP_PlayerPrepareSettings       prepareSettings;
    BIP_PlayerSettings              playerSettings;
    BIP_PlayerStartSettings         startSettings;
    BIP_PlayerGetStatusFromServerSettings   getStatusFromServerSettings;

    BIP_PlayerStreamInfo            streamInfo;
    NEXUS_TransportType             containerType;
    NEXUS_TransportType             transportTypeForPlaypump2;
    bool                            useNexusPlaypump;
    bool                            usePlaypump2ForAudio;
    bool                            useLiveIpMode;
    BIP_PlayerClockRecoveryMode     clockRecoveryMode;

    /* Callbacks to App */
    bool                            reachedEndOfStream;
    bool                            reachedBeginningOfStream;
    bool                            receivedNetworkError;
    bool                            seekCompleted;

    NEXUS_CallbackDesc              endOfStreamCallback;
    NEXUS_CallbackDesc              beginningOfStreamCallback;
    NEXUS_CallbackDesc              errorCallback;
    BIP_CallbackDesc                pbipOrPlaybackCallbackViaArbTimer;

    /* Common API State */
    BIP_TimerHandle                 hApiTimer;
    B_Time                          apiStartTime;
    bool                            apiTimerActive;

    /* One Arb per API */
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_PlayerSettings          *pSettings;
    } getSettingsApi;
    struct
    {
        BIP_ArbHandle               hArb;
        const BIP_PlayerSettings    *pSettings;
    } setSettingsApi;
    struct
    {
        BIP_ArbHandle               hArb;
        const char                  *pUrl;            /*!< [in] URL of media to be played */
        BIP_PlayerConnectSettings   *pSettings;       /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    } connectApi;
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_MediaInfoHandle         *phMediaInfo;
        const BIP_PlayerProbeMediaInfoSettings *pSettings;
    } probeMediaInfoApi;
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_PlayerStreamInfo        *pStreamInfo;
    } getProbedStreamInfo;
    struct
    {
        BIP_ArbHandle                           hArb;
        const BIP_PlayerPrepareSettings         *pPrepareSettings;
        const BIP_PlayerSettings                *pPlayerSettings;
        const BIP_PlayerProbeMediaInfoSettings  *pProbeSettings;
        const BIP_PlayerStreamInfo              *pStreamInfo;
        BIP_PlayerPrepareStatus                 *pPrepareStatus;
    } prepareApi;
    struct
    {
        BIP_ArbHandle               hArb;
        unsigned                    trackId;
        NEXUS_PidChannelHandle      *phPidChannel;
        const BIP_PlayerOpenPidChannelSettings     *pSettings;
    } openPidChannelApi;
    struct
    {
        BIP_ArbHandle               hArb;
    } closeAllPidChannelsApi;
    struct
    {
        BIP_ArbHandle               hArb;
        NEXUS_PidChannelHandle      hPidChannel;
    } closePidChannelApi;
    struct
    {
        BIP_ArbHandle               hArb;
        const BIP_PlayerStartSettings     *pSettings;
    } startApi;
    struct
    {
        BIP_ArbHandle               hArb;
    } stopApi;
    struct
    {
        BIP_ArbHandle               hArb;
    } playApi;
    struct
    {
        BIP_ArbHandle                           hArb;
        const BIP_PlayerPlayAtRateSettings      *pSettings;
        const char                              *pRate;
        int                                     rate;
    } playAtRateApi;
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_PlayerPauseSettings     *pSettings;
    } pauseApi;
    struct
    {
        BIP_ArbHandle               hArb;
        unsigned long               seekPositionInMs;
    } seekApi;
    struct
    {
        BIP_ArbHandle               hArb;
        bool                        forward;
    } playByFrameApi;
    struct
    {
        BIP_ArbHandle               hArb;
    } disconnectApi;
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_PlayerStatus            *pStatus;
    } getStatusApi;
    struct
    {
        BIP_ArbHandle                           hArb;
        BIP_PlayerGetStatusFromServerSettings   *pSettings;
        BIP_PlayerStatusFromServer              *pServerStatus;
    } getStatusFromServerApi;
    struct
    {
        BIP_ArbHandle               hArb;
    } printStatusApi;
    struct
    {
        BIP_ArbHandle               hArb;
    } destroyApi;

    BIP_PlayerStats                 stats;
} BIP_Player;

#define BIP_PLAYER_PRINTF_FMT  \
    "[hPlayer=%p state=%s subState=%s playerContainer=%s url=%s] "

#define BIP_PLAYER_PRINTF_ARG(pObj)             \
    (void *)(pObj),                                     \
    BIP_ToStr_BIP_PlayerState((pObj)->state),   \
    BIP_ToStr_BIP_PlayerSubState((pObj)->subState) \
    BIP_ToStr_BIP_PlayerContainerType((pObj)->streamInfo.containerType),        \
    (pObj)->hUrl->urlRaw

#define BIP_PLAYER_STATE_PRINTF_FMT  \
    "[hPlayer=%p state=%s subState=%s] "

#define BIP_PLAYER_STATE_PRINTF_ARG(pObj)                                       \
    (void *)(pObj),                                                                     \
    BIP_ToStr_BIP_PlayerState((pObj)->state),                                   \
    BIP_ToStr_BIP_PlayerSubState((pObj)->subState)

#define BIP_Player_IsVideoTrackAdded(hPlayer)   \
    (hPlayer)->pbipState.psi.videoCodec != NEXUS_VideoCodec_eNone && (hPlayer)->pbipState.psi.videoPid

#define BIP_Player_TrickmodeSupported(hPlayer)   \
    ( (hPlayer)->dataAvailabilityModel == BIP_PlayerDataAvailabilityModel_eLimitedRandomAccess ? true : \
      (hPlayer)->dataAvailabilityModel == BIP_PlayerDataAvailabilityModel_eFullRandomAccess ? true : false )

#define BIP_PLAYER_STATE(state) BIP_ToStr_BIP_PlayerState(state)

/* Forward declaration of state processing function */
void processPlayerState( void *hObject, int value, BIP_Arb_ThreadOrigin threadOrigin );
