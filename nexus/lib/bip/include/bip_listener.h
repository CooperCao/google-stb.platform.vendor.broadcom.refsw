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

#ifndef BIP_LISTENER_H
#define BIP_LISTENER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bip.h"
#include "bip_types.h"
#include "bip_socket.h"

typedef struct BIP_Listener *BIP_ListenerHandle;

/**
 * Summary:
 * BIP Listener Class to receive & accept incoming connection requests.
 *
 * Description:
 * Abstraction of BSD Socket APIs to allow caller to create a listener type socket and receive new connections on it.
 **/

/**
 * Summary:
 * Listener create settings. Dummy settings for now.
 **/
typedef struct BIP_ListenerCreateSettings
{
    unsigned unused;
    /* Dummy */
} BIP_ListenerCreateSettings;

/**
 * Summary:
 * Listener Start Settings.
 **/
typedef struct BIP_ListenerStartSettings
{
    const char *pPort;                          /* port # to bind to */
    const char *pInterfaceName;                 /* optional: interface name to bind this listener to. */
    BIP_NetworkAddressType ipAddressType;       /* if pInterface is defined, then ipAddressType allows caller */
                                                /* to specify its perference about using the IP Address associated with this interface. */
    int queueDepth;                             /* 16 vs 32 max outstanding connections  */
} BIP_ListenerStartSettings;

/**
 * Summary:
 * Listener  settings.
 **/
typedef struct BIP_ListenerSettings
{
    BIP_CallbackDesc connectedCallback;       /* callback to indicate the next connection request from client has arrived */
    BIP_CallbackDesc errorCallback;           /* callback to indicate error on listening socket */
    BIP_CallbackDesc asyncAcceptedCallback;   /* callback to indicate that an asynch BIP_Listener_Accept has completed */
} BIP_ListenerSettings;

/**
 * Summary:
 * API to get Listener default Create Settings.
 **/
void BIP_Listener_GetDefaultCreateSettings( BIP_ListenerCreateSettings *pSettings );


/**
 * Summary:
 * API to create BIP Listener object.
 *
 * Description:
 *
 * This API creates a listening socket for receiving connection requests on a stream based TCP socket.
 *
 *  Input:  Null or Dummy Struct
 *  Will have a NULL passed in or have a Dummy Settings for now, In case we want to change
 *  This will just malloc the Listener handle. mutex, internal list
 *  Set the default settings here.  store in in handle.
 **/
BIP_ListenerHandle BIP_Listener_Create( const BIP_ListenerCreateSettings *pSettings );

/**
 * Summary:
 * API to get Listener default Settings.
 **/
void BIP_Listener_GetDefaultSettings(  BIP_ListenerSettings *pSettings );

/**
 * Summary:
 * API to Set Settings  the listener
 *
 * Description:
 *
 *  This will allow some Listener settings to change during run time.
 *  Note  that certain settings need the Listener to Stop, set new settings and then Start.
 *
 **/
BIP_Status BIP_Listener_SetSettings( BIP_ListenerHandle listener, BIP_ListenerSettings *pSettings  );

/**
 * Summary:
 * API to Set Settings  the listener
 *
 * Description:
 *
 *  This will allow some Listener settings to change during run time. As well we need to put a note
 * that certain settings need the Listener to Stop, set settings, start
 *
 **/
void BIP_Listener_GetSettings( BIP_ListenerHandle listener, BIP_ListenerSettings *pSettings  );

/**
 * Summary:
 * API to get Listener default Start Settings.
 **/
void BIP_Listener_GetDefaultStartSettings(  BIP_ListenerStartSettings *pSettings );

/**
 * Summary:
 * API to Start listener
 *
 * Description:
 *
 * This API indicates io thread to start monitoring this listener for new connection requests .
 *  Will do the socket(),  bind(), and Listen () calls
 * As well as Add to IOchecker
 *
 **/
BIP_Status BIP_Listener_Start( BIP_ListenerHandle listener, BIP_ListenerStartSettings *pSettings);

/**
 * Summary:
 * API to Stop the listener
 *
 * Description:
 *
 * This API indicates io thread to stop monitoring this listener for new connection requests.
 * Caller can use this to suspend receiving any new connections.
 * Flow:  Disable and Remove from IO checker AND  close socket ( must remove from kernel list, otherwise clients will can
 * connect and will  be sitting there).
 *
 **/
BIP_Status BIP_Listener_Stop( BIP_ListenerHandle listener);

/**
 * Summary:
 * API to Accept new connection on the listener
 *
 * Description:
 *
 * After creator of BIP_Listener object gets the connectedCallback, it can invoke this API to accept the new connection.
 * API returns the BIP_Socket from the socketFd value associated with the newly accepted/created socket to the caller.
 *
 * Note on usage of timeout parameter:
 * timeout: 0 -> non-blocking, calls accept in the non-blocking mode and returns the result, set socket non blocking, or use a select (1 call), or use  iochecker simple Poll <<.
 * timeout: value in msec -> blocking with timeout, return when new connection request received or timeout expires,based on a dataready callback from IOchecker.
 * timeout: -1 -> blocks until a new connection request is received. request io checker, set the event in the callback(connected callback, same event come in through destroy and stop, then check flag that it was from destory)
 *
 **/
    BIP_SocketHandle BIP_Listener_Accept( BIP_ListenerHandle listener, int timeout);

/**
 * Summary:
 * Destroy Listener
 *
 * Description:
 *
 * If BIP_Listener_Stop is not called, it will be implicit called before destroying. Free Listener Handle.
 **/
void BIP_Listener_Destroy( BIP_ListenerHandle listener );

/**
 * Summary:
 * Listener Status
 **/
typedef struct BIP_ListenerStatus
{
    int socketFd; /* associated with the listener socket */
    unsigned numConnectionsPending;
    unsigned numConnectionsAccepted;
    /* TODO: add more fields: local & remote ip address, interface name, socket type, etc. */
} BIP_ListenerStatus;

/**
 * Summary:
 * Get BIP Listener Status
 **/
BIP_Status BIP_Listener_GetStatus( BIP_ListenerHandle listener, BIP_ListenerStatus *pStatus );

/* ********************************************************************************************** */
#ifdef __cplusplus
}
#endif

#endif /* BIP_LISTENER_H */
