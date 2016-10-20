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

#include "bmemperf.h"
#include "bmemperf_lib.h"
#include "bconsole.h"

char g_uuid[36];

char         *g_client_name[BMEMPERF_MAX_NUM_CLIENT];
int           g_MegaBytes           = 0;                   /* set to 1 when user wants data displayed in megabytes instead of megabits (default) */
int           g_MegaBytesDivisor[2] = {1, 8};
char         *g_MegaBytesStr[2] = {"Mbps", "MBps"};
char         *g_client_name[BMEMPERF_MAX_NUM_CLIENT];
bmemperf_info g_bmemperf_info;
bool          gPerfError = false;

static void negotiate(
    int            sock,
    unsigned char *buf,
    int            len
    )
{
    int i;

    if (( buf[1] == DO ) && ( buf[2] == CMD_WINDOW_SIZE ))
    {
        unsigned char tmp1[10] = {255, 251, 31};
        if (send( sock, tmp1, 3, 0 ) < 0) {exit( 1 ); }

        unsigned char tmp2[10] = {255, 250, 31, 0, 80, 0, 24, 255, 240};
        if (send( sock, tmp2, 9, 0 ) < 0) {exit( 1 ); }
        return;
    }

    for (i = 0; i < len; i++)
    {
        if (buf[i] == DO) {buf[i] = WONT; }
        else if (buf[i] == WILL)
        {
            buf[i] = DO;
        }
    }

    if (send( sock, buf, len, 0 ) < 0) {exit( 1 ); }

    return;
}                                                          /* negotiate */

static struct termios tin;

static void terminal_set(
    void
    )
{
    // save terminal configuration
    tcgetattr( STDIN_FILENO, &tin );

    static struct termios tlocal;
    memcpy( &tlocal, &tin, sizeof( tin ));
    cfmakeraw( &tlocal );
    tcsetattr( STDIN_FILENO, TCSANOW, &tlocal );
}

static void terminal_reset(
    void
    )
{
    // restore terminal upon exit
    tcsetattr( STDIN_FILENO, TCSANOW, &tin );
}

/* ******************************************************************************************************
*                                                                                                       *
* Function - ttyConsole_log                                                                             *
*            Uses the same input style as printf().                                                     *
*                                                                                                       *
********************************************************************************************************/
static void ttyConsole_log(
    const char *szFormat,
    ...
    )
{
    char                 str[256];
    unsigned long int    nLen           = sizeof( str );
    static unsigned char LogFileNameSet = 0;
    static char          sLogFile[128];
    int                  fd = 0;
    char                 tempFilename1[TEMP_FILE_FULL_PATH_LEN];

    memset( str, 0, sizeof str );
    if (LogFileNameSet == 0)
    {
        memset( sLogFile, 0, sizeof sLogFile );
        sprintf( tempFilename1, "bconsole_tty0_%s.log", g_uuid );
        PrependTempDirectory( sLogFile, sizeof( sLogFile ), tempFilename1 );

        fprintf( stderr, "%s: log file is (%s)\n", __FUNCTION__, sLogFile );
        LogFileNameSet = 1;
    }

    va_list arg_ptr;
    va_start( arg_ptr, szFormat );
    vsnprintf( str, nLen-1, szFormat, arg_ptr );
    va_end( arg_ptr );

    if (strlen( sLogFile ))
    {
        fd = open( sLogFile, ( O_CREAT | O_WRONLY | O_APPEND ));
        if (fd)
        {
            write( fd, str, strlen( str ));
            close( fd );
            chmod( sLogFile, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH );

            /* output the string to terminal */
            fprintf( stderr, "%s", str );
        }
        else
        {
            fprintf( stderr, "%s: Could not open (%s)\n", __FUNCTION__, sLogFile );
        }
    }
    else
    {
        fprintf( stderr, "%s: log file (%s) is invalid\n", __FUNCTION__, sLogFile );
        fprintf( stderr, "%s", str );
    }

    return;
}                                                          /* ttyConsole_log */

static int fifo_write(
    int         fd,
    const char *buffer,
    int         buffer_len
    )
{
    fprintf( stderr, "Writing len %d ... to fd %d ... (%s) \n", buffer_len, fd, buffer );
    write( fd, buffer, buffer_len );
    return( 0 );
}

static int process_command(
    const char *ttyCommand
    )
{
    int  pipe;
    int  numbytes = strlen( ttyCommand );
    char tempFilename1[TEMP_FILE_FULL_PATH_LEN];
    char tempFilename2[TEMP_FILE_FULL_PATH_LEN];

    sprintf( tempFilename1, "%s_%s", PIPE_IN, g_uuid );
    PrependTempDirectory( tempFilename2, sizeof( tempFilename2 ), tempFilename1 );

    pipe = open( tempFilename2, O_WRONLY );
    /*printf( "~DEBUG~%s: open(%s) O_WRONLY returned pipe %d ... numbytes %d \n", __FUNCTION__, tempFilename2, pipe, numbytes );*/

    if (pipe > 0)
    {
        if (numbytes)
        {
            /* special case: user entered return into an empty input element */
            if (( strlen( ttyCommand ) == 1 ) && ( ttyCommand[0] == '.' ))
            {
            }
            else
            {
                fifo_write( pipe, ttyCommand, numbytes );
            }
            fifo_write( pipe, "\n\r", 2 );
        }
        close( pipe );
    }
    else
    {
        printf( "~DEBUG~%s: open(%s) O_WRONLY FAILED ... numbytes %d \n", __FUNCTION__, tempFilename2, numbytes );
    }
    return( 0 );
}                                                          /* process_command */

static void *ttyConsole_thread(
    int readKlog_thread_id
    )
{
    int                sock;
    int                max_fd = 0;
    struct sockaddr_in server;
    unsigned char      buf[BUFLEN + 1];
    int                len;
    int                port     = 23;
    int                pipes[2] = {0, 0};
    struct timeval     ts;
    int                ldaemon = daemon( 1, 1 );
    char               one_line[128];
    char               pipe0Filename[TEMP_FILE_FULL_PATH_LEN];
    char               pipe1Filename[TEMP_FILE_FULL_PATH_LEN];

    ttyConsole_log( "%s is pid %d ... daemon %d \n", __FUNCTION__, getpid(), ldaemon );
    do
    {
        //Create socket
        sock = socket( AF_INET, SOCK_STREAM, 0 );
        if (sock == -1)
        {
            ttyConsole_log( "Could not create socket. Error" );
            break;
        }

        server.sin_addr.s_addr = inet_addr( "127.0.0.1" );
        server.sin_family      = AF_INET;
        server.sin_port        = htons( port );

        ttyConsole_log( "connect() to addr 0x%lx using port %d \n", (long int) server.sin_addr.s_addr, port );
        //Connect to remote server
        if (connect( sock, (struct sockaddr *)&server, sizeof( server )) < 0)
        {
            ttyConsole_log( "connect failed. Error\n" );
            break;
        }
        ttyConsole_log( "Connected...\n" );

        // set terminal
        terminal_set();
        atexit( terminal_reset );

        ts.tv_sec  = 1;                                    // 1 second
        ts.tv_usec = 0;

        /* create two FIFOs (named pipes) ... one for pseudo keyboard into telnet client ... one for pseudo display output from telnet */
        sprintf( one_line, "%s_%s", PIPE_IN, g_uuid );
        PrependTempDirectory( pipe0Filename, sizeof( pipe0Filename ), one_line );
        remove( pipe0Filename );
        mkfifo( pipe0Filename, 0666 );

        pipes[0] = open( pipe0Filename, O_RDWR );
        if (pipes[0] <= 0)
        {
            ttyConsole_log( "open(%s) failed. Error\n", pipe0Filename );
            break;
        }
        ttyConsole_log( "open(%s) successful ... %d \n", pipe0Filename, pipes[0] );

        sprintf( one_line, "%s_%s", PIPE_OUT, g_uuid );
        PrependTempDirectory( pipe1Filename, sizeof( pipe1Filename ), one_line );
        remove( pipe1Filename );
        mkfifo( pipe1Filename, 0666 );

        pipes[1] = open( pipe1Filename, O_RDWR );
        if (pipes[1] <= 0)
        {
            close( pipes[0] );
            ttyConsole_log( "open(%s) failed. Error\n", pipe1Filename );
            break;
        }
        ttyConsole_log( "open(%s) successful ... %d \n", pipe1Filename, pipes[1] );

        memset( one_line, 0, sizeof( one_line ));

        while (1)
        {
            // select setup
            fd_set fds;
            FD_ZERO( &fds );
            if (sock != 0) {FD_SET( sock, &fds ); }
            FD_SET( pipes[0], &fds );
            max_fd = sock;
            if (pipes[0] > sock)
            {
                max_fd = pipes[0];
            }

            // wait for data
            int nready = select( max_fd + 1, &fds, (fd_set *)NULL, (fd_set *)NULL, &ts );

            if (nready < 0)
            {
                ttyConsole_log( "select. Error\n" );
                break;
            }
            else if (nready == 0)
            {
                ts.tv_sec  = 1;                            // 1 second
                ts.tv_usec = 0;
            }
            else if (( sock != 0 ) && FD_ISSET( sock, &fds ))
            {
                // start by reading a single byte
                int rv;
                /*fprintf(stderr, "Try: recv(%d) \n\r", sock );*/
                rv = recv( sock, buf, 1, 0 );

                if (rv < 0)
                {
                    break;
                }
                else if (rv == 0)
                {
                    ttyConsole_log( "Connection closed by the remote end\n\r" );
                    break;
                }

                if (buf[0] == CMD)
                {
                    // read 2 more bytes
                    len = recv( sock, buf + 1, 2, 0 );
                    if (len  < 0) {break; }
                    else if (len == 0)
                    {
                        ttyConsole_log( "Connection closed by the remote end\n\r" );
                        break;
                    }
                    negotiate( sock, buf, 3 );
                }
                else
                {
                    len      = 1;
                    buf[len] = '\0';
                    write( pipes[1], buf, 1 );
                    if (( buf[0] == '\n' ) && strlen( one_line ))
                    {
                        strncat( one_line, (char *) buf, sizeof( one_line ) - strlen( one_line ) -1 );
                        ttyConsole_log( one_line );

                        memset( one_line, 0, sizeof( one_line ));
                    }
                    else if (buf[0] == '\r')
                    {
                    }
                    else
                    {
                        strncat( one_line, (char *) buf, sizeof( one_line ) - strlen( one_line ) -1 );
                    }
                    fflush( 0 );
                }
            }
            else if (FD_ISSET( pipes[0], &fds ))
            {
                int numbytes = 0;
                /*fprintf(stderr, "Try: getc(%d) \n\r", pipes[0] );*/
                numbytes = read( pipes[0], buf, 1 );       //fgets(buf, 1, stdin);
                if (numbytes)
                {
                    /*fprintf(stderr, "Got: getc(%d) %02x '%c' \n\r", pipes[0], buf[0], buf[0] );*/
                    if (send( sock, buf, 1, 0 ) < 0) {break; }
                }
            }
        }                                                  /* while forever */
    } while (0);

    ttyConsole_log( "%s - Closing pipes\n", __FUNCTION__ );
    close( sock );
    close( pipes[0] );
    close( pipes[1] );

    ttyConsole_log( "%s - Removing pipes\n", __FUNCTION__ );
    remove( pipe0Filename );
    remove( pipe1Filename );

    /* kill klog_thread */
    if (readKlog_thread_id > 0)
    {
        char cmd[32];
        sprintf( cmd, "kill -9 %d", readKlog_thread_id );
        ttyConsole_log( "%s - system(%s) \n", __FUNCTION__, cmd );
        system( cmd );
    }

    ttyConsole_log( "exit ... %d \n", getpid());
    exit( 0 );
}                                                          /* ttyConsole_thread */

static void *readKlog_thread(
    void *data
    )
{
    int   rc          = 0;
    char *buffer      = NULL;
    bool  bInfoOutput = false;
    int   pipe        = 0;

    ttyConsole_log( "%s - my pid %d \n", __FUNCTION__, getpid());
    while (1)
    {
        rc = klogctl( 10, NULL, 0 );
        /*if (bInfoOutput == false) {ttyConsole_log( "after klogctl(10:size of ring buffer): rc %d; \n", rc ); }*/

        rc = klogctl( 9, NULL, 0 );
        if (( bInfoOutput == false ) && ( rc > 0 )) {ttyConsole_log( "after klogctl(9:unread chars in ring buffer): rc %d; pid %d \n", rc, getpid()); }

        if (( rc > 0 ) && ( rc < 65536 ))
        {
            buffer = (char *)malloc( rc + 1 );
            if (buffer)
            {
                char tempFilename1[TEMP_FILE_FULL_PATH_LEN];
                char tempFilename2[TEMP_FILE_FULL_PATH_LEN];

                memset( buffer, 0, rc + 1 );

                rc = klogctl( 2, buffer, rc );

                if (bInfoOutput == false) {ttyConsole_log( "after klogctl: rc %d; strlen(buffer) %d \n", rc, strlen( buffer )); }
                sprintf( tempFilename1, "%s_%s", PIPE_OUT, g_uuid );
                PrependTempDirectory( tempFilename2, sizeof( tempFilename2 ), tempFilename1 );

                pipe = open( tempFilename2, O_WRONLY );
                /*ttyConsole_log( "%s: open(%s) O_WRONLY returned %d \n", __FUNCTION__, tempFilename2, pipe );*/

                if (pipe > 0)
                {
                    write( pipe, buffer, rc );
                    /* if the kernel buffer does NOT end with a newline, output one now */
                    if (buffer[rc-1] != '\n')
                    {
                        write( pipe, "\n", 1 );
                    }
                    close( pipe );
                }

                Bsysperf_Free( buffer );
            }
        }

        usleep( 1000000/2 );                               /* sleep for half a second */
    }

    ttyConsole_log( "%s: exit() \n", __FUNCTION__ );
    exit( 0 );
}                                                          /* readKlog_thread */

static int read_results(
    void
    )
{
    int  pipe = 0;
    char buf[MAX_BUF];
    int  bytes      = 1;
    int  flags      = 0;
    int  pass_count = 0;
    char tempFilename1[TEMP_FILE_FULL_PATH_LEN];
    char tempFilename2[TEMP_FILE_FULL_PATH_LEN];

    sprintf( tempFilename1, "%s_%s", PIPE_OUT, g_uuid );
    PrependTempDirectory( tempFilename2, sizeof( tempFilename2 ), tempFilename1 );

    /* open the pipe that the telnet thread is using to write to */
    pipe = open( tempFilename2, O_RDONLY );
    /*printf("open(%s) O_RDONLY returned pipe %d \n", tempFilename2, pipe );*/

    if (pipe <= 0)
    {
        printf( "~DEBUG~open(%s) failed ... %s~", tempFilename2, strerror( errno ));
        return( 0 );
    }

    flags = fcntl( pipe, F_GETFL, 0 );                     /* get current flags */
    fcntl( pipe, F_SETFL, flags | O_NONBLOCK );            /* set new flags along with previous flags */

    do
    {
        memset( buf, 0, sizeof( buf ));
        /*printf("try ... read(%d, bytes %d) \n", pipe, MAX_BUF );*/
        bytes = read( pipe, buf, MAX_BUF-1 );
        if (bytes > 0)
        {
            /*printf("Rcvd: %d (", bytes );*/
            if (pass_count == 0)
            {
                printf( "~ttyContents~" );
            }
            printf( "%s", buf );

            if (bytes < ( MAX_BUF-1 ))
            {
                printf( "~" );
            }

            pass_count++;
        }
    } while (bytes == ( MAX_BUF-1 ));
    close( pipe );
    return( 0 );
}                                                          /* read_results */

int main(
    int   argc,
    char *argv[]
    )
{
    bmemperf_version_info versionInfo;
    struct utsname        uname_info;
    char                 *queryString  = NULL;
    int                   epochSeconds = 0;
    int                   tzOffset     = 0;
    int                   ttyConsole   = 0;
    char                  ttyCommand[128];

    printf( "Content-type: text/html\n\n" );

    queryString = getenv( "QUERY_STRING" );

    if (queryString == NULL)
    {
        printf( "QUERY_STRING env is not defined\n" );
        return( 0 );
    }
    printf( "\n~DEBUG~queryString (%s)~", queryString );

    memset( ttyCommand, 0, sizeof( ttyCommand ));
    memset( g_uuid, 0, sizeof( g_uuid ));

    if (strlen( queryString ))
    {
        scanForInt( queryString, "datetime=", &epochSeconds );
        scanForInt( queryString, "tzoffset=", &tzOffset );
        scanForInt( queryString, "ttyConsole=", &ttyConsole ); /* 1 => INIT; 2 -> KILL_SUBTHREADS */
        scanForStr( queryString, "ttyCommand=", sizeof( ttyCommand ), ttyCommand );
        scanForStr( queryString, "uuid=", sizeof( g_uuid ), g_uuid );
    }

    /* if browser provided a new date/time value; this only happens once during initialization */
    if (epochSeconds)
    {
        struct timeval now = {1400000000, 0};

        strncpy( versionInfo.platform, getPlatform(), sizeof( versionInfo.platform ) - 1 );
        strncpy( versionInfo.platVersion, getPlatformVersion(), sizeof( versionInfo.platVersion ) - 1 );
        versionInfo.majorVersion = MAJOR_VERSION;
        versionInfo.minorVersion = MINOR_VERSION;
        printf( "~PLATFORM~%s", versionInfo.platform );
        printf( "~PLATVER~%s", versionInfo.platVersion );
        printf( "~VERSION~Ver: %u.%u~", versionInfo.majorVersion, versionInfo.minorVersion );

        uname( &uname_info );
        printf( "~UNAME~%d-bit %s %s~", ( sizeof( char * ) == 8 ) ? 64 : 32, uname_info.machine, uname_info.release );

        now.tv_sec = epochSeconds - ( tzOffset * 60 );
        settimeofday( &now, NULL );

        usleep( 100 );
    }

    printf( "~STBTIME~%s~", DayMonDateYear( 0 ));

    #if 0
    printf( "\n~DEBUG~ttyConsole %d~", ttyConsole );
    if (strlen( g_uuid )) {printf( "\n~DEBUG~g_uuid %s~", g_uuid ); }
    if (strlen( ttyCommand )) {printf( "\n~DEBUG~ttyCommand len %d ... (%s)~", strlen( ttyCommand ), ttyCommand ); }
    #endif

    if (ttyConsole == TTY_CONSOLE_CMD_INIT)
    {
        int klog_pid = 0;
        int tty0_pid = 0;
        klog_pid = daemonize( NULL );                      /* this causes a fork to happen and another (child) thread gets started */
        /*printf("after daemonize(): klog_pid %d (%d) \n", klog_pid, getpid() );*/
        if (klog_pid == 0)                                 /* 0 is the child; greater than zero is the parent */
        {
            /* start a background thread that will read the output from drivers (similar to dmesg utility) */
            readKlog_thread( NULL );
        }
        else if (klog_pid > 0)                             /* this is the parent from the above fork() */
        {
            tty0_pid = daemonize( NULL );                  /* this causes a fork to happen and another (child) thread gets started */
            /*printf("after daemonize(): tty0_pid %d (%d) \n", tty0_pid , getpid() );*/
            if (tty0_pid == 0)                             /* 0 is the child; greater than zero is the parent */
            {
                /* start a background thread that will perform telnet I/O */
                /* send the pid of the readKlog_thread started above; if telnet session ends, telnet thread will cleanup (kill) the readKlog_thread */
                ttyConsole_thread( klog_pid );
            }
        }
    }
    else if (ttyConsole == TTY_CONSOLE_CMD_GET_STATUS)
    {
        read_results();
    }
    else if (ttyConsole == TTY_CONSOLE_CMD_EXIT)           /* kill subthreads */
    {
    }

    if (strlen( ttyCommand ))                              /* the user entered a command for the command line shell */
    {
        decodeURL( ttyCommand );
        process_command( ttyCommand );

        usleep( 100000 );                                  /* wait a tenth of a second and then read the results of the shell command */

        read_results();
    }

    printf( "~ALLDONE~" );

    return( 0 );
}                                                          /* main */
