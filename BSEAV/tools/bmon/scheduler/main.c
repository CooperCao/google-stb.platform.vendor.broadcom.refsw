/******************************************************************************
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
 *****************************************************************************/

#include <assert.h>
#include <string.h>
#include "b_os_lib.h"
/*#define BMON_TRACE_ENABLE*/
#include "bmon_defines.h"
#include "scheduler.h"

#define FPRINTF           fprintf /* fprintf to enable, NOFPRINTF to disable */

#define MAX_PAYLOAD_SIZE  (20 * 1024)

static char testData[MAX_PAYLOAD_SIZE];

static void testDataReceivedCallback(
        bmon_scheduler_configHandle hConfig,
        char *                      payloadBuffer,
        size_t                      payloadSize,
        void *                      pParam,
        int                         nData
        )
{
    BMON_TRACE("entry");
    assert(NULL != payloadBuffer);

    UNUSED(pParam);
    UNUSED(nData);

    if (0 == payloadSize)
    {
        FPRINTF(stderr, "Error: EMPTY data buffer received!\n");
        return;
    }

    if (1024 < strlen(payloadBuffer))
    {
        payloadBuffer[1024] = '\0';
        FPRINTF(stderr, "==> hConfig:%p Data Buffer Received:\n%s[truncated to 1024 bytes]\n\n", (void *)hConfig, payloadBuffer);
    }
    else
    {
        FPRINTF(stderr, "==> hConfig:%p Data Buffer Received:\n%s\n\n", (void *)hConfig, payloadBuffer);
    }
    BMON_TRACE("exit");
} /* testDataReceivedCallback */

int main(
        int     argc,
        char ** argv
        )
{
    int rc = 0;

    UNUSED(argc);
    UNUSED(argv);

    bmon_scheduler_settings_t   settingsTest;
    bmon_scheduler_configHandle hConfig1   = NULL;
    bmon_scheduler_configHandle hConfig2   = NULL;
    cJSON *                     objectRoot = NULL;
    bool bCollectFirst                     = true; /* collect data immediately upon adding config */

    BMON_TRACE("entry");

    B_Os_Memset(testData, 0, sizeof(testData));

    bmon_scheduler_getDefaultSettings(&settingsTest);
    settingsTest.payloadSize   = sizeof(testData);
    settingsTest.payloadBuffer = testData;
    settingsTest.callbackFunc  = testDataReceivedCallback;

    rc = bmon_scheduler_open(&settingsTest);
    CHECK_ERROR_GOTO("unable to open scheduler", rc, error);

    /* there are 2 ways to programmatically create a JSON config:
     *     1. generate JSON as a string and submit it to the scheduler.
     *        this technique requires a text buffer and would most likely
     *        be used when reading a config from a file.
     *     2. generate JSON as a cJSON object and submit it to the scheduler.
     *        this technique does NOT require a text buffer and is simpler
     *        when generating a config programmatically.
     */

    /* option1: generate/add config1 JSON using string buffer */
    {
        char strConfigJson[2048];

        objectRoot = bmon_scheduler_configCreate("config1", "config description 1");
        CHECK_PTR_ERROR_GOTO("unable to create config", objectRoot, rc, -1, error);

        /* this is the data we want to periodically retrieve */
        rc = bmon_scheduler_configAddUri(objectRoot, "http://10.10.0.1:8080/hdmi_output/output/txHardware", 1, 5, bCollectFirst);
        CHECK_ERROR_GOTO("unable to add uri to config", rc, error);
        rc = bmon_scheduler_configAddUri(objectRoot, "hdmi_output/output/basicEdid", 2, 20, bCollectFirst);
        CHECK_ERROR_GOTO("unable to add uri to config", rc, error);
        rc = bmon_scheduler_configAddUri(objectRoot, "hdmi_output/output?include=basicEdid+txHardware", 4, 40, bCollectFirst);
        CHECK_ERROR_GOTO("unable to add uri to config", rc, error);
        rc = bmon_scheduler_configAddUri(objectRoot, "hdmi_output", 5, 0, bCollectFirst);
        CHECK_ERROR_GOTO("unable to add uri to config", rc, error);

        /* generate JSON text into strConfigJson buffer */
        rc = bmon_scheduler_configGenerate(objectRoot, strConfigJson, sizeof(strConfigJson));
        CHECK_ERROR_GOTO("unable to generate config JSON string", rc, error);

        /* after generating config JSON string, we do not need objectRoot anymore */
        bmon_scheduler_configDestroy(objectRoot);
        objectRoot = NULL;

        /* start data collection using JSON string */
        hConfig1 = bmon_scheduler_addString(strConfigJson);
        CHECK_ERROR_GOTO("unable to add configuration", rc, error);

        FPRINTF(stderr, "==>\n==> Starting hConfig1:%p\n==>\n", (void *)hConfig1);
    }

    /* option2: generate/add config2 JSON directly */
    {
        objectRoot = bmon_scheduler_configCreate("config2", "config description 2");
        CHECK_PTR_ERROR_GOTO("unable to create config", objectRoot, rc, -1, error);

        /* this is the data we want to periodically retrieve */
        rc = bmon_scheduler_configAddUri(objectRoot, "hdmi_output/output/rxEdid?exclude=videoFormatSupported", 6, 40, bCollectFirst);
        CHECK_ERROR_GOTO("unable to add uri to config", rc, error);
        rc = bmon_scheduler_configAddUri(objectRoot, "video_decoder", 7, 0, bCollectFirst);
        CHECK_ERROR_GOTO("unable to add uri to config", rc, error);

        /* start data collection using cJSON object */
        hConfig2 = bmon_scheduler_add(objectRoot);
        CHECK_ERROR_GOTO("unable to add configuration", rc, error);

        FPRINTF(stderr, "==>\n==> Starting hConfig2:%p\n==>\n", (void *)hConfig2);

        /* after starting data collection we no longer need the cJSON object */
        bmon_scheduler_configDestroy(objectRoot);
        objectRoot = NULL;
    }

    /* sleep while collecting hConfig1 and hConfig2 data */
    B_Thread_Sleep(11000);

    /* stop data collection for hConfig1 only */
    FPRINTF(stderr, "==>\n==> Stopping hConfig1(%p) - hConfig2(%p) will continue working\n==>\n", (void *)hConfig1, (void *)hConfig2);
    bmon_scheduler_remove(hConfig1);
    hConfig1 = NULL;

    /* sleep while collecting hConfig2 data */
    B_Thread_Sleep(15000);

    FPRINTF(stderr, "==>\n==> Stopping hConfig2(%p)\n==>\n", (void *)hConfig2);
error:
    FPRINTF(stderr, "Cleaning up...\n");
    if (NULL != hConfig2)
    {
        /* stop data collection */
        bmon_scheduler_remove(hConfig2);
        hConfig2 = NULL;
    }
    if (NULL != hConfig1)
    {
        /* stop data collection */
        bmon_scheduler_remove(hConfig1);
        hConfig1 = NULL;
    }

    if (NULL != objectRoot)
    {
        bmon_scheduler_configDestroy(objectRoot);
        objectRoot = NULL;
    }

    bmon_scheduler_close();
    FPRINTF(stderr, "Done.\n");
    BMON_TRACE("exit");
    return(rc);
} /* main */