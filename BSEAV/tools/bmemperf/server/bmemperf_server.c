/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.

 ******************************************************************************/

#include "bmemperf_types64.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "bstd.h"
#include "bmemperf_server.h"
#include "bmemperf_cgi.h"
#include "bmemperf_utils.h"
#include "nexus_platform.h"
#include "bchp_common.h"
#include "bmemperf_powerprobe.h"

#define PRINTF2                      noprintf
#define MAXHOSTNAME                  80
#define PERF_FILE_FULL_PATH_LEN      64
#define SATA_USB_FILE_FULL_PATH_LEN  64
#define LINUX_TOP_FILE_FULL_PATH_LEN 64
#define GETPID                       (unsigned long)getpid()

static void reusePort( int sock );

static unsigned char Quit = 0;

static bool                    initializationDone = false;
static unsigned int            g_client_list[BMEMPERF_NUM_MEMC][BMEMPERF_MAX_SUPPORTED_CLIENTS]; /* BMEMPERF_CGI_INVALID indicates no client id is desired */
static bmemperf_client_data    g_savedClientDataSorted[BMEMPERF_NUM_MEMC][BMEMPERF_MAX_NUM_CLIENT];
static bmemperf_irq_data       g_savedIrqData;
static unsigned long int       g_ContextSwitchesPrev = 0;
static unsigned long int       g_ContextSwitchesDelta = 0;
static bmemperf_boxmode_source g_boxmodeSource = {0, 0};
int   g_MegaBytes           = 0; /* set to 1 when user wants data displayed in megabytes instead of megabits (default) */
int   g_MegaBytesDivisor[2] = {1, 8};
char *g_MegaBytesStr[2] = {"Mbps", "MBps"};
bsysperf_netStatistics g_netStats[NET_STATS_MAX];
int                    g_netStatsIdx = -1;                 /* index to entries added to g_netStats array */

typedef struct
{
    char deviceName[32];
} bmemperf_device_name;

typedef struct
{
    long int          uptime;
    unsigned int      major;
    unsigned int      minor;
    unsigned long int readsCompletedSuccessfully;
    unsigned long int readsMerged;
    unsigned long int readsSectors;
    unsigned long int readsMilliseconds;
    unsigned long int writesCompletedSuccessfully;
    unsigned long int writesMerged;
    unsigned long int writesSectors;
    unsigned long int writesMilliseconds;
} bmemperf_device_data;

typedef struct
{
    float readMbps;
    float writeMbps;
} bmemperf_device_mbps;

#define BMEMPERF_SATA_USB_HISTORY_MAX 10
static pthread_t         gSataUsbThreadId = 0;
static bool              gSataUsbFirstPassAfterInit = false;
static unsigned long int gSataUsbTimeout  = 0; /* gather function increments it; request function clears it out; if we reach 5, stop gathering */
bmemperf_device_name     gSataUsbDeviceNames[BMEMPERF_SATA_USB_MAX];
bmemperf_device_data     gSataUsbDataPrev[BMEMPERF_SATA_USB_MAX];
bmemperf_device_data     gSataUsbDataNow[BMEMPERF_SATA_USB_MAX];
bmemperf_device_mbps     gSataUsbMbps[BMEMPERF_SATA_USB_MAX];
bmemperf_device_mbps     gSataUsbMbpsHistory[BMEMPERF_SATA_USB_MAX][BMEMPERF_SATA_USB_HISTORY_MAX];
unsigned int             gSataUsbMbpsHistoryIdx = 0;
pthread_mutex_t          gSataUsbMutex;
static pthread_t         gPowerProbeThreadId = 0;
static unsigned long int gPowerProbeTelnet_pid = 0;
static char              gPowerProbeIpAddr[INET6_ADDRSTRLEN];
static unsigned char     gPowerProbeShunts[POWER_PROBE_MAX];
static float             gPowerProbeVoltage[POWER_PROBE_MAX];
static float             gPowerProbeCurrent[POWER_PROBE_MAX];
static unsigned char     gPowerProbeConnected[POWER_PROBE_MAX];
static unsigned char     gPowerProbeQuitCount = 0;
static char              gServerStreamerIpAddr[INET6_ADDRSTRLEN];

typedef struct
{
    pthread_t         ThreadId;
    unsigned long int Telnet_pid;
    char              IpAddr[INET6_ADDRSTRLEN];
    unsigned char     QuitCount;
    long int          Action; /* 0 => do nothing; 1 => add a stream; 2 stop LIFO stream */
    int               Reset;
    unsigned long int ActiveThreadIndex;
    unsigned long int ActiveThreadPids[CLIENT_STREAMER_THREAD_MAX];
    unsigned long int ActiveThreadReferers[CLIENT_STREAMER_THREAD_MAX];
    char              LogFile[64];
} REMOTE_CLIENT_T;
static REMOTE_CLIENT_T gRemoteClient[BASPMON_MAX_NUM_CLIENTS];

/**
 *  Function: This function will initialize the specified mutex variable.
 **/
int Bmemperf_Server_InitMutex(
    pthread_mutex_t *mutex
    )
{
    if (pthread_mutex_init( mutex, NULL ))
    {
        printf( "%s: pthread_mutex_init failure; \n", __FUNCTION__ );
        return( 1 );
    }
    else
    {
        return( 0 );
    }
}

/**
 *  Function: This function will lock the specified mutex variable.
 **/
int Bmemperf_Server_LockMutex(
   pthread_mutex_t *mutex
    )
{
    int rc = 0;

    rc = pthread_mutex_lock( mutex );
    return( rc );
}

/**
 *  Function: This function will unlock the specified mutex variable.
 **/
void Bmemperf_Server_UnlockMutex(
   pthread_mutex_t *mutex
    )
{
    if (pthread_mutex_unlock( mutex ))
    {
        fprintf( stderr, "pthread_mutex_unlock failed" );
        assert( 0 );
    }
    return;
}

/**
 *  Function: This function will uninit the specified mutex variable.
 **/
void Bmemperf_Server_UninitMutex(
   pthread_mutex_t *mutex
    )
{
    pthread_mutex_destroy( mutex );
    return;
}

/**
 *  Function: This function is the prototype for Bmemperf_ReadRequest() API.
 **/
static int Bmemperf_ReadRequest(
    int                psd,
    struct sockaddr_in from,
    bmemperf_request  *pRequest,
    bmemperf_response *pResponse
    );

/**
 *  Function: This function will close the specified socket and exit the app.
 **/
static int CloseAndExit(
    int         socketFd,
    const char *reason
    )
{
    PRINTF( "%s: socket %d; reason (%s)\n", __FUNCTION__, socketFd, reason );
    if (socketFd>0) {close( socketFd ); }
    exit( 0 );
}

/**
 *  Function: This function will open a connection to a specific port that will be used to receive and send data
 *  from the user's browser.
 **/
static int startServer(
    void
    )
{
    int                fromlen;
    int                struct_length;
    int                pn;
    int                sd           = 0, psd = 0;
    unsigned long int  requestCount = 0;
    struct sockaddr_in server;
    struct sockaddr_in from;
    bmemperf_request   request;
    bmemperf_response  response;

    /* initialize the boxmode information (i.e. does it come from env variable or /proc file system */
    memset( &g_boxmodeSource, 0, sizeof( g_boxmodeSource ));
    get_boxmode( &g_boxmodeSource );
    printf( "Changing boxmode to %d\n", g_boxmodeSource.boxmode );
    bmemperf_boxmode_init( g_boxmodeSource.boxmode );

    /* Construct name of socket */
    server.sin_family      = AF_INET;
    server.sin_addr.s_addr = htonl( INADDR_ANY );
    pn                     = htons( BMEMPERF_SERVER_PORT );
    server.sin_port        = pn;

    /* Create socket on which to send and receive */

    sd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

    if (sd < 0)
    {
        perror( "opening stream socket" );
        exit( -1 );
    }

    /* this allows the server to re-start quickly instead of fully wait for TIME_WAIT which can be as large as 2 minutes */
    reusePort( sd );

    if (bind( sd, (struct sockaddr *)&server, sizeof( server )) < 0)
    {
        CloseAndExit( sd, "binding name to stream socket" );
    }

    /* get port information and  prints it out */
    struct_length = sizeof( server );

    if (getsockname( sd, (struct sockaddr *)&server, (unsigned int *)&struct_length ))
    {
        CloseAndExit( sd,  "getting socket name" );
    }
    PRINTF( "%s - Server Port is: %d\n", __FUNCTION__, ntohs( server.sin_port ));

    /* accept TCP connections from clients and fork a process to serve each */
    listen( sd, 4 );
    fromlen = sizeof( from );

    while (Quit == 0) {
        PRINTF( "%s - Waiting to accept socket connections; Quit %u; requestCount %lu\n", __FUNCTION__, Quit, requestCount );
        psd = accept( sd, (struct sockaddr *)&from, (unsigned int *)&fromlen );
        requestCount++;
        memset( &request, 0, sizeof( request ));
        memset( &response, 0, sizeof( response ));
        Bmemperf_ReadRequest( psd, from, &request, &response );
    }

    CloseAndExit( sd, "quit detected" );

    return( 0 );
} /* startServer */

static int addClientIdToList(
    unsigned int client_id,
    unsigned int request_memc
    )
{
    unsigned int client;

    if (( request_memc < BMEMPERF_NUM_MEMC ) && ( client_id < BMEMPERF_MAX_NUM_CLIENT ))
    {
        /* loop through global list to add the user's request */
        for (client = 0; client<BMEMPERF_MAX_SUPPORTED_CLIENTS; client++)
        {
            PRINTF( "%s: g_client_list[%u][%u] is %u\n", __FUNCTION__, request_memc, client, g_client_list[request_memc][client] );
            /* if client id is already in the table */
            if (g_client_list[request_memc][client] == client_id)
            {
                PRINTF( "%s: for memc %u, client_id %u is already in the global table at idx %u\n", __FUNCTION__, request_memc, client_id, client );
                break;
            }
            else if (g_client_list[request_memc][client] == BMEMPERF_CGI_INVALID)
            {
                PRINTF( "%s: for memc %u, client_id %u added to the global table at idx %u\n", __FUNCTION__, request_memc, client_id, client );
                g_client_list[request_memc][client] = client_id;
                break;
            }
        }
    }
    else
    {
        printf( "%s: memc (%u) or client_id (%u) is invalid\n", __FUNCTION__, request_memc, client_id );
    }
    return( 0 );
} /* addClientIdToList */

static int deleteClientIdFromList(
    unsigned int client_id,
    unsigned int request_memc
    )
{
    unsigned int client, remaining_client;

    if (( request_memc < BMEMPERF_NUM_MEMC ) && ( client_id < BMEMPERF_MAX_NUM_CLIENT ))
    {
        /* loop through global list to delete the user's request */
        for (client = 0; client<BMEMPERF_MAX_SUPPORTED_CLIENTS; client++)
        {
            PRINTF( "%s: g_client_list[%u][%u] is %u\n", __FUNCTION__, request_memc, client, g_client_list[request_memc][client] );
            /* if client id is already in the table */
            if (g_client_list[request_memc][client] == client_id)
            {
                g_client_list[request_memc][client] = BMEMPERF_CGI_INVALID;
                PRINTF( "%s: for memc %u, client_id %u has been deleted from the global table at idx %u\n", __FUNCTION__, request_memc,
                    client_id, client );

                /* now move any remaining clients up one row*/
                for (remaining_client = client; remaining_client<BMEMPERF_MAX_SUPPORTED_CLIENTS-1; remaining_client++)
                {
                    PRINTF( "%s: g_client_list[%u][%u] replaced with %u\n", __FUNCTION__, request_memc, remaining_client,
                        g_client_list[request_memc][remaining_client+1] );
                    g_client_list[request_memc][remaining_client] = g_client_list[request_memc][remaining_client+1];
                }
                /* tag the last entry as unused */
                g_client_list[request_memc][BMEMPERF_MAX_SUPPORTED_CLIENTS-1] = BMEMPERF_CGI_INVALID;
                PRINTF( "%s: g_client_list[%u][%u] replaced with %u\n", __FUNCTION__, request_memc, BMEMPERF_MAX_SUPPORTED_CLIENTS-1, BMEMPERF_CGI_INVALID );

                /* we found the one to delete; don't look any further in the list */
                break;
            }
            else if (g_client_list[request_memc][client] == BMEMPERF_CGI_INVALID)
            {
                /* no need to look any further */
                break;
            }
        }
        PRINTF( "%s: after deletion: ", __FUNCTION__ );
        for (client = 0; client<BMEMPERF_MAX_SUPPORTED_CLIENTS; client++)
        {
            int temp = g_client_list[request_memc][client];
            PRINTF( "%u:%-3d ", client, temp );
        }
        PRINTF( "\n" );
    }
    else
    {
        printf( "%s: memc (%u) or client_id (%u) is invalid\n", __FUNCTION__, request_memc, client_id );
    }
    return( 0 );
} /* deleteClientIdFromList */

static unsigned char setClientErrCnt(
    unsigned int  memc,
    unsigned int  client_id,
    unsigned char newValue
    )
{
    unsigned int client;

    for (client = 0; client<BMEMPERF_MAX_NUM_CLIENT; client++)
    {
        if (g_savedClientDataSorted[memc][client].client_id == client_id)
        {
            PRINTF( "%s: match -> g_savedClientDataSorted[%u][%u] <- %u for client_id %u\n", __FUNCTION__, memc, client, newValue, client_id );
            g_savedClientDataSorted[memc][client].err_cnt = newValue;
            return( 0 );
        }
    }

    return( 0 );
}

/**
 *  Function: This function will determine if the specified client had a block out error in the previous pass.
 *  If an error is detected, its error counter will be incremented. The error count will max out at UCHAR_MAX.
 **/
static int updateClientErrCnt(
    unsigned int client2,
    unsigned int memc
    )
{
    if (bmemperf_get_arb_err( g_savedClientDataSorted[memc][client2].client_id, memc ))
    {
        if (( g_savedClientDataSorted[memc][client2].err_cnt < UCHAR_MAX ) && ( g_savedClientDataSorted[memc][client2].rr == 0 ))
        {
            g_savedClientDataSorted[memc][client2].err_cnt++;
            #if 0
            printf( "error detected for client %3u in memc %u; client_id %3u; bo_val %u (%p)\n", client2, memc, g_savedClientDataSorted[memc][client2].client_id,
                g_savedClientDataSorted[memc][client2].block_out, &( g_savedClientDataSorted[memc][client2].err_cnt ));
            #endif
        }
    }

    return( 0 );
}

/**
 *  Function: When the web page is first displayed, we display a list of clients based on the bandwidth values
 *  during that first pass. Subsequent passes, we want to display the client list in the same order as this
 *  first pass through. In order to keep this first-pass list, we have to search the first-pass list for the
 *  client that we are needing to update.
 **/
static int updateClient(
    unsigned int          client_id,
    unsigned int          memc,
    bmemperf_client_data *pClientData
    )
{
    unsigned int client2;

    /* find this client's entry in the saved array */
    for (client2 = 0; client2<BMEMPERF_MAX_NUM_CLIENT; client2++)
    {
        if (g_savedClientDataSorted[memc][client2].client_id == client_id)
        {
            g_savedClientDataSorted[memc][client2].bw          = pClientData->bw;
            g_savedClientDataSorted[memc][client2].is_detailed = pClientData->is_detailed;
            g_savedClientDataSorted[memc][client2].rr          = pClientData->rr;
            g_savedClientDataSorted[memc][client2].block_out   = pClientData->block_out;

            updateClientErrCnt( client2, memc );

            return( 0 );
        }
    }

    PRINTF( "%s: client_id %u not found in saved array\n", __FUNCTION__, client_id );

    /* if we get here, then the client_id was not found in the previous saved list; add it to the end */
    for (client2 = 0; client2<BMEMPERF_MAX_NUM_CLIENT; client2++)
    {
        if (g_savedClientDataSorted[memc][client2].client_id == BMEMPERF_CGI_INVALID)
        {
            g_savedClientDataSorted[memc][client2] = *pClientData;
            PRINTF( "new memc%u:%u id %u; bw %u; \n", memc, client2, client_id, pClientData->bw );

            updateClientErrCnt( client2, memc );

            return( 0 );
        }
        else
        {
            PRINTF( "%s: for memc %u, saved element %u taken by id %u\n", __FUNCTION__, memc, client2, g_savedClientDataSorted[memc][client2].client_id );
        }
    }

    return( 0 );
} /* updateClient */

/**
 *  Function: This function will call get_interrupt_counts() API to copy the interrupt counts to a local
 *  structure. This local structure will be used to determine the delta counts from the last pass through.
 **/
static int bmemperf_computeIrqData(
    unsigned int       numActiveCpus,
    bmemperf_irq_data *pIrqData
    )
{
    unsigned int             cpu = 0;
    unsigned int             irq = 0;
    bmemperf_irq_data        irqDataNow;
    float                    uptimeDelta = 0;
    unsigned long int        irqTotal    = 0;
    static float             irqAvg      = 0;
    static unsigned long int irqAvgCount = 0;
    static unsigned long int irqAvgFirst = 0;
    bmemperf_proc_stat_info  lProcStatInfo;

    memset( &irqDataNow, 0, sizeof( irqDataNow ));
    memset( &lProcStatInfo, 0, sizeof( lProcStatInfo ));

    getUptime( &irqDataNow.uptime );
    PRINTF( "%5.3f - %5.3f = %5.3f\n", irqDataNow.uptime, g_savedIrqData.uptime, irqDataNow.uptime - g_savedIrqData.uptime );

    /* calculate the cummulative number of interrupts that happened since we booted up */
    get_interrupt_counts( &irqDataNow );

    PRINTF( "%s: numCpus %u: ", __FUNCTION__, numActiveCpus );
    /* subtract off the number of interrupts that we saw the last pass through */
    for (cpu = 0; cpu < numActiveCpus; cpu++)
    {
        PRINTF2( "%u:", cpu );
        /* if no data has been saved previously, return 0; otherwise, the returned value could be a very very large number */
        if (g_savedIrqData.irqCount[cpu] == 0)
        {
            pIrqData->irqCount[cpu] = 0;
        }
        else
        {
            if (g_savedIrqData.uptime > 0)
            {
                uptimeDelta = 1.0 * irqDataNow.irqCount[cpu];
                /*printf("%5.1f ", uptimeDelta );*/
                uptimeDelta -= 1.0 * g_savedIrqData.irqCount[cpu];
                /*printf("%5.1f ", uptimeDelta );*/
                /*uptimeDelta *= ( irqDataNow.uptime - g_savedIrqData.uptime ); */ /* adjust the value to account for 99 jiffies this second versus 100 this second */
                PRINTF( "(%5.1f)", uptimeDelta );
                pIrqData->irqCount[cpu] = uptimeDelta;
                PRINTF( "->%lu (%lu)\t", pIrqData->irqCount[cpu], irqDataNow.irqCount[cpu] - g_savedIrqData.irqCount[cpu] );
            }
            else
            {
                pIrqData->irqCount[cpu] = irqDataNow.irqCount[cpu] - g_savedIrqData.irqCount[cpu];
            }
        }
        PRINTF( "%-5lu ", pIrqData->irqCount[cpu] );
    }

    /* read interrupt data and context switch data from /proc/stat file */
    get_proc_stat_info( &lProcStatInfo );

    PRINTF("%s: delta (%lu) = now (%lu) - prev (%lu)\n", __FUNCTION__,  g_ContextSwitchesDelta, lProcStatInfo.contextSwitches, g_ContextSwitchesPrev );
    g_ContextSwitchesDelta = lProcStatInfo.contextSwitches - g_ContextSwitchesPrev;
    g_ContextSwitchesPrev = lProcStatInfo.contextSwitches;

    /* the irqTotal is computed from the file /proc/stat and not from adding entries from /proc/interrupts */
    irqDataNow.irqTotal = irqTotal = lProcStatInfo.irqTotal;

    /* adjust value to account for 99 jiffies per second versus 100 or 101 or 102 etc. */
    uptimeDelta = ( irqDataNow.uptime - g_savedIrqData.uptime );

    pIrqData->irqTotal = irqDataNow.irqTotal - g_savedIrqData.irqTotal;
    if (irqAvgFirst == 0)
    {
        irqAvgFirst = irqDataNow.irqTotal;
    }
    /* don't adjust the delta count if the uptime delta is way off */
    if (( uptimeDelta > 0 ) && ( uptimeDelta < 1.5 ))
    {
        float temp = 1.0 /*uptimeDelta*/;
        PRINTF2( "irqTotal was %-5lu (%-5lu - %-5lu); ", pIrqData->irqTotal, irqDataNow.irqTotal, g_savedIrqData.irqTotal );
        temp              *= pIrqData->irqTotal;
        pIrqData->irqTotal = temp;
        PRINTF2( "mul %5.2f is %-5lu;    ", 1.0 /*uptimeDelta*/, pIrqData->irqTotal );
        {
            float             delta    = pIrqData->irqTotal;
            float             oldAvg   = irqAvg;
            unsigned long int oldCount = irqAvgCount;

            irqAvg = ( irqAvg*irqAvgCount ) + delta;
            irqAvgCount++;
            irqAvg /= irqAvgCount;

            PRINTF2( " 2222 avg (%5.1f * %lu) + %5.1f = %5.1f; first (%lu - %lu)/%lu = %lu\n", oldAvg, oldCount, delta, irqAvg, irqDataNow.irqTotal, irqAvgFirst, irqAvgCount,
                ( irqDataNow.irqTotal - irqAvgFirst )/irqAvgCount );
        }
    }
    PRINTF( "irqTotal %-3lu (%lu - %lu); adj %5.1f\n", pIrqData->irqTotal, irqDataNow.irqTotal, g_savedIrqData.irqTotal, uptimeDelta );

    /* copy the IRQ details to the response buffer */
    PRINTF( "%s: memcpy to irqDetails ... %u bytes 1; %u bytes 2\n", __FUNCTION__,  sizeof( pIrqData->irqDetails ), sizeof( pIrqData->irqDetails ));
    memcpy( &pIrqData->irqDetails, &irqDataNow.irqDetails, sizeof( pIrqData->irqDetails ));

    /* copy the previous counts into the response buffer */
    for (cpu = 0; cpu < numActiveCpus; cpu++)
    {
        for (irq = 0; irq < BMEMPERF_IRQ_MAX_TYPES; irq++)
        {
            pIrqData->irqDetails[irq].irqCountPrev[cpu] = g_savedIrqData.irqDetails[irq].irqCount[cpu];
        }
    }
    /* save the current values to be used to compute the delta the next pass through */
    g_savedIrqData = irqDataNow;

    return( 0 );
} /* bmemperf_computeIrqData */

/**
 *  Function: This function will perform a 10-second perf record. It will run as a separate thread until the perf record
 *  operation is complete.
 **/
static void *bmemperf_perf_record_thread(
    void *data
    )
{
    char               line[MAX_LINE_LENGTH];
    unsigned long int *pduration = (unsigned long int *)data;
    char               perfFilename[PERF_FILE_FULL_PATH_LEN];

    PrependTempDirectory( perfFilename, sizeof( perfFilename ), PERF_REPORT_OUTPUT_FILE );

    if (data == NULL)
    {
        printf( "%s: arg1 cannot be NULL\n", __FUNCTION__ );
        exit( EXIT_FAILURE );
    }

    sprintf( line, "ulimit -c unlimited && %s/perf record -a -g sleep %ld > %s 2>&1", BIN_DIR, *pduration, perfFilename );
    printf( "%s: issuing system(%s)\n", __FUNCTION__, line );

    system( line );

    printf( "%s: complete.\n", __FUNCTION__ );

    pthread_exit( 0 );
} /* bmemperf_perf_record_thread */

/**
 *  Function: This function will start a new thread that will run until the perf record operation is complete.
 **/
static int bmemperf_start_perf_record(
    unsigned long int duration
    )
{
    pthread_t                perfRecordThreadId = 0;
    void                    *(*threadFunc)( void * );
    static unsigned long int threadOption = 0;

    threadFunc   = bmemperf_perf_record_thread;
    threadOption = duration;

    if (pthread_create( &perfRecordThreadId, NULL, threadFunc, (void *)&threadOption ))
    {
        printf( "%s: could not create thread for perf record; %s\n", __FUNCTION__, strerror( errno ));
    }

    return( 0 );
} /* bmemperf_start_perf_record */

/**
 *  Function: This function will perform a 10-second perf stat. It will run as a separate thread until the perf
 *  stat operation is complete.
 **/
static void *bmemperf_perf_stat_thread(
    void *data
    )
{
    char               line[MAX_LINE_LENGTH];
    unsigned long int *pduration = (unsigned long int *)data;
    char               perfFilename[PERF_FILE_FULL_PATH_LEN];

    PrependTempDirectory( perfFilename, sizeof( perfFilename ), PERF_STAT_OUTPUT_FILE );

    if (data == NULL)
    {
        printf( "%s: arg1 cannot be NULL\n", __FUNCTION__ );
        exit( EXIT_FAILURE );
    }

    /* activate perf with limit to top 20 */
    sprintf( line, "%s/perf stat -a -e bus-cycles,cycles,instructions,cache-references,cache-misses,L1-dcache-loads,"
                   "L1-dcache-load-misses,L1-dcache-stores,L1-dcache-store-misses,L1-icache-loads,L1-icache-load-misses,"
                   "LLC-loads,LLC-load-misses,LLC-stores,LLC-store-misses,dTLB-load-misses,dTLB-store-misses,iTLB-load-misses,"
                   "iTLB-load-misses sleep %ld > %s 2>&1", BIN_DIR, *pduration, perfFilename );
    printf( "%s: issuing system(%s)\n", __FUNCTION__, line );

    system( line );

    printf( "%s: complete.\n", __FUNCTION__ );

    pthread_exit( 0 );
} /* get_perfreport_data */

static int bmemperf_sata_usb_array_update(
    bmemperf_device_data *lSataUsbDataNow,
    const char           *lDeviceName
    )
{
    unsigned int idx;

    for (idx = 0; idx<BMEMPERF_SATA_USB_MAX; idx++)
    {
        /* if we find a slot that does not have a device name in it, the one we are searching for does not exist in the array */
        if (strlen( gSataUsbDeviceNames[idx].deviceName ) == 0)
        {
            break;
        }
        else if (strcmp( gSataUsbDeviceNames[idx].deviceName, lDeviceName ) == 0)
        {
            /*printf( "adding sata_usb data to idx %u for (%s)\n", idx, lSataUsbDataNow->deviceName );*/
            gSataUsbDataNow[idx] = *lSataUsbDataNow;
            return( 0 );
        }
    }

    printf( "Could not find deviceName (%s) in gSataUsbDeviceNames.\n", lDeviceName );
    return( 0 );
} /* bmemperf_sata_usb_array_update */

static bool bmemperf_linux_top_stop = false; /* set to true when CMD_STOP_LINUX_TOP is detected */
/**
 *  Function: This function will handle the signal SIGTERM. This signal is caught when the server is
 *            attempting to pthread_kill() the thread that is performing the Linux Top command.
 *            When the user unchecks the LinuxTop checkbox on the web page, a command is sent to the
 *            server which in turn signals the controlling thread to stop performing the Linux Top
 *            command.
 **/
static void bmemperf_linux_top_sigterm(
    int signum
    )
{
    bmemperf_linux_top_stop = true;
    PRINTF( "%s: got signal %d; bmemperf_linux_top_stop now is %d\n", __FUNCTION__, signum, bmemperf_linux_top_stop );
    return;
}

/**
 *  Function: This function will call the 'top' utility until ordered to stop.
 **/
static void *bmemperf_linux_top_thread(
    void *data
    )
{
    char cmd[128];
    char linuxTopFilename[LINUX_TOP_FILE_FULL_PATH_LEN];

    BSTD_UNUSED( data );

    PrependTempDirectory( linuxTopFilename, sizeof( linuxTopFilename ), LINUX_TOP_OUTPUT_FILE );
    PRINTF( "%s: filename (%s)\n", __FUNCTION__, linuxTopFilename );

    sprintf( cmd, "%s/top -b -d.1 -n 1 > %s", BIN_DIR, linuxTopFilename );

    /* collect data until the user un-checks the LinuxTop Stats box */
    while (bmemperf_linux_top_stop == false)
    {
        PRINTF( "issuing cmd (%s)\n", cmd );
        system( cmd );

        usleep( 1000 * 500 ); /* 500 milliseconds */
    }

    printf( "%s: pthread_exit()\n", __FUNCTION__ );
    pthread_exit( 0 );
} /* bmemperf_linux_top_thread */

/**
 *  Function: This function will use the 'df' utility to scan for currently mounted devices. Only these devices
 *  will be searched for in the /proc/diskstats file during the one-second gathering.
 **/
static void bmemperf_sata_usb_scan(
    void
    )
{
    FILE *cmd = NULL;
    char  line[MAX_LENGTH_CPU_STATUS];
    int   gSataUsbDeviceNamesIdx = 0;

    strncpy( line, "df | grep \"^/dev/\" ", sizeof ( line ) - 1 );
    cmd = popen( line, "r" );

    do
    {
        memset( line, 0, sizeof( line ));

        fgets( line, MAX_LENGTH_CPU_STATUS, cmd ); /* /dev/sda4            958852568    917028 909919940   0% /data */

        if (strlen( line ))
        {
            char *pos = strchr( &line[1], '/' ); /* find the beginning of the unique device name (skipping over the first slash) */
            if (pos)
            {
                char *posspace = NULL;
                pos++;
                posspace = strchr( pos, ' ' ); /* find the first space after the device name */
                if (posspace)
                {
                    *posspace = '\0'; /* null-terminate the device name */
                    /* if there is space to add this device name to the list */
                    if (gSataUsbDeviceNamesIdx < BMEMPERF_SATA_USB_MAX)
                    {
                        strncpy( gSataUsbDeviceNames[gSataUsbDeviceNamesIdx].deviceName, pos, sizeof( gSataUsbDeviceNames[gSataUsbDeviceNamesIdx].deviceName ) - 1 );
                        PRINTF( "%s: added gSataUsbDeviceNames[%d]:(%s)\n", __FUNCTION__, gSataUsbDeviceNamesIdx, gSataUsbDeviceNames[gSataUsbDeviceNamesIdx].deviceName );
                        gSataUsbDeviceNamesIdx++;
                    }
                }
            }
        }
    } while (strlen( line ));
    PRINTF( "\n" );

    pclose( cmd );
    return;
} /* getCpuOnline */

/**
 *  Function: This function will read the /proc/diskstats file to gather statistics for SATA and USB drives.
 **/
static void *bmemperf_sata_usb_thread(
    void *data
    )
{
    if (data == NULL)
    {
        printf( "%s: arg1 cannot be NULL\n", __FUNCTION__ );
        exit( EXIT_FAILURE );
    }

    Bmemperf_Server_LockMutex( &gSataUsbMutex );

    printf( "%s: clearing gSataUsb arrays. \n", __FUNCTION__ );
    memset( &gSataUsbDataPrev, 0, sizeof( gSataUsbDataPrev ));
    memset( &gSataUsbDeviceNames, 0, sizeof( gSataUsbDeviceNames ));
    memset( &gSataUsbMbpsHistory, 0, sizeof( gSataUsbMbpsHistory ));
    gSataUsbMbpsHistoryIdx = 0;
    gSataUsbFirstPassAfterInit = true;

    /* scan for currently-mounted device names */
    bmemperf_sata_usb_scan(); /* function updates gSataUsbDeviceNames[] array */

    Bmemperf_Server_UnlockMutex( &gSataUsbMutex );

    /* collect data until no one has requested USB/SATA for 5 seconds */
    while (gSataUsbTimeout <  5)
    {
        unsigned int         idx      = 0;
        char                *pos      = NULL;
        char                *posEol   = NULL;
        char                *posBol   = NULL;
        char                *contents = NULL;
        char                 cmd[128];
        unsigned char        differenceFound = 0;
        bmemperf_device_data lSataUsbDataNow;
        char                 sataUsbFilename[SATA_USB_FILE_FULL_PATH_LEN];

        gSataUsbTimeout++; /* request function should zero this out every time a request comes in; if we reach time, stop gathering */
        if (gSataUsbTimeout > 1)
        {
            printf( "%s: gSataUsbTimeout (%ld)\n", __FUNCTION__, gSataUsbTimeout );
        }

        PrependTempDirectory( sataUsbFilename, sizeof( sataUsbFilename ), SATA_USB_OUTPUT_FILE );
        PRINTF( "%s: filename (%s)\n", __FUNCTION__, sataUsbFilename );

        differenceFound = 0;

        memset( &gSataUsbDataNow, 0, sizeof( gSataUsbDataNow ));
        memset( &lSataUsbDataNow, 0, sizeof( lSataUsbDataNow ));

        sprintf( cmd, "cp -f /proc/diskstats %s", sataUsbFilename );
        /*printf( "issuing cmd (%s)\n", cmd );*/
        system( cmd );

        contents = GetFileContents( sataUsbFilename );

        if (contents == NULL)
        {
            printf( "file (%s) had no contents\n", sataUsbFilename );
            continue;
        }

        /* loop through known devices and find the matching line in the diskstats contents */
        for (idx = 0; idx<BMEMPERF_SATA_USB_MAX; idx++)
        {
            if (strlen( gSataUsbDeviceNames[idx].deviceName ) == 0)
            {
                /* we have reached the end of known devices */
                break;
            }

            pos = strstr( contents, gSataUsbDeviceNames[idx].deviceName );
            /*printf("%s: found device (%s) at offset %d\n", __FUNCTION__, gSataUsbDeviceNames[idx].deviceName, pos - contents );*/

            /* if the device name was found in diskstats file */
            if (pos)
            {
                char lDeviceName[32];

                posEol = strchr( pos, '\n' );
                /*printf("%s: for device (%s) posEol %p\n", __FUNCTION__, gSataUsbDeviceNames[idx].deviceName, posEol );*/
                if (posEol)
                {
                    posEol[0] = '\0'; /* null-terminate the line */
                }

                posBol = pos - 12;

                /*printf("%s: for device (%s) line (%s)\n", __FUNCTION__, gSataUsbDeviceNames[idx].deviceName, posBol );*/
                /*
                   8       0 sda 70715 13758 8161780 6120 4 1 5 0 0 2950 6120
                   8       1 sda1 70714 13758 8161772 6090 4 1 5 0 0 2920 6090
                  31      10 mtdblock10 0 0 0 0 0 0 0 0 0 0 0
                 179       0 mmcblk0 170 9048 9257 610 0 0 0 0 0 330 610
                 179       1 mmcblk0p1 169 9048 9249 600 0 0 0 0 0 320 600
                */

                sscanf( posBol, "%u %u %s %lu %lu %lu %lu %lu %lu %lu %lu ", &lSataUsbDataNow.major, &lSataUsbDataNow.minor, &lDeviceName[0],
                    &lSataUsbDataNow.readsCompletedSuccessfully, &lSataUsbDataNow.readsMerged, &lSataUsbDataNow.readsSectors, &lSataUsbDataNow.readsMilliseconds,
                    &lSataUsbDataNow.writesCompletedSuccessfully, &lSataUsbDataNow.writesMerged, &lSataUsbDataNow.writesSectors, &lSataUsbDataNow.writesMilliseconds );
                bmemperf_sata_usb_array_update( &lSataUsbDataNow, lDeviceName );

                /* debug */
                if (( lSataUsbDataNow.readsSectors != gSataUsbDataPrev[idx].readsSectors ) || ( lSataUsbDataNow.writesSectors != gSataUsbDataPrev[idx].writesSectors ))
                {
                    PRINTF( "%s: /dev/%s: rd %-8lu, wr %-8lu\n", __FUNCTION__, lDeviceName, lSataUsbDataNow.readsSectors, lSataUsbDataNow.writesSectors );
                }
                if (posEol)
                {
                    posEol[0] = '\n'; /* restore the newline character */
                }
            }
        } /* end for each device name */

        Bsysperf_Free( contents );

        Bmemperf_Server_LockMutex( &gSataUsbMutex );
        /* for each I/O device */
        for (idx = 0; idx<sizeof( gSataUsbDataNow )/sizeof( gSataUsbDataNow[0] ); idx++)
        {
            unsigned long int deltaSectors = 0;
            if (( gSataUsbDataNow[idx].readsSectors > gSataUsbDataPrev[idx].readsSectors ) || ( gSataUsbDataNow[idx].writesSectors > gSataUsbDataPrev[idx].writesSectors ))
            {
                differenceFound            = 1;
                deltaSectors               = gSataUsbDataNow[idx].readsSectors - gSataUsbDataPrev[idx].readsSectors;
                gSataUsbMbps[idx].readMbps = deltaSectors * 512.0 * 8.0 / 1024.0 / 1024.0;
                /*if (strlen( gSataUsbDataPrev[idx].deviceName ))*/
                {
                    PRINTF( "device[%u] (%-4s) %6.2f Mbps ", idx, gSataUsbDeviceNames[idx].deviceName, gSataUsbMbps[idx].readMbps );
                }

                deltaSectors                = gSataUsbDataNow[idx].writesSectors - gSataUsbDataPrev[idx].writesSectors;
                gSataUsbMbps[idx].writeMbps = deltaSectors * 512.0 * 8.0 / 1024.0 / 1024.0;
                /*if (strlen( gSataUsbDataPrev[idx].deviceName ))*/
                {
                    PRINTF( "%6.2f Mbps\n", gSataUsbMbps[idx].writeMbps );
                }
            }
            else
            {
                gSataUsbMbps[idx].readMbps  = 0;
                gSataUsbMbps[idx].writeMbps = 0;
                PRINTF( "device[%u] zero; ", idx );
            }

            /* save for 10-second history */
            if ( gSataUsbFirstPassAfterInit == false )
            {
                gSataUsbMbpsHistory[idx][gSataUsbMbpsHistoryIdx].readMbps  = gSataUsbMbps[idx].readMbps;
                gSataUsbMbpsHistory[idx][gSataUsbMbpsHistoryIdx].writeMbps = gSataUsbMbps[idx].writeMbps;
            }
        }

        /* the first pass through, the previous numbers are zero but the current readSectors/writeSectors are very large; skip the first pass */
        gSataUsbFirstPassAfterInit = false;

        gSataUsbMbpsHistoryIdx++;
        if (gSataUsbMbpsHistoryIdx >= BMEMPERF_SATA_USB_HISTORY_MAX) /* wrap around to zero if it's time to */
        {
            gSataUsbMbpsHistoryIdx = 0;
        }

        Bmemperf_Server_UnlockMutex( &gSataUsbMutex );
        PRINTF( "\n" );

        if (differenceFound)
        {
            Bmemperf_Server_LockMutex( &gSataUsbMutex );
            memcpy( &gSataUsbDataPrev, &gSataUsbDataNow, sizeof( gSataUsbDataNow ));
            Bmemperf_Server_UnlockMutex( &gSataUsbMutex );
        }

        /*remove( sataUsbFilename );*/

        sleep( 1 );
    }

    Bmemperf_Server_LockMutex( &gSataUsbMutex );
    printf( "%s: clearing gSataUsb arrays. \n", __FUNCTION__ );
    memset( &gSataUsbDataPrev, 0, sizeof( gSataUsbDataPrev ));
    memset( &gSataUsbDeviceNames, 0, sizeof( gSataUsbDeviceNames ));
    memset( &gSataUsbMbpsHistory, 0, sizeof( gSataUsbMbpsHistory ));
    gSataUsbMbpsHistoryIdx = 0;
    gSataUsbFirstPassAfterInit = true;
    Bmemperf_Server_UnlockMutex( &gSataUsbMutex );

    gSataUsbThreadId = 0;

    printf( "%s: complete.\n", __FUNCTION__ );

    pthread_exit( 0 );
} /* bmemperf_sata_usb_thread */

/**
 *  Function: This function will start a new thread that will run until the perf stat operation is complete.
 **/
static int bmemperf_start_perf_cache(
    unsigned long int duration
    )
{
    pthread_t                perfRecordThreadId = 0;
    void                    *(*threadFunc)( void * );
    static unsigned long int threadOption = 0;

    threadFunc   = bmemperf_perf_stat_thread;
    threadOption = duration;

    if (pthread_create( &perfRecordThreadId, NULL, threadFunc, (void *)&threadOption ))
    {
        printf( "%s: could not create thread for perf stat; %s\n", __FUNCTION__, strerror( errno ));
    }

    return( 0 );
}

/**
 *  Function: This function will compute the 10-second average in Mbps for data reads and writes to the
 *  various SATA and USB devices.
 **/
float bmemperf_sata_usb_compute_average(
    unsigned int deviceIdx,
    unsigned int readOrWrite /* 0: readMbps; 1: writeMbps */
    )
{
    unsigned int idx     = 0;
    float        average = 0.0;

    /* if the deviceName is invalid, stop computing */
    if (gSataUsbDeviceNames[deviceIdx].deviceName[0] == '\0')
    {
        return( average );
    }

    #ifdef DEBUG
    if ( deviceIdx == 0 ) printf( "%s: device[%u].%-5s: idx %d: %-5s ", __FUNCTION__, deviceIdx,
                                  gSataUsbDeviceNames[deviceIdx].deviceName, gSataUsbMbpsHistoryIdx,
                                  ( readOrWrite==0 ) ? "read" : "write" );
    #endif
    for (idx = 0; idx<BMEMPERF_SATA_USB_HISTORY_MAX; idx++)
    {
        if (readOrWrite == 0 /* user requested readMbps */)
        {
            average += gSataUsbMbpsHistory[deviceIdx][idx].readMbps;
            #ifdef DEBUG
            if ( deviceIdx == 0 ) printf("%-5.2f ", gSataUsbMbpsHistory[deviceIdx][idx].readMbps );
            #endif
        }
        else
        {
            average += gSataUsbMbpsHistory[deviceIdx][idx].writeMbps;
            #ifdef DEBUG
            if ( deviceIdx == 0 ) printf("%-5.2f ", gSataUsbMbpsHistory[deviceIdx][idx].writeMbps );
            #endif
        }
    }
    average /= BMEMPERF_SATA_USB_HISTORY_MAX;
    #ifdef DEBUG
    if ( deviceIdx == 0 ) printf( "... avg %5.2f Mbps \n", average );
    #endif
    return( average );
} /* bmemperf_sata_usb_compute_average */

/**
 *  Function: This function will gather the SATA and USB data that is collected by a separate thread and save
 *  the data into the response buffer being prepared to send back to the client.
 **/
static int bmemperf_sata_usb_gather(
    bmemperf_response *pResponse
    )
{
    unsigned int idx = 0;

    if (pResponse == NULL)
    {
        return( -1 );
    }

    gSataUsbTimeout = 0;

    for (idx = 0; idx<sizeof( gSataUsbDeviceNames )/sizeof( gSataUsbDeviceNames[0] ); idx++)
    {
        /* the first time we reach an empty device name, stop looking for data */
        if (gSataUsbDeviceNames[idx].deviceName[0] == '\0')
        {
            break;
        }
        strncpy( pResponse->response.cpuIrqData.sataUsbData[idx].deviceName, gSataUsbDeviceNames[idx].deviceName,
            sizeof( pResponse->response.cpuIrqData.sataUsbData[0].deviceName ) - 1 );
        #ifdef INSTANT_RESPONSE
        pResponse->response.cpuIrqData.sataUsbData[idx].readMbps  = gSataUsbMbps[idx].readMbps;
        pResponse->response.cpuIrqData.sataUsbData[idx].writeMbps = gSataUsbMbps[idx].writeMbps;
        #else /* use 10-second average */
        pResponse->response.cpuIrqData.sataUsbData[idx].readMbps  = bmemperf_sata_usb_compute_average( idx, 0 /* compute readMbps */ );
        pResponse->response.cpuIrqData.sataUsbData[idx].writeMbps = bmemperf_sata_usb_compute_average( idx, 1 /* compute writeMbps */ );
        #endif /* if defined INSTANT_RESPONE */
    }

    return( 0 );
} /* bmemperf_sata_usb_gather */

/**
 *  Function: This function will start a new thread that will gather the SATA and USB bandwidth statistics.
 **/
static int bmemperf_sata_usb_start(
    unsigned long int option
    )
{
    void                    *(*threadFunc)( void * );
    static unsigned long int threadOption = 0;

    /* if the thread has not already been started, start it now */
    if (( gSataUsbThreadId == 0 ) && ( option == BMEMPERF_CMD_START_SATA_USB ))
    {
        threadFunc   = bmemperf_sata_usb_thread;
        threadOption = 0;

        if (pthread_create( &gSataUsbThreadId, NULL, threadFunc, (void *)&threadOption ))
        {
            printf( "%s: could not create thread for sataUsb; %s\n", __FUNCTION__, strerror( errno ));
        }
        else
        {
            printf( "%s: Thread for SATA/USB started successfully; id %lx\n", __FUNCTION__, (long int) gSataUsbThreadId );
        }
    }
    else
    {
        PRINTF( "%s: Thread for SATA/USB already started; id %lx\n", __FUNCTION__, (long int) gSataUsbThreadId );
        if (option == BMEMPERF_CMD_STOP_SATA_USB)
        {
            /* gSataUsbThreadId = 0; thread will timeout by itself after 5 seconds of inactivity */
        }
    }

    return( 0 );
} /* bmemperf_sata_usb_start */

/**
 *  Function: This function will telnet to the Power Probe board and poll for the current and
 *            voltage for up to 3 probes.
 **/
static void *bmemperf_power_probe_thread(
    void *data
    )
{
    struct sockaddr_in  srv;
    int                 socket_fd=0;
    int                 probe=0;
    char                hostip[INET6_ADDRSTRLEN];
    char                hostcmd[64];
    char                response[1024];
    struct timeval      tv1, tv2;
    unsigned long long int microseconds1 = 0, microseconds2 = 0;
    unsigned long int   microseconds_delta = 0;

    if (data == NULL)
    {
        printf( "%s: arg1 cannot be NULL\n", __FUNCTION__ );
        goto exit;
    }

    memset( &tv1, 0, sizeof(tv1) );
    memset( &tv2, 0, sizeof(tv2) );

    strncpy ( hostip, gPowerProbeIpAddr, sizeof(hostip) );

    /* create a connection to the server */
    srv.sin_family = AF_INET; // use the internet addr family
    srv.sin_port = htons( TELNET_PORT ); // dedicated telnet port = 23
    srv.sin_addr.s_addr = inet_addr( hostip );

    /* create the socket */
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("%s:%lu:%lu socket() failed\n", __FUNCTION__, GETPID, pthread_self() );
        /*perror("socket");*/
        goto exit;
    }

    /* connect to the power probe's telnet server */
    printf("%s:%lu:%lu Connecting to (%s) port %d\n", __FUNCTION__, GETPID, pthread_self(), hostip, TELNET_PORT );
    if (connect(socket_fd, (struct sockaddr*) &srv, sizeof(srv)) < 0) {
        printf("%s:%lu:%lu connect() failed\n", __FUNCTION__, GETPID, pthread_self() );
        /*perror("connect");*/
        goto exit;
    } else {
        printf("%s:%lu:%lu Connection to (%s) successful.\n\n", __FUNCTION__, GETPID, pthread_self(), hostip );
    }


    memset( response, 0, sizeof(response) );
    bmemperf_get_response ( __FUNCTION__, socket_fd, "login:", response, sizeof(response), NULL );


    PRINTF("%s:%lu:%lu sending User Name\n", __FUNCTION__, GETPID, pthread_self() );
    bmemperf_send_data( socket_fd, __FUNCTION__, 0, NULL, "root\n", 5, NULL );

    memset( response, 0, sizeof(response) );
    bmemperf_get_response ( __FUNCTION__, socket_fd, "# ", response, sizeof(response), NULL );
    /*usleep( 50 );*/


    /* if we do not receive a request from the browser within 5 seconds, assume they stopped wanting power probe info and exit */
    while ( gPowerProbeQuitCount < 5 )
    {
        gettimeofday(&tv1, NULL);
        microseconds1 = (tv1.tv_sec * 1000000LL);
        /*printf("microseconds1 %llu (%lu) (%06lu)\n", microseconds1, tv1.tv_sec, tv1.tv_usec );*/
        microseconds1 += tv1.tv_usec;
        /*printf("microseconds1 %llu\n", microseconds1 );*/

        /*
        voltcont read_vSense 0
        voltcont read_current 0 2
        */
        for ( probe=0; probe<3; probe++ )
        {
            char *eol = NULL;
            char *eov = NULL; /* end of value we want to parse */
            float fVoltage = 0.0;
            float fCurrent = 0.0;
            sprintf( hostcmd, "voltcont read_vSense %d", probe );
            bmemperf_send_data( socket_fd, __FUNCTION__, 0, NULL, hostcmd, strlen(hostcmd), NULL );
            /*usleep( 50 );*/
            memset( response, 0, sizeof(response) );
            bmemperf_get_response ( __FUNCTION__, socket_fd, "# " /*hostcmd*/, response, sizeof(response), NULL );
            eol = strchr( &response[3], '\n'); /* first char might be carriage-return */
            if ( eol )
            {
                eol++;
                eov = strchr( eol, '\n');
                if ( eov ) *eov='\0';
            }
            /*printf("for (%s) ... got response (%s)\n", hostcmd, eol );*/
            sscanf( eol, "%6f", &fVoltage );
            gPowerProbeVoltage[probe] = fVoltage;

            sprintf( hostcmd, "voltcont read_current %d %d", probe, gPowerProbeShunts[probe] );
            bmemperf_send_data( socket_fd, __FUNCTION__, 0, NULL, hostcmd, strlen(hostcmd), NULL );

            /*usleep( 50 );*/

            memset( response, 0, sizeof(response) );
            bmemperf_get_response ( __FUNCTION__, socket_fd, "# " /*hostcmd*/, response, sizeof(response), NULL );
            eol = strchr(response, '\n');
            if ( eol )
            {
                eol++;
                eov = strchr( eol, '\n');
                if ( eov ) *eov='\0';
            }
            /*printf("for (%s) ... got response (%s)\n", hostcmd, eol );*/
            sscanf( eol, "%6f", &fCurrent );
            /*printf("for (%-25s) ... got (%5.3f)\n", hostcmd, fCurrent );*/
            gPowerProbeCurrent[probe] = fCurrent;
        }

        gettimeofday(&tv2, NULL);
        microseconds2 = (tv2.tv_sec * 1000000LL);
        /*printf("microseconds2 %llu (%lu) (%06lu)\n", microseconds2, tv2.tv_sec, tv2.tv_usec );*/
        microseconds2 += tv2.tv_usec;
        /*printf("microseconds2 %llu\n", microseconds2 );
        printf("tv1 %lu.%06lu ... tv2 %lu.%06lu\n", tv1.tv_sec, tv1.tv_usec, tv2.tv_sec, tv2.tv_usec);*/
        microseconds_delta = (microseconds2 - microseconds1);
        /*printf("delta microseconds (%llu - %llu) ... %lu\n", microseconds2, microseconds1, microseconds_delta );*/

        /* if it took less than a full second (200000 usec) to get all of the probe data, wait the remaining part of a full second (800000 usec) */
        if ( microseconds_delta < 1000000 )
        {
            PRINTF( "%s:%lu:%lu need to usleep(%lu)\n\n\n", __FUNCTION__, GETPID, pthread_self(), (1000000 - microseconds_delta) );
            usleep( (1000000 - microseconds_delta) );
        }

        /* each time the server gets a request for power probe data, the server will set this to 0 */
        gPowerProbeQuitCount++;

        printf( "%s:%lu:%lu gPowerProbeQuitCount (%u)\n", __FUNCTION__, GETPID, pthread_self(), gPowerProbeQuitCount );

    }   /* while gPowerProbeQuitCount < 10 */

    bmemperf_send_data( socket_fd, __FUNCTION__, 0, NULL, "quit", 4, NULL );

    close( socket_fd );
    socket_fd = 0;

exit:
    gPowerProbeThreadId = 0;
    gPowerProbeTelnet_pid = 0;
    for(probe=0; probe<POWER_PROBE_MAX; probe++)
    {
        gPowerProbeShunts[probe] = 0;
        gPowerProbeVoltage[probe] = 0.0;
        gPowerProbeCurrent[probe] = 0.0;
        gPowerProbeConnected[probe] = 0;
    }

    PRINTF( "%s:%lu:%lu exiting. gPowerProbeThreadId is 0\n", __FUNCTION__, GETPID, pthread_self() );

    /* if I call pthread_exit() in this thread, the parent thread will exit also */
    PRINTF( "%s:%lu:%lu sleeping()\n", __FUNCTION__, GETPID, pthread_self() );
    sleep( 123456789 );

    return NULL;
} /* bmemperf_power_probe_thread */


/**
 *  Function: This function will start a new thread that will telnet to the specified IP address
 *            and gather Voltage and Current for the specified power probes.
 **/
static int bmemperf_power_probe_start(
    unsigned long int option
    )
{
    void                    *(*threadFunc)( void * );
    static unsigned long int threadOption = 0;

    PRINTF("%s:%lu:%lu gPowerProbeThreadId %lu ... gPowerProbeTelnet_pid %lu\n", __FUNCTION__,
            GETPID, pthread_self(), (unsigned long int) gPowerProbeThreadId, gPowerProbeTelnet_pid );
    /* if the thread has not already been started, start it now */
    if ( gPowerProbeThreadId == 0 )
    {
        if ( gPowerProbeTelnet_pid == 0 )
        {
            /* start a background thread that will perform telnet I/O */
            threadFunc   = bmemperf_power_probe_thread;
            threadOption = 0;

            if (pthread_create( &gPowerProbeThreadId, NULL, threadFunc, (void *)&threadOption ))
            {
                printf( "%s:%lu:%lu could not create thread for bmemperf_power_probe_thread; %s\n", __FUNCTION__,
                        GETPID, pthread_self(), strerror( errno ));
            }
            else
            {
                printf( "%s:%lu:%lu Thread for bmemperf_power_probe_thread started successfully; id %lx\n", __FUNCTION__,
                        GETPID, pthread_self(), (long int) gPowerProbeThreadId );
            }
        }
    }
    else
    {
        int status = 0;
        PRINTF( "%s:%lu:%lu Thread for bmemperf_power_probe_thread already started; id %lx\n", __FUNCTION__,
                GETPID, pthread_self(), (long int) gPowerProbeThreadId );
        waitpid( gPowerProbeTelnet_pid, &status, WNOHANG );

        /*printf( "%s:%lu:%lu after waitpid() ... status %d\n", __FUNCTION__, GETPID, pthread_self(), status );*/
        /* if the child has exited for some reason */
        if ( status == 13 )
        {
            printf( "%s:%lu:%lu PID %lu has exited with rc %d (%s)!!!!!\n", __FUNCTION__,
                    GETPID, pthread_self(), gPowerProbeTelnet_pid, status, strerror(errno) );
            gPowerProbeTelnet_pid = 0;
            gPowerProbeThreadId = 0;
        }
    }

    return( 0 );
} /* bmemperf_power_probe_start */

/**
 *  Function: This function will save the list.csv response from the power probe board to a series of
 *            csv files ... numbered from 0 to 9.
 **/
static unsigned long int   ResponseFileCount = 0;
static int save_response( char * hostcmd, char *response, double fTimeDelta )
{
    if ( hostcmd && response )
    {
        int len = strlen(response);
        if ( len > 80 )
        {
            char eol = response[80];
            response[80] = '\0';
            /*printf("L%u: %s for (%s) ... got response ... len %d (%s)\n", __LINE__, HhMmSsMsec(0), hostcmd, len, response );*/
            response[80] = eol;
        }
        else
        {
            /*printf("L%u: %s for (%s) ... got response (%s)\n", __LINE__, HhMmSsMsec(0), hostcmd, response );*/
        }

        /* if the data is long enough, save the response to a file */
        {
            char   Filename[64];
            char   probeDataFullFilename[PERF_FILE_FULL_PATH_LEN];
            FILE  *fp = NULL;

            if ( strlen(response) > 100 )
            {
                sprintf( Filename, "probedata%03lu.dat", ResponseFileCount );
                PrependTempDirectory( probeDataFullFilename, sizeof( probeDataFullFilename ), Filename );
                fp = fopen( probeDataFullFilename, "w");
                if ( fp )
                {
                    char timeDelta[64];
                    sprintf( timeDelta, "Delta %6.03f\n", fTimeDelta );
                    printf( "%s", timeDelta );
                    fwrite( timeDelta, 1, strlen(timeDelta), fp);

                    fwrite( response, 1, strlen(response), fp);
                    fclose( fp );
                    /*printf( "Wrote %lu bytes to to file (%s) \n", (unsigned long int) strlen(response), Filename );*/
                }
                else
                {
                    printf( "Could not open file (%s) for writing\n", Filename );
                }
                ResponseFileCount++;
            }
        }
    }

    return 0;
}
/**
 *  Function: This function will telnet to the Power Probe board. The thread will issue a command
 *            to extract voltage and current for 1000 points per second.
 **/
static void *bmemperf_power_probe_1000_thread(
    void *data
    )
{
    int                     idx        = 0;
    int                     socket_fd  = 0;
    int                     probe      = 0;
    unsigned char           PingPong   = 0;
    double                  fTimeStart = 0.0;
    double                  fTimeNow   = 0.0;
    int                     option     = 0;
    FILE                   *fp         = NULL;
    struct sockaddr_in      srv;
    char                    hostip[INET6_ADDRSTRLEN];
    char                    hostcmd[128];
    char                    response[30*1024];
    struct timeval          tv0, tv1, tv2;
    unsigned long long int  microseconds1 = 0, microseconds2 = 0;
    unsigned long int       microseconds_delta = 0;
    char                    probeDataFullFilename[PERF_FILE_FULL_PATH_LEN];
    char                    probeDataFilename[PERF_FILE_FULL_PATH_LEN];

    if (data == NULL)
    {
        printf( "%s: arg1 cannot be NULL\n", __FUNCTION__ );
        goto exit;
    }

    option = (int) * (int*)data;
    printf("%s:%lu:%lu option %d\n", __FUNCTION__, (unsigned long int) getpid(), pthread_self(), option );

    memset( &tv0, 0, sizeof(tv0) );
    memset( &tv1, 0, sizeof(tv1) );
    memset( &tv2, 0, sizeof(tv2) );

    strncpy ( hostip, gPowerProbeIpAddr, sizeof(hostip) );

    for(idx=0; idx<2; idx++ )
    {
        sprintf( probeDataFilename, "list_%d.csv", idx );
        PrependTempDirectory( probeDataFullFilename, sizeof( probeDataFullFilename ), probeDataFilename );
        remove( probeDataFullFilename );
    }

    /* create a connection to the server */
    srv.sin_family = AF_INET; // use the internet addr family
    srv.sin_port = htons(23); // dedicated telnet port = 23
    srv.sin_addr.s_addr = inet_addr( hostip );

    /* create the socket */
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("%s:%lu:%lu socket() failed\n", __FUNCTION__, (unsigned long int) getpid(), pthread_self() );
        /*perror("socket");*/
        goto exit;
    }

    /* connect to the power probe's telnet server */
    printf("%s:%lu:%lu Connecting to (%s) port %d\n", __FUNCTION__, (unsigned long int) getpid(), pthread_self(), hostip, 23 );
    if (connect(socket_fd, (struct sockaddr*) &srv, sizeof(srv)) < 0) {
        printf("%s:%lu:%lu connect() failed\n", __FUNCTION__, (unsigned long int) getpid(), pthread_self() );
        /*perror("connect");*/
        goto exit;
    } else {
        printf("%s:%lu:%lu Connection to (%s) successful.\n\n", __FUNCTION__, (unsigned long int) getpid(), pthread_self(), hostip );
    }


    memset( response, 0, sizeof(response) );
    bmemperf_get_response ( __FUNCTION__, socket_fd, "login:", response, sizeof(response), NULL );

    printf("%s:%lu:%lu sending User Name\n", __FUNCTION__, (unsigned long int) getpid(), pthread_self() );
    bmemperf_send_data( socket_fd, __FUNCTION__, 0, NULL, "root\n", 5, NULL );

    memset( response, 0, sizeof(response) );
    bmemperf_get_response ( __FUNCTION__, socket_fd, "# ", response, sizeof(response), NULL );
    printf("for (%s) ... got response len (%d)\n", "root", (unsigned)strlen(response) );
    usleep( 50 );

#define LIST_FILE_FULL_PATH_LEN 64
    /* if the power_probe_list.txt file exists, send its contents over to the power probe board */
    {
        FILE  *fp         = NULL;
        char   probeDataFullFilename[LIST_FILE_FULL_PATH_LEN];
        char   probeDataFilename[LIST_FILE_FULL_PATH_LEN];
        char   probeDataFileLine[64];
        char  *rc = NULL;

        sprintf( hostcmd, "rm /tmp/list.txt" );
        printf("%s:%lu:%lu sending (%s)\n", __FUNCTION__, (unsigned long int) getpid(), pthread_self(), hostcmd );
        bmemperf_send_data( socket_fd, __FUNCTION__, 0, NULL, hostcmd, strlen(hostcmd), NULL );

        memset( response, 0, sizeof(response) );
        bmemperf_get_response ( __FUNCTION__, socket_fd, "# ", response, sizeof(response), NULL );
        printf("for (%s) ... got response len (%d)\n", hostcmd, (unsigned)strlen(response) );
        usleep( 50 );

        strcpy( probeDataFilename, POWER_PROBE_LIST_DAT_FILENAME );
        PrependTempDirectory( probeDataFullFilename, sizeof( probeDataFullFilename ), probeDataFilename );
        fp = fopen( probeDataFullFilename, "r" );
        printf("fopen (%s) returned %p\n", probeDataFullFilename, fp );
        fflush(stdout);fflush(stderr);

        while ( fp && !feof(fp) )
        {
            rc = fgets( probeDataFileLine, sizeof(probeDataFileLine), fp );
            if ( rc != NULL )
            {
                int len = strlen( probeDataFileLine );
                if ( len && probeDataFileLine[ len - 1 ] == '\n' ) probeDataFileLine[ len - 1 ] = '\0'; /* remove newline character */

                sprintf( hostcmd, "echo \"%s\" >> /tmp/list.txt", probeDataFileLine );
                printf("%s:%lu:%lu sending list.txt line (%s)\n", __FUNCTION__, (unsigned long int) getpid(), pthread_self(), probeDataFileLine );
                fflush(stdout);fflush(stderr);
                bmemperf_send_data( socket_fd, __FUNCTION__, 0, NULL, hostcmd, strlen(hostcmd), NULL );

                memset( response, 0, sizeof(response) );
                bmemperf_get_response ( __FUNCTION__, socket_fd, "# ", response, sizeof(response), NULL );
                usleep( 50 );
            }
        }
    }

    gettimeofday(&tv0, NULL); /* remember the time when things started; used later to determine how many seconds we have been collecting data */
    fTimeStart = (tv0.tv_usec * 1.0) / 1000000.0 + (tv0.tv_sec * 1.0);
    printf( "fTimeStart %8.06f sec %ld usec %ld\n", fTimeStart, tv0.tv_sec, tv0.tv_usec );

    /* if we do not receive a request from the browser within 5 seconds, assume they stopped wanting power probe info and exit */
    while ( gPowerProbeQuitCount < 5 )
    {
        float fCurrent = 0.0;

        gettimeofday(&tv1, NULL);
        microseconds1 = (tv1.tv_sec * 1000000LL);
        /*printf("microseconds1 %llu (%lu) (%06lu)\n", microseconds1, tv1.tv_sec, tv1.tv_usec );*/
        microseconds1 += tv1.tv_usec;
        /*printf("microseconds1 %llu\n", microseconds1 );*/

        if ( option == POWER_PROBE_ONE_HERTZ )
        {
            /*
            Usage: voltcont write_list [numRails] [mode] [count_sec]
            Writes /tmp/list.txt to /tmp/list.csv up to count_sec

            arg mode (1=current only, 2=voltage only, else=all)
            */

            sprintf( hostcmd, "voltcont write_list 2 9 1" );
            /*printf("for (%s) ... sending\n", hostcmd );*/
            bmemperf_send_data( socket_fd, __FUNCTION__, 0, NULL, hostcmd, strlen(hostcmd), NULL );

            sleep( 1 ); /* typically takes 1.5 seconds to collect 1000 points of data */

            memset( response, 0, sizeof(response) );
            /*printf("for (%s) ... getting response \n", hostcmd );*/
            bmemperf_get_response ( __FUNCTION__, socket_fd, "# ", response, sizeof(response), NULL );
            fTimeNow = (tv1.tv_usec * 1.0) / 1000000.0 + (tv1.tv_sec * 1.0);
            printf("L%d - tv0: %ld.%06ld ... tv1: %ld.%06ld ... %8.06f %8.06f ... delta %8.06f\n", __LINE__,
                    (long int) tv0.tv_sec, (long int) tv0.tv_usec, (long int) tv1.tv_sec, (long int) tv1.tv_usec,
                    fTimeStart, fTimeNow, fTimeNow - fTimeStart  );
            save_response( hostcmd, response, fTimeNow - fTimeStart );

            sprintf( hostcmd, "cat /tmp/list.csv && echo \"# \"" );
            /*printf("for (%s) ... sending\n", hostcmd );*/
            bmemperf_send_data( socket_fd, __FUNCTION__, 0, NULL, hostcmd, strlen(hostcmd), NULL );

            usleep( 50 ); /* expect about 26,000 bytes back */

            memset( response, 0, sizeof(response) );
            /*printf("for (%s) ... getting response \n", hostcmd );*/
            bmemperf_get_response ( __FUNCTION__, socket_fd, "# ", response, sizeof(response), NULL );
            /*printf("for (%s) ... got response len (%d)\n", hostcmd, strlen(response) );*/

            /* read the previous CSV data from saved file */
            if ( ResponseFileCount )
            {
                char  Filename[64];
                FILE *fp = NULL;

                sprintf( Filename, "probedata%03lu.dat", ResponseFileCount-1 );
                PrependTempDirectory( probeDataFullFilename, sizeof( probeDataFullFilename ), Filename );
                fp = fopen( probeDataFullFilename, "r");
                if ( fp )
                {
                    fread( response, 1, sizeof(response), fp );
                    fclose( fp );
                }
                if ( ResponseFileCount >= 10 ) ResponseFileCount = 0;
            }

            if ( strlen(response) > 100 )
            {
                PingPong = 0;
#if 0
                int len = 0;

                /* if list_0.csv exists */
                sprintf( probeDataFilename, "list_%d.csv", PingPong );
                PrependTempDirectory( probeDataFullFilename, sizeof( probeDataFullFilename ), probeDataFilename );
                len = GetFileLength( probeDataFullFilename );
                printf( "L%u len (%s) is %d\n", __LINE__, probeDataFullFilename, len );
                if ( GetFileLength( probeDataFullFilename ) > 0 )
                {
                    /* client has not had a chance to consume the data from list_0.csv; write data to list_1.csv */
                    PingPong = 1;
                    printf( "L%u - PingPong %d\n", __LINE__, PingPong );
                }
                else /* list_0.csv does not exist */
                {
                    /* if list_1.csv exists */
                    sprintf( probeDataFilename, "list_%d.csv", 1 );
                    PrependTempDirectory( probeDataFullFilename, sizeof( probeDataFullFilename ), probeDataFilename );
                    if ( GetFileLength( probeDataFullFilename ) > 0 )
                    {
                        char probeDataFullFilename0[PERF_FILE_FULL_PATH_LEN];
                        char probeDataFilename0[PERF_FILE_FULL_PATH_LEN];

                        sprintf( probeDataFilename0, "list_%d.csv", 0 );
                        PrependTempDirectory( probeDataFullFilename0, sizeof( probeDataFullFilename0 ), probeDataFilename0 );

                        /* rename list_1.csv to list_0.csv */
                        rename( probeDataFullFilename, probeDataFullFilename0 );
                        printf( "L%u - rename(%s,%s)\n", __LINE__, probeDataFullFilename, probeDataFullFilename0 );

                        PingPong = 1;
                        printf( "L%u - PingPong %d\n", __LINE__, PingPong );
                    }
                    else
                    {
                        PingPong = 0;
                        printf( "L%u - PingPong %d\n", __LINE__, PingPong );
                    }
                }
#endif

                /* write the new file */
                sprintf( probeDataFilename, "list_%d.csv", PingPong );
                PrependTempDirectory( probeDataFullFilename, sizeof( probeDataFullFilename ), probeDataFilename );
#if 1
                fp = fopen( probeDataFullFilename, "a");
                if ( fp ) {
                    fwrite( response, 1, strlen(response), fp );
                    fclose( fp );
                }
#else
                SetFileContents( probeDataFullFilename, response );
#endif
                printf("wrote %d bytes to (%s)\n", (unsigned)strlen(response), probeDataFullFilename );
            }

            gPowerProbeCurrent[probe] = fCurrent;
        }
        else if ( option == POWER_PROBE_RETURN_CONNECTED_PROBES )
        {
            char *eol = NULL;
            for( probe=0; probe<POWER_PROBE_MAX; probe++ )
            {
                sprintf( hostcmd, "voltcont lead_connected %d", probe );
                /*printf("for (%s) ... sending\n", hostcmd );*/
                bmemperf_send_data( socket_fd, __FUNCTION__, 0, NULL, hostcmd, strlen(hostcmd), NULL );

                memset( response, 0, sizeof(response) );
                /*printf("for (%s) ... getting response \n", hostcmd );*/
                bmemperf_get_response ( __FUNCTION__, socket_fd, "# ", response, sizeof(response), NULL );
                /*printf("for (%s) ... got response len (%d) (%s)\n", hostcmd, strlen(response), response );*/
                if ( ( eol = strstr( response, hostcmd ) ) )
                {
                    eol = strstr( eol, "\n" );
                    if ( eol )
                    {
                        printf( "for (%s) ... response is (%c)\n", hostcmd, eol[1] );
                        if ( eol[1] == '1' ) /* if the probe lead is connected to something */
                        {
                            gPowerProbeConnected[probe] = 1;
                        }
                        else
                        {
                            gPowerProbeConnected[probe] = 0;
                        }
                    }
                }
            }

            if( gPowerProbeQuitCount < 3 ) gPowerProbeQuitCount = 3; /* give the thread one more second to allow main server to read active probes */
        }

        gettimeofday(&tv2, NULL);
        microseconds2 = (tv2.tv_sec * 1000000LL);
        /*printf("microseconds2 %llu (%lu) (%06lu)\n", microseconds2, tv2.tv_sec, tv2.tv_usec );*/
        microseconds2 += tv2.tv_usec;
        /*printf("microseconds2 %llu\n", microseconds2 );
        printf("tv1 %lu.%06lu ... tv2 %lu.%06lu\n", tv1.tv_sec, tv1.tv_usec, tv2.tv_sec, tv2.tv_usec);*/
        microseconds_delta = (microseconds2 - microseconds1);
        /*printf("delta microseconds (%llu - %llu) ... %lu\n", microseconds2, microseconds1, microseconds_delta );*/

        /* if it took less than a full second (200000 usec) to get all of the probe data, wait the remaining part of a full second (800000 usec) */
        if ( microseconds_delta < 1000000 )
        {
            PRINTF( "%s:%lu:%lu need to usleep(%lu)\n", __FUNCTION__, (unsigned long int) getpid(), pthread_self(), (1000000 - microseconds_delta) );
            usleep( (1000000 - microseconds_delta) );
        }

        /* each time the server gets a request for power probe data, the server will set this to 0 */
        gPowerProbeQuitCount++;
    }   /* while gPowerProbeQuitCount has not max'ed out */

    bmemperf_send_data( socket_fd, __FUNCTION__, 0, NULL, "quit", 4, NULL );

    close( socket_fd );
    socket_fd = 0;

exit:
    gPowerProbeThreadId = 0;
    gPowerProbeTelnet_pid = 0;
    for(probe=0; probe<POWER_PROBE_MAX; probe++)
    {
        gPowerProbeShunts[probe] = 0;
        gPowerProbeVoltage[probe] = 0.0;
        gPowerProbeCurrent[probe] = 0.0;
        gPowerProbeConnected[probe] = 0;
    }

    printf( "%s:%lu:%lu exiting. gPowerProbeThreadId is 0\n", __FUNCTION__, (unsigned long int) getpid(), pthread_self() );

    /* if I call pthread_exit() in this thread, the parent thread will exit also */
    PRINTF( "%s:%lu:%lu sleeping()\n", __FUNCTION__, (unsigned long int) getpid(), pthread_self() );
    sleep( 123456789 );

    return NULL;
} /* bmemperf_power_probe_1000_thread */


/**
 *  Function: This function will start a new thread that will telnet to the specified IP address
 *            and gather Voltage and Current for the specified power probes.
 **/
static unsigned long int bmemperf_power_probe_1000_option = 0;
static int bmemperf_power_probe_1000_start(
    unsigned long int option
    )
{
    void                    *(*threadFunc)( void * );

    bmemperf_power_probe_1000_option = option;

    PRINTF("%s :%lu:%lu gPowerProbeThreadId %lu ... gPowerProbeTelnet_pid %lu\n", __FUNCTION__,
            (unsigned long int) getpid(), pthread_self(), (unsigned long int) gPowerProbeThreadId, gPowerProbeTelnet_pid );
    /* if the thread has not already been started, start it now */
    if ( gPowerProbeThreadId == 0 )
    {
        if ( gPowerProbeTelnet_pid == 0 )
        {
            /* start a background thread that will perform telnet I/O */
            threadFunc   = bmemperf_power_probe_1000_thread;

            if (pthread_create( &gPowerProbeThreadId, NULL, threadFunc, (void *)&bmemperf_power_probe_1000_option ))
            {
                printf( "%s :%lu:%lu could not create thread for bmemperf_power_probe_1000_thread; %s\n", __FUNCTION__,
                        (unsigned long int) getpid(), pthread_self(), strerror( errno ));
            }
            else
            {
                printf( "%s :%lu:%lu Thread for bmemperf_power_probe_1000_thread started successfully; id %lx\n", __FUNCTION__,
                        (unsigned long int) getpid(), pthread_self(), (long int) gPowerProbeThreadId );
            }
        }
    }
    else
    {
        int status = 0;
        PRINTF( "%s :%lu:%lu Thread for bmemperf_power_probe_1000_thread already started; id %lx\n", __FUNCTION__,
                (unsigned long int) getpid(), pthread_self(), (long int) gPowerProbeThreadId );
        waitpid( gPowerProbeTelnet_pid, &status, WNOHANG );

        /*printf( "%s :%lu:%lu after waitpid() ... status %d\n", __FUNCTION__, (unsigned long int) getpid(), pthread_self(), status );*/
        /* if the child has exited for some reason */
        if ( status == 13 )
        {
            printf( "%s :%lu:%lu PID %lu has exited with rc %d (%s)!!!!!\n", __FUNCTION__,
                    (unsigned long int) getpid(), pthread_self(), gPowerProbeTelnet_pid, status, strerror(errno) );
            gPowerProbeTelnet_pid = 0;
            gPowerProbeThreadId = 0;
        }
    }

    return( 0 );
} /* bmemperf_power_probe_1000_start */

#define CLIENT_STREAMER_MAX_THREADS 32
/**
 *  Function: This function will issue a command to the specified telnet socket.
 *            When the response comes back, it will search the response for instances
 *            of the wget command. There should be one wget line for each successfully
 *            launched streaming command. As the wget lines are found, the global
 *            array of wget PIDs will be updated.
 **/
static int bmemperf_client_streamer_search_threads( int socket_fd, int client )
{
    char *bol = NULL; /* beginning of line */
    char *eol = NULL; /* end of line */
    char  hostcmd[64];
    char  response[4096];

    memset( hostcmd, 0, sizeof(hostcmd) );
    memset( response, 0, sizeof(response) );

    sprintf( hostcmd, "ps -eaf | egrep \"wget|play\" | grep -v grep" );
    printffile( gRemoteClient[client].LogFile, "%s: client %d ... sending (%s)\n", __FUNCTION__, client, hostcmd );
    bmemperf_send_data( socket_fd, __FUNCTION__, 0, NULL, hostcmd, strlen(hostcmd), gRemoteClient[client].LogFile );
    /*usleep( 50 );*/
    memset( response, 0, sizeof(response) );
    bmemperf_get_response ( __FUNCTION__, socket_fd, "# " /*hostcmd*/, response, sizeof(response), gRemoteClient[client].LogFile );
    printffile( gRemoteClient[client].LogFile, "%s: client %d ... for (%s) ... got response length (%d)\n", __FUNCTION__, client, hostcmd, strlen(response) );
    printffile( gRemoteClient[client].LogFile, "%s: client %d ... (%s)\n", __FUNCTION__, client, response );

    /* scan the response to extract the pids of running wget threads */
    /*
       root      8519  8446  0 03:35 pts/0    00:00:00 wget -O /dev/null -q http://10.14.233.133:8089/DISCOVERY_HD_test.ts -U Referer:_0
       root      8482  8446  1 03:35 pts/0    00:00:00 wget -O /dev/null -q http://10.14.233.133:8089/DISCOVERY_HD_test.ts -U Referer:_1
       root      8483  8446  0 03:35 pts/0    00:00:00 wget -O /dev/null -q http://10.14.233.133:8089/DISCOVERY_HD_test.ts -U Referer:_2
       root      8487  8446  0 03:35 pts/0    00:00:00 wget -O /dev/null -q http://10.14.233.133:8089/DISCOVERY_HD_test.ts -U Referer:_3
       root      8526  8446  0 03:35 pts/0    00:00:00 wget -O /dev/null -q http://10.14.233.133:8089/DISCOVERY_HD_test.ts -U Referer:_4
       root      8533  8446  0 03:35 pts/0    00:00:00 wget -O /dev/null -q http://10.14.233.133:8089/DISCOVERY_HD_test.ts -U Referer:_5
       root      8537  8446  0 03:35 pts/0    00:00:00 wget -O /dev/null -q http://10.14.233.133:8089/DISCOVERY_HD_test.ts -U Referer:_6
       root      8544  8446  0 03:35 pts/0    00:00:00 wget -O /dev/null -q http://10.14.233.133:8089/DISCOVERY_HD_test.ts -U Referer:_7
       root      8551  8446  0 03:35 pts/0    00:00:00 wget -O /dev/null -q http://10.14.233.133:8089/DISCOVERY_HD_test.ts -U Referer:_8
       root      8561  8446  0 03:35 pts/0    00:00:00 wget -O /dev/null -q http://10.14.233.133:8089/DISCOVERY_HD_test.ts -U Referer:_9
    */
    bol = response;
    gRemoteClient[client].ActiveThreadIndex = 0;
    memset( gRemoteClient[client].ActiveThreadPids, 0, sizeof(gRemoteClient[client].ActiveThreadPids) ); /* array will be re-populated in the block below */
    memset( gRemoteClient[client].ActiveThreadReferers, 0, sizeof(gRemoteClient[client].ActiveThreadReferers) ); /* array will be re-populated in the block below */
    do
    {
        unsigned long int pid = 0;
        eol = strchr( bol, '\n');
        if ( eol )
        {
            *eol = '\0';
        }

        if ( strstr( bol, "wget -O " ) || strstr( bol, " play" ) )
        {
            char *space = strchr( bol, ' ');
            if ( space )
            {
                sscanf( space, "%lu", &pid );
                if ( pid )
                {
                    /* look for Referer: */
#define REFERER_TAG "Referer:_"
                    char *referer = strstr( bol, REFERER_TAG );
                    unsigned long int stream_idx = 0;

                    if ( gRemoteClient[client].ActiveThreadIndex < CLIENT_STREAMER_MAX_THREADS )
                    {
                        gRemoteClient[client].ActiveThreadPids[gRemoteClient[client].ActiveThreadIndex] = pid;
                    }

                    if ( referer )
                    {
                        referer += strlen( REFERER_TAG );
                        sscanf( referer, "%ld", &stream_idx );
                    }

                    if ( gRemoteClient[client].ActiveThreadIndex < CLIENT_STREAMER_MAX_THREADS )
                    {
                        gRemoteClient[client].ActiveThreadReferers[gRemoteClient[client].ActiveThreadIndex] = stream_idx;
                    }
                    gRemoteClient[client].ActiveThreadIndex++;
                    noprintf("%s: found wget pid (%lu) ... count %lu\n", __FUNCTION__, pid, gRemoteClient[client].ActiveThreadIndex );
                }
            }
        }
        if ( eol ) eol++;
        bol = eol;
        /*printflog("%s: while bol %p\n", __FUNCTION__, pid, bol );*/
    } while ( bol != NULL );
    return 0;
}   /* bmemperf_client_streamer_search_threads */

/**
 *  Function: This function will perform a telnet session to the specified client streamer IP address.
 **/
#define RESPONSE_SIZE 10240
static void *bmemperf_client_streamer_thread(
    void *data
    )
{
    struct sockaddr_in     srv;
    int                    idx          = 0;
    int                    client = 0; /* ranges from 0 .. BASPMON_MAX_NUM_CLIENTS */
    int                    socket_fd    = 0;
    int                    keepActive   = 0; /* Number of streams the browser wants us to keep active. If wget finishes, start another one. */
#if 0
    char                  *pos          = NULL;
    char                  *posBoaServer = NULL;
    char                   BoaServerPath[64];
#endif
    char                   ClientStreamerCommandLine[BASPMON_MAX_NUM_CLIENTS][BASPMON_MAX_NUM_STREAMS][128];
    char                   ClientStreamerTagline[32];
    char                   hostip[INET6_ADDRSTRLEN];
    char                   hostcmd[64];
    char                  *response = NULL;
    struct timeval         tv1, tv2;
    unsigned long long int microseconds1 = 0;
    unsigned long long int microseconds2 = 0;
    unsigned long int      microseconds_delta = 0;
    unsigned long int      retry_count        = 0;
    long int               recv_error         = 0;

    if (data == NULL)
    {
        printf( "%s:%lu arg1 cannot be NULL\n", __FUNCTION__, GETPID );
        goto exit;
    }

    client = (int) * (int*)data;
    memset( &tv1, 0, sizeof(tv1) );
    memset( &tv2, 0, sizeof(tv2) );
    gRemoteClient[client].QuitCount = 0;

    if ( strlen( gRemoteClient[client].IpAddr ) == 0 )
    {
        printf("%s:%lu gRemoteClientIpAddr[%d] is empty\n", __FUNCTION__, GETPID, client );
        goto exit;
    }

    response = malloc( RESPONSE_SIZE );
    if ( response == NULL )
    {
        printf("%s:%lu Client %d ... could not malloc(%d) bytes\n", __FUNCTION__, GETPID, client, RESPONSE_SIZE );
        goto exit;
    }

    strncpy ( hostip, gRemoteClient[client].IpAddr, sizeof(hostip) );

    /* create a connection to the server */
    srv.sin_family = AF_INET; // use the internet addr family
    srv.sin_port = htons( TELNET_PORT ); // dedicated telnet port = 23
    srv.sin_addr.s_addr = inet_addr( hostip );

    /* check to see if there are 16 commands for the remote client */
    for( idx=0; idx<BASPMON_MAX_NUM_STREAMS; idx++ )
    {
        sprintf( ClientStreamerTagline, "ClientStreamerCommandLine%d%d", client, idx );
        /* read the command line template from the configuration file */
        Bmemperf_GetCfgFileEntry( BASPMON_CFG_FILENAME, ClientStreamerTagline, &ClientStreamerCommandLine[client][idx][0],
                sizeof(ClientStreamerCommandLine[client][idx]) );
        printffile( gRemoteClient[client].LogFile, "%s:%lu in (%s) %s is (%s)\n", __FUNCTION__, GETPID, BASPMON_CFG_FILENAME,
                ClientStreamerTagline, &ClientStreamerCommandLine[client][idx][0] );
    }

RETRY:
    retry_count++;
    printffile( gRemoteClient[client].LogFile, "\n\n%s:%lu retry_count %lu\n", __FUNCTION__, GETPID, retry_count );

    if ( retry_count > 5 ) goto exit;

    /* create the socket */
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("%s:%lu socket() failed\n", __FUNCTION__, GETPID );
        /*perror("socket");*/
        goto exit;
    }

    /* connect to the client streamer's telnet server */
    printffile( gRemoteClient[client].LogFile, "%s:%lu Connecting to (%s) port %d\n", __FUNCTION__, GETPID, hostip, TELNET_PORT );
    if (connect(socket_fd, (struct sockaddr*) &srv, sizeof(srv)) < 0) {
        printffile( gRemoteClient[client].LogFile, "%s:%lu connect() failed\n", __FUNCTION__, GETPID );
        /*perror("connect");*/
        goto exit;
    } else {
        noprintf( "%s:%lu Connection to (%s) successful.\n\n", __FUNCTION__, GETPID, hostip );
    }

    memset( response, 0, RESPONSE_SIZE );
    recv_error = bmemperf_get_response ( __FUNCTION__, socket_fd, "login:", response, RESPONSE_SIZE, gRemoteClient[client].LogFile );
    printffile( gRemoteClient[client].LogFile, "%s:%lu ... after response login: rc %d\n", __FUNCTION__, GETPID, recv_error );
    if ( recv_error != 0 ) goto RETRY;


    printffile( gRemoteClient[client].LogFile, "%s:%lu sending User Name (root)\n", __FUNCTION__, GETPID );
    bmemperf_send_data( socket_fd, __FUNCTION__, 0, NULL, "root\n", 5, gRemoteClient[client].LogFile );
    /*usleep( 50 );*/
    memset( response, 0, RESPONSE_SIZE );
    recv_error = bmemperf_get_response ( __FUNCTION__, socket_fd, "# ", response, RESPONSE_SIZE, gRemoteClient[client].LogFile );
    printffile( gRemoteClient[client].LogFile, "%s:%lu ... after response root: rc %d\n", __FUNCTION__, GETPID, recv_error );
    if ( recv_error != 0 ) goto RETRY;

#if 0
    /* not needed if the only app we want to run is wget */
    /* we need to find out what directory contains the binaries */
    sprintf( hostcmd, "find / -name \"boa_server\"\n" );
    noprintffile( gRemoteClient[client].LogFile, "%s:%lu sending (%s)\n", __FUNCTION__, GETPID, hostcmd );
    bmemperf_send_data( socket_fd, __FUNCTION__, 0, NULL, hostcmd, strlen(hostcmd), gRemoteClient[client].LogFile );
    /*usleep( 50 );*/
    memset( response, 0, RESPONSE_SIZE );
    recv_error = bmemperf_get_response ( __FUNCTION__, socket_fd, "# ", response, RESPONSE_SIZE, gRemoteClient[client].LogFile );
    /*printffile( gRemoteClient[client].LogFile, "%s: got response (%s)\n", __FUNCTION__, response );*/
    if ( recv_error != 0 ) goto RETRY;
    posBoaServer = pos = strstr( response, "/boa_server" );
    if ( pos )
    {
        int count = 0;

        memset( &BoaServerPath, 0, sizeof(BoaServerPath) );

        /* search for beginning of boa_server line */
        while ( pos && pos != response && *pos != '\n' && count < 32 )
        {
            pos--;
            count++; /* failsafe ... do not look more than 32 characters */
        }

        if ( pos && *pos == '\n' )
        {
            pos++; /* Advance past the carriage return char. This should be the beginning of the path to boa_server. */
            *posBoaServer = '\0';
            strncpy( BoaServerPath, pos, sizeof(BoaServerPath) - 1 );
            noprintffile( gRemoteClient[client].LogFile, "%s: BoaServerPath is (%s)\n", __FUNCTION__, BoaServerPath );
        }
    }
#endif


    /* if we do not receive a request from the browser within 5 seconds, assume they stopped wanting client streamer info and exit */
    printffile( gRemoteClient[client].LogFile, "%s:%lu client %d ... %d\n", __FUNCTION__, GETPID, client, __LINE__ ); fflush(stderr); fflush(stdout);
    while ( ( gRemoteClient[client].QuitCount < 5 ) && ( gRemoteClient[client].Action != BMEMPERF_CMD_CLIENT_TERMINATE ) )
    {
        gettimeofday(&tv1, NULL);
        microseconds1 = (tv1.tv_sec * 1000000LL);
        microseconds1 += tv1.tv_usec;

        printf( "%s:%lu client %d ... gRemoteClientAction %ld ... gRemoteClientQuitCount %u ... gRemoteClientReset %d ... %s\n",
                __FUNCTION__, GETPID, client, gRemoteClient[client].Action, gRemoteClient[client].QuitCount, gRemoteClient[client].Reset, DateYyyyMmDdHhMmSs() );
        printffile( gRemoteClient[client].LogFile, "%s:%lu gRemoteClientAction %ld ... gRemoteClientQuitCount %u ... gRemoteClientReset %d ... %s\n",
                __FUNCTION__, GETPID, gRemoteClient[client].Action, gRemoteClient[client].QuitCount, gRemoteClient[client].Reset, DateYyyyMmDdHhMmSs() );

        keepActive += gRemoteClient[client].Action; /* could be plus or minus number */
        if ( gRemoteClient[client].Action ) printffile( gRemoteClient[client].LogFile, "%s:%lu Action %ld\n", __FUNCTION__, GETPID, gRemoteClient[client].Action );

        if ( gRemoteClient[client].Action == BMEMPERF_CMD_CLIENT_RESET )
        {
            sprintf( hostcmd, "/sbin/reboot" );
            printffile( gRemoteClient[client].LogFile, "%s:%lu sending (%s)\n", __FUNCTION__, GETPID, hostcmd );
            bmemperf_send_data( socket_fd, __FUNCTION__, 0, NULL, hostcmd, strlen(hostcmd), gRemoteClient[client].LogFile );
            /*usleep( 50 );*/
            memset( response, 0, RESPONSE_SIZE );
            recv_error = bmemperf_get_response ( __FUNCTION__, socket_fd, "# " /*hostcmd*/, response, RESPONSE_SIZE, gRemoteClient[client].LogFile );
            printffile( gRemoteClient[client].LogFile, "%s:%lu got response of (%d) bytes\n", __FUNCTION__, GETPID, strlen(response) );
            if ( recv_error != 0 ) goto RETRY;
        }

        else if ( gRemoteClient[client].Action > 0 ) /* if user requested we start a new stream */
        {
            unsigned long int gThreadIndex = gRemoteClient[client].ActiveThreadIndex;
            printffile( gRemoteClient[client].LogFile, "%s:%lu need to launch %ld threads ... gRemoteClientActiveThreadIndex %lu\n", __FUNCTION__, GETPID,
                    gRemoteClient[client].Action, gThreadIndex );
            for( idx=0; idx< gRemoteClient[client].Action; idx++ )
            {
                char *string_format= NULL;
                /* There could be 16 different launch commands in the CFG file; instead of always launching the 1st
                   one in the list, launch the 5th one ... or the 10th one ... or the 15th one ... based on how many
                   launch requests we have received from the browser. */
                string_format = strstr( &ClientStreamerCommandLine[client][idx + gThreadIndex ][0],  "%s" );
                if ( string_format )
                {
                    char *string_format= NULL;
                    /* There could be 16 different launch commands in the CFG file; instead of always launching the 1st
                       one in the list, launch the 5th one ... or the 10th one ... or the 15th one ... based on how many
                       launch requests we have received from the browser. */
                    string_format = strstr( &ClientStreamerCommandLine[client][idx + gThreadIndex ][0],  "%s" );
                    if ( string_format )
                    {
                        /* if there is a format tag for a string ... assume the format is for the server's IP address */
                        sprintf( hostcmd, &ClientStreamerCommandLine[client][idx + gThreadIndex ][0], gServerStreamerIpAddr );
                    }
                    else
                    {
                        sprintf( hostcmd, "%s", (char*) &(ClientStreamerCommandLine[idx + gThreadIndex ][0]) );
                    }
                    if ( strlen(hostcmd) )
                    {
                        printf("%s:%lu sending (%s)\n", __FUNCTION__, GETPID, hostcmd );
                        bmemperf_send_data( socket_fd, __FUNCTION__, 0, NULL, hostcmd, strlen(hostcmd), NULL );
                        usleep( 50 );
                        memset( response, 0, RESPONSE_SIZE );
                        bmemperf_get_response ( __FUNCTION__, socket_fd, "# " /*hostcmd*/, response, RESPONSE_SIZE, NULL );
                        printf("%s:%lu got response of (%d) bytes\n", __FUNCTION__, GETPID, (unsigned)strlen(response) );
                        /*printf("%s:%lu \n\n(%s) \n\n\n", __FUNCTION__, GETPID, response );*/
                    }
                    else
                    {
                        printf("%s:%lu ClientStreamerCommandLine[%lu] is empty (%s)\n", __FUNCTION__, GETPID,
                                idx + gRemoteClient[client].ActiveThreadIndex, hostcmd );
                    }
                }
            }
            gRemoteClient[client].Action = 0;
        }

        else if ( gRemoteClient[client].Action < 0 ) /* if user requested we stop a LIFO stream */
        {
            int idx=0;

            printffile( gRemoteClient[client].LogFile, "%s:%lu need to kill %ld threads ... gRemoteClientActiveThreadIndex %ld\n", __FUNCTION__, GETPID,
                    (long int) abs(gRemoteClient[client].Action), gRemoteClient[client].ActiveThreadIndex );
            printffile( gRemoteClient[client].LogFile, "%s:%lu pids: ", __FUNCTION__, GETPID );
            for( idx=0; idx < gRemoteClient[client].ActiveThreadIndex; idx++ )
            {
                printffile( gRemoteClient[client].LogFile,  "%ld/%ld ", gRemoteClient[client].ActiveThreadPids[idx], gRemoteClient[client].ActiveThreadReferers[idx] );
            }
            printffile( gRemoteClient[client].LogFile, "\n");

            for( idx=0; idx< abs(gRemoteClient[client].Action); idx++ )
            {
                printffile( gRemoteClient[client].LogFile,  "%s:%lu gRemoteClientActiveThreadIndex %ld ... gRemoteClientActiveThreadIndex %ld\n", __FUNCTION__,
                    GETPID, gRemoteClient[client].ActiveThreadIndex, gRemoteClient[client].ActiveThreadIndex );
                if ( ( gRemoteClient[client].ActiveThreadIndex > 0 ) && ( gRemoteClient[client].ActiveThreadIndex <= CLIENT_STREAMER_THREAD_MAX ) )
                {
                    printffile( gRemoteClient[client].LogFile,  "%s:%lu gRemoteClientActiveThreadPids[%ld] = %ld\n", __FUNCTION__, GETPID,
                            gRemoteClient[client].ActiveThreadIndex-1, gRemoteClient[client].ActiveThreadPids[gRemoteClient[client].ActiveThreadIndex-1] );
                    if ( gRemoteClient[client].ActiveThreadPids[gRemoteClient[client].ActiveThreadIndex-1] > 0 )
                    {
                        /* kill the last entry in the table */
                        sprintf( hostcmd, "kill -9 %lu", gRemoteClient[client].ActiveThreadPids[gRemoteClient[client].ActiveThreadIndex-1] );
                        gRemoteClient[client].ActiveThreadPids[gRemoteClient[client].ActiveThreadIndex] = 0;
                        gRemoteClient[client].ActiveThreadIndex--;
                        printffile( gRemoteClient[client].LogFile, "%s:%lu sending (%s) ... gRemoteClientActiveThreadIndex %ld\n", __FUNCTION__, GETPID,
                                hostcmd, gRemoteClient[client].ActiveThreadIndex );
                        bmemperf_send_data( socket_fd, __FUNCTION__, 0, NULL, hostcmd, strlen(hostcmd), gRemoteClient[client].LogFile );
                        /*usleep( 50 );*/
                        memset( response, 0, RESPONSE_SIZE );
                        recv_error = bmemperf_get_response ( __FUNCTION__, socket_fd, "# " /*hostcmd*/, response, RESPONSE_SIZE, gRemoteClient[client].LogFile );
                        printffile( gRemoteClient[client].LogFile, "%s:%lu got response of (%d) bytes\n", __FUNCTION__, GETPID, strlen(response) );
                        /*printffile( gRemoteClient[client].LogFile, "%s:%lu \n\n(%s) \n\n\n", __FUNCTION__, GETPID, response );*/
                        if ( recv_error != 0 ) goto RETRY;
                    }
                }
            }
            gRemoteClient[client].Action = 0;
        }

        /* search for wget threads that are still active */
        bmemperf_client_streamer_search_threads( socket_fd, client );

#if 0
        {
            int   relaunchMax = 0;

            /* if the number of active threads is less than we are expecting, re-launch another wget */
            relaunchMax = keepActive-gRemoteClient[client].ActiveThreadIndex;
            if ( relaunchMax > 0 && relaunchMax < CLIENT_STREAMER_THREAD_MAX )
            {
                int StreamsDetected[BASPMON_MAX_NUM_STREAMS];
                memset( StreamsDetected, -1, sizeof(StreamsDetected) );

#if 1
                printffile( gRemoteClient[client].LogFile,  "%s:%lu Found Referers ... ", __FUNCTION__, GETPID );
                for( idx=0; idx<gRemoteClient[client].ActiveThreadIndex+1; idx++ )
                {
                    printffile( gRemoteClient[client].LogFile,  "%ld ", gRemoteClient[client].ActiveThreadReferers[idx] );
                }
                printffile( gRemoteClient[client].LogFile,  "\n" );
#endif

                /* try to figure out if any previously running streams have died */
                for( idx=0; idx<gRemoteClient[client].ActiveThreadIndex; idx++ )
                {
                    if ( gRemoteClient[client].ActiveThreadReferers[idx] )
                    {
                        StreamsDetected[gRemoteClient[client].ActiveThreadReferers[idx] - 1] = gRemoteClient[client].ActiveThreadReferers[idx] ;
                    }
                }

#if 1
                printffile( gRemoteClient[client].LogFile,  "%s:%lu StreamsDetected ... ", __FUNCTION__, GETPID );
                for( idx=0; idx<gRemoteClient[client].ActiveThreadIndex+1; idx++ )
                {
                    printffile( gRemoteClient[client].LogFile,  "%d ", StreamsDetected[idx] );
                }
                printffile( gRemoteClient[client].LogFile,  "\n" );
#endif

                /* loop through again to find any missing Referers */
                for( idx=0; idx<gRemoteClient[client].ActiveThreadIndex+1; idx++ )
                {
                    /* if we could not find a referer for this stream, re-launch it */
                    if ( StreamsDetected[idx] == -1 && idx > 0 /* do not relaunch playmosaic */ )
                    {
                        sprintf( hostcmd, &ClientStreamerCommandLine[client][idx][0], gServerStreamerIpAddr );
                        printffile( gRemoteClient[client].LogFile, "%s:%lu relaunching [%d] (%s)\n", __FUNCTION__, GETPID, idx, hostcmd );
                        bmemperf_send_data( socket_fd, __FUNCTION__, 0, NULL, hostcmd, strlen(hostcmd), gRemoteClient[client].LogFile );
                        /*usleep( 50 );*/
                        memset( response, 0, RESPONSE_SIZE );
                        recv_error = bmemperf_get_response ( __FUNCTION__, socket_fd, "# " /*hostcmd*/, response, RESPONSE_SIZE, gRemoteClient[client].LogFile );
                        printffile( gRemoteClient[client].LogFile, "%s:%lu got response of (%d) bytes\n", __FUNCTION__, GETPID, strlen(response) );
                        /*printffile( gRemoteClient[client].LogFile, "%s:%lu \n\n(%s) \n\n\n", __FUNCTION__, GETPID, response );*/
                        if ( recv_error != 0 ) goto RETRY;
                    }
                }

                usleep( 100 );

                /* after we have re-launched one or more threads, scan for active wget pids */
                bmemperf_client_streamer_search_threads( socket_fd, client );
            }
        }
#endif
#if 1
        printffile( gRemoteClient[client].LogFile,  "%s:%lu Active Pids (%ld) ... ", __FUNCTION__, GETPID, gRemoteClient[client].ActiveThreadIndex);
        for( idx=0; idx<gRemoteClient[client].ActiveThreadIndex; idx++ )
        {
            printffile( gRemoteClient[client].LogFile,  "%ld/%ld ", gRemoteClient[client].ActiveThreadPids[idx], gRemoteClient[client].ActiveThreadReferers[idx] );
        }
        printffile( gRemoteClient[client].LogFile,  "\n" );
#endif

        gettimeofday(&tv2, NULL);
        microseconds2 = (tv2.tv_sec * 1000000LL);
        microseconds2 += tv2.tv_usec;
        microseconds_delta = (microseconds2 - microseconds1);

        if ( microseconds_delta > 980000 ) microseconds_delta = 980000;

        /* if it took less than a full second (e.g. 200000 usec) to get all of the data, wait the remaining part of a full second (800000 usec) */
        if ( microseconds_delta < 1000000 )
        {
            printffile( gRemoteClient[client].LogFile,  "%s:%lu need to usleep(%lu)\n", __FUNCTION__, GETPID, (1000000 - microseconds_delta) );
            usleep( (1000000 - microseconds_delta) );
        }

        /* each time the server gets a request for client streamer data, the server will set this to 0 */
        gRemoteClient[client].QuitCount++;
        if ( gRemoteClient[client].QuitCount > 1 ) printffile( gRemoteClient[client].LogFile,  "%s:%lu gRemoteClientQuitCount (%u)\n", __FUNCTION__,
                GETPID, gRemoteClient[client].QuitCount );
    }   /* while gRemoteClientQuitCount < 5 */

    printffile( gRemoteClient[client].LogFile,  "%s:%lu sending (%s) ... gRemoteClientActiveThreadIndex %ld\n", __FUNCTION__, GETPID, "quit",
            gRemoteClient[client].ActiveThreadIndex );
    bmemperf_send_data( socket_fd, __FUNCTION__, 0, NULL, "quit", 4, gRemoteClient[client].LogFile );

    close( socket_fd );
    socket_fd = 0;

exit:
    Bsysperf_Free( response );
    response = NULL;

    gRemoteClient[client].ThreadId = 0;
    gRemoteClient[client].Telnet_pid = 0;
    gRemoteClient[client].ActiveThreadIndex = 0;

    printffile( gRemoteClient[client].LogFile,  "%s:%lu exiting. gRemoteClientThreadId is 0\n", __FUNCTION__, GETPID );
    printf( "%s:%lu exiting. gRemoteClientThreadId is 0\n", __FUNCTION__, GETPID );

    /* if I call pthread_exit() in this thread, the parent thread will exit also */
    sleep( 123456789 );

    return NULL;
} /* bmemperf_client_streamer_thread */


/**
 *  Function: This function will perform a telnet session to the specified client streamer IP address and reboot it.
 **/
static void *bmemperf_client_streamer_reboot(
    int client
    )
{
    struct sockaddr_in     srv;
    int                    socket_fd    = 0;
    char                   hostip[INET6_ADDRSTRLEN];
    char                   hostcmd[64];
    char                  *response = NULL;

    response = malloc( RESPONSE_SIZE );
    if ( response == NULL )
    {
        printf("%s:%lu Client %d ... could not malloc(%d) bytes\n", __FUNCTION__, GETPID, client, RESPONSE_SIZE );
        goto exit;
    }

    printf("%s:%lu Client %d ... IP (%s)\n", __FUNCTION__, GETPID, client, gRemoteClient[client].IpAddr );
    strncpy ( hostip, gRemoteClient[client].IpAddr, sizeof(hostip) );

    /* create a connection to the server */
    srv.sin_family = AF_INET; // use the internet addr family
    srv.sin_port = htons( TELNET_PORT ); // dedicated telnet port = 23
    srv.sin_addr.s_addr = inet_addr( hostip );

    /* create the socket */
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("%s:%lu socket() failed\n", __FUNCTION__, GETPID );
        /*perror("socket");*/
        goto exit;
    }

    /* connect to the client streamer's telnet server */
    printffile( gRemoteClient[client].LogFile, "%s:%lu Connecting to (%s) port %d\n", __FUNCTION__, GETPID, hostip, TELNET_PORT );
    if (connect(socket_fd, (struct sockaddr*) &srv, sizeof(srv)) < 0) {
        printffile( gRemoteClient[client].LogFile, "%s:%lu connect() failed\n", __FUNCTION__, GETPID );
        /*perror("connect");*/
        goto exit;
    } else {
        noprintf( "%s:%lu Connection to (%s) successful.\n\n", __FUNCTION__, GETPID, hostip );
    }

    memset( response, 0, RESPONSE_SIZE );
    bmemperf_get_response ( __FUNCTION__, socket_fd, "login:", response, RESPONSE_SIZE, gRemoteClient[client].LogFile );


    printffile( gRemoteClient[client].LogFile, "%s:%lu sending User Name (root)\n", __FUNCTION__, GETPID );
    bmemperf_send_data( socket_fd, __FUNCTION__, 0, NULL, "root\n", 5, gRemoteClient[client].LogFile );
    memset( response, 0, RESPONSE_SIZE );
    bmemperf_get_response ( __FUNCTION__, socket_fd, "# ", response, RESPONSE_SIZE, gRemoteClient[client].LogFile );
    /*usleep( 50 );*/

    sprintf( hostcmd, "/sbin/reboot" );
    printffile( gRemoteClient[client].LogFile, "%s:%lu sending (%s)\n", __FUNCTION__, GETPID, hostcmd );
    bmemperf_send_data( socket_fd, __FUNCTION__, 0, NULL, hostcmd, strlen(hostcmd), gRemoteClient[client].LogFile );
    /*usleep( 50 );*/
    memset( response, 0, RESPONSE_SIZE );
    bmemperf_get_response ( __FUNCTION__, socket_fd, "# " /*hostcmd*/, response, RESPONSE_SIZE, gRemoteClient[client].LogFile );
    printffile( gRemoteClient[client].LogFile, "%s:%lu got response of (%d) bytes\n", __FUNCTION__, GETPID, strlen(response) );

exit:
    Bsysperf_Free( response );
    response = NULL;
    return 0;
} /* bmemperf_client_streamer_reboot */

/**
 *  Function: This function will start a new thread that will telnet to the specified IP address.
 **/
static int bmemperf_client_streamer_start(
    long int client,
    long int option
    )
{
    void                    *(*threadFunc)( void * );
    static unsigned long int threadOption = 0;

    printf("%s:%lu gRemoteClientThreadId %lx ... gRemoteClientTelnet_pid %lu ... Action %ld ... client %ld\n", __FUNCTION__,
            GETPID, (unsigned long int) gRemoteClient[client].ThreadId, gRemoteClient[client].Telnet_pid, option, client );
    /* if the thread has not already been started, start it now */
    if ( gRemoteClient[client].ThreadId == 0 )
    {
        if ( gRemoteClient[client].Telnet_pid == 0 )
        {
            /* start a background thread that will perform telnet I/O */
            threadFunc   = bmemperf_client_streamer_thread;
            threadOption = client;
            gRemoteClient[client].Action = option;

            if (pthread_create( &gRemoteClient[client].ThreadId, NULL, threadFunc, (void *)&threadOption ))
            {
                printf( "%s:%lu could not create thread for bmemperf_client_streamer_thread; %s\n", __FUNCTION__,
                        GETPID, strerror( errno ));
            }
            else
            {
                noprintf( "%s:%lu Thread for bmemperf_client_streamer_thread started successfully; id %lx\n", __FUNCTION__,
                        GETPID, (long int) gRemoteClient[client].ThreadId );
            }
        }
    }
    else
    {
        int status = 0;

        /* if the previous request is still attempting to complete, do not overwrite the global var */
        if ( option != 0 ) gRemoteClient[client].Action = option;

        printf( "%s:%lu Thread for bmemperf_client_streamer_thread already started; id %lx\n", __FUNCTION__,
                GETPID, (long int) gRemoteClient[client].ThreadId );
        waitpid( gRemoteClient[client].Telnet_pid, &status, WNOHANG );

        /*printf( "%s:%lu after waitpid() ... status %d\n", __FUNCTION__, GETPID, status );*/
        /* if the child has exited for some reason */
        if ( status == 13 )
        {
            printf( "%s:%lu PID %lu has exited with rc %d (%s)!!!!!\n", __FUNCTION__, GETPID, gRemoteClient[client].Telnet_pid, status, strerror(errno) );
            gRemoteClient[client].Telnet_pid = 0;
            gRemoteClient[client].ThreadId = 0;
        }
    }

    return( 0 );
} /* bmemperf_client_streamer_start */
/**
 *  Function: This function will start a new thread that will save he output from the "top" utility.
 **/
static int bmemperf_start_linux_top(
    unsigned long int option
    )
{
    static pthread_t         linuxTopThreadId = 0;
    void                    *(*threadFunc)( void * );
    static unsigned long int threadOption = 0;

    /* if the thread has not already been started, start it now */
    if (( linuxTopThreadId == 0 ) && ( option == BMEMPERF_CMD_START_LINUX_TOP ))
    {
        threadFunc   = bmemperf_linux_top_thread;
        threadOption = 0;

        bmemperf_linux_top_stop = false;

        signal( SIGTERM, bmemperf_linux_top_sigterm );

        if (pthread_create( &linuxTopThreadId, NULL, threadFunc, (void *)&threadOption ))
        {
            printf( "%s: could not create thread for linuxTop; %s\n", __FUNCTION__, strerror( errno ));
        }
        else
        {
            PRINTF( "%s: Thread for LinuxTop started successfully; id %lx\n", __FUNCTION__, (long int) linuxTopThreadId );
        }
    }
    else
    {
        PRINTF( "%s: Thread for LinuxTop already started; id %lx\n", __FUNCTION__, (long int) linuxTopThreadId );
        if (option == BMEMPERF_CMD_STOP_LINUX_TOP)
        {
            PRINTF( "%s: Stopping LinuxTop; id %lx\n", __FUNCTION__, (long int) linuxTopThreadId );
            bmemperf_linux_top_stop = true;
            if (linuxTopThreadId)
            {
                PRINTF( "%s: pthread_kill ( %lx ) \n", __FUNCTION__, (long int) linuxTopThreadId );
                pthread_kill( linuxTopThreadId, SIGTERM );
                linuxTopThreadId = 0;
            }
        }
    }

    return( 0 );
} /* bmemperf_start_linux_top */

/**
 *  Function: This function will read the user's request coming in from the browser
 **/
static int Bmemperf_ReadRequest(
    int                psd,
    struct sockaddr_in from,
    bmemperf_request  *pRequest,
    bmemperf_response *pResponse
    )
{
    int             rc         = 0;
    static bool     bFirstPass = true;
    static long int passcount  = 0;

    PRINTF( "%s - %u Started new thread serving %s ... passcount %ld\n", __FUNCTION__, ntohs( from.sin_port ), inet_ntoa( from.sin_addr), passcount );

    if (initializationDone == false)
    {
        PRINTF( "%s: memset(g_client_list to 0xff\n", __FUNCTION__ );
        memset( g_client_list, 0xFF, sizeof( g_client_list ));
        initializationDone = true;
    }

    if (( passcount == 0 ))
    {
        /* tell the server to reset its error counts back to zero */
        pRequest->cmd = BMEMPERF_CMD_RESET_ARB_ERRORS;
        rc = 1;
    }
    else if (( passcount == 1 ))
    {
        int memc, client;
        bmemperf_clientInfo     clientInfo; /* ids of the 10 clients for which the user can request details */
        bmemperf_columnTypes    sortColumn = BMEMPERF_COLUMN_MAX; /* needs to be MAX; bmemperf_server.c uses MAX value to determine if user has selected a different column on which to sort*/

        memset( &clientInfo, BMEMPERF_CGI_INVALID, sizeof( clientInfo ));

        /* tell the server to reset its error counts back to zero */
        pRequest->cmd = BMEMPERF_CMD_RESET_ARB_ERRORS;
        memset( pRequest, 0, sizeof( *pRequest ));
        memset( pResponse, 0, sizeof( *pResponse ));

        for (memc = 0; memc<NEXUS_NUM_MEMC; memc++)
        {
            for (client = 0; client<BMEMPERF_MAX_SUPPORTED_CLIENTS; client++)
            {
                pRequest->request.client_stats_data.client_list[memc][client] = clientInfo.clientIds[memc][client];
            }
            pRequest->request.client_stats_data.sortColumn[memc] = sortColumn;
        }
        rc = 1;
    }

#if 0
    if (( bFirstPass == true ))
    {
        unsigned int memc, client;

        /** initialize client list with client_id any say 10 for 0th index and INVALID for rest of the index ***/
        printf( "%s calling bmemperf_InitClientList()\n", __FUNCTION__ );
        bmemperf_InitClientList();

        printf( "%s calling bmemperf_init()\n", __FUNCTION__ );
        bmemperf_init();

        printf( "%s calling reset_error_counts()\n", __FUNCTION__ );
        reset_error_counts();

        for (memc = 0; memc<BMEMPERF_NUM_MEMC; memc++)
        {
            for (client = 0; client<BMEMPERF_MAX_NUM_CLIENT; client++)
            {
                g_savedClientDataSorted[memc][client].err_cnt = 0;
            }
        }
    }
#endif

    /* get data from  clients and send it back */
    PRINTF( "%u Server is reading request (%u bytes) from client ... \n", ntohs( from.sin_port ), sizeof( *pRequest ));

    if ( passcount >= 2 )
    {
        rc = recv( psd, pRequest, sizeof( *pRequest ), 0 );
    }

    if ( rc < 0)
    {
        PRINTF( "%u ", ntohs( from.sin_port ));
        CloseAndExit( psd, "receiving stream  message" );
    }

    if (rc > 0)
    {
        PRINTF( "%u %s: Received: from TCP/Client (%s); cmd (%u) \n", ntohs( from.sin_port ), DateYyyyMmDdHhMmSs(), inet_ntoa( from.sin_addr ), pRequest->cmd );
        if ( pRequest->cmdSecondary || pRequest->cmdSecondaryOption )
        {
            printf( "%s: cmd %u; cmdOption 0x%lx; cmdSecondary 0x%lx\n", __FUNCTION__, pRequest->cmd, (long int) pRequest->cmdSecondary,
                    pRequest->cmdSecondaryOption );
        }
        switch (pRequest->cmd) {
            case BMEMPERF_CMD_RESET_ARB_ERRORS:
            {
                unsigned int memc, client;

                reset_error_counts();

                for (memc = 0; memc<BMEMPERF_NUM_MEMC; memc++)
                {
                    for (client = 0; client<BMEMPERF_MAX_NUM_CLIENT; client++)
                    {
                        g_savedClientDataSorted[memc][client].err_cnt = 0;
                    }
                }
                break;
            }
            case BMEMPERF_CMD_GET_OVERALL_STATS:
            {
                unsigned int memc, client;

                sprintf( pResponse->padding, "====================this is response; boxmode %d ============================", g_boxmodeSource.boxmode );
                pResponse->cmd = pRequest->cmd;

                /*printf( "BMEMPERF_CMD_GET_OVERALL_STATS: boxmode request %d; prev %d\n", pRequest->boxmode, g_boxmodeSource.boxmode );*/
                /* check to see if the user has requested a boxmode change */
                if (pRequest->boxmode != g_boxmodeSource.boxmode)
                {
                    int memc;
                    int client;

                    printf( "Changing boxmode from %d to %d\n", g_boxmodeSource.boxmode, pRequest->boxmode );
                    bmemperf_boxmode_init( pRequest->boxmode );
                    g_boxmodeSource.boxmode = pRequest->boxmode;
                    g_boxmodeSource.source  = BOXMODE_BROWSER;
                    for (memc = 0; memc<BMEMPERF_NUM_MEMC; memc++)
                    {
                        for (client = 0; client<BMEMPERF_MAX_SUPPORTED_CLIENTS; client++)
                        {
                            g_client_list[memc][client] = BMEMPERF_CGI_INVALID;
                        }
                    }
                }

                pResponse->boxmode = g_boxmodeSource.boxmode;
                pResponse->source  = g_boxmodeSource.source;

                /* transfer requests to global server's structure; this allows multiple browser clients to make requests */
                for (memc = 0; memc<BMEMPERF_NUM_MEMC; memc++)
                {
                    for (client = 0; client<BMEMPERF_MAX_SUPPORTED_CLIENTS; client++)
                    {
                        if (pRequest->request.client_stats_data.client_list[memc][client] != BMEMPERF_CGI_INVALID)
                        {
                            PRINTF( "\n\n%s: action reqeusted for 0x%x: client_id %u for memc %u\n", __FUNCTION__,
                                pRequest->request.client_stats_data.client_list[memc][client],
                                pRequest->request.client_stats_data.client_list[memc][client] & 0xFF, memc );
                            /* if we are enabling the client id */
                            if (pRequest->request.client_stats_data.client_list[memc][client] & 0x100)
                            {
                                PRINTF( "%s: enable client_id %u for memc %u\n", __FUNCTION__, pRequest->request.client_stats_data.client_list[memc][client] & 0xFF, memc );
                                addClientIdToList( pRequest->request.client_stats_data.client_list[memc][client] & 0xFF, memc );
                            }
                            else
                            {
                                PRINTF( "%s: DISABLE client_id %u for memc %u\n", __FUNCTION__, pRequest->request.client_stats_data.client_list[memc][client] & 0xFF, memc );
                                deleteClientIdFromList( pRequest->request.client_stats_data.client_list[memc][client] & 0xFF, memc );
                            }
                        }
                    }
                }

                /* this sets the client list */
                bmemperf_getOverallStats( &pResponse->response.overallStats ); /* sets .client_id */

                /* set flag if one or more of the top 10 clients is also being detailed */
                for (memc = 0; memc<BMEMPERF_NUM_MEMC; memc++)
                {
                    for (client = 0; client<BMEMPERF_MAX_SUPPORTED_CLIENTS; client++)
                    {
                        /* if client_id is being detailed */
                        if (g_client_list[memc][client] != BMEMPERF_CGI_INVALID)
                        {
                            unsigned int client256;
                            for (client256 = 0; client256<BMEMPERF_MAX_NUM_CLIENT; client256++)
                            {
                                if (pResponse->response.overallStats.clientOverallStats[memc].clientData[client256].client_id == g_client_list[memc][client])
                                {
                                    pResponse->response.overallStats.clientOverallStats[memc].clientData[client256].is_detailed = true;
                                    PRINTF( "%s: pResponse->response.overallStats.clientOverallStats[%u].clientData[%-3u].is_detailed - %u\n", __FUNCTION__, memc,
                                        client256, pResponse->response.overallStats.clientOverallStats[memc].clientData[client256].client_id );
                                    break;
                                }
                            }
                        }
                    }
                }

                /*printf( "%s: bFirstPass %u\n", __FUNCTION__, bFirstPass );*/
                if (( bFirstPass == true ))
                {
                    unsigned int memc, client;
                    bFirstPass = false;
                    PRINTF( "%s: saving client array sized [%u][%u] to g_savedClientDataSorted array; saved size %u; src size %u\n", __FUNCTION__,
                        BMEMPERF_NUM_MEMC, BMEMPERF_MAX_NUM_CLIENT, sizeof( g_savedClientDataSorted[memc][client] ),
                        sizeof( pResponse->response.overallStats.clientOverallStats[memc].clientData[client] ));
                    for (memc = 0; memc<BMEMPERF_NUM_MEMC; memc++)
                    {
                        for (client = 0; client<BMEMPERF_MAX_NUM_CLIENT; client++)
                        {
                            g_savedClientDataSorted[memc][client] = pResponse->response.overallStats.clientOverallStats[memc].clientData[client];
                        }
                    }
                }
                else
                {
                    unsigned int memc, client;
                    /* clear out previous bw values (for this pass, client may not have any bw usage in the past second ) */
                    for (memc = 0; memc<BMEMPERF_NUM_MEMC; memc++)
                    {
                        for (client = 0; client<BMEMPERF_MAX_NUM_CLIENT; client++)
                        {
                            g_savedClientDataSorted[memc][client].bw = 0;
                        }
                    }

                    /* update or add current data with saved data */
                    for (memc = 0; memc<BMEMPERF_NUM_MEMC; memc++)
                    {
                        for (client = 0; client<BMEMPERF_MAX_NUM_CLIENT; client++)
                        {
                            if (pResponse->response.overallStats.clientOverallStats[memc].clientData[client].client_id != BMEMPERF_CGI_INVALID)
                            {
                                updateClient( pResponse->response.overallStats.clientOverallStats[memc].clientData[client].client_id, memc,
                                    &pResponse->response.overallStats.clientOverallStats[memc].clientData[client] );
                            }
                        }

                        bmemperf_clear_arb_err( memc );
                    }

                    memcpy( &pResponse->response.overallStats.clientOverallStats, &g_savedClientDataSorted, sizeof( g_savedClientDataSorted ));
                }

                bmemperf_computeIrqData( pResponse->response.overallStats.cpuData.numActiveCpus, &pResponse->response.overallStats.irqData );

                bmemperf_getCpuUtilization( &pResponse->response.overallStats.cpuData );

                /*printf("PowerProbeIpAddr (%s) ... PowerProbeSelect %d ... cmdSecondaryOption %ld ... QuitCount %u\n",
                        pRequest->request.overall_stats_data.PowerProbeIpAddr,
                        pRequest->request.overall_stats_data.PowerProbeSelect, pRequest->cmdSecondaryOption, gPowerProbeQuitCount );*/
                if ( pRequest->request.overall_stats_data.PowerProbeIpAddr[0] == '1' &&
                     strlen( pRequest->request.overall_stats_data.PowerProbeIpAddr ) > 0 )
                {
                    int idx=0;

                    /* if we stop getting requests, the PowerProbeThread will count up to 5 seconds and then exit */
                    gPowerProbeQuitCount = 0;

                    /* copy the user's request data into global variables for use by PowerProbeThread */
                    strncpy( gPowerProbeIpAddr, pRequest->request.overall_stats_data.PowerProbeIpAddr, sizeof(gPowerProbeIpAddr) );
                    for(idx=0; idx<POWER_PROBE_MAX; idx++)
                    {
                        gPowerProbeShunts[idx] = pRequest->request.overall_stats_data.PowerProbeShunts[idx];
                    }

                    if ( pRequest->request.overall_stats_data.PowerProbeSelect == POWER_PROBE_BASPMON )
                    {
                        bmemperf_power_probe_start( pRequest->cmdSecondaryOption );
                    }
                    else if ( pRequest->request.overall_stats_data.PowerProbeSelect == POWER_PROBE_BPROBE )
                    {
                        bmemperf_power_probe_1000_start( pRequest->cmdSecondaryOption );
                        usleep( 500000 ); /* give thread a tenth of a second to complete */
                    }

                    /* copy PowerProbeThread's data into user's response buffer */
                    for(idx=0; idx<POWER_PROBE_MAX; idx++)
                    {
                        pResponse->response.overallStats.PowerProbeVoltage[idx] = gPowerProbeVoltage[idx];
                        pResponse->response.overallStats.PowerProbeCurrent[idx] = gPowerProbeCurrent[idx];
                        pResponse->response.overallStats.PowerProbeConnected[idx] = gPowerProbeConnected[idx];
                        /*printf( "%s: Probe %d is %d\n", __FUNCTION__, idx, pResponse->response.overallStats.PowerProbeConnected[idx] );*/
                    }
                }

                if ( pRequest->cmdSecondaryOption == BMEMPERF_CMD_CLIENT_RESET )
                {
                    char  line[32];
                    void *junk = bmemperf_client_streamer_reboot;
                    int   telnetPortActive = 0;

                    printf( "%d ... BMEMPERF_CMD_CLIENT_RESET detected (%p)\n", __LINE__, junk );
#if 1
                    for( client=0; client<BASPMON_MAX_NUM_CLIENTS; client++ )
                    {
                        telnetPortActive = Bmemperf_Ping( pRequest->request.overall_stats_data.ClientStreamerIpAddr[client], TELNET_PORT );
                        if ( telnetPortActive != -1 )
                        {
                            /* copy the user's request data into global variables for use by ClientStreamerThread */
                            strncpy( gRemoteClient[client].IpAddr, pRequest->request.overall_stats_data.ClientStreamerIpAddr[client], INET6_ADDRSTRLEN - 1 );
                            strncpy( gServerStreamerIpAddr, pRequest->request.overall_stats_data.ServerStreamerIpAddr, sizeof(gServerStreamerIpAddr) );
                            bmemperf_client_streamer_reboot( client );
                        }
                    }
#endif

                    sprintf( line, "/sbin/reboot" );
                    system( line ); /* the system will reboot with this command ... control will never come back to here */
                }

                if ( pRequest->cmdSecondaryOption == BMEMPERF_CMD_CLIENT_TERMINATE )
                {
                    printf( "%s:%d ... BMEMPERF_CMD_CLIENT_TERMINATE detected\n", __FUNCTION__, __LINE__ );
                    for( client=0; client<BASPMON_MAX_NUM_CLIENTS; client++ )
                    {
                        gRemoteClient[client].QuitCount = 5;
                    }
                }
                else
                {
                    for( client=0; client<BASPMON_MAX_NUM_CLIENTS; client++ )
                    {
#if 0
                        if ( pRequest->cmdSecondaryOption )
                        {
                             printf( "%d ... Client %d ... IP (%-13s) Action %ld (0x%lx) ... QuitCount %d \n", __LINE__, client,
                                   pRequest->request.overall_stats_data.ClientStreamerIpAddr[client], pRequest->cmdSecondaryOption,
                                   pRequest->cmdSecondaryOption, gRemoteClient[client].QuitCount );
                        }
#endif
                        if ( ( pRequest->request.overall_stats_data.ClientStreamerIpAddr[client][0] == '1' &&
                             strlen( pRequest->request.overall_stats_data.ClientStreamerIpAddr[client]) > 0 ) || pRequest->cmdSecondary )
                        {
                            noprintf( "%d ... Client %d ... cmdSecondary %u ... cmdSecondaryOption %ld\n", __LINE__, client,
                            pRequest->cmdSecondary, (long int) pRequest->cmdSecondaryOption );
                            if ( pRequest->cmdSecondaryOption || pRequest->cmdSecondary )
                            {
                                int idx = abs(pRequest->cmdSecondaryOption)%10;

                                if ( idx == client )
                                {
                                    long int cmd2ndOption = (long int) pRequest->cmdSecondaryOption;
                                    printf("REQUEST: ClientIp (%s) ... ServerIp (%s) ... cmd2nd 0x%x ... cmd2ndOption %ld\n",
                                        pRequest->request.overall_stats_data.ClientStreamerIpAddr[client],
                                        pRequest->request.overall_stats_data.ServerStreamerIpAddr, pRequest->cmdSecondary, cmd2ndOption );

                                    /* copy the user's request data into global variables for use by ClientStreamerThread */
                                    strncpy( gRemoteClient[client].IpAddr, pRequest->request.overall_stats_data.ClientStreamerIpAddr[client], INET6_ADDRSTRLEN - 1 );
                                    strncpy( gServerStreamerIpAddr, pRequest->request.overall_stats_data.ServerStreamerIpAddr, sizeof(gServerStreamerIpAddr) );

                                    bmemperf_client_streamer_start( client, (long int) pRequest->cmdSecondaryOption/10 );
                                }
                            }
                            /* if we stop getting requests, the ClientStreamerThread will count up to 5 seconds and then exit */
                            if ( gRemoteClient[client].QuitCount < 5 ) gRemoteClient[client].QuitCount = 0;

                            /* copy ClientStreamerThread's data into user's response buffer */
                            pResponse->response.overallStats.ClientStreamerThreadCount[client] = gRemoteClient[client].ActiveThreadIndex;
                        }
                    }
                }

                if ( passcount >= 2)
                {
                    PRINTF( "%s: sending cmd (%u); %u bytes\n", __FUNCTION__, pResponse->cmd, sizeof( *pResponse ));
                    if (send( psd, pResponse, sizeof( *pResponse ), 0 ) < 0)
                    {
                        CloseAndExit( psd, "sending response cmd" );
                    }
                }
                break;
            }
            case BMEMPERF_CMD_GET_CLIENT_STATS:
            {
                pResponse->cmd = pRequest->cmd;

#if defined ( BMEMPERF_CGI_DEBUG )
                {
                    unsigned int memc = 0, client = 0;
                    int          temp;
                    for (memc = 0; memc<NEXUS_NUM_MEMC; memc++)
                    {
                        printf( "%s: step1 memc %u: ", __FUNCTION__, memc );
                        for (client = 0; client<BMEMPERF_MAX_SUPPORTED_CLIENTS; client++)
                        {
                            temp = g_client_list[memc][client];
                            printf( "%3u:%-4d ", client, temp );
                        }
                        printf( "\n" );
                    }
                }
#endif /* ifdef BMEMPERF_CGI_DEBUG */

                setClientList( g_client_list );

#if defined ( BMEMPERF_CGI_DEBUG )
                {
                    unsigned int memc = 0, client = 0;
                    for (memc = 0; memc<NEXUS_NUM_MEMC; memc++)
                    {
                        printf( "%s: step2 memc %u: ", __FUNCTION__, memc );
                        for (client = 0; client<BMEMPERF_MAX_SUPPORTED_CLIENTS; client++)
                        {
                            printf( "%3u:%-4u ", pResponse->response.clientDetailStats.clientStats[memc].perClientStats[client].clientId,
                                pResponse->response.clientDetailStats.clientStats[memc].perClientStats[client].clientDataBW );
                        }
                        printf( "\n" );
                    }
                }
#endif /* ifdef BMEMPERF_CGI_DEBUG */

                bmemperf_getClientStats( &pResponse->response.clientDetailStats );

#if defined ( BMEMPERF_CGI_DEBUG )
                {
                    unsigned int memc = 0, client = 0;
                    for (memc = 0; memc<NEXUS_NUM_MEMC; memc++)
                    {
                        printf( "%s: step3 memc %u: ", __FUNCTION__, memc );
                        for (client = 0; client<BMEMPERF_MAX_SUPPORTED_CLIENTS; client++)
                        {
                            printf( "%3u:%-4u ", pResponse->response.clientDetailStats.clientStats[memc].perClientStats[client].clientId,
                                pResponse->response.clientDetailStats.clientStats[memc].perClientStats[client].clientDataBW );
                        }
                        printf( "\n" );
                    }
                }
#endif /* ifdef BMEMPERF_CGI_DEBUG */

                if ( passcount >= 2 )
                {
                    PRINTF( "%s: sending cmd (%u); %u bytes\n", __FUNCTION__, pResponse->cmd, sizeof( *pResponse ));
                    if (send( psd, pResponse, sizeof( *pResponse ), 0 ) < 0)
                    {
                        CloseAndExit( psd, "sending response cmd" );
                    }
                }
                break;
            }
            case BMEMPERF_CMD_SET_CLIENT_BOVAL_RRFLAG:
            {
                pResponse->cmd = pRequest->cmd;
#ifdef BMEMPERF_CGI_DEBUG
                printf( "%s: client_id = %u, memc_index= %u, block_out = %u, rr = %u \n", __FUNCTION__,
                    pRequest->request.client_rts_setting.client_id, pRequest->request.client_rts_setting.memc_index, pRequest->request.client_rts_setting.block_out,
                    pRequest->request.client_rts_setting.rr );
#endif
                setRTSForClient( &( pRequest->request.client_rts_setting ));
                if (pRequest->request.client_rts_setting.block_out != BMEMPERF_CGI_INVALID)
                {
                    setClientErrCnt( pRequest->request.client_rts_setting.memc_index, pRequest->request.client_rts_setting.client_id, 0 );
                }

#ifdef BMEMPERF_CGI_DEBUG
                printf( "%s: setting BO_VAL = %d and RR_FLAG = %d for client = %u and memc = %u; err_cnt %u\n", __FUNCTION__,
                    pRequest->request.client_rts_setting.block_out,
                    pRequest->request.client_rts_setting.rr,
                    pRequest->request.client_rts_setting.client_id,
                    pRequest->request.client_rts_setting.memc_index,
                    g_savedClientDataSorted[pRequest->request.client_rts_setting.memc_index][pRequest->request.client_rts_setting.client_id].err_cnt );
#endif /* ifdef BMEMPERF_CGI_DEBUG */
                break;
            }

            case BMEMPERF_CMD_GET_CPU_IRQ_INFO:
            {
                pResponse->cmd = pRequest->cmd;
                bmemperf_getCpuUtilization( &pResponse->response.cpuIrqData.cpuData );

                PRINTF( "%s: detected BMEMPERF_CMD_GET_CPU_IRQ_INFO (%u); numCpus %u\n", __FUNCTION__, pResponse->cmd,
                    pResponse->response.cpuIrqData.cpuData.numActiveCpus );
                bmemperf_computeIrqData( pResponse->response.cpuIrqData.cpuData.numActiveCpus, &pResponse->response.cpuIrqData.irqData );

                if (( pRequest->cmdSecondary == BMEMPERF_CMD_START_PERF_DEEP ) && ( pRequest->cmdSecondaryOption > 0 ) && ( pRequest->cmdSecondaryOption < 61 ))
                {
                    bmemperf_start_perf_record( pRequest->cmdSecondaryOption );
                }
                else if (( pRequest->cmdSecondary == BMEMPERF_CMD_START_PERF_CACHE ) && ( pRequest->cmdSecondaryOption > 0 ) && ( pRequest->cmdSecondaryOption < 61 ))
                {
                    bmemperf_start_perf_cache( pRequest->cmdSecondaryOption );
                }
                else if (( pRequest->cmdSecondary == BMEMPERF_CMD_START_SATA_USB ) || ( pRequest->cmdSecondary == BMEMPERF_CMD_STOP_SATA_USB ))
                {
                    bmemperf_sata_usb_start( pRequest->cmdSecondaryOption );

                    bmemperf_sata_usb_gather( pResponse );
                }
                else if (( pRequest->cmdSecondary == BMEMPERF_CMD_START_LINUX_TOP ) || ( pRequest->cmdSecondary == BMEMPERF_CMD_STOP_LINUX_TOP ))
                {
                    bmemperf_start_linux_top( pRequest->cmdSecondaryOption );
                }

                pResponse->response.overallStats.contextSwitches = g_ContextSwitchesDelta;

                if ( passcount >= 2 )
                {
                    PRINTF( "%s: sending cmd (%u); %u bytes\n", __FUNCTION__, pResponse->cmd, sizeof( *pResponse ));
                    if (send( psd, pResponse, sizeof( *pResponse ), 0 ) < 0)
                    {
                        CloseAndExit( psd, "sending response cmd" );
                    }
                }

                break;
            }

            case BMEMPERF_CMD_QUIT:
            {
                Quit = 1;
                break;
            }
            default:
            {
                printf( "Server: unknown request %d\n", pRequest->cmd );
                break;
            }
        } /* switch */
    }
    else
    {
        PRINTF( "%u TCP/Client: %s\n", ntohs( from.sin_port ), inet_ntoa( from.sin_addr ));
        PRINTF( "%u Disconnected..\n", ntohs( from.sin_port ));
    }

    if ( psd ) close( psd );

    passcount++;

    return( 0 );
} /* Bmemperf_ReadRequest */

/**
 *  Function: This function will configure the socket so that the address can be re-used without a long timeout.
 **/
static void reusePort(
    int s
    )
{
    int one = 1;

    if (setsockopt( s, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof( one )) == -1)
    {
        printf( "error in setsockopt,SO_REUSEPORT \n" );
        exit( -1 );
    }
}

/**
 *  Function: This function is the main function that controls the logic of the app.
 **/
int main(
    int   argc,
    char *argv[]
    )
{
    int       client                = 0;
    pthread_t dataGatheringThreadId = 0;
    void     *(*threadFunc)( void * );

    BSTD_UNUSED( argc );

    memset( &g_savedClientDataSorted, 0, sizeof( g_savedClientDataSorted ));
    memset( &g_savedIrqData, 0, sizeof( g_savedIrqData ));
    memset( &gSataUsbMbps, 0, sizeof( gSataUsbMbps ));
    memset( &gRemoteClient, 0, sizeof( gRemoteClient ));

    for( client=0; client<BASPMON_MAX_NUM_CLIENTS; client++ )
    {
        sprintf( gRemoteClient[client].LogFile, "/tmp/remote_client%d.log", client );
    }

    Bmemperf_Server_InitMutex( &gSataUsbMutex );

    printf( "%s: bmemperf_response size %ld\n", __FUNCTION__, (long int) sizeof( bmemperf_response ));

    /* read the DDR and SCB frequencies from the appropriate /sys files; values were stored there by BOLT */
    bmemperf_init_scbFreq();
    bmemperf_init_ddrFreq();

    threadFunc = dataFetchThread;
    if (pthread_create( &dataGatheringThreadId, NULL, threadFunc, (void *)NULL ))
    {
        printf( "%s: could not create thread for data gathering; %s\n", __FUNCTION__, strerror( errno ));
    }
    else
    {
        startServer();
        {
            unsigned int idx = 0;
            for (idx = 0; idx<5000; idx++) {
                PRINTF( "%s: sleep(1000)\n", argv[0] );
                sleep( 1000 );
            }
        }
    }

    PRINTF( "%s exiting.\n", argv[0] );

    return( 0 );
} /* main */
