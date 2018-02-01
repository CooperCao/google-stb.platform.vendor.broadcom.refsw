/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bip_priv.h"

BDBG_MODULE( bip_dtcp_ip_client );

BDBG_OBJECT_ID( BIP_DtcpIpClient );
BDBG_OBJECT_ID( BIP_DtcpIpClientFactoryAkeEntry );

BIP_SETTINGS_ID(BIP_DtcpIpClientFactoryInitSettings);
BIP_SETTINGS_ID(BIP_DtcpIpClientFactoryAkeEntry);
BIP_SETTINGS_ID(BIP_DtcpIpClientFactoryDoAkeSettings);

typedef struct BIP_DtcpIpClientFactory *BIP_DtcpIpClientFactoryHandle;

/*
 * Each BIP_DtcpIpClientFactoryAkeEntry represents a DTCP/IP host that BIP has authenticated (completed AKE) with.
 */
typedef struct BIP_DtcpIpClientFactoryAkeEntry
{
    BDBG_OBJECT( BIP_DtcpIpClientFactoryAkeEntry )

    B_MutexHandle                                   hStateMutex;                /* Mutex for AkeEntry state. */
    BLST_Q_ENTRY(BIP_DtcpIpClientFactoryAkeEntry)   akeEntry_link;              /* Link for next & previous AkeEntry list. */
    BIP_StringHandle                                hServerIp;                  /* Server Identifier: IP address. */
    BIP_StringHandle                                hServerPort;                /* Server Identifier: Port#. */
    void                                            *pDtcpIpLibAkeCtx;          /* AKE handle returned by the DTCP/IP lib for this Server. */
    BIP_DtcpIpClientFactoryAkeEntryState            state;                      /* AKE Entry state. */
    BIP_DtcpIpClientFactoryHandle                   hDtcpIpClientFactory;       /* Handle of the Parent DtcpIpFactory Object. */
    BIP_Status                                      dtcpIpAkeCompletionStatus;  /* Completion status for the DoAke call to the DTCP/IP library. */
    B_ThreadHandle                                  hThread;
} BIP_DtcpIpClientFactoryAkeEntry;

typedef enum BIP_DtcpIpClientFactoryState
{
    BIP_DtcpIpClientFactoryState_eNotInitialized,
    BIP_DtcpIpClientFactoryState_eInitialized,
    BIP_DtcpIpClientFactoryState_eMax
} BIP_DtcpIpClientFactoryState;

/*
 * The BIP_DtcpIpClientFactory class is a singleton (only one instance) that maintains the list of BIP_DtcpIpClientFactoryAkeEntry structures.
 */
typedef struct BIP_DtcpIpClientFactory
{
    B_MutexHandle                               hStateMutex;            /* Mutex for DtcpIpClientFactory state. */
    void                                        *pDtcpIpLibInitCtx;
    BIP_DtcpIpClientFactoryState                state;
    BLST_Q_HEAD(akeEntry_head, BIP_DtcpIpClientFactoryAkeEntry) akeEntry_head;

    struct
    {
        BIP_ArbHandle                           hArb;
        const char                              *pServerIp;
        const char                              *pServerPort;
        BIP_DtcpIpClientFactoryDoAkeSettings    *pSettings;
        BIP_DtcpIpClientFactoryAkeEntryHandle   *phAkeEntry;
    } doAkeApi;
    struct
    {
        BIP_ArbHandle                           hArb;
        BIP_DtcpIpClientFactoryAkeEntryStatus   *pAkeEntryStatus;
        BIP_DtcpIpClientFactoryAkeEntryHandle   hAkeEntry;
    } getAkeEntryStatusApi;
} BIP_DtcpIpClientFactory;

/* Allocate a single instance of the DtcpIpClientFactory. */
static BIP_DtcpIpClientFactory g_BIP_DtcpIpClientFactory;

static void processDtcpIpClientFactoryState( void *hObject, int value, BIP_Arb_ThreadOrigin threadOrigin);

static void destroyAkeEntry(
    BIP_DtcpIpClientFactoryAkeEntryHandle hAkeEntry
    )
{
    BIP_DtcpIpClientFactoryHandle hDtcpIpClientFactory = &g_BIP_DtcpIpClientFactory;

    if (hAkeEntry == NULL) return;
    if (hAkeEntry->pDtcpIpLibAkeCtx)
    {
        B_Error rc;

        BDBG_MSG(( BIP_MSG_PRE_FMT "Cleanup hAkeEntry=%p: Closing pDtcpIpLibAkeCtx=%p" BIP_MSG_PRE_ARG, (void *)hAkeEntry, hAkeEntry->pDtcpIpLibAkeCtx ));
        rc = DtcpAppLib_CloseAke(hDtcpIpClientFactory->pDtcpIpLibInitCtx, hAkeEntry->pDtcpIpLibAkeCtx);
        BDBG_ASSERT(rc==B_ERROR_SUCCESS);
    }
    if (hAkeEntry->hServerIp) BIP_String_Destroy(hAkeEntry->hServerIp);
    if (hAkeEntry->hServerPort) BIP_String_Destroy(hAkeEntry->hServerPort);
    if (hAkeEntry->hStateMutex) B_Mutex_Destroy(hAkeEntry->hStateMutex);
    B_Os_Free(hAkeEntry);
} /* destroyAkeEntry */

void BIP_DtcpIpClientFactory_Uninit()
{
    BIP_DtcpIpClientFactoryHandle hDtcpIpClientFactory = &g_BIP_DtcpIpClientFactory;
    BIP_DtcpIpClientFactoryAkeEntryHandle hAkeEntry;

    for (
            hAkeEntry = BLST_Q_FIRST(&hDtcpIpClientFactory->akeEntry_head);
            hAkeEntry;
            hAkeEntry = BLST_Q_FIRST(&hDtcpIpClientFactory->akeEntry_head)
        )
    {
        BLST_Q_REMOVE(&hDtcpIpClientFactory->akeEntry_head, hAkeEntry, akeEntry_link);
        if (hAkeEntry->hThread) B_Thread_Destroy(hAkeEntry->hThread);
        destroyAkeEntry(hAkeEntry);
    }

    if (hDtcpIpClientFactory->doAkeApi.hArb)
    {
        BIP_Arb_Destroy(hDtcpIpClientFactory->doAkeApi.hArb);
        hDtcpIpClientFactory->doAkeApi.hArb = NULL;
    }

    if (hDtcpIpClientFactory->getAkeEntryStatusApi.hArb)
    {
        BIP_Arb_Destroy(hDtcpIpClientFactory->getAkeEntryStatusApi.hArb);
        hDtcpIpClientFactory->getAkeEntryStatusApi.hArb = NULL;
    }

    if (hDtcpIpClientFactory->pDtcpIpLibInitCtx)
    {
        DtcpAppLib_Shutdown(hDtcpIpClientFactory->pDtcpIpLibInitCtx);
        hDtcpIpClientFactory->pDtcpIpLibInitCtx = NULL;
        DtcpCleanupHwSecurityParams();
    }

    if (hDtcpIpClientFactory->hStateMutex)
    {
        B_Mutex_Destroy(hDtcpIpClientFactory->hStateMutex);
        hDtcpIpClientFactory->hStateMutex = NULL;
    }

    hDtcpIpClientFactory->state = BIP_DtcpIpClientFactoryState_eNotInitialized;
    BDBG_MSG(( BIP_MSG_PRE_FMT "BIP_DtcpIpClientFactory: Uninitialized" BIP_MSG_PRE_ARG ));
    return;
}

BIP_Status BIP_DtcpIpClientFactory_Init(
    BIP_DtcpIpClientFactoryInitSettings *pSettings
    )
{
    B_Error rc;
    BIP_Status bipStatus;
    BIP_DtcpIpClientFactoryHandle hDtcpIpClientFactory = &g_BIP_DtcpIpClientFactory;
    BIP_DtcpIpClientFactoryInitSettings defaultSettings;

    if (hDtcpIpClientFactory->state == BIP_DtcpIpClientFactoryState_eInitialized) return BIP_SUCCESS;

    BDBG_MSG(( BIP_MSG_PRE_FMT "BIP_DtcpIpClientFactory: Initializing" BIP_MSG_PRE_ARG ));
    BKNI_Memset(hDtcpIpClientFactory, 0, sizeof(*hDtcpIpClientFactory));

    if (pSettings == NULL)
    {
        BIP_DtcpIpClientFactory_GetDefaultInitSettings( &defaultSettings );
        pSettings = &defaultSettings;
    }

    BIP_SETTINGS_ASSERT(pSettings, BIP_DtcpIpClientFactoryInitSettings);

    /* Init the AkeEntry list head. */
    BLST_Q_INIT(&hDtcpIpClientFactory->akeEntry_head);

    /* Create a mutext to protect the state. */
    hDtcpIpClientFactory->hStateMutex = B_Mutex_Create( NULL );
    BIP_CHECK_GOTO(( hDtcpIpClientFactory->hStateMutex ), ( "B_Mutex_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    /* Initialize DTCP/IP library. */
    rc = DtcpInitHWSecurityParams(NULL);
    if (rc == BERR_NOT_SUPPORTED)
    {
        bipStatus = BIP_ERR_NOT_SUPPORTED;
        BDBG_LOG(( BIP_MSG_PRE_FMT "DTCP/IP Feature is not supported: Please compile with DTCP_IP_SUPPORT if DTCP/IP Support is desired.!" BIP_MSG_PRE_ARG ));
        goto error;
    }
    BIP_CHECK_GOTO( ( rc == B_ERROR_SUCCESS ), ( "DtcpInitHWSecurityParams() Failed"), error, BIP_ERR_DTCPIP_HW_SECURITY_PARAMS, bipStatus );

    hDtcpIpClientFactory->pDtcpIpLibInitCtx = DtcpAppLib_Startup(
            B_DeviceMode_eSink,             /* DtcpIpClient will be a sink device for receiving the stream. */
            pSettings->pcpUsageRuleEnabled,
            pSettings->keyFormat,
            false                           /* TODO; CKS Check: Need more info on this. */
            );
    BIP_CHECK_GOTO( ( hDtcpIpClientFactory->pDtcpIpLibInitCtx ), ( "DtcpAppLib_Startup() Failed"), error, BIP_ERR_DTCPIP_CLIENT_INIT, bipStatus );

    hDtcpIpClientFactory->doAkeApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hDtcpIpClientFactory->doAkeApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hDtcpIpClientFactory->getAkeEntryStatusApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hDtcpIpClientFactory->getAkeEntryStatusApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    BDBG_MSG(( BIP_MSG_PRE_FMT "BIP_DtcpIpClientFactory is Initialized." BIP_MSG_PRE_ARG ));

    hDtcpIpClientFactory->state = BIP_DtcpIpClientFactoryState_eInitialized;
    return (BIP_SUCCESS);

error:
    BIP_DtcpIpClientFactory_Uninit();
    return (bipStatus);
} /* BIP_DtcpIpClientFactory_Init */

BIP_Status BIP_DtcpIpClientFactory_DoAke_impl(
    const char                              *pServerIp,         /*!< [in]  Required: IP Address of Server to do AKE with. */
    const char                              *pServerPort,       /*!< [in]  Required: Port # of Server to do AKE with. */
    BIP_DtcpIpClientFactoryDoAkeSettings    *pSettings,         /*!< [in]  Optional Settings. */
    BIP_DtcpIpClientFactoryAkeEntryHandle   *phAkeEntry,        /*!< [out] Required: returned AKE handle, valid upon success. */
    BIP_CallbackDesc                        *pAsyncCallback,    /*!< [in]  Async completion callback: called at API completion. */
    BIP_Status                              *pAsyncStatus       /*!< [out] Completion status of async API. */
                                                                /*!< Note: App can specify either both pAsyncCallback & pAsyncStatus or either one of them depending upon how it wants to use Async API. */
    )
{
    BIP_Status                              bipStatus = BIP_SUCCESS;
    BIP_ArbSubmitSettings                   arbSettings;
    BIP_ApiSettings                         apiSettings;
    BIP_ArbHandle                           hArb;
    BIP_DtcpIpClientFactoryHandle           hDtcpIpClientFactory = &g_BIP_DtcpIpClientFactory;
    BIP_DtcpIpClientFactoryDoAkeSettings    defaultSettings;

    BIP_CHECK_GOTO(( hDtcpIpClientFactory->state == BIP_DtcpIpClientFactoryState_eInitialized),( "DtcpIpClientFactory is not yet initialized, did you forget to either compile with DTCP_IP_SUPPORT or call BIP_DtcpIpClientFactory_Init() "), error, BIP_ERR_INVALID_API_SEQUENCE, bipStatus );
    BIP_CHECK_GOTO(( pServerIp ),   ( "pServerIp argument can't be NULL"), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pServerPort ), ( "pServerPort argument can't be NULL"), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( phAkeEntry ),  ( "phAkeEntry argument can't be NULL"), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    if (!pSettings)
    {
        BIP_DtcpIpClientFactory_GetDefaultDoAkeSettings( &defaultSettings );
        pSettings = &defaultSettings;
    }

    BIP_SETTINGS_ASSERT(pSettings, BIP_DtcpIpClientFactoryDoAkeSettings);

    /* Serialize access to Settings state among another thread calling the same API. */
    hArb = hDtcpIpClientFactory->doAkeApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hDtcpIpClientFactory->doAkeApi.pServerIp = pServerIp;
    hDtcpIpClientFactory->doAkeApi.pServerPort = pServerPort;
    hDtcpIpClientFactory->doAkeApi.phAkeEntry = phAkeEntry;
    hDtcpIpClientFactory->doAkeApi.pSettings = pSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hDtcpIpClientFactory;
    arbSettings.arbProcessor = processDtcpIpClientFactoryState;
    arbSettings.waitIfBusy = true;   /* TODO: for now, only allow 1 AKE at a time (99% of the usage!) as client typically interacts with one home gateway server & PIP is typically started after main video is up! */
    {
        BKNI_Memset(&apiSettings, 0, sizeof(apiSettings));
        if (pAsyncCallback) apiSettings.asyncCallback = *pAsyncCallback;
        apiSettings.pAsyncStatus = pAsyncStatus;
    }

    /* Invoke state machine. */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, &apiSettings);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS || bipStatus == BIP_INF_IN_PROGRESS), ( "BIP_Arb_SubmitRequest() Failed" ), error, bipStatus, bipStatus );

error:
    return (bipStatus);
}

BIP_Status BIP_DtcpIpClientFactory_DoAke(
    const char                              *pServerIp,         /*!< [in]  Required: IP Address of Server to do AKE with. */
    const char                              *pServerPort,       /*!< [in]  Required: Port # of Server to do AKE with. */
    BIP_DtcpIpClientFactoryDoAkeSettings    *pSettings,         /*!< [in]  Optional Settings. */
    BIP_DtcpIpClientFactoryAkeEntryHandle   *phAkeEntry         /*!< [out] Required: returned AKE handle, valid upon success. */
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;

    BDBG_MSG(( BIP_MSG_PRE_FMT "ENTER --------------------->" BIP_MSG_PRE_ARG ));

    /* NOTE: This top level function only checks arguments specific for the Async version of the BIP API. */
    /* The common arguments in the async & non-async version are checked in the _BIP_Player_API(). */

    bipStatus = BIP_DtcpIpClientFactory_DoAke_impl( pServerIp, pServerPort, pSettings, phAkeEntry, NULL, NULL );

    BDBG_MSG(( BIP_MSG_PRE_FMT "EXIT <--------------------" BIP_MSG_PRE_ARG ));

    return (bipStatus);
}

BIP_Status BIP_DtcpIpClientFactory_DoAkeAsync(
    const char                              *pServerIp,         /*!< [in]  Required: IP Address of Server to do AKE with. */
    const char                              *pServerPort,       /*!< [in]  Required: Port # of Server to do AKE with. */
    BIP_DtcpIpClientFactoryDoAkeSettings    *pSettings,         /*!< [in]  Optional Settings. */
    BIP_DtcpIpClientFactoryAkeEntryHandle   *phAkeEntry,        /*!< [out] Required: returned AKE handle, valid upon success. */
    BIP_CallbackDesc                        *pAsyncCallback,    /*!< [in]  Async completion callback: called at API completion. */
    BIP_Status                              *pAsyncStatus       /*!< [out] Completion status of async API. */
                                                                /*!< Note: App can specify either both pAsyncCallback & pAsyncStatus or either one of them depending upon how it wants to use Async API. */
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;

    BDBG_MSG(( BIP_MSG_PRE_FMT "ENTER --------------------->" BIP_MSG_PRE_ARG ));

    /* NOTE: This top level function only checks arguments specific for the Async version of the BIP API. */
    /* The common arguments in the async & non-async version are checked in the _BIP_Player_API(). */

    if ( (pAsyncStatus == NULL && pAsyncCallback == NULL) || (pAsyncStatus == NULL && pAsyncCallback && pAsyncCallback->callback == NULL) )
    {
        BDBG_ERR(( BIP_MSG_PRE_FMT  "Error: Both pAsyncStatus & pAsyncCallback can't be NULL" BIP_MSG_PRE_ARG ));
        bipStatus = BIP_ERR_INVALID_PARAMETER;
        goto error;
    }

    bipStatus = BIP_DtcpIpClientFactory_DoAke_impl( pServerIp, pServerPort, pSettings, phAkeEntry, pAsyncCallback, pAsyncStatus);

    BDBG_MSG(( BIP_MSG_PRE_FMT "EXIT <--------------------" BIP_MSG_PRE_ARG ));
error:
    return (bipStatus);
}

BIP_Status BIP_DtcpIpClientFactory_GetAkeEntryStatus(
    BIP_DtcpIpClientFactoryAkeEntryHandle   hDtcpIpClientFactoryAkeEntry,
    BIP_DtcpIpClientFactoryAkeEntryStatus   *pAkeEntryStatus
    )
{
    BIP_Status                              bipStatus = BIP_SUCCESS;
    BIP_ArbSubmitSettings                   arbSettings;
    BIP_ArbHandle                           hArb;
    BIP_DtcpIpClientFactoryHandle           hDtcpIpClientFactory = &g_BIP_DtcpIpClientFactory;

    BDBG_MSG(( BIP_MSG_PRE_FMT "ENTER --------------------->" BIP_MSG_PRE_ARG ));
    BDBG_OBJECT_ASSERT( hDtcpIpClientFactoryAkeEntry, BIP_DtcpIpClientFactoryAkeEntry );

    BIP_CHECK_GOTO(( hDtcpIpClientFactory->state == BIP_DtcpIpClientFactoryState_eInitialized),( "DtcpIpClientFactory is not yet initialized, did you forget to either compile with DTCP_IP_SUPPORT or call BIP_DtcpIpClientFactory_Init() "), error, BIP_ERR_INVALID_API_SEQUENCE, bipStatus );
    BIP_CHECK_GOTO(( pAkeEntryStatus ),     ( "pAkeEntryStatus argument can't be NULL"), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( hDtcpIpClientFactoryAkeEntry ),( "hDtcpIpClientFactoryAkeEntry argument can't be NULL"), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    /* Serialize access to Settings state among another thread calling the same API. */
    hArb = hDtcpIpClientFactory->getAkeEntryStatusApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hDtcpIpClientFactory->getAkeEntryStatusApi.pAkeEntryStatus = pAkeEntryStatus;
    hDtcpIpClientFactory->getAkeEntryStatusApi.hAkeEntry = hDtcpIpClientFactoryAkeEntry;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hDtcpIpClientFactory;
    arbSettings.arbProcessor = processDtcpIpClientFactoryState;
    arbSettings.waitIfBusy = true;

    /* Invoke state machine. */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS || bipStatus == BIP_INF_IN_PROGRESS), ( "BIP_Arb_SubmitRequest() Failed" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "EXIT <--------------------" BIP_MSG_PRE_ARG ));
    return (bipStatus);
}

/******************************  DtcpIpClientFactory State Machine logic. ******************************/

static BIP_Status processDtcpIpClientFactoryAkeEntryState_locked( BIP_DtcpIpClientFactoryAkeEntryHandle hAkeEntry, BIP_Arb_ThreadOrigin threadOrigin);

static void doDtcpIpAkeViaArbCallback(
    void *ctx
    )
{
    BIP_DtcpIpClientFactoryAkeEntryHandle hAkeEntry = ctx;
    BIP_Status completionStatus;

    BDBG_ASSERT(hAkeEntry);
    BDBG_OBJECT_ASSERT( hAkeEntry, BIP_DtcpIpClientFactoryAkeEntry );

    B_Mutex_Lock( hAkeEntry->hStateMutex );
    {
        B_Error rc;

        BDBG_MSG(( BIP_MSG_PRE_FMT "hAkeEntry=%p state=%s: Calling DtcpAppLib_DoAkeOrVerifyExchKey()" BIP_MSG_PRE_ARG, (void *)hAkeEntry, BIP_ToStr_BIP_DtcpIpClientFactoryAkeEntryState(hAkeEntry->state) ));

        {
            long port;
            const char *pServerIp;
            void *pDtcpIpLibInitCtx;
            void *pDtcpIpLibAkeCtx;

            pDtcpIpLibInitCtx = hAkeEntry->hDtcpIpClientFactory->pDtcpIpLibInitCtx;
            pServerIp = BIP_String_GetString(hAkeEntry->hServerIp);
            port = strtol(BIP_String_GetString(hAkeEntry->hServerPort), NULL, 10);
            pDtcpIpLibAkeCtx = hAkeEntry->pDtcpIpLibAkeCtx;  /* may point to an existing dtcpIpLib's Ake context which is used for "fast" AKE. */

            B_Mutex_Unlock( hAkeEntry->hStateMutex );
            rc = DtcpAppLib_DoAkeOrVerifyExchKey(pDtcpIpLibInitCtx, pServerIp, (short)port, &pDtcpIpLibAkeCtx);
            B_Mutex_Lock( hAkeEntry->hStateMutex );
            /* coverity[use] */
            hAkeEntry->pDtcpIpLibAkeCtx = pDtcpIpLibAkeCtx;
        }

        /* AKE is successful. */
        if (rc == B_ERROR_SUCCESS)
        {
            hAkeEntry->dtcpIpAkeCompletionStatus = BIP_SUCCESS;
            BDBG_MSG(( BIP_MSG_PRE_FMT "hAkeEntry=%p: DtcpAppLib_DoAkeOrVerifyExchKey() Success" BIP_MSG_PRE_ARG, (void *)hAkeEntry ));
        }
        else
        {
            /* Error case. */
            BDBG_ERR(( BIP_MSG_PRE_FMT "hAkeEntry=%p: DtcpAppLib_DoAkeOrVerifyExchKey() Failed" BIP_MSG_PRE_ARG, (void *)hAkeEntry ));
            hAkeEntry->dtcpIpAkeCompletionStatus = BIP_ERR_DTCPIP_CLIENT_AKE;
        }
        processDtcpIpClientFactoryAkeEntryState_locked(hAkeEntry, BIP_Arb_ThreadOrigin_eTimer);
    }
    B_Mutex_Unlock( hAkeEntry->hStateMutex );

    /* Tell ARB to do any deferred work. */
    completionStatus = BIP_Arb_DoDeferred( hAkeEntry->hDtcpIpClientFactory->doAkeApi.hArb, BIP_Arb_ThreadOrigin_eTimer );
    BDBG_ASSERT( completionStatus == BIP_SUCCESS );
} /* doDtcpIpAkeViaArbCallback */

static BIP_Status processDtcpIpClientFactoryAkeEntryState_locked(
    BIP_DtcpIpClientFactoryAkeEntryHandle hAkeEntry,
    BIP_Arb_ThreadOrigin threadOrigin
    )
{
    BIP_ArbHandle                   hArb;
    BIP_Status                      completionStatus = BIP_INF_IN_PROGRESS;
    BIP_DtcpIpClientFactoryHandle   hDtcpIpClientFactory;

    BSTD_UNUSED(threadOrigin);
    BDBG_ASSERT(hAkeEntry);

    hDtcpIpClientFactory = hAkeEntry->hDtcpIpClientFactory;
    BDBG_ASSERT(hDtcpIpClientFactory);

    BDBG_MSG(( BIP_MSG_PRE_FMT "hAkeEntry=%p state=%s" BIP_MSG_PRE_ARG, (void *)hAkeEntry, BIP_ToStr_BIP_DtcpIpClientFactoryAkeEntryState(hAkeEntry->state) ));

    if ( hAkeEntry->state == BIP_DtcpIpClientFactoryAkeEntryState_eNewAke )
    {
        /*
         * We are in NewAke state (meaning caller has invoked _DoAke()), we setup to do Ake!
         * Note: the DTCP/IP library will internally do the faster AKE first (VerifyExchKey) before attempting to do "slower" (full) Ake.
         */
        if (hAkeEntry->hThread) B_Thread_Destroy(hAkeEntry->hThread);
        hAkeEntry->hThread = B_Thread_Create(
                "BIP_DtcpIpClient_DoAke",   /* Thread Name - Optional */
                doDtcpIpAkeViaArbCallback,  /* Thread Main Routine */
                hAkeEntry,                  /* Parameter provided to threadFunction */
                NULL                        /* Pass NULL for defaults */
                );
        if (!hAkeEntry->hThread)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hAkeEntry=%p: state=%s: B_Thread_Create() Failed!" BIP_MSG_PRE_ARG,
                        (void *)hAkeEntry, BIP_ToStr_BIP_DtcpIpClientFactoryAkeEntryState(hAkeEntry->state) ));
            hAkeEntry->state = BIP_DtcpIpClientFactoryAkeEntryState_eAkeDone;
            hAkeEntry->dtcpIpAkeCompletionStatus = BIP_ERR_OUT_OF_SYSTEM_MEMORY;
        }
        else
        {
            hAkeEntry->state = BIP_DtcpIpClientFactoryAkeEntryState_eWaitingForAke;
            hAkeEntry->dtcpIpAkeCompletionStatus = BIP_INF_IN_PROGRESS;

            BDBG_MSG(( BIP_MSG_PRE_FMT "hAkeEntry=%p state=%s: Scheduled DoAke processing via BIP Context for serverIp=%s serverPort=%s" BIP_MSG_PRE_ARG,
                        (void *)hAkeEntry, BIP_ToStr_BIP_DtcpIpClientFactoryAkeEntryState(hAkeEntry->state), BIP_String_GetString(hAkeEntry->hServerIp), BIP_String_GetString(hAkeEntry->hServerPort) ));
        }
    }

    if (hAkeEntry->state == BIP_DtcpIpClientFactoryAkeEntryState_eWaitingForAke)
    {
        if (hAkeEntry->dtcpIpAkeCompletionStatus != BIP_INF_IN_PROGRESS)
        {
            hAkeEntry->state = BIP_DtcpIpClientFactoryAkeEntryState_eAkeDone;
        }
    }

    if (hAkeEntry->state == BIP_DtcpIpClientFactoryAkeEntryState_eAkeDone)
    {
        /* Check the completion status of Ake & Complete the DoAke() Request. */
        if (hAkeEntry->dtcpIpAkeCompletionStatus == BIP_SUCCESS)
        {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hAkeEntry=%p: DoAke Successful with ServerIp=%s ServerDtcpIpPort=%s pDtcpIpLibAkeCtx=%p" BIP_MSG_PRE_ARG,
                        (void *)hAkeEntry, BIP_String_GetString(hAkeEntry->hServerIp), BIP_String_GetString(hAkeEntry->hServerPort), hAkeEntry->pDtcpIpLibAkeCtx ));
        }
        else
        {
            /* Error case. */
            /* We keep the AkeEntry in the AkeEntry list as caller may re-attempt AKE for the next channel. */
            /* Entries only get removed & freed during DtcpIpClientFactory_Uninit() */
            BDBG_ERR(( BIP_MSG_PRE_FMT "hAkeEntry=%p: DoAke Failed with ServerIp=%s ServerDtcpIpPort=%s" BIP_MSG_PRE_ARG,
                        (void *)hAkeEntry, BIP_String_GetString(hAkeEntry->hServerIp), BIP_String_GetString(hAkeEntry->hServerPort) ));
        }

        hAkeEntry->state = BIP_DtcpIpClientFactoryAkeEntryState_eIdle;
        completionStatus = hAkeEntry->dtcpIpAkeCompletionStatus;

        if ( BIP_Arb_IsBusy(hArb = hDtcpIpClientFactory->doAkeApi.hArb))
        {
            *hDtcpIpClientFactory->doAkeApi.phAkeEntry = hAkeEntry;
            BIP_Arb_CompleteRequest(hArb, completionStatus);
        }
        else
        {
            BDBG_ASSERT(NULL);
        }
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "hAkeEntry=%p state=%s completionStatus=%s" BIP_MSG_PRE_ARG, (void *)hAkeEntry, BIP_ToStr_BIP_DtcpIpClientFactoryAkeEntryState(hAkeEntry->state), BIP_StatusGetText(completionStatus) ));

    return (completionStatus);
} /* processDtcpIpClientFactoryAkeEntryState_locked */

static void processDtcpIpClientFactoryState(
    void                                    *pObject,
    int                                     value,
    BIP_Arb_ThreadOrigin                    threadOrigin
    )
{
    BIP_DtcpIpClientFactoryHandle           hDtcpIpClientFactory = pObject;
    BIP_ArbHandle                           hArb;
    BIP_Status                              completionStatus = BIP_INF_IN_PROGRESS;
    BIP_DtcpIpClientFactoryAkeEntryHandle   hAkeEntry = NULL;

    BSTD_UNUSED(value);
    BDBG_ASSERT(hDtcpIpClientFactory);

    B_Mutex_Lock( hDtcpIpClientFactory->hStateMutex );
    BDBG_MSG(( BIP_MSG_PRE_FMT  "ENTER --------------------> hDtcpIpClientFactory=%p " BIP_MSG_PRE_ARG, (void *)(hDtcpIpClientFactory) ));

    /* Check if we have a new _DoAke() API called from App. */
    if (BIP_Arb_IsNew(hArb = hDtcpIpClientFactory->doAkeApi.hArb))
    {
        if (hDtcpIpClientFactory->state != BIP_DtcpIpClientFactoryState_eInitialized)
        {
            BDBG_WRN(( BIP_MSG_PRE_FMT "BIP_DtcpIpClientFactory_DoAke() is not allowed until BIP_DtcpIpClientFactory_Init() is called once!" BIP_MSG_PRE_ARG ));
            completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, completionStatus);
        }
        else
        {
            /* App called _DoAke(), lets start that processing. */
            BIP_Arb_AcceptRequest(hArb);
            BDBG_MSG(( BIP_MSG_PRE_FMT "Accepted BIP_DtcpIpClientFactory_DoAke() Arb Request!" BIP_MSG_PRE_ARG ));

            /* See if we already have an AkeEntry for this Server. */
            for (
                    hAkeEntry = BLST_Q_FIRST(&hDtcpIpClientFactory->akeEntry_head);
                    hAkeEntry;
                    hAkeEntry = BLST_Q_NEXT(hAkeEntry, akeEntry_link)
                )
            {
                if (
                        strncmp(hDtcpIpClientFactory->doAkeApi.pServerIp, BIP_String_GetString(hAkeEntry->hServerIp), strlen(BIP_String_GetString(hAkeEntry->hServerIp))) == 0 &&
                        strncmp(hDtcpIpClientFactory->doAkeApi.pServerPort, BIP_String_GetString(hAkeEntry->hServerPort), strlen(BIP_String_GetString(hAkeEntry->hServerPort))) == 0
                   )
                {
                    /* Match found, use this entry. */
                    BDBG_MSG(( BIP_MSG_PRE_FMT "Using existing hAkeEntry=%p for serverIp=%s serverPort=%s" BIP_MSG_PRE_ARG, (void *)hAkeEntry, hDtcpIpClientFactory->doAkeApi.pServerIp, hDtcpIpClientFactory->doAkeApi.pServerPort ));
                    break;
                }
            }

            /* No Ake entry for this Server, so create one. */
            if (hAkeEntry == NULL)
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "No existing AkeEntry found, create new one for serverIp=%s serverPort=%s" BIP_MSG_PRE_ARG, hDtcpIpClientFactory->doAkeApi.pServerIp, hDtcpIpClientFactory->doAkeApi.pServerPort ));
                hAkeEntry = B_Os_Calloc( 1, sizeof( BIP_DtcpIpClientFactoryAkeEntry ));

                if (hAkeEntry)
                {
                    hAkeEntry->hServerIp = BIP_String_CreateFromChar(hDtcpIpClientFactory->doAkeApi.pServerIp);
                    hAkeEntry->hServerPort = BIP_String_CreateFromChar(hDtcpIpClientFactory->doAkeApi.pServerPort);
                    hAkeEntry->hStateMutex = B_Mutex_Create( NULL );
                    BDBG_OBJECT_SET( hAkeEntry, BIP_DtcpIpClientFactoryAkeEntry );
                }

                /* Now check for errors during any of these _Create() */
                if (hAkeEntry == NULL)
                {
                    BDBG_ERR(( BIP_MSG_PRE_FMT "Failed to Allocate a New AkeEntry" BIP_MSG_PRE_ARG ));

                    completionStatus = BIP_ERR_OUT_OF_SYSTEM_MEMORY;
                    BIP_Arb_CompleteRequest(hArb, completionStatus);
                }
                else if (hAkeEntry->hServerIp == NULL || hAkeEntry->hServerPort == NULL || hAkeEntry->hStateMutex == NULL)
                {
                    BDBG_ERR(( BIP_MSG_PRE_FMT "Failed to Create a AkeEntry element: hAkeEntry=%p hServerIp=%p hServerPort=%p hStateMutex=%p" BIP_MSG_PRE_ARG,
                                (void *)hAkeEntry, (void *)hAkeEntry->hServerIp, (void *)hAkeEntry->hServerPort, (void *)hAkeEntry->hStateMutex ));
                    destroyAkeEntry(hAkeEntry);
                    hAkeEntry = NULL;

                    completionStatus = BIP_ERR_OUT_OF_SYSTEM_MEMORY;
                    BIP_Arb_CompleteRequest(hArb, completionStatus);
                }
                else
                {
                    BDBG_MSG(( BIP_MSG_PRE_FMT "Created a New AkeEntry: hAkeEntry=%p for serverIp=%s serverPort=%s" BIP_MSG_PRE_ARG,
                                (void *)hAkeEntry, hDtcpIpClientFactory->doAkeApi.pServerIp, hDtcpIpClientFactory->doAkeApi.pServerPort ));

                    /* Now insert it into the AkeEntry list. */
                    BLST_Q_INSERT_TAIL( &hDtcpIpClientFactory->akeEntry_head, hAkeEntry, akeEntry_link);
                    hAkeEntry->hDtcpIpClientFactory = hDtcpIpClientFactory;

                    completionStatus = BIP_INF_IN_PROGRESS;
                    /* This AkeEntry is further processed in its own state machine below. */
                }
            }
            else
            {
                /* AkeEntry is found for this Server, we will run its state machine below to process it. */
                completionStatus = BIP_INF_IN_PROGRESS;
            }

            if (completionStatus == BIP_INF_IN_PROGRESS)
            {
                /* For this _DoAke() API, lets change the state to kick-off AKE processing. */
                hAkeEntry->state = BIP_DtcpIpClientFactoryAkeEntryState_eNewAke;
            }
        }
    }
    else if (BIP_Arb_IsNew(hArb = hDtcpIpClientFactory->getAkeEntryStatusApi.hArb))
    {
        BIP_Arb_AcceptRequest(hArb);
        BDBG_MSG(( BIP_MSG_PRE_FMT "Accepted BIP_DtcpIpClientFactory_GetAkeEntryStatus() Arb Request!" BIP_MSG_PRE_ARG ));

        hAkeEntry = hDtcpIpClientFactory->getAkeEntryStatusApi.hAkeEntry;
        /* Process this AkeEntry in its own state machine below. */
        if (hAkeEntry->pDtcpIpLibAkeCtx)
        {
            hDtcpIpClientFactory->getAkeEntryStatusApi.pAkeEntryStatus->pDtcpIpLibAkeCtx = hAkeEntry->pDtcpIpLibAkeCtx;
            hDtcpIpClientFactory->getAkeEntryStatusApi.pAkeEntryStatus->pServerIp = BIP_String_GetString(hAkeEntry->hServerIp);
            hDtcpIpClientFactory->getAkeEntryStatusApi.pAkeEntryStatus->pServerPort = BIP_String_GetString(hAkeEntry->hServerPort);
            BDBG_MSG(( BIP_MSG_PRE_FMT "hAkeEntry=%p: GetStatus done: ServerIp=%s ServerDtcpIpPort=%s pDtcpIpLibAkeCtx=%p" BIP_MSG_PRE_ARG,
                        (void *)hAkeEntry, BIP_String_GetString(hAkeEntry->hServerIp), BIP_String_GetString(hAkeEntry->hServerPort), hAkeEntry->pDtcpIpLibAkeCtx ));
        }
        else
        {
            hDtcpIpClientFactory->getAkeEntryStatusApi.pAkeEntryStatus->pDtcpIpLibAkeCtx = NULL;
            hDtcpIpClientFactory->getAkeEntryStatusApi.pAkeEntryStatus->pServerIp = NULL;
            hDtcpIpClientFactory->getAkeEntryStatusApi.pAkeEntryStatus->pServerPort = NULL;
        }

        completionStatus = BIP_SUCCESS;
        BIP_Arb_CompleteRequest(hArb, completionStatus);
    }


    if (hAkeEntry && completionStatus == BIP_INF_IN_PROGRESS)
    {
        B_Mutex_Lock( hAkeEntry->hStateMutex );
        completionStatus = processDtcpIpClientFactoryAkeEntryState_locked(hAkeEntry, threadOrigin);
        B_Mutex_Unlock( hAkeEntry->hStateMutex );
    }
    /*
     * Done with state processing. We have to unlock state machine before asking Arb to do any deferred callbacks!
     */
    B_Mutex_Unlock( hDtcpIpClientFactory->hStateMutex );

    BDBG_MSG(( BIP_MSG_PRE_FMT "EXIT: completionStatus=%s <-------------------" BIP_MSG_PRE_ARG, BIP_StatusGetText(completionStatus) ));

    /* Tell ARB to do any deferred work. */
    completionStatus = BIP_Arb_DoDeferred( hDtcpIpClientFactory->doAkeApi.hArb, threadOrigin );
    BDBG_ASSERT( completionStatus == BIP_SUCCESS );
    return;
} /* processDtcpIpClientFactoryState */

#if 0
/******************************  DtcpIpClient State Machine logic. ******************************/
/* TODO: DtcpIpClient interface is not yet implemented as it is only needed when we dont use PBIP for streaming out. */
typedef struct BIP_DtcpIpClient
{
    B_MutexHandle                           hStateMutex;
    BIP_DtcpIpClientFactoryAkeEntryHandle   hDtcpIpClientFactoryAkeEntry;
    void                                    *pDtcpIpLibDecryptionCtx;
} BIP_DtcpIpClient;
/* DtcpIpClietn: Create, Destroy, DoAke, OpenDecryptionContext, CloseDecryptionContext, GetStatus, Decrypt, */
#endif
