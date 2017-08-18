/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bip_rtsp_server.h"
#include "liveMedia/bip_rtsp_lm_server.h"
#include "bip_rtsp_lm_server.h"

BDBG_MODULE( bip_rtsp_session );
BDBG_OBJECT_ID( BIP_RtspLiveMediaSession );

typedef struct BIP_RtspLiveMediaSession
{
    BDBG_OBJECT( BIP_RtspLiveMediaSession )
    BIP_RtspServer::BIP_RtspClientSession *lmRtspClientSession;
    int *pidList;
} BIP_RtspLiveMediaSession;

BIP_RtspLiveMediaSessionHandle BIP_RtspLiveMediaSession_CreateFromRequest(
    char                                   *requestStr,
    void                                   *lmRtspClientConnectionPtr,
    BIP_RtspLiveMediaSessionCreateSettings *pSettings
    )
{
    BSTD_UNUSED( pSettings );
    BIP_RtspLiveMediaSessionHandle            hRtspLmSession         = NULL;
    BIP_RtspServer::BIP_RtspClientConnection *lmRtspClientConnection = (BIP_RtspServer::BIP_RtspClientConnection *)lmRtspClientConnectionPtr;

    hRtspLmSession = (BIP_RtspLiveMediaSessionHandle) BKNI_Malloc( sizeof( *hRtspLmSession ));
    BIP_CHECK_PTR_GOTO( hRtspLmSession, "Memory Allocation Failed", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );
    BKNI_Memset(hRtspLmSession, 0, sizeof(*hRtspLmSession));
    BDBG_OBJECT_SET( hRtspLmSession, BIP_RtspLiveMediaSession );

    /* create a new RTCPClientSession object */
    hRtspLmSession->lmRtspClientSession = lmRtspClientConnection->createNewClientSession( lmRtspClientConnection, requestStr );
    BIP_CHECK_PTR_GOTO( hRtspLmSession->lmRtspClientSession, "Memory Allocation Failed", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );

    BDBG_MSG(( "%s: hRtspLmSession %p", BSTD_FUNCTION, (void *)hRtspLmSession ));
    return( hRtspLmSession );

error:
    if (hRtspLmSession)
    {
        BKNI_Free( hRtspLmSession );
    }
    return( NULL );
} // BIP_RtspLiveMediaSession_CreateFromRequest

/**
 * Summary:
 * Destroy socket
 *
 * Description:
 **/
void BIP_RtspLiveMediaSession_Destroy(
    BIP_RtspLiveMediaSessionHandle hRtspLmSession
    )
{
    BDBG_OBJECT_ASSERT( hRtspLmSession, BIP_RtspLiveMediaSession );

    BDBG_MSG(( "%s: hRtspLmSession %p", BSTD_FUNCTION, (void *)hRtspLmSession ));
    if (hRtspLmSession)
    {
        if ( hRtspLmSession->pidList ) {
            BDBG_MSG(( "%s: Free pidList (%p)", BSTD_FUNCTION, (void *)hRtspLmSession->pidList ));
            BKNI_Free( hRtspLmSession->pidList );
        }

        if ( hRtspLmSession->lmRtspClientSession  )
        {
            BDBG_MSG(( "%s: delete hRtspLmSession->lmRtspClientSession (%p)", BSTD_FUNCTION, (void *)hRtspLmSession->lmRtspClientSession ));
            delete hRtspLmSession->lmRtspClientSession;
        }

        BDBG_MSG(( "%s: Free hRtspLmSession (%p)", BSTD_FUNCTION, (void *)hRtspLmSession ));
        BKNI_Free( hRtspLmSession );
    }
}

void BIP_RtspLiveMediaSession_GetSettings(
    BIP_RtspLiveMediaSessionHandle    hRtspLmSession,
    BIP_RtspLiveMediaSessionSettings *pSettings
    )
{
    BSTD_UNUSED( hRtspLmSession );
    BSTD_UNUSED( pSettings );
}

BIP_Status BIP_RtspLiveMediaSession_SetSettings(
    BIP_RtspLiveMediaSessionHandle    hRtspLmSession,
    BIP_RtspLiveMediaSessionSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT( hRtspLmSession, BIP_RtspLiveMediaSession );
    BDBG_ASSERT( pSettings );

    /* just pass thru the settings to the actual C++ class: BIP_RtspServer */

    /* set the callback with the liveMedia object */
    hRtspLmSession->lmRtspClientSession->setMessageReceivedCallback( pSettings->messageReceivedCallback );
    hRtspLmSession->lmRtspClientSession->setIgmpMembershipReportCallback(pSettings->igmpMembershipReportCallback);
    return( BIP_SUCCESS );
}

BIP_Status BIP_RtspLiveMediaSession_SendResponse(
    BIP_RtspLiveMediaSessionHandle hRtspLmSession,
    BIP_RtspResponseStatus         responseStatus
    )
{
    BDBG_MSG(("%s: calling sendResponse(%x)", BSTD_FUNCTION, responseStatus ));
    hRtspLmSession->lmRtspClientSession->sendResponse( responseStatus );
    return( BIP_SUCCESS );
}

BIP_Status BIP_RtspLiveMediaSession_ReportLockStatus(
    BIP_RtspLiveMediaSessionHandle hRtspLmSession,
    bool                           bLockStatus
    )
{
    BDBG_MSG(("%s: bLockStatus (%d)", BSTD_FUNCTION, bLockStatus ));
    hRtspLmSession->lmRtspClientSession->reportLockStatus( bLockStatus );
    return( BIP_SUCCESS );
}

BIP_Status BIP_RtspLiveMediaSession_SendResponseUsingRequest(
    BIP_RtspLiveMediaSessionHandle hRtspLmSession,
    BIP_RtspResponseStatus         responseStatus,
    char                          *pRequestBuffer
    )
{
    hRtspLmSession->lmRtspClientSession->sendResponseUsingRequest( responseStatus, pRequestBuffer );
    return( BIP_SUCCESS );
}

/**
 * Summary:
 * Copy current RTSP Message
 *
 * Description:
 *
 * BIP_RtspSession class invokes this API to copy the currently received RTSP Message into its buffer so that it can
 * queue up this message request for app's consumption.
 *
 **/
void BIP_RtspLiveMediaSession_CopyMessage(
    BIP_RtspLiveMediaSessionHandle hRtspLmSession,
    char                          *pBuffer
    )
{
    BDBG_MSG(( "%s: hRtspLmSession (%p)", BSTD_FUNCTION,  (void *)hRtspLmSession ));
    BDBG_MSG(( "%s: hRtspLmSession->lmRtspClientSession (%p)", BSTD_FUNCTION,  (void *)hRtspLmSession->lmRtspClientSession ));
    BDBG_MSG(( "%s: sessionId %08X; calling hRtspLmSession->lmRtspClientSession->copyReceivedMessage(%s)", BSTD_FUNCTION,
               hRtspLmSession->lmRtspClientSession->getClientSessionId(), pBuffer ));
    hRtspLmSession->lmRtspClientSession->copyReceivedMessageSession( pBuffer );
}
#if NEXUS_HAS_FRONTEND
/**
 * Summary:
 * Parse Satellite Settings
 *
 * Description:
 *
 **/
BIP_Status BIP_RtspLiveMediaSession_ParseSatelliteSettings(
    BIP_RtspLiveMediaSessionHandle   hRtspLmSession,
    NEXUS_FrontendSatelliteSettings *pSatelliteSettings,
    NEXUS_FrontendDiseqcSettings *pDiseqcSettings,
    BIP_AdditionalSatelliteSettings *pAddSatelliteSettings
    )
{
    unsigned freqMhz;
    unsigned rollOffHundredTimes;
    BIP_RtspServer::BIP_RtspClientSession *clientSession = NULL;

    BDBG_OBJECT_ASSERT( hRtspLmSession, BIP_RtspLiveMediaSession );

    clientSession = hRtspLmSession->lmRtspClientSession;

    /* Copy srcId */
    pAddSatelliteSettings->signalSourceId = clientSession->satelliteSettings.src;


    NEXUS_Frontend_GetDefaultSatelliteSettings( pSatelliteSettings );
    pSatelliteSettings->frequency  = clientSession->satelliteSettings.freq *1000000;
    freqMhz = clientSession->satelliteSettings.freq;
    /* Adjust Frequency  if looks like a transponder frequency  */
/**
 *The local oscillator frequency determines what block of incoming frequencies is downconverted to the frequencies expected by the receiver.
 *For example, to downconvert the incoming signals from Astra 1KR, which transmits in a frequency block of 10.70-11.70 GHz,
 *to within a standard European receiver’s IF tuning range of 950-2150 MHz, a 9.75 GHz local oscillator frequency is used,
 *producing a block of signals in the band 950-1950 MHz.
 *For the block of higher transmission frequencies used by Astra 2A and 2B (11.70-12.75 GHz),
 *a different local oscillator frequency converts the block of incoming frequencies.
 *Typically, a local oscillator frequency of 10.60 GHz is used to downconvert the block to 1100-2150 MHz,
 *which is still within the receiver’s 950-2150 MHz IF tuning range.
 **/

    if (freqMhz >= 10700 && freqMhz < 11700)
    {
         unsigned interFreqMhz;
         /*Universal LNB("Astra"LNB) low band case */
         interFreqMhz = (freqMhz - 9750);

#if 0
        /* 4528 no longer supported so don't add this conversion */
         pSatelliteSettings->frequency = (2000 - interFreqMhz) * 1000000; /* nexus tune freq */
#else
        pSatelliteSettings->frequency = interFreqMhz * 1000000;
#endif

         BDBG_MSG(("Universal(Astra)LNB low band case: transponder freq in Mhz %u, L-Band Freq in Mhz %d ", freqMhz, interFreqMhz));
         pDiseqcSettings->toneEnabled = false;
    }
    else if(freqMhz >=11700 && freqMhz <= 12750)
    {
         /*Universal LNB("Astra"LNB)  high band case */
         pSatelliteSettings->frequency = (freqMhz - 10600) * 1000000;
         BDBG_MSG(("Universal(Astra)LNB high band case: transponder freq in Mhz %u, L-Band Freq in Mhz %d ", freqMhz, freqMhz - 10600));
         pDiseqcSettings->toneEnabled = true;
    }
    else
    {
        BDBG_MSG(("Assuming L-Band frequency passed in: 950-2150 MHz(No conversion), L-Band Freq in Mhz %d ", freqMhz));
        pSatelliteSettings->frequency  = freqMhz *1000000;
        /*What should Tone enabled be set to  does */
        /* defaults for San Diego lab */
        pDiseqcSettings->toneEnabled = true;
    }

    pSatelliteSettings->symbolRate = clientSession->satelliteSettings.sr *1000;

    if (strcmp(  clientSession->satelliteSettings.msys, "dvbs" ) == 0)
    {
        pSatelliteSettings->mode = NEXUS_FrontendSatelliteMode_eDvb;
    }
    else if (strcmp( clientSession->satelliteSettings.msys, "dvbs2" ) == 0)
    {
        if (strcmp( clientSession->satelliteSettings.mtype, "qpsk" ) == 0)
        {
            pSatelliteSettings->mode = NEXUS_FrontendSatelliteMode_eQpskLdpc;
        }
        else if (strcmp( clientSession->satelliteSettings.mtype, "8psk" ) == 0)
        {
            pSatelliteSettings->mode = NEXUS_FrontendSatelliteMode_e8pskLdpc;
        }
        else
        {
            BDBG_ERR(( "%s:if msys is set mtype(%s) must be set. Error", BSTD_FUNCTION, clientSession->satelliteSettings.mtype ));
            goto error;
        }

        if (( pSatelliteSettings->mode  == NEXUS_FrontendSatelliteMode_eQpskLdpc ) ||
            ( pSatelliteSettings->mode  == NEXUS_FrontendSatelliteMode_e8pskLdpc ))
        {
            /* for DVBS2, update the pilotTone settings */

            if (strcmp( clientSession->satelliteSettings.plts, "on" ) == 0)
            {
                pSatelliteSettings->ldpcPilot     = true;
                pSatelliteSettings->ldpcPilotPll  = true;
                pSatelliteSettings->ldpcPilotScan = false;
            }
            else if (strcmp( clientSession->satelliteSettings.plts, "off" ) == 0)
            {
                pSatelliteSettings->ldpcPilot     = false;
                pSatelliteSettings->ldpcPilotPll  = false;
                pSatelliteSettings->ldpcPilotScan = false;
            }
            else
            {
                bool eightpsk =  pSatelliteSettings->mode == NEXUS_FrontendSatelliteMode_e8pskLdpc ? 1 : 0;
                pSatelliteSettings->ldpcPilot     = eightpsk;
                pSatelliteSettings->ldpcPilotPll  = eightpsk;
                pSatelliteSettings->ldpcPilotScan = !eightpsk;
            }

            BDBG_MSG(( "%s: ldpcPilot %d, ldpcPilotPll %d ldpcPilotScan %d", BSTD_FUNCTION, pSatelliteSettings->ldpcPilot, pSatelliteSettings->ldpcPilotPll, pSatelliteSettings->ldpcPilotScan ));
        }
    }
    else
    {
        BDBG_ERR(( "%s:  Sat Modulation mode %d is not yet supported or not set", BSTD_FUNCTION, pSatelliteSettings->mode ));
        goto error;
    }

 #if 0   /* TODO  this logic causes frontend not able to lock. Brought from ip_streamer */
    if (clientSession->satelliteSettings.fec)
    {
        pSatelliteSettings->codeRate.numerator   = clientSession->satelliteSettings.fec == 910 ? 9 : clientSession->satelliteSettings.fec / 10;
        pSatelliteSettings->codeRate.denominator = clientSession->satelliteSettings.fec == 910 ? 10 : clientSession->satelliteSettings.fec % 10;
    }
#endif // if 0
    rollOffHundredTimes = clientSession->satelliteSettings.ro * 100;
    if(rollOffHundredTimes == 10)
    {
        BDBG_ERR(( "%s: ERROR: don't support  set BSAT_ACQ_NYQUIST_10 in Nexus API yet", BSTD_FUNCTION ));
    }
    else if (rollOffHundredTimes == 35 )
    {
         pSatelliteSettings->nyquist20 = false;
    }
    else if (rollOffHundredTimes == 20 || rollOffHundredTimes == 25 )
    {
         pSatelliteSettings->nyquist20 = true;
    }
    else
    {
        BDBG_MSG(( "%s: roll_off value (%f) not set to 0.35,0.25, or 0.20. Defaulting to 0.20", BSTD_FUNCTION,clientSession->satelliteSettings.ro ));
        pSatelliteSettings->nyquist20 = true;
    }

    if (strcmp(  clientSession->satelliteSettings.pol, "h" ) == 0)
    {
        pDiseqcSettings->voltage= NEXUS_FrontendDiseqcVoltage_e18v;
    }
    else if(strcmp(  clientSession->satelliteSettings.pol, "v" ) == 0)
    {
        pDiseqcSettings->voltage= NEXUS_FrontendDiseqcVoltage_e13v;
    }
    else
    {
        BDBG_ERR(( "%s: Not sure what to do with pol of value or not set: %s (yet)", BSTD_FUNCTION,clientSession->satelliteSettings.pol ));
    }
    return( BIP_SUCCESS );

error:

    return( BIP_ERR_INTERNAL );
} // BIP_RtspLiveMediaSession_ParseSatelliteSettings
#endif
/**
 * Summary:
 * Get Transport Status
 *
 **/
BIP_Status BIP_RtspLiveMediaSession_GetTransportStatus(
    BIP_RtspLiveMediaSessionHandle hRtspLmSession,
    BIP_RtspTransportStatus      *pTransportStatus
    )
{
    BIP_RtspServer::BIP_RtspClientSession *clientSession = NULL;

    BDBG_OBJECT_ASSERT( hRtspLmSession, BIP_RtspLiveMediaSession );

    clientSession     = hRtspLmSession->lmRtspClientSession;
    *pTransportStatus = clientSession->transportStatus;

    BDBG_MSG(( "%s:  pTransportStatus ->clientIpStr %s", BSTD_FUNCTION, ( pTransportStatus )->clientAddressStr ));

    BDBG_MSG(( "%s:  pTransportStatus ->isMulticast %d", BSTD_FUNCTION, ( pTransportStatus )->isMulticast ));

    return( BIP_SUCCESS );
}

BIP_Status  BIP_RtspLiveMediaSession_GetPids(
    BIP_RtspLiveMediaSessionHandle hRtspLmSession,
    BIP_PidInfo                  *pPids
    )
{
    int i;
    BIP_Status rc;
    BIP_RtspServer::BIP_RtspClientSession *clientSession = NULL;

    /* BIP_Rtsp_SatSession *pSession = NULL; */

    BDBG_OBJECT_ASSERT( hRtspLmSession, BIP_RtspLiveMediaSession );
    /*   BIP_CHECK_PTR_GOTO( pSatelliteSettings, "pSettings is invalid", error, BIP_ERR_INVALID_PARAMETER );*/

    clientSession       = hRtspLmSession->lmRtspClientSession;

    if ( hRtspLmSession->pidList ) BKNI_Free( hRtspLmSession->pidList );
    hRtspLmSession->pidList = (int*) BKNI_Malloc(sizeof(int) * clientSession->satelliteSettings.pidListCount);
    BIP_CHECK_GOTO( (hRtspLmSession), ("Memory Allocation Failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );
    BKNI_Memset(hRtspLmSession->pidList, 0,  sizeof(int) * clientSession->satelliteSettings.pidListCount);
    pPids->pidList = hRtspLmSession->pidList;
    pPids->pidListCount = clientSession->satelliteSettings.pidListCount;
    for(i=0;i < pPids->pidListCount ;i++)
    {
        pPids->pidList[i] = clientSession->satelliteSettings.pidList[i];
    }

    rc = BIP_SUCCESS;
error:
    return( rc );
}
