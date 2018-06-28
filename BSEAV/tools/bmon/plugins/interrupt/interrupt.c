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

/*#define BMON_TRACE_ENABLE*/
#include "interrupt.h"
#include "bmon_defines.h"
#include "bmon_json.h"
#include "bmon_interrupt.h"
#include "bmon_get_uptime.h"

#define FPRINTF  NOFPRINTF /* fprintf to enable, NOFPRINTF to disable */

/**
 *  Function: This function will
 *     1. parse the filter
 *     2. Collect required data
 *     3. Convert to json format
 *     4. Return number of bytes written to the buffer
 **/
int interrupt_get_data(
        const char * filter,
        char *       json_output,
        unsigned int json_output_size
        )
{
    int              rc         = 0;
    cJSON *          objectRoot = NULL;
    cJSON *          objectData = NULL;
    cJSON *          objectItem = NULL;
    char *           contents   = NULL;
    bmon_interrupt_t interrupt_data;

    assert(NULL != filter);
    assert(NULL != json_output);
    assert(0 < json_output_size);

    memset(&interrupt_data, 0, sizeof(interrupt_data));

    /* initialize cJSON */
    objectRoot = json_Initialize();
    CHECK_PTR_ERROR_GOTO("Unable to allocate JSON object", objectRoot, rc, -1, error);

    /* generate JSON header */
    objectData = json_GenerateHeader(objectRoot, INTERRUPT_PLUGIN_NAME, INTERRUPT_PLUGIN_DESCRIPTION, NULL, INTERRUPT_PLUGIN_VERSION);
    CHECK_PTR_ERROR_GOTO("Unable to generate JSON header", objectData, rc, -1, error);

    /* collect plugin data */
    {
        objectItem = json_AddArrayElement(objectData);
        CHECK_PTR_GOTO(objectItem, skipCpuStats);

        rc = bmon_interrupt_get_counts(&interrupt_data);
        CHECK_ERROR_GOTO("bmon_interrupt_get_counts() failed", rc, skipCpuStats);

        json_AddNumber(objectItem, filter, objectData, "interruptsTotal", interrupt_data.interrupt_total);

        rc = bmon_get_uptime(&interrupt_data.uptime_msec);
        CHECK_ERROR_GOTO("bmon_get_uptime() failed", rc, skipCpuStats);

        json_AddNumber(objectItem, filter, objectData, "uptimeMsec", interrupt_data.uptime_msec);

        rc = bmon_get_proc_stat_info(&interrupt_data);
        CHECK_ERROR_GOTO("bmon_get_proc_stat_info() failed", rc, skipCpuStats);

        json_AddNumber(objectItem, filter, objectData, "contextSwitches", interrupt_data.context_switches);
    }
skipCpuStats:

    {
        cJSON *      objectInterrupts = NULL;
        cJSON *      objectCpuCounts  = NULL;
        int          i                = 0;
        int          cpu              = 0;
        unsigned int numCpusConf      = sysconf(_SC_NPROCESSORS_CONF);

        objectInterrupts = json_AddArray(objectData, NO_FILTER, objectData, "interrupts");
        CHECK_PTR_ERROR_GOTO("Failure adding interrupts array JSON object", objectInterrupts, rc, -1, error);

        for (i = 0; i < BMON_INTERRUPTS_MAX_TYPES; i++)
        {
            if (interrupt_data.interrupt[i].name[0] == '\0')
            {
                /* we have reached the end of known interrupts */
                break;
            }

            objectItem = json_AddArrayElement(objectInterrupts);
            CHECK_PTR_GOTO(objectItem, error);

            json_AddString(objectItem, filter, objectData, "name", interrupt_data.interrupt[i].name);

            objectCpuCounts = json_AddArray(objectItem, NO_FILTER, objectData, "cpuCounts");
            CHECK_PTR_ERROR_GOTO("Failure adding cpuCounts array JSON object", objectCpuCounts, rc, -1, error);

            for (cpu = 0; cpu < numCpusConf; cpu++)
            {
                if (true == interrupt_data.cpu[cpu].active)
                {
                    json_AddNumber(objectCpuCounts, filter, objectData, NULL, interrupt_data.interrupt[i].cpu_count[cpu]);
                }
            }
        }
    }

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
} /* interrupt_get_data */

#if defined (BMON_PLUGIN)
#define PAYLOAD_SIZE  (10 * 1024)
/**
 *  Function: This function will coordinate collecting interrupt data and once that is done,
 *            it will convert the interrupt data to a JSON format and send the JSON data back
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

    /* if the caller provided a filter, use the caller's filter */
    if (argc > 1)
    {
        pFilter = argv[1];
    }

    rc = interrupt_get_data(pFilter, payload, PAYLOAD_SIZE);
    CHECK_ERROR_GOTO("Failure retrieving interrupt data", rc, error);

    /* send response back to user */
    printf("%s\n", payload);
    fflush(stdout);

error:
    return(rc);
} /* main */

#endif /* defined(BMON_PLUGIN) */