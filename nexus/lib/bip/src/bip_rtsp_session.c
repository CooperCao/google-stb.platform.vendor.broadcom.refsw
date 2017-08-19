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
#include "bip_rtsp_session_impl.h"
#include <string.h>
#include <stdio.h>

BDBG_MODULE( bip_rtsp_session );
BDBG_OBJECT_ID( BIP_RtspSession );

void BIP_RtspSession_GetDefaultCreateSettings(
    BIP_RtspSessionCreateSettings *pSettings
    )
{
    BKNI_Memset( pSettings, 0, sizeof( BIP_RtspSessionCreateSettings ));
}

static void destroyPendingMessageInfoList(
    BIP_RtspSessionHandle hRtspSession
    )
{
    BIP_RtspSessionMessageInfo *messageInfo;

    if (!hRtspSession) {return; }

    while (( messageInfo = BLST_Q_FIRST( &hRtspSession->pendingMessageInfoListHead )) != NULL)
    {
        BLST_Q_REMOVE( &hRtspSession->pendingMessageInfoListHead, messageInfo, pendingMessageInfoListNext );
        if (messageInfo->pBuffer) {BKNI_Free( messageInfo->pBuffer ); messageInfo->pBuffer =0; }
        BKNI_Free( messageInfo );
    }
}

static void rtspSessionDestroy(
    BIP_RtspSessionHandle hRtspSession
    )
{
    if (!hRtspSession)
    {
        return;
    }

    /* StopStreamer incase its not stopped */
    BIP_RtspSession_StopStreamer( hRtspSession );
    if (hRtspSession->dataReadyEvent) {BKNI_DestroyEvent( hRtspSession->dataReadyEvent ); hRtspSession->dataReadyEvent=0; }
    if (hRtspSession->hRtspLmSession) {BIP_RtspLiveMediaSession_Destroy( hRtspSession->hRtspLmSession ); hRtspSession->hRtspLmSession=NULL; }
    if (hRtspSession->lock) {BKNI_DestroyMutex( hRtspSession->lock ); hRtspSession->lock =0; }
    if (hRtspSession->pInterfaceName) {BKNI_Free( hRtspSession->pInterfaceName ); hRtspSession->pInterfaceName =0; }

    destroyPendingMessageInfoList( hRtspSession );
    BDBG_OBJECT_DESTROY( hRtspSession, BIP_RtspSession );
    BKNI_Free( hRtspSession );
} /* rtspSessionDestroy */

BIP_RtspSessionHandle BIP_RtspSession_CreateFromRequest(
    char                          *requestStr,
    void                          *lmSocket,
    BIP_RtspSessionCreateSettings *pSettings
    )
{
    int                           rc;
    BIP_RtspSessionHandle         hRtspSession = NULL;
    BIP_RtspSessionCreateSettings defaultSettings;
    BIP_RtspLiveMediaSocketHandle rtspLmSocket = (BIP_RtspLiveMediaSocketHandle)lmSocket;

    /* Create the hRtspSession object */
    hRtspSession = BKNI_Malloc( sizeof( *hRtspSession ));
    BIP_CHECK_PTR_GOTO( hRtspSession, "Memory Allocation Failed", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );
    BKNI_Memset( hRtspSession, 0, sizeof( *hRtspSession ));
    BDBG_OBJECT_SET( hRtspSession, BIP_RtspSession );

    if (NULL == pSettings)
    {
        BIP_RtspSession_GetDefaultCreateSettings( &defaultSettings );
        pSettings = &defaultSettings;
    }
    hRtspSession->createSettings = *pSettings;

    /* create liveMedia RtspSession object */
    hRtspSession->hRtspLmSession = BIP_RtspLiveMediaSocket_CreateSession( rtspLmSocket, requestStr );
    BIP_CHECK_PTR_GOTO( hRtspSession->hRtspLmSession, "Memory Allocation Failed", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );

    rc = BKNI_CreateMutex( &hRtspSession->lock );
    BIP_CHECK_ERR_NZ_GOTO( rc, "BKNI_CreateMutex() Failed", error );

    rc = BKNI_CreateEvent( &hRtspSession->dataReadyEvent );
    BIP_CHECK_ERR_NZ_GOTO( rc, "BKNI_CreateEvent() Failed", error );

    BDBG_MSG(( "%s: hRtspSession %p", BSTD_FUNCTION, (void *)hRtspSession ));
    return( hRtspSession );

error:
    rtspSessionDestroy( hRtspSession );

    return( NULL );
} /* BIP_RtspSession_CreateFromRequest */

/**
 * Summary:
 * Destroy rtsp session
 *
 * Description:
 **/
void BIP_RtspSession_Destroy(
    BIP_RtspSessionHandle hRtspSession
    )
{
    BDBG_OBJECT_ASSERT( hRtspSession, BIP_RtspSession );
    BDBG_MSG(( "%s: hRtspSession %p", BSTD_FUNCTION, (void *)hRtspSession ));
    rtspSessionDestroy( hRtspSession );
}

void BIP_RtspSession_GetSettings(
    BIP_RtspSessionHandle    hRtspSession,
    BIP_RtspSessionSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT( hRtspSession, BIP_RtspSession );
    BDBG_ASSERT( pSettings );
    *pSettings = hRtspSession->settings;
}

/* TODO: this is just temporary for proof-of-concept. It will be replaced by placing each of the new sessions into a list of connection objects */
static void rtspMessageReceivedCallback(
    void *context,
    int   messageLength
    )
{
    BIP_RtspSessionHandle hRtspSession = (BIP_RtspSessionHandle)context;
    BIP_RtspSessionMessageInfo      *messageInfo  = NULL;

    BDBG_OBJECT_ASSERT( hRtspSession, BIP_RtspSession );
    BDBG_ASSERT( messageLength );
    BDBG_MSG(( "%s: hRtspSession %p, messageLength %d", BSTD_FUNCTION, (void *)hRtspSession, messageLength ));

    /* Create new messageInfo entry to hold this message */
    messageInfo = (BIP_RtspSessionMessageInfo *)BKNI_Malloc( sizeof( BIP_RtspSessionMessageInfo ));
    BIP_CHECK_PTR_GOTO( messageInfo, "Memory Allocation Failed", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );
    BKNI_Memset( messageInfo, 0, sizeof( BIP_RtspSessionMessageInfo ));

    /* Allocate space for copying the message, 1 extra byte for storing the null char (makes it a string) */
    messageInfo->pBuffer = (char *)BKNI_Malloc( messageLength + 1 );
    BIP_CHECK_PTR_GOTO( messageInfo, "Message Buffer Allocation Failed", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );
    BKNI_Memset( messageInfo->pBuffer, 0, messageLength + 1 );

    /* Copy the actual message */
    messageInfo->bufferLength = messageLength;
    BDBG_MSG(( "%s: calling BIP_RtspLiveMediaSession_CopyMessage() -> (%p)", BSTD_FUNCTION, (void *)messageInfo->pBuffer ));
    BIP_CHECK_PTR_GOTO( hRtspSession->hRtspLmSession, "hRtspSession->hRtspLmSession NULL", error, BIP_ERR_INVALID_PARAMETER );
    BIP_RtspLiveMediaSession_CopyMessage( hRtspSession->hRtspLmSession, messageInfo->pBuffer );
    messageInfo->pBuffer[messageLength] = '\0';
    BDBG_MSG(( "%s: messageLength %d; message (%s)(%p); len %zu", BSTD_FUNCTION, messageInfo->bufferLength, messageInfo->pBuffer, (void *)messageInfo->pBuffer,
               strlen( messageInfo->pBuffer )));
    BIP_CHECK_ERR_LEZ_GOTO( strlen( messageInfo->pBuffer ), "Message buffer was empty", "", error, BIP_ERR_INVALID_PARAMETER );

    /* Queue this new message to the list of messages pending for app to receive */
    BKNI_AcquireMutex( hRtspSession->lock );
    BLST_Q_INSERT_TAIL( &hRtspSession->pendingMessageInfoListHead, messageInfo, pendingMessageInfoListNext );
    BKNI_ReleaseMutex( hRtspSession->lock );
    messageInfo = NULL;

    /* Indicate to callback thread to invoke the connected callback. */
    /* TODO: for now, we are directly invoking the callback */
    if (hRtspSession->settings.messageReceivedCallback.callback)
    {
        BDBG_MSG(( "%s: calling messageReceivedCallback", BSTD_FUNCTION ));
        hRtspSession->settings.messageReceivedCallback.callback( hRtspSession->settings.messageReceivedCallback.context, hRtspSession->settings.messageReceivedCallback.param );
    }
    return;

error:
    BDBG_MSG(( "%s: message (%p); pBuffer %p", BSTD_FUNCTION, (void *)messageInfo, (void *)messageInfo->pBuffer ));
    fflush( stdout ); fflush( stderr );
    if (messageInfo && messageInfo->pBuffer)
    {
        BKNI_Free( messageInfo->pBuffer );
    }
    if (messageInfo)
    {
        BKNI_Free( messageInfo );
    }
    BDBG_MSG(( "%s: done", BSTD_FUNCTION ));
    return;
} /* rtspMessageReceivedCallback */

/* TODO: this is just temporary for proof-of-concept. It will be replaced by placing each of the new sessions into a list of connection objects */
static void rtspIgmpMembershipReportCallback(
    void *context,
    int   igmpStatus
    )
{
    BIP_RtspSessionHandle hRtspSession = (BIP_RtspSessionHandle)context;

    BDBG_OBJECT_ASSERT( hRtspSession, BIP_RtspSession );
    BDBG_MSG(( "%s: hRtspSession %p", BSTD_FUNCTION, (void *)hRtspSession ));

    BKNI_AcquireMutex( hRtspSession->lock );
    hRtspSession->lastIgmpStatus = (BIP_RtspIgmpMemRepStatus)igmpStatus;
    BKNI_ReleaseMutex( hRtspSession->lock );

    /* Indicate to callback thread to invoke the connected callback. */
    /* TODO: for now, we are directly invoking the callback */
    if (hRtspSession->settings.igmpMembershipReportEventCallback.callback)
    {
        BDBG_MSG(( "%s: calling callback", BSTD_FUNCTION ));
        hRtspSession->settings.igmpMembershipReportEventCallback.callback( hRtspSession->settings.igmpMembershipReportEventCallback.context, hRtspSession->settings.igmpMembershipReportEventCallback.param );
    }
} /* rtspIgmpMembershipReportCallback */

BIP_Status BIP_RtspSession_SetSettings(
    BIP_RtspSessionHandle    hRtspSession,
    BIP_RtspSessionSettings *pSettings
    )
{
    BIP_Status errCode = BIP_SUCCESS;

    BDBG_OBJECT_ASSERT( hRtspSession, BIP_RtspSession );
    BDBG_ASSERT( pSettings );
    BIP_RtspLiveMediaSessionSettings lmSessionSettings;

    /* validate parameters */
    hRtspSession->settings = *pSettings;

    /* TODO: */
    /* add pSettings->connectedCallback to the list of BIP_CallbackDesc callbacks */
    /* callback thread waits on an event to process this callbackDesc list */
    /* it invokes callbacks for Descriptors for whom enable bit is set */

    /* also set callback with the rtsp lm session */
    BIP_RtspLiveMediaSession_GetSettings( hRtspSession->hRtspLmSession, &lmSessionSettings );
    lmSessionSettings.messageReceivedCallback.callback      = rtspMessageReceivedCallback;
    lmSessionSettings.messageReceivedCallback.context       = hRtspSession;
    lmSessionSettings.igmpMembershipReportCallback.callback = rtspIgmpMembershipReportCallback;
    lmSessionSettings.igmpMembershipReportCallback.context  = hRtspSession;
    BIP_RtspLiveMediaSession_SetSettings( hRtspSession->hRtspLmSession, &lmSessionSettings );

    return( errCode );
} /* BIP_RtspSession_SetSettings */

/**
 * Summary:
 * Rtsp Session API to recv a RTSP Message on RTSP Session
 **/
void BIP_RtspSession_GetIgmpStatus(
    BIP_RtspSessionHandle     hRtspSession,
    BIP_RtspIgmpMemRepStatus *rtspIgmpStatus
    )
{
    BDBG_OBJECT_ASSERT( hRtspSession, BIP_RtspSession );

    BKNI_AcquireMutex( hRtspSession->lock );
    *rtspIgmpStatus              = hRtspSession->lastIgmpStatus;
    hRtspSession->lastIgmpStatus = BIP_RtspIgmpMemRepStatus_eNone; /* Clear status */
    BKNI_ReleaseMutex( hRtspSession->lock );
} /* BIP_RtspSession_GetIgmpStatus */

/**
 * Summary:
 * Rtsp Session API to recv a RTSP Message on RTSP Session
 **/
BIP_Status BIP_RtspSession_RecvRequest(
    BIP_RtspSessionHandle hRtspSession,
    BIP_RtspRequestHandle hRtspRequest
    )
{
    BIP_RtspSessionMessageInfo *messageInfo;

    BDBG_OBJECT_ASSERT( hRtspSession, BIP_RtspSession );

    /* get the next message request from the list of pending messages (which were queued up during the receiveMessageCallback) on this session ctx */
    BKNI_AcquireMutex( hRtspSession->lock );

    /* TODO: think about session lifetime */
    messageInfo = BLST_Q_FIRST( &hRtspSession->pendingMessageInfoListHead );
    if (messageInfo)
    {
        BLST_Q_REMOVE_HEAD( &hRtspSession->pendingMessageInfoListHead, pendingMessageInfoListNext );
    }
    BKNI_ReleaseMutex( hRtspSession->lock );
    if (!messageInfo)
    {
        return( BIP_ERR_NOT_AVAILABLE );
    }
    BIP_RtspRequest_SetBuffer( hRtspRequest, messageInfo->pBuffer, messageInfo->bufferLength );

    BDBG_MSG(( "%s: hRtspSession %p, hRtspRequest %p", BSTD_FUNCTION, (void *)hRtspSession, (void *)hRtspRequest ));

    /* TODO: free up the message info or should we keep these objects into a free-list ? */
    BKNI_Free( messageInfo );

    return( BIP_SUCCESS );
} /* BIP_RtspSession_RecvRequest */

/**
 * Summary:
 * API to send RTSP response for previous request on a session
 **/
BIP_Status BIP_RtspSession_SendResponse(
    BIP_RtspSessionHandle  hRtspSession,
    BIP_RtspResponseHandle hRtspResponse
    )
{
    BIP_Status              errCode        = BIP_SUCCESS;
    BIP_RtspResponseStatus responseStatus = 0;

    BDBG_OBJECT_ASSERT( hRtspSession, BIP_RtspSession );

    /* validate parameters */
    if (BIP_RtspResponse_StatusValid( hRtspResponse ) == false)
    {
        BIP_RtspResponse_GetStatus( hRtspResponse, &responseStatus );
        BDBG_MSG(( "%s: Response Status (%d) is not set", BSTD_FUNCTION, responseStatus ));
        return( BIP_ERR_INVALID_PARAMETER );
    }
    BIP_RtspResponse_GetStatus( hRtspResponse, &responseStatus );
    BDBG_MSG(( "%s: responseStatus %x", BSTD_FUNCTION, responseStatus ));
    BIP_RtspLiveMediaSession_SendResponse( hRtspSession->hRtspLmSession, responseStatus );

    BDBG_MSG(( "%s: hRtspSession %p, hRtspResponse %p", BSTD_FUNCTION, (void *)hRtspSession, (void *)hRtspResponse ));

    return( errCode );
} /* BIP_RtspSession_SendResponse */

/**
 * Summary:
 * API to report lock status to RTSP session
 **/
BIP_Status BIP_RtspSession_ReportLockStatus(
    BIP_RtspSessionHandle  hRtspSession,
    bool                   bLockStatus
    )
{
    BIP_Status              errCode        = BIP_SUCCESS;

    BDBG_OBJECT_ASSERT( hRtspSession, BIP_RtspSession );

    BIP_RtspLiveMediaSession_ReportLockStatus( hRtspSession->hRtspLmSession, bLockStatus);

    BDBG_MSG(( "%s: hRtspSession %p, bLockStatus %d", BSTD_FUNCTION, (void *)hRtspSession, bLockStatus ));

    return( errCode );
} /* BIP_RtspSession_SendResponse */

BIP_Status BIP_RtspSession_SendResponseUsingRequest(
    BIP_RtspSessionHandle  hRtspSession,
    BIP_RtspResponseHandle hRtspResponse,
    BIP_RtspRequestHandle  hRtspRequest
    )
{
    BIP_Status              errCode        = BIP_SUCCESS;
    BIP_RtspResponseStatus responseStatus = BIP_RtspResponseStatus_eInvalid;

    BDBG_OBJECT_ASSERT( hRtspSession, BIP_RtspSession );
    BIP_RtspResponse_GetStatus( hRtspResponse, &responseStatus );
    /* validate parameters */
    if (BIP_RtspResponse_StatusValid( hRtspResponse ) == false)
    {
        BDBG_MSG(( "%s: Response Status (%d) is not set", BSTD_FUNCTION, responseStatus ));
        return( BIP_ERR_INVALID_PARAMETER );
    }
    BIP_RtspLiveMediaSession_SendResponseUsingRequest( hRtspSession->hRtspLmSession, responseStatus, BIP_RtspRequest_GetBuffer( hRtspRequest ));

    BDBG_MSG(( "%s: hRtspSession %p, hRtspRequest %p, hRtspResponse %p", BSTD_FUNCTION, (void *)hRtspSession, (void *)hRtspRequest, (void *)hRtspResponse ));

    return( errCode );
} /* BIP_RtspSession_SendResponseUsingRequest */

#if 0
/* TODO: add these functions */
BIP_Status BIP_RtspSession_RecvResponse( BIP_RtspSessionHandle session, BIP_RtspResponseHandle hRtspResponse );

/**
 * Summary:
 * Rtsp Session API to send a RTSP Message on RTSP Session
 **/
BIP_Status BIP_RtspSession_SendRequest( BIP_RtspSessionHandle session, BIP_RtspRequestHandle hRtspRequest );
BIP_Status BIP_RtspSession_SendResponse( BIP_RtspSessionHandle session, BIP_RtspResponseHandle hRtspResponse );

#endif /* if 0 */
#if NEXUS_HAS_FRONTEND
/**
 * Summary:
 * APIs to parse SesSatIp URL fields from a give RTSP Settings
 **/
BIP_Status BIP_RtspSession_ParseSatelliteSettings(
    BIP_RtspSessionHandle            hRtspSession,
    NEXUS_FrontendSatelliteSettings *pSatelliteSettings,
    NEXUS_FrontendDiseqcSettings    *pDiseqcSettings,
    BIP_AdditionalSatelliteSettings *pAddSatelliteSettings
    )
{
    /* TODO: we call this parse function only for SesSatIp case, we know this flag via Listener settings, need to carry that flag around */
    return( BIP_RtspLiveMediaSession_ParseSatelliteSettings( hRtspSession->hRtspLmSession, pSatelliteSettings, pDiseqcSettings, pAddSatelliteSettings ));
}
#endif
/**
 * Summary:
 * APIs to parse SesSatIp URL fields from a give RTSP Settings
 **/

BIP_Status BIP_RtspSession_GetTransportStatus(
    BIP_RtspSessionHandle    hRtspSession,
    BIP_RtspTransportStatus *pTransportStatus
    )
{
    return( BIP_RtspLiveMediaSession_GetTransportStatus( hRtspSession->hRtspLmSession, pTransportStatus ));
}

BIP_Status BIP_RtspSession_GetPids(
    BIP_RtspSessionHandle hRtspSession,
    BIP_PidInfo          *pPids
    )
{
    return( BIP_RtspLiveMediaSession_GetPids( hRtspSession->hRtspLmSession, pPids ));
}

/**
 * Summary:
 * API to Stop Streaming
 **/
BIP_Status BIP_RtspSession_StopStreamer(
    BIP_RtspSessionHandle hRtspSession
    )
{
    if (hRtspSession->liveStreamingHandle)
    {
        B_PlaybackIp_LiveStreamingStop( hRtspSession->liveStreamingHandle );
        B_PlaybackIp_LiveStreamingClose( hRtspSession->liveStreamingHandle );
        hRtspSession->liveStreamingHandle = NULL;
    }
    return( BIP_SUCCESS );
}

/**
 * Summary:
 * API to Start Streaming. For now use TunerInputSettings. Later we will have a BIP_RtspSession_SetTunerInputSettings
 **/
void BIP_RtspSession_GetDefaultStartStreamerSettings(
    BIP_StreamerTunerInputSettings *pStreamerSettings
    )
{
    memset( pStreamerSettings, 0, sizeof( BIP_StreamerTunerInputSettings ));
}

static void dataReadyCallbackIpDst(
    void *context,
    int   param
    )
{
    BIP_RtspSessionHandle hRtspSession = (BIP_RtspSessionHandle)context;

    BSTD_UNUSED( param );
    BDBG_ASSERT( hRtspSession->dataReadyEvent );
    BKNI_SetEvent( hRtspSession->dataReadyEvent );
} /* dataReadyCallbackIpDst */

BIP_Status BIP_RtspSession_SetInterfaceName(
    BIP_RtspSessionHandle hRtspSession,
    char                 *pInterfaceName
    )
{
    BIP_Status rc;

    hRtspSession->pInterfaceName = BKNI_Malloc(( strlen( pInterfaceName )) + 1 );
    BIP_CHECK_GOTO(( hRtspSession->pInterfaceName ), ( "BKNI Malloc Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    BKNI_Memset( hRtspSession->pInterfaceName, 0, strlen( pInterfaceName ) + 1 );
    BKNI_Memcpy( hRtspSession->pInterfaceName, pInterfaceName, strlen( pInterfaceName ));

    rc = BIP_SUCCESS;
error:
    return( rc );
}

/**
 * Summary:
 * API to Start Streaming
 **/
BIP_Status BIP_RtspSession_StartStreamer(
    BIP_RtspSessionHandle hRtspSession,
    BIP_StreamerTunerInputSettings *pStreamerSettings
    )
{
    BIP_Status                             rc;
    BIP_RtspTransportStatus               transportStatus;
    B_PlaybackIpLiveStreamingOpenSettings liveStreamingSettings;

    BIP_CHECK_GOTO(( hRtspSession->liveStreamingHandle == NULL ), ( "Streamer is already Started, app should Stop it first! " ), error, BIP_ERR_INVALID_PARAMETER, rc );
    BIP_CHECK_GOTO(( pStreamerSettings->hRecpump ), ( "Nexus Recpump handle is NULL" ), error, BIP_ERR_INVALID_PARAMETER, rc );

    rc = BIP_RtspSession_GetTransportStatus( hRtspSession, &transportStatus );
    BIP_CHECK_GOTO(( !rc ), ( "BIP_RtspSession_GetTransportStatus Failed!" ), error, BIP_ERR_INVALID_PARAMETER, rc );

    BDBG_MSG(( "%s: Streaming To %s:%d, isMuticast %d, serverIpStr %s, i/f %s", BSTD_FUNCTION,  transportStatus.clientAddressStr, transportStatus.clientRTPPortNum, transportStatus.isMulticast, transportStatus.serverAddressStr, hRtspSession->pInterfaceName ));

    /* call IP Applib to setup the streaming from Rave buffers */

    memset( &liveStreamingSettings, 0, sizeof( liveStreamingSettings ));
    liveStreamingSettings.rtpUdpSettings.interfaceName = hRtspSession->pInterfaceName;
    if (( transportStatus.streamingMode == BIP_StreamingMode_eRTP_TCP ) || ( transportStatus.streamingMode == BIP_StreamingMode_eRTP_UDP ))
    {
        liveStreamingSettings.protocol = B_PlaybackIpProtocol_eRtp;
    }
    else
    {
        liveStreamingSettings.protocol = B_PlaybackIpProtocol_eUdp;
    }

    liveStreamingSettings.rtpUdpSettings.streamingPort = transportStatus.clientRTPPortNum;
    strncpy( liveStreamingSettings.rtpUdpSettings.streamingIpAddress, transportStatus.clientAddressStr,
        sizeof( liveStreamingSettings.rtpUdpSettings.streamingIpAddress )-1 );

#if 0
    /* TODO: see if we need to get events back from the PBIP streamer, it doesn't seem necessary for RTSP case */
    liveStreamingSettings.eventCallback = ipStreamerEventCallback;
#endif

    liveStreamingSettings.recpumpHandle  = pStreamerSettings->hRecpump;
    liveStreamingSettings.dataReadyEvent = hRtspSession->dataReadyEvent;
    /* Params for Sending Rtp Null packets. This is part of SAT IP spec */
    liveStreamingSettings.sendNullRtpPktsOnTimeout = true;
    liveStreamingSettings.dataReadyTimeoutInterval = 300; /* ms */
    liveStreamingSettings.ipTtl = transportStatus.clientTTL;
    hRtspSession->liveStreamingHandle              = B_PlaybackIp_LiveStreamingOpen( &liveStreamingSettings );
    BIP_CHECK_GOTO(( hRtspSession->liveStreamingHandle ), ( "B_PlaybackIp_LiveStreamingOpen Failed!" ), error, BIP_ERR_INTERNAL, rc );
    /* Setup and start recpump before you start live streaming as it will call get recpump buffer */
    {
        NEXUS_Error           nrc = NEXUS_SUCCESS;
        NEXUS_RecpumpSettings recpumpSettings;
        NEXUS_Recpump_GetSettings( pStreamerSettings->hRecpump, &recpumpSettings );
        recpumpSettings.data.dataReady.callback = dataReadyCallbackIpDst;
        recpumpSettings.data.dataReady.context  = hRtspSession;
        nrc = NEXUS_Recpump_SetSettings( pStreamerSettings->hRecpump, &recpumpSettings );
        BIP_CHECK_GOTO(( !nrc ), ( "NEXUS_Recpump_SetSettings Failed!" ), error, BIP_ERR_INVALID_PARAMETER, rc );
    }
    rc = NEXUS_Recpump_Start( pStreamerSettings->hRecpump );
    BIP_CHECK_GOTO(( !rc ), ( "NEXUS_Recpump_Start Failed" ), error, NEXUS_UNKNOWN, rc );

    rc = B_PlaybackIp_LiveStreamingStart( hRtspSession->liveStreamingHandle );
    BIP_CHECK_GOTO(( !rc ), ( "B_PlaybackIp_LiveStreamingStart Failed!" ), error, BIP_ERR_INTERNAL, rc );


    return( BIP_SUCCESS );

error:
    return( rc );
} /* BIP_RtspSession_StartStreamer */
