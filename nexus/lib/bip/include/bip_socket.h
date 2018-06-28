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

#ifndef BIP_SOCKET_H
#define BIP_SOCKET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bip_types.h"
typedef struct BIP_Socket *BIP_SocketHandle;

/**
 * Summary:
 * BIP Sokcet APIs
 *
 * Description:
 * Abstraction of BSD Socket APIs to allow Apps to send & receive any type of data (e.g. HTTP Messages, AV payloads, etc.).
 **/

/**
 * Summary:
 * API to create new Socket object using an existing socketFd.
 *
 * Description:
 *
 * The caller already has the TCP socket from which this API will create a BIP_Socket object.
 * E.g. Caller uses BIP_Listener to get a new socket by calling BIP_Listener_Accept and then
 * calls this API to create a BIP_Socket object using that socketFd.
 *
 **/
BIP_SocketHandle BIP_Socket_CreateFromFd(int socketFd);


/* BIP Socket Types */
typedef enum
{
    BIP_SocketType_eTcp,   /* TCP SOCK_STREAM type Socket */
    BIP_SocketType_eUdpTx, /* UDP TX SOCK_DGRAM Type Socket */
    BIP_SocketType_eUdpRx, /* UDP RX SOCK_DGRAM Type Socket */
    BIP_SocketType_eMax
} BIP_SocketType ;

/**
 * Summary:
 * Get Default Socket Create settings
 **/
typedef struct BIP_SocketCreateSettings
{
    BIP_SocketType type;                      /* socket type: TCP, UDPTx, UDPRx  */
    char *pPort;                               /* local port # to bind to: required for UDP Rx type, optional or N/A otherwise */
    char *pIpAddress;                         /* ipAddress to bind to: required for UDP Rx type, optional or N/A otherwise */
    char *pInterfaceName;                     /* interface name to send packets on: required for UDPTx type, optional otherwise */
} BIP_SocketCreateSettings;
void BIP_Socket_GetDefaultCreateSettings( BIP_SocketCreateSettings *pSettings );

/**
 * Summary:
 * API to create new BIP Socket
 *
 * Description:
 *
 * This API allows caller to create a BIP Socket using particular socket settings. Caller can create a TCP or UDP type socket.
 * TCP type socket created using this API will need to be connected to the server using the BIP_Socket_Connect API.
 *
 **/
BIP_SocketHandle BIP_Socket_Create(const BIP_SocketCreateSettings *pSettings );

/**
 * Summary:
 * Customizable Socket Settings
 **/
typedef struct BIP_SocketSettings
{
    BIP_CallbackDesc dataReadyCallback;       /* callback to indicate some data is available for reading on the socket */
    BIP_CallbackDesc errorCallback;           /* callback to indicate error on socket: peer closed the socket or some network error */
    BIP_CallbackDesc writeReadyCallback;      /* callback to indicate socket is writable: outgoing connect is complete or more data can be written on this socket */
} BIP_SocketSettings;

/**
 * Summary:
 * Get current Socket Settings
 **/
void BIP_Socket_GetSettings( BIP_SocketHandle socket, BIP_SocketSettings *pSettings );

/**
 * Summary:
 * Update current Socket Settings
 **/
BIP_Status BIP_Socket_SetSettings( BIP_SocketHandle socket, BIP_SocketSettings *pSettings );

/**
 * Summary:
 * Customizable Connect Settings
 **/
typedef struct BIP_SocketConnectSettings
{
    BIP_ApiSettings api;          /* Generic API settings. */
    struct
    {
        char *pPort;        /* TCP: server's Port; UDP Tx: Port used in Sendto; UDP Rx: Sending Server's Port, set only if server's port is known */
        char *pIpAddress;     /* TCP: server's IP; UDP Tx: IP used in Sendto; UDP Rx: Sending Server's IP, set only if server's IP Address is known */
    } input;
    struct
    {
        int unused;
    } output;
} BIP_SocketConnectSettings;

/**
 * Summary:
 * Get Default Socket Connect Settings
 **/
void BIP_Socket_GetDefaultConnectSettings( BIP_SocketConnectSettings *pSettings );

/**
 * Summary:
 * API to Connect to the specified remote.
 *
 * Description:
 *
 * For TCP type socket, this API will connect to specified remote server by using the BSD socket's connect API.
 * For UDPTx type socket, this API will send outgoing data to specified remote address using the BSD socket's sendTo API.
 * For UDPRx type socket, this API will allow caller to only receive data from the specified remote address.
 *
 * Note on the timeout parameter (units in msec):
 *
 * For TCP socket:
 * timeout: 0 -> non-blocking, starts connect processing but returns BIP_INF_IN_PROGRESS. connect completion of success is notified via writeReadyCallback or via errorCallback for any other errors.
 * timeout: value -> blocking with timeout, returns connection success via BIP_SUCCESS or BIP_INF_TIMEOUT if connect doesn't complete within timeout msec, otherwise other BIP Errors are set to indiate other failures.
 * timeout: -1 -> blocks until connect completes and returns connection success via BIP_SUCCESS or other BIP Error to indicate failure.
 *
 * For UDP Tx or Rx type socket:
 * timeout parameter is ignored as underlying socket call doesn't involve any interaction with the peer and thus completes quickly.
 *
 **/
BIP_Status BIP_Socket_Connect( BIP_SocketHandle hSocket, BIP_SocketConnectSettings *pSettings );

/**
 * Summary:
 * Customizable Receive Settings
 **/
typedef struct BIP_SocketRecvSettings
{
    BIP_ApiSettings api;          /* Generic API settings. */
    struct
    {
        size_t   bytesToRead;       /* Number of bytes to read. */
    } input;
    struct
    {
        char    *pBuffer;           /* Where to put the received data. */
        ssize_t *pBytesRead;        /* Where to put the number of bytes actually read.  */
    } output;
} BIP_SocketRecvSettings;

/**
 * Summary:
 * Get Default Socket Recieve Settings
 **/
void BIP_Socket_GetDefaultSocketRecvSettings(BIP_SocketRecvSettings  *pSocketRecvSettings);

/**
 * Summary:
 * BIP Socket API to Recv Data.
 *
 * Description:
 *
 * This API allows caller to recv upto a max of bytesToRead bytes in the pBuffer.
 * bytesRead is only valid if BIP_SUCCESS is returned.
 *
 * Same API is used for receiving data on all BIP socket types.
 *
 * Note on timeout parameter usage:
 *
 * timeout: 0 -> non-blocking, returns whatever data is currently available on the socket in bytesRead when BIP_SUCESS is set. BIP_Status otherwise is set.
 * timeout: value -> blocking with timeout, returns when either upto bytesToRead bytes are read (BIP_SUCCESS) or timeout expires (BIP_INF_TIMEOUT) or BIP Error happens.
 * timeout: -1 -> blocking until bytesToRead buffers are read & BIP_SUCCESS is returned. BIP_Status in failure cases is set.
 *
 **/
BIP_Status BIP_Socket_Recv( BIP_SocketHandle hSocket, BIP_SocketRecvSettings  *pSocketRecvSettings );


/**
 * Summary:
 * Customizable Send Settings
 **/
typedef struct BIP_SocketSendSettings
{
    BIP_ApiSettings api;          /* Generic API settings. */
    struct
    {
        size_t bytesToSend;       /* Number of bytes to read. */
        const char  *pBuffer;           /* Pointer to buffer with data to be sent. */
    } input;
    struct
    {
        ssize_t *pBytesSent;        /* Pointer to variable containing the number of bytes actually sent.          */
    } output;
} BIP_SocketSendSettings;

/**
 * Summary:
 * Get Default Socket Send Settings
 **/
void BIP_Socket_GetDefaultSocketSendSettings(BIP_SocketSendSettings  *pSocketSendSettings);

/**
 * Summary:
 * BIP Socket API to Send Data.
 *
 * Description:
 *
 * This API allows caller to send upto a max of bytesToSend bytes in the pBuffer.
 * bytesWritten is only valid if BIP_SUCCESS is returned.
 *
 * Same API is used for sending data on all BIP socket types.
 *
 * Note on timeout parameter usage (only applies to TCP socket type):
 *
 * timeout: 0 -> non-blocking, if tries to send upto the bytesLength bytes that underlying socket will current accept.
 * timeout: value -> blocking with timeout, returns when either bytesToSend bytes are sent or timeout expires.
 * timeout: -1 -> blocks until bytesToSend bytes are sent
 *
 * Return value:
 * Returns BIP_SUCCESS if API can send any bytes and updates the actual length in the bytesSent.
 * Otherwise, returns BIP_Status. Common ones are:
 *
 * BIP_INF_TIMEOUT: failed to send any bytes in the timeout interval
 * BIP_WOULD_BLOCK: can't send any bytes as no socket buffer space is currently available. App would need to retry sending.
 **/

BIP_Status BIP_Socket_Send( BIP_SocketHandle hSocket, BIP_SocketSendSettings  *pSocketSendSettings );

/**
 * Summary:
 * Destroy socket
 *
 * Description:
 * Implementation note: Make sure that a blocking Send or Recv calls are aborted before initiating the destroy sequence! Use Mutex w/ timeout!
 **/
void BIP_Socket_Destroy( BIP_SocketHandle hSocket );

typedef struct BIP_SocketStatus
{
    int socketFd;
    char *pInterfaceName;
    const char *pLocalIpAddress;        /* IP Address string for the local IP address being used for this socketFd. */
    const char *pRemoteIpAddress;       /* IP Address string for the remote IP address being used for this socketFd. */
    /* TODO: add more fields: local & remote ip address, interface name, socket type, etc. */
} BIP_SocketStatus;

/**
 * Summary:
 * Get BIPSocket Status
 **/
BIP_Status BIP_Socket_GetStatus( BIP_SocketHandle hSocket, BIP_SocketStatus *pStatus );

/* ********************************************************************************************** */
#ifdef __cplusplus
}
#endif

#endif /* BIP_SOCKET_H */
