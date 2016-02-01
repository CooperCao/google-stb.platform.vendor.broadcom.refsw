/******************************************************************************
 * (c) 2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "bip_types.h"
#include "bip_priv.h"
#include "bip_rtsp_lm_server.h"
#include "bip_rtsp_server.h"
#include "bip_rtsp_socket.h"
#include "bip_rtsp_socket_impl.h"

BDBG_MODULE( bip_rtsp_listener );
BDBG_OBJECT_ID( BIP_RtspListener );

typedef struct BIP_RtspListener
{
    BDBG_OBJECT( BIP_RtspListener )
    BIP_RtspListenerCreateSettings createSettings;
    BIP_RtspListenerSettings        settings;
    BIP_RtspLiveMediaListenerHandle hRtspLmListener;
    BLST_Q_HEAD( rtspSocketListHead, BIP_RtspSocket ) rtspSocketListHead; /* list of active RtspSocket objects */
    BKNI_MutexHandle lock;
} BIP_RtspListener;

void BIP_RtspListener_GetDefaultCreateSettings(
    BIP_RtspListenerCreateSettings *pSettings
    )
{
    BKNI_Memset( pSettings, 0, sizeof( BIP_RtspListenerCreateSettings ));
    pSettings->port               = 554;
    pSettings->rtspSessionTimeout = 60;
}

void cleanupRtspSocketList(
    BIP_RtspListenerHandle hRtspListener
    )
{
    BIP_RtspSocketHandle hRtspSocket;

    if (!hRtspListener) {return; }
    if (!hRtspListener->lock) {return; }

    BKNI_AcquireMutex( hRtspListener->lock );
    for (hRtspSocket = BLST_Q_FIRST( &hRtspListener->rtspSocketListHead );
         hRtspSocket;
         hRtspSocket = BLST_Q_FIRST( &hRtspListener->rtspSocketListHead ))
    {
        BLST_Q_REMOVE( &hRtspListener->rtspSocketListHead, hRtspSocket, rtspSocketListNext );
        BIP_RtspSocket_Destroy( hRtspSocket );
    }
    BKNI_ReleaseMutex( hRtspListener->lock );
} /* cleanupRtspSocketList */

static void rtspListenerDestroy(
    BIP_RtspListenerHandle hRtspListener
    )
{
    if (!hRtspListener) {return; }

    /* freeup any rtspSockets that are still active */
    cleanupRtspSocketList( hRtspListener );

    if (hRtspListener->hRtspLmListener) {BIP_RtspLiveMediaListener_Destroy( hRtspListener->hRtspLmListener ); }

    if (hRtspListener->lock) {BKNI_DestroyMutex( hRtspListener->lock ); }

    BDBG_OBJECT_DESTROY( hRtspListener, BIP_RtspListener );

    /* Then finally free BIP_RtspListener struct itself */
    BKNI_Free( hRtspListener );
}

BIP_RtspListenerHandle BIP_RtspListener_Create(
    BIP_RtspListenerCreateSettings *pSettings
    )
{
    int                    rc;
    BIP_RtspListenerHandle hRtspListener = NULL;

    hRtspListener = BKNI_Malloc( sizeof( *hRtspListener ));
    BIP_CHECK_PTR_GOTO( hRtspListener, "Memory Allocation Failed", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );

    BKNI_Memset( hRtspListener, 0, sizeof( *hRtspListener ));
    BDBG_OBJECT_SET( hRtspListener, BIP_RtspListener );

    /* Handle CreateSettings. */
    if (NULL == pSettings)
    {
        /* If CreateSettings not passed, use default CreateSettings. */
        BIP_RtspListener_GetDefaultCreateSettings( &hRtspListener->createSettings );
    }
    else
    {
        /* If caller passed CreateSettings, use those. */
        BKNI_Memcpy( &hRtspListener->createSettings, pSettings, sizeof( hRtspListener->createSettings ));
    }
    pSettings = &hRtspListener->createSettings;  /* Now we can use our own copy of the CreateSettings. */

    /* TODO: */
    /* BIP_Init or some other BIP initialization routine should create and initialize a queue where of callback descriptors */
    /* this queue is common for both client & server interfaces */

    /* create liveMedia Rtsp listener */
    {
        BIP_RtspLiveMediaListenerCreateSettings lmListenerCreateSettings;

        BIP_RtspLiveMediaListener_GetDefaultCreateSettings( &lmListenerCreateSettings );
        lmListenerCreateSettings.port               = pSettings->port;
        lmListenerCreateSettings.rtspSessionTimeout = pSettings->rtspSessionTimeout;
        lmListenerCreateSettings.hIgmpListener      = pSettings->hIgmpListener;
        hRtspListener->hRtspLmListener              = BIP_RtspLiveMediaListener_Create( &lmListenerCreateSettings );
        BIP_CHECK_PTR_GOTO( hRtspListener->hRtspLmListener, "Memory Allocation Failed", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );
    }

    rc = BKNI_CreateMutex( &hRtspListener->lock );
    BIP_CHECK_ERR_NZ_GOTO( rc, "BKNI_CreateMutex() Failed", error );

    BDBG_MSG(( "%s: hRtspListener %p", __FUNCTION__, hRtspListener ));
    return( hRtspListener );

error:
    rtspListenerDestroy( hRtspListener );
    return( NULL );
} /* BIP_RtspListener_Create */

void BIP_RtspListener_Destroy(
    BIP_RtspListenerHandle hRtspListener
    )
{
    BDBG_OBJECT_ASSERT( hRtspListener, BIP_RtspListener );

    rtspListenerDestroy( hRtspListener );
} /* BIP_RtspListener_Destroy */

void BIP_RtspListener_GetSettings(
    BIP_RtspListenerHandle    hRtspListener,
    BIP_RtspListenerSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT( hRtspListener, BIP_RtspListener );
    BDBG_ASSERT( pSettings );
    *pSettings = hRtspListener->settings;
}

static void hRtspListenerMessageReceivedCallback(
    void *ctx,
    int   messageLength
    )
{
    BIP_RtspListenerHandle hRtspListener = (BIP_RtspListenerHandle)ctx;

    BSTD_UNUSED( messageLength );

    /* indicate to callback thread to invoke the messageReceived callback */
    /* TODO: for now, we are directly invoking the callback */
    BDBG_MSG(( "%s: checking callback (%p)", __FUNCTION__, hRtspListener->settings.messageReceivedCallback.callback ));
    if (hRtspListener->settings.messageReceivedCallback.callback)
    {
        BDBG_MSG(( "%s: calling callback()", __FUNCTION__ ));
        hRtspListener->settings.messageReceivedCallback.callback( hRtspListener->settings.messageReceivedCallback.context, hRtspListener->settings.messageReceivedCallback.param );
    }
}

static void BIP_Listener_CreateSocket(
    void *ctx,
    int   socketFd
    )
{
    BIP_RtspListenerHandle hRtspListener = (BIP_RtspListenerHandle)ctx;
    int                    rc;
    BIP_RtspSocketHandle   hRtspSocket = NULL;
    BIP_RtspSocketSettings socketSettings;
    struct sockaddr_in     peerIpAddress;
    socklen_t              peerAddrLen = sizeof( peerIpAddress );

    /* Got a new connection request on the listener, get peer socket info */
    rc = getpeername( socketFd, (struct sockaddr *)&peerIpAddress, (socklen_t *)&peerAddrLen );
    BIP_CHECK_ERR_NZ_GOTO( rc, "getpeername() Failed", error );

    /* Create RtspSocket object to receive/send messages on */
    hRtspSocket = BIP_RtspSocket_CreateFromFd( socketFd, hRtspListener->hRtspLmListener, NULL /*pSettings*/ );
    BIP_CHECK_PTR_GOTO( hRtspSocket, "Memory Allocation Failed", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );

    /* setup callbacks to receive the next message & any socket errors */
    BIP_RtspSocket_GetSettings( hRtspSocket, &socketSettings );
    socketSettings.messageReceivedCallback.context  = hRtspListener;
    socketSettings.messageReceivedCallback.callback = hRtspListenerMessageReceivedCallback;
    rc = BIP_RtspSocket_SetSettings( hRtspSocket, &socketSettings );
    BIP_CHECK_ERR_NZ_GOTO( rc, "BIP_RtspSocket_SetSettings Failed", error );

    /* Queue this new hRtspSocket to the list of sockets on this listener */
    BKNI_AcquireMutex( hRtspListener->lock );
    BLST_Q_INSERT_TAIL( &hRtspListener->rtspSocketListHead, hRtspSocket, rtspSocketListNext );
    BKNI_ReleaseMutex( hRtspListener->lock );

    BDBG_MSG(( "%s: new connection on socketFd %d from peer %s, hRtspSocket %p", __FUNCTION__, socketFd, inet_ntoa( peerIpAddress.sin_addr ), hRtspSocket ));
    return;

error:
    if (hRtspSocket)
    {
        BIP_RtspSocket_Destroy( hRtspSocket );
    }
} /* BIP_Listener_CreateSocket*/

static void hRtspListenerConnectedCallback(
    void *ctx,
    int   socketFd
    )
{
    BIP_RtspListenerHandle hRtspListener = (BIP_RtspListenerHandle)ctx;

    BDBG_MSG(( "%s: calling BIP_Listener_CreateSocket()", __FUNCTION__ ));
    BIP_Listener_CreateSocket( hRtspListener, socketFd );
} /* hRtspListenerConnectedCallback */

static void hRtspListenerGetRtpStatisticsCallback(
    void *ctx,
    int  streamId
    )
{
    BIP_RtspListenerHandle hRtspListener = (BIP_RtspListenerHandle)ctx;

    BDBG_MSG(( "%s: streamId (%d); hRtspListener (%p)", __FUNCTION__, streamId, hRtspListener ));
    BIP_CHECK_PTR_GOTO( hRtspListener, "hRtspListener is null", error, BIP_ERR_INVALID_PARAMETER );

error:
    return;
} /* hRtspListenerGetRtpStatisticsCallback */

BIP_Status BIP_RtspListener_SetSettings(
    BIP_RtspListenerHandle    hRtspListener,
    BIP_RtspListenerSettings *pSettings
    )
{
    BIP_Status errCode;
    BIP_RtspLiveMediaListenerSettings lmListenerSettings;

    BDBG_OBJECT_ASSERT( hRtspListener, BIP_RtspListener );
    BDBG_ASSERT( pSettings );
    /* TODO validate pSettings fields */
    hRtspListener->settings = *pSettings;

    /* TODO: */
    /* Add pSettings->connectedCallback & others to the list of BIP_CallbackDesc callbacks */
    /* callback thread waits on an event to process this callbackDesc list */
    /* it then invokes callbacks on descriptors with callbackFire flag set */

    /* also set our callback with the rtsp lm listener */
    /* this is because it is the LiveMedia library that is processing the socket for any events and invoking this callback */
    BIP_RtspLiveMediaListener_GetSettings( hRtspListener->hRtspLmListener, &lmListenerSettings );
    lmListenerSettings.connectedCallback.callback = hRtspListenerConnectedCallback;
    lmListenerSettings.connectedCallback.context  = hRtspListener;
    lmListenerSettings.getRtpStatisticsCallback.callback = hRtspListenerGetRtpStatisticsCallback;
    lmListenerSettings.getRtpStatisticsCallback.context  = hRtspListener;
    errCode = BIP_RtspLiveMediaListener_SetSettings( hRtspListener->hRtspLmListener, &lmListenerSettings );
    BIP_CHECK_ERR_NZ_GOTO( errCode, "SetSettings Failed", error );

    errCode = BIP_SUCCESS;
    return( errCode );

error:
    return( errCode );
} /* BIP_RtspListener_SetSettings */

void BIP_RtspListener_Stop(
    BIP_RtspListenerHandle hRtspListener
    )
{
    BDBG_MSG(( "%s: hRtspListener %p", __FUNCTION__, hRtspListener ));
    BIP_RtspLiveMediaListener_Stop( hRtspListener->hRtspLmListener );
}

BIP_Status BIP_RtspListener_Start(
    BIP_RtspListenerHandle hRtspListener
    )
{
    BIP_Status errCode = BIP_SUCCESS;

    BDBG_OBJECT_ASSERT( hRtspListener, BIP_RtspListener );
    BDBG_MSG(( "%s: hRtspListener %p", __FUNCTION__, hRtspListener ));

    errCode = BIP_RtspLiveMediaListener_Start( hRtspListener->hRtspLmListener );
    BIP_CHECK_ERR_NZ_GOTO( errCode, "SetSettings Failed", error );

    errCode = BIP_SUCCESS;
    return( errCode );

error:
    return( errCode );
}

/**
 * Summary:
 * Rtsp Listener API to recv a RTSP Message on RTSP Listener
 **/
BIP_Status BIP_RtspListener_RecvRequest(
    BIP_RtspListenerHandle hRtspListener,
    BIP_RtspRequestHandle  hRtspRequest
    )
{
    BIP_Status            rc = BIP_ERR_NOT_AVAILABLE;
    BIP_RtspSocketHandle hRtspSocket;
    BIP_RtspSocketHandle hRtspSocketNext = NULL;

    BKNI_AcquireMutex( hRtspListener->lock );
    hRtspSocket = BLST_Q_FIRST( &hRtspListener->rtspSocketListHead );
    /* Loop until we find one rtspSocket with a pending messages */
    while (hRtspSocket)
    {
        rc = BIP_RtspSocket_RecvRequest( hRtspSocket, hRtspRequest );
        if (rc == BIP_ERR_NOT_AVAILABLE)
        {
            /* No more messages available for this hRtspSocket, so break out */
            BDBG_MSG(( "No message available for hRtspSocket %p, try the next one", hRtspSocket ));
            hRtspSocket = BLST_Q_NEXT( hRtspSocket, rtspSocketListNext );
            continue;
        }
        else if (rc != BIP_SUCCESS)
        {
            /* Error while receiving the next RtspRequest, destroy this rtspSocket */
            BDBG_MSG(( "error while receiving the next Request on rtspSocket %p, destroying socket", hRtspSocket )); /* happens when socket closes */

            /* Remove socket from our list and then destroy it */
            hRtspSocketNext = BLST_Q_NEXT( hRtspSocket, rtspSocketListNext );
            BDBG_MSG(( "destroyed RtspSocket %p, next %p", hRtspSocket, hRtspSocketNext ));
            BLST_Q_REMOVE( &hRtspListener->rtspSocketListHead, hRtspSocket, rtspSocketListNext );
            BIP_RtspSocket_Destroy( hRtspSocket );

            /* Now point to the next socket */
            hRtspSocket = hRtspSocketNext;

            /* We return this code as app doesn't really track individual sockets */
            rc = BIP_ERR_NOT_AVAILABLE;

            /* Continue to top to see if next rtspSocket has a pending message */
            continue;
        }
        else
        {
            BDBG_MSG(( "%s: hRtspSocket %p, hRtspRequest %p", __FUNCTION__, hRtspSocket, hRtspRequest ));
            /* TODO: Reset idle timeout for this socket */

            /* TODO: Also, reset the timeout function that prunes the sockets w/ errors as we go thru each socket once here and prune them if there is error on it */
            rc = BIP_SUCCESS;
            break;
        }
    }
    BKNI_ReleaseMutex( hRtspListener->lock );

    return( rc );
} /* BIP_RtspListener_RecvRequest */

/**
 * Summary:
 * Rtsp Listener API to create a new RTSP Session for a given RTSP Request
 **/
BIP_RtspSessionHandle BIP_RtspListener_CreateSession(
    BIP_RtspListenerHandle hRtspListener,
    BIP_RtspRequestHandle  hRtspRequest
    )
{
    BIP_RtspSessionHandle hRtspSession;
    BIP_RtspSocketHandle  hRtspSocket;
    BIP_Status             rc;

    BDBG_MSG(( "%s: creating session on rtspSession %p, hRtspRequest %p", __FUNCTION__, hRtspListener, hRtspRequest ));
    hRtspSocket = BIP_RtspRequest_GetSocket( hRtspRequest );

    hRtspSession = BIP_RtspSocket_CreateSession( hRtspSocket, hRtspRequest );
    BIP_CHECK_GOTO(( hRtspSession ), ( "Failed to create session" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    rc = BIP_RtspSession_SetInterfaceName( hRtspSession, BIP_RtspSocket_GetInterfaceName( hRtspSocket ));
    BIP_CHECK_GOTO(( hRtspSession ), ( "BIP_RtspSession_SetInterfaceName Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );
    return( hRtspSession );

error:
    return( NULL );
} /* BIP_RtspListener_CreateSession */

/**
 * TODO:
 * Add a function that runs via a periodic timer and prunes rtspSockets with errors or one which have been idle for a while.
 * This needs to be done via timer because a client may open a connection and before it sends a RTSP request
 * it crashes w/o sending any message or closing the TCP connection.
 * This timer is reset by the BIP_RtspListener_RecvMessage() as that function also prunes any rtspSockets w/ errors.
 **/
