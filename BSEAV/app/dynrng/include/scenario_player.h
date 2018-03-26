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
#ifndef SCENARIO_PLAYER_H__
#define SCENARIO_PLAYER_H__ 1

#include "platform_types.h"
#include "file_manager.h"
#include <stdbool.h>

extern const char * SCENARIO_PLAYER_EXIT_SCENARIO_NAME;

typedef struct ScenarioPlayer * ScenarioPlayerHandle;

typedef struct Scenario
{
    char * scenarioPath;
    char * streamPaths[MAX_STREAMS];
    unsigned streamCount;
    char * imagePath;
    char * bgPath;
    PlatformUsageMode usageMode;
    unsigned layout;
    bool osd;
    bool info;
    bool forceRestart;
    bool startPaused;
    bool stcTrick;
    PlatformPlayMode playMode;
    struct
    {
        PlatformDynamicRangeProcessingMode vid;
        PlatformDynamicRangeProcessingMode gfx;
    } processing;
    struct
    {
        int max;
        int min;
    } gfxLuminance;
    PlatformPictureInfo pictureInfo;
} Scenario;

typedef void (*ScenarioChangedCallback)(void * context, const Scenario * pScenario);
typedef int (*UnrecognizedScenarioSyntaxCallback)(void * context, const char * name, const char * value);

typedef struct ScenarioPlayerCreateSettings
{
    FileManagerHandle filer;
    struct
    {
        ScenarioChangedCallback callback;
        void * context;
    } scenarioChanged;
    struct
    {
        UnrecognizedScenarioSyntaxCallback callback;
        void * context;
    } unrecognizedSyntax;
} ScenarioPlayerCreateSettings;

bool scenario_player_file_filter(const char * path);
void scenario_player_get_default_create_settings(ScenarioPlayerCreateSettings * pSettings);
ScenarioPlayerHandle scenario_player_create(const ScenarioPlayerCreateSettings * pSettings);
void scenario_player_destroy(ScenarioPlayerHandle player);
int scenario_player_play_scenario(ScenarioPlayerHandle player, const char * scenarioName);
void scenario_player_abort(ScenarioPlayerHandle player);

#endif /* SCENARIO_PLAYER_H__ */
