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
 ******************************************************************************/

#ifndef BSAGELIB_CLIENT_H_
#define BSAGELIB_CLIENT_H_

#include "bsagelib_tools.h"
#include "bsagelib_interfaces.h"


/* This handle is used to store the context of a SAGElib Client */
typedef struct BSAGElib_P_Client *BSAGElib_ClientHandle;

#include "bsagelib.h"
#include "bsagelib_rpc.h"


#ifdef _cplusplus
extern "C" {
#endif


/*= Module Overview *********************************************************
  Overview: Private.
***************************************************************************/

/* ---------------------------------------- */
/* Configure SAGElib general parameters
 *   register interfaces, ... */

/*
 * Rpc interface:
 *
 * This interface is used to register callbacks that are fired when receiving specific events from the SAGE-side
 * Regular usage is to use BSAGElib_Rpc_IndicationRecvCallback, BSAGElib_Rpc_ResponseRecvCallback, BSAGElib_Rpc_ResponseCallback
 * The extra 'BSAGElib_Rpc_ResponseISRCallback' is a semi-private API and should NOT be used.
 *
 * BSAGElib_RpcInterface::indicationRecv_isr is is fired when receiving an indication
 * BSAGElib_RpcInterface::responseRecv_isr is fired when receiving a response (to a former request)
 * The latest is just a callback to notify upper layer that a response callback is pending processing
 * To fire the associated BSAGElib_RpcInterface::response callback, one should get out from ISR, then
 * make use of BSAGElib_Client_DispatchResponseCallbacks() (in order to process the callbacks into the caller context).
 */


/* Rpc indication callback is fired when a SAGE-->Host indication is received
 * Note: this callback is called under interrupt hence it's treatment should be very quick and non-blocking */
typedef void (*BSAGElib_Rpc_IndicationRecvCallback)(BSAGElib_RpcRemoteHandle handle, void *async_argument, uint32_t indication_id, uint32_t value);

/* Rpc response received callback is fired when a SAGE-->Host response to a former Host-->SAGE request is received
 * Note: this callback is called under interrupt hence it's treatment should be very quick and non-blocking */
typedef void (*BSAGElib_Rpc_ResponseRecvCallback)(BSAGElib_RpcRemoteHandle handle, void *async_argument);

/* Semi-private. Rpc response ISR (can only be used if user API is manipulating SAGElib_Rpc API direcrly)
 * Note: this callback is called under interrupt hence it's treatment should be very quick and non-blocking */
typedef void (*BSAGElib_Rpc_ResponseISRCallback)(BSAGElib_RpcRemoteHandle handle, void *async_argument, uint32_t async_id, BERR_Code error);

typedef struct {
    /* Called under ISR */
    BSAGElib_Rpc_IndicationRecvCallback indicationRecv_isr;
    BSAGElib_Rpc_ResponseRecvCallback responseRecv_isr;
    BSAGElib_Rpc_ResponseISRCallback response_isr;/* should not be used - semi-private API , replaces responseRecv_isr if set. */
} BSAGElib_RpcInterface;

/* ---------------------------------------- */
/* Open/Close SAGElib clients, register callbacks (watchdog, ...)  */

/*
 * SAGElib client settings structure
 * Passed upon BSAGElib_OpenClient() call in order to configure the SAGE library client
 * - provides synchronization interfaces to integrate with upper software architecture
 *   ! note that default configuration retreived through BSAGElib_GetDefaultClientSettings
 *     should be the 'default' ones to use for 'regular' clients
 * - provides Rpc interface in order to receive SAGE messages
 *
 * See BSAGElib_GetDefaultClientSettings, BSAGElib_OpenClient
*/
typedef struct {
    BSAGElib_SyncInterface i_sync_sage;
    BSAGElib_SyncInterface i_sync_hsm;
    BSAGElib_SyncInterface i_sync_cache;
    BSAGElib_RpcInterface i_rpc;
} BSAGElib_ClientSettings;

/* Get default SAGE library client settings structure.
 * Mandatory to use for retro compatibility uppon updates */
void
BSAGElib_GetDefaultClientSettings(
    BSAGElib_Handle hSAGElib,
    BSAGElib_ClientSettings *settings /* [in/out] */);

/* Initialize SAGE library client and retreive its handle */
BERR_Code
BSAGElib_OpenClient(
    BSAGElib_Handle hSAGElib, /* returned by HBSAGElib_Open */
    BSAGElib_ClientHandle *clientHandle, /* [out] SAGElib client handle */
    BSAGElib_ClientSettings *settings);

/* Uninitialize the SAGE library client
 * None of other SAGE library client sub APIs (BSAGElib_Rai, BSAGElib_Rpc)
 * can be used afterward as given handle will be invalidated */
void
BSAGElib_CloseClient(
    BSAGElib_ClientHandle handle /* [in] SAGElib client handle */);

typedef struct BSAGElib_ResponseData
{
    BSAGElib_RpcRemoteHandle remote;
    void *async_arg;
    BERR_Code rc;
    uint32_t async_id;
} BSAGElib_ResponseData;

/*
Get response from sage

Returns:
BERR_SUCCESS - got data
BERR_NOT_AVAILABLE - no data
other non-zero value - error
*/
BERR_Code
BSAGElib_Client_GetResponse(
    BSAGElib_ClientHandle hSAGElibClient,
    BSAGElib_ResponseData *data
    );

/* ---------------------------------------- */
/* Power Management  */

BERR_Code
BSAGElib_Standby(
    BSAGElib_ClientHandle hSAGElibClient,
    BSAGElib_eStandbyMode mode);

/* ---------------------------------------- */


#ifdef _cplusplus
}
#endif


#endif /* BSAGELIB_CLIENT_H_ */
