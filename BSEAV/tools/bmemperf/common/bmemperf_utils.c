/******************************************************************************
 * (c) 2014 Broadcom Corporation
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
#include "bstd.h"
#include "bmemperf_server.h"
#include "bmemperf_cgi.h"
#include "bmemperf_utils.h"
#include "bmemperf_info.h"
#include "nexus_platform.h"

#define FILENAME_TAG "filename=\""

#ifdef USE_BOXMODES
extern bmemperf_boxmode_info g_boxmodes[20];
#endif /* USE_BOXMODES */
extern int   g_MegaBytes;
extern int   g_MegaBytesDivisor[2];
extern char *g_MegaBytesStr[2];

#define UTILS_LOG_FILE_FULL_PATH_LEN 64

extern char         *g_client_name[BMEMPERF_MAX_NUM_CLIENT];
extern bmemperf_info g_bmemperf_info;

const char *nofprintf(
    FILE       *stdsomething,
    const char *format,
    ...
    )
{
    if (stdsomething == NULL)
    {
    }
    return( format );
}

char *getClientName(
    int client_index
    )
{
    return( &g_client_name[client_index][0] );
} /* getClientName */

pid_t daemonize(
    void
    )
{
    pid_t pid = 0, sid = 0;
    int   fd  = 0;

    /* already a daemon */
    if (getppid() == 1)
    {
        printf( "%s: already a daemon\n", __FUNCTION__ );
        return( 0 );
    }

    /* Fork off the parent process */
    pid = fork();
    if (pid < 0)
    {
        printf( "%s: fork() command failed\n", __FUNCTION__ );
        exit( EXIT_FAILURE );
    }

    if (pid > 0)
    {
        /*printf( "%u: This is the parent; exiting immediately\n", pid );*/
    }
    else
    {
        char utilsLogFilename[UTILS_LOG_FILE_FULL_PATH_LEN];

        /*printf( "%u: This is the child; starting daemonize process\n", pid );*/

        /* At this point we are executing as the child process */
        /* Create a new SID for the child process */
        sid = setsid();
        if (sid < 0)
        {
            exit( EXIT_FAILURE );
        }

        printf( "%s: new sid is %d\n", __FUNCTION__, sid );
        /* Change the current working directory. */
        if (( chdir( GetTempDirectoryStr())) < 0)
        {
            exit( EXIT_FAILURE );
        }

        fd = open( "/dev/null", O_RDWR, 0 );

        if (fd != -1)
        {
            /*printf( "%u: Assign STDIN to /dev/null\n", pid );*/
            dup2( fd, STDIN_FILENO );

            if (fd > 2)
            {
                close( fd );
            }
        }

        PrependTempDirectory( utilsLogFilename, sizeof( utilsLogFilename ), "bmemperf_utils.log" );
        fd = open( utilsLogFilename, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR  );

        if (fd != -1)
        {
            /*printf( "%u: Assign STDOUT/STDERR to fd (%u)\n", pid, fd );*/
            dup2( fd, STDOUT_FILENO );
            dup2( fd, STDERR_FILENO );
        }

        /*reset file creation mask */
        umask( 0177 );
    }

    return( pid );
} /* daemonize */

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
    unsigned int newlen           = 0;
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
        printf( "%s: Could not stat (%s)\n", __FUNCTION__, tempFilename ); fflush( stdout ); fflush( stderr );
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
    newlen = strlen( pos ) + 1 /* null string terminator */;

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

    if (pFileNameDecoded) {free( pFileNameDecoded ); }
    if (contents) {free( contents ); }

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
        if (cgi_query)
        {
            free( cgi_query );
        }
    }

    return( 0 );
} /* readFileFromBrowser */

int sort_on_id(
    const void *a,
    const void *b
    )
{
    int rc = 0;

    bmemperf_client_data *client_a = (bmemperf_client_data *)a;
    bmemperf_client_data *client_b = (bmemperf_client_data *)b;

    rc = client_b->client_id - client_a->client_id;

    return( rc );
} /* sort_on_id */

int sort_on_id_rev(
    const void *a,
    const void *b
    )
{
    return( sort_on_id( b, a ));
} /* sort_on_id_rev */

int sort_on_bw(
    const void *a,
    const void *b
    )
{
    int rc = 0;

    bmemperf_client_data *client_a = (bmemperf_client_data *)a;
    bmemperf_client_data *client_b = (bmemperf_client_data *)b;

    rc = client_b->bw - client_a->bw;

    return( rc );
} /* sort_on_bw */

int sort_on_bw_rev(
    const void *a,
    const void *b
    )
{
    return( sort_on_bw( b, a ));
} /* sort_on_bw_rev */

int changeFileExtension(
    char       *destFilename,
    int         sizeofDestFilename,
    const char *srcFilename,
    const char *fromext,
    const char *toext
    )
{
    char *pos = NULL;

    PRINTF( "~%s: strcpy(%s)(%s)(%d)\n~", __FUNCTION__, destFilename, srcFilename, sizeofDestFilename );
    strncpy( destFilename, srcFilename, sizeofDestFilename );

    /* change the extension to new one ...   .tgz to 2.csv */
    pos = strstr( destFilename, fromext );
    PRINTF( "~%s: dest (%s)(%p); pos (%p); size (%d)\n~", __FUNCTION__, destFilename, destFilename, pos, sizeofDestFilename );
    PRINTF( "~%s: src  (%s)(%p); from (%s); to (%s)\n~", __FUNCTION__, srcFilename, srcFilename, fromext, toext );
    if (pos)
    {
        pos[0] = '\0'; /* null-terminate the beginning part of the name */
        strncat( destFilename, toext, sizeofDestFilename - strlen( destFilename ));
    }

    PRINTF( "destfilename (%s)\n", destFilename );

    return( 0 );
} /* changeFileExtension */

/**
 *  Function: This function will create the HTML needed to display the Top X clients.
 **/
int clientListCheckboxes(
    unsigned int top10Number
    )
{
    unsigned int idx = 0, memc = 0, client_idx = 0, displayCount[NEXUS_NUM_MEMC];

    printf( "~CLIENT_LIST_CHECKBOXES~" );
    PRINTF( "top10Number %u<br>\n", top10Number );

    memset( displayCount, 0, sizeof( displayCount ));

#define COLUMNS_PER_MEMC 2
    printf( "<table style=border-collapse:collapse; border=1 >\n" );
    for (idx = 0; idx<top10Number+2 /*there are two header rows */; idx++)
    {
        printf( "<tr>" );
        for (memc = 0; memc<g_bmemperf_info.num_memc; memc++)
        {
            /* 1st row */
            if (idx==0)
            {
                printf( "<th colspan=%u >MEMC %u</th>", COLUMNS_PER_MEMC, memc );
            }
            /* 2nd row */
            else if (idx==1)
            {
                printf( "<th style=\"width:50px;\"  >ID</th>" );
                printf( "<th style=\"width:150px;\" align=left >Name</th>" );
            }
            else
            {
                client_idx = idx-2;
                if (( strncmp( g_client_name[client_idx], "sage", 4 ) != 0 ) && ( strncmp( g_client_name[client_idx], "bsp", 3 ) != 0 ))
                {
                    printf( "<td id=client%umemc%ubox align=left ><input type=checkbox id=client%umemc%u onclick=\"MyClick(event);\" >%u</td>",
                        client_idx, memc, client_idx, memc, client_idx );
                    printf( "<td id=client%umemc%uname >%s</td>", client_idx, memc, g_client_name[client_idx] );
                }
            }
        }
        PRINTF( "<br>\n" );
        printf( "</tr>\n" );
    }
    printf( "</table>\n" );
    return( 0 );
} /* clientListCheckboxes */

int getCpuOnline(
    bmemperf_cpu_status *pCpuStatus
    )
{
    unsigned int cpu = 0;
    FILE        *cmd = NULL;
    char         line[MAX_LENGTH_CPU_STATUS];

    if (pCpuStatus == NULL)
    {
        return( -1 );
    }

    /* clear out the array */
    memset( pCpuStatus, 0, sizeof( *pCpuStatus ));

    sprintf( line, "grep processor /proc/cpuinfo" );
    cmd = popen( line, "r" );

    PRINTF( "detected cpu: " );
    do {
        memset( line, 0, sizeof( line ));
        fgets( line, MAX_LENGTH_CPU_STATUS, cmd );
        if (strlen( line ))
        {
            char *pos = strchr( line, ':' );
            if (pos)
            {
                pos++;
                cpu = strtoul( pos, NULL, BMEMPERF_CPU_VALUE_LENGTH );
                PRINTF( "%u ", cpu );
                if (cpu < BMEMPERF_MAX_NUM_CPUS)
                {
                    pCpuStatus->isActive[cpu] = true;
                }
            }
        }
    } while (strlen( line ));
    PRINTF( "\n" );

    pclose( cmd );
    return( 0 );
} /* getCpuOnline */

#ifdef USE_BOXMODES
int bmemperf_boxmode_init(
    long int boxmode
    )
{
    unsigned int idx         = 0;
    bool         bMatchFound = false;

    PRINTF( "~boxmode:%ld; before ... %u, %u, %u\n~", boxmode, g_bmemperf_info.num_memc, g_bmemperf_info.ddrFreqInMhz, g_bmemperf_info.scbFreqInMhz );

    if (boxmode > 0)
    {
        /* loop through all boxmodes to find the client names, MHz, and SCB freq for the current box mode */
        for (idx = 0; idx<( sizeof( g_boxmodes )/sizeof( g_boxmodes[0] )); idx++)
        {
            if (( g_boxmodes[idx].boxmode > 0 ) && g_boxmodes[idx].g_bmemperf_info && g_boxmodes[idx].g_client_name)
            {
                if (boxmode == (long int) g_boxmodes[idx].boxmode)
                {
                    printf( "~boxmode:%ld; copying from idx %u\n~", boxmode, idx );
                    memcpy( &g_bmemperf_info, g_boxmodes[idx].g_bmemperf_info, sizeof( g_bmemperf_info ));
                    printf( "~boxmode:%ld; %u, %u, %u\n~", boxmode, g_bmemperf_info.num_memc, g_bmemperf_info.ddrFreqInMhz, g_bmemperf_info.scbFreqInMhz );
                    memcpy( &g_client_name, g_boxmodes[idx].g_client_name, sizeof( g_client_name ));
                    boxmode     = g_boxmodes[idx].boxmode;
                    bMatchFound = true;
                    break;
                }
            }
            else
            {
                printf( "~boxmode:%ld; stopped looking at idx %u\n~", boxmode, idx );
                break;
            }
        }
    }

    /* if a match was not found above, then use the first entry in the array */
    if (bMatchFound == false)
    {
        idx = 0;
        printf( "~boxmode:%ld; copying from default idx %u\n~", boxmode, idx );
        memcpy( &g_bmemperf_info, g_boxmodes[idx].g_bmemperf_info, sizeof( g_bmemperf_info ));
        memcpy( &g_client_name, g_boxmodes[idx].g_client_name, sizeof( g_client_name ));
        boxmode = g_boxmodes[idx].boxmode;
    }

    PRINTF( "~boxmode: returning %ld\n~", boxmode );
    return( boxmode );
} /* bmemperf_boxmode_init */

int getBoxModeHtml(
    char *buf,
    int   len,
    int   boxmodePlatform
    )
{
    unsigned int            idx       = 0;
    int                     remaining = len;
    int                     written   = 0;
    char                   *output    = buf;
    bmemperf_boxmode_source boxmodeSource;
    char                   *boxmodeSources[BOXMODE_SOURCES_NUM] = {"NEXUS", "BOLT", "BROWSER"};
    char                    boxmodeSourceStr[16];

    memset( buf, '\0', len );
    memset( &boxmodeSource, 0, sizeof( boxmodeSource ));
    memset( &boxmodeSourceStr, 0, sizeof( boxmodeSourceStr ));

    get_boxmode( &boxmodeSource );

    written    = snprintf( output, remaining, "<select id=\"boxmode\" onchange=\"MyClick(event);\" onmousedown=\"MySelect(event);\" >" );
    remaining -= written;
    output    += written;

    /* loop through all boxmodes to find the client names, DDR freq, and SCB freq for the current box mode */
    for (idx = 0; idx<( sizeof( g_boxmodes )/sizeof( g_boxmodes[0] )); idx++)
    {
        if (g_boxmodes[idx].boxmode == boxmodeSource.boxmode)
        {
            snprintf( boxmodeSourceStr, sizeof( boxmodeSourceStr ), "  (%s)", boxmodeSources[boxmodeSource.source] );
        }
        else
        {
            boxmodeSourceStr[0] = '\0';
        }
        if (( g_boxmodes[idx].boxmode > 0 ) && g_boxmodes[idx].g_bmemperf_info && g_boxmodes[idx].g_client_name)
        {
            written = snprintf( output, remaining, "<option value=%d %s >BoxMode %d%s</option>", g_boxmodes[idx].boxmode,
                    ( boxmodePlatform == g_boxmodes[idx].boxmode ) ? "selected" : "", g_boxmodes[idx].boxmode, boxmodeSourceStr );

            if (written>=remaining)
            {
                remaining = 0;
                break;
            }
            remaining -= written;
            output    += written;
        }
        else
        {
            break;
        }
    }

    written    = snprintf( output, remaining, "</select >" );
    remaining -= written;
    output    += written;

    /*printf("%s",buf);*/
    return( len-remaining );
} /* getBoxModeHtml */

#endif /* USE_BOXMODES */

int overall_stats_html(
    bmemperf_response *pResponse
    )
{
    unsigned int      memc = 0;
    unsigned int      cpu  = 0;
    unsigned int      numProcessorsConfigured = sysconf( _SC_NPROCESSORS_CONF );
    unsigned int      numProcessorsOnline     = sysconf( _SC_NPROCESSORS_ONLN );
    float             averageCpuUtilization   = 0.0;
    bmemperf_irq_data irqData;
    char              irqTotalStr[64];
    unsigned int      numberOfMemc = g_bmemperf_info.num_memc;

    PRINTF( "%s: numberOfMemc %u<br>\n", __FUNCTION__, numberOfMemc );
    printf( "~OVERALL~" );
    printf( "<table ><tr><th>OVERALL STATISTICS:</th></tr></table>\n" );
    printf( "<table style=border-collapse:collapse; border=1 >\n" );

    memset( &irqData, 0, sizeof( irqData ));
    memset( &irqTotalStr, 0, sizeof( irqTotalStr ));

    for (memc = 0; memc<numberOfMemc; memc++)
    {
        /* first column */
        if (memc == 0)
        {
            printf( "<tr><th>&nbsp;</th>" );
        }
        printf( "<th>MEMC&nbsp;%u</th>", memc );

        /* last column */
        if (( memc+1 ) == numberOfMemc)
        {
            printf( "</tr>\n" );
        }
    }
    for (memc = 0; memc<numberOfMemc; memc++)
    {
        /* first column */
        if (memc == 0)
        {
            printf( "<tr><td>Data BW&nbsp;(%s)</td>", g_MegaBytesStr[g_MegaBytes] );
        }
        printf( "<td>%u</td>", pResponse->response.overallStats.systemStats[memc].dataBW / g_MegaBytesDivisor[g_MegaBytes] );

        /* last column */
        if (( memc+1 ) == numberOfMemc)
        {
            printf( "</tr>\n" );
        }
    }
    for (memc = 0; memc<numberOfMemc; memc++)
    {
        /* first column */
        if (memc == 0)
        {
            printf( "<tr><td>Transaction&nbsp;BW&nbsp;(%s)</td>", g_MegaBytesStr[g_MegaBytes] );
        }
        printf( "<td>%u</td>", pResponse->response.overallStats.systemStats[memc].transactionBW / g_MegaBytesDivisor[g_MegaBytes] );

        /* last column */
        if (( memc+1 ) == numberOfMemc)
        {
            printf( "</tr>\n" );
        }
    }
    for (memc = 0; memc<numberOfMemc; memc++)
    {
        /* first column */
        if (memc == 0)
        {
            printf( "<tr><td>Idle&nbsp;BW&nbsp;(%s)</td>", g_MegaBytesStr[g_MegaBytes] );
        }
        printf( "<td>%u</td>", pResponse->response.overallStats.systemStats[memc].idleBW / g_MegaBytesDivisor[g_MegaBytes] );

        /* last column */
        if (( memc+1 ) == numberOfMemc)
        {
            printf( "</tr>\n" );
        }
    }
    for (memc = 0; memc<numberOfMemc; memc++)
    {
        /* first column */
        if (memc == 0)
        {
            printf( "<tr><td>Utilization %%</td>" );
        }
        printf( "<td>%5.2f</td>", pResponse->response.overallStats.systemStats[memc].dataUtil );

        /* last column */
        if (( memc+1 ) == numberOfMemc)
        {
            printf( "</tr>\n" );
        }
    }
    for (memc = 0; memc<numberOfMemc; memc++)
    {
        /* first column */
        if (memc == 0)
        {
            printf( "<tr><td>DDR&nbsp;Freq&nbsp;(MHz)</td>" );
        }
        printf( "<td>%u</td>", pResponse->response.overallStats.systemStats[memc].ddrFreqMhz );

        /* last column */
        if (( memc+1 ) == numberOfMemc)
        {
            printf( "</tr>\n" );
        }
    }
    for (memc = 0; memc<numberOfMemc; memc++)
    {
        /* first column */
        if (memc == 0)
        {
            printf( "<tr><td>SCB&nbsp;Freq&nbsp;(MHz)</td>" );
        }
        printf( "<td>%u</td>", pResponse->response.overallStats.systemStats[memc].scbFreqMhz );

        /* last column */
        if (( memc+1 ) == numberOfMemc)
        {
            printf( "</tr>\n" );
        }
    }

    printf( "</table>~" );

    /* convert 999999 to 999,999 ... e.g. add commas */
    convert_to_string_with_commas( pResponse->response.overallStats.irqData.irqTotal,  irqTotalStr, sizeof( irqTotalStr ));

    printf( "~DETAILSTOP~" );
    printf( "<table border=0 style=\"border-collapse:collapse;\" width=\"100%%\" ><tr><th width=160px align=left >CLIENT DETAILS:</th>" );
    printf( "<th align=right ><table border=0 style=\"border-collapse:collapse;\" ><tr><th align=left>CPU&nbsp;UTILIZATION:&nbsp;&nbsp;</th>" );
    for (cpu = 0; cpu<numProcessorsConfigured; cpu++)
    {
        /* 255 indicates this CPU is offline */
        if (pResponse->response.overallStats.cpuData.idlePercentage[cpu] != 255)
        {
            averageCpuUtilization += pResponse->response.overallStats.cpuData.idlePercentage[cpu];
            printf( "<td align=center style=\"width:50px;border-top: solid thin; border-right: solid thin; border-left: solid thin; border-bottom: solid thin\" >%u%%</td>",
                pResponse->response.overallStats.cpuData.idlePercentage[cpu] );
        }
    }
    averageCpuUtilization /= numProcessorsOnline;
    printf( "<td align=center style=\"width:50px;border-top: solid thin; border-right: solid thin; border-left: solid thin; border-bottom: solid thin; "
            "background-color:lightgray;\"  >%5.1f%%</td>", averageCpuUtilization );
    printf( "<td>&nbsp;&nbsp;Irq/sec:&nbsp;&nbsp;</td>"
            "<td align=center style=\"width:50px;border-top: solid thin; border-right: solid thin; border-left: solid thin; border-bottom: solid thin; "
            "background-color:lightgray;\"  >%s</td>", irqTotalStr );
    printf( "</tr></table></th></tr></table>~\n" );

    return( 0 );
} /* overall_stats_html */

unsigned int getNextNonZeroIdx(
    unsigned int       displayCount,
    unsigned int       memc,
    bmemperf_response *pResponse,
    int                sortColumn
    )
{
    unsigned int client;
    unsigned int nonZeroCount = 0;

    BSTD_UNUSED( sortColumn );

    for (client = 0; client<BMEMPERF_MAX_NUM_CLIENT; client++)
    {
        /* if bandwidth is non-zero OR we are sorting on ID column or NAME column */
        /*if (pResponse->response.overallStats.clientOverallStats[memc].clientData[client].bw > 0 || abs(sortColumn) != BMEMPERF_COLUMN_BW )*/
        if (pResponse->response.overallStats.clientOverallStats[memc].clientData[client].client_id != BMEMPERF_CGI_INVALID)
        {
            if (nonZeroCount == displayCount)
            {
                PRINTF( "%s: for memc %u, idx %u\n", __FUNCTION__, memc, client );
                break;
            }
            nonZeroCount++;
        }
    }
    return( client );
} /* getNextNonZeroIdx */

/**
 *  Function: This function will create the HTML needed to display the Top X clients.
 **/
int top10_html(
    bmemperf_response *pResponse,
    unsigned int       top10Number
    )
{
    unsigned int idx = 0, memc = 0, client_idx = 0, displayCount[NEXUS_NUM_MEMC];
    char         client_checked[8];
    unsigned int numberOfMemc = g_bmemperf_info.num_memc;

    printf( "%s: top10Number %u; numberOfMemc %u\n", __FUNCTION__, top10Number, numberOfMemc );
    printf( "~TOP10~" );

    for (memc = 0; memc<numberOfMemc; memc++)
    {
        displayCount[memc] = 0;
    }

#define TOP_COLUMNS_PER_MEMC 5
    printf( "<table style=border-collapse:collapse; border=1 >\n" );
    for (idx = 0; idx<top10Number+2 /*there are two header rows */; idx++)
    {
        printf( "<tr>" );
        for (memc = 0; memc<numberOfMemc; memc++)
        {
            /* 1st row */
            if (idx==0)
            {
                printf( "<th colspan=%u >MEMC %u</th>", TOP_COLUMNS_PER_MEMC, memc );
            }
            /* 2nd row */
            else if (idx==1)
            {
                /* negative values mean sort the opposite direction */
                int temp = (int) pResponse->response.overallStats.sortColumn[memc];
                PRINTF( "%s: sortColumn temp %d\n", __FUNCTION__, temp );
                printf( "<th style=\"width:50px;\"  id=sort%uid   onclick=\"MyClick(event);\" >ID</th>", memc );
                printf( "<th style=\"width:150px;\" id=sort%uname onclick=\"MyClick(event);\" align=left >Name</th>", memc );
                printf( "<th style=\"width:40px;\"><table><tr><th id=sort%ubw   onclick=\"MyClick(event);\" >BW&nbsp;(%s)</th></tr></table></th>", memc,
                    g_MegaBytesStr[g_MegaBytes] );
                printf( "<th>RR</th><th>Block&nbspOut</th>\n" );
            }
            else
            {
                client_idx = idx-2;
                {
                    char        *background     = NULL;
                    unsigned int background_len = 0;
                    char         rr_checked[8];
                    unsigned int nonZeroIdx = 0;
                    bool         isChecked  = 0;
                    int          temp       = (int) pResponse->response.overallStats.sortColumn[memc];

                    nonZeroIdx = getNextNonZeroIdx( displayCount[memc], memc, pResponse, temp );

                    if (memc==0) {PRINTF( "%s: idx %u: client %u: memc %u; displayCount %u; nonZeroIdx %u;<br>\n", __FUNCTION__, idx, client_idx, memc, displayCount[memc], nonZeroIdx ); }
                    /* if BW is zero OR name contains unprintable character, do not print anything */
                    if (nonZeroIdx >= BMEMPERF_MAX_NUM_CLIENT)
                    {
                        printf( "<td colspan=%u>&nbsp;</td>\n", TOP_COLUMNS_PER_MEMC );
                    }
                    else
                    {
                        isChecked = pResponse->response.overallStats.clientOverallStats[memc].clientData[nonZeroIdx].is_detailed;
                        if (memc==0)
                        {
                            PRINTF( "memc %u; cnt:%u-nonZero %u; id %u; chkd-%u;<br>", memc, displayCount[memc], nonZeroIdx,
                                pResponse->response.overallStats.clientOverallStats[memc].clientData[nonZeroIdx].client_id, isChecked );
                        }
                        memset( client_checked, 0, sizeof( client_checked ));
                        memset( rr_checked, 0, sizeof( rr_checked ));

                        if (isChecked)
                        {
                            strncpy( client_checked, "checked", sizeof( client_checked ));
                        }

                        if (pResponse->response.overallStats.clientOverallStats[memc].clientData[nonZeroIdx].rr)
                        {
                            strncpy( rr_checked, "checked", sizeof( rr_checked ));
                        }

                        if (pResponse->response.overallStats.clientOverallStats[memc].clientData[nonZeroIdx].err_cnt)
                        {
                            background_len = strlen( BACKGROUND_RED ) + 1;
                            background     = malloc( background_len );
                            strncpy( background, BACKGROUND_RED, background_len-1 );
                            background[background_len-1] = '\0';
                        }
                        else if (isChecked)
                        {
                            background_len = strlen( BACKGROUND_YELLOW ) + 1;
                            background     = malloc( background_len );
                            strncpy( background, BACKGROUND_YELLOW, background_len-1 );
                            background[background_len-1] = '\0';
                        }
                        else
                        {
                            background_len = strlen( BACKGROUND_WHITE ) + 1;
                            background     = malloc( background_len );
                            strncpy( background, BACKGROUND_WHITE, background_len-1 );
                            background[background_len-1] = '\0';
                        }
                        printf( "<td align=left %s ><input type=checkbox id=client%umemc%u onclick=\"MyClick(event);\" %s >%u</td>", background,
                            pResponse->response.overallStats.clientOverallStats[memc].clientData[nonZeroIdx].client_id, memc, client_checked,
                            pResponse->response.overallStats.clientOverallStats[memc].clientData[nonZeroIdx].client_id );
                        printf( "<td %s >%s",
                            background, g_client_name[pResponse->response.overallStats.clientOverallStats[memc].clientData[nonZeroIdx].client_id] );
                        if (pResponse->response.overallStats.clientOverallStats[memc].clientData[nonZeroIdx].err_cnt)
                        {
                            printf( " (%u)", pResponse->response.overallStats.clientOverallStats[memc].clientData[nonZeroIdx].err_cnt );
                        }
                        printf( "</td><td align=center %s >%u</td>",
                            background, pResponse->response.overallStats.clientOverallStats[memc].clientData[nonZeroIdx].bw / g_MegaBytesDivisor[g_MegaBytes] );
                        printf( "<td %s ><input type=checkbox id=client%urrobin%u %s onclick=\"MyClick(event);\" ></td>",
                            background, pResponse->response.overallStats.clientOverallStats[memc].clientData[nonZeroIdx].client_id, memc, rr_checked );
                        printf( "<td %s ><input type=text %s id=client%ublockout%u onmouseover=\"MyMouseOver(event);\" onmouseout=\"MyMouseOut(event);\" size=5 value=%u "
                                "onchange=\"MyChange(event);\" /></td>\n", background, background,
                            pResponse->response.overallStats.clientOverallStats[memc].clientData[nonZeroIdx].client_id, memc,
                            pResponse->response.overallStats.clientOverallStats[memc].clientData[nonZeroIdx].block_out );

                        if (background)
                        {
                            free( background );
                        }
                    }

                    displayCount[memc]++;
                }
            }
        }
        PRINTF( "<br>\n" );
        printf( "</tr>\n" );
    }
    printf( "</table>\n" );
    return( 0 );
} /* top10_html */

int get_boxmode(
    bmemperf_boxmode_source *pboxmode_info
    )
{
    unsigned    boxMode = 0;
    const char *override;
    FILE       *pFile = NULL;

    if (pboxmode_info == NULL)
    {
        return( -1 );
    }

    override = getenv( "B_REFSW_BOXMODE" );
    if (override)
    {
        boxMode = atoi( override );
    }
    if (!boxMode)
    {
        pFile = fopen( BOXMODE_RTS_FILE, "r" );
        pboxmode_info->source = BOXMODE_RTS;
    }
    else
    {
        pboxmode_info->source = BOXMODE_ENV;
    }
    if (pFile)
    {
        char buf[64];
        if (fgets( buf, sizeof( buf ), pFile ))
        {
            /* example: 20140402215718_7445_box1, but skip over everything (which may change) and get to _box# */
            char *p = strstr( buf, "_box" );
            if (p)
            {
                boxMode = atoi( &p[4] );
            }
        }
        fclose( pFile );
    }
    pboxmode_info->boxmode = boxMode;
    return( 0 );
} /* get_boxmode */

/**
 *  Function: This function will scan the specified string and return true if a number digit is found; it will
 *  return false otherwise.
 **/
bool hasNumeric(
    const char *mystring
    )
{
    const char *nextchar = mystring;

    while (*nextchar != '\0')
    {
        if (( '0' <= *nextchar ) && ( *nextchar <= '9' ))
        {
            return( 1 );
        }
        nextchar++;
        /*printf("nextchar (%c)\n", *nextchar );*/
    }

    /*printf("string (%s) has no numbers\n", mystring );*/
    return( 0 );
} /* hasNumeric */
