/******************************************************************************
 * (c) 2007-2015 Broadcom Corporation
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

#ifndef BIP_RTSP_SESSION_H
#define BIP_RTSP_SESSION_H

#ifdef __cplusplus
extern "C" {
#endif

#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#endif
#include "bip_streamer.h"

typedef struct BIP_RtspSession *BIP_RtspSessionHandle;

/* TODO: see if we need to have createSettings in this call, or just two of these handles is sufficient */
typedef struct BIP_RtspSessionCreateSettings
{
    int   port;      /* required */
    char *ipAddress; /* optional: bind to this specific address and then listen on it */
    /* add connection timeout ? */
    bool enableSesSatIpFeatures;
    /* TODO: should we have a Ses SatIp specific structure: deviceId, multicastAddress, */
    /* define all SAT>IP specific settings */
    /* check if we should make them generic configurable settings, this way they become a configurable option for a generic RTSP server. */
} BIP_RtspSessionCreateSettings;

/**
 * Summary:
 * Get Default RtspSession settings
 **/
void BIP_RtspSession_GetDefaultCreateSettings( BIP_RtspSessionCreateSettings *pSettings );
void BIP_RtspSession_GetDefaultSesSatIpCreateSettings( BIP_RtspSessionCreateSettings *pSettings );

/**
 * Summary:
 * API to create session for incoming RTSP requests
 *
 * Description:
 **/
BIP_RtspSessionHandle BIP_RtspSession_CreateFromRequest( char *pRequest, void *pRtspLmSocket, BIP_RtspSessionCreateSettings *pSettings );

typedef struct BIP_RtspSessionSettings
{
    BIP_CallbackDesc messageReceivedCallback;           /* optional: for event driven mode */
    BIP_CallbackDesc igmpMembershipReportEventCallback; /* optional: for event driven mode */
} BIP_RtspSessionSettings;

/**
 * Summary:
 * API to Get Session Settings
 *
 * Description:
 **/
void BIP_RtspSession_GetSettings( BIP_RtspSessionHandle session, BIP_RtspSessionSettings *pSettings );

/**
 * Summary:
 * API to Set Session Settings
 *
 * Description:
 **/
BIP_Status BIP_RtspSession_SetSettings( BIP_RtspSessionHandle session, BIP_RtspSessionSettings *pSettings );

/**
 * Summary:
 * Rtsp Session API to recv a RTSP Message on RTSP Session
 **/
BIP_Status BIP_RtspSession_RecvRequest( BIP_RtspSessionHandle rtspSession, BIP_RtspRequestHandle rtspRequest );

/**
 * Summary:
 * API to send response for previous request on RTSP Session
 **/
BIP_Status BIP_RtspSession_SendResponse( BIP_RtspSessionHandle rtspSession, BIP_RtspResponseHandle rtspResponse );

/**
 * Summary:
 * API to report lock status to RTSP session
 **/
BIP_Status BIP_RtspSession_ReportLockStatus( BIP_RtspSessionHandle  hRtspSession, bool bLockStatus );

/**
 * Summary:
 * API to send response for a given request on RTSP Session
 **/
BIP_Status BIP_RtspSession_SendResponseUsingRequest( BIP_RtspSessionHandle rtspSession, BIP_RtspResponseHandle rtspResponse, BIP_RtspRequestHandle rtspRequest );

#if NEXUS_HAS_FRONTEND
/**
 * Summary:
 * API to Get SatelliteSettings with this session
 **/
BIP_Status BIP_RtspSession_ParseSatelliteSettings(
    BIP_RtspSessionHandle rtspSession,
    NEXUS_FrontendSatelliteSettings *pSatelliteSettings,
    NEXUS_FrontendDiseqcSettings *pDiseqcSettings,
    BIP_AdditionalSatelliteSettings *pAddSatelliteSettings
    );
#endif

/**
 * Summary:
 * API to Get IGMP Event status: which event is it?
 *
 * Description:
 * Returns current IGMP event status on a RTSP session. If there is no current IGMP event on this rtsp session, it returns error. TODO  change to  igmp event.
 **/
typedef enum
{
    BIP_RtspIgmpMemRepStatus_eNone,
    BIP_RtspIgmpMemRepStatus_eJoin,
    BIP_RtspIgmpMemRepStatus_eLeave,
    BIP_RtspIgmpMemRepStatus_eInvalid
} BIP_RtspIgmpMemRepStatus;

void BIP_RtspSession_GetIgmpStatus( BIP_RtspSessionHandle rtspSession, BIP_RtspIgmpMemRepStatus *igmpEventStatus );


/**
 * Summary:
 * API to Start Streaming
 **/
void      BIP_RtspSession_GetDefaultStartStreamerSettings( BIP_StreamerTunerInputSettings *pStreamerInputSettings );
BIP_Status BIP_RtspSession_StartStreamer( BIP_RtspSessionHandle rtspSession, BIP_StreamerTunerInputSettings *pStreamerInputSettings );

/**
 * Summary:
 * API to Stop Streaming
 **/
BIP_Status BIP_RtspSession_StopStreamer( BIP_RtspSessionHandle rtspSession );

/**
 * Summary:
 * API to Get Session related Transport Status: Port, IP address, etc.
 **/
BIP_Status BIP_RtspSession_GetTransportStatus( BIP_RtspSessionHandle rtspSession, BIP_RtspTransportStatus *pTransportStatus );
BIP_Status BIP_RtspSession_GetPids( BIP_RtspSessionHandle rtspSession, BIP_PidInfo *pPids );

/**
 * Summary:
 * Destroy RtspSession
 *
 * Description:
 **/
void BIP_RtspSession_Destroy( BIP_RtspSessionHandle session );

/**
 * Summary:
 * API to set the socket Interface name associated with this socket
 *
 * Description:
 **/
BIP_Status BIP_RtspSession_SetInterfaceName( BIP_RtspSessionHandle hRtspSession, char *pInterfaceName );
#ifdef __cplusplus
}
#endif

#endif /* BIP_RTSP_SESSION_H */
