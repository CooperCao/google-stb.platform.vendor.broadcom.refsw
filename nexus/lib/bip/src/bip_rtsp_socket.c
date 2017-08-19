/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "bip_priv.h"
#include "bip_rtsp_lm_server.h"
#include "bip_rtsp_server.h"
#include "bip_rtsp_socket.h"
#include "bip_rtsp_socket_impl.h"

BDBG_MODULE( bip_rtsp_socket );
BDBG_OBJECT_ID( BIP_RtspSocket );

void BIP_RtspSocket_GetDefaultCreateSettings(
    BIP_RtspSocketCreateSettings *pSettings
    )
{
    BKNI_Memset( pSettings, 0, sizeof( BIP_RtspSocketCreateSettings ));
}

static void destroyPendingMessageInfoList(
    BIP_RtspSocketHandle hRtspSocket
    )
{
    BIP_RtspSocketMessageInfo *messageInfo;

    if (!hRtspSocket) {return; }

    for (messageInfo = BLST_Q_FIRST( &hRtspSocket->pendingMessageInfoListHead );
         messageInfo;
         messageInfo = BLST_Q_FIRST( &hRtspSocket->pendingMessageInfoListHead ))
    {
        BLST_Q_REMOVE( &hRtspSocket->pendingMessageInfoListHead, messageInfo, pendingMessageInfoListNext );
        if (messageInfo->pBuffer)
        {
            BKNI_Free( messageInfo->pBuffer );
        }
        BKNI_Free( messageInfo );
    }
} /* destroyPendingMessageInfoList */

static void rtspSocketDestroy(
    BIP_RtspSocketHandle hRtspSocket
    )
{
    if (!hRtspSocket)
    {
        return;
    }

    if (hRtspSocket->hRtspLmSocket) {BIP_RtspLiveMediaSocket_Destroy( hRtspSocket->hRtspLmSocket ); }

    destroyPendingMessageInfoList( hRtspSocket );

    if (hRtspSocket->lock) {BKNI_DestroyMutex( hRtspSocket->lock ); }
    if (hRtspSocket->pInterfaceName) {BKNI_Free( hRtspSocket->pInterfaceName ); hRtspSocket->pInterfaceName =NULL; }

    BDBG_OBJECT_DESTROY( hRtspSocket, BIP_RtspSocket );
    BKNI_Free( hRtspSocket );
} /* rtspSocketDestroy */


#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>



static int getInterfaceName(
    BIP_RtspSocketHandle hRtspSocket,
    int                  socketFd
    )
{
    int                     rc=-1;
    struct sockaddr_in      localIpAddress;
    socklen_t               length=sizeof(localIpAddress);
    char                   *pInterfaceName = NULL;
    struct if_nameindex    *pNameIndexArray = NULL;  /* Pointer to start of array. */
    struct if_nameindex    *pThisNameIndex;          /* Iterates through array. */

    BKNI_Memset(&localIpAddress, 0, sizeof(localIpAddress));

    /* Find local address associated with this socket */
    rc = getsockname( socketFd, (struct sockaddr *)&localIpAddress, &length );
    BIP_CHECK_GOTO(( !rc ), ( "getsockName() Failed, errno %d", errno ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    BDBG_MSG(( BIP_MSG_PRE_FMT "localIpAddress: "BIP_INET_ADDR_FMT
               BIP_MSG_PRE_ARG, BIP_INET_ADDR_ARG(localIpAddress.sin_addr) ));

    /* Get the list of interface names. */
    pNameIndexArray = if_nameindex();
    BIP_CHECK_GOTO((pNameIndexArray != NULL), ( "if_nameindex() Failed"), error, BIP_StatusFromErrno(errno), rc );

    /* Loop through each interface until we find one that matches our localIpAddress. */
    for ( pThisNameIndex=pNameIndexArray ; pThisNameIndex->if_name!=NULL ; pThisNameIndex++ )
    {
        char *          if_name = pThisNameIndex->if_name;
        struct ifreq    ifr;
        size_t          if_nameLen = strlen(if_name);
        int             myerrno;

        /* If this interface's name is too long, skip it. */
        if (if_nameLen > sizeof(ifr.ifr_name) - 1 )
        {
            BDBG_WRN((BIP_MSG_PRE_FMT "%s:%d: Network interface name: %s exceeds %zu bytes! Skipping..."
                      BIP_MSG_PRE_ARG,BSTD_FUNCTION, __LINE__, if_name, sizeof(ifr.ifr_name)-1 ));
            continue;
        }

        /* Now use the SIOCGIFADDR ioctl to get the address of the interface. */
        memcpy( ifr.ifr_name, if_name, if_nameLen);     /* Copy interface name into ifreq struct. */
        ifr.ifr_name[if_nameLen] = 0;                   /* Add null termination. */

        rc = ioctl(socketFd,SIOCGIFADDR,&ifr);
        if (rc != 0)
        {
            myerrno = errno;

            if (myerrno == EADDRNOTAVAIL) {
                BDBG_MSG(( BIP_MSG_PRE_FMT "No address available for iface=%s. Skipping..."
                           BIP_MSG_PRE_ARG, if_name ));
            }
            else
            {
                BDBG_WRN(( BIP_MSG_PRE_FMT "Ioctl SIOCGIFADDR failed for iface=%s, errno=%u. Skipping..."
                           BIP_MSG_PRE_ARG, if_name, myerrno ));
            }
            continue;   /* If we couldn't get the address of this interface, just skip it. */
        }

        /* Now check to see if the interface matches our localIpAddress. */
        {
            struct sockaddr_in  ifaceIpAddress;

            memcpy(&ifaceIpAddress, &ifr.ifr_addr, sizeof(ifaceIpAddress));
            BDBG_MSG(( BIP_MSG_PRE_FMT "local Ip: "BIP_INET_ADDR_FMT", iface %s: "BIP_INET_ADDR_FMT""
                       BIP_MSG_PRE_ARG, BIP_INET_ADDR_ARG(localIpAddress.sin_addr.s_addr),
                       if_name,         BIP_INET_ADDR_ARG(ifaceIpAddress.sin_addr.s_addr)));

            if (localIpAddress.sin_addr.s_addr == ifaceIpAddress.sin_addr.s_addr)
            {
                pInterfaceName = pThisNameIndex->if_name;
                break;      /* Found it!  We're done. */
            }
        }
    } /* End for each interface name. */

    /* If we found the interface name, make a copy and and add to the rtspSocket. */
    if (pInterfaceName)
    {
        int pInterfaceNameLen = strlen( pInterfaceName ) + 1 ;

        hRtspSocket->pInterfaceName = BKNI_Malloc( pInterfaceNameLen );
        BIP_CHECK_GOTO(( hRtspSocket->pInterfaceName ), ( "BKNI Malloc Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

        BKNI_Memset( hRtspSocket->pInterfaceName, 0, pInterfaceNameLen );
        strncpy( hRtspSocket->pInterfaceName, pInterfaceName, pInterfaceNameLen-1 );
        hRtspSocket->pInterfaceName[strlen(pInterfaceName)] = '\0';
        rc = 0;
    }
    else
    {
        rc = -1;
    }

error:
    if (pNameIndexArray) {
        if_freenameindex(pNameIndexArray);
    }
    return( rc );
} /* getInterfaceName */

BIP_RtspSocketHandle BIP_RtspSocket_CreateFromFd(
    int                           socketFd,
    void                         *lmListener,
    BIP_RtspSocketCreateSettings *pSettings
    )
{
    int                             rc;
    BIP_RtspSocketHandle            hRtspSocket = NULL;
    BIP_RtspSocketCreateSettings    defaultSettings;
    BIP_RtspLiveMediaListenerHandle hRtspLmListener = (BIP_RtspLiveMediaListenerHandle)lmListener;

    /* Create the rtspSocket object */
    BDBG_MSG((BIP_MSG_PRE_FMT "hRtspSocket Malloc(%zu); " BIP_MSG_PRE_ARG,  sizeof( *hRtspSocket ) ));
    hRtspSocket = BKNI_Malloc( sizeof( *hRtspSocket ));
    BIP_CHECK_PTR_GOTO( hRtspSocket, "Memory Allocation Failed", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );
    BKNI_Memset( hRtspSocket, 0, sizeof( *hRtspSocket ));
    BDBG_OBJECT_SET( hRtspSocket, BIP_RtspSocket );

    rc = getInterfaceName( hRtspSocket, socketFd );
    BIP_CHECK_GOTO(( !rc ), ( "getInterfaceName Failed, errno %d", errno ), error, rc, rc );

    /* If user didn't specify create settings, use the default settings */
    if (NULL == pSettings)
    {
        BDBG_MSG((BIP_MSG_PRE_FMT "BIP_RtspSocket_GetDefaultCreateSettings" BIP_MSG_PRE_ARG ));
        BIP_RtspSocket_GetDefaultCreateSettings( &defaultSettings );
        pSettings = &defaultSettings;
    }
    hRtspSocket->createSettings = *pSettings;

    /* Create liveMedia RtspSocket object */
    BDBG_MSG((BIP_MSG_PRE_FMT "BIP_RtspLiveMediaListener_CreateSocket" BIP_MSG_PRE_ARG));
    hRtspSocket->hRtspLmSocket = BIP_RtspLiveMediaListener_CreateSocket( hRtspLmListener, socketFd );
    BIP_CHECK_PTR_GOTO( hRtspSocket->hRtspLmSocket, "Memory Allocation Failed", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );

    BDBG_MSG((BIP_MSG_PRE_FMT "BKNI_CreateMutex" BIP_MSG_PRE_ARG));
    rc = BKNI_CreateMutex( &hRtspSocket->lock );
    BIP_CHECK_ERR_NZ_GOTO( rc, "BKNI_CreateMutex() Failed", error );

    /* Set state to idle */
    hRtspSocket->state = BIP_RtspSocketState_eIdle;
    BDBG_MSG((BIP_MSG_PRE_FMT "hRtspSocket %p" BIP_MSG_PRE_ARG , (void *)hRtspSocket ));

    BDBG_MSG((BIP_MSG_PRE_FMT "returning %p" BIP_MSG_PRE_ARG, (void *)hRtspSocket ));
    return( hRtspSocket );

error:
    rtspSocketDestroy( hRtspSocket );

    return( NULL );
} /* BIP_RtspSocket_CreateFromFd */

/**
 * Summary:
 * Destroy rtsp socket
 *
 * Description:
 **/
void BIP_RtspSocket_Destroy(
    BIP_RtspSocketHandle hRtspSocket
    )
{
    BDBG_OBJECT_ASSERT( hRtspSocket, BIP_RtspSocket );
    BDBG_MSG(( BIP_MSG_PRE_FMT "hRtspSocket %p" BIP_MSG_PRE_ARG, (void *)hRtspSocket ));
    rtspSocketDestroy( hRtspSocket );
} /* BIP_RtspSocket_Destroy */

char *BIP_RtspSocket_GetInterfaceName(
    BIP_RtspSocketHandle hRtspSocket
    )
{
    BDBG_OBJECT_ASSERT( hRtspSocket, BIP_RtspSocket );
    return( hRtspSocket->pInterfaceName );
}

void BIP_RtspSocket_GetSettings(
    BIP_RtspSocketHandle    hRtspSocket,
    BIP_RtspSocketSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT( hRtspSocket, BIP_RtspSocket );
    BDBG_ASSERT( pSettings );
    *pSettings = hRtspSocket->settings;
}

static void rtspSocketMessageReceivedCallback(
    void *context,
    int   messageLength
    )
{
    BIP_RtspSocketHandle hRtspSocket = (BIP_RtspSocketHandle)context;
    BIP_RtspSocketMessageInfo     *messageInfo = NULL;

    BDBG_OBJECT_ASSERT( hRtspSocket, BIP_RtspSocket );
    BDBG_ASSERT( messageLength );
    BDBG_MSG(( BIP_MSG_PRE_FMT "hRtspSocket %p, messageLength %d"BIP_MSG_PRE_ARG, (void *)hRtspSocket, messageLength ));

    /* Create new messageInfo entry to hold this message */
    messageInfo = (BIP_RtspSocketMessageInfo *)BKNI_Malloc( sizeof( BIP_RtspSocketMessageInfo ));
    BIP_CHECK_PTR_GOTO( messageInfo, "Memory Allocation Failed", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );

    BKNI_Memset( messageInfo, 0, sizeof( BIP_RtspSocketMessageInfo ));

    /* Allocate space for copying the message, 1 extra byte for storing the null char (makes it a string) */
    messageInfo->pBuffer = (char *)BKNI_Malloc( messageLength + 1 );
    BIP_CHECK_PTR_GOTO( messageInfo->pBuffer, "Message Buffer Allocation Failed", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );
    BKNI_Memset( messageInfo->pBuffer, 0, messageLength + 1 );

    /* Copy the actual message */
    messageInfo->bufferLength = messageLength;
    BIP_RtspLiveMediaSocket_CopyMessage( hRtspSocket->hRtspLmSocket, messageInfo->pBuffer );
    messageInfo->pBuffer[messageLength] = '\0';

    /* Also, cache a pointer to the rtspSocket handle on which this message was received. This enables us to send the response back on the same socket */
    messageInfo->hRtspSocket = hRtspSocket;

    BDBG_MSG(( BIP_MSG_PRE_FMT "messageLength %d; buffer (%s)" BIP_MSG_PRE_ARG, messageInfo->bufferLength, messageInfo->pBuffer ));
    /* Queue this new message to the list of messages pending for app to receive */
    BKNI_AcquireMutex( hRtspSocket->lock );
    BLST_Q_INSERT_TAIL( &hRtspSocket->pendingMessageInfoListHead, messageInfo, pendingMessageInfoListNext );
    hRtspSocket->state = BIP_RtspSocketState_eMessagePending;
    BKNI_ReleaseMutex( hRtspSocket->lock );
    messageInfo = NULL;

    if (hRtspSocket->settings.messageReceivedCallback.callback)
    {
        hRtspSocket->settings.messageReceivedCallback.callback( hRtspSocket->settings.messageReceivedCallback.context, hRtspSocket->settings.messageReceivedCallback.param );
    }
    return;

error:
    if (!messageInfo)
    {
        return;
    }

    if (messageInfo->pBuffer)
    {
        BKNI_Free( messageInfo->pBuffer );
    }
    BKNI_Free( messageInfo );
} /* rtspSocketMessageReceivedCallback */

static void rtspSocketErrorCallback(
    void *context,
    int   socketError
    )
{
    BIP_RtspSocketHandle hRtspSocket = (BIP_RtspSocketHandle)context;

    BDBG_MSG(( BIP_MSG_PRE_FMT "hRtspSocket %p, socketError %d, socketState %d" BIP_MSG_PRE_ARG, (void *)hRtspSocket, socketError, hRtspSocket->state ));
    BDBG_OBJECT_ASSERT( hRtspSocket, BIP_RtspSocket );

    BKNI_AcquireMutex( hRtspSocket->lock );
    hRtspSocket->state = BIP_RtspSocketState_eError;
    BKNI_ReleaseMutex( hRtspSocket->lock );

    if (hRtspSocket->settings.errorCallback.callback)
    {
        hRtspSocket->settings.errorCallback.callback( hRtspSocket->settings.errorCallback.context, socketError );
    }
} /* rtspSocketErrorCallback */

BIP_Status BIP_RtspSocket_SetSettings(
    BIP_RtspSocketHandle    hRtspSocket,
    BIP_RtspSocketSettings *pSettings
    )
{
    BIP_Status errCode = BIP_SUCCESS;

    BDBG_OBJECT_ASSERT( hRtspSocket, BIP_RtspSocket );
    BDBG_ASSERT( pSettings );
    BIP_RtspLiveMediaSocketSettings lmSocketSettings;

    /* TODO: Validate parameters */
    hRtspSocket->settings = *pSettings;

    /* Set callback with the rtsp lm socket */
    BIP_RtspLiveMediaSocket_GetSettings( hRtspSocket->hRtspLmSocket, &lmSocketSettings );
    lmSocketSettings.messageReceivedCallback.callback = rtspSocketMessageReceivedCallback;
    lmSocketSettings.messageReceivedCallback.context  = hRtspSocket;
    lmSocketSettings.errorCallback.callback           = rtspSocketErrorCallback;
    lmSocketSettings.errorCallback.context            = hRtspSocket;
    BIP_RtspLiveMediaSocket_SetSettings( hRtspSocket->hRtspLmSocket, &lmSocketSettings );

    return( errCode );
} /* BIP_RtspSocket_SetSettings */

/**
 * Summary:
 * Rtsp Socket API to recv a RTSP Message on RTSP Socket
 **/
BIP_Status BIP_RtspSocket_RecvRequest(
    BIP_RtspSocketHandle  hRtspSocket,
    BIP_RtspRequestHandle hRtspRequest
    )
{
    BIP_RtspSocketMessageInfo *messageInfo;

    BDBG_OBJECT_ASSERT( hRtspSocket, BIP_RtspSocket );

    /* Get the next message request from the list of pending messages (which were queued up during the receiveMessageCallback) on this socket ctx */
    BKNI_AcquireMutex( hRtspSocket->lock );

    /* Check if this socket any error set via the lower LM layer */
    if (hRtspSocket->state == BIP_RtspSocketState_eError)
    {
        /**
         * Since got an error on this socket, we wont be able to send out the response on it.
         * So purge all pending messages on this socket and then return error to caller.
         **/
        BDBG_MSG(( BIP_MSG_PRE_FMT "Got Error on hRtspSocket %p" BIP_MSG_PRE_ARG, (void *)hRtspSocket )); /* this happens when socket closes */
        destroyPendingMessageInfoList( hRtspSocket );
        BKNI_ReleaseMutex( hRtspSocket->lock );
        return( BIP_ERR_OS_CHECK_ERRNO );
    }

    /* Now take the first message off the pendingMessageInfoList */
    messageInfo = BLST_Q_FIRST( &hRtspSocket->pendingMessageInfoListHead );
    if (messageInfo)
    {
        BLST_Q_REMOVE_HEAD( &hRtspSocket->pendingMessageInfoListHead, pendingMessageInfoListNext );
    }
    else
    {
        /* no more pending messages on this socket, so return to idle state */
        hRtspSocket->state = BIP_RtspSocketState_eIdle;
    }
    BKNI_ReleaseMutex( hRtspSocket->lock );
    if (!messageInfo)
    {
        return( BIP_ERR_NOT_AVAILABLE );
    }

    /* Cache pointers to the actual request buffer & rtspSocket */
    BDBG_MSG(( BIP_MSG_PRE_FMT "calling BIP_RtspRequest_SetBuffer(%s)" BIP_MSG_PRE_ARG, messageInfo->pBuffer ));
    BIP_RtspRequest_SetBuffer( hRtspRequest, messageInfo->pBuffer, messageInfo->bufferLength );
    BIP_RtspRequest_SetSocket( hRtspRequest, messageInfo->hRtspSocket );

    /* TODO: rtspRequest is now setup, so free up the message info or should we keep these objects into a free-list ? */
    BKNI_Free( messageInfo );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hRtspSocket %p, hRtspRequest %p" BIP_MSG_PRE_ARG, (void *)hRtspSocket, (void *)hRtspRequest ));
    return( BIP_SUCCESS );
} /* BIP_RtspSocket_RecvRequest */

BIP_RtspSessionHandle BIP_RtspSocket_CreateSession(
    BIP_RtspSocketHandle  hRtspSocket,
    BIP_RtspRequestHandle request
    )
{
    BIP_RtspSessionHandle hRtspSession = NULL;

    hRtspSession = BIP_RtspSession_CreateFromRequest( BIP_RtspRequest_GetBuffer( request ), hRtspSocket->hRtspLmSocket, NULL );

    return( hRtspSession );
}

#if 0
BIP_Status BIP_RtspSocket_RecvResponse( BIP_RtspSocketHandle socket, BIP_RtspResponseHandle rtspResponse );

/**
 * Summary:
 * Rtsp Socket API to send a RTSP Message on RTSP Socket
 **/
BIP_Status BIP_RtspSocket_SendRequest( BIP_RtspSocketHandle socket, BIP_RtspRequestHandle hRtspRequest );
BIP_Status BIP_RtspSocket_SendResponse( BIP_RtspSocketHandle socket, BIP_RtspResponseHandle rtspResponse );

#endif /* if 0 */
