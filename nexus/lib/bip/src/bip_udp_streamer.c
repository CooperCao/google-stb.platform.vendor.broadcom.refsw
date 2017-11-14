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
#include "bip_udp_streamer_impl.h"
#include "bip_streamer_priv.h"

BDBG_MODULE( bip_udp_streamer );
BDBG_OBJECT_ID( BIP_UdpStreamer );
BIP_SETTINGS_ID(BIP_UdpStreamerCreateSettings);
BIP_SETTINGS_ID(BIP_UdpStreamerOutputSettings);
BIP_SETTINGS_ID(BIP_UdpStreamerStartSettings);
BIP_SETTINGS_ID_DECLARE(BIP_TranscodeProfile);
BIP_SETTINGS_ID_DECLARE(BIP_TranscodeNexusHandles);

/* Forward declaration of state processing function */
void processUdpStreamerState( void *hObject, int value, BIP_Arb_ThreadOrigin threadOrigin );

void BIP_UdpStreamer_GetDefaultSettings(
    BIP_UdpStreamerSettings *pSettings
    )
{
    BKNI_Memset( pSettings, 0, sizeof( BIP_UdpStreamerSettings ));
}

static void udpStreamerDestroy(
    BIP_UdpStreamerHandle hUdpStreamer
    )
{
    if (!hUdpStreamer) return;
    BDBG_MSG(( BIP_MSG_PRE_FMT "Destroying hUdpStreamer %p" BIP_MSG_PRE_ARG, (void *)hUdpStreamer ));

    if (hUdpStreamer->hStreamer) BIP_Streamer_Destroy(hUdpStreamer->hStreamer);
    if (hUdpStreamer->getStatusApi.hArb) BIP_Arb_Destroy(hUdpStreamer->getStatusApi.hArb);
    if (hUdpStreamer->setSettingsApi.hArb) BIP_Arb_Destroy(hUdpStreamer->setSettingsApi.hArb);
    if (hUdpStreamer->getSettingsApi.hArb) BIP_Arb_Destroy(hUdpStreamer->getSettingsApi.hArb);
    if (hUdpStreamer->fileInputSettingsApi.hArb) BIP_Arb_Destroy(hUdpStreamer->fileInputSettingsApi.hArb);
    if (hUdpStreamer->tunerInputSettingsApi.hArb) BIP_Arb_Destroy(hUdpStreamer->tunerInputSettingsApi.hArb);
    if (hUdpStreamer->recpumpInputSettingsApi.hArb) BIP_Arb_Destroy(hUdpStreamer->recpumpInputSettingsApi.hArb);
    if (hUdpStreamer->outputSettingsApi.hArb) BIP_Arb_Destroy(hUdpStreamer->outputSettingsApi.hArb);
    if (hUdpStreamer->setProgramApi.hArb) BIP_Arb_Destroy(hUdpStreamer->setProgramApi.hArb);
    if (hUdpStreamer->addTrackApi.hArb) BIP_Arb_Destroy(hUdpStreamer->addTrackApi.hArb);
    if (hUdpStreamer->addTranscodeProfileApi.hArb) BIP_Arb_Destroy(hUdpStreamer->addTranscodeProfileApi.hArb);
    if (hUdpStreamer->setTranscodeNexusHandlesApi.hArb) BIP_Arb_Destroy(hUdpStreamer->setTranscodeNexusHandlesApi.hArb);
    if (hUdpStreamer->startApi.hArb) BIP_Arb_Destroy(hUdpStreamer->startApi.hArb);
    if (hUdpStreamer->stopApi.hArb) BIP_Arb_Destroy(hUdpStreamer->stopApi.hArb);
    if (hUdpStreamer->destroyApi.hArb) BIP_Arb_Destroy(hUdpStreamer->destroyApi.hArb);

    if (hUdpStreamer->hStateMutex) B_Mutex_Destroy( hUdpStreamer->hStateMutex );


    BDBG_MSG(( BIP_MSG_PRE_FMT "hUdpStreamer %p: Destroyed" BIP_MSG_PRE_ARG, (void *)hUdpStreamer ));
    BDBG_OBJECT_DESTROY( hUdpStreamer, BIP_UdpStreamer );
    B_Os_Free( hUdpStreamer );

} /* udpStreamerDestroy */

BIP_UdpStreamerHandle BIP_UdpStreamer_Create(
    const BIP_UdpStreamerCreateSettings *pCreateSettings
    )
{
    BIP_Status                      bipStatus;
    BIP_UdpStreamerHandle          hUdpStreamer = NULL;
    BIP_UdpStreamerCreateSettings  defaultSettings;

    /* Create the udpStreamer object */
    hUdpStreamer = B_Os_Calloc( 1, sizeof( BIP_UdpStreamer ));
    BIP_CHECK_GOTO(( hUdpStreamer != NULL ), ( "Failed to allocate memory (%zu bytes) for UdpStreamer Object", sizeof(BIP_UdpStreamer) ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
    BIP_SETTINGS_ASSERT(pCreateSettings, BIP_UdpStreamerCreateSettings);

    BDBG_OBJECT_SET( hUdpStreamer, BIP_UdpStreamer );

    /* Create mutex to synchronize state machine from being run via callbacks (BIP_UdpSocket or timer) & Caller calling APIs. */
    hUdpStreamer->hStateMutex = B_Mutex_Create(NULL);
    BIP_CHECK_GOTO(( hUdpStreamer->hStateMutex ), ( "B_Mutex_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    if (NULL == pCreateSettings)
    {
        BIP_UdpStreamer_GetDefaultCreateSettings( &defaultSettings );
        pCreateSettings = &defaultSettings;
    }
    hUdpStreamer->createSettings = *pCreateSettings;

    BIP_UdpStreamer_GetDefaultSettings(&hUdpStreamer->settings);

    /* Create API ARBs: one per API */
    hUdpStreamer->getSettingsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hUdpStreamer->getSettingsApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hUdpStreamer->setSettingsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hUdpStreamer->setSettingsApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hUdpStreamer->getStatusApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hUdpStreamer->getStatusApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hUdpStreamer->fileInputSettingsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hUdpStreamer->fileInputSettingsApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hUdpStreamer->tunerInputSettingsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hUdpStreamer->tunerInputSettingsApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hUdpStreamer->recpumpInputSettingsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hUdpStreamer->recpumpInputSettingsApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hUdpStreamer->outputSettingsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hUdpStreamer->outputSettingsApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hUdpStreamer->addTrackApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hUdpStreamer->addTrackApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hUdpStreamer->addTranscodeProfileApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hUdpStreamer->addTranscodeProfileApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hUdpStreamer->setTranscodeNexusHandlesApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hUdpStreamer->setTranscodeNexusHandlesApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hUdpStreamer->startApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hUdpStreamer->startApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hUdpStreamer->stopApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hUdpStreamer->stopApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hUdpStreamer->destroyApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hUdpStreamer->destroyApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    /* Now create the BIP_Streamer object. */
    {
        BIP_StreamerCreateSettings settings;

        BIP_Streamer_GetDefaultCreateSettings( &settings );
        hUdpStreamer->hStreamer                    = BIP_Streamer_Create( &settings );
        BIP_CHECK_GOTO(( hUdpStreamer->hStreamer ), ( "BIP_Streamer_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

        hUdpStreamer->pStreamer = BIP_Streamer_GetObject_priv( hUdpStreamer->hStreamer );
        BIP_CHECK_GOTO(( hUdpStreamer->pStreamer ), ( "BIP_Streamer_GetStreamerOjbect Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
    }

    /* coverity [missing_lock] */
    hUdpStreamer->state = BIP_UdpStreamerState_eIdle;
    BDBG_MSG(( BIP_MSG_PRE_FMT "Created hUdpStreamer %p: state %d" BIP_MSG_PRE_ARG, (void *)hUdpStreamer, hUdpStreamer->state));

    BDBG_MSG((    BIP_MSG_PRE_FMT "Created: " BIP_UDP_STREAMER_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_UDP_STREAMER_PRINTF_ARG(hUdpStreamer)));
    BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Created: " BIP_UDP_STREAMER_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_UDP_STREAMER_PRINTF_ARG(hUdpStreamer)));

    return ( hUdpStreamer );

error:
    udpStreamerDestroy( hUdpStreamer );
    return ( NULL );
} /* BIP_UdpStreamer_Create */

/**
 * Summary:
 * Destroy udp socket
 *
 * Description:
 **/
void BIP_UdpStreamer_Destroy(
    BIP_UdpStreamerHandle hUdpStreamer
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hUdpStreamer, BIP_UdpStreamer );

    BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Destroying: " BIP_UDP_STREAMER_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_UDP_STREAMER_PRINTF_ARG(hUdpStreamer)));

    /* Serialize access to Settings state among another thread calling the same _GetSettings API. */
    hArb = hUdpStreamer->destroyApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hUdpStreamer;
    arbSettings.arbProcessor = processUdpStreamerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine to notify it about object being destroyed. */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_UdpStreamer_Destroy" ), error, bipStatus, bipStatus );

    /* Now release resources & destroy UdpStreamer object. */
error:
    udpStreamerDestroy( hUdpStreamer );

} /* BIP_UdpStreamer_Destroy */

void BIP_UdpStreamer_GetSettings(
        BIP_UdpStreamerHandle    hUdpStreamer,
        BIP_UdpStreamerSettings *pSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hUdpStreamer, BIP_UdpStreamer );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hUdpStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hUdpStreamer));

    BIP_CHECK_GOTO(( hUdpStreamer ), ( "hUdpStreamer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pSettings ), ( "pSettings can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    /* Serialize access to Settings state among another thread calling the same _GetSettings API. */
    hArb = hUdpStreamer->getSettingsApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hUdpStreamer->getSettingsApi.pSettings = pSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hUdpStreamer;
    arbSettings.arbProcessor = processUdpStreamerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_UdpStreamer_GetSettings" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hUdpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hUdpStreamer, bipStatus ));

    return;
}

BIP_Status BIP_UdpStreamer_SetSettings(
    BIP_UdpStreamerHandle    hUdpStreamer,
    BIP_UdpStreamerSettings *pSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hUdpStreamer, BIP_UdpStreamer );
    BDBG_ASSERT( pSettings );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hUdpStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hUdpStreamer));

    BIP_CHECK_GOTO(( hUdpStreamer ), ( "hUdpStreamer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pSettings ), ( "pSettings can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hUdpStreamer->setSettingsApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hUdpStreamer->setSettingsApi.pSettings = pSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hUdpStreamer;
    arbSettings.arbProcessor = processUdpStreamerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_UdpStreamer_SetSettings" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hUdpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hUdpStreamer, bipStatus ));

    return( bipStatus );
} /* BIP_UdpStreamer_SetSettings */

BIP_Status BIP_UdpStreamer_SetFileInputSettings(
    BIP_UdpStreamerHandle          hUdpStreamer,
    const char                      *pMediaFileAbsolutePathName,
    BIP_StreamerStreamInfo          *pStreamerStreamInfo,
    BIP_StreamerFileInputSettings   *pFileInputSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_StreamerFileInputSettings defaultFileInputSettings;

    BDBG_OBJECT_ASSERT( hUdpStreamer, BIP_UdpStreamer );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hUdpStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hUdpStreamer));

    BIP_CHECK_GOTO(( hUdpStreamer ), ( "hUdpStreamer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pMediaFileAbsolutePathName ), ( "pMediaFileAbsolutePathName can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pStreamerStreamInfo), ( "pStreamerStreamInfo can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pStreamerStreamInfo->transportType == NEXUS_TransportType_eTs ), ( "Only MPEG2 TS container is supported over UDP: containerType %d", pStreamerStreamInfo->transportType ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    /* Note: Rest of parameter validation happens in the BIP_Streamer class. */

    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hUdpStreamer->fileInputSettingsApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    if ( pFileInputSettings == NULL )
    {
        BIP_Streamer_GetDefaultFileInputSettings( &defaultFileInputSettings );
        pFileInputSettings = &defaultFileInputSettings;
    }
    /* Move the API arguments into it's argument list so the state machine can find them. */
    hUdpStreamer->fileInputSettingsApi.pStreamerStreamInfo = pStreamerStreamInfo;
    hUdpStreamer->fileInputSettingsApi.pMediaFileAbsolutePathName = pMediaFileAbsolutePathName;
    hUdpStreamer->fileInputSettingsApi.pFileInputSettings = pFileInputSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hUdpStreamer;
    arbSettings.arbProcessor = processUdpStreamerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_UdpStreamer_SetFileInputSettings" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hUdpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hUdpStreamer, bipStatus ));

    return( bipStatus );
} /* BIP_UdpStreamer_SetSettings */

BIP_Status BIP_UdpStreamer_SetTunerInputSettings(
    BIP_UdpStreamerHandle          hUdpStreamer,
    NEXUS_ParserBand                hParserBand,
    BIP_StreamerStreamInfo          *pStreamerStreamInfo,
    BIP_StreamerTunerInputSettings  *pTunerInputSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_StreamerTunerInputSettings  defaultTunerInputSettings;

    BDBG_OBJECT_ASSERT( hUdpStreamer, BIP_UdpStreamer );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hUdpStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hUdpStreamer));

    BIP_CHECK_GOTO(( hUdpStreamer ), ( "hUdpStreamer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    /* Note: Rest of parameter validation happens in the BIP_Streamer class. */

    if ( pTunerInputSettings == NULL )
    {
        BIP_Streamer_GetDefaultTunerInputSettings( &defaultTunerInputSettings );
        pTunerInputSettings = &defaultTunerInputSettings;
    }

    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hUdpStreamer->tunerInputSettingsApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hUdpStreamer->tunerInputSettingsApi.hParserBand = hParserBand;
    hUdpStreamer->tunerInputSettingsApi.pStreamerStreamInfo = pStreamerStreamInfo;
    hUdpStreamer->tunerInputSettingsApi.pTunerInputSettings = pTunerInputSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hUdpStreamer;
    arbSettings.arbProcessor = processUdpStreamerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_UdpStreamer_SetTunerInputSettings" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hUdpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hUdpStreamer, bipStatus ));

    return( bipStatus );
} /* BIP_UdpStreamer_SetSettings */

BIP_Status BIP_UdpStreamer_SetRecpumpInputSettings(
    BIP_UdpStreamerHandle    hUdpStreamer,
    NEXUS_RecpumpHandle       hRecpump,
    BIP_StreamerRecpumpInputSettings *pRecpumpInputSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_StreamerRecpumpInputSettings defaultSettings;

    BDBG_OBJECT_ASSERT( hUdpStreamer, BIP_UdpStreamer );
    BDBG_ASSERT( pRecpumpInputSettings );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hUdpStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hUdpStreamer));

    BIP_CHECK_GOTO(( hUdpStreamer ), ( "hUdpStreamer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( hRecpump ), ( "hRecpump can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hUdpStreamer->recpumpInputSettingsApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    if ( pRecpumpInputSettings == NULL )
    {
        BIP_Streamer_GetDefaultRecpumpInputSettings( &defaultSettings );
        pRecpumpInputSettings = &defaultSettings;
    }

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hUdpStreamer->recpumpInputSettingsApi.pRecpumpInputSettings = pRecpumpInputSettings;
    hUdpStreamer->recpumpInputSettingsApi.hRecpump = hRecpump;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hUdpStreamer;
    arbSettings.arbProcessor = processUdpStreamerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_UdpStreamer_SetRecpumpInputSettings" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hUdpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hUdpStreamer, bipStatus ));

    return( bipStatus );
} /* BIP_UdpStreamer_SetRecpumpInputSettings */

BIP_Status BIP_UdpStreamer_AddTrack(
    BIP_UdpStreamerHandle hUdpStreamer,
    BIP_StreamerTrackInfo *pStreamerTrackInfo,
    BIP_StreamerTrackSettings *pTrackSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_StreamerTrackSettings defaultTrackSettings;

    BDBG_OBJECT_ASSERT( hUdpStreamer, BIP_UdpStreamer );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hUdpStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hUdpStreamer));

    BIP_CHECK_GOTO(( hUdpStreamer ), ( "hUdpStreamer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pStreamerTrackInfo ), ( "pStreamerTrackInfo can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    /* Note: Rest of parameter validation happens in the BIP_Streamer class. */

    if ( pTrackSettings == NULL )
    {
        BIP_Streamer_GetDefaultTrackSettings( &defaultTrackSettings );
        pTrackSettings = &defaultTrackSettings;
    }

    hArb = hUdpStreamer->addTrackApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hUdpStreamer->addTrackApi.pStreamerTrackInfo = pStreamerTrackInfo;
    hUdpStreamer->addTrackApi.pTrackSettings = pTrackSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hUdpStreamer;
    arbSettings.arbProcessor = processUdpStreamerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_UdpStreamer_AddTrack" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hUdpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hUdpStreamer, bipStatus ));

    return( bipStatus );
}

BIP_Status BIP_UdpStreamer_SetOutputSettings(
    BIP_UdpStreamerHandle          hUdpStreamer,
    BIP_UdpStreamerProtocol        streamerProtocol,
    const char                     *pStreamerIpAddress,
    const char                     *pStreamerPort,
    const char                     *pStreamerInterfaceName,
    BIP_UdpStreamerOutputSettings  *pOutputSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_UdpStreamerOutputSettings defaultSettings;

    BDBG_OBJECT_ASSERT( hUdpStreamer, BIP_UdpStreamer );
    BDBG_ASSERT( pOutputSettings );
    BIP_SETTINGS_ASSERT(pOutputSettings, BIP_UdpStreamerOutputSettings)

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hUdpStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hUdpStreamer));

    BIP_CHECK_GOTO(( hUdpStreamer ), ( "hUdpStreamer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pStreamerPort ), ( "pStreamerPort can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pStreamerIpAddress ), ( "pStreamerIpAddress can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pStreamerInterfaceName ), ( "pStreamerInterfaceName can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pOutputSettings && !pOutputSettings->enableDtcpIp ), ( "DTCP/IP is not supported for UDP Streamer as DTCP/IP library doesn't provide this feature. " ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( streamerProtocol < BIP_UdpStreamerProtocol_eMax ), ( "streamerProtocol %d is not valid", streamerProtocol ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    if ( pOutputSettings == NULL )
    {
        BIP_UdpStreamer_GetDefaultOutputSettings( &defaultSettings );
        pOutputSettings = &defaultSettings;
    }
    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hUdpStreamer->outputSettingsApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hUdpStreamer->outputSettingsApi.pOutputSettings = pOutputSettings;
    hUdpStreamer->outputSettingsApi.streamerProtocol = streamerProtocol;
    hUdpStreamer->outputSettingsApi.pStreamerIpAddress = pStreamerIpAddress;
    hUdpStreamer->outputSettingsApi.pStreamerPort = pStreamerPort;
    hUdpStreamer->outputSettingsApi.pStreamerInterfaceName = pStreamerInterfaceName;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hUdpStreamer;
    arbSettings.arbProcessor = processUdpStreamerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_UdpStreamer_SetOutputSettings" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hUdpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hUdpStreamer, bipStatus ));

    return( bipStatus );
} /* BIP_UdpStreamer_SetSettings */

BIP_Status BIP_UdpStreamer_AddTranscodeProfile(
    BIP_UdpStreamerHandle  hUdpStreamer,
    BIP_TranscodeProfile    *pTranscodeProfile
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BIP_CHECK_GOTO(( hUdpStreamer ), ( "hUdpStreamer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pTranscodeProfile ), ( "pTranscodeProfile can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    BDBG_OBJECT_ASSERT( hUdpStreamer, BIP_UdpStreamer );
    BIP_SETTINGS_ASSERT(pTranscodeProfile, BIP_TranscodeProfile);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hUdpStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hUdpStreamer));

    /* Note: Rest of parameter validation happens in the BIP_Streamer class. */

    hArb = hUdpStreamer->addTranscodeProfileApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hUdpStreamer->addTranscodeProfileApi.pTranscodeProfile = pTranscodeProfile;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hUdpStreamer;
    arbSettings.arbProcessor = processUdpStreamerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_UdpStreamer_AddTranscodeProfile" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hUdpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hUdpStreamer, bipStatus ));

    return( bipStatus );
}

BIP_Status BIP_UdpStreamer_SetTranscodeHandles(
    BIP_UdpStreamerHandle       hUdpStreamer,
    BIP_TranscodeNexusHandles   *pTranscodeNexusHandles
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BIP_CHECK_GOTO(( hUdpStreamer ), ( "hUdpStreamer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pTranscodeNexusHandles ), ( "pTranscodeNexusHandles can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    BDBG_OBJECT_ASSERT( hUdpStreamer, BIP_UdpStreamer );
    BIP_SETTINGS_ASSERT(pTranscodeNexusHandles, BIP_TranscodeNexusHandles);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hUdpStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hUdpStreamer));

    hArb = hUdpStreamer->setTranscodeNexusHandlesApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hUdpStreamer->setTranscodeNexusHandlesApi.pTranscodeNexusHandles = pTranscodeNexusHandles;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hUdpStreamer;
    arbSettings.arbProcessor = processUdpStreamerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_UdpStreamer_AddTranscodeProfile" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hUdpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hUdpStreamer, bipStatus ));

    return( bipStatus );
}

BIP_Status BIP_UdpStreamer_Start(
    BIP_UdpStreamerHandle    hUdpStreamer,
    BIP_UdpStreamerStartSettings *pSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_UdpStreamerStartSettings defaultStartSettings;

    BDBG_OBJECT_ASSERT( hUdpStreamer, BIP_UdpStreamer );
    BIP_SETTINGS_ASSERT(pSettings, BIP_UdpStreamerStartSettings)

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hUdpStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hUdpStreamer));

    BIP_CHECK_GOTO(( hUdpStreamer ), ( "hUdpStreamer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    if ( pSettings == NULL )
    {
        BIP_UdpStreamer_GetDefaultStartSettings( &defaultStartSettings );
        pSettings = &defaultStartSettings;
    }
    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hUdpStreamer->startApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hUdpStreamer->startApi.pSettings = pSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hUdpStreamer;
    arbSettings.arbProcessor = processUdpStreamerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_UdpStreamer_Start" ), error, bipStatus, bipStatus );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hUdpStreamer %p: Streamer Started: completionStatus: %s" BIP_MSG_PRE_ARG, (void *)hUdpStreamer, BIP_StatusGetText(bipStatus) ));

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hUdpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hUdpStreamer, bipStatus ));

    return ( bipStatus );
} /* BIP_UdpStreamer_Start */

BIP_Status BIP_UdpStreamer_Stop(
    BIP_UdpStreamerHandle    hUdpStreamer
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hUdpStreamer, BIP_UdpStreamer );

    BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Stopping: " BIP_UDP_STREAMER_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_UDP_STREAMER_PRINTF_ARG(hUdpStreamer)));
    BDBG_MSG((    BIP_MSG_PRE_FMT "Stopping: " BIP_UDP_STREAMER_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_UDP_STREAMER_PRINTF_ARG(hUdpStreamer)));

    BIP_CHECK_GOTO(( hUdpStreamer ), ( "hUdpStreamer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hUdpStreamer->stopApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hUdpStreamer;
    arbSettings.arbProcessor = processUdpStreamerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_UdpStreamer_Stop" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hUdpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hUdpStreamer, bipStatus ));

    return( bipStatus );
} /* BIP_UdpStreamer_Stop */

BIP_Status  BIP_UdpStreamer_GetStatus(
        BIP_UdpStreamerHandle  hUdpStreamer,
        BIP_UdpStreamerStatus  *pStatus
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hUdpStreamer, BIP_UdpStreamer );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hUdpStreamer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hUdpStreamer));

    BIP_CHECK_GOTO(( hUdpStreamer ), ( "hUdpStreamer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pStatus ), ( "pStatus can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    /* Serialize access to Settings state among another thread calling the same _GetStatus API. */
    hArb = hUdpStreamer->getStatusApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hUdpStreamer->getStatusApi.pStatus = pStatus;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hUdpStreamer;
    arbSettings.arbProcessor = processUdpStreamerState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_UdpStreamer_GetStatus" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hUdpStreamer %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hUdpStreamer, bipStatus ));

    return ( bipStatus );
}
