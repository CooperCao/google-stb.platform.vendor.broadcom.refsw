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
#include "b_os_lib.h"
/*#define BMON_TRACE_ENABLE*/
#include "bmon_defines.h"
#include "scheduler.h"
#define FPRINTF fprintf
/* fprintf to enable, NOFPRINTF to disable */

#define MAX_PAYLOAD_SIZE (20 * 1024)
static char testData[MAX_PAYLOAD_SIZE];
static void testDataReceivedCallback(bmon_scheduler_configHandle hConfig, char *payloadBuffer, size_t payloadSize, void *pParam, int nData)
{
    BMON_TRACE("entry");
    assert(NULL != payloadBuffer);
    UNUSED(pParam);
    UNUSED(nData);
    UNUSED(hConfig);
    if (0 == payloadSize)
    {
        FPRINTF(stderr, "Error: EMPTY data buffer received!\n");
        return;
    }
    payloadBuffer[payloadSize] = '\0';
    FPRINTF(stdout, "%s\n", payloadBuffer);
    BMON_TRACE("exit");
}

/* testDataReceivedCallback */
int main(int argc, char **argv)
{
    int rc = 0;
    int n = 0;
    char strConfigJson[2048];
    UNUSED(argc);
    UNUSED(argv);
    bmon_scheduler_settings_t settingsTest;
    bmon_scheduler_configHandle hConfig1 = NULL;

    BMON_TRACE("entry");
    B_Os_Memset(testData, 0, sizeof(testData));
    bmon_scheduler_getDefaultSettings(&settingsTest);
    settingsTest.payloadSize = sizeof(testData);
    settingsTest.payloadBuffer = testData;
    settingsTest.callbackFunc = testDataReceivedCallback;
    rc = bmon_scheduler_open(&settingsTest);
    CHECK_ERROR_GOTO("unable to open scheduler", rc, error);
    while (1)
    {
        n = scanf("%s", strConfigJson);
        if (n == EOF)
        {
            FPRINTF(stderr, "scanf return error\n");
            break;
        }
        FPRINTF(stderr, "new config received:%s\n", strConfigJson);
        if(NULL != hConfig1)
        {
            /* stop data collection */
            bmon_scheduler_remove(hConfig1);
            hConfig1 = NULL;
        }
        /* start data collection using JSON string */
        hConfig1 = bmon_scheduler_addString(strConfigJson);
        if (hConfig1 == NULL)
        {
            break;
        }
    }
error:
    if (NULL != hConfig1)
    {
        /* stop data collection */
        bmon_scheduler_remove(hConfig1);
        hConfig1 = NULL;
    }
    bmon_scheduler_close();
    FPRINTF(stderr, "Done.\n");
    BMON_TRACE("exit");
    return (rc);
}
/* main */
