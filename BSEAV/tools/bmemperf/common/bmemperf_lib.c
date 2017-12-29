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
#include <fcntl.h>
#include <locale.h>
#include <math.h>
#include <sys/stat.h>
#include <errno.h>
#include <termios.h>
#include <stdint.h>
#include <assert.h>
#include <sys/mman.h>
#include <pthread.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <ifaddrs.h>


#include "bmemperf.h"
#include "bmemperf_cgi.h"
#include "bmemperf_info.h"
#include "bmemperf_utils.h"

#include "bstd.h"
#include "bkni.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_xpt_full_pid_parser.h"
#ifdef    ASP_SUPPORT
#include "bchp_xpt_rave.h"
#include "bchp_xpt_mcpb.h"
#include "bchp_xpt_mcpb_ch0.h"
#include "bchp_xpt_mcpb_ch1.h"
#ifndef BCHP_XPT_MEMDMA_MCPB_CH1_DMA_DESC_CONTROL
#include "bchp_xpt_memdma_mcpb_ch1.h"
#endif
#ifndef BCHP_XPT_MEMDMA_MCPB_CH1_DMA_DESC_CONTROL
#include "bxpt_dma_helper.h"
#endif
#include "bchp_asp_mcpb_ch0.h"
#include "bchp_asp_mcpb_ch1.h"
#include "bchp_asp_mcpb.h"
#endif
#if BCHP_MEMC_DDR23_SHIM_ADDR_CNTL_0_REG_START
#include "bchp_memc_ddr23_shim_addr_cntl_0.h"
#endif

#ifdef BMEMCONFIG_BOXMODE_SUPPORTED
#include "bmemperf_lib.h"
#include "boxmodes_defines.h"
extern bmemconfig_box_info g_bmemconfig_box_info[BMEMCONFIG_MAX_BOXMODES];
#endif /* BMEMCONFIG_BOXMODE_SUPPORTED*/

#define PRINTFLOG noprintf
#define FILENAME_TAG "filename=\""
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__) /* shorter version of __FILE__ */
static bmemperf_cpu_percent g_cpuData;

const char *noprintf(
    const char *format,
    ...
    )
{
    return( format );
}

int gethostbyaddr2(
    const char *HostName,
    int         port,
    char       *HostAddr,
    int         HostAddrLen
    )
{
    char            portStr[8];
    struct addrinfo hints, *res, *p;
    int             status = 0;

    if (HostAddr == NULL)
    {
        fprintf( stderr, "ERROR - HostAddr is null\n" );
        return( 2 );
    }

    memset( &portStr, 0, sizeof portStr );
    memset( &hints, 0, sizeof hints );
    hints.ai_family   = AF_UNSPEC;                         /* AF_INET or AF_INET6 to force version */
    hints.ai_socktype = SOCK_STREAM;

    sprintf( portStr, "%u", port );
    if (( status = getaddrinfo( HostName, portStr, &hints, &res )) != 0)
    {
        fprintf( stderr, "ERROR - getaddrinfo: %s\n", gai_strerror( status ));
        return( 2 );
    }

    for (p = res; p != NULL; p = p->ai_next)
    {
        void *addr;

        PRINTF( "family is %u; AF_INET is %u\n", p->ai_family, AF_INET );
        if (p->ai_family == AF_INET)
        {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &( ipv4->sin_addr );

            /* convert the IP to a string and print it: */
            inet_ntop( p->ai_family, addr, HostAddr, HostAddrLen );

            PRINTF( "Hostname: %s; IP Address: %s\n", HostName, HostAddr );
            break;
        }
        else
        {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &( ipv6->sin6_addr );
        }
    }

    return( 0 );
}                                                          /* gethostbyaddr2 */

int scanForInt(
    const char *queryRequest,
    const char *formatStr,
    int        *returnValue
    )
{
    char  newstr[32];
    char *pos = strstr( queryRequest, formatStr );

    /*printf("looking for (%s) in (%s); pos is %p<br>\n", formatStr, queryRequest, pos );*/
    if (pos)
    {
        strncpy( newstr, formatStr, sizeof( newstr ));
        strncat( newstr, "%d", sizeof( newstr ));
        sscanf( pos, newstr, returnValue );
        PRINTF( "%s is %d\n", formatStr, *returnValue );
    }
    return( 0 );
}

int scanForStr(
    const char  *queryRequest,
    const char  *formatStr,
    unsigned int returnValuelen,
    char        *returnValue
    )
{
    char *pos = strstr( queryRequest, formatStr );

    /*printf( "looking for (%s) in (%s); pos is (%s)<br>\n", formatStr, queryRequest, pos );*/
    if (pos)
    {
        char *delimiter = NULL;

        delimiter = strstr( pos+strlen( formatStr ), "&" );
        if (delimiter)
        {
            unsigned int len = delimiter - pos - strlen( formatStr );
            PRINTF( "delimiter (%s)\n", delimiter );
            strncpy( returnValue, pos+strlen( formatStr ), len );
            returnValue[len] = '\0';
        }
        else
        {
            strncpy( returnValue, pos+strlen( formatStr ), returnValuelen );
        }
        PRINTF( "%s is (%s)\n", formatStr, returnValue );
    }
    return( 0 );
}                                                          /* scanForStr */

/**
 *  Function: This function will return a string that contains the current date and time.
 **/
char *DateYyyyMmDdHhMmSs(
    void
    )
{
    static char    fmt [64];
    struct timeval tv;
    struct tm     *tm;

    memset( fmt, 0, sizeof( fmt ));
    gettimeofday( &tv, NULL );
    if (( tm = localtime( &tv.tv_sec )) != NULL)
    {
        strftime( fmt, sizeof fmt, "%Y%m%d-%H%M%S", tm );
    }

    /*printf("%s: returning (%s)\n", __FUNCTION__, fmt);*/
    return( fmt );
}

char *HhMmSs(
    unsigned long int timestamp
    )
{
    static char    fmt[64];
    struct timeval tv;
    struct tm     *tm;

    memset( fmt, 0, sizeof( fmt ));

    if (timestamp == 0)
    {
        gettimeofday( &tv, NULL );
    }
    else
    {
        tv.tv_sec = timestamp;
    }

    if (( tm = localtime( &tv.tv_sec )) != NULL)
    {
        strftime( fmt, sizeof fmt, "%H:%M:%S", tm );
    }

    return( fmt );
}                                                          /* HhMmSs */

char *HhMmSsMsec(
    unsigned long int timestamp
    )
{
    static char    fmt[64];
    struct timeval tv;
    struct tm     *tm;

    memset( fmt, 0, sizeof( fmt ));

    if (timestamp == 0)
    {
        gettimeofday( &tv, NULL );
    }
    else
    {
        tv.tv_sec = timestamp;
    }

    if (( tm = localtime( &tv.tv_sec )) != NULL)
    {
        char msec[8];
        sprintf( msec, ".%06d", (int) (tv.tv_usec) );
        strftime( fmt, sizeof fmt, "%H:%M:%S", tm );
        strcat( fmt, msec );
    }

    return( fmt );
}                                                          /* HhMmSsMsec */

char *DayMonDateYear(
    unsigned long int timestamp
    )
{
    static char    fmt[64];
    struct timeval tv;
    struct tm     *tm;

    memset( fmt, 0, sizeof( fmt ));

    if (timestamp == 0)
    {
        gettimeofday( &tv, NULL );
    }
    else
    {
        tv.tv_sec = timestamp;
    }

    if (( tm = localtime( &tv.tv_sec )) != NULL)
    {
        strftime( fmt, sizeof fmt, "%a %b %d, %Y %H:%M:%S", tm );
    }

    return( fmt );
}                                                          /* DayMonDateYear */

/**
 *  Function: This function will convert a number to its string value while adding commas to separate the
 *  thousands, millions, billions, etc.
 **/
char *getPlatformVersion(
    void
    )
{
    char              major = (char)( BCHP_VER>>16 ) + 'A';
    unsigned long int minor = ( BCHP_VER>>12 )&0xF;
    static char       version[8];

    memset( version, 0, sizeof( version ));
    /* D0 is 0x30000 */
    sprintf( version, "%c%lx", major, minor );
    return( version );
}

char *getPlatform(
    void
    )
{
    static char platform[8];

    memset( platform, 0, sizeof( platform ));
    sprintf( platform, "%u", NEXUS_PLATFORM );
    return( platform );
}                                                          /* getPlatform */

int Close(
    int socketFd
    )
{
    PRINTF( "%s: socket %d\n", __FUNCTION__, socketFd );
    if (socketFd>0) {close( socketFd ); }
    return( 0 );
}

int send_request_read_response(
    struct sockaddr_in *server,
    unsigned char      *request,
    int                 request_len,
    unsigned char      *response,
    int                 response_len,
    int                 server_port,
    int                 cmd
    )
{
    int                rc = 0;
    int                sd = 0;
    struct sockaddr_in from;
    int                fromlen;

    PRINTF( "%s: reqlen %d; reslen %d; server_port %d\n", __FUNCTION__, request_len, response_len, server_port );
    /*   Create socket on which to send and receive */
    sd = socket( AF_INET, SOCK_STREAM, 0 );

    if (sd<0)
    {
        printf( "%s: ERROR creating socket\n", __FUNCTION__ );
        return( -1 );
    }

    /* Connect to server */
    PRINTF( "%s: connect(sock %u; port %u)\n", __FUNCTION__, sd, server_port );
    if (connect( sd, (struct sockaddr *) server, sizeof( *server )) < 0)
    {
        Close( sd );
        printf( "%s: ERROR connecting to server\n", __FUNCTION__ );
        return( -1 );
    }

    fromlen = sizeof( from );
    if (getpeername( sd, (struct sockaddr *)&from, (unsigned int *) &fromlen )<0)
    {
        Close( sd );
        printf( "%s: ERROR could not get peername\n", __FUNCTION__ );
        return( -1 );
    }

    PRINTF( "Connected to TCPServer1: " );
    PRINTF( "%s:%d\n", inet_ntoa( from.sin_addr ), ntohs( from.sin_port ));
    #if 0
    struct hostent *hp = NULL;
    if (( hp = gethostbyaddr((char *) &from.sin_addr.s_addr, sizeof( from.sin_addr.s_addr ), AF_INET )) == NULL)
    {
        fprintf( stderr, "Can't find host %s\n", inet_ntoa( from.sin_addr ));
    }
    else
    {
        PRINTF( "(Name is : %s)\n", hp->h_name );
    }
    #endif /* if 0 */

    memcpy( request, &cmd, sizeof( cmd ));

    PRINTF( "Sending request cmd (%d) to server; sizeof(request) %u\n", cmd, request_len );
    if (send( sd, request, request_len, 0 ) < 0)
    {
        printf( "%s: failed to send cmd %u to socket %u\n", __FUNCTION__, cmd, sd );
        return( -1 );
    }

    PRINTF( "Reading response from server; sizeof(response) %u\n", response_len );
    if (( rc = recv( sd, response, response_len, 0 )) < 0)
    {
        printf( "%s: failed to recv cmd %u from socket %u\n", __FUNCTION__, cmd, sd );
        return( -1 );
    }

    Close( sd );
    return( 0 );
}                                                          /* send_request_read_response */

int convert_to_string_with_commas(
    unsigned long int value,
    char             *valueStr,
    unsigned int      valueStrLen
    )
{
    unsigned long int numCommas = 0;
    unsigned long int numDigits = 0;
    char             *newString = NULL;
    unsigned long int newLen    = 0;

    numCommas = log10( value )/3;
    numDigits = trunc( log10( value ))+1;

    newLen    = numDigits + numCommas + 1 /* null terminator */;
    newString = (char *) malloc( newLen );
    if (newString)
    {
        unsigned int comma      = 0;
        unsigned int groupStart = 0;

        memset( newString, 0, newLen );

        sprintf( newString, "%lu", value );
        /*printf("%s: newString (%s); len %lu; space %lu\n", __FUNCTION__, newString, strlen(newString), newLen );*/

        /* 999999999 -> 999,999,999 */
        for (comma = 1; comma<=numCommas; comma++) {
            groupStart = numDigits - ( 3*comma );
            /*           1 ...           1111 */ /* 8,9,10 -> 11,12,13 */ /* 5,6,7 -> 7,8,9 */ /* 2,3,4 -> 3,4,5 */
            /* 01234567890 ... 01234567890123 */
            /* 11222333444 ... 11 222 333 444 */
            /*printf("%s: [%lu] <- [%u]\n", __FUNCTION__, groupStart + 2 + numCommas- (comma - 1) , groupStart + 2);*/
            newString[groupStart + 2 + numCommas - ( comma - 1 )] = newString[groupStart + 2];
            /*printf("%s: [%lu] <- [%u]\n", __FUNCTION__, groupStart + 1 + numCommas- (comma - 1) , groupStart + 1);*/
            newString[groupStart + 1 + numCommas - ( comma - 1 )] = newString[groupStart + 1];
            /*printf("%s: [%lu] <- [%u]\n", __FUNCTION__, groupStart + 0 + numCommas- (comma - 1) , groupStart + 0);*/
            newString[groupStart + 0 + numCommas - ( comma - 1 )] = newString[groupStart + 0];
            /*printf("%s: [%lu] <- ','\n", __FUNCTION__, groupStart - 1 + numCommas- (comma - 1)  );*/
            newString[groupStart - 1 + numCommas - ( comma - 1 )] = ',';
            /*printf("%s: comma %u; groupStart %u; (%s)\n", __FUNCTION__, comma, groupStart, newString);*/
        }

        if (valueStr && ( valueStrLen>1 ))
        {
            strncpy( valueStr, newString, valueStrLen -1 );
        }
        /*printf("%s", newString );*/

        Bsysperf_Free( newString );
    }

    return( 0 );
}                                                          /* convert_to_string_with_commas */

unsigned long int getSecondsSinceEpoch(
    void
    )
{
    struct timeval tv;

    gettimeofday( &tv, NULL );

    return( tv.tv_sec );
}

static float gUptime = 0.0;

int getUptime(
    float *uptime
    )
{
    if (uptime == NULL)
    {
        return( 0 );
    }

    *uptime = gUptime;

    return( 0 );
}                                                          /* getUptime */

int setUptime(
    void
    )
{
    size_t numBytes     = 0;
    FILE  *fpProcUptime = NULL;
    char   bufProcStat[256];

    fpProcUptime = fopen( "/proc/uptime", "r" );
    if (fpProcUptime==NULL)
    {
        printf( "could not open /proc/uptime\n" );
        return( -1 );
    }

    numBytes = fread( bufProcStat, 1, sizeof( bufProcStat ), fpProcUptime );
    fclose( fpProcUptime );

    if (numBytes)
    {
        sscanf( bufProcStat, "%f.2", &gUptime );
#ifdef  BMEMPERF_CGI_DEBUG
        {
            static float uptimePrev = 0;
            printf( "uptime: %8.2f (delta %8.2f)\n", gUptime, ( gUptime - uptimePrev ));
            uptimePrev = gUptime;
        }
#endif /* ifdef  BMEMPERF_CGI_DEBUG */
    }
    return( 0 );
}                                                          /* setUptime */

char *GetFileContents(
    const char *filename
    )
{
    char       *contents = NULL;
    FILE       *fpInput  = NULL;
    struct stat statbuf;

    if (lstat( filename, &statbuf ) == -1)
    {
        /*printf( "%s: Could not stat (%s)\n", __FUNCTION__, filename );*/
        return( NULL );
    }

    contents = malloc( statbuf.st_size + 1 );
    if (contents == NULL) {return( NULL ); }

    memset( contents, 0, statbuf.st_size + 1 );

    if (statbuf.st_size)
    {
        fpInput = fopen( filename, "r" );
        fread( contents, 1, statbuf.st_size, fpInput );
        fclose( fpInput );
    }

    return( contents );
}                                                          /* GetFileContents */

/**
 *  Function: This function will read in the contents of the specified file without looking
 *            at the file size first. When reading some files in the /proc file system,
 *            the lstat() API always returns a zero length. Some of the files in the /proc
 *            file system still have contents that can be read ... even if the length is 0.
 **/
char *GetFileContentsProc(
    const char *filename,
    int         max_expected_file_size
    )
{
    char       *contents = NULL;
    FILE       *fpInput  = NULL;

    contents = malloc( max_expected_file_size + 1 );
    if (contents == NULL) {return( NULL ); }

    memset( contents, 0, max_expected_file_size + 1 );

    fpInput = fopen( filename, "r" );
    fread( contents, 1, max_expected_file_size, fpInput );
    fclose( fpInput );

    return( contents );
}                                                          /* GetFileContentsProc */

char *GetFileContentsSeek(
    const char *filename,
    unsigned int offset
    )
{
    unsigned int remaining = 0;
    char       *contents = NULL;
    FILE       *fpInput  = NULL;
    struct stat statbuf;

    if (lstat( filename, &statbuf ) == -1)
    {
        /*printf( "%s: Could not stat (%s)\n", __FUNCTION__, filename );*/
        return( NULL );
    }

    if ( offset >= statbuf.st_size ) {
        return( NULL );
    }

    remaining = statbuf.st_size - offset;
    printf("%s: remaining %d; size %d\n", __FUNCTION__, remaining, (int) statbuf.st_size );
    contents = malloc( remaining + 1 );
    if (contents == NULL) {return( NULL ); }

    memset( contents, 0, remaining + 1 );

    if ( remaining )
    {
        fpInput = fopen( filename, "r" );
        fseek( fpInput, offset, SEEK_SET );
        fread( contents, 1, remaining, fpInput );
        fclose( fpInput );
    }

    return( contents );
}                                                          /* GetFileContents */

unsigned int GetFileLength(
    const char *filename
    )
{
    struct stat statbuf;

    if (lstat( filename, &statbuf ) == -1)
    {
        /*printf( "%s: Could not stat (%s)\n", __FUNCTION__, filename );*/
        return( 0 );
    }

    return( statbuf.st_size );
}                                                          /* GetFileLength */

/**
 *  Function: This function will return to the user the known temporary path name. In Linux system, this
 *  file will be /tmp/. In Android systems, it will be dictated by the environment variable B_ANDROID_TEMP.
 **/
char *GetTempDirectoryStr(
    void
    )
{
    static char tempDirectory[TEMP_FILE_FULL_PATH_LEN] = "empty";
    char       *contents     = NULL;
    char       *posErrorLog  = NULL;
    char       *posLastSlash = NULL;
    char       *posEol       = NULL;

    PRINTF( "~%s: tempDirectory (%s)\n~", __FUNCTION__, tempDirectory );
    /* if the boa.conf file has no yet been scanned for the temporary directory, do it now */
    if (strncmp( tempDirectory, "empty", 5 ) == 0)
    {
        contents = GetFileContents( "boa.conf" );

        /* if the contents of boa.conf were successfully read */
        if (contents)
        {
            posErrorLog = strstr( contents, "\nErrorLog " );
            PRINTF( "~%s: posErrorLog (%p)\n~", __FUNCTION__, posErrorLog );
            if (posErrorLog != NULL)
            {
                posErrorLog += strlen( "\nErrorLog " );
                /* look for the end of the ErrorLog line */
                posEol = strstr( posErrorLog, "\n" );

                PRINTF( "~%s: posErrorLog (%p); posEol (%p)\n~", __FUNCTION__, posErrorLog, posEol );
                /* if end of ErrorLog line found */
                if (posEol)
                {
                    posEol[0] = '\0';                      /* terminate the end of the line so that the strrchr() call works just on this line */

                    posLastSlash = strrchr( posErrorLog, '/' );
                    PRINTF( "~%s: posLastSlash (%p)(%s)\n~", __FUNCTION__, posLastSlash, posErrorLog );
                }
            }
            else
            {
                PRINTF( "~ALERT~%s: could not find ErrorLog line in boa.conf\n~", __FUNCTION__ );
            }
        }
    }

    /* if the last forward slash was found on the ErrorLog line */
    if (posErrorLog && posLastSlash)
    {
        posLastSlash[1] = '\0';
        PRINTF( "~%s: detected temp directory in boa.conf of (%s)\n~", __FUNCTION__, posErrorLog );
        strncpy( tempDirectory, posErrorLog, sizeof( tempDirectory ) -1 );
    }
    /* if the temp directory is already set to something previously */
    else if (strncmp( tempDirectory, "empty", 5 ) != 0)
    {
        /* use the previous setting */
    }
    else
    {
        strncpy( tempDirectory, "/tmp/", sizeof( tempDirectory ) -1 );
        PRINTF( "~%s: using default temp directory of (%s)\n~", __FUNCTION__, tempDirectory );
    }

    Bsysperf_Free( contents );

    PRINTF( "~%s: returning (%s)\n~", __FUNCTION__, tempDirectory );
    return( tempDirectory );
}                                                          /* GetTempDirectoryStrg */

int get_proc_stat_info(
    bmemperf_proc_stat_info *pProcStatInfo
    )
{
    unsigned long int irqTotal = 0;
    FILE             *fpStats  = NULL;
    char              buf[1024];
    char             *pos = NULL;

    if (pProcStatInfo == NULL)
    {
        return( -1 );                                      /* invalid parameter */
    }

    fpStats = fopen( PROC_STAT_FILE, "r" );
    if (fpStats != NULL)
    {
        while (fgets( buf, sizeof( buf ), fpStats ))
        {
            pos = strchr( buf, '\n' );
            if (pos)
            {
                *pos = 0;                                  /* null-terminate the line */
            }
            if (strncmp( buf, "intr ", 5 ) == 0)
            {
                /* just read the first number. it contains the total number of interrupts since boot */
                sscanf( buf + 5, "%lu", &( pProcStatInfo->irqTotal ));
                PRINTF( "%s: irqTotal %lu\n", __FUNCTION__, pProcStatInfo->irqTotal );
            }
            else if (strncmp( buf, "ctxt ", 5 ) == 0)
            {
                sscanf( buf + 5, "%lu", &( pProcStatInfo->contextSwitches ));
                PRINTF( "%s: contextSwitches %lu\n", __FUNCTION__, pProcStatInfo->contextSwitches );
                break;                                     /* after finding context switches, do not need to look any further in the file */
            }
        }

        fclose( fpStats );
    }
    else
    {
        fprintf( stderr, "Could not open %s\n", PROC_STAT_FILE );
    }
    return( irqTotal );
}                                                          /* get_interrupt_total */

int get_interrupt_counts(
    bmemperf_irq_data *irqData
    )
{
    struct stat       statbuf;
    char             *contents      = NULL;
    size_t            numBytes      = 0;
    FILE             *fpText        = NULL;
    char             *posstart      = NULL;
    char             *posend        = NULL;
    unsigned long int lineNum       = 0;
    unsigned long int cpu           = 0;
    unsigned int      numProcessors = sysconf( _SC_NPROCESSORS_ONLN );
    unsigned int      numCpusActive = 0;
    unsigned int      numLines      = 0;
    char              cmd[64+TEMP_FILE_FULL_PATH_LEN];
    char              tempFilename[TEMP_FILE_FULL_PATH_LEN];

    PrependTempDirectory( tempFilename, sizeof( tempFilename ), TEMP_INTERRUPTS_FILE );

    setlocale( LC_NUMERIC, "" );

    memset( &statbuf, 0, sizeof( statbuf ));

    /* create a shell command to concatinate the cumuulative interrupt counts file to the temp directory */
    sprintf( cmd, "cat %s > %s", PROC_INTERRUPTS_FILE, tempFilename );
    system( cmd );

    if (lstat( tempFilename, &statbuf ) == -1)
    {
        printf( "%s - (%s); lstat failed; %s\n", __FUNCTION__, tempFilename, strerror( errno ));
        return( -1 );
    }

    /*printf("size of (%s) %lu\n", tempFilename, statbuf.st_size);*/

    if (statbuf.st_size == 0)
    {
        printf( "could not determine interrupts file (%s) size.\n", tempFilename );
        return( -1 );
    }

    contents = (char *) malloc( statbuf.st_size + 1 );
    if (contents == NULL)
    {
        printf( "could not malloc(%lu+1) bytes\n", (unsigned long int) statbuf.st_size );
        return( -1 );
    }

    memset( contents, 0, statbuf.st_size + 1 );

    fpText = fopen( tempFilename, "r" );
    if (fpText == NULL)
    {
        printf( "could not fopen(%s)\n", tempFilename );
        return( -1 );
    }
    numBytes = fread( contents, 1, statbuf.st_size, fpText );

    fclose( fpText );

    /* we are done with the temp file; delete it. */
    remove( tempFilename );

    if (numBytes != (unsigned int) statbuf.st_size)
    {
        printf( "tried to fread %lu bytes but got %lu\n", (unsigned long int) statbuf.st_size, (unsigned long int) numBytes );
        return( -1 );
    }

    posstart = contents;                                   /* start parsing the file at the beginning of the file */

    /* step through the file line by line, searching for interrupt counts for each CPU */
    do {
        posend = strstr( posstart, "\n" );
        if (posend)
        {
            *posend = '\0';
            posend++;
        }
        /*printf("next line (%s); posend %p\n", posstart, posend );*/

        numLines++;

        if (lineNum == 0)
        {
            char *cp, *restOfLine;
            restOfLine = posstart;

            /*printf("numProcessors %u\n", numProcessors);*/
            while (( cp = strstr( restOfLine, "CPU" )) != NULL && numCpusActive < numProcessors)
            {
                cpu = strtoul( cp + 3, &restOfLine, BMEMPERF_IRQ_VALUE_LENGTH );
                numCpusActive++;
            }
            /*printf("found %u cpus in header; numProcessors %u\n", numCpusActive, numProcessors );*/
        }
        else if (strlen( posstart ))
        {
            char        *cp         = NULL;
            char        *restOfLine = NULL;
            char        *pos        = NULL;
            unsigned int irqIdx     = lineNum - 1;         /* the first line has the header on it */

            if (irqIdx < BMEMPERF_IRQ_MAX_TYPES)
            {
                /* Skip over "IRQNAME:" */
                cp = strchr( posstart, ':' );
                if (!cp)
                {
                    continue;
                }

                cp++;

                pos  = cp;
                pos += numCpusActive * ( BMEMPERF_IRQ_VALUE_LENGTH + 1 ); /* each number is 10 digits long separated by 1 space */
                pos += 2;                                                 /* after all of the numbers are listed, the name is separated by 2 more spaces */

                /* some names have a few spaces at the beginning of them; advance the pointer past all of the beginning spaces */
                while (*pos == ' ')
                {
                    pos++;
                }

                /* the line is long enough to have a name at the end of it */
                if (pos < ( cp + strlen( cp )))
                {
                    strncpy( irqData->irqDetails[irqIdx].irqName, pos, sizeof( irqData->irqDetails[irqIdx].irqName ));
                    PRINTF( "%s: added new irq to idx %2u (%s)\n", __FUNCTION__, irqIdx, pos );
                }

                PRINTF( "line %3u: (%s) ", numLines, cp );
                for (cpu = 0; cpu < numCpusActive; cpu++)
                {
                    unsigned long int value = 0;

                    /* parse the next value from the current line */
                    value = strtoul( cp, &restOfLine, BMEMPERF_IRQ_VALUE_LENGTH );

                    /* add interrupt count to the running value for this CPU */
                    irqData->irqCount[cpu] += value;

                    /* save the value for this specific IRQ */
                    irqData->irqDetails[irqIdx].irqCount[cpu] = value;
                    PRINTF( "%lu ", value );

                    cp = restOfLine;
                    if (cp == NULL)
                    {
                        break;
                    }
                }
                PRINTF( "\n" );
            }
            else
            {
                fprintf( stderr, "%s: irqIdx (%u) exceeded BMEMPERF_IRQ_MAX_TYPES\n", __FUNCTION__, irqIdx );
            }
        }

        posstart = posend;

        lineNum++;
    } while (posstart != NULL);

    Bsysperf_Free( contents );

    /* add up all of the cpu's interrupts into one big total */
    for (cpu = 0; cpu < numCpusActive; cpu++)
    {
        irqData->irqTotal += irqData->irqCount[cpu];
    }
    PRINTF( "after %u lines, total %lu\n\n\n", numLines, irqData->irqTotal );

    return( 0 );
}                                                          /* get_interrupt_counts */

/**
 *  Function: This function will prepend the specified file name with the known temporary path name.
 **/
void PrependTempDirectory(
    char       *filenamePath,
    int         filenamePathLen,
    const char *filename
    )
{
    if (filenamePath)
    {
        strncpy( filenamePath, GetTempDirectoryStr(), filenamePathLen -1 );
        strncat( filenamePath, filename,              filenamePathLen -1 );

        PRINTF( "~%s: returning (%s)\n~", __FUNCTION__, filenamePath );
    }

    return;
}                                                          /* PrependTempDirectory */

int set_cpu_utilization(
    void
    )
{
    bmemperf_getCpuUtilization( &g_cpuData );

#if 0
    PRINTF( "%s: detected BMEMPERF_CMD_GET_CPU_IRQ_INFO (%u); numCpus %u\n", __FUNCTION__, pResponse->cmd,
        pResponse->response.cpuIrqData.cpuData.numActiveCpus );
    bmemperf_computeIrqData( pResponse->response.cpuIrqData.cpuData.numActiveCpus, &pResponse->response.cpuIrqData.irqData );
#endif

    return( 0 );
}

int get_cpu_utilization(
    bmemperf_cpu_percent *pcpuData
    )
{
    if (pcpuData != NULL)
    {
        memcpy( pcpuData, &g_cpuData, sizeof( bmemperf_cpu_percent ));
    }

    return( 0 );
}

/**
    This public function will return the CPU utilization numbers that are calculated once a second in
    a separate 1 hertz thread (dataFetchThread).
**/
int bmemperf_getCpuUtilization(
    bmemperf_cpu_percent *pCpuData
    )
{
    if (pCpuData)
    {
        PRINTF( "%s: memcpy() %u bytes; numActiveCpus %u\n", __FUNCTION__, sizeof( *pCpuData ), g_cpuData.numActiveCpus );
        memcpy( pCpuData, &g_cpuData, sizeof( *pCpuData ));
    }

    return( 0 );
}                                                          /* bmemperf_getCpuUtilization */

/**
    This private function will compute the CPU utilization for each active CPU in the system. The percentage ranges
    from 0 to 100. This is a private function that gets called once a second in the dataFetchThread thread.
    It uses two files: (1) /proc/uptime and (2) /proc/stat

    External users should call the Bmemperf_getCpuUtilization function.
**/
int P_getCpuUtilization(
    void
    )
{
    FILE                *fpProcStat = NULL;
    char                 bufProcStat[1024];
    int                  numCpusConf = 0;
    unsigned long int    clk         = 0;
    unsigned char        cpu         = 0;
    char                *pos         = NULL;
    char                 cpuTag[6];
    float                cpuPercent = 0.0;
    bmemperf_cpu_percent gCpuDataNow;

    memset( &bufProcStat, 0, sizeof( bufProcStat ));
    memset( &gCpuDataNow, 0, sizeof( gCpuDataNow ));

    clk         = sysconf( _SC_CLK_TCK );
    numCpusConf = sysconf( _SC_NPROCESSORS_CONF );
    if (numCpusConf > BMEMPERF_MAX_NUM_CPUS)
    {
        numCpusConf = BMEMPERF_MAX_NUM_CPUS;
    }

#ifdef  BMEMPERF_CGI_DEBUG
    printf( "configured %u; clk_tck %lu\n", numCpusConf, clk );
#else
    BSTD_UNUSED( clk );
#endif

    fpProcStat = fopen( "/proc/stat", "r" );
    if (fpProcStat == NULL)
    {
        printf( "could not open /proc/stat\n" );
    }
    else
    {
        getUptime( &gCpuDataNow.uptime );

        fread( bufProcStat, 1, sizeof( bufProcStat ), fpProcStat );

        fclose( fpProcStat );

#ifdef  BMEMPERF_CGI_DEBUG
        {
            char *posIntr = NULL;
            printf( "%s: uptime now (%8.2f); prev (%8.2f); delta (%8.2f)", __FUNCTION__, gCpuDataNow.uptime, g_cpuData.uptime,
                gCpuDataNow.uptime - g_cpuData.uptime );
            /* look for line that starts with "intr" */
            posIntr = strstr( bufProcStat, "intr" );       /* DEBUG */
            if (posIntr)
            {
                *posIntr = '\0';
                printf( "; /proc/stat\n(%s)", bufProcStat );
                *posIntr = 'i';
            }
            printf( "\n" );                                /* DEBUG */
        }
#endif /* ifdef  BMEMPERF_CGI_DEBUG */

        pos = bufProcStat;

        g_cpuData.numActiveCpus = sysconf( _SC_NPROCESSORS_ONLN ); /* will be lower than SC_NPROCESSORS_CONF if some CPUs are disabled */

        for (cpu = 0; cpu<numCpusConf; cpu++)
        {
            cpuPercent = 0.0;

            sprintf( cpuTag, "cpu%u ", cpu );
            pos = strstr( pos, cpuTag );
            if (pos)
            {
                pos += strlen( cpuTag );
                /*
                    This is the sscanf that mpstat.c uses:
                    &cp->cpu_user, &cp->cpu_nice, &cp->cpu_system, &cp->cpu_idle, &cp->cpu_iowait, &cp->cpu_irq, &cp->cpu_softirq, &cp->cpu_steal, &cp->cpu_guest
                */
                sscanf( pos, "%lu %lu %lu %lu %lu %lu %lu %lu ",
                    &gCpuDataNow.user[cpu], &gCpuDataNow.nice[cpu], &gCpuDataNow.system[cpu], &gCpuDataNow.idle[cpu],
                    &gCpuDataNow.cpu_iowait[cpu], &gCpuDataNow.cpu_irq[cpu], &gCpuDataNow.cpu_softirq[cpu], &gCpuDataNow.cpu_steal[cpu] );
                if (gCpuDataNow.uptime > g_cpuData.uptime)
                {
                    /* compute the cpu percentage based on the current numbers minus the previous numbers */
                    cpuPercent = ( gCpuDataNow.user[cpu] - g_cpuData.user[cpu] +
                                   gCpuDataNow.system[cpu] - g_cpuData.system[cpu] +
                                   gCpuDataNow.nice[cpu] - g_cpuData.nice[cpu] +
                                   gCpuDataNow.cpu_iowait[cpu] - g_cpuData.cpu_iowait[cpu] +
                                   gCpuDataNow.cpu_irq[cpu] - g_cpuData.cpu_irq[cpu] +
                                   gCpuDataNow.cpu_softirq[cpu] - g_cpuData.cpu_softirq[cpu] +
                                   gCpuDataNow.cpu_steal[cpu] - g_cpuData.cpu_steal[cpu]  ) /
                        ( gCpuDataNow.uptime-g_cpuData.uptime ) + .5 /* round */;
                    #if 0
                    printf( "%s: cpu %u: (%lu - %lu + %lu - %lu + %lu - %lu + %lu - %lu + %lu - %lu + %lu - %lu + %lu - %lu ) / (%8.2f) + .5 = %6.2f;\n",
                        __FUNCTION__, cpu,
                        gCpuDataNow.user[cpu], g_cpuData.user[cpu],
                        gCpuDataNow.system[cpu], g_cpuData.system[cpu],
                        gCpuDataNow.nice[cpu], g_cpuData.nice[cpu],
                        gCpuDataNow.cpu_iowait[cpu], g_cpuData.cpu_iowait[cpu],
                        gCpuDataNow.cpu_irq[cpu], g_cpuData.cpu_irq[cpu],
                        gCpuDataNow.cpu_softirq[cpu], g_cpuData.cpu_softirq[cpu],
                        gCpuDataNow.cpu_steal[cpu], g_cpuData.cpu_steal[cpu],
                        gCpuDataNow.uptime-g_cpuData.uptime, cpuPercent );
                     #endif /* if 0 */
                }
                if (cpuPercent > 100.0)
                {
                    cpuPercent = 100.0;
                }
                if (cpuPercent < 0.0)
                {
                    cpuPercent = 0.0;
                }

                /* save the current data to the global structure to use during the next pass */
                gCpuDataNow.idlePercentage[cpu] = g_cpuData.idlePercentage[cpu] = (unsigned char) cpuPercent;
#ifdef  BMEMPERF_CGI_DEBUG
                printf( "%3u ", gCpuDataNow.idlePercentage[cpu] );
#endif

                g_cpuData.user[cpu]        = gCpuDataNow.user[cpu];
                g_cpuData.nice[cpu]        = gCpuDataNow.nice[cpu];
                g_cpuData.system[cpu]      = gCpuDataNow.system[cpu];
                g_cpuData.cpu_iowait[cpu]  = gCpuDataNow.cpu_iowait[cpu];
                g_cpuData.cpu_irq[cpu]     = gCpuDataNow.cpu_irq[cpu];
                g_cpuData.cpu_softirq[cpu] = gCpuDataNow.cpu_softirq[cpu];
                g_cpuData.cpu_steal[cpu]   = gCpuDataNow.cpu_steal[cpu];
            }
            else
            {
#ifdef  BMEMPERF_CGI_DEBUG
                printf( "cpu %u is offline\n", cpu );
#endif
                gCpuDataNow.idlePercentage[cpu] = g_cpuData.idlePercentage[cpu] = 255;
                pos = bufProcStat;                         /* pos is NULL at this point; reset it to start searching back at the beginning of the stat buffer */
            }
        }
#ifdef  BMEMPERF_CGI_DEBUG
        printf( "\n" );
#endif

        g_cpuData.uptime = gCpuDataNow.uptime;
    }

    return( 0 );
}                                                          /* P_getCpuUtilization */

/**
 *  Function: This function will copy the tail end of the /tmp/boa_error.log file to
 *  send the error messages back to the browser to let the user know as much about
 *  run-time errors as possible.
 **/
char *bmemperf_get_boa_error_log(
    const char *appname
    )
{
    char *contents     = NULL;
    char *contentsTail = NULL;
    char  tagLine[64];
    char  tempFilename[TEMP_FILE_FULL_PATH_LEN];

    PrependTempDirectory( tempFilename, sizeof( tempFilename ), "boa_error.log" );

    printf( "%s: appname (%s); boa_error (%s)\n", __FUNCTION__, appname, tempFilename );
    contents = GetFileContents( tempFilename );

    if (contents)
    {
        char *pos              = (char *) appname;
        char *prevAppNameFound = NULL;
        char  appnameNoExtention[32];

        if (( appname[0] == '.' ) && ( appname[1] == '/' ))
        {
            pos = (char *) &appname[2];
        }
        strncpy( appnameNoExtention, pos, sizeof( appnameNoExtention ));

        /* determine if the appname has an extention to it */
        pos = strchr( appnameNoExtention, '.' );
        if (pos)
        {
            *pos = 0;                                      /* we do not want the appname to include the extention */
        }
        sprintf( tagLine, "%s: argc ", appnameNoExtention );
        /*printf("%s: tagLine is (%s)\n", __FUNCTION__, tagLine );*/
        /* search through the file to find the LAST mention of the appname */

        pos = contents;

        /* look for a line like this: ### 00:00:00.000 bboxreport: argc 1; */
        do
        {
            pos = strstr( pos, tagLine );
            if (pos)
            {
                prevAppNameFound = pos;
                pos++;
            }
            /*printf("%s: bottom while ... pos (%p); prev (%p)\n", __FUNCTION__, pos, prevAppNameFound );*/
        } while (pos);

        /* if we found the last mention of appname */
        if (prevAppNameFound)
        {
            if (strlen( prevAppNameFound ))
            {
                int len = strlen( prevAppNameFound ) + 20; /* we want space to back up 17 characters to include the BDBG time */
                /* allocate enough space to hold the tail end of the log file */
                contentsTail = (char *) malloc( len );

                if (contentsTail)
                {
                    prevAppNameFound -= 17;
                    strncpy( contentsTail, prevAppNameFound, len );
                }
            }
        }
        else
        {
            contentsTail = (char *) malloc( 32 );

            if (contentsTail)
            {
                sprintf( contentsTail, "No error details found." );
            }
        }

        Bsysperf_Free( contents );
    }

    return( contentsTail );
} /* bmemperf_get_boa_error_log */

/**
 *  Function: This function will return a count of the number of carriage returns found in the specified string.
 **/
unsigned int bmemperf_get_boa_error_log_line_count(
    const char *errorLogContents
    )
{
    unsigned int line_count = 0;
    char        *pos        = NULL;

    pos = (char *) errorLogContents;                       /* start looking for carriage returns at the beginning of the contents */
    do
    {
        line_count++;
        pos = strchr( ++pos, '\n' );
    } while (pos);

    return( line_count );
}

static volatile unsigned int *g_pMem  = NULL;

/**
 *  Function: This function will read the specified register offset. If it hasn't been done prior to this call,
 *  the function will also open the driver and mmap to the register space.
 **/
unsigned int bmemperf_readReg32( unsigned int offset )
{
    volatile unsigned int *pMemTemp = NULL;
    unsigned int           returnValue = 0;

    if (g_pMem == NULL)
    {
        g_pMem = (volatile unsigned int*) bmemperf_openDriver_and_mmap();
    }

    pMemTemp  = (unsigned int *) g_pMem;
    /*printf("\n~DEBUG~%s:%u g_pMem %p; offset 0x%x; ~", __FILENAME__, __LINE__, (void*) g_pMem, offset );
    fflush(stdout);fflush(stderr);*/
    if (BCHP_REGISTER_START & offset)
    {
        offset -= BCHP_REGISTER_START;
        /*PRINTFLOG( "%s:%u - (offset 0x%x); g_pMem %p \n", __FILENAME__, __LINE__, offset, g_pMem );*/
    }
    /* some chips (74371, 7271, 7344, 7364, 7250, 7260, 7586, 7268, 7366) have a non-zero base address; if one of these, subtract the base offset */
    else if (BCHP_REGISTER_START && offset)
    {
        /*PRINTFLOG( "%s:%u - (offset 0x%x); g_pMem %p \n", __FILENAME__, __LINE__, offset, g_pMem );*/
        offset -= BCHP_REGISTER_START;
    }
    pMemTemp += offset >>2;
    /*printf("%s:%u g_pMem %p; offset 0x%x; BCHP_REGISTER_START 0x%x;   pMemTemp 0x%x \n", __FILENAME__, __LINE__,
            (void*) g_pMem, offset, BCHP_REGISTER_START, (unsigned int) pMemTemp );
    fflush(stdout);fflush(stderr);*/

    returnValue = *pMemTemp;
    /*printf("\n~DEBUG~%s:%u returnValue 0x%x ~", __FILENAME__, __LINE__, (unsigned int ) returnValue );
    fflush(stdout);fflush(stderr);*/

    return ( returnValue );
} /* bmemperf_readReg32 */

/**
 *  Function: This function will write the specified register offset. If it hasn't been done prior to this call,
 *  the function will also open the driver and mmap to the register space.
 **/
unsigned int bmemperf_writeReg32( unsigned int offset, unsigned int new_value )
{
    volatile unsigned int *pMemTemp = NULL;

    if (g_pMem == NULL)
    {
        g_pMem = (volatile unsigned int*) bmemperf_openDriver_and_mmap();
    }

    pMemTemp  = (unsigned int *) g_pMem;
    /*printf("\n~DEBUG~%s:%u g_pMem %p; offset 0x%x; ~", __FILENAME__, __LINE__, (void*) g_pMem, offset );
    fflush(stdout);fflush(stderr);*/
    if (BCHP_REGISTER_START & offset)
    {
        offset -= BCHP_REGISTER_START;
        /*PRINTFLOG( "%s:%u - (offset 0x%x); g_pMem %p \n", __FILENAME__, __LINE__, offset, g_pMem );*/
    }
    /* some chips (74371, 7271, 7344, 7364, 7250, 7260, 7586, 7268, 7366) have a non-zero base address; if one of these, subtract the base offset */
    else if (BCHP_REGISTER_START && offset)
    {
        /*PRINTFLOG( "%s:%u - (offset 0x%x); g_pMem %p \n", __FILENAME__, __LINE__, offset, g_pMem );*/
        offset -= BCHP_REGISTER_START;
    }
    pMemTemp += offset >>2;
    /*printf("\n~DEBUG~%s:%u g_pMem %p; offset 0x%x; BCHP_REGISTER_START 0x%x;   pMemTemp 0x%x ~", __FILENAME__, __LINE__,
            (void*) g_pMem, offset, BCHP_REGISTER_START, (unsigned int) pMemTemp );
    fflush(stdout);fflush(stderr);*/

    *pMemTemp = new_value;
    /*printf("\n~DEBUG~%s:%u returnValue 0x%x ~", __FILENAME__, __LINE__, (unsigned int ) returnValue );
    fflush(stdout);fflush(stderr);*/

    return ( new_value );
} /* bmemperf_writeReg32 */

#ifdef BMEMCONFIG_READ32_SUPPORTED
/* bchp_asp_mcpb_ch0.h ... #define BCHP_ASP_MCPB_CH0_DCPM_LOCAL_PACKET_COUNTER */
int Bsysperf_GetXptData( bmemperf_xpt_details *pxpt ) /* XPT Rave stats. */
{
#ifdef ASP_SUPPORT
    unsigned long int /*uint64_t*/ validOffset, readOffset, baseOffset, endOffset;
    unsigned long int /*uint64_t*/ cdbSize, cdbDepth;

    readOffset = bmemperf_readReg32( BCHP_XPT_RAVE_CX0_AV_CDB_READ_PTR);
    validOffset = bmemperf_readReg32( BCHP_XPT_RAVE_CX0_AV_CDB_VALID_PTR);
    baseOffset = bmemperf_readReg32( BCHP_XPT_RAVE_CX0_AV_CDB_BASE_PTR);
    endOffset = bmemperf_readReg32( BCHP_XPT_RAVE_CX0_AV_CDB_END_PTR);
    cdbSize = endOffset - baseOffset;

    pxpt->xptRavePacketCount = bmemperf_readReg32( BCHP_XPT_RAVE_PACKET_COUNT );

    if (validOffset > readOffset)
    {
        cdbDepth = validOffset - readOffset;
    }
    else
    {
        cdbDepth = validOffset + cdbSize - readOffset;
    }

    {
        int idx = 0;
        unsigned long int mask = cdbDepth;
        unsigned long int /*uint64_t*/ xptRunStatus;
        unsigned long int pidOffset = (BCHP_XPT_MCPB_CH1_PARSER_BAND_ID_CTRL-BCHP_XPT_MCPB_CH0_PARSER_BAND_ID_CTRL) & 0xffff;
        unsigned long int pktOffset = (BCHP_XPT_MEMDMA_MCPB_CH1_DCPM_LOCAL_PACKET_COUNTER -BCHP_XPT_MEMDMA_MCPB_CH0_DCPM_LOCAL_PACKET_COUNTER) & 0xffff;
        unsigned long int outputCount = 0;
        unsigned long int totalPktCnt = 0;

        xptRunStatus = bmemperf_readReg32( BCHP_XPT_MCPB_RUN_STATUS_0_31 );
        /*fprintf( stderr, "%s - cdbDepth %lu; cdbSize %lu; xpt_mcpb_run_status bits 0x%lx;  pktOffset 0x%lx \n", __FUNCTION__, cdbDepth, cdbSize, xptRunStatus, pktOffset );*/
        for( idx=0; idx<32; idx++)
        {
            mask = 1 << idx;
            if ( xptRunStatus & mask )
            {
                unsigned long int pid = bmemperf_readReg32( BCHP_XPT_MCPB_CH0_PARSER_BAND_ID_CTRL              + (idx*pidOffset) );
                unsigned long int pkt = bmemperf_readReg32( BCHP_XPT_MEMDMA_MCPB_CH0_DCPM_LOCAL_PACKET_COUNTER + (idx*pktOffset) );

                pid = (pid & BCHP_XPT_MCPB_CH0_PARSER_BAND_ID_CTRL_PB_PARSER_BAND_ID_MASK ) >> BCHP_XPT_MCPB_CH0_PARSER_BAND_ID_CTRL_PB_PARSER_BAND_ID_SHIFT;
                pxpt->xptPid[idx] = pid;
                pxpt->xptActive[idx] = 1;
                pxpt->xptPktCount[idx] = pkt;
                if ( pkt > 0 )
                {
                    if ( outputCount == 0 ) /* first time we found a non-zero value */
                    {
                        PRINTFLOG( "%s\n", DateYyyyMmDdHhMmSs() );
                    }
                    totalPktCnt += pkt;
                    outputCount++;
                    /*fprintf( stderr, "xpt_mcpb chan %2d is active; pid (0x%lx) 0x%lx;   pktCount %ld;   addr 0x%lx;   total %ld \n", idx,
                        BCHP_XPT_MCPB_CH0_PARSER_BAND_ID_CTRL + (idx*pidOffset), pid,
                        pkt, BCHP_XPT_MEMDMA_MCPB_CH0_DCPM_LOCAL_PACKET_COUNTER + (idx*pktOffset), totalPktCnt );*/
                }
            }
        }
    }
#endif /* ASP_SUPPORT */

    return 1;
}

int Bsysperf_GetAspData( bmemperf_asp_details *pasp )
{
#ifdef ASP_SUPPORT
    int idx = 0;
    unsigned long int mask = 0;
    unsigned long int /*uint64_t*/ aspRunStatus;
    unsigned long int pktOffset = (BCHP_ASP_MCPB_CH1_DCPM_LOCAL_PACKET_COUNTER - BCHP_ASP_MCPB_CH0_DCPM_LOCAL_PACKET_COUNTER) & 0xffff;

    aspRunStatus = bmemperf_readReg32( BCHP_ASP_MCPB_RUN_STATUS_0_31 );
    for( idx=0; idx<32; idx++)
    {
        mask = 1 << idx;
        if ( aspRunStatus & mask )
        {
            unsigned long int pkt = bmemperf_readReg32( BCHP_ASP_MCPB_CH0_DCPM_LOCAL_PACKET_COUNTER + (idx*pktOffset) );
            unsigned long int outputCount = 0;
            unsigned long int totalPktCnt = 0;

            pasp->aspActive[idx] = 1;
            pasp->aspPktCount[idx] = pkt;
            if ( pkt > 0 )
            {
                if ( outputCount == 0 ) /* first time we found a non-zero value */
                {
                    PRINTFLOG( "%s\n", DateYyyyMmDdHhMmSs() );
                }
                totalPktCnt += pkt;
                PRINTFLOG( "asp_mcpb chan %2d is active; pktCount %ld;   addr 0x%lx;   total %ld \n", idx, pkt,
                    BCHP_ASP_MCPB_CH0_DCPM_LOCAL_PACKET_COUNTER + (idx*pktOffset), totalPktCnt );
            }
        }
    }
#endif /* ASP_SUPPORT */

    return 1;
}

/*
Product ID for this chip (7252 ... SUN_TOP_CTRL.html#SUN_TOP_CTRL_PRODUCT_ID)
[31: 8] chip ID. May contain either a four digit ID or a five digit chip ID. If the bits found at 31:28 are zero,
then the remaining bits from 27:8 represent a five digit chip IC. If any of the bits found at 31:28 are non-zero,
then the bits from 31:16 represent a four digit chip IC. Each group of four bits may represent any value from 0-9.
*/
/**
 *  Function: On some platforms (like 97252 which is a variant of the 97445 but with two MEMCs), there is no easy
 *  way to determine how many MEMCs the run-time platform supports. We used to be able to use NEXUS_MAX_MEMCS and
 *  NEXUS_NUM_MEMCS, but after all of the sub-variant platforms were merged into one, those values no longer worked
 *  properly. For example, on the 97252, the value NEXUS_NUM_MEMCs used to be 2. After the merge, the value was
 *  changed to 3 because that is the number of MEMCs in the 97445. Once the merge happened, I had to devise another
 *  way to determine how many MEMCs were supported on the platform. My method uses the SUN_TOP_CTRL_PRODUCT_ID
 *  register to do this.
 **/
char *getProductIdStr(
    void
    )
{
    uint32_t    productId        = 0;
    uint32_t    familyId         = 0;
    static char lProductIdStr[8] = "";

    /* if the product id string has not yet been initialized, do it now. */
    if (strlen( lProductIdStr ) == 0)
    {
        memset( &lProductIdStr, 0, sizeof( lProductIdStr ));

        g_pMem = bmemperf_openDriver_and_mmap();
        /*printf("%s:%u - g_pMem %p\n", __FILE__, __LINE__, (void*) g_pMem );fflush(stdout);fflush(stderr);*/

        if ( g_pMem )
        {
            /*printf("~DEBUG~%s:%u g_pMem %p ~", __FILE__, __LINE__, g_pMem );fflush(stdout);fflush(stderr);*/
            familyId = bmemperf_readReg32( BCHP_SUN_TOP_CTRL_CHIP_FAMILY_ID );
            productId = bmemperf_readReg32( BCHP_SUN_TOP_CTRL_PRODUCT_ID );

            PRINTF("%s:%u familyId 0x%x; productId 0x%x \n", __FILE__, __LINE__, familyId, productId );

            if (productId&0xF0000000)                          /* if upper nibble is non-zero, this value contains a 4-digit product id in bits 31:16 */
            {
                snprintf( lProductIdStr, sizeof( lProductIdStr ) - 1, "%x", productId>>16 );
            }
            else
            {
                snprintf( lProductIdStr, sizeof( lProductIdStr ) - 1, "%x", productId>>8 );
                /*printf("%s:%u lProductIdStr (%s) \n", __FILE__, __LINE__, lProductIdStr );*/
                if (strcmp( lProductIdStr, "72511" ) == 0)
                {
                    strncpy( lProductIdStr, "7251S", sizeof( lProductIdStr ) - 1 );
                }
                else if ( (strcmp( lProductIdStr, "72525" ) == 0 ) ||
                          (strcmp( lProductIdStr, "72521" ) == 0 ) ||
                          (strcmp( lProductIdStr, "74481" ) == 0 ) ||
                          (strcmp( lProductIdStr, "74491" ) == 0 ) ||
                          (strcmp( lProductIdStr, "74495" ) == 0 )
                        )
                {
                    strncpy( lProductIdStr, "7252S", sizeof( lProductIdStr ) - 1 );
                }
            }
            PRINTFLOG( "~DEBUG~productId %x (%s) ... familyId %x~", productId, lProductIdStr, familyId );
        }
        else
        {
            printf("~FATAL~could not open driver\n~");
        }
    }

    return( lProductIdStr );
} /* getProductIdStr */

unsigned long int Bmemperf_PidChannelGetPccEnable( int hwPidChannelIndex )
{
    unsigned long int pcc_error_en_reg = 0;

    if ( hwPidChannelIndex < BMEMPERF_PID_CHANNEL_MAX )
    {
        /*printf( "%s: XPT_FULL_PID 0x%x ... hwPid %d ... times 4 %d ... result 0x%x\n", __FUNCTION__,
                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_BASE, hwPidChannelIndex, hwPidChannelIndex*4,
                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_BASE + (hwPidChannelIndex*4) );*/
        pcc_error_en_reg = bmemperf_readReg32( BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_BASE + (hwPidChannelIndex*4) );
    }

    return ( pcc_error_en_reg );
}
int Bmemperf_PidChannelSetPccEnable( int hwPidChannelIndex )
{
    unsigned long int pcc_error_en_reg = 0;

    if ( hwPidChannelIndex < BMEMPERF_PID_CHANNEL_MAX )
    {
        /*printf( "%s: XPT_FULL_PID 0x%x ... hwPid %d ... times 4 %d ... result 0x%x\n", __FUNCTION__,
                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_BASE, hwPidChannelIndex, hwPidChannelIndex*4,
                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_BASE + (hwPidChannelIndex*4) );*/
        pcc_error_en_reg = bmemperf_readReg32( BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_BASE + (hwPidChannelIndex*4) );

        /* set the various bits that are needed */
        pcc_error_en_reg |= BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_PCC_ERROR_EN_MASK;
        /*pcc_error_en_reg |= BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_PROC_CC_MASK;*/

        bmemperf_writeReg32( BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_BASE + (hwPidChannelIndex*4), pcc_error_en_reg );
    }

    return 0;
}

unsigned long int Bmemperf_PidChannelGetAutoSyncDetect( int hwPidChannelIndex )
{
    unsigned long int auto_sync_reg = 0;
#ifdef ASP_SUPPORT

    if ( hwPidChannelIndex < BMEMPERF_PID_CHANNEL_MAX )
    {
        auto_sync_reg = bmemperf_readReg32( BCHP_XPT_MCPB_CH0_SP_TS_CONFIG + (hwPidChannelIndex*4) );
    }

#endif /* ASP_SUPPORT */
    return ( auto_sync_reg );
}
int Bmemperf_PidChannelSetAutoSyncDetect( int hwPidChannelIndex )
{
#ifdef ASP_SUPPORT
    unsigned long int auto_sync_reg = 0;

    if ( hwPidChannelIndex < BMEMPERF_PID_CHANNEL_MAX )
    {
        auto_sync_reg = bmemperf_readReg32( BCHP_XPT_MCPB_CH0_SP_TS_CONFIG + (hwPidChannelIndex*4) );

        /* set the various bits that are needed */
        auto_sync_reg |= BCHP_XPT_MCPB_CH0_SP_TS_CONFIG_MPEG_TS_AUTO_SYNC_DETECT_MASK;

        bmemperf_writeReg32( BCHP_XPT_MCPB_CH0_SP_TS_CONFIG + (hwPidChannelIndex*4), auto_sync_reg );
    }
#endif /* ASP_SUPPORT */

    return 0;
}
#endif /* BMEMCONFIG_READ32_SUPPORTED */

#ifdef NEXUS_MODE_proxy
#endif

/**
 *  Function: This function will open the device used for reading and writing registers.
 *            The device name is configurable by modifying the file "device_node.cfg".
 *            The Android system is doing away with access to /dev/mem because of
 *            security holes. This mechanism will allow Android to specify whatever
 *            device name they need to allow access to the register space.
 **/
int bmemperfOpenDriver( void )
{
    int   fd           = 0;
    char *contents     = NULL;
    char  device_name[32];
    char  tempFilename[TEMP_FILE_FULL_PATH_LEN];

    memset(device_name, 0, sizeof(device_name));
    memset(tempFilename, 0, sizeof(tempFilename));

    strncpy( tempFilename, "device_node.cfg", sizeof( tempFilename ) - 1 );

    /*printf( "%s: device node configuration file is (%s)\n", __FUNCTION__, tempFilename );*/
    /* attempt to open and read the contents of the configuration file (Android use) */
    contents = GetFileContents( tempFilename );

    /* if the file existed and has some contents to it */
    if (contents)
    {
        /* use the contents to override the device driver name */
        strncpy( device_name, contents, sizeof(device_name) -1 );
        Bsysperf_Free( contents );

        /* if the last character is a new-line character */
        if (strlen(device_name) && device_name[strlen(device_name)-1] == '\n' )
        {
            device_name[strlen(device_name)-1] = 0;
        }
        /*printf( "%s: read device name (%s) from configuration file \n", __FUNCTION__, device_name );*/
    }
    else
    {
        strncpy( device_name, "/dev/mem", sizeof(device_name) -1 );
    }

    fd = open( device_name, O_RDWR|O_SYNC );  /*O_SYNC for uncached address */

    /* if the open failed */
    if ( fd < 0 )
    {
        /*printf("%s: open (%s) failed (%s); \n", __FUNCTION__, device_name, strerror(errno) );*/
    }


    /*printf("%s: returning fd %d\n", __FUNCTION__, fd );*/
    return ( fd );
}

void *bmemperfMmap( int g_memFd )
{
    void *pMem = NULL;
    #if 0
    PRINTFLOG( "%s: mmap64(NULL, mapped_size 0x%x, PROT_READ %u, MAP_SHARED %u, g_memFd %u, BCHP_PYHS_OFF %x; BCHP_REG_START %x)\n",
        __FUNCTION__, ( BCHP_REGISTER_SIZE<<2 ), PROT_READ|PROT_WRITE, MAP_SHARED, g_memFd, BCHP_PHYSICAL_OFFSET, BCHP_REGISTER_START );
    #endif

    pMem = mmap64( 0, ( BCHP_REGISTER_SIZE<<2 ), PROT_READ|PROT_WRITE, MAP_SHARED, g_memFd, BCHP_PHYSICAL_OFFSET + BCHP_REGISTER_START );

    return ( pMem );
}
/**
 *  Function: This function will compute several characteristics for the specified MEMC:
 *            1) the bus width (either 16-bit bus or 32-bit bus)
 *            2) the CAS burst length (either 4 bytes, 8 bytes, or 16 bytes
 *            The results are returned to the user in the specified structure.
 **/
static BMEMPERF_BUS_BURST_INFO bus_burst_info[BMEMPERF_NUM_MEMC];
static unsigned int bmemperf_bus_burst_info( /* needs g_pMem */
    unsigned int memc_index,
    volatile unsigned int *g_pMem
    )
{
    static bool           info_retrieved[BMEMPERF_NUM_MEMC];
    volatile unsigned int arb_reg_val    = 0;
    volatile unsigned int device_type    = 0;
    volatile unsigned int *pTempReg = NULL;

    /*printf("%s: memc %u; g_pMem %p \n", __FUNCTION__, memc_index, (void*) g_pMem );
    fflush(stdout);fflush(stderr);*/
    /* if the registers have already been read, do not repeat the reads */
    if ( (memc_index >= BMEMPERF_NUM_MEMC) || (info_retrieved[memc_index]) )
    {
        return( 1 );
    }

    if (g_pMem == NULL)
    {
        g_pMem = (volatile unsigned int*) bmemperf_openDriver_and_mmap();
    }

    if (g_pMem)
    {
        info_retrieved[memc_index] = true;

#ifdef BCHP_MEMC_DDR23_SHIM_ADDR_CNTL_0_CONFIG
        /* older 40-nm chips */
        pTempReg = (volatile unsigned int *)((unsigned long int)g_pMem + BCHP_MEMC_DDR23_SHIM_ADDR_CNTL_0_CONFIG );

        /* read the current value of the register */
        arb_reg_val = device_type = *pTempReg;

        arb_reg_val = ( BCHP_MEMC_DDR23_SHIM_ADDR_CNTL_0_CONFIG_DRAM_WIDTH_MASK & arb_reg_val ) >> BCHP_MEMC_DDR23_SHIM_ADDR_CNTL_0_CONFIG_DRAM_WIDTH_SHIFT;
        /* DRAM Width.
           00: DDR32BIT
           01: DDR16BIT
           10 - reserved
           11 - reserved
        */
        device_type = ( BCHP_MEMC_DDR23_SHIM_ADDR_CNTL_0_CONFIG_DDR_MODE_MASK & arb_reg_val ) >> BCHP_MEMC_DDR23_SHIM_ADDR_CNTL_0_CONFIG_DDR_MODE_SHIFT;
        /* DDR Mode.
           0: DDR3 mode
           1: DDR2 mode
        */

        if (arb_reg_val == 1)
        {
            bus_burst_info[memc_index].interface_bit_width = 16;
        }
        else /* DRAM_WIDTH_32 ... default */
        {
            bus_burst_info[memc_index].interface_bit_width = 32;
        }

        if ( device_type == 1 /* DDR2 */ )
        {
            bus_burst_info[memc_index].burst_length = 4;
            strncpy( bus_burst_info[memc_index].ddr_type, "DDR2", sizeof( bus_burst_info[memc_index].ddr_type ) -1 );
        }
        else /* DDR3 ... default */
        {
            bus_burst_info[memc_index].burst_length = 8;
            strncpy( bus_burst_info[memc_index].ddr_type, "DDR3", sizeof( bus_burst_info[memc_index].ddr_type ) -1 );
        }
#else
        /* newer 28-nm chips */
        pTempReg = (volatile unsigned int *)((unsigned long int)g_pMem + (BCHP_MEMC_DDR_0_CNTRLR_CONFIG  - BCHP_REGISTER_START ) );

        /* read the current value of the register */
        arb_reg_val = device_type = *pTempReg;

        arb_reg_val = ( BCHP_MEMC_DDR_0_CNTRLR_CONFIG_DRAM_TOTAL_WIDTH_MASK & arb_reg_val ) >> BCHP_MEMC_DDR_0_CNTRLR_CONFIG_DRAM_TOTAL_WIDTH_SHIFT;
        device_type = ( BCHP_MEMC_DDR_0_CNTRLR_CONFIG_DRAM_DEVICE_TYPE_MASK & device_type ) >> BCHP_MEMC_DDR_0_CNTRLR_CONFIG_DRAM_DEVICE_TYPE_SHIFT;

        if (arb_reg_val == MEMC_DRAM_WIDTH_16)
        {
            bus_burst_info[memc_index].interface_bit_width = 16;
        }
        else /* MEMC_DRAM_WIDTH_32 ... default */
        {
            bus_burst_info[memc_index].interface_bit_width = 32;
        }

        if ( device_type == MEMC_DRAM_TYPE_LPDDR4 )
        {
            bus_burst_info[memc_index].burst_length = 16;
            strncpy( bus_burst_info[memc_index].ddr_type, "LPDDR4", sizeof( bus_burst_info[memc_index].ddr_type ) -1 );
        }
        else if ( device_type == MEMC_DRAM_TYPE_DDR2 )
        {
            bus_burst_info[memc_index].burst_length = 4;
            strncpy( bus_burst_info[memc_index].ddr_type, "DDR2", sizeof( bus_burst_info[memc_index].ddr_type ) -1 );
        }
        else if ( device_type == MEMC_DRAM_TYPE_DDR4 )
        {
            bus_burst_info[memc_index].burst_length = 8;
            strncpy( bus_burst_info[memc_index].ddr_type, "DDR4", sizeof( bus_burst_info[memc_index].ddr_type ) -1 );
        }
        else /* MEMC_DRAM_TYPE_DDR3 ... default */
        {
            bus_burst_info[memc_index].burst_length = 8;
            strncpy( bus_burst_info[memc_index].ddr_type, "DDR3", sizeof( bus_burst_info[memc_index].ddr_type ) -1 );
        }
#endif
#ifdef  BMEMPERF_DEBUG
        printf( "%s: arb_reg_val %x; interface_bit_width [%u] (%u); device_type %d\n", __FUNCTION__, arb_reg_val, memc_index,
                bus_burst_info[memc_index].interface_bit_width, device_type );
        printf( "%s: arb_reg_val %x; burst_length [%u] (%u)\n", __FUNCTION__, arb_reg_val, memc_index, bus_burst_info[memc_index].interface_bit_width );
#endif
    }
    else
    {
        printf( "FATAL ERROR: g_pMem %p; \n", (void *) (intptr_t) g_pMem );
        bus_burst_info[memc_index].interface_bit_width = 32;
        bus_burst_info[memc_index].burst_length = 4;
    }

    return( 0 );
}

/**
 *  Function: This function will return the width of the DDR bus: either 16 or 32 bits.
 **/
unsigned int bmemperf_bus_width(
    unsigned int memc_index,
    volatile unsigned int *g_pMem
    )
{
    /* if the index is invalid, return a commonly valid value */
    if ( memc_index >= BMEMPERF_NUM_MEMC )
    {
        return( 32 );
    }

    bmemperf_bus_burst_info( memc_index, g_pMem );

    return( bus_burst_info[memc_index].interface_bit_width );
} /* bmemperf_bus_width */

/**
 *  Function: This function will return the CAS burst length: either 4 bytes (DDR2), 8 bytes (DDR3), or 16 bytes
 *  (LPDDR4).
 **/
unsigned int bmemperf_burst_length(
    unsigned int memc_index,
    volatile unsigned int *g_pMem
    )
{
    /* if the index is invalid, return a commonly valid value */
    if ( memc_index >= BMEMPERF_NUM_MEMC )
    {
        return( 8 );
    }

    bmemperf_bus_burst_info( memc_index, g_pMem );

    return( bus_burst_info[memc_index].burst_length );
} /* bmemperf_burst_length */

/**
 *  Function: This function will return a string description of the DDR type used in the system.
 **/
char *bmemperf_get_ddrType(
    unsigned int memc_index,
    volatile unsigned int *g_pMem
    )
{
    if ( memc_index >= BMEMPERF_NUM_MEMC )
    {
        return ( "UNKNOWN" );
    }

    bmemperf_bus_burst_info( memc_index, g_pMem );

    return ( bus_burst_info[ memc_index ].ddr_type );
} /* bmemperf_get_ddrType */

/**
 *  Function: This function will convert a CAS cycle count to a normal cycle count. There is a specific way to
 *  do this using the burst length. The burst length differs between DDR2, DDR3/DDR4, and LPDDR4.
 **/
int bmemperf_cas_to_cycle(
    unsigned int memc_index,
    volatile unsigned int *g_pMem
    )
{
    return ( bmemperf_burst_length(memc_index, g_pMem ) / 2 );
}

/**
 *  Function: This function will open the memory device that will be used to read and write registers in the
 *  system. Once the memory device has been successfylly opened, this function will also memory map the memory
 *  device.
 **/
volatile unsigned int *bmemperf_openDriver_and_mmap(
    void
    )
{
    volatile unsigned int *l_pMem = NULL;
    int                    l_memFd = 0;

    /* Open driver for memory mapping */
    l_memFd = bmemperfOpenDriver();

    if (l_memFd == -1)
    {
        printf( "Failed to bmemperfOpenDriver() ... fd %d \n", l_memFd );
        return( NULL );
    }

    fcntl( l_memFd, F_SETFD, FD_CLOEXEC );

    l_pMem = bmemperfMmap( l_memFd );

    /*printf("%s: l_pMem %p\n", __FUNCTION__, (void*) l_pMem );*/
    if (!l_pMem)
    {
        printf( "Failed to bmemperfMmap() fd=%d, addr 0x%08x\n", l_memFd, BCHP_PHYSICAL_OFFSET );
        return( NULL );
    }

    return( l_pMem );
} /* bmemperf_openDriver_and_mmap */

/**
 *  Function: This function will convert from a millisecond counter to a one-second counter taking care to use
 *  64-bit arithmetic to prevent overflow of large numbers.
 **/
unsigned int convert_from_msec (
    unsigned int msec_value,
    unsigned int g_interval
    )
{
    unsigned int        temp;
    unsigned long long  ulltemp; /* the trans_read counts can be ~ 7 million; need space to multiply by 1000 */

    ulltemp = msec_value;
    ulltemp = ulltemp * 1000 / g_interval;
    temp = ulltemp;

    return ( temp );
}

/**
 *  Function: This function will use the output from the ifconfig utility to determine what our IP4 loopback address is.
 **/
unsigned int get_my_ip4_addr( void )
{
    char  line[200];
    FILE *cmd = NULL;
    struct in_addr sin_temp_addr;

    memset( &sin_temp_addr, 0, sizeof( sin_temp_addr ) );

    sprintf( line, "/sbin/ifconfig | /bin/egrep 'Link encap|inet addr' | /bin/awk '{if ( $1 == \"inet\"  && substr($2,6,3) != \"127\" ) { printf substr($2,6) \" \";} }'" );
    cmd = popen( line, "r" );

    memset( line, 0, sizeof( line ));
    fgets( line, sizeof(line), cmd );

    pclose( cmd );

    if ( strlen(line) > 1 )
    {
        int len = strlen(line);
        if ( line[len-1] == '\n') line[len-1] = 0;
    }

    inet_aton( line, &sin_temp_addr);
    /*printf("~DEBUG~%s:%u: %s (0x%lx)~", __FILE__, __LINE__, line, (unsigned long int) sin_temp_addr.s_addr );*/

    return( sin_temp_addr.s_addr );
}

/**
 *  Function: This function will use the output from the ifconfig utility to determine what our local IP4 address is.
 *            The local address is the one that starts with 192.168.
 **/
char *get_my_ip4_local( void )
{
    static char  line[200];
    FILE *cmd = NULL;
    struct in_addr sin_temp_addr;

    memset( &sin_temp_addr, 0, sizeof( sin_temp_addr ) );

    sprintf( line, "/sbin/ifconfig | grep \"192.168\" | /bin/awk '{print substr($2,6);}'" );
    cmd = popen( line, "r" );

    memset( line, 0, sizeof( line ));
    fgets( line, sizeof(line), cmd );

    pclose( cmd );

    if ( strlen(line) > 1 )
    {
        int len = strlen(line);
        if ( line[len-1] == '\n') line[len-1] = 0;
    }

    return( line );
}

unsigned long int getPidOf( const char * processName )
{
    char                line[MAX_LENGTH_GETPIDOF_CMD];
    unsigned long int   pid = 0;
    unsigned long int   temp = 0;
    FILE               *cmd = NULL;

    sprintf(line, "ps -eaf | grep \"%s\" | grep -v grep | awk '{print $2;}'", processName );
    cmd = popen( line, "r" );

    /* We could find two or three matching PIDS. Loop until we find the last one */
    do
    {
        memset(line, 0, sizeof(line) );
        fgets( line, MAX_LENGTH_GETPIDOF_CMD, cmd );
        temp = 0;
        temp = strtoul( line, NULL, 10 );
        if ( temp ) pid = temp;
    } while ( temp );

    pclose( cmd );
    return( pid );
}

const char * executable_fullpath( const char * exe_name )
{
    static char   line[MAX_LENGTH_GETPIDOF_CMD];
    FILE         *cmd = NULL;

    sprintf(line, "which %s", exe_name );
    cmd = popen( line, "r" );

    memset(line, 0, sizeof(line) );
    fgets( line, MAX_LENGTH_GETPIDOF_CMD, cmd );
    pclose( cmd );

    return( line );
}

const char * get_executing_command( const char * exe_name )
{
    static char   line[MAX_LENGTH_GETPIDOF_CMD];
    FILE         *cmd = NULL;

    sprintf(line, "ps -eaf | /bin/awk '{print $8;}' | grep \"%s\"", exe_name );
    cmd = popen( line, "r" );

    memset(line, 0, sizeof(line) );
    fgets( line, MAX_LENGTH_GETPIDOF_CMD, cmd );
    pclose( cmd );

    return( line );
}

int replace_space_with_nbsp( char *buffer, long int buffer_size )
{
    char             *token = NULL;
    int               count = 0;
    char             *new_buffer = NULL;

    if (buffer == NULL || strlen(buffer) == 0 || buffer_size == 0 )
    {
        return ( 0 );
    }

    new_buffer = (char*) malloc(buffer_size);
    if (new_buffer == NULL)
    {
        printf("%s:%u: could not malloc() %d bytes \n", __FUNCTION__, __LINE__, (int) buffer_size );
        return ( 0 );
    }

    memset(new_buffer, 0, buffer_size);
    token = strtok( buffer, " " );
    count = 0;
    while (token)
    {
        /* if we have already copied something into the output buffer, add the delimiter */
        if (count > 0)
        {
            strncat( new_buffer, "&nbsp;", buffer_size - strlen(new_buffer) -1 );
        }
        strncat( new_buffer, token, buffer_size - strlen(new_buffer) -1 );
        /*printf( "token is (%s) ... copied to (%s) \n", token, new_buffer );*/
        token = strtok( NULL, " " );
        count++;
    }

    memset(buffer, 0, buffer_size);
    strncpy(buffer, new_buffer, buffer_size );

    Bsysperf_Free(new_buffer);

    return ( 0 );
}

#define PROCESS_CMDLINE_RETURN_BUFFER_LEN 512
/**
 *  Function: This function will scan through the current list of active processes looking for the one that
 *  matches the specified search string. More than one match could occur. A buffer of matching command line
 *  matches will be returned to the user. The caller is responsible for freeing the return buffer. For example:
 *  a serch for "server" could return these two lines:
 *       root    14564 ./boa_server -c ./
 *       root    16385 ./bsysperf_server
 **/
char *Bsysperf_GetProcessCmdline( const char * process_name )
{
    int            i             = 0;
    long int       num_bytes     = 0;
    DIR           *dir           = NULL;
    struct dirent *entry         = NULL;
    FILE          *fp            = NULL;
    char          *return_buffer = NULL;
    char           cmdline_contents[512];
    char           cmdline_filename[128];

    dir = opendir("/proc");
    while ((entry = readdir(dir)) != NULL)
    {
        /* only interested in names that start with a number and are at least 3 characters long */
        if ( strlen(entry->d_name) > 2 && '1' <= entry->d_name[0] && entry->d_name[0] <= '9' )
        {
            memset(cmdline_contents, 0, sizeof(cmdline_contents) );

            /* create a full-path name to the cmdline file */
            strncpy(cmdline_filename, "/proc/", sizeof(cmdline_filename) -1 );
            strncat(cmdline_filename, entry->d_name, sizeof(cmdline_filename) - strlen(cmdline_filename) -1 );
            strncat(cmdline_filename, "/cmdline", sizeof(cmdline_filename) - strlen(cmdline_filename) -1 );

            fp = fopen( cmdline_filename, "r");
            if ( fp )
            {
                num_bytes = fread(cmdline_contents, 1, sizeof(cmdline_contents), fp );
                if ( num_bytes )
                {
                    /*printf("~DEBUG~cmdline_filename (%s) ... bytes (%ld) ~", cmdline_filename, num_bytes );*/

                    /* the cmdline file is a series of lines with no spaces; add the spaces back in */
                    for(i=0; i<num_bytes; i++ )
                    {
                        if ( cmdline_contents[i] == '\0' ) cmdline_contents[i] = ' ';
                    }
                    if ( strstr(cmdline_contents, process_name ) )
                    {
                        /* if the space for the return buffer has not been allocated yet, do it now */
                        if (return_buffer == NULL)
                        {
                            return_buffer = (char*) malloc( PROCESS_CMDLINE_RETURN_BUFFER_LEN );
                            if ( return_buffer == NULL )
                            {
                                return ( return_buffer );
                            }
                            memset(return_buffer, 0, PROCESS_CMDLINE_RETURN_BUFFER_LEN );
                        }
                        /*printf("%s: %ld: (%s) \n", cmdline_filename, num_bytes, cmdline_contents );*/

                        if ( strlen(return_buffer) )
                        {
                            strncat( return_buffer, "\n", PROCESS_CMDLINE_RETURN_BUFFER_LEN - strlen(return_buffer) -1 );
                        }
                        strncat( return_buffer, "root    ",       PROCESS_CMDLINE_RETURN_BUFFER_LEN - strlen(return_buffer) -1 );
                        strncat( return_buffer, entry->d_name,    PROCESS_CMDLINE_RETURN_BUFFER_LEN - strlen(return_buffer) -1 );
                        strncat( return_buffer, "   ",            PROCESS_CMDLINE_RETURN_BUFFER_LEN - strlen(return_buffer) -1 );
                        strncat( return_buffer, cmdline_contents, PROCESS_CMDLINE_RETURN_BUFFER_LEN - strlen(return_buffer) -1 );
                        /*printf("~DEBUG~return_buffer (%s)~", return_buffer );*/
                    }
                }
                fclose(fp);
            }
        }
    }
    closedir(dir);
    return ( return_buffer );
}

/**
 *  Function: This function will scan through the current list of active processes looking for the one that
 *            matches the specified search string. If more than one match occurs, the first one will be returned.
 *            This API is much faster than getPidOf(). The getPidOf() API typically takes 15 milliseconds per call.
 *            This API takes about 2 milliseconds per call.
 **/
int Bsysperf_GetProcessPidOf( const char * process_name )
{
    int            pid           = 0;
    long int       num_bytes     = 0;
    DIR           *dir           = NULL;
    struct dirent *entry         = NULL;
    FILE          *fp            = NULL;
    char           cmdline_contents[512];
    char           cmdline_filename[128];

    dir = opendir("/proc");
    while ((entry = readdir(dir)) != NULL)
    {
        /* only interested in names that start with a number and are at least 3 characters long */
        if ( strlen(entry->d_name) > 2 && '1' <= entry->d_name[0] && entry->d_name[0] <= '9' )
        {
            memset(cmdline_contents, 0, sizeof(cmdline_contents) );

            /* create a full-path name to the cmdline file */
            strncpy(cmdline_filename, "/proc/", sizeof(cmdline_filename) -1 );
            strncat(cmdline_filename, entry->d_name, sizeof(cmdline_filename) - strlen(cmdline_filename) -1 );
            strncat(cmdline_filename, "/cmdline", sizeof(cmdline_filename) - strlen(cmdline_filename) -1 );

            fp = fopen( cmdline_filename, "r");
            if ( fp )
            {
                num_bytes = fread(cmdline_contents, 1, sizeof(cmdline_contents), fp );
                if ( num_bytes )
                {
                    if ( strstr(cmdline_contents, process_name ) )
                    {
                        sscanf( entry->d_name, "%d", &pid );
                    }
                    /*printf("~DEBUG~cmdline_filename (%s) ... bytes (%ld) ... pid (%d) ... name (%s) ... (%s)~",
                            cmdline_filename, num_bytes, pid, entry->d_name, cmdline_contents );*/
                }
                fclose(fp);
                if ( pid ) break;
            }
        }
    }
    closedir(dir);
    return ( pid );
}

char * Bsysperf_ReplaceNewlineWithNull ( char *buffer )
{
    char * posEol = NULL;

    posEol = strchr( buffer, '\n' );
    if (posEol)
    {
        posEol[0] = '\0';  /* null-terminate the line */
    }

    return ( posEol );
}

int Bsysperf_RestoreNewline( char * posEol )
{
    if ( posEol )
    {
        posEol[0] = '\n';  /* restore the newline character */
    }

    return ( 0 );
}

/**
 *  Function: This function will convert the specified character to it's integer hexidecimal value.
 **/
static long int atoix( char c )
{
    char lchar[2];
    lchar[0] = c;
    lchar[1] = 0;
    return strtol(lchar, NULL, 16);
}

/**
 *  Function: This function will convert the specified string from the encoded version that comes from the
 *  browser to a decoded version that is a standard C string. For example "%20" gets changed to a space " "; the
 *  value "%2F" gets changed to "/".
 **/
int decodeURL ( char * URL )
{
    char c, *s=NULL, *d=NULL;

    for (d = s = URL; *s; s++, d++)
    {
        c = *s;
        if (c == '%')
        {
            c = *++s;
            if (c == '%') c = '%';
            else {
                c = atoix(c) << 4 | atoix(*++s);
            }
            *d = c;
        }
        else
        {
            *d = c;
        }
    }
    *d = '\0';

    return 0;
}

/**
 *  Function: This function will return the frequency for the specified CPU.
 *            You can use this command to list the current frequency:
 *            $ cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq
 **/
int get_cpu_frequency(
    unsigned int cpuId
    )
{
    long int      freq = 0;
    FILE         *fp  = NULL;
    struct stat   statbuf;
    char          cpuinfo_cur_freq[64];

    memset( &statbuf, 0, sizeof( statbuf ));

    sprintf( cpuinfo_cur_freq, "/sys/devices/system/cpu/cpu%u/cpufreq/cpuinfo_cur_freq", cpuId );

    PRINTF( "%s:%u - trying lstat(%s)\n", __FUNCTION__, __LINE__, cpuinfo_cur_freq );
    if ( (lstat( cpuinfo_cur_freq, &statbuf ) == -1) || (statbuf.st_size == 0) )
    {
        PRINTF( "%s:%u - lstat(%s) failed; %s\n", __FUNCTION__, __LINE__, cpuinfo_cur_freq, strerror( errno ));
        return( freq );
    }

    fp = fopen( cpuinfo_cur_freq, "r" );
    if ( fp )
    {
        int  num_bytes = 0;
        char freq_buffer[32];
        memset(freq_buffer, 0, sizeof(freq_buffer));

        num_bytes = fread(freq_buffer, 1, sizeof(freq_buffer) - 1, fp );
        PRINTF( "%s:%u - fread() returned num_bytes %d \n", __FUNCTION__, __LINE__, num_bytes );
        if ( num_bytes )
        {
            sscanf( freq_buffer, "%ld", &freq );
            PRINTF( "%s:%u - sscanf(%s) returned freq %ld \n", __FUNCTION__, __LINE__, freq_buffer, freq );
            freq /= 1000;
        }
    }

    return ( freq );
}                                                          /* get_cpu_frequency */

/**
 *  Function: This function will loop through all processors in the system
 *  and look for the CPU frequency associated each one. If at least one
 *  non-zero frequency is found, then all of the discovered frequencies will
 *  be output using the CPUFREQUENCY tag. For older versions of BOLT (pre v1.20)
 *  and kernel versions before 4.1-1.3, this CPU frequency data will not be
 *  found in the /sys file system.
 **/
int output_cpu_frequencies( void )
{
    int numCpusConf = 0;
    int cpu = 0;
    char CPUFREQUENCY_line[512];
    char cpu_freq_str[32];
    int  cpu_freq_int;
    bool cpu_freq_nonzero = false;

    memset(CPUFREQUENCY_line, 0, sizeof(CPUFREQUENCY_line));

    numCpusConf = sysconf( _SC_NPROCESSORS_CONF );
    if (numCpusConf > BMEMPERF_MAX_NUM_CPUS)
    {
        numCpusConf = BMEMPERF_MAX_NUM_CPUS;
    }

    for (cpu = 0; cpu < numCpusConf; cpu++)
    {
        cpu_freq_int = get_cpu_frequency(cpu);

        /* if at least one non-zero cpu frequency is found, remember this fact */
        if ( cpu_freq_nonzero == false && cpu_freq_int > 0 ) cpu_freq_nonzero = true;

        sprintf( cpu_freq_str, "%06u ", cpu_freq_int );
        strncat( CPUFREQUENCY_line, cpu_freq_str, sizeof(CPUFREQUENCY_line) - strlen(CPUFREQUENCY_line) - 1 );
    }

    if ( cpu_freq_nonzero )
    {
        /* output the CPU frequency data */
        printf( "~CPUFREQUENCY~%u %s~", numCpusConf, CPUFREQUENCY_line );
    }

    return ( 0 );
}

/**
 *  Function: This function will read the current scaling governor from the /sys
 *            file system. The string value will be converted to an internal
 *            enum and returned to the caller.
 *            File: /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
 **/
int get_governor_control ( int cpu )
{
    DVFS_GOVERNOR_TYPES value = DVFS_GOVERNOR_PERFORMANCE;
    char  filename[128];
    char *contents = NULL;

    memset(filename, 0, sizeof(filename));

    sprintf( filename, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_governor", cpu );
    contents = GetFileContents( filename );

    /* Hosahalli - The following command will list all the available governors. We do not want to list
       the "userspace" in your tool. First reason is we have not defined what that should be. Linux
       kernel CPUfreq framework makes this available as standard offering.
          cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors
          conservative ondemand userspace powersave performance
    */
    if ( contents )
    {
        /*printf("for filename (%s); contents (%s)\n", filename, contents );*/
        if ( strstr(contents, "conservative" ) )
        {
            value = DVFS_GOVERNOR_CONSERVATIVE;
        }
        else if ( strstr(contents, "performance" ) )
        {
            value = DVFS_GOVERNOR_PERFORMANCE;
        }
        else if ( strstr(contents, "powersave" ) )
        {
            value = DVFS_GOVERNOR_POWERSAVE;
        }
        else if ( strstr(contents, "ondemand" ) )
        {
            value = DVFS_GOVERNOR_ONDEMAND;
        }
        Bsysperf_Free( contents );
    }

    return ( value );
}

/**
 *  Function: This function will set the scaling governor value using the caller's
 *            input value. The input enum will be converted to the appropriate
 *            string value before the string is stored in the /sys file system.
 *            echo performance >/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
 **/
int set_governor_control ( int cpu, DVFS_GOVERNOR_TYPES GovernorSetting )
{
    char new_setting[32];
    char cmd[64];

    memset(new_setting, 0, sizeof(new_setting));
    memset(cmd, 0, sizeof(cmd));

    switch (GovernorSetting)
    {
        case DVFS_GOVERNOR_CONSERVATIVE:
        {
            strncpy( new_setting, "conservative", sizeof(new_setting) - 1 );
            break;
        }
        default:
        case DVFS_GOVERNOR_PERFORMANCE:
        {
            strncpy( new_setting, "performance", sizeof(new_setting) - 1 );
            break;
        }
        case DVFS_GOVERNOR_POWERSAVE:
        {
            strncpy( new_setting, "powersave", sizeof(new_setting) - 1 );
            break;
        }
        case DVFS_GOVERNOR_ONDEMAND:
        {
            strncpy( new_setting, "ondemand", sizeof(new_setting) - 1 );
            break;
        }
    }
    sprintf( cmd, "echo %s >/sys/devices/system/cpu/cpu%d/cpufreq/scaling_governor", new_setting, cpu );
    printf("~issuing cmd (%s) ~", cmd );
    system( cmd );

    return ( 0 );
}

/**
 *  Function: This function will return a string that contains the various CPU
 *            frequencies that are supported by the chip. Sometimes, the number
 *            of CPU frequencies is just 1.
 *            cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies
 **/
int get_cpu_frequencies_supported( int cpu, char *cpu_frequencies_supported, int cpu_frequencies_supported_size )
{
    char cpu_frequencies_supported_filename[128];
    char *contents = NULL;

    if ( cpu_frequencies_supported == NULL ) return -1;

    sprintf( cpu_frequencies_supported_filename, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_available_frequencies", cpu );
    contents = GetFileContents ( cpu_frequencies_supported_filename );

    if ( contents )
    {
        strcpy( cpu_frequencies_supported, contents );
        Bsysperf_Free( contents );
    }
    return ( 0 );
}

/**
 *  Function: This function will output the BYTES tag followed by a human-readable
 *            number of bytes in the file.
 **/
int output_file_size_in_human( const char *sFilename )
{
    struct stat statbuf;

    /* if a capture file name is known */
    if (strlen( sFilename ))
    {
        if (lstat( sFilename, &statbuf ) == -1)
        {
            /*printf( "Could not stat (%s)\n", sFilename );*/
        }
        else
        {
            printf( "~BYTES~" );
            if (statbuf.st_size < 1024)
            {
                printf( "%lu bytes\n", (unsigned long int) statbuf.st_size );
            }
            else if (statbuf.st_size < 1024*1024)
            {
                float kilobytes = 1.0 * statbuf.st_size /1024.;
                printf( "%.2f KB\n", kilobytes );
            }
            else if (statbuf.st_size < 1024*1024*1024)
            {
                float megabytes = 1.0 * statbuf.st_size /1024. / 1024.;
                printf( "%.2f MB\n", megabytes );
            }
            else
            {
                float gigabytes = 1.0 * statbuf.st_size /1024. / 1024. / 1024.;
                printf( "%.2f GB\n", gigabytes );
            }
            printf( "~\n" );
        }
    }

    return ( 0 );
}

char *decodeFilename(
    const char *filename
    )
{
    char        *pos         = NULL;
    char        *prev        = NULL;
    char        *decodedName = NULL;
    unsigned int pass        = 0;

    if (filename == NULL)
    {
        return( NULL );
    }

    PRINTF( "%s: filename (%s)\n", __FUNCTION__, filename );
    decodedName = malloc( strlen( filename ));
    if (decodedName == NULL)
    {
        PRINTF( "%s: could not malloc %u bytes for decodedName\n", __FUNCTION__, strlen( filename ));
        return( NULL );
    }
    PRINTF( "%s: malloc %u bytes for decodedName\n", __FUNCTION__, strlen( filename ));
    memset( decodedName, 0, strlen( filename ));

    pos = prev = (char *) filename;

    while (( pos = strstr( pos, "%2F" )))
    {
        strncat( decodedName, prev, pos - prev );
        strncat( decodedName, "/", 1 );
        PRINTF( "%s: pass %u: newname (%s)\n", __FUNCTION__, pass, decodedName );
        pos += 3;
        prev = pos;
        pass++;
    }
    strcat( decodedName, prev );
    PRINTF( "%s: pass %u: newname (%s)\n", __FUNCTION__, pass, decodedName );

    return( decodedName );
} /* decodeFilename */

int sendFileToBrowser(
    const char *filename
    )
{
    char        *contents = NULL;
    FILE        *fpInput  = NULL;
    struct stat  statbuf;
    char        *pFileNameDecoded = NULL;
    char        *pos              = NULL;
    char         tempFilename[TEMP_FILE_FULL_PATH_LEN];

    PRINTF( "~%s: filename (%p)\n~", __FUNCTION__, filename ); fflush( stdout ); fflush( stderr );
    PRINTF( "~%s: filename (%s)\n~", __FUNCTION__, filename ); fflush( stdout ); fflush( stderr );
    pFileNameDecoded = decodeFilename( filename );

    PRINTF( "~%s: pFileNameDecoded (%s)\n~", __FUNCTION__, filename ); fflush( stdout ); fflush( stderr );
    if (pFileNameDecoded == NULL)
    {
        return( -1 );
    }

    memset ( tempFilename, 0, sizeof(tempFilename) );
    strncpy ( tempFilename, pFileNameDecoded, sizeof(tempFilename) -1 );
    PRINTF( "~strncpy(%s)\n~", tempFilename ); fflush( stdout ); fflush( stderr );

    if (lstat( tempFilename, &statbuf ) == -1)
    {
        /*printf( "%s: Could not stat (%s)\n", __FUNCTION__, tempFilename ); fflush( stdout ); fflush( stderr );*/
        return( -1 );
    }
    /* remove the tempdir part from beginning of the file */
    pos = strrchr( tempFilename, '/' );
    PRINTF( "~pos 1 (%s)(%p)\n~", pos, pos ); fflush( stdout ); fflush( stderr );
    PRINTF( "~tempFilename (%s)\n~", tempFilename ); fflush( stdout ); fflush( stderr );
    PRINTF( "~statbuf.st_size %lld\n~", statbuf.st_size ); fflush( stdout ); fflush( stderr );
    if (pos)
    {
        pos++; /* advance pointer past the slash character */
        PRINTF( "~pos 2 (%s)(%p); len %d\n~", pos, pos, strlen( pos )); fflush( stdout ); fflush( stderr );
    }
    else
    {
        pos = (char *) tempFilename;
        PRINTF( "~pos 3 (%s)(%p); len %d\n~", pos, pos, strlen( pos )); fflush( stdout ); fflush( stderr );
    }

    if (statbuf.st_size == 0)
    {
        printf( "Content-Disposition: form-data; filename=\"%s\" \n", pos ); fflush( stdout ); fflush( stderr );
    }
    else
    {
        contents = malloc( statbuf.st_size );
        PRINTF( "%s: malloc(%llu); contents (%p)\n", __FUNCTION__, statbuf.st_size, contents ); fflush( stdout ); fflush( stderr );
        if (contents == NULL) {return( -1 ); }

        fpInput = fopen( tempFilename, "r" );
        PRINTF( "%s: fread(%llu)\n", __FUNCTION__, statbuf.st_size ); fflush( stdout ); fflush( stderr );
        fread( contents, 1, statbuf.st_size, fpInput );
        fclose( fpInput );

        printf( "Content-Type: application/octet-stream\n" ); fflush( stdout ); fflush( stderr );
        /* Disposition line has to come AFTER the Content-Type line if length is non-zero */
        printf( "Content-Disposition: attachment; filename=\"%s\" \n", pos ); fflush( stdout ); fflush( stderr );
        printf( "Content-Length: %lu\n", (unsigned long int) statbuf.st_size ); fflush( stdout ); fflush( stderr );
        printf( "\n" );

        fwrite( contents, 1, statbuf.st_size, stdout );
    }

    Bsysperf_Free( pFileNameDecoded );
    Bsysperf_Free( contents );

    return( 0 );
} /* sendFileToBrowser */

int readFileFromBrowser(
    const char       *contentType,
    const char       *contentLength,
    char             *sFilename,
    unsigned long int lFilenameLen
    )
{
    int   ContentLengthInt;
    char *boundaryString = NULL;
    char *cgi_query      = NULL;
    FILE *fOut           = NULL;

    /* How much data do we expect? */

    PRINTF( "Content-type: text/html\n\n" );
    PRINTF( "len (%s); type (%s)<br>\n", contentLength, contentType );
    if (( contentLength == NULL ) ||
        ( sscanf( contentLength, "%d", &ContentLengthInt ) != 1 ))
    {
        PRINTF( "could not sscanf CONTENT_LENGTH<br>\n" );
        return( -1 );
    }
    /* Create space for it: */

    cgi_query = malloc( ContentLengthInt + 1 );
    PRINTF( "malloc'ed space for %d bytes<br>\n", ContentLengthInt + 1 );

    if (cgi_query == NULL)
    {
        PRINTF( "could not malloc(%d) bytes<br>\n", ContentLengthInt + 1 );
        return( -1 );
    }
    /* Read it in: */

    boundaryString = strstr( contentType, "boundary=" );
    if (boundaryString == NULL)
    {
        PRINTF( "could not find boundary string<br>\n" );
        return( -1 );
    }
    boundaryString += strlen( "boundary=" );

    {
        int           byteCount        = 0;
        int           fileBytes        = 0;
        int           fgetBytes        = 0;
        int           headerBytes      = 0;
        unsigned char endOfHeaderFound = 0;    /* set to true when \r\n found on a line by itself */
        unsigned char endOfFileFound   = 0;    /* set to true when file contents have been read */

        while (byteCount < ContentLengthInt && endOfFileFound == 0)
        {
            if (endOfHeaderFound == 0)
            {
                fgets( &cgi_query[fileBytes], ContentLengthInt + 1, stdin );
                fgetBytes  = strlen( &cgi_query[fileBytes] );
                byteCount += fgetBytes;
                PRINTF( "got %u bytes (%s)<br>\n", fgetBytes, &cgi_query[fileBytes] );

                /* if next line has a boundary directive on it */
                if (strstr( &cgi_query[fileBytes], boundaryString ))
                {
                    headerBytes += strlen( cgi_query ) + strlen( boundaryString ) + strlen( "--" ) + strlen( "--" ) + strlen( "\r\n" ) + strlen( "\r\n" );
                    PRINTF( "found boundary (%s); len %d; headerBytes %d; remaining %d<br>\n", &cgi_query[fileBytes], strlen( cgi_query ), headerBytes, ContentLengthInt - headerBytes );
                    cgi_query[fileBytes] = '\0';
                }
                else if (strncmp( &cgi_query[fileBytes], "Content-", 8 ) == 0)  /* if next line has a Content directive on it */
                {
                    headerBytes += strlen( cgi_query );
                    PRINTF( "found directive (%s); len %d; headerBytes %d; remaining %d<br>\n", &cgi_query[fileBytes], strlen( cgi_query ), headerBytes, ContentLengthInt - headerBytes );
                    if (strncmp( &cgi_query[fileBytes], "Content-Disposition:", 20 ) == 0)  /* if next line has a Content-Disposition directive on it */
                    {
                        char *filename = NULL;
                        /* find filename tag */
                        filename = strstr( &cgi_query[fileBytes], FILENAME_TAG );
                        if (filename)
                        {
                            char *end_of_filename = NULL;
                            filename       += strlen( FILENAME_TAG ); /* advance pointer past the tag name */
                            end_of_filename = strchr( filename, '"' );
                            if (end_of_filename)
                            {
                                unsigned int tempFilenameLen = end_of_filename - filename + strlen( GetTempDirectoryStr());
                                /* if destination string has space for the filename string passed in from the browser */
                                if (sFilename && ( tempFilenameLen < lFilenameLen ))
                                {
                                    memset( sFilename, 0, lFilenameLen );
                                    strncpy( sFilename, GetTempDirectoryStr(), lFilenameLen );
                                    strncat( sFilename, filename, end_of_filename - filename );
                                    PRINTF( "%s: sFilename %p:(%s)<br>\n", __FUNCTION__, sFilename, sFilename );
                                }
                            }
                        }
                    }
                    cgi_query[fileBytes] = '\0';
                }
                else if (( endOfHeaderFound == 0 ) && ( cgi_query[fileBytes] == '\r' ) && ( cgi_query[fileBytes + 1] == '\n' ))
                {
                    headerBytes += strlen( cgi_query );
                    PRINTF( "found end of header (%s) (skipping these bytes); headerBytes %d; remaining %d<br>\n", &cgi_query[fileBytes], headerBytes,
                        ContentLengthInt - headerBytes );
                    endOfHeaderFound     = 1;
                    cgi_query[fileBytes] = '\0';
                }
            }
            else
            {
                fgetBytes  = fread( &cgi_query[fileBytes], 1, ContentLengthInt - headerBytes, stdin );
                byteCount += fgetBytes;
                PRINTF( "added %d bytes to file contents<br>\n", fgetBytes );
                fileBytes       += fgetBytes;
                endOfHeaderFound = 0;
                endOfFileFound   = 1;
            }

            PRINTF( "byteCount %d; ContentLengthInt %d; fileBytes %d<br>\n", byteCount, ContentLengthInt, fileBytes );
        }

        PRINTF( "%s: fopen (%s)<br>\n", __FUNCTION__, sFilename );
        fOut = fopen( sFilename, "w" );
        if (fOut)
        {
            PRINTF( "%s: fwrite (%d) bytes<br>\n", __FUNCTION__, fileBytes );
            fwrite( cgi_query, 1, fileBytes, fOut );
            fclose( fOut );
            PRINTF( "<h3>Output file is (%s)</h3>\n", sFilename );
        }
        /* free the malloc'ed space if it was malloc'ed successfully */
        Bsysperf_Free( cgi_query );
    }

    return( 0 );
} /* readFileFromBrowser */

/**
 *  Function: This function returns to the caller the number of bytes contained in the specified file.
 **/
int getFileSize(
    const char *filename
    )
{
    struct stat file_stats;

    if (filename == NULL)
    {
        return( 0 );
    }

    if (stat( filename, &file_stats ) != 0)
    {
        PRINTF( "<!-- ERROR getting stats for file (%s) -->\n", filename );
        return( 0 );
    }

    return( file_stats.st_size );
} /* getFileSize */

/**
 *  Function: This function returns to the caller the contents of the specified file. Space will be
 *  malloc'ed for the file size. It is up to the caller to free the contents pointer.
 **/
char *getFileContents(
    const char *filename
    )
{
    FILE             *pFile    = NULL;
    char             *contents = NULL;
    unsigned long int filesize = 0;

    if (filename == NULL)
    {
        return( contents );
    }

    filesize = getFileSize( filename );
    if (filesize == 0)
    {
        return( contents );
    }
    contents = malloc( filesize + 1 );
    /* if we successfully allocated space for te file*/
    if (contents)
    {
        memset( contents, 0, filesize + 1 );
        pFile = fopen( filename, "r" );
        fread( contents, 1, filesize, pFile );
        fclose( pFile );
    }
    return( contents );
} /* getFileContents */

/**
 *  Function: This function will find the last occurance of the string in the buffer.
 **/
char *Bsysperf_FindLastStr ( const char * buffer, const char * searchStr )
{
    char *pos = (char*) buffer;
    char *posPrev = NULL;

    if ( buffer == NULL ) return posPrev;

    do
    {
        posPrev = pos;

        pos = strstr( (posPrev+1), searchStr ); /* start searching for next string 1 char after previous find */
        /*printf( "%s:%u buffer %p;  pos %p (off %d); prev (%p) search (%s) \n", __FILENAME__, __LINE__, buffer, pos, (pos-buffer), posPrev, searchStr );*/
    } while ( pos );

    return posPrev;
}
/**
 *  Function: This function will work like the printf statement but will send the
 *            output to a temporary log file instead of STDOUT.
 **/
void printflog(const char * szFormat, ... )
{
    char str[BASPMON_CFG_MAX_LINE_LEN];
    unsigned long int nLen = sizeof(str);
    static char sLogFile[128];
    static unsigned char LogFileNameSet = 0;
    int fd=0;

    memset(str, 0, sizeof str);
    if ( LogFileNameSet == 0 ) {
        memset(sLogFile, 0, sizeof sLogFile );
        sprintf ( sLogFile, "/tmp/bsysperf.log" );
        /*fprintf ( stderr, "%s: log file is (%s)\n", __FUNCTION__, sLogFile );*/
        LogFileNameSet=1;
    }

    va_list arg_ptr;
    va_start(arg_ptr, szFormat );
    #ifdef _WINDOWS_
    _vsnprintf(str, nLen-1, szFormat, arg_ptr );
    #else
    vsnprintf(str, nLen-1, szFormat, arg_ptr );
    #endif
    va_end(arg_ptr);

    if ( strlen( sLogFile ) ) {
        fd = open ( sLogFile, (O_CREAT | O_WRONLY | O_APPEND), 0777 );
        if ( fd ) {
            write ( fd, str, strlen(str) );
            close ( fd );
            chmod ( sLogFile, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH );

            /* output the string to terminal */
            /*fprintf ( stderr, "%s", str );*/
        } else {
            fprintf ( stderr, "%s: Could not open (%s)\n", __FUNCTION__, sLogFile );
        }
    } else {
        fprintf ( stderr, "%s: log file (%s) is invalid\n", __FUNCTION__, sLogFile );
        fprintf ( stderr, "%s", str );
    }

    return;
}

/**
 *  Function: This function will work like the printf statement but will send the
 *            output to the specified file instead of STDOUT.
 **/
void printffile(const char *sLogFile, const char * szFormat, ... )
{
    char str[256];
    unsigned long int nLen = sizeof(str);
    int fd=0;

    memset(str, 0, sizeof str);

    va_list arg_ptr;
    va_start(arg_ptr, szFormat );
    #ifdef _WINDOWS_
    _vsnprintf(str, nLen-1, szFormat, arg_ptr );
    #else
    vsnprintf(str, nLen-1, szFormat, arg_ptr );
    #endif
    va_end(arg_ptr);

    if ( strlen( sLogFile ) ) {
        fd = open ( sLogFile, (O_CREAT | O_WRONLY | O_APPEND), 0777 );
        if ( fd ) {
            write ( fd, str, strlen(str) );
            close ( fd );
            chmod ( sLogFile, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH );

            /* output the string to terminal */
            /*fprintf ( stderr, "%s", str );*/
        } else {
            fprintf ( stderr, "%s: Could not open (%s)\n", __FUNCTION__, sLogFile );
        }
    } else {
        fprintf ( stderr, "%s: log file (%s) is invalid\n", __FUNCTION__, sLogFile );
        fprintf ( stderr, "%s", str );
    }

    return;
}

typedef struct
{
    char name[16];
} NAME16;
/**
 *  Function: This function will create the DVFS Control HTML table using the tag DvfsControl.
 **/
int Bsysperf_DvfsCreateHtml( bool bIncludeFrequencies, bool bMinimalFields )
{
    int       cpu           = 0;
    int       numCpusConf   = 0;
    long int  cpu_freq_int  = 0;
    char     *contents      = NULL;
    char      cpu_frequencies_supported[128];
    int       cell_width    = 50;
    int       font_size     = 14;
    NAME16    LongNames[4] = {{"Conservative"},{"Performance"},{"Power Save"},{"On Demand"}};
    NAME16    ShortNames[4] = {{"Consrv"},{"Perfrm"},{"PwrSav"},{"OnDem"}};
    NAME16   *WhichNamesToUse = &LongNames[0];

    numCpusConf = sysconf( _SC_NPROCESSORS_CONF );
    if (numCpusConf > BMEMPERF_MAX_NUM_CPUS)
    {
        numCpusConf = BMEMPERF_MAX_NUM_CPUS;
    }

    contents = GetFileContents( "/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors" );

    printf( "~DvfsControl~" );
    printf( "<table cols=9 border=0 style=\"border-collapse:collapse;\" cellpadding=3 >" );
    if ( bMinimalFields == false )
    {
        printf( "<tr><th colspan=9 class=whiteborders18 align=left >%s</th></tr>", "DVFS Controls" );

        printf( "<tr><th colspan=9 class=whiteborders18 align=left >%s</th></tr>", "Power Saving Techniques" );
    }
    else
    {
        WhichNamesToUse = &ShortNames[0];
        cell_width = 20;
        font_size  = 12;
    }
    printf( "<tr %s ><td colspan=9><table border=0 style=\"border-collapse:collapse;\" ><tr>", (bMinimalFields)?" ": "style=\"outline: thin solid\"" );
    if ( contents && strstr(contents, "conservative") )
    {
        printf( "<td style=\"font-size:%d\" ><input type=radio name=radioGovernor id=radioGovernor%d value=%d onclick=\"MyClick(event);\" >%s</td>",
                font_size, DVFS_GOVERNOR_CONSERVATIVE, DVFS_GOVERNOR_CONSERVATIVE, (char*) &(WhichNamesToUse[0]) );
        printf( "<td width=%d>&nbsp;</td>", cell_width ); /* spacer */
    }
    if ( contents && strstr(contents, "performance") )
    {
        printf( "<td style=\"font-size:%d\"><input type=radio name=radioGovernor id=radioGovernor%d value=%d onclick=\"MyClick(event);\" >%s</td>",
                font_size, DVFS_GOVERNOR_PERFORMANCE, DVFS_GOVERNOR_PERFORMANCE, (char*) &(WhichNamesToUse[1]) );
        printf( "<td width=%d>&nbsp;</td>", cell_width ); /* spacer */
    }
    if ( contents && strstr(contents, "powersave") )
    {
        printf( "<td style=\"font-size:%d\"><input type=radio name=radioGovernor id=radioGovernor%d value=%d onclick=\"MyClick(event);\" >%s</td>",
                font_size, DVFS_GOVERNOR_POWERSAVE, DVFS_GOVERNOR_POWERSAVE, (char*) &(WhichNamesToUse[2]) );
        printf( "<td width=%d>&nbsp;</td>", cell_width ); /* spacer */
    }
    if ( contents && strstr(contents, "ondemand") )
    {
        printf( "<td style=\"font-size:%d\"><input type=radio name=radioGovernor id=radioGovernor%d value=%d onclick=\"MyClick(event);\" >%s</td>",
                font_size, DVFS_GOVERNOR_ONDEMAND, DVFS_GOVERNOR_ONDEMAND, (char*) &(WhichNamesToUse[3]) );
        printf( "</tr></table></td></tr>" );
    }

    if ( bIncludeFrequencies )
    {
        printf( "<tr><th colspan=9 class=whiteborders18 align=left >%s</th></tr>", "Frequencies Supported" );
        printf( "<tr bgcolor=lightgray style=\"outline: thin solid\" >");
        printf( "<td align=center style=\"border-right: 1px black solid;\" >CPU</td>");
        printf( "<td align=left style=\"border-right: 1px black solid;\" >Frequencies Supported</td>" );
        printf( "<td align=center > Current</td></tr>" );
        for (cpu = 0; cpu < numCpusConf; cpu++)
        {
            cpu_freq_int = get_cpu_frequency(cpu) * 1000;
            memset( cpu_frequencies_supported, 0, sizeof(cpu_frequencies_supported) );

            get_cpu_frequencies_supported( cpu, cpu_frequencies_supported, sizeof(cpu_frequencies_supported) );

            printf("<tr><td class=black_allborders align=center >%d</td><td class=black_allborders >%s</td>",
                    cpu, cpu_frequencies_supported );
            printf("<td class=black_allborders align=center >%ld</td></tr>", cpu_freq_int );
        }
    }

    printf( "</table>~" ); /* end DvfsControl */

    printf( "~GovernorSetting~%d~", get_governor_control( 0 ) );

    Bsysperf_Free( contents );

    return ( 0 );
}

/*
   /proc/net/snmp contents ... look for these two lines:
Tcp: RtoAlgorithm RtoMin RtoMax MaxConn ActiveOpens PassiveOpens AttemptFails EstabResets CurrEstab InSegs OutSegs RetransSegs InErrs OutRsts InCsumErrors
Tcp: 1 200 120000 -1 8 2 0 0 2 690709 417282 11 0 3 0

    8 active connection openings
    2 passive connection openings
    0 failed connection attempts
    0 connection resets received
    2 connections established
    690717 segments received
    417292 segments sent out
    11 segments retransmitted
    0 bad segments received
    6 resets sent
*/
int Bsysperf_GetTcpStatistics(
    char * outputBuffer,
    int    outputBufferLen
    )
{
    char *contents;
    char  one_line[128];
    FILE *fp=NULL;
    char *pos=NULL;
    char *tcp=NULL;
    long int unk1, unk2, unk3, unk4;
    long int connactive, connpassive, connfailed, connresets, connestablished, segreceived, segsent, segretransmitted, segbad, resetsent;

    contents = malloc( PATH_PROCNET_SNMP_MAX );
    if ( contents == NULL )
    {
        printf("Could not malloc %d bytes\n", PATH_PROCNET_SNMP_MAX );
        return -1;
    }

    fp = fopen( PATH_PROCNET_SNMP, "r" );
    if ( fp == NULL )
    {
        printf("Could not open file %s\n", PATH_PROCNET_SNMP );
        Bsysperf_Free( contents );
        return -1;
    }

    memset( contents, 0, PATH_PROCNET_SNMP_MAX );

    fread( contents, 1, PATH_PROCNET_SNMP_MAX, fp );
    fclose(fp);

    pos = strstr( contents, "Tcp: RtoAlgorithm" );
    if ( pos )
    {
        pos++; /* skip past the Tcp: tag so that we don't find it again */
        tcp = strstr( pos, "Tcp:" );
        if ( tcp )
        {
            sscanf( tcp, "Tcp: %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld\n", &unk1, &unk2, &unk3, &unk4,
                &connactive, &connpassive, &connfailed, &connresets, &connestablished, &segreceived, &segsent,
                &segretransmitted, &segbad, &resetsent );
            memset( one_line, 0, sizeof(one_line) );
            snprintf( one_line, sizeof(one_line), "Tcp:\n");
            strncpy( outputBuffer, one_line, outputBufferLen );
            memset( one_line, 0, sizeof(one_line) );
            snprintf( one_line, sizeof(one_line), " %ld active connection openings\n", connactive );
            strncat( outputBuffer, one_line, outputBufferLen );
            memset( one_line, 0, sizeof(one_line) );
            snprintf( one_line, sizeof(one_line), " %ld passive connection openings\n", connpassive );
            strncat( outputBuffer, one_line, outputBufferLen );
            memset( one_line, 0, sizeof(one_line) );
            snprintf( one_line, sizeof(one_line), " %ld failed connection attempts\n", connfailed );
            strncat( outputBuffer, one_line, outputBufferLen );
            memset( one_line, 0, sizeof(one_line) );
            snprintf( one_line, sizeof(one_line), " %ld connection resets received\n", connresets );
            strncat( outputBuffer, one_line, outputBufferLen );
            memset( one_line, 0, sizeof(one_line) );
            snprintf( one_line, sizeof(one_line), " %ld connections established\n", connestablished );
            strncat( outputBuffer, one_line, outputBufferLen );
            memset( one_line, 0, sizeof(one_line) );
            snprintf( one_line, sizeof(one_line), " %ld segments received\n", segreceived );
            strncat( outputBuffer, one_line, outputBufferLen );
            memset( one_line, 0, sizeof(one_line) );
            snprintf( one_line, sizeof(one_line), " %ld segments sent out\n", segsent );
            strncat( outputBuffer, one_line, outputBufferLen );
            memset( one_line, 0, sizeof(one_line) );
            snprintf( one_line, sizeof(one_line), " %ld segments retransmitted\n", segretransmitted );
            strncat( outputBuffer, one_line, outputBufferLen );
            memset( one_line, 0, sizeof(one_line) );
            snprintf( one_line, sizeof(one_line), " %ld bad segments received\n", segbad);
            strncat( outputBuffer, one_line, outputBufferLen );
            memset( one_line, 0, sizeof(one_line) );
            snprintf( one_line, sizeof(one_line), " %ld resets sent\n", resetsent );
            strncat( outputBuffer, one_line, outputBufferLen );
        }
    }
    Bsysperf_Free( contents );
    return 0;
}   /* Bsysperf_GetTcpStatistics */

/**
 *  Function: This function will read the specified configuration file and search for the
 *            specified tag line. If the tag line is found, the value associated with the
 *            tag will be copied to the user's output buffer.
 **/
int Bmemperf_GetCfgFileEntry(
    const char* cfg_filename,
    const char* cfg_tagline,
    char* output_buffer,
    int output_buffer_len
    )
{
    FILE *fp = NULL;
    char *oneline = NULL;

    if ( cfg_filename == NULL || cfg_tagline == NULL || output_buffer == NULL || output_buffer_len <=0 )
    {
        return ( -1 );
    }
    oneline = malloc( BASPMON_CFG_MAX_LINE_LEN );
    if ( oneline == NULL ) {
        fprintf( stderr, "%s - could not malloc(%d) for oneline \n", __FUNCTION__, BASPMON_CFG_MAX_LINE_LEN );
        return ( -1 );
    }
    memset( oneline, 0, BASPMON_CFG_MAX_LINE_LEN );

    /*printflog( "\n\n");*/
    /*printflog( "%s:%u - filename (%s) ... tagline (%s) \n", __FUNCTION__, __LINE__, cfg_filename, cfg_tagline );*/

    fp = fopen( cfg_filename, "r" );
    if ( fp == NULL )
    {
        return ( -1 );
    }

    PRINTF( "%s: tagline (%s)\n", __FUNCTION__, cfg_tagline );
    while (fgets( oneline, BASPMON_CFG_MAX_LINE_LEN, fp ))
    {
        /*printflog( "%s:%u newline len %d ... (%s)\n", __FUNCTION__, __LINE__, strlen(oneline), oneline );*/
        /* if we found a matching line */
        if ( strstr( oneline, cfg_tagline ) )
        {
            /*printflog( "%s:%u - line len (%d) \n", __FUNCTION__, __LINE__, strlen(oneline) );*/
            char *bov = strchr( oneline, '"'); /* determine the beginning of the assocated value */
            char *eov = NULL;
            if ( bov )
            {
                eov = strchr( (bov+1), '"'); /* determine the end of the assocated value */
            }
            if ( bov && eov )
            {
                bov++; /* skip the beginning double-quote */
                *eov = '\0'; /* overwrite the ending double-quote with the string terminator */
                strncpy( output_buffer, bov, output_buffer_len -1 );
                break;
            }
        }
        memset( oneline, 0, BASPMON_CFG_MAX_LINE_LEN );
    }

    fclose( fp );

    Bsysperf_Free( oneline );

    return ( 0 );
}

/**
 *  Function: This function will write the specified contents to the specified file.
 **/
int SetFileContents(
    const char* filename,
    const char* contents
    )
{
    FILE *fp = NULL;
    if ( filename == NULL || contents == NULL || strlen(contents) == 0 )
    {
        return ( -1 );
    }

    fp = fopen( filename, "w" );

    /* if the file was successfully opened for writing */
    if ( fp )
    {
        fwrite( contents, 1, strlen(contents), fp );
        fclose( fp );
    }

    return ( 0 );
}

/**
 *  Function: This function will read the specified configuration file and search for the
 *            specified tag line. If the tag line is found, the value associated with the
 *            tag will be updated with the user's new value.
 **/
int Bmemperf_SetCfgFileEntry(
    const char* cfg_filename,
    const char* cfg_tagline,
    char* new_value
    )
{
    char *contents = NULL;
    char *bot      = NULL;
    char *bov      = NULL;
    char *eov      = NULL;

    if ( cfg_filename == NULL || cfg_tagline == NULL || new_value == NULL || strlen(new_value) == 0 )
    {
        return ( -1 );
    }

    contents = getFileContents( cfg_filename );

    if ( contents )
    {
        /* find beginning of tag line */
        bot = strstr( contents, cfg_tagline );
        if ( bot )
        {
            int new_len = strlen(new_value);

            bov = strchr( bot, '"');
            /* if the double-quote at the beginning of the tag's value is found */
            if ( bov )
            {
                bov++;
                eov = strchr( bov, '"'); /* find the end of the existing value */
                /*                     bov             eov */
                /*                     |               |   */
                /*    old contents ... "123.123.123.123"  */
                /*    new contents ... "10.0.0.1"         */
                if ( eov )
                {
                    memcpy( bov, new_value, new_len ); /* copy 10.0.0.1 */
                    if ( new_len >= (eov-bov) )
                    {
                        /* new value completely overwrote old value ... need to end the double-quote */
                        bov[new_len] = '"';
                    }
                    else
                    {
                        /* new value is shorter than old value ... need to end the double-quote AND pad with spaces */
                        int spaces = (eov-bov) - new_len;
                        if ( spaces > 0 )
                        {
                            int idx=0;
                            bov[ new_len + idx ] = '"'; /* end the new string value */
                            for(idx=1; idx<spaces+1; idx++)
                            {
                                bov[ new_len + idx ] = ' ';
                            }
                        }
                    }

                    SetFileContents( cfg_filename, contents );
                }
            }
        }
        Bsysperf_Free( contents );
    }

    return ( 0 );
}

/**
 *  Function: This function will look in the provided string for the specified tag string. If the tag string is
 *  found on the line, the converted integer value will be returned.
 **/
static int scan_for_tag(
    const char             *line,
    const char             *tag,
    unsigned long long int *value
    )
{
    int                     rc     = -1;
    char                   *pos    = NULL;
    unsigned long long int  lvalue = 0;
    char                    valueStr[64];

    pos = strstr( line, tag );
    if (pos)
    {
        pos   += strlen( tag );
        lvalue = strtoull( pos, NULL, BSYSPERF_VALUE_BASE );
        if (strstr( tag, "RX packets:" ) && ( lvalue > 0 ))
        {
            valueStr[0] = 0;
            convert_to_string_with_commas( lvalue, valueStr, sizeof( valueStr ));
            PRINTF( "%s %lu (%s) (%s) (addr -> %p)<br>\n", tag, lvalue, valueStr, pos, (void *)value );
        }

        if (value)
        {
            *value = lvalue;
        }

        rc = 0;
    }
    /* -1 is used to indicate the tag was not found; rc=0 indicates the tag was found and some values were scanned */
    return( rc );
}                                                          /* scan_for_tag */

/**
 *  Function: This function will use the 'ethtool' utility to read RX and TX data for the specified network interface.
 *            Since the data from the ifconfig utility has been deemed to be unreliable, the kernel team has suggested
 *            we use the 'ethtool' utility to get statistics for the "asp" interface.
 **/
extern bsysperf_netStatistics g_netStats[NET_STATS_MAX];
extern int                    g_netStatsIdx;                 /* index to entries added to g_netStats array */
static int get_ethtool_data(
    int netStatsIdx
    )
{
    char *rc  = NULL;
    char *pos = NULL;
    FILE *cmd = NULL;
    char  line[MAX_LINE_LENGTH];
    unsigned int line_count = 0;
    unsigned long long llu_bytes = 0;

    if ( netStatsIdx < 0 || netStatsIdx > NET_STATS_MAX ) return 0;

/* These counters are from the embedded network switch's perspective and thus are reversed.
 * In other words, when ASP is streaming out, you will see the Rx Count on ASP device go up
 * as it is receiving packets from ASP. Likewise, when ASP is receiving packets, the switch
 * is transmitting those packets to ASP (which in turn came from a network client on another
 * switch's port), and thus TxCount would go up!
*/
#define ASP_RX_SEARCH_TAG "TxOctets"
#define ASP_TX_SEARCH_TAG "RxOctets"

    /* create a line similar to this ... /bin/ethtool -S asp | grep "RxOctets|TxOctets"
     * We should get two lines back similar to this:
     *      RxOctets: 14259285
     *      RxOctets: 432809372
     */
    sprintf( line, "%s -S %s | grep -E \"%s|%s\"", ETHTOOL_UTILITY, g_netStats[netStatsIdx].name, ASP_RX_SEARCH_TAG, ASP_TX_SEARCH_TAG );
    cmd = popen( line, "r" );

    /* we are only expecting to read 2 lines ... one for RX and one for TX */
    while ( cmd && line_count < 2 )
    {
        char search_string[32];

        memset( line, 0, sizeof( line ));
        memset( search_string, 0, sizeof( search_string ));

        rc = fgets( line, MAX_LINE_LENGTH, cmd );

        /* if something was read in */
        if ( rc )
        {
            /* remove end-of-line character if present */
            pos = strchr( line, '\n' );
            if (pos != NULL)
            {
                *pos = '\0';
            }

            PRINTF( "~%s: got len %u: line (%s)~", __FUNCTION__, (int) strlen( line ), line );

            sprintf( search_string, "%s: ", ASP_TX_SEARCH_TAG );
            pos = strstr( line, search_string );
            if (pos != NULL)
            {
                pos += strlen( search_string ); /* advance pointer to where the byte could should be located */
                sscanf( pos, "%llu", &llu_bytes );

                g_netStats[netStatsIdx].txBytes = llu_bytes;
                PRINTF( "~%s: for (%s): %s now set to (%llu)~", __FUNCTION__, ASP_TX_SEARCH_TAG, g_netStats[netStatsIdx].name, llu_bytes );
            }

            sprintf( search_string, "%s: ", ASP_RX_SEARCH_TAG );
            pos = strstr( line, search_string );
            if (pos != NULL)
            {
                pos += strlen( search_string ); /* advance pointer to where the byte could should be located */
                sscanf( pos, "%llu", &llu_bytes );

                g_netStats[netStatsIdx].rxBytes = llu_bytes;
                PRINTF( "~%s: for (%s): %s now set to (%llu)~", __FUNCTION__, ASP_RX_SEARCH_TAG, g_netStats[netStatsIdx].name, llu_bytes );
            }
        }
        else
        {
            break; /* once we get a read error, stop reading */
        }

        line_count++;
    }

    pclose( cmd );

    return( 0 );
} /* get_ethtool_data */

/**
 *  Function: This function will gather various network statistics
 **/
int get_netstat_data(
    bsysperf_netStatistics *pNetStats
    )
{
    char *pos = NULL;
    FILE *cmd = NULL;
    char  line[MAX_LINE_LENGTH];
#if DEBUG_WLANZERO
    struct timeval tv;
    bool   bFoundWlan0 = false;
#endif /* DEBUG_WLANZERO */

    if (pNetStats == NULL)
    {
        return( -1 );
    }

#if DEBUG_WLANZERO
    gettimeofday( &tv, NULL );
    printflog( "time now ... %d.%06d =====================\n", tv.tv_sec, tv.tv_usec );
#endif /* DEBUG_WLANZERO */

    /* clear out the array */
    memset( pNetStats, 0, sizeof( *pNetStats ));
    g_netStatsIdx = -1;

    sprintf( line, "%s", IFCONFIG_UTILITY );
    cmd = popen( line, "r" );

    do {
        memset( line, 0, sizeof( line ));
        fgets( line, MAX_LINE_LENGTH, cmd );
        if (strlen( line ))
        {
#if DEBUG_WLANZERO
            if ( bFoundWlan0 ) printflog( "%s", line );
#endif /* DEBUG_WLANZERO */
            /* if something is in column 1, it must be a name of another interface */
            if (( 'a' <= line[0] ) && ( line[0] <= 'z' ))
            {
                /* if there is room for another interface */
                if (g_netStatsIdx < (NET_STATS_MAX-1) )
                {
                    g_netStatsIdx++;

                    /* look for a space; that marks the end of the i/f name */
                    pos = strchr( line, ' ' );
                    if (pos)
                    {
                        *pos = '\0';                       /* null-terminate the i/f name */
                        strncpy( pNetStats[g_netStatsIdx].name, line, sizeof( pNetStats[g_netStatsIdx].name ) - 1 );
#if DEBUG_WLANZERO
                        if ( strcmp( pNetStats[g_netStatsIdx].name, "wlan0") == 0 ) {
                            *pos = ' ';
                            printflog( "%s", line );
                            *pos = '\0';
                            bFoundWlan0 = true;
                        }
#endif /* DEBUG_WLANZERO */
                    }
                }
                else
                {
                    printf( "%s: not enough room for new interface (%s) in array; max'ed out at %u\n", __FUNCTION__, line, g_netStatsIdx );
                }
            }

            /* if we are working on an interface that was successfully saved in the global structure (i.e. we haven't run out of space) */
            if ( (g_netStatsIdx >= 0) && (g_netStatsIdx < NET_STATS_MAX) )
            {
                int rc = 0;

                /* if line has IP address on it */
                if (( pos = strstr( line, "inet addr:" )))
                {
                    char *ipAddrStart = NULL;

                    pos += strlen( "inet addr:" );

                    ipAddrStart = pos;

                    /* look for a space; that marks the end of the i/f name */
                    pos = strchr( ipAddrStart, ' ' );
                    if (pos)
                    {
                        *pos = '\0';                       /* null-terminate the IP address */
                        strncpy( pNetStats[g_netStatsIdx].ipAddress, ipAddrStart, sizeof( pNetStats[g_netStatsIdx].ipAddress ) - 1 );
                        PRINTF( "IF (%s) has IP addr (%s)<br>\n", pNetStats[g_netStatsIdx].name, pNetStats[g_netStatsIdx].ipAddress );
                    }
                }
                /*
                eth0      Link encap:Ethernet  HWaddr 00:10:18:D2:C3:C9
                          inet addr:10.14.244.188  Bcast:10.14.245.255  Mask:255.255.254.0
                          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
                          RX packets:11832 errors:0 dropped:0 overruns:0 frame:0
                          TX packets:2150 errors:0 dropped:0 overruns:0 carrier:0
                          collisions:0 txqueuelen:1000
                          RX bytes:2429458 (2.3 MiB)  TX bytes:632148 (617.3 KiB)
                */

                rc = scan_for_tag( line, "RX packets:", &pNetStats[g_netStatsIdx].rxErrors );
                /* if the RX packets tag was found, now scan for errors tag on the same line */
                if (rc == 0)
                {
                    rc = scan_for_tag( line, " errors:", &pNetStats[g_netStatsIdx].rxErrors );
                }

                rc = scan_for_tag( line, "TX packets:", &pNetStats[g_netStatsIdx].txErrors );
                /* if the TX packets tag was found, now scan for errors tag on the same line */
                if (rc == 0)
                {
                    rc = scan_for_tag( line, " errors:", &pNetStats[g_netStatsIdx].txErrors );
                }
                rc = scan_for_tag( line, "RX bytes:", &pNetStats[g_netStatsIdx].rxBytes );
                /* if the RX bytes tag was found, now scan for the Tx bytes tag on the same line */
                if (rc == 0)
                {
                    scan_for_tag( line, "TX bytes:", &pNetStats[g_netStatsIdx].txBytes );

                    /* before moving on to the next interface device, check to see if we are processing the "asp" interface */
                    if ( strcmp( pNetStats[g_netStatsIdx].name, "asp" ) == 0 )
                    {
                        get_ethtool_data( g_netStatsIdx );
                    }
                }
                /*printf("~DEBUG~ip_addr (%s) ... TX bytes (%lld)~", pNetStats[g_netStatsIdx].ipAddress, pNetStats[g_netStatsIdx].txBytes );*/
            }
        }
    } while (strlen( line ));
    PRINTF( "\n" );

    pclose( cmd );

    return( 0 );
}                                                          /* get_netstat_data */

/**
 *  Function: This function will perform a quick check to see if the specified IP address
 *            has the necessary daemon running to accept connections to the server using
 *            the specified port. It is used to determine if a telnet will succeed. Checking
 *            beforehand like this allows the code to bypass the 5-second timeout that the
 *            normal telnet connect would take.
 **/
int Bmemperf_Ping( const char * addr, int port )
{
    struct sockaddr_in address;
    short int          socked_fd = -1;
    fd_set             fdset;
    struct timeval     tv = {0, 20000};

    memset( &address, 0, sizeof(address) );

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(addr); /* assign the address */
    address.sin_port = htons(port);            /* translate int2port num */

    socked_fd = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(socked_fd, F_SETFL, O_NONBLOCK);

    connect(socked_fd, (struct sockaddr *)&address, sizeof(address));

    FD_ZERO(&fdset);
    FD_SET(socked_fd, &fdset);

    if (select(socked_fd + 1, NULL, &fdset, NULL, &tv) == 1)
    {
        int so_error;
        socklen_t len = sizeof so_error;

        getsockopt(socked_fd, SOL_SOCKET, SO_ERROR, &so_error, &len);

        if (so_error == 0)
        {
            PRINTF("%s: telnet port to (%s) succeeded\n", __FUNCTION__, addr );
            return 0;
        }
    }

    close(socked_fd);
    PRINTF("%s: telnet port to (%s) failed\n", __FUNCTION__, addr );
    return -1;
}

/**
 *  Function: This function will enable or disable the specified CPU. Negative values indicate we need to
 *  DISABLE the specified CPU; positive values mean we need to ENABLE the CPU. For example, a -3 means DISABLE
 *  CPU 3; a +3 means ENABLE CPU 3
 **/
void Bmemperf_ChangeCpuState(
    int new_state
    )
{
    char         line[MAX_LINE_LENGTH];
    unsigned int disableEnable = ( new_state<1 ) ? 0 : 1;

    /* you cannot disable/enable CPU 0 */
    if (new_state == 0)
    {
        return;
    }
    sprintf( line, "echo %u > /sys/devices/system/cpu/cpu%d/online", disableEnable, abs( new_state ));
    printf( "~issuing system(%s)~", line );
    system( line );
}

/**
 *  Function: This function will format an integer to output an indicator for kilo, mega, or giga.
 **/
static char outString[32];
char *formatul(
    unsigned long long int value
    )
{
    float fValue = value;

    if (value < 1024)
    {
        sprintf( outString, "%llu", value );
    }
    else if (value < 1024 * 1024)
    {
        sprintf( outString, "%5.1f K", fValue / 1024 );
    }
    else if (value < 1024 * 1024 * 1024)
    {
        sprintf( outString, "%6.2f M", fValue / 1024 / 1024 );
    }
    else
    {
        sprintf( outString, "%7.3f G", fValue / 1024 / 1024 / 1024 );
    }

    return( outString );
}                                                          /* formatul */

unsigned long int delta_time_microseconds(
    unsigned long int seconds,
    unsigned long int microseconds
    )
{
    struct timeval         tv2;
    unsigned long long int microseconds1      = 0;
    unsigned long long int microseconds2      = 0;
    unsigned long int      microseconds_delta = 0;

    memset( &tv2, 0, sizeof( tv2 ));

    gettimeofday( &tv2, NULL );
    microseconds1      = ( seconds * 1000000LL );       /* q-scale shift the seconds left to allow for addition of microseconds */
    microseconds1     += microseconds;
    microseconds2      = ( tv2.tv_sec * 1000000LL );       /* q-scale shift the seconds left to allow for addition of microseconds */
    microseconds2     += tv2.tv_usec;                      /* add in microseconds */
    microseconds_delta = ( microseconds2 - microseconds1 );
    /*printf( "now: %lu.%06lu ... input: %lu.%06lu ... elapsed time %lu milliseconds\n", tv2.tv_sec, tv2.tv_usec, seconds, microseconds, microseconds_delta/1000 );*/

    return( microseconds_delta );
}

/**
 *  Function: This function will determine the IP address of the specified interface name.
 *            If the address is found, it will be copied into the specified user buffer.
 **/
int get_my_ip_addr_from_ifname( const char *ifname, char *ipaddr, int ipaddr_len )
{
    int fd = 0;
    struct ifreq ifr;

    if ( ifname && strlen(ifname) )
    {
        fd = socket(AF_INET, SOCK_DGRAM, 0);

        if ( fd )
        {
            ifr.ifr_addr.sa_family = AF_INET; /* I want to get an IPv4 IP address */

            strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);

            ioctl(fd, SIOCGIFADDR, &ifr);

            close(fd);

            if ( ipaddr && ipaddr_len > strlen( inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr)) )
            {
                strncpy( ipaddr, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), ipaddr_len-1 );
            }
        }
    }

    return 0;
}

/**
 *  Function: This function will find the IP4 addresses of all network interfaces.
 **/
int get_my_ip4_addr_all( unsigned long int list[], int list_max )
{
    int             ifa_count = 0;
    struct ifaddrs *ifaddr, *ifa;
    int             s, n;
    char            host[NI_MAXHOST];

    if (getifaddrs( &ifaddr ) == -1)
    {
        fprintf( stderr, "getifaddrs() failed\n" );
        return -1;
    }

    /* loop through all of the interfaces found */
    for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++)
    {
        if (ifa->ifa_addr == NULL) continue;

        if (( ifa->ifa_addr->sa_family == AF_INET ) || ( ifa->ifa_addr->sa_family == AF_INET6 ))
        {
            s = getnameinfo( ifa->ifa_addr, ( ifa->ifa_addr->sa_family == AF_INET ) ? sizeof( struct sockaddr_in ) : sizeof( struct sockaddr_in6 ),
                    host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST );
            if (s != 0)
            {
                fprintf( stderr, "getnameinfo() failed: %s\n", gai_strerror( s ));
                return -1;
            }

            if ( strcmp( host, "127.0.0.1" ) == 0 ) printf( "%-6s <%s> %lX\n", ifa->ifa_name, host, (unsigned long int) inet_addr(host) );
            if ( ifa_count < list_max && inet_addr(host) != 0x100007F ) /* ignore local addr 127.0.0.1 */
            {
                list[ifa_count] = inet_addr(host);

                ifa_count++;
            }
        }
    }

    freeifaddrs( ifaddr );
    return 0;
}
