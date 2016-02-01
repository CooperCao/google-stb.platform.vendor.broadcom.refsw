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

#ifndef BIP_RTSP_LIVEMEDIA_SOCKET_H
#define BIP_RTSP_LIVEMEDIA_SOCKET_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct BIP_RtspLiveMediaSocket *BIP_RtspLiveMediaSocketHandle;

/**
 * Summary:
 * Get Default RtspLiveMediaSocket settings
 **/
typedef struct BIP_RtspLiveMediaSocketCreateSettings
{
    int unused;
} BIP_RtspLiveMediaSocketCreateSettings;
void BIP_RtspLiveMediaSocket_GetDefaultCreateSettings( BIP_RtspLiveMediaSocketCreateSettings *pSettings );

/**
 * Summary:
 * C Wrapper to LiveMedia BIP_RtspClient object
 *
 * Description:
 *
 **/
BIP_RtspLiveMediaSocketHandle BIP_RtspLiveMediaSocket_CreateFromFd( unsigned socketFd, void *lmRtspServer, BIP_RtspLiveMediaSocketCreateSettings *pSettings );

#if 0
/* TODO: see if this API is needed for the client side */
BIP_RtspLiveMediaSocketHandle BIP_RtspLiveMediaSocket_CreateFromUrl( char *url, BIP_RtspLiveMediaSocketCreateSettings *pSettings );
#endif

/**
 * Summary:
 * Rtsp Socket settings to customize the RTSP Socket
 **/
typedef struct BIP_RtspLiveMediaSocketSettings
{
    BIP_CallbackDesc messageReceivedCallback; /* callback to indicate the next RTSP message from peer */
    BIP_CallbackDesc errorCallback;           /* callback to indicate error on socket: peer closed the socket or some network error */
} BIP_RtspLiveMediaSocketSettings;

void      BIP_RtspLiveMediaSocket_GetSettings( BIP_RtspLiveMediaSocketHandle socket, BIP_RtspLiveMediaSocketSettings *pSettings );
BIP_Status BIP_RtspLiveMediaSocket_SetSettings( BIP_RtspLiveMediaSocketHandle socket, BIP_RtspLiveMediaSocketSettings *pSettings );

/**
 * Summary:
 * Copy the currently received RTSP message from BIP_Rtsp Server class into the BIP_Socket specified buffer
 *
 * Description:
 *
 * BIP_RtspSocket class invokes this API to copy the currently received RTSP Message into its buffer so that it can
 * queue up this message request for app's consumption.
 **/
void BIP_RtspLiveMediaSocket_CopyMessage( BIP_RtspLiveMediaSocketHandle rtspLmSocket, char *pBuffer );

/**
 * Summary:
 * API to create new RTSP Session from a RTSP Socket
 *
 * Description:
 *
 * App calls this API to create a new RTSP Session when it has received a SETUP Request on a RTSP Socket
 *
 **/
/* TODO: see if we need to have createSettings in this call, or just two of these handles is sufficient */
BIP_RtspLiveMediaSessionHandle BIP_RtspLiveMediaSocket_CreateSession( BIP_RtspLiveMediaSocketHandle rtspLmSocket, char *pRequestStr );

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
#endif

/**
 * Summary:
 * Destroy socket
 *
 * Description:
 **/
void BIP_RtspLiveMediaSocket_Destroy( BIP_RtspLiveMediaSocketHandle rtspLmSocket );
void BIP_RtspLiveMediaRequest_Parse( char *pBuffer, unsigned bufferLength );

#ifdef __cplusplus
}
#endif

#endif /* BIP_RTSP_LIVEMEDIA_SOCKET_H */
