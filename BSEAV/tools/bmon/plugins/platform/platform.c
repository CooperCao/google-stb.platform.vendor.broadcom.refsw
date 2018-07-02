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
#include <arpa/inet.h>
#include <errno.h>
#include <assert.h>

#include "platform.h"
#include "bmon_get_file_contents_proc.h"

/*#define BMON_TRACE_ENABLE*/
#include "bmon_defines.h"
#include "bmon_json.h"

/**
 *  Function: This function will
 *     1. parse the filter
 *     2. Collect required data
 *     3. Convert to json format
 *     4. Return number of bytes written to the buffer
 **/
int platform_get_data(
    const char  *filter,
    char        *json_output,
    unsigned int json_output_size
    )
{
    int            rc           = 0;
    int            i            = 0;
    char *         contents     = NULL;
    bmon_platform_t platform_data;
    cJSON *     objectRoot = NULL;
    cJSON *     objectData = NULL;

    assert(NULL != filter);
    assert(NULL != json_output);
    assert(0 < json_output_size);

    memset( &platform_data, 0, sizeof( platform_data ));

    /* initialize cJSON */
    objectRoot = json_Initialize();
    CHECK_PTR_ERROR_GOTO("Unable to allocate JSON object", objectRoot, rc, -1, error);

    /* generate JSON header */
    objectData = json_GenerateHeader(objectRoot, PLATFORM_PLUGIN_NAME, PLATFORM_PLUGIN_DESCRIPTION, NULL, PLATFORM_PLUGIN_VERSION);
    CHECK_PTR_ERROR_GOTO("Unable to generate JSON header", objectData, rc, -1, error);

    /* Get system name, release number and hostname */
    uname( &platform_data.uts );

    contents = bmon_get_file_contents_proc( "/proc/device-tree/bolt/board", sizeof( platform_data.bolt_board ));
    if (contents)
    {
        strncpy( platform_data.bolt_board, contents, sizeof( platform_data.bolt_board ) -1 );
        free( contents );
    }

    contents = bmon_get_file_contents_proc( "/proc/device-tree/bolt/tag", sizeof( platform_data.bolt_version ));
    if (contents)
    {
        strncpy( platform_data.bolt_version, contents, sizeof( platform_data.bolt_version ) -1 );
        free( contents );
    }

    contents = bmon_get_file_contents_proc( "/proc/device-tree/bolt/family-id", sizeof( platform_data.bolt_family_id ));
    if (contents)
    {
        memcpy((void *) &platform_data.bolt_family_id, contents, sizeof( platform_data.bolt_family_id ));
        free( contents );
        platform_data.bolt_family_id.platform_id      = htons( platform_data.bolt_family_id.platform_id );
        platform_data.bolt_family_id.platform_version = htons( platform_data.bolt_family_id.platform_version );
    }

    {
        cJSON * objectItem = NULL;

        objectItem = json_AddArrayElement(objectData);
        CHECK_PTR_GOTO(objectItem, skipData);

        {
            char strTmp[32];
            cJSON * objectBolt = NULL;

            objectBolt = json_AddObject(objectItem, filter, objectData, "bolt");
            CHECK_PTR_GOTO(objectBolt, skipBolt);

            json_AddString(objectBolt, filter, objectData, "board", platform_data.bolt_board);
            json_AddString(objectBolt, filter, objectData, "version", platform_data.bolt_version);

            snprintf(strTmp, sizeof(strTmp), "%x", platform_data.bolt_family_id.platform_id);
            json_AddString(objectBolt, filter, objectData, "platformId", strTmp);

            snprintf(strTmp, sizeof(strTmp), "%04x", platform_data.bolt_family_id.platform_version);
            json_AddString(objectBolt, filter, objectData, "platformVersion", strTmp);
        }
skipBolt:

        {
            cJSON * objectUts = NULL;
            objectUts = json_AddObject(objectItem, filter, objectData, "uts");
            CHECK_PTR_GOTO(objectUts, skipUts);

            json_AddString(objectUts, filter, objectData, "sysName", platform_data.uts.sysname);
            json_AddString(objectUts, filter, objectData, "release", platform_data.uts.release);
            json_AddString(objectUts, filter, objectData, "machine", platform_data.uts.machine);
            json_AddString(objectUts, filter, objectData, "nodeName", platform_data.uts.nodename);
            json_AddString(objectUts, filter, objectData, "version", platform_data.uts.version);
        }
    }
skipUts:
skipData:

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
} /* platform_get_data */

#if defined ( BMON_PLUGIN )
/**
 *  Function: This function will coordinate collecting platform data and once that is done,
 *            it will convert the platform data to a JSON format and send the JSON data back
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

    rc = platform_get_data(pFilter, payload, PAYLOAD_SIZE);
    CHECK_ERROR_GOTO("Failure retrieving platform data", rc, error);

    /* send response back to user */
    printf( "%s\n", payload );
    fflush(stdout);

error:
    return(rc);
} /* main */

#endif /* defined(BMON_PLUGIN) */
