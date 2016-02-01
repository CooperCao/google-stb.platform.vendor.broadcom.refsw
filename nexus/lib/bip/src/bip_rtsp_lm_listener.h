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

#ifndef BIP_RTSP_LIVEMEDIA_LISTENER_H
#define BIP_RTSP_LIVEMEDIA_LISTENER_H

#ifdef __cplusplus
extern "C" {
#endif
#include "bip_igmp_listener.h"


typedef struct BIP_RtspLiveMediaListener *BIP_RtspLiveMediaListenerHandle;

typedef struct BIP_RtspLiveMediaListenerCreateSettings
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
} BIP_RtspLiveMediaListenerCreateSettings;

/**
 * Summary:
 * Get Default RtspListener settings
 **/
void BIP_RtspLiveMediaListener_GetDefaultCreateSettings( BIP_RtspLiveMediaListenerCreateSettings *pSettings );

/**
 * Summary:
 * API to create listener for incoming RTSP requests
 *
 * Description:
 **/
BIP_RtspLiveMediaListenerHandle BIP_RtspLiveMediaListener_Create( BIP_RtspLiveMediaListenerCreateSettings *pSettings );

typedef struct BIP_RtspLiveMediaListenerSettings
{
    BIP_CallbackDesc connectedCallback; /* optional: for event driven mode */
    BIP_CallbackDesc getRtpStatisticsCallback;
} BIP_RtspLiveMediaListenerSettings;

/**
 * Summary:
 * API to Get Listener Settings
 *
 * Description:
 **/
void BIP_RtspLiveMediaListener_GetSettings( BIP_RtspLiveMediaListenerHandle listener, BIP_RtspLiveMediaListenerSettings *pSettings );

/**
 * Summary:
 * API to Set Listener Settings
 *
 * Description:
 **/
BIP_Status BIP_RtspLiveMediaListener_SetSettings( BIP_RtspLiveMediaListenerHandle listener, BIP_RtspLiveMediaListenerSettings *pSettings );

/**
 * Summary:
 * API to Start/Stop the listener
 *
 * Description:
 **/
BIP_Status BIP_RtspLiveMediaListener_Start( BIP_RtspLiveMediaListenerHandle listener /*, BIP_RtspLiveMediaListenerStartSettings *pSettings*/ );
void      BIP_RtspLiveMediaListener_Stop( BIP_RtspLiveMediaListenerHandle listener );

/**
 * Summary:
 * API to create new RTSP Socket from a RTSP Listener
 *
 * Description:
 *
 * This API accepts a new connection from the listener socket and wraps it as new RtspSocket object
 *
 **/
BIP_RtspLiveMediaSocketHandle BIP_RtspLiveMediaListener_CreateSocket( BIP_RtspLiveMediaListenerHandle listener, int socketFd );

/**
 * Summary:
 * Destroy RtspListener
 *
 * Description:
 **/
void BIP_RtspLiveMediaListener_Destroy( BIP_RtspLiveMediaListenerHandle listener );

#ifdef __cplusplus
}
#endif

#endif /* BIP_RTSP_LIVEMEDIA_LISTENER_H */
