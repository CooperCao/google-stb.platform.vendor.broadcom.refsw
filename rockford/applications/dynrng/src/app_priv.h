/******************************************************************************
 * Broadcom Proprietary and Confidential. (c) 2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/
#ifndef APP_PRIV_H__
#define APP_PRIV_H__ 1

#include "plm.h"
#include "nl2l.h"
#include "platform.h"
#include "app.h"
#include "args.h"
#include "image_viewer.h"
#include "stream_player.h"
#include "scenario_player.h"
#include "osd.h"
#include "shell.h"
#include <stdbool.h>

typedef struct App
{
    ArgsHandle args;
    bool done;
    PlatformHandle platform;
    PlatformDisplayHandle display;
    PlatformGraphicsHandle gfx;
    char gfxFormat[16];
    char vidFormat[16];
    PlatformReceiverHandle rx;
    PlatformInputHandle input;
    PlatformModel model;
    struct
    {
        PlatformDynamicRange dynrng;
        PlatformColorSpace colorSpace;
        bool dynrngLock;
    } output;
    bool pig;
    struct
    {
        PlmHandle vid[MAX_MOSAICS];
        PlmHandle gfx;
    } plm;
    Nl2lHandle nl2l;
    StreamPlayerHandle streamPlayer[MAX_MOSAICS];
    ScenarioPlayerHandle scenarioPlayer;
    ImageViewerHandle thumbnail;
    ImageViewerHandle background;
    OsdHandle osd;
    ShellHandle shell;
    char * prevStreamPaths[MAX_MOSAICS];
    unsigned streamCount;
    unsigned layout;
} App;

void app_p_hotplug_occurred(void * context, int param);
void app_p_stream_info_changed(void * context, int param);
void app_p_handle_scenario(void * context, Scenario * pScenario);
void app_p_thumbnail_changed(void * context, PlatformPictureHandle pic);
void app_p_background_changed(void * context, PlatformPictureHandle pic);

PlatformDynamicRange app_p_compute_output_dynamic_range(AppHandle hApp, PlatformDynamicRange input);

void app_p_update_gfx_plm(AppHandle app);
void app_p_update_vid_plm(AppHandle app);
void app_p_update_vid_model(AppHandle app);
void app_p_update_gfx_model(AppHandle app);
void app_p_update_out_model(AppHandle app);
void app_p_update_sel_model(AppHandle app);
void app_p_update_rcv_model(AppHandle app);
void app_p_reapply_plm(AppHandle app);
void app_p_update_model(AppHandle app);
void app_p_apply_scenario(AppHandle app, const Scenario * pScenario);
void app_p_set_pig_mode(AppHandle app, bool pig);

void app_p_print_remote_usage(AppHandle app);

void app_p_next_video_setting(void * context, int param);
void app_p_prev_video_setting(void * context, int param);
void app_p_next_graphics_setting(void * context, int param);
void app_p_prev_graphics_setting(void * context, int param);
void app_p_next_stream(void * context, int param);
void app_p_prev_stream(void * context, int param);
void app_p_next_thumbnail(void * context, int param);
void app_p_prev_thumbnail(void * context, int param);
void app_p_toggle_guide(void * context, int param);
void app_p_toggle_osd(void * context, int param);
void app_p_toggle_pause(void * context, int param);
void app_p_toggle_pig(void * context, int param);
void app_p_toggle_details(void * context, int param);
void app_p_toggle_forced_sdr(void * context, int param);
void app_p_toggle_output_dynamic_range_lock(void * context, int param);
void app_p_cycle_output_dynamic_range(void * context, int param);
void app_p_cycle_colorimetry(void * context, int param);
void app_p_cycle_background(void * context, int param);
void app_p_run_scenario(void * context, int param);
void app_p_run_command_shell(void * context, int param);
void app_p_quit(void * context, int param);

#endif /* APP_PRIV_H__ */
