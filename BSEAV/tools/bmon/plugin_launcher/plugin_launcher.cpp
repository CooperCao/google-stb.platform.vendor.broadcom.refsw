/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1. This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#if defined __cplusplus
#include <iostream>
#include <functional>
#include <string>
#include <sys/time.h>
#include <unistd.h>
#endif /* if defined __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>

#if defined __cplusplus
extern "C" {
#endif
#include "plugin_launcher.h"
#include "bmon_defines.h"
#include "bmon_json.h"
#if defined __cplusplus
}
#endif

#define MIN(x, y)  (((x) > (y)) ? (y) : (x))

#pragma GCC diagnostic ignored "-Wvariadic-macros"
#define NOPRINTF(...)
#define NOFPRINTF(...)

#if defined __cplusplus
#else
char *strcasestr( const char *haystack, const char *needle );
#endif

#define RC_CHECK_GOTO( TEST, DESCRIPTION, RETURN_CODE ) if (!( TEST )) {fprintf( stderr, DESCRIPTION ); goto error; }
#define PLUGIN_FILE_DEFAULT "fileread"
#define FPRINTF NOFPRINTF

static char HTML_NAME_DEFAULT[]   =  "index.html";
static char PLUGIN_NAME_DEFAULT[] =  "/";
static char PLUGIN_ARGS_DEFAULT[] =  "/";

/**
  *  Function: This function will compute a predictable and unique hash value for the specified
  *            input string. The value is computed using the C++ hash function to eliminate the need
  *            for this API to link with some external library that would have a compatible hash function.
  *            Function will compute an 8-character string for 32-bit chips and a 16-character string for
  *            64-bit chips.
 **/
static int compute_hash(
    const char *input_string,
    char       *output_string,
    int         output_string_len
    )
{
    int bytes = 0;

#if defined __cplusplus
    std::string            str1( input_string );
    std::hash<std::string> str_hash;

    bytes = snprintf( output_string, output_string_len, "%lx", (long int) str_hash( str1 )); /* 78969d175283c4f7 */
    FPRINTF(stderr, "%s - input (%s) ... output (%s) ... output_len (%d) ... snprintf len (%d) \n", __FUNCTION__, input_string, output_string, output_string_len, bytes );

    if (bytes > output_string_len)
    {
        fprintf( stderr, "user buffer was too short ... given %d bytes but needed %d bytes \n", output_string_len, bytes );
    }
#endif /* if defined __cplusplus */
    bytes = MIN(bytes,output_string_len);
    FPRINTF(stderr, "%s - returning (%d) ... sizeof(str_hash) (%d) ... sizeof(ptr) (%d) \n", __FUNCTION__, bytes, sizeof(str_hash(str1)), sizeof(output_string) );
    return( bytes );
} /* compute_hash */

static bool Bhttpd_IsHtmlFile(
    const char *filename
    )
{
    bool rc = false;

    if (strcasestr( filename, ".html" ) || strcasestr( filename, ".ico" ) || strcasestr( filename, ".jpg" ) ||
        strcasestr( filename, ".png" ) || strcasestr( filename, ".js" ) || strcasestr( filename, ".css" ))
    {
        rc = true;
    }
    return( rc );
}

#if defined __cplusplus
extern "C" {
#endif

int plugin_launcher(
    plugin_launcher_t *params
    )
{
    int            bytes_prev          = 0;
    int            size_payload        = 0;
    int            pipe_parent_read[2] = {0, 0};
    char          *plugin_name         = NULL;
    char          *plugin_args         = NULL;
    char          *token_ptr           = NULL;
    int            rc                  = 0;
    pid_t          pid = 0;
    bool           b_url_path_is_html_file        = false;
    char           s_server_port[32];
#ifdef PLUGIN_LAUNCHER_DEBUG
    struct timeval tv1                       = {0, 0};
    struct timeval tv2                       = {0, 0};
    struct timeval tv3                       = {0, 0};
    unsigned long long microseconds1         = 0;
    unsigned long long microseconds2         = 0;
#endif /* PLUGIN_LAUNCHER_DEBUG */
    char           pHash[128];
    int            len                       = 0;
    char           DEFAULT_BHTTPD_PORT_STRING[8];
    const char    *envv[12] = {"PATH=/bin:/usr/bin:/usr/local/bin:./",
                               "LD_LIBRARY_PATH=/bin:/usr/bin:/usr/local/bin:./",
                               "SERVER_SOFTWARE=bhttpd/1.0",
                               NULL /* SERVER_PORT=12345 ...  this field is dynamic and will get changed below */,
                               "GATEWAY_INTERFACE=CGI/1.1",
                               "REQUEST_METHOD=GET",
                               "SERVER_PROTOCOL=HTTP/1.1",
                               NULL /* USER_AGENT */,
                               NULL /* REMOTE_IP */,
                               NULL /* SESSION_ID */,
                               NULL /* QUERY_STRING ... this entry will only be set if browser is used and URL has ?abc=123 */,
                               NULL /* array should always be null-terminated */};
    cJSON * objectRoot = NULL;
    cJSON * objectData = NULL;

    FPRINTF( stderr, "%s (%s) ... size_payload (%d)\n", __FUNCTION__, params->uri, params->size_payload ); fflush( stderr ); fflush( stdout );
    FPRINTF( stderr, "%s() ... p_payload (%p) ... remote (%s)\n", __FUNCTION__, params->p_payload, params->s_remote_ip_address ); fflush( stderr ); fflush( stdout );
    FPRINTF( stderr, "%s() ... s_user_agent (%s)\n", __FUNCTION__, params->s_user_agent ); fflush( stderr ); fflush( stdout );
    if (params == NULL)
    {
        fprintf( stderr, "%s - params cannot be NULL \n", __FUNCTION__ );
        return( 0 );
    }

    if (params->uri == NULL)
    {
        fprintf( stderr, "%s - params->uri cannot be NULL \n", __FUNCTION__ );
        return( 0 );
    }

    if (params->p_payload == NULL)
    {
        fprintf( stderr, "%s - params->p_payload cannot be NULL \n", __FUNCTION__ );
        return( 0 );
    }

    if (params->s_user_agent[0] == '\0')
    {
        fprintf( stderr, "%s - params->s_user_agent cannot be empty \n", __FUNCTION__ );
        return( 0 );
    }

    if (params->s_remote_ip_address[0] == '\0')
    {
        fprintf( stderr, "%s - params->s_remote_ip_address cannot be empty \n", __FUNCTION__ );
        return( 0 );
    }

    memset( s_server_port, 0, sizeof(s_server_port) );
    memset(pHash, 0, sizeof(pHash));

    rc = compute_hash( params->s_user_agent,  &pHash[0], HASH_STRING_LEN ); /* e.g. input (curl/7.54.0) ... output (c3a0889e4058438c)*/
    RC_CHECK_GOTO(( rc != 0 ), ( "compute_hash(s_user_agent) Failed" ), 0 );
    len = strlen(pHash);
    FPRINTF(stderr, "%s - hash(%s) is (%s) ... len (%d) \n", __FUNCTION__, params->s_user_agent, &pHash[0], len );
    rc = compute_hash( params->s_remote_ip_address, &pHash[len], HASH_STRING_LEN - len ); /* e.g. input (10.14.233.36) ... output (cc620460bb0a7877) */
    RC_CHECK_GOTO(( rc != 0 ), ( "compute_hash(s_remote_ip_address) Failed" ), 0 );
    FPRINTF(stderr, "%s - hash(%s) is (%s) ... len (%d) \n", __FUNCTION__, params->s_remote_ip_address, &pHash[len], strlen(pHash) );
    /* final hash (c3a0889e4058438ccc620460bb0a7877) */

    FPRINTF( stderr, "%s - RemoteIpAddress (%s) ... User-Agent (%s) \n", __FUNCTION__, params->s_remote_ip_address, params->s_user_agent );
    FPRINTF( stderr, "%s - hash (%s) \n", __FUNCTION__, pHash );

    rc = pipe( pipe_parent_read );
    RC_CHECK_GOTO(( rc == 0 ), ( "pipe(WRITE) Failed" ), rc );

#ifdef PLUGIN_LAUNCHER_DEBUG
    gettimeofday( &tv1, NULL );
    fprintf( stderr, "%d %d.%06d - parent_read:  %d and %d\n", getpid(), (int)tv1.tv_sec%1000, (int)tv1.tv_usec, pipe_parent_read[0], pipe_parent_read[1] );
    microseconds1 = tv1.tv_sec;
    microseconds1 *= 1000000;
    microseconds1 += tv1.tv_usec;
#endif /* PLUGIN_LAUNCHER_DEBUG */

    b_url_path_is_html_file = Bhttpd_IsHtmlFile( params->uri );
    pid                = fork();
    FPRINTF( stderr,  "%s - fork() returned ... pid %d \n", __FUNCTION__, pid );
    fflush( stdout ); fflush( stderr );

    switch (pid) {
        case 0:                                            /* this is the child */
        {
            unsigned int bytes  = 0;
            int          status = 0;
            char         s_query_string[1024];
            char         qmark_query_string[1024];
            char         s_user_agent[256];
            char         s_remote_ip_address[128];
            char         s_session_id[80];                   /* SESSION_ID=145cf06aef77e20fc54965b1aff7002e473e2483499a678f4196bb7f5c25e2fc */
            char        *p_query_string = NULL;
            char        *p_working_path = (char *) params->uri;
            char         plugin_name_with_dir[64];
            char        *argv[]   = {plugin_name_with_dir, plugin_args /* URL from browser */, NULL /* end of list */};

            /* since the port number is dictated by the caller, create space for a string that could vary from call to call */
            envv[3] = s_server_port;
            snprintf((char *) envv[3], sizeof(s_server_port) - 1, "SERVER_PORT=%d", params->server_port );

            FPRINTF( stderr,  "CHILD: params->uri (%s)  \n", params->uri );
            /* By the time the child gets here, the parent should have launched the child process and blocked
               while reading from the parent_read pipe. We need to spawn a separate plugin that will process the URL
               and write the response into the parent_read pipe. */

            /*
               case 1:  wifi?rssi,snr
               case 2:  cgi?bsysperf.cgi?random=18839394
               case 3:  cgi-bin/bsysperf.cgi?random=18839394
               case 4:  index.html or play.png etc.
             */
            if (( strncmp( params->uri, "/cgi-bin", 8 ) == 0 ))
            {
                p_working_path = (char *) &params->uri[8];
                sprintf( plugin_name_with_dir, "./%s", PLUGIN_FILE_DEFAULT );
                FPRINTF( stderr,  "CHILD: p_working_path 1 (%s)  \n", p_working_path );
            }
            else if (( strncmp( params->uri, "/cgi", 4 ) == 0 ))
            {
                p_working_path = (char *) &params->uri[4];
                sprintf( plugin_name_with_dir, "./%s", PLUGIN_FILE_DEFAULT );
                FPRINTF( stderr,  "CHILD: p_working_path 2 (%s)  \n", p_working_path );
            }
            else
            {
                FPRINTF( stderr,  "CHILD: p_working_path 3 (%s)  \n", p_working_path );
            }

#if 1
            char *slash = strchr( &p_working_path[1], '/' );
            char *query = strchr( p_working_path, '?' );
            char  separator = 0;
            /*
               Example requests:
               plugin_launch(/cpu)
               plugin_launch(/wifi/powerStats)
               plugin_launch(/hdmi_output/output/videoFormat?exclude=thisisatest)
               plugin_launch(/system_cmd?script=nexus.client&cmd=play%20/mnt/nfs/demo_bin/streams/big_buck_bunny.ts)

               There are 5 cases we need to be concerned with:
               1) there is no slash and there is no query character
               2) there is a slash but no query
               3) there is a query but no slash
               4) both a slash and a query, but the slash comes before the query
               5) both a slash and a query, but the query comes before the slash
            */
            if ( slash && query ) {
                /* if both are found, the argument list should start from the slash ***IF*** it occurs before the query */
                if ( slash > query ) {
                    /* plugin_launch(/system_cmd?script=nexus.client&cmd=play%20/mnt/nfs/demo_bin/streams/bigbugbunny.ts)*/
                    plugin_args = query;
                } else {
                    /* plugin_launch(/hdmi_output/output/videoFormat?exclude=thisisatest) */
                    plugin_args = slash;
                }
            } else if ( slash ) {
                plugin_args = slash;
            } else if ( query ) {
                plugin_args = query;
            }
            if ( plugin_args ) {
                separator = plugin_args[0];
                plugin_args[0] = '\0'; /* null terminate the plugin name */
                plugin_args++; /* advance past the separator */
                p_query_string = plugin_args;
            }
            plugin_name = p_working_path;
#else
            plugin_name = strtok_r((char *) p_working_path, "/", &token_ptr );
            if (plugin_name)
            {
                plugin_args = strtok_r( NULL, "^", &token_ptr );
            }
#endif
            /*fprintf( stderr,  "CHILD: L%u URL (%s) ... plugin name (%s) ... args (%s) \n", __LINE__, p_working_path,
                  plugin_name, plugin_args ); */

            dup2( pipe_parent_read[WRITE_END], STDOUT_FILENO );
            RC_CHECK_GOTO(( rc != -1 ), ( "dup2(parent_read(WRITE_END) Failed" ), rc );
            /*fprintf( stderr, "%d %d.%06d - dup2( %d, %d) \n", getpid(), (int)tv.tv_sec%1000, (int)tv.tv_usec,
                  pipe_parent_read[WRITE_END], STDOUT_FILENO );*/
            close( pipe_parent_read[WRITE_END] );
            pipe_parent_read[WRITE_END] = 0;
            /*fprintf( stderr, "%d %d.%06d - close(%d) \n", getpid(), (int)tv.tv_sec%1000, (int)tv.tv_usec, pipe_parent_read[WRITE_END] );*/
            close( pipe_parent_read[READ_END] );
            pipe_parent_read[READ_END] = 0;
            /*fprintf( stderr, "%d %d.%06d - close(%d) \n", getpid(), (int)tv.tv_sec%1000, (int)tv.tv_usec, pipe_parent_read[READ_END] );*/

            /* fill in values from request header */
            envv[7] = s_user_agent;
            snprintf( s_user_agent, sizeof( s_user_agent ) - 1, "USER_AGENT=%s", params->s_user_agent );
            envv[8] = s_remote_ip_address;
            snprintf( s_remote_ip_address, sizeof( s_remote_ip_address ) - 1, "REMOTE_ADDR=%s", params->s_remote_ip_address );
            envv[9] = s_session_id;
            snprintf( s_session_id, sizeof( s_session_id ) - 1, "SESSION_ID=%s", pHash );

            /*fprintf( stderr,  "CHILD: L%u plugin_args (%s)  \n", __LINE__, plugin_args ); */
            if (p_working_path)
            {
                if (p_query_string)
                {
                    unsigned int bytes = 0;
                    bytes = snprintf( s_query_string, sizeof( s_query_string ) - 1, "QUERY_STRING=%s", p_query_string );
                    if (bytes == sizeof( s_query_string ))
                    {
                        fprintf( stderr, "%s - ERROR s_query_string max'ed out ... %u ... string len %u\n", __FUNCTION__,
                            sizeof( s_query_string ), (unsigned int)strlen( p_query_string ));
                    }
                    envv[10] = s_query_string;
                }
            }

            /*fprintf( stderr,  "CHILD: L%u plugin_name (%s) ... p_query_string (%s)  \n", __LINE__, plugin_name, p_query_string ); */
            /*fprintf( stderr,  "CHILD: L%u p_working_path (%s) ... cgi-bin ... args (%s) ... IsHtml %d) \n", __LINE__,
              p_working_path, plugin_args, b_url_path_is_html_file ); */

            if (strlen( p_working_path ) == 1)
            {
                sprintf( plugin_name_with_dir, "./%s", PLUGIN_FILE_DEFAULT );
            }
            else if (b_url_path_is_html_file)
            {
                /*FPRINTF( stderr,  "CHILD: IsHtmlFile - url (%s) ... cgi-bin ... args (%s)  \n", p_working_path, plugin_args ); */
                sprintf( plugin_name_with_dir, "./%s", PLUGIN_FILE_DEFAULT );
                argv[1] = &p_working_path[1];
            }
            else if (NULL != p_query_string)
            {
                /* add '?' back to query string */
                memset(qmark_query_string, 0, sizeof(qmark_query_string));
                /* separator could be either '?' or '/' .. we need the '?' character but do NOT need the '/' character */
                if ( separator == '/' )
                {
                    snprintf(qmark_query_string, sizeof(qmark_query_string), "%s", p_query_string);
                }
                else
                {
                    snprintf(qmark_query_string, sizeof(qmark_query_string), "%c%s", separator, p_query_string);
                }
                /*fprintf( stderr,  "CHILD: L%u qmark_query_string (%s) .. separator (%c)  \n", __LINE__, qmark_query_string, separator ); */

                if ( plugin_name[0] == '/' ) {
                    sprintf( plugin_name_with_dir, "./plugins%s", plugin_name ); /* executables are in the subdirectory "plugins" */
                } else {
                    sprintf( plugin_name_with_dir, "./plugins/%s", plugin_name ); /* executables are in the subdirectory "plugins" */
                }
                argv[1] = qmark_query_string;
                /*fprintf( stderr,  "CHILD: url (%s) ... ELSE ... args (%s)  ... argv[1] (%s) \n", p_working_path, plugin_args, argv[1] ); */
            }
            else
            {
                if ( plugin_name[0] == '/' ) {
                    sprintf( plugin_name_with_dir, "./plugins%s", plugin_name ); /* executables are in the subdirectory "plugins" */
                } else {
                    sprintf( plugin_name_with_dir, "./plugins/%s", plugin_name ); /* executables are in the subdirectory "plugins" */
                }
                argv[1] = p_query_string;
                /*fprintf( stderr,  "CHILD: L%u url (%s) ... ELSE ... args (%s)  ... argv[1] (%s) \n", __LINE__, p_working_path,
                  plugin_args, argv[1] ); */
            }

            /*FPRINTF( stderr,  "CHILD: argv[1] 1 (%s) \n", argv[1] ); */
            /* if the arguments to the plugin have not been set at this point, use defaults */
            if (argv[1] == NULL) {
                if ( plugin_args && strlen(plugin_args) ) {
                    argv[1] = plugin_args;
                } else {
                    argv[1] = PLUGIN_NAME_DEFAULT;
                }
            }
            FPRINTF( stderr,  "CHILD: argv[1] 2 (%s) \n", argv[1] );

            if (strstr( argv[0], "fileread" ) && argv[1] && ( strlen( argv[1] ) == 1 )) {argv[1] = HTML_NAME_DEFAULT; }

            /*fprintf( stderr,  "CHILD: execve(%s) ... argv0 (%s) ... argv1 (%s)  \n", plugin_name_with_dir, argv[0], argv[1] ); */
            execve( plugin_name_with_dir, (char *const *) argv, (char *const *) &envv[0] );

            /* If the execve() call succeeds, we will never get here */

            FPRINTF( stderr,  "CHILD: execve(%s) with argv1 (%s) FAILED  \n", argv[0], argv[1] );
            size_payload = -1;

            /* this is a forked child. when processing is done, simply exit. there is nothing left for us to do */
            exit(0);

            /*
            6 SERVER_ADMIN=
            7 HTTP_USER_AGENT=Mozilla/5.0 (Macintosh; Intel Mac OS X 10_11_6) AppleWebKit/537.36 Chrome/60.0.3112.113 Safari/537.36
            8 HTTP_REFERER=http://10.14.232.219/index.html
            9 HTTP_REFERER=http://10.14.232.219/index.html
           10 HTTP_ACCEPT_ENCODING=gzip, deflate
           11 HTTP_ACCEPT_LANGUAGE=en-US,en;q=0.8,co;q=0.6,fr;q=0.4
           13 HTTP_HOST=10.14.232.219
           14 SERVER_ADDR=10.14.232.219
             */
        }

        case -1:                                           /* this is an error */
        {
            RC_CHECK_GOTO(( 1 ), ( "fork() Failed" ), rc );
            break;
        }

        default:                                           /* this is the parent again */
        {
            int buffers = 1;                               /* number of payload buffers */
            int bytes   = 0;

#ifdef PLUGIN_LAUNCHER_DEBUG
            gettimeofday( &tv2, NULL );
#endif /* PLUGIN_LAUNCHER_DEBUG */

            usleep( 50 );
            close( pipe_parent_read[WRITE_END] );
            pipe_parent_read[WRITE_END] = 0;

            {
                int spaceAvail = params->size_payload;
                do
                {
                    FPRINTF( stderr,  "PARENT: reading from client %d ... space for %d bytes \n", pipe_parent_read[READ_END], BUFFER_SIZE - bytes_prev );
                    fflush( stdout ); fflush( stderr );
                    bytes = read( pipe_parent_read[READ_END], &params->p_payload[bytes_prev], MIN(LINUX_EMBEDDED_BUFFER_SIZE, spaceAvail) );
                    FPRINTF( stderr,  "PARENT: read(pipe %d) returned %d bytes \n", pipe_parent_read[READ_END], bytes );
                    FPRINTF( stderr,  "PARENT: bytes_prev:%d\n", bytes_prev);

                    /* if we received some bytes ... the number could be less than the total in the pipe */
                    if (bytes > 0)
                    {
                        FPRINTF( stderr,  "PARENT: recvd (%s) \n", &params->p_payload[bytes_prev] );
                        spaceAvail -= bytes;
                        bytes_prev += bytes;
                        buffers++;
                        FPRINTF( stderr,  "PARENT: buffer space avail (%d) \n", spaceAvail);
                    }
                    else if (bytes < 0)
                    {
                        FPRINTF( stderr,  "PARENT: read(%d) FAILED ... bytes %d ... params->size_payload %d ... (%s) \n", LINUX_EMBEDDED_BUFFER_SIZE,
                            bytes, params->size_payload, strerror( errno ));
                    }
                    else                                   /* no bytes were read; the other end of the pipe terminated */
                    {
                        FPRINTF( stderr,  "PARENT: read(pipe %d) is complete ... got %d bytes ... bytes_prev %d \n", pipe_parent_read[READ_END], bytes, bytes_prev );
                    }
                } while (bytes > 0);
            }

            size_payload = bytes + bytes_prev;
            FPRINTF( stderr,  "PARENT: %s - size_payload (%d) ... last byte 0x%02x \n", __FUNCTION__, size_payload, params->p_payload[size_payload-1] );

            /* if the forked child did not send any payload back to us, formulate a payload to send back to the browser */
            if (size_payload == 0)
            {
                /* initialize cJSON */
                objectRoot = json_Initialize();
                CHECK_PTR_ERROR_GOTO("Unable to allocate JSON object", objectRoot, rc, -1, error);

                /* generate JSON header */
                objectData = json_GenerateHeader(objectRoot, PLUGIN_LAUNCHER_NAME, PLUGIN_LAUNCHER_DESCRIPTION, NULL, PLUGIN_LAUNCHER_VERSION);
                CHECK_PTR_ERROR_GOTO("Unable to generate JSON header", objectData, rc, -1, json_error);

                json_AddNull(objectRoot, NO_FILTER, objectRoot, "data");
                json_GenerateError(objectRoot, BMON_CALLER_ERR_RES_NOT_FOUND, "Specified plugin failed to execve()");

json_error:

                /* copy JSON data to supplied buffer */
                rc = json_Print(objectRoot, params->p_payload, params->size_payload );
                CHECK_ERROR("Failure printing JSON to allocated buffer", rc);

                json_Uninitialize(&objectRoot);

                size_payload = strlen(params->p_payload);

                /*fprintf(stderr, "%s:L%u - rc (%d) .. (%s) \n", __FUNCTION__, __LINE__, rc, params->p_payload );*/
            }

#ifdef APPEND_PERFORMANCE_DATA
            /* if we are sending an entire file back to the user, don't bother appending performance data */
            if (b_url_path_is_html_file)
            {
            }
            else
            {
                gettimeofday( &tv3, NULL );

                size_payload = Bhttpd_AppendDurationDeltas( params->p_payload, BUFFER_SIZE*buffers, tv1, tv2, tv3, size_payload );
                RC_CHECK_GOTO(( size_payload > 0 ), ( "Bhttpd_AppendDurationDeltas() Failed" ), size_payload );
                FPRINTF( stderr,  "PARENT: %s - new sizepayload (%lu) \n", __FUNCTION__, size_payload );
            }
#endif /* ifdef APPEND_PERFORMANCE_DATA */

#ifdef PLUGIN_LAUNCHER_DEBUG
            gettimeofday( &tv2, NULL );
            /*fprintf( stderr, "%d %d.%06d - parent_read:  %d and %d\n", getpid(), (int)tv2.tv_sec%1000, (int)tv2.tv_usec, pipe_parent_read[0], pipe_parent_read[1] );*/
            microseconds2 = tv2.tv_sec;
            microseconds2 *= 1000000;
            microseconds2 += tv2.tv_usec;

            FPRINTF(stderr, "PARENT: delta %lu milliseconds\n", (microseconds2 - microseconds1)/1000 );
#endif /* PLUGIN_LAUNCHER_DEBUG */

            break;
        }
    }                                                      /* switch */

error:
    if (pipe_parent_read[WRITE_END])
    {
        close( pipe_parent_read[WRITE_END] );
        pipe_parent_read[WRITE_END] = 0;
    }

    if (pipe_parent_read[READ_END])
    {
        close( pipe_parent_read[READ_END] );
        pipe_parent_read[READ_END] = 0;
    }

    /* if a child process was successfully spawned, wait for it to exit */
    if (pid)
    {
        int status = 0;
        FPRINTF( stderr, "PARENT: %s L%d waitpid(%d)\n", __FUNCTION__, __LINE__, pid );
        wait( &status );
        FPRINTF( stderr, "PARENT: %s L%d waitpid(%d) returned status %d\n", __FUNCTION__, __LINE__, pid, status );
    }
    else
    {
        FPRINTF( stderr, "%s L%d waitpid(%d) skipped ... child failed to fork\n", __FUNCTION__, __LINE__, pid );
    }

    FPRINTF( stderr, "%s L%d pid (%d) returning (%d) \n", __FUNCTION__, __LINE__, pid, size_payload );
    return( size_payload );
}                                                          /* plugin_launch */

#if defined __cplusplus
}
#endif
