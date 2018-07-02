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

#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

#if NXCLIENT_SUPPORT
#include "nxclient.h"
#include "nexus_platform_client.h"
#else
#include "nexus_platform.h"
#endif

#include "nxclient.h"
#include "nexus_platform.h"
#include "nexus_pid_channel.h"

#if NEXUS_HAS_TRANSPORT
#include "nexus_playpump.h"
#include "nexus_parser_band.h"
#endif

#if 0
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#endif

#include "transport.h"

/*#define BMON_TRACE_ENABLE*/
#include "bmon_defines.h"
#include "bmon_json.h"
#include "bmon_utils.h"
#include "bmon_nx.h"

#define APPEND_STRING( mystring )           \
    num_bytes = strlen( mystring );         \
    strncat( buf, mystring, buf_size - 1 ); \
    buf      += num_bytes;                  \
    buf_size -= num_bytes;                  \
    /*fprintf(stderr, "%s - num_bytes (%d) ... buf_size (%d) ... (%s) \n", __FUNCTION__, num_bytes, buf_size, mystring );*/

static unsigned char PidChannelInitialization[BMON_TRANSPORT_NUM_VIDEO_DECODERS];

/**
 *  Function: This function will
 *     1. parse the filter
 *     2. Collect required data
 *     3. Convert to json format
 *     4. Return number of bytes written to the buffer
 **/
int transport_get_data(
    const char  *filter,
    char        *json_output,
    unsigned int json_output_size
    )
{
    int                          rc         = 0;
    cJSON *                      objectRoot = NULL;
    cJSON *                      objectData = NULL;
    cJSON *                      objectContexts = NULL;
    int                          i          = 0;
    NEXUS_InterfaceName          interfaceName;
    int                   ch_number    = 0;
    struct timeval        tv           = {0,0};
    bmon_transport_t      transport_data;
    NEXUS_Error                  nxrc;

    assert(NULL != filter);
    assert(NULL != json_output);
    assert(0 < json_output_size);

    /* initialize cJSON */
    objectRoot = json_Initialize();
    CHECK_PTR_ERROR_GOTO("Unable to allocate JSON object", objectRoot, rc, -1, error);

    /* generate JSON header */
    objectData = json_GenerateHeader(objectRoot, TRANSPORT_PLUGIN_NAME, TRANSPORT_PLUGIN_DESCRIPTION, NULL, TRANSPORT_PLUGIN_VERSION);
    CHECK_PTR_ERROR_GOTO("Unable to generate JSON header", objectData, rc, -1, error);

    memset( &transport_data, 0, sizeof( transport_data ));
    memset( &PidChannelInitialization, 0, sizeof( PidChannelInitialization ));

    /* add plug in data */

    gettimeofday( &tv, NULL );

#if NEXUS_HAS_VIDEO_DECODER
    {
        video_decoder_info_t         VideoDecoderInfo;
        NEXUS_InterfaceName          interfaceName;
        NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
        size_t num;
        int    i = 0;

        memset( &VideoDecoderInfo, 0, sizeof( VideoDecoderInfo ));

        NEXUS_Platform_GetDefaultInterfaceName( &interfaceName );
        strcpy( interfaceName.name, "NEXUS_VideoDecoder" );

        rc = NEXUS_Platform_GetObjects( &interfaceName, objects, MAX_OBJECTS, &num );
        BDBG_ASSERT( !rc );

        FPRINTF( stderr, "vdec %d\n", num );
        for (i = 0; i<num && i<BMON_TRANSPORT_NUM_VIDEO_DECODERS; i++)
        {
            VideoDecoderInfo.videoDecoderIndex            = i;
            transport_data.transport[i].videoDecoderIndex = i;
            bmon_process_video_decoder( i, objects[i].object, &ch_number, &VideoDecoderInfo ); /* returns ch_number */

            bmon_parser_band_init( VideoDecoderInfo.band );
            bmon_pid_channel_init( VideoDecoderInfo.pidChannelIndex, &PidChannelInitialization[0] );
            transport_data.transport[i].pidChannelIndex = VideoDecoderInfo.pidChannelIndex;

            transport_data.transport[i].ch_status          = 1;
            transport_data.transport[i].live_vs_playback   = 1;
            transport_data.transport[i].pb_full_percentage = VideoDecoderInfo.fifoSize ? (unsigned)( VideoDecoderInfo.fifoDepth*100/VideoDecoderInfo.fifoSize ) : 0;
            transport_data.transport[i].cdb_queueDepth     = VideoDecoderInfo.queueDepth;
            transport_data.transport[i].xpt_err.tei_err    = VideoDecoderInfo.errors.teiErrors;
            transport_data.transport[i].xpt_err.cc_err     = VideoDecoderInfo.errors.ccErrors;

            if (ch_number >= PLAYBACK_CHANNEL_OFFSET)
            {
                bmon_find_and_process_playpump( VideoDecoderInfo.band, &transport_data.transport[i].xpt_err, &transport_data.transport[i].instant_rate );
            }
        }
    }

    /* do not use filter for hdmi_output - we always want this included for each output */
    objectContexts = json_AddArray(objectData, NO_FILTER, objectData, "contexts");
    CHECK_PTR_ERROR_GOTO("Failure adding transport JSON object", objectContexts, rc, -1, error);

    for (i = 0; i<BMON_TRANSPORT_NUM_VIDEO_DECODERS; i++)
    {
        cJSON * objectItem = NULL;

        if (transport_data.transport[i].ch_status)
        {
            cJSON * objectXptError = NULL;

            objectItem = json_AddArrayElement(objectContexts);
            CHECK_PTR_GOTO(objectItem, skipContexts);

            json_AddBool(objectItem, filter, objectData, "chStatus", (1 == transport_data.transport[i].ch_status) ? true : false);
            json_AddBool(objectItem, filter, objectData, "liveVsPlayback", (1 == transport_data.transport[i].live_vs_playback) ? true : false);
            json_AddNumber(objectItem, filter, objectData, "pbFullPercentage", transport_data.transport[i].pb_full_percentage);
            json_AddNumber(objectItem, filter, objectData, "cdbQueueDepth", transport_data.transport[i].cdb_queueDepth);

            {
                objectXptError = json_AddObject(objectItem, filter, objectData, "xptError");
                CHECK_PTR_GOTO(objectXptError, skipXptError);

                json_AddNumber(objectXptError, filter, objectData, "tei", transport_data.transport[i].xpt_err.tei_err);
                json_AddNumber(objectXptError, filter, objectData, "cc", transport_data.transport[i].xpt_err.cc_err);
                json_AddNumber(objectXptError, filter, objectData, "cc", transport_data.transport[i].xpt_err.oos_err);
            }
skipXptError:

            json_AddNumber(objectItem, filter, objectData, "pidChannelIndex", transport_data.transport[i].pidChannelIndex);
        }
    }
skipContexts:
#endif /* NEXUS_HAS_VIDEO_DECODER */

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
}                                                          /* transport_get_data */

#if defined ( BMON_PLUGIN )
/**
 *  Function: This function will coordinate collecting transport data and once that is done,
 *            it will convert the transport data to a JSON format and send the JSON data back
 *            to the browser or curl or wget.
 **/
#define PAYLOAD_SIZE 2048
int main(
    int    argc,
    char * argv[],
    char * envv[]
    )
{
    int    rc              = 0;
    char   filterDefault[] = "/";
    char * pFilter         = filterDefault;
    char   payload[PAYLOAD_SIZE];

#if NEXUS_HAS_HDMI_OUTPUT
    {
        NxClient_JoinSettings joinSettings;
        NEXUS_Error           nxrc;

        /* connect to NxServer */
        NxClient_GetDefaultJoinSettings(&joinSettings);
        joinSettings.mode = NEXUS_ClientMode_eVerified;
        nxrc              = NxClient_Join(&joinSettings);
        CHECK_NEXUS_ERROR_GOTO("Failure NxClient Join", rc, nxrc, errorNxClient);
    }
#endif /* if NEXUS_HAS_HDMI_OUTPUT */

    /* if the caller provided a filter, use the caller's filter */
    if (argc > 1)
    {
        pFilter = argv[1];
    }

    memset(payload, 0, sizeof(payload));

    rc = transport_get_data(pFilter, payload, PAYLOAD_SIZE);
    CHECK_ERROR_GOTO("Failure retrieving transport data from Nexus", rc, error);

    /* send response back to user */
    printf("%s\n", payload);
    fflush(stdout);

error:

#if NEXUS_HAS_HDMI_OUTPUT
    NxClient_Uninit();
errorNxClient:
#endif

    return(rc);
}                                                          /* main */

#endif /* defined(BMON_PLUGIN) */
