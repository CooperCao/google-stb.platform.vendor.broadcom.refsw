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

#ifndef BIP_RTSP_LISTENER_H
#define BIP_RTSP_LISTENER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BIP_RtspListener *BIP_RtspListenerHandle;

#include "bip_igmp_listener.h"
typedef struct BIP_RtspListenerCreateSettings
{
    int   port;      /* RTSP Listener port #, default is 554 */
    char *ipAddress; /* optional: bind to this specific address and then listen on it */
    int   rtspSessionTimeout;      /* in seconds, RTSP session is timedout if there is no heartbeat from client in this interval, default is 60sec */
    /* add connection timeout ? */
    bool enableSesSatIpFeatures;
    /* TODO: should we have a Ses SatIp specific structure: deviceId, multicastAddress, */
    /* define all SAT>IP specific settings */
    /* check if we should make them generic configurable settings, this way they become a configurable option for a generic RTSP server. */
    BIP_IgmpListenerHandle hIgmpListener; /* optional: app should create IgmpListener & pass it here if it wants RtspListener to also monitor IGMP (multicast) messages */
} BIP_RtspListenerCreateSettings;

/**
 * Summary:
 * Get Default RtspListener settings
 **/
void BIP_RtspListener_GetDefaultCreateSettings( BIP_RtspListenerCreateSettings *pSettings );
void BIP_RtspListener_GetDefaultSesSatIpCreateSettings( BIP_RtspListenerCreateSettings *pSettings );

/**
 * Summary:
 * API to create listener for incoming RTSP requests
 *
 * Description:
 **/
BIP_RtspListenerHandle
BIP_RtspListener_Create(
    BIP_RtspListenerCreateSettings *pSettings /* optional, can be NULL */
    );

typedef struct BIP_RtspListenerSettings
{
    BIP_CallbackDesc messageReceivedCallback; /* callback to indicate the next RTSP message from any client */
} BIP_RtspListenerSettings;

/**
 * Summary:
 * API to Get Listener Settings
 *
 * Description:
 **/
void BIP_RtspListener_GetSettings( BIP_RtspListenerHandle listener, BIP_RtspListenerSettings *pSettings );

/**
 * Summary:
 * API to Set Listener Settings
 *
 * Description:
 **/
BIP_Status BIP_RtspListener_SetSettings( BIP_RtspListenerHandle listener, BIP_RtspListenerSettings *pSettings );

/**
 * Summary:
 * API to Start/Stop the listener
 *
 * Description:
 **/
BIP_Status BIP_RtspListener_Start( BIP_RtspListenerHandle listener /*, BIP_RtspListenerStartSettings *pSettings*/ );
void      BIP_RtspListener_Stop( BIP_RtspListenerHandle listener );

/**
 * Summary:
 * Rtsp Listener API to recv a RTSP Message on RTSP Listener
 **/
BIP_Status BIP_RtspListener_RecvRequest( BIP_RtspListenerHandle rtspListener, BIP_RtspRequestHandle rtspRequest );

/**
 * Summary:
 * Rtsp Listener API to create a new RTSP Session for a given RTSP Request
 **/
BIP_RtspSessionHandle BIP_RtspListener_CreateSession( BIP_RtspListenerHandle rtspListener, BIP_RtspRequestHandle request );

/**
 * Summary:
 * Destroy RtspListener
 *
 * Description:
 **/
void BIP_RtspListener_Destroy( BIP_RtspListenerHandle listener );

#ifdef __cplusplus
}
#endif

#endif /* BIP_RTSP_LISTENER_H */
