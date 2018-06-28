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
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
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
#endif /* if NXCLIENT_SUPPORT */

#if NEXUS_HAS_AUDIO
#include "nexus_audio_decoder.h"
#endif

#include "audio.h"

/*#define BMON_TRACE_ENABLE*/
#include "bmon_defines.h"
#include "bmon_convert.h"
#include "bmon_json.h"

#define FPRINTF  NOFPRINTF /* fprintf to enable, NOFPRINTF to disable */

#define SLASH    "/"
#define ADD_SLASH(a)  SLASH a

/**
 *  Function: This function will collect all requred audio data and output in JSON format
 **/
int audio_get_data(
        const char * filter,
        char *       data,
        size_t       data_size
        )
{
    int                          rc         = 0;
    cJSON *                      objectRoot = NULL;
    cJSON *                      objectData = NULL;
    size_t                       numObjects = 0;
    int                          i          = 0;
    NEXUS_InterfaceName          interfaceName;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
    NEXUS_Error                  nxrc;

    assert(NULL != filter);
    assert(NULL != data);
    assert(0 < data_size);

    /* initialize cJSON */
    objectRoot = json_Initialize();
    CHECK_PTR_ERROR_GOTO("Unable to allocate JSON object", objectRoot, rc, -1, error);

    /* generate JSON header */
    objectData = json_GenerateHeader(objectRoot, AUDIO_PLUGIN_NAME, AUDIO_PLUGIN_DESCRIPTION, NULL, AUDIO_PLUGIN_VERSION);
    CHECK_PTR_ERROR_GOTO("Unable to generate JSON header", objectData, rc, -1, error);

#if NEXUS_HAS_AUDIO
    NEXUS_Platform_GetDefaultInterfaceName(&interfaceName);
    strcpy(interfaceName.name, "NEXUS_AudioDecoder");

    nxrc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &numObjects);
    CHECK_NEXUS_ERROR_GOTO("Failure retrieving Nexus platform objects", rc, nxrc, error);

    for (i = 0; (i < numObjects) && (i < NEXUS_NUM_PARSER_BANDS); i++)
    {
        cJSON *                  objectAudioDecoder = NULL;
        NEXUS_AudioDecoderStatus status;

        /* get audio decoder data from Nexus */
        nxrc = NEXUS_AudioDecoder_GetStatus(objects[i].object, &status);
        CHECK_NEXUS_ERROR_GOTO("Failture retrieving audio decoder status", rc, nxrc, error);

        /* generate JSON for audio decoder status */

        /* do not use filter for audio decoder - we always want this included for each output */
        objectAudioDecoder = json_AddObject(objectData, NO_FILTER, objectData, "decoder");
        CHECK_PTR_ERROR_GOTO("Failure adding audio decoder JSON object", objectAudioDecoder, rc, -1, error);

        /* do not use filter for index - we always want this included */
        json_AddNumber(objectAudioDecoder, NO_FILTER, objectData, "index", i);

        json_AddBool(objectAudioDecoder, filter, objectData, "started", status.started);
        json_AddBool(objectAudioDecoder, filter, objectData, "locked", status.locked);

        json_AddString(objectAudioDecoder, filter, objectData, "codec", audioCodecToString(status.codec));
        json_AddNumber(objectAudioDecoder, filter, objectData, "sampleRate", status.sampleRate);
        json_AddNumber(objectAudioDecoder, filter, objectData, "framesDecoded", status.framesDecoded);
        json_AddNumber(objectAudioDecoder, filter, objectData, "frameErrors", status.frameErrors);
        json_AddNumber(objectAudioDecoder, filter, objectData, "dummyFrames", status.dummyFrames);
        json_AddNumber(objectAudioDecoder, filter, objectData, "numBytesDecoded", status.numBytesDecoded);
        json_AddNumber(objectAudioDecoder, filter, objectData, "numWatchdogs", status.numWatchdogs);

        json_AddNumber(objectAudioDecoder, filter, objectData, "numFifoOverflows", status.numFifoOverflows);
        json_AddNumber(objectAudioDecoder, filter, objectData, "numFifoUnderflows", status.numFifoUnderflows);
        json_AddNumber(objectAudioDecoder, filter, objectData, "fifoDepth", status.fifoDepth);
        json_AddNumber(objectAudioDecoder, filter, objectData, "fifoSize", status.fifoSize);
        json_AddNumber(objectAudioDecoder, filter, objectData, "queuedFrames", status.queuedFrames);
        json_AddNumber(objectAudioDecoder, filter, objectData, "queuedOutput", status.queuedOutput);

        json_AddNumber(objectAudioDecoder, filter, objectData, "pts", status.pts);
        json_AddString(objectAudioDecoder, filter, objectData, "ptsType", ptsTypeToString(status.ptsType));
        json_AddNumber(objectAudioDecoder, filter, objectData, "ptsStcDifference", status.ptsStcDifference);
        json_AddNumber(objectAudioDecoder, filter, objectData, "ptsErrorCount", status.ptsErrorCount);
    }
#endif /* NEXUS_HAS_AUDIO */

error:
    /* copy JSON data to supplied buffer */
    rc = json_Print(objectRoot, data, data_size);
    CHECK_ERROR("Failure printing JSON to allocated buffer", rc);

    json_Uninitialize(&objectRoot);

    if (0 <= rc)
    {
        /* return size of data if not error value */
        rc = strlen(data);
    }

    return(rc);
} /* audio_get_data */

#if defined (BMON_PLUGIN)
#define PAYLOAD_SIZE  (10 * 1024)
/**
 *  Function: This function will coordinate collecting audio data and send the JSON data back
 *            to the browser or curl or wget.
 **/
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

#if NEXUS_HAS_AUDIO
    {
        NxClient_JoinSettings joinSettings;
        NEXUS_Error           nxrc;

        /* connect to NxServer */
        NxClient_GetDefaultJoinSettings(&joinSettings);
        joinSettings.mode = NEXUS_ClientMode_eVerified;
        nxrc              = NxClient_Join(&joinSettings);
        CHECK_NEXUS_ERROR_GOTO("Failure NxClient Join", rc, nxrc, errorNxClient);
    }
#endif /* if NEXUS_HAS_AUDIO */

    /* if the caller provided a filter, use the caller's filter */
    if (argc > 1)
    {
        pFilter = argv[1];
    }

    memset(payload, 0, sizeof(payload));

    rc = audio_get_data(pFilter, payload, PAYLOAD_SIZE);
    CHECK_ERROR_GOTO("Failure retrieving Audio Decoder data from Nexus", rc, error);

    /* send response back to user */
    printf("%s\n", payload);
    fflush(stdout);

error:

#if NEXUS_HAS_HDMI_OUTPUT
    NxClient_Uninit();
errorNxClient:
#endif

    return(rc);
} /* main */

#endif /* defined(BMON_PLUGIN) */