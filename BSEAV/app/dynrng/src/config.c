/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/
#include "config.h"
#include "config_priv.h"
#include "string_map.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static const Config defaultConfig =
{
    "../etc/dynrng/scenarios/plm",
    "../share/dynrng/streams",
    "../share/dynrng/images",
    {
        { /* dims */
            1920,
            1080
        },
        { /* colors */
            0xffffffff, /* text, white 50% */
            0x00000000, /* mainPanelBg, black 0% */
            0x00000000, /* instructionPanelBg, black 0% */
            0xff000000, /* infoPanelBg, black 100% */
            0xff000000  /* detailsPanelBg, black 100% */
        }
    },
    true
};

void config_p_get_default(ConfigHandle cfg)
{
    assert(cfg);
    memset(cfg, 0, sizeof(*cfg));
    memcpy(&cfg->osd, &defaultConfig.osd, sizeof(cfg->osd));
    cfg->dbvOutputModeAutoSelection = defaultConfig.dbvOutputModeAutoSelection;
}

static char * config_p_get_path(char * path, const char * def)
{
    char * newPath = path;
    if (!newPath)
    {
        newPath = strdup(def);
    }
    return newPath;
}

#define CONFIG_P_GET_PATH(NAME) cfg->NAME = config_p_get_path(cfg->NAME, defaultConfig.NAME)

ConfigHandle config_create(StringMapHandle cfgMap)
{
    ConfigHandle cfg;
    int curarg = 1;
    StringMapCursorHandle cursor = NULL;
    const StringPair * pair;

    cfg = malloc(sizeof(*cfg));
    assert(cfg);
    config_p_get_default(cfg);

    cursor = string_map_create_key_cursor(cfgMap);

    for (pair = string_map_cursor_first(cursor); pair; pair = string_map_cursor_next(cursor))
    {
        if (!strcmp(pair->key, "streamsPath")) {
            cfg->streamsPath = strdup(pair->value);
        }
        else if (!strcmp(pair->key, "imagesPath")) {
            cfg->imagesPath = strdup(pair->value);
        }
        else if (!strcmp(pair->key, "scenariosPath")) {
            cfg->scenariosPath = strdup(pair->value);
        }
        else if (!strcmp(pair->key, "osd.colors.textFg")) {
            cfg->osd.colors.textFg = strtoul(pair->value, NULL, 0);
        }
        else if (!strcmp(pair->key, "osd.colors.mainPanelBg")) {
            cfg->osd.colors.mainPanelBg = strtoul(pair->value, NULL, 0);
        }
        else if (!strcmp(pair->key, "osd.colors.infoPanelBg")) {
            cfg->osd.colors.infoPanelBg = strtoul(pair->value, NULL, 0);
        }
        else if (!strcmp(pair->key, "osd")) {
            char * p = strchr(pair->value, 'x');
            if (!p) { printf("osd element syntax: widthxheight; example: 1920x1080\n"); goto error; }
            *p++ = 0;
            cfg->osd.dims.width = strtoul(pair->value, NULL, 0);
            cfg->osd.dims.height = strtoul(p, NULL, 0);
        }
        else if (!strcmp(pair->key, "dbvOutputModeAutoSelect")) {
            cfg->dbvOutputModeAutoSelection = strtoul(pair->value, NULL, 0) > 0 ? true : false;
        }
        curarg++;
    }

    /* fixup paths */
    CONFIG_P_GET_PATH(streamsPath);
    CONFIG_P_GET_PATH(imagesPath);
    CONFIG_P_GET_PATH(scenariosPath);

    return cfg;

error:
    config_destroy(cfg);
    return NULL;
}

void config_destroy(ConfigHandle cfg)
{
    if (!cfg) return;
    if (cfg->streamsPath) free(cfg->streamsPath);
    if (cfg->imagesPath) free(cfg->imagesPath);
    if (cfg->scenariosPath) free(cfg->scenariosPath);
    free(cfg);
}
