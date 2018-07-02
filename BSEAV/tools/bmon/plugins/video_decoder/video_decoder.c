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
#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <inttypes.h>
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
#include "nexus_video_decoder_extra.h"

#include "video_decoder.h"

/*#define BMON_TRACE_ENABLE*/
#include "bmon_defines.h"
#include "bmon_convert.h"
#include "bmon_json.h"
#include "bmon_nx.h"

#define FPRINTF  NOFPRINTF /* fprintf to enable, NOFPRINTF to disable */

/**
 *  Function: This function will collect all required video decoder data and output in JSON format
 **/
int video_decoder_get_data(
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
    objectData = json_GenerateHeader(objectRoot, VIDEO_DECODER_PLUGIN_NAME, VIDEO_DECODER_PLUGIN_DESCRIPTION, NULL, VIDEO_DECODER_PLUGIN_VERSION);
    CHECK_PTR_ERROR_GOTO("Unable to generate JSON header", objectData, rc, -1, error);

#if NEXUS_HAS_VIDEO_DECODER
    NEXUS_Platform_GetDefaultInterfaceName(&interfaceName);
    strcpy(interfaceName.name, "NEXUS_VideoDecoder");

    nxrc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &numObjects);
    CHECK_NEXUS_ERROR_GOTO("Failure retrieving Nexus platform objects", rc, nxrc, error);

    /* This can't be NEXUS_NUM_VIDEO_DECODERS because one video decoder can have up to four contexts ... when doing mosaic */
    for (i = 0; (i < numObjects) && (i < NEXUS_NUM_PARSER_BANDS); i++)
    {
        cJSON *                          objectVideoDecoder = NULL;
        NEXUS_VideoDecoderStatus         status;
        NEXUS_VideoDecoderExtendedStatus statusExtended;
        pid_channel_info_t               pidChannelInfo;

        /* get video decoder data from Nexus */
        nxrc = NEXUS_VideoDecoder_GetStatus(objects[i].object, &status);
        CHECK_NEXUS_ERROR_GOTO("Failture retrieving video decoder status", rc, nxrc, error);

        nxrc = NEXUS_VideoDecoder_GetExtendedStatus(objects[i].object, &statusExtended);
        CHECK_NEXUS_ERROR_GOTO("Failture retrieving video decoder extended status", rc, nxrc, error);

        if (statusExtended.pidChannelIndex != -1)
        {
            NEXUS_PidChannelHandle hPidChannelHandle = bmon_get_pid_channel_handle(statusExtended.pidChannelIndex);
            bmon_process_pid_channel(0, hPidChannelHandle, &pidChannelInfo);
        }

        /* generate JSON for video decoder status */

        /* do not use filter for video decoder - we always want this included for each output */
        objectVideoDecoder = json_AddObject(objectData, NO_FILTER, objectData, "decoder");
        CHECK_PTR_ERROR_GOTO("Failure adding video decoder JSON object", objectVideoDecoder, rc, -1, error);

        /* do not use filter for index - we always want this included */
        json_AddNumber(objectVideoDecoder, NO_FILTER, objectData, "index", i);

        json_AddNumber(objectVideoDecoder, filter, objectData, "numDecodeErrors", status.numDecodeErrors);
        json_AddNumber(objectVideoDecoder, filter, objectData, "numDisplayUnderflows", status.numDisplayUnderflows);
        json_AddNumber(objectVideoDecoder, filter, objectData, "pidChannelIndex", pidChannelInfo.pidChannelIndex);
        json_AddNumber(objectVideoDecoder, filter, objectData, "fifoDepth", status.fifoDepth);
        json_AddNumber(objectVideoDecoder, filter, objectData, "fifoSize", status.fifoSize);
        json_AddNumber(objectVideoDecoder, filter, objectData, "queueDepth", status.queueDepth);
        json_AddBool(objectVideoDecoder, filter, objectData, "started", status.started);

        {
            cJSON * sourceGeom = NULL;
            sourceGeom = json_AddObject(objectVideoDecoder, filter, objectData, "source");
            CHECK_PTR_GOTO(sourceGeom, skipSourceGeom);

            json_AddNumber(sourceGeom, filter, objectData, "width", status.source.width);
            json_AddNumber(sourceGeom, filter, objectData, "height", status.source.height);
        }
skipSourceGeom:

        {
            cJSON * codedGeom = NULL;
            codedGeom = json_AddObject(objectVideoDecoder, filter, objectData, "coded");
            CHECK_PTR_GOTO(codedGeom, skipCodedGeom);

            json_AddNumber(codedGeom, filter, objectData, "width", status.coded.width);
            json_AddNumber(codedGeom, filter, objectData, "height", status.coded.height);
        }
skipCodedGeom:

        {
            cJSON * displayGeom = NULL;
            displayGeom = json_AddObject(objectVideoDecoder, filter, objectData, "display");
            CHECK_PTR_GOTO(displayGeom, skipDisplayGeom);

            json_AddNumber(displayGeom, filter, objectData, "width", status.display.width);
            json_AddNumber(displayGeom, filter, objectData, "height", status.display.height);
        }
skipDisplayGeom:

        json_AddString(objectVideoDecoder, filter, objectData, "aspectRatio", aspectRatioToString(status.aspectRatio));
        json_AddString(objectVideoDecoder, filter, objectData, "frameRate", videoFrameRateToString(status.frameRate));
        json_AddBool(objectVideoDecoder, filter, objectData, "interlaced", status.interlaced);
        json_AddString(objectVideoDecoder, filter, objectData, "format", videoFormatToString(status.format));
        json_AddString(objectVideoDecoder, filter, objectData, "protocolLevel", videoProtocolLevelToString(status.protocolLevel));
        json_AddString(objectVideoDecoder, filter, objectData, "protocolProfile", videoProtocolProfileToString(status.protocolProfile));
        json_AddBool(objectVideoDecoder, filter, objectData, "muted", status.muted);

        {
            cJSON * timeCode = NULL;
            timeCode = json_AddObject(objectVideoDecoder, filter, objectData, "timeCode");
            CHECK_PTR_GOTO(timeCode, skipTimeCode);

            json_AddNumber(timeCode, filter, objectData, "hours", status.timeCode.hours);
            json_AddNumber(timeCode, filter, objectData, "minutes", status.timeCode.minutes);
            json_AddNumber(timeCode, filter, objectData, "seconds", status.timeCode.seconds);
            json_AddNumber(timeCode, filter, objectData, "pictures", status.timeCode.pictures);
        }
skipTimeCode:

        json_AddNumber(objectVideoDecoder, filter, objectData, "pictureTag", status.pictureTag);
        json_AddString(objectVideoDecoder, filter, objectData, "pictureCoding", pictureCodingToString(status.pictureCoding));
        json_AddBool(objectVideoDecoder, filter, objectData, "tsm", status.tsm);
        json_AddNumber(objectVideoDecoder, filter, objectData, "pts", status.pts);
        json_AddNumber(objectVideoDecoder, filter, objectData, "ptsStcDifference", status.ptsStcDifference);
        json_AddString(objectVideoDecoder, filter, objectData, "ptsType", ptsTypeToString(status.ptsType));
        json_AddBool(objectVideoDecoder, filter, objectData, "firstPtsPassed", status.firstPtsPassed);
        json_AddNumber(objectVideoDecoder, filter, objectData, "cabacBinDepth", status.cabacBinDepth);
        json_AddNumber(objectVideoDecoder, filter, objectData, "enhancementFifoDepth", status.enhancementFifoDepth);
        json_AddNumber(objectVideoDecoder, filter, objectData, "enhancementFifoSize", status.enhancementFifoSize);
        json_AddNumber(objectVideoDecoder, filter, objectData, "numDecoded", status.numDecoded);
        json_AddNumber(objectVideoDecoder, filter, objectData, "numDisplayed", status.numDisplayed);
        json_AddNumber(objectVideoDecoder, filter, objectData, "numIFramesDisplayed", status.numIFramesDisplayed);
        json_AddNumber(objectVideoDecoder, filter, objectData, "numDecodeOverflows", status.numDecodeOverflows);
        json_AddNumber(objectVideoDecoder, filter, objectData, "numDisplayErrors", status.numDisplayErrors);
        json_AddNumber(objectVideoDecoder, filter, objectData, "numDecodeDrops", status.numDecodeDrops);
        json_AddNumber(objectVideoDecoder, filter, objectData, "numPicturesReceived", status.numPicturesReceived);
        json_AddNumber(objectVideoDecoder, filter, objectData, "numDisplayDrops", status.numDisplayDrops);
        json_AddNumber(objectVideoDecoder, filter, objectData, "ptsErrorCount", status.ptsErrorCount);
        json_AddNumber(objectVideoDecoder, filter, objectData, "avdStatusBlock", status.avdStatusBlock);
        json_AddNumber(objectVideoDecoder, filter, objectData, "numWatchdogs", status.numWatchdogs);

        {
            char strBytesDecoded[24];
            snprintf(strBytesDecoded, sizeof(strBytesDecoded), "%" PRId64, status.numBytesDecoded);
            json_AddString(objectVideoDecoder, filter, objectData, "numBytesDecoded", strBytesDecoded);
        }

        json_AddNumber(objectVideoDecoder, filter, objectData, "fifoEmptyEvents", status.fifoEmptyEvents);
        json_AddNumber(objectVideoDecoder, filter, objectData, "fifoNoLongerEmptyEvents", status.fifoNoLongerEmptyEvents);
    }

#endif /* NEXUS_HAS_VIDEO_DECODER */

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
} /* video_decoder_get_data */

#if defined (BMON_PLUGIN)
#define PAYLOAD_SIZE  (10 * 1024)
/**
 *  Function: This function will coordinate collecting video decoder data and send the JSON data back
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

#if NEXUS_HAS_VIDEO_DECODER
    {
        NxClient_JoinSettings joinSettings;
        NEXUS_Error           nxrc;

        /* connect to NxServer */
        NxClient_GetDefaultJoinSettings(&joinSettings);
        joinSettings.mode = NEXUS_ClientMode_eVerified;
        nxrc              = NxClient_Join(&joinSettings);
        CHECK_NEXUS_ERROR_GOTO("Failure NxClient Join", rc, nxrc, errorNxClient);
    }
#endif /* if NEXUS_HAS_VIDEO_DECODER */

    /* if the caller provided a filter, use the caller's filter */
    if (argc > 1)
    {
        pFilter = argv[1];
    }

    memset(payload, 0, sizeof(payload));

    rc = video_decoder_get_data(pFilter, payload, PAYLOAD_SIZE);
    CHECK_ERROR_GOTO("Failure retrieving video decoder data from Nexus", rc, error);

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
