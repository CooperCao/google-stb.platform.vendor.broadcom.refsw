/********************************************************************************************
*     (c)2004-2016 Broadcom Corporation                                                     *
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
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
*********************************************************************************************/
/*! \file b_dtcp_ip_transport_priv.h
 *  \brief DTCP-IP connection handler header defininations.
 */
#ifndef B_DTCP_IP_CONNECTION_H
#define B_DTCP_IP_CONNECTION_H

#ifdef __cplusplus
extern "C"
{
#endif

#define INVALID_SOCKET  -1
#define HTTP_CONNECT_TIMEOUT 2 /* In seconds */
/*! \brief connection mode.
 */
typedef enum B_ConnectionMode
{
    B_ConnectionMode_eServer,
    B_ConnectionMode_eClient
}B_ConnectionMode_T;

/*! \struct B_DTCP_IP_Connection
 *  \brief a DTCP_IP connection data structure.
 */
typedef struct B_DTCP_IP_Connection
{
    BLST_S_ENTRY(B_DTCP_IP_Connection) node;    /* list entry node */
    char  LocalAddr[32];        /*!< local IP address */
    char  RemoteAddr[32];       /*!< remote IP address */
    int Port;                   /*!< connection port */
    B_ConnectionMode_T Mode;    /*!< connection mode, server or client */
    bool IsMaster;              /*!< flag indicate if this is a master connection*/

    int ListenerSocket;         /*!< Socket fd for listening on connection request */
    int ConnectSocket;          /*!< Socket fd when connection established.*/
    int ListenerTimeout;        /*!< time out value for listener thread */
    bool readerThreadDone;      /* Flag for connection thread to indicate whether reader thread is still alive of not*/
    bool connectionThreadDone;      /* Flag for connection thread to indicate whether reader thread is still alive of not*/
#if 0
    int ReaderTimeout;          /*!< time out value for reader thread */
#endif
    B_ThreadHandle hReaderThread;       /*!< Reader thread handle */
    B_ThreadHandle hConnectionThread;   /*!< connection thread handle */
    B_DTCP_TransportCallbackFunc_Ptr  OnClosedConnection;   /*!< close connection callback */
    B_DTCP_TransportCallbackFunc_Ptr  OnNewConnection;      /*!< new connection request callback */
    B_EventHandle  hConnTerminated;     /*!< connection thread terminated event handle */
    B_EventHandle  hReaderTerminated;       /*!< Reader thread terminated event handle */

    B_TransportEvent_T EventType;       /*!< Type of the occurred event.*/
    B_EventHandle  hNetEvent;           /*!< Data ready or network error event handle */
    B_EventHandle  hBufCleared;         /*!< event indicating buffer has been read out by other threads. */
    bool bufRead;
    B_MutexHandle  hMutex;              /*!< mutex to protect the NetEvent. */
    int DataSize;                       /*!< bytes length of data to be read. */
    void * reserved;                    /*!< can be used for any data pointer for parameter passing purpose. */
    bool socketClosed;                  /*!< flag to indicate to reader thread that main thread has closed the socket and thus start its cleanup */
}B_DTCP_IP_Connection_T;

/*! \brief IP transport layer specific data.
 */
typedef struct B_DTCP_IP_TransportData
{
    /* Source only function variables */
    char LocalIp[MAX_IP_ADDR_SIZE];
    int LocalPort;
    B_ThreadHandle hListenThread;
    B_DTCP_TransportCallbackFunc_Ptr ConnCallback;
    void * pStack;
    /* Source/sink function variables */
    bool masterShutDown;
    B_DTCP_IP_Connection_T * MasterConn;
    BLST_S_HEAD(Connections, B_DTCP_IP_Connection) Connection_list;
  B_MutexHandle ConnectionChgMutex;

}B_DTCP_IP_TransportData_T;

/*! \brief utility to get the event type.
 *
 * \param[in] Conn connection context pointer.
 * \retval one of the enum values defined in B_TransportEvent_T
 */
B_TransportEvent_T B_DTCP_IP_GetEventType_Ex(B_DTCP_IP_Connection_T * Conn);

/*! \brief utility to get data size available to read.
 *
 * \param[in] Conn connection context pointer.
 * \retval data size available for read.
 */
int B_DTCP_IP_GetDataSize_Ex(B_DTCP_IP_Connection_T * Conn);

/*! \brief Open a DTCP-IP connection,
 *
 * upon successful completion of this function, a connection object will
 * be allocated and initialized. If the mode is "server", a listener thread will
 * be spawned and start listening to incoming connection. If the mode is "client"
 * it will initiate connection to remote host.

 * \param[in] Mode server or client mode.
 * \param[in] LocalAddr Local IP address, optional
 * \param[in] RemoteAddr Remote IP address.
 * \param[in] Port TCP port number.
 * \param[in] OnNewConnCallback callback function when there is new connection, only required for server mode.
 * \param[in] OnCloseConneCallback callback function when connection is closed by transport layer.
 * \param[in, out] pProtocol pointer to IP transport protocol specific data.
 * \param[out] ConnectionId connection identifier, e.g socket id.
 */
BERR_Code B_DTCP_IP_OpenConnection(B_ConnectionMode_T Mode,
        char * LocalAddr,
        char * RemoteAddr,
        int Port,
        B_DTCP_TransportCallbackFunc_Ptr OnNewConnCallback,
        B_DTCP_TransportCallbackFunc_Ptr OnClosedConnCallback,
        B_DTCP_IP_TransportData_T * pProtocol,
        int * ConnectionId);

/*! \brief close a opened connection.
 *  \param[in] Conn connection context pointer.
 *  \param[in] pProtocol pointer to IP transport layer protocol data.
 *
 *  If it's a master connection, e.g. a listenning session,  all exsiting connections will be closed.
 */
BERR_Code B_DTCP_IP_CloseConnection(B_DTCP_IP_Connection_T * Conn,
        B_DTCP_IP_TransportData_T * pProtocol);


/*! \brief Wait for connection request, launch user supplied reader thread.
 *  \param[in] master a master connection context pointer.
 */
BERR_Code B_WaitForConnection(B_DTCP_IP_Connection_T * master,
                              B_DTCP_IP_TransportData_T * pProtocol);

/*------------------function prototypes in b_sccket_wrapper.c-------------------------- */
int B_SocketRecvData( int sfd, unsigned char * buffer, int length);
int B_SocketSendData(int sfd, unsigned char * buffer, int length);
BERR_Code B_SocketKeepAlive(int s);
BERR_Code B_SocketWaitForData(int sfd, B_TransportEvent_T * Event,
        B_EventHandle hEvent, B_EventHandle hBufCleared, B_MutexHandle hMutex,
        int * size, bool *socketClosed, int Timeout);
#if 0
BERR_Code B_SocketWaitForData( int maxfd, B_DTCP_IP_TransportData_T * pProtocol, int Timeout);
#endif
BERR_Code B_SocketBind(int sfd, char * LocalAddr, int port);
BERR_Code B_SocketConnect(int sfd, char * RemoteAddr, int port);
BERR_Code B_SocketListen(int sfd);
int B_SocketAccept(int sfd, char * RemoteAddr, int size);
int B_SocketOpen(void);
int B_SocketShutdown(int sfd);
int B_SocketClose(int sfd);

#ifdef __cplusplus
}
#endif
#endif /* B_DTCP_IP_CONNECTION_H */
