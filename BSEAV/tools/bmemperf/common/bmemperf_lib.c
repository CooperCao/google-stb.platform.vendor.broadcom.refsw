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
#include <sys/types.h>
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
#include <signal.h>
#include <stdint.h>
#include <assert.h>
#include <sys/mman.h>
#include <pthread.h>

#include "bmemperf.h"
#include "bmemperf_cgi.h"
#include "bmemperf_info.h"
#include "bmemperf_utils.h"

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

    PRINTF("%s: reqlen %d; reslen %d; server_port %d\n", __FUNCTION__, request_len, response_len, server_port );
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

        free( newString );
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
    long int numBytes     = 0;
    FILE    *fpProcUptime = NULL;
    char     bufProcStat[256];

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
        printf( "%s: Could not stat (%s)\n", __FUNCTION__, filename );
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

    if (contents)
    {
        free( contents );
    }
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
    unsigned long     numBytes      = 0;
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

    if (numBytes != statbuf.st_size)
    {
        printf( "tried to fread %lu bytes but got %lu\n", (unsigned long int) statbuf.st_size, numBytes );
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

    if (contents) {free( contents ); }

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
