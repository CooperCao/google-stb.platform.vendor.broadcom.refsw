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
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#include <string.h>
#include <stdio.h>

#include "bip_priv.h"
#include "bip_socket.h"

BDBG_MODULE(bip_socket);

BDBG_OBJECT_ID( BIP_Socket );
BDBG_OBJECT_ID( BIP_SocketRecvArb );

BIP_CLASS_DECLARE(BIP_Socket);

/* States used for shutting down and object prior to object destruction. */
typedef enum
{
    BIP_SocketShutdownState_eNormal,         /* Normal existing (not shutdown) state.          */
    BIP_SocketShutdownState_eStartShutdown,  /* Starting shutdown... No further APIs accepted. */
    BIP_SocketShutdownState_eFinishingArbs,  /* Waiting for busy arbs to finish                */
    BIP_SocketShutdownState_eShutdownDone,   /* Object is shut down and ready to be destroyed  */
    BIP_SocketShutdownState_eMax             /* eMax: last enum.                               */
} BIP_SocketShutdownState;

/* States used for processing a BIP_Socket_Recv() API request. */
typedef enum
{
    BIP_SocketRecvState_eIdle,          /* No receive in progress */
    BIP_SocketRecvState_eNew,           /* Starting on new receive request */
    BIP_SocketRecvState_eReceiving,     /* Receive in progress */
    BIP_SocketRecvState_eMax            /* eMax: last enum. */
} BIP_SocketRecvState;

/* States used for processing a BIP_Socket_Connect() API request. */
typedef enum
{
    BIP_SocketConnectState_eNotConnected,  /* Not yet connected to the peer. */
    BIP_SocketConnectState_eNew,           /* Starting on new connect request */
    BIP_SocketConnectState_eConnecting,    /* Connect in progress */
    BIP_SocketConnectState_eDone,          /* Connect is done, status reflects success or failure. */
    BIP_SocketConnectState_eConnected,     /* Connected to the peer. */
    BIP_SocketConnectState_eMax            /* eMax: last enum. */
} BIP_SocketConnectState;

/* States used for processing a BIP_Socket_Send() API request. */
typedef enum
{
    BIP_SocketSendState_eIdle,          /* No receive in progress */
    BIP_SocketSendState_eNew,           /* Starting on new receive request */
    BIP_SocketSendState_eSending,       /* Send in progress */
    BIP_SocketSendState_eMax            /* eMax: last enum. */
} BIP_SocketSendState;

/* States to remember whether an IoChecker is enabled or not. */
typedef enum
{
    BIP_SocketIoCheckerState_eIdle,     /* Io Checker is not checking. */
    BIP_SocketIoCheckerState_eBusy,     /* IoChecker is checking. */
    BIP_SocketIoCheckerState_eMax       /* eMax: last enum. */
} BIP_SocketIoCheckerState;

/* States to keep track of when we should send callbacks to our client. */
typedef enum
{
    BIP_SocketCallbackState_eDisabled,    /* dataReady callback is not enabled by app. */
    BIP_SocketCallbackState_eArmed,       /* Callback enabled, fire callback when dataReady occurs. */
    BIP_SocketCallbackState_eTriggered,   /* Callback has fired, don't re-arm until data is read. */
    BIP_SocketCallbackState_eMax          /* eMax: last enum. */
} BIP_SocketCallbackState;


typedef struct BIP_SocketRecvArb  /* AppArb for BIP_Socket_Recv() */
{
    BIP_Arb    arb;     /* Arb "base class" struct. Must be first field in AppArb struct! */
    BDBG_OBJECT( BIP_SocketRecvArb )

    BIP_SocketRecvSettings  settings;   /* Copy of Recv settings passed by caller. */
    BLST_Q_ENTRY(BIP_SocketRecvArb)         recvArbQueueNext;

} BIP_SocketRecvArb;


/* The object's data structure: */
struct BIP_Socket
{
    BDBG_OBJECT( BIP_Socket )

    struct
    {
        BIP_ArbHandle       hArb;
    } shutdownApi;

    struct
    {
        BIP_ArbHandle       hArb;
        BIP_SocketSettings *pSettings;
    } getSettingsApi;

    struct
    {
        BIP_ArbHandle       hArb;
        BIP_SocketSettings *pSettings;
    } setSettingsApi;

    BIP_ArbListHandle   hRecvArbList;
    BLST_Q_HEAD( recvArbQueueHead, BIP_SocketRecvArb)    recvArbQueueHead;

    struct
    {
        BIP_ArbHandle               hArb;
        BIP_SocketConnectSettings  settings;   /* Copy of Connect settings passed by caller. */
    } connectApi;

    struct
    {
        BIP_ArbHandle           hArb;
        BIP_SocketSendSettings  settings;   /* Copy of Recv settings passed by caller. */
    } sendApi;

    int                      socketFd;
    bool                     socketAtEofOrError;
    B_MutexHandle            hObjectLock;
    B_EventHandle            hObjectShutdownEvent;  /* Used for waiting until object is ready to destroy. */
    BIP_SocketCreateSettings createSettings;
    BIP_SocketSettings       settings;
    char                     remoteIpAddress[INET6_ADDRSTRLEN];
    char                     localIpAddress[INET6_ADDRSTRLEN];

    BIP_CLASS_DEFINE_INSTANCE_VARS(BIP_Socket);  /* Per-instance data used by BIP_Class. */

    struct      /* Variables used for Connecting to a BIP_Socket.  */
    {
        BIP_SocketConnectState              state;
        BIP_IoCheckerHandle                 hIoChecker;         /* handle to IoChecker for writeReady.  */
        BIP_SocketIoCheckerState            ioCheckerState;
        BIP_SocketCallbackState             writeReadyCallbackState;
        B_SchedulerTimerId                  timerId;
        B_Time                              startTime;
        BIP_Status                          completionStatus;   /* BIP_INF_IN_PROGRESS means Connect is not complete yet. */
        bool                                connectCallbackFired;
    } connect;

    struct      /* Variables used for Recieving data from a BIP_Socket.  */
    {
        BIP_SocketRecvState                 state;
        BIP_SocketRecvArb                  *pAppArb;
        BIP_IoCheckerHandle                 hIoChecker;         /* handle to IoChecker for dataReady.  */
        BIP_SocketIoCheckerState            ioCheckerState;
        BIP_SocketCallbackState             dataReadyCallbackState;
        B_SchedulerTimerId                  timerId;
        size_t                              bytesRead;          /* Number of bytes actually read. */
        B_Time                              startTime;
        BIP_Status                          completionStatus;   /* BIP_INF_IN_PROGRESS means Recv is not complete yet. */
    } recv;

    struct      /* Variables used for Recieving data from a BIP_Socket.  */
    {
        BIP_SocketSendState                 state;
        BIP_IoCheckerHandle                 hIoChecker;         /* handle to IoChecker for writeReady.  */
        BIP_SocketIoCheckerState            ioCheckerState;
        BIP_SocketCallbackState             writeReadyCallbackState;
        B_SchedulerTimerId                  timerId;
        size_t                              bytesSent;          /* Number of bytes actually read.               */
        B_Time                              startTime;
        BIP_Status                          completionStatus;   /* BIP_INF_IN_PROGRESS means Send is not complete yet. */
    } send;

    struct      /* Variables used for shutting down the object.  */
    {
        BIP_SocketShutdownState             state;
    } shutdown;

};

#define BIP_SOCKET_PRINTF_FMT  \
    "[hSocket=%p Type=%s Port=%s Ip=%s Iface=%s fd=%d Eof/Err=%s localIP=%s remoteIp=%s]"

#define BIP_SOCKET_PRINTF_ARG(pObj)                                                         \
    (void *)(pObj),                                                                                 \
    (pObj)->createSettings.type==BIP_SocketType_eTcp   ? "Tcp"   :                          \
    (pObj)->createSettings.type==BIP_SocketType_eUdpTx ? "UdpTx" :                          \
    (pObj)->createSettings.type==BIP_SocketType_eUdpRx ? "UdpRx" :                          \
                                                         "<Undefined>",                     \
    (pObj)->createSettings.pPort          ? (pObj)->createSettings.pPort          : "NULL", \
    (pObj)->createSettings.pIpAddress     ? (pObj)->createSettings.pIpAddress     : "NULL", \
    (pObj)->createSettings.pInterfaceName ? (pObj)->createSettings.pInterfaceName : "NULL", \
    (pObj)->socketFd,                                                                       \
    (pObj)->socketAtEofOrError ? "Y" : "N",                                                 \
    (pObj)->localIpAddress,                                                                 \
    (pObj)->remoteIpAddress


/******************************************************************************
 *  Check to see if the object has been completely shut down (so that it can
 *  be destroyed.
 ******************************************************************************/
static BIP_Status  BIP_Socket_CheckForObjectShutdown(
    BIP_SocketHandle hSocket
    )
{
    BIP_Status  rc = BIP_SUCCESS;

    BDBG_OBJECT_ASSERT( hSocket, BIP_Socket );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Entry..." BIP_MSG_PRE_ARG, (void *)hSocket ));

    if (hSocket->shutdown.state != BIP_SocketShutdownState_eShutdownDone) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Not ready to shut down, returning..." BIP_MSG_PRE_ARG, (void *)hSocket ));
        return(BIP_ERR_OBJECT_BEING_DESTROYED);  /* Destruction still in progress. */
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Setting hObjectShutdownEvent" BIP_MSG_PRE_ARG, (void *)hSocket ));
    B_Event_Set(hSocket->hObjectShutdownEvent);

    return(rc);

} /* BIP_Socket_CheckForObjectShutdown */


/* Prototype for the "generic" processState entry. */
static void processState(void *hObject, int value, BIP_Arb_ThreadOrigin threadOrigin);

/******************************************************************************
 *  Here are some wrapper functions for processState when it gets run from
 *  various sources (IoChecker and Timer callbacks).
 ******************************************************************************/

/* Callback from Connect IoChecker. */
static void processStateFromConnectIoCheckerCallback(void *pContext, int param, BIP_IoCheckerEvent eventMask)
{
    BIP_Status          rc;

    BIP_SocketHandle    hSocket = (BIP_SocketHandle) pContext;
    BSTD_UNUSED(param);
    BSTD_UNUSED(eventMask);

    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Entry..." BIP_MSG_PRE_ARG, (void *)hSocket ));

    rc = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_Socket, hSocket);
    if (rc != BIP_SUCCESS) { return; }

    hSocket->connect.ioCheckerState = BIP_SocketIoCheckerState_eIdle;
    hSocket->connect.connectCallbackFired = true;

    BIP_CLASS_UNLOCK(BIP_Socket, hSocket);

    processState( hSocket, 0, BIP_Arb_ThreadOrigin_eIoChecker);
}

/* Callback from Recv IoChecker. */
static void processStateFromRecvIoCheckerCallback(void *pContext, int param, BIP_IoCheckerEvent eventMask)
{
    BIP_Status          rc;

    BIP_SocketHandle    hSocket = (BIP_SocketHandle) pContext;
    BSTD_UNUSED(param);
    BSTD_UNUSED(eventMask);

    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Entry..." BIP_MSG_PRE_ARG, (void *)hSocket ));

    rc = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_Socket, hSocket);
    if (rc != BIP_SUCCESS) { return; }

    hSocket->recv.ioCheckerState = BIP_SocketIoCheckerState_eIdle;

    BIP_CLASS_UNLOCK(BIP_Socket, hSocket);

    processState( hSocket, 0, BIP_Arb_ThreadOrigin_eIoChecker);
}

/* Callback from Send IoChecker. */
static void processStateFromSendIoCheckerCallback(void *pContext, int param, BIP_IoCheckerEvent eventMask)
{
    BIP_Status          rc;
    BIP_SocketHandle    hSocket = (BIP_SocketHandle) pContext;
    BSTD_UNUSED(param);
    BSTD_UNUSED(eventMask);

    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Entry..." BIP_MSG_PRE_ARG, (void *)hSocket ));

    rc = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_Socket, hSocket);
    if (rc != BIP_SUCCESS) { return; }

    hSocket->send.ioCheckerState = BIP_SocketIoCheckerState_eIdle;
    BIP_CLASS_UNLOCK(BIP_Socket, hSocket);

    processState( hSocket, 0, BIP_Arb_ThreadOrigin_eIoChecker);
}


/* Callback from the recv timer. */
static void processStateFromTimerCallback_Recv(void *pContext)
{
    BIP_Status          rc;
    BIP_SocketHandle    hSocket = (BIP_SocketHandle) pContext;
    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Entry..." BIP_MSG_PRE_ARG, (void *)hSocket ));

    rc = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_Socket, hSocket);
    if (rc != BIP_SUCCESS) { return; }

    if (hSocket->recv.timerId) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Got BIP_Timer callback for Recv, marking timer as self-destroyed" BIP_MSG_PRE_ARG, (void *)hSocket ));
        hSocket->recv.timerId = NULL;   /* Indicate timer not active. */
    }
    BIP_CLASS_UNLOCK(BIP_Socket, hSocket);

    processState( hSocket, 0, BIP_Arb_ThreadOrigin_eTimer);
}

/* Callback from the connect timer. */
void processStateFromTimerCallback_Connect(void *pContext)
{
    BIP_Status          rc;
    BIP_SocketHandle    hSocket = (BIP_SocketHandle) pContext;
    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Entry..." BIP_MSG_PRE_ARG, (void *)hSocket ));

    rc = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_Socket, hSocket);
    if (rc != BIP_SUCCESS) { return; }

    if (hSocket->connect.timerId) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Got BIP_Timer callback for Connect, marking timer as self-destroyed" BIP_MSG_PRE_ARG, (void *)hSocket ));
        hSocket->connect.timerId = NULL;   /* Indicate timer not active. */
    }
    BIP_CLASS_UNLOCK(BIP_Socket, hSocket);

    processState( hSocket, 0, BIP_Arb_ThreadOrigin_eTimer);
}

/* Callback from the send timer. */
void processStateFromTimerCallback_Send(void *pContext)
{
    BIP_Status          rc;
    BIP_SocketHandle    hSocket = (BIP_SocketHandle) pContext;
    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Entry..." BIP_MSG_PRE_ARG, (void *)hSocket ));

    rc = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_Socket, hSocket);
    if (rc != BIP_SUCCESS) { return; }

    if (hSocket->send.timerId) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Got BIP_Timer callback for Send, marking timer as self-destroyed" BIP_MSG_PRE_ARG, (void *)hSocket ));
        hSocket->send.timerId = NULL;   /* Indicate timer not active. */
    }
    BIP_CLASS_UNLOCK(BIP_Socket, hSocket);

    processState( hSocket, 0, BIP_Arb_ThreadOrigin_eTimer);
}

static bool getRemoteIpAddress(
    int socketFd,
    char *socketName,
    int socketNameSize
    )
{
    bool rc = false;
    int error = 0;
    struct sockaddr_storage socketAddress;
    socklen_t socketAddressLen = sizeof(socketAddress);

    if ( (rc = getpeername( socketFd, (struct sockaddr*)&socketAddress, &socketAddressLen )) == 0 )
    {
        if ((error = getnameinfo( (struct sockaddr*)&socketAddress, socketAddressLen, socketName, socketNameSize, 0, 0, NI_NUMERICHOST )) == 0)
        {
            BDBG_MSG(( BIP_MSG_PRE_FMT "socketFd %d: Remote IP Address: %s" BIP_MSG_PRE_ARG, socketFd, socketName ));
            rc = true;
        }
        else
        {
            BDBG_WRN((BIP_MSG_PRE_FMT "socketFd %d: getnameinfo failed: errString %s, errno %d" BIP_MSG_PRE_ARG, socketFd, gai_strerror(error), errno ));
        }
    }
    else
    {
        int savedErrno = errno;
        BDBG_WRN((BIP_MSG_PRE_FMT "socketFd %d: getpeername failed: errString %s, errno %d rc=%d" BIP_MSG_PRE_ARG, socketFd, strerror(savedErrno), savedErrno, rc ));
    }
    if (rc == false) BDBG_WRN((BIP_MSG_PRE_FMT "socketFd %d: Remote IP address is not yet associated with this socket, errString %s, errno %d" BIP_MSG_PRE_ARG, socketFd, error==0?"":gai_strerror(error), errno ));
    return rc;
}

static bool getLocalIpAddress(
    int socketFd,
    char *socketName,
    int socketNameSize
    )
{
    bool rc = false;
    int error = 0;
    struct sockaddr_storage socketAddress;
    socklen_t socketAddressLen = sizeof(socketAddress);

    if ( getsockname( socketFd, (struct sockaddr*)&socketAddress, &socketAddressLen ) == 0 )
    {
        if ((error = getnameinfo( (struct sockaddr*)&socketAddress, socketAddressLen, socketName, socketNameSize, 0, 0, NI_NUMERICHOST )) == 0)
        {
            BDBG_MSG(( BIP_MSG_PRE_FMT "socketFd %d: Peer IP Address: %s" BIP_MSG_PRE_ARG, socketFd, socketName ));
            rc = true;
        }
    }
    if (rc == false) BDBG_WRN((BIP_MSG_PRE_FMT "socketFd %d: Local IP address is not yet associated with this socket, errString %s, errno %d" BIP_MSG_PRE_ARG, socketFd, error==0?"":gai_strerror(error), errno ));
    return rc;
}

static BIP_Status configureBipSocket(
    BIP_SocketHandle    hSocket
    )
{
    BIP_Status rc = BIP_SUCCESS;

    {
        BIP_IoCheckerCreateSetting  ioCheckerCreateSettings;

        ioCheckerCreateSettings.fd = hSocket->socketFd;
        ioCheckerCreateSettings.settings.callBackFunction = processStateFromRecvIoCheckerCallback;
        ioCheckerCreateSettings.settings.callBackContext =  hSocket;
        ioCheckerCreateSettings.settings.callBackParam   = 0;
        hSocket->recv.hIoChecker = BIP_IoChecker_Create(&ioCheckerCreateSettings);
        BIP_CHECK_GOTO(( hSocket->recv.hIoChecker !=NULL ), ( "BIP_IoChecker_Create failed" ), error, BIP_ERR_INTERNAL, rc );

        ioCheckerCreateSettings.fd = hSocket->socketFd;
        ioCheckerCreateSettings.settings.callBackFunction = processStateFromSendIoCheckerCallback;
        ioCheckerCreateSettings.settings.callBackContext =  hSocket;
        ioCheckerCreateSettings.settings.callBackParam   = 0;
        hSocket->send.hIoChecker = BIP_IoChecker_Create(&ioCheckerCreateSettings);
        BIP_CHECK_GOTO(( hSocket->send.hIoChecker !=NULL ), ( "BIP_IoChecker_Create failed" ), error, BIP_ERR_INTERNAL, rc );
    }

    if (getRemoteIpAddress( hSocket->socketFd, hSocket->remoteIpAddress, INET6_ADDRSTRLEN) == false)
    {
        BDBG_WRN(( BIP_MSG_PRE_FMT "hSocket %p: Couldn't obtain the Socket Peer Name" BIP_MSG_PRE_ARG, (void *)hSocket ));
    }
    if (getLocalIpAddress( hSocket->socketFd, hSocket->localIpAddress, INET6_ADDRSTRLEN) == false)
    {
        BDBG_WRN(( BIP_MSG_PRE_FMT "hSocket %p: Couldn't obtain the Socket Peer Name" BIP_MSG_PRE_ARG, (void *)hSocket ));
    }

error:
    return (rc);
} /* configureBipSocket */


/******************************************************************************
 *  The state processor to handle the connect related processing.
 *  This function must be called with the hObjectLock already locked.
 ******************************************************************************/
static void processConnectState_locked(void *hObject, int value, BIP_Arb_ThreadOrigin threadOrigin)
{
    BIP_SocketHandle    hSocket = (BIP_SocketHandle) hObject;
    BIP_ArbHandle       hArb;
    BIP_Status          rc = BIP_SUCCESS;
    struct addrinfo *   pAddrInfoList = NULL;
    int                 savedErrno = 0;

    BSTD_UNUSED(value);
    BSTD_UNUSED(threadOrigin);

    BDBG_ASSERT(hSocket);
    BDBG_OBJECT_ASSERT(hSocket,BIP_Socket);

    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Entry..." BIP_MSG_PRE_ARG, (void *)hSocket ));

    /**************************************************************************
     * Start by handling the Arbs for the BIP_Socket_Connect API.
     **************************************************************************/
    if (BIP_Arb_IsNew(hArb = hSocket->connectApi.hArb)) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Got Connect Arb request!" BIP_MSG_PRE_ARG, (void *)hSocket ));

        if (hSocket->shutdown.state != BIP_SocketShutdownState_eNormal) {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: BIP_Socket is shutting down, rejecting request (BIP_ERR_OBJECT_BEING_DESTROYED) " BIP_MSG_PRE_ARG, (void *)hSocket ));
            BIP_Arb_RejectRequest(hArb, BIP_ERR_OBJECT_BEING_DESTROYED);
        }
        else {
            /* If our connect state isn't NotConnected, fail the request. */
            if (hSocket->connect.state != BIP_SocketConnectState_eNotConnected) {
                BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Can't call BIP_Player_Connect in this current connect state=%d " BIP_MSG_PRE_ARG, (void *)hSocket, hSocket->connect.state ));
                BIP_Arb_RejectRequest(hArb, BIP_ERR_INVALID_API_SEQUENCE);
            }
            else /* The connect state is eNotConnected, go ahead and start a new connect. */
            {
                hSocket->connect.state             = BIP_SocketConnectState_eNew;
                hSocket->connect.completionStatus  = BIP_INF_IN_PROGRESS;

                BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Connecting to ip:port=%s:%s" BIP_MSG_PRE_ARG,
                            (void *)hSocket, hSocket->connectApi.settings.input.pIpAddress, hSocket->connectApi.settings.input.pPort ));

                BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Calling BIP_Arb_AcceptRequest()" BIP_MSG_PRE_ARG, (void *)hSocket ));
                BIP_Arb_AcceptRequest(hArb);
            }
        }
    }

    /**************************************************************************
    *  BIP_SocketConnectState_eNew: Start a new Connect request.
    **************************************************************************/
    if (hSocket->connect.state == BIP_SocketConnectState_eNew) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Handling Connect State: eNew" BIP_MSG_PRE_ARG, (void *)hSocket ));
        B_Time_Get(&hSocket->connect.startTime);
        if (hSocket->connectApi.settings.api.timeout > 0) {
            BIP_TimerCreateSettings timerCreateSettings;

            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Starting timer for %ld ms" BIP_MSG_PRE_ARG, (void *)hSocket, (long int)hSocket->connectApi.settings.api.timeout ));
            timerCreateSettings.input.callback    = processStateFromTimerCallback_Connect;
            timerCreateSettings.input.pContext    = hSocket;
            timerCreateSettings.input.timeoutInMs = hSocket->connectApi.settings.api.timeout;
            hSocket->connect.timerId = BIP_Timer_Create(&timerCreateSettings);
        }

        /* Setup the socket & connect to the server. */
        {
            int             sfd;
            int             rc;
            struct addrinfo hints;
            struct addrinfo *pAddrInfo = NULL;

            BKNI_Memset(&hints, 0, sizeof(hints));
            hints.ai_family     = AF_UNSPEC;
            hints.ai_socktype   = hSocket->createSettings.type == BIP_SocketType_eTcp ? SOCK_STREAM : SOCK_DGRAM;
            hints.ai_protocol   = hSocket->createSettings.type == BIP_SocketType_eTcp ? IPPROTO_TCP: IPPROTO_UDP;
            do {
                rc = getaddrinfo(
                        hSocket->connectApi.settings.input.pIpAddress,
                        hSocket->connectApi.settings.input.pPort,
                        &hints, &pAddrInfoList);
            } while (rc == EAI_AGAIN);
            BIP_CHECK_GOTO((rc == 0), ("%s: ERROR: getaddrinfo() failed for server:port: %s:%s, error=%s", BSTD_FUNCTION,
                                        hSocket->connectApi.settings.input.pIpAddress, hSocket->connectApi.settings.input.pPort, gai_strerror(rc)),
                                        errorInNewState, BIP_ERR_OS_ERRNO, hSocket->connect.completionStatus);
            for (pAddrInfo = pAddrInfoList; pAddrInfo != NULL; pAddrInfo = pAddrInfo->ai_next) {
                /* Print the server ip & port info to which we will attempt the socket & connect calls. */
                {
                    char serverName[NI_MAXHOST+1];
                    char serverPort[NI_MAXSERV+1];

                    BKNI_Memset(serverName, 0, sizeof(serverName));
                    BKNI_Memset(serverPort, 0, sizeof(serverPort));
                    rc = getnameinfo(pAddrInfo->ai_addr, pAddrInfo->ai_addrlen, serverName, sizeof(serverName), serverPort, sizeof(serverPort), NI_NUMERICHOST | NI_NUMERICSERV);
                    if (rc == 0) {
                        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p:  connect to %s:%s socket family=%d type=%d protocol=%d " BIP_MSG_PRE_ARG,
                                    (void *)hSocket, serverName, serverPort,
                                    pAddrInfo->ai_family, pAddrInfo->ai_socktype, pAddrInfo->ai_protocol
                                    ));
                    }
                    else {
                        BDBG_WRN(( BIP_MSG_PRE_FMT "hSocket %p: getnameinfo() failed, error=%s" BIP_MSG_PRE_ARG, (void *)hSocket, gai_strerror(rc)));
                    }
                }
                sfd = socket(pAddrInfo->ai_family, pAddrInfo->ai_socktype|SOCK_NONBLOCK, pAddrInfo->ai_protocol);
                savedErrno = errno;
                if (sfd == -1) continue;

                /* socket() is successful, try connect(). */
                BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: socket() successful, try connect()" BIP_MSG_PRE_ARG, (void *)hSocket ));
                do {
                    rc = connect(sfd, pAddrInfo->ai_addr, pAddrInfo->ai_addrlen);
                    savedErrno = errno;
                } while (rc != 0 && (savedErrno == EAGAIN || savedErrno == EINTR));

                if (savedErrno == 0 || savedErrno == EINPROGRESS) break;                  /* Success */
                close(sfd);
            }
            BIP_CHECK_GOTO((pAddrInfo), ("%s: ERROR: socket()/connect() failed for server:port: %s:%s, errno=%d", BSTD_FUNCTION,
                                        hSocket->connectApi.settings.input.pIpAddress, hSocket->connectApi.settings.input.pPort, savedErrno),
                                        errorInNewState, BIP_StatusFromErrno( savedErrno ), hSocket->connect.completionStatus);
            /* we are here for either success or in-progress cases only, save the socket. */
            hSocket->socketFd = sfd;

            /* check if connect is in progress. */
            if (savedErrno == EINPROGRESS) {
                /* Create IO Checker to monitor the socket for write ready event (which gets set when socket is connected). */
                /* Note: it is not enabled yet. */
                BIP_IoCheckerCreateSetting  ioCheckerCreateSettings;

                ioCheckerCreateSettings.fd = hSocket->socketFd;
                ioCheckerCreateSettings.settings.callBackFunction = processStateFromConnectIoCheckerCallback;
                ioCheckerCreateSettings.settings.callBackContext =  hSocket;
                ioCheckerCreateSettings.settings.callBackParam   = 0;
                hSocket->connect.hIoChecker = BIP_IoChecker_Create(&ioCheckerCreateSettings);
                BIP_CHECK_GOTO(( hSocket->connect.hIoChecker !=NULL ), ( "BIP_IoChecker_Create failed" ), errorInNewState, BIP_ERR_INTERNAL, hSocket->connect.completionStatus );
                hSocket->connect.ioCheckerState = BIP_SocketIoCheckerState_eIdle;

                hSocket->connect.completionStatus = BIP_INF_IN_PROGRESS;
                BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: non-blocking connect() started!" BIP_MSG_PRE_ARG, (void *)hSocket));
            }
            else {
                hSocket->connect.completionStatus = BIP_SUCCESS;
            }

errorInNewState:
            if (hSocket->connect.completionStatus == BIP_INF_IN_PROGRESS || hSocket->connect.completionStatus == BIP_SUCCESS) /* eConnecting state will check for connect status. */ {
                hSocket->connect.state = BIP_SocketConnectState_eConnecting;
            }
            else {
                hSocket->connect.state = BIP_SocketConnectState_eDone;
            }
            if (pAddrInfoList) {
                freeaddrinfo(pAddrInfoList);
                pAddrInfoList = NULL;
            }
        } /* setup & connect socket */
    } /* state == eNew */

    /**************************************************************************
     *  BIP_SocketConnectState_eConnecting: check if socket is connected.
     **************************************************************************/
    if (hSocket->connect.state == BIP_SocketConnectState_eConnecting) {
        if ( hSocket->connect.connectCallbackFired) {
            int socketError;
            socklen_t length = sizeof(socketError);

            /**********************************************************************
             *  Check for various completions. First, check the status by connect().
             **********************************************************************/
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Handling Connect State: eConnecting" BIP_MSG_PRE_ARG, (void *)hSocket ));
                rc = getsockopt(hSocket->socketFd, SOL_SOCKET, SO_ERROR, &socketError, &length);
                savedErrno = errno;
                BIP_CHECK_GOTO(( rc == 0 ), ( "hSocket=%p: getsockopt() failed, errno=%d", (void *)hSocket, savedErrno ), errorInConnectingState, BIP_ERR_INTERNAL, hSocket->connect.completionStatus );
                /* Got the socketError, check if the previous connect() completed successfully or it got an error. */
                if (socketError == 0) {
                    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Connected to ip:port=%s:%s" BIP_MSG_PRE_ARG,
                                (void *)hSocket, hSocket->connectApi.settings.input.pIpAddress, hSocket->connectApi.settings.input.pPort ));
                    hSocket->connect.completionStatus = BIP_SUCCESS;
                }
                else if (socketError == EINPROGRESS) {
                    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: connect() still in progress!" BIP_MSG_PRE_ARG, (void *)hSocket ));
                    hSocket->connect.completionStatus = BIP_INF_IN_PROGRESS;
                }
                else {
                    /* we got some error during connect. */
                    hSocket->connect.completionStatus = BIP_StatusFromErrno( socketError );
                    hSocket->socketAtEofOrError = true;
                    close(hSocket->socketFd);
                    hSocket->socketFd = -1;
                    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: connect() failed, socketError=%d, completionStatus=%s"
                                BIP_MSG_PRE_ARG, (void *)hSocket, socketError, BIP_StatusGetText(hSocket->connect.completionStatus) ));
                }
                if (hSocket->connect.completionStatus == BIP_SUCCESS)
                {
                    hSocket->connect.completionStatus = configureBipSocket(hSocket);
                }
            }
        }

errorInConnectingState:
        /**********************************************************************
         *  Check for completion by object destruction.
         **********************************************************************/
        if (hSocket->shutdown.state != BIP_SocketShutdownState_eNormal) {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Aborting connect due to object destruction!" BIP_MSG_PRE_ARG, (void *)hSocket ));
            hSocket->connect.completionStatus = BIP_ERR_OBJECT_BEING_DESTROYED;
        }

        /**********************************************************************
         *  Check for completion by timeout.
         **********************************************************************/
        if (hSocket->connect.completionStatus == BIP_INF_IN_PROGRESS) {
            if (hSocket->connectApi.settings.api.timeout >= 0) {
                B_Time      timeNow;
                int         elapsedTimeInMs;

                B_Time_Get(&timeNow);
                elapsedTimeInMs = B_Time_Diff(&timeNow,&hSocket->connect.startTime);
                BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: connect() Timeout Check: So far: %ld ms, limit is %ld ms." BIP_MSG_PRE_ARG,
                            (void *)hSocket, (long int)elapsedTimeInMs , (long int)hSocket->connectApi.settings.api.timeout ));
                if (elapsedTimeInMs >= hSocket->connectApi.settings.api.timeout) {
                    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: connect() Timed Out!" BIP_MSG_PRE_ARG, (void *)hSocket ));
                    hSocket->connect.completionStatus = BIP_INF_TIMEOUT;
                }
            }
        }

        /* Finally, nable IO Checker if our status is still in-progress. */
        if (hSocket->connect.completionStatus == BIP_INF_IN_PROGRESS) {
            if (hSocket->connect.ioCheckerState != BIP_SocketIoCheckerState_eBusy) {
                BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Enabling ioChecker for ePollOut" BIP_MSG_PRE_ARG, (void *)hSocket ));
                hSocket->connect.completionStatus = BIP_IoChecker_Enable(hSocket->connect.hIoChecker, BIP_IoCheckerEvent_ePollOut);
                if ( hSocket->connect.completionStatus == BIP_SUCCESS) {
                    hSocket->connect.ioCheckerState = BIP_SocketIoCheckerState_eBusy;
                }
                else {
                    BDBG_ERR(( BIP_MSG_PRE_FMT "hSocket %p: BIP_IoChecker_Enable() failed to enable ioChecker for ePollOut" BIP_MSG_PRE_ARG, (void *)hSocket ));
                }
            }
        }
        else {
            /* Completion status is NOT in-progress, we we are done. */
            hSocket->connect.state = BIP_SocketConnectState_eDone;
        }
    }  /* End if    hSocket->connect.state == BIP_SocketConnectState_eConnecting */

    if (hSocket->connect.state == BIP_SocketConnectState_eDone) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Handling Connect State: eDone" BIP_MSG_PRE_ARG, (void *)hSocket ));
        BDBG_ASSERT (hSocket->connect.completionStatus != BIP_INF_IN_PROGRESS);
        /* Cancel timer. */
        if (hSocket->connect.timerId != NULL)
        {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Cancelling timer for Connect" BIP_MSG_PRE_ARG, (void *)hSocket ));
            BIP_Timer_Destroy(hSocket->connect.timerId);
            hSocket->connect.timerId = NULL;
        }

        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Calling BIP_Arb_CompleteRequest()." BIP_MSG_PRE_ARG, (void *)hSocket ));
        BIP_Arb_CompleteRequest(hSocket->connectApi.hArb, hSocket->connect.completionStatus);

        /* invoke writeReady callback if configured. */
        if (hSocket->connect.writeReadyCallbackState == BIP_SocketCallbackState_eArmed && !hSocket->socketAtEofOrError) {
            BDBG_WRN(( BIP_MSG_PRE_FMT "hSocket %p: CheckNow: Socket has room! Firing callback!" BIP_MSG_PRE_ARG, (void *)hSocket ));
            BIP_Arb_AddDeferredCallback(hSocket->setSettingsApi.hArb, &hSocket->settings.writeReadyCallback);
            hSocket->connect.writeReadyCallbackState = BIP_SocketCallbackState_eTriggered;
        }
        if (hSocket->connect.completionStatus == BIP_SUCCESS) {
            hSocket->connect.state = BIP_SocketConnectState_eConnected;
        }
        else {
            hSocket->connect.state = BIP_SocketConnectState_eNotConnected;
        }
        if (hSocket->connect.hIoChecker) BIP_IoChecker_Destroy(hSocket->connect.hIoChecker);
        hSocket->connect.hIoChecker = NULL;
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Returning." BIP_MSG_PRE_ARG, (void *)hSocket ));

    return;
}


/******************************************************************************
 *  The state processor to handle the Recv related processing.
 *  This function must be called with the hObjectLock already locked.
 ******************************************************************************/
static void processRecvState_locked(void *hObject, int value, BIP_Arb_ThreadOrigin threadOrigin)
{
    BIP_SocketHandle     hSocket = (BIP_SocketHandle) hObject;
    BIP_ArbHandle        hArb;
    BIP_Status           rc = BIP_SUCCESS;

    BSTD_UNUSED(value);
    BSTD_UNUSED(threadOrigin);

    BDBG_ASSERT(hSocket);
    BDBG_OBJECT_ASSERT(hSocket,BIP_Socket);

    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Entry..." BIP_MSG_PRE_ARG, (void *)hSocket ));

    /**************************************************************************
     * Start by handling the Arbs for the BIP_Socket_Recv API.
     **************************************************************************/
    hArb = BIP_ArbList_GetNewArb(hSocket->hRecvArbList);
    if (hArb) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Got Recv Arb request!" BIP_MSG_PRE_ARG, (void *)hSocket ));

        if (hSocket->shutdown.state != BIP_SocketShutdownState_eNormal) {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: BIP_Socket is shutting down, rejecting request (BIP_ERR_OBJECT_BEING_DESTROYED) " BIP_MSG_PRE_ARG, (void *)hSocket ));
            BIP_Arb_RejectRequest(hArb, BIP_ERR_OBJECT_BEING_DESTROYED);
        }
        else {
            /* Add the new Arb to our list of active Arbs. */
            BLST_Q_INSERT_TAIL(&hSocket->recvArbQueueHead, BIP_APPARB_FROM_ARB( hArb, BIP_SocketRecvArb), recvArbQueueNext);

            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Calling BIP_Arb_AcceptRequest()" BIP_MSG_PRE_ARG, (void *)hSocket ));
            BIP_Arb_AcceptRequest(hArb);
        }
    }

    /**************************************************************************
    *  Handle the Recv state...
    **************************************************************************/
    if (hSocket->recv.state == BIP_SocketRecvState_eIdle) {
        BIP_SocketRecvArb   *pRecvArb;

        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Handling Receive State: eIdle" BIP_MSG_PRE_ARG, (void *)hSocket ));

        pRecvArb = BLST_Q_FIRST(&hSocket->recvArbQueueHead);
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Oldest Recv queued Recv Arb: %p" BIP_MSG_PRE_ARG, (void *)hSocket, (void *)pRecvArb ));

        if (pRecvArb) {
            BDBG_OBJECT_ASSERT(pRecvArb, BIP_SocketRecvArb);

            BLST_Q_REMOVE(&hSocket->recvArbQueueHead, pRecvArb, recvArbQueueNext);
            hSocket->recv.pAppArb           = pRecvArb;
            hSocket->recv.state             = BIP_SocketRecvState_eNew;
            hSocket->recv.completionStatus  = BIP_INF_IN_PROGRESS;

            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Recv pBuffer:%p bytesToRead:%ld" BIP_MSG_PRE_ARG, (void *)hSocket, (void *)pRecvArb->settings.output.pBuffer, (long int)pRecvArb->settings.input.bytesToRead ));
        }
    }

    /**************************************************************************
    *  BIP_SocketRecvState_eNew: Start a new Recv request.
    **************************************************************************/
    if (hSocket->recv.state == BIP_SocketRecvState_eNew) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Handling Receive State: eNew" BIP_MSG_PRE_ARG, (void *)hSocket ));
        B_Time_Get(&hSocket->recv.startTime);
        if (hSocket->recv.pAppArb->settings.api.timeout > 0) {
            BIP_TimerCreateSettings timerCreateSettings;

            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Starting timer for %ld ms" BIP_MSG_PRE_ARG, (void *)hSocket, (long int)hSocket->recv.pAppArb->settings.api.timeout ));
            timerCreateSettings.input.callback    = processStateFromTimerCallback_Recv;
            timerCreateSettings.input.pContext    = hSocket;
            timerCreateSettings.input.timeoutInMs = hSocket->recv.pAppArb->settings.api.timeout;
            hSocket->recv.timerId = BIP_Timer_Create(&timerCreateSettings);
        }

        hSocket->recv.bytesRead = 0;
        hSocket->recv.state = BIP_SocketRecvState_eReceiving;
    }

    /**************************************************************************
     *  BIP_SocketRecvState_eReceiving: If there is any data sitting in the
     *  socket, then read it.
     **************************************************************************/
    if (hSocket->recv.state == BIP_SocketRecvState_eReceiving) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Handling Receive State: eReceiving" BIP_MSG_PRE_ARG, (void *)hSocket ));

        rc = BIP_Fd_CheckNow(hSocket->socketFd, BIP_IoCheckerEvent_ePollIn);
        if (rc == 0 ) {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: CheckNow: Nothing there" BIP_MSG_PRE_ARG, (void *)hSocket ));

            if (hSocket->recv.dataReadyCallbackState == BIP_SocketCallbackState_eTriggered) {
                hSocket->recv.dataReadyCallbackState = BIP_SocketCallbackState_eArmed;
            }
        }
        else
        {
            ssize_t bytesRead;
            int     myErrno;

            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: CheckNow: Something there (0x%0X)! Trying to read %zd bytes"
                       BIP_MSG_PRE_ARG, (void *)hSocket, rc, hSocket->recv.pAppArb->settings.input.bytesToRead - hSocket->recv.bytesRead ));
            bytesRead = recv( hSocket->socketFd,
                              hSocket->recv.pAppArb->settings.output.pBuffer + hSocket->recv.bytesRead,
                              hSocket->recv.pAppArb->settings.input.bytesToRead - hSocket->recv.bytesRead,
                              0
                              );
            myErrno = errno;

            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: After read, bytesRead: %ld" BIP_MSG_PRE_ARG, (void *)hSocket, (long int)bytesRead ));
            if (bytesRead < 0)
            {
                hSocket->recv.completionStatus = BIP_StatusFromErrno( myErrno );
                hSocket->socketAtEofOrError = true;
            }
            else if (bytesRead == 0) /* End of file */
            {
                hSocket->socketAtEofOrError = true;
                hSocket->recv.completionStatus = BIP_SUCCESS;
            }
            else /* Must be success (bytesRead>0) */
            {
                hSocket->recv.bytesRead += bytesRead;
                hSocket->recv.completionStatus = BIP_SUCCESS;
            }
        }

        /**********************************************************************
         *  Check for completion by timeout.
         **********************************************************************/
        if (hSocket->recv.completionStatus == BIP_INF_IN_PROGRESS) {
            if (hSocket->recv.pAppArb->settings.api.timeout >= 0) {
                B_Time      timeNow;
                int         elapsedTimeInMs;

                B_Time_Get(&timeNow);
                elapsedTimeInMs = B_Time_Diff(&timeNow,&hSocket->recv.startTime);
                BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Read Timeout Check: So far: %ld ms, limit is %ld ms." BIP_MSG_PRE_ARG, (void *)hSocket, (long int)elapsedTimeInMs , (long int)hSocket->recv.pAppArb->settings.api.timeout ));
                if (elapsedTimeInMs >= hSocket->recv.pAppArb->settings.api.timeout) {
                    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Read Timed Out!" BIP_MSG_PRE_ARG, (void *)hSocket ));
                    hSocket->recv.completionStatus = BIP_INF_TIMEOUT;
                }
            }
        }

        /**********************************************************************
         *  Check for completion by object destruction.
         **********************************************************************/
        if (hSocket->shutdown.state != BIP_SocketShutdownState_eNormal) {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Aborting Read due to object destruction!" BIP_MSG_PRE_ARG, (void *)hSocket ));
            hSocket->recv.completionStatus = BIP_ERR_OBJECT_BEING_DESTROYED;
        }

        /**********************************************************************
         *  Check for completion.
         **********************************************************************/
        if (hSocket->recv.completionStatus != BIP_INF_IN_PROGRESS) {

            if (hSocket->recv.timerId != NULL)
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Cancelling timer for Recv" BIP_MSG_PRE_ARG, (void *)hSocket ));
                BIP_Timer_Destroy(hSocket->recv.timerId);
                hSocket->recv.timerId = NULL;
            }

            if (hSocket->recv.pAppArb->settings.output.pBytesRead) {
                *hSocket->recv.pAppArb->settings.output.pBytesRead = hSocket->recv.bytesRead;    /* Set caller's output variable. */
            }

            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Calling BIP_Arb_CompleteRequest()." BIP_MSG_PRE_ARG, (void *)hSocket ));
            BIP_Arb_CompleteRequest(BIP_ARB_FROM_APPARB(hSocket->recv.pAppArb), hSocket->recv.completionStatus);

            hSocket->recv.state = BIP_SocketRecvState_eIdle;
        }
    }  /* End if    hSocket->recv.state == BIP_SocketRecvState_eReceiving */


    /**************************************************************************
     *  Now see if we need to call our customer's readReady callback.
     *
     *  Note that we only call the customer's callback if the socket still
     *  has data now.  So if the customer enables the readReady callback, then
     *  does a blocking or async read, the readReady callback does not fire as
     *  long as the data is being consumed by the read.  After the read
     *  completes and data is still available in the socket, then the readReady
     *  callback will be called.
     *
     *  Also, don't do any more dataReady callbacks after somebody has read the
     *  end-of-file (success with zero bytes).
     **************************************************************************/
    if (hSocket->recv.dataReadyCallbackState == BIP_SocketCallbackState_eArmed &&
        ! hSocket->socketAtEofOrError ) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Checking for DataReady..." BIP_MSG_PRE_ARG, (void *)hSocket ));

        rc = BIP_Fd_CheckNow(hSocket->socketFd, BIP_IoCheckerEvent_ePollIn);
        if (rc != 0 ) {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: CheckNow: Got data! Firing callback!" BIP_MSG_PRE_ARG, (void *)hSocket ));

            BIP_Arb_AddDeferredCallback(hSocket->setSettingsApi.hArb, &hSocket->settings.dataReadyCallback);
            hSocket->recv.dataReadyCallbackState = BIP_SocketCallbackState_eTriggered;
        }
    }

    /**************************************************************************
     *  Now see if we need to enable/disable any ioCheckers.  We need an
     *  enabled ioChecker if either:
     *    1) we have a read in progress, or
     *    2) our customer wants a dataReady callback (and it's armed).
     **************************************************************************/
    if (hSocket->recv.state == BIP_SocketRecvState_eReceiving  ||
        hSocket->recv.dataReadyCallbackState == BIP_SocketCallbackState_eArmed ) {
        if (hSocket->recv.hIoChecker && hSocket->recv.ioCheckerState != BIP_SocketIoCheckerState_eBusy) {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Enabling ioChecker for ePollIn" BIP_MSG_PRE_ARG, (void *)hSocket ));
            rc = BIP_IoChecker_Enable(hSocket->recv.hIoChecker, BIP_IoCheckerEvent_ePollIn);
            BIP_CHECK_GOTO((rc == BIP_SUCCESS), ( "BIP_IoChecker_Enable() failed" ), error, rc, rc );

            hSocket->recv.ioCheckerState = BIP_SocketIoCheckerState_eBusy;
        }
    }
    else
    {
        if (hSocket->recv.hIoChecker && hSocket->recv.ioCheckerState != BIP_SocketIoCheckerState_eIdle) {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Disabling ioChecker for ePollIn" BIP_MSG_PRE_ARG, (void *)hSocket ));
            rc = BIP_IoChecker_Disable(hSocket->recv.hIoChecker, BIP_IoCheckerEvent_ePollIn);
            BIP_CHECK_GOTO((rc == BIP_SUCCESS), ( "BIP_IoChecker_Disable() failed" ), error, rc, rc );

            hSocket->recv.ioCheckerState = BIP_SocketIoCheckerState_eIdle;
        }
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Returning." BIP_MSG_PRE_ARG, (void *)hSocket ));

error:
    return;
}


/******************************************************************************
 *  The state processor to handle the Send related processing.
 *  This function must be called with the hObjectLock already locked.
 ******************************************************************************/
static void processSendState_locked(void *hObject, int value, BIP_Arb_ThreadOrigin threadOrigin)
{
    BIP_SocketHandle    hSocket = (BIP_SocketHandle) hObject;
    BIP_ArbHandle       hArb;
    BIP_Status           rc = BIP_SUCCESS;

    BSTD_UNUSED(value);
    BSTD_UNUSED(threadOrigin);

    BDBG_ASSERT(hSocket);
    BDBG_OBJECT_ASSERT(hSocket,BIP_Socket);

    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Entry..." BIP_MSG_PRE_ARG, (void *)hSocket ));

    /**************************************************************************
     * Start by handling the Arbs for the BIP_Socket_Send API.
     **************************************************************************/
    if (BIP_Arb_IsNew(hArb = hSocket->sendApi.hArb)) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Got Send Arb request!" BIP_MSG_PRE_ARG, (void *)hSocket ));

        if (hSocket->shutdown.state != BIP_SocketShutdownState_eNormal) {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: BIP_Socket is shutting down, rejecting request (BIP_ERR_OBJECT_BEING_DESTROYED) " BIP_MSG_PRE_ARG, (void *)hSocket ));
            BIP_Arb_RejectRequest(hArb, BIP_ERR_OBJECT_BEING_DESTROYED);
        }
        else {
            /* If our send state isn't Idle, fail the request.  Actually, the Arb shouldn't allow this case to occur. */
            if (hSocket->send.state != BIP_SocketSendState_eIdle) {
                BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Calling BIP_Arb_RejectRequest(). Current send.state:%ld " BIP_MSG_PRE_ARG, (void *)hSocket, (long int)hSocket->send.state ));
                BIP_Arb_RejectRequest(hArb, BIP_ERR_ALREADY_IN_PROGRESS);
            }
            else /* The receive state is Idle, go ahead and start a new send. */
            {
                hSocket->send.state             = BIP_SocketSendState_eNew;
                hSocket->send.completionStatus  = BIP_INF_IN_PROGRESS;

                BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Send pBuffer:%p bytesToSend:%ld" BIP_MSG_PRE_ARG, (void *)hSocket, hSocket->sendApi.settings.input.pBuffer, (long int)hSocket->sendApi.settings.input.bytesToSend ));

                BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Calling BIP_Arb_AcceptRequest()" BIP_MSG_PRE_ARG, (void *)hSocket ));
                BIP_Arb_AcceptRequest(hArb);
            }
        }
    }

    /**************************************************************************
    *  Handle the Send state...
    **************************************************************************/

    /**************************************************************************
    *  BIP_SocketSendState_eNew: Start a new Send request.
    **************************************************************************/
    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Handling Send State" BIP_MSG_PRE_ARG, (void *)hSocket ));
    if (hSocket->send.state == BIP_SocketSendState_eNew) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Handling Send State: eNew" BIP_MSG_PRE_ARG, (void *)hSocket ));
        B_Time_Get(&hSocket->send.startTime);
        if (hSocket->sendApi.settings.api.timeout > 0) {
            BIP_TimerCreateSettings timerCreateSettings;

            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Starting timer for %ld ms" BIP_MSG_PRE_ARG, (void *)hSocket, (long int)hSocket->sendApi.settings.api.timeout ));
            timerCreateSettings.input.callback    = processStateFromTimerCallback_Send;
            timerCreateSettings.input.pContext    = hSocket;
            timerCreateSettings.input.timeoutInMs = hSocket->sendApi.settings.api.timeout;
            hSocket->send.timerId = BIP_Timer_Create(&timerCreateSettings);
        }

        hSocket->send.bytesSent = 0;
        hSocket->send.state = BIP_SocketSendState_eSending;
    }

    /**************************************************************************
     *  BIP_SocketSendState_eSending: If there is any room in the
     *  socket, then write our data to it.
     **************************************************************************/
    if (hSocket->send.state == BIP_SocketSendState_eSending) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Handling Send State: eSending" BIP_MSG_PRE_ARG, (void *)hSocket ));

        rc = BIP_Fd_CheckNow(hSocket->socketFd, BIP_IoCheckerEvent_ePollOut);
        if (rc == 0 ) {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: CheckNow: No room in socket" BIP_MSG_PRE_ARG, (void *)hSocket ));

            if (hSocket->send.writeReadyCallbackState == BIP_SocketCallbackState_eTriggered) {
                hSocket->send.writeReadyCallbackState = BIP_SocketCallbackState_eArmed;
            }
        }
        else
        {
            ssize_t bytesSent;
            int     myErrno;

            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: CheckNow: Socket has some room (0x%0X)! Trying to send %zd bytes"
                       BIP_MSG_PRE_ARG, (void *)hSocket, rc, hSocket->sendApi.settings.input.bytesToSend - hSocket->send.bytesSent ));
            bytesSent = send( hSocket->socketFd,
                              hSocket->sendApi.settings.input.pBuffer + hSocket->send.bytesSent,
                              hSocket->sendApi.settings.input.bytesToSend - hSocket->send.bytesSent,
                              MSG_NOSIGNAL  /* This prevents generation of SIGPIPE when other end breaks connection. */
                              );
            myErrno = errno;    /* Save errno before it gets clobbered by next system call. */

            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: After write, bytesSent: %ld" BIP_MSG_PRE_ARG, (void *)hSocket, (long int)bytesSent ));
            if (bytesSent < 0)
            {
                hSocket->send.completionStatus = BIP_StatusFromErrno( myErrno );
                hSocket->socketAtEofOrError = true;
            }
            else if (bytesSent == 0) /* End of file */
            {
                hSocket->socketAtEofOrError = true;
                hSocket->send.completionStatus = BIP_SUCCESS;
            }
            else /* Must be success (bytesSent>0) */
            {
                hSocket->send.bytesSent += bytesSent;
                hSocket->send.completionStatus = BIP_SUCCESS;
            }
        }

        /**********************************************************************
         *  Check for completion by timeout.
         **********************************************************************/
        if (hSocket->send.completionStatus == BIP_INF_IN_PROGRESS) {
            if (hSocket->sendApi.settings.api.timeout >= 0) {
                B_Time      timeNow;
                int         elapsedTimeInMs;

                B_Time_Get(&timeNow);
                elapsedTimeInMs = B_Time_Diff(&timeNow,&hSocket->send.startTime);
                BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Write Timeout Check: So far: %ld ms, limit is %ld ms." BIP_MSG_PRE_ARG, (void *)hSocket, (long int)elapsedTimeInMs , (long int)hSocket->sendApi.settings.api.timeout ));
                if (elapsedTimeInMs >= hSocket->sendApi.settings.api.timeout) {
                    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Write Timed Out!" BIP_MSG_PRE_ARG, (void *)hSocket ));
                    hSocket->send.completionStatus = BIP_INF_TIMEOUT;
                }
            }
        }

        /**********************************************************************
         *  Check for completion by object destruction.
         **********************************************************************/
        if (hSocket->shutdown.state != BIP_SocketShutdownState_eNormal) {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Aborting Write due to object destruction!" BIP_MSG_PRE_ARG, (void *)hSocket ));
            hSocket->send.completionStatus = BIP_ERR_OBJECT_BEING_DESTROYED;
        }

        /**********************************************************************
         *  Check for completion.
         **********************************************************************/
        if (hSocket->send.completionStatus != BIP_INF_IN_PROGRESS) {

            if (hSocket->send.timerId != NULL)
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Cancelling timer for Send" BIP_MSG_PRE_ARG, (void *)hSocket ));
                BIP_Timer_Destroy(hSocket->send.timerId);
                hSocket->send.timerId = NULL;
            }

            if (hSocket->sendApi.settings.output.pBytesSent) {
                *hSocket->sendApi.settings.output.pBytesSent = hSocket->send.bytesSent;    /* Set caller's output variable. */
            }

            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Calling BIP_Arb_CompleteRequest()." BIP_MSG_PRE_ARG, (void *)hSocket ));
            BIP_Arb_CompleteRequest(hSocket->sendApi.hArb, hSocket->send.completionStatus);

            hSocket->send.state = BIP_SocketSendState_eIdle;
        }
    }  /* End if    hSocket->send.state == BIP_SocketSendState_eSending */


    /**************************************************************************
     *  Now see if we need to call our customer's writeReady callback.
     *
     *  Note that we only call the customer's callback if the socket still
     *  has room for more data.  So if the customer enables the writeReady
     *  callback, then does a blocking or async write, the writeReady callback
     *  does not fire as long as data is being written.  After the write
     *  completes and room is still available in the socket, then the writeReady
     *  callback will be called.
     **************************************************************************/
    if (hSocket->send.writeReadyCallbackState == BIP_SocketCallbackState_eArmed &&
        ! hSocket->socketAtEofOrError) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Checking for WriteReady..." BIP_MSG_PRE_ARG, (void *)hSocket ));

        rc = BIP_Fd_CheckNow(hSocket->socketFd, BIP_IoCheckerEvent_ePollOut);
        if (rc != 0 ) {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: CheckNow: Socket has room! Firing callback!" BIP_MSG_PRE_ARG, (void *)hSocket ));

            BIP_Arb_AddDeferredCallback(hSocket->setSettingsApi.hArb, &hSocket->settings.writeReadyCallback);
            hSocket->send.writeReadyCallbackState = BIP_SocketCallbackState_eTriggered;
        }
    }

    /**************************************************************************
     *  Now see if we need to enable/disable any ioCheckers.  We need an
     *  enabled ioChecker if either:
     *    1) we have a write in progress, or
     *    2) our customer wants a writeReady callback (and it's armed).
     **************************************************************************/
    if (hSocket->send.state == BIP_SocketSendState_eSending  ||
        hSocket->send.writeReadyCallbackState == BIP_SocketCallbackState_eArmed ) {
        if (hSocket->send.hIoChecker && hSocket->send.ioCheckerState != BIP_SocketIoCheckerState_eBusy) {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Enabling ioChecker for ePollOut" BIP_MSG_PRE_ARG, (void *)hSocket ));
            rc = BIP_IoChecker_Enable(hSocket->send.hIoChecker, BIP_IoCheckerEvent_ePollOut);
            BIP_CHECK_GOTO((rc == BIP_SUCCESS), ( "BIP_IoChecker_Enable() failed" ), error, rc, rc );

            hSocket->send.ioCheckerState = BIP_SocketIoCheckerState_eBusy;
        }
    }
    else
    {
        if (hSocket->send.hIoChecker && hSocket->send.ioCheckerState != BIP_SocketIoCheckerState_eIdle) {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Disabling ioChecker for ePollIn" BIP_MSG_PRE_ARG, (void *)hSocket ));
            rc = BIP_IoChecker_Disable(hSocket->send.hIoChecker, BIP_IoCheckerEvent_ePollOut);
            BIP_CHECK_GOTO((rc == BIP_SUCCESS), ( "BIP_IoChecker_Disable() failed" ), error, rc, rc );

            hSocket->send.ioCheckerState = BIP_SocketIoCheckerState_eIdle;
        }
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Returning." BIP_MSG_PRE_ARG, (void *)hSocket ));

error:
    return;
}


/******************************************************************************
 *  The mother of the state processor functions.
 ******************************************************************************/
static void processState(void *hObject, int value, BIP_Arb_ThreadOrigin threadOrigin)
{
    BIP_SocketHandle    hSocket = (BIP_SocketHandle) hObject;
    BIP_ArbHandle       hArb;
    BIP_Status          completionStatus = BIP_SUCCESS;
    BIP_Status          rc = BIP_SUCCESS;

    BSTD_UNUSED(value);

    BDBG_ASSERT(hSocket);
    BDBG_OBJECT_ASSERT(hSocket,BIP_Socket);

    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Entry..." BIP_MSG_PRE_ARG, (void *)hSocket ));

    /**************************************************************************
     *  Take the object lock and keep it locked until we're done.
     **************************************************************************/
    rc = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_Socket, hSocket);
    if (rc != BIP_SUCCESS) { return; }

    /**************************************************************************
     *  Start by handling the Arbs for all of the APIs (except for Send and
     *  Recv).
     **************************************************************************/
    /* BIP_Socket_Destroy: Request for object shutdown */
    if (BIP_Arb_IsNew(hArb = hSocket->shutdownApi.hArb)) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Got Shutdown Arb request!" BIP_MSG_PRE_ARG, (void *)hSocket ));

        if (hSocket->shutdown.state != BIP_SocketShutdownState_eNormal) {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: BIP_Socket is shutting down, rejecting request (BIP_ERR_OBJECT_BEING_DESTROYED) " BIP_MSG_PRE_ARG, (void *)hSocket ));
            BIP_Arb_RejectRequest(hArb, BIP_ERR_OBJECT_BEING_DESTROYED);
        }
        else {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Calling BIP_Arb_AcceptRequest()" BIP_MSG_PRE_ARG, (void *)hSocket ));
            BIP_Arb_AcceptRequest(hArb);

            hSocket->shutdown.state = BIP_SocketShutdownState_eStartShutdown;
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Initiating BIP_Socket destruction..." BIP_MSG_PRE_ARG, (void *)hSocket ));

            completionStatus = BIP_SUCCESS;
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Calling BIP_Arb_CompleteRequest(), completionStatus:0x%X" BIP_MSG_PRE_ARG, (void *)hSocket, completionStatus ));
            BIP_Arb_CompleteRequest(hArb, completionStatus);
        }
    }

    /* BIP_Socket_GetSettings: */
    if (BIP_Arb_IsNew(hArb = hSocket->getSettingsApi.hArb)) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Got GetSettings Arb request!" BIP_MSG_PRE_ARG, (void *)hSocket ));

        if (hSocket->shutdown.state != BIP_SocketShutdownState_eNormal) {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: BIP_Socket is shutting down, rejecting request (BIP_ERR_OBJECT_BEING_DESTROYED) " BIP_MSG_PRE_ARG, (void *)hSocket ));
            BIP_Arb_RejectRequest(hArb, BIP_ERR_OBJECT_BEING_DESTROYED);
        }
        else
        {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Calling BIP_Arb_AcceptRequest()" BIP_MSG_PRE_ARG, (void *)hSocket ));
            BIP_Arb_AcceptRequest(hArb);

            BKNI_Memcpy(hSocket->getSettingsApi.pSettings, &hSocket->settings, sizeof *hSocket->getSettingsApi.pSettings );

            /* All done with this Arb, mark it completed. */
            completionStatus = BIP_SUCCESS;
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Calling BIP_Arb_CompleteRequest(), completionStatus:0x%X" BIP_MSG_PRE_ARG, (void *)hSocket, completionStatus ));
            BIP_Arb_CompleteRequest(hArb, completionStatus);
        }
    }

    /* BIP_Socket_SetSettings: */
    if (BIP_Arb_IsNew(hArb = hSocket->setSettingsApi.hArb)) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Got SetSettings Arb request!" BIP_MSG_PRE_ARG, (void *)hSocket ));

        if (hSocket->shutdown.state != BIP_SocketShutdownState_eNormal) {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: BIP_Socket is shutting down, rejecting request (BIP_ERR_OBJECT_BEING_DESTROYED) " BIP_MSG_PRE_ARG, (void *)hSocket ));
            BIP_Arb_RejectRequest(hArb, BIP_ERR_OBJECT_BEING_DESTROYED);
        }
        else {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Calling BIP_Arb_AcceptRequest()" BIP_MSG_PRE_ARG, (void *)hSocket ));
            BIP_Arb_AcceptRequest(hArb);

            /* See if they enabled the dataReady callback... */
            if ( ! hSocket->settings.dataReadyCallback.callback  &&
                hSocket->setSettingsApi.pSettings->dataReadyCallback.callback) {

                BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Enabling DataReady callbacks to app" BIP_MSG_PRE_ARG, (void *)hSocket ));
                BDBG_ASSERT(hSocket->recv.dataReadyCallbackState == BIP_SocketCallbackState_eDisabled);
                hSocket->recv.dataReadyCallbackState = BIP_SocketCallbackState_eArmed;
            }

            /* See if they disabled the dataReady callback... */
            if (hSocket->settings.dataReadyCallback.callback  &&
                ! hSocket->setSettingsApi.pSettings->dataReadyCallback.callback) {

                BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Disabling DataReady callbacks to app" BIP_MSG_PRE_ARG, (void *)hSocket ));
                BDBG_ASSERT(hSocket->recv.dataReadyCallbackState == BIP_SocketCallbackState_eArmed ||
                            hSocket->recv.dataReadyCallbackState == BIP_SocketCallbackState_eTriggered );
                hSocket->recv.dataReadyCallbackState = BIP_SocketCallbackState_eDisabled;
            }
            hSocket->settings.dataReadyCallback = hSocket->setSettingsApi.pSettings->dataReadyCallback;


            /* See if they enabled the writeReady callback... */
            if ( ! hSocket->settings.writeReadyCallback.callback  &&
                hSocket->setSettingsApi.pSettings->writeReadyCallback.callback) {

                BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Enabling WriteReady callbacks to app" BIP_MSG_PRE_ARG, (void *)hSocket ));
                BDBG_ASSERT(hSocket->send.writeReadyCallbackState == BIP_SocketCallbackState_eDisabled);
                hSocket->send.writeReadyCallbackState = BIP_SocketCallbackState_eArmed;
            }

            /* See if they disabled the writeReady callback... */
            if (hSocket->settings.writeReadyCallback.callback  &&
                ! hSocket->setSettingsApi.pSettings->writeReadyCallback.callback) {

                BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Disabling WriteReady callbacks to app" BIP_MSG_PRE_ARG, (void *)hSocket ));
                BDBG_ASSERT(hSocket->send.writeReadyCallbackState == BIP_SocketCallbackState_eArmed ||
                            hSocket->send.writeReadyCallbackState == BIP_SocketCallbackState_eTriggered );
                hSocket->send.writeReadyCallbackState = BIP_SocketCallbackState_eDisabled;
            }
            hSocket->settings.writeReadyCallback = hSocket->setSettingsApi.pSettings->writeReadyCallback;

            completionStatus = BIP_SUCCESS;
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Calling BIP_Arb_CompleteRequest(), completionStatus:0x%X" BIP_MSG_PRE_ARG, (void *)hSocket, completionStatus ));
            BIP_Arb_CompleteRequest(hArb, completionStatus);
        }
    }

    /**************************************************************************
     *  Call the functions to handle the Send and Recv request processing.
     **************************************************************************/
    processConnectState_locked(hObject, value, threadOrigin);
    processRecvState_locked(hObject, value, threadOrigin);
    processSendState_locked(hObject, value, threadOrigin);

    /**************************************************************************
     *  Now do the object shutdown processing.
     **************************************************************************/
    if (hSocket->shutdown.state == BIP_SocketShutdownState_eStartShutdown) {    /* Beginning destruction. */

        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Beginning BIP_Socket destruction..." BIP_MSG_PRE_ARG, (void *)hSocket ));
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Disabling dataReady and writeReady callbacks..." BIP_MSG_PRE_ARG, (void *)hSocket ));

        hSocket->recv.dataReadyCallbackState = BIP_SocketCallbackState_eDisabled;
        hSocket->send.writeReadyCallbackState = BIP_SocketCallbackState_eDisabled;

        hSocket->shutdown.state = BIP_SocketShutdownState_eFinishingArbs;
    }

    if (hSocket->shutdown.state == BIP_SocketShutdownState_eFinishingArbs) {
        bool    arbsAreStillBusy = false;

        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: BIP_Socket destruction waiting for unfinished Arbs..." BIP_MSG_PRE_ARG, (void *)hSocket ));

        if ( ! BIP_Arb_IsIdle(hSocket->getSettingsApi.hArb)) {
            arbsAreStillBusy = true;
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: BIP_Socket destruction is waiting for getSettingsApi..." BIP_MSG_PRE_ARG, (void *)hSocket ));
        }

        if ( ! BIP_Arb_IsIdle(hSocket->setSettingsApi.hArb)) {
            arbsAreStillBusy = true;
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: BIP_Socket destruction is waiting for setSettingsApi..." BIP_MSG_PRE_ARG, (void *)hSocket ));
        }

        if ( ! BIP_ArbList_IsEmpty(hSocket->hRecvArbList)) {
            arbsAreStillBusy = true;
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: BIP_Socket destruction is waiting for recvApi..." BIP_MSG_PRE_ARG, (void *)hSocket ));
        }

        if ( ! BIP_Arb_IsIdle(hSocket->sendApi.hArb)) {
            arbsAreStillBusy = true;
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: BIP_Socket destruction is waiting for sendApi..." BIP_MSG_PRE_ARG, (void *)hSocket ));
        }

        if ( ! BIP_Arb_IsIdle(hSocket->shutdownApi.hArb)) {
            arbsAreStillBusy = true;
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: BIP_Socket destruction is waiting for shutdownApi..." BIP_MSG_PRE_ARG, (void *)hSocket ));
        }

        if (!arbsAreStillBusy) {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: BIP_Socket Arbs are all Idle..." BIP_MSG_PRE_ARG, (void *)hSocket ));
            hSocket->shutdown.state = BIP_SocketShutdownState_eShutdownDone;
            BIP_Socket_CheckForObjectShutdown(hSocket);
        }
    }

    /**************************************************************************
    *  Unlock the object mutex and do any request completions that are needed.
    *  After unlocking, don't use the socket handle or any socket data
    *  because it might have already been destroyed by another thread!
    **************************************************************************/
    BIP_CLASS_UNLOCK(BIP_Socket, hSocket);
    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Unlocking object Mutex." BIP_MSG_PRE_ARG, (void *)hSocket ));

    rc = BIP_Arb_DoDeferred(NULL, threadOrigin);
    BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "BIP_Arb_DoDeferred() failed" ), error, rc, rc );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Returning." BIP_MSG_PRE_ARG, (void *)hSocket ));
    return;
}


/******************************************************************************
 *  From here down are the public API functions...
 ******************************************************************************/
void BIP_Socket_GetDefaultCreateSettings(
    BIP_SocketCreateSettings *pSettings
    )
{
    BKNI_Memset( pSettings, 0, sizeof( BIP_SocketCreateSettings ));
    pSettings->type        = BIP_SocketType_eTcp;
    pSettings->pPort        = "80";
}

static BIP_SocketHandle createBipSocket(void)
{
    BIP_Status               rc;
    BIP_SocketHandle         hSocket = NULL;

    /* Create the socket object */
    hSocket = B_Os_Calloc(1, sizeof( *hSocket ));
    BIP_CHECK_GOTO(( hSocket !=NULL ), ( "Memory Allocation failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    rc = BIP_CLASS_ADD_INSTANCE(BIP_Socket, hSocket);
    BIP_CHECK_GOTO((rc==BIP_SUCCESS), ( "BIP_CLASS_ADD_INSTANCE failed" ), error, rc, rc );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Allocated " BIP_MSG_PRE_ARG, (void *)hSocket ));

    BDBG_OBJECT_SET( hSocket, BIP_Socket );

    BLST_Q_INIT(&hSocket->recvArbQueueHead);

    hSocket->shutdownApi.hArb = BIP_Arb_Create(NULL, NULL);
    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Created hArb:%p" BIP_MSG_PRE_ARG, (void *)hSocket, (void *)hSocket->shutdownApi.hArb ));

    hSocket->getSettingsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Created hArb:%p" BIP_MSG_PRE_ARG, (void *)hSocket, (void *)hSocket->getSettingsApi.hArb ));

    hSocket->setSettingsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Created hArb:%p" BIP_MSG_PRE_ARG, (void *)hSocket, (void *)hSocket->setSettingsApi.hArb ));

    hSocket->hRecvArbList = BIP_ArbList_Create(NULL);
    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Created hRecvArbList:%p" BIP_MSG_PRE_ARG, (void *)hSocket, (void *)hSocket->hRecvArbList ));

    hSocket->connectApi.hArb = BIP_Arb_Create(NULL, NULL);
    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Created hArb:%p" BIP_MSG_PRE_ARG, (void *)hSocket, (void *)hSocket->connectApi.hArb ));

    hSocket->sendApi.hArb = BIP_Arb_Create(NULL, NULL);
    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Created hArb:%p" BIP_MSG_PRE_ARG, (void *)hSocket, (void *)hSocket->sendApi.hArb ));

    hSocket->hObjectLock = B_Mutex_Create( NULL );
    BIP_CHECK_GOTO(( hSocket->hObjectLock !=NULL ), ( "B_Mutex_Create failed" ), error, BIP_ERR_INTERNAL, rc );
    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Created hObjectLock:%p" BIP_MSG_PRE_ARG, (void *)hSocket, (void *)hSocket->hObjectLock ));

    hSocket->hObjectShutdownEvent = B_Event_Create(NULL);
    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Created hObjectShutdownEvent:%p" BIP_MSG_PRE_ARG, (void *)hSocket, hSocket->hObjectShutdownEvent ));
    BIP_CHECK_GOTO(( hSocket->hObjectShutdownEvent !=NULL ), ( "B_Event_Create failed" ), error, BIP_ERR_INTERNAL, rc );

    return( hSocket );

error:
    if (hSocket) {BIP_Socket_Destroy( hSocket ); }
    return( NULL );
} /* createBipSocket */

BIP_SocketHandle BIP_Socket_CreateFromFd(
    int                             socketFd
    )
{
    BIP_Status               rc;
    BIP_SocketHandle         hSocket = NULL;

    /* Create the socket object */
    hSocket = createBipSocket();
    BIP_CHECK_GOTO(( hSocket !=NULL ), ( "createBipSocket failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    hSocket->socketFd = socketFd;

    rc = configureBipSocket(hSocket);
    BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "configureBipSocket failed" ), error, rc, rc );

    BDBG_MSG((    BIP_MSG_PRE_FMT "Created: " BIP_SOCKET_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_SOCKET_PRINTF_ARG(hSocket)));
    BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Created: " BIP_SOCKET_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_SOCKET_PRINTF_ARG(hSocket)));

    return( hSocket );

error:
    if (hSocket) {BIP_Socket_Destroy( hSocket ); }
    return( NULL );
} /* BIP_Socket_CreateFromFd */


BIP_SocketHandle BIP_Socket_Create(
        const BIP_SocketCreateSettings *pSettings
    )
{
    BIP_Status               rc;
    BIP_SocketHandle         hSocket = NULL;

    /* Create the socket object */
    hSocket = createBipSocket();
    BIP_CHECK_GOTO(( hSocket !=NULL ), ( "createBipSocket failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    if (!pSettings)
    {
        BIP_Socket_GetDefaultCreateSettings(&hSocket->createSettings);
    }
    else
    {
        hSocket->createSettings = *pSettings;
    }

    return (hSocket);
error:
    return NULL;
}

/**
 * Summary:
 * Destroy bip socket
 *
 * Description:
 **/
void BIP_Socket_Destroy(
    BIP_SocketHandle hSocket
    )
{
    BIP_Status              rc = BIP_SUCCESS;
    BIP_ArbSubmitSettings   arbSettings;
    BIP_ArbHandle           hArb;

    BDBG_MSG((    BIP_MSG_PRE_FMT "Destroying socket: " BIP_SOCKET_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_SOCKET_PRINTF_ARG(hSocket)));
    BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Destroying socket: " BIP_SOCKET_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_SOCKET_PRINTF_ARG(hSocket)));

    rc = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_Socket, hSocket);
    BIP_CHECK_GOTO((rc==BIP_SUCCESS), ("BIP_CLASS_LOCK_AND_CHECK_INSTANCE failed."), error, rc, rc);

    BDBG_OBJECT_ASSERT( hSocket, BIP_Socket );

    /* Get the handle for the Shutdown Arb. */
    hArb = hSocket->shutdownApi.hArb;

    /* Tell the Arb that we're starting an API. */
    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Calling BIP_Arb_Acquire() hArb:%p" BIP_MSG_PRE_ARG, (void *)hSocket, (void *)hArb ));
    rc = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((rc == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error_locked, rc, rc );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    /* (No API arguments for the shutdown arb.) */

    /* Now configure the Arb.  Since this is an immediate, synchronous API, we don't need to use a completion callback
     * or a completion event. */
    BIP_Arb_GetDefaultSubmitSettings(&arbSettings);
    arbSettings.hObject           = hSocket;
    arbSettings.arbProcessor      = processState;

    BIP_CLASS_UNLOCK(BIP_Socket, hSocket);

    /* Send the Arb to the state machine. */
    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Calling BIP_Arb_SubmitRequest() hArb:%p" BIP_MSG_PRE_ARG, (void *)hSocket, (void *)hArb ));
    rc = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BDBG_ASSERT(rc == BIP_SUCCESS);

    rc = BIP_CLASS_REMOVE_INSTANCE(BIP_Socket, hSocket);
    if (rc == BIP_ERR_INVALID_HANDLE) return;   /* Object has already been destroyed (by another thread). */

    B_Event_Reset(hSocket->hObjectShutdownEvent);

    BIP_Socket_CheckForObjectShutdown(hSocket);

    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Waiting for Socket to finish up..." BIP_MSG_PRE_ARG, (void *)hSocket ));
    rc = B_Event_Wait(hSocket->hObjectShutdownEvent, B_WAIT_FOREVER);
    BIP_CHECK_GOTO((rc == B_ERROR_SUCCESS), ( "B_Event_Wait failed" ), error, rc, rc );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Socket is finished! It's okay to shut down!  Here we go..." BIP_MSG_PRE_ARG, (void *)hSocket ));

    if (hSocket->socketFd != -1 ) {
        shutdown(hSocket->socketFd, SHUT_RDWR);
        if (hSocket->socketFd != -1) close(hSocket->socketFd);
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Closed socketFd:%d" BIP_MSG_PRE_ARG, (void *)hSocket, hSocket->socketFd ));
        hSocket->socketFd = -1;
    }

    if (hSocket->recv.hIoChecker) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Destroying recv hIoChecker:%p" BIP_MSG_PRE_ARG, (void *)hSocket, (void *)hSocket->recv.hIoChecker ));
        BIP_IoChecker_Destroy(hSocket->recv.hIoChecker);
    }

    if (hSocket->send.hIoChecker) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Destroying send hIoChecker:%p" BIP_MSG_PRE_ARG, (void *)hSocket, (void *)hSocket->send.hIoChecker ));
        BIP_IoChecker_Destroy(hSocket->send.hIoChecker);
    }

    if (hSocket->hObjectShutdownEvent) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Destroying hObjectShutdownEvent:%p" BIP_MSG_PRE_ARG, (void *)hSocket, (void *)hSocket->hObjectShutdownEvent ));
        B_Event_Destroy( hSocket->hObjectShutdownEvent);
    }

    if (hSocket->hObjectLock) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Destroying hObjectLock:%p" BIP_MSG_PRE_ARG, (void *)hSocket, (void *)hSocket->hObjectLock ));
        B_Mutex_Destroy( hSocket->hObjectLock );
    }

    if (hSocket->connectApi.hArb) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Destroying hArb:%p" BIP_MSG_PRE_ARG, (void *)hSocket, (void *)hSocket->connectApi.hArb ));
        BIP_Arb_Destroy(hSocket->connectApi.hArb);
    }

    if (hSocket->sendApi.hArb) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Destroying hArb:%p" BIP_MSG_PRE_ARG, (void *)hSocket, (void *)hSocket->sendApi.hArb ));
        BIP_Arb_Destroy(hSocket->sendApi.hArb);
    }

    if (hSocket->hRecvArbList) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Destroying hRecvArbList:%p" BIP_MSG_PRE_ARG, (void *)hSocket, (void *)hSocket->hRecvArbList ));
        BIP_ArbList_Destroy(hSocket->hRecvArbList);
    }

    if (hSocket->setSettingsApi.hArb) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Destroying hArb:%p" BIP_MSG_PRE_ARG, (void *)hSocket, (void *)hSocket->setSettingsApi.hArb ));
        BIP_Arb_Destroy(hSocket->setSettingsApi.hArb);
    }

    if (hSocket->getSettingsApi.hArb) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Destroying hArb:%p" BIP_MSG_PRE_ARG, (void *)hSocket, (void *)hSocket->getSettingsApi.hArb ));
        BIP_Arb_Destroy(hSocket->getSettingsApi.hArb);
    }

    if (hSocket->shutdownApi.hArb) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Destroying hArb:%p" BIP_MSG_PRE_ARG, (void *)hSocket, (void *)hSocket->shutdownApi.hArb ));
        BIP_Arb_Destroy(hSocket->shutdownApi.hArb);
    }

    BDBG_ASSERT( BLST_Q_EMPTY( &hSocket->recvArbQueueHead));

    BDBG_OBJECT_DESTROY( hSocket, BIP_Socket );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Freeing object memory" BIP_MSG_PRE_ARG, (void *)hSocket ));
    B_Os_Free( hSocket );

    return;

error_locked:
    BIP_CLASS_UNLOCK(BIP_Socket, hSocket);
error:
    return;

} /* BIP_Socket_Destroy */


void BIP_Socket_GetSettings(
    BIP_SocketHandle    hSocket,
    BIP_SocketSettings *pSettings
    )
{
    BIP_Status              rc = BIP_SUCCESS;
    BIP_ArbSubmitSettings   arbSettings;
    BIP_ArbHandle           hArb;

    BDBG_OBJECT_ASSERT( hSocket, BIP_Socket );
    BDBG_ASSERT( pSettings );

    rc = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_Socket, hSocket);
    BIP_CHECK_GOTO((rc==BIP_SUCCESS), ("BIP_CLASS_LOCK_AND_CHECK_INSTANCE failed."), error, rc, rc);

    /* Get the handle for the SetSettings Arb. */
    hArb = hSocket->getSettingsApi.hArb;

    /* Tell the Arb that we're starting an API. */
    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Calling BIP_Arb_Acquire() hArb:%p" BIP_MSG_PRE_ARG, (void *)hSocket, (void *)hArb ));
    rc = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((rc == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error_locked, rc, rc );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hSocket->getSettingsApi.pSettings = pSettings;

    /* Now configure the Arb.  Since this is an immediate, synchronous API, we don't need to use a completion callback
     * or a completion event. */
    BIP_Arb_GetDefaultSubmitSettings(&arbSettings);
    arbSettings.hObject           = hSocket;
    arbSettings.arbProcessor      = processState;

    BIP_CLASS_UNLOCK(BIP_Socket, hSocket);

    /* Send the Arb to the state machine. */
    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Calling BIP_Arb_SubmitRequest() hArb:%p" BIP_MSG_PRE_ARG, (void *)hSocket, (void *)hArb ));
    rc = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BDBG_ASSERT(rc == BIP_SUCCESS);
    return;

error_locked:
    BIP_CLASS_UNLOCK(BIP_Socket, hSocket);
error:
    BDBG_ASSERT(rc == BIP_SUCCESS);
    return;
}


BIP_Status BIP_Socket_SetSettings(
    BIP_SocketHandle    hSocket,
    BIP_SocketSettings *pSettings
    )
{
    BIP_Status               rc = BIP_SUCCESS;
    BIP_ArbSubmitSettings   arbSettings;
    BIP_ArbHandle           hArb;

    BDBG_OBJECT_ASSERT( hSocket, BIP_Socket );
    BDBG_ASSERT( pSettings );

    rc = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_Socket, hSocket);
    BIP_CHECK_GOTO((rc==BIP_SUCCESS), ("BIP_CLASS_LOCK_AND_CHECK_INSTANCE failed."), error, rc, rc);

    /* Get the handle for the SetSettings Arb. */
    hArb = hSocket->setSettingsApi.hArb;

    /* Tell the Arb that we're starting an API. */
    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Calling BIP_Arb_Acquire() hArb:%p" BIP_MSG_PRE_ARG, (void *)hSocket, (void *)hArb ));
    rc = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((rc == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error_locked, rc, rc );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hSocket->setSettingsApi.pSettings = pSettings;

    /* Now configure the Arb.  Since this is an immediate, synchronous API, we don't need to use a completion callback
     * or a completion event. */
    BIP_Arb_GetDefaultSubmitSettings(&arbSettings);
    arbSettings.hObject           = hSocket;
    arbSettings.arbProcessor      = processState;

    BIP_CLASS_UNLOCK(BIP_Socket, hSocket);

    /* Send the Arb to the state machine. */
    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Calling BIP_Arb_SubmitRequest() hArb:%p" BIP_MSG_PRE_ARG, (void *)hSocket, (void *)hArb ));
    rc = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    return rc;

error_locked:
    BIP_CLASS_UNLOCK(BIP_Socket, hSocket);
error:
    return(rc);
} /* BIP_Socket_SetSettings */

void BIP_Socket_GetDefaultConnectSettings( BIP_SocketConnectSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(BIP_SocketConnectSettings));
    pSettings->api.timeout = -1;   /* blocking mode */
}

BIP_Status BIP_Socket_Connect(
    BIP_SocketHandle    hSocket,
    BIP_SocketConnectSettings *pSettings
    )
{
    BIP_Status              rc = BIP_SUCCESS;
    BIP_ArbSubmitSettings   arbSettings;
    BIP_ArbHandle           hArb;

    BDBG_OBJECT_ASSERT( hSocket, BIP_Socket );
    BDBG_ASSERT( pSettings );

    rc = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_Socket, hSocket);
    BIP_CHECK_GOTO((rc==BIP_SUCCESS), ("BIP_CLASS_LOCK_AND_CHECK_INSTANCE failed."), error, rc, rc);

    /* Get the handle for the SetSettings Arb. */
    hArb = hSocket->connectApi.hArb;

    /* Tell the Arb that we're starting an API. */
    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Calling BIP_Arb_Acquire() hArb:%p" BIP_MSG_PRE_ARG, (void *)hSocket, (void *)hArb ));
    rc = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((rc == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error_locked, rc, rc );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hSocket->connectApi.settings = *pSettings;

    /* Now configure the Arb.  Since this is an immediate, synchronous API, we don't need to use a completion callback
     * or a completion event. */
    BIP_Arb_GetDefaultSubmitSettings(&arbSettings);
    arbSettings.hObject           = hSocket;
    arbSettings.arbProcessor      = processState;

    BIP_CLASS_UNLOCK(BIP_Socket, hSocket);

    /* Send the Arb to the state machine. */
    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Calling BIP_Arb_SubmitRequest() hArb:%p" BIP_MSG_PRE_ARG, (void *)hSocket, (void *)hArb ));
    rc = BIP_Arb_Submit(hArb, &arbSettings, &pSettings->api);
    return rc;

error_locked:
    BIP_CLASS_UNLOCK(BIP_Socket, hSocket);
error:
    return(rc);
} /* BIP_Socket_Connect */


void BIP_Socket_GetDefaultSocketRecvSettings(
   BIP_SocketRecvSettings  *pSocketRecvSettings
   )
{
    BKNI_Memset(pSocketRecvSettings, 0, sizeof(BIP_SocketRecvSettings));
    pSocketRecvSettings->api.timeout = 0;   /* Non-blocking mode */
}



BIP_Status BIP_Socket_Recv(
    BIP_SocketHandle          hSocket,
    BIP_SocketRecvSettings   *pSocketRecvSettings
    )
{
    BIP_Status               rc = BIP_SUCCESS;
    BIP_ArbSubmitSettings   arbSettings;
    BIP_SocketRecvArb      *pAppArb;
    BIP_ArbHandle           hArb;
    BIP_ArbListHandle       hArbList = hSocket->hRecvArbList;

    BDBG_OBJECT_ASSERT( hSocket, BIP_Socket );
    BDBG_ASSERT( pSocketRecvSettings );

    rc = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_Socket, hSocket);
    BIP_CHECK_GOTO((rc==BIP_SUCCESS), ("BIP_CLASS_LOCK_AND_CHECK_INSTANCE failed."), error, rc, rc);

    pAppArb =  BIP_ARB_CREATE_APPARB( BIP_SocketRecvArb );
    BIP_CHECK_GOTO(( pAppArb !=NULL ), ( "Memory Allocation failed" ), error_locked, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    BDBG_OBJECT_SET( pAppArb, BIP_SocketRecvArb );

    hArb = BIP_ARB_FROM_APPARB(pAppArb);
    BDBG_OBJECT_ASSERT( hArb, BIP_Arb );

    /* Tell the Arb that we're starting an API. */
    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Calling BIP_Arb_Acquire() hArb:%p" BIP_MSG_PRE_ARG, (void *)hSocket, (void *)hArb));
    rc = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((rc == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error_locked, rc, rc );

    /* Move the API arguments into the object so the state machine can find them. */
    pAppArb->settings = *pSocketRecvSettings;

    /* Now configure the Arb. */
    BIP_Arb_GetDefaultSubmitSettings(&arbSettings);
    arbSettings.hObject           = hSocket;            /* Object handle. */
    arbSettings.arbProcessor      = processState;       /* Function to be called to process the Arb. */
    arbSettings.waitIfBusy        = false;              /* Should simultaneous calls to this API be serialized. */

    BIP_CLASS_UNLOCK(BIP_Socket, hSocket);

    /* Send the Arb to the state machine. */
    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Calling BIP_ArbList_SubmitRequest() hArbList:%p pAppArb:%p" BIP_MSG_PRE_ARG, (void *)hSocket, (void *)hArbList, (void *)pAppArb ));
    rc = BIP_ArbList_Submit(hArbList, hArb, &arbSettings, &pSocketRecvSettings->api);
    return rc;

error_locked:
    BIP_CLASS_UNLOCK(BIP_Socket, hSocket);
error:
    return rc;

} /* BIP_Socket_Recv */


void BIP_Socket_GetDefaultSocketSendSettings(
   BIP_SocketSendSettings  *pSocketSendSettings
   )
{
    BKNI_Memset(pSocketSendSettings, 0, sizeof(BIP_SocketSendSettings));
    pSocketSendSettings->api.timeout = 0;   /* Non-blocking mode */
}


BIP_Status BIP_Socket_Send(
    BIP_SocketHandle          hSocket,
    BIP_SocketSendSettings   *pSocketSendSettings
    )
{
    BDBG_ASSERT( hSocket );

    BIP_Status               rc = BIP_SUCCESS;
    BIP_ArbSubmitSettings   arbSettings;
    BIP_ArbHandle           hArb;

    BDBG_OBJECT_ASSERT( hSocket, BIP_Socket );
    BDBG_ASSERT( pSocketSendSettings );

    rc = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_Socket, hSocket);
    BIP_CHECK_GOTO((rc==BIP_SUCCESS), ("BIP_CLASS_LOCK_AND_CHECK_INSTANCE failed."), error, rc, rc);

    /* Get the handle for the SetSettings Arb. */
    hArb = hSocket->sendApi.hArb;

    /* Tell the Arb that we're starting an API. */
    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Calling BIP_Arb_Acquire() hArb:%p" BIP_MSG_PRE_ARG, (void *)hSocket, (void *)hArb ));
    rc = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((rc == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error_locked, rc, rc );

    /* Move the API arguments into the object so the state machine can find them. */
    hSocket->sendApi.settings = *pSocketSendSettings ;

    /* Now configure the Arb. */
    BIP_Arb_GetDefaultSubmitSettings(&arbSettings);
    arbSettings.hObject           = hSocket;            /* Object handle. */
    arbSettings.arbProcessor      = processState;       /* Function to be called to process the Arb. */
    arbSettings.waitIfBusy        = false;              /* Should simultaneous calls to this API be serialized. */

    BIP_CLASS_UNLOCK(BIP_Socket, hSocket);

    /* Send the Arb to the state machine. */
    BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Calling BIP_Arb_SubmitRequest() hArb:%p" BIP_MSG_PRE_ARG, (void *)hSocket, (void *)hArb ));
    rc = BIP_Arb_Submit(hArb, &arbSettings, &pSocketSendSettings->api);
    return rc;

error_locked:
    BIP_CLASS_UNLOCK(BIP_Socket, hSocket);
error:
    return rc;

} /* BIP_Socket_Send */


BIP_Status BIP_Socket_GetStatus(
    BIP_SocketHandle  hSocket,
    BIP_SocketStatus *pStatus
    )
{
    BIP_Status  rc;
    if (( hSocket == NULL ) || ( pStatus == NULL ))
    {
        return( BIP_ERR_INVALID_PARAMETER );
    }
    BDBG_OBJECT_ASSERT( hSocket, BIP_Socket );

    rc = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_Socket, hSocket);
    BIP_CHECK_GOTO((rc==BIP_SUCCESS), ("BIP_CLASS_LOCK_AND_CHECK_INSTANCE failed."), error, rc, rc);

    pStatus->socketFd = hSocket->socketFd;
    pStatus->pRemoteIpAddress = hSocket->remoteIpAddress;
    pStatus->pLocalIpAddress = hSocket->localIpAddress;
    BIP_CLASS_UNLOCK(BIP_Socket, hSocket);

    return( BIP_SUCCESS );

error:
    return(rc);
}
