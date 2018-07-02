/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include <net/if.h>

#include "bmon_utils.h"

const char *noprintf(
    const char *format,
    ...
    )
{
    return( format );
}

const char *nofprintf(
    FILE       *stream,
    const char *format,
    ...
    )
{
    return( format );
}

/**
 *  Function: This function will create a usage string to send back to the browser. It will loop through all of the known
 *            expected fields and list these fields in the usage string.
 **/
int bmon_set_usage_response(
    char       *buffer,
    int         buffer_size,
    const char *Module,
    const char *KnownFields[],
    int         KnownFieldsMax
    )
{
    int idx = 0;

    /*fprintf( stderr, "buffer %p; module %p; KownFields %p; max %d", buffer, Module, KnownFields, KnownFieldsMax );*/
    if (( buffer == NULL ) || ( Module == NULL ) || ( KnownFields == NULL )) {return( 0 ); }

    /*fprintf( stderr, "buffer %p; size %d", buffer, buffer_size ); fflush(stdout); fflush(stderr);*/
    strncat( buffer, "{ \"Usage\":\"/", buffer_size - strlen( buffer ) - 1 );
    strncat( buffer, Module, buffer_size - strlen( buffer ) - 1 );
    strncat( buffer, "/", buffer_size - strlen( buffer ) - 1 );
    /*fprintf( stderr, "KnownFieldsMax %d ", KnownFieldsMax ); fflush(stdout); fflush(stderr);*/
    for (idx = 0; idx<KnownFieldsMax; idx++) {
        if (idx) {strncat( buffer, ",", buffer_size - strlen( buffer ) - 1 ); }
        strncat( buffer, KnownFields[idx], buffer_size - strlen( buffer ) - 1 );
        /*fprintf( stderr, "adding %s ... len now is %d", KnownFields[idx], strlen(buffer) ); fflush(stdout); fflush(stderr);*/
    }
    strncat( buffer, " }", buffer_size - strlen( buffer ) - 1 );

    return( 0 );
}                                                          /* bmon_set_usage_response */

int bmon_trim_line(
    char *line
    )
{
    unsigned int len = 0;

    if (line && strlen( line ))
    {
        len = strlen( line );
        if (line[len-1] == '\n')
        {
            line[len-1] = 0;
        }
    }

    return( 0 );
}  /* bmon_trim_line */

/**
 *  Function: This function will return a string that contains the current date and time.
 **/
char *bmon_date_yyyy_mm_dd_hh_mm_ss(
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
}   /* bmon_date_yyyy_mm_dd_hh_mm_ss */

unsigned long int bmon_get_seconds_since_epoch(
    void
    )
{
    struct timeval tv;

    gettimeofday( &tv, NULL );

    return( tv.tv_sec );
}   /* bmon_get_seconds_since_epoch */

/* this API is overloaded until BWL usage gets integrated into baseline with bmon changes */
char *GetFileContents(
    const char *filename
    )
{
    return ( bmon_get_file_contents(filename) );
}
char *bmon_get_file_contents(
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
#if DEBUG
    fprintf( stderr, "%s - file (%s) malloc(%d) ... contents %p\n", __FUNCTION__, filename, (int) ( statbuf.st_size + 1 ), contents );
#endif /* DEBUG */

    memset( contents, 0, statbuf.st_size + 1 );

    if (statbuf.st_size)
    {
        fpInput = fopen( filename, "r" );
        fread( contents, 1, statbuf.st_size, fpInput );
        fclose( fpInput );
    }

    return( contents );
}                                                          /* bmon_get_file_contents */

/**
 *  Function: This function will return to the user the known temporary path name. In Linux system, this
 *  file will be /tmp/. In Android systems, it will be dictated by the environment variable B_ANDROID_TEMP.
 **/
char *bmon_get_temp_directory_str(
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
        contents = bmon_get_file_contents( "boa.conf" );

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

            FREE_SAFE( contents );
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

    FREE_SAFE( contents );

    PRINTF( "~%s: returning (%s)\n~", __FUNCTION__, tempDirectory );
    return( tempDirectory );
}                                                          /* bmon_get_temp_directory_str */

/**
 *  Function: This function will read in the contents of the specified file without looking
 *            at the file size first. When reading some files in the /proc file system,
 *            the lstat() API always returns a zero length. Some of the files in the /proc
 *            file system still have contents that can be read ... even if the length is 0.
 **/
char *bmon_get_file_contents_proc(
    const char *filename,
    int         max_expected_file_size
    )
{
    char *contents = NULL;
    FILE *fpInput  = NULL;

    contents = malloc( max_expected_file_size + 1 );
    if (contents == NULL) {return( NULL ); }

    memset( contents, 0, max_expected_file_size + 1 );

    fpInput = fopen( filename, "r" );
    if ( fpInput )
    {
        fread( contents, 1, max_expected_file_size, fpInput );
        fclose( fpInput );
    }
    else
    {
        /*fprintf(stderr, "%s - failed to open (%s) for reading\n", __FUNCTION__, filename );fflush(stderr);*/
    }

    return( contents );
}                                                          /* bmon_get_file_contents_proc */

/**
 *  Function: This function will prepend the specified file name with the known temporary path name.
 **/
void bmon_prepend_temp_directory(
    char       *filenamePath,
    int         filenamePathLen,
    const char *filename
    )
{
    if (filenamePath)
    {
        strncpy( filenamePath, bmon_get_temp_directory_str(), filenamePathLen -1 );
        strncat( filenamePath, filename,              filenamePathLen -1 );

        PRINTF( "~%s: returning (%s)\n~", __FUNCTION__, filenamePath );
    }

    return;
}                                                          /* bmon_prepend_temp_directory */

char *bmon_get_time_now_str(
    void
    )
{
    static char    timeStr[64];
    struct timeval tv;

    gettimeofday( &tv, NULL );
    sprintf( timeStr, "%d.%06d", (int) tv.tv_sec, (int) tv.tv_usec );
    return( timeStr );
}

unsigned long int bmon_delta_time_microseconds(
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
    microseconds1      = ( seconds * 1000000LL );          /* q-scale shift the seconds left to allow for addition of microseconds */
    microseconds1     += microseconds;
    microseconds2      = ( tv2.tv_sec * 1000000LL );       /* q-scale shift the seconds left to allow for addition of microseconds */
    microseconds2     += tv2.tv_usec;                      /* add in microseconds */
    microseconds_delta = ( microseconds2 - microseconds1 );
    /*printf( "now: %lu.%06lu ... input: %lu.%06lu ... elapsed time %lu milliseconds\n", tv2.tv_sec, tv2.tv_usec, seconds, microseconds, microseconds_delta/1000 );*/

    return( microseconds_delta );
} /* bmon_delta_time_microseconds */

/**
 *  Function: This function will determine the IP address of the specified interface name.
 *            If the address is found, it will be copied into the specified user buffer.
 **/
int bmon_get_my_ip_addr_from_ifname(
    const char *ifname,
    char       *ipaddr,
    int         ipaddr_len
    )
{
    int          fd = 0;
    int          rc = 0;
    struct ifreq ifr;

    if (ifname && strlen( ifname ))
    {
        fd = socket( AF_INET, SOCK_DGRAM, 0 );

        if (fd)
        {
            ifr.ifr_addr.sa_family = AF_INET;              /* I want to get an IPv4 IP address */

            strncpy( ifr.ifr_name, ifname, IFNAMSIZ-1 );

            rc = ioctl( fd, SIOCGIFADDR, &ifr );

            if ( rc ) {
                fprintf( stderr, "%s - ioctl(SIOCGIFADDR) failed \n", __FUNCTION__ );
            } else {
                fprintf( stderr, "%s - ioctl(SIOCGIFADDR) returned %d\n", __FUNCTION__, rc );
            }

            close( fd );

            if ( rc==0 && ipaddr && ( ipaddr_len > strlen( inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr )->sin_addr ))))
            {
                strncpy( ipaddr, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr )->sin_addr ), ipaddr_len-1 );
            }
        }
    }

    return( 0 );
} /* bmon_get_my_ip_addr_from_ifname */

/**
 *  Function: This function will read the specified configuration file and search for the
 *            specified tag line. If the tag line is found, the value associated with the
 *            tag will be copied to the user's output buffer.
 **/
int bmon_get_cfg_file_entry(
    const char *cfg_filename,
    const char *cfg_tagline,
    char       *output_buffer,
    int         output_buffer_len
    )
{
    FILE *fp = NULL;
    char  oneline[512];

    if (( cfg_filename == NULL ) || ( cfg_tagline == NULL ) || ( output_buffer == NULL ) || ( output_buffer_len <=0 ))
    {
        return( -1 );
    }
    memset( oneline, 0, sizeof( oneline ));

    fp = fopen( cfg_filename, "r" );
    if (fp == NULL)
    {
        return( -1 );
    }

    PRINTF( "%s: tagline (%s)\n", __FUNCTION__, cfg_tagline );
    while (fgets( oneline, sizeof( oneline ), fp ))
    {
        PRINTF( "%s: newline (%s)\n", __FUNCTION__, oneline );
        /* if we found a matching line */
        if (strstr( oneline, cfg_tagline ))
        {
            char *bov = strchr( oneline, '"' );            /* determine the beginning of the assocated value */
            char *eov = NULL;
            if (bov)
            {
                eov = strchr(( bov+1 ), '"' );             /* determine the end of the assocated value */
            }
            if (bov && eov)
            {
                bov++;                                     /* skip the beginning double-quote */
                *eov = '\0';                               /* overwrite the ending double-quote with the string terminator */
                strncpy( output_buffer, bov, output_buffer_len -1 );
                break;
            }
        }
        memset( oneline, 0, sizeof( oneline ));
    }

    fclose( fp );

    return( 0 );
} /* bmon_get_cfg_file_entry */

char *bmon_date_str(
    void
    )
{
    static char    fmt [64];
    struct timeval tv;
    struct tm     *tm;

    memset( fmt, 0, sizeof( fmt ));
    gettimeofday( &tv, NULL );
    if (( tm = gmtime( &tv.tv_sec )) != NULL)
    {
#if 0
        strftime( fmt, sizeof fmt, "%Y%m%d-%H%M%S", tm );
        strftime( fmt, sizeof fmt, "%+", tm );
#else
        strftime( fmt, sizeof fmt, "%a, %d %b %Y %T %Z", tm );
#endif
    }

    return( fmt );
}                                                          /* bmon_date_str */

int bmon_get_mac_addr(
    const char *ifname,
    char       *macAddrBuffer,
    int         macAddrBufferLen
    )
{
    int                 sfd = 0;
    unsigned char      *u   = NULL;
    struct ifreq        ifr;
    struct sockaddr_in *sin = (struct sockaddr_in *) &ifr.ifr_addr;

    memset( &ifr, 0, sizeof ifr );

    if (0 > ( sfd = socket( AF_INET, SOCK_STREAM, 0 )))
    {
        fprintf( stderr, "%s - socket(AF_INET, SOCK_STREAM) Failed\n", __FUNCTION__ );
        return( -1 );
    }

    strcpy( ifr.ifr_name, ifname );
    sin->sin_family = AF_INET;

    if (0 == ioctl( sfd, SIOCGIFADDR, &ifr ))
    {
        /*printf("%s: %sn", ifr.ifr_name, inet_ntoa(sin->sin_addr));*/
    }

    if (0 > ioctl( sfd, SIOCGIFHWADDR, &ifr ))
    {
        fprintf( stderr, "%s - ioctl(sfd, SIOCGIFHWADDR) Failed\n", __FUNCTION__ );
        return( -1 );
    }

    u = (unsigned char *) &ifr.ifr_addr.sa_data;

    if (u[0] + u[1] + u[2] + u[3] + u[4] + u[5])
    {
        snprintf( macAddrBuffer, macAddrBufferLen, "%02x:%02x:%02x:%02x:%02x:%02x", u[0], u[1], u[2], u[3], u[4], u[5] );
    }

    return( 0 );
} /* bmon_get_mac_addr */

int bmon_remove_colons(
    char       *strBuffer
    )
{
    char *newStr = strBuffer;
    char *oldStr = strBuffer;

    if ( strBuffer == NULL ) return 0;

    /* 11:22:33:44 */
    while ( *oldStr ) {
        if ( *oldStr == ':' ) {
            oldStr++;
            //fprintf( stderr, "%s: found colon ... newStr (%s) ... old (%s)\n", __FUNCTION__, newStr, oldStr );
        }
        if ( newStr != oldStr ) *newStr = *oldStr;
        //fprintf( stderr, "%s: return (%s) newStr (%s) ... old (%s)\n", __FUNCTION__, strBuffer, newStr, oldStr );
        oldStr++;
        newStr++;
    }
    *newStr = '\0';
    //fprintf( stderr, "%s: return (%s) \n", __FUNCTION__, strBuffer );

    return( 0 );
} /* bmon_remove_colons */

/**
 *  Function: This function returns to the caller the number of bytes contained in the specified file.
 **/
int bmon_get_file_size(
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
} /* bmon_get_file_size */
