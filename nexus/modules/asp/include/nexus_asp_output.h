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
#ifndef NEXUS_ASP_OUTPUT_H
#define NEXUS_ASP_OUTPUT_H

#include "nexus_asp_output_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
private API
***************************************************************************/

/**
NEXUS AspOutput is a high level abstraction of the streaming out portion of the ASP HW Block.

Advanced Stream Processor (ASP) is a H/W block that can read AV data from Rave Context,
packetize it using TCP/IP protocols (HTTP/UDP/RTP), and directly send it to network peers.
It provides complete offloading of streaming portion of a AV session.
In addition, it can also optionally encrypt outgoing AV payloads before streaming them out.

Normal stream output flows like this:
    NEXUS_Recpump (RAVE Ctx) ->  NEXUS_AspOutput (ASP Ch) -> Network.
**/

/**
Summary:
API to create an AspOutput.

Note: AspOutput sets its initial state to _Idle when it returns from this API. This can be
checked via the NEXUS_AspOutputStatus.state variable obtained from _AspOutput_GetStatus().

**/
void NEXUS_AspOutput_GetDefaultCreateSettings(
    NEXUS_AspOutputCreateSettings       *pSettings  /* [out] */
    );

NEXUS_AspOutputHandle NEXUS_AspOutput_Create(       /* attr{destructor=NEXUS_AspOutput_Destroy} */
    const NEXUS_AspOutputCreateSettings *pSettings  /* attr{null_allowed=y} */
    );

/**
Summary:
API to Get Current Settings.
**/
void NEXUS_AspOutput_GetSettings(
    NEXUS_AspOutputHandle              hAspOutput,
    NEXUS_AspOutputSettings            *pSettings   /* [out] */
    );


/**
Summary:
API to Update Current Settings.
**/
NEXUS_Error NEXUS_AspOutput_SetSettings(
    NEXUS_AspOutputHandle              hAspOutput,
    const NEXUS_AspOutputSettings      *pSettings
    );


/**
Summary:
API to Get Current DTCP-IP Settings
**/
void NEXUS_AspOutput_GetDtcpIpSettings(
    NEXUS_AspOutputHandle              hAspOutput,
    NEXUS_AspOutputDtcpIpSettings      *pSettings /* [out] */
    );

/**
Summary:
API to Set Current DTCP-IP Settings.  This must be called prior to NEXUS_AspOutput_Start().
**/
NEXUS_Error NEXUS_AspOutput_SetDtcpIpSettings(
    NEXUS_AspOutputHandle               hAspOutput,
    const NEXUS_AspOutputDtcpIpSettings *pSettings
    );



/**
Summary:
API to Connect to AspOutput.

This API is used during the process of migrating the connection from Host to ASP.
It allows app to pass network socket state that caller has previously obtained from Linux.
After this API, ASP becomes responsible for all network activity for that connection.

Note: AspOutput changes its state to _Connected when it returns from this API. This can be
checked via the NEXUS_AspOutputStatus.state variable obtained from _AspOutput_GetStatus().

**/
void NEXUS_AspOutput_GetDefaultTcpSettings(
    NEXUS_AspTcpSettings                    *pSettings  /* [out] */
    );

void NEXUS_AspOutput_GetDefaultConnectHttpSettings(
    NEXUS_AspOutputConnectHttpSettings       *pSettings  /* [out] */
    );

NEXUS_Error NEXUS_AspOutput_ConnectHttp(
    NEXUS_AspOutputHandle                       hAspOutput,
    const NEXUS_AspTcpSettings                  *pTcpSettings,
    const NEXUS_AspOutputConnectHttpSettings    *pSettings
    );


/**
Summary:
API to receive the incoming HTTP Request (from remote) for host access.

The incoming HttpRequest is accessed one buffer at a time with the following APIs.
If the HttpRequest spans more than one buffer, each buffer must be completely consumed
(either parsed or copied) before the next buffer can be accessed.

The buffer containing the end of the HttpRequest can be partially consumed by specifying
the number of bytes that belong to the HttpRequest.  Any remaining bytes will be treated as payload.
**/
NEXUS_Error NEXUS_AspOutput_GetHttpRequestData(
    NEXUS_AspOutputHandle               hAspOutput,
    const void                          **pBuffer,      /* [out] attr{memory=cached} pointer to buffer containing data read from network. */
    unsigned                            *pByteCount     /* [out] number of bytes available in the data buffer pBuffer. */
    );


NEXUS_Error NEXUS_AspOutput_HttpRequestDataConsumed(
    NEXUS_AspOutputHandle               hAspOutput,
    bool                                requestCompleted,     /* [in] false => Entire buffer is consumed. End of HttpRequest not found, more data is required. */
                                                              /*      true  => End of the HttpRequest has been found and consumed. No more data is required. */

    unsigned                            bytesConsumed         /* [in] If requestCompleted is true this is the number of bytes consumed from the current buffer.*/
    );                                                        /*      Else bytesConsumed must be equal to byte count returned by NEXUS_AspOutput_GetHttpRequestData. */


/**
Summary:
API to provide an HTTP response to be sent out on the network.

Note: This API can only be called when NEXUS_AspOutputStatus.state is _eConnected.

**/
NEXUS_Error NEXUS_AspOutput_SendHttpResponse(
    NEXUS_AspOutputHandle               hAspOutput,
    const void                          *pBuffer,       /* [in] attr{nelem=byteCount} pointer to HTTP response to be sent to network. */
    unsigned                            byteCount       /* [in] number of bytes in HTTP Response buffer. */
    );


/**
Summary:
Get Default StartSettings.
**/
void NEXUS_AspOutput_GetDefaultStartSettings(
    NEXUS_AspOutputStartSettings       *pSettings   /* [out] */
    );


/**
Summary:
Start an AspOutput.

AspOutput will start streaming out after this API returns.

Note: AspOutput changes its state to _eStreaming when it returns from this API. This can be
checked via the NEXUS_AspOutputStatus.state variable obtained from _AspOutput_GetStatus().

**/
NEXUS_Error NEXUS_AspOutput_Start(
    NEXUS_AspOutputHandle              hAspOutput,
    const NEXUS_AspOutputStartSettings *pSettings
    );


/**
Summary:
API to Get Current Status.
**/
NEXUS_Error NEXUS_AspOutput_GetStatus(
    NEXUS_AspOutputHandle              hAspOutput,
    NEXUS_AspOutputStatus              *pStatus     /* [out] */
    );

NEXUS_Error NEXUS_AspOutput_GetHttpStatus(
    NEXUS_AspOutputHandle              hAspOutput,
    NEXUS_AspOutputHttpStatus          *pStatus     /* [out] */
    );


/**
Summary:
API to provide AV buffer to the caller before ASP sends it out.

This is an optional API that is NOT needed in the default feed mode:
    NEXUS_AspOutputStartSettings.feedMode == NEXUS_AspOutputFeedMode_eAuto.

When NEXUS_AspOutputStartSettings.feedMode == NEXUS_AspOutputFeedMode_eHost

    This API returns a buffer available to be filled with data to be output.
    After the caller fills the buffer, it must call NEXUS_AspOutput_BufferSubmit()
    to submit the buffer to ASP which will then send it out to the network.

When NEXUS_AspOutputStartSettings.feedMode == NEXUS_AspOutputFeedMode_eAutoWithHostEncrytion

    This API returns a buffer that contains current data from Recpump before it has
    been sent to the network.  It allows the application to encrypt the data (in place).
    After the caller encrypts the buffer, it must call NEXUS_AspOutput_BufferSubmit()
    to return the buffer back to ASP which will then send it out to the network.

Note: This API can only be called when NEXUS_AspOutputStatus.state is _eStreaming.

**/
NEXUS_Error NEXUS_AspOutput_GetBufferWithWrap(
    NEXUS_AspOutputHandle               hAspOutput,
    void                                **pBuffer,   /* [out] attr{memory=cached} pointer to data buffer which can be written to network. */
    unsigned                            *pByteCount, /* [out] size of the available space in the pBuffer before the wrap. */
    void                                **pBuffer2,  /* [out] attr{null_allowed=y;memory=cached} optional pointer to data after wrap. */
    unsigned                            *pByteCount2 /* [out] size of the available space in the pBuffer2 that can be written to network. */
    );

/**
Summary:
API to submit a buffer back to AspOutput.

This allows app to notify AspOutput that app is done with the all or some of the buffer
(indicated via the byteCount parameter).
This buffer was previously obtained using the NEXUS_AspOutput_GetBufferWithWrap() API.

Note: This API can only be called when NEXUS_AspOutputStatus.state is _eStreaming.

**/
NEXUS_Error NEXUS_AspOutput_BufferSubmit(
    NEXUS_AspOutputHandle               hAspOutput,
    unsigned                            byteCount   /* number of bytes that AspOutput can send to the network. */
    );


/**
Summary:
Stop an AspOutput.

Description:
This API stops the data transmission by the AspOutput.
It does NOT synchronize the protocol state with the network peer.

Note: AspOutput changes its state to _Connected when it returns from this API. This can be
checked via the NEXUS_AspOutputStatus.state variable obtained from _AspOutput_GetStatus().

For normal stopping, caller should use NEXUS_AspOutput_Finish().

**/
void NEXUS_AspOutput_Stop(
    NEXUS_AspOutputHandle              hAspOutput
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
checked via the NEXUS_AspOutputStatus.state variable obtained from _AspOutput_GetStatus().

For immediate (un-clean) stopping, app should use NEXUS_AspOutput_Stop().
**/
void NEXUS_AspOutput_Finish(
    NEXUS_AspOutputHandle              hAspOutput
    );


/**
Summary:
API to disconnect from an AspOutput.

This call transfers the ownership of connection from ASP to Host and returns this
connection state back in the NEXUS_AspOutputDisconnectStatus.

Note: AspOutput changes its state to _eIdle when it returns from this API. This can be
checked via the NEXUS_AspOutputStatus.state variable obtained from _AspOutput_GetStatus().

**/
NEXUS_Error NEXUS_AspOutput_Disconnect(
    NEXUS_AspOutputHandle                       hAspOutput,
    NEXUS_AspOutputDisconnectStatus             *pStatus
    );

/**
Summary:
Stops and Destroys an AspOutput. The handle can no longer be used.
**/
void NEXUS_AspOutput_Destroy(
    NEXUS_AspOutputHandle hAspOutput
    );


#ifdef __cplusplus
}
#endif

#endif
