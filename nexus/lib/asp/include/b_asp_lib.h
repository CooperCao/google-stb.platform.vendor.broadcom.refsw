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

#ifndef __B_ASP_LIB_H__
#define __B_ASP_LIB_H__

#include <stdio.h>
#include <unistd.h>
#include "b_os_lib.h"
#include "bkni.h"
#include "b_asp_lib_types.h"

/**
ASP Channel APIs used by the Streaming in/out Application to offload connection to/from ASP.
**/
typedef struct B_AspChannel* B_AspChannelHandle;

/**
Summary:
Initialize ASP Library.

Description:

Note: this API doesn't initialize NEXUS. That must be done by the caller before calling B_AspChannel_Create().
**/
void B_Asp_GetDefaultInitSettings(
    B_AspInitSettings                       *pSettings    /* [out] */
    );

B_Error B_Asp_Init(
    const B_AspInitSettings                 *pSettings
    );

/* API to un-initialize ASP library. */
void B_Asp_Uninit(
    void
    );

/**
Summary:
Create AspChannel object.
**/
void B_AspChannel_GetDefaultCreateSettings(
    B_AspStreamingProtocol                  streamingProtocol,
    B_AspChannelCreateSettings              *pSettings    /* [out] */
    );

/**
Summary:
Create a AspChannel.

Description:
This API should be called when Caller is ready to Offload Network Connection from Host to ASP. It creates
an instance of the ASP Streaming Channel.

Note: The connection must be offloaded before **any AV Data transfer begins** on this connection.

For caller using AspChannel with B_AspHttpStreaming Protocol & B_AspStreamingMode_eIn mode, it must call this API
after TCP connection is established & before it sends out the HTTP Request to the Server.

For caller using AspChannel with B_AspHttpStreaming Protocol & B_AspStreamingMode_eOut mode, it must call this API
after TCP connection is established, a Valid HTTP Request is received from the Client, and
before caller sends the HTTP Response.

In both cases, after creating the AspChannel, caller must call the B_AspChannel_SendPayload() to send out
HTTP Request (for HTTP StreamIn) & HTTP Response (for HTTP StreamOut).

Caller can then use the B_AspChannel_RecvPayload() to receive the HTTP Response for HTTP StreamIn mode (
after it gets the dataReadyCallback).

When caller is using AspChannel with UDP or RTP Type Streaming protocols, then it must create the UDP type
socket for relevant Streaming mode using BSD socket() API, call bind() & connect() API to associate the
local & remote IP/Port with the socket. Then caller must create the AspChannel using this socket.

This API does following:
-Validate that connection offload is allowed for given pSettings->mediaInfoSettings.transportType, otherwise, return error.
-Validate streamIn or streamOut mode settings.
-Validate DRM settings if enabled.
-Get 5-tuple (localIp, localPort, remoteIp, remotePort, layer4 protocol) associated with socketFdToOffload.
-Get receive & write queue size associated with the socketFdToOffload. These settings are needed to size the ASP FIFOs.
-Call NEXUS_AspChannel_Create() if caller didn't provide pSettings->hAsp NEXUS ASP Channel handle.
-Open AspNetFilter Driver and set the 5-tuple flow info via its ioctl command to capture any stray or re-assembled pkts.
-Freeze socketFdToOffload in Linux & retrieve TCP or UDP Connection State using multiple ioctl calls.
-Configure Network Switch to Redirect all incoming non-fragmented Ethernet frames for this 5-tuple flow to ASP HW (TCP/HTTP Only).

**/
B_AspChannelHandle B_AspChannel_Create(
    int                                     socketFdToOffload,     /* in: fd of socket to be offloaded from host to ASP.
                                                                      Socket will now be owned by ASP.
                                                                      Caller shouldn't use this socket anymore!
                                                                      TODO: clarify if this socket can/should be closed internally or by caller. */
    const B_AspChannelCreateSettings        *pSettings
    );

/**
Summary:
Destroy AspChannel object.

Description:
Stops streaming from ASP (if B_AspChannel_StopStreaming API wasn't called),
migrate connection to back to Linux, and return socketFd of the socket to
which connection is migrated back to if requested in pSocketFd.

**/
void B_AspChannel_Destroy(
    B_AspChannelHandle                      hAspChannel,
    int                                     *pSocketFd    /* out: fd of socket offloaded back from ASP to host. */
    );

/**
Summary:
Send Data via AspChannel to the network.

Description:

For caller using AspChannel with B_AspHttpStreaming Protocol & B_AspStreamingMode_eIn mode, it must call this API
to send the HTTP Request out.

For caller using AspChannel with B_AspHttpStreaming Protocol & B_AspStreamingMode_eOut mode, it must call this API
to send the HTTP Response to the network client.

This API will prepare the NEXUS_AspChannelStartSettings using Network settings from previous steps &
then call NEXUS_AspChannel_Start().

Returns:
== 0: for success & pBytesSent contains the number of bytes given to the ASP Channel for sending on to network.
<  0 for Error cases.

Note: This is a non-blocking API. If there is no space to buffer up the data, it will return an EAGAIN type
error code. Caller can send more data when it gets the spaceAvailableCallback.
**/
B_Error B_AspChannel_GetWriteBufferWithWrap(
    B_AspChannelHandle                  hAspChannel,
    void                                **pBuffer,   /* [out] attr{memory=cached} pointer to data buffer which can be written to network. */
    unsigned                            *pAmount,    /* [out] size of the available space in the pBuffer before the wrap. */
    void                                **pBuffer2,  /* [out] attr{null_allowed=y;memory=cached} optional pointer to data after wrap. */
    unsigned                            *pAmount2    /* [out] size of the available space in the pBuffer2 that can be written to network. */
    );

B_Error B_AspChannel_WriteComplete(
    B_AspChannelHandle                  hAspChannel,
    unsigned                            amount       /* number of bytes written to the buffer. */
    );


/**
Summary:
Recv Data via AspChannel from the network.

Description:

For caller using AspChannel with B_AspHttpStreaming Protocol & B_AspStreamingIn mode, it must call this API
after calling B_Asp_WriteComplete() to receive the HTTP response back from server.

Returns:
== 0: for success & pBytesReceived contains the number of bytes received using ASP Channel from the network.
<  0 for Error cases.

Note: This is a non-blocking API. If there is no data available, it will return an EAGAIN type
error code. Caller can send more data when it gets the dataReadyCallback.
**/
B_Error B_AspChannel_GetReadBufferWithWrap(
    B_AspChannelHandle                  hAspChannel,
    const void                          **pBuffer,   /* [out] attr{memory=cached} pointer to buffer containing data read from network. */
    unsigned                            *pAmount,    /* [out] number of bytes available in the data buffer pBuffer. */
    const void                          **pBuffer2,  /* [out] attr{null_allowed=y;memory=cached} pointer to buffer after wrap containing data read from network. */
    unsigned                            *pAmount2    /* [out] number of bytes available in the data buffer pBuffer2. */
    );

B_Error B_AspChannel_ReadComplete(
    B_AspChannelHandle                  hAspChannel,
    unsigned                            bytesRead    /* number of bytes read/consumed by the caller. */
    );


/**
Summary:
Start StreamingIn to Playpump or StreamingOut from Recpump.

Description:
If B_AspChannelStartSettings.mode == B_AspChannelStreamingMode_eIn, data flows from
    Network -> NEXUS_AspCh (XPT ASP Ch) -> [NEXUS_Dma (XPT M2M DMA)] -> Playpump (XPT Playback Ch).

If B_AspChannelStartSettings.mode == B_AspChannelStreamingMode_eOut, data flows from
    NEXUS_Recpump (RAVE Ctx) ->  NEXUS_AspCh (XPT ASP Ch) -> Network.

If caller hasn't called B_AspChannel_SendPayload() API before calling this API, it will
prepare the NEXUS_AspChannelStartSettings using Network settings from previous steps &
then call NEXUS_AspChannel_Start(). This will notifty ASP to start streaming.

Otherwise, it will just notify ASP to start streaming.
**/
B_Error B_AspChannel_StartStreaming(
    B_AspChannelHandle                      hAspChannel
    );

/**
Summary:
Stop Streaming flow for an AspChannel.

ASP FW will stop feeding AV Stream into XPT Playback or outof XPT Rave pipe.
**/
void B_AspChannel_StopStreaming(
    B_AspChannelHandle                      hAspChannel
    );

/**
Summary:
API to Get Current Settings.
**/
void B_AspChannel_GetSettings(
    B_AspChannelHandle                      hAspChannel,
    B_AspChannelSettings                    *pSettings   /* [out] */
    );


/**
Summary:
API to Update Current Settings.
**/
B_Error B_AspChannel_SetSettings(
    B_AspChannelHandle                      hAspChannel,
    const B_AspChannelSettings              *pSettings
    );


/**
Summary:
API to Get Current Status.
**/
B_Error B_AspChannel_GetStatus(
    B_AspChannelHandle                      hAspChannel,
    B_AspChannelStatus                      *pStatus     /* [out] */
    );

/**
Summary:
API to Get Current Status.
**/
B_Error B_AspChannel_PrintStatus(
    B_AspChannelHandle                      hAspChannel
    );

/**
Summary:
API to Get Current DTCP-IP Settings
**/
void B_AspChannel_GetDtcpIpSettings(
    B_AspChannelHandle                      hAspChannel,
    B_AspChannelDtcpIpSettings              *pSettings /* [out] */
    );

/**
Summary:
API to Set Current DTCP-IP Settings.  This must be called prior to B_AspChannel_Start().
**/
B_Error B_AspChannel_SetDtcpIpSettings(
    B_AspChannelHandle                      hAspChannel,
    const B_AspChannelDtcpIpSettings        *pSettings
    );

#endif /* __B_ASP_LIB_H__*/
