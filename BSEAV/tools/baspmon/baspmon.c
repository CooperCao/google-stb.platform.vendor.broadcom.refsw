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
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/utsname.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/klog.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "bstd.h"
#include "bmemperf_server.h"
#include "bmemperf_utils.h"
#include "bmemperf_info.h"
#include "bmemperf_cgi.h"

#include "bmemperf.h"
#include "bmemperf_lib.h"
#include "boxmodes_defines.h"

#define MEMDMA_NAME_WR "xpt_wr_memdma"
#define MEMDMA_NAME_RD "xpt_rd_memdma"
#define SYSPORT_NAME_WR "sysport_wr"
#define SYSPORT_NAME_RD "sysport_rd"
#define XPT_BITRATE_NUM 32

#if 0
#ifdef PRINTF
#undef PRINTF
#endif
#define PRINTF printf
#endif
char g_uuid[36];

char         *g_client_name[BMEMPERF_MAX_NUM_CLIENT];
int           g_MegaBytes           = 0;                   /* set to 1 when user wants data displayed in megabytes instead of megabits (default) */
int           g_MegaBytesDivisor[2] = {1, 8};
char         *g_MegaBytesStr[2] = {"Mbps", "MBps"};
bmemperf_info g_bmemperf_info;
bool          gPerfError = false;

extern bmemconfig_box_info g_bmemconfig_box_info[BMEMCONFIG_MAX_BOXMODES];

static int getProductIdMemc(
    void
    )
{
    unsigned int idx     = 0;
    int          numMemc = 1;
    char         *lProductIdStr = getProductIdStr();

    /* loop through global array to find the specified boxmode */
    for (idx = 0; idx < ( sizeof( g_bmemconfig_box_info )/sizeof( g_bmemconfig_box_info[0] )); idx++)
    {
        PRINTF( "~DEBUG~%s: g_bmemconfig_box_info[%d].strProductId is %s ... comparing with %s~\n", __FUNCTION__, idx,
                g_bmemconfig_box_info[idx].strProductId, lProductIdStr );
        /* some 7439 IDs have 7252s (boxmode 24) and some have 7252S (boxmode 2, 3, 4, 9, 27) */
        if ( lProductIdStr && strcasecmp ( g_bmemconfig_box_info[idx].strProductId, lProductIdStr ) == 0 )
        {
            numMemc = g_bmemconfig_box_info[idx].numMemc;
            PRINTF( "~DEBUG~%s: match at boxmode %u; numMemc %u ~\n", __FUNCTION__, idx, numMemc );
            break;
        }
    }

    /* we did not find the specified product id */
    return( numMemc );
}  /* getBoxModeMemc */

int main(
    int   argc,
    char *argv[]
    )
{
    int                     rc           = 0;
    int                     memc         = 0;
    int                     client       = 0;
    int                     cpu          = 0;
    int                     idx          = 0;
    struct utsname          uname_info;
    char                   *boltVersion  = NULL;
    char                   *queryString  = NULL;
    int                     epochSeconds = 0;
    int                     tzOffset     = 0;
    int                     boxmode      = 1;
    struct sockaddr_in      server;
    struct hostent         *hp           = NULL;
    char                    ThisHost[80];
    char                    ServerIpAddr[INET6_ADDRSTRLEN]; /* 127.0.0.1 */
    char                    LocalIpAddr[INET6_ADDRSTRLEN]; /* 10.14.233.136 or 192.168.1.10 */
    unsigned int            numberOfMemcs = 0;
    volatile unsigned int  *g_pMem = NULL;
    float                   cpuMemcUtil = 0.0;
    float                   cpuAvg = 0.0;
    long int                cpuIrqs = 0;
    unsigned int            bpc = BitsPerCycle( 0 );
    unsigned int            ddr = 0;
    unsigned int            numProcessorsConfigured = sysconf( _SC_NPROCESSORS_CONF );
    unsigned int            numProcessorsOnline     = sysconf( _SC_NPROCESSORS_ONLN );
    unsigned long int       sysportIrqs     = 0;
    unsigned long int       sysportMemcUtil = 0;
    unsigned long int       memdmaMemcUtil  = 0;
    unsigned long int       xptMemcUtil     = 0;
    float                   xptMbpsTotal    = 0.0;
    unsigned long int       aspMemcUtil     = 0;
    unsigned long int       aspChanCount    = 0;
    float                   aspMbpsTotal    = 0.0;
    char                    valueStr[64];
    unsigned char           xptStatus[XPT_BITRATE_NUM];
    unsigned char           aspStatus[XPT_BITRATE_NUM];
    struct timespec         time1;
    struct timeval          tv1, tv2;
    unsigned long long int  microseconds1 = 0, microseconds2 = 0;
    unsigned long int       microseconds_delta = 0;
    bmemperf_boxmode_source boxmodeSource;
    bmemperf_version_info   versionInfo;
    bmemperf_request        request;
    bmemperf_response       response;
    bmemperf_xpt_details    xptDetails;
    bmemperf_asp_details    aspDetails;
    char                    PowerProbeIpAddr[INET6_ADDRSTRLEN];
    char                    PowerProbeShunts[32];
    char                    ResetOption[2]; /* either "A" for ASP mode or "N" for non-ASP mode */
    char                    ClientStreamerIpAddr[INET6_ADDRSTRLEN];
    int                     ClientStreamerAction; /* either +1 or -1 */
    char                    CfgNewValueName[64];
    char                    CfgNewValueContents[64];
    struct stat             statbuf;
    char                    BCM45316_IMAGE[64];
    char                    RIGHT_HAND_LAYOUT_IMAGE[64];
    char                    EXTERNAL_DISK_IMAGE[64];

    /* Open driver for memory mapping and mmap() it */
    g_pMem = bmemperf_openDriver_and_mmap();

    if (g_pMem == NULL)
    {
        printf( "Failed to bmemperf_openDriver_and_mmap() ... g_pMem %p \n", (void*) g_pMem );
        return( -1 );
    }

    printf( "Content-type: text/html\n\n" );

    queryString = getenv( "QUERY_STRING" );

    if (queryString == NULL)
    {
        printf( "QUERY_STRING env is not defined\n" );
        return( 0 );
    }
    printf( "~DEBUG~queryString (%s)~", queryString );

    memset( g_uuid, 0, sizeof( g_uuid ));
    memset( &uname_info, 0, sizeof(uname_info));
    memset( &request, 0, sizeof( request ));
    memset( &response, 0, sizeof( response ));
    memset( &boxmodeSource, 0, sizeof( boxmodeSource ));
    memset( &xptStatus, 0, sizeof( xptStatus ));
    memset( &aspStatus, 0, sizeof( aspStatus ));
    memset( &tv1, 0, sizeof( tv1 ));
    memset( &tv2, 0, sizeof( tv2 ));
    memset( &xptDetails, 0, sizeof( xptDetails ));
    memset( &aspDetails, 0, sizeof( aspDetails ));
    memset( &PowerProbeIpAddr, 0, sizeof( PowerProbeIpAddr ));
    memset( &PowerProbeShunts, 0, sizeof( PowerProbeShunts ));
    memset( &ResetOption, 0, sizeof( ResetOption ));
    memset( &ClientStreamerIpAddr, 0, sizeof( ClientStreamerIpAddr ));
    memset( &CfgNewValueName, 0, sizeof( CfgNewValueName ));
    memset( &CfgNewValueContents, 0, sizeof( CfgNewValueContents ));
    memset( &BCM45316_IMAGE, 0, sizeof( BCM45316_IMAGE ));
    memset( &RIGHT_HAND_LAYOUT_IMAGE, 0, sizeof( RIGHT_HAND_LAYOUT_IMAGE ));
    memset( &EXTERNAL_DISK_IMAGE, 0, sizeof( EXTERNAL_DISK_IMAGE ));

    gettimeofday( &tv1, NULL );
    microseconds1 = (tv1.tv_sec * 1000000LL); /* q-scale shift the seconds left to allow for addition of microseconds */
    microseconds1 += tv1.tv_usec; /* add in microseconds */


    clock_gettime( CLOCK_REALTIME, &time1 );
    srandom ( (unsigned int) time1.tv_nsec/1000 );

    if (strlen( queryString ))
    {
        scanForInt( queryString, "datetime=", &epochSeconds );
        scanForInt( queryString, "tzoffset=", &tzOffset );
        scanForStr( queryString, "uuid=", sizeof( g_uuid ), g_uuid );
        scanForStr( queryString, "PowerProbeIpAddr=", sizeof( PowerProbeIpAddr ), PowerProbeIpAddr );
        scanForStr( queryString, "PowerProbeShunts=", sizeof( PowerProbeShunts ), PowerProbeShunts );
        scanForStr( queryString, "ResetOption=", sizeof( ResetOption ), ResetOption );
        scanForInt( queryString, "ClientStreamerAction=", &ClientStreamerAction );
        scanForStr( queryString, "ClientStreamerIpAddr=", sizeof( ClientStreamerIpAddr ), ClientStreamerIpAddr );
        scanForStr( queryString, "LocalIpAddr=", sizeof( LocalIpAddr), LocalIpAddr);
        scanForStr( queryString, "PowerProbeIpAddrNew=", sizeof( CfgNewValueContents ), CfgNewValueContents );
        if ( strlen(CfgNewValueContents) )
        {
            strncpy( CfgNewValueName, "PowerProbeIpAddr", sizeof(CfgNewValueName) -1 );
        }
        else
        {
            scanForStr( queryString, "ClientStreamerIpAddrNew=", sizeof( CfgNewValueContents ), CfgNewValueContents );
            if ( strlen(CfgNewValueContents) )
            {
                strncpy( CfgNewValueName, "ClientStreamerIpAddr", sizeof(CfgNewValueName) -1 );
            }
        }
    }

    if ( strlen(ResetOption) )
    {
        char line[MAX_LINE_LENGTH];
        printf( "~DEBUG~ResetOption (%s) ~", ResetOption );
        if ( ResetOption[0] == 'A' ) /* configure for ASP mode */
        {
            FILE *fp = fopen( "asp", "a" );
            if ( fp ) fclose(fp);
            printf( "~DEBUG~creating file (asp)~" );
        }
        else if ( ResetOption[0] == 'N' ) /* non-ASP mode */
        {
            remove( "asp" );
            printf( "~DEBUG~deleting file (asp)~" );
        }
        fflush(stdout); /* make sure the file I/O has completed */
        fflush(stderr);

        sprintf( line, "/sbin/reboot" );
        /*printf( "~DEBUG~issuing system(%s)~ALLDONE~", line );*/

        /* the system will reboot with this command ... control will never come back to here */
        system( line );
    }

    get_boxmode( &boxmodeSource );
    boxmode = boxmodeSource.boxmode;
    numberOfMemcs = getProductIdMemc();

    /* if browser provided a new date/time value; this only happens once during initialization */
    if (epochSeconds)
    {
        unsigned long int pid = 0;
        struct timeval    now = {1400000000, 0};

        strncpy( versionInfo.platform, getPlatform(), sizeof( versionInfo.platform ) - 1 );
        strncpy( versionInfo.platVersion, getPlatformVersion(), sizeof( versionInfo.platVersion ) - 1 );
        versionInfo.majorVersion = MAJOR_VERSION;
        versionInfo.minorVersion = MINOR_VERSION;
        printf( "~PLATFORM~%s", versionInfo.platform );
        printf( "~VARIANT~%s", getProductIdStr() );
        printf( "~PLATVER~%s", versionInfo.platVersion );
        printf( "~VERSION~Tool Ver: %u.%u~", versionInfo.majorVersion, versionInfo.minorVersion );

        boltVersion = getFileContents( "/proc/device-tree/bolt/tag" );
        if ( boltVersion )
        {
            printf( "~BOLTVER~%s", boltVersion );
            Bsysperf_Free( boltVersion );
        }

        uname( &uname_info );
        printf( "~UNAME~%d-bit %s %s~", ( sizeof( char * ) == 8 ) ? 64 : 32, uname_info.machine, uname_info.release );

        printf( "~MEMCNUM~%u~", numberOfMemcs );

        printf( "~BITSPERSECOND~%d~DDRFREQ~%d~", bpc, ddr );

        Bmemperf_GetCfgFileEntry( BASPMON_CFG_FILENAME, "PowerProbeIpAddr", PowerProbeIpAddr, sizeof(PowerProbeIpAddr) );
        printf( "~PowerProbeIpAddr~%s~", PowerProbeIpAddr );

        Bmemperf_GetCfgFileEntry( BASPMON_CFG_FILENAME, "ClientStreamerIpAddr", ClientStreamerIpAddr, sizeof(ClientStreamerIpAddr) );
        printf( "~ClientStreamerIpAddr~%s~", ClientStreamerIpAddr );

        Bmemperf_GetCfgFileEntry( BASPMON_CFG_FILENAME, "BCM45316_IMAGE", BCM45316_IMAGE, sizeof(BCM45316_IMAGE) );
        printf( "~BCM45316_IMAGE~%s~", BCM45316_IMAGE );

        Bmemperf_GetCfgFileEntry( BASPMON_CFG_FILENAME, "RIGHT_HAND_LAYOUT_IMAGE", RIGHT_HAND_LAYOUT_IMAGE, sizeof(RIGHT_HAND_LAYOUT_IMAGE) );
        printf( "~RIGHT_HAND_LAYOUT_IMAGE~%s~", RIGHT_HAND_LAYOUT_IMAGE );

        Bmemperf_GetCfgFileEntry( BASPMON_CFG_FILENAME, "EXTERNAL_DISK_IMAGE", EXTERNAL_DISK_IMAGE, sizeof(EXTERNAL_DISK_IMAGE) );
        printf( "~EXTERNAL_DISK_IMAGE~%s~", EXTERNAL_DISK_IMAGE );

        if (lstat( "asp", &statbuf ) != -1)
        {
            printf( "~checkboxAsp~" ); /* let the browser know that the asp file exists */
        }

        pid = getPidOf( "http_file_streamer" );
        printf( "~http_file_streamer_pid~%lu~", pid );

        now.tv_sec = epochSeconds - ( tzOffset * 60 );
        settimeofday( &now, NULL );

        usleep( 100 );
    }

    strncpy( ThisHost, "localhost", sizeof( ThisHost ));
    getservbyname( "echo", "tcp" );

    strncpy( ThisHost, "localhost", sizeof( ThisHost ));

    PRINTF( "TCP/Client running at HOST (%s)\n", ThisHost );
    if (( hp = gethostbyname2( ThisHost, AF_INET )) == NULL)
    {
        fprintf( stderr, "Can't find host %s\n", "localhost" );
        exit( -1 );
    }
    bcopy( hp->h_addr, &( server.sin_addr ), hp->h_length );
    PRINTF( "TCP/Client INET ADDRESS is: (%s)\n", inet_ntoa( server.sin_addr ));

    rc = gethostbyaddr2( ThisHost, BMEMPERF_SERVER_PORT, ServerIpAddr, sizeof( ServerIpAddr ));
    if (rc != 0)
    {
        perror( "ERROR getting address for port" );
        return( -1 );
    }

    inet_pton( AF_INET, ServerIpAddr, &server.sin_addr );

    /* Construct name of socket to send to. */
    server.sin_family = AF_INET;
    server.sin_port   = htons( BMEMPERF_SERVER_PORT );

    /* if the user is wanting power probe voltage/current */
    if ( strlen(PowerProbeIpAddr) && strlen(PowerProbeShunts) )
    {
        char              *token = NULL; /* PowerProbeShunts=2,2,10,0,0,0,0,0 */
        int                count = 0;
        unsigned long int  shunt = 0;
        strncpy( &request.request.overall_stats_data.PowerProbeIpAddr[0], &PowerProbeIpAddr[0], sizeof(request.request.overall_stats_data.PowerProbeIpAddr) - 1);

        token = strtok( PowerProbeShunts, "," );
        count = 0;
        while (token && count < POWER_PROBE_MAX )
        {
            sscanf( token, "%lu", &shunt );
            /*printf( "~DEBUG~probe %d is %lu (%s) ~", count, shunt, token );*/
            token = strtok( NULL, "," );
            request.request.overall_stats_data.PowerProbeShunts[count] = shunt;
            count++;
        }
    }

    /* if the user is wanting client streamer interaction AND this is NOT the first pass */
    if ( strlen( ClientStreamerIpAddr) && epochSeconds == 0 )
    {
        char *myIpAddr = getenv( "HTTP_HOST" );
        /* if the browser is talking with the local IP address network, find the corresponding local IP server address */
        if ( strstr( ClientStreamerIpAddr, "192.168" ) != NULL )
        {
            myIpAddr = get_my_ip4_local();
        }
        strncpy( &request.request.overall_stats_data.ClientStreamerIpAddr[0], &ClientStreamerIpAddr[0], sizeof(request.request.overall_stats_data.ClientStreamerIpAddr) - 1);
        strncpy( &request.request.overall_stats_data.ServerStreamerIpAddr[0], myIpAddr, sizeof(request.request.overall_stats_data.ServerStreamerIpAddr) - 1);
        request.cmdSecondaryOption = ClientStreamerAction;
        PRINTF( "~DEBUG~ClientIp (%s) ... ServerIp (%s) ... Action %d~", ClientStreamerIpAddr, myIpAddr, ClientStreamerAction );
    }

    bmemperf_boxmode_init( boxmode );

    request.boxmode = boxmode;

    PRINTF( "~DEBUG~Sending request (0x%x) to port (%u) ... cmdSecondary 0x%x ... option 0x%lx ~", BMEMPERF_CMD_GET_OVERALL_STATS,
            BMEMPERF_SERVER_PORT, request.cmdSecondary, request.cmdSecondaryOption );
    rc = send_request_read_response( &server, (unsigned char*) &request, sizeof(request),
            (unsigned char*) &response, sizeof(response), BMEMPERF_SERVER_PORT, BMEMPERF_CMD_GET_OVERALL_STATS );
    if (rc < 0)
    {
        perror( "error sending request " );
        return( -1 );
    }

    PRINTF( "~DEBUG~Received from server (%d): cmd (%d); boxmode %u; numberOfMemcs %u~", BMEMPERF_SERVER_PORT, response.cmd, boxmode, numberOfMemcs );

    ddr = response.response.overallStats.systemStats[idx].ddrFreqMhz;
    PRINTF( "~DEBUG~Response BMEMPERF_SERVER_PORT ddrFreqMhz %u~", ddr );
    bmemperf_set_ddrFreq( 0, ddr );

    /* if browser provided a new date/time value; this only happens once during initialization */
    if (epochSeconds)
    {
        for( memc=0; memc<numberOfMemcs; memc++ )
        {
            int bus_width = bmemperf_bus_width( memc, g_pMem );
            printf( "~MEMCDESC~MEMCDESC%u,MEMC%u %d-bit %s@%u MHz -&nbsp;~", memc, memc, bus_width, bmemperf_get_ddrType( 0, g_pMem ),
                    g_bmemconfig_box_info[boxmode].ddrFreq );
        }
    }

    for( memc=0; memc<numberOfMemcs; memc++ )
    {
        printf( "~MEMCUTIL~MEMCUTIL%u,%5.2f%%~", memc, response.response.overallStats.systemStats[memc].dataUtil );

        for( client=0; client<BMEMPERF_MAX_NUM_CLIENT; client++ )
        {
            if ( response.response.overallStats.clientOverallStats[memc].clientData[client].client_id != BMEMPERF_CGI_INVALID )
            {
                if (strncmp( g_client_name[response.response.overallStats.clientOverallStats[memc].clientData[client].client_id], "sysport_", 8) == 0 )
                {
                    sysportMemcUtil += response.response.overallStats.clientOverallStats[memc].clientData[client].bw;
                    if ( response.response.overallStats.clientOverallStats[memc].clientData[client].bw > 0 )
                    {
                        printf( "~DEBUG~%u: memc %d ... client %3d MATCHED ... BW %3d (%s) ... sysportMemcUtil (%ld)~", __LINE__, memc, client,
                                response.response.overallStats.clientOverallStats[memc].clientData[client].bw,
                                g_client_name[response.response.overallStats.clientOverallStats[memc].clientData[client].client_id], sysportMemcUtil );
                    }
                }
                /* if the client name is one of the 9 or so transport clients:
                   "xpt_wr_tuner_rs",
                   "xpt_wr_card_rs",
                   "xpt_wr_cdb",
                   "xpt_wr_itb_msg",
                   "xpt_rd_rs",
                   "xpt_rd_card_rs",
                   "xpt_rd_pb",
                   "xpt_wr_memdma",
                   "xpt_rd_memdma",
                */
                else if ( strncmp( g_client_name[response.response.overallStats.clientOverallStats[memc].clientData[client].client_id], "xpt_", 4) == 0 )
                {
                    xptMemcUtil += response.response.overallStats.clientOverallStats[memc].clientData[client].bw;
                    if ( response.response.overallStats.clientOverallStats[memc].clientData[client].bw > 0 )
                    {
                        PRINTF( "~DEBUG~%u: memc %d ... client %3d MATCHED ... BW %3d (%s) ... xptMemcUtil (%ld)~", __LINE__, memc, client,
                                response.response.overallStats.clientOverallStats[memc].clientData[client].bw,
                                g_client_name[response.response.overallStats.clientOverallStats[memc].clientData[client].client_id], xptMemcUtil );
                    }
                    if ( strcmp( g_client_name[response.response.overallStats.clientOverallStats[memc].clientData[client].client_id], MEMDMA_NAME_WR ) == 0 ||
                         strcmp( g_client_name[response.response.overallStats.clientOverallStats[memc].clientData[client].client_id], MEMDMA_NAME_RD ) == 0 )
                    {
                        memdmaMemcUtil += response.response.overallStats.clientOverallStats[memc].clientData[client].bw;
                        if ( response.response.overallStats.clientOverallStats[memc].clientData[client].bw > 0 )
                        {
                            PRINTF( "~DEBUG~%u: memc %d ... client %3d MATCHED ... BW %3d (%s) ... memdmaMemcUtil (%ld)~", __LINE__, memc, client,
                                    response.response.overallStats.clientOverallStats[memc].clientData[client].bw,
                                    g_client_name[response.response.overallStats.clientOverallStats[memc].clientData[client].client_id], memdmaMemcUtil );
                        }
                    }
                }
                /* if the client name is one of the asp clients */
                else if ( strncmp( g_client_name[response.response.overallStats.clientOverallStats[memc].clientData[client].client_id], "asp_", 4) == 0 )
                {
                    aspMemcUtil += response.response.overallStats.clientOverallStats[memc].clientData[client].bw;
                    if ( response.response.overallStats.clientOverallStats[memc].clientData[client].bw > 0 )
                    {
                        PRINTF( "~DEBUG~%u: memc %d ... client %3d MATCHED ... BW %3d (%s) ... aspMemcUtil (%ld)~", __LINE__, memc, client,
                                response.response.overallStats.clientOverallStats[memc].clientData[client].bw,
                                g_client_name[response.response.overallStats.clientOverallStats[memc].clientData[client].client_id], aspMemcUtil );
                    }
                }
                /* if the client name is one of the cpu clients */
                else if ( strstr( g_client_name[response.response.overallStats.clientOverallStats[memc].clientData[client].client_id], "_cpu_mcp_") != NULL )
                {
                    cpuMemcUtil += response.response.overallStats.clientOverallStats[memc].clientData[client].bw;
                    if ( response.response.overallStats.clientOverallStats[memc].clientData[client].bw > 0 )
                    {
                        PRINTF( "~DEBUG~%u: memc %d ... client %3d MATCHED ... BW %3d (%s) ... cpuMemc (%5.2f)~", __LINE__, memc, client,
                                response.response.overallStats.clientOverallStats[memc].clientData[client].bw,
                                g_client_name[response.response.overallStats.clientOverallStats[memc].clientData[client].client_id], cpuMemcUtil );
                    }
                }
            }
        }
    }

    bmemperf_init();

    printf("~MEMDMA_MEMC~Util: %5.2f%%<br>Mbps: %6.1f~", bmemperf_convert_bw_to_utilization( 0, memdmaMemcUtil ), memdmaMemcUtil/1.0 );

    cpuIrqs = response.response.overallStats.irqData.irqTotal;
    valueStr[0] = 0;
    convert_to_string_with_commas( cpuIrqs, valueStr, sizeof( valueStr ));
    for (cpu = 0; cpu<numProcessorsConfigured; cpu++)
    {
        /* 255 indicates this CPU is offline */
        if ( response.response.overallStats.cpuData.idlePercentage[cpu] != 255)
        {
            cpuAvg += response.response.overallStats.cpuData.idlePercentage[cpu];
        }
    }
    cpuAvg /= numProcessorsOnline;
    printf( "~CPUMEMCUTIL~Util:%5.2f%%<br>Mbps: %6.1f~CPULOAD~CPU Avg:%5.1f%%~CPUIRQS~%ld~",
            bmemperf_convert_bw_to_utilization( 0, cpuMemcUtil ), cpuMemcUtil/1.0, cpuAvg, cpuIrqs);
    PRINTF( "~DEBUG~cpuMemcUtil %5.2f BW converted to Utilization = %5.2f~", cpuMemcUtil/1.0,
            bmemperf_convert_bw_to_utilization( 0, cpuMemcUtil/1.0 ) );
#ifdef __arm__
    printf( "~CPUFREQ~ARM CPU - %d MHz~", get_cpu_frequency( 0 ) );
#endif
#ifdef __mips__
    printf( "~CPUFREQ~MIPS CPU - %d MHz~", get_cpu_frequency( 0 ) );
#endif

    printf("~SYSPORT_UTIL~%5.1f~SYSPORT_MBPS~%6.1f~", bmemperf_convert_bw_to_utilization( 0, sysportMemcUtil ), sysportMemcUtil/1.0 );
    PRINTF( "~DEBUG~sysportMemcUtil %5.2f BW converted to Utilization = %6.1f~", sysportMemcUtil/1.0,
            bmemperf_convert_bw_to_utilization( 0, sysportMemcUtil/1.0 ) );


    /* if the user is wanting power probe voltage/current */
    if ( strlen(PowerProbeIpAddr) && strlen(PowerProbeShunts) )
    {
        for(idx=0; idx<POWER_PROBE_MAX; idx++)
        {
            if ( idx < 3)
            {
                /*printf("~DEBUG~Probe %d: V %5.3f ... C %5.3f~", idx, response.response.overallStats.PowerProbeVoltage[idx],
                    response.response.overallStats.PowerProbeCurrent[idx] );*/
                printf( "~POWERPROBE~%d,%5.3f~", idx,
                    (response.response.overallStats.PowerProbeVoltage[idx] * response.response.overallStats.PowerProbeCurrent[idx]) );
            }
        }
    }

    printf( "~ClientStreamerThreadCount~%ld~", response.response.overallStats.ClientStreamerThreadCount );

    PRINTF( "~DEBUG~Sending request (0x%x) to port (%u) ... cmdSecondary 0x%x ... option 0x%lx ~", BMEMPERF_CMD_GET_CPU_IRQ_INFO,
            BSYSPERF_SERVER_PORT, request.cmdSecondary, request.cmdSecondaryOption );
    server.sin_port   = htons( BSYSPERF_SERVER_PORT );
    rc = send_request_read_response( &server, (unsigned char*) &request, sizeof(request), (unsigned char*) &response,
            sizeof(response), BSYSPERF_SERVER_PORT, BMEMPERF_CMD_GET_CPU_IRQ_INFO );
    if (rc < 0)
    {
        PRINTF( "error sending BMEMPERF_CMD_GET_CPU_IRQ_INFO request; rc %d \n", rc );
        printf( "~FATAL~ERROR connecting to server. Make sure bsysperf_server is running.~" );
        return( -1 );
    }
    PRINTF( "~DEBUG~Received from server (%d): cmd (%d)~", BSYSPERF_SERVER_PORT, response.cmd );
    {
        int               irq       = response.response.cpuIrqData.irqData.irqTotal;
        int               delta     = 0;

        for (irq = 0; irq < BMEMPERF_IRQ_MAX_TYPES; irq++) {
            /* if the entry has a valid IRQ name */
            if (strlen( response.response.cpuIrqData.irqData.irqDetails[irq].irqName ) &&
                strstr( response.response.cpuIrqData.irqData.irqDetails[irq].irqName, "GIC 92") )
            {
                /* determine if something on the row is non-zero */
                for (cpu = 0; cpu < response.response.cpuIrqData.cpuData.numActiveCpus; cpu++) {
                    delta = response.response.cpuIrqData.irqData.irqDetails[irq].irqCount[cpu] -
                            response.response.cpuIrqData.irqData.irqDetails[irq].irqCountPrev[cpu];
                    /*if ( delta  > 0)*/
                    {
                        sysportIrqs += delta;
                    }
                }
                break;
            }
        }
    }
    printf("~SYSPORT_IRQ~IRQS: %ld~", sysportIrqs );

    Bsysperf_GetXptData( &xptDetails );
    printf( "~XPTRAVEPACKETCOUNT~%ld~", xptDetails.xptRavePacketCount );

    printf( "~XPTRATE~" );
    for( idx=0; idx<XPT_BITRATE_NUM; idx++)
    {
        if ( idx ) printf( "," ); /* comma separator */
        if ( xptDetails.xptPid[idx] == 0 )
        {
            printf( "0" );
        }
        else
        {
            printf( "0x%lx %ld", xptDetails.xptPid[idx], xptDetails.xptPktCount[idx] );
            xptMbpsTotal += xptDetails.xptPktCount[idx];
        }
    }

    Bsysperf_GetAspData( &aspDetails );

    printf( "~ASPRATE~" );
    for( idx=0; idx<XPT_BITRATE_NUM; idx++)
    {
        if ( idx ) printf( "," ); /* comma separator */
        if ( aspDetails.aspPktCount[idx] == 0 )
        {
            printf( "0" );
        }
        else
        {
            printf( "%ld", aspDetails.aspPktCount[idx] );
            aspMbpsTotal += aspDetails.aspPktCount[idx];
            aspChanCount++;
        }
    }

    if ( aspChanCount > 0 )
    {
        printf( "~ACTIVE_BLOCK~ASP~" );
        /*printf( "~DEBUG~ACTIVE_BLOCK ASP ... sec %ld~", tv1.tv_sec );*/
    }
    else
    {
        printf( "~ACTIVE_BLOCK~SYSTEM_PORT~" );
        /*printf( "~DEBUG~ACTIVE_BLOCK SYSTEM_PORT ... sec %ld~", tv1.tv_sec );*/
    }

    if ( strlen( CfgNewValueName ) && strlen( CfgNewValueContents ) )
    {
        Bmemperf_SetCfgFileEntry( BASPMON_CFG_FILENAME, CfgNewValueName, CfgNewValueContents );
    }

    printf( "~STBTIME~%s~", DayMonDateYear( 0 ));

    gettimeofday( &tv2, NULL);
    microseconds2 = (tv2.tv_sec * 1000000LL); /* q-scale shift the seconds left to allow for addition of microseconds */
    microseconds2 += tv2.tv_usec; /* add in microseconds */
    microseconds_delta = (microseconds2 - microseconds1);
    printf( "~DEBUG~elapsed time %lu milliseconds", microseconds_delta/1000 );

    printf( "~ALLDONE~" );

    return( 0 );
}                                                          /* main */
