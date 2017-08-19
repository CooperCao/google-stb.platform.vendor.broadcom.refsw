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
#include "scenario_player.h"
#include "scenario_player_priv.h"
#include "name_value_file_parser_priv.h"
#include "platform_types.h"
#include "util_priv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

bool scenario_player_file_filter(const char * path)
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

void scenario_player_get_default_create_settings(ScenarioPlayerCreateSettings * pSettings)
{
    assert(pSettings);
    memset(pSettings, 0, sizeof(*pSettings));
}

ScenarioPlayerHandle scenario_player_create(const ScenarioPlayerCreateSettings * pSettings)
{
    ScenarioPlayerHandle player;

    assert(pSettings);
    assert(pSettings->filer);

    player = malloc(sizeof(*player));
    if (!player) goto error;
    memset(player, 0, sizeof(*player));
    memcpy(&player->createSettings, pSettings, sizeof(*pSettings));
    player->log = fopen("dynrng-report.txt", "w");

    return player;
error:
    scenario_player_destroy(player);
    return NULL;
}

void scenario_player_destroy(ScenarioPlayerHandle player)
{
    if (!player) return;

    if (player->log) fclose(player->log);
    free(player);
}

void scenario_player_p_print(ScenarioPlayerHandle player)
{
    unsigned i;
    Scenario * pScenario;

    assert(player);
    pScenario = player->pScenario;
    printf("current scenario '%s'\n", pScenario->scenarioPath);
    for(i=0; i<MAX_MOSAICS; i++) {
        printf("stream%d = '%s'\n", i, pScenario->streamPaths[i]);
    }
}

void scenario_player_p_get_default_scenario(Scenario * pScenario)
{
    memset(pScenario, 0, sizeof(*pScenario));
    pScenario->gamut = PlatformColorimetry_eAuto;
    pScenario->dynrng = PlatformDynamicRange_eAuto;
    pScenario->processing.vid = PlatformDynamicRangeProcessingMode_eAuto;
    pScenario->processing.gfx = PlatformDynamicRangeProcessingMode_eAuto;
    pScenario->osd = true;
}

static void scenario_player_p_get_reset_scenario(Scenario * pScenario)
{
    scenario_player_p_get_default_scenario(pScenario);
    pScenario->gamut = PlatformColorimetry_e709;
    pScenario->dynrng = PlatformDynamicRange_eInvalid;
}

static void scenario_player_p_commit_scenario(ScenarioPlayerHandle player)
{
    printf("commit\n");
    if (player->createSettings.scenarioChanged.callback)
    {
        player->createSettings.scenarioChanged.callback(player->createSettings.scenarioChanged.context, player->pScenario);
    }
}

static void scenario_player_p_handle_reset(ScenarioPlayerHandle player)
{
    printf("Resetting to known state...\n");
    scenario_player_p_get_reset_scenario(player->pScenario);
    scenario_player_p_commit_scenario(player);
}

static void scenario_player_p_handle_waituser(ScenarioPlayerHandle player)
{
    printf("\n\n\nHit enter to continue; q to stop this scenario: ");
    fflush(stdout);
    fgets(player->input, MAX_INPUT_LEN, stdin);
}

static void scenario_player_p_handle_results(ScenarioPlayerHandle player)
{
    int pass = 0;
    printf("\n\n\nPASS (1) or FAIL (0)> ");
    fflush(stdout);
    fgets(player->input, MAX_INPUT_LEN, stdin);
    switch (player->input[0])
    {
        default:
        case '1':
            pass = 1;
            break;
        case 'q':
            pass = -1;
            break;
        case '0':
            printf("\n\n\nComment> ");
            fflush(stdout);
            fgets(player->input, MAX_INPUT_LEN, stdin);
            break;
    }
    if (pass >= 0 && player->log)
    {
        fprintf(player->log, "%s %s", player->pScenario->scenarioPath, pass == 1 ? "PASS" : "FAIL");
        if (!pass && strlen(player->input))
        {
            fprintf(player->log, ": %s", player->input);
        }
        fprintf(player->log, "\n");
    }
}

static void scenario_player_p_handle_echo(ScenarioPlayerHandle player, char * line)
{
    char * s;
    char * e;

    (void)player;

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
        printf("%s\n", s);
        fflush(stdout);
    }
}

static void scenario_player_p_handle_command(ScenarioPlayerHandle player, const char * line)
{
    char * commandLine;
    char * command;

    commandLine = strdup(line);
    command = strtok(commandLine, " ");

    if (!strcmp(command, "commit"))
    {
        scenario_player_p_commit_scenario(player);
    }
    else if (!strcmp(command, "echo"))
    {
        scenario_player_p_handle_echo(player, commandLine + strlen(command) + 1);
    }
    else if (!strcmp(command, "results"))
    {
        scenario_player_p_handle_results(player);
    }
    else if (!strcmp(command, "waituser"))
    {
        scenario_player_p_handle_waituser(player);
    }
    else if (!strcmp(command, "reset"))
    {
        scenario_player_p_handle_reset(player);
    }
    if (commandLine) free(commandLine);
}

static void scenario_player_p_set_variable(ScenarioPlayerHandle player, const char * name, const char * value)
{
    int v;
    Scenario * pScenario = player->pScenario;
    int nlen;

    nlen = strlen(name);

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
    else if (!strcmp(name, "processing.vid"))
    {
        if (!strcasecmp(value, "off") || !strcasecmp(value, "0"))
        {
            pScenario->processing.vid = PlatformDynamicRangeProcessingMode_eOff;
        }
        else
        {
            pScenario->processing.vid = PlatformDynamicRangeProcessingMode_eAuto;
        }
    }
    else if (!strcmp(name, "processing.gfx"))
    {
        if (!strcasecmp(value, "off") || !strcasecmp(value, "0"))
        {
            pScenario->processing.gfx = PlatformDynamicRangeProcessingMode_eOff;
        }
        else
        {
            pScenario->processing.gfx = PlatformDynamicRangeProcessingMode_eAuto;
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
    else if (!strcmp(name, "usage"))
    {
        if (!strcasecmp(value, "pig"))
        {
            pScenario->usageMode = PlatformUsageMode_ePictureInGraphics;
        }
        else if (!strcasecmp(value, "mosaic"))
        {
            pScenario->usageMode = PlatformUsageMode_eMosaic;
        }
        else if (!strcasecmp(value, "pip"))
        {
            pScenario->usageMode = PlatformUsageMode_eMainPip;
        }
        else
        {
            pScenario->usageMode = PlatformUsageMode_eFullScreenVideo;
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

static void scenario_player_p_handle_nvp(void * ctx, const char * name, const char * value)
{
    ScenarioPlayerHandle player = ctx;

    assert(player);

    if (value)
    {
        /* set variable */
        scenario_player_p_set_variable(player, name, value);
    }
    else
    {
        /* other command */
        scenario_player_p_handle_command(player, name);
    }
}

void scenario_player_p_play_scenario(ScenarioPlayerHandle player, Scenario * pScenario, const char * path)
{
    NameValueFileParserHandle parser = NULL;

    assert(pScenario);
    assert(path);

    parser = name_value_file_parser_create(path, &scenario_player_p_handle_nvp, player);
    if (!parser)
    {
        printf("scenario_player: unable to create name value file parser at '%s'\n", path);
        return;
    }

    printf("scenario_player: playing scenario '%s'\n", path);

    player->pScenario = pScenario;
    scenario_player_p_get_default_scenario(pScenario);
    name_value_file_parser_parse(parser);

    /* commit always at end so that explicit commit is not required */
    scenario_player_p_commit_scenario(player);
    printf("scenario_player: completed scenario '%s'\n", path);
    player->pScenario = NULL;
}

static const char * const EXIT_SCENARIO = "exit.txt";

void scenario_player_play_scenario(ScenarioPlayerHandle player, int scenarioNumber)
{
    Scenario scenario;
    const char * path;
    char name[32];
    unsigned i;

    assert(player);

    memset(&scenario, 0, sizeof(scenario));

    switch (scenarioNumber)
    {
        case SCENARIO_PLAYER_RESET:
            printf("scenario player: reset\n");
            player->pScenario = &scenario;
            scenario_player_p_handle_reset(player);
            player->pScenario = NULL;
            return;
            break;
        case SCENARIO_PLAYER_EXIT:
            printf("scenario player: exit\n");
            snprintf(name, 32, EXIT_SCENARIO);
            break;
        default:
            snprintf(name, 32, "%d.txt", scenarioNumber);
            break;
    }
    path = file_manager_find(player->createSettings.filer, name);
    if (!path) { if (strcmp(name, EXIT_SCENARIO)) printf("scenario_player: NULL path returned for scenario\n"); return; }
    scenario_player_p_play_scenario(player, &scenario, path);

    /* TODO: cache scenarios */
    if (scenario.bgPath) free(scenario.bgPath);
    if (scenario.imagePath) free(scenario.imagePath);
    for (i = 0; i < MAX_MOSAICS; i++)
    {
        if (scenario.streamPaths[i]) free(scenario.streamPaths[i]);
    }
}
