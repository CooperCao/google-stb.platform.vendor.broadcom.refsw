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
#ifndef __B_ASP_OUTPUT_H__
#define __B_ASP_OUTPUT_H__

#include "b_asp_output_types.h"

/**
ASP Output APIs used by the Streaming out Application to output AV data to network clients.
**/
typedef struct B_AspOutput *B_AspOutputHandle;

/**
Summary:
Get Default create settings.
**/
void B_AspOutput_GetDefaultCreateSettings(
    B_AspOutputCreateSettings               *pSettings    /* [out] */
    );

/**
Summary:
Create an AspOutput.

**/
B_AspOutputHandle B_AspOutput_Create(
    const B_AspOutputCreateSettings         *pSettings   /* [in] */
    );

/**
Summary:
API to Get Current Settings.
**/
void B_AspOutput_GetSettings(
    B_AspOutputHandle                       hAspOutput,
    B_AspOutputSettings                     *pSettings   /* [out] */
    );


/**
Summary:
API to Update Current Settings.
**/
NEXUS_Error B_AspOutput_SetSettings(
    B_AspOutputHandle                       hAspOutput,
    const B_AspOutputSettings               *pSettings   /* [in] */
    );


/**
Summary:
API to Get Current DTCP-IP Settings
**/
void B_AspOutput_GetDtcpIpSettings(
    B_AspOutputHandle                       hAspOutput,
    B_AspOutputDtcpIpSettings               *pSettings /* [out] */
    );

/**
Summary:
API to Set Current DTCP-IP Settings.  This must be called prior to B_AspOutput_Start().
**/
NEXUS_Error B_AspOutput_SetDtcpIpSettings(
    B_AspOutputHandle                       hAspOutput,
    const B_AspOutputDtcpIpSettings         *pSettings  /* [in] */
    );

/**
Summary:
API to Get Default ConnectHttpSettings
**/
void B_AspOutput_GetDefaultConnectHttpSettings(
    B_AspOutputConnectHttpSettings          *pSettings /* [out] */
    );

/**
Summary:
API to Connect to AspOutput.

This API migrates an existing network socket state from Linux to the ASP.
After this, ASP becomes responsible for all network activity for that connection.

Note: After the successfull return from this API, App must not use the socketFd
until it calls B_AspOutput_Disconnect().

Note2:
This API should be called when Caller is ready to Offload Network Connection from Host to ASP.

The connection must be offloaded BEFORE ANY AV DATA TRANSFER BEGINS on this connection.

App can call this API either immediately after TCP connection is accepted,
or after it has also received a valid HTTP Request from a Client.

**/
NEXUS_Error B_AspOutput_ConnectHttp(
    B_AspOutputHandle                       hAspOutput,
    int                                     socketFdToOffload,  /* [in] */
    const B_AspOutputConnectHttpSettings    *pSettings          /* [in] */
    );


/**
Summary:
API to receive the incoming HTTP Request (from remote) for host access.

The incoming HttpRequest is accessed one buffer at a time with the following APIs.
If the HttpRequest spans more than one buffer, each buffer must be completely consumed
(either parsed or copied) before the next buffer can be accessed.

The buffer containing the end of the HttpRequest can be partially consumed by specifying
the number of bytes that belong to the HttpRequest.  Any remaining bytes will be treated as payload.

This is an optional API that is NOT needed if App chooses to directly receive the HTTP Request
using Linux socket APIs & before calling B_AspOutput_Connect().

This API is intended to be called after the application receives a B_AspOutputSettings.httpRequestDataReady callback.

In the case that a partial HTTP request is returned, the caller should wait for the next httpRequestDataReady callback,
then call this API again to receive additional data.

Note: This API can only be called when B_AspOutputStatus.state is _eConnected.

**/
NEXUS_Error B_AspOutput_GetHttpRequestData(
    B_AspOutputHandle                   hAspOutput,
    const void                          **pBuffer,      /* [out] pointer to buffer containing data read from network. */
    unsigned                            *pByteCount     /* [out] number of bytes available in the data buffer pBuffer. */
    );


NEXUS_Error B_AspOutput_HttpRequestDataConsumed(
    B_AspOutputHandle                   hAspOutput,
    bool                                requestCompleted,     /* [in] false => Entire buffer is consumed. End of HttpRequest not found, more data is required. */
                                                              /*      true  => End of the HttpRequest has been found and consumed. No more data is required. */

    unsigned                            bytesConsumed         /* [in] If requestComplete is true this is the number of bytes consumed from the current buffer.*/
    );                                                       /*       Else bytesConsumed must be equal to byte count returned by NEXUS_AspOutput_GetHttpRequestData. */


/**
Summary:
API to provide an HTTP response to be sent out on the network.

Note: This API can only be called when B_AspOutputStatus.state is _eConnected.

**/
NEXUS_Error B_AspOutput_SendHttpResponse(
    B_AspOutputHandle                       hAspOutput,
    void                                    *pBuffer,       /* [in] pointer to HTTP response to be sent to network. */
    unsigned                                byteCount       /* [in] number of bytes in HTTP Response buffer. */
    );


/**
Summary:
Get Default StartSettings.
**/
void B_AspOutput_GetDefaultStartSettings(
    B_AspOutputStartSettings                *pSettings      /* [out] */
    );


/**
Summary:
Start an AspOutput.

AspOutput will start streaming out after this API returns.

Note: This API can only be called when B_AspOutputStatus.state is _eConnected.
AspOutput changes its state to _eStreaming when it returns from this API. This can be
checked via the B_AspOutputStatus.state variable obtained from _AspOutput_GetStatus().

**/
NEXUS_Error B_AspOutput_Start(
    B_AspOutputHandle                       hAspOutput,
    const B_AspOutputStartSettings          *pSettings
    );


/**
Summary:
API to Get Current Status.

**/
NEXUS_Error B_AspOutput_GetStatus(
    B_AspOutputHandle                       hAspOutput,
    B_AspOutputStatus                       *pStatus     /* [out] */
    );


/**
Summary:
API to provide AV buffer to the caller before ASP sends it out.

This is an optional API that is NOT needed in the default feed mode:
    B_AspOutputStartSettings.feedMode == B_AspOutputFeedMode_eAuto.

When B_AspOutputStartSettings.feedMode == B_AspOutputFeedMode_eHost

    This API returns a buffer available to be filled with data to be output.
    After the caller fills the buffer, it must call B_AspOutput_BufferSubmit()
    to submit the buffer to ASP which will then send it out to the network.

When B_AspOutputStartSettings.feedMode == B_AspOutputFeedMode_eAutoWithHostEncrytion

    This API returns a buffer that contains current data from Recpump before it has
    been sent to the network.  It allows the application to encrypt the data (in place).
    After the caller encrypts the buffer, it must call B_AspOutput_BufferSubmit()
    to return the buffer back to ASP which will then send it out to the network.

Note: This API can only be called when B_AspOutputStatus.state is _eStreaming or _eStreamingDone.

**/
NEXUS_Error B_AspOutput_GetBufferWithWrap(
    B_AspOutputHandle                       hAspOutput,
    void                                    **pBuffer,   /* [out] attr{memory=cached} pointer to data buffer which can be written to network. */
    unsigned                                *pByteCount, /* [out] size of the available space in the pBuffer before the wrap. */
    void                                    **pBuffer2,  /* [out] attr{null_allowed=y;memory=cached} optional pointer to data after wrap. */
    unsigned                                *pByteCount2 /* [out] size of the available space in the pBuffer2 that can be written to network. */
    );

/**
Summary:
API to submit a buffer back to AspOuput.

This allows app to notify AspOutput that app is done with the all or some of the buffer
(indicated via the byteCount parameter).
This buffer was previously obtained using the B_AspOutput_GetBufferWithWrap() API.

Note: This API can only be called when B_AspOutputStatus.state is _eStreaming or _eStreamingDone.

**/
NEXUS_Error B_AspOutput_BufferSubmit(
    B_AspOutputHandle                       hAspOutput,
    unsigned                                byteCount   /* number of bytes that AspOutput can send to the network. */
    );


/**
Summary:
Stop an AspOutput.

Description:
This API stops the data transmission by the AspOutput.
It does NOT synchronize the protocol state with the network peer.

Note: AspOutput changes its state to _Connected when it returns from this API. This can be
checked via the B_AspOutputStatus.state variable obtained from _AspOutput_GetStatus().

For normal stopping, caller should use B_AspOutput_Finish().

**/
void B_AspOutput_Stop(
    B_AspOutputHandle                       hAspOutput
    );


/**
Summary:
API to cleanly stop an AspOutput.

Description:
This API initiates the finishing of data transmission by the AspOutput & returns immediately
to the caller.

It will synchronize the protocol state with the network peer (if required for a protocol).
E.g. This may involve waiting for TCP ACKs for any pending data and thus may take time.
AspOutput will invoke the statusChanged callback when finished.

Note: AspOutput changes its state to _Connected when output is cleaned stopped. This can be
checked via the B_AspOutputStatus.state variable obtained from _AspOutput_GetStatus().

For immediate (un-clean) stopping, app should use B_AspOutput_Stop().
**/
void B_AspOutput_Finish(
    B_AspOutputHandle                       hAspOutput
    );

/**
Summary:
API to Get Current Status.
**/
void B_AspOutput_PrintStatus(
    B_AspOutputHandle                       hAspOutput
    );

/**
Summary:
API to disconnect from AspOutput.

This call transfers the ownership of connection from ASP to Host.
It recreates the connection state back in the Linux socket that was
passed in the B_AspOutput_Connect*().

After this API returns, caller must close this socket using "close()".
TODO: clarify if this socket can be used for further sending or receiving.

**/
NEXUS_Error B_AspOutput_Disconnect(
    B_AspOutputHandle                       hAspOutput,
    B_AspOutputDisconnectStatus             *pStatus
    );

/**
Summary:
Destroy AspOutput object.

Description:
Stops and Destroys an AspOutput. The handle can no longer be used.

**/
void B_AspOutput_Destroy(
    B_AspOutputHandle                       hAspOutput
    );

#endif /* __B_ASP_OUTPUT_H__*/
