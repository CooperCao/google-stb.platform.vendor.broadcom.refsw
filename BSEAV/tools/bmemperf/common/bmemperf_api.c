/******************************************************************************
 *    (c)2013-2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELYn
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
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ******************************************************************************/
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <math.h>

#include "bmemperf.h"
#include "bmemperf_cgi.h"
#include "bmemperf_info.h"
#include "bmemperf_utils.h"

pthread_t dataReadThread = 0;

bmemperf_response bmemperfResp;

/*This specifies top 10 clients specific data to be collected */

unsigned int g_interval = 1000; /**g_interval is in msec unit*/

bmemperf_client_casdata_Index g_client_cas_data_index_for_m0[BMEMPERF_MAX_NUM_CLIENT];
bmemperf_client_casdata_Index g_client_cas_data_index_for_m1[BMEMPERF_MAX_NUM_CLIENT];
bmemperf_client_casdata_Index g_client_cas_data_index_for_m2[BMEMPERF_MAX_NUM_CLIENT];

unsigned int                g_memc_max_min_mode = 0;
extern bmemperf_info        g_bmemperf_info;
static bmemperf_cpu_percent g_cpuData;

unsigned int g_ddr_clock_frequency = 0;

extern bmemperf_data bmemperfData[BMEMPERF_NUM_MEMC];

/**This structure is will be maintained by the app,
 * response will be updated from this structure */
typedef struct bmemperf_global_stats
{
    bmemperf_overall_stats overallStats;
    bmemperf_client_stats  clientDetailStats;
    pthread_mutex_t        lock;
} bmemperf_global_stats;

typedef struct bmemperf_client_list_info
{
    unsigned int    clientListEnabled;
    unsigned int    client_list[BMEMPERF_NUM_MEMC][BMEMPERF_MAX_SUPPORTED_CLIENTS];
    pthread_mutex_t lock;
} bmemperf_client_list_info;

bmemperf_client_list_info client_list_info;

bmemperf_global_stats globalStats;

#if 0
/**
    This private function will compute I/O utilization. It uses the file: (1) /proc/diskstats.

    External users should call the Bmemperf_getIostat() function.
**/
static int P_getIostat(
    void
    )
{
    FILE                *fpProcFile = NULL;
    unsigned int         major       = 0;
    unsigned int         minor       = 0;
    char                 deviceName[16];
    char                *posEol      = NULL;
    char                *posBol      = NULL;
    char                *posDev      = NULL;
    bmemperf_iostat      gIostatDataNow;
    const char           iostatFilename[] = "/proc/diskstats";
    char                *contents = NULL;

    memset( &bufProcFile, 0, sizeof( bufProcFile ));
    memset( &gIostatDataNow, 0, sizeof( gIostatDataNow ));

    fpProcFile = fopen( iostatFilename, "r" );
    if (fpProcFile == NULL)
    {
        printf( "could not open file (%s)\n", iostatFilename );
    }
    else
    {
        getUptime( &gIostatDataNow.uptime );

        contents = GetFileContents ( iostatFilename );

#ifdef  BMEMPERF_CGI_DEBUG
#endif
        if (contents == NULL)
        {
            printf( "file (%s) had no contents\n", iostatFilename );
            return;
        }

        posBol = contents; /* start processing at the beginning of the file */

        do {
            posEol = strchr(posBol, '\n');

            if (posEol)
            {
                printf( "%s: uptime now (%8.2f); prev (%8.2f); delta (%8.2f)", __FUNCTION__, gIostatDataNow.uptime, globalStats.overallStats.iostatData.uptime,
                    gIostatDataNow.uptime-globalStats.overallStats.iostatData.uptime );

                posEol[0] = '\0'; /* null-terminate the line */

                sscanf ( posBol, "%u %u %s", &major, &minor, &deviceName );
                printf("got line for device (%s)\n", deviceName );

                /* look for line that starts with "sd" */
                posDev = strstr( posBol, " sd" );
                if (posDev)
                {
                    printf("found device (%s)\n", posDev );
                }
                printf( "\n" );
            }
        }

        for (iostat = 0; iostat<numIostatsConf; iostat++)
        {
            iostatPercent = 0.0;

            sprintf( iostatTag, "iostat%u ", iostat );
            pos = strstr( pos, iostatTag );
            if (pos)
            {
                pos += strlen( iostatTag );
                /*
                    This is the sscanf that mpstat.c uses:
                    &cp->iostat_user, &cp->iostat_nice, &cp->iostat_system, &cp->iostat_idle, &cp->iostat_iowait, &cp->iostat_irq, &cp->iostat_softirq, &cp->iostat_steal, &cp->iostat_guest
                */
                sscanf( pos, "%lu %lu %lu %lu %lu %lu %lu %lu ",
                    &gIostatDataNow.user[iostat], &gIostatDataNow.nice[iostat], &gIostatDataNow.system[iostat], &gIostatDataNow.idle[iostat],
                    &gIostatDataNow.iostat_iowait[iostat], &gIostatDataNow.iostat_irq[iostat], &gIostatDataNow.iostat_softirq[iostat], &gIostatDataNow.iostat_steal[iostat] );
                if (gIostatDataNow.uptime > globalStats.overallStats.iostatData.uptime)
                {
                    /* compute the iostat percentage based on the current numbers minus the previous numbers */
                    iostatPercent = ( gIostatDataNow.user[iostat] - globalStats.overallStats.iostatData.user[iostat] +
                                   gIostatDataNow.system[iostat] - globalStats.overallStats.iostatData.system[iostat] +
                                   gIostatDataNow.nice[iostat] - globalStats.overallStats.iostatData.nice[iostat] +
                                   gIostatDataNow.iostat_iowait[iostat] - globalStats.overallStats.iostatData.iostat_iowait[iostat] +
                                   gIostatDataNow.iostat_irq[iostat] - globalStats.overallStats.iostatData.iostat_irq[iostat] +
                                   gIostatDataNow.iostat_softirq[iostat] - globalStats.overallStats.iostatData.iostat_softirq[iostat] +
                                   gIostatDataNow.iostat_steal[iostat] - globalStats.overallStats.iostatData.iostat_steal[iostat]  ) /
                        ( gIostatDataNow.uptime-globalStats.overallStats.iostatData.uptime ) + .5 /* round */;
                    #if 0
                    printf( "%s: iostat %u: (%lu - %lu + %lu - %lu + %lu - %lu + %lu - %lu + %lu - %lu + %lu - %lu + %lu - %lu ) / (%8.2f) + .5 = %6.2f;\n",
                        __FUNCTION__, iostat,
                        gIostatDataNow.user[iostat], globalStats.overallStats.iostatData.user[iostat],
                        gIostatDataNow.system[iostat], globalStats.overallStats.iostatData.system[iostat],
                        gIostatDataNow.nice[iostat], globalStats.overallStats.iostatData.nice[iostat],
                        gIostatDataNow.iostat_iowait[iostat], globalStats.overallStats.iostatData.iostat_iowait[iostat],
                        gIostatDataNow.iostat_irq[iostat], globalStats.overallStats.iostatData.iostat_irq[iostat],
                        gIostatDataNow.iostat_softirq[iostat], globalStats.overallStats.iostatData.iostat_softirq[iostat],
                        gIostatDataNow.iostat_steal[iostat], globalStats.overallStats.iostatData.iostat_steal[iostat],
                        gIostatDataNow.uptime-globalStats.overallStats.iostatData.uptime, iostatPercent );
                     #endif /* if 0 */
                }
                if (iostatPercent > 100.0)
                {
                    iostatPercent = 100.0;
                }
                if (iostatPercent < 0.0)
                {
                    iostatPercent = 0.0;
                }

                /* save the current data to the global structure to use during the next pass */
                gIostatDataNow.idlePercentage[iostat] = globalStats.overallStats.iostatData.idlePercentage[iostat] = (unsigned char) iostatPercent;
#ifdef  BMEMPERF_CGI_DEBUG
                printf( "%3u ", gIostatDataNow.idlePercentage[iostat] );
#endif

                globalStats.overallStats.iostatData.user[iostat]        = gIostatDataNow.user[iostat];
                globalStats.overallStats.iostatData.nice[iostat]        = gIostatDataNow.nice[iostat];
                globalStats.overallStats.iostatData.system[iostat]      = gIostatDataNow.system[iostat];
                globalStats.overallStats.iostatData.iostat_iowait[iostat]  = gIostatDataNow.iostat_iowait[iostat];
                globalStats.overallStats.iostatData.iostat_irq[iostat]     = gIostatDataNow.iostat_irq[iostat];
                globalStats.overallStats.iostatData.iostat_softirq[iostat] = gIostatDataNow.iostat_softirq[iostat];
                globalStats.overallStats.iostatData.iostat_steal[iostat]   = gIostatDataNow.iostat_steal[iostat];
            }
            else
            {
#ifdef  BMEMPERF_CGI_DEBUG
                printf( "iostat %u is offline\n", iostat );
#endif
                gIostatDataNow.idlePercentage[iostat] = globalStats.overallStats.iostatData.idlePercentage[iostat] = 255;
                pos = bufProcFile; /* pos is NULL at this point; reset it to start searching back at the beginning of the stat buffer */
            }
        }
#ifdef  BMEMPERF_CGI_DEBUG
        printf( "\n" );
#endif

        globalStats.overallStats.iostatData.uptime = gIostatDataNow.uptime;
    }
    return( 0 );
} /* P_getIostat */
#endif

void setRTSForClient(
    bmemperf_cmd_set_client_rts *clientRtsSetting
    )
{
#ifdef  BMEMPERF_CGI_DEBUG
    printf( "%s: client_id = %u, memc_index= %u, block_out = %u, rr = %u \n", __FUNCTION__,
        clientRtsSetting->client_id, clientRtsSetting->memc_index, clientRtsSetting->block_out, clientRtsSetting->rr );
#endif
    bmemperf_set_arb_vals( clientRtsSetting->client_id, clientRtsSetting->memc_index, clientRtsSetting->block_out, clientRtsSetting->rr );
}

static void getTopBWConsumerClientStats(
    void
    )
{
    bmemperf_client_data *ptrClientData;
    unsigned int          i, j, k;
    unsigned int          client_index;
    int                   array_range;
    int                   memc_index  = 0;
    int                   rc          = 0;
    volatile unsigned int arb_reg_val = 0;

    for (k = 0; k<BMEMPERF_NUM_MEMC; k++)
    {
        volatile unsigned int dataCycle;

        memc_index = k;

        for (j = 0; j < BMEMPERF_MAX_NUM_CLIENT; j++)
        {
            client_index = j;

            /** 0 - 255 for fisrt memc and next 255 for memc 1 and so on */
            dataCycle = bmemperf_get_ddr_stat_cas_client_data( client_index, memc_index );
            if (memc_index == 0)
            {
                g_client_cas_data_index_for_m0[j].data  = dataCycle;
                g_client_cas_data_index_for_m0[j].index = client_index;
            }
            else if (memc_index == 1)
            {
                g_client_cas_data_index_for_m1[j].data  = dataCycle;
                g_client_cas_data_index_for_m1[j].index = client_index;
            }
            else if (memc_index == 2)
            {
                g_client_cas_data_index_for_m2[j].data  = dataCycle;
                g_client_cas_data_index_for_m2[j].index = client_index;
            }
        }
    }
    array_range = BMEMPERF_MAX_NUM_CLIENT;
    /** sort the data  **/
    qsort( g_client_cas_data_index_for_m0, array_range, sizeof( g_client_cas_data_index_for_m0[0] ), comapare_cas_data );
    qsort( g_client_cas_data_index_for_m1, array_range, sizeof( g_client_cas_data_index_for_m1[0] ), comapare_cas_data );
    qsort( g_client_cas_data_index_for_m2, array_range, sizeof( g_client_cas_data_index_for_m2[0] ), comapare_cas_data );

    for (k = 0; k<BMEMPERF_NUM_MEMC; k++)
    {
        unsigned int value;
        float        bw, temp;
        memc_index =  k;

        for (i = 0; i < BMEMPERF_MAX_NUM_CLIENT; i++)
        {
            /* g_client_cas_data_index_for_m1 */
            if (memc_index == 0)
            {
                client_index = g_client_cas_data_index_for_m0[i].index;
                value        = g_client_cas_data_index_for_m0[i].data;
            }
            else if (memc_index == 1)
            {
                client_index = g_client_cas_data_index_for_m1[i].index;
                value        = g_client_cas_data_index_for_m1[i].data;
            }
            else if (memc_index == 2)
            {
                client_index = g_client_cas_data_index_for_m2[i].index;
                value        = g_client_cas_data_index_for_m2[i].data;
            }

            temp = ( g_interval*BMEMPERF_DDR0_CLOCK_FREQ_UNIT );

            /**  Important:The consumption count is in units of DRAM clock
             *   cycles. Note that all supported DRAM devices are BL=8 DDR
             *   devices, so a CAS is always equal to (4) DRAM clock
             *   cycles**/

            value = ( value *( BLOCK_LENGTH>>1 ));

            bw = bmemperf_computeBWinMbps( value, temp, memc_index );

            value = bw;

            /*Acquire mutex before writing*/
            rc = pthread_mutex_lock( &globalStats.lock );
            if (rc)
            {
                printf( "%s: mutex lock err - %d\n", __FUNCTION__, rc );
            }

            /* Get rr and block_Outval */
            arb_reg_val = bmemperf_collect_arb_details( client_index, memc_index ); /* In this case client_index is same as client_id */

            ptrClientData = &( globalStats.overallStats.clientOverallStats[memc_index].clientData[i] );

            ptrClientData->bw        = bw;
            ptrClientData->client_id = client_index;
            ptrClientData->rr        = bmemperfData[memc_index].rrEn;
            ptrClientData->block_out = bmemperfData[memc_index].boVal;

            #if 0
            snprintf( ptrClientData->clientName, MAX_CLIENT_NAME_SIZE, "%s",  getClientName( client_index ));
            #endif

#ifdef  BMEMPERF_CGI_DEBUG
            /* to shorten debug, only show memc 0*/
            if (memc_index==0)
            {
                printf( "%s: idx %u: %4u  %-6d %-20s %-6d\n", __FUNCTION__, memc_index,  i, ptrClientData->client_id, ptrClientData->clientName, ptrClientData->bw );
            }
#endif /* ifdef  BMEMPERF_CGI_DEBUG */

            pthread_mutex_unlock( &globalStats.lock );
        }
    }
} /* getTopBWConsumerClientStats */

void getOverAllSystemBW(
    void
    )
{
    int memc_index = 0;
    int i;
    int rc;

    bmemperf_system_stats *ptrSystemStats;

    for (i = 0; i< BMEMPERF_NUM_MEMC; i++)
    {
        memc_index = i;
        bmemperf_normalmode_system_data( memc_index );

        /*acquire mutex before this*/
        rc = pthread_mutex_lock( &globalStats.lock );
        if (rc)
        {
            printf( "%s: mutex lock err - %d\n", __FUNCTION__, rc );
        }
        ptrSystemStats = &( globalStats.overallStats.systemStats[memc_index] );

        ptrSystemStats->dataBW        = bmemperfData[memc_index].dataBW;
        ptrSystemStats->dataUtil      = bmemperfData[memc_index].dataUtil;
        ptrSystemStats->idleBW        = bmemperfData[memc_index].idleBW;
        ptrSystemStats->transactionBW = bmemperfData[memc_index].transactionBW;
        ptrSystemStats->ddrFreqMhz    = g_bmemperf_info.ddrFreqInMhz;
        ptrSystemStats->scbFreqMhz    = g_bmemperf_info.scbFreqInMhz;

#ifdef  BMEMPERF_CGI_DEBUG
        printf( "\n %s ", __FUNCTION__ );
        printf( "%3u  %6u %6u %6u %6.2f\n",  memc_index, ptrSystemStats->dataBW,
            ptrSystemStats->transactionBW,
            ptrSystemStats->idleBW,
            ptrSystemStats->dataUtil );
#endif /* ifdef  BMEMPERF_CGI_DEBUG */
        pthread_mutex_unlock( &globalStats.lock );
    }
} /* getOverAllSystemBW */

void getClientSpecificData(
    unsigned int *ptrClientLoop
    )
{
    unsigned int memc_index;
    unsigned int i;
    unsigned int clientId;
    int          rc;

    bmemperf_per_client_stats *ptrPerClientStats;

    for (i = 0; i< BMEMPERF_NUM_MEMC; i++)
    {
        memc_index = i;

        rc = pthread_mutex_lock( &client_list_info.lock );
        if (rc)
        {
            printf( "%s: mutex lock err - %d\n", __FUNCTION__, rc );
        }
        clientId = client_list_info.client_list[memc_index][ptrClientLoop[memc_index]];

        rc = pthread_mutex_unlock( &client_list_info.lock );
        if (rc)
        {
            printf( "%s: mutex unlock err - %d\n", __FUNCTION__, rc );
        }

        if (clientId >= BMEMPERF_MAX_NUM_CLIENT)
        {
            printf( "%s: clientId %u is invalid\n", __FUNCTION__, clientId );
            return;
        }

        /**** Normal mode system count information Done ****/
        bmemperf_normalmode_client_service_data( memc_index );

        /* Acquire mutex before this */
        rc = pthread_mutex_lock( &globalStats.lock );
        if (rc)
        {
            printf( "%s: mutex unlock err - %d\n", __FUNCTION__, rc );
        }
        ptrPerClientStats = &( globalStats.clientDetailStats.clientStats[memc_index].perClientStats[ptrClientLoop[memc_index]] );

        /*printf("%s: copying clientID %u for memc %u\n", __FUNCTION__, clientId, i );*/
        ptrPerClientStats->clientId                = clientId;
        ptrPerClientStats->avgClientDataSizeInBits = bmemperfData[memc_index].avgClientDataSizeInBits;
        ptrPerClientStats->clientDataBW            = bmemperfData[memc_index].clientDataBW;
        ptrPerClientStats->clientDataUtil          = bmemperfData[memc_index].clientDataUtil;
        ptrPerClientStats->clientRdTransInPerc     = bmemperfData[memc_index].clientRdTransInPerc;
        ptrPerClientStats->clientWrTransInPerc     = bmemperfData[memc_index].clientWrTransInPerc;
        ptrPerClientStats->clientTransBW           = bmemperfData[memc_index].clientTransBW;
        #if 0
        strncpy( ptrPerClientStats->clientName, getClientName( clientId ), sizeof( ptrPerClientStats->clientName ));
        #endif

#ifdef BMEMPERF_CGI_DEBUG
        printf( "%s:---------> memc_id = %u ;ptrClientLoop[memc_index] = %u; client_id = %u; clientDataBw = %u \n", __FUNCTION__, memc_index, ptrClientLoop[memc_index], ptrPerClientStats->clientId, ptrPerClientStats->clientDataBW );
#endif

#if 0
        printf( "\n %s ", __FUNCTION__ );
        printf( "Client_id = %d %3u  %6u %6u %6u %6u %6.2f\n", clientId, memc_index, ptrPerClientStats->clientDataBW,
            ptrPerClientStats->avgClientDataSizeInBits,
            ptrPerClientStats->clientRdTransInPerc,
            ptrPerClientStats->clientWrTransInPerc,
            ptrPerClientStats->clientDataUtil );
#endif /* if 0 */
        pthread_mutex_unlock( &globalStats.lock );
    }
} /* getClientSpecificData */

#if 0

void getTopClientList(
    unsigned int num_client
    )
{
    unsigned int            memc_index;
    unsigned int            i, j;
    bmemperf_overall_stats *ptrOverallStats;

    ptrOverallStats = &( globalStats.overallStats );

    for (i = 0; i< BMEMPERF_NUM_MEMC; i++)
    {
        memc_index = i;

        /*Get the top x clients for the memc*/
        for (j = 0; j < num_client; j++)
        {
            client_list[memc_index][j] = ptrOverallStats->clientOverallStats[memc_index].clientData[j].client_id;
        }
    }
} /* getTopClientList */

#endif /* if 0 */

void setClientList(
    unsigned int client_list[BMEMPERF_NUM_MEMC][BMEMPERF_MAX_SUPPORTED_CLIENTS]
    )
{
    unsigned int memc, client, tracking_count = 0;

    int rc = 0;

#ifdef BMEMPERF_CGI_DEBUG
    printf( "%s: Client id %u, %u, %u\n", __FUNCTION__, client_list[0][0], client_list[1][0], client_list[2][0] );
#endif

    rc = pthread_mutex_lock( &client_list_info.lock );
    if (rc)
    {
        printf( "%s: mutex lock err client_list_info - %d\n", __FUNCTION__, rc );
    }

    /* Acquire mutex before this */
    rc = pthread_mutex_lock( &globalStats.lock );
    if (rc)
    {
        printf( "%s: mutex lock err globalStats - %d\n", __FUNCTION__, rc );
    }

    client_list_info.clientListEnabled = true;

    for (memc = 0; memc < BMEMPERF_NUM_MEMC; memc++)
    {
        tracking_count = 0;

        for (client = 0; client < BMEMPERF_MAX_SUPPORTED_CLIENTS; client++)
        {
            client_list_info.client_list[memc][client] = client_list[memc][client];

            /* if client_id is being tracked */
            if (client_list_info.client_list[memc][client] != BMEMPERF_INVALID)
            {
                unsigned int client2 = 0;
                tracking_count++; /* used to zero out the end of the globalStats array to erase client_ids that are dropped from tracking */
                PRINTF( "%s: memc %u: cnt %u; tracking client_id[%u] = %u; globalStats: ", __FUNCTION__, memc, tracking_count, client, client_list_info.client_list[memc][client] );
                for (client2 = 0; client2<BMEMPERF_MAX_SUPPORTED_CLIENTS; client2++)
                {
                    PRINTF( "%u, ", globalStats.clientDetailStats.clientStats[memc].perClientStats[client2].clientId );
                }
                PRINTF( "\n" );
            }
        }

        /* Because there is no way to know that the list of tracked clients has decreased (from 5 to 4 for example), after we determine that we are
           tracking just 4 clients, go back through the globalStats array and zero out the end of the list ... from client index 5 through the end */
        PRINTF( "%s: memc %u: zeroing out global: ", __FUNCTION__, memc );
        for (client = tracking_count; client < BMEMPERF_MAX_SUPPORTED_CLIENTS; client++)
        {
            memset( &globalStats.clientDetailStats.clientStats[memc].perClientStats[client], 0, sizeof( globalStats.clientDetailStats.clientStats[memc].perClientStats[client] ));
            PRINTF( "%u, ", client );
        }
        PRINTF( "\n" );

        /* if the first entry in the list is invalid (no clients are being detailed), set first entry to default */
        if (client_list_info.client_list[memc][0] == BMEMPERF_INVALID)
        {
            client_list_info.client_list[memc][0] = 10;
        }
    }

    rc = pthread_mutex_unlock( &globalStats.lock );
    if (rc)
    {
        printf( "%s: mutex unlock err globalStats - %d\n", __FUNCTION__, rc );
    }

    rc = pthread_mutex_unlock( &client_list_info.lock );
    if (rc)
    {
        printf( "%s: mutex unlock unck err client_list_info - %d\n", __FUNCTION__, rc );
    }
} /* setClientList */

static void initClientList(
    void
    )
{
    unsigned int i;

    /** initialize client list with client_id any say 10 for 0th index and INVALID for rest of the index ***/
    client_list_info.clientListEnabled = true;
    for (i = 0; i <BMEMPERF_NUM_MEMC; i++)
    {
        memset( client_list_info.client_list[i], BMEMPERF_INVALID, ( sizeof( unsigned int )*BMEMPERF_MAX_SUPPORTED_CLIENTS ));
        /** any client id is fine this just for the sake of configuration **/
        client_list_info.client_list[i][0] = 10;
    }
}

/**
 *  Function: This function will reset the ARB error registers. It will also clear out the running err_cnt
 *  counters for each client in each memc.
 **/
int reset_error_counts(
    void
    )
{
    unsigned int memc = 0;

    printf( "%s: resetting ARB errors\n", __FUNCTION__ );

    /* reset any residual errors back to zero */
    for (memc = 0; memc<BMEMPERF_NUM_MEMC; memc++)
    {
        bmemperf_clear_arb_err( memc );
    }

    return( 0 );
} /* reset_error_counts */

void *dataFetchThread(
    void *data
    )
{
    unsigned int         tempClientId = 10;
    int                  memc_index   = 0;
    unsigned int         i;
    int                  rc = 0;
    unsigned int         clientLoop[BMEMPERF_NUM_MEMC];
    bmemperf_cpu_percent cpuData;

    BSTD_UNUSED( data );

    /**initializing client loop counter and clientloop reset flag **/
    memset( clientLoop, 0, ( sizeof( unsigned int )*BMEMPERF_NUM_MEMC ));
    memset( &cpuData, 0, sizeof( cpuData ));
    memset( &g_cpuData, 0, sizeof( g_cpuData ));

    /** create mutex ***/
    pthread_mutex_init( &client_list_info.lock, NULL );
    pthread_mutex_init( &globalStats.lock, NULL );

    /** initialize client list with client_id any say 10 for 0th index and INVALID for rest of the index ***/
    initClientList();

    bmemperf_init();

    reset_error_counts();

    /**** First time execute to get clientList ready.*/
    for (i = 0; i< BMEMPERF_NUM_MEMC; i++)
    {
        memc_index = i;
        /* for top and overall data configure any client number it doesn't matter*/
        tempClientId = 10;
        bmemperf_configure_stat_control_reg( tempClientId, memc_index );
    }

    usleep(( g_interval*1000 ));

    getTopBWConsumerClientStats();

#if 0
    getTopClientList( BMEMPERF_MAX_SUPPORTED_CLIENTS );
    updateClientList = 0;
#endif

    /**** Got the first client list **********/
    while (1)
    {
        setUptime();

        set_cpu_utilization();

        /** for(clientLoop = 0; clientLoop < BMEMPERF_MAX_SUPPORTED_CLIENTS; clientLoop++) **/
        {
            /****************************************/
            rc = pthread_mutex_lock( &client_list_info.lock );
            if (rc)
            {
                printf( "%s: mutex lock err - %d\n", __FUNCTION__, rc );
            }
            for (i = 0; i< BMEMPERF_NUM_MEMC; i++)
            {
                memc_index = i;

                tempClientId = client_list_info.client_list[memc_index][clientLoop[memc_index]];

                if (tempClientId < BMEMPERF_MAX_NUM_CLIENT)
                {
                    /*printf("%s: valid ... memc %u; clientId %u\n", __FUNCTION__, i, tempClientId );*/
                    bmemperf_configure_stat_control_reg( tempClientId, memc_index );
                }
                else
                {
                    /** relooping for that very memc since it has no more client mentioned **/
                    clientLoop[memc_index] = 0;
                    tempClientId           = client_list_info.client_list[memc_index][clientLoop[memc_index]];
                    bmemperf_configure_stat_control_reg( tempClientId, memc_index );
                    /*printf("%s: invalid ... memc %u; clientId %u\n", __FUNCTION__, i, tempClientId );*/
                }
#ifdef BMEMPERF_CGI_DEBUG

                printf( "%s: client id = %u ; clientLoop = %u , programed for memc_Id= %u  ; tempClientId = %u\n",
                    __FUNCTION__,
                    client_list_info.client_list[memc_index][clientLoop[memc_index]],
                    clientLoop[memc_index],
                    memc_index, tempClientId );
#endif /* ifdef BMEMPERF_CGI_DEBUG */
            }

            rc = pthread_mutex_unlock( &client_list_info.lock );
            if (rc)
            {
                printf( "%s: mutex unlock err - %d\n", __FUNCTION__, rc );
            }

            usleep(( g_interval*1000 ));

            /** this functions works for all memc **/
            getTopBWConsumerClientStats();

#if 0
            /*currently we update the list once the earlier list is done */
            if (!clientLoop && updateClientList)
            {
                getTopClientList( BMEMPERF_MAX_SUPPORTED_CLIENTS );
                updateClientList = 1;
            }
#endif /* if 0 */

            /** this functions works for all memc **/
            getOverAllSystemBW();

            getClientSpecificData( clientLoop );

            for (i = 0; i< BMEMPERF_NUM_MEMC; i++)
            {
                memc_index = i;
                clientLoop[memc_index]++;
            }

            P_getCpuUtilization();

            fflush( stdout );
            fflush( stderr );
        } /*end of clientLoop */
    }
} /* dataFetchThread */

int bmemperf_getClientStats(
    bmemperf_client_stats *clientDetailStats
    )
{
    int rc;

    /* bmemperfResp.cmd = BMEMPERF_CMD_GET_CLIENT_STATS;*/
    /* Acquire mutex before this */
    rc = pthread_mutex_lock( &globalStats.lock );
    if (rc)
    {
        printf( "%s: mutex lock err - %d\n", __FUNCTION__, rc );
        goto error;
    }

#ifdef BMEMPERF_CGI_DEBUG

    unsigned int idx = 0, jdx = 0;
    for (jdx = 0; jdx<3; jdx++)
    {
        for (idx = 0; idx<3; idx++)
        {
            if (globalStats.clientDetailStats.clientStats[jdx].perClientStats[idx].clientId == 201)
            {
                printf( "MEMC %u %u; %u; %u; %u; %u; %5.2f\n", jdx,
                    globalStats.clientDetailStats.clientStats[jdx].perClientStats[idx].clientId,
                    globalStats.clientDetailStats.clientStats[jdx].perClientStats[idx].clientDataBW,
                    globalStats.clientDetailStats.clientStats[jdx].perClientStats[idx].clientRdTransInPerc,
                    globalStats.clientDetailStats.clientStats[jdx].perClientStats[idx].clientWrTransInPerc,
                    globalStats.clientDetailStats.clientStats[jdx].perClientStats[idx].avgClientDataSizeInBits/8,
                    globalStats.clientDetailStats.clientStats[jdx].perClientStats[idx].clientDataUtil );
            }
        }
    }

    printf( "%s: globalStats.clientDetailStats.clientStats[0].perClientStats[0].clientId = %d\n", __FUNCTION__,
        globalStats.clientDetailStats.clientStats[0].perClientStats[0].clientId );
    printf( "%s: globalStats.clientDetailStats.clientStats[0].perClientStats[0].clientDataBW = %d\n", __FUNCTION__,
        globalStats.clientDetailStats.clientStats[0].perClientStats[0].clientDataBW );
#endif /* ifdef BMEMPERF_CGI_DEBUG */

    memcpy( clientDetailStats, &( globalStats.clientDetailStats ), sizeof( bmemperf_client_stats ));
    pthread_mutex_unlock( &globalStats.lock );
    return( 0 );

error:
    return( -1 );
} /* bmemperf_getClientStats */

int bmemperf_getOverallStats(
    bmemperf_overall_stats *overallStats
    )
{
    int rc;

    /* bmemperfResp.cmd = BMEMPERF_CMD_GET_OVERALL_STATS;*/
    /* Acquire mutex before this */

    rc = pthread_mutex_lock( &globalStats.lock );
    if (rc)
    {
        printf( "%s: mutex lock err - %d\n", __FUNCTION__, rc );
        goto error;
    }

    memcpy( overallStats, &( globalStats.overallStats ), sizeof( bmemperf_overall_stats ));

    pthread_mutex_unlock( &globalStats.lock );
    return( 0 );

error:
    return( -1 );
} /* bmemperf_getOverallStats */
