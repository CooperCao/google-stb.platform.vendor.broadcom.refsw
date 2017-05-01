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
    player->log = fopen("dynrng-report.txt", "w");
    return player;
}

void scenario_player_destroy(ScenarioPlayerHandle player)
{
    if (!player) return;

    if (player->switcher)
    {
        file_switcher_destroy(player->switcher);
    }
    if (player->log) fclose(player->log);
    free(player);
}

void scenario_player_p_print(ScenarioPlayerHandle player, const Scenario * pScenario)
{
    unsigned i;

    assert(player);
    printf("current scenario '%s'\n", file_switcher_get_path(player->switcher));
    for(i=0; i<MAX_MOSAICS; i++) {
        printf("stream%d = '%s'\n", i, pScenario->streamPaths[i]);
    }
#if 0 /* BANDREWS - we don't want to expose what options can go in a scenario file to just anyone */
    printf("# scenario %d\n", file_switcher_get_position(player->switcher));
    for(i=0; i<MAX_MOSAICS; i++) {
        printf("stream%d = '%s'\n", i, pScenario->streamPaths[i]);
    }
    printf("plm.vid = %u\n", pScenario->plm.vidIndex);
    printf("plm.gfx = %u\n", pScenario->plm.gfxIndex);
    printf("image = %s\n", pScenario->imagePath);
    printf("bg = %s\n", pScenario->bgPath);
    printf("pig = %s\n", pScenario->pig ? "on" : "off");
    printf("osd = %s\n", pScenario->osd ? "on" : "off");
    printf("details = %s\n", pScenario->details ? "on" : "off");
    printf("guide = %s\n", pScenario->guide ? "on" : "off");
    printf("gamut = %s\n", platform_get_colorimetry_name(pScenario->gamut));
    printf("dynrng = %s\n", platform_get_dynamic_range_name(pScenario->dynrng));
#endif
}

void scenario_player_p_get_default_scenario(Scenario * pScenario)
{
    memset(pScenario, 0, sizeof(*pScenario));
    pScenario->gamut = PlatformColorimetry_eAuto;
    pScenario->dynrng = PlatformDynamicRange_eAuto;
    pScenario->osd = true;
}

static void scenario_player_p_commit_scenario(ScenarioPlayerHandle player, Scenario * pScenario)
{
    printf("commit\n");
    if (player->scenarioChanged.callback)
    {
        player->scenarioChanged.callback(player->scenarioChanged.context, pScenario);
    }
}

static void scenario_player_p_handle_waituser(ScenarioPlayerHandle player, Scenario * pScenario)
{
    printf("\n\n\nHit enter to continue; q to stop this scenario: ");
    fflush(stdout);
    fgets(player->input, MAX_INPUT_LEN, stdin);
}

static void scenario_player_p_handle_results(ScenarioPlayerHandle player, Scenario * pScenario)
{
    int pass = 0;
    printf("\n\n\nPASS (1) or FAIL (0)> ");
    fflush(stdout);
    fgets(player->input, MAX_INPUT_LEN, stdin);
    switch (player->input[0])
    {
        case '1':
            pass = true;
            break;
        case 'q':
            pass = -1;
            break;
        default:
        case '0':
            printf("\n\n\nComment> ");
            fflush(stdout);
            fgets(player->input, MAX_INPUT_LEN, stdin);
            break;
    }
    if (pass >= 0 && player->log)
    {
        fprintf(player->log, "%s %s", pScenario->scenarioPath, pass == 1 ? "PASS" : "FAIL");
        if (!pass && strlen(player->input))
        {
            fprintf(player->log, ": %s", player->input);
        }
        fprintf(player->log, "\n");
    }
}

static void scenario_player_p_handle_echo(ScenarioPlayerHandle player, Scenario * pScenario, char * line)
{
    char * s;
    char * e;

    s = strchr(line, '"');
    if (s)
    {
        e = strrchr(line, '"');
    }
    else
    {
        s = strchr(line, '\'');
        e = strrchr(line, '\'');
    }
    if (!s)
    {
        printf("scenario_player: echo requires that the printable string be quoted\n");
        return;
    }
    if (!e)
    {
        printf("scenario_player: end quote symbol missing or mismatched\n");
        return;
    }
    s++;
    *e-- = 0;

    if (strlen(s))
    {
        printf("\n\n\n%s\n\n\n", s);
        fflush(stdout);
    }
}

static void scenario_player_p_handle_command(ScenarioPlayerHandle player, Scenario * pScenario, char * line)
{
    char * command = strtok(line, " ");
    if (!strcmp(command, "commit"))
    {
        scenario_player_p_commit_scenario(player, pScenario);
    }
    else if (!strcmp(command, "echo"))
    {
        scenario_player_p_handle_echo(player, pScenario, line + strlen(command) + 1);
    }
    else if (!strcmp(command, "results"))
    {
        scenario_player_p_handle_results(player, pScenario);
    }
    else if (!strcmp(command, "waituser"))
    {
        scenario_player_p_handle_waituser(player, pScenario);
    }
}

static void scenario_player_p_set_variable(ScenarioPlayerHandle player, Scenario * pScenario, const char * name, int nlen, const char * value)
{
    int v;

    if (!strcmp(name, "stream"))
    {
        pScenario->streamPaths[0] = set_string(pScenario->streamPaths[0], value);
        if (pScenario->streamPaths[0] && strlen(pScenario->streamPaths[0]))
        {
            pScenario->streamCount++;
        }
    }
    else if ((nlen > 7) && !strncmp(name, "stream[", 7) && (name[nlen - 1] == ']'))
    {
        if (sscanf(name + 7, "%d", &v) && (v < MAX_MOSAICS))
        {
            pScenario->streamPaths[v] = set_string(pScenario->streamPaths[v], value);
            if (pScenario->streamPaths[v] && strlen(pScenario->streamPaths[v]))
            {
                pScenario->streamCount++;
            }
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
        pScenario->imagePath = set_string(pScenario->imagePath, value);
    }
    else if (!strcmp(name, "bg"))
    {
        pScenario->bgPath = set_string(pScenario->bgPath, value);
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
        if (!strcasecmp(value, "off") || !strtoul(value, NULL, 0))
        {
            pScenario->osd = false;
        }
    }
    else if (!strcmp(name, "forceRestart"))
    {
        if (!strcasecmp(value, "on") || strtoul(value, NULL, 0))
        {
            pScenario->forceRestart = true;
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
    else if (!strcmp(name, "dynrng"))
    {
        if (!strcasecmp(value, "auto"))
        {
            pScenario->dynrng = PlatformDynamicRange_eAuto;
        }
        else if (!strcasecmp(value, "sdr"))
        {
            pScenario->dynrng = PlatformDynamicRange_eSdr;
        }
        else if (!strcasecmp(value, "hlg"))
        {
            pScenario->dynrng = PlatformDynamicRange_eHlg;
        }
        else if (!strcasecmp(value, "hdr10"))
        {
            pScenario->dynrng = PlatformDynamicRange_eHdr10;
        }
        else if (!strcasecmp(value, "dvs"))
        {
            pScenario->dynrng = PlatformDynamicRange_eDolbyVision;
        }
        else if (!strcasecmp(value, "disabled"))
        {
            pScenario->dynrng = PlatformDynamicRange_eInvalid;
        }
        else
        {
            pScenario->dynrng = (PlatformDynamicRange)strtoul(value, NULL, 0);
        }
    }
    else if (!strcmp(name, "layout"))
    {
        if (sscanf(value, "%d", &v))
        {
            pScenario->layout = v;
        }
    }
    else
    {
        printf("scenario_player: unrecognized variable name: '%s', ignored\n", name);
    }
}

void scenario_player_p_play_scenario(ScenarioPlayerHandle player, Scenario * pScenario, const char * path)
{
    FILE * f;
    static char line[LINE_LEN];
    char * p;
    char * name;
    char * value;
    int nlen;

    assert(pScenario);
    assert(path);

    f = fopen(path, "r");
    if (!f)
    {
        printf("scenario_player: unable to open '%s'\n", path);
        return;
    }

    printf("scenario_player: playing scenario '%s'\n", path);

    scenario_player_p_get_default_scenario(pScenario);
    while (!feof(f))
    {
        memset(line, 0, LINE_LEN);
        if (!fgets(line, LINE_LEN, f)) break;

        /* get rid of newline */
        p = strchr(line, '\n');
        if (p) *p = 0;
        /* get rid of comments and everything after */
        p = strchr(line, '#');
        if (p) *p = 0;

        p = strchr(line, '=');

        name = line;
        value = NULL;
        if (p)
        {
            *p = 0;
            value = p + 1;
            /* trim whitespace */
            value = trim(value);
        }
        name = trim(name);
        nlen = strlen(name);

        if (nlen)
        {
            if (value)
            {
                /* set variable */
                scenario_player_p_set_variable(player, pScenario, name, nlen, value);
            }
            else
            {
                /* other command */
                scenario_player_p_handle_command(player, pScenario, name);
            }
        }
    }
    /* commit always at end so that explicit commit is not required */
    scenario_player_p_commit_scenario(player, pScenario);
    printf("scenario_player: completed scenario '%s'\n", path);
}

void scenario_player_play_scenario(ScenarioPlayerHandle player, int scenarioNumber)
{
    Scenario scenario;
    const char * path;
    char name[32];
    unsigned i;
    int scenarioIndex;

    assert(player);

    memset(&scenario, 0, sizeof(scenario));

    if (scenarioNumber < 0)
    {
        printf("scenario player: reset\n");
/*        file_switcher_set_position(player->switcher, scenarioNumber);*/
        return;
    }
    snprintf(name, 32, "%d.txt", scenarioNumber);
    scenarioIndex = file_switcher_find(player->switcher, name);
    file_switcher_set_position(player->switcher, scenarioIndex);
    path = file_switcher_get_path(player->switcher);
    if (!path) { printf("scenario_player: NULL path returned for scenario\n"); return; }
    scenario_player_p_play_scenario(player, &scenario, path);

    /* TODO: cache scenarios */
    if (scenario.bgPath) free(scenario.bgPath);
    if (scenario.imagePath) free(scenario.imagePath);
    for (i = 0; i < MAX_MOSAICS; i++)
    {
        if (scenario.streamPaths[i]) free(scenario.streamPaths[i]);
    }
}
