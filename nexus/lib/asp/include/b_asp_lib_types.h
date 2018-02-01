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

#include <stdio.h>
#include "nexus_types.h"
#include "nexus_asp.h"

typedef struct B_AspInitSettings
{
    unsigned                                userId;     /* Credentials to run ASP Manager Daemon. */
    unsigned                                groupid;
} B_AspInitSettings;

typedef enum B_AspStreamingProtocol
{
    B_AspStreamingProtocol_eHttp,
    B_AspStreamingProtocol_eUdp,
    B_AspStreamingProtocol_eRtp,
    B_AspStreamingProtocol_eMax
} B_AspStreamingProtocol;

typedef enum B_AspStreamingMode
{
    B_AspStreamingMode_eIn,           /* ASP Channel is used for receiving & playing AV data from network. */
    B_AspStreamingMode_eOut,          /* ASP Channel is used for streaming out AV data to network. */
    B_AspStreamingMode_eMax
} B_AspStreamingMode;

typedef struct B_AspHttpSettings
{
    struct {
        unsigned                            major;
        unsigned                            minor;
    } version;
    bool                                    enableChunkTransferEncoding;
    unsigned                                chunkSize;
} B_AspHttpSettings;

typedef struct B_AspUdpSettings
{
    bool                                    disableChecksum;
} B_AspUdpSettings;

typedef struct B_AspRtpSettings
{
    B_AspUdpSettings                        udp;
    unsigned                                startingSequenceNumber;
    /* TODO: add more RTP related settings. */
} B_AspRtpSettings;

typedef struct B_AspChannelDtcpIpSettings
{
    NEXUS_AspChannelDtcpIpSettings          settings;
} B_AspChannelDtcpIpSettings;

/**
Summary:
B_AspChannel_Start related settings.
**/
typedef struct B_AspChannelCreateSettings
{
    NEXUS_AspChannelCreateBufferSettings    writeFifo;                      /* Defaults to 4K. Need to be tuned if host needs to directly write/stream-out AV instead of ASP FW streaming from the Recpump.
                                                                               See detailed usage comment in the nexus_asp_types.h.
                                                                               Caller should tune the socket write & read queue sizes according to its streaming environment. */

    B_AspStreamingProtocol                  protocol;
    union {
        B_AspHttpSettings                   http;                           /* Applies when protocol==B_AspStreamingProtocol_eHttp */
        B_AspUdpSettings                    udp;                            /* Applies when protocol==B_AspStreamingProtocol_eUdp  */
        B_AspRtpSettings                    rtp;                            /* Applies when protocol==B_AspStreamingProtocol_eRtp  */
    } protocolSettings;

    B_AspStreamingMode                      mode;
    struct {
        struct {                                                            /* Applies when mode==B_AspStreamingMode_eIn */
            NEXUS_PlaypumpHandle            hPlaypump;                      /* Playpump Handle is required. */
            NEXUS_DmaHandle                 hDma;                           /* DMA handle is only required if Stream needs to be decrypted. */
            NEXUS_RecpumpHandle             hRecpump;                       /* Optional Recpump Handle: for debug purposes. */
        } streamIn;

        struct {                                                            /* Applies when mode==B_AspStreamingMode_eOut */
            NEXUS_RecpumpHandle             hRecpump;                       /* Recpump handle is required. */
            NEXUS_PlaypumpHandle            hPlaypump;                      /* Optional Playpump Handle: for debug purposes. */
        } streamOut;
    } modeSettings;

    NEXUS_AspChannelHandle                  hAsp;                           /* Optional: if NULL, it will be internally opened. */

    struct {
        NEXUS_TransportType                 transportType;                  /* Required */
        unsigned                            maxBitRate;                     /* Optional: should be set if available. */
    } mediaInfoSettings;

    NEXUS_AspChannelDrmType                 drmType;                        /* Defaults to No DRM. */
    NEXUS_KeySlotHandle                     hDrmKeySlot;                    /* Required if DRM is selected. KeySlot Handle where DRM Module will load the key. */
} B_AspChannelCreateSettings;

/**
Summary:
B_AspChannel_Get/_Set related settings.
**/
typedef struct B_AspChannelSettings
{
    NEXUS_CallbackDesc                      dataReady;                      /* invoked when new data is available from the network. */
    NEXUS_CallbackDesc                      spaceAvailable;                 /* invoked when new data can be written to network. */
    NEXUS_CallbackDesc                      stateChanged;                   /* invoked when AspChannel State changes. Call B_AspChannel_GetStatus() to get the new state. */
    NEXUS_CallbackDesc                      noMoreDataFromRemote;           /* invoked when remote peer has indicated that it has no more data to send (by sending TCP FIN). */
} B_AspChannelSettings;

typedef enum B_AspChannelState
{
    /* TODO: this needs another look. */
    B_AspChannelState_eIdle = 0,                                            /* channel is idle either after _Create() or _Stop() call. */
    B_AspChannelState_eCreated = B_AspChannelState_eIdle,                   /* channel is created either after B_AspChannel_Create() or _Stop() call. */
    B_AspChannelState_eStarted,                                             /* channel is started after _Start(). */
    B_AspChannelState_eWaitingForStreamingStart,                            /* channel is waiting for Streaming to start. */
    B_AspChannelState_eStartedStreaming,                                    /* channel is Streaming AV. */
    B_AspChannelState_eRcvdEndOfStream,                                     /* channel has received EndOfStream signal (TCP FIN) from network peer. */
    B_AspChannelState_eNetworkError,                                        /* channel has received Error (TCP RST) from network peer. */
    B_AspChannelState_eWaitingForStreamingStop,                             /* channel is waiting to for stop sequence to complete. */
    B_AspChannelState_eWaitingForAbort,                                     /* channel is waiting to for abort sequence to complete. */
    B_AspChannelState_eStoppedStreaming,                                    /* channel has stopped streaming. */
    B_AspChannelState_eMax
} B_AspChannelState;

/**
Summary:
B_AspChannel_GetStatus related.
**/
typedef struct B_AspChannelStatus
{
    B_AspChannelState                   state;
    NEXUS_AspChannelStatus              nexusStatus;
    /* TODO: add network state. */
} B_AspChannelStatus;
