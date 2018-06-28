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
 *
 * Module Description:
 *
 *****************************************************************************/

#include "thermal_config.h"
#include "namevalue.h"

#include "bstd.h"
#include "bkni.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

BDBG_MODULE(thermal_config);

const namevalue_t agents[] = {
    {"cpu_pstate", NxClient_CoolingAgent_eCpuPstate},
    {"cpu_idle", NxClient_CoolingAgent_eCpuIdle},
    {"v3d", NxClient_CoolingAgent_eGraphics3D},
    {"m2mc", NxClient_CoolingAgent_eGraphics2D},
    {"display", NxClient_CoolingAgent_eDisplay},
    {"stop_pip", NxClient_CoolingAgent_eStopPip},
    {"stop_main", NxClient_CoolingAgent_eStopMain},
    {"user", NxClient_CoolingAgent_eUser},
    {NULL, 0}
};

char *trimwhitespace(char *str)
{
  char *end;
  /*Trim leading space*/
  while(isspace((unsigned char)*str)) str++;
  if(*str == 0)
    return str;
  /*Trim trailing space*/
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;
  *(end+1) = 0;
  return str;
}

NEXUS_Error parse_thermal_config_file(const char *filename, NxClient_ThermalConfiguration *config)
{
    NEXUS_Error rc=NEXUS_SUCCESS;
    FILE *fp;
    unsigned i=0;
    unsigned max_priorities = sizeof(config->priorityTable)/sizeof(config->priorityTable[0]);

    if(!config) {
        return NEXUS_INVALID_PARAMETER;
    }

    BKNI_Memset(config, 0, sizeof(*config));
    /* Initialize defaults */
    config->overTempThreshold = 90000;
    config->overTempReset = 110000;
    config->hysteresis = 2500;
    config->overPowerThreshold = 5000;
    config->pollInterval = 1000;
    config->tempDelay = 2000;
    config->thetaJC = 5500;

    fp = fopen(filename, "r");
    if(fp) {
        BDBG_MSG(("Reading thermal configuration from %s", filename));

        while (!feof(fp)) {
            char buf[256];
            char *s;

            BKNI_Memset(buf, 0, 256);
            fgets(buf, 256, fp);

            s = buf;
            s += strspn(s, " \t");
            if (*s && *s != '#') {
                char *str;
                str = strchr(s, '\n');
                if (str) *str = 0;
                str = strchr(s, '#');
                if (str) *str = 0;
                str = strchr(s, '=');
                if (str) {
                    if(strstr(s, "over_temp_threshold")) {
                        config->overTempThreshold = 1000*atoi(++str);
                    } else if (strstr(s, "over_temp_reset")) {
                        config->overTempReset = 1000*atoi(++str);
                    } else if (strstr(s, "temp_hysteresis")) {
                        config->hysteresis = 1000*atof(++str);
                    } else if (strstr(s, "over_power_threshold")) {
                        config->overPowerThreshold = 1000*atof(++str);
                    } else if (strstr(s, "poll_interval")) {
                        config->pollInterval = 1000*atoi(++str);
                    } else if (strstr(s, "temp_delay")) {
                        config->tempDelay = 1000*atoi(++str);
                    } else if (strstr(s, "theta_jc")) {
                        config->thetaJC = 1000*atof(++str);
                    } else if (strstr(s, "agent")) {
                        str++;
                        str = trimwhitespace(str);
                        if (i<max_priorities) {
                            config->priorityTable[i++].agent = lookup(agents, str);
                        } else {
                            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
                            continue;
                        }
                    } else {
                        BDBG_WRN(("Unknown Thermal Configuration %s", s));
                    }
                }
            }
        }
        fclose(fp);
    } else {
        unsigned j;
        BDBG_WRN(("Thermal config file %s not found. Using defaults", filename));
        for (j=1; j<=4; j++) {
            BDBG_ASSERT(i<max_priorities);
            config->priorityTable[i++].agent = NxClient_CoolingAgent_eCpuPstate;
            config->priorityTable[i++].agent = NxClient_CoolingAgent_eCpuIdle;
#if NEXUS_HAS_GRAPHICSV3D
            config->priorityTable[i++].agent = NxClient_CoolingAgent_eGraphics3D;
#endif
            config->priorityTable[i++].agent = NxClient_CoolingAgent_eGraphics2D;
            config->priorityTable[i++].agent = NxClient_CoolingAgent_eUser;
        }
    }

    return rc;
}
