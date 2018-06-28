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
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <assert.h>

#include "network.h"
#include "bmon_get_file_contents_proc.h"
#include "bmon_get_my_ip_addr_from_ifname.h"

/*#define BMON_TRACE_ENABLE*/
#include "bmon_defines.h"
#include "bmon_json.h"

static int bmon_get_proc_net_dev( bmon_network_t *pnetwork_data )
{
    char *contents        = NULL;
    char *token           = NULL;
    int   token_count     = 0;
    int   interface_count = 0;
    char *pos             = NULL;

    do {
        contents = bmon_get_file_contents_proc( NETWORK_STATS_FILENAME, 2048 );
        if ( contents == NULL ) {
            fprintf( stderr, "%s - could not open (%s) \n", __FUNCTION__, NETWORK_STATS_FILENAME );
            break;
        }

        /*fprintf( stderr, "%s - got contents (%s)\n", __FUNCTION__, contents );*/

        pos = contents;

        pos = strchr( pos, '\n' ); /* find end of first line */
        if ( pos == NULL ) break;

        pos++;
        pos = strchr( pos, '\n' ); /* find end of second line */
        if ( pos == NULL ) break;

        pos++;

        token = strtok( pos, " " );
        while (token)
        {
            //fprintf( stderr, "tok [%d] (%s) \n", token_count, token );

            if ( token_count == 0 ) { /* first element on the line is the interface name */
                if ( strlen(token) > 1 && token[strlen(token)-1] == ':' ) {
                    token[strlen(token)-1] = '\0'; /* get rid of colon at the end of the interface name */
                }
                strncpy( &pnetwork_data->interface[interface_count].name[0], token, sizeof( pnetwork_data->interface[interface_count].name ) );
                bmon_get_my_ip_addr_from_ifname( token, &pnetwork_data->interface[interface_count].ipv4_address[0], sizeof(pnetwork_data->interface[interface_count].ipv4_address ) );
                //fprintf( stderr, "%s - token (%s) ... name[%d] is (%s) ... addr (%s)\n", __FUNCTION__, token, interface_count,
                //        pnetwork_data->interface[interface_count].name, pnetwork_data->interface[interface_count].ipv4_address );
            } else if ( token_count == 1 ) { /* rx_bytes */
                sscanf( token, "%llu", &pnetwork_data->interface[interface_count].rx_bytes );
                //fprintf( stderr, "%s - token (%s) ... rx_bytes %llu \n", __FUNCTION__, token, pnetwork_data->interface[interface_count].rx_bytes );
            } else if ( token_count == 3 ) { /* rx_errors */
                sscanf( token, "%llu", &pnetwork_data->interface[interface_count].rx_errors );
                //fprintf( stderr, "%s - token (%s) ... rx_errors %llu \n", __FUNCTION__, token, pnetwork_data->interface[interface_count].rx_errors );
            } else if ( token_count == 9 ) { /* tx_bytes */
                sscanf( token, "%llu", &pnetwork_data->interface[interface_count].tx_bytes );
                //fprintf( stderr, "%s - token (%s) ... tx_bytes %llu \n", __FUNCTION__, token, pnetwork_data->interface[interface_count].tx_bytes );
            } else if ( token_count == 11 ) { /* tx_errors */
                sscanf( token, "%llu", &pnetwork_data->interface[interface_count].tx_errors );
                //fprintf( stderr, "%s - token (%s) ... tx_errors %llu \n", __FUNCTION__, token, pnetwork_data->interface[interface_count].tx_errors );
            }

            token_count++;

            if ( strchr(token, '\n') ) { /* if end of this line */
                token_count = 0;
                interface_count++;
            }

            token = strtok( NULL, " " );
        }

        free( contents );
    } while ( 0 );

    return( 0 );
}
/**
 *  Function: This function will collect all requred network data and output the JSON object.
 **/
int network_get_data( const char *filter, char *json_output, unsigned int json_output_size )
{
    int            rc           = 0;
    int            i            = 0;
    bmon_network_t network_data;
    cJSON *     objectRoot = NULL;
    cJSON *     objectData = NULL;

    assert(NULL != filter);
    assert(NULL != json_output);
    assert(0 < json_output_size);

    memset( &network_data, 0, sizeof(network_data) );

    /* initialize cJSON */
    objectRoot = json_Initialize();
    CHECK_PTR_ERROR_GOTO("Unable to allocate JSON object", objectRoot, rc, -1, error);

    /* generate JSON header */
    objectData = json_GenerateHeader(objectRoot, NETWORK_PLUGIN_NAME, NETWORK_PLUGIN_DESCRIPTION, NULL, NETWORK_PLUGIN_VERSION);
    CHECK_PTR_ERROR_GOTO("Unable to generate JSON header", objectData, rc, -1, error);

    bmon_get_proc_net_dev( &network_data );

    {
        cJSON * objectNetworks = NULL;
        cJSON * objectItem      = NULL;

        /* do not use filter for history - we always want this included for each output */
        objectNetworks = json_AddArray(objectData, NO_FILTER, objectData, "networks");
        CHECK_PTR_ERROR_GOTO("Failure adding ir history array JSON object", objectNetworks, rc, -1, error);

        for (i = 0; i<BMON_MAX_NUM_NETWORKS; i++)
        {
            objectItem = json_AddArrayElement(objectNetworks);
            CHECK_PTR_GOTO(objectItem, skipNetworks);

            /* we have reached the end of known networks */
            if ( network_data.interface[i].name[0] == '\0' )
            {
                break;
            }

            json_AddString(objectItem, filter, objectData, "name", network_data.interface[i].name);
            json_AddString(objectItem, filter, objectData, "ipv4Address", network_data.interface[i].ipv4_address);
            json_AddString(objectItem, filter, objectData, "ipv6Address", network_data.interface[i].ipv6_address);
            json_AddNumber(objectItem, filter, objectData, "rxBytes", network_data.interface[i].rx_bytes);
            json_AddNumber(objectItem, filter, objectData, "txBytes", network_data.interface[i].tx_bytes);
            json_AddNumber(objectItem, filter, objectData, "rxErrors", network_data.interface[i].rx_errors);
            json_AddNumber(objectItem, filter, objectData, "txErrors", network_data.interface[i].tx_errors);
        }
    }
skipNetworks:

error:
    /* copy JSON data to supplied buffer */
    rc = json_Print(objectRoot, json_output, json_output_size);
    CHECK_ERROR("Failure printing JSON to allocated buffer", rc);

    json_Uninitialize(&objectRoot);

    if (0 <= rc)
    {
        /* return size of data if not error value */
        rc = strlen(json_output);
    }
    return(rc);
}

#if defined ( BMON_PLUGIN )
/**
 *  Function: This function will coordinate collecting network data and once that is done,
 *            it will convert the network data to a JSON format and send the JSON data back
 *            to the browser or curl or wget.
 **/
#define PAYLOAD_SIZE 2048
int main(
    int   argc,
    char *argv[]
    )
{
    int    rc = 0;
    char   filterDefault[] = "/";
    char * pFilter         = filterDefault;
    char   payload[PAYLOAD_SIZE];

    /* if the caller provided a filter, use the caller's filter */
    if (argc > 1)
    {
        pFilter = argv[1];
    }

    memset(payload, 0, sizeof(payload));

    rc = network_get_data(pFilter, payload, PAYLOAD_SIZE);
    CHECK_ERROR_GOTO("Failure retrieving network data", rc, error);

    /* send response back to user */
    printf( "%s\n", payload );
    fflush(stdout);

error:
    return(rc);
}                                                          /* main */

#endif /* defined(BMON_PLUGIN) */
