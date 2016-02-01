/***************************************************************************
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
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bstd.h"
#include "bkni.h"
#include "blst_list.h"
#include "blst_queue.h"
#include <netinet/ip.h>
#include <arpa/inet.h>
#include "nexus_platform.h"

#include "bip.h"
#include "bip_igmp_listener.h"

/*Global */
static bool gGotSigInt  = 0;
int         gExitThread = 0;

void signalHandler(
    int signal
    )
{
    printf( "Got SIGINT (%d): Cleaning up!!!", signal );
    gGotSigInt = 1;
}

int userInputCheck(void)
{
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); /*STDIN_FILENO is 0*/
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

typedef struct AppCtx
{
    BIP_IgmpListenerHandle igmpListener;
} AppCtx;

BDBG_MODULE( igmp_server );

static void igmpListenerMembershipReportCallback(
    void *context,
    int   param
    )
{
    AppCtx *appCtx = context;
    BIP_IgmpListener_MembershipReportStatus_T pStatus;

    BSTD_UNUSED( param );

    while (!BIP_IgmpListener_GetMembershipReportStatus(        appCtx->igmpListener,        &pStatus ))
    {
        unsigned int multicast;
        BIP_IgmpListener_MembershipReportType mRepType;
        multicast = htonl( pStatus.multicast_address );
        mRepType  = pStatus.memRepType;
        BDBG_MSG(( "%s:In App igmpListenerJoinCallback status %s membershipReport Status %d...", __FUNCTION__, inet_ntoa( *(struct in_addr *)&( multicast )), mRepType ));
        /* app then must decide if they want to remove monitoring the mutlicast address by giving at BIP_IgmpListener_RemoveGroupAddress  */
    }
} /* igmpListenerMembershipReportCallback */
int initNexus(
    AppCtx *appCtx
    )
{
    static NEXUS_PlatformSettings platformSettings;
    BSTD_UNUSED( appCtx );
    NEXUS_Platform_GetDefaultSettings( &platformSettings );
    /* TODO: take this out as we need the frontends for the live sources */
    platformSettings.openFrontend = true;
    return( NEXUS_Platform_Init( &platformSettings ));
}

int init(
    AppCtx *appCtx
    )
{
    BIP_IgmpListenerCreateSettings igmpListenerCreateSettings;
    BIP_IgmpListenerSettings       igmpListenerSettings;

    /**
       * create listener for RTSP based streaming sessions
       **/
    BIP_IgmpSesSatIpListener_GetDefaultCreateSettings( &igmpListenerCreateSettings );

    appCtx->igmpListener = BIP_IgmpListener_Create( &igmpListenerCreateSettings );

    /* set callback  settings */
    BIP_IgmpListener_GetSettings( appCtx->igmpListener, &igmpListenerSettings );
    igmpListenerSettings.membershipReportCallback.callback = igmpListenerMembershipReportCallback;
    igmpListenerSettings.membershipReportCallback.context  = appCtx;
    BIP_IgmpListener_SetSettings( appCtx->igmpListener, &igmpListenerSettings );

    /* start listening for new requests */
    BIP_IgmpListener_Start( appCtx->igmpListener );

    return( 0 );
} /* init */

int main(
    int    argc,
    char **argv
    )
{
    AppCtx *appCtx;

    BSTD_UNUSED( argc );
    BSTD_UNUSED( argv );

    signal( SIGINT, signalHandler );

    BKNI_Init();
    B_Os_Init();
    appCtx = BKNI_Malloc( sizeof( AppCtx ));
    initNexus( appCtx );
    init( appCtx );
    BKNI_Sleep( 100 );

    /* Populate the list */
    BIP_IgmpListener_AddGroupAddress( appCtx->igmpListener, ntohl( inet_addr( "239.1.1.1" )), 1 );
    BIP_IgmpListener_AddGroupAddress( appCtx->igmpListener, ntohl( inet_addr( "239.1.1.2" )), 2 );



    BIP_IgmpListener_DelGroupAddress(appCtx->igmpListener,  ntohl(inet_addr("239.1.1.2")));

    BIP_IgmpListener_AddGroupAddress( appCtx->igmpListener, ntohl( inet_addr( "239.1.1.2" )), 2 );
    BIP_IgmpListener_AddGroupAddress( appCtx->igmpListener, ntohl( inet_addr( "239.1.1.3" )), 2 );
    BIP_IgmpListener_AddGroupAddress( appCtx->igmpListener, ntohl( inet_addr( "239.1.1.2" )), 2 );


    BIP_IgmpListener_DelGroupAddress(appCtx->igmpListener,  ntohl(inet_addr("239.1.1.1")));

    while(!gExitThread)
    {
        if(userInputCheck())
        {

           char buffer[256];
           char *buf;

           BDBG_MSG(("Recieved UserInput")); fflush(stdout);
           fgets(buffer, 256, stdin);
           if (feof(stdin)) break;
           buffer[strlen(buffer)-1] = 0; /* chop off \n */

           buf = strtok(buffer, ";");
           if (!buf) continue;
           if (!strcmp(buf, "q") || !strcmp(buf, "quit")) {
              BDBG_ERR(("recieved quit "));
              gExitThread = true;
           }
        }


        BKNI_Sleep( 1000 );
    }

    /* Uninit */
    BIP_IgmpListener_Stop( appCtx->igmpListener );

    BIP_IgmpListener_Destroy( appCtx->igmpListener );

    BKNI_Free(appCtx);
    NEXUS_Platform_Uninit();
    B_Os_Uninit();
    BKNI_Uninit();

    return( 0 );
} /* main */
