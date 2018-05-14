/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/
#include "bip_priv.h"
#include "bip_http_streamer_impl.h"
#include "bip_streamer_priv.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

BDBG_MODULE( bip_http_streamer );
BDBG_OBJECT_ID( BIP_HttpStreamer );

BIP_CLASS_DECLARE(BIP_HttpStreamer);
BIP_SETTINGS_ID(BIP_HttpStreamerOutputSettings);
BIP_SETTINGS_ID(BIP_HttpStreamerCreateSettings);
BIP_SETTINGS_ID(BIP_HttpStreamerStartSettings);
BIP_SETTINGS_ID(BIP_HttpStreamerProcessRequestSettings);
BIP_SETTINGS_ID_DECLARE(BIP_TranscodeProfile);
BIP_SETTINGS_ID_DECLARE(BIP_TranscodeNexusHandles);

/* Forward declaration of state processing function */
void processHttpStreamerState( void *hObject, int value, BIP_Arb_ThreadOrigin threadOrigin );

void BIP_HttpStreamer_GetDefaultSettings(
    BIP_HttpStreamerSettings *pSettings
    )
{
    BKNI_Memset( pSettings, 0, sizeof( BIP_HttpStreamerSettings ));
}

static void httpStreamerDestroy(
    BIP_HttpStreamerHandle hHttpStreamer
    )
{
    BIP_Status    rc = BIP_SUCCESS;

    if (!hHttpStreamer) return;
    BDBG_MSG(( BIP_MSG_PRE_FMT "Destroying hHttpStreamer %p" BIP_MSG_PRE_ARG, (void *)hHttpStreamer ));

    rc = BIP_CLASS_REMOVE_INSTANCE( BIP_HttpStreamer, hHttpStreamer );
    if (rc == BIP_ERR_INVALID_HANDLE) return;   /* Object has already been destroyed (by another thread). */

    if (hHttpStreamer->hStreamer) BIP_Streamer_Destroy(hHttpStreamer->hStreamer);
    if (hHttpStreamer->printStatusApi.hArb) BIP_Arb_Destroy(hHttpStreamer->printStatusApi.hArb);
    if (hHttpStreamer->getStatusApi.hArb) BIP_Arb_Destroy(hHttpStreamer->getStatusApi.hArb);
    if (hHttpStreamer->setSettingsApi.hArb) BIP_Arb_Destroy(hHttpStreamer->setSettingsApi.hArb);
    if (hHttpStreamer->getSettingsApi.hArb) BIP_Arb_Destroy(hHttpStreamer->getSettingsApi.hArb);
    if (hHttpStreamer->fileInputSettingsApi.hArb) BIP_Arb_Destroy(hHttpStreamer->fileInputSettingsApi.hArb);
    if (hHttpStreamer->tunerInputSettingsApi.hArb) BIP_Arb_Destroy(hHttpStreamer->tunerInputSettingsApi.hArb);
    if (hHttpStreamer->ipInputSettingsApi.hArb) BIP_Arb_Destroy(hHttpStreamer->ipInputSettingsApi.hArb);
#if NEXUS_HAS_HDMI_INPUT
    if (hHttpStreamer->hdmiInputSettingsApi.hArb) BIP_Arb_Destroy(hHttpStreamer->hdmiInputSettingsApi.hArb);
#endif
    if (hHttpStreamer->recpumpInputSettingsApi.hArb) BIP_Arb_Destroy(hHttpStreamer->recpumpInputSettingsApi.hArb);
    if (hHttpStreamer->outputSettingsApi.hArb) BIP_Arb_Destroy(hHttpStreamer->outputSettingsApi.hArb);
    if (hHttpStreamer->setProgramApi.hArb) BIP_Arb_Destroy(hHttpStreamer->setProgramApi.hArb);
    if (hHttpStreamer->addTrackApi.hArb) BIP_Arb_Destroy(hHttpStreamer->addTrackApi.hArb);
    if (hHttpStreamer->setTranscodeNexusHandlesApi.hArb) BIP_Arb_Destroy(hHttpStreamer->setTranscodeNexusHandlesApi.hArb);
    if (hHttpStreamer->addTranscodeProfileApi.hArb) BIP_Arb_Destroy(hHttpStreamer->addTranscodeProfileApi.hArb);
    if (hHttpStreamer->getResponseHeaderApi.hArb) BIP_Arb_Destroy(hHttpStreamer->getResponseHeaderApi.hArb);
    if (hHttpStreamer->setResponseHeaderApi.hArb) BIP_Arb_Destroy(hHttpStreamer->setResponseHeaderApi.hArb);
    if (hHttpStreamer->startApi.hArb) BIP_Arb_Destroy(hHttpStreamer->startApi.hArb);
    if (hHttpStreamer->stopApi.hArb) BIP_Arb_Destroy(hHttpStreamer->stopApi.hArb);
    if (hHttpStreamer->processRequestApi.hArb) BIP_Arb_Destroy(hHttpStreamer->processRequestApi.hArb);
    if (hHttpStreamer->destroyApi.hArb) BIP_Arb_Destroy(hHttpStreamer->destroyApi.hArb);

    if (hHttpStreamer->hStateMutex) B_Mutex_Destroy( hHttpStreamer->hStateMutex );

    if (hHttpStreamer->response.hHttpResponse) BIP_HttpResponse_Destroy(hHttpStreamer->response.hHttpResponse, hHttpStreamer);

    if( hHttpStreamer->hCustomResponseHeaderList) BIP_HttpHeaderList_Destroy( hHttpStreamer->hCustomResponseHeaderList, &hHttpStreamer->createSettings);

    if ( hHttpStreamer->atomPipe ) batom_pipe_destroy( hHttpStreamer->atomPipe );
    if ( hHttpStreamer->atomFactory) batom_factory_destroy( hHttpStreamer->atomFactory );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p: Destroyed" BIP_MSG_PRE_ARG, (void *)hHttpStreamer ));

    BDBG_OBJECT_DESTROY( hHttpStreamer, BIP_HttpStreamer );
    B_Os_Free( hHttpStreamer );

} /* httpStreamerDestroy */

static void httpStreamerEndOfStreamingCallback( void *hObject, int value )
{
    /* Callback from BIP_Streamer. */
    BIP_Status bipStatus;
    BIP_HttpStreamerHandle hHttpStreamer = hObject;

    bipStatus = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_HttpStreamer, hHttpStreamer);
    if (bipStatus != BIP_SUCCESS) { return; }

    BDBG_ASSERT(hHttpStreamer);
    if (hHttpStreamer)
    {
        BDBG_OBJECT_ASSERT( hHttpStreamer, BIP_HttpStreamer);
        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer:state %p: %d" BIP_MSG_PRE_ARG, (void *)hHttpStreamer, hHttpStreamer->state ));

        B_Mutex_Lock( hHttpStreamer->hStateMutex );
        hHttpStreamer->state = BIP_HttpStreamerState_eStreamingDone;
        B_Mutex_Unlock( hHttpStreamer->hStateMutex );
    }
    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);

    processHttpStreamerState( (BIP_HttpStreamerHandle) hObject, value, BIP_Arb_ThreadOrigin_eBipCallback);
}

BIP_HttpStreamerHandle BIP_HttpStreamer_Create(
    const BIP_HttpStreamerCreateSettings *pCreateSettings
    )
{
    BIP_Status                      bipStatus;
    BIP_HttpStreamerHandle          hHttpStreamer = NULL;
    BIP_HttpStreamerCreateSettings  defaultSettings;

    BIP_SETTINGS_ASSERT(pCreateSettings, BIP_HttpStreamerCreateSettings);

    /* Create the httpStreamer object */
    hHttpStreamer = B_Os_Calloc( 1, sizeof( BIP_HttpStreamer ));
    BIP_CHECK_GOTO(( hHttpStreamer != NULL ), ( "Failed to allocate memory (%zu bytes) for HttpStreamer Object", sizeof(BIP_HttpStreamer) ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    bipStatus = BIP_CLASS_ADD_INSTANCE(BIP_HttpStreamer, hHttpStreamer);
    BIP_CHECK_GOTO((bipStatus==BIP_SUCCESS), ( "BIP_CLASS_ADD_INSTANCE failed" ), error, bipStatus, bipStatus );

    BDBG_OBJECT_SET( hHttpStreamer, BIP_HttpStreamer );

    /* Create mutex to synchronize state machine from being run via callbacks (BIP_HttpSocket or timer) & Caller calling APIs. */
    hHttpStreamer->hStateMutex = B_Mutex_Create(NULL);
    BIP_CHECK_GOTO(( hHttpStreamer->hStateMutex ), ( "B_Mutex_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    if (NULL == pCreateSettings)
    {
        BIP_HttpStreamer_GetDefaultCreateSettings( &defaultSettings );
        pCreateSettings = &defaultSettings;
    }
    hHttpStreamer->createSettings = *pCreateSettings;

    BIP_HttpStreamer_GetDefaultSettings(&hHttpStreamer->settings);

    /* Create API ARBs: one per API */
    hHttpStreamer->getSettingsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpStreamer->getSettingsApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hHttpStreamer->setSettingsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpStreamer->setSettingsApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hHttpStreamer->getStatusApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpStreamer->getStatusApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hHttpStreamer->printStatusApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpStreamer->printStatusApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hHttpStreamer->fileInputSettingsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpStreamer->fileInputSettingsApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hHttpStreamer->tunerInputSettingsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpStreamer->tunerInputSettingsApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hHttpStreamer->ipInputSettingsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpStreamer->ipInputSettingsApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

#if NEXUS_HAS_HDMI_INPUT
    hHttpStreamer->hdmiInputSettingsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpStreamer->hdmiInputSettingsApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
#endif

    hHttpStreamer->recpumpInputSettingsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpStreamer->recpumpInputSettingsApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hHttpStreamer->outputSettingsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpStreamer->outputSettingsApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    /* . */
    hHttpStreamer->setProgramApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpStreamer->setProgramApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hHttpStreamer->addTrackApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpStreamer->addTrackApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hHttpStreamer->setTranscodeNexusHandlesApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpStreamer->setTranscodeNexusHandlesApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hHttpStreamer->addTranscodeProfileApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpStreamer->addTranscodeProfileApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hHttpStreamer->getResponseHeaderApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpStreamer->getResponseHeaderApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hHttpStreamer->setResponseHeaderApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpStreamer->setResponseHeaderApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hHttpStreamer->startApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpStreamer->startApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hHttpStreamer->stopApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpStreamer->stopApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hHttpStreamer->destroyApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpStreamer->destroyApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hHttpStreamer->processRequestApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpStreamer->processRequestApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    /* Now create the BIP_Streamer object. */
    {
        BIP_StreamerCreateSettings settings;

        BIP_Streamer_GetDefaultCreateSettings( &settings );
        settings.endOfStreamingCallback.context     = hHttpStreamer;
        settings.endOfStreamingCallback.callback    = httpStreamerEndOfStreamingCallback;
        settings.softErrorCallback.context          = hHttpStreamer;
        settings.softErrorCallback.callback         = httpStreamerEndOfStreamingCallback;
        hHttpStreamer->hStreamer                    = BIP_Streamer_Create( &settings );
        BIP_CHECK_GOTO(( hHttpStreamer->hStreamer ), ( "BIP_Streamer_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

        hHttpStreamer->pStreamer = BIP_Streamer_GetObject_priv( hHttpStreamer->hStreamer );
        BIP_CHECK_GOTO(( hHttpStreamer->pStreamer ), ( "BIP_Streamer_GetStreamerOjbect Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
    }

    /* Create a response object that we can use for sending HTTP Response. */
    {
        hHttpStreamer->response.hHttpResponse = BIP_HttpResponse_Create(hHttpStreamer, NULL);
        BIP_CHECK_GOTO(( hHttpStreamer->response.hHttpResponse ), ( "BIP_HttpResponse_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
    }

    {
        hHttpStreamer->atomFactory = batom_factory_create( bkni_alloc, 64 );
        BIP_CHECK_GOTO(( hHttpStreamer->atomFactory ), ( "batom_factory_create() Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

        hHttpStreamer->atomPipe = batom_pipe_create( hHttpStreamer->atomFactory );
        BIP_CHECK_GOTO(( hHttpStreamer->atomPipe ), ( "batom_factory_create() Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
    }

    {
        hHttpStreamer->hCustomResponseHeaderList = BIP_HttpHeaderList_Create( &hHttpStreamer->createSettings );
        BIP_CHECK_GOTO(( hHttpStreamer->hCustomResponseHeaderList ),    ( "BIP_HttpHeaderList_Create() failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
    }

    hHttpStreamer->state = BIP_HttpStreamerState_eIdle;
    BDBG_MSG(( BIP_MSG_PRE_FMT "Created hHttpStreamer %p: state %d" BIP_MSG_PRE_ARG, (void *)hHttpStreamer, hHttpStreamer->state));

    BDBG_MSG((    BIP_MSG_PRE_FMT "Created: " BIP_HTTP_STREAMER_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_HTTP_STREAMER_PRINTF_ARG(hHttpStreamer)));
    BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Created: " BIP_HTTP_STREAMER_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_HTTP_STREAMER_PRINTF_ARG(hHttpStreamer)));

    return ( hHttpStreamer );

error:
    httpStreamerDestroy( hHttpStreamer );
    return ( NULL );
} /* BIP_HttpStreamer_Create */

/**
 * Summary:
 * Destroy http socket
 *
 * Description:
 **/
void BIP_HttpStreamer_Destroy(
    BIP_HttpStreamerHandle hHttpStreamer
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    bipStatus = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_HttpStreamer, hHttpStreamer);
    BIP_CHECK_GOTO((bipStatus==BIP_SUCCESS), ("BIP_CLASS_LOCK_AND_CHECK_INSTANCE failed."), error, bipStatus, bipStatus);

    BDBG_OBJECT_ASSERT( hHttpStreamer, BIP_HttpStreamer );

    BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Destroying: " BIP_HTTP_STREAMER_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_HTTP_STREAMER_PRINTF_ARG(hHttpStreamer)));

    /* Serialize access to Settings state among another thread calling the same _GetSettings API. */
    hArb = hHttpStreamer->destroyApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    if(bipStatus != BIP_SUCCESS)
    {
        BDBG_ERR((BIP_MSG_PRE_FMT "BIP_Arb_Acquire() Failed" BIP_MSG_PRE_ARG));
        BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);
        goto error;
    }

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpStreamer;
    arbSettings.arbProcessor = processHttpStreamerState;
    arbSettings.waitIfBusy = true;

    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);

    /* Invoke state machine to notify it about object being destroyed. */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_HttpStreamer_Destroy" ), error, bipStatus, bipStatus );

    /* Now release resources & destroy HttpStreamer object. */
error:
    httpStreamerDestroy( hHttpStreamer );

} /* BIP_HttpStreamer_Destroy */

void BIP_HttpStreamer_GetSettings(
        BIP_HttpStreamerHandle    hHttpStreamer,
        BIP_HttpStreamerSettings *pSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    bipStatus = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_HttpStreamer, hHttpStreamer);
    BIP_CHECK_GOTO((bipStatus==BIP_SUCCESS), ("BIP_CLASS_LOCK_AND_CHECK_INSTANCE failed."), error, bipStatus, bipStatus);

    BDBG_OBJECT_ASSERT( hHttpStreamer, BIP_HttpStreamer );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hHttpStreamer));

    BIP_CHECK_GOTO(( pSettings ), ( "pSettings can't be NULL" ), error_locked, BIP_ERR_INVALID_PARAMETER, bipStatus );

    /* Serialize access to Settings state among another thread calling the same _GetSettings API. */
    hArb = hHttpStreamer->getSettingsApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error_locked, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hHttpStreamer->getSettingsApi.pSettings = pSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpStreamer;
    arbSettings.arbProcessor = processHttpStreamerState;
    arbSettings.waitIfBusy = true;

    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_HttpStreamer_GetSettings" ), error, bipStatus, bipStatus );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, bipStatus ));

    return;

error_locked:
    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, bipStatus ));

    return;
}

BIP_Status BIP_HttpStreamer_SetSettings(
    BIP_HttpStreamerHandle    hHttpStreamer,
    BIP_HttpStreamerSettings *pSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    bipStatus = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_HttpStreamer, hHttpStreamer);
    BIP_CHECK_GOTO((bipStatus==BIP_SUCCESS), ("BIP_CLASS_LOCK_AND_CHECK_INSTANCE failed."), error, bipStatus, bipStatus);

    BDBG_OBJECT_ASSERT( hHttpStreamer, BIP_HttpStreamer );
    BDBG_ASSERT( pSettings );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hHttpStreamer));

    BIP_CHECK_GOTO(( pSettings ), ( "pSettings can't be NULL" ), error_locked, BIP_ERR_INVALID_PARAMETER, bipStatus );

    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hHttpStreamer->setSettingsApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error_locked, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hHttpStreamer->setSettingsApi.pSettings = pSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpStreamer;
    arbSettings.arbProcessor = processHttpStreamerState;
    arbSettings.waitIfBusy = true;;

    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_HttpStreamer_SetSettings" ), error, bipStatus, bipStatus );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, bipStatus ));
    return( bipStatus );

error_locked:
    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);
error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, bipStatus ));

    return( bipStatus );
} /* BIP_HttpStreamer_SetSettings */

BIP_Status BIP_HttpStreamer_SetFileInputSettings(
    BIP_HttpStreamerHandle          hHttpStreamer,
    const char                      *pMediaFileAbsolutePathName,
    BIP_StreamerStreamInfo          *pStreamerStreamInfo,
    BIP_StreamerFileInputSettings   *pFileInputSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_StreamerFileInputSettings defaultFileInputSettings;

    bipStatus = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_HttpStreamer, hHttpStreamer);
    BIP_CHECK_GOTO((bipStatus==BIP_SUCCESS), ("BIP_CLASS_LOCK_AND_CHECK_INSTANCE failed."), error, bipStatus, bipStatus);

    BDBG_OBJECT_ASSERT( hHttpStreamer, BIP_HttpStreamer );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hHttpStreamer));

    BIP_CHECK_GOTO(( pMediaFileAbsolutePathName ), ( "pMediaFileAbsolutePathName can't be NULL" ), error_locked, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pStreamerStreamInfo), ( "pStreamerStreamInfo can't be NULL" ), error_locked, BIP_ERR_INVALID_PARAMETER, bipStatus );

    /* Note: Rest of parameter validation happens in the BIP_Streamer class. */

    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hHttpStreamer->fileInputSettingsApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error_locked, bipStatus, bipStatus );

    if ( pFileInputSettings == NULL )
    {
        BIP_Streamer_GetDefaultFileInputSettings( &defaultFileInputSettings );
        pFileInputSettings = &defaultFileInputSettings;
    }
    /* Move the API arguments into it's argument list so the state machine can find them. */
    hHttpStreamer->fileInputSettingsApi.pStreamerStreamInfo = pStreamerStreamInfo;
    hHttpStreamer->fileInputSettingsApi.pMediaFileAbsolutePathName = pMediaFileAbsolutePathName;
    hHttpStreamer->fileInputSettingsApi.pFileInputSettings = pFileInputSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpStreamer;
    arbSettings.arbProcessor = processHttpStreamerState;
    arbSettings.waitIfBusy = true;;

    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_HttpStreamer_SetFileInputSettings" ), error, bipStatus, bipStatus );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, bipStatus ));
    return( bipStatus );

error_locked:
    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);
error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, bipStatus ));

    return( bipStatus );
} /* BIP_HttpStreamer_SetSettings */

BIP_Status BIP_HttpStreamer_SetTunerInputSettings(
    BIP_HttpStreamerHandle          hHttpStreamer,
    NEXUS_ParserBand                hParserBand,
    BIP_StreamerStreamInfo          *pStreamerStreamInfo,
    BIP_StreamerTunerInputSettings  *pTunerInputSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_StreamerTunerInputSettings  defaultTunerInputSettings;

    bipStatus = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_HttpStreamer, hHttpStreamer);
    BIP_CHECK_GOTO((bipStatus==BIP_SUCCESS), ("BIP_CLASS_LOCK_AND_CHECK_INSTANCE failed."), error, bipStatus, bipStatus);

    BDBG_OBJECT_ASSERT( hHttpStreamer, BIP_HttpStreamer );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hHttpStreamer));

    /* Note: Rest of parameter validation happens in the BIP_Streamer class. */

    if ( pTunerInputSettings == NULL )
    {
        BIP_Streamer_GetDefaultTunerInputSettings( &defaultTunerInputSettings );
        pTunerInputSettings = &defaultTunerInputSettings;
    }

    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hHttpStreamer->tunerInputSettingsApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error_locked, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hHttpStreamer->tunerInputSettingsApi.hParserBand = hParserBand;
    hHttpStreamer->tunerInputSettingsApi.pStreamerStreamInfo = pStreamerStreamInfo;
    hHttpStreamer->tunerInputSettingsApi.pTunerInputSettings = pTunerInputSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpStreamer;
    arbSettings.arbProcessor = processHttpStreamerState;
    arbSettings.waitIfBusy = true;;

    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_HttpStreamer_SetTunerInputSettings" ), error, bipStatus, bipStatus );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, bipStatus ));
    return( bipStatus );

error_locked:
    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);
error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, bipStatus ));

    return( bipStatus );
} /* BIP_HttpStreamer_SetSettings */

BIP_Status BIP_HttpStreamer_SetIpInputSettings(
    BIP_HttpStreamerHandle          hHttpStreamer,
    BIP_PlayerHandle                hPlayer,
    BIP_StreamerStreamInfo          *pStreamerStreamInfo,
    BIP_StreamerIpInputSettings     *pIpInputSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_StreamerIpInputSettings  defaultIpInputSettings;

    bipStatus = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_HttpStreamer, hHttpStreamer);
    BIP_CHECK_GOTO((bipStatus==BIP_SUCCESS), ("BIP_CLASS_LOCK_AND_CHECK_INSTANCE failed."), error, bipStatus, bipStatus);

    BDBG_OBJECT_ASSERT( hHttpStreamer, BIP_HttpStreamer );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hHttpStreamer));

    BIP_CHECK_GOTO(( hPlayer ), ( "hPlayer pointer can't be NULL" ), error_locked, BIP_ERR_INVALID_PARAMETER, bipStatus );
    /* Note: Rest of parameter validation happens in the BIP_Streamer class. */

    if ( pIpInputSettings == NULL )
    {
        BIP_Streamer_GetDefaultIpInputSettings( &defaultIpInputSettings );
        pIpInputSettings = &defaultIpInputSettings;
    }

    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hHttpStreamer->ipInputSettingsApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error_locked, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hHttpStreamer->ipInputSettingsApi.hPlayer = hPlayer;
    hHttpStreamer->ipInputSettingsApi.pStreamerStreamInfo = pStreamerStreamInfo;
    hHttpStreamer->ipInputSettingsApi.pIpInputSettings = pIpInputSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpStreamer;
    arbSettings.arbProcessor = processHttpStreamerState;
    arbSettings.waitIfBusy = true;;

    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_HttpStreamer_SetIpInputSettings" ), error, bipStatus, bipStatus );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, bipStatus ));
    return( bipStatus );

error_locked:
    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);
error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, bipStatus ));

    return( bipStatus );
} /* BIP_HttpStreamer_SetSettings */

BIP_Status BIP_HttpStreamer_SetRecpumpInputSettings(
    BIP_HttpStreamerHandle    hHttpStreamer,
    NEXUS_RecpumpHandle       hRecpump,
    BIP_StreamerRecpumpInputSettings *pRecpumpInputSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_StreamerRecpumpInputSettings defaultSettings;

    bipStatus = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_HttpStreamer, hHttpStreamer);
    BIP_CHECK_GOTO((bipStatus==BIP_SUCCESS), ("BIP_CLASS_LOCK_AND_CHECK_INSTANCE failed."), error, bipStatus, bipStatus);

    BDBG_OBJECT_ASSERT( hHttpStreamer, BIP_HttpStreamer );
    BDBG_ASSERT( pRecpumpInputSettings );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hHttpStreamer));

    BIP_CHECK_GOTO(( hRecpump ), ( "hRecpump can't be NULL" ), error_locked, BIP_ERR_INVALID_PARAMETER, bipStatus );

    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hHttpStreamer->recpumpInputSettingsApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error_locked, bipStatus, bipStatus );

    if ( pRecpumpInputSettings == NULL )
    {
        BIP_Streamer_GetDefaultRecpumpInputSettings( &defaultSettings );
        pRecpumpInputSettings = &defaultSettings;
    }

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hHttpStreamer->recpumpInputSettingsApi.pRecpumpInputSettings = pRecpumpInputSettings;
    hHttpStreamer->recpumpInputSettingsApi.hRecpump = hRecpump;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpStreamer;
    arbSettings.arbProcessor = processHttpStreamerState;
    arbSettings.waitIfBusy = true;;

    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_HttpStreamer_SetRecpumpInputSettings" ), error, bipStatus, bipStatus );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, bipStatus ));
    return( bipStatus );

error_locked:
    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);
error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, bipStatus ));

    return( bipStatus );
} /* BIP_HttpStreamer_SetRecpumpInputSettings */

#if NEXUS_HAS_HDMI_INPUT
BIP_Status BIP_HttpStreamer_SetHdmiInputSettings(
    BIP_HttpStreamerHandle    hHttpStreamer,
    NEXUS_HdmiInputHandle     hHdmiInput,
    BIP_StreamerHdmiInputSettings *pHdmiInputSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_StreamerHdmiInputSettings defaultSettings;

    bipStatus = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_HttpStreamer, hHttpStreamer);
    BIP_CHECK_GOTO((bipStatus==BIP_SUCCESS), ("BIP_CLASS_LOCK_AND_CHECK_INSTANCE failed."), error, bipStatus, bipStatus);

    BDBG_OBJECT_ASSERT( hHttpStreamer, BIP_HttpStreamer );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hHttpStreamer));

    BIP_CHECK_GOTO(( hHdmiInput ), ( "hHdmiInput can't be NULL" ), error_locked, BIP_ERR_INVALID_PARAMETER, bipStatus );

    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hHttpStreamer->hdmiInputSettingsApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error_locked, bipStatus, bipStatus );

    if ( pHdmiInputSettings == NULL )
    {
        BIP_Streamer_GetDefaultHdmiInputSettings( &defaultSettings );
        pHdmiInputSettings = &defaultSettings;
    }

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hHttpStreamer->hdmiInputSettingsApi.pHdmiInputSettings = pHdmiInputSettings;
    hHttpStreamer->hdmiInputSettingsApi.hHdmiInput = hHdmiInput;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpStreamer;
    arbSettings.arbProcessor = processHttpStreamerState;
    arbSettings.waitIfBusy = true;;

    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_HttpStreamer_SetHdmiInputSettings" ), error, bipStatus, bipStatus );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, bipStatus ));
    return( bipStatus );

error_locked:
    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);
error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, bipStatus ));

    return( bipStatus );
} /* BIP_HttpStreamer_SetHdmiInputSettings */
#endif

BIP_Status BIP_HttpStreamer_AddTrack(
    BIP_HttpStreamerHandle hHttpStreamer,
    BIP_StreamerTrackInfo *pStreamerTrackInfo,
    BIP_StreamerTrackSettings *pTrackSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_StreamerTrackSettings defaultTrackSettings;

    bipStatus = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_HttpStreamer, hHttpStreamer);
    BIP_CHECK_GOTO((bipStatus==BIP_SUCCESS), ("BIP_CLASS_LOCK_AND_CHECK_INSTANCE failed."), error, bipStatus, bipStatus);

    BDBG_OBJECT_ASSERT( hHttpStreamer, BIP_HttpStreamer );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hHttpStreamer));

    BIP_CHECK_GOTO(( pStreamerTrackInfo ), ( "pStreamerTrackInfo can't be NULL" ), error_locked, BIP_ERR_INVALID_PARAMETER, bipStatus );
    /* Note: Rest of parameter validation happens in the BIP_Streamer class. */

    if ( pTrackSettings == NULL )
    {
        BIP_Streamer_GetDefaultTrackSettings( &defaultTrackSettings );
        pTrackSettings = &defaultTrackSettings;
    }

    hArb = hHttpStreamer->addTrackApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error_locked, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hHttpStreamer->addTrackApi.pStreamerTrackInfo = pStreamerTrackInfo;
    hHttpStreamer->addTrackApi.pTrackSettings = pTrackSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpStreamer;
    arbSettings.arbProcessor = processHttpStreamerState;
    arbSettings.waitIfBusy = true;;

    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_HttpStreamer_AddTrack" ), error, bipStatus, bipStatus );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, bipStatus ));
    return( bipStatus );

error_locked:
    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);
error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, bipStatus ));

    return( bipStatus );
}

BIP_Status BIP_HttpStreamer_SetOutputSettings(
    BIP_HttpStreamerHandle          hHttpStreamer,
    BIP_HttpStreamerProtocol        streamerProtocol,
    BIP_HttpStreamerOutputSettings  *pOutputSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_HttpStreamerOutputSettings defaultSettings;

    bipStatus = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_HttpStreamer, hHttpStreamer);
    BIP_CHECK_GOTO((bipStatus==BIP_SUCCESS), ("BIP_CLASS_LOCK_AND_CHECK_INSTANCE failed."), error, bipStatus, bipStatus);

    BDBG_OBJECT_ASSERT( hHttpStreamer, BIP_HttpStreamer );
    BDBG_ASSERT( pOutputSettings );
    BIP_SETTINGS_ASSERT(pOutputSettings, BIP_HttpStreamerOutputSettings)

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hHttpStreamer));

    BIP_CHECK_GOTO(( streamerProtocol < BIP_HttpStreamerProtocol_eMax ), ( "streamerProtocol %d is not valid", streamerProtocol ), error_locked, BIP_ERR_INVALID_PARAMETER, bipStatus );

    if ( pOutputSettings == NULL )
    {
        BIP_HttpStreamer_GetDefaultOutputSettings( &defaultSettings );
        pOutputSettings = &defaultSettings;
    }
    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hHttpStreamer->outputSettingsApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error_locked, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hHttpStreamer->outputSettingsApi.pOutputSettings = pOutputSettings;
    hHttpStreamer->outputSettingsApi.streamerProtocol = streamerProtocol;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpStreamer;
    arbSettings.arbProcessor = processHttpStreamerState;
    arbSettings.waitIfBusy = true;;

    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_HttpStreamer_SetOutputSettings" ), error, bipStatus, bipStatus );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, bipStatus ));
    return( bipStatus );

error_locked:
    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);
error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, bipStatus ));

    return( bipStatus );
} /* BIP_HttpStreamer_SetSettings */

BIP_Status BIP_HttpStreamer_AddTranscodeProfile(
    BIP_HttpStreamerHandle  hHttpStreamer,
    BIP_TranscodeProfile    *pTranscodeProfile
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    bipStatus = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_HttpStreamer, hHttpStreamer);
    BIP_CHECK_GOTO((bipStatus==BIP_SUCCESS), ("BIP_CLASS_LOCK_AND_CHECK_INSTANCE failed."), error, bipStatus, bipStatus);

    BIP_CHECK_GOTO(( pTranscodeProfile ), ( "pTranscodeProfile can't be NULL" ), error_locked, BIP_ERR_INVALID_PARAMETER, bipStatus );

    BDBG_OBJECT_ASSERT( hHttpStreamer, BIP_HttpStreamer );
    BIP_SETTINGS_ASSERT(pTranscodeProfile, BIP_TranscodeProfile);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hHttpStreamer));

    /* Note: Rest of parameter validation happens in the BIP_Streamer class. */

    hArb = hHttpStreamer->addTranscodeProfileApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error_locked, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hHttpStreamer->addTranscodeProfileApi.pTranscodeProfile = pTranscodeProfile;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpStreamer;
    arbSettings.arbProcessor = processHttpStreamerState;
    arbSettings.waitIfBusy = true;;

    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_HttpStreamer_AddTranscodeProfile" ), error, bipStatus, bipStatus );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, bipStatus ));
    return( bipStatus );

error_locked:
    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);
error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, bipStatus ));

    return( bipStatus );
}

BIP_Status BIP_HttpStreamer_SetTranscodeHandles(
    BIP_HttpStreamerHandle      hHttpStreamer,
    BIP_TranscodeNexusHandles   *pTranscodeNexusHandles
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    bipStatus = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_HttpStreamer, hHttpStreamer);
    BIP_CHECK_GOTO((bipStatus==BIP_SUCCESS), ("BIP_CLASS_LOCK_AND_CHECK_INSTANCE failed."), error, bipStatus, bipStatus);

    BIP_CHECK_GOTO(( pTranscodeNexusHandles ), ( "pTranscodeNexusHandles can't be NULL" ), error_locked, BIP_ERR_INVALID_PARAMETER, bipStatus );

    BDBG_OBJECT_ASSERT( hHttpStreamer, BIP_HttpStreamer );
    BIP_SETTINGS_ASSERT(pTranscodeNexusHandles, BIP_TranscodeNexusHandles);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hHttpStreamer));

    hArb = hHttpStreamer->setTranscodeNexusHandlesApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error_locked, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hHttpStreamer->setTranscodeNexusHandlesApi.pTranscodeNexusHandles = pTranscodeNexusHandles;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpStreamer;
    arbSettings.arbProcessor = processHttpStreamerState;
    arbSettings.waitIfBusy = true;;

    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_HttpStreamer_AddTranscodeProfile" ), error, bipStatus, bipStatus );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, bipStatus ));
    return( bipStatus );

error_locked:
    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);
error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, bipStatus ));

    return( bipStatus );
}

BIP_Status BIP_HttpStreamer_GetResponseHeader(
    BIP_HttpStreamerHandle  hHttpStreamer,          /*!< in: Streamer handle which will prepare & send the HTTP Response. */
    const char              *pHeaderName,           /*!< in: header name whose value needs to be returned. */
    const char              **pHeaderValue          /*!< in: address of a char pointer. */
    )
{
    BIP_Status brc = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    brc = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_HttpStreamer, hHttpStreamer);
    BIP_CHECK_GOTO((brc==BIP_SUCCESS), ("BIP_CLASS_LOCK_AND_CHECK_INSTANCE failed."), error, brc, brc);

    BDBG_OBJECT_ASSERT( hHttpStreamer, BIP_HttpStreamer );

    BIP_CHECK_GOTO(( pHeaderName ), ( "pHeaderName pointer can't be NULL" ), error_locked, BIP_ERR_INVALID_PARAMETER, brc );
    BIP_CHECK_GOTO(( pHeaderValue ), ( "pHeaderValue pointer can't be NULL" ), error_locked, BIP_ERR_INVALID_PARAMETER, brc );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hHttpStreamer));

    /* Serialize access to state among another thread calling the same API. */
    hArb = hHttpStreamer->getResponseHeaderApi.hArb;
    brc = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error_locked, brc, brc );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hHttpStreamer->getResponseHeaderApi.hHttpStreamer = hHttpStreamer;
    hHttpStreamer->getResponseHeaderApi.pHeaderName = pHeaderName;
    hHttpStreamer->getResponseHeaderApi.pHeaderValue = pHeaderValue;
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpStreamer;
    arbSettings.arbProcessor = processHttpStreamerState;
    arbSettings.waitIfBusy = true;;

    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);

    /* Invoke state machine via the Arb Submit API */
    brc = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_HttpStreamer_GetResponseHeader" ), error, brc, brc );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus %s  <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_StatusGetText(brc) ));
    return( brc );

error_locked:
    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);
error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus %s  <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_StatusGetText(brc) ));

    return( brc );
}

BIP_Status BIP_HttpStreamer_SetResponseHeader(
    BIP_HttpStreamerHandle  hHttpStreamer,          /*!< in: Streamer handle which will prepare & send the HTTP Response. */
    const char              *pHeaderName,           /*!< in: header name whose value needs to be returned. */
    const char              *pHeaderValue           /*!< in: address of a char pointer. */
    )
{
    BIP_Status brc = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    brc = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_HttpStreamer, hHttpStreamer);
    BIP_CHECK_GOTO((brc==BIP_SUCCESS), ("BIP_CLASS_LOCK_AND_CHECK_INSTANCE failed."), error, brc, brc);

    BDBG_OBJECT_ASSERT( hHttpStreamer, BIP_HttpStreamer );

    BIP_CHECK_GOTO(( pHeaderName ), ( "pHeaderName pointer can't be NULL" ), error_locked, BIP_ERR_INVALID_PARAMETER, brc );
    /* pHeaderValue can be NULL. */

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hHttpStreamer));

    /* Serialize access to state among another thread calling the same API. */
    hArb = hHttpStreamer->setResponseHeaderApi.hArb;
    brc = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error_locked, brc, brc );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hHttpStreamer->setResponseHeaderApi.hHttpStreamer = hHttpStreamer;
    hHttpStreamer->setResponseHeaderApi.pHeaderName = pHeaderName;
    hHttpStreamer->setResponseHeaderApi.pHeaderValue = pHeaderValue;
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpStreamer;
    arbSettings.arbProcessor = processHttpStreamerState;
    arbSettings.waitIfBusy = true;

    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);

    /* Invoke state machine via the Arb Submit API */
    brc = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_HttpStreamer_SetResponseHeader" ), error, brc, brc );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus %s  <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_StatusGetText(brc) ));
    return( brc );

error_locked:
    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);
error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus %s  <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_StatusGetText(brc) ));

    return( brc );
}

BIP_Status BIP_HttpStreamer_Start(
    BIP_HttpStreamerHandle    hHttpStreamer,
    BIP_HttpStreamerStartSettings *pSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    bipStatus = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_HttpStreamer, hHttpStreamer);
    BIP_CHECK_GOTO((bipStatus==BIP_SUCCESS), ("BIP_CLASS_LOCK_AND_CHECK_INSTANCE failed."), error, bipStatus, bipStatus);

    BDBG_OBJECT_ASSERT( hHttpStreamer, BIP_HttpStreamer );
    BIP_SETTINGS_ASSERT(pSettings, BIP_HttpStreamerStartSettings);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hHttpStreamer));

    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hHttpStreamer->startApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error_locked, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hHttpStreamer->startApi.pSettings = pSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpStreamer;
    arbSettings.arbProcessor = processHttpStreamerState;
    arbSettings.waitIfBusy = true;;

    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_HttpStreamer_Start" ), error, bipStatus, bipStatus );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p: Streamer Started: completionStatus: %s" BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_StatusGetText(bipStatus) ));

    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, bipStatus ));
    return ( bipStatus );

error_locked:
    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);
error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, bipStatus ));

    return ( bipStatus );
} /* BIP_HttpStreamer_Start */

BIP_Status BIP_HttpStreamer_Stop(
    BIP_HttpStreamerHandle    hHttpStreamer
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    bipStatus = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_HttpStreamer, hHttpStreamer);
    BIP_CHECK_GOTO((bipStatus==BIP_SUCCESS), ("BIP_CLASS_LOCK_AND_CHECK_INSTANCE failed."), error, bipStatus, bipStatus);

    BDBG_OBJECT_ASSERT( hHttpStreamer, BIP_HttpStreamer );

    BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Stopping: " BIP_HTTP_STREAMER_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_HTTP_STREAMER_PRINTF_ARG(hHttpStreamer)));
    BDBG_MSG((    BIP_MSG_PRE_FMT "Stopping: " BIP_HTTP_STREAMER_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_HTTP_STREAMER_PRINTF_ARG(hHttpStreamer)));

    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hHttpStreamer->stopApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error_locked, bipStatus, bipStatus );

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpStreamer;
    arbSettings.arbProcessor = processHttpStreamerState;
    arbSettings.waitIfBusy = true;;

    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_HttpStreamer_Stop" ), error, bipStatus, bipStatus );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, bipStatus ));
    return( bipStatus );

error_locked:
    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);
error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, bipStatus ));

    return( bipStatus );
} /* BIP_HttpStreamer_Stop */

BIP_Status BIP_HttpStreamer_ProcessRequest(
    BIP_HttpStreamerHandle                  hHttpStreamer,
    BIP_HttpSocketHandle                    hHttpSocket,
    BIP_CallbackDesc                        *pRequestProcessedCallback,
    BIP_HttpStreamerProcessRequestSettings  *pSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_HttpStreamerProcessRequestSettings defaultSettings;

    bipStatus = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_HttpStreamer, hHttpStreamer);
    BIP_CHECK_GOTO((bipStatus==BIP_SUCCESS), ("BIP_CLASS_LOCK_AND_CHECK_INSTANCE failed."), error, bipStatus, bipStatus);

    if (!pSettings)
    {
        BIP_HttpStreamer_GetDefaultProcessRequestSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    BDBG_OBJECT_ASSERT( hHttpStreamer, BIP_HttpStreamer );
    BIP_SETTINGS_ASSERT(pSettings, BIP_HttpStreamerProcessRequestSettings);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hHttpStreamer));

    BIP_CHECK_GOTO(( hHttpSocket ), ( "hHttpSocket can't be NULL" ), error_locked, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pRequestProcessedCallback->callback ), ( "requestProcessedCallback can't be NULL" ), error_locked, BIP_ERR_INVALID_PARAMETER, bipStatus );

    /* Serialize access to Settings state among another thread calling the same API. */
    hArb = hHttpStreamer->processRequestApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error_locked, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hHttpStreamer->processRequestApi.pSettings = pSettings;
    hHttpStreamer->processRequestApi.hHttpSocket = hHttpSocket;
    hHttpStreamer->processRequestApi.pRequestProcessedCallback = pRequestProcessedCallback;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpStreamer;
    arbSettings.arbProcessor = processHttpStreamerState;
    arbSettings.waitIfBusy = true;;

    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, &pSettings->apiSettings);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS || bipStatus == BIP_INF_IN_PROGRESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_HttpStreamer_ProcessRequest" ), error, bipStatus, bipStatus );

    BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Started: " BIP_HTTP_STREAMER_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_HTTP_STREAMER_PRINTF_ARG(hHttpStreamer)));

    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, bipStatus ));
    return ( bipStatus );

error_locked:
    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);
error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, bipStatus ));

    return ( bipStatus );
} /* BIP_HttpStreamer_ProcessRequest */

BIP_Status  BIP_HttpStreamer_GetStatus(
        BIP_HttpStreamerHandle  hHttpStreamer,
        BIP_HttpStreamerStatus  *pStatus
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    bipStatus = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_HttpStreamer, hHttpStreamer);
    BIP_CHECK_GOTO((bipStatus==BIP_SUCCESS), ("BIP_CLASS_LOCK_AND_CHECK_INSTANCE failed."), error, bipStatus, bipStatus);

    BDBG_OBJECT_ASSERT( hHttpStreamer, BIP_HttpStreamer );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hHttpStreamer));

    BIP_CHECK_GOTO(( pStatus ), ( "pStatus can't be NULL" ), error_locked, BIP_ERR_INVALID_PARAMETER, bipStatus );

    /* Serialize access to Settings state among another thread calling the same _GetStatus API. */
    hArb = hHttpStreamer->getStatusApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error_locked, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hHttpStreamer->getStatusApi.pStatus = pStatus;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpStreamer;
    arbSettings.arbProcessor = processHttpStreamerState;
    arbSettings.waitIfBusy = true;;

    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_HttpStreamer_GetStatus" ), error, bipStatus, bipStatus );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, bipStatus ));
    return ( bipStatus );

error_locked:
    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);
error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer, bipStatus ));

    return ( bipStatus );
}

void BIP_HttpStreamer_PrintStatus(
    BIP_HttpStreamerHandle hHttpStreamer
    )
{
    BIP_Status bipStatus;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    bipStatus = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_HttpStreamer, hHttpStreamer);
    BIP_CHECK_GOTO((bipStatus==BIP_SUCCESS), ("BIP_CLASS_LOCK_AND_CHECK_INSTANCE failed."), error, bipStatus, bipStatus);

    BDBG_ASSERT( hHttpStreamer );
    if ( !hHttpStreamer ) goto error_locked;

    BDBG_OBJECT_ASSERT( hHttpStreamer, BIP_HttpStreamer );

    /* Serialize access to Settings state among another thread calling the same _GetSettings API. */
    hArb = hHttpStreamer->printStatusApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error_locked, bipStatus, bipStatus );

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpStreamer;
    arbSettings.arbProcessor = processHttpStreamerState;
    arbSettings.waitIfBusy = true;;

    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);

    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed" ), error, bipStatus, bipStatus );
    return;

error_locked:
    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);
error:
    return;
} /* BIP_HttpStreamer_PrintStatus */
