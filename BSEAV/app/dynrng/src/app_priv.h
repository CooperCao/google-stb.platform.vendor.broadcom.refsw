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
#ifndef APP_PRIV_H__
#define APP_PRIV_H__ 1

#include "platform.h"
#include "app.h"
#include "args.h"
#include "config.h"
#include "image_viewer.h"
#include "stream_player.h"
#include "scenario_player.h"
#include "osd.h"
#include <stdbool.h>
#include <signal.h>
#include <pthread.h>

typedef struct App
{
    ArgsHandle args;
    ConfigHandle cfg;
    bool done;
    PlatformHandle platform;
    PlatformDisplayHandle display;
    PlatformGraphicsHandle gfx;
    char gfxFormat[16];
    char vidFormat[16];
    PlatformHdmiReceiverHandle rx;
    PlatformInputHandle input;
    PlatformModel model;
    struct
    {
        PlatformDynamicRange dynrng;
        bool dynrngLock;
    } output;
    PlatformUsageMode usageMode;
    struct
    {
        struct
        {
            PlatformDynamicRangeProcessingCapabilities caps;
            PlatformDynamicRangeProcessingMode mode;
        } vid;
        struct
        {
            PlatformDynamicRangeProcessingCapabilities caps;
            PlatformDynamicRangeProcessingMode mode;
        } gfx;
    } processing;
    FileManagerHandle streamsFiler;
    FileManagerHandle scenariosFiler;
    FileManagerHandle imagesFiler;
    StreamPlayerHandle streamPlayer[MAX_MOSAICS];
    ScenarioPlayerHandle scenarioPlayer;
    ImageViewerHandle thumbnail;
    ImageViewerHandle background;
    OsdHandle osd;
    char * prevStreamPaths[MAX_MOSAICS];
    unsigned streamCount;
    unsigned layout;
    unsigned mosaicCount;
    struct sigaction term;
    pthread_t uiThread;
} App;

void app_p_hotplug_occurred(void * context, int param);
void app_p_stream_info_changed(void * context, int param);
void app_p_handle_scenario(void * context, Scenario * pScenario);
void app_p_thumbnail_changed(void * context, PlatformPictureHandle pic);
void app_p_background_changed(void * context, PlatformPictureHandle pic);

PlatformDynamicRange app_p_compute_output_dynamic_range(AppHandle hApp);

void app_p_update_gfx_processing_model(AppHandle app);
void app_p_update_vid_processing_model(AppHandle app);
void app_p_update_vid_model(AppHandle app);
void app_p_update_gfx_model(AppHandle app);
void app_p_update_out_model(AppHandle app);
void app_p_apply_vid_processing(AppHandle app);
void app_p_apply_gfx_processing(AppHandle app);
void app_p_update_model(AppHandle app);
void app_p_apply_scenario(AppHandle app, const Scenario * pScenario);
void app_p_set_usage_mode(AppHandle app, PlatformUsageMode usageMode);

void app_p_toggle_processing(void * context, int param);
void app_p_toggle_vid_processing(void * context, int param);
void app_p_toggle_gfx_processing(void * context, int param);
void app_p_toggle_pause(void * context, int param);
#if 0
void app_p_toggle_output_dynamic_range_lock(void * context, int param);
void app_p_cycle_output_dynamic_range(void * context, int param);
#endif
void app_p_run_scenario(void * context, int param);
void app_p_quit(void * context, int param);

#endif /* APP_PRIV_H__ */
