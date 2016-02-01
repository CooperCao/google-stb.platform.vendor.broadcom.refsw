/******************************************************************************
 * (c) 2007-2014 Broadcom Corporation
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

#ifndef BIP_RTSP_SOCKET_H
#define BIP_RTSP_SOCKET_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BIP_RtspSocket *BIP_RtspSocketHandle;

/**
 * Summary:
 * Rtsp Sokcet APIs
 *
 * Description:
 * Abstraction of BSD Socket APIs to allow Apps to send & receive RTSP messages & data
 **/

/**
 * Summary:
 * Get Default RtspSocket settings
 **/
typedef struct BIP_RtspSocketCreateSettings
{
    int unused;
} BIP_RtspSocketCreateSettings;
void BIP_RtspSocket_GetDefaultCreateSettings( BIP_RtspSocketCreateSettings *pSettings );

/**
 * Summary:
 * API to create new RTSP Socket from a URL
 *
 * Description:
 *
 * This API allows to create RtspSocket object using an RTSP URL. The API is designed for
 * a media player (client) usage and used internally by BIP_Client interface.
 *
 **/
BIP_RtspSocketHandle BIP_RtspSocket_CreateFromUrl( char *url, BIP_RtspSocketCreateSettings *pSettings );

/**
 * Summary:
 * API to create new RTSP Socket using an existing socketFd and specific listener
 *
 * Description:
 *
 * This API allows to create RtspSocket object using an existing socket. The caller already has the
 * TCP socket on which RTSP communication will happen.
 *
 * This API is also used internally by the BIP_RtspListener interface which exposes this BIP_RtspSocket
 * back to the MediaServer App. App can then receive RtspRequest messages & send out RtspResponse
 * using this RtspSocket.
 *
 **/
BIP_RtspSocketHandle BIP_RtspSocket_CreateFromFd( int socketFd, void *lmListener, BIP_RtspSocketCreateSettings *pSettings );

/**
 * Summary:
 * Customizable RtspSocket Settings
 **/
typedef struct BIP_RtspSocketSettings
{
    BIP_CallbackDesc messageReceivedCallback; /* callback to indicate the next RTSP message from peer */
    BIP_CallbackDesc errorCallback;           /* callback to indicate error on socket: peer closed the socket or some network error */
} BIP_RtspSocketSettings;

/**
 * Summary:
 * Get current RtspSocket Settings
 **/
void BIP_RtspSocket_GetSettings( BIP_RtspSocketHandle socket, BIP_RtspSocketSettings *pSettings );

/**
 * Summary:
 * Update current RtspSocket Settings
 **/
BIP_Status BIP_RtspSocket_SetSettings( BIP_RtspSocketHandle socket, BIP_RtspSocketSettings *pSettings );

/**
 * Summary:
 * Rtsp Socket API to recv a RTSP Message on RTSP Socket
 **/
BIP_Status BIP_RtspSocket_RecvRequest( BIP_RtspSocketHandle socket, BIP_RtspRequestHandle rtspRequest );

/**
 * Summary:
 * Rtsp Socket API to create a new RTSP Session for a given RTSP Request
 **/
BIP_RtspSessionHandle BIP_RtspSocket_CreateSession( BIP_RtspSocketHandle rtspSocket, BIP_RtspRequestHandle request );
#if 0
BIP_Status BIP_RtspSocket_RecvResponse( BIP_RtspSocketHandle socket, BIP_RtspResponseHandle rtspResponse );

/**
 * Summary:
 * Rtsp Socket API to send a RTSP Message on RTSP Socket
 **/
BIP_Status BIP_RtspSocket_SendRequest( BIP_RtspSocketHandle socket, BIP_RtspRequestHandle rtspRequest );
BIP_Status BIP_RtspSocket_SendResponse( BIP_RtspSocketHandle socket, BIP_RtspResponseHandle rtspResponse );
#endif

/**
 * Summary:
 * Destroy socket
 *
 * Description:
 **/
void BIP_RtspSocket_Destroy( BIP_RtspSocketHandle connection );

/**
 * Summary:
 * API to return the interface name associated with this socket connection.
 *
 * Description:
 **/
char *BIP_RtspSocket_GetInterfaceName( BIP_RtspSocketHandle hRtspSocket );

/* ********************************************************************************************** */
#ifdef __cplusplus
}
#endif

#endif /* BIP_RTSP_SOCKET_H */
