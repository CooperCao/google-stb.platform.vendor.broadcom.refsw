/***************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 **************************************************************************/
#include "nxclient.h"
#include "nexus_platform_client.h"
#include "nexus_surface.h"
#include "nexus_surface_client.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "picdecoder.h"
#include <unistd.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <assert.h>
#include "screenshot.h"

/*#define BMON_TRACE_ENABLE*/
#include "bmon_defines.h"
#include "bmon_convert.h"
#include "bmon_json.h"


BDBG_MODULE(screenshot);

int screenshot_get_data(const char * filter,char * data, size_t data_size)
{
    int            rc           = 0;
    int            i            = 0;
    cJSON *     objectRoot = NULL;
    cJSON *     objectData = NULL;
    cJSON *     objectScreenshot=NULL;
    cJSON * objectItem = NULL;
    const char *filename = "screenshot.bmp";
    unsigned width = 720, height = 480;

    assert(NULL != filter);
    assert(NULL != data);
    assert(0 < data_size);

    /* initialize cJSON */
    objectRoot = json_Initialize();
    CHECK_PTR_ERROR_GOTO("Unable to allocate JSON object", objectRoot, rc, -1, error);

    /* generate JSON header */
    objectData = json_GenerateHeader(objectRoot, SCREENSHOT_PLUGIN_NAME, SCREENSHOT_PLUGIN_DESCRIPTION, NULL, SCREENSHOT_PLUGIN_VERSION);
    CHECK_PTR_ERROR_GOTO("Unable to generate JSON header", objectData, rc, -1, error);

    /* do not use filter for history - we always want this included for each output */
    objectScreenshot = json_AddArray(objectData, NO_FILTER, objectData, "screenshot");
    CHECK_PTR_ERROR_GOTO("Failure adding ir  rray JSON object", objectScreenshot, rc, -1, error);

    {
        NEXUS_SurfaceHandle surface;
        NEXUS_SurfaceCreateSettings createSettings;
        NEXUS_Error nrc=0;

        NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
        createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
        createSettings.width = width;
        createSettings.height = height;
        surface = NEXUS_Surface_Create(&createSettings);

        nrc = NxClient_Screenshot(NULL, surface);
        CHECK_NEXUS_ERROR_GOTO("Failture retrieving video decoder extended status", rc, nrc, error);

        rc = picdecoder_write_bmp(filename, surface, 24);
        CHECK_NEXUS_ERROR_GOTO("Failed to write bmp file", rc, nrc, error);
        /* BDBG_WRN(("wrote %s", filename));*/
    }

    /* create object to be an array item which contains multiple objects */
    objectItem = json_AddArrayElement(objectData);
    CHECK_PTR_GOTO(objectItem, error);
    json_AddString(objectItem, filter, objectData, "uri", filename);

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
}

#if defined (BMON_PLUGIN)
#define PAYLOAD_SIZE  (4 * 1024)
int main(int    argc,char * argv[],char * envv[])
{
    int rc=0;
    char   payload[PAYLOAD_SIZE];
    char   filterDefault[] = "/";
    char * pFilter   = filterDefault;
    NxClient_JoinSettings joinSettings;
    NEXUS_Error nrc;

    /* connect to server and nexus */
    NxClient_GetDefaultJoinSettings(&joinSettings);
    joinSettings.mode = NEXUS_ClientMode_eVerified;
    nrc = NxClient_Join(&joinSettings);
    CHECK_NEXUS_ERROR_GOTO("Failure NxClient Join", rc, nrc, errorNxClient);

    /* if the caller provided a filter, use the caller's filter */
    if (argc > 1)
    {
        pFilter = argv[1];
    }
    memset(payload, 0, sizeof(payload));
    rc=  screenshot_get_data(pFilter,payload,PAYLOAD_SIZE);
    CHECK_ERROR_GOTO("Failure retrieving screenshot data from Nexus", rc, error);
    /* send response back to user */
    printf("%s\n", payload);
    fflush(stdout);
error:
    NxClient_Uninit();
errorNxClient:
    return rc;
}
#endif
