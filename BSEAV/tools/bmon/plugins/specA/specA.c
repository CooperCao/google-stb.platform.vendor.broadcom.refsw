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
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <assert.h>
#include "specA.h"

#include "src/cwebsocket/client.h"
#include "src/cwebsocket/subprotocol/echo/echo_client.h"

/*#define BMON_TRACE_ENABLE*/
#include "bmon_defines.h"
#include "bmon_convert_macros.h"
#include "bmon_json.h"
#include "bmon_uri.h"
#include "bmon_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "specA.h"
#include "plugin_header.h"
#include "bmon_convert_macros.h"

#define ENABLE_SSL
#define ENABLE_THREADS
#define FPRINTF nofprintf

static unsigned char recvd_data   = 0;
static unsigned int  fStartHz     = 700000000;
static unsigned int  fStopHz      = 800000000;
static unsigned int  fftSize      = 1024;
static unsigned int  numOfSamples = 1024;
static cJSON        *objectRootSave   = NULL;
static char         *payloadSave      = NULL;
static char          cmRequest[512];

static char  g_filter[128];
static char *g_cmResponse = NULL;
static char  default_uri[128];

static void OnMessage(
    void               *arg,
    cwebsocket_message *message
    )
{
    unsigned int messageLen = strlen( message->payload );

    /* if we got something back from the CM */
    if (messageLen)
    {
        g_cmResponse = calloc( 1, messageLen + 1 );
        if (g_cmResponse)
        {
            strncpy( g_cmResponse, message->payload, messageLen );
        }
    }

    recvd_data = 1;
} /* OnMessage */

/**
 *  Function: This function will
 *     1. parse the filter
 *     2. Collect required data
 *     3. Convert to json format
 *     4. Return number of bytes written to the buffer
 **/
int specA_get_data(
    const char  *filter,
    char        *json_output,
    unsigned int json_output_size
    )
{
    int               num_bytes        = 0;
    int               rc               = 0;
    static cJSON            *objectRoot       = NULL;
    static cJSON            *objectData       = NULL;
    static cJSON            *objectItem1      = NULL;
    static cwebsocket_client websocket_client;
    static char              cmip[32];
    static char              cmport[32];

    if (( json_output == NULL ) || ( json_output_size ==0 ))
    {
        return( -1 );
    }

    strncpy( default_uri, "ws://172.16.1.1:8080/Frontend", sizeof(default_uri) );

    /* if the user provided a filter string */
    if (filter)
    {
        char strStartHz[32];
        char strStopHz[32];
        char strfftSize[8];
        char strnumOfSamples[8];

        strncpy( g_filter, filter, sizeof( g_filter ) - 1 );

        memset( cmip, 0, sizeof( cmip ));
        memset( cmport, 0, sizeof( cmport ));
        memset( strStartHz, 0, sizeof( strStartHz ));
        memset( strStopHz, 0, sizeof( strStopHz ));
        memset( strfftSize, 0, sizeof( strfftSize ));
        memset( strnumOfSamples, 0, sizeof( strnumOfSamples ));

        /* did the user specify an alternate cm ip address */
        rc = bmon_uri_find_tagvalue( filter, "cmip", cmip, sizeof( cmip ));
        if (rc == 0)                                       /* if the user did NOT provide the IP address of the CM, use the default */
        {
            strncpy( cmip, "172.16.1.3", sizeof( cmip ));
        }

        /* did the user specify an alternate cm port */
        rc = bmon_uri_find_tagvalue( filter, "cmport", cmport, sizeof( cmport ));
        if (rc == 0)                                       /* if the user did NOT provide the port of the CM, use the default */
        {
            strncpy( cmport, "8080", sizeof( cmport ));
        }

        /* create the websocket request */
        snprintf( default_uri, 128/*sizeof( default_uri )*/, "ws://%s:%s/Frontend", cmip, cmport );

        /* did the user specify an alternate fStartHz */
        rc = bmon_uri_find_tagvalue( filter, "fStartHz", strStartHz, sizeof( strStartHz ));
        if (rc)                                            /* if the api found the string */
        {
            fStartHz = atoi( strStartHz );
        }

        /* did the user specify an alternate fStopHz */
        rc = bmon_uri_find_tagvalue( filter, "fStopHz", strStopHz, sizeof( strStopHz ));
        if (rc)                                            /* if the api found the string */
        {
            fStopHz = atoi( strStopHz );
        }

        /* did the user specify an alternate fftSize */
        rc = bmon_uri_find_tagvalue( filter, "fftSize", strfftSize, sizeof( strfftSize ));
        if (rc)                                            /* if the api found the string */
        {
            fftSize = atoi( strfftSize );
        }

        /* did the user specify an alternate numOfSamples */
        rc = bmon_uri_find_tagvalue( filter, "numOfSamples", strnumOfSamples, sizeof( strnumOfSamples ));
        if (rc)                                            /* if the api found the string */
        {
            numOfSamples = atoi( strnumOfSamples );
        }
    }

    memset( cmRequest, 0, sizeof( cmRequest ));
    /* create the cm request string */
    snprintf( cmRequest, sizeof( cmRequest ), "{\"jsonrpc\":\"2.0\",\"method\":\"Frontend::GetFrontendSpectrumData\",\"params\":{\"coreID\":0,\"fStartHz\":%u,\"fStopHz\":%u,\"fftSize\":%u,\"gain\":1,\"numOfSamples\":%u},\"id\":\"0\"}", fStartHz, fStopHz, fftSize, numOfSamples );


    /* initialize cJSON */
    objectRoot = json_Initialize();
    CHECK_PTR_ERROR_GOTO( "Unable to allocate JSON object", objectRoot, rc, -1, json_InitializeFailure );
    objectRootSave = objectRoot;

    /* generate JSON header */
    objectData = json_GenerateHeader( objectRoot, SPECA_PLUGIN_NAME, SPECA_PLUGIN_DESCRIPTION, NULL, SPECA_PLUGIN_VERSION );
    CHECK_PTR_ERROR_GOTO( "Unable to generate JSON header", objectData, rc, -1, json_GenerateHeaderFailure );

    /* add plug in data */
    websocket_client.subprotocol            = cwebsocket_subprotocol_echo_client_new(); // Hardcoding instead of negotiating
    websocket_client.subprotocol->name      = "rpc-frontend";
    websocket_client.subprotocol->onmessage = OnMessage;
    cwebsocket_client_init( &websocket_client, &websocket_client.subprotocol, 1 );
    websocket_client.uri = default_uri;

#if 0
    websocket_client.flags |= WEBSOCKET_FLAG_AUTORECONNECT; // OPTIONAL - retry failed connections
    websocket_client.retry  = 5;                            // OPTIONAL - seconds to wait before retrying
#endif

    rc = cwebsocket_client_connect( &websocket_client );
    /* sometimes ... after this call ... the objectRoot pointer has been nulled */
    if ( objectRoot == NULL ) {
        objectRoot = objectRootSave;
    }

    objectItem1 = json_AddArrayElement( objectData );
    CHECK_PTR_GOTO( objectItem1, objectItemError );
    json_AddString( objectItem1, g_filter, objectData, "cmip", cmip );
    json_AddString( objectItem1, g_filter, objectData, "cmport", cmport );

    if ( rc == -1)
    {
        FPRINTF( stderr, "L%u connect failure ... json_AddArrayElement!!! json_output (%p) objectRoot (%p)  \n", __LINE__, json_output, objectRoot );
        fflush( stderr );

        json_AddString( objectItem1, g_filter, objectData, "status", "connect failure" );

        goto connectFailure;
    }
    cwebsocket_client_write_data( &websocket_client, cmRequest, strlen( cmRequest ), TEXT_FRAME );
    cwebsocket_client_read_data( &websocket_client );

    /* wait for the read callback to trigger and fill the global string with the cm response */
    while (recvd_data == 0) {
        usleep( 100000 );
    }

    /* if the cm responded with something */
    if (g_cmResponse)
    {
        json_AddString( objectItem1, g_filter, objectData, "status", "success" );
    }

json_InitializeFailure:

json_GenerateHeaderFailure:

connectFailure:

objectItemError:

error:

    /* copy JSON data to supplied buffer */
    rc = json_Print( objectRoot, json_output, json_output_size );
    CHECK_ERROR( "Failure printing JSON to allocated buffer", rc );

    FPRINTF(stderr, "L%u g_cmResponse (%p) json_output len %d \n", __LINE__, g_cmResponse, strlen(json_output) );fflush(stderr);
    if (g_cmResponse)
    {
        /* The end of the JSON should be ... "data":[{"status":"connect failure"}]\} */
        unsigned int json_len = strlen( json_output );
        if (json_len > 10)
        {
            char *end_of_json = &json_output[json_len-2];

            /* if the output ends in the two expected characters, then append the CM response string */
            if (( end_of_json[0] == ']' ) && ( end_of_json[1] == '}' ))
            {
                end_of_json--; /* point to the ending brace for data structure */
                *end_of_json = '\0'; /* we do not want the part of the string after this */

                strncat( end_of_json, ",", json_output_size - json_len );

                json_len++;

                end_of_json++;

                /* copy the cm response to the end of the running json response */
                strncpy( end_of_json, &g_cmResponse[1], json_output_size - json_len );

                /* compute the new length that includes the response from the cm */
                json_len = strlen( json_output );

                /* add the two ending characters that were overwritten by the cm response */
                strncat( json_output, "]}", json_output_size - json_len );
            }
        }

        free( g_cmResponse );
    }
    else
    {
    }

#if 0
    json_Uninitialize( &objectRoot );
#endif

    if (0 <= rc)
    {
        /* return size of data if not error value */
        rc = strlen( json_output );
    }

    return( rc );
} /* specA_get_data */

#if defined ( BMON_PLUGIN )
/**
 *  Function: This function will coordinate collecting power data and once that is done,
 *            it will convert the power data to a JSON format and send the JSON data back
 *            to the browser or curl or wget.
 **/
#define PAYLOAD_SIZE 20480
int main(
    int   argc,
    char *argv[]
    )
{
    int   ret     = 0;
    static char *payload = NULL;
    char *argv1Decoded = NULL;
    char  argv1Default[] = "/";
    int   argv1DecodedSize = 0;

    payload = malloc( PAYLOAD_SIZE );
    if (payload == NULL)
    {
        return( -1 );
    }
    payloadSave = payload;

    /* if the user provided some filter parameters */
    if ( argc > 1 )
    {
        /* the decoded size should ALWAYS be smaller than the encoded size */
        argv1DecodedSize = strlen( argv[1] ) + 1;
        if ( argv1DecodedSize )
        {
            /* allocate space for the decoded uri */
            argv1Decoded = calloc( 1, argv1DecodedSize );
            if ( argv1Decoded == NULL )
            {
                fprintf(stderr, "%s - could not calloc(%d) bytes for argv1Decoded buffer \n", argv[0], argv1DecodedSize );
            } else {
                bmon_uri_decode( argv[1], argv1Decoded, argv1DecodedSize );
            }
        }
    }

    /* if there is no valid filter parameter, use a default one */
    if ( argv1Decoded == NULL )
    {
        argv1Decoded = argv1Default;
    }

    ret = specA_get_data( argv1Decoded, payload, PAYLOAD_SIZE );

    /* Sometimes ... the specA_get_data() API causes the payload pointer to be nulled. Appears to be a problem with cwebsocket API */
    if ( payload == NULL && payloadSave ) {
        FPRINTF( stderr, "L%u restored payload (%p) \n", __LINE__, ret, payload ); fflush( stderr );
        payload = payloadSave;
    }

    /* send response back to user */
    if (payload)
    {
        printf( "%s\n", payload );
        free( payload );
    }

    /* if space was allocated for the decoded uri string, free it now */
    if ( argv1Decoded && argv1Decoded != argv1Default )
    {
        free( argv1Decoded );
    }

    return( 0 );
} /* main */

#endif /* defined(BMON_PLUGIN) */
