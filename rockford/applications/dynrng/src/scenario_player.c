/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *
 * Module Description:
 *
 *****************************************************************************/
#include "scenario_player.h"
#include "scenario_player_priv.h"
#include "platform_types.h"
#include "util_priv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

bool scenario_player_p_file_filter(const char * path)
{
    assert(path);

    if
    (
        strstr(path, ".txt")
    )
    {
        return true;
    }
    else
    {
        return false;
    }
}

ScenarioPlayerHandle scenario_player_create(const char * path, ScenarioChangedCallback scenarioChanged, void * scenarioChangedContext)
{
    ScenarioPlayerHandle player;

    player = malloc(sizeof(*player));
    assert(player);
    memset(player, 0, sizeof(*player));
    player->scenarioChanged.callback = scenarioChanged;
    player->scenarioChanged.context = scenarioChangedContext;
    player->switcher = file_switcher_create("scenario", path, &scenario_player_p_file_filter, false);
    assert(player->switcher);
    return player;
}

void scenario_player_destroy(ScenarioPlayerHandle player)
{
    if (!player) return;

    if (player->switcher)
    {
        file_switcher_destroy(player->switcher);
    }
    free(player);
}

void scenario_player_p_print_scenario(ScenarioPlayerHandle player, const Scenario * pScenario)
{
    assert(player);
    printf("Currently playing scenario '%s'\n", file_switcher_get_path(player->switcher));
    printf("# scenario %d\n", file_switcher_get_position(player->switcher));
    printf("stream = %u\n", pScenario->streamIndex);
    printf("plm.vid = %u\n", pScenario->plm.vidIndex);
    printf("plm.gfx = %u\n", pScenario->plm.gfxIndex);
    printf("image = %u\n", pScenario->imageIndex);
    printf("bg = %u\n", pScenario->bgIndex);
    printf("pig = %s\n", pScenario->pig ? "on" : "off");
    printf("osd = %s\n", pScenario->osd ? "on" : "off");
    printf("details = %s\n", pScenario->details ? "on" : "off");
    printf("guide = %s\n", pScenario->guide ? "on" : "off");
    printf("gamut = %s\n", platform_get_colorimetry_name(pScenario->gamut));
    printf("eotf = %s\n", platform_get_dynamic_range_name(pScenario->eotf));
}

void scenario_player_p_get_default_scenario(Scenario * pScenario)
{
    memset(pScenario, 0, sizeof(*pScenario));
    pScenario->gamut = PlatformColorimetry_eAuto;
    pScenario->eotf = PlatformDynamicRange_eAuto;
    pScenario->osd = true;
}

void scenario_player_p_load_scenario(ScenarioPlayerHandle player, Scenario * pScenario, const char * path)
{
    FILE * f;
    static char line[LINE_LEN];
    char * p;
    char * name;
    char * value;
    int v;

    assert(pScenario);
    assert(path);

    scenario_player_p_get_default_scenario(pScenario);
    f = fopen(path, "r");
    if (!f)
    {
        printf("Unable to open '%s'; using default scenario\n", path);
        return;
    }

    while (!feof(f))
    {
        memset(line, 0, LINE_LEN);
        if (!fgets(line, LINE_LEN, f)) break;

        /* get rid of newline */
        p = strchr(line, '\n');
        if (p) *p = 0;
        /* get rid of comments */
        p = strchr(line, '#');
        if (p) *p = 0;

        p = strchr(line, '=');
        if (!p) continue;

        name = line;
        value = p + 1;
        *p = 0;

        /* trim whitespace */
        while (*name != 0 && isspace(*name)) name++;
        p--;
        while (p > name && isspace(*p)) *p-- = 0;
        while (*value != 0 && isspace(*value)) value++;
        p = value + strlen(value) - 1;
        while (p > value && isspace(*p)) *p-- = 0;

        if (strlen(value) && strlen(name))
        {
            if (!strcmp(name, "stream"))
            {
                if (sscanf(value, "%d", &v))
                {
                    pScenario->streamIndex = v;
                }
            }
            else if (!strcmp(name, "plm.vid"))
            {
                if (sscanf(value, "%d", &v))
                {
                    pScenario->plm.vidIndex = v;
                }
            }
            else if (!strcmp(name, "plm.gfx"))
            {
                if (sscanf(value, "%d", &v))
                {
                    pScenario->plm.gfxIndex = v;
                }
            }
            else if (!strcmp(name, "image"))
            {
                if (sscanf(value, "%d", &v))
                {
                    pScenario->imageIndex = v;
                }
            }
            else if (!strcmp(name, "bg"))
            {
                if (sscanf(value, "%d", &v))
                {
                    pScenario->bgIndex = v;
                }
            }
            else if (!strcmp(name, "pig"))
            {
                if (!strcasecmp(value, "on") || strtoul(value, NULL, 0))
                {
                    pScenario->pig = true;
                }
            }
            else if (!strcmp(name, "osd"))
            {
                if (!strcasecmp(value, "on") || strtoul(value, NULL, 0))
                {
                    pScenario->osd = true;
                }
            }
            else if (!strcmp(name, "details"))
            {
                if (!strcasecmp(value, "on") || strtoul(value, NULL, 0))
                {
                    pScenario->details = true;
                }
            }
            else if (!strcmp(name, "guide"))
            {
                if (!strcasecmp(value, "on") || strtoul(value, NULL, 0))
                {
                    pScenario->guide = true;
                }
            }
            else if (!strcmp(name, "gamut"))
            {
                if (!strcasecmp(value, "auto"))
                {
                    pScenario->gamut = PlatformColorimetry_eAuto;
                }
                else if (!strcasecmp(value, "601"))
                {
                    pScenario->gamut = PlatformColorimetry_e601;
                }
                else if (!strcasecmp(value, "709"))
                {
                    pScenario->gamut = PlatformColorimetry_e709;
                }
                else if (!strcasecmp(value, "2020"))
                {
                    pScenario->gamut = PlatformColorimetry_e2020;
                }
                else
                {
                    pScenario->gamut = (PlatformColorimetry)strtoul(value, NULL, 0);
                }
            }
            else if (!strcmp(name, "eotf"))
            {
                if (!strcasecmp(value, "auto"))
                {
                    pScenario->eotf = PlatformDynamicRange_eAuto;
                }
                else if (!strcasecmp(value, "sdr"))
                {
                    pScenario->eotf = PlatformDynamicRange_eSdr;
                }
                else if (!strcasecmp(value, "hlg"))
                {
                    pScenario->eotf = PlatformDynamicRange_eHlg;
                }
                else if (!strcasecmp(value, "hdr10"))
                {
                    pScenario->eotf = PlatformDynamicRange_eHdr10;
                }
                else
                {
                    pScenario->eotf = (PlatformDynamicRange)strtoul(value, NULL, 0);
                }
            }
            else
            {
                printf("Unrecognized scenario variable name: '%s', ignored\n", name);
            }
        }
    }
#ifdef DEBUG
    printf("scenario_player: read file '%s'\n", path);
#endif
}

static void scenario_player_p_play_scenario(ScenarioPlayerHandle player, Scenario * pScenario)
{
    scenario_player_p_print_scenario(player, pScenario);
    if (player->scenarioChanged.callback)
    {
        player->scenarioChanged.callback(player->scenarioChanged.context, pScenario);
    }
}

static void scenario_player_p_play_default_scenario(ScenarioPlayerHandle player)
{
    Scenario scenario;
    printf("scenario: playing default scenario\n");
    scenario_player_p_get_default_scenario(&scenario);
    scenario_player_p_play_scenario(player, &scenario);
}

void scenario_player_play_scenario(ScenarioPlayerHandle player, int scenarioIndex)
{
    Scenario scenario;
    const char * path;
    unsigned count;
    assert(player);

    if (scenarioIndex < 0)
    {
        printf("scenario player reset\n");
        file_switcher_set_position(player->switcher, scenarioIndex);
        return;
    }
    count = file_switcher_get_count(player->switcher);
    if (scenarioIndex >= count) { printf("scenario: index %u out of bounds (%u)\n", scenarioIndex, count); goto fail; }
    if (file_switcher_get_position(player->switcher) == scenarioIndex) return;
    file_switcher_set_position(player->switcher, scenarioIndex);
    path = file_switcher_get_path(player->switcher);
    if (!path) { printf("scenario: NULL path returned for scenario\n"); goto fail; }
    scenario_player_p_load_scenario(player, &scenario, path);
    scenario_player_p_play_scenario(player, &scenario);
    return;

fail:
    scenario_player_p_play_default_scenario(player);
}
