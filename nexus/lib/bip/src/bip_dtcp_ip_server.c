/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

/**
 * DTCP/IP Server Module implementation
 * Its mainly a wrapper to the Broadcom's DTCP/IP library.
*/

#include "bip_priv.h"
#include "bip_dtcp_ip_server.h"

/*
 * States for DtcpIpServer Object (main server object),
 *  states reflecting DtcpIpServer APIs visiable to Apps: Create, Start, Stop of Server
 */
BDBG_MODULE( bip_http_server );
BDBG_OBJECT_ID( BIP_DtcpIpServer );
BIP_SETTINGS_ID(BIP_DtcpIpServerStartSettings);

typedef enum BIP_DtcpIpServerStartState
{
    BIP_DtcpIpServerStartState_eUninitialized = 0,     /* Initial state: object is just created but fully initialzed */
    BIP_DtcpIpServerStartState_eIdle,                  /* Idle state: Server object is just created, but it hasn't yet started listening for Requests. */
    BIP_DtcpIpServerStartState_eStarted,               /* Server is started where it has started Listener and is ready to accept incoming connections & HTTP Requests. */
    BIP_DtcpIpServerStartState_eStopped,               /* Server is stopped and is ready to be started again. */
    BIP_DtcpIpServerStartState_eMax
} BIP_DtcpIpServerStartState;

typedef struct BIP_DtcpIpServer
{
    BDBG_OBJECT( BIP_DtcpIpServer )

    BIP_DtcpIpServerCreateSettings  createSettings;
    BIP_DtcpIpServerStartSettings   startSettings;

    BIP_DtcpIpServerStartState      startState;                     /* State */
    BIP_Status                      completionStatus;
    B_MutexHandle                   hStateMutex;                    /* Mutex to synchronize a API invocation with callback invocation */

    BIP_StringHandle                hAkePort;
    BIP_StringHandle                hKeyFileAbsolutePathname;

    void *                          pDtcpIpLibInitCtx;              /* DTCP/IP library init ctx: void * as there is no explict type defined in DTCP/IP lib. */

    /* One Arb per API */
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_DtcpIpServerStartSettings *pSettings;
    } startApi;
    struct
    {
        BIP_ArbHandle               hArb;
    } stopApi;
    struct
    {
        BIP_ArbHandle               hArb;
    } destroyApi;
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_DtcpIpServerStatus           *pStatus;
    } getStatusApi;

    /* Stats. */
    struct
    {
        unsigned                    numAcceptedConnections;     /* total connections accepted since _Start */
    } stats;
} BIP_DtcpIpServer;

#define BIP_DTCP_IP_SERVER_PRINTF_FMT  \
    "[hDtcpIpServer=%p akePort=%s]"

#define BIP_DTCP_IP_SERVER_PRINTF_ARG(pObj)   \
    (void *)(pObj),                                   \
    (pObj)->startSettings.pAkePort

/* Forward declaration of state processing function */
void processDtcpIpServerState( void *hObject, int value, BIP_Arb_ThreadOrigin );


void BIP_DtcpIpServer_GetDefaultCreateSettings(
    BIP_DtcpIpServerCreateSettings *pSettings
    )
{
    B_Os_Memset( pSettings, 0, sizeof( BIP_DtcpIpServerCreateSettings ));
}

static void dtcpServerDestroy(
    BIP_DtcpIpServerHandle hDtcpIpServer
    )
{
    if (!hDtcpIpServer) return;
    BDBG_MSG(( BIP_MSG_PRE_FMT "Destroying hDtcpIpServer %p" BIP_MSG_PRE_ARG, (void *)hDtcpIpServer ));

    if (hDtcpIpServer->startApi.hArb) BIP_Arb_Destroy(hDtcpIpServer->startApi.hArb);
    if (hDtcpIpServer->stopApi.hArb) BIP_Arb_Destroy(hDtcpIpServer->stopApi.hArb);
    if (hDtcpIpServer->destroyApi.hArb) BIP_Arb_Destroy(hDtcpIpServer->destroyApi.hArb);
    if (hDtcpIpServer->getStatusApi.hArb) BIP_Arb_Destroy(hDtcpIpServer->getStatusApi.hArb);

    if (hDtcpIpServer->hStateMutex) B_Mutex_Destroy( hDtcpIpServer->hStateMutex );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hDtcpIpServer %p: Destroyed" BIP_MSG_PRE_ARG, (void *)hDtcpIpServer ));

    BDBG_OBJECT_DESTROY( hDtcpIpServer, BIP_DtcpIpServer );
    B_Os_Free( hDtcpIpServer );

} /* dtcpServerDestroy */

BIP_DtcpIpServerHandle BIP_DtcpIpServer_Create(
    const BIP_DtcpIpServerCreateSettings *pCreateSettings
    )
{
    BIP_Status                          brc;
    BIP_DtcpIpServerHandle              hDtcpIpServer = NULL;
    BIP_DtcpIpServerCreateSettings      defaultSettings;

    /* Create the object */
    hDtcpIpServer = B_Os_Calloc( 1, sizeof( BIP_DtcpIpServer ));
    BIP_CHECK_GOTO(( hDtcpIpServer != NULL ), ( "Failed to allocate memory (%zu bytes) for DtcpIpServer Object", sizeof(BIP_DtcpIpServer) ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

    BDBG_OBJECT_SET( hDtcpIpServer, BIP_DtcpIpServer );

    /* Create mutex to synchronize state machine from being run via callbacks (BIP_Socket or timer) & Caller calling APIs. */
    hDtcpIpServer->hStateMutex = B_Mutex_Create(NULL);
    BIP_CHECK_GOTO(( hDtcpIpServer->hStateMutex ), ( "B_Mutex_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

    if (NULL == pCreateSettings)
    {
        BIP_DtcpIpServer_GetDefaultCreateSettings( &defaultSettings );
        pCreateSettings = &defaultSettings;
    }
    hDtcpIpServer->createSettings = *pCreateSettings;

    /* Create API ARBs: one per API */
    hDtcpIpServer->startApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hDtcpIpServer->startApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

    hDtcpIpServer->stopApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hDtcpIpServer->stopApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

    hDtcpIpServer->destroyApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hDtcpIpServer->destroyApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

    hDtcpIpServer->getStatusApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hDtcpIpServer->getStatusApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hDtcpIpServer %p: Created ARBs" BIP_MSG_PRE_ARG, (void *)hDtcpIpServer));

    hDtcpIpServer->startState = BIP_DtcpIpServerStartState_eIdle;

    BDBG_MSG((    BIP_MSG_PRE_FMT "Created: " BIP_DTCP_IP_SERVER_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_DTCP_IP_SERVER_PRINTF_ARG(hDtcpIpServer)));
    BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Created: " BIP_DTCP_IP_SERVER_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_DTCP_IP_SERVER_PRINTF_ARG(hDtcpIpServer)));

    return ( hDtcpIpServer );

error:
    dtcpServerDestroy( hDtcpIpServer );
    return ( NULL );
} /* BIP_DtcpIpServer_Create */

/**
 * Summary:
 * Destroy HTTP Server
 *
 * Description:
 **/
void BIP_DtcpIpServer_Destroy(
    BIP_DtcpIpServerHandle hDtcpIpServer
    )
{
    BIP_Status brc;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hDtcpIpServer, BIP_DtcpIpServer );

    BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Destroying: " BIP_DTCP_IP_SERVER_PRINTF_FMT
                      BIP_MSG_PRE_ARG, BIP_DTCP_IP_SERVER_PRINTF_ARG(hDtcpIpServer)));

    hArb = hDtcpIpServer->destroyApi.hArb;
    brc = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, brc, brc );

    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hDtcpIpServer;
    arbSettings.arbProcessor = processDtcpIpServerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    brc = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_HttpServer_GetSettings" ), error, brc, brc );

error:
    /* Now free the DtcpIpServer's resources. */
    dtcpServerDestroy( hDtcpIpServer );

} /* BIP_DtcpIpServer_Destroy */


BIP_Status BIP_DtcpIpServer_Start(
    BIP_DtcpIpServerHandle    hDtcpIpServer,
    BIP_DtcpIpServerStartSettings *pSettings
    )
{
    BIP_Status brc = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_DtcpIpServerStartSettings defaultSettings;

    BDBG_OBJECT_ASSERT( hDtcpIpServer, BIP_DtcpIpServer );
    BIP_SETTINGS_ASSERT(pSettings, BIP_DtcpIpServerStartSettings);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hDtcpIpServer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hDtcpIpServer));

    BIP_CHECK_GOTO(( hDtcpIpServer ), ( "hDtcpIpServer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, brc );

    if (pSettings == NULL)
    {
        BIP_DtcpIpServer_GetDefaultStartSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }
    else
    {   /* If they passed settings to us, do some validation. */
        BIP_CHECK_GOTO(( pSettings->pAkePort ), ( "pSettings->pAkePort can't be NULL, app must provide Port Number to listen on!"), error, BIP_ERR_INVALID_PARAMETER, brc );
        BIP_CHECK_GOTO(( pSettings->pAkePort && (atoi(pSettings->pAkePort) > 0) ), ( "pSettings->pAkePort is not valid %s", pSettings->pAkePort ), error, BIP_ERR_INVALID_PARAMETER, brc );
    }

    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hDtcpIpServer->startApi.hArb;
    brc = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, brc, brc );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hDtcpIpServer->startApi.pSettings = pSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hDtcpIpServer;
    arbSettings.arbProcessor = processDtcpIpServerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    brc = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_DtcpIpServer_Start" ), error, brc, brc );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hDtcpIpServer %p: completionStatus %s  <--------------------- " BIP_MSG_PRE_ARG, (void *)hDtcpIpServer, BIP_StatusGetText(brc) ));

    return ( brc );
} /* BIP_DtcpIpServer_Start */

BIP_Status BIP_DtcpIpServer_Stop(
    BIP_DtcpIpServerHandle    hDtcpIpServer
    )
{
    BIP_Status brc = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hDtcpIpServer, BIP_DtcpIpServer );

    BDBG_MSG((    BIP_MSG_PRE_FMT "Stopping: " BIP_DTCP_IP_SERVER_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_DTCP_IP_SERVER_PRINTF_ARG(hDtcpIpServer)));
    BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Stopping: " BIP_DTCP_IP_SERVER_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_DTCP_IP_SERVER_PRINTF_ARG(hDtcpIpServer)));

    BIP_CHECK_GOTO(( hDtcpIpServer ), ( "hDtcpIpServer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, brc );

    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hDtcpIpServer->stopApi.hArb;
    brc = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, brc, brc );

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hDtcpIpServer;
    arbSettings.arbProcessor = processDtcpIpServerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    brc = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_DtcpIpServer_Stop" ), error, brc, brc );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hDtcpIpServer %p: completionStatus %s  <--------------------- " BIP_MSG_PRE_ARG, (void *)hDtcpIpServer, BIP_StatusGetText(brc) ));

    return( brc );
} /* BIP_DtcpIpServer_Stop */

BIP_Status BIP_DtcpIpServer_GetStatus(
    BIP_DtcpIpServerHandle    hDtcpIpServer,
    BIP_DtcpIpServerStatus         *pStatus
    )
{
    BIP_Status brc = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hDtcpIpServer, BIP_DtcpIpServer );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hDtcpIpServer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hDtcpIpServer));

    BIP_CHECK_GOTO(( hDtcpIpServer ), ( "hDtcpIpServer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, brc );
    BIP_CHECK_GOTO(( pStatus ), ( "pStatus pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, brc );

    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hDtcpIpServer->getStatusApi.hArb;
    brc = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, brc, brc );

    hDtcpIpServer->getStatusApi.pStatus = pStatus;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hDtcpIpServer;
    arbSettings.arbProcessor = processDtcpIpServerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    brc = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_DtcpIpServer_DestroyStreamer" ), error, brc, brc );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hDtcpIpServer %p: completionStatus %s  <--------------------- " BIP_MSG_PRE_ARG, (void *)hDtcpIpServer, BIP_StatusGetText(brc) ));

    return (brc);
} /* BIP_DtcpIpServer_GetStatus */

struct BIP_DtcpIpServerStartStateNames
{
    BIP_DtcpIpServerStartState state;
    char *pStateName;
}gDtcpIpServerStartState[] = {
    {BIP_DtcpIpServerStartState_eUninitialized, "UnInitialized"},
    {BIP_DtcpIpServerStartState_eIdle, "Idle"},
    {BIP_DtcpIpServerStartState_eStarted, "Started"},
    {BIP_DtcpIpServerStartState_eMax, "MaxState"}
};
#define BIP_DTCP_IP_SERVER_START_STATE(state) \
    gDtcpIpServerStartState[state].pStateName

static void
destroyDtcpIpServer(
    BIP_DtcpIpServerHandle hDtcpIpServer
    )
{
    BDBG_ASSERT(hDtcpIpServer);

    if (hDtcpIpServer->hAkePort) BIP_String_Destroy( hDtcpIpServer->hAkePort );
    if (hDtcpIpServer->hKeyFileAbsolutePathname) BIP_String_Destroy( hDtcpIpServer->hKeyFileAbsolutePathname );

    if (hDtcpIpServer->pDtcpIpLibInitCtx)
    {
        DtcpAppLib_Shutdown(hDtcpIpServer->pDtcpIpLibInitCtx);
        hDtcpIpServer->pDtcpIpLibInitCtx = NULL;
    }

    DtcpCleanupHwSecurityParams();
    BDBG_MSG(("DTCP/IP Library is un-initialized"));
} /* stopDtcpIpServer */

static void
stopDtcpIpServer(
    BIP_DtcpIpServerHandle hDtcpIpServer
    )
{
    BDBG_ASSERT(hDtcpIpServer);

    if (hDtcpIpServer->pDtcpIpLibInitCtx)
    {
        DtcpAppLib_CancelListen( hDtcpIpServer->pDtcpIpLibInitCtx );
    }

    BDBG_MSG(("DTCP/IP Server is stoppped"));
} /* stopDtcpIpServer */

static void
stopAndDestroyDtcpIpServer(
    BIP_DtcpIpServerHandle hDtcpIpServer
    )
{
    BDBG_ASSERT(hDtcpIpServer);
    if (!hDtcpIpServer) return;
    stopDtcpIpServer(hDtcpIpServer);
    destroyDtcpIpServer(hDtcpIpServer);
}

static BIP_Status startDtcpIpServer(
    BIP_DtcpIpServerHandle hDtcpIpServer
    )
{
    B_Error rc;
    BIP_Status bipStatus = BIP_ERR_INVALID_PARAMETER;

    BDBG_ASSERT(hDtcpIpServer);

    rc = DtcpInitHWSecurityParams(NULL);
    BIP_CHECK_GOTO( ( rc == B_ERROR_SUCCESS ), ( "hDtcpIpServer %p: DtcpInitHWSecurityParams() Failed", (void *)hDtcpIpServer ), error, BIP_ERR_DTCPIP_HW_SECURITY_PARAMS, bipStatus );

    hDtcpIpServer->hAkePort = BIP_String_CreateFromPrintf("%s", hDtcpIpServer->startApi.pSettings->pAkePort );
    hDtcpIpServer->hKeyFileAbsolutePathname = BIP_String_CreateFromPrintf("%s", hDtcpIpServer->startApi.pSettings->pKeyFileAbsolutePathname );
    BIP_CHECK_GOTO( ( hDtcpIpServer->hAkePort || hDtcpIpServer->hKeyFileAbsolutePathname ), ("String Creation Failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hDtcpIpServer->pDtcpIpLibInitCtx = DtcpAppLib_Startup(
            B_DeviceMode_eSource, /* DtcpIpServer will be a source device for sending out the streams. */
            hDtcpIpServer->startSettings.pcpUsageRuleEnabled,
            hDtcpIpServer->startSettings.keyFormat,
            false /* TODO; CKS Check: Need more info on this. */
            );
    BIP_CHECK_GOTO( ( hDtcpIpServer->pDtcpIpLibInitCtx ), ( "hDtcpIpServer %p: DtcpAppLib_Startup() Failed", (void *)hDtcpIpServer ), error, BIP_ERR_DTCPIP_SERVER_START, bipStatus );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hDtcpIpServer %p: DtcpAppLib_Startup is successfull, pDtcpIpLibInitCtx %p" BIP_MSG_PRE_ARG, (void *)hDtcpIpServer, hDtcpIpServer->pDtcpIpLibInitCtx ));

    rc = DtcpAppLib_Listen(
            hDtcpIpServer->pDtcpIpLibInitCtx,
            NULL /* TODO: src ip */,
            atoi(BIP_String_GetString(hDtcpIpServer->hAkePort))
            );
    BIP_CHECK_GOTO( ( rc == B_ERROR_SUCCESS ), ( "hDtcpIpServer %p: DtcpAppLib_Listen() Failed", (void *)hDtcpIpServer ), error, BIP_ERR_DTCPIP_SERVER_LISTEN, bipStatus );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hDtcpIpServer %p: DtcpAppLib_Listen is successfull, pDtcpIpLibInitCtx %p" BIP_MSG_PRE_ARG, (void *)hDtcpIpServer, hDtcpIpServer->pDtcpIpLibInitCtx ));

    bipStatus = BIP_SUCCESS;

error:
    if (bipStatus != BIP_SUCCESS && hDtcpIpServer) stopAndDestroyDtcpIpServer(hDtcpIpServer);

    return (bipStatus);
} /* startDtcpIpServer */

void processDtcpIpServerState(
    void *                  hObject,
    int                     value,
    BIP_Arb_ThreadOrigin    threadOrigin
    )
{
    BIP_ArbHandle           hArb;
    BIP_DtcpIpServerHandle  hDtcpIpServer = hObject;

    BSTD_UNUSED(value);

    BDBG_ASSERT(hDtcpIpServer);
    BDBG_OBJECT_ASSERT( hDtcpIpServer, BIP_DtcpIpServer);

    /*
     ***************************************************************************************************************
     * DtcpIpServer State Machine Processing:
     ***************************************************************************************************************
     */

    B_Mutex_Lock( hDtcpIpServer->hStateMutex );
    BDBG_MSG(( BIP_MSG_PRE_FMT "ENTRY ---> hDtcpIpServer %p: state %s, threadOrigin %d "
                BIP_MSG_PRE_ARG, (void *)hDtcpIpServer, BIP_DTCP_IP_SERVER_START_STATE(hDtcpIpServer->startState), threadOrigin ));
    /*
     ***************************************************************************************************************
     * First, we check API Arbs to see if state processing is being run thru any of these APIs.
     ***************************************************************************************************************
     */
    if (BIP_Arb_IsNew(hArb = hDtcpIpServer->startApi.hArb))
    {
        /* App is starting the Server, make sure we are in the correct state to do so! */
        if (hDtcpIpServer->startState != BIP_DtcpIpServerStartState_eIdle)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hServer %p: BIP_DtcpIpServer_Start not allowed in this state: %s"
                        BIP_MSG_PRE_ARG, (void *)hDtcpIpServer, BIP_DTCP_IP_SERVER_START_STATE(hDtcpIpServer->startState)));
            hDtcpIpServer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hDtcpIpServer->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);
            BDBG_MSG(( BIP_MSG_PRE_FMT "hDtcpIpServer %p: Accepted _Start Arb: state %s!"
                        BIP_MSG_PRE_ARG, (void *)hDtcpIpServer, BIP_DTCP_IP_SERVER_START_STATE(hDtcpIpServer->startState) ));

            /* Start dtcp/ip server if enabled. */
            hDtcpIpServer->startSettings = *hDtcpIpServer->startApi.pSettings;
            hDtcpIpServer->completionStatus = startDtcpIpServer(hDtcpIpServer);

            if ( hDtcpIpServer->completionStatus == BIP_SUCCESS )
            {
                /* We have successfully started the DTCP/IP Server, so switch to the Started state! */
                hDtcpIpServer->startState = BIP_DtcpIpServerStartState_eStarted;

                BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Started: " BIP_DTCP_IP_SERVER_PRINTF_FMT
                            BIP_MSG_PRE_ARG, BIP_DTCP_IP_SERVER_PRINTF_ARG(hDtcpIpServer)));

                BIP_MSG_SUM(( BIP_MSG_PRE_FMT "hDtcpIpServer %p, state %s: BIP_DtcpIpServer Started on Port %s -----<>"
                            BIP_MSG_PRE_ARG, (void *)hDtcpIpServer, BIP_DTCP_IP_SERVER_START_STATE(hDtcpIpServer->startState), hDtcpIpServer->startSettings.pAkePort));
            }

            BIP_Arb_CompleteRequest( hDtcpIpServer->startApi.hArb, hDtcpIpServer->completionStatus);
        }
    }
    else if (BIP_Arb_IsNew(hArb = hDtcpIpServer->stopApi.hArb))
    {
        if (hDtcpIpServer->startState != BIP_DtcpIpServerStartState_eStarted)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hDtcpIpServer %p: BIP_DtcpIpServer_Sttop not allowed in this state: %s, we must be in BIP_DtcpIpServerStartState_eStarted state! "
                        BIP_MSG_PRE_ARG, (void *)hDtcpIpServer, BIP_DTCP_IP_SERVER_START_STATE(hDtcpIpServer->startState)));
            hDtcpIpServer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hDtcpIpServer->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);

            BDBG_MSG(( BIP_MSG_PRE_FMT "hDtcpIpServer %p: Accepted _Stop Arb: state %s!"
                        BIP_MSG_PRE_ARG, (void *)hDtcpIpServer, BIP_DTCP_IP_SERVER_START_STATE(hDtcpIpServer->startState) ));

            stopDtcpIpServer(hDtcpIpServer);

            /* Since we stopped the BIP_Listener, we are back to ReadyToStart state! */
            hDtcpIpServer->startState = BIP_DtcpIpServerStartState_eStopped;

            BIP_Arb_CompleteRequest( hDtcpIpServer->stopApi.hArb, hDtcpIpServer->completionStatus);

            BIP_MSG_SUM(( BIP_MSG_PRE_FMT "hDtcpIpServer %p, state %s: BIP_DtcpIpServer Stopped on Port %s -----<>"
                        BIP_MSG_PRE_ARG, (void *)hDtcpIpServer, BIP_DTCP_IP_SERVER_START_STATE(hDtcpIpServer->startState), hDtcpIpServer->startSettings.pAkePort));
        }
    }
    else if (BIP_Arb_IsNew(hArb = hDtcpIpServer->getStatusApi.hArb))
    {
        BIP_Arb_AcceptRequest(hArb);

        hDtcpIpServer->getStatusApi.pStatus->pDtcpIpLibCtx = hDtcpIpServer->pDtcpIpLibInitCtx;
        if ( hDtcpIpServer->hAkePort )
        {
            hDtcpIpServer->getStatusApi.pStatus->pAkePort = BIP_String_GetString( hDtcpIpServer->hAkePort );
        }
        else
        {
            hDtcpIpServer->getStatusApi.pStatus->pAkePort = NULL;
        }

        BIP_Arb_CompleteRequest( hDtcpIpServer->getStatusApi.hArb, BIP_SUCCESS );
        BDBG_MSG(( BIP_MSG_PRE_FMT "hDtcpIpServer %p: BIP_DtcpIpServer_GetStatus(): pDtcpIpLibCtx %p" BIP_MSG_PRE_ARG, (void *)hDtcpIpServer, hDtcpIpServer->pDtcpIpLibInitCtx ));
    }
    else if (BIP_Arb_IsNew(hArb = hDtcpIpServer->destroyApi.hArb))
    {
        /*
         * App has issued BIP_DtcpIpServer_Destroy API.
         * So far there is nothing to do inside the state machine.
         */
        BIP_Arb_AcceptRequest(hArb);

        BIP_Arb_CompleteRequest( hDtcpIpServer->destroyApi.hArb, BIP_SUCCESS );

        if (hDtcpIpServer->startState == BIP_DtcpIpServerStartState_eStarted)
            stopDtcpIpServer(hDtcpIpServer);
        destroyDtcpIpServer(hDtcpIpServer);

        BDBG_MSG((BIP_MSG_PRE_FMT "%s: done .." BIP_MSG_PRE_ARG, __FUNCTION__));
    } /* while reRunProcessState */

    B_Mutex_Unlock( hDtcpIpServer->hStateMutex );

    BDBG_MSG(( BIP_MSG_PRE_FMT "EXIT <--- hDtcpIpServer %p" BIP_MSG_PRE_ARG, (void *)hDtcpIpServer ));
    return;
}
