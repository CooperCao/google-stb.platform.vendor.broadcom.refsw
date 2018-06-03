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
#include "util.h"
#include "error.h"
#include "name_value_file_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "dbv.h"
#include "dbv_priv.h"

void dbv_get_default_create_settings(DbvCreateSettings * pSettings)
{
    assert(pSettings);
    memset(pSettings, 0, sizeof(*pSettings));
}

static void dbv_p_get_default_platform_settings(DbvPlatformSettings * pSettings)
{
    memset(pSettings, 0, sizeof(*pSettings));
}

#if 0
static void dbv_parse_config(DbvHandle dbv)
{
    StringMapHandle m = dbv->createSettings.cfgMap;
}
#endif

DbvHandle dbv_create(const DbvCreateSettings * pSettings)
{
    DbvHandle dbv;

    assert(pSettings);

    dbv = malloc(sizeof(*dbv));
    if (!dbv) goto error;
    memset(dbv, 0, sizeof(*dbv));
    memcpy(&dbv->createSettings, pSettings, sizeof(*pSettings));
    dbv_p_get_default_platform_settings(&dbv->platformSettings);

#if 0
    dbv_parse_config(dbv);
#endif

    return dbv;
error:
    dbv_destroy(dbv);
    return NULL;
}

void dbv_destroy(DbvHandle dbv)
{
    if (!dbv) return;
    free(dbv);
}

void dbv_p_print(DbvHandle dbv)
{
    assert(dbv);
}

static int dbv_p_set_variable(DbvHandle dbv, const char * name, const char * value)
{
    int result = 0;

    if (dbv->createSettings.platformSettingsRequest.callback)
    {
        if (!strcasecmp(name, "priority") || !strcasecmp(name, "prio"))
        {
            if (!strcasecmp(value, "gfx") || !strcasecmp(value, "graphic") || !strcasecmp(value, "graphics"))
            {
                dbv->platformSettings.renderingPriority = PlatformRenderingPriority_eGraphics;
            }
            else if (!strcasecmp(value, "vid") || !strcasecmp(value, "video"))
            {
                dbv->platformSettings.renderingPriority = PlatformRenderingPriority_eVideo;
            }
            else
            {
                dbv->platformSettings.renderingPriority = PlatformRenderingPriority_eAuto;
            }
        }
        else
        {
            result = -ERR_NOT_FOUND;
        }
        dbv->createSettings.platformSettingsRequest.callback(dbv->createSettings.platformSettingsRequest.context, &dbv->platformSettings);
    }
    else
    {
        result = -ERR_NOT_FOUND;
    }

    return result;
}

#if 0
static int dbv_p_handle_command(DbvHandle dbv, const char * line)
{
    char * commandLine;
    char * command;
    int result = 0;

    commandLine = strdup(line);
    command = strtok(commandLine, " ");

    if (!strcmp(command, "xxx"))
    {
        result = dbv_p_handle_xxx(dbv, commandLine + strlen(command) + 1);
    }
    else
    {
        result = -ERR_NOT_FOUND;
    }
    if (commandLine) free(commandLine);
    return result;
}
#endif

int dbv_handle_scenario_nvp(DbvHandle dbv, const char * name, const char * value)
{
    int result = -ERR_NOT_FOUND;

    assert(dbv);

#if 0
    if (!value)
    {
        /* command */
        result = dbv_p_handle_command(dbv, name);
    }
    else
#endif
    {
        /* set variable */
        result = dbv_p_set_variable(dbv, name, value);
    }

    return result;
}
