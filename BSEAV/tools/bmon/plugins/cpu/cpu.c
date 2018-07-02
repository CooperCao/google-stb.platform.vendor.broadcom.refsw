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
 *****************************************************************************/
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

#include "cpu.h"
#include "bmon_cpu_get_frequency.h"

/*#define BMON_TRACE_ENABLE*/
#include "bmon_defines.h"
#include "bmon_convert_macros.h"
#include "bmon_json.h"
#include "bmon_uri.h"

STRING_MAP_START(dvfsGovernorNameMap)
STRING_MAP_ENTRY(DVFS_GOVERNOR_CONSERVATIVE, "conservative")
STRING_MAP_ENTRY(DVFS_GOVERNOR_ONDEMAND, "ondemand")
STRING_MAP_ENTRY(DVFS_GOVERNOR_USERSPACE, "userspace")
STRING_MAP_ENTRY(DVFS_GOVERNOR_POWERSAVE, "powersave")
STRING_MAP_ENTRY(DVFS_GOVERNOR_PERFORMANCE, "performance")
STRING_MAP_END()

ENUM_TO_STRING_FUNCTION(dvfsGovernorToString, DVFS_GOVERNOR_TYPES, dvfsGovernorNameMap)
STRING_TO_ENUM_FUNCTION(stringToDvfsGovernor, DVFS_GOVERNOR_TYPES, dvfsGovernorNameMap)

/**
 *  Function: This function will set the scaling governor value using the caller's
 *            input value. The input enum will be converted to the appropriate
 *            string value before the string is stored in the /sys file system.
 *            echo performance >/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
 **/
static int set_governor_control(
        int                 cpu,
        DVFS_GOVERNOR_TYPES GovernorSetting
        )
{
    char cmd[128];

    memset(cmd, 0, sizeof(cmd));

    sprintf(cmd, "echo %s >/sys/devices/system/cpu/cpu%d/cpufreq/scaling_governor", dvfsGovernorToString(GovernorSetting), cpu);
    fprintf(stderr, "~issuing cmd (%s) ~\n", cmd);
    system(cmd);

    return(0);
}

static int bmon_get_uptime(unsigned long long int * puptime)
{
    size_t             numBytes     = 0;
    FILE *             fpProcUptime = NULL;
    char               bufProcStat[256];
    float              f_uptime      = 0.0;
    float              f_uptime_msec = 0.0;
    unsigned long long uptime_msec   = 0;

    if (puptime == NULL) { return(0); }

    fpProcUptime = fopen("/proc/uptime", "r");
    if (fpProcUptime == NULL)
    {
        fprintf(stderr, "could not open /proc/uptime\n");
        return(-1);
    }

    numBytes = fread(bufProcStat, 1, sizeof(bufProcStat), fpProcUptime);
    fclose(fpProcUptime);

    if (numBytes)
    {
        sscanf(bufProcStat, "%f.2", &f_uptime);
        f_uptime_msec = f_uptime * 1000.0;
        uptime_msec   = f_uptime_msec;
        *puptime      = uptime_msec;
    }
    return(0);
} /* bmon_get_uptime */

/**
 *  This function will gather the CPU usage data for each active CPU in the system. The usages
 *  include user, system, idle, irqs, etc. It uses two files: (1) /proc/uptime and (2) /proc/stat.
 **/
static int bmon_cpu_get_proc_data(bmon_cpu_t * pcpu_data)
{
    FILE *        fpProcStat = NULL;
    char          bufProcStat[1024];
    int           numCpusConf = 0;
    unsigned char cpu         = 0;
    char *        pos         = NULL;
    char          cpuTag[6];
    bmon_cpu_t    cpu_data_prev;

    memset(&bufProcStat, 0, sizeof(bufProcStat));
    memset(&cpu_data_prev, 0, sizeof(cpu_data_prev));

    numCpusConf = sysconf(_SC_NPROCESSORS_CONF);
    if (numCpusConf > BMON_CPU_MAX_NUM)
    {
        numCpusConf = BMON_CPU_MAX_NUM;
    }

    /*FPRINTF( stderr, "%s - numCpusConf %d \n", __FUNCTION__, numCpusConf );*/

    fpProcStat = fopen("/proc/stat", "r");
    if (fpProcStat == NULL)
    {
        fprintf(stderr, "could not open /proc/stat\n");
    }
    else
    {
        bmon_get_uptime(&pcpu_data->uptime_msec);

        fread(bufProcStat, 1, sizeof(bufProcStat), fpProcStat);

        fclose(fpProcStat);

        pos = bufProcStat;

        pcpu_data->num_active_cpus = sysconf(_SC_NPROCESSORS_ONLN); /* will be lower than SC_NPROCESSORS_CONF if some CPUs are disabled */

        for (cpu = 0; cpu < numCpusConf; cpu++)
        {
            sprintf(cpuTag, "cpu%u ", cpu);
            pos = strstr(pos, cpuTag);
            if (pos)
            {
                pcpu_data->cpu[cpu].active = 1;
                pos                       += strlen(cpuTag);
                /*
                 *  This is the sscanf that mpstat.c uses:
                 *  &cp->cpu_user, &cp->cpu_nice, &cp->cpu_system, &cp->cpu_idle, &cp->cpu_iowait, &cp->cpu_irq, &cp->cpu_softirq, &cp->cpu_steal, &cp->cpu_guest
                 */
                sscanf(pos, "%u %u %u %u %u %u %u %u ",
                        &pcpu_data->cpu[cpu].user, &pcpu_data->cpu[cpu].nice, &pcpu_data->cpu[cpu].system, &pcpu_data->cpu[cpu].idle,
                        &pcpu_data->cpu[cpu].iowait, &pcpu_data->cpu[cpu].irq, &pcpu_data->cpu[cpu].softirq, &pcpu_data->cpu[cpu].steal);
            }
            else
            {
                pcpu_data->cpu[cpu].active = 0;
                pos                        = bufProcStat;                         /* pos is NULL at this point; reset it to start searching back at the beginning of the stat buffer */
            }
        }
    }

    return(0);
} /* bmon_cpu_get_proc_data */

/**
 *  Function: This function will collect all requred cpu data and store the data into a
 *            known structure.
 **/
int cpu_get_data(
        const char * filter,
        char *       data,
        unsigned int data_size
        )
{
    cJSON * objectRoot = NULL;
    cJSON * objectData = NULL;
    cJSON * objectCpus = NULL;
    int     rc         = 0;

    int        num_bytes            = 0;
    char *     buf                  = data;
    int        buf_size             = data_size;
    int        i                    = 0;
    int        active_count         = 0;
    int        numCpusConf          = 0;
    char *     pos                  = NULL;
    bool       b_appended_something = false;
    char       one_line[128];
    char       error_msg[32];
    char       strGovernor[8];
    char       strCpu[8];
    bmon_cpu_t cpu_data;

    assert(NULL != filter);
    assert(NULL != data);
    assert(0 < data_size);

    memset(&error_msg, 0, sizeof(error_msg));
    memset(&cpu_data, 0, sizeof(cpu_data));

    /* initialize cJSON */
    objectRoot = json_Initialize();
    CHECK_PTR_ERROR_GOTO("Unable to allocate JSON object", objectRoot, rc, -1, error);

    /* generate JSON header */
    objectData = json_GenerateHeader(objectRoot, CPU_PLUGIN_NAME, CPU_PLUGIN_DESCRIPTION, NULL, CPU_PLUGIN_VERSION);
    CHECK_PTR_ERROR_GOTO("Unable to generate JSON header", objectData, rc, -1, error);

    cpu_data.total_cpus = bmon_cpu_get_frequencies(&(cpu_data.frequency_mhz[0]));
    bmon_cpu_get_proc_data(&cpu_data);
    /* fprintf(stderr, "%s - filter (%s) \n", __FUNCTION__, filter ); if user is attempting to change one of the settings */
    if ( bmon_uri_find_tagvalue(filter, "cpu", strCpu, sizeof(strCpu)) )
    {
        unsigned int cpu         = 0;
        int          numCpusConf = 0;

        numCpusConf = sysconf(_SC_NPROCESSORS_CONF);

        sscanf(strCpu, "%u", &cpu);

        do
        {
            FILE * fp = NULL;
            char   filename[32];
            char   strActive[8];
            int    bytes  = 0;
            int    active = 0;

            if ((cpu == 0) || (cpu >= numCpusConf))
            {
                sprintf(error_msg, "CPU (%u) is invalid", cpu);
                break;
            }

            if (0 == bmon_uri_find_tagvalue(filter, "active", strActive, sizeof(strActive)))
            {
                break;
            }

            sscanf(strActive, "%u", &active);

            /* make sure user entered a 0 or a 1 */
            if ((active != 0) && (active != 1))
            {
                sprintf(error_msg, "active state (%s) is invalid", pos);
                break;
            }
            sprintf(filename, "/sys/devices/system/cpu/cpu%u/online", cpu);
            fp = fopen(filename, "w");

            if (fp == NULL)
            {
                sprintf(error_msg, "Could not open (%s)", filename);
                break;
            }
            /*fprintf(stderr, "%s - fwrite  (%s) to file (%s)  \n", __FUNCTION__, strActive, filename );*/
            bytes = fwrite( strActive, 1, 1, fp);
            fclose(fp);
            if (bytes != 1)
            {
                sprintf(error_msg, "write (%s) to (%s) Failed", strActive, filename);
                break;
            }
        }
        while (0);
    }
    else /* if user is attempting to change one of the settings */
    if (bmon_uri_find_tagvalue(filter, "governor", strGovernor, sizeof(strGovernor)))
    {
        unsigned int which = 0;
        sscanf(strGovernor, "%u", &which);
        /* fprintf(stderr, "%s - governor (%u) \n", __FUNCTION__, which ); */
        if (which && (which < DVFS_GOVERNOR_MAX))
        {
            set_governor_control(0, which);
        }
    }

    /* add plug in data */

    numCpusConf = sysconf(_SC_NPROCESSORS_CONF);
    /*fprintf( stderr, "%s - numCpusConf %d \n", __FUNCTION__, numCpusConf );*/

    {
        cJSON * objectItem = NULL;

        /* create object to be an array item which contains multiple objects */
        objectItem = json_AddArrayElement(objectData);
        CHECK_PTR_GOTO(objectItem, skipCpus);

        json_AddNumber(objectItem, filter, objectData, "total_cpus", cpu_data.total_cpus);
        json_AddNumber(objectItem, filter, objectData, "num_active_cpus", cpu_data.num_active_cpus);
        json_AddNumber(objectItem, filter, objectData, "uptime_msec", cpu_data.uptime_msec);
    }

    objectCpus = json_AddArray(objectData, filter, objectData, "cpus");
    CHECK_PTR_GOTO(objectCpus, skipCpus);

    for (i = 0; i < numCpusConf; i++)
    {
        cJSON * objectItem = NULL;

        /* create object to be an array item which contains multiple objects */
        objectItem = json_AddArrayElement(objectCpus);
        CHECK_PTR_GOTO(objectItem, skipCpus);

        json_AddBool(objectItem, filter, objectData, "active", cpu_data.cpu[i].active);
        json_AddNumber(objectItem, filter, objectData, "idle", (double)cpu_data.cpu[i].idle);
        json_AddNumber(objectItem, filter, objectData, "user", (double)cpu_data.cpu[i].user);
        json_AddNumber(objectItem, filter, objectData, "system", (double)cpu_data.cpu[i].system);
        json_AddNumber(objectItem, filter, objectData, "nice", (double)cpu_data.cpu[i].nice);
        json_AddNumber(objectItem, filter, objectData, "iowait", (double)cpu_data.cpu[i].iowait);
        json_AddNumber(objectItem, filter, objectData, "irq", (double)cpu_data.cpu[i].irq);
        json_AddNumber(objectItem, filter, objectData, "softirq", (double)cpu_data.cpu[i].softirq);
        json_AddNumber(objectItem, filter, objectData, "steal", (double)cpu_data.cpu[i].steal);
        json_AddNumber(objectItem, filter, objectData, "frequency_mhz", (double)cpu_data.frequency_mhz[i]);
    }
skipCpus:

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
} /* cpu_get_data */

#if defined (BMON_PLUGIN)
#define PAYLOAD_SIZE  (2 * 1024)
/**
 *  Function: This function will coordinate collecting cpu data and once that is done,
 *            it will convert the cpu data to a JSON format and send the JSON data back
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

    memset(payload, 0, sizeof(payload));

    rc = cpu_get_data(pFilter, payload, PAYLOAD_SIZE);
    CHECK_ERROR_GOTO("Failure retrieving CPU data", rc, error);

    /* send response back to user */
    printf("%s\n", payload);
    fflush(stdout);

error:
    return(rc);
} /* main */

#endif /* defined(BMON_PLUGIN) */
