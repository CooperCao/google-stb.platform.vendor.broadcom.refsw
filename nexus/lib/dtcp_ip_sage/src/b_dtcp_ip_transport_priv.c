/********************************************************************************************
*     (c)2004-2015 Broadcom Corporation                                                     *
*                                                                                           *
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,   *
*  and may only be used, duplicated, modified or distributed pursuant to the terms and      *
*  conditions of a separate, written license agreement executed between you and Broadcom    *
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants*
*  no license (express or implied), right to use, or waiver of any kind with respect to the *
*  Software, and Broadcom expressly reserves all rights in and to the Software and all      *
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU       *
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY                    *
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.                                 *
*
*  Except as expressly set forth in the Authorized License,                                 *
*
*  1.     This program, including its structure, sequence and organization, constitutes     *
*  the valuable trade secrets of Broadcom, and you shall use all reasonable efforts to      *
*  protect the confidentiality thereof,and to use this information only in connection       *
*  with your use of Broadcom integrated circuit products.                                   *
*                                                                                           *
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"          *
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR                   *
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO            *
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES            *
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,            *
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION             *
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF              *
*  USE OR PERFORMANCE OF THE SOFTWARE.                                                      *
*                                                                                           *
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS         *
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR             *
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR               *
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF             *
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT              *
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE            *
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF              *
*  ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *     DTCP-IP AKE connection handler.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
*********************************************************************************************/
/*! \file b_dtcp_ake_msg.c
 *  \brief implement DTCP-IP connection handler functions.
 */
#include <stdio.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bdbg.h"
#include "b_dtcp_applib.h"
#include "b_dtcp_ip_ake.h"
#include "b_dtcp_status_codes.h"
#include "b_dtcp_transport.h"
#include "b_dtcp_ip_transport_priv.h"
#include "b_dtcp_stack.h"
#include "b_dtcp_ip_stack.h"

BDBG_MODULE(b_dtcp_ip);

#define DTCP_READER_TIMEOUT        1        /*!< This refers to "select()" timeout value */

#define WAIT_FOR_READER_THREAD_TIMEOUT_MSEC 10 /* used to wait for reader thread to exit */
#define WAIT_FOR_READER_THREAD_TIMEOUT_MSEC_MAX 3000

static void B_DTCP_IP_DestroyConnection(B_DTCP_IP_Connection_T * Conn);

struct __b_connection_thread_param
{
    B_DTCP_IP_Connection_T * Conn;
    B_DTCP_IP_TransportData_T pProtocol;
};
/*! \brief utility to get the event type.
 *
 * \param[in] Conn connection context pointer.
 * \retval one of the enum values defined in B_TransportEvent_T
 */
B_TransportEvent_T B_DTCP_IP_GetEventType_Ex(B_DTCP_IP_Connection_T * Conn)
{
    B_TransportEvent_T Event;
    B_Mutex_Lock(Conn->hMutex);
    Event = Conn->EventType;
    B_Mutex_Unlock(Conn->hMutex);

    return Event;
}
/*! \brief utility to get data size available to read.
 *
 * \param[in] Conn connection context pointer.
 * \retval data size available for read.
 */
int B_DTCP_IP_GetDataSize_Ex(B_DTCP_IP_Connection_T * Conn)
{
    int size;
    B_Mutex_Lock(Conn->hMutex);
    size = Conn->DataSize;
    B_Mutex_Unlock(Conn->hMutex);
    return size;
}
/*! \brief a reader thread function.
 *  \param[in] UserData user data pointer, points to a connection context.
 *
 *  When the connection is initialized and established, both the server and
 *  the client will spawn this reader thread, the main function of this thread
 *  is to wait for data ready to be read, set the event to wake up other
 *  thread who is waiting.
 */
static void B_DTCP_IP_ReaderThreadFunc(void * UserData)
{
    B_DTCP_IP_Connection_T * Conn = UserData;
    BERR_Code retValue = BERR_SUCCESS;
    BDBG_ASSERT(UserData);

    Conn->readerThreadDone = false;

    BDBG_MSG(("Starting reader thread.... sfd %d", Conn->ConnectSocket));
    retValue =  B_SocketWaitForData(
            Conn->ConnectSocket,
            &Conn->EventType,
            Conn->hNetEvent,
            Conn->hBufCleared,
            Conn->hMutex,
            &Conn->DataSize,
            &Conn->socketClosed,
            DTCP_READER_TIMEOUT
    );

    Conn->readerThreadDone = true;

    #if 0
    if (Conn->Mode == B_ConnectionMode_eClient &&
            (Conn->EventType == B_Transport_eClosed ||  Conn->EventType == B_Transport_eError ))
    {
        B_DTCP_IP_TransportData_T *pProtocol = (B_DTCP_IP_TransportData_T *)Conn->reserved;

        B_DTCP_IP_CloseConnection(Conn, pProtocol);

        if ((retValue = B_SocketClose(Conn->ConnectSocket)) != BERR_SUCCESS)
        {
            BDBG_WRN(("Unable to shutdown socket for %s: %d ", Conn->RemoteAddr, retValue));
        }
        Conn->ConnectSocket = INVALID_SOCKET;

        B_DTCP_IP_DestroyConnection(Conn);
    }
    #endif

    BDBG_MSG(("Reader Thread Exiting sfd %d", Conn->ConnectSocket));
    B_Event_Set(Conn->hNetEvent);
    B_Event_Set(Conn->hReaderTerminated);
}

/*! \brief helper function to spwan connection thread and call user supplied new connection function.
 *  \param[in] UserData user data pointer, points to a connection context.
 *  \retval none.
 */
static void B_DTCP_IP_ConnThreadFunc(void * UserData)
{
    B_DTCP_IP_Connection_T * Conn = (B_DTCP_IP_Connection_T *)UserData;
    B_DTCP_IP_TransportData_T * pProtocol = (B_DTCP_IP_TransportData_T *)(Conn->reserved);
    struct __dtcp_ip_stack_data sdata;
    int retValue = BERR_SUCCESS;

    BDBG_ASSERT(Conn);
    BDBG_ASSERT(pProtocol);
    BDBG_ENTER(B_DTCP_IP_ConnThreadFunc);

    BKNI_Memcpy(sdata.RemoteIp, Conn->RemoteAddr, MAX_IP_ADDR_SIZE);
    BKNI_Memcpy(sdata.LocalIp, Conn->LocalAddr, MAX_IP_ADDR_SIZE);
    sdata.LocalPort = Conn->Port;
    sdata.RemotePort = 0; /*TODO */
    sdata.ConnectSocket = Conn->ConnectSocket;
    BDBG_MSG(("Starting new connection thread...."));
    /*
     * call the stack's new connection callback, pass in the socket id
     * we use this as AKE session id.
     */
    retValue = Conn->OnNewConnection(pProtocol->pStack, (void*)&sdata);
    BDBG_MSG(("Callback terminated with : %d\n", retValue));

    /*
     * If stack layer returned us a error code, and it's not a socket error,
     * it means AKE failed so we close connection before terminating this thread.
    */
    if (pProtocol->masterShutDown == false)
    {
        BDBG_MSG(("Connection thread terminated with %d\n", retValue));
        B_Event_Set(Conn->hConnTerminated);
        retValue = B_DTCP_IP_CloseConnection( Conn, pProtocol );
        if(retValue != BERR_SUCCESS)
        {
            BDBG_ERR(("returned %d when trying to close connection\n"));
        }
        Conn->connectionThreadDone = true;
        BDBG_MSG(("Connection thread marked for destroy\n"));
    }

    BDBG_LEAVE(B_DTCP_IP_ConnThreadFunc);
    return ;
}
/*! \brief create and initialize a connection context.
 *  \param[in] RemoteAddr can be NULL if it's server context.
 *  \param[in] Port port number.
 *  \param[in] Mode connection mode, server/client.
 *  \param[in] ReaderTimeOut Timeout for reader thread.
 *  \param[in] ConnClosedcallback pointer to connection close callback.
 *  \param[in] OnNewConncallback pointer to connection close callback.
 *  \param[in] IsMaster a flag indicate if this is a master connection (server only).
 *  \retval pointer to created connection context or NULL if it failed.
 */
static B_DTCP_IP_Connection_T * B_DTCP_IP_CreateConnection(
        char * RemoteAddr,
        int Port,
        B_ConnectionMode_T Mode,
        int ReaderTimeOut,
        B_DTCP_TransportCallbackFunc_Ptr OnNewConnCallback,
        B_DTCP_TransportCallbackFunc_Ptr OnClosedConnCallback,
        bool IsMaster)
{
    B_DTCP_IP_Connection_T * Conn ;

    BDBG_ENTER(B_DTCP_IP_CreateConnection);

    Conn = (B_DTCP_IP_Connection_T *)BKNI_Malloc(sizeof(B_DTCP_IP_Connection_T));

    if (Conn == NULL) {
        BDBG_ERR(("failed to allocate memory for new connection context"));
        return NULL;
    }
    BKNI_Memset(Conn, 0, sizeof(B_DTCP_IP_Connection_T));
    if(IsMaster == false)
        BKNI_Memcpy(Conn->RemoteAddr, RemoteAddr, 32);
    Conn->Port = Port;
    Conn->Mode = Mode;
    Conn->IsMaster = IsMaster;
    Conn->ListenerSocket = INVALID_SOCKET;
    Conn->ConnectSocket = INVALID_SOCKET;
    /*Conn->ReaderTimeout = DTCP_READER_TIMEOUT; */
    (void)ReaderTimeOut;
    Conn->bufRead = true;
    Conn->OnNewConnection = OnNewConnCallback;
    Conn->OnClosedConnection = OnClosedConnCallback;
    Conn->readerThreadDone = false;
    Conn->connectionThreadDone = false;
    Conn->socketClosed = false;
    if((Conn->hNetEvent = B_Event_Create(NULL)) == NULL)
    {
        BDBG_ERR(("Failed to create NetEvent"));
        goto err4;
    }
    if((Conn->hConnTerminated = B_Event_Create(NULL)) == NULL)
    {
        BDBG_ERR(("Failed to create NetEvent"));
        goto err3;
    }
    if((Conn->hReaderTerminated = B_Event_Create(NULL)) == NULL)
    {
        BDBG_ERR(("Failed to create hReaderTerminated"));
        goto err3;
    }

    if((Conn->hBufCleared = B_Event_Create(NULL)) == NULL)
    {
        BDBG_ERR(("Failed to create NetEvent"));
        goto err2;
    }
    if((Conn->hMutex = B_Mutex_Create(NULL)) == NULL)
    {
        BDBG_ERR(("Failed to create mutex for new connection"));
        goto err1;
    }

    BDBG_LEAVE(B_DTCP_IP_CreateConnection);
    return Conn;

err1:    /*B_Event_Destroy(Conn->hBufCleared); */
err2:
    B_Event_Destroy(Conn->hConnTerminated);
    B_Event_Destroy(Conn->hReaderTerminated);
err3:    B_Event_Destroy(Conn->hNetEvent);
err4:    BKNI_Free(Conn);
    return NULL;
}
/*! \brief destroy a connection context, release resource.
 *  \param[in] Conn connection context pointer to be destroyed.
 *
 *  For server connection, make sure connection thread is terminated before calling this!
 *  For client connection, make sure reader thread is terminited before calling this!
 */
static void B_DTCP_IP_DestroyConnection(B_DTCP_IP_Connection_T * Conn)
{
    if(Conn)
    {
        if(Conn->IsMaster && Conn->ListenerSocket != INVALID_SOCKET)
        {
            BDBG_WRN(("ListenerSocket %d is not closed when trying to destroy connection", Conn->ListenerSocket));
        }else if (Conn->ConnectSocket != INVALID_SOCKET) {
            BDBG_WRN(("ConnectSocket %d is not closed when trying to destroy connection", Conn->ConnectSocket));
        }

        B_Event_Destroy(Conn->hConnTerminated);
        B_Event_Destroy(Conn->hReaderTerminated);
        B_Mutex_Destroy(Conn->hMutex);
        B_Event_Destroy(Conn->hNetEvent);
        B_Event_Destroy(Conn->hBufCleared);
        BKNI_Free(Conn);
        BDBG_MSG(("****CONNECTION DESTROYED****"));
    }
}


/*! \brief Wait for connection request, launch user supplied reader thread.
 *  \param[in] master a master connection context pointer.
 */
BERR_Code B_WaitForConnection(B_DTCP_IP_Connection_T * master, B_DTCP_IP_TransportData_T * pProtocol)
{
    B_DTCP_IP_Connection_T * Conn;

    /* For garbage collection */
    B_DTCP_IP_Connection_T * iter, *tmp;

    int ConnectSocket;
    char RemoteAddr[32];
    char ThreadName[64];
    B_Error retCode = B_ERROR_SUCCESS;

    BDBG_ENTER(B_WaitForConnection);
    BDBG_ASSERT(master);
    BDBG_ASSERT(pProtocol);

    /* the loop will exit when listener socket is closed. */
    while(true)
    {
        BDBG_MSG(("Waiting for connection...\n"));
        if(B_SocketListen(master->ListenerSocket) != BERR_SUCCESS)
            return BERR_SOCKET_ERROR;
        /* This will block untill remote initiate a connection request.*/
        if((ConnectSocket = B_SocketAccept(master->ListenerSocket, RemoteAddr, 32)) == INVALID_SOCKET)
        {
            BDBG_ERR(("Error when accepting connection\n"));
            return BERR_SOCKET_ERROR;
        }
        BDBG_MSG(("Accepted connection request from %s sfd=%d", RemoteAddr, ConnectSocket));
        Conn = B_DTCP_IP_CreateConnection(
                RemoteAddr,
                master->Port,
                B_ConnectionMode_eServer,
                DTCP_READER_TIMEOUT,
                master->OnNewConnection,            /* OnNewConnCallback */
                master->OnClosedConnection,
                false                                /* IsMaster */
                );
        if(Conn == NULL) {
            BDBG_ERR(("Failed to create connection for request from %s\n", RemoteAddr));
            B_SocketClose(ConnectSocket);
            return BERR_UNKNOWN;
        }
        Conn->ConnectSocket = ConnectSocket;
        Conn->reserved = (void*)pProtocol;
        /*
         * Start a reader thread, the thread function is a internal function, the main
         * purpose is to wait for data, and set the event to wake up other waiting thread.
         * (AKE session thread).
         */
        snprintf(ThreadName, 64, "Reader:%s", Conn->RemoteAddr);
        B_Event_Reset(Conn->hNetEvent);
        B_Event_Reset(Conn->hReaderTerminated);
        Conn->hReaderThread = B_Thread_Create(
                ThreadName,
                B_DTCP_IP_ReaderThreadFunc,
                (void*)Conn,
                NULL);
        if(Conn->hReaderThread == NULL)
        {
            BDBG_ERR(("Failed to create reader thread"));
            B_SocketClose(Conn->ConnectSocket);
            B_DTCP_IP_DestroyConnection(Conn);
            return BERR_OS_ERROR;
        }else {
            /* Wait for reader thread entering wait for data loop */
            B_Event_Wait(Conn->hNetEvent, 3000);
        }

        B_Mutex_Lock(pProtocol->ConnectionChgMutex);
        BLST_S_INSERT_HEAD(&(pProtocol->Connection_list), Conn, node);
        B_Mutex_Unlock(pProtocol->ConnectionChgMutex);

        /*
         * Start a new connection thread, the thread function should be implmented by
         * DTCP AKE component, the main function is to allocate an AKE session and
         * waiting for message from sink device.
         */
        snprintf(ThreadName, 64, "Connection:%s", Conn->RemoteAddr);
        Conn->hConnectionThread = B_Thread_Create(
                ThreadName,
                B_DTCP_IP_ConnThreadFunc,
                (void*)Conn,
                NULL);
        if(Conn->hConnectionThread == NULL) {
            BDBG_ERR(("Failed to create connection thread"));
            B_Event_Reset(Conn->hNetEvent);
            B_SocketClose(Conn->ConnectSocket);
            /* wait for reader thread to finish before we destroy it */
            if (Conn->readerThreadDone == false) {
                retCode = B_Event_Wait(Conn->hNetEvent, 3000); /* wait for 3 seconds */
                if (retCode == B_ERROR_TIMEOUT) {
                    BDBG_ERR(("hnet event timeout"));
                }
            }

            B_Thread_Destroy(Conn->hReaderThread);
            Conn->ConnectSocket = INVALID_SOCKET;

            B_Mutex_Lock(pProtocol->ConnectionChgMutex);
            BLST_S_REMOVE(&(pProtocol->Connection_list), Conn, B_DTCP_IP_Connection, node);
            B_Mutex_Unlock(pProtocol->ConnectionChgMutex);
            B_DTCP_IP_DestroyConnection(Conn);
            return BERR_OS_ERROR;
        }

        /*Run Garbage collection */
        BDBG_MSG(("Garbage Collection !!"));
        B_Mutex_Lock(pProtocol->ConnectionChgMutex);
        iter = BLST_S_FIRST(&(pProtocol->Connection_list));
        B_Mutex_Unlock(pProtocol->ConnectionChgMutex);
        while(iter != NULL){
            BDBG_MSG(("Iter Not NULL"));
            tmp = iter;
            B_Mutex_Lock(pProtocol->ConnectionChgMutex);
            iter = BLST_S_NEXT(iter, node);
            B_Mutex_Unlock(pProtocol->ConnectionChgMutex);

            if (tmp->connectionThreadDone == true)
            {
                BDBG_MSG(("Destroying stale connection threads"));
                B_Thread_Destroy(tmp->hConnectionThread);
                B_Mutex_Lock(pProtocol->ConnectionChgMutex);
                BLST_S_REMOVE(&(pProtocol->Connection_list), tmp, B_DTCP_IP_Connection, node);
                B_Mutex_Unlock(pProtocol->ConnectionChgMutex);

                B_DTCP_IP_DestroyConnection(tmp);
            }
        }
    }

    BDBG_LEAVE(B_WaitForConnection);
    return BERR_SUCCESS;
}

/*! \brief Open a DTCP-IP connection,
 *  For source device, this function will block , waiting for client connection.
 *  For sink devie, this function will return , the pProtocol->MasterConn points to the created connection context.
 * \param[in] Mode server or client mode.
 * \param[in] LocalAddr Local IP address, optional
 * \param[in] RemoteAddr Remote IP address.
 * \param[in] Port TCP port number.
 * \param[in] OnNewConnCallback callback function when there is new connection, only required for server mode.
 * \param[in] OnCloseConneCallback callback function when connection is closed by transport layer.
 * \param[in, out] pProtocol pointer to IP transport protocol specified data.
 * \param[out] ConnectionId, if it's a client connection, ConnectionId is the socket fd.
 */
BERR_Code B_DTCP_IP_OpenConnection(B_ConnectionMode_T Mode,
        char * LocalAddr,
        char * RemoteAddr,
        int Port,
        B_DTCP_TransportCallbackFunc_Ptr OnNewConnCallback,
        B_DTCP_TransportCallbackFunc_Ptr OnClosedConnCallback,
        B_DTCP_IP_TransportData_T * pProtocol,
        int * ConnectionId)
{
    int sfd = INVALID_SOCKET;
    BERR_Code retValue = BERR_SUCCESS;
    B_DTCP_IP_Connection_T * Conn = NULL;
    char ThreadName[32];

    BDBG_ENTER(B_DTCP_IP_OpenConnection);
    BDBG_ASSERT(pProtocol);

    if(Mode == B_ConnectionMode_eServer)
    {
        BDBG_ASSERT(OnNewConnCallback);
        BDBG_MSG(("Openning server connection..."));

        sfd = B_SocketOpen();
        if (sfd == INVALID_SOCKET) {
            BDBG_ERR(("Failed to open socket!"));
            return BERR_SOCKET_ERROR;
        }

        /* Create a global master connection context.*/
        Conn = B_DTCP_IP_CreateConnection(
                NULL,                        /* Remote Address. */
                Port,                        /* TCP port # */
                B_ConnectionMode_eServer,    /* Connection Mode. */
                DTCP_READER_TIMEOUT,        /* Reader timeout in seconds */
                OnNewConnCallback,            /* new connection callback.*/
                OnClosedConnCallback,        /* Close connection callbac */
                true);                        /* Is master */
        if(Conn == NULL)
        {
            BDBG_ERR(("Failed to create master connection"));
            B_SocketClose(sfd);
            return BERR_UNKNOWN;
        }
        Conn->ListenerSocket = sfd;

        if (LocalAddr)
        {
            BKNI_Memcpy(Conn->LocalAddr, LocalAddr, 32);
        }
        else
        {
            BKNI_Memset(Conn->LocalAddr, 0, 32);
        }
        retValue = B_SocketBind(Conn->ListenerSocket, Conn->LocalAddr, Conn->Port);
        if(retValue != BERR_SUCCESS)
        {
            BDBG_ERR(("listening on %s:%d failed, ret %d \n", LocalAddr, Conn->Port, retValue));
            perror("B_SocketBind()");
            B_SocketClose(sfd);
            Conn->ListenerSocket = INVALID_SOCKET;
            B_DTCP_IP_DestroyConnection(Conn);
            return BERR_SOCKET_ERROR;
        }
        pProtocol->MasterConn = Conn;
        pProtocol->masterShutDown = false;
    }else if (Mode == B_ConnectionMode_eClient)
    {
        BDBG_MSG(("Openning client connection...\n"));
        if((sfd = B_SocketOpen()) == INVALID_SOCKET)
        {
            BDBG_ERR(("Failed to open socket!"));
            return BERR_SOCKET_ERROR;
        }
        Conn = B_DTCP_IP_CreateConnection(
                RemoteAddr,
                Port,
                B_ConnectionMode_eClient,
                DTCP_READER_TIMEOUT,
                NULL,
                OnClosedConnCallback,
                false);
        if(Conn == NULL)
        {
            BDBG_ERR(("Failed to create master connection"));
            B_SocketClose(sfd);
            return BERR_UNKNOWN;
        }
        Conn->ConnectSocket = sfd;
        Conn->reserved = (void*)pProtocol;
        *ConnectionId = sfd;
        BDBG_MSG(("sfd=%d\n", sfd));
        if((retValue = B_SocketConnect(sfd,RemoteAddr, Port)) != BERR_SUCCESS)
        {
            BDBG_ERR(("Failed to connect to %s:%d\n", RemoteAddr, Port));
            B_SocketClose(sfd);
            Conn->ConnectSocket = INVALID_SOCKET;
            B_DTCP_IP_DestroyConnection(Conn);
            return retValue;
        }
        /*
         * Start a reader thread, the thread function is a internal function, the main
         * purpose is to wait for data, and set the event to wake up other waiting thread.
         * (AKE session thread).
         */
        snprintf(ThreadName, 32, "Reader:%s", RemoteAddr);

        B_Event_Reset(Conn->hReaderTerminated);
        Conn->hReaderThread = B_Thread_Create(
                ThreadName,
                B_DTCP_IP_ReaderThreadFunc,
                (void*)Conn,
                NULL);
        if(Conn->hReaderThread == NULL)
        {
            BDBG_ERR(("Failed to create reader thread"));
            B_SocketClose(sfd);
            B_DTCP_IP_DestroyConnection(Conn);
            return BERR_OS_ERROR;
        }else {
            /* Wait for reader thread entering wait for data loop */
            B_Event_Wait(Conn->hNetEvent, 3000);
            B_Event_Reset(Conn->hNetEvent);
            BDBG_MSG(("Succesfully Opened client connection"));
            retValue = BERR_SUCCESS;
        }
        B_Mutex_Lock(pProtocol->ConnectionChgMutex);
        BLST_S_INSERT_HEAD(&(pProtocol->Connection_list), Conn, node);
        B_Mutex_Unlock(pProtocol->ConnectionChgMutex);
    }

    BDBG_LEAVE(B_DTCP_IP_OpenConnection);
    return retValue;
}


static bool B_DTCP_IP_FindSocketInList_Locked(B_DTCP_IP_Connection_T * Conn,
        B_DTCP_IP_TransportData_T * pProtocol,
    int sfdToFind)
{
    B_DTCP_IP_Connection_T * currentConn;
    bool found = false;

    B_Mutex_Lock(pProtocol->ConnectionChgMutex);

    /* so walk the current list and see if it's still valid */
    currentConn = BLST_S_FIRST(&(pProtocol->Connection_list));

    BDBG_MSG(("%s:%d currentConn(%p)\n", __FUNCTION__, __LINE__, currentConn));

    while(currentConn != NULL)
    {
        if(currentConn->ConnectSocket == sfdToFind)
        {
            if (currentConn != Conn)
            {
                BDBG_ERR(("%s:%d currentConn=%p != Conn=%p\n", __FUNCTION__, __LINE__, currentConn, Conn));
            }
            found = true;
            break;
        }
        currentConn = BLST_S_NEXT(currentConn, node);
    }

    B_Mutex_Unlock(pProtocol->ConnectionChgMutex);
    return found;
}



/*! \brief close a opened connection.
 *    \param[in] Conn connection context pointer.
 *    \param[in] pProtocol pointer to IP transport layer protocol data.
 *
 *  If it's a master connection, e.g. a listenning session,  all exsiting connections will be closed.
 */
BERR_Code B_DTCP_IP_CloseConnection(B_DTCP_IP_Connection_T * Conn,
        B_DTCP_IP_TransportData_T * pProtocol)
{
    B_DTCP_IP_Connection_T * iter, * tmp;

    BERR_Code retValue = BERR_SUCCESS;
    B_Error retCode = B_ERROR_SUCCESS;
    int wait_timeout_ms = 0;


    BDBG_ASSERT(Conn);
    BDBG_ENTER(B_DTCP_IP_CloseConnection);

    if(Conn->Mode == B_ConnectionMode_eServer)
    {
        if(Conn->IsMaster == true)
        {
            BDBG_MSG(("CLOSING MASTER CONN"));
            if ((retValue = B_SocketShutdown(Conn->ListenerSocket)) != BERR_SUCCESS)
            {
                BDBG_WRN(("%d Unable to shutdown Listener socket : %d ", __LINE__, retValue));
            }
            if((retValue = B_SocketClose(Conn->ListenerSocket) != BERR_SUCCESS))
            {
                BDBG_WRN(("Unable to close listenning socket\n"));
            }
            Conn->ListenerSocket = INVALID_SOCKET;
            /* close all accepted connections*/
            B_Mutex_Lock(pProtocol->ConnectionChgMutex);
            iter = BLST_S_FIRST(&(pProtocol->Connection_list));
            B_Mutex_Unlock(pProtocol->ConnectionChgMutex);
            while(iter != NULL){
                B_Event_Reset(Conn->hNetEvent);
                B_Event_Reset(iter->hConnTerminated);

                B_Thread_Destroy(iter->hConnectionThread);

                tmp = iter;
                B_Mutex_Lock(pProtocol->ConnectionChgMutex);
                iter = BLST_S_NEXT(iter, node);
                BLST_S_REMOVE(&(pProtocol->Connection_list), tmp, B_DTCP_IP_Connection, node);
                B_Mutex_Unlock(pProtocol->ConnectionChgMutex);

                B_DTCP_IP_DestroyConnection(tmp);
            }
            /* Destroy master connection */
            B_DTCP_IP_DestroyConnection(pProtocol->MasterConn);

        }
        else
        {
            if ( pProtocol->masterShutDown != true )
            {
                int len = 0;
                if ((retValue = B_SocketShutdown(Conn->ConnectSocket)) != BERR_SUCCESS)
                {
                    BDBG_MSG(("%d Unable to shutdown socket for %s: %d ", __LINE__, Conn->RemoteAddr, retValue));
                }
                BDBG_MSG(("Socket SHUTDOWN done ...sfd %d", Conn->ConnectSocket));

                #if 0
                B_Mutex_Lock(pProtocol->ConnectionChgMutex);
                BLST_S_REMOVE(&(pProtocol->Connection_list), Conn, B_DTCP_IP_Connection, node);
                B_Mutex_Unlock(pProtocol->ConnectionChgMutex);
                #endif

            /* there might be some packets pending,  drain any pending data. The reader thread won't exit until all data are drained */
            wait_timeout_ms = 0;
            retCode = B_Event_Wait(Conn->hReaderTerminated, WAIT_FOR_READER_THREAD_TIMEOUT_MSEC);
            while ( (retCode != B_ERROR_SUCCESS) && wait_timeout_ms < WAIT_FOR_READER_THREAD_TIMEOUT_MSEC_MAX )
            {
                unsigned char *tmp_buf = NULL;
                int recv_bytes = 0;

                len = B_DTCP_IP_GetDataSize_Ex(Conn);

                BDBG_MSG(("%s:%d ConnectSocket=%d %d bytes pending \n", __FUNCTION__, __LINE__, Conn->ConnectSocket, Conn->DataSize));

                tmp_buf = BKNI_Malloc(Conn->DataSize);
                if (!tmp_buf)
                {
                    BDBG_ERR(("%s(%d): malloc failed\n", __FUNCTION__, __LINE__));
                    break;
                }
                recv_bytes = B_SocketRecvData( Conn->ConnectSocket, tmp_buf, len);
                    BDBG_MSG(("%s:%d ConnectSocket=%d %d bytes read \n", __FUNCTION__, __LINE__, Conn->ConnectSocket, recv_bytes));
                B_Event_Set(Conn->hBufCleared);
                if (tmp_buf) BKNI_Free(tmp_buf);
                tmp_buf = NULL;

                /* force to wait the reader thread to wait for some time*/
                retCode = B_Event_Wait(Conn->hReaderTerminated, WAIT_FOR_READER_THREAD_TIMEOUT_MSEC); /* wait for reader thread to timeout */
                if (retCode == B_ERROR_TIMEOUT) {
                    BDBG_MSG(("hReaderTerminated event was timed out"));
                } else if (retCode != B_ERROR_SUCCESS) {
                    BDBG_ERR(("threadHaltedEvent gave error"));
                }
                wait_timeout_ms += WAIT_FOR_READER_THREAD_TIMEOUT_MSEC;
            }
            if (retCode == B_ERROR_SUCCESS) {
                BDBG_MSG(("hReaderTerminated wokeup"));
            } else
            {
                BDBG_ERR(("hReaderTerminated eror, retCode %d, timeout %dms", retCode, wait_timeout_ms));
            }

                if (Conn->readerThreadDone == true)
                {
                    BDBG_MSG(("%s(%d) : Thread Destroy ... EventType %d sfd %d", __FUNCTION__, __LINE__, Conn->EventType, Conn->ConnectSocket));
                    B_Thread_Destroy(Conn->hReaderThread);
                }
                else
                {
                    BDBG_WRN(("Reader Thread destroyed before it finished processing ...EventType %d sfd %d", Conn->EventType, Conn->ConnectSocket));
                }

                /* Close a single  accepted connection */
                if((retValue = B_SocketClose(Conn->ConnectSocket)) != BERR_SUCCESS)
                {
                    BDBG_WRN(("%d Unable to close socket for %s : %d\n", __LINE__, Conn->RemoteAddr, retValue));
                }
                else
                {
                    Conn->ConnectSocket = INVALID_SOCKET;
                    Conn->socketClosed = true;
                }
                /*
                 * In rare cases, we still get here when the reader thread has not exited(bug in socket shutdown?).
                 * we have to wait the reader thread to exit, before destroy it.
                 * b_SocketClose and sockedClosed == trun will  force it to exit.
                 */
                while (Conn->readerThreadDone != true)
                {
                    retCode = B_Event_Wait(Conn->hReaderTerminated, WAIT_FOR_READER_THREAD_TIMEOUT_MSEC); /* wait for reader thread to timeout */
                    if (retCode == B_ERROR_SUCCESS) {
                        B_Thread_Destroy(Conn->hReaderThread);
                        break;
                    }
                }

                #if 0
                B_DTCP_IP_DestroyConnection(Conn);
                #endif
            }
        }
    }
    else
    {
        int sfdOnEntry;

        /* save the socket on entry */
        sfdOnEntry = Conn->ConnectSocket;

        /* while we were locked out the connection could have gotten destroyed */
        if (B_DTCP_IP_FindSocketInList_Locked( Conn, pProtocol, sfdOnEntry ))
        {
            int len = 0;
            BDBG_MSG(("%s:%d sfdOnEntry=%d found Conn->ConnectSocket=%d\n", __FUNCTION__, __LINE__, sfdOnEntry, Conn->ConnectSocket));

            if ((retValue = B_SocketShutdown(Conn->ConnectSocket)) != BERR_SUCCESS)
            {
                BDBG_MSG(("Unable to shutdown socket for %s: %d ", Conn->RemoteAddr, retValue));
            }
            BDBG_MSG(("Socket SHUTDOWN done ..."));

            /* remove from link list */
            B_Mutex_Lock(pProtocol->ConnectionChgMutex);
            BLST_S_REMOVE(&(pProtocol->Connection_list), Conn, B_DTCP_IP_Connection, node);
            B_Mutex_Unlock(pProtocol->ConnectionChgMutex);

            BDBG_MSG(("%s:%d before waiting: sfdOnEntry=%d found Conn->ConnectSocket=%d Conn->DataSize %d\n", __FUNCTION__, __LINE__, sfdOnEntry, Conn->ConnectSocket, Conn->DataSize));

            /* there might be some packets pending,  drain any pending data. The reader thread won't exit until all data are drained */
            wait_timeout_ms = 0;
            retCode = B_Event_Wait(Conn->hReaderTerminated, WAIT_FOR_READER_THREAD_TIMEOUT_MSEC);
            while ( (retCode != B_ERROR_SUCCESS) && wait_timeout_ms < WAIT_FOR_READER_THREAD_TIMEOUT_MSEC_MAX )
            {
                unsigned char *tmp_buf = NULL;
                int recv_bytes = 0;

            len = B_DTCP_IP_GetDataSize_Ex(Conn);

                BDBG_MSG(("%s:%d sfdOnEntry=%d %d bytes pending \n", __FUNCTION__, __LINE__, sfdOnEntry, Conn->DataSize));

                tmp_buf = BKNI_Malloc(Conn->DataSize);
                if (!tmp_buf)
                {
                    BDBG_ERR(("%s(%d): malloc failed\n", __FUNCTION__, __LINE__));
                    break;
                }
                recv_bytes = B_SocketRecvData( Conn->ConnectSocket, tmp_buf, len);
                BDBG_MSG(("%s:%d ConnectSocket=%d %d bytes read \n", __FUNCTION__, __LINE__, Conn->ConnectSocket, recv_bytes));

                B_Event_Set(Conn->hBufCleared);
                if (tmp_buf) BKNI_Free(tmp_buf);
                tmp_buf = NULL;

                /* force to wait the reader thread to wait for some time*/
                retCode = B_Event_Wait(Conn->hReaderTerminated, WAIT_FOR_READER_THREAD_TIMEOUT_MSEC); /* wait for reader thread to timeout */
                if (retCode == B_ERROR_TIMEOUT) {
                    BDBG_MSG(("hReaderTerminated event was timed out"));
                } else if (retCode != B_ERROR_SUCCESS) {
                    BDBG_ERR(("threadHaltedEvent gave error"));
                }
                wait_timeout_ms += WAIT_FOR_READER_THREAD_TIMEOUT_MSEC;
            }
            if (retCode == B_ERROR_SUCCESS) {
                BDBG_MSG(("hReaderTerminated wokeup"));
            } else
            {
                BDBG_ERR(("hReaderTerminated eror, retCode %d, timeout %dms", retCode, wait_timeout_ms));
            }

                if (Conn->readerThreadDone == true)
                {
                BDBG_MSG(("%s(%d) : Thread Destroy ... EventType %d", __FUNCTION__, __LINE__, Conn->EventType));
                    B_Thread_Destroy(Conn->hReaderThread);
                }
                else
            {
                BDBG_MSG(("Reader Thread destroyed before it finished processing ...EventType %d", Conn->EventType));
                BDBG_ASSERT(0);
            }

            if ((retValue = B_SocketClose(Conn->ConnectSocket)) != BERR_SUCCESS)
            {
                BDBG_MSG(("Unable to shutdown socket for %s: %d ", Conn->RemoteAddr, retValue));
            }
            else
            {
                BDBG_MSG(("Socket Close done ..."));
                Conn->ConnectSocket = INVALID_SOCKET;
                Conn->socketClosed = true;
            }

            /*
             * In rare cases, we still get here when the reader thread has not exited (bug in socket shutdown?).
             * we have to wait the reader thread to exit, before destroy it.
             * b_SocketClose and sockedClosed == trun will  force it to exit.
             */
            while (Conn->readerThreadDone != true)
            {
                retCode = B_Event_Wait(Conn->hReaderTerminated, WAIT_FOR_READER_THREAD_TIMEOUT_MSEC); /* wait for reader thread to timeout */
                if (retCode == B_ERROR_SUCCESS) {
                    B_Thread_Destroy(Conn->hReaderThread);
                    break;
                }
            }
            B_DTCP_IP_DestroyConnection(Conn);
        }
    }

    BDBG_LEAVE(B_DTCP_IP_CloseConnection);
    return retValue;
}
