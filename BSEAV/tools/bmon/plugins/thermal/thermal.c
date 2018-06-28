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

#include "thermal.h"

/*#define BMON_TRACE_ENABLE*/
#include "bmon_defines.h"
#include "bmon_convert.h"
#include "bmon_json.h"

#define FPRINTF         NOFPRINTF /* fprintf to enable, NOFPRINTF to disable */

/**
 *  Function: This function will collect all requred thermal data and output in JSON format
 **/
int thermal_get_data(
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
    objectData = json_GenerateHeader(objectRoot, THERMAL_PLUGIN_NAME, THERMAL_PLUGIN_DESCRIPTION, NULL, THERMAL_PLUGIN_VERSION);
    CHECK_PTR_ERROR_GOTO("Unable to generate JSON header", objectData, rc, -1, error);

    {
        cJSON *                   objectThermalStatus = NULL;
        cJSON *                   objectActiveThermalConfig = NULL;
        cJSON *                   objectAvailableThermalConfigs = NULL;
        cJSON *                   objectcoolingAgents = NULL;
        cJSON *                   objectPriorityTable = NULL;
        cJSON *                   objectItem      = NULL;
        cJSON *                   objectItem1      = NULL;
        cJSON *                   pRetJSON        = NULL;
        NxClient_ThermalStatus thermalStatus;
        NxClient_ThermalConfiguration thermalConfiguration;
        NxClient_ThermalConfigurationList thermalConfigList;
        NEXUS_Error nxrc=0;
        int rc=0;
        double tempInC=0;
        const char *coolingAgentName[NxClient_CoolingAgent_eMax] = {"unknown","cpuPstate","cpuIdle","graphics3D","graphics2D","display","stopPip","stopMain","user"};
        struct timeval tv;
        double lastAppliedTime=0;
        double lastRemovedTime=0;
        NxClient_CoolingAgent agent;

        nxrc =  NxClient_GetThermalStatus(&thermalStatus);
        CHECK_NEXUS_ERROR_GOTO("Failure retrieving thermal status", rc, nxrc, error);
        nxrc = NxClient_GetThermalConfiguration(thermalStatus.activeTempThreshold,&thermalConfiguration);
        CHECK_NEXUS_ERROR_GOTO("Failure retrieving thermal status", rc, nxrc, error);

        objectThermalStatus = json_AddObject(objectData, filter, objectData, "status");
        CHECK_PTR_GOTO(objectThermalStatus, skipthermalStatus);
        tempInC = (double)thermalStatus.temperature/1000.0;
        json_AddNumber(objectThermalStatus, filter, objectData, "temperatureCelsius", tempInC);
        tempInC = (double)thermalStatus.activeTempThreshold/1000.0;
        json_AddNumber(objectThermalStatus, filter, objectData, "overTemperatureThresholdCelsius", tempInC);
        objectcoolingAgents = json_AddArray(objectThermalStatus, filter, objectData, "coolingAgents");

        for(i=0;i<NXCLIENT_MAX_THERMAL_PRIORITIES;i++)
        {
            agent = thermalConfiguration.priorityTable[i].agent;
                if(agent == NxClient_CoolingAgent_eUnknown || agent >= NxClient_CoolingAgent_eMax)
                    continue;
            NEXUS_GetWallclockFromTimestamp( &thermalStatus.priorityTable[i].lastAppliedTime,&tv);
            lastAppliedTime = (double)tv.tv_sec+(double)tv.tv_usec/1000000;

            NEXUS_GetWallclockFromTimestamp( &thermalStatus.priorityTable[i].lastRemovedTime,&tv);
            lastRemovedTime = (double)tv.tv_sec+(double)tv.tv_usec/1000000;

            if(lastAppliedTime || lastRemovedTime)
            {
                objectItem = json_AddArrayElement(objectcoolingAgents);
                json_AddBool(objectItem, filter, objectData, "inUse", thermalStatus.priorityTable[i].inUse);
                json_AddString(objectItem, filter, objectData, "name", coolingAgentName[agent]);
                json_AddNumber( objectItem, filter, objectData, "lastAppliedTime", lastAppliedTime);
                json_AddNumber( objectItem, filter, objectData, "lastRemovedTime", lastRemovedTime);
            }
        }
skipthermalStatus:
        objectActiveThermalConfig = json_AddObject(objectData, filter, objectData, "config");
        CHECK_PTR_GOTO(objectActiveThermalConfig, skipthermalConfig);
        tempInC = (double)thermalConfiguration.overTempThreshold/1000.0;
        json_AddNumber( objectActiveThermalConfig, filter, objectData, "overTemperatureThresholdCelsius", tempInC);
        json_AddNumber( objectActiveThermalConfig, filter, objectData, "hysteresis", thermalConfiguration.hysteresis);
        tempInC = (double)thermalConfiguration.overTempThreshold/1000.0;
        json_AddNumber( objectActiveThermalConfig, filter, objectData, "overTemperatureResetCelsius", tempInC);
        json_AddNumber( objectActiveThermalConfig, filter, objectData, "pollIntervalMSec", thermalConfiguration.pollInterval);
        json_AddNumber( objectActiveThermalConfig, filter, objectData, "tempDelayMSec", thermalConfiguration.tempDelay);
        json_AddNumber( objectActiveThermalConfig, filter, objectData, "thetaJC", thermalConfiguration.thetaJC);
        objectPriorityTable = json_AddArray(objectActiveThermalConfig, filter, objectData, "coolingAgents");
        for(i=0;i<NXCLIENT_MAX_THERMAL_PRIORITIES;i++)
        {
            agent = thermalConfiguration.priorityTable[i].agent;
            if(agent == NxClient_CoolingAgent_eUnknown || agent >= NxClient_CoolingAgent_eMax)
                continue;
            objectItem = json_AddArrayElement(objectPriorityTable);
            json_AddString(objectItem, filter, objectData, "name", coolingAgentName[agent]);
        }
skipthermalConfig:
        NxClient_GetThermalConfigurationList(&thermalConfigList);
        objectAvailableThermalConfigs = json_AddArray(objectData, filter, objectData, "availableConfigs");
        for(i=0;i<NXCLIENT_MAX_THERMAL_CONFIGS;i++)
        {
            if(thermalConfigList.tempThreshold[i])
            {
                int j=0;
                nxrc = NxClient_GetThermalConfiguration(thermalConfigList.tempThreshold[i],&thermalConfiguration);
                if(nxrc)
                    continue;
                /* CHECK_NEXUS_ERROR_GOTO("Failure retrieving ther configuration",rc, nxrc, error);*/
                objectItem = json_AddArrayElement(objectAvailableThermalConfigs);
                tempInC = (double)thermalConfigList.tempThreshold[i]/1000.0;
                json_AddNumber( objectItem, filter, objectData, "overTemperatureThresholdCelsius", tempInC);
                json_AddNumber( objectItem, filter, objectData, "hysteresis", thermalConfiguration.hysteresis);
                tempInC = (double)thermalConfiguration.overTempThreshold/1000.0;
                json_AddNumber( objectItem, filter, objectData, "overTemperatureResetCelsius", tempInC);
                json_AddNumber( objectItem, filter, objectData, "pollIntervalMSec", thermalConfiguration.pollInterval);
                json_AddNumber( objectItem, filter, objectData, "tempDelayMSec", thermalConfiguration.tempDelay);
                json_AddNumber( objectItem, filter, objectData, "thetaJC", thermalConfiguration.thetaJC);
                objectPriorityTable = json_AddArray(objectItem, filter, objectData, "coolingAgents");
                for(j=0;j<NXCLIENT_MAX_THERMAL_PRIORITIES;j++)
                {
                    agent = thermalConfiguration.priorityTable[j].agent;
                    if(agent == NxClient_CoolingAgent_eUnknown || agent >= NxClient_CoolingAgent_eMax)
                        continue;
                    objectItem1 = json_AddArrayElement(objectPriorityTable);
                    json_AddString(objectItem1, filter, objectData, "name", coolingAgentName[agent]);
                }
            }
        }
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
} /* thermal_get_data */

#if defined (BMON_PLUGIN)
#define PAYLOAD_SIZE  (10 * 1024)
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

    rc = thermal_get_data(pFilter, payload, PAYLOAD_SIZE);
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