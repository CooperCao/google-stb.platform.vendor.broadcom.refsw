/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

#ifndef PLAYER_CMD_H

#ifndef NXCLIENT_SUPPORT
#include "nexus_platform.h"
#else
#include "nxclient.h"
#endif

#include "bip.h"

#define JUMP_OFFSET_IN_MS 30000
typedef enum
{
    PlayerCmd_ePlay,
    PlayerCmd_ePause,
    PlayerCmd_eSeek,
    PlayerCmd_eRelativeSeekFwd,
    PlayerCmd_eRelativeSeekRev,
    PlayerCmd_ePlayAtRate,
    PlayerCmd_ePlayAtRawRate,
    PlayerCmd_eFrameAdvance,
    PlayerCmd_eFrameReverse,
    PlayerCmd_ePrintStatus,
    PlayerCmd_ePrintServerStatus,
    PlayerCmd_ePrintAllStatus,
    PlayerCmd_ePlayNewUrl,
    PlayerCmd_ePlayLanguageSpecificTrack,
    PlayerCmd_ePlayBsmodSpecificTrack,
    PlayerCmd_eStressTrickModes,
    PlayerCmd_eQuit,
    PlayerCmd_eMax
} PlayerCmd;

typedef struct
{
    unsigned    seekPositionInMs;
    char        playSpeed[8];
    PlayerCmd   cmd;
    PlayerCmd   prevCmd;
    int rate;
} CmdOptions;

typedef enum BMediaPlayerState
{
    BMediaPlayerState_eDisconnected,
    BMediaPlayerState_eWaitingForConnect,
    BMediaPlayerState_eWaitingForProbe,
    BMediaPlayerState_eWaitingForPrepare,
    BMediaPlayerState_eWaitingForStart,
    BMediaPlayerState_eStarted,
    BMediaPlayerState_ePaused,
    BMediaPlayerState_eWaitingForSeek,
    BMediaPlayerState_eMax
} BMediaPlayerState;

#define DTCP_IP_KEY_FORMAT_COMMON_DRM   "commonDrm"
#define DTCP_IP_KEY_FORMAT_TEST         "test"
typedef struct AppCtx
{
    /* Command line options. */
    BIP_StringHandle                hInterfaceName;
    BIP_StringHandle                hUrl;
    BIP_StringHandle                hDtcpIpKeyFormat;
    BIP_StringHandle                hLanguage;
    BIP_MediaInfoAudioAc3Bsmod      ac3ServiceType; /* Default is UINT_MAX which means undefined*/

    bool                            enableContinousPlay;
    bool                            enableDtcpIp;
    bool                            printStatus;
    bool                            printServerStatus;
    bool                            disableVideo;
    bool                            disableAudio;
    bool                            startPaused;
    bool                            enableTimeshifting;
    unsigned short                  maxWidth;
    unsigned short                  maxHeight;
    unsigned                        initialPlaybackPositionInMs;
    unsigned                        jumpOffsetInMs;
    unsigned                        playTimeInMs;
    B_Time                          playStartTime;
    bool                            playStartTimeSet;
    unsigned                        streamDurationInMs;
    unsigned                        timeshiftBufferMaxDurationInMs;

    NEXUS_VideoFormat               displayFormat;
    BIP_PlayerHandle                hPlayer;
#ifdef NXCLIENT_SUPPORT
    /* Cached Handles. */
    unsigned                        connectId;
    NxClient_AllocResults           allocResults;
    unsigned                        audioConnectId;
    NxClient_AllocResults           audioAllocResults;
    NEXUS_SimpleVideoDecoderHandle  hSimpleVideoDecoder;
    NEXUS_SimpleAudioDecoderHandle  hSimpleAudioDecoder;
    NEXUS_SimpleStcChannelHandle    hSimpleStcChannel;
#else
    NEXUS_PlatformSettings          platformSettings;
#endif

    BMediaPlayerState               playerState;
    B_EventHandle                   hCompletionEvent;
    BIP_Status                      asyncApiCompletionStatus;
    BIP_CallbackDesc                asyncCallbackDesc;  /* async completion callback. */
    BIP_MediaInfoHandle             hMediaInfo;
    BIP_PlayerStreamInfo            playerStreamInfo;
    BIP_PlayerPrepareStatus         prepareStatus;
    bool                            mediaProbeStarted;
    bool                            prepareStarted;
    bool                            playbackDone;
    bool                            userQuit;

    unsigned                        lastSeekPositionInMs;
    bool                            usePlaypump;
    bool                            usePlayback;
    bool                            enablePayloadScanning;
    bool                            enableAutoPlayAfterStartingPaused;
    bool                            stressTrickModes;
    bool                            disableDynamicTrackSelection;

#define MAX_IP_NETWORK_JITTER_IN_MS 500
    uint32_t                        maxIpNetworkJitterInMs;
    bool                            enableLowLatencyMode;
    int                             clockRecoveryMode;
    int                             audioDecoderLatencyMode;
    int                             disablePrecisionLipsync;
    int                             stcSyncMode;
    bool                            disableTsm;
    bool                            enableAudioPrimer;
    unsigned                        trackGroupIndex;
    bool                            bufDepthInMsec;
    bool                            sineTone;
    bool                            enableHwOffload;
    bool                            enableTransportTimestamps;
    BIP_StringHandle                hRecordFile;
} AppCtx;

void unInitAppCtx( AppCtx *pAppCtx);
AppCtx *initAppCtx( void );
BIP_Status parseOptions(int argc, char *argv[], AppCtx *pAppCtx);
BIP_Status runTimeCmdParsing(AppCtx *pAppCtx,CmdOptions *pCmdOptions);
BIP_Status stressTrickModes(AppCtx *pAppCtx);
bool playerGetTrackOfType( BIP_MediaInfoHandle hMediaInfo, BIP_MediaInfoTrackType trackType, unsigned trackGroupIndex, BIP_MediaInfoTrack *pMediaInfoTrackOut);
bool playerGetSpecialAudioTrackType(BIP_MediaInfoHandle hMediaInfo, BIP_MediaInfoTrack *pMediaInfoTrackOut, const char *language, BIP_MediaInfoAudioAc3Bsmod serviceType);


#endif /* #ifndef PLAYER_CMD_H */
