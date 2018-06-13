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
#ifndef __B_ASP_INPUT_H__
#define __B_ASP_INPUT_H__

#include "b_asp_input_types.h"

/**
ASP Input APIs used by the Streaming in Application to input AV data from network clients.
**/
typedef struct B_AspInput *B_AspInputHandle;

/**
Summary:
Get Default create settings.
**/
void B_AspInput_GetDefaultCreateSettings(
    B_AspInputCreateSettings               *pSettings    /* [out] */
    );

/**
Summary:
Create an AspInput.

**/
B_AspInputHandle B_AspInput_Create(
    const B_AspInputCreateSettings         *pSettings   /* [in] */
    );

/**
Summary:
API to Get Current Settings.
**/
void B_AspInput_GetSettings(
    B_AspInputHandle                       hAspInput,
    B_AspInputSettings                     *pSettings   /* [out] */
    );


/**
Summary:
API to Update Current Settings.
**/
NEXUS_Error B_AspInput_SetSettings(
    B_AspInputHandle                       hAspInput,
    const B_AspInputSettings               *pSettings   /* [in] */
    );


/**
Summary:
API to Get Current DTCP-IP Settings
**/
void B_AspInput_GetDtcpIpSettings(
    B_AspInputHandle                       hAspInput,
    B_AspInputDtcpIpSettings               *pSettings /* [out] */
    );

/**
Summary:
API to Set Current DTCP-IP Settings.  This must be called prior to B_AspInput_Start().
**/
NEXUS_Error B_AspInput_SetDtcpIpSettings(
    B_AspInputHandle                       hAspInput,
    const B_AspInputDtcpIpSettings         *pSettings  /* [in] */
    );

/**
Summary:
API to Get Default ConnectHttpSettings
**/
void B_AspInput_GetDefaultConnectHttpSettings(
    B_AspInputConnectHttpSettings          *pSettings /* [out] */
    );

/**
Summary:
API to Connect an AspInput.

This API migrates an existing network socket state from Linux to the ASP.
After this, ASP becomes responsible for all network activity for that connection.

Note: After the successfull return from this API, App must not use the socketFd
until it calls B_AspInput_Disconnect().

Note2:
This API should be called when Caller is ready to Offload the Network Connection from Host to ASP.

The connection must be offloaded BEFORE ANY AV DATA TRANSFER BEGINS on this connection.

App must call this API immediately after TCP connection is established to the server.
It should prepare the outgoing valid HTTP Request but use the B_AspInput_SendHttpRequest()
to send it to the server. App can't directly send the HTTP Request to the server.
Likewise, it must use the B_AspInput_RecvHttpResponseData() to receive the HTTP Response.

HTTP Request & Response needs to be sent & received via ASP because server starts streaming
immediately after sending the HTTP Response. This makes connection offload from Linux not
clean as host will drop all packets being received during the connection migration.
Thus server will need to retransmit these packets which will cause initial latency & waste
network bandwidth too.

**/
NEXUS_Error B_AspInput_ConnectHttp(
    B_AspInputHandle                        hAspInput,
    int                                     socketFdToOffload,  /* [in] */
    const B_AspInputConnectHttpSettings     *pSettings          /* [in] */
    );


/**
Summary:
API to provide an HTTP request to be sent out on the network.

Note: This API can only be called when B_AspInputStatus.state is _eConnected.

**/
NEXUS_Error B_AspInput_SendHttpRequest(
    B_AspInputHandle                        hAspInput,
    void                                    *pBuffer,       /* [in] pointer to HTTP request to be sent to network. */
    unsigned                                byteCount       /* [in] number of bytes in HTTP Requesat buffer. */
    );


/**
Summary:
API to receive the incoming HTTP Response (from remote) for host access.

The incoming HttpResponse is accessed one buffer at a time with the following APIs.
If the HttpResponse spans more than one buffer, each buffer must be completely consumed
(either parsed or copied) before the next buffer can be accessed.

The buffer containing the end of the HttpResponse can be partially consumed by specifying
the number of bytes that belong to the HttpResponse.  Any remaining bytes will be treated as payload.

This API is intended to be called after the application receives a B_AspInputSettings.httpResponseDataReady callback.

In the case that a partial HTTP request is returned, the caller should wait for the next httpResponseDataReady callback,
then call this API again to receive additional data.

Note: This API can only be called when B_AspInputStatus.state is _eConnected.
**/
NEXUS_Error B_AspInput_GetHttpResponseData(
    B_AspInputHandle                    hAspInput,
    const void                          **pBuffer,      /* [out] pointer to buffer containing data read from network. */
    unsigned                            *pByteCount     /* [out] number of bytes available in the data buffer pBuffer. */
    );


NEXUS_Error B_AspInput_HttpResponseDataConsumed(
    B_AspInputHandle                    hAspInput,
    bool                                responseCompleted,    /* [in] false => Entire buffer is consumed. End of HttpResponse not found, more data is required. */
                                                              /*      true  => End of the HttpResponse has been found and consumed. No more data is required. */

    unsigned                            bytesConsumed         /* [in] If responseComplete is true this is the number of bytes consumed from the current buffer.*/
    );                                                        /*       Else bytesConsumed must be equal to byte count returned by NEXUS_AspInput_GetHttpResponseData. */


/**
Summary:
Get Default StartSettings.
**/
void B_AspInput_GetDefaultStartSettings(
    B_AspInputStartSettings                 *pSettings      /* [out] */
    );


/**
Summary:
Start an AspInput.

AspInput will start streaming in after this API returns.

Note: This API can only be called when B_AspInputStatus.state is _eConnected.
AspInput changes its state to _eStreaming when it returns from this API. This can be
checked via the B_AspInputStatus.state variable obtained from _AspInput_GetStatus().

**/
NEXUS_Error B_AspInput_Start(
    B_AspInputHandle                        hAspInput,
    const B_AspInputStartSettings           *pSettings
    );


/**
Summary:
API to Get Current Status.

**/
NEXUS_Error B_AspInput_GetStatus(
    B_AspInputHandle                        hAspInput,
    B_AspInputStatus                        *pStatus     /* [out] */
    );


/**
Summary:
API to provide AV buffer to the caller after ASP receives it from the remote peer.

This is an optional API that is NOT needed in the default feed mode:
    B_AspInputStartSettings.feedMode == B_AspInputFeedMode_eAuto.

When B_AspInputStartSettings.feedMode == B_AspInputFeedMode_eHost

    This API returns a buffer filled with the received AV data and expects the caller
    to consume this data. After that, the caller must use NEXUS_AspInput_BufferComplete()
    to indicate the number of bytes consumed.

When B_AspInputStartSettings.feedMode == B_AspInputFeedMode_eAutoWithHostDecryption

    This API returns a buffer filled with the received AV data.
    It allows the application to decrypt the data (in place).
    After the caller decrypts the buffer, it must call NEXUS_AspInput_BufferComplete()
    to return the buffer back to ASP Input which will then feed it to the XPT Playback Channel.

Note: This API can only be called when B_AspInputStatus.state is _eStreaming.

**/
NEXUS_Error B_AspInput_GetBuffer(
    B_AspInputHandle                        hAspInput,
    void                                    **pBuffer,   /* [out] attr{memory=cached} pointer to data buffer which has been received from the network. */
    unsigned                                *pByteCount  /* [out] number of bytes available in the pBuffer. */
    );

/**
Summary:
API to provide a buffer back to AspInput.

This allows app to notify AspInput that app is done with the all or some of the buffer
(indicated via the byteCount parameter).
This buffer was previously obtained using the B_AspInput_GetBuffer() API.

Note: This API can only be called when B_AspInputStatus.state is _eStreaming.

**/
NEXUS_Error B_AspInput_BufferComplete(
    B_AspInputHandle                        hAspInput,
    unsigned                                byteCount   /* number of bytes processed. */
    );


/**
Summary:
Stop an AspInput.

Description:
This API stops the data reception by the AspInput.
It does NOT synchronize the protocol state with the network peer.

For normal stopping, caller should use B_AspInput_Finish().

**/
void B_AspInput_Stop(
    B_AspInputHandle                        hAspInput
    );


/**
Summary:
API to Print the Current Status to the debug log.
**/
void B_AspInput_PrintStatus(
    B_AspInputHandle                        hAspInput
    );

/**
Summary:
API to disconnect from AspInput.

This call transfers the ownership of connection from ASP to Host.
It recreates the connection state back in the Linux socket that was
passed in the B_AspInput_Connect*().

After this API returns, caller must close this socket using "close()".
TODO: clarify if this socket can be used for further sending or receiving.

**/
NEXUS_Error B_AspInput_Disconnect(
    B_AspInputHandle                        hAspInput,
    B_AspInputDisconnectStatus              *pStatus
    );

/**
Summary:
Destroy an AspInput object.

Description:
Stops and Destroys an AspInput. The handle can no longer be used.

**/
void B_AspInput_Destroy(
    B_AspInputHandle                       hAspInput
    );

#endif /* __B_ASP_INPUT_H__*/
