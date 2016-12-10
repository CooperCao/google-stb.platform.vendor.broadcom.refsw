/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
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
#include <signal.h>
#include <errno.h>
#include "bstd.h"
#include "bmemperf_server.h"
#include "bmemperf_cgi.h"
#include "bmemperf_utils.h"
#include "bmemperf_info.h"
#include "nexus_platform.h"

#define MAXHOSTNAME       80
#define CONTENT_TYPE_HTML "Content-type: text/html\n\n"

int                  gGotSigInt = 0;
extern char         *g_client_name[BMEMPERF_MAX_NUM_CLIENT];
extern bmemperf_info g_bmemperf_info;
int                  g_MegaBytes           = 0; /* set to 1 when user wants data displayed in megabytes instead of megabits (default) */
int                  g_MegaBytesDivisor[2] = {1, 8};
char                *g_MegaBytesStr[2] = {"Mbps", "MBps"};
char                *boxmodeSourceStr[BOXMODE_SOURCES_NUM] = {"B_REFSW_BOXMODE", BOXMODE_RTS_FILE, "BROWSER"};

typedef struct
{
    char clientName[MAX_CLIENT_NAME_SIZE];
} bmemperf_clientName;

static void signalHandler(
    int signal
    )
{
    printf( "Got signal (%d): Cleaning up!!!\n", signal );
    gGotSigInt = 1;
}

/**
 *  Function: This function is the main entry point for the BMEMPERF client app.
 **/
int main(
    int   argc,
    char *argv[]
    )
{
    int                 rc     = 0;
    int                 memc   = 0;
    int                 client = 0;
    char               *pos    = NULL;
    struct sockaddr_in  server;
    struct hostent     *hp = NULL;
    char                ThisHost[80];
    bmemperf_request    request;
    bmemperf_response   response1, response2;
    bmemperf_clientInfo clientInfo; /* ids of the 10 clients for which the user can request details */
    int                 clientCount[NEXUS_NUM_MEMC];
    unsigned int        client_id   = 0;
    unsigned int        client_memc = 0;
    int                 boxmode     = 99999;

#define BOXMODE_DROPDOWN_SIZE 2048
    char buf[BOXMODE_DROPDOWN_SIZE];

    char                    ServerIpAddr[INET6_ADDRSTRLEN];
    unsigned int            top10Number = BMEMPERF_MAX_NUM_CLIENT;
    unsigned long int       numBytes    = 0;
    bmemperf_version_info   versionInfo;
    bmemperf_boxmode_source boxmodeSource;

    FILE *fpCapture = NULL;
    char  sCaptureFilename[TEMP_FILE_FULL_PATH_LEN];
    char  lCaptureFilename[TEMP_FILE_FULL_PATH_LEN]; /* the capture file name without the temp directory name */
    char *queryString   = NULL;
    char  action[20];
    char  datetime[20];

    BSTD_UNUSED( argc );

    signal( SIGINT, signalHandler );
    signal( SIGTERM, signalHandler );
    memset( action, 0, sizeof( action ));
    memset( datetime, 0, sizeof( datetime ));
    memset( sCaptureFilename, 0, sizeof( sCaptureFilename ));
    memset( lCaptureFilename, 0, sizeof( lCaptureFilename ));
    memset( &clientInfo, BMEMPERF_CGI_INVALID, sizeof( clientInfo ));
    memset( &versionInfo, 0, sizeof( versionInfo ));
    memset( &response1, 0, sizeof( response1 ));
    memset( &response2, 0, sizeof( response2 ));
    memset( &boxmodeSource, 0, sizeof( boxmodeSource ));

    queryString   = getenv( "QUERY_STRING" );

    /* you cannot send the content type string before checking for sendFileToBrower() */

    if (queryString && strlen( queryString ))
    {
        scanForStr( queryString, "action=", sizeof( action ), action );
        scanForStr( queryString, "datetime=", sizeof( datetime ), datetime );
    }
    else
    {
        printf( CONTENT_TYPE_HTML );
        printf( "~ERROR: QUERY_STRING is not defined~\n" );
        return( -1 );
    }

    scanForStr( queryString, "capturefile=", sizeof( lCaptureFilename ), lCaptureFilename );
    /* if the user provided a capture file, prepend the temp directory path to the file name */
    if (strlen( lCaptureFilename ))
    {
        PRINTF( "~%s: calling PrependTempDirectory(); lCaptureFile (%s)\n~", __FUNCTION__, lCaptureFilename );
        PrependTempDirectory( sCaptureFilename, sizeof( sCaptureFilename ), lCaptureFilename );
    }
    PRINTF( "~%s: lCapture (%s); sCapture (%s)\n~", __FUNCTION__, lCaptureFilename, sCaptureFilename );

    /* this is a special request to upload the state file. once uploaded, we need to exit. */
    if (queryString && strstr( queryString, "variable=fetchfile" ))
    {
        char sZipFile[TEMP_FILE_FULL_PATH_LEN];

        changeFileExtension( sZipFile, sizeof( sZipFile ), sCaptureFilename, ".dat", ".tgz" );
        sendFileToBrowser( sZipFile );
        return( 0 );
    }

    printf( CONTENT_TYPE_HTML );
    fflush( stdout );
    fflush( stderr );
    printf( "~queryString (%s)\n~", queryString );

    get_boxmode( &boxmodeSource );

    if (queryString)
    {
        scanForInt( queryString, "boxmode=", &boxmode );

        /* if the browser did not request a specific box mode, use the bolt rts value */
        if (boxmode <= 0)
        {
            boxmode = boxmodeSource.boxmode;
        }

        if (boxmode != 99999)
        {
            getBoxModeHtml( buf, sizeof( buf ), boxmode );
            printf( "~BOXMODEHTML~<span> Using client names for: </span>%s~", buf );
        }
    }

    printf( "~BOXMODE~%d~", boxmode );
    bmemperf_boxmode_init( boxmode );

    pos = queryString;
    if (pos && ( pos = strstr( pos, "variable=client" )))
    {
        unsigned int enableDisable = 0;

        while (pos && ( pos = strstr( pos, "variable=client" )))
        {
            pos += strlen( "variable=client" );
            sscanf( pos, "%u", &client_id );
            /*printf( "~detected client_id (%u)~\n", client_id );*/
            pos = strstr( pos, "memc" );
            if (pos)
            {
                pos += strlen( "memc" );
                sscanf( pos, "%u", &client_memc );
                /* look for separator that tells us enable (1) or disable (0) */
                enableDisable = 0;
                pos           = strstr( pos, ":" );
                if (pos)
                {
                    pos++;
                    sscanf( pos, "%u", &enableDisable );
                }
                if (client_memc < NEXUS_NUM_MEMC)
                {
                    if (clientCount[client_memc] < BMEMPERF_MAX_SUPPORTED_CLIENTS)
                    {
                        clientInfo.clientIds[client_memc][clientCount[client_memc]] = enableDisable<<8 | client_id; /* shift enable flag to upper byte */
                        printf( "~detected client_id (%u); memc %u; %s (0x%x)\n", client_id, client_memc, ( enableDisable ) ? "ENABLE" : "DISABLE",
                            clientInfo.clientIds[client_memc][clientCount[client_memc]] );
                        clientCount[client_memc]++;
                    }
                }
            }
        }
    }

    printf( "%s: 1 capture file (%s)\n", __FUNCTION__, sCaptureFilename );

    strncpy( versionInfo.platform, getPlatform(), sizeof( versionInfo.platform )-1 );
    strncpy( versionInfo.platVersion, getPlatformVersion(), sizeof( versionInfo.platVersion )-1 );
    versionInfo.majorVersion   = MAJOR_VERSION;
    versionInfo.minorVersion   = MINOR_VERSION;
    versionInfo.sizeOfResponse = sizeof( response1 );
    printf( "~PLATFORM~%s~", versionInfo.platform );
    printf( "~PLATVER~%s~", versionInfo.platVersion );
    printf( "~STBTIME~%s~", DayMonDateYear( 0 ));
    printf( "~VERSION~Ver: %u.%u~", versionInfo.majorVersion, versionInfo.minorVersion );
    printf( "~NUMMEMC~%u~", NEXUS_NUM_MEMC );

    /* if browser provided a new date/time value */
    if (strlen( datetime ))
    {
        char cmd[64];
        sprintf( cmd, "export TZ=\"GST-8\" && date %s > /dev/null", datetime );
        system( cmd );
    }

    printf( "%s: action (%s)\n", __FUNCTION__, action );
    if (( strcmp( action, "start" ) == 0 ) ||  ( strcmp( action, "getstats" ) == 0 ) ||  ( strcmp( action, "setdetails" ) == 0 ) ||
        ( strcmp( action, "stop" ) == 0 ))
    {
        printf( "%s: detected action %s\n", __FUNCTION__, action );

        strncpy( ThisHost, "localhost", sizeof( ThisHost ));
        getservbyname( "echo", "tcp" );

        strncpy( ThisHost, "localhost", sizeof( ThisHost ));

        printf( "TCP/Client running at HOST (%s)\n", ThisHost );
        if (( hp = gethostbyname2( ThisHost, AF_INET )) == NULL)
        {
            fprintf( stderr, "Can't find host %s\n", "localhost" );
            exit( -1 );
        }
        bcopy( hp->h_addr, &( server.sin_addr ), hp->h_length );
        printf( "TCP/Client INET ADDRESS is: (%s)\n", inet_ntoa( server.sin_addr ));

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

        /* skip fopen if we are just updating client details */
        if (( strcmp( action, "start" ) == 0 ) ||  ( strcmp( action, "getstats" ) == 0 ))
        {
            fpCapture = fopen( sCaptureFilename, "a" );

            if (fpCapture == NULL)
            {
                printf( "%s: could not open capture file (%s); Error (%s); exiting.\n", __FUNCTION__, sCaptureFilename, strerror( errno ));
                return( -1 );
            }
        }

        for (memc = 0; memc<NEXUS_NUM_MEMC; memc++)
        {
            for (client = 0; client<BMEMPERF_MAX_SUPPORTED_CLIENTS; client++)
            {
                request.request.client_stats_data.client_list[memc][client] = clientInfo.clientIds[memc][client];
                if (clientInfo.clientIds[memc][client] != BMEMPERF_CGI_INVALID)
                {
                    printf( "details: sending request for memc = %u ,client_id %u; enable %x (0x%x)\n", memc,
                        request.request.client_stats_data.client_list[memc][client]&0xFF,
                        request.request.client_stats_data.client_list[memc][client]&0x100, request.request.client_stats_data.client_list[memc][client] );
                }
            }
        }
        PRINTF( "\n" );

        request.boxmode = boxmode;

        printf( "~send_request_read_response with boxmode (%d)\n~", boxmode );
        rc = send_request_read_response( &server, (unsigned char*) &request, sizeof(request),
                (unsigned char*) &response1, sizeof(response1), BMEMPERF_SERVER_PORT, BMEMPERF_CMD_GET_OVERALL_STATS );
        if (rc < 0)
        {
            perror( "error sending request " );
            return( -1 );
        }
        printf( "Received from server: cmd (%d)\n", response1.cmd );

        /* skip fwrite if we are just updating client details */
        if (( strcmp( action, "start" ) == 0 ) ||  ( strcmp( action, "getstats" ) == 0 ))
        {
            if (strcmp( action, "start" ) == 0)
            {
                memcpy( &response1, &versionInfo, sizeof( versionInfo ));
                numBytes = fwrite( &response1, 1, sizeof( response1 ), fpCapture );
                printf( "\n\nfwrite version %lu bytes 11111\n\n\n", numBytes );
                memset( &response1, 0, sizeof( response1 ));
            }
            response1.timestamp = getSecondsSinceEpoch();
            numBytes            = fwrite( &response1, 1, sizeof( response1 ), fpCapture );
            printf( "fwrite response1 %lu bytes 22222\n", numBytes );
        }

        for (memc = 0; memc<NEXUS_NUM_MEMC; memc++)
        {
            memset( &request.request.client_stats_data.client_list[memc][0], BMEMPERF_INVALID, ( sizeof( unsigned int )*BMEMPERF_MAX_SUPPORTED_CLIENTS ));
        }

        rc = send_request_read_response( &server, (unsigned char*) &request, sizeof(request),
                (unsigned char*) &response2, sizeof(response1), BMEMPERF_SERVER_PORT, BMEMPERF_CMD_GET_CLIENT_STATS );

        printf( "Received from server: cmd (%d)\n", response2.cmd );

        /* skip fwrite if we are just updating client details */
        if (( strcmp( action, "start" ) == 0 ) ||  ( strcmp( action, "getstats" ) == 0 ))
        {
            response2.timestamp = getSecondsSinceEpoch();
            numBytes            = fwrite( &response2, 1, sizeof( response2 ), fpCapture );
            printf( "fwrite response2 %lu bytes\n", numBytes );
        }

        if (fpCapture)
        {
            printf( "%s: closing (%s)\n", argv[0], sCaptureFilename );
            fclose( fpCapture );
        }

        printf( "%s: 2 capture file (%s)\n", __FUNCTION__, sCaptureFilename );

        output_file_size_in_human( sCaptureFilename );
    }

    if (( strcmp( action, "init" ) == 0 ) || ( strcmp( action, "stop" ) == 0 ))
    {
        printf( "%s: detected action %s\n", __FUNCTION__, action );
        clientListCheckboxes( top10Number );
    }

    if (strcmp( action, "stop" ) == 0)
    {
        int   rc = 0;
        char  sZipFile[TEMP_FILE_FULL_PATH_LEN];
        char  cmd[TEMP_FILE_FULL_PATH_LEN * 2 + 20 ];
        char *pos = NULL;

        memset( sZipFile, 0, sizeof( sZipFile ));
        strncpy( sZipFile, lCaptureFilename, sizeof( sZipFile ));
        printf( "~%s: 1 sZipFile (%s); lCaptureFilename (%s)\n~", __FUNCTION__, sZipFile, lCaptureFilename );

        changeFileExtension( sZipFile, sizeof( sZipFile ), lCaptureFilename, ".dat", ".tgz" );

        /* do not include the beginning directory name in the tgz file */
        pos = strstr( sCaptureFilename, "bmemperf_capture_" );
        printf( "~%s: 2 sZipFile (%s); sCaptureFilename (%s); pos %p\n~", __FUNCTION__, sZipFile, sCaptureFilename, pos );
        if (pos)
        {
            snprintf( cmd, sizeof( cmd ), "(cd %s && tar -cvzf %s %s)", GetTempDirectoryStr(), sZipFile, pos );
            printf( "~%s: issuing cmd (%s); len %u\n~", __FUNCTION__, cmd, (unsigned int) strlen( cmd ));
            rc = system( cmd );

            if (rc == -1)
            {
                printf( "~ALERT~system command (%s) failed~", cmd );
            }
            else
            {
                printf( "~TGZSUCCESSFUL~" );
            }
        }
    }

    printf( "~ALLDONE~\n" );

    return( 0 );
} /* main */
