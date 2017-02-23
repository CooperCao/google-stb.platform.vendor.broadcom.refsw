/*=============================================================================
Broadcom Proprietary and Confidential. (c)2012 Broadcom.
All rights reserved.

Project  :  SpyHook
Module   :  Proxy server for Android

FILE DESCRIPTION

Provides a local proxy server than can be run with root privileges to allow
applications which have been built without INTERNET permissions to be
monitored.

                             ** WARNING **

Given this program could be used in a more general-purpose way to circumvent
Android's security mechanisms the good folks at Google might not be entirely
happy with this tool being released into the wild!

TOP LEVEL FUNCTIONAL DESCRIPTION

The proxy communicates with the client through 4 OS FIFOs ("named pipes"):

FIFO_CTRL - receives control messages from the client
FIFO_ACK  - provides control messages acknowledges to the client
FIFO_TX   - handles data traffic from client to IP connection
FIFO_RX   - handles data traffic from IP connection to client

Schematically the arrangement looks like:

  /-------------\   FIFO_CTRL     /---------------\
  |             +>>>>>>>>>>>>>>>>>+               |
  |             |                 |               |
  |             |    FIFO_ACK     |               |            /----------\
  |             +<<<<<<<<<<<<<<<<<+               |            |          |
  |   CLIENT    |                 |    PROXY      +<==/ IP /==>+  SERVER  |
  |     APP     |    FIFO_TX      |               |            |          |
  |             +>>>>>>>>>>>>>>>>>+               |            \----------/
  |             |                 |               |
  |             |    FIFO_RX      |               |
  |             +<<<<<<<<<<<<<<<<<+               |
  \-------------/                 \---------------/

The following commands are supported:

    open <IP_ADDRESS_OR_SERVER_NAME> <PORT_NUM>

        Opens a connection to the server identified by IP address or name, and
        a port number.

    close

        Closes the current connection.

    exit

        Ends the proxy session, terminating the proxy process.

The command processor operates in one of two states.  In the IDLE state it is
waiting for either an "open" command or an "exit" command.  A successful "open"
moves the command processor into the OPEN state.  The "close" command then
closes the open connection and returns the command processor to the IDLE state.
It is an error to try to execute a command in the wrong state.

   START    /------------\      open        /------------\
   -------->|            |----------------->|            |
            |    IDLE    |                  |    OPEN    |
   <--------|            |<-----------------|            |
    exit    \------------/     close        \------------/

=============================================================================*/

/*****************************************************************************/
/* System Includes                                                           */
/*****************************************************************************/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

/*****************************************************************************/
/* Project Includes                                                          */
/*****************************************************************************/

#include "spyproxy.h"

/*****************************************************************************/
/* Macros, constants                                                         */
/*****************************************************************************/

#define FIFO_PATH           "/data/app/spyhook"

#define FIFO_PATH_PERMS     (S_IRWXU|S_IRWXG|S_IRWXO)
#define FIFO_PERMS          (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)

#define ACK(fp)             do{int r;dmsg(1,"Sending ACK\n");r=fputs("1\n",(fp));if(r<0)oops("FAILED TO ACK\n");fflush((fp));}while(0)
#define NAK(fp)             do{int r;dmsg(1,"Sending NAK\n");r=fputs("0\n",(fp));if(r<0)oops("FAILED TO NAK\n");fflush((fp));}while(0)

#define BUFLEN              ( 1024 )

/*****************************************************************************/
/* Private Data.  Declare as static.                                         */
/*****************************************************************************/

/* Global socket handle */
static int g_sock = -1;

/* Control verbosity */
static int g_verbose = 0;
static int g_debug   = 0;

/* TX and RX thread objects */
static pthread_t tx_thread, rx_thread;

/* These two volatile flags control the run-state of the two threads.
   Probably not the best way to do this, but it works for now.
*/
static volatile int run_rx = 0, run_tx = 0;

/* The comms thread states */
enum {
    THD_READY,
    THD_RUNNING,
    THD_STOPPING,
    THD_STOPPED
};

/*****************************************************************************/
/* Private function prototypes.  Declare as static.                          */
/*****************************************************************************/

static void msg( char *, ... );
static void dmsg( int, char *, ... );
static void oops( char *, ... );
static void delay( unsigned int );
static void * thd_tx( void * );
static void * thd_rx( void * );
static int start_thread( volatile int *, pthread_t *, void * (*)( void *) );
static int stop_thread( volatile int * );
static int start_server( char *, unsigned int );
static int stop_server( void );
static void cmd_processor( FILE *, FILE * );
static int create_assets( void );
static int delete_assets( void );

/*****************************************************************************/
/* Private functions.  Declare as static.                                    */
/*****************************************************************************/

/*****************************************************************************/
/**
    Print out a message on stderr.

    To be used for informational messages only, to provide information on the
    running status of the program.  Won't appear unless verbose mode enabled
    with the "-v" or "--verbose" command line flags.
**/
static void msg( char *fmt, ... )
{
    if ( g_verbose )
    {
        va_list ap;

        va_start( ap, fmt );
        fprintf( stderr, "SpyProxy :: " );
        vfprintf( stderr, fmt, ap );
        fflush( stderr );
        va_end( ap );
    }
}

/*****************************************************************************/
/**
    Print out a debug message on stderr.

    To be used for development debug messages only, to provide detailed
    information on the running status of the program.  Only really for use
    by developers, not for general use.

    The level of debug verbosity is controlled by the number of "-d" that are
    on the command line.  More "-d" means a higher level of verbosity.
    USE WITH CAUTION!!

    Use level 0 for the most important, with increasing level for less
    important messages.  (Yes.. I know... its upside-down.... *shrugs*)

    Won't appear unless debug mode is enabled with the "-d" or "--debug"
    command line flags.  Also turns on verbose messages.
**/
static void dmsg( int lvl, char *fmt, ... )
{
    if ( g_debug > lvl )
    {
        va_list ap;

        va_start( ap, fmt );
        fprintf( stderr, "SpyProxy :-DEBUG-: " );
        vfprintf( stderr, fmt, ap );
        fflush( stderr );
        va_end( ap );
    }
}

/*****************************************************************************/
/**
    Print out an error message on stderr.

    Generally an "oops" causes a break in normal operation so should always be
    recorded.
**/
static void oops( char *fmt, ... )
{
    va_list ap;

    va_start( ap, fmt );
    fprintf( stderr, "SpyProxy :: *** ERROR *** " );
    vfprintf( stderr, fmt, ap );
    fflush( stderr );
    va_end( ap );
}

/*****************************************************************************/
/**
    Sleep for a period of milliseconds.
**/
static void delay( unsigned int delay_ms  )
{
    usleep( 1000 * delay_ms );
}

/*****************************************************************************/
/**
    Transmit thread

    Receives bytes from the TX FIFO and sends them to the IP connection.
**/
static void * thd_tx( void * NOT_USED )
{
    int fd;
    char buf[BUFLEN];

    if ( run_tx != THD_READY )
    {
        oops( "TX thread not ready\n" );
        return NULL;
    }

    msg( "Starting TX thread...\n" );

    fd = open( FIFO_TX, O_RDONLY | O_NONBLOCK );
    if ( fd < 0 )
    {
        oops( "Failed to open TX FIFO\n" );
        return NULL;
    }

    run_tx = THD_RUNNING;

    while ( run_tx == THD_RUNNING )
    {
        ssize_t n = read( fd, buf, sizeof(buf) );
        dmsg( 2, "TX: read() from FIFO returned %d\n", n );

        if ( n > 0 )
        {
            ssize_t nn;

            msg( "TX: sending %d bytes to server.\n", n );

            /* Note to self: need to think about this next section a bit more.
               Because we're talking to a socket the returned value is either
               n OR -1 and check errno.  Most of the error cases look fatal so
               it may well be worth aborting the entire connection if this
               happens.
             */
            nn = write( g_sock, buf, n );
            dmsg( 2, "TX: write() to socket returned %d\n", nn );

            if ( nn != n )
            {
                oops( "Some TX bytes got lost along the way...\n" );
            }
        }
        else
        {
            delay( 5 );
        }
    }

    msg( "Stopping TX thread...\n" );
    close( fd );
    run_tx = THD_STOPPED;
    return NULL;
}

/*****************************************************************************/
/**
    Receive thread

**/
static void * thd_rx( void * NOT_USED )
{
    int fd;
    char buf[BUFLEN];

    if ( run_rx != THD_READY )
    {
        oops( "RX thread not ready\n" );
        return NULL;
    }
    msg( "Starting RX thread...\n" );

    run_rx = THD_RUNNING;

    do {
        fd = open( FIFO_RX, O_WRONLY | O_NONBLOCK);
        if ( fd < 0 )
            delay( 5 );
    } while ( run_rx == THD_RUNNING && fd < 0 );

    while ( run_rx == THD_RUNNING )
    {
        ssize_t n = recv( g_sock, buf, sizeof(buf), MSG_DONTWAIT );
        dmsg( 2, "RX: recv() return %d\n", n );

        if ( n > 0 )
        {
            ssize_t nn;

            msg( "RX: received %d bytes from server.\n", n );

            /* Note to self: do we want abort if any other failure conditions
               arise?
             */
            nn = write( fd, buf, n );
            dmsg( 2, "RX: write() to FIFO returned %d\n", nn );

            if (nn == -1 )
            {
                oops( "Failed to write to RX FIFO.\n" );
            }
            else if ( nn < n )
            {
                oops( "%d bytes not pushed into RX FIFO.\n", n - nn );
            }
        }
        else if ( n == 0 )
        {
            msg( "The other end has closed the FIFO\n" );
            run_rx = THD_STOPPING;
        }
        else if ( n < 0 )
        {
            if ( errno == EAGAIN || errno == EWOULDBLOCK )
            {
                /* This is normal: there was nothing to read from the socket. */
                delay( 5 );
            }
            else
            {
                oops( "Internal error in RX thread\n" );
                run_rx = THD_STOPPING;
            }
        }
    }

    msg( "Stopping RX thread...\n" );
    if ( fd >= 0 )
        close( fd );
    run_rx = THD_STOPPED;
    return NULL;
}

/*****************************************************************************/
/**
    Start a thread
**/
static int start_thread( volatile int *prun, pthread_t *pthread,
                         void * (* start_routine)( void *))
{
    int status;

    *prun = THD_READY;
    dmsg( 0, "Starting thread...\n" );

    status = pthread_create( pthread, NULL, start_routine, NULL );
    if ( 0 == status )
    {
        while ( *prun != THD_RUNNING )
            ;
    }
    else
        dmsg( 1, "Failed to start thread: %s\n", strerror(status) );

    return status;
}

/*****************************************************************************/
/**
    Stop a thread
**/
static int stop_thread( volatile int *prun )
{
    dmsg( 0, "Stopping thread...\n" );
    if ( *prun == THD_RUNNING )
    {
        *prun = THD_STOPPING;
        while ( *prun != THD_STOPPED )
            delay( 5 );
    }

    return 0;
}

/*****************************************************************************/
/**
    Open a socket to the specified server:port connection.
**/
static int start_server( char * srv_name, unsigned int port )
{
    struct sockaddr_in serv_addr;
    struct hostent *server;

    g_sock = socket( AF_INET, SOCK_STREAM, 0 );
    if ( g_sock < 0 )
    {
        oops( "Failed to open IP socket - I can haz root?\n" );
        return EXIT_FAILURE;
    }

    memset( (char *)&serv_addr, 0, sizeof(serv_addr) );
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons( port );

    if ( !inet_aton( srv_name, &serv_addr.sin_addr ) )
    {
        server = gethostbyname( srv_name );
        if ( !server )
        {
            oops( "No such host: %s (%s)\n", srv_name, hstrerror(h_errno) );
            return EXIT_FAILURE;
        }

        memcpy( (char *)&serv_addr.sin_addr.s_addr,
                (char *)server->h_addr,
                server->h_length );
    }

    if ( connect( g_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr) ) )
    {
        oops( "Failed to connect to %s (%s)\n", srv_name, strerror(errno) );
        return EXIT_FAILURE;
    }

    start_thread( &run_tx, &tx_thread, thd_tx );
    start_thread( &run_rx, &rx_thread, thd_rx );

    return EXIT_SUCCESS;
}

/*****************************************************************************/
/**
    Shutdown the comms threads and close the connection to the server.
**/
static int stop_server( void )
{
    dmsg( 0, "Stopping all services...\n" );

    stop_thread( &run_tx );
    stop_thread( &run_rx );

    shutdown( g_sock, SHUT_RDWR );
    close( g_sock );
    g_sock = -1;
    return EXIT_SUCCESS;
}

/*****************************************************************************/
/**
    Command processor.

    Receives commands from the client through the fp_ctrl FIFO, processes it,
    and returns an ACK or NAK to the fp_ack FIFO.
    This function only exits in failure cases where there is no point for the
    show to carry on.
**/
static void cmd_processor( FILE *fp_ctrl, FILE *fp_ack )
{
    char buf[BUFLEN];
	enum {
        ST_IDLE,
        ST_OPEN
    } state = ST_IDLE;

    msg( "Ready to process connections...\n" );

	do {
		char *s = fgets( buf, sizeof(buf), fp_ctrl );
		if ( s )
        {

            dmsg( 1, "Got a command string \"%s\" in %s state.  Parsing...\n",
                    buf,
                    state == ST_IDLE ? "IDLE" : "OPEN" );

			if ( state == ST_IDLE && strncmp( buf, "open", 4 ) == 0 )
            {
                char srv_name[BUFLEN];
                unsigned int portnum;

                dmsg( 1, "Received OPEN command\n" );

                /* Parse out the ip address and port number */
                sscanf( buf, "open %s %u", srv_name, &portnum );

                msg( "OPEN srv_name = %s, portnum = %u\n",
                         srv_name, portnum );

                if ( EXIT_SUCCESS == start_server( srv_name, portnum ) )
                {
                    state = ST_OPEN;
                    ACK( fp_ack );
                }
                else
                {
                    NAK( fp_ack );
                }
            }
            else if ( state == ST_OPEN && strncmp( buf, "close", 5 ) == 0 )
            {
                dmsg( 1, "Received CLOSE command\n" );

                if ( EXIT_SUCCESS == stop_server() )
                {
                    msg( "CLOSE\n" );
                    state = ST_IDLE;
                    ACK( fp_ack );
                }
                else
                {
                    NAK( fp_ack );
                }
            }
            else if ( state == ST_IDLE && strncmp( buf, "exit", 4 ) == 0 )
            {
                msg( "Bye bye!\n" );
                ACK( fp_ack );
                break;
            }
            else
            {
                oops( "ERR: unknown command \"%s\" in state %s\n",
                        buf,
                        state == ST_IDLE ? "IDLE" : "OPEN" );
                NAK( fp_ack );
            }
        }
        else
        {
            delay( 5 );
        }
	} while ( 1 );
}

/*****************************************************************************/
/**
    Create file system components.

    Note: the directory is allowed to persist, so ignore the pre-existing
    error.
**/
static int create_assets( void )
{
    int status;
    int i;
    char *fifo[] = { FIFO_CTRL, FIFO_ACK, FIFO_TX, FIFO_RX, NULL };

    msg( "Creating FIFO directory\n" );

    status = mkdir( FIFO_PATH, FIFO_PATH_PERMS );
    if ( status && errno != EEXIST )
    {
        oops( "Could not create \"" FIFO_PATH "\"" );
        return EXIT_FAILURE;
    }

    delete_assets();

    msg( "Creating FIFOs\n" );
    for ( i = 0; fifo[i]; i++ )
    {
        status = mkfifo( fifo[i], FIFO_PERMS );
        if ( status )
        {
            oops( "Could not create FIFO %s\n", fifo[i] );
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

/*****************************************************************************/
/**
    Delete file system components.
**/
static int delete_assets( void )
{
    msg( "Deleting FIFO files.\n" );
    unlink( FIFO_CTRL );
    unlink( FIFO_ACK );
    unlink( FIFO_TX );
    unlink( FIFO_RX );

    return EXIT_SUCCESS;
}

/*****************************************************************************/
/* Public functions.  Defined in header file ( except main() ).              */
/*****************************************************************************/

/*****************************************************************************/
/**
    SpyHook Proxy main application.

    Open the command and ack FIFOs and then run the command processor.  When it
    returns close down the FIFOs and finish.
**/
int main( int argc, char *argv[] )
{
    int status;

	FILE *fp_ctrl;
    FILE *fp_ack;

    if ( argc > 1 )
    {
        int i;
        for ( i = 1; i < argc; i++ )
        {
            if ( !strcmp( argv[i], "-v" ) || !strcmp( argv[i], "--verbose") )
                g_verbose = 1;
            else if ( !strcmp( argv[i], "-d" ) || !strcmp( argv[i], "--debug") )
            {
                /* This is a private feature, not for general use */
                g_verbose = 1;
                g_debug++;
            }
            else if ( !strcmp( argv[i], "-h") || !strcmp( argv[i], "--help" ) )
            {
                fprintf( stderr,
                         "Usage:\n"
                         "   spyproxy [-h|--help] [-v|--verbose]\n"
                         "\n"
                         "   -h|--help      show this help information\n"
                         "   -v|--verbose   enable verbose messages to console\n"
                         "\n" );
                return EXIT_SUCCESS;
            }
            else
                oops( "Ignoring unknown command line flag \"%s\"\n", argv[i] );
        }
    }

    if ( g_debug )
        dmsg( 0, "Debug messages at %d\n", g_debug );

    msg( "Copyright (c) 2012 Broadcom Corp.\n" );

    if ( EXIT_SUCCESS != create_assets() )
        return EXIT_FAILURE;

    dmsg( 0, "Opening FIFOs\n" );
    fp_ctrl = fopen( FIFO_CTRL, "r" );
    fp_ack  = fopen( FIFO_ACK,  "w" );

    cmd_processor( fp_ctrl, fp_ack );

    dmsg( 0, "Closing FIFOs\n" );
    fclose( fp_ack );
	fclose( fp_ctrl );

    delete_assets();

	return 0;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
