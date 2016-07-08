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

#ifndef BIP_UDP_STREAMER_IMPL_H
#define BIP_UDP_STREAMER_IMPL_H

#include "bip_priv.h"
#include "bip_streamer_impl.h"
#include "b_playback_ip_lib.h"

/*
 * States for UdpStreamer Object (main server object),
 *  states reflecting UdpStreamer APIs visiable to Apps: Create, Start, Stop of Server
 */
typedef enum BIP_UdpStreamerOutputState
{
    BIP_UdpStreamerOutputState_eNotSet,                /* Streamer Output is not set or cleared. */
    BIP_UdpStreamerOutputState_eSet,                   /* Streamer Output is set. */
    BIP_UdpStreamerOutputState_eMax
} BIP_UdpStreamerOutputState;

typedef enum BIP_UdpStreamerState
{
    BIP_UdpStreamerState_eUninitialized = 0,           /* Initial state: object is just created but not fully initialzed. */
    BIP_UdpStreamerState_eIdle,                        /* Idle state: streamer object is either just created or is just stopped. */
    BIP_UdpStreamerState_eSetupComplete,               /* Transitional state when UdpStreamer_Start is called before we switch to WaitingForProcessRequestApi. */
    BIP_UdpStreamerState_eStreaming,                   /* Streamer is now streaming the media. */
    BIP_UdpStreamerState_eStreamingDone,               /* Transitional state: Streamer is now done streaming the media and letting us know via the callback. */
    BIP_UdpStreamerState_eWaitingForStopApi,           /* Streamer reached either EOF or an Error, notified endOfStreaming Callback to app and now waiting for Stop(). */
    BIP_UdpStreamerState_eMax
} BIP_UdpStreamerState;

typedef struct BIP_UdpStreamer
{
    BDBG_OBJECT( BIP_UdpStreamer )

    B_MutexHandle                               hStateMutex;                          /* Mutex to synchronize a API invocation with callback invocation */
    BIP_Status                                  completionStatus;

    BIP_StreamerHandle                          hStreamer;
    const BIP_Streamer                          *pStreamer;

    /* Cached Settings. */
    BIP_UdpStreamerCreateSettings              createSettings;
    BIP_UdpStreamerSettings                    settings;
    BIP_UdpStreamerStartSettings               startSettings;

    /* UdpStreamer & its sub-states. */
    BIP_UdpStreamerState                       state;                                /* Main Streamer state. */
    struct
    {
        BIP_UdpStreamerOutputState             state;                          /* Streamer sub-state for its output. */
        BIP_UdpStreamerProtocol                streamerProtocol;
        BIP_UdpStreamerOutputSettings          settings;
        BIP_StringHandle                       hIpAddress;
        BIP_StringHandle                       hPort;
        BIP_StringHandle                       hInterfaceName;
    } output;
    struct
    {
        B_PlaybackIpFileStreamingHandle             hFileStreamer;
        B_PlaybackIpLiveStreamingHandle             hLiveStreamer;
        BIP_CallbackDesc                            pbipEndOfStreamingCallback;
    } playbackIpState;

    /* One Arb per API */
    struct
    {
        BIP_ArbHandle                           hArb;
        BIP_UdpStreamerSettings                *pSettings;
    } getSettingsApi;
    struct
    {
        BIP_ArbHandle                           hArb;
        BIP_UdpStreamerSettings                *pSettings;
    } setSettingsApi;
    struct
    {
        BIP_ArbHandle                           hArb;
        const char                              *pMediaFileAbsolutePathName;
        BIP_StreamerStreamInfo                  *pStreamerStreamInfo;
        BIP_StreamerFileInputSettings           *pFileInputSettings;
    } fileInputSettingsApi;
    struct
    {
        BIP_ArbHandle                           hArb;
        NEXUS_ParserBand                        hParserBand;
        BIP_StreamerStreamInfo                  *pStreamerStreamInfo;
        BIP_StreamerTunerInputSettings          *pTunerInputSettings;
    } tunerInputSettingsApi;
    struct
    {
        BIP_ArbHandle                           hArb;
        NEXUS_RecpumpHandle                     hRecpump;
        BIP_StreamerRecpumpInputSettings        *pRecpumpInputSettings;
    } recpumpInputSettingsApi;
    struct
    {
        BIP_ArbHandle                           hArb;
        unsigned                                programIndex;
    } setProgramApi;
    struct
    {
        BIP_ArbHandle                           hArb;
        BIP_StreamerTrackInfo                   *pStreamerTrackInfo;
        BIP_StreamerTrackSettings               *pTrackSettings;
    } addTrackApi;
    struct
    {
        BIP_ArbHandle                           hArb;
        BIP_UdpStreamerProtocol                 streamerProtocol;
        const char                              *pStreamerIpAddress;
        const char                              *pStreamerPort;
        const char                              *pStreamerInterfaceName;
        BIP_UdpStreamerOutputSettings           *pOutputSettings;
    } outputSettingsApi;
    struct
    {
        BIP_ArbHandle                           hArb;
        BIP_TranscodeProfile                    *pTranscodeProfile;
    } addTranscodeProfileApi;
    struct
    {
        BIP_ArbHandle                           hArb;
        BIP_TranscodeNexusHandles               *pTranscodeNexusHandles;
    } setTranscodeNexusHandlesApi;
    struct
    {
        BIP_ArbHandle                           hArb;
        BIP_UdpStreamerStartSettings           *pSettings;
    } startApi;
    struct
    {
        BIP_ArbHandle                           hArb;
    } stopApi;
    struct
    {
        BIP_ArbHandle                           hArb;
        BIP_UdpStreamerStatus                  *pStatus;
    } getStatusApi;
    struct
    {
        BIP_ArbHandle                           hArb;
    } destroyApi;

    BIP_UdpStreamerStats                       stats;
} BIP_UdpStreamer;

#define BIP_UDP_STREAMER_PRINTF_FMT  \
    "[hUdpStreamer=%p hStreamer=%p State=%s IP=%s Port=%s Proto=%s Iface=%s endOfStrmCB=%s]"

#define BIP_UDP_STREAMER_PRINTF_ARG(pObj)                                                               \
    (void *)(pObj),                                                                                             \
    (void *)(pObj->hStreamer),                                                                                  \
    (pObj)->state==BIP_UdpStreamerState_eIdle                  ? "Idle"                            :    \
    (pObj)->state==BIP_UdpStreamerState_eStreaming             ? "Streaming"                       :    \
    (pObj)->state==BIP_UdpStreamerState_eStreamingDone         ? "StreamingDone"                   :    \
    (pObj)->state==BIP_UdpStreamerState_eWaitingForStopApi     ? "WaitingForStopApi"               :    \
                                                                  "StateNotMapped",                     \
                                                                                                        \
    (pObj)->output.hIpAddress?BIP_String_GetString((pObj)->output.hIpAddress):"",                       \
    (pObj)->output.hPort?BIP_String_GetString((pObj)->output.hPort):"",                                 \
                                                                                                        \
    (pObj)->output.streamerProtocol==BIP_UdpStreamerProtocol_ePlainUdp   ? "PlaingUDP"   :              \
    (pObj)->output.streamerProtocol==BIP_UdpStreamerProtocol_eRtp      ?   "RTP"         :              \
                                                                           "<undefined>",               \
    (pObj)->output.hInterfaceName?BIP_String_GetString((pObj)->output.hInterfaceName):"",               \
    (pObj)->createSettings.endOfStreamingCallback.callback  ?  "Y" : "N"

#endif /* BIP_UDP_STREAMER_IMPL_H */
