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
#include "bip_player_impl.h"
#include "nexus_timebase.h"
#include "nexus_video_window.h"
#include "nexus_video_adj.h"

BDBG_MODULE( bip_player );

BDBG_OBJECT_ID( BIP_Player );
BIP_SETTINGS_ID(BIP_PlayerCreateSettings);
BIP_SETTINGS_ID(BIP_PlayerConnectSettings);
BIP_SETTINGS_ID(BIP_PlayerProbeMediaInfoSettings);
BIP_SETTINGS_ID(BIP_PlayerPrepareSettings);
BIP_SETTINGS_ID(BIP_PlayerStartSettings);
BIP_SETTINGS_ID(BIP_PlayerPlayAtRateSettings);
BIP_SETTINGS_ID(BIP_PlayerPauseSettings);
BIP_SETTINGS_ID(BIP_PlayerSettings);
BIP_SETTINGS_ID(BIP_PlayerOpenPidChannelSettings);
BIP_SETTINGS_ID(BIP_PlayerGetStatusFromServerSettings);

void processPlayerState( void *jObject, int value, BIP_Arb_ThreadOrigin threadOrigin );

static void destroyPlayer(
    BIP_PlayerHandle hPlayer
    )
{
    /* Note: BIP_Player_Destroy() API is first run thru the state machine to make sure if Stop/Disconnect the player first. This is the final step of the destroy API. */

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "Destoring..." BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    if (hPlayer->printStatusApi.hArb) BIP_Arb_Destroy(hPlayer->printStatusApi.hArb);
    if (hPlayer->getStatusApi.hArb) BIP_Arb_Destroy(hPlayer->getStatusApi.hArb);
    if (hPlayer->getStatusFromServerApi.hArb) BIP_Arb_Destroy(hPlayer->getStatusFromServerApi.hArb);
    if (hPlayer->setSettingsApi.hArb) BIP_Arb_Destroy(hPlayer->setSettingsApi.hArb);
    if (hPlayer->getSettingsApi.hArb) BIP_Arb_Destroy(hPlayer->getSettingsApi.hArb);
    if (hPlayer->connectApi.hArb) BIP_Arb_Destroy(hPlayer->connectApi.hArb);
    if (hPlayer->probeMediaInfoApi.hArb) BIP_Arb_Destroy(hPlayer->probeMediaInfoApi.hArb);
    if (hPlayer->getProbedStreamInfo.hArb) BIP_Arb_Destroy(hPlayer->getProbedStreamInfo.hArb);
    if (hPlayer->prepareApi.hArb) BIP_Arb_Destroy(hPlayer->prepareApi.hArb);
    if (hPlayer->openPidChannelApi.hArb) BIP_Arb_Destroy(hPlayer->openPidChannelApi.hArb);
    if (hPlayer->closePidChannelApi.hArb) BIP_Arb_Destroy(hPlayer->closePidChannelApi.hArb);
    if (hPlayer->closeAllPidChannelsApi.hArb) BIP_Arb_Destroy(hPlayer->closeAllPidChannelsApi.hArb);
    if (hPlayer->startApi.hArb) BIP_Arb_Destroy(hPlayer->startApi.hArb);
    if (hPlayer->stopApi.hArb) BIP_Arb_Destroy(hPlayer->stopApi.hArb);
    if (hPlayer->playApi.hArb) BIP_Arb_Destroy(hPlayer->playApi.hArb);
    if (hPlayer->playAtRateApi.hArb) BIP_Arb_Destroy(hPlayer->playAtRateApi.hArb);
    if (hPlayer->playByFrameApi.hArb) BIP_Arb_Destroy(hPlayer->playByFrameApi.hArb);
    if (hPlayer->pauseApi.hArb) BIP_Arb_Destroy(hPlayer->pauseApi.hArb);
    if (hPlayer->seekApi.hArb) BIP_Arb_Destroy(hPlayer->seekApi.hArb);
    if (hPlayer->disconnectApi.hArb) BIP_Arb_Destroy(hPlayer->disconnectApi.hArb);
    if (hPlayer->destroyApi.hArb) BIP_Arb_Destroy(hPlayer->destroyApi.hArb);
    if (hPlayer->hPreferredAudioLanguage) BIP_String_Destroy(hPlayer->hPreferredAudioLanguage);
    if (hPlayer->hStateMutex) B_Mutex_Destroy( hPlayer->hStateMutex );

    BIP_MSG_TRC(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "Destroyed: " BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer)));
    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "Destoryed: EXIT <--------------------" BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    BDBG_OBJECT_DESTROY( hPlayer, BIP_Player );
    B_Os_Free( hPlayer );

    return;
} /* destroyPlayer */

BIP_PlayerHandle BIP_Player_Create(
    BIP_PlayerCreateSettings *pSettings
    )
{
    BIP_Status          bipStatus;
    BIP_PlayerHandle    hPlayer;
    BIP_PlayerCreateSettings defaultCreateSettings;

    hPlayer = B_Os_Calloc( 1, sizeof( BIP_Player ));
    if (NULL == hPlayer)
    {
        BERR_TRACE( BIP_ERR_OUT_OF_SYSTEM_MEMORY );
        return(NULL);
    }

    BDBG_OBJECT_SET( hPlayer, BIP_Player );

    BIP_SETTINGS_ASSERT(pSettings, BIP_PlayerCreateSettings);

    if (pSettings == NULL)
    {
        BIP_Player_GetDefaultCreateSettings(&defaultCreateSettings);
        pSettings = &defaultCreateSettings;
    }
    hPlayer->createSettings = *pSettings;

    hPlayer->lockedTimebase = NEXUS_Timebase_eInvalid;
    hPlayer->freeRunTimebase = NEXUS_Timebase_eInvalid;

    BIP_Player_GetDefaultSettings(&hPlayer->playerSettings);
    BIP_Player_GetDefaultConnectSettings( &hPlayer->connectSettings );

    /* Create API ARBs: one per API */
    hPlayer->getSettingsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hPlayer->getSettingsApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hPlayer->setSettingsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hPlayer->setSettingsApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hPlayer->connectApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hPlayer->connectApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hPlayer->probeMediaInfoApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hPlayer->probeMediaInfoApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hPlayer->getProbedStreamInfo.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hPlayer->getProbedStreamInfo.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hPlayer->prepareApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hPlayer->prepareApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hPlayer->openPidChannelApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hPlayer->openPidChannelApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hPlayer->closeAllPidChannelsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hPlayer->closeAllPidChannelsApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hPlayer->closePidChannelApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hPlayer->closePidChannelApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hPlayer->startApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hPlayer->startApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hPlayer->stopApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hPlayer->stopApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hPlayer->playApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hPlayer->playApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hPlayer->playAtRateApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hPlayer->playAtRateApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hPlayer->playByFrameApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hPlayer->playByFrameApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hPlayer->pauseApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hPlayer->pauseApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hPlayer->seekApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hPlayer->seekApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hPlayer->getStatusApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hPlayer->getStatusApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hPlayer->getStatusFromServerApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hPlayer->getStatusFromServerApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hPlayer->printStatusApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hPlayer->printStatusApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hPlayer->disconnectApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hPlayer->disconnectApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hPlayer->destroyApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hPlayer->destroyApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hPlayer->hStateMutex = B_Mutex_Create(NULL);
    BIP_CHECK_GOTO(( hPlayer->hStateMutex ), ( "B_Mutex_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hPlayer->hPreferredAudioLanguage = BIP_String_Create( );
    BIP_CHECK_GOTO(( hPlayer->hPreferredAudioLanguage ), ( "BIP_String_Create() Failed"), error, BIP_ERR_PLAYER_MISSING_CONTENT_TYPE, bipStatus );

    hPlayer->state = BIP_PlayerState_eDisconnected;
    hPlayer->subState = BIP_PlayerSubState_eIdle;

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "Created: EXIT <--------------------" BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    BIP_MSG_TRC(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "Created: " BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer)));
    return (hPlayer);

error:
    if (hPlayer) destroyPlayer(hPlayer);
    return(NULL);
}

void BIP_Player_Destroy(
    BIP_PlayerHandle hPlayer
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));

    BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Destroying: " BIP_PLAYER_STATE_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer)));

    /* Serialize access to Settings state among another thread calling the same API. */
    hArb = hPlayer->destroyApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hPlayer;
    arbSettings.arbProcessor = processPlayerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine to notify it about object being destroyed. */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_Player_Destroy" ), error, bipStatus, bipStatus );

    /* Now release resources & destroy Player object. */
error:
    destroyPlayer( hPlayer );

} /* BIP_Player_Destroy */

static BIP_Status BIP_Player_ConnectAsync_impl(
    BIP_PlayerHandle            hPlayer,          /*!< [in] Handle of BIP_Player. */
    const char                  *pUrl,            /*!< [in] URL of media to be played */
    BIP_PlayerConnectSettings   *pSettings,       /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    BIP_CallbackDesc            *pAsyncCallback,  /*!< [in] Async completion callback: called at API completion. */
    BIP_Status                  *pAsyncStatus     /*!< [out] Completion status of async API. */
    )
{
    BIP_ArbHandle hArb;
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbSubmitSettings arbSettings;
    BIP_ApiSettings apiSettings;
    BIP_PlayerConnectSettings defaultConnectSettings;

    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );

    if (!pSettings)
    {
        BIP_Player_GetDefaultConnectSettings(&defaultConnectSettings);
        pSettings = &defaultConnectSettings;
    }

    BIP_SETTINGS_ASSERT(pSettings, BIP_PlayerConnectSettings);

    BIP_CHECK_GOTO(( pUrl ), ( "pUrl argument can't be NULL"), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    /* Serialize access to Settings state among another thread calling the same API. */
    hArb = hPlayer->connectApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hPlayer->connectApi.pUrl = pUrl;
    hPlayer->connectApi.pSettings = pSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hPlayer;
    arbSettings.arbProcessor = processPlayerState;
    arbSettings.waitIfBusy = true;
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
} /* BIP_Player_ConnectAsync_impl */

BIP_Status BIP_Player_Connect(
    BIP_PlayerHandle            hPlayer,          /*!< [in] Handle of BIP_Player. */
    const char                  *pUrl,            /*!< [in] URL of media to be played */
    BIP_PlayerConnectSettings   *pSettings        /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));

    bipStatus = BIP_Player_ConnectAsync_impl(hPlayer, pUrl, pSettings, NULL, NULL);

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "EXIT <--------------------"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    return (bipStatus);
} /* BIP_Player_Connect */

BIP_Status BIP_Player_ConnectAsync(
    BIP_PlayerHandle            hPlayer,          /*!< [in] Handle of BIP_Player. */
    const char                  *pUrl,            /*!< [in] URL of media to be played */
    BIP_PlayerConnectSettings   *pSettings,       /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    BIP_CallbackDesc            *pAsyncCallback,  /*!< [in] Async completion callback: called at API completion. */
    BIP_Status                  *pAsyncStatus     /*!< [out] Completion status of async API. */
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;

    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    /* NOTE: This top level function only checks arguments specific for the Async version of the BIP API. */
    /* The common arguments in the async & non-async version are checked in the _BIP_Player_API(). */

    if ( (pAsyncStatus == NULL && pAsyncCallback == NULL) || (pAsyncStatus == NULL && pAsyncCallback && pAsyncCallback->callback == NULL) )
    {
        BDBG_ERR(( BIP_MSG_PRE_FMT  BIP_PLAYER_STATE_PRINTF_FMT "Error: Both pAsyncStatus & pAsyncCallback can't be NULL" BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
        bipStatus = BIP_ERR_INVALID_PARAMETER;
        goto error;
    }

    bipStatus = BIP_Player_ConnectAsync_impl(hPlayer, pUrl, pSettings, pAsyncCallback, pAsyncStatus);

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "EXIT <--------------------"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
error:
    return (bipStatus);

} /* BIP_Player_ConnectAsync */

static BIP_Status BIP_Player_ProbeMediaInfoAsync_impl(
    BIP_PlayerHandle                        hPlayer,          /*!< [in] Handle of BIP_Player. */
    BIP_PlayerProbeMediaInfoSettings        *pSettings,       /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    BIP_MediaInfoHandle                     *phMediaInfo,     /*!< [out] Pointer to meta data resulting from the media probe. */
    BIP_CallbackDesc                        *pAsyncCallback,  /*!< [in] Async completion callback: called at API completion. */
    BIP_Status                              *pAsyncStatus     /*!< [out] Completion status of async API. */
    )
{
    BIP_ArbHandle hArb;
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbSubmitSettings arbSettings;
    BIP_ApiSettings apiSettings;
    BIP_PlayerProbeMediaInfoSettings defaultProbeMediaInfoSettings;

    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );

    if (!pSettings)
    {
        BIP_Player_GetDefaultProbeMediaInfoSettings(&defaultProbeMediaInfoSettings);
        pSettings = &defaultProbeMediaInfoSettings;
    }

    BIP_SETTINGS_ASSERT(pSettings, BIP_PlayerProbeMediaInfoSettings);

    BIP_CHECK_GOTO(( phMediaInfo ), ( "phMediaInfo argument can't be NULL"), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    /* Serialize access to Settings state among another thread calling the same API. */
    hArb = hPlayer->probeMediaInfoApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hPlayer->probeMediaInfoApi.phMediaInfo = phMediaInfo;
    hPlayer->probeMediaInfoApi.pSettings = pSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hPlayer;
    arbSettings.arbProcessor = processPlayerState;
    arbSettings.waitIfBusy = true;
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
} /* BIP_Player_ProbeMediaInfoAsync_impl */

BIP_Status BIP_Player_ProbeMediaInfo(
    BIP_PlayerHandle                        hPlayer,          /*!< [in] Handle of BIP_Player. */
    BIP_PlayerProbeMediaInfoSettings        *pSettings,       /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    BIP_MediaInfoHandle                     *phMediaInfo      /*!< [out] Pointer to meta data resulting from the media probe. */
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));

    bipStatus = BIP_Player_ProbeMediaInfoAsync_impl(hPlayer, pSettings, phMediaInfo, NULL, NULL);

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "EXIT <--------------------"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    return (bipStatus);
} /* BIP_Player_ProbeMediaInfo */

BIP_Status BIP_Player_ProbeMediaInfoAsync(
    BIP_PlayerHandle                        hPlayer,          /*!< [in] Handle of BIP_Player. */
    BIP_PlayerProbeMediaInfoSettings        *pSettings,       /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    BIP_MediaInfoHandle                     *phMediaInfo,     /*!< [out] Pointer to meta data resulting from the media probe. */
    BIP_CallbackDesc                        *pAsyncCallback,  /*!< [in] Async completion callback: called at API completion. */
    BIP_Status                              *pAsyncStatus     /*!< [out] Completion status of async API. */
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;

    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    /* NOTE: This top level function only checks arguments specific for the Async version of the BIP API. */
    /* The common arguments in the async & non-async version are checked in the _BIP_Player_API(). */

    if ( (pAsyncStatus == NULL && pAsyncCallback == NULL) || (pAsyncStatus == NULL && pAsyncCallback && pAsyncCallback->callback == NULL) )
    {
        BDBG_ERR(( BIP_MSG_PRE_FMT  BIP_PLAYER_STATE_PRINTF_FMT "Error: Both pAsyncStatus & pAsyncCallback can't be NULL"
                   BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
        bipStatus = BIP_ERR_INVALID_PARAMETER;
        goto error;
    }

    bipStatus = BIP_Player_ProbeMediaInfoAsync_impl(hPlayer, pSettings, phMediaInfo, pAsyncCallback, pAsyncStatus);

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "EXIT <--------------------"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
error:
    return (bipStatus);

} /* BIP_Player_ProbeMediaInfoAsync */

BIP_Status BIP_Player_GetProbedStreamInfo(
    BIP_PlayerHandle        hPlayer,        /*!< [in] Handle of BIP_Player. */
    BIP_PlayerStreamInfo    *pStreamInfo    /*!< [out] Pointer to the BIP_PlayerStreamInfo to be filled. */
    )
{
    BIP_ArbHandle hArb;
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );
    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));

    BIP_CHECK_GOTO(( pStreamInfo ), ( "pStreamInfo argument can't be NULL"), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    /* Serialize access to Settings state among another thread calling the same API. */
    hArb = hPlayer->getProbedStreamInfo.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hPlayer->getProbedStreamInfo.pStreamInfo = pStreamInfo;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hPlayer;
    arbSettings.arbProcessor = processPlayerState;
    arbSettings.waitIfBusy = true;

    /* Invoke state machine. */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS || bipStatus == BIP_INF_IN_PROGRESS), ( "BIP_Arb_SubmitRequest() Failed" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "EXIT <--------------------"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    return (bipStatus);
} /* BIP_Player_GetProbedStreamInfo */

void  BIP_Player_GetSettings(
    BIP_PlayerHandle    hPlayer,    /*!< [in] Handle of BIP_Player. */
    BIP_PlayerSettings  *pSettings  /*!< [out] Pointer to caller's struct to be filled with the current BIP_PlayerSettings. */
    )
{
    BIP_ArbHandle hArb;
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    BIP_CHECK_GOTO(( pSettings ), ( "pSettings argument can't be NULL"), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    /* Serialize access to Settings state among another thread calling the same API. */
    hArb = hPlayer->getSettingsApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hPlayer->getSettingsApi.pSettings = pSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hPlayer;
    arbSettings.arbProcessor = processPlayerState;
    arbSettings.waitIfBusy = true;

    /* Invoke state machine */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS || bipStatus == BIP_INF_IN_PROGRESS), ( "BIP_Arb_SubmitRequest() Failed" ), error, bipStatus, bipStatus );

    BIP_SETTINGS_SET(pSettings, BIP_PlayerSettings);

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "EXIT <--------------------"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    return;
}

BIP_Status BIP_Player_SetSettings(
    BIP_PlayerHandle            hPlayer,     /*!< [in] Handle of BIP_Player. */
    const BIP_PlayerSettings    *pSettings   /*!< [in] Pointer to BIP_PlayerSettings structure. */
    )
{
    BIP_ArbHandle hArb;
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    BIP_CHECK_GOTO(( pSettings ), ( "pSettings argument can't be NULL"), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_SETTINGS_ASSERT(pSettings, BIP_PlayerSettings);


    /* Serialize access to Settings state among another thread calling the same API. */
    hArb = hPlayer->setSettingsApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hPlayer->setSettingsApi.pSettings = pSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hPlayer;
    arbSettings.arbProcessor = processPlayerState;
    arbSettings.waitIfBusy = true;

    /* Invoke state machine. */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS || bipStatus == BIP_INF_IN_PROGRESS), ( "BIP_Arb_SubmitRequest() Failed" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "EXIT <--------------------"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    return (bipStatus);
} /* BIP_Player_SetSettings */

BIP_Status BIP_Player_PrepareAsync_impl(
    BIP_PlayerHandle                         hPlayer,           /*!< [in]  Handle of BIP_Player. */
    const BIP_PlayerPrepareSettings          *pPrepareSettings, /*!< [in]  Optional. Prepare related settings. */
    const BIP_PlayerSettings                 *pPlayerSettings,  /*!< [in]  Optional. Initial player settings to use */
    const BIP_PlayerProbeMediaInfoSettings   *pProbeSettings,   /*!< [in]  Optional. How to probe */
    const BIP_PlayerStreamInfo               *pStreamInfo,      /*!< [in]  Optional. Use this stream info. */
    BIP_PlayerPrepareStatus                  *pPrepareStatus,   /*!< [out] Optional. Status returned by BIP_Player_Prepare(). */
    BIP_CallbackDesc                         *pAsyncCallback,   /*!< [in]  Async completion callback: called at API completion. */
    BIP_Status                               *pAsyncStatus      /*!< [out] Completion status of async API. */
    )
{
    BIP_ArbHandle                       hArb;
    BIP_Status                          bipStatus = BIP_SUCCESS;
    BIP_ArbSubmitSettings               arbSettings;
    BIP_ApiSettings                     apiSettings;
    BIP_PlayerProbeMediaInfoSettings    defaultProbeMediaInfoSettings;
    BIP_PlayerPrepareSettings           defaultPrepareSettings;
    BIP_PlayerSettings                  defaultPlayerSettings;

    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );

    /* Check if caller is providing correct state if it is using player for decoding purposes. */
    if (pPlayerSettings)
    {
        bool avDecodeEnabled = false;
        if ( pPlayerSettings->videoTrackSettings.pidTypeSettings.video.decoder || pPlayerSettings->audioTrackSettings.pidTypeSettings.audio.primary )
        {
            avDecodeEnabled = true;
            BIP_CHECK_GOTO(( pPlayerSettings->playbackSettings.stcChannel ),    ( "hPlayer=%p: stcChannel can't be NULL for non-simple AV decoding case!", (void *)hPlayer ), error, BIP_ERR_INVALID_PARAMETER , bipStatus);
        }
        if ( pPlayerSettings->videoTrackSettings.pidTypeSettings.video.simpleDecoder || pPlayerSettings->audioTrackSettings.pidTypeSettings.audio.simpleDecoder )
        {
            avDecodeEnabled = true;
            BIP_CHECK_GOTO(( pPlayerSettings->playbackSettings.simpleStcChannel ),    ( "hPlayer=%p: simpleStcChannel can't be NULL for simple(NxClient) AV decoding case!", (void *)hPlayer ), error, BIP_ERR_INVALID_PARAMETER , bipStatus);
        }

        if (pPlayerSettings->videoTrackSettings.pidTypeSettings.video.decoder)
        {
            /* App is using non-simple decoder handle, make sure it also gives us additional required handles. */
            BIP_CHECK_GOTO(((pPlayerSettings->hDisplay && pPlayerSettings->hWindow)),
                    ( "App must provide hDisplay=%p hWindow=%p handles for non-simple Nexus mode usage!", (void *)pPlayerSettings->hDisplay, (void *)pPlayerSettings->hWindow), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
        }
        if (avDecodeEnabled || pPlayerSettings->videoTrackId != UINT_MAX || pPlayerSettings->audioTrackId != UINT_MAX || pPlayerSettings->pPreferredAudioLanguage )
        {
            BIP_CHECK_GOTO(( pPrepareStatus ), ( "hPlayer=%p: pPlayerStatus can't be NULL: AV PidChannels are returned via this structure!", (void *)hPlayer ), error, BIP_ERR_INVALID_PARAMETER , bipStatus);
        }
    }

    if (pStreamInfo)
    {
        BIP_CHECK_GOTO(( pStreamInfo->transportType != NEXUS_TransportType_eUnknown && pStreamInfo->transportType < NEXUS_TransportType_eMax), ( "hPlayer=%p: invalid StreamInfo: transportType=%d!", (void *)hPlayer, pStreamInfo->transportType ), error, BIP_ERR_INVALID_PARAMETER , bipStatus);

        BIP_CHECK_GOTO(( pStreamInfo->containerType >= BIP_PlayerContainerType_eNexusTransportType && pStreamInfo->containerType < BIP_PlayerContainerType_eMax),
                ( "hPlayer=%p: invalid StreamInfo: containerType=%d!", (void *)hPlayer, pStreamInfo->containerType ), error, BIP_ERR_INVALID_PARAMETER , bipStatus);
    }

    if ( pPrepareSettings && pPrepareSettings->pauseTimeoutAction == NEXUS_PlaybackLoopMode_ePlay && pPrepareSettings->timeshiftBufferMaxDurationInMs == 0)
    {
        BIP_CHECK_GOTO(( 0 ), ( "hPlayer=%p: timeshiftBufferMaxDurationInMs=%d can't be <= 0 for pauseAction=NEXUS_PlaybackLoopMode_ePlay",
                    (void *)hPlayer, pPrepareSettings->timeshiftBufferMaxDurationInMs), error, BIP_ERR_INVALID_PARAMETER , bipStatus);
    }
    if (!pProbeSettings)
    {
        BIP_Player_GetDefaultProbeMediaInfoSettings(&defaultProbeMediaInfoSettings);
        pProbeSettings = &defaultProbeMediaInfoSettings;
    }
    if (!pPrepareSettings)
    {
        BIP_Player_GetDefaultPrepareSettings(&defaultPrepareSettings);
        pPrepareSettings = &defaultPrepareSettings;
    }
    if (!pPlayerSettings)
    {
        defaultPlayerSettings = hPlayer->playerSettings; /* playerSettings are either the last settings from SetSettings or default values initialied during BIP_Player_Create. */
        pPlayerSettings = &defaultPlayerSettings;
    }

    BIP_SETTINGS_ASSERT(pPrepareSettings, BIP_PlayerPrepareSettings);
    BIP_SETTINGS_ASSERT(pPlayerSettings,  BIP_PlayerSettings);
    BIP_SETTINGS_ASSERT(pProbeSettings,   BIP_PlayerProbeMediaInfoSettings);

    /* Serialize access to Settings state among another thread calling the same API. */
    hArb = hPlayer->prepareApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hPlayer->prepareApi.pPrepareSettings = pPrepareSettings;
    hPlayer->prepareApi.pProbeSettings = pProbeSettings;
    hPlayer->prepareApi.pPlayerSettings = pPlayerSettings;
    hPlayer->prepareApi.pStreamInfo = pStreamInfo;
    hPlayer->prepareApi.pPrepareStatus = pPrepareStatus;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hPlayer;
    arbSettings.arbProcessor = processPlayerState;
    arbSettings.waitIfBusy = true;
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
} /* BIP_Player_Prepare_impl */


BIP_Status BIP_Player_Prepare(
    BIP_PlayerHandle                         hPlayer,           /*!< [in]  Handle of BIP_Player. */
    const BIP_PlayerPrepareSettings          *pPrepareSettings, /*!< [in]  Required. How to prepare() */
    const BIP_PlayerSettings                 *pPlayerSettings,  /*!< [in]  Required. Current player settings to use */
    const BIP_PlayerProbeMediaInfoSettings   *pProbeSettings,   /*!< [in]  Optional. How to probe */
    const BIP_PlayerStreamInfo               *pStreamInfo,      /*!< [in]  Optional. Use this stream info. */
    BIP_PlayerPrepareStatus                  *pPrepareStatus    /*!< [out] Required. Important values returned by BIP_Player_Prepare() */
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));

    bipStatus = BIP_Player_PrepareAsync_impl(hPlayer, pPrepareSettings, pPlayerSettings, pProbeSettings, pStreamInfo, pPrepareStatus, NULL, NULL);

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "EXIT <--------------------"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    return (bipStatus);
} /* BIP_Player_Prepare */

/**
Summary:
Async version of BIP_Player_Prepare().
API to Prepare Player for Playing.

See Also:
BIP_Player_Prepare
BIP_Player_GetDefaultPrepareSettings
BIP_PlayerPrepareSettings
BIP_PlayerPrepareStatus
**/
BIP_Status BIP_Player_PrepareAsync(
    BIP_PlayerHandle                         hPlayer,           /*!< [in]  Handle of BIP_Player. */
    const BIP_PlayerPrepareSettings          *pPrepareSettings, /*!< [in]  Required. How to prepare() */
    const BIP_PlayerSettings                 *pPlayerSettings,  /*!< [in]  Required. Current player settings to use */
    const BIP_PlayerProbeMediaInfoSettings   *pProbeSettings,   /*!< [in]  Optional. How to probe */
    const BIP_PlayerStreamInfo               *pStreamInfo,      /*!< [in]  Optional. Use this stream info. */
    BIP_PlayerPrepareStatus                  *pPrepareStatus,   /*!< [out] Required. Important values returned by BIP_Player_Prepare() */
    BIP_CallbackDesc                         *pAsyncCallback,   /*!< [in]  Async completion callback: called at API completion. */
    BIP_Status                               *pAsyncStatus      /*!< [out] Completion status of async API. */
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;

    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    /* NOTE: This top level function only checks arguments specific for the Async version of the BIP API. */
    /* The common arguments in the async & non-async version are checked in the _BIP_Player_API(). */

    if ( (pAsyncStatus == NULL && pAsyncCallback == NULL) || (pAsyncStatus == NULL && pAsyncCallback && pAsyncCallback->callback == NULL) )
    {
        BDBG_ERR(( BIP_MSG_PRE_FMT  BIP_PLAYER_STATE_PRINTF_FMT "Error: Both pAsyncStatus & pAsyncCallback can't be NULL"
                   BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
        bipStatus = BIP_ERR_INVALID_PARAMETER;
        goto error;
    }

    bipStatus = BIP_Player_PrepareAsync_impl(hPlayer, pPrepareSettings, pPlayerSettings, pProbeSettings, pStreamInfo, pPrepareStatus, pAsyncCallback, pAsyncStatus);

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "EXIT <--------------------"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
error:
    return (bipStatus);

} /* BIP_Player_PrepareAsync */

BIP_Status BIP_Player_ClosePidChannel(
    BIP_PlayerHandle                        hPlayer,        /*!< [in] Handle of BIP_Player. */
    NEXUS_PidChannelHandle                  hPidChannel     /*!< [in] pid channel handle to be close. */
    )
{
    BIP_ArbHandle hArb;
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    BIP_CHECK_GOTO(( hPidChannel ), ( "hPidChannel argument can't be NULL"), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    /* Serialize access to Settings state among another thread calling the same API. */
    hArb = hPlayer->closePidChannelApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hPlayer->closePidChannelApi.hPidChannel = hPidChannel;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hPlayer;
    arbSettings.arbProcessor = processPlayerState;
    arbSettings.waitIfBusy = true;

    /* Invoke state machine. */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS || bipStatus == BIP_INF_IN_PROGRESS), ( "BIP_Arb_SubmitRequest() Failed" ), error, bipStatus, bipStatus );

error:

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "EXIT <--------------------"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    return (bipStatus);
} /* BIP_Player_ClosePidChannel */

BIP_Status BIP_Player_CloseAllPidChannels(
    BIP_PlayerHandle hPlayer        /*!< [in] Handle of BIP_Player. */
    )
{
    BIP_ArbHandle hArb;
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );

    /* Serialize access to Settings state among another thread calling the same API. */
    hArb = hPlayer->closeAllPidChannelsApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hPlayer;
    arbSettings.arbProcessor = processPlayerState;
    arbSettings.waitIfBusy = true;

    /* Invoke state machine. */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS || bipStatus == BIP_INF_IN_PROGRESS), ( "BIP_Arb_SubmitRequest() Failed" ), error, bipStatus, bipStatus );

error:

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "EXIT <--------------------"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    return (bipStatus);
} /* BIP_Player_CloseAllPidChannel */

BIP_Status BIP_Player_OpenPidChannel(
    BIP_PlayerHandle                        hPlayer,        /*!< [in] Handle of BIP_Player. */
    unsigned                                trackId,        /*!< Unique Track ID (PID for MPEG2-TS, track_ID for ISOBMFF). */
    const BIP_PlayerOpenPidChannelSettings  *pSettings,     /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    NEXUS_PidChannelHandle                  *phPidChannel   /*!< [out] pointer to the pid channel handle that is opened for the given trackId. */
    )
{
    BIP_ArbHandle hArb;
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );

    BIP_CHECK_GOTO(( pSettings ), ( "pSettings argument can't be NULL"), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( trackId != UINT_MAX ), ( "trackId argument can't set to %u", UINT_MAX), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( phPidChannel ), ( "phPidChannel argument can't be NULL"), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    BIP_SETTINGS_ASSERT(pSettings, BIP_PlayerOpenPidChannelSettings);

    /* Serialize access to Settings state among another thread calling the same API. */
    hArb = hPlayer->openPidChannelApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hPlayer->openPidChannelApi.trackId = trackId;
    hPlayer->openPidChannelApi.pSettings = pSettings;
    hPlayer->openPidChannelApi.phPidChannel = phPidChannel;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hPlayer;
    arbSettings.arbProcessor = processPlayerState;
    arbSettings.waitIfBusy = true;

    /* Invoke state machine. */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS || bipStatus == BIP_INF_IN_PROGRESS), ( "BIP_Arb_SubmitRequest() Failed" ), error, bipStatus, bipStatus );

error:

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "EXIT <--------------------"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    return (bipStatus);
} /* BIP_Player_OpenPidChannel */

BIP_Status  BIP_Player_StartAsync_impl(
    BIP_PlayerHandle                hPlayer,         /*!< [in] Handle of BIP_Player. */
    const BIP_PlayerStartSettings   *pSettings,      /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    BIP_CallbackDesc                *pAsyncCallback, /*!< [in] Async completion callback: called at API completion. */
    BIP_Status                      *pAsyncStatus    /*!< [out] Completion status of async API. */
    )
{
    BIP_ArbHandle hArb;
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbSubmitSettings arbSettings;
    BIP_ApiSettings apiSettings;
    BIP_PlayerStartSettings defaultStartSettings;

    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );

    if (!pSettings)
    {
        BIP_Player_GetDefaultStartSettings(&defaultStartSettings);
        pSettings = &defaultStartSettings;
    }
    BIP_SETTINGS_ASSERT(pSettings, BIP_PlayerStartSettings);

    /* Serialize access to Settings state among another thread calling the same API. */
    hArb = hPlayer->startApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hPlayer->startApi.pSettings = pSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hPlayer;
    arbSettings.arbProcessor = processPlayerState;
    arbSettings.waitIfBusy = true;
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
} /* BIP_Player_StartAsync_impl */

BIP_Status  BIP_Player_Start(
    BIP_PlayerHandle                hPlayer,        /*!< [in] Handle of BIP_Player. */
    const BIP_PlayerStartSettings   *pSettings      /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));

    bipStatus = BIP_Player_StartAsync_impl(hPlayer, pSettings, NULL, NULL);

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "EXIT <--------------------"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    return (bipStatus);
} /* BIP_Player_Start */

BIP_Status  BIP_Player_StartAsync(
    BIP_PlayerHandle                hPlayer,         /*!< [in] Handle of BIP_Player. */
    const BIP_PlayerStartSettings   *pSettings,      /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    BIP_CallbackDesc                *pAsyncCallback, /*!< [in] Async completion callback: called at API completion. */
    BIP_Status                      *pAsyncStatus    /*!< [out] Completion status of async API. */
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;

    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    /* NOTE: This top level function only checks arguments specific for the Async version of the BIP API. */
    /* The common arguments in the async & non-async version are checked in the _BIP_Player_API(). */

    if ( (pAsyncStatus == NULL && pAsyncCallback == NULL) || (pAsyncStatus == NULL && pAsyncCallback && pAsyncCallback->callback == NULL) )
    {
        BDBG_ERR(( BIP_MSG_PRE_FMT  BIP_PLAYER_STATE_PRINTF_FMT "Error: Both pAsyncStatus & pAsyncCallback can't be NULL"
                   BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
        bipStatus = BIP_ERR_INVALID_PARAMETER;
        goto error;
    }

    bipStatus = BIP_Player_StartAsync_impl(hPlayer, pSettings, pAsyncCallback, pAsyncStatus);

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "EXIT <--------------------"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    return (bipStatus);

} /* BIP_Player_StartAsync */

void  BIP_Player_Stop(
    BIP_PlayerHandle hPlayer        /*!< [in] Handle of BIP_Player. */
    )
{
    BIP_ArbHandle hArb;
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    /* Serialize access to Settings state among another thread calling the same API. */
    hArb = hPlayer->stopApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hPlayer;
    arbSettings.arbProcessor = processPlayerState;
    arbSettings.waitIfBusy = true;

    /* Invoke state machine. */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS ), ( "BIP_Arb_SubmitRequest() Failed" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "EXIT <--------------------"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    return;
} /* BIP_Player_Stop */

void BIP_Player_Disconnect(
    BIP_PlayerHandle hPlayer
    )
{
    BIP_ArbHandle hArb;
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));

    /* Serialize access to Settings state among another thread calling the same API. */
    hArb = hPlayer->disconnectApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hPlayer;
    arbSettings.arbProcessor = processPlayerState;
    arbSettings.waitIfBusy = true;

    /* Invoke state machine. */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "EXIT <--------------------"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
}

BIP_Status  BIP_Player_GetStatus(
    BIP_PlayerHandle    hPlayer,        /*!< [in] Handle of BIP_Player. */
    BIP_PlayerStatus    *pStatus        /*!< [out] Pointer to caller's struct to receive the current status. */
    )
{
    BIP_ArbHandle hArb;
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );

    BIP_MSG_TRC(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));

    /* Serialize access to Settings state among another thread calling the same API. */
    hArb = hPlayer->getStatusApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    hPlayer->getStatusApi.pStatus = pStatus;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hPlayer;
    arbSettings.arbProcessor = processPlayerState;
    arbSettings.waitIfBusy = true;

    /* Invoke state machine. */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed" ), error, bipStatus, bipStatus );

error:
    BIP_MSG_TRC(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "EXIT <--------------------"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    return (bipStatus);
}

void  BIP_Player_PrintStatus(
    BIP_PlayerHandle    hPlayer        /*!< [in] Handle of BIP_Player. */
    )
{
    BIP_ArbHandle hArb;
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );

    BIP_MSG_TRC(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));

    /* Serialize access to Settings state among another thread calling the same API. */
    hArb = hPlayer->printStatusApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hPlayer;
    arbSettings.arbProcessor = processPlayerState;
    arbSettings.waitIfBusy = true;

    /* Invoke state machine. */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed" ), error, bipStatus, bipStatus );

error:
    BIP_MSG_TRC(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "EXIT <--------------------"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
}

BIP_Status BIP_Player_Pause(
    BIP_PlayerHandle    hPlayer,        /*!< [in] Handle of BIP_Player. */
    BIP_PlayerPauseSettings *pSettings
    )
{
    BIP_ArbHandle hArb;
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbSubmitSettings arbSettings;
    BIP_PlayerPauseSettings defaultPauseSettings;

    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));

    if (!pSettings)
    {
        BIP_Player_GetDefaultPauseSettings(&defaultPauseSettings);
        pSettings = &defaultPauseSettings;
    }
    BIP_SETTINGS_ASSERT(pSettings, BIP_PlayerPauseSettings);

    /* Serialize access to Settings state among another thread calling the same API. */
    hArb = hPlayer->pauseApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    hPlayer->pauseApi.pSettings = pSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hPlayer;
    arbSettings.arbProcessor = processPlayerState;
    arbSettings.waitIfBusy = true;

    /* Invoke state machine. */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "EXIT <--------------------"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    return (bipStatus);
}

BIP_Status  BIP_Player_PlayAsync_impl(
    BIP_PlayerHandle                hPlayer,         /*!< [in] Handle of BIP_Player. */
    BIP_CallbackDesc                *pAsyncCallback, /*!< [in] Async completion callback: called at API completion. */
    BIP_Status                      *pAsyncStatus    /*!< [out] Completion status of async API. */
    )
{
    BIP_ArbHandle hArb;
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbSubmitSettings arbSettings;
    BIP_ApiSettings apiSettings;

    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );

    /* Serialize access to Settings state among another thread calling the same API. */
    hArb = hPlayer->playApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hPlayer;
    arbSettings.arbProcessor = processPlayerState;
    arbSettings.waitIfBusy = true;
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
} /* BIP_Player_PlayAsync_impl */

BIP_Status  BIP_Player_Play(
    BIP_PlayerHandle                hPlayer         /*!< [in] Handle of BIP_Player. */
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));

    bipStatus = BIP_Player_PlayAsync_impl(hPlayer, NULL, NULL);

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "EXIT <--------------------"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    return (bipStatus);
} /* BIP_Player_Play */

BIP_Status  BIP_Player_PlayAsync(
    BIP_PlayerHandle                hPlayer,         /*!< [in] Handle of BIP_Player. */
    BIP_CallbackDesc                *pAsyncCallback, /*!< [in] Async completion callback: called at API completion. */
    BIP_Status                      *pAsyncStatus    /*!< [out] Completion status of async API. */
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;

    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    /* NOTE: This top level function only checks arguments specific for the Async version of the BIP API. */
    /* The common arguments in the async & non-async version are checked in the _BIP_Player_API(). */

    if ( (pAsyncStatus == NULL && pAsyncCallback == NULL) || (pAsyncStatus == NULL && pAsyncCallback && pAsyncCallback->callback == NULL) )
    {
        BDBG_ERR(( BIP_MSG_PRE_FMT  BIP_PLAYER_STATE_PRINTF_FMT "Error: Both pAsyncStatus & pAsyncCallback can't be NULL"
                   BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
        bipStatus = BIP_ERR_INVALID_PARAMETER;
        goto error;
    }

    bipStatus = BIP_Player_PlayAsync_impl(hPlayer, pAsyncCallback, pAsyncStatus);

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "EXIT <--------------------"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    return (bipStatus);

} /* BIP_Player_PlayAsync */

BIP_Status  BIP_Player_SeekAsync_impl(
    BIP_PlayerHandle                hPlayer,            /*!< [in] Handle of BIP_Player. */
    unsigned                        seekPositionInMs,   /*!< [in] Time position in milliseconds to Seek to. */
    BIP_CallbackDesc                *pAsyncCallback,    /*!< [in] Async completion callback: called at API completion. */
    BIP_Status                      *pAsyncStatus       /*!< [out] Completion status of async API. */
    )
{
    BIP_ArbHandle hArb;
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbSubmitSettings arbSettings;
    BIP_ApiSettings apiSettings;

    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );

    /* Serialize access to Settings state among another thread calling the same API. */
    hArb = hPlayer->seekApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hPlayer->seekApi.seekPositionInMs = seekPositionInMs;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hPlayer;
    arbSettings.arbProcessor = processPlayerState;
    arbSettings.waitIfBusy = true;
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
} /* BIP_Player_SeekAsync_impl */

BIP_Status  BIP_Player_Seek(
    BIP_PlayerHandle                hPlayer,            /*!< [in] Handle of BIP_Player. */
    unsigned                        seekPositionInMs    /*!< [in] Time position in milliseconds to Seek to. */
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));

    bipStatus = BIP_Player_SeekAsync_impl(hPlayer, seekPositionInMs, NULL, NULL);

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "EXIT <--------------------"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    return (bipStatus);
} /* BIP_Player_Seek */

BIP_Status  BIP_Player_SeekAsync(
    BIP_PlayerHandle                hPlayer,            /*!< [in] Handle of BIP_Player. */
    unsigned                        seekPositionInMs,   /*!< [in] Time position in milliseconds to Seek to. */
    BIP_CallbackDesc                *pAsyncCallback,    /*!< [in] Async completion callback: called at API completion. */
    BIP_Status                      *pAsyncStatus       /*!< [out] Completion status of async API. */
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;

    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    /* NOTE: This top level function only checks arguments specific for the Async version of the BIP API. */
    /* The common arguments in the async & non-async version are checked in the _BIP_Player_API(). */

    if ( (pAsyncStatus == NULL && pAsyncCallback == NULL) || (pAsyncStatus == NULL && pAsyncCallback && pAsyncCallback->callback == NULL) )
    {
        BDBG_ERR(( BIP_MSG_PRE_FMT  BIP_PLAYER_STATE_PRINTF_FMT "Error: Both pAsyncStatus & pAsyncCallback can't be NULL"
                   BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
        bipStatus = BIP_ERR_INVALID_PARAMETER;
        goto error;
    }

    bipStatus = BIP_Player_SeekAsync_impl(hPlayer, seekPositionInMs, pAsyncCallback, pAsyncStatus);

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "EXIT <--------------------"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    return (bipStatus);

} /* BIP_Player_SeekAsync */

BIP_Status  BIP_Player_PlayAtRateAsStringAsync_impl(
    BIP_PlayerHandle                    hPlayer,         /*!< [in] Handle of BIP_Player. */
    const char                          *pRate,          /*!< [in] Playback rate as a multiple of normal. */
    const BIP_PlayerPlayAtRateSettings  *pSettings,      /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    BIP_CallbackDesc                    *pAsyncCallback, /*!< [in] Async completion callback: called at API completion. */
    BIP_Status                          *pAsyncStatus    /*!< [out] Completion status of async API. */
    )
{
    BIP_ArbHandle hArb;
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbSubmitSettings arbSettings;
    BIP_ApiSettings apiSettings;
    BIP_PlayerPlayAtRateSettings defaultPlayAtRateSettings;

    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );

    BIP_CHECK_GOTO(( pRate ),   ( "hPlayer=%p: pRate can't be NULL: must provide Rate String for trickmode operation!", (void *)hPlayer ), error, BIP_ERR_INVALID_PARAMETER , bipStatus);
    if (!pSettings)
    {
        BIP_Player_GetDefaultPlayAtRateSettings(&defaultPlayAtRateSettings);
        pSettings = &defaultPlayAtRateSettings;
    }
    BIP_SETTINGS_ASSERT(pSettings, BIP_PlayerPlayAtRateSettings);

    /* Serialize access to Settings state among another thread calling the same API. */
    hArb = hPlayer->playAtRateApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hPlayer->playAtRateApi.pSettings = pSettings;
    hPlayer->playAtRateApi.pRate = pRate;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hPlayer;
    arbSettings.arbProcessor = processPlayerState;
    arbSettings.waitIfBusy = true;
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
} /* BIP_Player_PlayAtRateAsStringAsync_impl */

BIP_Status  BIP_Player_PlayAtRateAsString(
    BIP_PlayerHandle                    hPlayer,         /*!< [in] Handle of BIP_Player. */
    const char                          *pRate,          /*!< [in] Playback rate as a multiple of normal. */
    const BIP_PlayerPlayAtRateSettings  *pSettings       /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));

    bipStatus = BIP_Player_PlayAtRateAsStringAsync_impl(hPlayer, pRate, pSettings, NULL, NULL);

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "EXIT <--------------------"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    return (bipStatus);
} /* BIP_Player_PlayAtRateAsString */

BIP_Status  BIP_Player_PlayAtRateAsStringAsync(
    BIP_PlayerHandle                    hPlayer,         /*!< [in] Handle of BIP_Player. */
    const char                          *pRate,          /*!< [in] Playback rate as a multiple of normal. */
    const BIP_PlayerPlayAtRateSettings  *pSettings,      /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    BIP_CallbackDesc                    *pAsyncCallback, /*!< [in] Async completion callback: called at API completion. */
    BIP_Status                          *pAsyncStatus    /*!< [out] Completion status of async API. */
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;

    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    /* NOTE: This top level function only checks arguments specific for the Async version of the BIP API. */
    /* The common arguments in the async & non-async version are checked in the _BIP_Player_API(). */

    if ( (pAsyncStatus == NULL && pAsyncCallback == NULL) || (pAsyncStatus == NULL && pAsyncCallback && pAsyncCallback->callback == NULL) )
    {
        BDBG_ERR(( BIP_MSG_PRE_FMT  BIP_PLAYER_STATE_PRINTF_FMT "Error: Both pAsyncStatus & pAsyncCallback can't be NULL"
                   BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
        bipStatus = BIP_ERR_INVALID_PARAMETER;
        goto error;
    }

    bipStatus = BIP_Player_PlayAtRateAsStringAsync_impl(hPlayer, pRate, pSettings, pAsyncCallback, pAsyncStatus);

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "EXIT <--------------------"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    return (bipStatus);

} /* BIP_Player_PlayAtRateAsync */

BIP_Status  BIP_Player_PlayAtRateAsync_impl(
    BIP_PlayerHandle                    hPlayer,         /*!< [in] Handle of BIP_Player. */
    int                                 rate,            /*!< [in] Playback rate defined in NEXUS_NORMAL_PLAY_SPEED units. See details in nexus_playback.h */
    const BIP_PlayerPlayAtRateSettings  *pSettings,      /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    BIP_CallbackDesc                    *pAsyncCallback, /*!< [in] Async completion callback: called at API completion. */
    BIP_Status                          *pAsyncStatus    /*!< [out] Completion status of async API. */
    )
{
    BIP_ArbHandle hArb;
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbSubmitSettings arbSettings;
    BIP_ApiSettings apiSettings;
    BIP_PlayerPlayAtRateSettings defaultPlayAtRateSettings;

    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );

    if (!pSettings)
    {
        BIP_Player_GetDefaultPlayAtRateSettings(&defaultPlayAtRateSettings);
        pSettings = &defaultPlayAtRateSettings;
    }
    BIP_SETTINGS_ASSERT(pSettings, BIP_PlayerPlayAtRateSettings);

    /* Serialize access to Settings state among another thread calling the same API. */
    hArb = hPlayer->playAtRateApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hPlayer->playAtRateApi.pSettings = pSettings;
    hPlayer->playAtRateApi.pRate = NULL;
    hPlayer->playAtRateApi.rate = rate;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hPlayer;
    arbSettings.arbProcessor = processPlayerState;
    arbSettings.waitIfBusy = true;
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
} /* BIP_Player_PlayAtRateAsync_impl */

BIP_Status  BIP_Player_PlayAtRate(
    BIP_PlayerHandle                    hPlayer,         /*!< [in] Handle of BIP_Player. */
    int                                 rate,            /*!< [in] Playback rate defined in NEXUS_NORMAL_PLAY_SPEED units. See details in nexus_playback.h */
    const BIP_PlayerPlayAtRateSettings  *pSettings       /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));

    bipStatus = BIP_Player_PlayAtRateAsync_impl(hPlayer, rate, pSettings, NULL, NULL);

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "EXIT <--------------------"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    return (bipStatus);
} /* BIP_Player_PlayAtRate */

BIP_Status  BIP_Player_PlayAtRateAsync(
    BIP_PlayerHandle                    hPlayer,         /*!< [in] Handle of BIP_Player. */
    int                                 rate,            /*!< [in] Playback rate defined in NEXUS_NORMAL_PLAY_SPEED units. See details in nexus_playback.h */
    const BIP_PlayerPlayAtRateSettings  *pSettings,      /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    BIP_CallbackDesc                    *pAsyncCallback, /*!< [in] Async completion callback: called at API completion. */
    BIP_Status                          *pAsyncStatus    /*!< [out] Completion status of async API. */
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;

    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    /* NOTE: This top level function only checks arguments specific for the Async version of the BIP API. */
    /* The common arguments in the async & non-async version are checked in the _BIP_Player_API(). */

    if ( (pAsyncStatus == NULL && pAsyncCallback == NULL) || (pAsyncStatus == NULL && pAsyncCallback && pAsyncCallback->callback == NULL) )
    {
        BDBG_ERR(( BIP_MSG_PRE_FMT  BIP_PLAYER_STATE_PRINTF_FMT "Error: Both pAsyncStatus & pAsyncCallback can't be NULL"
                   BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
        bipStatus = BIP_ERR_INVALID_PARAMETER;
        goto error;
    }

    bipStatus = BIP_Player_PlayAtRateAsync_impl(hPlayer, rate, pSettings, pAsyncCallback, pAsyncStatus);

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "EXIT <--------------------"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    return (bipStatus);

} /* BIP_Player_PlayAtRateAsync */


BIP_Status  BIP_Player_PlayByFrameAsync_impl(
    BIP_PlayerHandle                    hPlayer,         /*!< [in] Handle of BIP_Player. */
    bool                                forward,         /*!< [in] If true, play 1 frame in the forward direction, otherwise, 1 frame in the reverse direction. */
    BIP_CallbackDesc                    *pAsyncCallback, /*!< [in] Async completion callback: called at API completion. */
    BIP_Status                          *pAsyncStatus    /*!< [out] Completion status of async API. */
    )
{
    BIP_ArbHandle hArb;
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbSubmitSettings arbSettings;
    BIP_ApiSettings apiSettings;

    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );

    /* Serialize access to Settings state among another thread calling the same API. */
    hArb = hPlayer->playByFrameApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hPlayer->playByFrameApi.forward = forward;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hPlayer;
    arbSettings.arbProcessor = processPlayerState;
    arbSettings.waitIfBusy = true;
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
} /* BIP_Player_PlayByFrameAsync_impl */

BIP_Status  BIP_Player_PlayByFrame(
    BIP_PlayerHandle                    hPlayer,         /*!< [in] Handle of BIP_Player. */
    bool                                forward          /*!< [in] If true, play 1 frame in the forward direction, otherwise, 1 frame in the reverse direction. */
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));

    bipStatus = BIP_Player_PlayByFrameAsync_impl(hPlayer, forward, NULL, NULL);

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "EXIT <--------------------"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    return (bipStatus);
} /* BIP_Player_PlayByFrame */

BIP_Status  BIP_Player_PlayByFrameAsync(
    BIP_PlayerHandle                    hPlayer,         /*!< [in] Handle of BIP_Player. */
    bool                                forward,         /*!< [in] If true, play 1 frame in the forward direction, otherwise, 1 frame in the reverse direction. */
    BIP_CallbackDesc                    *pAsyncCallback, /*!< [in] Async completion callback: called at API completion. */
    BIP_Status                          *pAsyncStatus    /*!< [out] Completion status of async API. */
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;

    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    /* NOTE: This top level function only checks arguments specific for the Async version of the BIP API. */
    /* The common arguments in the async & non-async version are checked in the _BIP_Player_API(). */

    if ( (pAsyncStatus == NULL && pAsyncCallback == NULL) || (pAsyncStatus == NULL && pAsyncCallback && pAsyncCallback->callback == NULL) )
    {
        BDBG_ERR(( BIP_MSG_PRE_FMT  BIP_PLAYER_STATE_PRINTF_FMT "Error: Both pAsyncStatus & pAsyncCallback can't be NULL"
                   BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
        bipStatus = BIP_ERR_INVALID_PARAMETER;
        goto error;
    }

    bipStatus = BIP_Player_PlayByFrameAsync_impl(hPlayer, forward, pAsyncCallback, pAsyncStatus);

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "EXIT <--------------------"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    return (bipStatus);

} /* BIP_Player_PlayByFrameAsync */

BIP_Status  BIP_Player_GetStatusFromServerAsync_impl(
    BIP_PlayerHandle                         hPlayer,               /*!< [in]  Handle of BIP_Player. */
    BIP_PlayerGetStatusFromServerSettings    *pSettings,            /*!< [in]  Required: What settings to get from Server. */
    BIP_PlayerStatusFromServer               *pServerStatus,        /*!< [out] Required: Important values returned by BIP_Player_GetStatusFromServer() */
    BIP_CallbackDesc                         *pAsyncCallback,       /*!< [in]  Async completion callback: called at API completion. */
    BIP_Status                               *pAsyncStatus          /*!< [out] status of async API. */
    )
{
    BIP_ArbHandle hArb;
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbSubmitSettings arbSettings;
    BIP_ApiSettings apiSettings;

    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );

    BIP_CHECK_GOTO(( pSettings ),       ( "hPlayer=%p: pSettings can't be NULL", (void *)hPlayer ), error, BIP_ERR_INVALID_PARAMETER , bipStatus);
    BIP_CHECK_GOTO(( pServerStatus ),   ( "hPlayer=%p: pServerStatus can't be NULL", (void *)hPlayer ), error, BIP_ERR_INVALID_PARAMETER , bipStatus);

    BIP_SETTINGS_ASSERT(pSettings, BIP_PlayerGetStatusFromServerSettings);

    /* Serialize access to Settings state among another thread calling the same API. */
    hArb = hPlayer->getStatusFromServerApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hPlayer->getStatusFromServerApi.pSettings = pSettings;
    hPlayer->getStatusFromServerApi.pServerStatus = pServerStatus;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hPlayer;
    arbSettings.arbProcessor = processPlayerState;
    arbSettings.waitIfBusy = true;
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
} /* BIP_Player_GetStatusFromServerAsync_impl */

BIP_Status  BIP_Player_GetStatusFromServer(
    BIP_PlayerHandle                         hPlayer,               /*!< [in]  Handle of BIP_Player. */
    BIP_PlayerGetStatusFromServerSettings    *pSettings,            /*!< [in]  Required: What settings to get from Server. */
    BIP_PlayerStatusFromServer               *pServerStatus         /*!< [out] Required: Important values returned by BIP_Player_GetStatusFromServer() */
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));

    bipStatus = BIP_Player_GetStatusFromServerAsync_impl(hPlayer, pSettings, pServerStatus, NULL, NULL);

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "EXIT <--------------------"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    return (bipStatus);
} /* BIP_Player_GetStatusFromServer */

BIP_Status  BIP_Player_GetStatusFromServerAsync(
    BIP_PlayerHandle                         hPlayer,               /*!< [in]  Handle of BIP_Player. */
    BIP_PlayerGetStatusFromServerSettings    *pSettings,            /*!< [in]  Required: What settings to get from Server. */
    BIP_PlayerStatusFromServer               *pServerStatus,        /*!< [out] Required: Important values returned by BIP_Player_GetStatusFromServer() */
    BIP_CallbackDesc                         *pAsyncCallback,       /*!< [in]  Async completion callback: called at API completion. */
    BIP_Status                               *pAsyncStatus          /*!< [out] status of async API. */
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;

    BDBG_OBJECT_ASSERT( hPlayer, BIP_Player );

    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "ENTER --------------------->"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    /* NOTE: This top level function only checks arguments specific for the Async version of the BIP API. */
    /* The common arguments in the async & non-async version are checked in the _BIP_Player_API(). */

    if ( (pAsyncStatus == NULL && pAsyncCallback == NULL) || (pAsyncStatus == NULL && pAsyncCallback && pAsyncCallback->callback == NULL) )
    {
        BDBG_ERR(( BIP_MSG_PRE_FMT  BIP_PLAYER_STATE_PRINTF_FMT "Error: Both pAsyncStatus & pAsyncCallback can't be NULL"
                   BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
        bipStatus = BIP_ERR_INVALID_PARAMETER;
        goto error;
    }

    bipStatus = BIP_Player_GetStatusFromServerAsync_impl(hPlayer, pSettings, pServerStatus, pAsyncCallback, pAsyncStatus);

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT BIP_PLAYER_STATE_PRINTF_FMT "EXIT <--------------------"
               BIP_MSG_PRE_ARG, BIP_PLAYER_STATE_PRINTF_ARG(hPlayer) ));
    return (bipStatus);

} /* BIP_Player_GetStatusFromServerAsync */
