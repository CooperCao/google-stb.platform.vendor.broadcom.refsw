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

#include "bip_priv.h"
#include "liveMedia/bip_rtsp_lm_server.h"
#include "bip_rtsp_lm_server.h"
#include "bip_rtsp_socket.h"

BDBG_MODULE( bip_rtsp_socket );
BDBG_OBJECT_ID( BIP_RtspLiveMediaSocket );
typedef struct BIP_RtspLiveMediaSocket
{
    BDBG_OBJECT( BIP_RtspLiveMediaSocket )
    BIP_RtspServer *lmRtspServer;
    BIP_RtspServer::BIP_RtspClientConnection *lmRtspClientConnection;
} BIP_RtspLiveMediaSocket;

BIP_RtspLiveMediaSocketHandle
BIP_RtspLiveMediaSocket_CreateFromFd(
    unsigned                               socketFd,
    void                                  *lmRtspServerPtr,
    BIP_RtspLiveMediaSocketCreateSettings *pSettings
    )
{
    BSTD_UNUSED( pSettings );
    BIP_RtspLiveMediaSocketHandle rtspLmSocket = NULL;
    BIP_RtspServer               *lmRtspServer = (BIP_RtspServer *)lmRtspServerPtr;

    rtspLmSocket = (BIP_RtspLiveMediaSocketHandle) BKNI_Malloc( sizeof(*rtspLmSocket));
    BIP_CHECK_PTR_GOTO( rtspLmSocket, "Memory Allocation Failed", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );
    BKNI_Memset(rtspLmSocket, 0,  sizeof(*rtspLmSocket));
    BDBG_OBJECT_SET( rtspLmSocket, BIP_RtspLiveMediaSocket );

    /* create a new RTCPClientConnection object */
    rtspLmSocket->lmRtspClientConnection = lmRtspServer->createNewClientConnectionFull( socketFd );
    rtspLmSocket->lmRtspServer           = (BIP_RtspServer *)lmRtspServerPtr;

    return(rtspLmSocket);

error:
    if (rtspLmSocket)
    {
        BKNI_Free( rtspLmSocket );
    }
    return(NULL);
} // BIP_RtspLiveMediaSocket_CreateFromFd

/**
 * Summary:
 * Destroy socket
 *
 * Description:
 **/
void
BIP_RtspLiveMediaSocket_Destroy(
    BIP_RtspLiveMediaSocketHandle rtspLmSocket
    )
{
    if (rtspLmSocket)
    {
        BKNI_Free( rtspLmSocket );
    }
}

void
BIP_RtspLiveMediaSocket_GetSettings(
    BIP_RtspLiveMediaSocketHandle    socket,
    BIP_RtspLiveMediaSocketSettings *pSettings
    )
{
    BSTD_UNUSED( socket );
    BSTD_UNUSED( pSettings );
}

BIP_Status
BIP_RtspLiveMediaSocket_SetSettings(
    BIP_RtspLiveMediaSocketHandle    rtspLmSocket,
    BIP_RtspLiveMediaSocketSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT( rtspLmSocket, BIP_RtspLiveMediaSocket );
    BDBG_ASSERT( pSettings );

    /* just pass thru the settings to the actual C++ class: BIP_RtspServer */

    /* set the callback with the liveMedia object */
    rtspLmSocket->lmRtspClientConnection->setMessageReceivedCallback( pSettings->messageReceivedCallback );
    rtspLmSocket->lmRtspClientConnection->setErrorCallback( pSettings->errorCallback );
    return(BIP_SUCCESS);
}

/**
 * Summary:
 * Copy current RTSP Message
 *
 * Description:
 *
 * BIP_RtspSocket class invokes this API to copy the currently received RTSP Message into its buffer so that it can
 * queue up this message request for app's consumption.
 *
 **/
void
BIP_RtspLiveMediaSocket_CopyMessage(
    BIP_RtspLiveMediaSocketHandle rtspLmSocket,
    char                         *pBuffer
    )
{
    BDBG_MSG(("%s: calling rtspLmSocket->lmRtspClientConnection->copyReceivedMessage(%s)", __FUNCTION__, pBuffer ));
    rtspLmSocket->lmRtspClientConnection->copyReceivedMessageConnection( pBuffer );
}

#if 0
/**
 * Summary:
 * Rtsp Socket API to recv a RTSP Message on RTSP Socket
 **/
BIP_Status BIP_RtspLiveMediaSocket_RecvRequest( BIP_RtspLiveMediaSocketHandle socket, BIP_RtspRequestHandle rtspRequest );
BIP_Status BIP_RtspLiveMediaSocket_RecvResponse( BIP_RtspLiveMediaSocketHandle socket, BIP_RtspResponseHandle rtspResponse );

/**
 * Summary:
 * Rtsp Socket API to send a RTSP Message on RTSP Socket
 **/
BIP_Status BIP_RtspLiveMediaSocket_SendRequest( BIP_RtspLiveMediaSocketHandle socket, BIP_RtspRequestHandle rtspRequest );
BIP_Status BIP_RtspLiveMediaSocket_SendResponse( BIP_RtspLiveMediaSocketHandle socket, BIP_RtspResponseHandle rtspResponse );
#endif // if 0

#include "RTSPCommon.hh"
void
BIP_RtspLiveMediaRequest_Parse(
    char    *pBuffer,
    unsigned bufferLength
    )
{
    char     cmdName[RTSP_PARAM_STRING_MAX];
    char     urlPreSuffix[RTSP_PARAM_STRING_MAX];
    char     urlSuffix[RTSP_PARAM_STRING_MAX];
    char     cseq[RTSP_PARAM_STRING_MAX];
    char     sessionIdStr[RTSP_PARAM_STRING_MAX];
    unsigned contentLength = 0;

    pBuffer[bufferLength-2] = '\0'; // temporarily, for parsing
    Boolean parseSucceeded = parseRTSPRequestString( pBuffer, bufferLength,
            cmdName, sizeof(cmdName),
            urlPreSuffix, sizeof(urlPreSuffix),
            urlSuffix, sizeof(urlSuffix),
            cseq, sizeof(cseq),
            sessionIdStr, sizeof(sessionIdStr),
            contentLength );
    pBuffer[bufferLength-2] = '\r'; // restore its value

    BDBG_MSG(("%s: len %d, parseStatus %d, cmd %s, url: pre %s, suf %s, cseq %s, id %s",
              __FUNCTION__, bufferLength, parseSucceeded, cmdName,
              urlPreSuffix, urlSuffix, cseq, sessionIdStr));
} // BIP_RtspLiveMediaRequest_Parse

BIP_RtspLiveMediaSessionHandle
BIP_RtspLiveMediaSocket_CreateSession(
    BIP_RtspLiveMediaSocketHandle rtspLmSocket,
    char                         *requestStr
    )
{
    BIP_RtspLiveMediaSessionHandle rtspLmSession;

    BDBG_OBJECT_ASSERT( rtspLmSocket, BIP_RtspLiveMediaSocket );

    rtspLmSession = BIP_RtspLiveMediaSession_CreateFromRequest( requestStr, rtspLmSocket->lmRtspClientConnection, NULL /*pSettings*/ );
    BIP_CHECK_PTR_GOTO( rtspLmSession, "Memory Allocation Failed", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );

    return(rtspLmSession);

error:
    return(NULL);
}
