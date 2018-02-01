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
#include "name_value_file_parser.h"
#include "platform_types.h"
#include "util.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

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

    return player;
error:
    scenario_player_destroy(player);
    return NULL;
}

void scenario_player_destroy(ScenarioPlayerHandle player)
{
    if (!player) return;

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
    pScenario->pictureInfo.gamut = PlatformColorimetry_eMax;
    pScenario->pictureInfo.space = PlatformColorSpace_eMax;
    pScenario->pictureInfo.dynrng = PlatformDynamicRange_eMax;
    pScenario->pictureInfo.depth = -1;
    pScenario->pictureInfo.sampling = -1;
    pScenario->processing.vid = PlatformDynamicRangeProcessingMode_eMax;
    pScenario->processing.gfx = PlatformDynamicRangeProcessingMode_eMax;
    pScenario->osd = true;
    pScenario->info = true;
}

static void scenario_player_p_get_reset_scenario(Scenario * pScenario)
{
    scenario_player_p_get_default_scenario(pScenario);
    pScenario->pictureInfo.gamut = PlatformColorimetry_e709;
    pScenario->pictureInfo.dynrng = PlatformDynamicRange_eInvalid;
    pScenario->pictureInfo.space = PlatformColorSpace_eYCbCr;
    pScenario->pictureInfo.depth = 8;
    pScenario->pictureInfo.sampling = 444;
    pScenario->pictureInfo.format.width = 1920;
    pScenario->pictureInfo.format.height = 1080;
    pScenario->pictureInfo.format.rate = 6000;
}

static int scenario_player_p_commit_scenario(ScenarioPlayerHandle player)
{
    printf("commit\n");
    if (player->createSettings.scenarioChanged.callback)
    {
        player->createSettings.scenarioChanged.callback(player->createSettings.scenarioChanged.context, player->pScenario);
    }
    return 0;
}

static int scenario_player_p_reset(ScenarioPlayerHandle player)
{
    printf("Replacing current scenario with reset scenario...\n");
    scenario_player_p_get_reset_scenario(player->pScenario);
    scenario_player_p_commit_scenario(player);
    return 0;
}

static int scenario_player_p_sleep(ScenarioPlayerHandle player, unsigned seconds)
{
    (void)player;
    printf("Sleeping...\n");
    sleep(seconds);
    return 0;
}

static int scenario_player_p_handle_command(ScenarioPlayerHandle player, const char * line)
{
    char * commandLine;
    char * command;
    int result;

    commandLine = strdup(line);
    command = strtok(commandLine, " ");

    if (!strcmp(command, "reset"))
    {
        result = scenario_player_p_reset(player);
    }
    else if (!strcmp(command, "commit"))
    {
        result = scenario_player_p_commit_scenario(player);
    }
    else if (!strcmp(command, "sleep"))
    {
        unsigned seconds = atoi(commandLine + strlen(command) + 1);
        result = scenario_player_p_sleep(player, seconds);
    }
    else
    {
        result = -ERR_NOT_FOUND;
    }
    if (commandLine) free(commandLine);
    return result;
}

static int scenario_player_p_set_variable(ScenarioPlayerHandle player, const char * name, const char * value)
{
    int v;
    Scenario * pScenario = player->pScenario;
    int nlen;
    int result = 0;

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
    else if (!strcmp(name, "info"))
    {
        if (!strcasecmp(value, "off") || !strtoul(value, NULL, 0))
        {
            pScenario->info = false;
        }
    }
    else if (!strcmp(name, "forceRestart"))
    {
        if (!strcasecmp(value, "on") || strtoul(value, NULL, 0))
        {
            pScenario->forceRestart = true;
        }
    }
    else if (!strcmp(name, "startPaused"))
    {
        if (!strcasecmp(value, "on") || strtoul(value, NULL, 0))
        {
            pScenario->startPaused = true;
        }
    }
    else if (!strcmp(name, "stcTrick"))
    {
        if (!strcasecmp(value, "off") || !strtoul(value, NULL, 0))
        {
            pScenario->stcTrick = false;
        }
    }
    else if (!strcmp(name, "gamut"))
    {
        if (!strcasecmp(value, "auto"))
        {
            pScenario->pictureInfo.gamut = PlatformColorimetry_eAuto;
        }
        else if (!strcasecmp(value, "601"))
        {
            pScenario->pictureInfo.gamut = PlatformColorimetry_e601;
        }
        else if (!strcasecmp(value, "709"))
        {
            pScenario->pictureInfo.gamut = PlatformColorimetry_e709;
        }
        else if (!strcasecmp(value, "2020"))
        {
            pScenario->pictureInfo.gamut = PlatformColorimetry_e2020;
        }
        else
        {
            pScenario->pictureInfo.gamut = (PlatformColorimetry)strtoul(value, NULL, 0);
        }
    }
    else if (!strcmp(name, "space"))
    {
        if (!strcasecmp(value, "auto"))
        {
            pScenario->pictureInfo.space = PlatformColorSpace_eAuto;
        }
        else if (!strcasecmp(value, "rgb"))
        {
            pScenario->pictureInfo.space = PlatformColorSpace_eRgb;
        }
        else if (!strcasecmp(value, "yuv") || !strcasecmp(value, "ycbcr"))
        {
            pScenario->pictureInfo.space = PlatformColorSpace_eYCbCr;
        }
        else
        {
            pScenario->pictureInfo.space = (PlatformColorSpace)strtoul(value, NULL, 0);
        }
    }
    else if (!strcmp(name, "dynrng"))
    {
        if (!strcasecmp(value, "auto"))
        {
            pScenario->pictureInfo.dynrng = PlatformDynamicRange_eAuto;
        }
        else if (!strcasecmp(value, "sdr"))
        {
            pScenario->pictureInfo.dynrng = PlatformDynamicRange_eSdr;
        }
        else if (!strcasecmp(value, "hlg"))
        {
            pScenario->pictureInfo.dynrng = PlatformDynamicRange_eHlg;
        }
        else if (!strcasecmp(value, "hdr10"))
        {
            pScenario->pictureInfo.dynrng = PlatformDynamicRange_eHdr10;
        }
        else if (!strcasecmp(value, "dbv"))
        {
            pScenario->pictureInfo.dynrng = PlatformDynamicRange_eDolbyVision;
        }
        else if (!strcasecmp(value, "disabled"))
        {
            pScenario->pictureInfo.dynrng = PlatformDynamicRange_eInvalid;
        }
        else
        {
            pScenario->pictureInfo.dynrng = (PlatformDynamicRange)strtoul(value, NULL, 0);
        }
    }
    else if (!strcmp(name, "layout"))
    {
        if (sscanf(value, "%d", &v))
        {
            pScenario->layout = v;
        }
    }
    else if (!strcmp(name, "depth"))
    {
        if (sscanf(value, "%d", &v))
        {
            switch (v)
            {
                case 8:
                    pScenario->pictureInfo.depth = 8;
                    break;
                case 10:
                    pScenario->pictureInfo.depth = 10;
                    break;
                case 12:
                    pScenario->pictureInfo.depth = 12;
                    break;
                default:
                case 0:
                    pScenario->pictureInfo.depth = 0;
                    break;
            }
        }
    }
    else if (!strcmp(name, "sampling"))
    {
        if (sscanf(value, "%d", &v))
        {
            switch (v)
            {
                case 420:
                    pScenario->pictureInfo.sampling = 420;
                    break;
                case 422:
                    pScenario->pictureInfo.sampling = 422;
                    break;
                default:
                case 444:
                    pScenario->pictureInfo.sampling = 444;
                    break;
            }
        }
    }
    else if (!strcmp(name, "luminance.gfx.max"))
    {
        if (sscanf(value, "%d", &v))
        {
            pScenario->gfxLuminance.max = v;
        }
    }
    else if (!strcmp(name, "luminance.gfx.min"))
    {
        if (sscanf(value, "%d", &v))
        {
            pScenario->gfxLuminance.min = v;
        }
    }
    else if (!strcmp(name, "playmode"))
    {
        if (!strcasecmp(value, "default"))
        {
            pScenario->playMode = PlatformPlayMode_eDefault;
        }
        else if (!strcasecmp(value, "loop"))
        {
            pScenario->playMode = PlatformPlayMode_eLoop;
        }
        else if (!strcasecmp(value, "once"))
        {
            pScenario->playMode = PlatformPlayMode_eOnce;
        }
        else
        {
            pScenario->playMode = (PlatformPlayMode)strtoul(value, NULL, 0);
        }
    }
    else if (!strcmp(name, "format"))
    {
        const char * scan = strchr(value, 'p');
        if (!scan) scan = strchr(value, 'i');
        if (scan)
        {
            pScenario->pictureInfo.format.height = atoi(value);
            if (*scan == 'i') pScenario->pictureInfo.format.interlaced = true;
            else pScenario->pictureInfo.format.interlaced = false;
            scan++;
            if (!strcmp(scan, "23.976"))
            {
                pScenario->pictureInfo.format.rate = 24;
                pScenario->pictureInfo.format.dropFrame = true;
            }
            else if (!strcmp(scan, "24"))
            {
                pScenario->pictureInfo.format.rate = 24;
                pScenario->pictureInfo.format.dropFrame = false;
            }
            else if (!strcmp(scan, "25"))
            {
                pScenario->pictureInfo.format.rate = 25;
                pScenario->pictureInfo.format.dropFrame = false;
            }
            else if (!strcmp(scan, "29.97"))
            {
                pScenario->pictureInfo.format.rate = 30;
                pScenario->pictureInfo.format.dropFrame = true;
            }
            else if (!strcmp(scan, "30"))
            {
                pScenario->pictureInfo.format.rate = 30;
                pScenario->pictureInfo.format.dropFrame = false;
            }
            else if (!strcmp(scan, "50"))
            {
                pScenario->pictureInfo.format.rate = 50;
                pScenario->pictureInfo.format.dropFrame = false;
            }
            else if (!strcmp(scan, "59.94"))
            {
                pScenario->pictureInfo.format.rate = 60;
                pScenario->pictureInfo.format.dropFrame = true;
            }
            else if (!strcmp(scan, "60"))
            {
                pScenario->pictureInfo.format.rate = 60;
                pScenario->pictureInfo.format.dropFrame = false;
            }
        }
        else
        {
            pScenario->pictureInfo.format.height=1080;
            pScenario->pictureInfo.format.interlaced=false;
            pScenario->pictureInfo.format.rate=60;
            pScenario->pictureInfo.format.dropFrame=false;
        }
    }
    else
    {
        result = -ERR_NOT_FOUND;
    }
    return result;
}

static int scenario_player_p_handle_nvp(void * ctx, const char * name, const char * value)
{
    ScenarioPlayerHandle player = ctx;
    int result = 0;

    assert(player);

    if (player->aborted) { return -1; }

    if (value)
    {
        /* set variable */
        result = scenario_player_p_set_variable(player, name, value);
    }
    else
    {
        /* other command */
        result = scenario_player_p_handle_command(player, name);
    }

    if (result == -ERR_NOT_FOUND && player->createSettings.unrecognizedSyntax.callback)
    {
        result = player->createSettings.unrecognizedSyntax.callback(player->createSettings.unrecognizedSyntax.context, name, value);
        if (result < 0)
        {
            switch (result)
            {
                case -ERR_NOT_FOUND:
                    printf("Unrecognized scenario syntax: '%s %s'\n", name, value);
                    break;
                default:
                    player->aborted = true;
                    break;
            }
        }
    }

    return result;
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
    if (!player->aborted)
    {
        scenario_player_p_commit_scenario(player);
        printf("scenario_player: completed scenario '%s'\n", path);
    }
    else
    {
        printf("scenario_player: aborted scenario '%s'\n", path);
    }
    player->pScenario = NULL;
}

const char * SCENARIO_PLAYER_EXIT_SCENARIO_NAME = "exit";

int scenario_player_play_scenario(ScenarioPlayerHandle player, const char * scenarioName)
{
    int result = 0;
    Scenario scenario;
    const char * path;
    char * fullname;
    unsigned i;

    assert(player);

    player->aborted = false;

    fullname = malloc(strlen(scenarioName) + 4 + 1);
    if (!fullname) { printf("scenario_player: out of memory\n"); result = -ERR_OOM; goto end; }
    strcpy(fullname, scenarioName);
    strcat(fullname, ".txt");

    memset(&scenario, 0, sizeof(scenario));

    path = file_manager_find(player->createSettings.filer, fullname);
    if (!path)
    {
        if (strcmp(scenarioName, SCENARIO_PLAYER_EXIT_SCENARIO_NAME)) result = -ERR_NOT_FOUND;
        goto end;
    }
    scenario_player_p_play_scenario(player, &scenario, path);

    /* TODO: cache scenarios */
    if (scenario.bgPath) free(scenario.bgPath);
    if (scenario.imagePath) free(scenario.imagePath);
    for (i = 0; i < MAX_MOSAICS; i++)
    {
        if (scenario.streamPaths[i]) free(scenario.streamPaths[i]);
    }
    if (player->aborted)
    {
        player->pScenario = &scenario;
        scenario_player_p_reset(player);
        player->pScenario = NULL;
        result = -ERR_ABORT;
    }
end:
    return result;
}

void scenario_player_abort(ScenarioPlayerHandle player)
{
    assert(player);
    player->aborted = true;
}
