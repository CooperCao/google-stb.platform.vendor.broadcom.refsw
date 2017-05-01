/******************************************************************************
 * Copyright (C) 2016-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Extension of RTSP Server Class to provide a SAT>IP Specific RTSP Server
 **/

#include "bip_priv.h"
#include "bip_igmp_listener.h"
#include "bip_rtsp_lm_server.h"
#include "liveMedia/bip_rtsp_lm_server.h"
#include "RTSPCommon.hh"
#include "GroupsockHelper.hh"
#include "blst_slist.h"
#include "nexus_platform_features.h"
#include "bip_rtsp_lm_sink.h"

BDBG_MODULE( bip_rtsp_lm_server );
BDBG_FILE_MODULE( bip_rtsp_lm_server_destructor );
#define BDBG_DESTRUCTOR( bdbg_args ) BDBG_MODULE_MSG( bip_rtsp_lm_server_destructor, bdbg_args );
BDBG_FILE_MODULE( bip_rtsp_lm_server_reqsend );
#define BDBG_REQSEND( bdbg_args ) BDBG_MODULE_MSG( bip_rtsp_lm_server_reqsend, bdbg_args );

#define USE_IGMP 1
/*
#0  0xb6f5d760 in B_Mutex_Lock (mutex=0xda7adead) at /build/detrick/refsw/gitrepo033/nexus/lib/os/src/linuxuser/b_os_mutex.c:138
#1  0xb5e63868 in verifySpecificMulticast (session=0x1460c0) at /build/detrick/refsw/gitrepo033/nexus/lib/../lib/bip/src/bip_igmp_listener.c:1329
#2  0xb6f5f0cc in B_Scheduler_Run (scheduler=0x1458f0) at /build/detrick/refsw/gitrepo033/nexus/lib/os/src/b_os_scheduler.c:540
#3  0xb6f5dab4 in B_Thread_P_Launcher (pParam=0x145a28) at /build/detrick/refsw/gitrepo033/nexus/lib/os/src/linuxuser/b_os_thread.c:365

*/

#define MAX_SDP_DESCRIPTION_LEN 256
#define STREAMID_FORMAT         "stream=%d"
#define RESPONSE_BUFFER_SIZE    768

static u_int16_t gStreamId = 0;

typedef struct BIP_Rtsp_IgmpContext
{
    unsigned int    sessionId;             /*Session id */
    BIP_RtspServer *fOurServer;
} BIP_Rtsp_IgmpContext;

/**
 *  Function: This function is the callback function that is used to wrap the C++ function so that we can
 *  the function from scheduleDelayedTask() API.
 **/
void BIP_Rtsp_GetRtpStatisticsCallback(
    void *context
    )
{
    BIP_RtspServer::BIP_RtspClientSession *pClientSession = (BIP_RtspServer::BIP_RtspClientSession *)context;
    BIP_RtspLmSessionStreamState           stateNow       = BIP_RtspLmSessionStreamState_Idle;

    BIP_CHECK_PTR_GOTO( pClientSession, "pClientSession is NULL", error, BIP_ERR_INVALID_PARAMETER );

    stateNow = pClientSession->getState();

    // if the stream is in one of the playing states
    if (stateNow == BIP_RtspLmSessionStreamState_MulticastPlay ||
        stateNow == BIP_RtspLmSessionStreamState_UnicastPlay ||
        stateNow == BIP_RtspLmSessionStreamState_IgmpJoinStreaming )
    {
        pClientSession->getRtpStatistics( pClientSession );
    }

    pClientSession->scheduleGetRtpStatistics(); // re-schedule the periodic
error:
    return;
} // BIP_Rtsp_GetRtpStatisticsCallback

/**
 *  Function: This function will examine the specified bitmask of parameters that were provided in the URL. If a
 *  specific parameter was provided, then this function will update the corresponding parameter in the satellite
 *  and/or transport structures.
 *
 *  This function is used so that we only update those parameters that are specified in the URL. Sometimes, the
 *  user just provides one parameter at a time.
 **/
int BIP_RtspServer::BIP_RtspClientSession:: updateSettings(
    int                                        bitmaskOptionsFound,
    BIP_RtspLiveMediaSessionSatelliteSettings *pSatelliteSettings,
    BIP_RtspTransportStatus                   *pTransportStatus
    )
{
    BIP_RtspServer::BIP_RtspClientSession *pClientSession = this;

    BDBG_MSG(( "%s: session %p, bitmask 0x%08X", __FUNCTION__, (void *)pClientSession, bitmaskOptionsFound ));
    BIP_CHECK_PTR_GOTO( pClientSession, "pClientSession is invalid", error, BIP_ERR_INVALID_PARAMETER );

    /* if client specifies just the session id (and not steam=xyz), this is a valid request ... as long as we can figure out the session -> process it */
    #if 0
    if (!( bitmaskOptionsFound & 0x00001 ))   // streamId
    {
        BDBG_ERR(( "%s: ERROR ... stream ID is required to update existing stream", __FUNCTION__ ));
        return( 0 );
    }
    #endif

    if (bitmaskOptionsFound & 0x00002)   // satSettings->freq
    {
        pClientSession->satelliteSettings.freq = pSatelliteSettings->freq;
        BDBG_MSG(( "%s: new freq %d", __FUNCTION__, pClientSession->satelliteSettings.freq ));
    }
    if (bitmaskOptionsFound & 0x00004)   // satSettings->mtype
    {
        strncpy( pClientSession->satelliteSettings.mtype, pSatelliteSettings->mtype, sizeof( pClientSession->satelliteSettings.mtype )-1 );
        BDBG_MSG(( "%s: new mtype %s", __FUNCTION__, pClientSession->satelliteSettings.mtype ));
    }
    if (bitmaskOptionsFound & 0x00008)   // satSettings->msys
    {
        strncpy( pClientSession->satelliteSettings.msys, pSatelliteSettings->msys, sizeof( pClientSession->satelliteSettings.msys )-1 );
        BDBG_MSG(( "%s: new msys %s", __FUNCTION__, pClientSession->satelliteSettings.msys ));
    }
    if (bitmaskOptionsFound & 0x00010)   // satSettings->pilotTone
    {
        pClientSession->satelliteSettings.pilotTone = pSatelliteSettings->pilotTone;
        BDBG_MSG(( "%s: new pilotTone %d", __FUNCTION__, pClientSession->satelliteSettings.pilotTone ));
    }
    if (bitmaskOptionsFound & 0x00020)   // satSettings->sr
    {
        pClientSession->satelliteSettings.sr = pSatelliteSettings->sr;
        BDBG_MSG(( "%s: new sr %d", __FUNCTION__, pClientSession->satelliteSettings.sr ));
    }
    if (bitmaskOptionsFound & 0x00040)   // satSettings->fec
    {
        pClientSession->satelliteSettings.fec = pSatelliteSettings->fec;
        BDBG_MSG(( "%s: new fec %d", __FUNCTION__, pClientSession->satelliteSettings.fec ));
    }
    if (bitmaskOptionsFound & 0x00080)   // satSettings->ro
    {
        pClientSession->satelliteSettings.ro = pSatelliteSettings->ro;
        BDBG_MSG(( "%s: new ro %f", __FUNCTION__, pClientSession->satelliteSettings.ro ));
    }
    if (bitmaskOptionsFound & 0x00100)   // satSettings->pol_int
    {
        pClientSession->satelliteSettings.pol_int = pSatelliteSettings->pol_int;
        strncpy( pClientSession->satelliteSettings.pol, pSatelliteSettings->pol, sizeof( pClientSession->satelliteSettings.pol )-1 );
        BDBG_MSG(( "%s: new pol_int %d", __FUNCTION__, pClientSession->satelliteSettings.pol_int ));
        BDBG_MSG(( "%s: new pol %s", __FUNCTION__, pClientSession->satelliteSettings.pol ));
    }
    if (bitmaskOptionsFound & 0x00200)   // satSettings->pidListCount
    {
        int idx = 0;
        pClientSession->satelliteSettings.pidListCount = pSatelliteSettings->pidListCount;

        BDBG_MSG(( "%s: new pidListCount %d", __FUNCTION__, pClientSession->satelliteSettings.pidListCount ));
        BKNI_Memset( pClientSession->satelliteSettings.pidList, 0, sizeof( pClientSession->satelliteSettings.pidList ));
        for (idx = 0; idx<pSatelliteSettings->pidListCount; idx++) {
            pClientSession->satelliteSettings.pidList[idx] = pSatelliteSettings->pidList[idx];
            BDBG_MSG(( "%s: new pidList[%d] = %d", __FUNCTION__, idx, pClientSession->satelliteSettings.pidList[idx] ));
        }
    }
    if (bitmaskOptionsFound & 0x40000)   // satSettings->addPidListCount
    {
        int curPid;

        /* tmpBuf string point to <num>,<num2>,<num3>... 0*/
        if (pSatelliteSettings->addPidListCount > 0)
        {
            int h;
            for (h = 0;
                 h < pSatelliteSettings->addPidListCount; /* leaving one space for last entry, moves str to something smaller*/
                 h++
                 )
            {
                curPid = pSatelliteSettings->addPidList[h];
                /* Check if Pid already exist */
                bool alreadyInPidList = false;
                {
                    int i;
                    for (i = 0; i<pClientSession->satelliteSettings.pidListCount; i++) {
                        BDBG_MSG(( "pid[%d] = 0x%x of %d pids", i, pClientSession->satelliteSettings.pidList[i], pClientSession->satelliteSettings.pidListCount ));
                        if (curPid ==  pClientSession->satelliteSettings.pidList[i])
                        {
                            /* found in pid list */
                            alreadyInPidList = true;
                            break;
                        }
                    }
                }

                if (alreadyInPidList ==false)
                {
                    /* Add to pidList  to the end*/

                    BDBG_MSG(( "%s: addPid param. %d to end of Pid List.", __FUNCTION__, curPid ));
                    pClientSession->satelliteSettings.pidList[( pClientSession->satelliteSettings.pidListCount )++] = curPid;
                }
                else
                {
                    BDBG_WRN(( "%s: addPid param. %d already in list of pids.", __FUNCTION__, curPid ));
                }
                BDBG_MSG(( "%s: pidList[%d] %d", __FUNCTION__, ( pClientSession->satelliteSettings.pidListCount )-1, pClientSession->satelliteSettings.pidList[( pClientSession->satelliteSettings.pidListCount )-1] ));

                alreadyInPidList = false; /* reset back for next pid*/
            }

            if (( pClientSession->satelliteSettings.pidListCount == BIP_MAX_PIDS_PER_PROGRAM ))
            {
                BDBG_ERR(( "%s: pidList is not big enough to store all pids, current size %d", __FUNCTION__, BIP_MAX_PIDS_PER_PROGRAM ));
            }

            /* Testing */
            {
                int i;
                for (i = 0; i<pClientSession->satelliteSettings.pidListCount; i++) {
                    BDBG_MSG(( "pid[%d] = 0x%x of %d pids", i, pClientSession->satelliteSettings.pidList[i], pClientSession->satelliteSettings.pidListCount ));
                }
            }
        }
    }
    if (bitmaskOptionsFound & 0x80000)   // satSettings->delPidListCount
    {
        int curPid;
        /* tmpBuf string point to <num>,<num2>,<num3>... 0*/
        if (pSatelliteSettings->delPidListCount > 0)
        {
            int h;

            for (h = 0;
                 h < pSatelliteSettings->delPidListCount; /* leaving one space for last entry, moves str to something smaller*/
                 h++
                 )
            {
                curPid = pSatelliteSettings->delPidList[h];
                /* Check if Pid already exist */
                {
                    int i;
                    for (i = 0; i<pClientSession->satelliteSettings.pidListCount && i < BIP_MAX_PIDS_PER_PROGRAM; i++) {
                        BDBG_MSG(( "pid[%d] = 0x%x of %d pids", i, pClientSession->satelliteSettings.pidList[i], pClientSession->satelliteSettings.pidListCount ));
                        if (curPid == pClientSession->satelliteSettings.pidList[i])
                        {
                            /* remove this pid */

                            BDBG_MSG(( "%s: delPid param. Found and Removing %d from current  list of pids.", __FUNCTION__, curPid ));
                            int j;
                            pClientSession->satelliteSettings.pidListCount--;                  /* shorten list by 1*/
                            for (j = i; j<BIP_MAX_PIDS_PER_PROGRAM-1; j++) /* move over remaing elements to left*/
                            {
                                pClientSession->satelliteSettings.pidList[j] = pClientSession->satelliteSettings.pidList[j+1];
                            }
                            /*change last element to 0, pidList count is one less*/
                            pClientSession->satelliteSettings.pidList[BIP_MAX_PIDS_PER_PROGRAM-1] = 0;
                            break;
                        }

                        BDBG_WRN(( "%s: delPid param. %d pid not Found in current Pid list.", __FUNCTION__, curPid ));
                    }
                }
            }

            /* Testing */
            {
                int i;
                for (i = 0; i<pClientSession->satelliteSettings.pidListCount && i < BIP_MAX_PIDS_PER_PROGRAM; i++) {
                    BDBG_MSG(( "pid[%d] = 0x%x of %d pids", i, pClientSession->satelliteSettings.pidList[i], pClientSession->satelliteSettings.pidListCount ));
                }
            }
        }
    }

    if (bitmaskOptionsFound & 0x00400)   // satSettings->src
    {
        pClientSession->satelliteSettings.src = pSatelliteSettings->src;
        BDBG_MSG(( "%s: new src %d", __FUNCTION__, pClientSession->satelliteSettings.src ));
    }
    if (bitmaskOptionsFound & 0x00800)   // satSettings->fe
    {
        pClientSession->satelliteSettings.fe = pSatelliteSettings->fe;
        BDBG_MSG(( "%s: new fe %d", __FUNCTION__, pClientSession->satelliteSettings.fe ));
    }
    if (bitmaskOptionsFound & 0x01000)   // transport->streamingMode
    {
        pClientSession->transportStatus.streamingMode = pTransportStatus->streamingMode;
        BDBG_MSG(( "%s: new streamingMode %d", __FUNCTION__, pClientSession->transportStatus.streamingMode ));
    }
    if (bitmaskOptionsFound & 0x02000)   // transport->isMulticast
    {
        pClientSession->transportStatus.isMulticast = pTransportStatus->isMulticast;
        BDBG_MSG(( "%s: new isMulticast %d", __FUNCTION__, pClientSession->transportStatus.isMulticast ));
    }
    if (bitmaskOptionsFound & 0x04000)   // ???
    {
    }
    if (bitmaskOptionsFound & 0x08000)   // transport->clientAddressStr
    {
        /* Free old malloc of old address as we are updating to new address */
        if (pClientSession->transportStatus.clientAddressStr)
        {
            BKNI_Free( pClientSession->transportStatus.clientAddressStr );
            pClientSession->transportStatus.clientAddressStr = NULL;
        }

        pClientSession->transportStatus.clientAddressStr = (char *) BKNI_Malloc( strlen( pTransportStatus->clientAddressStr ) + 1 );
        BIP_CHECK_PTR_GOTO( pClientSession->transportStatus.clientAddressStr, "BKNI_Malloc() for pClientSession->transportStatus.clientAddressStr failed",
            error_ipaddr, BIP_ERR_OUT_OF_SYSTEM_MEMORY );
        BKNI_Memset( pClientSession->transportStatus.clientAddressStr, 0, strlen( pTransportStatus->clientAddressStr ) + 1 );
        strncpy( pClientSession->transportStatus.clientAddressStr, pTransportStatus->clientAddressStr, strlen( pTransportStatus->clientAddressStr ));
error_ipaddr:
        BDBG_ERR(( "%s: new clientAddressStr (%p) (%s); len (%u); orig len (%u)", __FUNCTION__, pClientSession->transportStatus.clientAddressStr,
                   pClientSession->transportStatus.clientAddressStr, (unsigned)strlen( pClientSession->transportStatus.clientAddressStr ),
                   (unsigned)strlen( pTransportStatus->clientAddressStr )));
    }
    if (bitmaskOptionsFound & 0x10000)   // transport->destinationTTL
    {
        pClientSession->transportStatus.clientTTL = pTransportStatus->clientTTL;
        BDBG_MSG(( "%s: new clientTTL %c", __FUNCTION__, pClientSession->transportStatus.clientTTL ));
    }
    if (bitmaskOptionsFound & 0x20000)   // transport->clientRTPPortNum and clientRTCPPortNum
    {
        pClientSession->transportStatus.clientRTPPortNum = pTransportStatus->clientRTPPortNum;
        BDBG_MSG(( "%s: new clientRTPPortNum %d-%d", __FUNCTION__, pClientSession->transportStatus.clientRTPPortNum, pClientSession->transportStatus.clientRTPPortNum+1 ));
    }
error:
    return( 0 );
} // updateSettings

/**
 *  Function: This function is the callback function that is used to wrap the C++ function so that we can call
 *  the function from scheduleDelayedTask() API.
 **/
void BIP_RtspServer::BIP_RtspClientSession::getRtpStatistics(
    void *context
    )
{
    BIP_RtspClientSession *pSession = (BIP_RtspClientSession *)context;

    BIP_CHECK_PTR_GOTO( pSession, "pSession is NULL", error, BIP_ERR_INVALID_PARAMETER );

    BIP_CHECK_PTR_GOTO( pSession->fOurServer->fGetRtpStatisticsCallback.callback, "fGetRtpStatisticsCallback.callback is NULL", error, BIP_ERR_INVALID_PARAMETER );

#if 0
    BDBG_ERR(( "%s: calling fOurServer->callback: streamId (%d)", __FUNCTION__, pSession->satelliteSettings.streamId ));
    pSession->fOurServer->fGetRtpStatisticsCallback.callback( pSession->fOurServer->fGetRtpStatisticsCallback.context,
        pSession->satelliteSettings.streamId );
#endif

error:
    return;
} // getRtpStatistics

/**
 *  Function: This function will walk through the linked list trying to match the user's streamId with one
 *  already in the linked list.
 **/
BIP_RtspServer::BIP_RtspClientSession *BIP_RtspServer::BIP_RtspClientSession::BIP_Rtsp_FindOwnerByStreamId(
    int streamId
    )
{
    char                   streamIdStr[32];
    BIP_RtspClientSession *pSession = NULL;

    snprintf( streamIdStr, sizeof( streamIdStr ), STREAMID_FORMAT, streamId );
    BDBG_MSG(( "%s: streamId %d; str (%s); fOurStreamId %d", __FUNCTION__, streamId, streamIdStr, fOurStreamId ));
    BIP_CHECK_PTR_GOTO( fOurServer, "fOurServer is null", no_streamids, BIP_ERR_INVALID_PARAMETER );
    /* if something has been added to the hashtable */
    BDBG_MSG(( "%s: fOurServer %p", __FUNCTION__, (void *)fOurServer ));
    BIP_CHECK_PTR_GOTO( fOurServer->fOurStreamIds, "fOurStreamIds is null", no_streamids, BIP_ERR_INVALID_PARAMETER );
    BDBG_MSG(( "%s: fOurServer->fOurStreamIds %p", __FUNCTION__, (void *)fOurServer->fOurStreamIds ));
    pSession = (BIP_RtspServer::BIP_RtspClientSession *)( fOurServer->fOurStreamIds->Lookup( streamIdStr ));
no_streamids:

    BDBG_MSG(( "%s: returning %p", __FUNCTION__, (void *)pSession ));
    return( pSession );
} // BIP_Rtsp_FindOwnerByStreamId

/**
 *  Function: This function will walk through the linked list trying to match the user's sessionId with one
 *  already in the linked list.
 **/
BIP_RtspServer::BIP_RtspClientSession *BIP_RtspServer::BIP_Rtsp_FindSessionBySessionId(
    u_int32_t sessionId
    )
{
    char                  *sessionIdStr = NULL;
    BIP_RtspClientSession *pSession     = NULL;

    BIP_CHECK_PTR_GOTO( fClientSessions, "fClientSessions is invalid", error, BIP_ERR_INVALID_PARAMETER );

    sessionIdStr = BIP_Rtsp_CreateSessionIdStr( sessionId );

    /* if something has been added to the hashtable */
    pSession = (BIP_RtspServer::BIP_RtspClientSession *)( fClientSessions->Lookup( sessionIdStr ));

    BIP_CHECK_PTR_GOTO( sessionIdStr, "sessionIdStr is null", no_sessionid, BIP_ERR_INVALID_PARAMETER );
    BKNI_Free( sessionIdStr );
no_sessionid:

error:
    BDBG_MSG(( "%s: returning %p; (%08lX)", __FUNCTION__, (void *)pSession, ( pSession ) ? pSession->satelliteSettings.owner : 0 ));
    return( pSession );
} // BIP_Rtsp_FindSessionBySessionId

/**
 *  Function: This function will walk through the linked list trying to match the specified octet with
 *  one already in the linked list.
 **/
BIP_RtspServer::BIP_RtspClientSession *BIP_RtspServer::BIP_RtspClientSession::BIP_Rtsp_FindSessionByMulticastOctet(
    unsigned char octet
    )
{
    char                   ipAddressStr[32];
    BIP_RtspClientSession *pSession = NULL;

    snprintf( ipAddressStr, sizeof( ipAddressStr ), "239.1.1.%d", octet );

    /* if something has been added to the hashtable */
    BIP_CHECK_PTR_GOTO( fOurServer->fClientAddresses, "fClientSessions is invalid", no_addresses, BIP_ERR_INVALID_PARAMETER );
    BDBG_MSG(( "%s: fOurServer->fClientAddresses %p", __FUNCTION__, (void *)fOurServer->fClientAddresses ));
    pSession = (BIP_RtspServer::BIP_RtspClientSession *)( fOurServer->fClientAddresses->Lookup( ipAddressStr ));
no_addresses:

    return( pSession );
} // BIP_Rtsp_FindSessionByMulticastOctet

/**
 *  Function: This function will compute the next usable IP address octet. The computed octet will be added to the
 *  multicast address template given to us by the app ... usually 239.<deviceid>.1.
 **/
static unsigned char gMulticastNextOctet = 0;
unsigned char BIP_RtspServer::BIP_RtspClientSession:: BIP_Rtsp_ComputeNextMulticastOctet(
    void
    )
{
    unsigned char          idx      = 0;
    BIP_RtspClientSession *pSession = NULL;

    BKNI_EnterCriticalSection();
    do {
        idx++;
        gMulticastNextOctet++;
        /* if the value just wrapped around to zero */
        if (gMulticastNextOctet>=250)
        {
            gMulticastNextOctet = 1;
        }

        /* check to make sure this octet is not being used by any other stream */
        pSession = BIP_Rtsp_FindSessionByMulticastOctet( gMulticastNextOctet );
    } while (pSession != NULL && idx<32 /* safety check to catch infinite loop */);
    BKNI_LeaveCriticalSection();

    BDBG_MSG(( "%s: returning %d; idx %d", __FUNCTION__, gMulticastNextOctet, idx ));
    return( gMulticastNextOctet );
} // BIP_Rtsp_ComputeNextMulticastOctet

/**
 *  Function: This function will perform various sanity checks on the request. The attempt is to verify the
 *  request is valid before sending it to the App. The goal is to only send fully-validated requests to the App.
 *
 **/
Boolean BIP_RtspServer::BIP_RtspClientConnection:: requestIsValid(
    const char *sessionIdStr
    )
{
    Boolean                rc             = false; // assume request will be invalid
    long unsigned int      sessionIdInt   = 0;
    BIP_RtspClientSession *pClientSession = NULL;

    BIP_CHECK_PTR_GOTO( sessionIdStr, "sessionIdStr is invalid", error, BIP_ERR_INVALID_PARAMETER );
    sscanf( sessionIdStr, "%08lX", &sessionIdInt );

    pClientSession =  fOurServer->BIP_Rtsp_FindSessionBySessionId( sessionIdInt );
    // if user provided a sessionIdStr and it matched one already in the hash table
    if (pClientSession)
    {
    }

error:
    rc = True;
    BDBG_MSG(( "%s: returning %d", __FUNCTION__, rc ));
    return( rc );
} // requestIsValid

Boolean BIP_RtspServer::BIP_RtspClientConnection:: requestIsFromNonOwner(
    const char *sessionIdStr
    )
{
    Boolean                rc             = true; // assume will be from non-owner
    long unsigned int      sessionIdInt   = 0;
    BIP_RtspClientSession *pClientSession = NULL;

    BIP_CHECK_PTR_GOTO( sessionIdStr, "sessionIdStr is invalid", error, BIP_ERR_INVALID_PARAMETER );
    sscanf( sessionIdStr, "%08lX", &sessionIdInt );

    pClientSession =  fOurServer->BIP_Rtsp_FindSessionBySessionId( sessionIdInt );
    // if user provided a sessionIdStr and it matched one already in the hash table, this user is the owner
    if (pClientSession)
    {
        rc = false; // this is the owner
    }

error:
    BDBG_MSG(( "%s: returning %d", __FUNCTION__, rc ));
    return( rc );
} // requestIsFromNonOwner

/**
 *  Function: This function will determine the IP address from the user's socket number.
 **/
char *BIP_RtspServer::BIP_RtspClientSession:: GetIpAddressFromSocket(
    long int clientSocket
    )
{
    BIP_Status          rc;
    struct sockaddr_in peerIpAddress;
    socklen_t          peerAddrLen = sizeof( peerIpAddress );
    char              *ipstr       = (char *) BKNI_Malloc( INET6_ADDRSTRLEN );
    int                port;

    BIP_CHECK_PTR_GOTO( ipstr, "ipstr is invalid", error, BIP_ERR_INVALID_PARAMETER );
    BKNI_Memset( ipstr, 0, INET6_ADDRSTRLEN );
    /* Got a new connection request on the listener, get peer socket info */
    rc = getpeername( clientSocket, (struct sockaddr *)&peerIpAddress, (socklen_t *)&peerAddrLen );
    BIP_CHECK_GOTO(( rc==0 ), ( "getpeername failed ..." ), error, rc, rc );

    // deal with both IPv4 and IPv6:
    if (peerIpAddress.sin_family == AF_INET)
    {
        struct sockaddr_in *s = (struct sockaddr_in *)&peerIpAddress;
        port = ntohs( s->sin_port );
        inet_ntop( AF_INET, &s->sin_addr, ipstr, INET6_ADDRSTRLEN );
    }
    else   // AF_INET6
    {
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&peerIpAddress;
        port = ntohs( s->sin6_port );
        inet_ntop( AF_INET6, &s->sin6_addr, ipstr, INET6_ADDRSTRLEN );
    }

    BDBG_MSG(( "Client IP address (%s); ", ipstr ));

error:
    return( ipstr );
} // GetIpAddressFromSocket

/**
 *  Function: This function will compute the next stream id.
 **/
int BIP_RtspServer::BIP_RtspClientSession:: BIP_Rtsp_ComputeNextStreamId(
    void
    )
{
    unsigned char          idx      = 0;
    BIP_RtspClientSession *pSession = NULL;

    BDBG_MSG(( "%s: top", __FUNCTION__ ));
    BKNI_EnterCriticalSection();
    do {
        idx++;
        gStreamId++;
        /* if the value just wrapped around to zero */
        if (gStreamId==0)
        {
            gStreamId++;
        }

        /* check to make sure this streamId is not being used by any other stream */
        pSession = BIP_Rtsp_FindOwnerByStreamId( gStreamId );
    } while (pSession != NULL && idx<32 /* safety check to catch infinite loop */);
    BKNI_LeaveCriticalSection();

    BDBG_MSG(( "%s: returning new streamId %d; idx %d", __FUNCTION__, gStreamId, idx ));
    return( gStreamId );
} // BIP_Rtsp_ComputeNextStreamId

/**
 *  Function: This function will create a new string that is composed of the beginning part of the multicast
 *  address and appended with the last octet of the multicast address.
 **/
char *BIP_RtspServer::BIP_RtspClientSession:: BIP_Rtsp_CreateMulticastAddressStr(
    const char   *addrTemplate,
    unsigned char lastOctet
    )
{
    char *destMulticastAddrStr = NULL;
    int   newAddressLen        = 0;

    BIP_CHECK_PTR_GOTO( addrTemplate, "addrTemplate is invalid", error, BIP_ERR_INVALID_PARAMETER );

    newAddressLen = strlen( addrTemplate ) + 5 /* space for .123 and null character */ +20 /* just to give added insurance */;
    BDBG_MSG(( "%s: template (%s)", __FUNCTION__, addrTemplate ));
    destMulticastAddrStr = (char *) BKNI_Malloc( newAddressLen );
    BIP_CHECK_PTR_GOTO( destMulticastAddrStr, "BKNI_Malloc() for destMulticastAddrStr failed", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );

    BKNI_Memset( destMulticastAddrStr, 0, newAddressLen );
    snprintf( destMulticastAddrStr, newAddressLen-1, "%s%u", addrTemplate, lastOctet );

    BDBG_MSG(( "%s: returning (%s)", __FUNCTION__, destMulticastAddrStr ));

error:
    return( destMulticastAddrStr );
} // BIP_Rtsp_CreateMulticastAddressStr

/**
 *  Function: This function will extract the rtsp url from the specified request buffer.
 **/
char *extractUrl(
    const char *const fullRequestStr
    )
{
    char *urlTemp    = (char *) BKNI_Malloc( strlen( fullRequestStr ) + 1 );
    char *posRtsp    = NULL;
    char *posSpace   = NULL;
    char *rtspReturn = NULL;

    BIP_CHECK_PTR_GOTO( urlTemp, "urlTemp is invalid", error, BIP_ERR_INVALID_PARAMETER );

    // make a temporary copy of the URL
    BKNI_Memset( urlTemp, 0, strlen( fullRequestStr ) + 1 );
    strncpy( urlTemp, fullRequestStr, strlen( fullRequestStr ));
    BDBG_MSG(( "%s: urlTemp len %u; (%s)", __FUNCTION__, (unsigned)strlen( urlTemp ), urlTemp ));
    if (( posRtsp = strstr( urlTemp, "rtsp://" ))) /* bypass the DESCRIBE or PLAY tag */
    {
        BDBG_MSG(( "%s: posRtsp %p; (%s)", __FUNCTION__, posRtsp, posRtsp ));
        if (( posSpace = strchr( posRtsp, ' ' ))) /* find the end of the URL */
        {
            BDBG_MSG(( "%s: posSpace %p; (%s)", __FUNCTION__, posSpace, posSpace ));
            *posSpace  = '\0'; /* null-terminate the URL string */
            rtspReturn = (char *) BKNI_Malloc( strlen( posRtsp ) + 1 );
            BIP_CHECK_PTR_GOTO( rtspReturn, "rtspReturn is null", no_rtspreturn, BIP_ERR_INVALID_PARAMETER );
            BKNI_Memset( rtspReturn, 0, strlen( posRtsp )+1 );
            strncpy( rtspReturn, posRtsp, strlen( posRtsp ));
no_rtspreturn:

            BDBG_MSG(( "%s: rtspReturn %p; (%s); len %u", __FUNCTION__, rtspReturn, rtspReturn, (unsigned)strlen( posRtsp )));
        }
    }
    BKNI_Free( urlTemp );

error:
    return( rtspReturn );
} // extractUrl

/**
 *  Function: This function will format the response buffer necessary for responding to a PLAY request.
 **/
int setResponsePlay(
    char       *outputBuffer,
    int         outputBufferLen,
    const char *fRequestBuffer
    )
{
    char *rtspStart = extractUrl((char *)fRequestBuffer );

    BIP_CHECK_PTR_GOTO( outputBuffer, "outputBuffer is invalid", error, BIP_ERR_INVALID_PARAMETER );
    BIP_CHECK_ERR_LEZ_GOTO( outputBufferLen,  "outputBufferLen is invalid", "", error, BIP_ERR_INVALID_PARAMETER );
    BIP_CHECK_PTR_GOTO( fRequestBuffer, "fRequestBuffer is invalid", error, BIP_ERR_INVALID_PARAMETER );

    BDBG_MSG(( "%s: rtspStart (%s); fRequestBuffer (%s) ", __FUNCTION__, rtspStart, fRequestBuffer ));
    snprintf( outputBuffer, outputBufferLen - 1, "200 OK\r\nRTP-Info:url=%s", rtspStart );
error:

    if (rtspStart) {BKNI_Free( rtspStart ); }
    return( 0 );
}

/**
 *  Function: This function will create a session id string from a session id integer.
 **/
char *BIP_Rtsp_CreateSessionIdStr(
    unsigned long int sessionId
    )
{
    char *sessionIdStr = (char *) BKNI_Malloc( RTSP_PARAM_STRING_MAX );

    BIP_CHECK_PTR_GOTO( sessionIdStr, "malloc failed for sessionIdStr", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );
    BKNI_Memset( sessionIdStr, 0, RTSP_PARAM_STRING_MAX );
    snprintf( sessionIdStr, RTSP_PARAM_STRING_MAX-1, "%08lX", sessionId );

error:
    return( sessionIdStr );
}

void _BIP_Rtsp_ProcessIgmpMembershipReport(
    void *context
    )
{
    BIP_RtspServer *fOurServer = (BIP_RtspServer *)context;

    fOurServer->BIP_Rtsp_ProcessIgmpMembershipReport( context );
}

void BIP_RtspServer:: BIP_Rtsp_ProcessIgmpMembershipReport(
    void *context
    )
{
    BDBG_MSG(( "%s: ++++++++++++++++++++++++++++++ context %p", __FUNCTION__, context ));

    BIP_RtspServer *pOurServer   = (BIP_RtspServer *) context;
    char           *sessionIdStr = NULL;
    BIP_RtspServer::BIP_RtspClientSession    *pClientSession = NULL;
    struct sockaddr_in                        clientAddr;
    BIP_IgmpListenerHandle                    hIgmpListener = NULL;
    BIP_IgmpListener_MembershipReportStatus_T memRepStatus;
    BIP_CallbackDesc callbackDesc;

    BIP_CHECK_PTR_GOTO( context, "Context is invalid", error, BIP_ERR_INVALID_PARAMETER );

    BKNI_Memset((void *) &callbackDesc, 0, sizeof( callbackDesc ));
    hIgmpListener = pOurServer->fIgmpListener;
    BIP_CHECK_PTR_GOTO( hIgmpListener, "hIgmpListener is invalid", error, BIP_ERR_INVALID_PARAMETER );

    while (BIP_IgmpListener_GetMembershipReportStatus( hIgmpListener, &memRepStatus ) == BIP_SUCCESS)
    {
        int temp = htonl( memRepStatus.multicast_address );
        char    ipAddrBuf[INET_ADDRSTRLEN];

        inet_ntop(AF_INET, &temp, ipAddrBuf, sizeof(ipAddrBuf));
        BDBG_MSG(( "%s: BIP_IgmpListener_GetMembershipReportStatus() returned ipAddr (%s)", __FUNCTION__, ipAddrBuf));

        /* prepare to convert integer ipaddress to string */
        clientAddr.sin_addr.s_addr = htonl( memRepStatus.multicast_address );

        pClientSession = (BIP_RtspServer::BIP_RtspClientSession *)( pOurServer->fClientAddresses->Lookup( inet_ntoa( clientAddr.sin_addr )));
        if (pClientSession ==NULL)
        {
            BDBG_ERR(( "pClientSession->Lookup() Can't find valid Session corresponding to ip address (%s)", ipAddrBuf));
            return;
        }

        sessionIdStr = BIP_Rtsp_CreateSessionIdStr( pClientSession->satelliteSettings.owner );
        BDBG_MSG(( "%s: report type %d; state is (%d);  sessionIdStr (%s); pClientSession %p", __FUNCTION__,
                   memRepStatus.memRepType, pClientSession->getState(), sessionIdStr, (void *)pClientSession ));

        if (memRepStatus.memRepType == BIP_IgmpListener_MembershipReportType_eJoin)
        {
            /* Find State and possibly send callback. */
            if (pClientSession->getState() == BIP_RtspLmSessionStreamState_MulticastPlay)
            {
                BDBG_MSG(( "%s: JOIN Membership Report from IgmpListener.Ignore in Play", __FUNCTION__ ));
            }
            else if (( pClientSession->getState() == BIP_RtspLmSessionStreamState_IgmpJoinStreaming ) ||
                     ( pClientSession->getState() == BIP_RtspLmSessionStreamState_IgmpJoinPending ) ||
                     ( pClientSession->getState() == BIP_RtspLmSessionStreamState_IgmpJoinChannelChangePending )
                     )
            {
                BDBG_MSG(( "%s: JOIN Membership Report from IgmpListener. Ignore already in SETUP Streaming/StreamPending", __FUNCTION__ ));
            }
            else if (( pClientSession->getState() == BIP_RtspLmSessionStreamState_MulticastSetup ) ||
                     ( pClientSession->getState() == BIP_RtspLmSessionStreamState_IgmpLeavePending )
                     )
            {
                BDBG_MSG(( "%s: JOIN Membership Report from IgmpListener. Start Streaming, in SETUP not streaming or was in LeavePending", __FUNCTION__ ));
                /* we are SETUP, not streaming-> fire callback */
                callbackDesc = pClientSession->getIgmpMembershipReportCallback();
            }

            /* ignore these states too
                   BIP_RtspLmSessionStreamState_Idle = 0,
                   BIP_RtspLmSessionStreamState_UnicastSetupPending,
                   BIP_RtspLmSessionStreamState_UnicastSetup,
                   BIP_RtspLmSessionStreamState_UnicastPlayPending,
                   BIP_RtspLmSessionStreamState_UnicastPlay,
                  BIP_RtspLmSessionStreamState_MulticastPlayPending,
                   BIP_RtspLmSessionStreamState_MulticastPlay,
                   BIP_RtspLmSessionStreamState_UnicastChannelChangePending,
                   BIP_RtspLmSessionStreamState_MulticastChannelChangePending,
                 BIP_RtspLmSessionStreamState_TeardownPending,
                   BIP_RtspLmSessionStreamState_TimeoutPending,
             */

            /* if callback is set, trigger it */
            if (callbackDesc.callback)
            {
                BDBG_MSG(( "%s: eJoin calling sendResponse(); igmpStatus %d", __FUNCTION__, BIP_RtspIgmpMemRepStatus_eJoin ));
                pClientSession->sendResponse( BIP_RtspResponseStatus_eSuccess ); /* set state to BIP_RtspLmSessionStreamState_IgmpJoinPending */
                callbackDesc.callback( callbackDesc.context, BIP_RtspIgmpMemRepStatus_eJoin );
            }
        }
        else if (memRepStatus.memRepType == BIP_IgmpListener_MembershipReportType_eLeave)
        {
            /* Find State and possibly send callback. */
            if (pClientSession->getState() == BIP_RtspLmSessionStreamState_MulticastPlay)
            {
                BDBG_MSG(( "%s: LEAVE Membership Report from IgmpListener. Ignore in MulitcastPlay", __FUNCTION__ ));
            }
            else if (( pClientSession->getState() == BIP_RtspLmSessionStreamState_IgmpJoinStreaming ) ||
                     ( pClientSession->getState() == BIP_RtspLmSessionStreamState_IgmpJoinPending ) ||
                     ( pClientSession->getState() == BIP_RtspLmSessionStreamState_IgmpJoinChannelChangePending )
                     )
            {
                /* we are JOIN,streaming -> fire callback */
                BDBG_MSG(( "%s: LEAVE Membership Report from IgmpListener. Stop.  In Setup Streaming/StreamPending", __FUNCTION__ ));
                callbackDesc = pClientSession->getIgmpMembershipReportCallback();
            }
            else if (( pClientSession->getState() == BIP_RtspLmSessionStreamState_MulticastSetup ) ||
                     ( pClientSession->getState() == BIP_RtspLmSessionStreamState_IgmpLeavePending )
                     )
            {
                BDBG_MSG(( "%s: LEAVE Membership Report from IgmpListener. Ignore in Setup Not Streaming or Leave Pending", __FUNCTION__ ));
            }

            /* ignore these states too
                   BIP_RtspLmSessionStreamState_Idle = 0,
                   BIP_RtspLmSessionStreamState_UnicastSetupPending,
                   BIP_RtspLmSessionStreamState_UnicastSetup,
                   BIP_RtspLmSessionStreamState_UnicastPlayPending,
                   BIP_RtspLmSessionStreamState_UnicastPlay,
                  BIP_RtspLmSessionStreamState_MulticastPlayPending,
                   BIP_RtspLmSessionStreamState_MulticastPlay,
                   BIP_RtspLmSessionStreamState_UnicastChannelChangePending,
                   BIP_RtspLmSessionStreamState_MulticastChannelChangePending,
                 BIP_RtspLmSessionStreamState_TeardownPending,
                   BIP_RtspLmSessionStreamState_TimeoutPending,
             */

            /* if callback is set, trigger it */
            if (callbackDesc.callback)
            {
                BDBG_MSG(( "%s: eLeave calling sendResponse(); igmpStatus(%d)", __FUNCTION__, BIP_RtspIgmpMemRepStatus_eLeave ));
                pClientSession->sendResponse( BIP_RtspResponseStatus_eSuccess ); /* set state to BIP_RtspLmSessionStreamState_IgmpLeavePending */
                callbackDesc.callback( callbackDesc.context, BIP_RtspIgmpMemRepStatus_eLeave );
            }
        }
        else
        {
            BDBG_ERR(( "%s: Unknown MembershipType received from IgmpListener. Ignore", __FUNCTION__ ));
        }
    }
    BDBG_MSG(( "%s: Done processing IGMP Listener Report Statuses", __FUNCTION__ ));

error:
    if (sessionIdStr) {BKNI_Free( sessionIdStr ); }

    return;
} // BIP_Rtsp_ProcessIgmpMembershipReport

EventTriggerId BIP_RtspServer::eventMembershipReportTriggerId = 0;
static void BIP_Rtsp_IgmpMembershipReportCallback(
    void *context,
    int   param
    )
{
    BSTD_UNUSED( param );
    BIP_RtspServer *fOurServer = (BIP_RtspServer *)context;
    fOurServer->envir().taskScheduler().triggerEvent( BIP_RtspServer::eventMembershipReportTriggerId, fOurServer );
}

void BIP_RtspServer::setIgmpCallbacks()
{
    /* Add  Callbacks for IGMP here*/
    BIP_IgmpListenerSettings igmpListenerSettings;

    BIP_IgmpListener_GetSettings( fIgmpListener, &igmpListenerSettings );
    igmpListenerSettings.membershipReportCallback.callback = BIP_Rtsp_IgmpMembershipReportCallback;
    igmpListenerSettings.membershipReportCallback.context  = this;
    BIP_IgmpListener_SetSettings( fIgmpListener, &igmpListenerSettings );
}

BIP_RtspServer *BIP_RtspServer::createNew(
    UsageEnvironment           &env,
    Port                        rtspPort,
    UserAuthenticationDatabase *authDatabase,
    unsigned                    reclamationTestSeconds,
    char                       *multicastAddressTemplate
    )
{
    int ourSocket = setUpOurSocket( env, rtspPort );

    if (ourSocket == -1) {return( NULL ); }

    return( new BIP_RtspServer( env, ourSocket, rtspPort, authDatabase, reclamationTestSeconds, multicastAddressTemplate ));
}

/**
 *  Function: This function is the main constructor for the BIP_RtspServer class.
 **/
BIP_RtspServer
::BIP_RtspServer(
    UsageEnvironment           &env,
    int                         ourSocket,
    Port                        rtspPort,
    UserAuthenticationDatabase *authDatabase,
    unsigned                    reclamationTestSeconds,
    char                       *multicastAddressTemplate
    )
    : RTSPServer( env, ourSocket, rtspPort, authDatabase, 0 /* do not send reclamationTestSeconds to RTSPServer.
    It causes its liveness functions to timeout. We want our derived class's liveness functions to timeout. */),
    fOurStreamIds( HashTable::create( STRING_HASH_KEYS )),
    fClientSessions( HashTable::create( STRING_HASH_KEYS )),
    fClientAddresses( HashTable::create( STRING_HASH_KEYS )),
    fReclamationTestSeconds( reclamationTestSeconds )
{
    fClientConnection = NULL;
    fOurMulticastAddressTemplate =NULL;
    fIgmpListener = NULL;
    fConnectedCallback.callback = NULL;
    fConnectedCallback.context = NULL;
    fConnectedCallback.param = 0;
    fGetRtpStatisticsCallback.callback = NULL;
    fGetRtpStatisticsCallback.context = NULL;
    fGetRtpStatisticsCallback.param = 0;

    BDBG_MSG(( "%s: constructor top; BIP::fReclamationTestSeconds %d", __FUNCTION__, fReclamationTestSeconds ));
    BIP_CHECK_PTR_GOTO( multicastAddressTemplate, "multicastAddressTemplate is NULL", error_template, BIP_ERR_INVALID_PARAMETER );

    fOurMulticastAddressTemplate = (char *) BKNI_Malloc( strlen( multicastAddressTemplate ) +1 );
    BIP_CHECK_PTR_GOTO( fOurMulticastAddressTemplate, "Memory Allocation Failed for fOurMulticastAddressTemplate", error_malloc, BIP_ERR_OUT_OF_SYSTEM_MEMORY );
    BKNI_Memset( fOurMulticastAddressTemplate, 0, strlen( multicastAddressTemplate ) +1 );
    strncpy( fOurMulticastAddressTemplate, multicastAddressTemplate, strlen( multicastAddressTemplate ));
    BDBG_MSG(( "%s; BKNI_Malloc(len(%s)); len %u; returned %p", __FUNCTION__, multicastAddressTemplate, (unsigned)strlen( multicastAddressTemplate ), fOurMulticastAddressTemplate ));
error_malloc:
error_template:

    if (eventMembershipReportTriggerId == 0)
    {
        eventMembershipReportTriggerId = envir().taskScheduler().createEventTrigger( _BIP_Rtsp_ProcessIgmpMembershipReport );
    }
    BDBG_MSG(( "%s: multicastAddressTemplate %s, this %p", __FUNCTION__, fOurMulticastAddressTemplate, (void *)this ));
}

/**
 *  Function: This function is the main destructor for the BIP_RtspServer class.
 **/
BIP_RtspServer::~BIP_RtspServer()
{
    HashTable::Iterator   *iter = HashTable::Iterator::create( *fClientSessions );
    BIP_RtspClientSession *pSession;
    char const            *key; // dummy

    BDBG_MSG(( "%s: destructor %p", __FUNCTION__, (void *)this ));
    while (( pSession = (BIP_RtspServer::BIP_RtspClientSession *)( iter->next( key ))) != NULL) {
        delete pSession;
    }
    delete iter;

    if (fOurMulticastAddressTemplate)
    {
        BKNI_Free( fOurMulticastAddressTemplate );
    }
}

BIP_RtspServer::BIP_RtspClientConnection *BIP_RtspServer::createNewClientConnectionFull(
    int clientSocket
    )
{
    BIP_RtspServer::BIP_RtspClientConnection *clientConnection = NULL;
    struct sockaddr_in clientAddr;

    BKNI_Memset((void *) &clientAddr, 0, sizeof( clientAddr ));

    BDBG_MSG(( "%s: clientSocket %d; calling new BIP_RtspClientConnection()", __FUNCTION__, clientSocket ));
    clientConnection = new BIP_RtspClientConnection( *this, clientSocket, clientAddr );
    BDBG_MSG(( "%s: clientSocket %d: IP addr %s, this %p", __FUNCTION__, clientSocket, inet_ntoa(clientConnection->fClientInputAddr.sin_addr), (void *)this ));
    BDBG_MSG(( "%s: this %p, clientConnection %p", __FUNCTION__, (void *)this, (void *)clientConnection ));
    this->fClientConnection = (BIP_RtspServer::BIP_RtspClientConnection *)clientConnection;
    return( clientConnection );
} // createNewClientConnectionFull

/**
 *  Function: This function will create a new client connection
 **/
BIP_RtspServer::BIP_RtspClientConnection *BIP_RtspServer::createNewClientConnection(
    int                clientSocket,
    struct sockaddr_in clientAddr
    )
{
    BSTD_UNUSED( clientAddr );

    BDBG_MSG(( "%s: socket %d; clientAddr (%s); Server (%p)", __FUNCTION__, clientSocket, inet_ntoa( clientAddr.sin_addr ), (void *)this ));
    if (this->fConnectedCallback.callback)
    {
        this->fConnectedCallback.callback( this->fConnectedCallback.context, clientSocket );
    }

    BDBG_MSG(( "%s: DONE for socket %d; clientAddr (%s)", __FUNCTION__, clientSocket, inet_ntoa( clientAddr.sin_addr )));
    /* TODO: this is not ideal but works for now as the caller doesn't check the return code */
    return( NULL );
}

/**
 *  Function: This function is the main constructor for the BIP_RtspServer::BIP_RtspClientConnection class.
 **/
BIP_RtspServer::BIP_RtspClientConnection
::BIP_RtspClientConnection(
    BIP_RtspServer    &ourServer,
    int                clientSocket,
    struct sockaddr_in clientAddr
    )
    : RTSPClientConnection( ourServer, clientSocket, clientAddr ),
    fClientSessionId( 0 ),
    fClientInputSocket( clientSocket ),
    fClientInputAddr( clientAddr ),
    fOurServer((BIP_RtspServer *)&ourServer ),
    fLivenessCheckTask( 0 ),
    fIsTimingOut( False )
{
    BDBG_MSG(( "%s: constructor top (%p)", __FUNCTION__, (void *)this ));
    fIsActive = True;
    BKNI_Memset( &fClientInputAddr, 0, sizeof( fClientInputAddr ));

    noteLiveness();

    BDBG_MSG(( "%s: clientSocket %d: IP addr %s, this %p; fOurServer %p", __FUNCTION__, clientSocket, inet_ntoa( clientAddr.sin_addr ), (void *)this, (void *)fOurServer ));
    resetRequestBuffer();
    if (fClientInputSocket > 0)
    {
        BDBG_MSG(( "%s: fClientInputSocket setBackgroundHandling(%d)", __FUNCTION__, fClientInputSocket ));
        // Arrange to handle incoming requests:
        envir().taskScheduler().setBackgroundHandling( fClientInputSocket, SOCKET_READABLE|SOCKET_EXCEPTION,
            (TaskScheduler::BackgroundHandlerProc *)&incomingRequestHandler, this );
    }
    BDBG_MSG(( "%s: constructor done", __FUNCTION__ ));
}

/**
 *  Function: This function will reset the connection's request buffer back to empty state.
 **/
void BIP_RtspServer::BIP_RtspClientConnection::resetRequestBuffer()
{
    fRequestBytesAlreadySeen = 0;
    fRequestBufferBytesLeft  = sizeof fRequestBuffer;
    fLastCRLF                = &fRequestBuffer[-3]; // hack: Ensures that we don't think we have end-of-msg if the data starts with <CR><LF>
}

/**
 *  Function: This function will set the connection's request buffer to the specified string value.
 **/
void BIP_RtspServer::BIP_RtspClientConnection::setRequestBuffer(
    const char *newContents
    )
{
    resetRequestBuffer();
    strncpy((char *)fRequestBuffer, newContents, sizeof( fRequestBuffer )-1 );
    fLastCRLF = &fRequestBuffer[0] + strlen( newContents );
    strncat((char *)fRequestBuffer, "\r\n", sizeof( fRequestBuffer )-1 );
    fRequestBufferBytesLeft = sizeof fRequestBuffer - strlen((char *)fRequestBuffer );
}

void BIP_RtspServer::BIP_RtspClientConnection::incomingRequestHandler(
    void *instance,
    int /*mask*/
    )
{
    BIP_RtspClientConnection *session = (BIP_RtspClientConnection *)instance;
    //session->incomingRequestHandler1();
    struct sockaddr_in dummy; // 'from' address, meaningless in this case

    BDBG_MSG(( "%s: socketFd %d, alreadySeen %d; buffer %p !!!!!!!!!!!!!!!!!!!!!!", __FUNCTION__, session->fClientInputSocket, session->fRequestBytesAlreadySeen, &session->fRequestBuffer[session->fRequestBytesAlreadySeen] ));
    int bytesRead = readSocket( session->envir(), session->fClientInputSocket, &session->fRequestBuffer[session->fRequestBytesAlreadySeen], session->fRequestBufferBytesLeft, dummy );
    BDBG_MSG(( "%s: Derived RTSPServer:============================================================================", __FUNCTION__ ));
    BDBG_MSG(( "BIP_RtspServer:%s: BIP_RtspClientSession; bytesRead (%d); fRequestBufferBytesLeft (%u)",
               __FUNCTION__, bytesRead, session->fRequestBufferBytesLeft ));
    session->handleRequestBytes( bytesRead );
} // incomingRequestHandler

/**
 *  Function: This function is the destructor for the BIP_RtspClientConnection class.
 **/
BIP_RtspServer::BIP_RtspClientConnection::~BIP_RtspClientConnection()
{
    HashTable::Iterator   *iter     = HashTable::Iterator::create( *fOurServer->fClientSessions );
    BIP_RtspClientSession *pSession = NULL;
    char const            *key; // dummy

    BDBG_MSG(( "%s: destructor %p; fLivenessCheckTask %p; fIsTimingOut %d; fIsActive %d", __FUNCTION__, (void *)this, fLivenessCheckTask, fIsTimingOut, fIsActive ));

    if (fLivenessCheckTask)
    {
        // Turn off any liveness checking:
        BDBG_MSG(( "%s: unscheduleDelayedTask(fLivenessCheckTask(%p)\n", __FUNCTION__, fLivenessCheckTask ));
        envir().taskScheduler().unscheduleDelayedTask( fLivenessCheckTask );
        fLivenessCheckTask = 0;
    }

    BDBG_MSG(( "%s: inputSock %d; outSocket %d", __FUNCTION__, fClientInputSocket, fClientOutputSocket ));
    if (fClientInputSocket>0) {::closeSocket( fClientInputSocket ); }
    if (fClientOutputSocket>0) {::closeSocket( fClientOutputSocket ); }

    while (( pSession = (BIP_RtspServer::BIP_RtspClientSession *)( iter->next( key ))) != NULL) {
        BDBG_MSG(( "%s: this %p: for session %p: (%08X); connection (%p)", __FUNCTION__, (void *)this, (void *)pSession, pSession->fSessionId, (void *)pSession->fClientConnection ));
        // if this connection is associated with any active session, break the association
        if (pSession->fClientConnection == this)
        {
            pSession->fClientConnection = NULL;
        }
    }
    delete iter;

    fIsActive = False;
}

/**
 *  Function: This function will handle every new connection request that comes in.
 **/
void BIP_RtspServer::BIP_RtspClientConnection::handleRequestBytes(
    int newBytesRead
    )
{
    BIP_Status rc = BIP_SUCCESS;
    int       send_rc = 0;
    int     numBytesRemaining = 0;
    Boolean bSendResponse     = true;

    BDBG_MSG(( "BIP_RtspClientConnection::%s: newBytesRead %d; fClientConnection %p; fIsTimingOut %d", __FUNCTION__, newBytesRead, (void *)this, fIsTimingOut ));

    ++fRecursionCount;

    do
    {
        BIP_RtspServer::BIP_RtspClientSession *clientSession = NULL;
        char streamIdStr[32];

        if (( newBytesRead < 0 ) || ((unsigned)newBytesRead >= fRequestBufferBytesLeft ))
        {
            // During session timeout processing, a temporary connection object is created along with a temporary socket.
            // If we get a read error (-1) from the temporary socket while we are processing the timeout, we simply want
            // to close the socket that got a read error on.
            if (fIsTimingOut)
            {
                BDBG_MSG(( "%s: clientConnection %p: is timing out", __FUNCTION__, (void *)this ));
                fIsActive = True;
                closeSockets();
                break;
            }

            // Either the client socket has died, or the request was too big for us.
            // Terminate this connection:
            BDBG_MSG(( "%s: BIP_RtspClientConnection %p: read %d new bytes (of %d); terminating connection!",
                       __FUNCTION__, (void *)this, newBytesRead, fRequestBufferBytesLeft ));
            fIsActive = False;
            if (this->fErrorCallback.callback)
            {
                this->fErrorCallback.callback( this->fErrorCallback.context, errno );
            }
            break;
        }

        Boolean        endOfMsg = False;
        unsigned char *ptr      = &fRequestBuffer[fRequestBytesAlreadySeen];
        ptr[newBytesRead] = '\0';
        BDBG_MSG(( "%s: BIP_RtspClientConnection %p: %s %d new bytes", __FUNCTION__, (void *)this, numBytesRemaining > 0 ? "processing" : "read", newBytesRead ));
        BDBG_REQSEND(( "BIP_RtspClientConnection::%s: REQ (%s)", __FUNCTION__, ptr )); // uses printf because buffer can be longer than 256 chars

        noteLiveness();

        // Look for the end of the message: <CR><LF><CR><LF>
        unsigned char *tmpPtr = fLastCRLF + 2;
        if (tmpPtr < fRequestBuffer) {tmpPtr = fRequestBuffer; }
        while (tmpPtr < &ptr[newBytesRead-1]) {
            if (( *tmpPtr == '\r' ) && ( *( tmpPtr+1 ) == '\n' ))
            {
                if (tmpPtr - fLastCRLF == 2)   // This is it:
                {
                    endOfMsg = True;
                    break;
                }
                fLastCRLF = tmpPtr;
            }
            ++tmpPtr;
        }

        fRequestBufferBytesLeft  -= newBytesRead;
        fRequestBytesAlreadySeen += newBytesRead;

        if (!endOfMsg)
        {
            break; // subsequent reads will be needed to complete the request
        }
        fRequestBuffer[fRequestBytesAlreadySeen] = '\0';
        char              cmdName[RTSP_PARAM_STRING_MAX];
        char              urlPreSuffix[RTSP_PARAM_STRING_MAX];
        char              urlSuffix[RTSP_PARAM_STRING_MAX];
        char              cseq[RTSP_PARAM_STRING_MAX];
        char              sessionIdStr[RTSP_PARAM_STRING_MAX];
        long unsigned int sessionIdInt  = 0;
        unsigned          contentLength = 0;
        fLastCRLF[2] = '\0'; // temporarily, for parsing

        BKNI_Memset( cmdName, 0, sizeof( cmdName ));
        BKNI_Memset( urlPreSuffix, 0, sizeof( urlPreSuffix ));
        BKNI_Memset( urlSuffix, 0, sizeof( urlSuffix ));
        BKNI_Memset( cseq, 0, sizeof( cseq ));
        BKNI_Memset( sessionIdStr, 0, sizeof( sessionIdStr ));

        // Parse the request string into command name and 'CSeq', then handle the command:
        /* look for Session: ... CSeq: ... Content-length: ... */
        Boolean parseSucceeded = parseRTSPRequestString((char *)fRequestBuffer, fLastCRLF+2 - fRequestBuffer,
                cmdName, sizeof cmdName,
                urlPreSuffix, sizeof urlPreSuffix,
                urlSuffix, sizeof urlSuffix,
                cseq, sizeof cseq,
                sessionIdStr, sizeof sessionIdStr,
                contentLength );
        fLastCRLF[2] = '\r'; // restore its value
        if (parseSucceeded)
        {
            BDBG_MSG(( "%s: parseRTSPRequestString() succeeded; got cmdName (%s), urlPreSuffix (%s), Length %u, with %d bytes following the message.",
                       __FUNCTION__, cmdName, urlPreSuffix, contentLength, (int)(ptr + newBytesRead - ( tmpPtr + 2 ))));
            BDBG_MSG(( "%s: urlSuffix (%s), CSeq (%s)", __FUNCTION__, urlSuffix, cseq ));
            // If there was a "Content-Length:" header, then make sure we've received all of the data that it specified:
            if (ptr + newBytesRead < tmpPtr + 2 + contentLength)
            {
                break; // we still need more data; subsequent reads will give it to us
            }
            // We now have a complete RTSP request.
            // Handle the specified command (beginning by checking those that don't require session ids):
            fCurrentCSeq = cseq;

            // if client provided a session id string, convert it to an integer
            if (sessionIdStr[0] != '\0')
            {
                sscanf( sessionIdStr, "%08lX", &sessionIdInt );
            }

            BDBG_MSG(( "%s: switch (%s); socket %d", __FUNCTION__, cmdName, fClientInputSocket ));
            if (strcmp( cmdName, "OPTIONS" ) == 0)
            {
                /* TODO: if we have a need to expose OPTIONS command to user application, then we will need to change the logic here */

                noteLiveness( sessionIdStr ); /* this is a "connection" liveness ... not a "session" liveness. */

                /* if the request contains a session id string, use it in the OPTIONS response */
                if (sessionIdStr[0] != '\0')
                {
                    fClientSessionId = sessionIdInt;
                }
                handleCmd_OPTIONS();
            }
            else if (( urlPreSuffix[0] == '\0' ) && ( urlSuffix[0] == '*' ) && ( urlSuffix[1] == '\0' ))
            {
                /* TODO: need this for the generic RTSP Server case? */
                // The special "*" URL means: an operation on the entire server.  This works only for GET_PARAMETER and SET_PARAMETER:
                if (strcmp( cmdName, "GET_PARAMETER" ) == 0)
                {
                    handleCmd_GET_PARAMETER((char const *)fRequestBuffer );
                }
                else if (strcmp( cmdName, "SET_PARAMETER" ) == 0)
                {
                    handleCmd_SET_PARAMETER((char const *)fRequestBuffer );
                }
                else
                {
                    handleCmd_notSupported();
                }
            }
            else if (strcmp( cmdName, "DESCRIBE" ) == 0)
            {
                /* TODO: if we have a need to expose DESCRIBE command to user application, then we will need to change the logic here */
                handleCmd_DESCRIBE( urlPreSuffix, urlSuffix, (char const *)fRequestBuffer );
                BDBG_MSG(( "%s: after DESCRIBE, bSendResponse %u", __FUNCTION__, bSendResponse ));
            }
            else if (strcmp( cmdName, "SETUP" ) == 0)
            {
                int bitmaskOptionsFound = 0;
                BIP_RtspLiveMediaSessionSatelliteSettings satelliteSettings;
                BIP_RtspTransportStatus                   transportStatus;
                BIP_RtspLmSessionStreamState              stateOnEntry = BIP_RtspLmSessionStreamState_Idle;

                BKNI_Memset( &satelliteSettings, 0, sizeof( satelliteSettings ));
                BKNI_Memset( &transportStatus, 0, sizeof( transportStatus ));

                /* VLC and ip_client split the suffix and pre-suffix differently than SATIP Tool */
                if (strncmp( urlSuffix, "track", 5 ) == 0)
                {
                    bitmaskOptionsFound = BIP_Rtsp_ParseUrlOptions( &satelliteSettings, &transportStatus, urlPreSuffix );
                }
                else
                {
                    bitmaskOptionsFound = BIP_Rtsp_ParseUrlOptions( &satelliteSettings, &transportStatus, (char const *)fRequestBuffer );
                }

                BDBG_MSG(( "%s: case %s; sessionIdStr (%s);  streamId %d; urlSuffix %s", __FUNCTION__, cmdName, sessionIdStr,
                            satelliteSettings.streamId, urlSuffix ));

                /* if an error was detected while parsing the URL */
                if (( bitmaskOptionsFound == -1 ) || ( requestIsValid( sessionIdStr ) == False ))
                {
                    handleCmd_bad();
                }
                /* A SETUP request can arrive in several ways: one with a session already active (2), and one that will cause a session to be created (1)
                   (1) SETUP rtsp://10.14.237.65:554/?src=1&freq=1119&pol=h&ro=0.35&msys=dvbs&mtype=qpsk&plts=off&sr=22000&fec=56&pids=0,256,257,260
                   (2) SETUP rtsp://10.14.237.65:554/stream=1?src=1&freq=1222&pol=h&ro=0.35&msys=dvbs&mtype=qpsk&plts=off&sr=22000&fec=56&pids=0,256,257,260
                   (3) a client can "copy" an existing session by specifying the stream= parameter along with a unicast request
                   (4) a SETUP request comes in while in PLAY state ... treat like a PLAY request ... channel change
                */
                /* No Session Id and no Stream Id.  Start a new client session */
                else if ((( sessionIdStr[0] == '\0' ) &&  ( satelliteSettings.streamId == 0 )))
                {
                    /* Invoke the messageReceived callback on the BIP_RtspClientConnection object */
                    /* This allows app to create a new BIP_RtspSession object */
                    if (this->fMessageReceivedCallback.callback)
                    {
                        BDBG_MSG(( "%s: triggering CALLBACK ... this->fMessageReceivedCallback.context(%s)", __FUNCTION__, fRequestBuffer ));
                        this->fMessageReceivedCallback.callback( this->fMessageReceivedCallback.context, fLastCRLF+2 - fRequestBuffer );
                        bSendResponse = false; /* the response will be sent after the APP has processed the request */
                    }
                    else
                    {
                        handleCmd_bad();
                    }
                }
                else if (( requestIsFromNonOwner( sessionIdStr ) && ( satelliteSettings.streamId > 0 )))  /* Joining existing stream as Non-Owner*/
                {
                    snprintf( streamIdStr, sizeof( streamIdStr ), STREAMID_FORMAT, satelliteSettings.streamId );
                    clientSession = (BIP_RtspServer::BIP_RtspClientSession *)( fOurServer->fOurStreamIds->Lookup( streamIdStr ));
                    if (clientSession ==NULL)
                    {
                        BDBG_WRN(( "%s: Non-owner can't match streamID with a clientSession(%p)", __FUNCTION__, (void *)clientSession ));
                        handleCmd_bad();
                    }
                    else if (!clientSession->transportStatus.isMulticast && transportStatus.isMulticast)
                    {
                        BDBG_WRN(( "%s: Non-owner wants Multicast when Owner is Unicast. Not Allowed", __FUNCTION__ ));
                        handleCmd_bad();
                    }
                    else if (this->fMessageReceivedCallback.callback)
                    {
                        BDBG_MSG(( "%s: triggering CALLBACK ... this->fMessageReceivedCallback.context(%s)", __FUNCTION__, fRequestBuffer ));
                        this->fMessageReceivedCallback.callback( this->fMessageReceivedCallback.context, fLastCRLF+2 - fRequestBuffer );
                        bSendResponse = false; /* the response will be sent after the APP has processed the request */
                    }
                    else
                    {
                        handleCmd_bad();
                    }
                }
                else
                {
                    // The request included a session id.  Make sure it's one that we have already set up:
                    clientSession = (BIP_RtspServer::BIP_RtspClientSession *)( fOurServer->fClientSessions->Lookup( sessionIdStr ));
                    if (clientSession == NULL)
                    {
                        handleCmd_sessionNotFound();
                    }
                    else
                    {
                        clientSession->noteLiveness();

                        stateOnEntry = clientSession->getState(); // used to determine if we got here after JOIN was received

                        BDBG_MSG(( "%s: req multicast (%s); session multicast (%s); clientAddressStr (%s)", __FUNCTION__,
                                   ( transportStatus.isMulticast ) ? "true" : "false", ( clientSession->fIsMulticast ) ? "true" : "false",
                                   clientSession->transportStatus.clientAddressStr ));
                        if (transportStatus.isMulticast) // request is multicast
                        {
                            if (!clientSession->fIsMulticast)   // we are switching from unicast
                            {
                                BDBG_MSG(( "%s: session %p (%08lX) switching from from unicast to multicast", __FUNCTION__, (void *)clientSession, clientSession->satelliteSettings.owner ));
                                clientSession->fIsMulticast = true;

                                /* Transition state */
                                if (stateOnEntry == BIP_RtspLmSessionStreamState_UnicastPlay ||
                                    stateOnEntry == BIP_RtspLmSessionStreamState_UnicastPlayPending
                                    )
                                {
                                    clientSession->setState( BIP_RtspLmSessionStreamState_MulticastChannelChangePending );
                                }
                                else if (stateOnEntry == BIP_RtspLmSessionStreamState_UnicastSetup ||
                                    stateOnEntry == BIP_RtspLmSessionStreamState_UnicastSetupPending
                                   )
                                {
                                    clientSession->setState( BIP_RtspLmSessionStreamState_MulticastSetupPending );
                                }
                                else
                                {BDBG_ERR(("%s: switching from from unicast to multicast. NOT sure what state to go to", __FUNCTION__ ));}

                                /* Clear Existing client Address */
                                if (clientSession->transportStatus.clientAddressStr)
                                {
                                    BKNI_Free( clientSession->transportStatus.clientAddressStr );
                                    clientSession->transportStatus.clientAddressStr = NULL;
                                }

                                // if the client did not provide a multicast address, we need to compute one
                                if (transportStatus.clientAddressStr == NULL)
                                {
                                    clientSession->transportStatus.multicastOctet = clientSession->BIP_Rtsp_ComputeNextMulticastOctet();

                                    clientSession->transportStatus.clientAddressStr =  clientSession->BIP_Rtsp_CreateMulticastAddressStr(
                                            fOurServer->fOurMulticastAddressTemplate, clientSession->transportStatus.multicastOctet );
                                    BDBG_MSG(( "%s: MULTICAST computed destMulticastAddrStr (%s)", __FUNCTION__,
                                               clientSession->transportStatus.clientAddressStr ));
                                }
                                else   // client provided a multicast address
                                {
                                    int newAddressLength = strlen( transportStatus.clientAddressStr )+1 +10 /* just for added insurance */;
                                    clientSession->transportStatus.clientAddressStr =  (char *) BKNI_Malloc( newAddressLength );
                                    BIP_CHECK_GOTO( clientSession->transportStatus.clientAddressStr,
                                        ("clientSession->transportStatus.clientAddressStr = malloc() failed"), error_clientAddressStr1, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );
                                    BKNI_Memset( clientSession->transportStatus.clientAddressStr, 0, newAddressLength );
                                    strncpy( clientSession->transportStatus.clientAddressStr, transportStatus.clientAddressStr, newAddressLength-1 );

                                }

                                if (clientSession->transportStatus.clientAddressStr != NULL)
                                {
                                    netAddressBits destinationAddress = 0;
                                    destinationAddress = inet_addr( clientSession->transportStatus.clientAddressStr );

#if ( USE_IGMP==1 )
                                    BDBG_MSG(( "%s: BIP_IgmpListener_AddGroupAddress 0x%x/(%s)", __FUNCTION__, ntohl( destinationAddress ),
                                               clientSession->transportStatus.clientAddressStr ));
                                    BIP_IgmpListener_AddGroupAddress( fOurServer->fIgmpListener, ntohl( destinationAddress ), 0 );
#else
                                    BDBG_ERR(( "%s: AddGroupAddress(%s) commented out", __FUNCTION__, clientSession->transportStatus.clientAddressStr ));
#endif                              // if ( USE_IGMP==1 )
                                }
                                else
                                {
error_clientAddressStr1:
                                    BDBG_ERR(( "%s: MULTICAST address is not valid", __FUNCTION__ ));
                                }

                                /* for Patterns 17, 18, 19, etc, was causing invalid state */
                                //clientSession->setState( BIP_RtspLmSessionStreamState_MulticastSetupPending );
                            }
                            else // this is a channel change
                            {
                                // if we are already in the PLAY state, just change the tuning parameters
                                if (stateOnEntry == BIP_RtspLmSessionStreamState_MulticastPlay)
                                {
                                    // state needs to be different from PlayPending so that when the response comes back from the App, we know to send a SETUP response
                                    clientSession->setState( BIP_RtspLmSessionStreamState_MulticastChannelChangePending );
                                }
                                else if (stateOnEntry == BIP_RtspLmSessionStreamState_IgmpJoinStreaming)
                                {
                                    /* JOIN followed by SETUP channel change */
                                    clientSession->setState( BIP_RtspLmSessionStreamState_IgmpJoinChannelChangePending );
                                }
                                else
                                {
                                    // we don't need to change state because we are already in setup and app has been notified previously
                                }
                            }
                        }
                        else // request is for unicast
                        {
                            if (clientSession->fIsMulticast)   // we are switching to unicast from multicast; notify IGMP
                            {
                                BDBG_MSG(( "%s: session %p (%08lX) switching from multicast to unicast", __FUNCTION__, (void *)clientSession, clientSession->satelliteSettings.owner ));


                                /* Transition State */
                                if (stateOnEntry == BIP_RtspLmSessionStreamState_MulticastPlay ||
                                    stateOnEntry == BIP_RtspLmSessionStreamState_MulticastPlayPending
                                    )
                                {
                                    clientSession->setState( BIP_RtspLmSessionStreamState_UnicastChannelChangePending );
                                }
                                else if (stateOnEntry == BIP_RtspLmSessionStreamState_MulticastSetup ||
                                    stateOnEntry == BIP_RtspLmSessionStreamState_MulticastSetupPending
                                   )
                                {
                                    clientSession->setState( BIP_RtspLmSessionStreamState_UnicastSetupPending );
                                }
                                else
                                {BDBG_ERR(("%s: switching from from mutlicat to unicast . NOT sure what state to go to", __FUNCTION__ ));}

                                /* Clear clientAddress */
                                if (clientSession->transportStatus.clientAddressStr)
                                {
#if ( USE_IGMP==1 )
                                    BDBG_MSG(( "%s: calling BIP_IgmpListener_DelGroupAddress; ipAddr %x; streamId %d", __FUNCTION__,
                                               ntohl( clientSession->fDestinationAddress ), clientSession->fOurStreamId ));
                                    BIP_IgmpListener_DelGroupAddress( fOurServer->fIgmpListener, ntohl( clientSession->fDestinationAddress ));

                                    BKNI_Free( clientSession->transportStatus.clientAddressStr );
                                    clientSession->transportStatus.clientAddressStr = NULL;
                                    BDBG_MSG(( "%s:%d: clientAddressStr is NULL", __FUNCTION__, __LINE__ ));
#else                               // if ( USE_IGMP==1 )
                                    BDBG_ERR(( "%s: DelGroupAddress(%x) commented out", __FUNCTION__, ntohl( clientSession->fDestinationAddress )));
#endif                              // if ( USE_IGMP==1 )
                                }

                                /* if client did not specify a destination address in the transport header, determine it from the socket */
                                if (transportStatus.clientAddressStr == NULL)
                                {
                                    transportStatus.clientAddressStr = clientSession->GetIpAddressFromSocket( fClientInputSocket );
                                    inet_aton( transportStatus.clientAddressStr, &fClientInputAddr.sin_addr );
                                    BDBG_MSG(( "%s:%d: GetIpAddressFromSocket() returned clientAddressStr (%s) ", __FUNCTION__, __LINE__,
                                               transportStatus.clientAddressStr ));
                                }

                                clientSession->transportStatus.multicastOctet = 0;
                                clientSession->fIsMulticast = 0;
                                if (clientSession->transportStatus.clientAddressStr)
                                {
                                    BKNI_Free( clientSession->transportStatus.clientAddressStr );
                                }
                                /* the new request has a client IP address */
                                if (transportStatus.clientAddressStr)
                                {
                                    clientSession->transportStatus.clientAddressStr = (char *) BKNI_Malloc( strlen( transportStatus.clientAddressStr ) + 1 );
                                    BIP_CHECK_GOTO( clientSession->transportStatus.clientAddressStr,
                                        ("clientSession->transportStatus.clientAddressStr = malloc() failed"), error_clientAddressStr2, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );
                                    BKNI_Memset( clientSession->transportStatus.clientAddressStr, 0, strlen( transportStatus.clientAddressStr )+1 );
                                    strncpy( clientSession->transportStatus.clientAddressStr, transportStatus.clientAddressStr,
                                        strlen( transportStatus.clientAddressStr ));

                                    BDBG_MSG(( "%s:%d: new clientAddressStr (%p) %s", __FUNCTION__, __LINE__,
                                               clientSession->transportStatus.clientAddressStr, clientSession->transportStatus.clientAddressStr ));
                                }
                                else
                                {
  error_clientAddressStr2:
                                    BDBG_ERR(( "%s:%d ERROR ... new request clientAddressStr() is NULL", __FUNCTION__, __LINE__ ));
                                     BDBG_MSG(( "%s:%d: new clientAddressStr (%p) %s", __FUNCTION__, __LINE__,
                                               clientSession->transportStatus.clientAddressStr, clientSession->transportStatus.clientAddressStr ));
                                }


                            }
                            else // unicast to staying in unicast
                            {
                                // if we are already in the PLAY state, just change the tuning parameters
                                if (stateOnEntry == BIP_RtspLmSessionStreamState_UnicastPlay)
                                {
                                    // state needs to be different from PlayPending so that when the response comes back from the App, we know to send a SETUP response
                                    clientSession->setState( BIP_RtspLmSessionStreamState_UnicastChannelChangePending );
                                }
                                else
                                {
                                    // we don't need to change state because we are already in setup and app has been notified previously
                                }
                            }
                        }

                        if (strchr( urlSuffix, '?' ))
                        {
                            BDBG_MSG(( "%s: detected optional settings for %s", __FUNCTION__, cmdName ));
                            clientSession->updateSettings( bitmaskOptionsFound, &satelliteSettings, &transportStatus );
                        }

                        /* these states do not require callback to App */
                        if (( stateOnEntry == BIP_RtspLmSessionStreamState_Idle ) ||
                            ( stateOnEntry == BIP_RtspLmSessionStreamState_UnicastSetup ) ||
                            ( stateOnEntry == BIP_RtspLmSessionStreamState_UnicastSetupPending ) ||
                            ( stateOnEntry == BIP_RtspLmSessionStreamState_MulticastSetup ) ||
                            ( stateOnEntry == BIP_RtspLmSessionStreamState_MulticastSetupPending ) ||
                            ( stateOnEntry == BIP_RtspLmSessionStreamState_TeardownPending ) ||
                            ( stateOnEntry == BIP_RtspLmSessionStreamState_TimeoutPending ) ||
                            ( stateOnEntry == BIP_RtspLmSessionStreamState_IgmpLeavePending )
                            )
                        {
                            char *responseBuffer = (char *) BKNI_Malloc( RESPONSE_BUFFER_SIZE );
                            BIP_CHECK_GOTO( responseBuffer, ("responseBuffer = malloc() failed"), error_responseBuffer1, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );
                            BKNI_Memset( responseBuffer, 0, RESPONSE_BUFFER_SIZE );
                            setResponseSetup( clientSession,  responseBuffer, RESPONSE_BUFFER_SIZE );
                            if (responseBuffer) {BKNI_Free( responseBuffer ); }
error_responseBuffer1:
                            BDBG_MSG(( "%s: skipping App callback; fResponseBuffer (%s) ", __FUNCTION__, (char *) fResponseBuffer ));

                            /* for Pattern 35 and 36 (channel change), move on the the proper state */
                            if ( clientSession->getState() == BIP_RtspLmSessionStreamState_MulticastSetupPending )
                            {
                                clientSession->setState( BIP_RtspLmSessionStreamState_MulticastSetup );
                            }
                            else if ( clientSession->getState() == BIP_RtspLmSessionStreamState_UnicastSetupPending )
                            {
                                clientSession->setState( BIP_RtspLmSessionStreamState_UnicastSetup );
                            }
                        }
                        else
                        {
                            /* we already have a BIP_RtspSession object for this sessionId; invoke messageReceived callback on this object */
                            if (clientSession->fMessageReceivedCallback.callback)
                            {
                                BDBG_MSG(( "%s: connecton %p; fLastCRLF %p; fRequestBuffer %p", __FUNCTION__, (void *)this, fLastCRLF, fRequestBuffer ));
                                char *pos = strstr((char *)fRequestBuffer, " RTSP/1.0" );
                                if (pos) {*pos = '\0'; }
                                BDBG_MSG(( "%s: hSession %p; triggering CALLBACK ... clientSession->fMessageReceivedCallback.context (%s)", __FUNCTION__, (void *)clientSession,
                                           fRequestBuffer ));
                                if (pos) {*pos = ' '; }
                                /* fClientConnection is used in callback from the session object; need to make sure the connection is the right one */
                                clientSession->fClientConnection = this;
                                clientSession->fMessageReceivedCallback.callback( clientSession->fMessageReceivedCallback.context,
                                    fLastCRLF+2 - fRequestBuffer );
                                bSendResponse = false; /* the response will be sent after the APP has processed the request */
                            }
                        }
                    }
                }
                if (transportStatus.clientAddressStr)
                {
                    BKNI_Free( transportStatus.clientAddressStr );
                    transportStatus.clientAddressStr = NULL;
                }
                BDBG_MSG(( "%s: after %s, bSendResponse %u", __FUNCTION__, cmdName, bSendResponse ));
            }
            else if (strcmp( cmdName, "PLAY" ) == 0)
            {
                int bitmaskOptionsFound = 0;
                BIP_RtspLiveMediaSessionSatelliteSettings satelliteSettings;
                BIP_RtspTransportStatus                   transportStatus;
                BIP_RtspLmSessionStreamState              stateOnEntry = BIP_RtspLmSessionStreamState_Idle;

                BKNI_Memset( &satelliteSettings, 0, sizeof( satelliteSettings ));
                BKNI_Memset( &transportStatus, 0, sizeof( transportStatus ));

                bitmaskOptionsFound = BIP_Rtsp_ParseUrlOptions( &satelliteSettings, &transportStatus, (char const *)fRequestBuffer );

                BDBG_MSG(( "%s: case %s; sessionIdStr (%s); streamId %d; urlSuffix %s", __FUNCTION__, cmdName, sessionIdStr,
                          satelliteSettings.streamId, urlSuffix ));
                BDBG_MSG(( "%s: from ip_client (%s)", __FUNCTION__, strstr((char *)fRequestBuffer, "Broadcom" )));

                /* if an error was detected while parsing the URL */
                if (( bitmaskOptionsFound == -1 ) || ( requestIsValid( sessionIdStr ) == False ))
                {
                    handleCmd_bad();
                }
                else if (( satelliteSettings.streamId > 0 ) || strstr((char *)fRequestBuffer, "Broadcom" ) || strstr((char *)fRequestBuffer, "LibVLC" ))
                {
                    // The request included a session id.  Make sure it's one that we have already set up:
                    clientSession = (BIP_RtspServer::BIP_RtspClientSession *)( fOurServer->fClientSessions->Lookup( sessionIdStr ));
                    if (clientSession == NULL)
                    {
                        handleCmd_sessionNotFound();
                    }
                    else
                    {
                        clientSession->noteLiveness();

                        stateOnEntry = clientSession->getState(); // used to determine if we got here after JOIN was received

                        if (clientSession->fIsMulticast)
                        {
                            /* if the session is already streaming as a result of having previously received a JOIN request */
                            if(stateOnEntry == BIP_RtspLmSessionStreamState_IgmpJoinStreaming && strchr( urlSuffix, '?' ))
                            {
                                /* SETUP, JOIN, PLAY w/ URL */
                                clientSession->setState( BIP_RtspLmSessionStreamState_MulticastPlayPending );
                            }
                            else if (stateOnEntry == BIP_RtspLmSessionStreamState_IgmpJoinStreaming)
                            {
                                /* skip the MulticastPlayPending state; go directly to MulticastPlay */
                                clientSession->setState( BIP_RtspLmSessionStreamState_MulticastPlay );
                            }
                            else
                            {
                                clientSession->setState( BIP_RtspLmSessionStreamState_MulticastPlayPending );
                            }
                        }
                        else
                        {
                            clientSession->setState( BIP_RtspLmSessionStreamState_UnicastPlayPending );
                        }
                        /* PLAY with URL Can change SatelliteSettings, but not Transport Status(done with SETUP) */
                        if (strchr( urlSuffix, '?' ))
                        {
                            BDBG_MSG(( "%s: detected optional settings for %s", __FUNCTION__, cmdName ));
                            clientSession->updateSettings( bitmaskOptionsFound, &satelliteSettings, &transportStatus );
                        }

                        /* these states do not require callback to App, Also handle case of JOIN followed by SETUP(channel change) */
                        /* PLAY with NO URL */
                        if (strchr( urlSuffix, '?' ) == NULL &&
                            (( stateOnEntry == BIP_RtspLmSessionStreamState_IgmpJoinPending ) ||
                             ( stateOnEntry == BIP_RtspLmSessionStreamState_IgmpJoinStreaming ) ||
                             ( stateOnEntry == BIP_RtspLmSessionStreamState_MulticastPlayPending ) ||
                             ( stateOnEntry == BIP_RtspLmSessionStreamState_MulticastPlay ) ||
                             ( stateOnEntry == BIP_RtspLmSessionStreamState_UnicastPlayPending ) ||
                             ( stateOnEntry == BIP_RtspLmSessionStreamState_UnicastPlay ) ||
                             ( stateOnEntry == BIP_RtspLmSessionStreamState_UnicastChannelChangePending ) ||
                             ( stateOnEntry == BIP_RtspLmSessionStreamState_MulticastChannelChangePending )
                            )
                            )
                        {
                            char *responseBuffer = (char *) BKNI_Malloc( RESPONSE_BUFFER_SIZE );
                            BIP_CHECK_GOTO( responseBuffer, ("responseBuffer = malloc() failed"), error_responseBuffer2, BIP_ERR_OUT_OF_SYSTEM_MEMORY,rc );
                            BKNI_Memset( responseBuffer, 0, RESPONSE_BUFFER_SIZE );
                            setResponsePlay( responseBuffer, RESPONSE_BUFFER_SIZE, (char *) fRequestBuffer );
                            setRTSPResponse( responseBuffer, clientSession->fOurSessionId ); /* add the RTSP/1.0 and CSeq and Session */
                            if (responseBuffer) {BKNI_Free( responseBuffer ); }
error_responseBuffer2:
                            BDBG_MSG(( "%s: skipping App callback; fResponseBuffer (%s) ", __FUNCTION__, (char *) fResponseBuffer ));

                            /* for Pattern 35 and 36 (channel change), move on the the proper state */
                            if ( clientSession->getState() == BIP_RtspLmSessionStreamState_MulticastPlayPending )
                            {
                                clientSession->setState( BIP_RtspLmSessionStreamState_MulticastPlay );
                            }
                            else if ( clientSession->getState() == BIP_RtspLmSessionStreamState_UnicastPlayPending )
                            {
                                clientSession->setState( BIP_RtspLmSessionStreamState_UnicastPlay );
                            }
                        }
                        else
                        {
                            /* we already have a BIP_RtspSession object for this sessionId */
                            /* invoke messageReceived callback on this object */
                            if (clientSession->fMessageReceivedCallback.callback)
                            {
                                BDBG_MSG(( "%s: connecton %p; fLastCRLF %p; fRequestBuffer %p", __FUNCTION__, (void *)this, fLastCRLF, fRequestBuffer ));
                                char *pos = strstr((char *)fRequestBuffer, " RTSP/1.0" );
                                if (pos) {*pos = '\0'; }
                                BDBG_MSG(( "%s: hSession %p; triggering CALLBACK ... clientSession->fMessageReceivedCallback.context (%s)", __FUNCTION__, (void *)clientSession, fRequestBuffer ));
                                if (pos) {*pos = ' '; }
                                /* fClientConnection is used in callback from the session object; need to make sure the connection is the right one */
                                clientSession->fClientConnection = this;
                                clientSession->fMessageReceivedCallback.callback( clientSession->fMessageReceivedCallback.context, fLastCRLF+2 - fRequestBuffer );
                                bSendResponse = false; /* the response will be sent after the APP has processed the request */
                            }
                        }
                    }
                }
                if (transportStatus.clientAddressStr)
                {
                    BKNI_Free( transportStatus.clientAddressStr );
                    transportStatus.clientAddressStr = NULL;
                }
                BDBG_MSG(( "%s: after %s, bSendResponse %u", __FUNCTION__, cmdName, bSendResponse ));
            }
            else if (strcmp( cmdName, "TEARDOWN" ) == 0)
            {
                int bitmaskOptionsFound = 0;
                BIP_RtspLiveMediaSessionSatelliteSettings satelliteSettings;
                BIP_RtspTransportStatus                   transportStatus;

                //BDBG_SetModuleLevel("bip_rtsp_lm_server", BDBG_eMsg);

                BKNI_Memset( &satelliteSettings, 0, sizeof( satelliteSettings ));
                BKNI_Memset( &transportStatus, 0, sizeof( transportStatus ));

                bitmaskOptionsFound = BIP_Rtsp_ParseUrlOptions( &satelliteSettings, &transportStatus, (char const *)fRequestBuffer );

                BDBG_MSG(( "%s: case %s; sessionIdStr (%s); streamId %d; urlSuffix %s", __FUNCTION__, cmdName, sessionIdStr,
                           satelliteSettings.streamId, urlSuffix ));

                /* if an error was detected while parsing the URL */
                if (( bitmaskOptionsFound == -1 ) || ( requestIsValid( sessionIdStr ) == False ))
                {
                    handleCmd_bad();
                }
                else if (satelliteSettings.streamId > 0)
                {
                    // The request included a session id.  Make sure it's one that we have already set up:
                    clientSession = (BIP_RtspServer::BIP_RtspClientSession *)( fOurServer->fClientSessions->Lookup( sessionIdStr ));
                    BDBG_MSG(( "%s: after Lookup(%s); clientSession %p()", __FUNCTION__, sessionIdStr, (void *)clientSession ));
                    if (clientSession == NULL)
                    {
                        handleCmd_sessionNotFound();
                    }
                    else
                    {
                        clientSession->setState( BIP_RtspLmSessionStreamState_TeardownPending );
                        BDBG_MSG(( "%s: calling handleCmd_TEARDOWN()", __FUNCTION__ ));
                        clientSession->handleCmd_TEARDOWN( this, NULL /*subsession*/, (char const *) fRequestBuffer );
                        BDBG_MSG(( "%s: handleCmd_TEARDOWN done; response (%s)", __FUNCTION__, fResponseBuffer ));

                        if (strlen((char *)fResponseBuffer ) == 0)
                        {
                            bSendResponse = false; // the response will be sent after the APP has processed the request

#if ( USE_IGMP==1 )
                            /* for multicast session, inform IGMP we are tearing down */
                            if (clientSession->fIsMulticast && clientSession->transportStatus.clientAddressStr)
                            {
                                BDBG_MSG(( "%s: calling BIP_IgmpListener_DelGroupAddress; ipAddr %x; streamId %d", __FUNCTION__,
                                           ntohl( clientSession->fDestinationAddress ), clientSession->fOurStreamId ));
                                BIP_IgmpListener_DelGroupAddress( fOurServer->fIgmpListener, ntohl( clientSession->fDestinationAddress ));
                            }
                            else
                            {
                                BDBG_MSG(( "%s: BIP_IgmpListener_DelGroupAddress() ignored; streamId %d is unicast OR clientAddressStr is null (%s)", __FUNCTION__,
                                           clientSession->fOurStreamId, clientSession->transportStatus.clientAddressStr ));
                            }
#else                       // if ( USE_IGMP==1 )
                            BDBG_ERR(( "%s: DelGroupAddress(%x) commented out", __FUNCTION__, ntohl( clientSession->fDestinationAddress )));
#endif                      // if ( USE_IGMP==1 )

                            /* we already have a BIP_RtspSession object for this sessionId */
                            /* invoke messageReceived callback on this object */
                            if (clientSession->fMessageReceivedCallback.callback)
                            {
                                BDBG_MSG(( "%s: connecton %p; fLastCRLF %p; fRequestBuffer %p", __FUNCTION__, (void *)this, fLastCRLF, fRequestBuffer ));
                                char *pos = strstr((char *)fRequestBuffer, " RTSP/1.0" );
                                if (pos) {*pos = '\0'; }
                                BDBG_MSG(( "%s: hSession %p; triggering CALLBACK ... clientSession->fMessageReceivedCallback.context (%s)", __FUNCTION__, (void *)clientSession, fRequestBuffer ));
                                if (pos) {*pos = ' '; }
                                /* fClientConnection is used in callback from the session object; need to make sure the connection is the right one */
                                clientSession->fClientConnection = this;
                                clientSession->fMessageReceivedCallback.callback( clientSession->fMessageReceivedCallback.context, fLastCRLF+2 - fRequestBuffer );
                            }
                            else
                            {
                                BDBG_MSG(( "%s: hSession %p", __FUNCTION__, (void *)clientSession));
                            }
                        }
                        else
                        {
                            bSendResponse = true; // an error was detected; send response now
                        }
                    }
                }
                else
                {
                    /* client provided sessionId==0 with streamId > 0 ... this is an unknown session */
                    BDBG_ERR(( "%s: ERROR: streamId (%d) provided, but no sessionId (%s)", __FUNCTION__, satelliteSettings.streamId, sessionIdStr ));
                    handleCmd_notFound();
                }

                if (transportStatus.clientAddressStr)
                {
                    BKNI_Free( transportStatus.clientAddressStr );
                    transportStatus.clientAddressStr = NULL;
                }
                BDBG_MSG(( "%s: after %s, bSendResponse %u", __FUNCTION__, cmdName, bSendResponse ));
            }
            else if (( strcmp( cmdName, "GET_PARAMETER" ) == 0 ))
            {
                BDBG_MSG(( "%s: case %s: sessionIdStr (%s)", __FUNCTION__, cmdName, sessionIdStr));
                /* Got a command in client session state */

                clientSession = (BIP_RtspServer::BIP_RtspClientSession *)( fOurServer->fClientSessions->Lookup( sessionIdStr ));
                if (clientSession == NULL)
                {
                    handleCmd_sessionNotFound();
                }
                else
                {
                    clientSession->noteLiveness();

                    handleCmd_OPTIONS();
                    bSendResponse = true;
                }
            }
            else
            {
                BDBG_ERR(( "%s: case UNKNOWN", __FUNCTION__ ));
                // The command is one that we don't handle:
                handleCmd_notSupported();
                BDBG_MSG(( "%s: after UNKNOWN, bSendResponse %u", __FUNCTION__, bSendResponse ));
            }
        }
        else
        {
            BDBG_ERR(( "%s: parseRTSPRequestString() failed, Bad RTSP Command", __FUNCTION__ ));
            handleCmd_bad();
            BDBG_MSG(( "%s: after parse failed, bSendResponse %u", __FUNCTION__, bSendResponse ));
        }

        /* we need to send a response now if an error was detected */
        if (bSendResponse)
        {
            BDBG_MSG(( "%s: sending response1: sock (%d); len (%u); cseq (%s)", __FUNCTION__, fClientOutputSocket, (unsigned)strlen((char *)fResponseBuffer ), cseq ));
            BDBG_REQSEND(( "%s: SENDING1 (%s)", __FUNCTION__, (char *)fResponseBuffer ));
            send_rc = send( fClientOutputSocket, (char const *)fResponseBuffer, strlen((char *)fResponseBuffer ), 0 );
            BIP_CHECK_GOTO(( send_rc >= 0 ), ( "send failed ..." ), error_send, BIP_ERR_OS_CHECK_ERRNO, rc );
                BDBG_MSG(( "BIP_RtspClientConnection::%s: send(%d) done; cseq (%s)", __FUNCTION__, fClientOutputSocket, cseq ));
error_send:
            if(send_rc <0)
            {   BDBG_ERR(( "BIP_RtspClientConnection::%s: Unable to send response. ", __FUNCTION__ ));}
        }

#if 0
        /* TODO: add this logic for generic RTSP Server case */
        if (( clientSession != NULL ) && clientSession->fStreamAfterSETUP && ( strcmp( cmdName, "SETUP" ) == 0 ))
        {
            // The client has asked for streaming to commence now, rather than after a
            // subsequent "PLAY" command.  So, simulate the effect of a "PLAY" command:
            clientSession->handleCmd_withinSession( this, "PLAY", urlPreSuffix, urlSuffix, (char const *)fRequestBuffer );
        }
#endif  // if 0

        // Check whether there are extra bytes remaining in the buffer, after the end of the request (a rare case).
        // If so, move them to the front of our buffer, and keep processing it, because it might be a following, pipelined request.
        unsigned requestSize = ( fLastCRLF+4-fRequestBuffer ) + contentLength;
        numBytesRemaining = fRequestBytesAlreadySeen - requestSize;
        resetRequestBuffer(); // to prepare for any subsequent request

        if (numBytesRemaining > 0)
        {
            memmove( fRequestBuffer, &fRequestBuffer[requestSize], numBytesRemaining );
            newBytesRead = numBytesRemaining;
        }
    } while (numBytesRemaining > 0);

    --fRecursionCount;
    if (!fIsActive)
    {
        // Note: The "fRecursionCount" test is for a pathological situation where we reenter the event loop and get called recursively
        // while handling a command (e.g., while handling a "DESCRIBE", to get a SDP description).
        // In such a case we don't want to actually delete ourself until we leave the outermost call.
        if (fRecursionCount > 0)
        {
            closeSockets();
        }
        else
        {
            BDBG_MSG(( "%s: delete myself (%p)", __FUNCTION__, (void *)this ));
            delete this;
        }
    }

    /*return rc; */
} // handleRequestBytes

/**
 *  Function: This function will format the response for a SETUP request method.
 **/
int BIP_RtspServer::BIP_RtspClientConnection::setResponseSetup(
    BIP_RtspClientSession *pClientSession,
    char                  *responseBuffer,
    int                    responseBufferLen
    )
{
    int bytesWritten = 0;

    BDBG_MSG(( "%s: fIsMulticast (%u); destinationAddr (%s/port %d); sourceAddr (%s/port %d)", __FUNCTION__, pClientSession->fIsMulticast,
               pClientSession->transportStatus.clientAddressStr, pClientSession->transportStatus.clientRTPPortNum,
               pClientSession->transportStatus.serverAddressStr, pClientSession->transportStatus.serverRTPPortNum ));

    if (pClientSession->fIsMulticast)
    {
        bytesWritten = snprintf( responseBuffer, responseBufferLen - 1,
                "200 OK\r\n"
                "Transport: RTP/AVP;multicast;destination=%s;source=%s;port=%d-%d;ttl=%d\r\n"
                "Session: %08X\r\n"
                "com.ses.streamID:%d",
                pClientSession->transportStatus.clientAddressStr, pClientSession->transportStatus.serverAddressStr,
                ( pClientSession->transportStatus.clientRTPPortNum ),
                ( pClientSession->transportStatus.clientRTPPortNum+1 ),
                pClientSession->transportStatus.clientTTL,
                pClientSession->fOurSessionId,
                pClientSession->fOurStreamId );
        BDBG_MSG(( "%s: created multicast response... len %d", __FUNCTION__, bytesWritten ));
        setRTSPResponse( responseBuffer, pClientSession->fOurSessionId ); /* add the RTSP/1.0 and CSeq and Session */
    }
    else /* unicast */
    {
        /* ip_client response expected:
        RTSP/1.0 200 OK
        CSeq: 213
        Date: Wed, Mar 13 2013 16:53:57 GMT
        Transport: RTP/AVP;unicast;destination=10.14.236.115;source=10.14.236.108;client_port=49748-49749;server_port=6970-6971
        Session: 42FDA7D1
        */

        if (pClientSession->fIsVlc || pClientSession->fIsIpClient)
        {
            bytesWritten = snprintf( responseBuffer, responseBufferLen - 1,
                    "RTSP/1.0 200 OK\r\n"
                    "CSeq: %s\r\n%s"
                    "Transport: RTP/AVP;unicast;destination=%s;source=%s;client_port=%d-%d;server_port=%d-%d\r\n"
                    "Session: %08X\r\n\r\n",
                    pClientSession->fClientConnection->fCurrentCSeq, dateHeader(),
                    pClientSession->transportStatus.clientAddressStr, pClientSession->transportStatus.serverAddressStr,
                    ( pClientSession->transportStatus.clientRTPPortNum ),
                    ( pClientSession->transportStatus.clientRTPPortNum+1 ),
                    ( pClientSession->transportStatus.serverRTPPortNum ),
                    ( pClientSession->transportStatus.serverRTPPortNum+1 ), pClientSession->fClientConnection->fClientSessionId );
            BKNI_Memset((char *)fResponseBuffer, 0, strlen((char *)fResponseBuffer )+1 );
            strncpy((char *)fResponseBuffer, responseBuffer, strlen((char *)fResponseBuffer ));
        }
        else
        {
            bytesWritten = snprintf( responseBuffer, responseBufferLen - 1,
                    "200 OK\r\n"
                    "Transport: RTP/AVP;unicast;destination=%s;source=%s;client_port=%d-%d;server_port=%d-%d\r\n"
                    "com.ses.streamID:%d",
                    pClientSession->transportStatus.clientAddressStr, pClientSession->transportStatus.serverAddressStr,
                    ( pClientSession->transportStatus.clientRTPPortNum ),
                    ( pClientSession->transportStatus.clientRTPPortNum+1 ),
                    ( pClientSession->transportStatus.serverRTPPortNum ),
                    ( pClientSession->transportStatus.serverRTPPortNum+1 ),
                    pClientSession->fOurStreamId );
            setRTSPResponse( responseBuffer, pClientSession->fOurSessionId ); /* add the RTSP/1.0 and CSeq and Session */
        }

        BDBG_MSG(( "%s: created unicast response... len %d", __FUNCTION__, bytesWritten ));
    }
    responseBuffer[bytesWritten] = '\0';

    BDBG_MSG(( "%s: response string:%s", __FUNCTION__, responseBuffer ));

    return( 0 );
} // setResponseSetup

/**
 *  Function: This function process the response coming back from the App. It is responsible for setting the
 *  state machine appropriately.
 **/
void BIP_RtspServer::BIP_RtspClientConnection::sendResponse(
    BIP_RtspResponseStatus responseStatus
    )
{
    int     rc;
    Boolean bSendResponse   = false;
    Boolean bIsIgmpResponse = false; // for IGMP processing, do not try to send any response to client
    BIP_RtspClientSession       *pClientSession =  NULL;
    BIP_RtspLmSessionStreamState stateNow       = BIP_RtspLmSessionStreamState_Idle;
    char responseBuffer[RESPONSE_BUFFER_SIZE];

    BDBG_MSG(( "Connection::%s:%d: responseStatus %x; fClientSessionId %08X; socket %d", __FUNCTION__, __LINE__, responseStatus,
               fClientSessionId, fClientInputSocket ));
    BDBG_MSG(( "%s:%d: fOurServer %p", __FUNCTION__, __LINE__, (void *)fOurServer ));
    pClientSession =  fOurServer->BIP_Rtsp_FindSessionBySessionId( fClientSessionId );
    BIP_CHECK_PTR_GOTO( pClientSession, "pClientSession is null", error, BIP_ERR_INVALID_PARAMETER );

    BKNI_Memset( responseBuffer, 0, sizeof( responseBuffer ));
    fResponseBuffer[0] = '\0';

    stateNow = pClientSession->getState();

    /* the state will be Idle when we are first creating a new session */
    if (stateNow == BIP_RtspLmSessionStreamState_Idle)
    {
        if (pClientSession->fIsMulticast)
        {
            pClientSession->setState( BIP_RtspLmSessionStreamState_MulticastSetupPending );
        }
        else
        {
            pClientSession->setState( BIP_RtspLmSessionStreamState_UnicastSetupPending );
        }
        pClientSession->fIsIpClient = ( strstr((char *)fRequestBuffer, "Broadcom" )!= NULL );
        pClientSession->fIsVlc      = ( strstr((char *)fRequestBuffer, "VLC media player" )!= NULL );
        BDBG_MSG(( "%s: isVlc (%d); isIpClient (%d)", __FUNCTION__, pClientSession->fIsVlc, pClientSession->fIsIpClient ));
    }

    stateNow = pClientSession->getState();

    if (( stateNow == BIP_RtspLmSessionStreamState_UnicastSetupPending ) ||
        ( stateNow == BIP_RtspLmSessionStreamState_MulticastSetupPending ))
    {
        pClientSession->setState((BIP_RtspLmSessionStreamState) ( stateNow + 1 ));
        if (responseStatus == BIP_RtspResponseStatus_eSuccess)
        {
            setResponseSetup( pClientSession, responseBuffer, sizeof( responseBuffer ));

            bSendResponse = true;
        }
        else /* BIP_RtspResponseStatus_eClientError*/
        {
            handleCmd_bad();

            bSendResponse = true;
        }
    }
    else if (( stateNow == BIP_RtspLmSessionStreamState_UnicastPlayPending ) ||
             ( stateNow == BIP_RtspLmSessionStreamState_MulticastPlayPending ))
    {
        /* you can get here two ways: response from PLAY method or response from JOIN request (no response required) */

        /* if we get here from a PLAY method (otherwise ... the requestBuffer will be empty) */
        if (fRequestBuffer[0] != '\0')
        {
            pClientSession->setState((BIP_RtspLmSessionStreamState) ( stateNow + 1 ));

            if (responseStatus == BIP_RtspResponseStatus_eSuccess)
            {
                setResponsePlay( responseBuffer, sizeof( responseBuffer ), (char *) fRequestBuffer );

                setRTSPResponse( responseBuffer, pClientSession->fOurSessionId ); /* add the RTSP/1.0 and CSeq and Session */
                BDBG_MSG(( "%s: response string:%s", __FUNCTION__, responseBuffer ));
                BDBG_MSG(( "%s: fResponseBuffer:%s", __FUNCTION__, fResponseBuffer ));

                bSendResponse = true;
            }
            else /* BIP_RtspResponseStatus_eClientError*/
            {
                handleCmd_bad();

                bSendResponse = true;
            }
        }
    }
    else if (stateNow == BIP_RtspLmSessionStreamState_UnicastChannelChangePending)
    {
        pClientSession->setState( BIP_RtspLmSessionStreamState_UnicastPlay );
        if (responseStatus == BIP_RtspResponseStatus_eSuccess)
        {
            setResponseSetup( pClientSession, responseBuffer, sizeof( responseBuffer ));

            bSendResponse = true;
        }
        else /* BIP_RtspResponseStatus_eClientError*/
        {
            handleCmd_bad();

            bSendResponse = true;
        }
    }
    else if (stateNow == BIP_RtspLmSessionStreamState_MulticastChannelChangePending)
    {
        pClientSession->setState( BIP_RtspLmSessionStreamState_MulticastPlay );
        if (responseStatus == BIP_RtspResponseStatus_eSuccess)
        {
            setResponseSetup( pClientSession, responseBuffer, sizeof( responseBuffer ));

            bSendResponse = true;
        }
        else /* BIP_RtspResponseStatus_eClientError*/
        {
            handleCmd_bad();

            bSendResponse = true;
        }
    }
    else if (stateNow == BIP_RtspLmSessionStreamState_TeardownPending)
    {
        pClientSession->setState( BIP_RtspLmSessionStreamState_Idle );

        snprintf( responseBuffer, sizeof( responseBuffer ) - 1,  "200 OK" );

        setRTSPResponse( responseBuffer, pClientSession->satelliteSettings.owner ); /* add the RTSP/1.0 and CSeq and Session */

        bSendResponse = true;
    }
    else if (stateNow == BIP_RtspLmSessionStreamState_TimeoutPending)
    {
        pClientSession->setState( BIP_RtspLmSessionStreamState_Idle );

        BDBG_MSG(( "%s: state is TIMEOUT_PENDING", __FUNCTION__ ));
        bSendResponse = false;
    }
    /* if we are processing an IGMP JOIN message ... it comes in during MulticastSetup state, move to JoinStreaming state */
    else if (stateNow == BIP_RtspLmSessionStreamState_MulticastSetup)
    {
        pClientSession->setState( BIP_RtspLmSessionStreamState_IgmpJoinPending );
        bIsIgmpResponse = true;
    }
    else if (stateNow == BIP_RtspLmSessionStreamState_IgmpJoinPending)
    {
        pClientSession->setState( BIP_RtspLmSessionStreamState_IgmpJoinStreaming );
        bIsIgmpResponse = true;
    }
    /* if we are processing an IGMP LEAVE message .. it comes in during JoinStreaming state, stop streaming */
    else if (stateNow == BIP_RtspLmSessionStreamState_IgmpJoinStreaming)
    {
        pClientSession->setState( BIP_RtspLmSessionStreamState_IgmpLeavePending );
        bIsIgmpResponse = true;
    }
    else if (stateNow == BIP_RtspLmSessionStreamState_IgmpJoinChannelChangePending)
    {   /* Case: SETUP, JOIN, SETUP */
        pClientSession->setState( BIP_RtspLmSessionStreamState_IgmpJoinStreaming );
        bIsIgmpResponse = false;  /* need to send a response for the SETUP */

        if (responseStatus == BIP_RtspResponseStatus_eSuccess)
        {
            setResponseSetup( pClientSession, responseBuffer, sizeof( responseBuffer ));

            bSendResponse = true;
        }
        else /* BIP_RtspResponseStatus_eClientError*/
        {
            handleCmd_bad();

            bSendResponse = true;
        }
    }
    /* if we are processing an IGMP LEAVE message .. when LeavePending is complete, go back to Setup */
    else if (stateNow == BIP_RtspLmSessionStreamState_IgmpLeavePending)
    {
        pClientSession->setState( BIP_RtspLmSessionStreamState_MulticastSetup );
        bIsIgmpResponse = true;
    }

    stateNow = pClientSession->getState();

    if (pClientSession->fClientConnection)
    {
        BDBG_MSG(( "%s: bSendResponse %d; fClientOutputSocket (%d); len fResponseBuffer (%u); responseBuffer (%u) ", __FUNCTION__,
                   bSendResponse, pClientSession->fClientConnection->fClientOutputSocket, (unsigned)strlen((char *)fResponseBuffer ), (unsigned)strlen( responseBuffer )));

        if (bIsIgmpResponse)
        {
            BDBG_MSG(( "%s: bIsIgmpResponse %d; ", __FUNCTION__, bIsIgmpResponse ));
        }

        if (( bSendResponse ) && ( pClientSession->fClientConnection->fClientOutputSocket> 0 ) && ( strlen((char *)fResponseBuffer )>0 ))
        {
            BDBG_MSG(( "%s: sending response2: sock (%d); len (%u); cseq (%s)", __FUNCTION__,
                       pClientSession->fClientConnection->fClientOutputSocket, (unsigned)strlen((char *)fResponseBuffer ),
                       pClientSession->fClientConnection->fCurrentCSeq ));
            BDBG_REQSEND(( "%s: SENDING2 (%s)", __FUNCTION__, (char *)fResponseBuffer ));
            rc = send( pClientSession->fClientConnection->fClientOutputSocket, (char const *)fResponseBuffer, strlen((char *)fResponseBuffer ), 0 );
            if (rc<=0)
            {
                BDBG_ERR(( "%s: send(%d) failed; errno (%d); (%s)", __FUNCTION__, pClientSession->fClientConnection->fClientOutputSocket, errno, strerror( errno )));
            }
            BIP_CHECK_GOTO(( rc >= 0 ), ( "send failed ..." ), error, rc, rc );

            BDBG_MSG(( "%s: send(%d) done", __FUNCTION__, pClientSession->fClientConnection->fClientOutputSocket ));
        }
        else
        {
            BDBG_MSG(( "%s: send() skipped", __FUNCTION__ ));
        }
    }

    //BDBG_SetModuleLevel("bip_rtsp_lm_server", BDBG_eErr);

error:
    BDBG_MSG(( "%s: done", __FUNCTION__ ));
    return;
} // sendResponse

/**
 *  Function: This function is a wrapper function used when a session wants to send a response for a specific
 *  connection.
 **/
void BIP_RtspServer::BIP_RtspClientSession::sendResponse(
    BIP_RtspResponseStatus responseStatus
    )
{
    BDBG_MSG(( "Session::%s:%d status %x; fSessionId %x; fClientConnection %p; fRtspClientSession %p", __FUNCTION__, __LINE__,
               responseStatus, fSessionId, (void *)fClientConnection, (void *)fRtspClientSession ));
    BIP_CHECK_PTR_GOTO( fClientConnection, "fClientConnection is null", error, BIP_ERR_INVALID_PARAMETER );
    /* TODO: parse the request header to find what method this reponse corresponds to and take appropriate action */
    fClientConnection->fClientSessionId = fSessionId;
    /* for quick prototype, send a good response */
    BDBG_MSG(( "%s: calling fClientConnection->sendResponse(%d)", __FUNCTION__, responseStatus ));
    fClientConnection->sendResponse( responseStatus );

error:
    return;
}

void BIP_RtspServer::BIP_RtspClientSession::sendResponseUsingRequest(
    BIP_RtspResponseStatus responseStatus,
    char                  *fullRequestStr
    )
{
    char *posSpace = NULL;
    char  method[16];

    BIP_CHECK_PTR_GOTO( fullRequestStr, "fullRequestStr returned null", error, BIP_ERR_INVALID_PARAMETER );

    posSpace = strchr( fullRequestStr, ' ' );
    BIP_CHECK_PTR_GOTO( posSpace, "could not find method delimiter in request", error, BIP_ERR_INVALID_PARAMETER );

    BKNI_Memset( method, 0, sizeof( method ));
    strncpy( method, fullRequestStr, posSpace - fullRequestStr );
    if (strcmp( method, "PLAY" ) == 0)
    {
        BDBG_MSG(( "%s: calling handleCmd_PLAY()", __FUNCTION__ ));
        handleCmd_PLAY( fClientConnection, NULL /*subsession*/, (char const *) fullRequestStr );
        BDBG_MSG(( "%s: handleCmd_PLAY done; calling sendResponse()", __FUNCTION__ ));
        fClientConnection->sendResponse( responseStatus );
    }
    else if (strcmp( method, "TEARDOWN" )==0)
    {
    }

error:
    return;
} // sendResponseUsingRequest

/**
 *  Function: This function is called from the App when the lock callback is triggered. It is used to store the
 *  lock status in the satelliteSettings so that it can be used when creating the SDP string for each session
 *  during a DESCRIBE request.
 **/
void BIP_RtspServer::BIP_RtspClientSession::reportLockStatus(
    bool bLockStatus
    )
{
    BDBG_MSG(( "%s: bLockStatus %d", __FUNCTION__, bLockStatus ));
    satelliteSettings.lock = bLockStatus;
    return;
} // reportLockStatus

/**
 *  Function: This function will use the PID values stored in the internal array and create a string that will
 *  be used when creating the SDP string for each session during a DESCRIBE request.
 **/
char *BIP_Rtsp_CreatePidListString(
    int pidListCount,
    int pidList[]
    )
{
    int   idx        = 0;
    char *pidListStr = (char *)BKNI_Malloc( BIP_MAX_PIDS_PER_PROGRAM*5 );
    char  pidStr[8]; /* string version for a single pid */

    BDBG_MSG(( "%s: count %d", __FUNCTION__, pidListCount ));
    BIP_CHECK_PTR_GOTO( pidListStr, "pidListStr = malloc() failed", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );
    BKNI_Memset( pidListStr, 0, BIP_MAX_PIDS_PER_PROGRAM*5 );

    for (idx = 0; idx<pidListCount; idx++) {
        /* the first pass through, add the pids= tag */
        if (idx==0)
        {
            strncpy( pidListStr, "pids=", BIP_MAX_PIDS_PER_PROGRAM*5 );
        }
        /* convert the pid integer to string */
        snprintf( pidStr, sizeof( pidStr ), "%d", pidList[idx] );
        BDBG_MSG(( "%s: idx %d: pid %d (str %s)", __FUNCTION__, idx, pidList[idx], pidStr ));
        /* append pid string to running string */
        strncat( pidListStr, pidStr, ( BIP_MAX_PIDS_PER_PROGRAM*5 )-1 );
        /* if there is another pid to process, add a comma */
        if (idx+1 < pidListCount)
        {
            strncat( pidListStr, ",", ( BIP_MAX_PIDS_PER_PROGRAM*5 )-1 );
        }
    }
error:
    return( pidListStr );
} // BIP_Rtsp_CreatePidListString

/**
 * Description: Create a Session Description Protocol (SDP) string. The format should be like this:
 *       m=video <port> RTP/AVP 33 (for unicast, port should be 0)
 *       c=IN IP4 <ipaddr>/<ttl>  (if multicast, ip/ttl; else 0.0.0.0)
 *       a=control:stream=<streamId>
 *       a=fmtp:33
 *       ver=<major>.<minor>;src=<srcID>;tuner=<feID>,<level>,<lock>,<quality>,<frequencyMHz>,
 *       <chr-polarisation>,<str-system>,<str-type>,<str-pilots>,<str-roll_off>,<symbol_rate>,<fec_inner>;pids=<pid0>,...,<pidn>
 *       a=inactive  (if non-playing ... inactive; else ... sendonly)
 *
 *  An example valid SDP is:
 *          m=video 5004 RTP/AVP 33
 *          c=IN IP4 239.0.0.8/5
 *          a=control:stream=0
 *          a=fmtp:33 ver=1.0;src=1;tuner=1,240,1,7,12402,v,dvbs,,,,27500,34;pids=0,16,56,112,168,1709
 **/
char *BIP_Rtsp_GenerateSdpDescription(
    BIP_RtspLiveMediaSessionSatelliteSettings *pSatelliteSettings,
    BIP_RtspTransportStatus                   *pTransportStatus,
    BIP_RtspLmSessionStreamState               state
    )
{
    unsigned int port = 0; /* only needed for multicast; zero for unicast */
    char         ipAddrStr[INET_ADDRSTRLEN];
    char         ttlStr[8];
    char         ro[5];               /* 0.35; not required for DVB-S */
    char         mtype[5];            /* 8psk; not required for DVB-S */
    char         pilots[4];           /* on/off; not required for DVB-S */
    int          signalLevel   = 224; /* -25dBm -> 224; -65dBm -> 32; no signal -> 0 */
    int          signalQuality = 7;   /* between 0..15; 0->highest error rate; 15->lowest error rate */
    char         sdpFormat[]   = "m=video %d RTP/AVP 33\r\n"
                                 "c=IN IP4 %s%s\r\n"
                                 "a=control:stream=%d\r\n"
                                 "a=fmtp:33 ver=1.0;src=%d;tuner=%u,%u,%u,%u,%u.00,%c,%s,%s,%s,%s,%u,%u;%s\r\n"
                                 "a=%s\r\n";
    char *sdpDescription = (char *)BKNI_Malloc( MAX_SDP_DESCRIPTION_LEN );
    char *pidListStr     = NULL;

    BIP_CHECK_PTR_GOTO( sdpDescription, "sdpDescription = malloc() failed", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );
    BKNI_Memset( sdpDescription, 0, MAX_SDP_DESCRIPTION_LEN );

    BIP_CHECK_PTR_GOTO( pSatelliteSettings, "pSatelliteSettings is NULL", error, BIP_ERR_INVALID_PARAMETER );

    BKNI_Memset( ipAddrStr, 0, sizeof( ipAddrStr ));

    if (pTransportStatus->isMulticast)
    {
        port = pTransportStatus->clientRTPPortNum;
        snprintf( ttlStr, sizeof( ttlStr ), "/%u", pSatelliteSettings->ttl );
        strncpy( ipAddrStr, pTransportStatus->clientAddressStr, sizeof( ipAddrStr ));
        ipAddrStr[sizeof(ipAddrStr)-1]= '\0';
    }
    else
    {
        port = 0;
        BKNI_Memset( ttlStr, 0, sizeof( ttlStr ));
        strncpy( ipAddrStr, "0.0.0.0", sizeof( ipAddrStr ));
    }
    pidListStr = BIP_Rtsp_CreatePidListString( pSatelliteSettings->pidListCount, pSatelliteSettings->pidList );

    if (strstr( pSatelliteSettings->msys, "dvbs2" ))
    {
        float ro_tmp = pSatelliteSettings->ro;
        snprintf( ro, sizeof( ro ), "%0f.2", ro_tmp/100.0 ); /* 0.35; not required for DVB-S */
        strncpy( mtype, pSatelliteSettings->mtype, sizeof( mtype ));
        mtype[sizeof(mtype)-1]= '\0';

        if (pSatelliteSettings->pilots)
        {
            strncpy( pilots, "on", sizeof( pilots ));
        }
        else
        {
            strncpy( pilots, "off", sizeof( pilots ));
        }
    }
    else   // stream is DVB-S
    {

        strncpy( ro, "0.35", sizeof( ro ));
        strncpy( mtype, "qpsk", sizeof( mtype ));
        strncpy( pilots, "off", sizeof( pilots ));
    }
    /*      a=fmtp:33 ver=<major>.<minor>;src=<srcID>;tuner=<feID>,<level>,<lock>,<quality>,<frequencyMhz>,
     *      <chr-polarisation>,<str-system>,<str-type>,<str-pilots>,<str-roll_off>,<symbol_rate>,<fec_inner>;pids=<pid0>,...,<pidn>
    *       a=inactive  (if non-playing ... inactive; else ... sendonly)
    */
    if (pSatelliteSettings->level != 0)
    {
        signalLevel = pSatelliteSettings->level;
    }
    if (pSatelliteSettings->quality != 0)
    {
        signalQuality = pSatelliteSettings->quality;
    }
    BDBG_MSG(( "%s: freq %d; state %d", __FUNCTION__, pSatelliteSettings->freq, state ));
    snprintf( sdpDescription, 256, sdpFormat, port, ipAddrStr, ttlStr, pSatelliteSettings->streamId,
        pSatelliteSettings->src, pSatelliteSettings->fe, signalLevel,
        pSatelliteSettings->lock, signalQuality, pSatelliteSettings->freq,
        ( pSatelliteSettings->pol_int ) ? 'v' : 'h', pSatelliteSettings->msys, mtype, pilots, ro,
        pSatelliteSettings->sr, pSatelliteSettings->fec, pidListStr,
        ( state==BIP_RtspLmSessionStreamState_UnicastPlay || state==BIP_RtspLmSessionStreamState_MulticastPlay ) ?  "sendonly" : "inactive" );

    if (pidListStr) {BKNI_Free( pidListStr ); }
    BDBG_MSG(( "%s: returning SDP len %u", __FUNCTION__, (unsigned)strlen( sdpDescription )));
    BDBG_MSG(( "%s: returning SDP (%s)", __FUNCTION__, sdpDescription ));
error:
    return( sdpDescription );
} // BIP_Rtsp_GenerateSdpDescription

static char const *sesSatIpCommandStr = "OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY";
/**
 *  Function: This function will handle the incoming OPTIONS RTSP request.
 **/
void BIP_RtspServer::BIP_RtspClientConnection:: handleCmd_OPTIONS()
{
    BDBG_MSG(( "%s: fClientSessionId %08X", __FUNCTION__, fClientSessionId ));
    if (fClientSessionId > 0)
    {
        snprintf((char *)fResponseBuffer, sizeof fResponseBuffer, "RTSP/1.0 200 OK\r\nCSeq: %s\r\nSession: %08lX\r\n%sPublic: %s\r\n\r\n",
            fCurrentCSeq, (long unsigned int)fClientSessionId, dateHeader(), sesSatIpCommandStr );
    }
    else
    {
        snprintf((char *)fResponseBuffer, sizeof fResponseBuffer, "RTSP/1.0 200 OK\r\nCSeq: %s\r\n%sPublic: %s\r\n\r\n", fCurrentCSeq, dateHeader(), sesSatIpCommandStr );
    }
}

/**
 *  Function: This function will create a new SDP string and concatinate it to the input string. This function will be
 *  call once for each known stream (active and incactive).
 **/
char *BIP_Rtsp_ConcatinateSdp(
    char                                      *sdpDescriptionCombined,
    BIP_RtspLiveMediaSessionSatelliteSettings *satelliteSettings,
    BIP_RtspTransportStatus                   *transportStatus,
    BIP_RtspLmSessionStreamState               state
    )
{
    int          idx         = 0;
    char        *streamIdSdp = NULL;
    unsigned int newlength   = 0;
    char        *newSdp      = NULL;

    // create the SDP description for this session
    streamIdSdp = BIP_Rtsp_GenerateSdpDescription( satelliteSettings, transportStatus, state );
    BIP_CHECK_PTR_GOTO( streamIdSdp, "BIP_Rtsp_GenerateSdpDescription returned null", error, BIP_ERR_INVALID_PARAMETER );

    newlength = strlen( sdpDescriptionCombined ) + strlen( streamIdSdp ) + 1 /*null termination */;
    newSdp    = (char *) BKNI_Malloc( newlength );
    BIP_CHECK_PTR_GOTO( newSdp, "malloc() for newSdp returned null", error, BIP_ERR_INVALID_PARAMETER );
    BKNI_Memset( newSdp, 0, newlength );

    BDBG_MSG(( "%s: newlength (%u) = prevlen (%u) + sdp %d (%u) + 1", __FUNCTION__, newlength, (unsigned)strlen( sdpDescriptionCombined ),
               idx, (unsigned)strlen( streamIdSdp )));
    // combine the accumulated SDP description with the one for the current streamId
    strncpy( newSdp, sdpDescriptionCombined, newlength-1 );
    strncat( newSdp, streamIdSdp,            newlength-1 );
    BKNI_Free( sdpDescriptionCombined ); // the contents of this buffer have been copied to the new buffer
    sdpDescriptionCombined = newSdp;     // we have a new combined SDP buffer to use
    BKNI_Free( streamIdSdp );

error:
    return( sdpDescriptionCombined );
} // BIP_Rtsp_ConcatinateSdp

/**
 *  Function: This function will create a new SDP string (specificly for VLC and ip_client) and
 *  concatinate it to the input string.
 **/
char *BIP_Rtsp_ConcatinateSdpVlc(
    char       *sdpDescriptionCombined,
    const char *urlSuffix
    )
{
    /*
                i=?src=1&fe=1&freq=11067&pol=v&msys=dvbs&mtype=qpsk&sr=20000&fec=12&pids=0,259,2305,2307
                t=0 0
                a=tool:LIVE555 Streaming Media v2013.03.07
                a=type:broadcast
                a=control:*
                a=range:npt=0-
                a=x-qt-text-nam:Session streamed by "ip_streamer"
                a=x-qt-text-inf:?src=1&fe=1&freq=11067&pol=v&msys=dvbs&mtype=qpsk&sr=20000&fec=12&pids=0,259,2305,2307
                m=video 0 RTP/AVP 33
                c=IN IP4 0.0.0.0
                b=AS:0
                a=control:track1
    */
    char sdpVlcFormat[] = "i=%s\r\n"
                          "t=0 0\r\n"
                          "a=tool:%s\r\n"
                          "a=type:broadcast\r\n"
                          "a=control:*\r\n"
                          "a=range:npt=0-\r\n"
                          "a=x-qt-text-nam:Session streamed by %s\r\n"
                          "a=x-qt-text-inf:%s\r\n"
                          "m=video 0 RTP/AVP 33\r\n"
                          "c=IN IP4 0.0.0.0\r\n"
                          "b=AS:0\r\n"
                          "a=control:track1\r\n";
    int          sdpVlcLen = strlen( sdpVlcFormat ) + ( 2*strlen( urlSuffix )) + ( 2*strlen( __FUNCTION__ ));
    unsigned int newlength = 0;
    char        *newSdp    = NULL;

    char *sdpVlcStr = (char *) BKNI_Malloc( sdpVlcLen );

    BIP_CHECK_PTR_GOTO( sdpVlcStr, "sdpVlcStr = malloc() failed", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );
    BKNI_Memset( sdpVlcStr, 0, sdpVlcLen );
    sdpVlcLen = snprintf( sdpVlcStr, sdpVlcLen, sdpVlcFormat, urlSuffix, __FUNCTION__, __FUNCTION__, urlSuffix );

    newlength = strlen( sdpDescriptionCombined ) + sdpVlcLen + 1 /*null termination */;
    newSdp    = (char *) BKNI_Malloc( newlength );
    BIP_CHECK_PTR_GOTO( newSdp, "malloc() for newSdp returned null", error, BIP_ERR_INVALID_PARAMETER );
    BKNI_Memset( newSdp, 0, newlength );

    BDBG_MSG(( "%s: newlength (%u) = prevlen (%u) + sdp %d + 1", __FUNCTION__, newlength, (unsigned)strlen( sdpDescriptionCombined ), sdpVlcLen ));
    // combine the accumulated SDP description with the one for the current streamId
    strncpy( newSdp, sdpDescriptionCombined, newlength-1 );
    strncat( newSdp, sdpVlcStr,              newlength-1 );
    BKNI_Free( sdpDescriptionCombined ); // the contents of this buffer have been copied to the new buffer
    sdpDescriptionCombined = newSdp;     // we have a new combined SDP buffer to use
    BKNI_Free( sdpVlcStr );
error:
    return( sdpDescriptionCombined );
} // BIP_Rtsp_ConcatinateSdpVlc

/**
 *  Function: This function will handle the initial processing of each DESCRIBE request.
 **/
void BIP_RtspServer::BIP_RtspClientConnection:: handleCmd_DESCRIBE(
    char const *urlPreSuffix,
    char const *urlSuffix,
    char const *fullRequestStr
    )
{
    BIP_Status rc                         = BIP_SUCCESS;
    int           numStreamsFound        = 0;
    char         *sdpDescriptionCombined = (char *) BKNI_Malloc( 1 );
    int           sdpDescriptionSize     = 0;
    AddressString ipAddressStr( ourIPAddress( envir()));
    unsigned      ipAddressStrSize = strlen( ipAddressStr.val());
    /* extract URL from: DESCRIBE rtsp://10.14.237.58:554/ RTSP/1.0  CSeq:27 */
    char *rtspStart = extractUrl( fullRequestStr );

    BIP_CHECK_GOTO(( sdpDescriptionCombined ), ( "Memory Allocation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );
    sdpDescriptionCombined[0] = '\0'; // an empty string

    BDBG_MSG(( "%s: GOT DESCRIBE Call at %s:  ", __FUNCTION__, __TIME__ ));
    BDBG_MSG(( "urlPreSuffix (%s)", urlPreSuffix ));
    BDBG_MSG(( "urlSuffix (%s)", urlSuffix ));
    BDBG_MSG(( "fullRequestStr (%s)", fullRequestStr ));
    BDBG_MSG(( "DESCRIBE request, ipAddressStr (%s)", ipAddressStr.val()));

    do {
        char sdpHeader[]  = "v=0\r\no=- 0 0 IN IP4 %s\r\ns=SatIPServer:1 %d\r\nt=0 0\r\n";
#if NEXUS_HAS_FRONTEND && defined(NEXUS_MAX_FRONTENDS)
        int  sdpHeaderLen = strlen( sdpHeader ) + ipAddressStrSize + (( NEXUS_MAX_FRONTENDS>9 ) ? 2 : 1 );
#else
        int  sdpHeaderLen = strlen( sdpHeader ) + ipAddressStrSize + 1;
#endif

        BKNI_Free( sdpDescriptionCombined );
        sdpDescriptionCombined = (char *) BKNI_Malloc( sdpHeaderLen );
        BIP_CHECK_GOTO(( sdpDescriptionCombined ), ( "Memory Allocation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );
        BKNI_Memset( sdpDescriptionCombined, 0,  sdpHeaderLen );
#if NEXUS_HAS_FRONTEND && defined(NEXUS_MAX_FRONTENDS)
        snprintf( sdpDescriptionCombined, sdpHeaderLen, sdpHeader, ipAddressStr.val(), NEXUS_MAX_FRONTENDS );
#else
        snprintf( sdpDescriptionCombined, sdpHeaderLen, sdpHeader, ipAddressStr.val(), 0 );
#endif

        {
            int bitmaskOptionsFound = 0;
            BIP_RtspLiveMediaSessionSatelliteSettings satelliteSettings;
            BIP_RtspTransportStatus                   transportStatus;

            BKNI_Memset( &satelliteSettings, 0, sizeof( satelliteSettings ));
            BKNI_Memset( &transportStatus, 0, sizeof( transportStatus ));
            bitmaskOptionsFound = BIP_Rtsp_ParseUrlOptions( &satelliteSettings, &transportStatus, urlSuffix );

            BDBG_MSG(( "%s: lstreamId %d; sdpLen (%u); sdp(%s)", __FUNCTION__, satelliteSettings.streamId, (unsigned)strlen( sdpDescriptionCombined ),
                       sdpDescriptionCombined ));

            /* if an error was detected while parsing the URL */
            if (bitmaskOptionsFound == -1)
            {
                handleCmd_bad();
            }
            /* for VLC and ip_client, DESCRIBE request contains all the tuning parameters and arrives before SETUP request */
            else if (( satelliteSettings.streamId == 0 ) && ( satelliteSettings.freq>0 ))
            {
                /*
                RTSP/1.0 200 OK
                CSeq : 3
                Date : Wed, Mar 13 2013 16 : 53 : 42 GMT
                Content-Base : ? src = 1&fe = 1&freq = 11067&pol = v&msys = dvbs&mtype = qpsk&sr = 20000&fec = 12&pids = 0, 259, 2305, 2307/
                Content-Type : application/sdp
                Content-Length : 496

                v= 0
                o=-1363193622541648 1 IN IP4 10.14.236.108
                s=Session streamed by "ip_streamer"
                i=?src=1&fe=1&freq=11067&pol=v&msys=dvbs&mtype=qpsk&sr=20000&fec=12&pids=0,259,2305,2307
                t=0 0
                a=tool:LIVE555 Streaming Media v2013.03.07
                a=type:broadcast
                a=control:*
                a=range:npt=0-
                a=x-qt-text-nam:Session streamed by "ip_streamer"
                a=x-qt-text-inf:?src=1&fe=1&freq=11067&pol=v&msys=dvbs&mtype=qpsk&sr=20000&fec=12&pids=0,259,2305,2307
                m=video 0 RTP/AVP 33
                c=IN IP4 0.0.0.0
                b=AS:0
                a=control:track1
                */
                sdpDescriptionCombined =  BIP_Rtsp_ConcatinateSdpVlc(  sdpDescriptionCombined,  urlSuffix );

                numStreamsFound = 1;
            }
            else
            {
                HashTable::Iterator   *iter = HashTable::Iterator::create( *fOurServer->fClientSessions );
                BIP_RtspClientSession *pSession;
                char const            *key; // dummy
                while (( pSession = (BIP_RtspServer::BIP_RtspClientSession *)( iter->next( key ))) != NULL) {
                    u_int32_t clientSessionId = pSession->fSessionId;
                    BDBG_MSG(( "%s: iterator stream numStreamsFound %d; sessionId 0x%08X", __FUNCTION__, numStreamsFound, clientSessionId ));
                    // user did not specify a streamId OR they did specify a streamId and we found the one they specified
                    if (( satelliteSettings.streamId == 0 ) ||
                        (( pSession->satelliteSettings.streamId > 0 ) && ( pSession->satelliteSettings.streamId == satelliteSettings.streamId )))
                    {
                        sdpDescriptionCombined = BIP_Rtsp_ConcatinateSdp( sdpDescriptionCombined, &pSession->satelliteSettings,
                                &pSession->transportStatus, pSession->getState());
                        numStreamsFound++;
                    }
                }
                delete iter;
            }     /* end else no freq provided */
            if (transportStatus.clientAddressStr)
            {
                BKNI_Free( transportStatus.clientAddressStr );
                transportStatus.clientAddressStr = NULL;
            }
        }         /* end if msys=dvbs */
    } while (0);

    // if no streams were found
    if (numStreamsFound == 0)
    {
        BDBG_MSG(( "%s: no streams were found; sending 404", __FUNCTION__ ));
        handleCmd_notFound();
    }
    else
    {
        sdpDescriptionSize = strlen( sdpDescriptionCombined ) + 2 /* extra CRLF at the end */;
        snprintf((char *)fResponseBuffer, sizeof fResponseBuffer,
            "RTSP/1.0 200 OK\r\nCSeq: %s\r\n"
            "%s"
            "Content-Type: application/sdp\r\n"
            "Content-Base: %s\r\n"
            "Content-Length: %d\r\n\r\n"
            "%s",
            fCurrentCSeq,
            dateHeader(),
            rtspStart,
            sdpDescriptionSize-2,
            sdpDescriptionCombined );
        BDBG_MSG(( "%s: len %u; ", __FUNCTION__, (unsigned)strlen((char *) fResponseBuffer )));
        BDBG_REQSEND(( "%s: SENDING...(%s)", __FUNCTION__, (char *) fResponseBuffer ));
    }

error:

    if (sdpDescriptionCombined) {BKNI_Free( sdpDescriptionCombined ); }
    if (rtspStart) {BKNI_Free( rtspStart ); }

    /*return rc; */

} // handleCmd_DESCRIBE

// Client Session related logic
BIP_RtspServer::BIP_RtspClientSession *BIP_RtspServer:: createNewClientSession(
    u_int32_t sessionId
    )
{
    BIP_RtspServer::BIP_RtspClientSession    *clientSession;
    BIP_RtspServer::BIP_RtspClientConnection *rtspClientConnection;

    BDBG_MSG(( "BIP_RtspServer::%s: top - sessionId %X", __FUNCTION__, sessionId ));
    rtspClientConnection = (BIP_RtspServer::BIP_RtspClientConnection *)this;

    BDBG_MSG(( "BIP_RtspServer:%s: new BIP_RtspClientSession 0x%08X", __FUNCTION__, sessionId ));

    clientSession = new BIP_RtspClientSession( *this, sessionId );
    BDBG_MSG(( "BIP_RtspServer:%s: returning clientSession %p", __FUNCTION__, (void *)clientSession ));
    BIP_CHECK_PTR_GOTO( clientSession, "clientSession is null", error, BIP_ERR_INVALID_PARAMETER );

error:
    return( clientSession );
} // createNewClientSession

#include "MPEG2TransportFileServerMediaSubsession.hh"
/**
 *  Function: This function is the main constructor for the BIP_RtspServer::BIP_RtspClientSession class.
 **/
BIP_RtspServer::BIP_RtspClientSession::
BIP_RtspClientSession(
    BIP_RtspServer&ourServer,
    u_int32_t      sessionId
    )
    : RTSPClientSession( ourServer, sessionId ), fLivenessCheckTask( NULL )
{
    BDBG_MSG(( "%s: constructor top", __FUNCTION__ ));
    char    streamName[16];
    Boolean reuseFirstSource = false;
    BDBG_MSG(( "%s: BIP_RtspServer::BIP_RtspClientSession:: top - sessionId 0x%08X", __FUNCTION__, sessionId ));
    fOurServer = (BIP_RtspServer *)&ourServer;
    BDBG_MSG(( "%s: fOurServer %p", __FUNCTION__, (void *)fOurServer ));
    if (fOurServer)
    {BDBG_MSG(( "%s: fOurServer->fOurStreamIds %p", __FUNCTION__, (void *)fOurServer->fOurStreamIds )); }
    fOurStreamId = BIP_Rtsp_ComputeNextStreamId();

    BKNI_Memset( streamName, 0, sizeof( streamName ));
    // we use streamId as the stream name. This will enable subsequent describe requests to provide SDP info for a particular streamId
    snprintf( streamName, sizeof( streamName )-1, "%d", fOurStreamId );

    /* TODO: this needs further study to make sure we are creating the SubSession correctly */
    fOurMediaSession = ServerMediaSession::createNew( envir(), streamName, "BRCM SAT>IP Server version: ", "1.0" ); /* 1 */
    BDBG_MSG(( "%s: fOurMediaSession    %p", __FUNCTION__, (void *)fOurMediaSession ));
    fOurMediaSubsession = MPEG2TransportFileServerMediaSubsession ::createNew( envir(), "LiveStream",  NULL /*indexfile*/, reuseFirstSource ); /* 2 */
    fOurMediaSession->addSubsession( fOurMediaSubsession );                                                                                    /*3 */
    BDBG_MSG(( "%s: addServerMediaSession(fOurMediaSubsession %p)", __FUNCTION__, (void *)fOurMediaSubsession ));
    ourServer.addServerMediaSession( fOurMediaSession ); /* 4 */

    // This is the first "SETUP" for this session.  Set up our array of states for all of this session's subsessions (tracks):
    if (fStreamStates == NULL)
    {
        ServerMediaSubsessionIterator iter( *fOurMediaSession );
        fNumStreamStates = 1; // we will only have one stream

        fStreamStates = new struct streamState[fNumStreamStates];

        iter.reset();
        ServerMediaSubsession *subsession;
        for (unsigned i = 0; i < fNumStreamStates; ++i) {
            subsession = iter.next();
            fStreamStates[i].subsession  = subsession;
            fStreamStates[i].streamToken = NULL; // for now; it may be changed by the "getStreamParameters()" call that comes later
        }
    }

    this->fOurServer        = (BIP_RtspServer *)&ourServer;
    this->fClientConnection = ((BIP_RtspServer *)( &ourServer ))->fClientConnection;
    fSessionId              = sessionId;
    fTeardownTaskToken      = NULL;
    fLivenessCheckTask      = NULL;
    fIsPlaying              = 0;
    fDestinationAddress     = 0;
    fIsMulticast            = 0;
    fRtspClientSession      = NULL;
    fState                  = BIP_RtspLmSessionStreamState_Idle;
    BKNI_Memset( &satelliteSettings, 0, sizeof( satelliteSettings ));
    BKNI_Memset( &transportStatus, 0, sizeof( transportStatus ));
    BKNI_Memset( &fMessageReceivedCallback, 0, sizeof( fMessageReceivedCallback ));
    BKNI_Memset( &fIgmpMembershipReportCallback, 0, sizeof( fIgmpMembershipReportCallback ));
    requestUrl = NULL;

    fRtpSource     = NULL;
    fRtpSink       = NULL;
    fRtcpInstance  = NULL;
    fRtpGroupsock  = NULL;
    fRtcpGroupsock = NULL;
    fTimestampFreq = 0;
    fIsVlc         = 0;
    fIsIpClient    = 0;

    this->noteLiveness();
    BDBG_MSG(( "%s: client streamId %d, mediaSession %p, BIP::clientSession %p, clientConnection %p, ourServer %p",
               __FUNCTION__, fOurStreamId, (void *)fOurMediaSession, (void *)this, (void *)this->fClientConnection, (void *)this->fOurServer ));
}

BIP_RtspServer::BIP_RtspClientSession::~BIP_RtspClientSession()
{
    char               keyString[16];
    struct sockaddr_in clientAddr;

    BDBG_DESTRUCTOR(( "%s: destructor %p", __FUNCTION__, (void *)this ));

    BIP_CHECK_PTR_GOTO( fLivenessCheckTask, "fLivenessCheckTask is null", no_liveness, BIP_ERR_INVALID_PARAMETER );
    BDBG_DESTRUCTOR(( "%s: unscheduleDelayedTask(fLivenessCheckTask(%p)\n", __FUNCTION__, fLivenessCheckTask ));
    envir().taskScheduler().unscheduleDelayedTask( fLivenessCheckTask );
    fLivenessCheckTask = 0;
no_liveness:

    // Turn off any periodic RTCPInstance processing
    if ( fGetRtpStatisticsId )
    {
        BDBG_DESTRUCTOR(( "%s: destructor; unscheduleDelayedTask fGetRtpStatisticsId %p", __FUNCTION__, (void *)fGetRtpStatisticsId ));
        envir().taskScheduler().unscheduleDelayedTask( fGetRtpStatisticsId );

        fGetRtpStatisticsId = 0;
    }

    /* RTCPInstance */
    stopRtcpReports();

    snprintf( keyString, sizeof( keyString ), "%08X", fOurSessionId );
    BDBG_DESTRUCTOR(( "%s: Remove(%s)", __FUNCTION__, keyString ));
    fOurServer->fClientSessions->Remove( keyString );
    BDBG_DESTRUCTOR(( "%s: destructor: fClientSessions->Remove (%s); Hashtable NumEntries(%d) ", __FUNCTION__, keyString, fOurServer->fClientSessions->numEntries()));

    clientAddr.sin_addr.s_addr = fDestinationAddress;
    snprintf( keyString, sizeof( keyString ), "%s", inet_ntoa( clientAddr.sin_addr ));
    fOurServer->fClientAddresses->Remove( keyString );
    BDBG_DESTRUCTOR(( "%s: destructor: fClientAddresses->Remove (%s); Hashtable NumEntries(%d) ", __FUNCTION__, keyString, fOurServer->fClientAddresses->numEntries()));

    snprintf( keyString, sizeof( keyString ), STREAMID_FORMAT, fOurStreamId );
    fOurServer->fOurStreamIds->Remove( keyString );
    BDBG_DESTRUCTOR(( "%s: destructor: fOurStreamIds->Remove (%s); Hashtable NumEntries(%d) ", __FUNCTION__, keyString, fOurServer->fOurStreamIds->numEntries()));
#if 0
    /* TODO: CD check this. TEARDOWN of Session followed by exit of HandleRequest bytes causes double delete of Client Connection */
    if (fClientConnection)
    {
        BDBG_MSG(( "%s: delete fClientConnection (%p)", __FUNCTION__, fClientConnection ));
        delete fClientConnection;
    fClientConnection = NULL;
    }
#else // if 0
    fClientConnection = NULL;
#endif // if 0

    BKNI_Memset((void *) &satelliteSettings, 0, sizeof( satelliteSettings ));

    if (transportStatus.clientAddressStr)
    {
        BKNI_Free( transportStatus.clientAddressStr );
        transportStatus.clientAddressStr = NULL;
    }

    if (transportStatus.serverAddressStr)
    {
        BKNI_Free( transportStatus.serverAddressStr );
        transportStatus.serverAddressStr = NULL;
    }

    BKNI_Memset((void *) &transportStatus, 0, sizeof( transportStatus ));
    if (requestUrl)
    {
        BKNI_Free( requestUrl );
        requestUrl = NULL;
    }

    #if 1
    // TODO: how should this be destroyed???
    BDBG_MSG(( "%s: fOurMediaSession (%p); fOurMediaSubsession (%p)", __FUNCTION__, (void *)fOurMediaSession, (void *)fOurMediaSubsession ));
    if (fOurMediaSession)
    {
        #if 0 // causes crash
        BDBG_ERR(( "%s: calling removeServerMediaSession(%p)", __FUNCTION__, fOurMediaSession ));
        fOurServer->removeServerMediaSession( fOurMediaSession ); /* ~4 */
        #endif

        #if 0 // causes crash
        BDBG_ERR(( "%s: calling deleteAllSubsessions (); fOurMediaSubsession (%p)", __FUNCTION__, fOurMediaSubsession ));
        fOurMediaSession->deleteAllSubsessions(); /* ~3 */
        #endif

        //fOurMediaSession = NULL;
    }
    #endif // if 1

    fOurServer         = NULL;
    fRtspClientSession = NULL;
    BKNI_Memset((void *) &satelliteSettings, 0, sizeof( satelliteSettings ));

    if (transportStatus.clientAddressStr)
    {
        BKNI_Free( transportStatus.clientAddressStr );
        transportStatus.clientAddressStr = NULL;
    }

    if (transportStatus.serverAddressStr)
    {
        BKNI_Free( transportStatus.serverAddressStr );
        transportStatus.serverAddressStr = NULL;
    }

    BKNI_Memset((void *) &transportStatus, 0, sizeof( transportStatus ));
    if (requestUrl)
    {
        BKNI_Free( requestUrl );
        requestUrl = NULL;
    }

    BDBG_DESTRUCTOR(( "%s: done", __FUNCTION__ ));
}

BIP_RtspServer::BIP_RtspClientSession *BIP_RtspServer::BIP_RtspClientConnection:: createNewClientSession(
    BIP_RtspServer::BIP_RtspClientConnection *clientConnection,
    char                                     *requestStr
    )
{
    char sessionIdStr[16];
    BIP_RtspServer::BIP_RtspClientSession *bipClientSession = NULL;

    BDBG_MSG(( "BIP_RtspServer::BIP_RtspClientConnection::%s: top", __FUNCTION__ ));
    BKNI_Memset( sessionIdStr, 0, sizeof( sessionIdStr ));
    // So create a new "RTSPClientSession" object for this request.
    // (We avoid choosing session id 0, because that has a special use (by "OnDemandServerMediaSubsession").)
    u_int32_t sessionId = 0;
    do
    {
        /* create session-id using a random (unused) 32-bit integer id (it's encoded as a 8-digit hex number). */
        sessionId = (u_int32_t)our_random32();
        BDBG_MSG(( "%s: new sessionId 0x%8X", __FUNCTION__, sessionId ));
        snprintf( sessionIdStr,sizeof(sessionIdStr),"%08X", sessionId );
        BDBG_MSG(( "%s: Creating BIP_RtspClientSession object: Session sessionIdStr (%s)", __FUNCTION__, sessionIdStr ));
    } while (sessionId == 0 || fOurServer->fClientSessions->Lookup( sessionIdStr ) != NULL);

    BDBG_MSG(( "%s: calling fOurServer->createNewClientSession(0x%x);", __FUNCTION__, sessionId ));
    /* now create the BIP_RtspClientSession object */
    bipClientSession = fOurServer->createNewClientSession( sessionId );
    BIP_CHECK_PTR_GOTO( bipClientSession, "Couldn't create new ClientSession", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );

    /* and add it to a hash table */
    BDBG_MSG(( "%s: Add(%s)->%p to fClientSessions %p", __FUNCTION__, sessionIdStr, (void *)bipClientSession, (void *)fOurServer->fClientSessions ));
    fOurServer->fClientSessions->Add( sessionIdStr, bipClientSession );
    bipClientSession->fClientConnection = clientConnection;

    BDBG_MSG(( "%s: Created BIP_RtspClientSession object (%p), Client Connx %p, Session sessionIdStr %s",
               __FUNCTION__, (void *)bipClientSession, (void *)clientConnection, sessionIdStr ));

    /* now carry out the setup related work and get the response ready */

    BDBG_MSG(( "%s: calling handleCmd_SETUP(); %d", __FUNCTION__, __LINE__ ));
    if(bipClientSession->handleCmd_SETUP( this, NULL /*urlPreSuffix*/, strstr( requestStr, "/?" ) /*urlSuffix*/, (char const *)requestStr ))
    {
        BDBG_ERR(( "%s: ERROR calling handleCmd_SETUP(); %d", __FUNCTION__, __LINE__ ));
        delete(bipClientSession);
        bipClientSession=NULL;
    }

error:

    return( bipClientSession );
} // createNewClientSession

void handleCmd_bad(
    BIP_RtspServer::BIP_RtspClientConnection *ourClientConnection,
    int                                       errorCode
    )
{
    unsigned responseBufferLen;
    char     responseBuffer[RESPONSE_BUFFER_SIZE];

    responseBufferLen = snprintf( responseBuffer, RESPONSE_BUFFER_SIZE-1, "%d \r\n", errorCode );
    responseBuffer[responseBufferLen] = '\0';
    ourClientConnection->setRTSPResponse( responseBuffer );
}

int parseToken(
    const char *input,
    char       *output,
    int         output_len,
    const char *begin,
    const char *end
    )
{
    char *p_begin = strstr((char *) input, begin );
    char *p_end   = NULL;
    char  temp;

    BKNI_Memset( output, 0, output_len );
    //printf("%s: top; in (%s); len %d; begin (%s); end (%s)\n", __FUNCTION__, input, output_len, begin, end );
    //printf("%s: p_begin %p\n", __FUNCTION__, p_begin );
    //printf("%s: p_end   %s\n", __FUNCTION__, p_end );
    if (p_begin)
    {
        p_begin += strlen((char *) begin );
        p_end    = strstr((char *) p_begin, end );
        if (!p_end)
        {
            return( -1 );
        }
        temp = *p_end;

        //BDBG_MSG(( "TokenFound = [%s]; (%s)", p_begin, begin ));
#ifndef DMS_CROSS_PLATFORMS
        strncpy((char *) output, p_begin, p_end - p_begin + 0 );
        BDBG_MSG(( "%s: token found (%s) (%s) output_len %d", __FUNCTION__, begin, output, output_len ));
#else
        if (output_len > 1)
        {
            strncpy((char *) output, p_begin, output_len-1 );
        }
#endif /* DMS_CROSS_PLATFORMS */

        return( 0 );
    }
    return( -1 ); /* Not found */
}                 // parseToken

void parsePidsFromCsvList(
    char *pidListBuffer,
    int  *pidList,
    int  *pidListCount
    )
{
    char *tmp1, *tmp2;

    *pidListCount = 0;
    BDBG_MSG(( "%s: buffer (%s); count %d", __FUNCTION__, pidListBuffer, *pidListCount ));
    /* tmpBuf string point to <num>,<num2>,<num3>... 0*/
    if (pidListBuffer == NULL)
    {
        return;
    }
    for (tmp1 = pidListBuffer;
         (( tmp2 = strstr( tmp1, "," )) && ( *pidListCount <BIP_MAX_PIDS_PER_PROGRAM-1 )); /* leaving one space for last entry */
         tmp1 = tmp2+1
         ) {
        *tmp2 = '\0'; /* replace , with null char, so as to null terminate the string */
        pidList[( *pidListCount )++] = strtol( tmp1, (char **)NULL, 10 );
        BDBG_MSG(( "%s: pidList[%d] %d", __FUNCTION__, ( *pidListCount )-1, pidList[( *pidListCount )-1] ));
        *tmp2 = ','; /* replace back with , */
    }
    /* now get the last pid */
    pidList[( *pidListCount )++] = strtol( tmp1, (char **)NULL, 10 );
    BDBG_MSG(( "%s: pidList[%d] %d", __FUNCTION__, ( *pidListCount )-1, pidList[( *pidListCount )-1] ));
    if (( *pidListCount == BIP_MAX_PIDS_PER_PROGRAM ) && ( tmp2 != NULL ) /*there was still an unprocessed pid in the list */)
    {
        BDBG_ERR(( "%s: pidList is not big enough to store all pids, current size %d", __FUNCTION__, BIP_MAX_PIDS_PER_PROGRAM ));
        return;
    }

#ifdef TEST
    {
        int i;
        for (i = 0; i<*pidListCount; i++) {
            BDBG_MSG(( "pid[%d] = %d of %d pids", i, pidList[i], *pidListCount ));
        }
    }
#endif // ifdef TEST
}      // parsePidsFromCsvList

int BIP_Rtsp_ParseUrlOptions(
    BIP_RtspLiveMediaSessionSatelliteSettings *satSettings,
    BIP_RtspTransportStatus                   *pTransportStatus,
    const char                                *urlSuffix
    )
{
    int bitmaskOptionsFound = 0;

#if NEXUS_HAS_FRONTEND
    char url[256];
    char tmpBuf[256];

    /* the parse logic requires each field to terminate with a '&'; append one to the end of the url (otherwise the parsePidsFromCsvList() logic will fail */
    strncpy( url, urlSuffix, sizeof( url )-1 );
    strncat( url, "&",       sizeof( url )-1 );
    /* "http://192.168.1.109:5000/?src=1&fe=1&freq=12226&pol=v&msys=dvbs&mtype=qpsk&sr=20000&fec=12&pids=0,16,17,18,20,260" */
    /* this check prevents parser from working when url simply has stream=1 in it
    if (strstr(url, "&msys=") == NULL) {
            return -1;
    } */

#if 0
    satSettings->srcType = 1 /* CAD TODO IpStreamerSrc_eSat*/;
#endif
    BDBG_MSG(( "%s: URL (%s)", __FUNCTION__, url ));

    /* stream=<int>&src=1   OR   stream=<int> RTSP/1.0 */
    if (parseToken( url, tmpBuf, sizeof( tmpBuf ), "stream=", "&" ) && parseToken( url, tmpBuf, sizeof( tmpBuf ), "stream=", " " ))
    {
        BDBG_MSG(( "%s: No stream Attribute found", __FUNCTION__ ));
    }
    else
    {
        bitmaskOptionsFound  |= 0x0001;
        satSettings->streamId = strtof( tmpBuf, (char **)NULL );
        BDBG_MSG(( "%s: streamId %u", __FUNCTION__, satSettings->streamId ));
    }

    /* freq=units in MHz */
    if (parseToken( url, tmpBuf, sizeof( tmpBuf ), "freq=", "&" ))
    {
        BDBG_MSG(( "%s: No Frequency Attribute found", __FUNCTION__ ));
    }
    else
    {
        bitmaskOptionsFound |= 0x0002;
        satSettings->freq    = strtof( tmpBuf, (char **)NULL );
        BDBG_MSG(( "%s:  freq in Mhz %d", __FUNCTION__, satSettings->freq ));
    }

    /* msys=dvbs | dvbs2 */
    if (parseToken( url, tmpBuf, sizeof( tmpBuf ), "msys=", "&" ))
    {
        BDBG_MSG(( "%s: Sat sys is not set", __FUNCTION__ ));
    }
    else
    {
        bitmaskOptionsFound |= 0x0004;
        strncpy( satSettings->msys, tmpBuf, sizeof( satSettings->msys )-1 );
        BDBG_MSG(( "%s: Sat sys is (%s)", __FUNCTION__, satSettings->msys ));
    }

    if (parseToken( url, tmpBuf, sizeof( tmpBuf ), "mtype=", "&" ))
    {
        BDBG_MSG(( "%s: Sat Modulation type is not set", __FUNCTION__ ));
    }
    else
    {
        bitmaskOptionsFound |= 0x0008;
        strncpy( satSettings->mtype, tmpBuf, sizeof( satSettings->mtype )-1 );
        BDBG_MSG(( "%s: Sat Modulation type is (%s)", __FUNCTION__, satSettings->mtype ));
    }

    if (parseToken( url, tmpBuf, sizeof( tmpBuf ), "plts=", "&" ))
    {
        BDBG_MSG(( "%s: Sat pilot tones is not set", __FUNCTION__ ));
    }
    else
    {
        bitmaskOptionsFound   |= 0x0010;
        satSettings->pilotTone = ( strcmp( tmpBuf, "on" )==0 ) ? 1 : 0;
        BDBG_MSG(( "%s: Sat pilotTone is set (%u), strlen tmpBuf %u", __FUNCTION__, satSettings->pilotTone, (unsigned)strlen( tmpBuf )));
    }

    /* sr=2000 in kSymb/s */
    if (parseToken( url, tmpBuf, sizeof( tmpBuf ), "sr=", "&" ))
    {
        BDBG_MSG(( "%s: No SymbolRate Attribute found", __FUNCTION__ ));
    }
    else
    {
        bitmaskOptionsFound |= 0x0020;
        satSettings->sr      = strtof( tmpBuf, (char **)NULL );
        BDBG_MSG(( "%s: Sat Symbol Rate %d in kSymb/s", __FUNCTION__, satSettings->sr ));
    }

    /* fec=12 | 23 | 34 | 56 | 78 | 35 | 45 | 910 */
    if (parseToken( url, tmpBuf, sizeof( tmpBuf ), "fec=", "&" ))
    {
        BDBG_MSG(( "%s: No fec Attribute found", __FUNCTION__ ));
    }
    else
    {
        bitmaskOptionsFound |= 0x0040;
        satSettings->fec     = strtof( tmpBuf, (char **)NULL );
        BDBG_MSG(( "%s: Sat FEC %d ", __FUNCTION__, satSettings->fec ));
    }

    if (parseToken( url, tmpBuf, sizeof( tmpBuf ), "ro=", "&" ))
    {
        BDBG_MSG(( "%s: No roll-off (ro) Attribute found", __FUNCTION__ ));
    }
    else
    {
        bitmaskOptionsFound |= 0x0080;
        satSettings->ro      = strtof( tmpBuf, (char **)NULL );
        BDBG_MSG(( "%s: Sat roll-off %f ", __FUNCTION__, satSettings->ro ));
    }
    /* pol=h | v | l | r polarization */
    if (parseToken( url, tmpBuf, sizeof( tmpBuf ), "pol=", "&" ))
    {
        BDBG_MSG(( "%s: No Polarization Attribute found", __FUNCTION__ ));
    }
    else
    {
        bitmaskOptionsFound |= 0x0100;
        satSettings->pol_int = ( strcmp( tmpBuf, "v" )==0 ) ? 0 : 1;
        strncpy( satSettings->pol, tmpBuf, sizeof( satSettings->pol )-1 );
        BDBG_MSG(( "%s: Polarization is %s", __FUNCTION__, satSettings->pol ));
    }

    /* parse these pid fields */
    /* pids=<num>,<num2>,<num3>... */
    /* pids=all | none */
    if (( parseToken( url, tmpBuf, sizeof( tmpBuf ), "addpids=", "&" ) != 0 ) &&
        ( parseToken( url, tmpBuf, sizeof( tmpBuf ), "delpids=", "&" ) != 0 ) &&
        ( parseToken( url, tmpBuf, sizeof( tmpBuf ), "pids=", "&" ) == 0 ))
    {
        bitmaskOptionsFound |= 0x0200;
        if (strcmp( tmpBuf, "all" ) == 0)
        {
            /* pids=all | none */
            BDBG_MSG(( "%s: pids is set to all for MPTS streaming. TODO", __FUNCTION__ ));
            /*cfg->enableAllpass = true; */
        }
        else if (strcmp( tmpBuf, "none" ) == 0)
        {
            BDBG_MSG(( "%s: pids is set none", __FUNCTION__ ));
            /*cfg->noDemuxOutput = true; */
            satSettings->pidListCount = 0;
        }
        else
        {
            BDBG_MSG(( "%s: parsePidsFromCsvList ", __FUNCTION__ ));
            /* pids=<num>,<num2>,<num3>... */
            int idx;
            parsePidsFromCsvList( tmpBuf, satSettings->pidList, &satSettings->pidListCount );
            BDBG_MSG(( "%s: pidListCount %d: ", __FUNCTION__, satSettings->pidListCount ));
            for (idx = 0; idx<satSettings->pidListCount; idx++) {
                BDBG_MSG(( "%d", satSettings->pidList[idx] ));
            }
        }
    }
    if (parseToken( url, tmpBuf, sizeof( tmpBuf ), "addpids=", "&" ) == 0)
    {
        bitmaskOptionsFound |= 0x40000;
        BDBG_MSG(( "%s: parseAddPids FromCsvList ", __FUNCTION__ ));
        /* pids=<num>,<num2>,<num3>... */
        int idx;
        parsePidsFromCsvList( tmpBuf, satSettings->addPidList, &satSettings->addPidListCount );
        BDBG_MSG(( "%s: pidListCount %d: ", __FUNCTION__, satSettings->addPidListCount ));
        for (idx = 0; idx<satSettings->addPidListCount; idx++) {
            BDBG_MSG(( "%d", satSettings->addPidList[idx] ));
        }
    }

    if (parseToken( url, tmpBuf, sizeof( tmpBuf ), "delpids=", "&" ) == 0)
    {
        bitmaskOptionsFound |= 0x80000;
        BDBG_MSG(( "%s: parseDelPids FromCsvList ", __FUNCTION__ ));
        /* pids=<num>,<num2>,<num3>... */
        int idx;
        parsePidsFromCsvList( tmpBuf, satSettings->delPidList, &satSettings->delPidListCount );
        BDBG_MSG(( "%s: pidListCount %d: ", __FUNCTION__, satSettings->delPidListCount ));
        for (idx = 0; idx<satSettings->delPidListCount; idx++) {
            BDBG_MSG(( "%d", satSettings->delPidList[idx] ));
        }
    }
#if 0
    if (!cfg->noDemuxOutput)
    {
        cfg->ipDstEnabled = true;
    }
#endif // if 0

    /* some sat farms have multiple sat dishes ... src=1 tells which of multiple dishes we are trying to access */
    if (parseToken( url, tmpBuf, sizeof( tmpBuf ), "src=", "&" ))
    {
        BDBG_MSG(( "%s: No src Attribute found", __FUNCTION__ ));
        satSettings->src = 0;
    }
    else
    {
        bitmaskOptionsFound |= 0x0400;
        satSettings->src     = strtol( tmpBuf, (char **)NULL, 10 );
    }

    /* some sat dishes have multiple fe in the center of the disk ... up to 4 */
    if (parseToken( url, tmpBuf, sizeof( tmpBuf ), "fe=", "&" ))
    {
        BDBG_MSG(( "%s: No fe Attribute found", __FUNCTION__ ));
        satSettings->fe = 0;
    }
    else
    {
        bitmaskOptionsFound |= 0x0800;
        satSettings->fe      = strtol( tmpBuf, (char **)NULL, 10 );
    }

#if 0
    /* if url provided ip addr*/
    if (parseToken( url, tmpBuf, sizeof( tmpBuf ), "addr=", "&" ))
    {
        BDBG_MSG(( "%s: No ipaddr Attribute found", __FUNCTION__ ));
        cfg->streamingIpAddress = NULL;
    }
    else
    {
        cfg->streamingIpAddress = (char *) BKNI_Malloc( tmpBuf + 1 );
        if (cfg->streamingIpAddress)
        {
            BKNI_Memset( cfg->streamingIpAddress, 0, tmpBuf + 1 );
            strncpy( cfg->streamingIpAddress, tmpBuf, sizeof( cfg->streamingIpAddress )-1 );
        }
        BDBG_MSG(( "%s: IPaddr (%s)", __FUNCTION__, cfg->streamingIpAddress ));
    }
    /* ir url provided ip port */
    if (parseToken( url, tmpBuf, sizeof( tmpBuf ), "port=", "&" ))
    {
        BDBG_MSG(( "%s: No port Attribute found", __FUNCTION__ ));
    }
    else
    {
        cfg->srcPort = cfg->clientPort = strtol( tmpBuf, (char **)NULL, 10 );
        BDBG_MSG(( "%s: IPport (%d)", __FUNCTION__, cfg->srcPort ));
    }
#endif // if 0

    /* TODO: not needed for DVBS */
    /* ro=<fixed point num> ignored for DVBS and for now */
    /* mcast=<dot-decimal ip addr> */

    /* Parse Transport Options */
    {
        BIP_RtspLiveMediaStreamingMode streamingMode                = BIP_StreamingMode_eRTP_UDP;
        char                          *clientsDestinationAddressStr = NULL;
        u_int8_t                       clientsDestinationTTL        = 0;
        unsigned short                 clientRTPPortNum             = 0, clientRTCPPortNum = 0;
        Boolean                        isUnicast = false, fIsMulticast = false;

        BDBG_MSG(( "%s: processing TransportHeader", __FUNCTION__ ));
        bitmaskOptionsFound |= parseTransportHeader( urlSuffix, streamingMode, clientsDestinationAddressStr, clientsDestinationTTL,
                clientRTPPortNum, clientRTCPPortNum, fIsMulticast, isUnicast );
        BDBG_MSG(( "%s: Client Requested Transport Parameters...", __FUNCTION__ ));
        BDBG_MSG(( "Client Requested Transport Parameters: streamingMode %s, dest addr %s, ttl %d, ports: RTP %d, RTCP %d, multicast %d, unicast %d",
                   streamingMode == BIP_StreamingMode_eRTP_UDP ? "RTP_UDP" : "Not Supported",
                   clientsDestinationAddressStr, clientsDestinationTTL,
                   clientRTPPortNum, clientRTCPPortNum,
                   fIsMulticast, isUnicast ));
        if (pTransportStatus->clientAddressStr)
        {
            BKNI_Free( pTransportStatus->clientAddressStr );
            pTransportStatus->clientAddressStr = NULL;
        }
        pTransportStatus->clientAddressStr = clientsDestinationAddressStr;
        BDBG_MSG(( "%s:%d: new clientAddressStr (%p) (%s); BKNI_Malloc() was in parseTransportHeader()", __FUNCTION__, __LINE__, pTransportStatus->clientAddressStr,
                   pTransportStatus->clientAddressStr ));
        pTransportStatus->streamingMode    = streamingMode;
        pTransportStatus->clientRTPPortNum = clientRTPPortNum;
        pTransportStatus->isMulticast      = fIsMulticast;
        pTransportStatus->serverRTPPortNum = 0;
        pTransportStatus->clientTTL        = clientsDestinationTTL;

        // validate various transport parameters, e.g.
        if (isUnicast && fIsMulticast)
        {
            BDBG_ERR(( "%s: Incorrect Transport Parameters: Client specified both unicast & multicast fields!!", __FUNCTION__ ));
            bitmaskOptionsFound = -1;
        }
        else if (isUnicast && ( clientRTPPortNum == 0 ))
        {
            BDBG_ERR(( "%s: Incorrect Transport Parameters: Client didn't provide client_port for unicast transport streaming request!!", __FUNCTION__ ));
            bitmaskOptionsFound = -1;
        }
    }
#else  // if NEXUS_HAS_FRONTEND
    BSTD_UNUSED( satSettings );
    BSTD_UNUSED( urlSuffix );
    BSTD_UNUSED( pTransportStatus );
#endif /* NEXUS_HAS_FRONTEND  */
    BDBG_MSG(( "%s: done; bitmask 0x%X", __FUNCTION__, bitmaskOptionsFound ));
    return( bitmaskOptionsFound );
} // BIP_Rtsp_ParseUrlOptions

/**
 *  Function: This function will parse the incoming transport header.
 **/
int parseTransportHeader(
    char const                     *buf,
    BIP_RtspLiveMediaStreamingMode &streamingMode,
    char *                         &clientAddressStr,
    u_int8_t                       &destinationTTL,
    portNumBits                    &clientRTPPortNum,  // if UDP
    portNumBits                    &clientRTCPPortNum, // if UDP
    Boolean                        &isMulticast,
    Boolean                        &isUnicast
    )
{
    int bitmaskOptionsFound = 0;

    // Initialize the result parameters to default values:
    streamingMode     = BIP_StreamingMode_eRTP_UDP;
    clientAddressStr  = NULL;
    destinationTTL    = 2; /* Default RTP multicast is 2 for ttl */
    clientRTPPortNum  = 0; /* CAD: was 1500. Why??? */
    clientRTCPPortNum = 1; /* CAD: was 1501. Why??? */
    isMulticast       = False;
    isUnicast         = False;

    portNumBits p1, p2;
    unsigned    ttl;
    BDBG_MSG(( "%s: buf %s", __FUNCTION__, buf ));

    // First, find "Transport:"
    while (1) {
        if (*buf == '\0')
        {
            return( 0 ); // not found
        }
        if (( *buf == '\r' ) && ( *( buf+1 ) == '\n' ) && ( *( buf+2 ) == '\r' ))
        {
            return( 0 ); // end of the headers => not found
        }
        if (_strncasecmp( buf, "Transport:", 10 ) == 0) {break; }
        ++buf;
    }

    // Then, run through each of the fields, looking for ones we handle:
    char const *fields = buf + 10;
    BDBG_MSG(( "%s: fields %s", __FUNCTION__, fields ));
    while (*fields == ' ') {++fields; }
    char *field = (char *) BKNI_Malloc( strlen( fields ) + 1 );
    BIP_CHECK_PTR_GOTO( field, "BKNI_Malloc() for field failed", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );
    BKNI_Memset( field, 0, strlen( fields ) + 1 );

    while (sscanf( fields, "%[^;\r\n]", field ) == 1)
    {
        BDBG_MSG(( "%s: field %s", __FUNCTION__, field ));
        if (strcmp( field, "RTP/AVP/TCP" ) == 0)
        {
            bitmaskOptionsFound |= 0x1000;
            streamingMode        = BIP_StreamingMode_eRTP_TCP;
        }
        else if (( strcmp( field, "RAW/RAW/UDP" ) == 0 ) || ( strcmp( field, "MP2T/H2221/UDP" ) == 0 ))
        {
            bitmaskOptionsFound |= 0x1000;
            streamingMode        = BIP_StreamingMode_eRAW_UDP;
        }
        else if (strcmp( field, "RTP/AVP" ) == 0)
        {
            bitmaskOptionsFound |= 0x1000;
            streamingMode        = BIP_StreamingMode_eRTP_UDP;
        }
        else if (strcmp( field, "multicast" ) == 0)
        {
            bitmaskOptionsFound |= 0x2000;
            isMulticast          = true;
        }
        else if (strcmp( field, "unicast" ) == 0)
        {
            bitmaskOptionsFound |= 0x4000;
            isUnicast            = true;
        }
        else if (_strncasecmp( field, "destination=", 12 ) == 0)
        {
            bitmaskOptionsFound |= 0x8000;
            // if the input address already has something in it
            if (clientAddressStr)
            {
                // if the parsed address matches already in use address, just use the existing one
                if (strcmp( clientAddressStr, field+12 ) == 0)
                {
                }
                else
                {
                    // free the existing address
                    BKNI_Free( clientAddressStr );

                    clientAddressStr = (char *) BKNI_Malloc( strlen( field+12 ) + 1 );
                    if (clientAddressStr)
                    {
                        BKNI_Memset( clientAddressStr, 0, strlen( field+12 ) + 1 );
                        strncpy( clientAddressStr, field+12, strlen( field+12 ));
                    }
                }
            }
            else
            {
                clientAddressStr = (char *) BKNI_Malloc( strlen( field+12 ) + 1 );
                if (clientAddressStr)
                {
                    BKNI_Memset( clientAddressStr, 0, strlen( field+12 ) + 1 );
                    strncpy( clientAddressStr, field+12, strlen( field+12 ));
                }
            }
        }
        else if (sscanf( field, "ttl=%u", &ttl ) == 1)
        {
            bitmaskOptionsFound |= 0x10000;
            destinationTTL       = (u_int8_t)ttl;
        }
        else if (sscanf( field, "client_port=%hu-%hu", &p1, &p2 ) == 2)
        {
            bitmaskOptionsFound |= 0x20000;
            clientRTPPortNum     = p1;
            clientRTCPPortNum    = streamingMode == BIP_StreamingMode_eRAW_UDP ? 0 : p2; // ignore the second port number if the client asked for raw UDP
        }
        else if (sscanf( field, "client_port=%hu", &p1 ) == 1)
        {
            bitmaskOptionsFound |= 0x20000;
            clientRTPPortNum     = p1;
            clientRTCPPortNum    = streamingMode == BIP_StreamingMode_eRAW_UDP ? 0 : p1 + 1;
        }
        else if (sscanf( field, "port=%hu-%hu", &p1, &p2 ) == 2)
        {
            bitmaskOptionsFound |= 0x20000;
            clientRTPPortNum     = p1;
            clientRTCPPortNum    = streamingMode == BIP_StreamingMode_eRAW_UDP ? 0 : p2; // ignore the second port number if the client asked for raw UDP

            BDBG_MSG(( "%s: Port Num %d - %d", __FUNCTION__, p1, p2 ));
        }

        fields += strlen( field );
        while (*fields == ';') {++fields; // skip over separating ';' chars
        }
        if (( *fields == '\0' ) || ( *fields == '\r' ) || ( *fields == '\n' )) {break; }
    }
    BKNI_Free( field );
error:

    if (isUnicast)
    {
        destinationTTL = 64; /* Default to 64 if unicast, destinationTTL should never be requested by client when in Unicast */
    }

    BDBG_MSG(( "%s: done; bitmask 0x%X", __FUNCTION__, bitmaskOptionsFound ));
    return( bitmaskOptionsFound );
} // parseTransportHeader

/**
 *  Function: This function will schedule the 5-second periodic to collect RTP statistics for the RTCP report.
 **/
void BIP_RtspServer::BIP_RtspClientSession:: scheduleGetRtpStatistics(
    void
    )
{
    int64_t timeoutMicroSeconds = 5 * 1000000;

    fGetRtpStatisticsId = envir().taskScheduler().scheduleDelayedTask( timeoutMicroSeconds, (TaskFunc *)&BIP_Rtsp_GetRtpStatisticsCallback,
            (void *)this );
}

/**
 *  Function: This function will unschedule the 5-second periodic to collect RTP statistics for the RTCP report.
 **/
void BIP_RtspServer::BIP_RtspClientSession:: unscheduleGetRtpStatistics(
    void
    )
{
    if (fGetRtpStatisticsId) {
        envir().taskScheduler().unscheduleDelayedTask( fGetRtpStatisticsId );
    }
}

/**
 *  Function: This function will perform steps necessary to start RTCP statistics.
 **/
void BIP_RtspServer::BIP_RtspClientSession:: startRtcpReports(
    void
    )
{
#define B_LM_MAX_CNAME_LEN     100
#define B_LM_ESTIMATED_RTCP_BW 160         // in kbps; for RTCP b/w share
#define B_MP2T_TIMESTAMP_FREQ  90000       // MP2T RTP timestamp frequency (90KHz)
    static unsigned char CNAME[B_LM_MAX_CNAME_LEN+1];
    fTimestampFreq = B_MP2T_TIMESTAMP_FREQ;

    // create (and start) a 'RTCP instance' for the RTP source:
    gethostname((char *)CNAME, B_LM_MAX_CNAME_LEN );
    CNAME[B_LM_MAX_CNAME_LEN] = '\0'; // just in case

    struct in_addr sessionAddress;
    BKNI_Memset( &sessionAddress, 0, sizeof( sessionAddress ));
    sessionAddress.s_addr = this->fDestinationAddress;

    fRtpGroupsock = new Groupsock( envir(), sessionAddress, ( transportStatus.clientRTPPortNum ), satelliteSettings.ttl );
    fRtpSink      = new BIP_RtpSink( envir(), (Groupsock *)fRtpGroupsock, 33, fTimestampFreq, "video/MP2T", 1 );
    BDBG_MSG(( "%s: RTCP: addr (%s); ports %d-%d; CNAME (%s); fRtpSink (%p); ttl %d", __FUNCTION__, inet_ntoa( sessionAddress ),
               transportStatus.clientRTPPortNum, transportStatus.clientRTPPortNum+1, CNAME, (void *)fRtpSink, satelliteSettings.ttl ));
    fRtcpGroupsock = new Groupsock( envir(), sessionAddress, ( transportStatus.clientRTPPortNum+1 ), satelliteSettings.ttl );
    fRtcpInstance  = RTCPInstance::createNew( envir(), fRtcpGroupsock, B_LM_ESTIMATED_RTCP_BW, CNAME,
            fRtpSink /* we're a server */,  NULL /* source */, (Boolean)False /*isSSMSource */ );
    fRtpSink->enableRTCPReports();

    scheduleGetRtpStatistics();
} // startRtcpReports

/**
 *  Function: This function will perform steps necessary to stop RTCP statistics.
 **/
void BIP_RtspServer::BIP_RtspClientSession:: stopRtcpReports(
    void
    )
{
    unscheduleGetRtpStatistics();

    //fRtpSink->disableRTCPReports();

    if (fRtcpInstance)
    {
        BDBG_DESTRUCTOR(( "%s: Medium::close (%p) ", __FUNCTION__, (void *)fRtcpInstance ));
        Medium::close( fRtcpInstance ); // Note: Sends a RTCP BYE
        fRtcpInstance = NULL;
    }

    if (fRtcpGroupsock)
    {
        delete fRtcpGroupsock;
        fRtcpGroupsock = NULL;
    }

    if (fRtpSink) // was after fRtcpInstance Medium::close; moved to before fRtcpInstance to resolve crash in RTCPIntance destructor
    {
        BDBG_DESTRUCTOR(( "%s: fRtpSink (%p)", __FUNCTION__, (void *)fRtpSink ));
        fRtpSink->destroy();
        fRtpSink = NULL;
    }

    if (fRtpGroupsock)
    {
        delete fRtpGroupsock;
        fRtpGroupsock = NULL;
    }

    return;
} // stopRtcpReports

/**
 *  Function: This function will process the incoming SETUP request.
 **/
BIP_Status BIP_RtspServer::BIP_RtspClientSession:: handleCmd_SETUP(
    BIP_RtspServer::BIP_RtspClientConnection *ourClientConnection,
    char const                               *urlPreSuffix,
    char const                               *urlSuffix,
    char const                               *fullRequestStr
    )
{
    BSTD_UNUSED( ourClientConnection );
    BIP_Status  rc = BIP_SUCCESS;
    // Check if this SETUP Request came in on an existing session (where a client sends this request to modify some of the transport parameters)?
    if (fIsPlaying)
    {
        BDBG_MSG(( "%s: Received SETUP on a RTSP Session where we have previously received PLAY request", __FUNCTION__ ));
        //TODO: determine if we need to cleanup anything from the previous streaming session. May be app needs to know this
    }

    // For SAT>IP, we only treat one track per ServerMediaSession. So normal SAT>IP clients would not have a track-id appended to the URI.
    // In that case, urlPreSuffix would be NULL and URI string would be part of urlSuffic parameters.
    // However, LiveMedia library based clients may append track1 to the URI and thus both of these params may be valid. In this case,
    // urlPreSuffix points to the URI and urlSuffix points to the track-id.
    char const *uriQueryString;
    if (urlPreSuffix != NULL) {uriQueryString = urlPreSuffix; }
    else {uriQueryString = urlSuffix; }
    BDBG_MSG(( "%s: Handle Setup Request: urlPreSuffix (%s) fullRequestStr (%s)", __FUNCTION__, urlPreSuffix, fullRequestStr ));
    BDBG_MSG(( "%s: urlSuffix %s; uriQueryString %s", __FUNCTION__, urlSuffix, uriQueryString ));

    do {
        // If stream= header is present in the uriQueryString, then check if this client is the owner of the stream or not.
        // -If it is not the owner, then it is the 2nd client trying to join an existing stream. Carefully parse its transport header requests.
        // --If original session is multicast, nly then 2nd client can request either multicast (join this multicast) or separate unicast.
        // --Or if original session is unicast, then 2nd client can only request unicast and not multicast.
        // TODO: Add this logic

        // Look for a "Transport:" header in the request string, to extract client parameters:
        BIP_RtspLiveMediaStreamingMode streamingMode;
        char                          *clientsDestinationAddressStr = NULL;
        netAddressBits                 destinationAddress           = 0;
        u_int8_t                       clientsDestinationTTL;
        portNumBits                    clientRTPPortNum, clientRTCPPortNum;
        Boolean                        isUnicast;
        struct sockaddr_in             sourceAddr;
        socklen_t                      namelen = 0;
        Port serverRTPPort( 0 );
        Port serverRTCPPort( 0 );
        Port clientRTPPort( 0 );
        Port clientRTCPPort( 0 );

        // If stream= header is present in the uriQueryString, then check if this client is the owner of the stream or not.
        // -If it is not the owner, then it is the 2nd client trying to join an existing stream. Carefully parse its transport header requests.
        // --If original session is multicast, only then 2nd client can request either multicast (join this multicast) or separate unicast.
        // --Or if original session is unicast, then 2nd client can only request unicast and not multicast.
        // TODO: Add this logic

        if (fullRequestStr && ( strstr( fullRequestStr, "msys=dvb" ) || strstr( fullRequestStr, "stream=" )))
        {
            /* ClientSession has been created and can store Satellite setting and transportSettings */
            BKNI_Memset( &this->satelliteSettings, 0, sizeof( this->satelliteSettings ));
            BKNI_Memset( &this->transportStatus, 0, sizeof( this->transportStatus ));
            BIP_Rtsp_ParseUrlOptions( &this->satelliteSettings, &this->transportStatus, fullRequestStr );  /* this is actually fullRequestBuffer passed in */

            /* if client is requesting new session and copying from existing stream */
            if (strstr( fullRequestStr, "stream=" ))
            {
                unsigned int lStreamId = 0;
                sscanf( strstr( fullRequestStr, "stream=" ), "stream=%u", &lStreamId );
                BDBG_MSG(( "%s: scanned streamID %d", __FUNCTION__, lStreamId ));
                if (lStreamId > 0)
                {
                    BIP_RtspClientSession *pSession = NULL;
                    pSession = BIP_Rtsp_FindOwnerByStreamId( lStreamId );
                    /* if we found an existing session that matches the specified stream ID */
                    if (pSession)
                    {
                        unsigned long int sessionId = this->satelliteSettings.owner;
                        BDBG_MSG(( "%s: copying existing session %p to new session %p", __FUNCTION__, (void *)pSession, (void *)this ));
                        /* copy the parameters from existing session to new session */
                        this->satelliteSettings          = pSession->satelliteSettings;
                        this->satelliteSettings.streamId = 0; // force the logic to create a new session
                        this->satelliteSettings.owner    = sessionId;
                        /* take the transport status from the URL coming in, not from previous session */
                    }
                }
            }
            /* TODO  Rest of funtion needs to change to use satelliteSettings and transportStatus*/
            BDBG_MSG(( "%s: streamId %d; freq %u ", __FUNCTION__, this->satelliteSettings.streamId, this->satelliteSettings.freq ));
            BDBG_MSG(( "%s: pidListCount %d ", __FUNCTION__, this->satelliteSettings.pidListCount ));

            if (this->satelliteSettings.streamId == 0)   // streamId was not specified by the client ... this is a new request
            {
                this->satelliteSettings.streamId = fOurStreamId;
                BDBG_MSG(( "%s: PUSHING NEW streamId %u; freq %u; src %u; fe %u; sr %u", __FUNCTION__, fOurStreamId,
                           this->satelliteSettings.freq, this->satelliteSettings.src, this->satelliteSettings.fe, this->satelliteSettings.sr ));

                // Look for a "Transport:" header in the request string, to extract client parameters:
                parseTransportHeader( fullRequestStr, streamingMode, clientsDestinationAddressStr, clientsDestinationTTL,
                    clientRTPPortNum, clientRTCPPortNum, fIsMulticast, isUnicast );
                BDBG_MSG(( "%s: Client Requested Transport Parameters...", __FUNCTION__ ));
                BDBG_MSG(( "Client Transport Info: streamingMode %s, clientsDestinationAddressStr (%s), ttl %d, ports: RTP %d, RTCP %d, multicast %d, unicast %d",
                           streamingMode == BIP_StreamingMode_eRTP_UDP ? "RTP_UDP" : "Not Supported",
                           clientsDestinationAddressStr, clientsDestinationTTL,
                           clientRTPPortNum, clientRTCPPortNum,
                           fIsMulticast, isUnicast ));

                this->satelliteSettings.owner          = fOurSessionId;
                this->transportStatus.isMulticast      = fIsMulticast;
                this->transportStatus.clientRTPPortNum = clientRTPPortNum;

                this->requestUrl = (char *) BKNI_Malloc( strlen( fullRequestStr ) + 1 );
                BIP_CHECK_GOTO( this->requestUrl, ("BKNI_Malloc() for this->requestUrl failed"), error_requestUrl, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );
                BKNI_Memset( this->requestUrl, 0, strlen( fullRequestStr ) + 1 );
                strncpy( this->requestUrl, fullRequestStr, strlen( fullRequestStr ));
error_requestUrl:
                this->transportStatus.clientTTL = clientsDestinationTTL;
                if (this->transportStatus.clientAddressStr)
                {
                    BKNI_Free( this->transportStatus.clientAddressStr );
                }

                /* if client did not specify a destination address in the transport header, determine it from the socket */
                if (clientsDestinationAddressStr==NULL)
                {
                    if (isUnicast)
                    {
                        this->transportStatus.clientAddressStr = GetIpAddressFromSocket( this->fClientConnection->fClientInputSocket );
                        inet_aton( this->transportStatus.clientAddressStr, &this->fClientConnection->fClientInputAddr.sin_addr );
                    }
                    else // isMulticast
                    {
                        /* Generate multicast address */
                        this->transportStatus.multicastOctet   = BIP_Rtsp_ComputeNextMulticastOctet();
                        this->transportStatus.clientAddressStr = BIP_Rtsp_CreateMulticastAddressStr( this->fOurServer->fOurMulticastAddressTemplate, this->transportStatus.multicastOctet );
                    }
                }
                else
                {
                    int newAddressLength = strlen( clientsDestinationAddressStr )+10;

                    /* address passed in from Transport Setting */
                    this->transportStatus.clientAddressStr = (char *) BKNI_Malloc( newAddressLength );
                    BIP_CHECK_GOTO( this->transportStatus.clientAddressStr, ("BKNI_Malloc() for this->transportStatus.clientAddressStr failed"), error_clientAddr, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );
                    BKNI_Memset( this->transportStatus.clientAddressStr, 0, newAddressLength );
                    strncpy( this->transportStatus.clientAddressStr, clientsDestinationAddressStr, newAddressLength-1 );
error_clientAddr:
                    BDBG_MSG(( "%s:%d: new clientAddressStr (%p) %s", __FUNCTION__, __LINE__,
                               this->transportStatus.clientAddressStr, this->transportStatus.clientAddressStr ));
                }

                /* we are done with the clientsDestinationAddressStr that was malloc'ed in parseTransportHeader() */
                if (clientsDestinationAddressStr)
                {
                    BKNI_Free( clientsDestinationAddressStr );
                }

                /* the Add(session) is done in createNewClientSession() */

                char streamIdStr[16];
                snprintf( streamIdStr, sizeof( streamIdStr ), STREAMID_FORMAT, this->satelliteSettings.streamId );
                BDBG_MSG(( "%s: Add(%s)->%p to fOurStreamIds %p", __FUNCTION__, streamIdStr, (void *)this, (void *)fOurServer->fOurStreamIds ));
                fOurServer->fOurStreamIds->Add( streamIdStr, this );

                BDBG_MSG(( "%s: Add(%s)->%p to fClientAddresses %p", __FUNCTION__, this->transportStatus.clientAddressStr, (void *)this, (void *)fOurServer->fClientAddresses ));
                if (this->transportStatus.clientAddressStr) {fOurServer->fClientAddresses->Add( this->transportStatus.clientAddressStr, this ); }

                BDBG_MSG(( "%s: SETUP: isUnicast %d; fIsMulticast %d", __FUNCTION__, isUnicast, fIsMulticast ));

#if 0
                if (isUnicast)
                {
                    // for unicast, get the Client's IP address from the one used for RTSP Session Request
                    // TODO: make this a public interface of SatIpClientConnection Class
                    BDBG_MSG(( "%s: extracting destinationAddr", __FUNCTION__ ));
                    BDBG_MSG(( "%s: fClientConnection (%p)", __FUNCTION__, (void *) this->fClientConnection ));
                    BDBG_MSG(( "%s: fClientInputAddr (%p)", __FUNCTION__, (void *) &this->fClientConnection->fClientInputAddr ));
                    BDBG_MSG(( "%s: ClientInputAddr (%s)", __FUNCTION__, inet_ntoa( this->fClientConnection->fClientInputAddr.sin_addr )));
                    destinationAddress = this->fClientConnection->fClientInputAddr.sin_addr.s_addr;
                    BDBG_MSG(( "%s: unicast: destinationAddr (%x)", __FUNCTION__, destinationAddress ));
                }
                else
                {
                    BDBG_MSG(( "%s: INSIDE MULTICAST ****************** ClientInputAddr (%s); str (%s)", __FUNCTION__,
                               inet_ntoa( this->fClientConnection->fClientInputAddr.sin_addr ), this->transportStatus.clientAddressStr ));

                    destinationAddress = inet_addr( this->transportStatus.clientAddressStr );
                    BDBG_MSG(( "%s: MULTICAST computed destMulticastAddrStr (%s)", __FUNCTION__, this->transportStatus.clientAddressStr ));

#if ( USE_IGMP==1 )
                    BDBG_MSG(( "%s: BIP_IgmpListener_AddGroupAddress 0x%x/(%s)", __FUNCTION__, ntohl( destinationAddress ),
                               this->transportStatus.clientAddressStr ));
                    BIP_IgmpListener_AddGroupAddress( fOurServer->fIgmpListener, ntohl( destinationAddress ), 0 );
#else
                    BDBG_ERR(( "%s: AddGroupAddress(%s) commented out", __FUNCTION__, this->transportStatus.clientAddressStr ));
#endif              // if ( USE_IGMP==1 )

#if 0
                    /*Todo: Don't think this is needed should not be NULL.   Check this */
                    // multicast case
                    // client may not provide either port or multicast address. server will need to choose!
                    if (this->transportStatus.clientAddressStr == NULL)
                    {

                        /*Todo: Don't think this is needed should not be NULL.   Check this */
                        this->transportStatus.multicastOctet   = this->BIP_Rtsp_ComputeNextMulticastOctet();
                        this->transportStatus.clientAddressStr =  this->BIP_Rtsp_CreateMulticastAddressStr(
                                fOurServer->fOurMulticastAddressTemplate, this->transportStatus.multicastOctet );

                        destinationAddress = inet_addr( this->transportStatus.clientAddressStr );
                        BDBG_MSG(( "%s: MULTICAST computed destMulticastAddrStr (%s)", __FUNCTION__, this->transportStatus.clientAddressStr ));

                        BDBG_MSG(( "%s: BIP_IgmpListener_AddGroupAddress 0x%x/(%s)", __FUNCTION__, ntohl( destinationAddress ),
                                   this->transportStatus.clientAddressStr ));
                        BIP_IgmpListener_AddGroupAddress( fOurServer->fIgmpListener, ntohl( destinationAddress ), 0 );
                    }
                    else
                    {
                        /* use the one provided by client */
                        destinationAddress = our_inet_addr( this->transportStatus.clientAddressStr );
                        BDBG_MSG(( "%s: MULTICAST provided clientsAddressStr (%s)", __FUNCTION__, this->transportStatus.clientAddressStr ));
                    }
                    BDBG_MSG(( "%s: multicast: destinationAddr (%x)", __FUNCTION__, destinationAddress ));
#endif              // if 0
                }
#endif
                fDestinationAddress = destinationAddress;

                clientRTPPort  = clientRTPPortNum;
                clientRTCPPort = clientRTCPPortNum;

                // Make sure that we transmit on the same interface that's used by the client (in case we're a multi-homed server):
                // TODO: we dont need this logic as we will use our own streaming library & socket
                namelen = sizeof sourceAddr;
                rc      = getsockname( this->fClientConnection->fClientInputSocket, (struct sockaddr *)&sourceAddr, &namelen );
                AddressString sourceAddrStr( sourceAddr );
                BIP_CHECK_GOTO(( rc==0 ), ( "getsockname failed ..." ), error_serverAddr, rc, rc );

                this->transportStatus.serverAddressStr = (char *) BKNI_Malloc( strlen( sourceAddrStr.val()) + 1 );
                BIP_CHECK_GOTO( this->transportStatus.serverAddressStr, ("BKNI_Malloc() for this->transportStatus.serverAddressStr failed"), error_serverAddr, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );
                BKNI_Memset( this->transportStatus.serverAddressStr, 0, strlen( sourceAddrStr.val()) + 1 );
                strncpy( this->transportStatus.serverAddressStr, sourceAddrStr.val(), strlen( sourceAddrStr.val()));
error_serverAddr:
                BDBG_MSG(( "%s: socket %d; serverAddrStr (%s)", __FUNCTION__, this->fClientConnection->fClientInputSocket,
                           this->transportStatus.serverAddressStr ));

                createRtpRtcpSockets( serverRTPPort, serverRTCPPort );

                this->transportStatus.serverRTPPortNum = ntohs( serverRTPPort.num());
                BDBG_MSG(( "%s: getStreamParameters done; clientAddr (%s); serverPort (%d), str ip addr (%s); streamToken (%p)",
                           __FUNCTION__, this->transportStatus.clientAddressStr,
                           this->transportStatus.serverRTPPortNum,
                           this->transportStatus.clientAddressStr, fStreamStates[0].streamToken ));

                #if 0
                startRtcpReports();
                #else
                BDBG_MSG(("%s: startRtcpReports() commented out", __FUNCTION__ ));
                #endif
            }
            else // this->satelliteSettings.streamId > 0
            {
                BIP_RtspClientSession *pSession = NULL;

                BDBG_MSG(( "Existing stream request; session %p", (void *)this ));
                pSession = BIP_Rtsp_FindOwnerByStreamId( this->fOurStreamId );
                if (pSession && fTeardownTaskToken)
                {
                    BDBG_MSG(( "%s: unscheduledDelayedTask for TEARDOWN; token %p", __FUNCTION__, fTeardownTaskToken ));
                    envir().taskScheduler().unscheduleDelayedTask( fTeardownTaskToken );
                    fTeardownTaskToken = NULL;
                }
            }
        }
    }
    while (0);

    return (rc);
}          // handleCmd_SETUP

void BIP_RtspServer::BIP_RtspClientSession:: createRtpRtcpSockets(
    Port&serverRTPPort,
    Port&serverRTCPPort
    )
{
    Groupsock  *rtpGroupsock=NULL;
    Groupsock  *rtcpGroupsock=NULL;
    portNumBits fInitialPortNum = 6970;
    // Normal case: We're streaming RTP (over UDP or TCP).  Create a pair of
    // groupsocks (RTP and RTCP), with adjacent port numbers (RTP port number even):
    NoReuse dummy( envir()); // ensures that we skip over ports that are already in use

    for (portNumBits serverPortNum = fInitialPortNum;; serverPortNum += 2) {
        struct in_addr dummyAddr; dummyAddr.s_addr = 0;

        serverRTPPort = serverPortNum;
        rtpGroupsock  = new Groupsock( envir(), dummyAddr, serverRTPPort, 255 );
        if (rtpGroupsock->socketNum() < 0)
        {
            delete rtpGroupsock;
            continue; // try again
        }

        serverRTCPPort = serverPortNum+1;
        rtcpGroupsock  = new Groupsock( envir(), dummyAddr, serverRTCPPort, 255 );
        if (rtcpGroupsock->socketNum() < 0)
        {
            delete rtpGroupsock;
            delete rtcpGroupsock;
            continue; // try again
        }

        break; // success
    }
    BDBG_MSG(("%s:%d rtpGroupsock is %d; rtcpGroupsock is %d ", __FUNCTION__, __LINE__, rtpGroupsock->socketNum(), rtcpGroupsock->socketNum() ));
    delete rtpGroupsock;
    delete rtcpGroupsock;
    return;
} // createRtpRtcpSocket

/**
 *  Function: This function will process the incoming PLAY request.
 **/
void BIP_RtspServer::BIP_RtspClientSession::handleCmd_PLAY(
    BIP_RtspServer::BIP_RtspClientConnection *ourClientConnection,
    ServerMediaSubsession                    *subsession,
    char const                               *fullRequestStr
    )
{
    BIP_Status rc = BIP_SUCCESS;
    BSTD_UNUSED( fullRequestStr );
    BDBG_MSG(( "%s: fOurServerMediaSession (%p); ourClientConnection->fClientInputSocket (%d); subsession %p", __FUNCTION__,
               (void *)fOurServerMediaSession, ourClientConnection->fClientInputSocket, (void *)subsession ));

    if (this->satelliteSettings.streamId > 0)
    {
        BIP_RtspClientSession *pOwner = NULL;
        BIP_RtspClientSession *pMine  = NULL;

        /* TODO:    change to function to use   fOurServer->fClientSessions->Lookup( sessionIdStr ) */
        pOwner = BIP_Rtsp_FindOwnerByStreamId( satelliteSettings.streamId );
        BIP_CHECK_GOTO( pOwner, ("BIP_Rtsp_FindOwnerByStreamId returned null"), error, BIP_ERR_INVALID_PARAMETER, rc );
        pMine = fOurServer->BIP_Rtsp_FindSessionBySessionId( fOurSessionId );
        BIP_CHECK_GOTO( pMine, ("BIP_Rtsp_FindOwnerBySessionId returned null"), error, BIP_ERR_INVALID_PARAMETER, rc );

        BDBG_MSG(( "%s: owner 0x%8lx; mine 0x%8lx; multicast %d", __FUNCTION__, pOwner->satelliteSettings.owner,
                   satelliteSettings.owner, pOwner->transportStatus.isMulticast ));
        if (pOwner->satelliteSettings.owner == fOurSessionId) // I am stream owner
        {
            if (pOwner->transportStatus.isMulticast)
            {
                pOwner->setState( BIP_RtspLmSessionStreamState_MulticastPlay );
            }
            else
            {
                pOwner->setState( BIP_RtspLmSessionStreamState_UnicastPlay );
            }
        }
        else   // not owner
        {
            BDBG_MSG(( "%s: NOT owner multicast %d", __FUNCTION__, pOwner->transportStatus.isMulticast ));
            if (pOwner->transportStatus.isMulticast)   // request from non-owner
            {
                if (( pOwner->getState() == BIP_RtspLmSessionStreamState_MulticastSetup ) ||
                    ( pOwner->getState() == BIP_RtspLmSessionStreamState_MulticastPlay ))
                {
                    if (pOwner->transportStatus.isMulticast) // client wants to join multicast
                    {                                        // no state change for us or multicast owner
                        /* nothing to do here */
                    }
                    else
                    {
                        pMine->setState( BIP_RtspLmSessionStreamState_UnicastPlay );
                    }
                }
                else if (( pOwner->getState() == BIP_RtspLmSessionStreamState_UnicastSetup ) ||
                         ( pOwner->getState() == BIP_RtspLmSessionStreamState_UnicastPlay ))
                {
                    handleCmd_bad( ourClientConnection, BIP_RtspSessionReturnCode_MethodNotValidInThisState );
                    return;
                }
            }
            else
            {
                if (( pOwner->getState() == BIP_RtspLmSessionStreamState_UnicastSetup )  ||
                    ( pOwner->getState() == BIP_RtspLmSessionStreamState_UnicastPlay ) ||
                    ( pOwner->getState() == BIP_RtspLmSessionStreamState_MulticastSetup ))
                {
                    pOwner->setState( BIP_RtspLmSessionStreamState_UnicastPlay );
                }
                else if (pOwner->getState() == BIP_RtspLmSessionStreamState_MulticastPlay)
                {
                    pOwner->setState( BIP_RtspLmSessionStreamState_UnicastPlay );
                }
            }
        }
    }

error:
    BDBG_MSG(( "%s: done", __FUNCTION__ ));
    /*return rc; */
} // handleCmd_PLAY

/**
 *  Function: This function will process the incoming TEARDOWN request.
 **/
void BIP_RtspServer::BIP_RtspClientSession:: handleCmd_TEARDOWN(
    BIP_RtspServer::BIP_RtspClientConnection *ourClientConnection,
    ServerMediaSubsession                    *subsession,
    char const                               *fullRequestStr
    )
{
    BIP_Status              rc       = BIP_SUCCESS;
    BIP_RtspClientSession *pSession = NULL;

    BDBG_MSG(( "%s: top: clientConnection %p; session %p; url (%s)", __FUNCTION__, (void *)ourClientConnection, (void *)subsession, fullRequestStr ));
    BSTD_UNUSED( subsession );
    BIP_CHECK_GOTO( ourClientConnection, ("ourClientConnection is NULL"), error, BIP_ERR_INVALID_PARAMETER,rc );

    // handle the request ... rtsp://<ipaddr>/stream=<streamId>

    if (this->satelliteSettings.streamId > 0)
    {
        // if user specified an invalid stream id
        if (this->satelliteSettings.streamId <= 0)
        {
            handleCmd_bad( ourClientConnection, BIP_RtspSessionReturnCode_BadRequest );
            return;
        }

        pSession = BIP_Rtsp_FindOwnerByStreamId( this->satelliteSettings.streamId );

        if (pSession)
        {
            BDBG_MSG(( "%s: isMulticast %d; owner (%08lX); req session (%08X)", __FUNCTION__, pSession->transportStatus.isMulticast,
                       pSession->satelliteSettings.owner, fOurSessionId ));
            // if (stream is multicast && I own this stream) OR (the stream is unicast ... always honor teardown of unicast )
            if (( pSession->transportStatus.isMulticast &&  ( pSession->satelliteSettings.owner == fOurSessionId )) ||
                ( !pSession->transportStatus.isMulticast ))
            {
                ourClientConnection->fResponseBuffer[0] = '\0'; // response will be created after App has processed the request

            }
            else
            {
                // non-owner is not allowed to teardown someone else's stream
                setRTSPResponse( ourClientConnection, "400 Bad Request" );
                rc = BIP_ERR_INVALID_PARAMETER;
            }
        }
    }

    unsigned i;
    for (i = 0; i < fNumStreamStates; ++i) {
        if (subsession == NULL /* means: aggregated operation */ || subsession == fStreamStates[i].subsession) {
            if (fStreamStates[i].subsession != NULL) {
                BDBG_MSG(("%s: deleteStream(fOurSessionId 0x%X; token %p)", __FUNCTION__, fOurSessionId, fStreamStates[i].streamToken ));
                fStreamStates[i].subsession->deleteStream(fOurSessionId, fStreamStates[i].streamToken);
                fStreamStates[i].subsession = NULL;
            }
        }
    }

error:

    return;
} // handleCmd_TEARDOWN

/**
 *  Function: This function is the callback function that is used to wrap the C++ function so that we can the function
 *  from scheduleDelayedTask() API.
 **/
void BIP_Rtsp_ConnectionLivenessCallback(
    void *context
    )
{
    BIP_RtspServer::BIP_RtspClientConnection *pClientConnection = (BIP_RtspServer::BIP_RtspClientConnection *)context;

    BIP_CHECK_PTR_GOTO( pClientConnection, "pClientConnection is NULL", error, BIP_ERR_INVALID_PARAMETER );

    BDBG_MSG(( "%s: pClientConnection %p", __FUNCTION__, (void *)pClientConnection ));
    pClientConnection->livenessTimeoutTask( pClientConnection );
error:
    return;
}

/**
 *  Function: This function is the callback function that is used to wrap the C++ function so that we can
 *  the function from scheduleDelayedTask() API.
 **/
void BIP_Rtsp_SessionLivenessCallback(
    void *context
    )
{
    BIP_RtspServer::BIP_RtspClientSession *pClientSession = (BIP_RtspServer::BIP_RtspClientSession *)context;

    BIP_CHECK_PTR_GOTO( pClientSession, "pClientSession is NULL", error, BIP_ERR_INVALID_PARAMETER );

    BDBG_MSG(( "%s: pClientSession %p", __FUNCTION__, (void *)pClientSession ));
    pClientSession->livenessTimeoutTask( pClientSession );
error:
    return;
}

/**
 *  Function: This function will reschedule the liveness countdown task to indicate that the session is
 *  still alive.
 **/
void BIP_RtspServer::BIP_RtspClientSession:: noteLiveness()
{
    int64_t timeoutMicroSeconds = fOurServer->fReclamationTestSeconds;

    timeoutMicroSeconds *= 1000000;

    BDBG_MSG(( "BIP_RtspClientSession::%s: rescheduleDelayedTask(fLivenessCheckTask %p (%d secs); pSession (%p)", __FUNCTION__,
               fLivenessCheckTask, fOurServer->fReclamationTestSeconds, (void *)this ));
    envir().taskScheduler().rescheduleDelayedTask( fLivenessCheckTask, timeoutMicroSeconds, (TaskFunc *)&BIP_Rtsp_SessionLivenessCallback, this );
}

/**
 *  Function: This function will perform various tasks when a connection has timed out.
 **/
void BIP_RtspServer::BIP_RtspClientConnection:: livenessTimeoutTask(
    BIP_RtspClientConnection *clientConnection
    )
{
    HashTable::Iterator   *iter     = NULL;
    BIP_RtspClientSession *pSession = NULL;
    char const            *key      = NULL; // dummy

    // If this gets called, the connection is assumed to have timed out, so delete it:

    // loop throu all session and zero out the fClientConnection field
    BDBG_ERR(( "Connection:: (%p) has timed out (due to inactivity); fLivenessCheckTask %p", (void *)clientConnection, fLivenessCheckTask ));

    BIP_CHECK_PTR_GOTO( clientConnection, "clientConnection is invalid", error, BIP_ERR_INVALID_PARAMETER );
    BIP_CHECK_PTR_GOTO( clientConnection->fOurServer, "fOurServer is invalid", error, BIP_ERR_INVALID_PARAMETER );

    iter = HashTable::Iterator::create( *clientConnection->fOurServer->fClientSessions );
    while (( pSession = (BIP_RtspServer::BIP_RtspClientSession *)( iter->next( key ))) != NULL) {
        BDBG_MSG(( "%s: iterator session %p", __FUNCTION__, (void *)pSession ));
        if (pSession->fClientConnection)
        {
            delete pSession->fClientConnection;
            pSession->fClientConnection = NULL; // this connection is being deleted; do not let the session use it anymore
        }
    }
    delete iter;
    BDBG_MSG(( "%s: delete clientConnection (%p)", __FUNCTION__, (void *)clientConnection ));
    delete clientConnection;
error:
    return;
} // livenessTimeoutTask

/**
 *  Function: This function will perform various tasks when a session has timed out.
 **/
void BIP_RtspServer::BIP_RtspClientSession:: livenessTimeoutTask(
    BIP_RtspClientSession *clientSession
    )
{
    // If this gets called, the client session is assumed to have timed out, so delete it:
    char const *streamName = ( clientSession->fOurServerMediaSession == NULL ) ? "???" : clientSession->fOurServerMediaSession->streamName();

    BDBG_ERR(( "Session:: (id (%x); stream name (%s) has timed out (due to inactivity)", clientSession->fOurSessionId, streamName ));

    BIP_CHECK_PTR_GOTO( clientSession, "clientSession is invalid", error, BIP_ERR_INVALID_PARAMETER );

    /* if a callback has been registered */
    if (clientSession->fMessageReceivedCallback.callback)
    {
        u_int32_t          fakeInputSocket = socket( AF_INET, SOCK_STREAM, 0 );
        struct sockaddr_in fakeAddr;
        BKNI_Memset((void *)&fakeAddr, 0, sizeof ( fakeAddr ));

        clientSession->setState( BIP_RtspLmSessionStreamState_TimeoutPending );

#if ( USE_IGMP==1 )
        /* for multicast session, inform IGMP we are tearing down */
        if (clientSession->fIsMulticast && clientSession->transportStatus.clientAddressStr)
        {
            BDBG_MSG(( "%s: calling BIP_IgmpListener_DelGroupAddress; ipAddr %x; streamId %d", __FUNCTION__,
                       ntohl( clientSession->fDestinationAddress ), clientSession->fOurStreamId ));
            BIP_IgmpListener_DelGroupAddress( fOurServer->fIgmpListener, ntohl( clientSession->fDestinationAddress ));
        }
        else
        {
            BDBG_MSG(( "%s: BIP_IgmpListener_DelGroupAddress() ignored; streamId %d is unicast OR clientAddressStr is null (%s)", __FUNCTION__,
                       clientSession->fOurStreamId, clientSession->transportStatus.clientAddressStr ));
        }
#else   // if ( USE_IGMP==1 )
        BDBG_ERR(( "%s: DelGroupAddress(%x) commented out", __FUNCTION__, ntohl( clientSession->fDestinationAddress )));
#endif  // if ( USE_IGMP==1 )

        /* Because this callback happens outside the "connection" object, we have to create a temporary connection object. */
        /* fClientConnection is used in callback from the session object; need to make sure the connection is the right one */
        clientSession->fClientConnection               = new BIP_RtspClientConnection( *fOurServer, fakeInputSocket, fakeAddr );
        clientSession->fClientConnection->fIsTimingOut = True;
        snprintf((char *) clientSession->fClientConnection->fRequestBuffer, sizeof( clientSession->fClientConnection->fRequestBuffer )-1,
            "TEARDOWN stream=%d\r\n\r\n", clientSession->fOurStreamId );
        clientSession->fClientConnection->fLastCRLF =  clientSession->fClientConnection->fRequestBuffer + 19;
        BDBG_MSG(( "%s: triggering CALLBACK ... clientSession->fMessageReceivedCallback.context (%s)", __FUNCTION__, clientSession->fClientConnection->fRequestBuffer ));
        clientSession->fMessageReceivedCallback.callback( clientSession->fMessageReceivedCallback.context,
            strlen((char *) clientSession->fClientConnection->fRequestBuffer ));
    }
    else
    {
        BDBG_MSG(( "%s: no callback registered; delete session immediately", __FUNCTION__ ));
        delete clientSession;
    }
error:
    return;
} // livenessTimeoutTask

/**
 *  Function: This function will reschedule the liveness countdown task to indicate that the connection is still
 *  alive.
 **/
void BIP_RtspServer::BIP_RtspClientConnection:: noteLiveness()
{
    int64_t timeoutMicroSeconds = ( fOurServer->fReclamationTestSeconds+5 ); /* 5 seconds longer than session */

    timeoutMicroSeconds *= 1000000;

    BDBG_MSG(( "BIP_RtspClientConnection::%s: rescheduleDelayedTask(fLivenessCheckTask->%p); pConnection (%p); secs %d", __FUNCTION__,
               fLivenessCheckTask, (void *)this, fOurServer->fReclamationTestSeconds+5 ));
    envir().taskScheduler().rescheduleDelayedTask( fLivenessCheckTask, timeoutMicroSeconds,
        (TaskFunc *)&BIP_Rtsp_ConnectionLivenessCallback, this );
    BDBG_MSG(( "BIP_RtspClientConnection::%s: rescheduleDelayedTask(fLivenessCheckTask->%p); pConnection (%p)", __FUNCTION__, fLivenessCheckTask, (void *)this ));
}

/**
 *  Function: This function will trigger the liveness API when an OPTIONS request arrives with a specific
 *  sessionId attached to it. Normally, the noteLiveness() API is only called when it is associated with a
 *  specific client Session. This API handles the use case when a client uses the OPTIONS command to show
 *  liveness of the client. The OPTIONS command arrives as part of a client "connection" not a client "session".
 *  In that event, we have to find the associated client session and then call the session's liveness API.
 **/
void BIP_RtspServer::BIP_RtspClientConnection:: noteLiveness(
    const char *sessionIdStr
    )
{
    BIP_RtspServer::BIP_RtspClientSession *clientSession = NULL;

    BDBG_MSG(( "BIP_RtspClientConnection::%s: top; this %p; sessionIdStr (%s)", __FUNCTION__, (void *)this, sessionIdStr ));

    // If the request included a "Session:" id, and it refers to a client session that's current ongoing, then use this
    // command to indicate 'liveness' on that client session:
    if (sessionIdStr[0] != '\0')
    {
        clientSession = (BIP_RtspServer::BIP_RtspClientSession *)( fOurServer->fClientSessions->Lookup( sessionIdStr ));
        if (clientSession != NULL)
        {
            clientSession->noteLiveness();
        }
    }
    return;
} // noteLiveness

void BIP_RtspServer::BIP_RtspClientConnection::  copyReceivedMessageConnection(
    char *dstBuffer
    )
{
    BIP_Status rc;

    BIP_CHECK_GOTO(( dstBuffer ), ( "Destination Buffer is Null for copyReceivedMessageConnection " ), error, BIP_ERR_INVALID_PARAMETER, rc );
    if (fRequestBuffer[0] != '\0' && (( fLastCRLF+2 - fRequestBuffer ) > 0 ))
    {
        BDBG_MSG(( "%s: connecton %p; calling memcpy(%p, %p, len = %d [%p+2-%p])", __FUNCTION__, (void *)this,
                   dstBuffer, fRequestBuffer, (int)(fLastCRLF+2 - fRequestBuffer), fLastCRLF, fRequestBuffer ));
        memcpy( dstBuffer, fRequestBuffer, ( fLastCRLF+2 - fRequestBuffer ));
    }
    else
    {
        BDBG_MSG(( "%s: connecton %p; something invalid ... skipping memcpy(%p, %p, len = %d (%p+2-%p)", __FUNCTION__, (void *)this,
                   dstBuffer, fRequestBuffer, (int)(fLastCRLF+2 - fRequestBuffer), fLastCRLF, fRequestBuffer ));
        dstBuffer[0] = '\0';
    }

error:

    return;
} // copyReceivedMessageConnection

void BIP_RtspServer::BIP_RtspClientSession::  copyReceivedMessageSession(
    char *dstBuffer
    )
{
    BDBG_MSG(( "%s: fClientConnection %p", __FUNCTION__, (void *)fClientConnection ));
    if (fClientConnection)
    {
        BDBG_MSG(( "%s: calling fClientConnection->copyReceivedMessage() -> %p", __FUNCTION__, dstBuffer ));
        fClientConnection->copyReceivedMessageConnection( dstBuffer );
    }
    return;
}

int BIP_RtspServer::BIP_RtspClientSession::  setState(
    BIP_RtspLmSessionStreamState state
    )
{
    char stateStr[32];

    fState = state;

    BKNI_Memset( stateStr, 0, sizeof( stateStr ));
    ( fState == BIP_RtspLmSessionStreamState_Idle ) ? strncpy( stateStr, "Idle", sizeof( stateStr )-1 ) :
    ( fState == BIP_RtspLmSessionStreamState_UnicastSetupPending ) ? strncpy( stateStr, "UnicastSetupPending", sizeof( stateStr )-1 ) :
    ( fState == BIP_RtspLmSessionStreamState_UnicastSetup ) ? strncpy( stateStr, "UnicastSetup", sizeof( stateStr )-1 ) :
    ( fState == BIP_RtspLmSessionStreamState_UnicastPlayPending ) ? strncpy( stateStr, "UnicastPlayPending", sizeof( stateStr )-1 ) :
    ( fState == BIP_RtspLmSessionStreamState_UnicastPlay ) ? strncpy( stateStr, "UnicastPlay", sizeof( stateStr )-1 ) :
    ( fState == BIP_RtspLmSessionStreamState_MulticastSetupPending ) ? strncpy( stateStr, "MulticastSetupPending", sizeof( stateStr )-1 ) :
    ( fState == BIP_RtspLmSessionStreamState_MulticastSetup ) ? strncpy( stateStr, "MulticastSetup", sizeof( stateStr )-1 ) :
    ( fState == BIP_RtspLmSessionStreamState_MulticastPlayPending ) ? strncpy( stateStr, "MulticastPlayPending", sizeof( stateStr )-1 ) :
    ( fState == BIP_RtspLmSessionStreamState_MulticastPlay ) ? strncpy( stateStr, "MulticastPlay", sizeof( stateStr )-1 ) :
    ( fState == BIP_RtspLmSessionStreamState_IgmpJoinPending ) ? strncpy( stateStr, "IgmpJoinPending", sizeof( stateStr )-1 ) :
    ( fState == BIP_RtspLmSessionStreamState_IgmpJoin ) ? strncpy( stateStr, "IgmpJoin", sizeof( stateStr )-1 ) :
    ( fState == BIP_RtspLmSessionStreamState_IgmpJoinStreaming ) ? strncpy( stateStr, "IgmpJoinStreaming", sizeof( stateStr )-1 ) :
    ( fState == BIP_RtspLmSessionStreamState_IgmpLeavePending ) ? strncpy( stateStr, "IgmpLeavePending", sizeof( stateStr )-1 ) :
    ( fState == BIP_RtspLmSessionStreamState_TeardownPending ) ? strncpy( stateStr, "TeardownPending", sizeof( stateStr )-1 ) :
    ( fState == BIP_RtspLmSessionStreamState_TimeoutPending ) ? strncpy( stateStr, "TimeoutPending", sizeof( stateStr )-1 ) :
    ( fState == BIP_RtspLmSessionStreamState_UnicastChannelChangePending ) ? strncpy( stateStr, "UnicastCCPending", sizeof( stateStr )-1 ) :
    ( fState == BIP_RtspLmSessionStreamState_MulticastChannelChangePending ) ? strncpy( stateStr, "MulticastCCPending", sizeof( stateStr )-1 ) :
        strncpy( stateStr, "UNKNOWN", sizeof( stateStr )-1 );
    BDBG_MSG(( "%s: for session %p (%08lX), state is %s; streamId %d", __FUNCTION__, (void *)this, this->satelliteSettings.owner, stateStr,
               satelliteSettings.streamId ));
    return( 0 );
} // setState

BIP_RtspLmSessionStreamState BIP_RtspServer::BIP_RtspClientSession::  getState(
    void
    )
{
#if 0
    sprintf( currentState, "%s", ( fState == BIP_RtspLmSessionStreamState_Idle ) ? "Idle" :
        ( fState == BIP_RtspLmSessionStreamState_UnicastSetupPending ) ? "UnicastSetupPending" :
        ( fState == BIP_RtspLmSessionStreamState_UnicastSetup ) ? "UnicastSetup" :
        ( fState == BIP_RtspLmSessionStreamState_UnicastPlayPending ) ? "UnicastPlayPending" :
        ( fState == BIP_RtspLmSessionStreamState_UnicastPlay ) ? "UnicastPlay" :
        ( fState == BIP_RtspLmSessionStreamState_MulticastSetupPending ) ? "MulticastSetupPending" :
        ( fState == BIP_RtspLmSessionStreamState_MulticastSetup ) ? "MulticastSetup" :
        ( fState == BIP_RtspLmSessionStreamState_MulticastPlayPending ) ? "MulticastPlayPending" :
        ( fState == BIP_RtspLmSessionStreamState_MulticastPlay ) ? "MulticastPlay" :
        ( fState == BIP_RtspLmSessionStreamState_IgmpJoinPending ) ? "IgmpJoinPending" :
        ( fState == BIP_RtspLmSessionStreamState_IgmpJoin ) ? "IgmpJoin" :
        ( fState == BIP_RtspLmSessionStreamState_IgmpJoinStreaming ) ? "IgmpJoinStreaming" :
        ( fState == BIP_RtspLmSessionStreamState_IgmpLeavePending ) ? "IgmpLeavePending" :
        ( fState == BIP_RtspLmSessionStreamState_TeardownPending ) ? "TeardownPending" :
        ( fState == BIP_RtspLmSessionStreamState_Teardown ) ? "Teardown" : "UNKNOWN" );
    /* putting above imbedded if structure directly in BDBG_ERR() macro causes it to output "MISSING" for some reason */
    BDBG_ERR(( "%s: state is %s", __FUNCTION__, currentState ));
#endif // if 0
    BDBG_MSG(( "%s: state is %d", __FUNCTION__, fState ));
    return( fState );
} // getState
