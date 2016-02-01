/******************************************************************************
 * (c) 2015 Broadcom Corporation
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

#include "bip_priv.h"   /* Include this first! */

#include "BasicUsageEnvironment.hh"

#include "bip_rtsp_lm_server.h"
#include "liveMedia/bip_rtsp_lm_server.h"

BDBG_MODULE( bip_rtsp_lm_listener );
BDBG_OBJECT_ID( BIP_RtspLiveMediaListener );

typedef struct BIP_RtspLiveMediaListener
{
    BDBG_OBJECT( BIP_RtspLiveMediaListener )
    BIP_RtspLiveMediaListenerCreateSettings createSettings;
    BIP_RtspLiveMediaListenerSettings settings;
    BIP_RtspServer                   *lmRtspServer;
    TaskScheduler                    *lmScheduler;
    UsageEnvironment                 *lmEnv;
    B_ThreadHandle                    lmSchedulerThread;
    char stopLmSchedulerLoop;
} BIP_RtspLiveMediaListener;

void
BIP_RtspLiveMediaListener_GetDefaultCreateSettings(
    BIP_RtspLiveMediaListenerCreateSettings *pSettings
    )
{
    BKNI_Memset( pSettings, 0, sizeof( BIP_RtspLiveMediaListenerCreateSettings ));
    pSettings->port = 554;
}

static void
rtspLiveMediaListenerDestroy(
    BIP_RtspLiveMediaListenerHandle hRtspLmListener
    )
{
    if (!hRtspLmListener)
    {
        return;
    }

    /* destroy the session object. this will cause the livemedia destructors to execute */
    if (hRtspLmListener->lmRtspServer) {delete hRtspLmListener->lmRtspServer; }

    /* TODO: lmEnv cleanup ? */
    if (hRtspLmListener->lmScheduler) {delete hRtspLmListener->lmScheduler; }

    BDBG_OBJECT_DESTROY( hRtspLmListener, BIP_RtspLiveMediaListener );
    BKNI_Free( hRtspLmListener );
}

BIP_RtspLiveMediaListenerHandle
BIP_RtspLiveMediaListener_Create(
    BIP_RtspLiveMediaListenerCreateSettings *pSettings
    )
{
    BIP_RtspLiveMediaListenerHandle         hRtspLmListener;
    BIP_RtspLiveMediaListenerCreateSettings defaultCreateSettings;

    if (NULL == pSettings)
    {
        BIP_RtspLiveMediaListener_GetDefaultCreateSettings( &defaultCreateSettings );
        pSettings = &defaultCreateSettings;
    }

    hRtspLmListener = (BIP_RtspLiveMediaListenerHandle) BKNI_Malloc( sizeof( *hRtspLmListener ));
    BIP_CHECK_PTR_GOTO( hRtspLmListener, "Memory Allocation Failed", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );
    BKNI_Memset( hRtspLmListener, 0, sizeof( *hRtspLmListener ));
    BDBG_OBJECT_SET( hRtspLmListener, BIP_RtspLiveMediaListener );
    BKNI_Memset( &hRtspLmListener->settings, 0, sizeof( BIP_RtspLiveMediaListenerSettings ));
    hRtspLmListener->createSettings = *pSettings;

    /* Create the LiveMedia scheduler & basic env, all RTSP socket related processing happens in this context */
    hRtspLmListener->lmScheduler = BasicTaskScheduler::createNew();
    BIP_CHECK_PTR_GOTO( hRtspLmListener->lmScheduler, "Memory Allocation Failed", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );
    hRtspLmListener->lmEnv = BasicUsageEnvironment::createNew( *hRtspLmListener->lmScheduler );
    BIP_CHECK_PTR_GOTO( hRtspLmListener->lmEnv, "Memory Allocation Failed", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );

    /* Create the LiveMedia RtspServer Class object */
    {
#define MULTICAST_ADDRESS_TEMPLATE "239.1.1."
        portNumBits rtspServerPortNum        = pSettings->port;
        hRtspLmListener->lmRtspServer = BIP_RtspServer::createNew( *hRtspLmListener->lmEnv, rtspServerPortNum, NULL,
                pSettings->rtspSessionTimeout, (char*)MULTICAST_ADDRESS_TEMPLATE );
        BIP_CHECK_PTR_GOTO( hRtspLmListener->lmRtspServer, "Memory Allocation Failed", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );
    }

    BDBG_MSG(( "%s: hRtspLmListener %p", __FUNCTION__, hRtspLmListener ));

    /* Pass IGMP Listener Handle to BIP_RtspServer Object */
    hRtspLmListener->lmRtspServer->setIgmpListenerHandle( pSettings->hIgmpListener );
    hRtspLmListener->lmRtspServer->setIgmpCallbacks();

    return( hRtspLmListener );

error:

    rtspLiveMediaListenerDestroy( hRtspLmListener );

    return( NULL );
} // BIP_RtspLiveMediaListener_Create

void
BIP_RtspLiveMediaListener_Destroy(
    BIP_RtspLiveMediaListenerHandle hRtspLmListener
    )
{
    BDBG_OBJECT_ASSERT( hRtspLmListener, BIP_RtspLiveMediaListener );

    rtspLiveMediaListenerDestroy( hRtspLmListener );
}

void
BIP_RtspLiveMediaListener_GetSettings(
    BIP_RtspLiveMediaListenerHandle    hRtspLmListener,
    BIP_RtspLiveMediaListenerSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT( hRtspLmListener, BIP_RtspLiveMediaListener );
    BDBG_ASSERT( pSettings );
    *pSettings = hRtspLmListener->settings;
}

BIP_Status
BIP_RtspLiveMediaListener_SetSettings(
    BIP_RtspLiveMediaListenerHandle    hRtspLmListener,
    BIP_RtspLiveMediaListenerSettings *pSettings
    )
{
    BIP_Status errCode = BIP_SUCCESS;

    BDBG_OBJECT_ASSERT( hRtspLmListener, BIP_RtspLiveMediaListener );
    BDBG_ASSERT( pSettings );

    /* just pass thru the settings to the actual C++ class: BIP_RtspServer */

    /* set the callback with the liveMedia object */
    hRtspLmListener->lmRtspServer->setConnectedCallback( pSettings->connectedCallback );
    hRtspLmListener->lmRtspServer->setGetRtpStatisticsCallback( pSettings->getRtpStatisticsCallback );
    return( errCode );
}

static void
lmSchedulerThread(
    void *data
    )
{
    BIP_RtspLiveMediaListenerHandle hRtspLmListener = (BIP_RtspLiveMediaListenerHandle)data;

    /* Note: this function doesn't return until we stop the loop */
    hRtspLmListener->lmEnv->taskScheduler().doEventLoop( &hRtspLmListener->stopLmSchedulerLoop );

    /* TODO: Send an event to the main thread that initiated the stop and is waiting for this thread to terminate */
}

BIP_Status
BIP_RtspLiveMediaListener_Start(
    BIP_RtspLiveMediaListenerHandle hRtspLmListener
    )
{
    BIP_Status errCode = BIP_ERR_INTERNAL;

    if (NULL == hRtspLmListener->lmSchedulerThread)
    {
        /* create a thread to carryout the LiveMedia eventLoop processing */
        hRtspLmListener->lmSchedulerThread = B_Thread_Create( "BipLMRtspSrvr", (B_ThreadFunc)lmSchedulerThread, (void *)hRtspLmListener, NULL );
        BIP_CHECK_PTR_GOTO( hRtspLmListener->lmSchedulerThread, "Thread Allocation Failed", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );
    }

    /* settings this flag will cause the liveMedia event loop to start/resume */
    hRtspLmListener->stopLmSchedulerLoop = 0;

    /* start IGMP listener */
    BIP_IgmpListener_Start( hRtspLmListener->createSettings.hIgmpListener );

    errCode = BIP_SUCCESS;
    return( errCode );

error:
    return( errCode );
} // BIP_RtspLiveMediaListener_Start

void
BIP_RtspLiveMediaListener_Stop(
    BIP_RtspLiveMediaListenerHandle hRtspLmListener
    )
{
    /* settings this flag will cause the liveMedia event loop to terminate and return */
    hRtspLmListener->stopLmSchedulerLoop = 1;

    /* TODO: wait on an event from the lmSchedulerThread to know that the thread is done */
    /* Then destroy the thread */
    /* for now, just adding a sleep here */
    BKNI_Sleep( 1000 );

    B_Thread_Destroy( hRtspLmListener->lmSchedulerThread );
    hRtspLmListener->lmSchedulerThread = NULL;

    /* TODO: should this thread be destroyed during _Destroy() ? */
}

BIP_RtspLiveMediaSocketHandle
BIP_RtspLiveMediaListener_CreateSocket(
    BIP_RtspLiveMediaListenerHandle hRtspLmListener,
    int                             socketFd
    )
{
    BIP_RtspLiveMediaSocketHandle rtspLmSocket;

    BDBG_OBJECT_ASSERT( hRtspLmListener, BIP_RtspLiveMediaListener );

    rtspLmSocket = BIP_RtspLiveMediaSocket_CreateFromFd( socketFd, hRtspLmListener->lmRtspServer, NULL /*pSettings*/ );

    BIP_CHECK_PTR_GOTO( rtspLmSocket, "Memory Allocation Failed", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );

    return( rtspLmSocket );

error:
    return( NULL );
}
