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

#include "bmon_types64.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/utsname.h>
#include <stdbool.h>
#include <sys/klog.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "bmon_utils.h"
#include "wifi_bwl_interface.h"

#define WIFI_INIT_RETRY 5

#ifdef DEBUG
#undef DEBUG
#endif

#ifdef BWL_SUPPORT
static BWL_Handle hBwl = NULL;

static int bmon_wifi_init(
    const char *ifname,
    const char *caller
    )
{
    int i   = 0;
    int err = BWL_ERR_SUCCESS;

    /*fprintf( stderr, "%s: %s ... hBwl (%p) ... (%s)\n", __FUNCTION__, caller, hBwl, ifname );*/
    /* Try to init the BWL interface */
    for (i = 0; i < WIFI_INIT_RETRY && hBwl == NULL; i++)
    {
        /*fprintf( stderr, "%s: %s BWL_Init(%s)\n", __FUNCTION__, caller, ifname );*/
        err = BWL_Init( &hBwl, (char *) ifname );
        if ( err == BWL_ERR_SUCCESS )
        {
            /*fprintf( stderr, "%s: returning hBwl (%p)\n", __FUNCTION__, hBwl );*/
            break;
        }
        else if ( err == BWL_ERR_IOCTL ) /* happens if wl.ko is not installed */
        {
            fprintf( stderr, "%s: BWL_ERR_IOCTL \n", __FUNCTION__ );
            break;
        }
        PRINTF( "%s: sleeping tenth of a second\n", __FUNCTION__ );
        usleep( 1000000/10 );                              /* sleep a tenth of a second */
    }

    return( err );
}

static int bmon_wifi_uninit(
    const char *caller
    )
{
    /*fprintf( stderr, "%s: %s BWL_Uninit(%p)\n", __FUNCTION__, caller, hBwl );*/

#if 1
    if( hBwl ) BWL_Uninit( hBwl );
    hBwl = NULL;
#endif

    return( 0 );
}

int bmon_wifi_clear_ampdu(
    const char *ifname
    )
{
    int   tRetCode = 0;

    noprintf( "%s:%u: hBwl #%p; ifname (%s) \n", __FILE__, __LINE__, hBwl, ifname );
    do
    {
        bmon_wifi_init( ifname, __FUNCTION__ );

        if (hBwl == NULL)
        {
            PRINTF( "%s() Failed to Init BWL! \n", __FUNCTION__ );
            tRetCode = -1;
            break;
        }
        else
        {
            BWL_API_CHECK( BWL_ClearAmpdu( hBwl ), tRetCode );
        }
        bmon_wifi_uninit(__FUNCTION__);
    } while (0);

err_out:
    return( tRetCode );
}                                                          /* bmon_wifi_clear_ampdu */


int bmon_wifi_chanim_get_count(
    const char *ifname,
    unsigned int *num
    )
{
    int   tRetCode = 0;
    Chanim_list_t *pData    = NULL;

    struct timeval tv;

    gettimeofday( &tv, NULL );

    noprintf( "~DEBUG~%s:%u: hBwl #%p ~", __FILE__, __LINE__, hBwl );
    do
    {
        bmon_wifi_init( ifname, __FUNCTION__ );

        if (hBwl == NULL)
        {
            PRINTF( "%s() Failed to Init BWL! \n", __FUNCTION__ );
            tRetCode = -1;
            break;
        }
        else
        {
            BWL_GetChanimNum( hBwl, num );
            PRINTF("returned %d elements in chanim", num);
        }
        bmon_wifi_uninit(__FUNCTION__);
        PRINTF( "%s - done       ... %s ... delta %lu msec\n", __FUNCTION__, bmon_get_time_now_str(),
                bmon_delta_time_microseconds( tv.tv_sec, tv.tv_usec)/1000 );
    } while (0);

    return( tRetCode );
}

/**
 *  Function: This function will call a BWL function to dump the chanim data similiar to the 'wl chanim_stats'
 *  command. The AMPDU data gets stored in the 'chanim' buffer and then processed.
 **/
int bmon_wifi_chanim_get_report(
    const char *ifname,
    chanim_data_t *data
    )
{
    uint32_t num;
    int   tRetCode = 0;
    Chanim_list_t *pData    = NULL;

    struct timeval tv;

    gettimeofday( &tv, NULL );

    noprintf( "~DEBUG~%s:%u: hBwl #%p ~", __FILE__, __LINE__, hBwl );
    do
    {
        bmon_wifi_init( ifname, __FUNCTION__ );

        if (hBwl == NULL)
        {
            PRINTF( "%s() Failed to Init BWL! \n", __FUNCTION__ );
            tRetCode = -1;
            break;
        }
        else
        {
            pData = (Chanim_list_t*)malloc( sizeof( Chanim_list_t ) );

            if( pData )
            {
                BWL_GetChanimResults( hBwl, &pData );

                int ii=0;
                for( ii = 0; ii < pData->length; ii++ )
                {
                    data[ii].glitchcnt = pData->congest[ii].glitchcnt;
                    data[ii].badplcp = pData->congest[ii].badplcp;
                    // ccastats[BMON_CCASTATS_V2_MAX];
                    data[ii].bgnoise = pData->congest[ii].bgnoise;
                    data[ii].chanspec = pData->congest[ii].chanspec;
                    data[ii].channum = pData->congest[ii].channum;
                    data[ii].timestamp = pData->congest[ii].timestamp;
                    data[ii].bphy_glitchcnt = pData->congest[ii].bphy_glitchcnt;
                    data[ii].bphy_badplcp = pData->congest[ii].bphy_badplcp;
                    data[ii].chan_idle = pData->congest[ii].chan_idle;

                    data[ii].tx = pData->congest[ii].tx;
                    data[ii].ibss = pData->congest[ii].inbss;
                    data[ii].obss = pData->congest[ii].obss;
                    data[ii].chan_idle = pData->congest[ii].chan_idle;
                    data[ii].busy = pData->congest[ii].busy;
                    data[ii].avail = pData->congest[ii].availcap;
                    data[ii].nwifi = pData->congest[ii].nonwifi;
                }

                free (pData);
            }
        }
        bmon_wifi_uninit(__FUNCTION__);
        PRINTF( "%s - done       ... %s ... delta %lu msec\n", __FUNCTION__, bmon_get_time_now_str(),
                bmon_delta_time_microseconds( tv.tv_sec, tv.tv_usec)/1000 );
    } while (0);

    return( tRetCode );
} /* bmon_wifi_ampdu_get_report */

/**
 *  Function: This function will call a BWL function to dump the AMPDU data similiar to the 'wl dump ampdu'
 *  command. The AMPDU data gets stored in the 'ampdu' buffer and then processed.
 **/
int bmon_wifi_ampdu_get_report(
    const char *ifname
    )
{
    int   tRetCode = 0;
    char *ampdu    = NULL;
    struct timeval tv;

    gettimeofday( &tv, NULL );

    noprintf( "~DEBUG~%s:%u: hBwl #%p ~", __FILE__, __LINE__, hBwl );
    do
    {
        bmon_wifi_init( ifname, __FUNCTION__ );

        if (hBwl == NULL)
        {
            PRINTF( "%s() Failed to Init BWL! \n", __FUNCTION__ );
            tRetCode = -1;
            break;
        }
        else
        {
            if (( ampdu = (char *) BWL_GetAmpdu( hBwl )) == NULL)
            {
                PRINTF( "~FATAL~BWL_GetAmpdu() failed with error %d~", tRetCode );
            }
            else
            {
                char *pos = ampdu;
                char  values[256];
                char  percentages[256];
                char  tempFilename[TEMP_FILE_FULL_PATH_LEN];

                bmon_prepend_temp_directory( tempFilename, sizeof( tempFilename ), FILENAME_WIFI_RECORD );

                remove( tempFilename );

                memset( values, 0, sizeof( values ));
                sprintf( values, "%lu\t%s\n", bmon_get_seconds_since_epoch(), bmon_date_yyyy_mm_dd_hh_mm_ss());
                bmon_wifi_ampdu_write_to_file( values, tempFilename );

                /*
                TX VHT  :  33(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
                        :  3(0%)  0(0%)  0(0%)  0(0%)  2746(8%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
                        :  1408(4%)  1631(5%)  21440(70%)  3284(10%)  11(0%)  10(0%)  9(0%)  11(0%)  6(0%)  0(0%)  0(0%)  0(0%)
                TX VHT SGI:  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
                          :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
                          :  8(0%)  0(0%)  14211(99%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
                RX VHT  :  3(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
                        :  0(0%)  0(0%)  0(0%)  33(0%)  36(0%)  1672(5%)  1930(5%)  127(0%)  15656(47%)  0(0%)  0(0%)  0(0%)
                        :  0(0%)  0(0%)  0(0%)  162(0%)  187(0%)  950(2%)  2132(6%)  2294(6%)  7351(22%)  427(1%)  0(0%)  0(0%)
                RX VHT SGI:  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
                          :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  15073(99%)  0(0%)  0(0%)  0(0%)
                          :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  21(0%)  0(0%)  0(0%)
                */
                noprintf("\n\n%s:%u: ampdu buffer len (%d) \n", __FUNCTION__, __LINE__, strlen(ampdu) );
                noprintf( "%s", ampdu );

                bmon_wifi_ampdu_parse_for_totals( pos, tempFilename, 1 /* TX==1 */ );
                noprintf("%s:%u: after ParseForTotals, buffer is (%s) (%p) \n", __FUNCTION__, __LINE__, pos, (void*) pos );

                bmon_wifi_ampdu_parse_for_totals( pos, tempFilename, 0 /* RX==0 */ );
                noprintf("%s:%u: after ParseForTotals, buffer is (%s) (%p) \n", __FUNCTION__, __LINE__, pos, (void*) pos );

                memset( values, 0, sizeof( values ));
                memset( percentages, 0, sizeof( percentages ));
                pos = bmon_wifi_ampdu_parse_vmt( pos, "TX VHT  :", values, percentages, tempFilename );

                memset( values, 0, sizeof( values ));
                memset( percentages, 0, sizeof( percentages ));
                pos = bmon_wifi_ampdu_parse_vmt( pos, "TX VHT SGI:", values, percentages, tempFilename );

                memset( values, 0, sizeof( values ));
                memset( percentages, 0, sizeof( percentages ));
                pos = bmon_wifi_ampdu_parse_vmt( pos, "RX VHT  :", values, percentages, tempFilename );

                memset( values, 0, sizeof( values ));
                memset( percentages, 0, sizeof( percentages ));
                pos = bmon_wifi_ampdu_parse_vmt( pos, "RX VHT SGI:", values, percentages, tempFilename );

                FREE_SAFE ( ampdu );
            }
        }
        bmon_wifi_uninit(__FUNCTION__);
        PRINTF( "%s - done       ... %s ... delta %lu msec\n", __FUNCTION__, bmon_get_time_now_str(),
                bmon_delta_time_microseconds( tv.tv_sec, tv.tv_usec)/1000 );
    } while (0);

    return( tRetCode );
}                                                          /* bmon_wifi_ampdu_get_report */

int bmon_wifi_read_ampdu_data(
    ampdu_data_t *pAmpduData
    )
{
    char  tempFilename[TEMP_FILE_FULL_PATH_LEN];
    char *contents = NULL;

    bmon_prepend_temp_directory( tempFilename, sizeof( tempFilename ), FILENAME_WIFI_RECORD /* CAD "wifi_record2.txt" */ );

    contents = bmon_get_file_contents( tempFilename );

    /*printflog( "~DEBUG~%s:%u: filename (%s); contents (%p) ~\n", __FUNCTION__, __LINE__, tempFilename, contents );*/

    /*
1465563108      20160610-125148
TX VHT   1x1    0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0                   TX VHT   1x1 %  0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0
TX VHT SGI 1x1  0       0       0               TX VHT SGI 1x1 %        0       0       0
RX VHT   1x1    0               RX VHT   1x1 %  0
RX VHT SGI 1x1  0               RX VHT SGI 1x1 %        0
    */
    /* if the contents of the file were successfully read */
    if (contents)
    {
        bool        bGoodLine = false;
        char       *pos       = contents;
        char       *eol       = NULL;
        char       *sgi       = NULL;
        char       *tab2      = NULL;
        char       *tot_mpdus = NULL;
        char       *tot_mpdus_sub = NULL;
        int         lines     = 0;
        antenna1_t *ant       = NULL;
        do
        {
            char             *token = NULL;
            int               count = 0;
            unsigned long int level = 0;

            bGoodLine = false;

            eol = strchr( pos, '\n' );
            if (eol)
            {
                *eol = '\0';                               /* null-terminate the line */
            }

            /* TX: look for a line like this ... tot_mpdus 10000 tot_ampdus 20000 mpduperampdu 30000 */
            tot_mpdus = strstr(pos, "tx_mpdus");
            if ( tot_mpdus )
            {
                /*printflog("~DEBUG~%s:%u: found line (%s) \n", __FUNCTION__, __LINE__, tot_mpdus );*/
                sscanf( tot_mpdus, "tx_mpdus %lu tx_ampdus %lu tx_mpduperampdu %lu", &pAmpduData->tx_mpdus, &pAmpduData->tx_ampdus, &pAmpduData->tx_mpduperampdu );

                /*printflog("tx_mpdus %u tx_ampdus %u mpduperampdu %u \n", pAmpduData->tx_mpdus, pAmpduData->tx_ampdus, pAmpduData->tx_mpduperampdu );*/
            }

            tot_mpdus = strstr(pos, "retry_ampdu");
            if ( tot_mpdus )
            {
                sscanf( tot_mpdus, "retry_ampdu %lu retry_mpdu %lu", &pAmpduData->retry_ampdu, &pAmpduData->retry_mpdu);
            }
            tot_mpdus = strstr(pos, "txaddbareq");
            if ( tot_mpdus )
            {
                tot_mpdus_sub=strstr(tot_mpdus,"txbar");
                sscanf( tot_mpdus_sub, "txbar %lu", &pAmpduData->txbar);
                tot_mpdus_sub=strstr(tot_mpdus,"rxba");
                sscanf( tot_mpdus_sub, "rxba %lu", &pAmpduData->rxba);
            }

            /* RX: look for a line like this ... rx_mpdus 10000 rx_ampdus 20000 rx_mpduperampdu 30000 */
            tot_mpdus = strstr(pos, "rx_mpdus");
            if ( tot_mpdus )
            {
                /*printflog("~DEBUG~%s:%u: found line (%s) \n", __FUNCTION__, __LINE__, tot_mpdus );*/
                sscanf( tot_mpdus, "rx_mpdus %lu rx_ampdus %lu rx_mpduperampdu %lu", &pAmpduData->rx_mpdus, &pAmpduData->rx_ampdus, &pAmpduData->rx_mpduperampdu );

                /*printflog("rx_mpdus %u rx_ampdus %u mpduperampdu %u \n", pAmpduData->rx_mpdus, pAmpduData->rx_ampdus, pAmpduData->rx_mpduperampdu );*/
            }

            tot_mpdus = strstr(pos, "rxaddbareq");
            if ( tot_mpdus )
            {
                tot_mpdus_sub=strstr(tot_mpdus,"rxbar");
                sscanf( tot_mpdus_sub, "rxbar %lu", &pAmpduData->rxbar);
                tot_mpdus_sub=strstr(tot_mpdus,"txba");
                sscanf( tot_mpdus_sub, "txba %lu", &pAmpduData->txba);
            }

            tab2 = strstr( pos, "\t\t" );
            if (tab2)
            {
                *tab2 = '\0';                              /* ignore the part of the line after the triple tab delimiter */
            }

            noprintf( "~DEBUG~found line (%s); ~\n", pos );

            sgi = strstr( pos, "SGI" );

            if (( pos[0] == 'T' ) && ( pos[1] == 'X' ))
            {
                ant       = &pAmpduData->antennas.tx[0];
                bGoodLine = true;
                noprintf( "~DEBUG~TX detected ant is (%p)~\n", (void *) ant );
            }
            else if (( pos[0] == 'R' ) && ( pos[1] == 'X' ))
            {
                ant       = &pAmpduData->antennas.rx[0];
                bGoodLine = true;
                noprintf( "~DEBUG~RX detected ant is (%p)~\n", (void *) ant );
            }
            else
            {
                /*printf( "~DEBUG~unknown line (%s)~\n", pos );*/
            }

            if (bGoodLine)
            {
                if (sgi)
                {
                    /* skip over the first four non-SGI antennas */
                    ant++;
                    ant++;
                    ant++;
                    ant++;
                    noprintf( "~DEBUG~SGI detected ant is (%p)~\n", (void *) ant );
                }
                else
                {
                    noprintf( "~DEBUG~SGI NOT detected ant is (%p)~\n", (void *) ant );
                }

                if (strstr( pos, "1x1" ))
                {
                    bGoodLine = true;
                }
                else if (strstr( pos, "2x2" ))
                {
                    ant++;                                 /* skip over the 1x1 antenna */
                    bGoodLine = true;
                }
                else if (strstr( pos, "3x3" ))
                {
                    ant++;                                 /* skip over the 1x1 antenna */
                    ant++;                                 /* skip over the 2x2 antenna */
                    bGoodLine = true;
                }
                else if (strstr( pos, "4x4" ))
                {
                    ant++;                                 /* skip over the 1x1 antenna */
                    ant++;                                 /* skip over the 2x2 antenna */
                    ant++;                                 /* skip over the 3x3 antenna */
                    bGoodLine = true;
                }

                if (bGoodLine)
                {
                    lines++;
                    noprintf( "~DEBUG~ant %p; antennas %p; sizeof 1 ant %u~", (void *) ant, (void *) &pAmpduData->antennas, sizeof( *ant ));
                    noprintf( "\n\n" );

                    token = strtok( pos, "\t" );
                    count = 0;
                    while (token)
                    {
                        /*printf("token is (%s) \n", token ); */
                        if (count == 0)
                        {
                            /*printf( "name token is (%s) ... copied to (%p) \n", token, &ant->name[0] );*/
                            strncpy( ant->name, token, sizeof( ant->name ) -1 );
                        }
                        else if (count <= BMON_NUM_MCS )
                        {
                            sscanf( token, "%ld", &level );
                            ant->level[count-1] = level;
                            /*if(ant->level[count-1]) fprintf( stderr, "level token is (%s) ... ant->level[%d]= %d (%p) \n",
                                    token, count-1, ant->level[count-1], (void *) &ant->level[count-1] );*/
                        }
                        token = strtok( NULL, "\t" );
                        count++;
                    }
                }
            }

            if (eol)
            {
                eol++;                                     /* skip over the end of line character ... should be pointing to beginning of next line ... or end of file */
            }

            pos = eol;
        } while (pos);

        FREE_SAFE( contents );
    }

    /*bmon_wifi_print_antennas( pAmpduData->antennas );*/

    return( 0 );
} /* bmon_wifi_read_ampdu_data */

int bmon_wifi_get_rssi_ant(
    const char              *ifname,
    phy_rssi_ant_t          *pRssiAnt
    )
{
    int                      tRetCode    = 0;
    phy_rssi_ant_t          *rssi_ant_p  = NULL;

    noprintf( "%s:%u: hBwl #%p \n", __FILE__, __LINE__, hBwl );
    do
    {
        bmon_wifi_init( ifname, __FUNCTION__ );

        if (hBwl == NULL)
        {
            PRINTF( "%s() Failed to Init BWL! \n", __FUNCTION__ );
            tRetCode = -1;
            break;
        }
        else
        {
            if (( rssi_ant_p = (phy_rssi_ant_t*) BWL_GetPhyRssiAnt( hBwl )) == NULL)
            {
                PRINTF( "~FATAL~BWL_GetPhyRssiAnt() failed with error %d~", tRetCode );
            }
            else
            {
                #if 0
                int idx;

                PRINTF( "%s:%u: rssi_ant->count #%d \n", __FILE__, __LINE__, rssi_ant_p->count );
                for (idx=0; idx<rssi_ant_p->count; idx++)
                {
                    int temp = rssi_ant_p->rssi_ant[idx];
                    PRINTF( "rssi[%d] %d (0x%x)  ", idx, temp, temp );
                }
                PRINTF("\n");
                #endif

                memcpy(pRssiAnt, rssi_ant_p, sizeof(*pRssiAnt) );

                FREE_SAFE( rssi_ant_p );
            }
        }
        bmon_wifi_uninit(__FUNCTION__);
    } while (0);

    return( tRetCode );
}

int bmon_wifi_get_counters(
    const char     *ifname,
    WiFiCounters_t *pCounters                              /* Counters structure */
    )
{
    int tRetCode = 0;

    do
    {
        bmon_wifi_init( ifname, __FUNCTION__ );

        if (hBwl == NULL)
        {
            PRINTF( "%s() Failed to Init BWL! \n", __FUNCTION__ );
            tRetCode = -1;
            break;
        }
        else
        {
            if (BWL_GetCounters( hBwl, pCounters ) == BWL_ERR_SUCCESS)
            {
            }
            else
            {
                PRINTF( "%s:%u: BWL_GetCounters() failed with error %d~", __FILE__, __LINE__, tRetCode );
            }
        }

        bmon_wifi_uninit(__FUNCTION__);
    } while (0);

    return( tRetCode );
}                                                          /* bmon_wifi_get_counters */

int bmon_wifi_get_samples(
    const char     *ifname,
    unsigned int   tSampleType,
    WiFiSamples_t *pSamples                              /* Samples structure */
    )
{
    int tRetCode = 0;

    do
    {
        bmon_wifi_init( ifname, __FUNCTION__ );

        if (hBwl == NULL)
        {
            PRINTF( "%s() Failed to Init BWL! \n", __FUNCTION__ );
            tRetCode = -1;
            break;
        }
        else
        {
            if (BWL_GetSamples( hBwl, tSampleType, pSamples ) == BWL_ERR_SUCCESS)
            {
            }
            else
            {
                printf( "%s:%u: BWL_GetSamples() failed with error %d~", __FILE__, __LINE__, tRetCode );
            }
        }

        bmon_wifi_uninit(__FUNCTION__);
    } while (0);

    return( tRetCode );
}                                                          /* bstbmon_wifi_get_counters */


int bmon_wifi_get_connected_info(
    const char *ifname,
    ScanInfo_t *tScanInfo
    )
{
    int i        = 0;
    int tRetCode = 0;

    do
    {
        bmon_wifi_init( ifname, __FUNCTION__ );

        if (hBwl == NULL)
        {
            PRINTF( "%s:%u: Failed to Init BWL! \n", __FILE__, __LINE__ );
            tRetCode = -1;
            break;
        }
        else
        {
            noprintf( "BWL_Init() succeeded after %d tries \n", i+1 );

            if (( tRetCode = BWL_GetConnectedInfo( hBwl, tScanInfo )) == BWL_ERR_SUCCESS)
            {
            }
            else
            {
                /*printf("~FATAL~BWL_GetConnectedInfo() failed with error %d~", tRetCode );*/
            }
        }
        bmon_wifi_uninit(__FUNCTION__);
    } while (0);

    return( tRetCode );
}                                                          /* bmon_wifi_get_connected_info */

int bmon_get_proc_net_tcp_bytes ( unsigned int *TxBytes, unsigned int *RxBytes )
{
    /* sl  local_address rem_address   st tx_queue rx_queue tr tm->when retrnsmt
      11: F901A8C0:BA0E F401A8C0:1F99 01 00000000:00549560 00:00000000 00000000     0        0 16539 1 cce35840 20 8 0 10 -1
     */
    char              my_wifi_ip_addr[INET6_ADDRSTRLEN];
    char              my_wifi_ip_addr_hex[INET6_ADDRSTRLEN];
    unsigned int line_num = 0;
    unsigned int socket_status = 0;
    unsigned long int local_address=0, rem_address=0;
    unsigned long int tx_queue=0, rx_queue=0;
    unsigned int      local_port=0, rem_port=0;
    char             *contents = NULL;
    struct in_addr    sin_temp_addr;

    if ( TxBytes == NULL || RxBytes == NULL ) return ( 0 );

    memset( &my_wifi_ip_addr, 0, sizeof( my_wifi_ip_addr ));
    memset( &sin_temp_addr, 0, sizeof( sin_temp_addr ) );

    /* determine if this board has wifi enabled */
    bmon_get_my_ip_addr_from_ifname( WIFI_INTERFACE_NAME, my_wifi_ip_addr, sizeof( my_wifi_ip_addr ) );

    inet_aton( my_wifi_ip_addr, &sin_temp_addr);
    sprintf( my_wifi_ip_addr_hex, "%lX", (unsigned long int) sin_temp_addr.s_addr );
#if DEBUG
    fprintf( stderr, "%s:%u: %s (0x%lX) ... (%s)\n", __FUNCTION__, __LINE__, my_wifi_ip_addr, (unsigned long int) sin_temp_addr.s_addr, my_wifi_ip_addr_hex);
#endif /* DEBUG */

    contents = bmon_get_file_contents_proc( PROC_NET_TCP_FILENAME, PROC_NET_TCP_SIZE_MAX );
    if ( contents )
    {
        char *begline        = contents;
        char *newline        = contents;

        do
        {
            newline = strchr( begline, '\n' );
            if ( newline == NULL ) break;

            *newline = '\0';

            /* if this line has our wireless IP address on it, then we need to process it */
            if ( strstr( begline, my_wifi_ip_addr_hex ) )
            {
    /* sl  local_address rem_address   st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode r sock mem re a q cg ss
       63: F901A8C0:C923 F401A8C0:1F99 01 00000000:0051769C 00:00000000 00000000     0        0 76795 2 cbbbad00 20 4 0 10 -1
    */
                sscanf( begline, "%u: %08lX:%04X %08lX:%04X %02X %08lX:%08lX", &line_num, &local_address, &local_port,
                        &rem_address, &rem_port, &socket_status, &tx_queue, &rx_queue );
                *TxBytes += tx_queue;
                *RxBytes += rx_queue;
#if DEBUG
                fprintf( stderr, "%s:%u - got line (%s) ... tx (%lx) rx (%lx)\n", __FUNCTION__, __LINE__, begline, tx_queue, rx_queue );
#endif /* DEBUG */
            }

            newline++;
            begline = newline;
        } while ( newline != NULL );

        /*printf( "TCP this is the server %d ... port %d\n", g_this_is_the_server, g_local_port_server );*/
        FREE_SAFE( contents );
    }
    else
    {
        fprintf( stderr, "%s - could not read file (%s)\n", __FUNCTION__, PROC_NET_TCP_FILENAME );
    }

    return ( 0 );
}

int bmon_wifi_ampdu_write_to_file(
    const char *buffer,
    const char *filename
    )
{
    FILE *fp = NULL;

    fp = fopen( filename, "a" );

    if (fp == NULL)
    {
        PRINTF( "%s:%u: could not open file (%s) for writing \n", __FUNCTION__, __LINE__, filename );
        return( -1 );
    }

    fwrite( buffer, 1, strlen( buffer ), fp );
    fclose( fp );

    return( 0 );
}                                                          /* bmon_wifi_ampdu_write_to_file */

/**
 *  Function: This function will search the provided buffer for the line that contains the three items:
 *  tot_mpdus, tot_ampdus, and mpduperampdu. Once the line is found, the values associated with these
 *  three tags will be scanned and written into the provided filename. The values in the provided file will
 *  eventually be read by the bsysperf.cgi when the user has checked the AMPDU Graph checkbox.
 *
 *  For TX, the line looks like this: tot_mpdus 0 tot_ampdus 0 mpduperampdu 0
 *  For RX, the line looks like this: rxampdu 0 rxmpdu 0 rxmpduperampdu 0 rxht 0 rxlegacy 0
 **/
char *bmon_wifi_ampdu_parse_for_totals(
    const char *buffer,
    const char *filename,
    int         RxOrTx /* RX==0 ... TX==1 */
    )
{
    char *pos = (char*) buffer;
    char *eol = NULL;
    char  output_line[256];

    noprintf("%s:%u: buffer (%p); filename (%s) \n", __FUNCTION__, __LINE__, buffer, filename );
    if (( buffer== NULL ) || ( filename == NULL ))
    {
        return( pos );
    }
    noprintf("%s:%u: buffer len (%u); filename (%s) \n", __FUNCTION__, __LINE__, strlen(buffer), filename );

    /* look for a line like this ... tot_mpdus 10000 tot_ampdus 20000 mpduperampdu 30000 */
    if ( RxOrTx == 0 )
    {
        pos = strstr(buffer, "rxampdu ");
    }
    else
    {
        pos = strstr(buffer, "tot_mpdus ");
    }
    if ( pos ) {
        unsigned long int tot_mpdus = 0;
        unsigned long int tot_ampdus = 0;
        unsigned long int mpduperampdu = 0;
        eol = strstr(pos, "\n");
        if ( eol ) {
            *eol = '\0';
        }

        memset( output_line, 0, sizeof(output_line) );

        /*printflog("~DEBUG~%s:%u: found line (%s)(%p) ... len %u~", __FUNCTION__, __LINE__, pos, (void*) pos, strlen(pos) );*/
        if ( RxOrTx == 0 )
        {
            sscanf( pos, "rxampdu %lu rxmpdu %lu rxmpduperampdu %lu", &tot_ampdus, &tot_mpdus, &mpduperampdu );
            snprintf(output_line, sizeof(output_line), "rx_mpdus\t%lu\trx_ampdus\t%lu\trx_mpduperampdu\t%lu\n", tot_mpdus, tot_ampdus, mpduperampdu );
            /*printflog("~DEBUG~%s:%u: rxampdu %lu rxmpdu %lu rxmpduperampdu %lu\n", __FUNCTION__, __LINE__, tot_ampdus, tot_mpdus, mpduperampdu );*/
        }
        else
        {
            sscanf( pos, "tot_mpdus %lu tot_ampdus %lu mpduperampdu %lu", &tot_mpdus, &tot_ampdus, &mpduperampdu );
            snprintf(output_line, sizeof(output_line), "tx_mpdus\t%lu\ttx_ampdus\t%lu\ttx_mpduperampdu\t%lu\n", tot_mpdus, tot_ampdus, mpduperampdu );
            /*printflog("~DEBUG~%s:%u: txampdu %lu txmpdu %lu txmpduperampdu %lu\n", __FUNCTION__, __LINE__, tot_ampdus, tot_mpdus, mpduperampdu );*/
        }

        bmon_wifi_ampdu_write_to_file( output_line, filename );

        pos += strlen(pos);

        /* if we null-terminated the line above, restore the new-line character */
        if ( eol ) {
            *eol = '\n';
        }
    }
    else
    {
        /*printf("%s:%u: tot_mpdus tag not found in buffer \n", __FUNCTION__, __LINE__ );*/
        pos = (char*) buffer;
    }

    return ( pos );
}

/*
 * Parse a line that looks similar to this and extract the 12 values and 12 percentages. There could be 2, or 3, or 4
 * subsequent lines after the first one ... depending on the wifi chip and the wifi router.
 *
 *                        1    1    2    2    3    3    4    4    5    5    6    6    7    7    8    8
 *              0    5    0    5    0    5    0    5    0    5    0    5    0    5    0    5    0    5
 *              TX VHT  :  33(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
 *                      :  3(0%)  0(0%)  0(0%)  0(0%)  2746(8%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
 *                      :  1408(4%)  1631(5%)  21440(70%)  3284(10%)  11(0%)  10(0%)  9(0%)  11(0%)  6(0%)  0(0%)  0(0%)  0(0%)
 *
 */
char *bmon_wifi_ampdu_parse_vmt(
    const char *buffer,
    const char *keyword,
    char       *values,
    char       *percentages,
    const char *filename
    )
{
    int          idx            = 0;
    unsigned int value          = 0;
    unsigned int percentage     = 0;
    int          antenna_count  = 0;
    char        *pos            = NULL;
    char        *line           = NULL;
    char        *beg_of_section = NULL;                    /* beginning of section */
    char         keywordStr[16];
    char         tempStr32[32];
    char         tempLine[512];
    int          debug = 0;

    if (( buffer== NULL ) || ( keyword == NULL ))
    {
        return( 0 );
    }

    beg_of_section = strstr( buffer, keyword );
    if ( buffer ) {
        if(debug) PRINTF( "%s:%u: BEFORE PROCESSING (%s) ... original buffer is len (%d) beg_of_section %p \n---------------------------------\n",
                __FUNCTION__, __LINE__, keyword, (int) strlen(buffer), beg_of_section );
    } else {
        if(debug) PRINTF( "%s:%u: BEFORE PROCESSING (%s) ... original buffer is len (0) \n---------------------------------\n",
                __FUNCTION__, __LINE__, keyword );
    }
    if (beg_of_section)
    {
        bool bValidLineFound = false;

        memset( keywordStr, 0, sizeof( keywordStr ));
        memset( tempLine, 0, sizeof( tempLine ));

        strncpy( tempLine, beg_of_section, MIN( sizeof(tempLine) - strlen(tempLine) - 1, strchr( beg_of_section, '\n' ) - beg_of_section ) );
        strncpy( keywordStr, beg_of_section, MIN( sizeof(tempLine) - strlen(tempLine) - 1, strchr( beg_of_section, ':' ) - beg_of_section ) );

        line = beg_of_section;

        do
        {
            bValidLineFound = false;

            /* TX VHT  :  33(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)                       */
            pos = strchr( tempLine, ':' );
            if(debug) PRINTF( "%s:%u: tempLine (%s) pos (%p) \n", __FUNCTION__, __LINE__, tempLine, pos );
            if (pos)
            {
                bool  bReplacedNewline = false;
                char *eol              = NULL;
                int   value_count      = 0;

                antenna_count++;                           /* used to decide 1x1, 2x2, 3x3, or 4x4 */

                bValidLineFound = true;

                if (( eol = strchr( pos, '\n' )))
                {
                    bReplacedNewline = true;
                    *eol             = 0;
                }

                memset( tempStr32, 0, sizeof( tempStr32 ));

                sprintf( tempStr32, "%s %dx%d\t", keywordStr, antenna_count, antenna_count );
                strcat( values, tempStr32 );

                sprintf( tempStr32, "%s %dx%d %%\t", keywordStr, antenna_count, antenna_count );
                strcat( percentages, tempStr32 );

                if(debug) PRINTF( "%s:%u: found line (%s) ... ant %d \n", __FUNCTION__, __LINE__, tempLine, antenna_count );
                pos += 3;                                  /*skip over the colon and the 1st two spaces */
                for (idx = 0; pos; idx++)
                {
                    memset( tempStr32, 0, sizeof( tempStr32 ));
                    value      = 0;
                    percentage = 0;
                    strncpy( tempStr32, pos, 11 );
                    sscanf( pos, "%d(%d)", &value, &percentage );
                    /*if (value || percentage )*/
                    {
                        char lvalue[16], lpercentage[8];
                        sprintf( lvalue, "%d", value );
                        sprintf( lpercentage, "%d", percentage );

                        value_count++;

                        if (value_count > 1)
                        {
                            strcat( values, "\t" );
                            strcat( percentages, "\t" );
                        }
                        strcat( values, lvalue );
                        strcat( percentages, lpercentage );

                        noprintf( "%s[%d]: offset (%ld) from (%s) value %d; percentage %d \n", keywordStr, idx,
                           (long int) ((unsigned int*)pos-(unsigned int*)line), tempStr32, value, percentage );
                    }

                    pos = strstr( pos, "  " );
                    if (pos)
                    {
                        pos += 2;                          /*skip over the two spaces */
                    }
                }

                if (bReplacedNewline && eol)
                {
                    *eol = '\n';
                }

                line = strchr( line, '\n' );
                if (line)
                {
                    line++;

                    memset( tempLine, 0, sizeof( tempLine ));
                    strncpy( tempLine, line, MIN( sizeof(tempLine) - strlen(tempLine) - 1, strchr( line, '\n' ) - line ) );
                    if(debug) PRINTF( "%s:%u: next line (%s) ... ant %d \n", __FUNCTION__, __LINE__, tempLine, antenna_count );
                }

                strcat( values, "\t\t\t" );                /* separate this line with a couple of tabs */
                strcat( percentages, "\n" );               /* put the percentages on the same line as the collected values just over to the right a few columns */

                bmon_wifi_ampdu_write_to_file( values, filename );
                bmon_wifi_ampdu_write_to_file( percentages, filename );
                if(debug) PRINTF( "%s:%u: values (%s) \n", __FUNCTION__, __LINE__, values );
                if(debug) PRINTF( "%s:%u: percen (%s) \n", __FUNCTION__, __LINE__, percentages );

                values[0]      = '\0';
                percentages[0] = '\0';
            }
            else
            {
                if(debug) PRINTF( "%s:%u: could not find colon after beg_of_section (%s) \n", __FUNCTION__, __LINE__, beg_of_section );
            }
            if(debug) PRINTF( "%s:%u: endwhile: pos %p;  line %p \n", __FUNCTION__, __LINE__, pos, line );
            if(debug && line) PRINTF( "%s:%u: endwhile: line[0] (%02x)\n", __FUNCTION__, __LINE__, line[0] );

            /* each TX/RX section may have 2, 3, or 4 more lines that begin with a space.
               If we encounter a line that does NOT start with a space, we have found the end of the TX/RX section */
        } while ( bValidLineFound && line && line[0] == ' ' );

        if (line)
        {
            memset( tempLine, 0, sizeof( tempLine ));
            strncpy( tempLine, line, MIN( sizeof(tempLine) - strlen(tempLine) - 1, strchr( line, '\n' ) - line ) );
        }
        if(debug) PRINTF( "%s:%u: found end of section at line (%s) \n", __FUNCTION__, __LINE__, tempLine );
    }
    else
    {
        if(debug) PRINTF( "%s:%u: could not find keyword (%s) \n", __FUNCTION__, __LINE__, keyword );
    }

    if(debug) PRINTF( "%s:%u: end ... line (%p) \n", __FUNCTION__, __LINE__, (void*) line );
    return( line );
}                                                          /* bmon_wifi_ampdu_parse_vmt */

int bmon_wifi_get_cca_stats(
    const char       *ifname,
    BWLGetCcaStats_t *lBWLGetCcaStats
    )
{
    int   num_addresses = 0;

    do
    {
        bmon_wifi_init( ifname, __FUNCTION__ );

        if (hBwl == NULL)
        {
            printf( "%s() Failed to Init BWL! \n", __FUNCTION__ );
            break;
        }
        else
        {
            BWL_GetCcaStats( hBwl, lBWLGetCcaStats );
            /*printflog( "%s: after BWL_GetCcaStats() ... rc %d\n", __FUNCTION__, num_addresses );*/
        }
        bmon_wifi_uninit(__FUNCTION__);
    } while (0);

    return( num_addresses );
}   /* bmon_wifi_get_cca_stats */
void bmon_wifi_trigger_scan(
    const char     *ifname
    )
{
    do
    {
        bmon_wifi_init( ifname, __FUNCTION__ );

        if (hBwl == NULL)
        {
            PRINTF( "%s() Failed to Init BWL! \n", __FUNCTION__ );
            break;
        }
        else
        {
            ScanParams_t ScanParams;

            memset( &ScanParams, 0, sizeof(ScanParams) );
            ScanParams.lActiveTime  = BWL_DEFAULT_SCAN_DWELL_TIME;
            ScanParams.lPassiveTime = BWL_DEFAULT_SCAN_DWELL_TIME;
            ScanParams.lHomeTime    = BWL_DEFAULT_SCAN_DWELL_TIME;
            BWL_Scan( hBwl, &ScanParams );
        }

        bmon_wifi_uninit(__FUNCTION__);
    } while (0);
}

ScanInfo_t* bmon_wifi_get_scanresults(
    const char     *ifname,
    unsigned int *num
    )
{
    ScanInfo_t  *pData = NULL;

    do
    {
        bmon_wifi_init( ifname, __FUNCTION__ );

        if (hBwl == NULL)
        {
            PRINTF( "%s() Failed to Init BWL! \n", __FUNCTION__ );
            break;
        }
        else
        {
            BWL_GetScannedApNum( hBwl, num );
            if (*num){
                pData = (ScanInfo_t*)malloc( (*num) * sizeof( ScanInfo_t ) );
                if( pData )
                {
                    BWL_GetScanResults( hBwl, pData );
#ifdef BMON_DEBUG
                    int ii=0;
                    for( ii = 0; ii < ulNumOfAp; ii++ )
                    {
                        /* display data */
                        printf( "SSID: \"%s\" BSSID: %02x:%02x:%02x:%02x:%02x:%02x RSSI: %d dB Chan: %d\n", pData[ii].tCredentials.acSSID,
                                pData[ii].BSSID.octet[0], pData[ii].BSSID.octet[1], pData[ii].BSSID.octet[2],
                                pData[ii].BSSID.octet[3], pData[ii].BSSID.octet[4], pData[ii].BSSID.octet[5],
                                pData[ii].lRSSI, pData[ii].ulChan );
                    }
#endif
                }
            }
        }

        bmon_wifi_uninit(__FUNCTION__);
    } while (0);
    return(pData);
}

int bmon_wifi_get_driver_version(
    const char *ifname,
    char *strDriverVersion,
    int   iDriverVersionLen
    )
{
    int   tRetCode = 0;
    char *dump_buf = NULL;
    char *pVersionS = NULL;
    char *pVersionE = NULL;

    strncpy( strDriverVersion, "unknown", iDriverVersionLen - 1 );
    do
    {
        bmon_wifi_init( ifname ,__FUNCTION__);

        if (hBwl == NULL)
        {
            PRINTF( "%s() Failed to Init BWL! \n", __FUNCTION__ );
            break;
        }

        if (( dump_buf = BWL_GetDriverVersion ( hBwl )) == NULL)
        {
            tRetCode = -1;
            break;
        }

        bmon_wifi_uninit(__FUNCTION__);

        pVersionS = strstr( dump_buf, "version " );
        if ( pVersionS )
        {
            pVersionS += strlen( "version " );

            pVersionE = strstr( pVersionS, " " );
            if ( pVersionE )
            {
                *pVersionE = '\0'; /* null-terminate the version string part */
                strncpy( strDriverVersion, pVersionS, iDriverVersionLen - 1 );
            }
        }
    } while (0);

    return( tRetCode );
}
int bmon_wifi_get_revs(
    const char *ifname,
    RevInfo_t  *tRevInfo
    )
{
    int i        = 0;
    int tRetCode = 0;

    do
    {
        bmon_wifi_init( ifname ,__FUNCTION__);
        if (hBwl == NULL)
        {
            tRetCode = -1;
            break;
        }
        else
        {
            memset( tRevInfo, 0, sizeof( RevInfo_t ));
            BWL_API_CHECK( BWL_GetRevInfo( hBwl, tRevInfo ), tRetCode );
        }
        bmon_wifi_uninit(__FUNCTION__);
    } while (0);

err_out:
    return( tRetCode );
}
static char *strBandwidth[6] = {"INVALID", "10MHz", "20MHz", "40MHz", "80MHz", "160MHz"};
char *bmon_wifi_get_BandwidthStr(
    Bandwidth_t bandwidth
    )
{
    return( strBandwidth[bandwidth] );
}

int bmon_wifi_get_NRate(
    const char *ifname,
    uint32_t   *pNRate
    )
{
    int tRetCode = 0;

    do
    {
        bmon_wifi_init( ifname ,__FUNCTION__);

        if (hBwl == NULL)
        {
            tRetCode = -1;
            break;
        }
        else
        {
            if (BWL_GetNRate( hBwl, pNRate ) == BWL_ERR_SUCCESS)
            {
            }
            else
            {
            }
        }
        bmon_wifi_uninit(__FUNCTION__);
    } while (0);

    return( tRetCode );
}
int bmon_wifi_get_pm2_rcv_dur(
    const char *ifname,
    uint32_t   *pm2_rcv_dur
    )
{
    int tRetCode = 0;

    do
    {
        bmon_wifi_init( ifname ,__FUNCTION__);

        if (hBwl == NULL)
        {
            tRetCode = -1;
            break;
        }
        else
        {
            if (BWL_GetPM2_rcv_dur( hBwl, pm2_rcv_dur ) == BWL_ERR_SUCCESS)
            {
            }
            else
            {
            }
        }
        bmon_wifi_uninit(__FUNCTION__);
    } while (0);

    return( tRetCode );
}
int bmon_wifi_set_pm2_rcv_dur(
    const char *ifname,
    uint32_t   pm2_rcv_dur
    )
{
    int tRetCode = 0;

    do
    {
        bmon_wifi_init( ifname ,__FUNCTION__);

        if (hBwl == NULL)
        {
            tRetCode = -1;
            break;
        }
        else
        {
            if (BWL_SetPM2_rcv_dur( hBwl, pm2_rcv_dur ) == BWL_ERR_SUCCESS)
            {
            }
            else
            {
            }
        }
        bmon_wifi_uninit(__FUNCTION__);
    } while (0);

    return( tRetCode );
}


char*  bmon_wifi_get_rate_print(
    char    *rate_buf,
    uint32_t rspec
    )
{
    uint        encode, rate, txexp, bw_val;
    const char *stbc;
    const char *ldpc;
    const char *sgi;
    const char *bw;

    encode = ( rspec & WL_RSPEC_ENCODING_MASK );
    rate   = ( rspec & WL_RSPEC_RATE_MASK );
    txexp  = ( rspec & WL_RSPEC_TXEXP_MASK ) >> WL_RSPEC_TXEXP_SHIFT;
    bw_val = ( rspec & WL_RSPEC_BW_MASK );
    stbc   = (( rspec & WL_RSPEC_STBC ) != 0 ) ? " stbc" : "";
    ldpc   = (( rspec & WL_RSPEC_LDPC ) != 0 ) ? " ldpc" : "";
    sgi    = (( rspec & WL_RSPEC_SGI )  != 0 ) ? " sgi"  : "";

    if (bw_val == WL_RSPEC_BW_UNSPECIFIED)
    {
        bw = "auto";
    }
    else if (bw_val == WL_RSPEC_BW_20MHZ)
    {
        bw = "20";
    }
    else if (bw_val == WL_RSPEC_BW_40MHZ)
    {
        bw = "40";
    }
    else if (bw_val == WL_RSPEC_BW_80MHZ)
    {
        bw = "80";
    }
    else if (bw_val == WL_RSPEC_BW_160MHZ)
    {
        bw = "160";
    }
    else if (bw_val == WL_RSPEC_BW_10MHZ)
    {
        bw = "10";
    }
    else if (bw_val == WL_RSPEC_BW_5MHZ)
    {
        bw = "5";
    }
    else if (bw_val == WL_RSPEC_BW_2P5MHZ)
    {
        bw = "2.5";
    }
    else
    {
        bw = "???";
    }

    if (( rspec & ~WL_RSPEC_TXEXP_MASK ) == 0)             /* Ignore TxExpansion for NULL rspec check */
    {
        sprintf( rate_buf, "auto" );
    }
    else if (encode == WL_RSPEC_ENCODE_RATE)
    {
        sprintf( rate_buf, "rate %d%s Mbps Tx Exp %d",
            rate/2, ( rate % 2 ) ? ".5" : "", txexp );
    }
    else if (encode == WL_RSPEC_ENCODE_HT)
    {
        sprintf( rate_buf, "ht mcs %d Tx Exp %d BW %s%s%s%s",
            rate, txexp, bw, stbc, ldpc, sgi );
    }
    else if (encode == WL_RSPEC_ENCODE_VHT)
    {
        uint mcs = ( rspec & WL_RSPEC_VHT_MCS_MASK );
        uint Nss = ( rspec & WL_RSPEC_VHT_NSS_MASK ) >> WL_RSPEC_VHT_NSS_SHIFT;
        sprintf( rate_buf, "vht mcs %d Nss %d Tx Exp %d BW %s%s%s%s",
            mcs, Nss, txexp, bw, stbc, ldpc, sgi );
    }
    else
    {
        sprintf( rate_buf, "<unknown encoding for ratespec 0x%08X>", rspec );
    }

    return( rate_buf );
}

int bmon_wifi_get_connected_stas(
    const char     *ifname,
    unsigned int   outputListLen,
    BWL_MAC_ADDRESS *outputList                              /* Samples structure */
    )
{
    int tRetCode = 0;

    do
    {
        bmon_wifi_init( ifname, __FUNCTION__ );

        if (hBwl == NULL)
        {
            PRINTF( "%s() Failed to Init BWL! \n", __FUNCTION__ );
            tRetCode = -1;
            break;
        }
        else
        {
            tRetCode=BWL_GetMacAssocList( hBwl, outputList, outputListLen );
        }

        bmon_wifi_uninit(__FUNCTION__);
    } while (0);

    return( tRetCode );
}                                                          /* bstbmon_wifi_get_counters */

int bmon_wifi_get_apsta(
    const char     *ifname,
    unsigned int   pulApSta
    )
{
    int tRetCode = 0;

    do
    {
        bmon_wifi_init( ifname, __FUNCTION__ );

        if (hBwl == NULL)
        {
            PRINTF( "%s() Failed to Init BWL! \n", __FUNCTION__ );
            tRetCode = -1;
            break;
        }
        else
        {
            tRetCode=BWL_GetApSta( hBwl, &pulApSta );
        }

        bmon_wifi_uninit(__FUNCTION__);
    } while (0);

    return( tRetCode );

}
int bmon_wifi_get_mode(
    const char     *ifname,
    unsigned int   *pulApSta
    )
{
    int tRetCode = 0;

    do
    {
        bmon_wifi_init( ifname, __FUNCTION__ );

        if (hBwl == NULL)
        {
            PRINTF( "%s() Failed to Init BWL! \n", __FUNCTION__ );
            tRetCode = -1;
            break;
        }
        else
        {
            tRetCode=BWL_GetApMode( hBwl, pulApSta );
        }

        bmon_wifi_uninit(__FUNCTION__);
    } while (0);

    return( tRetCode );

}

#endif /* BWL_SUPPORT */
