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
#include "nxclient_global.h"
#else
#include "nexus_platform.h"
#endif /* if NXCLIENT_SUPPORT */

#include "pmu.h"

/*#define BMON_TRACE_ENABLE*/
#include "bmon_defines.h"
#include "bmon_convert.h"
#include "bmon_json.h"

#define FPRINTF         NOFPRINTF /* fprintf to enable, NOFPRINTF to disable */
#define PMU_SYSFS "/sys/class/i2c-adapter/i2c-0/0-0040/iio:device0"
#define POWER_RAW "in_power2_raw"

int sysfs_get(const char *path, const char *filename, unsigned *val)
{
    FILE *fd;
    int ret = -1;
    char filepath[256];

    snprintf(filepath, 256, "%s/%s", path, filename);

    fd = fopen(filepath, "r");
    if (!fd) {
        return ret;
    }
    ret = fscanf(fd, "%u", val);
    fclose(fd);
    return 0;
}

/**
 *  Function: This function will collect all requred pmu data and output in JSON format
 **/
int pmu_get_data(
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
    objectData = json_GenerateHeader(objectRoot, PMU_PLUGIN_NAME, PMU_PLUGIN_DESCRIPTION, NULL, PMU_PLUGIN_VERSION);
    CHECK_PTR_ERROR_GOTO("Unable to generate JSON header", objectData, rc, -1, error);

    {
        cJSON *                   objectPmuStatus = NULL;
        int rc=0;
        double powerW=0;
        unsigned power;
        if (sysfs_get(PMU_SYSFS, POWER_RAW, &power))
        {
            json_AddNull(objectRoot, NO_FILTER, objectRoot, "data");
            json_GenerateError(objectRoot,BMON_CALLER_ERR_RES_NOT_FOUND,"PMU not supported by this platfrom");
            goto error;
        }
        powerW = (double)(2*power)/1000;
        json_AddNumber(objectPmuStatus, filter, objectData, "systemPowerWatt", powerW);
        objectPmuStatus = json_AddObject(objectData, filter, objectData, "power");
        CHECK_PTR_GOTO(objectPmuStatus, error);
        json_AddNull(objectPmuStatus, filter, objectData, "socPowerWatt");
        json_AddNull(objectPmuStatus, filter, objectData, "cpuPowerWatt");
    }
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
} /* pmu_get_data */

#if defined (BMON_PLUGIN)
#define PAYLOAD_SIZE  (1 * 1024)
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

    memset(payload, 0, sizeof(payload));

    rc = pmu_get_data(pFilter, payload, PAYLOAD_SIZE);
    CHECK_ERROR_GOTO("Failure retrieving pmu data", rc, error);

    /* send response back to user */
    printf("%s\n", payload);
    fflush(stdout);

error:

    return(rc);
} /* main */
#endif /* defined(BMON_PLUGIN) */
