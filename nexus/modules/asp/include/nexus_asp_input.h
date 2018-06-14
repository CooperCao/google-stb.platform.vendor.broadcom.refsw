/***************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 **************************************************************************/
#ifndef NEXUS_ASP_INPUT_H
#define NEXUS_ASP_INPUT_H

#include "nexus_asp_input_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
private API
***************************************************************************/

/**
NEXUS AspInput is a high level abstraction of the streaming in portion of the ASP HW Block.

Advanced Stream Processor (ASP) is a H/W block that can recieve AV data from a network peer,
de-packetize it according to TCP/IP protocols (HTTP/UDP/RTP), and makes it available for further
processing.  The AV data can be consumed manually by host software, or by feeding to the transport
hardware.  It provides complete offloading of streaming portion of a AV session.
In addition, it can also optionally decrypt incoming AV payloads.

Normal stream input flows like this:
    Network -> NEXUS_AspInput -> NEXUS_Playpump
**/

/**
Summary:
API to create an AspInput.

Note: AspInput sets its initial state to _Idle when it returns from this API. This can be
checked via the NEXUS_AspInputStatus.state variable obtained from _AspInput_GetStatus().

**/
void NEXUS_AspInput_GetDefaultCreateSettings(
    NEXUS_AspInputCreateSettings       *pSettings  /* [out] */
    );

NEXUS_AspInputHandle NEXUS_AspInput_Create(       /* attr{destructor=NEXUS_AspInput_Destroy} */
    const NEXUS_AspInputCreateSettings *pSettings  /* attr{null_allowed=y} */
    );

/**
Summary:
API to Get Current Settings.
**/
void NEXUS_AspInput_GetSettings(
    NEXUS_AspInputHandle              hAspInput,
    NEXUS_AspInputSettings            *pSettings   /* [out] */
    );


/**
Summary:
API to Update Current Settings.
**/
NEXUS_Error NEXUS_AspInput_SetSettings(
    NEXUS_AspInputHandle              hAspInput,
    const NEXUS_AspInputSettings      *pSettings
    );


/**
Summary:
API to Get Current DTCP-IP Settings
**/
void NEXUS_AspInput_GetDtcpIpSettings(
    NEXUS_AspInputHandle              hAspInput,
    NEXUS_AspInputDtcpIpSettings      *pSettings /* [out] */
    );

/**
Summary:
API to Set Current DTCP-IP Settings.  This must be called prior to NEXUS_AspInput_Start().
**/
NEXUS_Error NEXUS_AspInput_SetDtcpIpSettings(
    NEXUS_AspInputHandle               hAspInput,
    const NEXUS_AspInputDtcpIpSettings *pSettings
    );



/**
Summary:
API to Connect to AspInput.

This API is used during the process of migrating the connection from Host to ASP.
It allows app to pass network socket state that caller has previously obtained from Linux.
After this API, ASP becomes responsible for all network activity for that connection.

Note: AspInput changes its state to _Connected when it returns from this API. This can be
checked via the NEXUS_AspInputStatus.state variable obtained from _AspInput_GetStatus().

**/

void NEXUS_AspInput_GetDefaultTcpSettings(
    NEXUS_AspTcpSettings                    *pSettings  /* [out] */
    );

void NEXUS_AspInput_GetDefaultConnectHttpSettings(
    NEXUS_AspInputConnectHttpSettings       *pSettings  /* [out] */
    );

NEXUS_Error NEXUS_AspInput_ConnectHttp(
    NEXUS_AspInputHandle                       hAspInput,
    const NEXUS_AspTcpSettings                 *pTcpSettings,
    const NEXUS_AspInputConnectHttpSettings    *pSettings
    );

/**
Summary:
API to provide an HTTP request to be sent out on the network.

Note: This API can only be called when NEXUS_AspInputStatus.state is _eConnected.

**/
NEXUS_Error NEXUS_AspInput_SendHttpRequest(
    NEXUS_AspInputHandle                hAspInput,
    const void                          *pBuffer,       /* [in] attr{nelem=byteCount} pointer to HTTP request to be sent to network. */
    unsigned                            byteCount       /* [in] number of bytes in HTTP Request buffer. */
    );


/**
Summary:
API to receive the incoming HTTP Response (from remote) for host access.

The incoming HttpResponse is accessed one buffer at a time with the following APIs.
If the HttpResponse spans more than one buffer, each buffer must be completely consumed
(either parsed or copied) before the next buffer can be accessed.

The buffer containing the end of the HttpResponse can be partially consumed by specifying
the number of bytes that belong to the HttpResponse.  Any remaining bytes will be treated as payload.
**/
NEXUS_Error NEXUS_AspInput_GetHttpResponseData(
    NEXUS_AspInputHandle                hAspInput,
    const void                          **pBuffer,      /* [out] attr{memory=cached} pointer to buffer containing data read from network. */
    unsigned                            *pByteCount     /* [out] number of bytes available in the data buffer pBuffer. */
    );


NEXUS_Error NEXUS_AspInput_HttpResponseDataConsumed(
    NEXUS_AspInputHandle                hAspInput,
    bool                                responseCompleted,    /* [in] false => Entire buffer is consumed. End of HttpResponse not found, more data is required. */
                                                              /*      true  => End of the HttpResponse has been found and consumed. No more data is required. */

    unsigned                            bytesConsumed         /* [in] If responseCompleted is true this is the number of bytes consumed from the current buffer.*/
    );                                                        /*       Else bytesConsumed must be equal to byte count returned by NEXUS_AspInput_GetHttpResponseData. */


/**
Summary:
Get Default StartSettings.
**/
void NEXUS_AspInput_GetDefaultStartSettings(
    NEXUS_AspInputStartSettings       *pSettings   /* [out] */
    );


/**
Summary:
Start an AspInput.

AspInput will start receiving AV data from network after this API returns.

Note: AspInput changes its state to _eStreaming when it returns from this API. This can be
checked via the NEXUS_AspInputStatus.state variable obtained from _AspInput_GetStatus().

**/
NEXUS_Error NEXUS_AspInput_Start(
    NEXUS_AspInputHandle              hAspInput,
    const NEXUS_AspInputStartSettings *pSettings
    );


/**
Summary:
API to Get Current Status.
**/
NEXUS_Error NEXUS_AspInput_GetStatus(
    NEXUS_AspInputHandle              hAspInput,
    NEXUS_AspInputStatus              *pStatus     /* [out] */
    );

NEXUS_Error NEXUS_AspInput_GetHttpStatus(
    NEXUS_AspInputHandle              hAspInput,
    NEXUS_AspInputHttpStatus          *pStatus     /* [out] */
    );


/**
Summary:
API to provide AV buffer to the caller after ASP receives it from the remote peer.

This is an optional API that is NOT needed in the default feed mode:
    NEXUS_AspInputStartSettings.feedMode == NEXUS_AspInputFeedMode_eAuto.

When NEXUS_AspInputStartSettings.feedMode == NEXUS_AspInputFeedMode_eHost

    This API returns a buffer filled with the received AV data and expects the caller
    to consume this data. After that, the caller must use NEXUS_AspInput_BufferComplete()
    to indicate the number of bytes consumed.

When NEXUS_AspInputStartSettings.feedMode == NEXUS_AspInputFeedMode_eAutoWithHostDecryption

    This API returns a buffer filled with the received AV data.
    It allows the application to decrypt the data (in place).
    After the caller decrypts the buffer, it must call NEXUS_AspInput_BufferComplete()
    to return the buffer back to ASP Input which will then feed it to the XPT Playback Channel.

Note: This API can only be called when NEXUS_AspInputStatus.state is _eStreaming.

**/
NEXUS_Error NEXUS_AspInput_GetBuffer(
    NEXUS_AspInputHandle                hAspInput,
    void                                **pBuffer,   /* [out] attr{memory=cached} pointer to data buffer which has been received from the network. */
    unsigned                            *pByteCount  /* [out] number of bytes available in the pBuffer. */
    );

/**
Summary:
API to provide a buffer back to AspInput.

This allows app to notify AspInput that it is done with the all or some of the buffer
(indicated via the byteCount parameter).
This buffer was previously obtained using the NEXUS_AspInput_GetBuffer() API.

Note: This API can only be called when NEXUS_AspInputStatus.state is _eStreaming.

**/
NEXUS_Error NEXUS_AspInput_BufferComplete(
    NEXUS_AspInputHandle                hAspInput,
    unsigned                            byteCount   /* number of bytes processed. */
    );


/**
Summary:
Stop an AspInput.

Description:
This API stops the data reception by the AspInput.
It does NOT synchronize the protocol state with the network peer.

Note: AspInput changes its state to _Connected when it returns from this API. This can be
checked via the NEXUS_AspInputStatus.state variable obtained from _AspInput_GetStatus().

**/
void NEXUS_AspInput_Stop(
    NEXUS_AspInputHandle              hAspInput
    );


/**
Summary:
API to disconnect from an AspInput.

This call transfers the ownership of connection from ASP to Host and returns this
connection state back in the NEXUS_AspInputDisconnectStatus.

Note: AspInput changes its state to _eIdle when it returns from this API. This can be
checked via the NEXUS_AspInputStatus.state variable obtained from _AspInput_GetStatus().

**/
NEXUS_Error NEXUS_AspInput_Disconnect(
    NEXUS_AspInputHandle                       hAspInput,
    NEXUS_AspInputDisconnectStatus             *pStatus
    );

/**
Summary:
Stops and Destroys an AspInput. The handle can no longer be used.
**/
void NEXUS_AspInput_Destroy(
    NEXUS_AspInputHandle hAspInput
    );


#ifdef __cplusplus
}
#endif

#endif
