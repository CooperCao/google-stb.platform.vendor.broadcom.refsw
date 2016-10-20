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
#include <fcntl.h>
#include <locale.h>
#include <math.h>
#include <sys/stat.h>
#include <errno.h>
#include <termios.h>
#if 0
#include <signal.h>
#endif
#include <stdint.h>
#include <assert.h>
#include <sys/mman.h>
#include <pthread.h>
#include <dirent.h>

#include "bmemperf.h"
#include "bmemperf_cgi.h"
#include "bmemperf_info.h"
#include "bmemperf_utils.h"

#include "bstd.h"
#include "bkni.h"
#include "bchp_sun_top_ctrl.h"
#if BCHP_MEMC_DDR23_SHIM_ADDR_CNTL_0_REG_START
#include "bchp_memc_ddr23_shim_addr_cntl_0.h"
#endif

#define PRINTFLOG noprintf
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
    /*printf("%s:%u g_pMem %p; offset 0x%x; \n", __FILE__, __LINE__, (void*) g_pMem, offset );
    fflush(stdout);fflush(stderr);*/
    if (BCHP_REGISTER_START & offset)
    {
        offset -= BCHP_REGISTER_START;
        PRINTFLOG( "bmemperf_lib.c:%u - (offset 0x%x); g_pMem %p \n", __LINE__, offset, g_pMem );
    }
    pMemTemp += offset >>2;
    /*printf("%s:%u pMemTemp %p \n", __FILE__, __LINE__, (void*) pMemTemp );
    fflush(stdout);fflush(stderr);*/

    returnValue = *pMemTemp;
    /*printf("%s:%u returnValue 0x%x \n", __FILE__, __LINE__, (unsigned int ) returnValue );
    fflush(stdout);fflush(stderr);*/

    return ( returnValue );
} /* bmemperf_readReg32 */

#ifdef BMEMCONFIG_READ32_SUPPORTED
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
            /*printf("%s:%u\n", __FILE__, __LINE__ );fflush(stdout);fflush(stderr);*/
            familyId = bmemperf_readReg32( BCHP_SUN_TOP_CTRL_CHIP_FAMILY_ID );
            productId = bmemperf_readReg32( BCHP_SUN_TOP_CTRL_PRODUCT_ID );
            /*printf("%s:%u familyId 0x%x; productId 0x%x \n", __FILE__, __LINE__, familyId, productId );*/

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
 *  Function: This function will use the output from the ifconfig utility to determine what our IP4 address is.
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

unsigned long int getPidOf ( const char * processName )
{
    char                line[MAX_LENGTH_GETPIDOF_CMD];
    unsigned long int   pid = 0;
    FILE               *cmd = NULL;

    sprintf(line, "ps -eaf | grep \"%s\" | grep -v grep | awk '{print $2;}'", processName );
    cmd = popen( line, "r" );

    memset(line, 0, sizeof(line) );
    fgets( line, MAX_LENGTH_GETPIDOF_CMD, cmd );
    pid = strtoul( line, NULL, 10 );

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

    sprintf(line, "ps -eaf | grep \"%s\" | grep -v grep", exe_name );
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
        /* only interested in names that start with a number and are at least 4 characters long */
        if ( strlen(entry->d_name) > 3 && '1' <= entry->d_name[0] && entry->d_name[0] <= '9' )
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
