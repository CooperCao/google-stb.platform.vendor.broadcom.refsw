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

#include "nexus_ir_input_bmon.h"

#include "ir_input.h"

/*#define BMON_TRACE_ENABLE*/
#include "bmon_defines.h"
#include "bmon_convert.h"
#include "bmon_json.h"

#define FPRINTF         NOFPRINTF /* fprintf to enable, NOFPRINTF to disable */

#define MAX_IR_HISTORY  16

ssize_t formatTimeval(
        struct timeval * tv,
        char *           buf,
        size_t           sz
        )
{
    ssize_t     written = -1;
    struct tm * gm      = NULL;

    assert(NULL != tv);
    assert(NULL != buf);
    assert(0 < sz);

    gm = gmtime(&tv->tv_sec);

    if (gm)
    {
        written = (ssize_t)strftime(buf, sz, "%Y-%m-%dT%H:%M:%S", gm);
        if ((written > 0) && ((size_t)written < sz))
        {
            int w = snprintf(buf+written, sz-(size_t)written, ".%06dZ", tv->tv_usec);
            written = (w > 0) ? written + w : -1;
        }
    }
    return(written);
} /* formatTimeval */

/**
 *  Function: This function will collect all requred ir_input data and output in JSON format
 **/
int ir_input_get_data(
        const char * filter,
        char *       data,
        size_t       data_size
        )
{
    int         rc         = 0;
    cJSON *     objectRoot = NULL;
    cJSON *     objectData = NULL;
    int         i          = 0;
    NEXUS_Error nxrc;

    assert(NULL != filter);
    assert(NULL != data);
    assert(0 < data_size);

    /* initialize cJSON */
    objectRoot = json_Initialize();
    CHECK_PTR_ERROR_GOTO("Unable to allocate JSON object", objectRoot, rc, -1, error);

    /* generate JSON header */
    objectData = json_GenerateHeader(objectRoot, IR_INPUT_PLUGIN_NAME, IR_INPUT_PLUGIN_DESCRIPTION, NULL, IR_INPUT_PLUGIN_VERSION);
    CHECK_PTR_ERROR_GOTO("Unable to generate JSON header", objectData, rc, -1, error);

    {
        cJSON *                   objectIrHistory = NULL;
        cJSON *                   objectItem      = NULL;
        cJSON *                   pRetJSON        = NULL;
        NEXUS_IrInputEventHistory irHistory[MAX_IR_HISTORY];
        unsigned                  numIrHistory = 0;
        double dTime                              = 0.0;

        /* get IR history data from Nexus */
        NEXUS_IrInputModule_GetEventHistory(irHistory, MAX_IR_HISTORY, &numIrHistory);

        /* generate JSON for IR history */

        /* do not use filter for history - we always want this included for each output */
        objectIrHistory = json_AddArray(objectData, NO_FILTER, objectData, "history");
        CHECK_PTR_ERROR_GOTO("Failure adding ir history array JSON object", objectIrHistory, rc, -1, error);

        for (i = 0; i < numIrHistory; i++)
        {
            objectItem = json_AddArrayElement(objectIrHistory);
            CHECK_PTR_GOTO(objectItem, skipIrHistory);

            json_AddBool(objectItem, filter, objectData, "repeat", irHistory[i].event.repeat);
            json_AddNumber(objectItem, filter, objectData, "code", irHistory[i].event.code);
            json_AddNumber(objectItem, filter, objectData, "codeHigh", irHistory[i].event.codeHigh);
            json_AddNumber(objectItem, filter, objectData, "interval", irHistory[i].event.interval);
            json_AddBool(objectItem, filter, objectData, "preambleA", irHistory[i].event.preamble.preambleA);
            json_AddBool(objectItem, filter, objectData, "preambleB", irHistory[i].event.preamble.preambleB);
            json_AddString(objectItem, filter, objectData, "mode", irInputModeToString(irHistory[i].mode));
            {
                struct timeval tv;
                char           bufTime[32];

                NEXUS_GetWallclockFromTimestamp(&(irHistory[i].timestamp), &tv);
                dTime = (double)tv.tv_sec+(double)tv.tv_usec/1000000;
                json_AddNumber( objectItem, filter, objectData, "timestampSec", dTime);
#if 0
                if (0 < formatTimeval(&tv, bufTime, sizeof(bufTime)))
                {
                    json_AddString(objectItem, filter, objectData, "nxTimestamp", bufTime);
                }
#endif
            }
        }
    }
skipIrHistory:

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
} /* ir_input_get_data */

#if defined (BMON_PLUGIN)
#define PAYLOAD_SIZE  (MAX_IR_HISTORY * 384)
/**
 *  Function: This function will coordinate collecting ir input data and send the JSON data back
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

    {
        NxClient_JoinSettings joinSettings;
        NEXUS_Error           nxrc;

        /* connect to NxServer */
        NxClient_GetDefaultJoinSettings(&joinSettings);
        joinSettings.mode = NEXUS_ClientMode_eVerified;
        nxrc              = NxClient_Join(&joinSettings);
        CHECK_NEXUS_ERROR_GOTO("Failure NxClient Join", rc, nxrc, errorNxClient);
    }

    /* if the caller provided a filter, use the caller's filter */
    if (argc > 1)
    {
        pFilter = argv[1];
    }

    memset(payload, 0, sizeof(payload));

    rc = ir_input_get_data(pFilter, payload, PAYLOAD_SIZE);
    CHECK_ERROR_GOTO("Failure retrieving ir input data from Nexus", rc, error);

    /* send response back to user */
    printf("%s\n", payload);
    fflush(stdout);

error:
    NxClient_Uninit();
errorNxClient:

    return(rc);
} /* main */

#endif /* defined(BMON_PLUGIN) */