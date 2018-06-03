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
#if DYNRNG_HAS_CAPTURE
#include "capture.h"
#endif
#if DYNRNG_HAS_DBV
#include "dbv.h"
#endif
#if DYNRNG_HAS_CONSOLE
#include "console.h"
#endif
#if DYNRNG_HAS_PLAYLIST
#include "playlist.h"
#endif
#if DYNRNG_HAS_TESTER
#include "tester.h"
#endif
#if DYNRNG_HAS_PQ
#include "pq.h"
#endif
#include <stdbool.h>
#include <signal.h>
#include <pthread.h>

typedef enum UiMode
{
    UiMode_eBasic,
    UiMode_eExpert,
    UiMode_eMax
} UiMode;

typedef struct App
{
    ArgsHandle args;
    ConfigHandle cfg;

    struct
    {
        PlatformHandle handle;
        struct
        {
            PlatformDisplayHandle handle;
        } display;
        PlatformGraphicsHandle gfx;
        PlatformHdmiReceiverHandle rx;
        PlatformInputHandle input;
        PlatformModel model; /* what's been committed to hardware */
        struct
        {
            int min;
            int max;
        } gfxLuminance;
        struct
        {
            bool dynrngLock;
            PlatformPictureInfo pictureInfo; /* user settings */
        } output;
        PlatformUsageMode usageMode;
        PlatformListenerHandle usageNotifier;
        unsigned usageMessageIndex;
        bool blockedUsage;
        struct
        {
            struct
            {
                PlatformDynamicRangeProcessingCapabilities caps;
                PlatformDynamicRangeProcessingMode mode;
                PlatformPictureCtrlSettings picCtrl;
                int hdrPeakBrightness;
                int sdrPeakBrightness;
            } vid;
            struct
            {
                PlatformDynamicRangeProcessingCapabilities caps;
                PlatformDynamicRangeProcessingMode mode;
                PlatformPictureCtrlSettings picCtrl;
            } gfx;
        } processing;
        PlatformMediaPlayerHandle mediaPlayers[MAX_STREAMS];
        unsigned maxStreams;
        PlatformPlayMode playMode;
        PlatformPqSettings pq[MAX_STREAMS];
    } platform;

    struct
    {
        FileManagerHandle filer;
        StreamPlayerHandle players[MAX_STREAMS];
        char * prevPaths[MAX_STREAMS];
        unsigned count;
    } stream;
    struct
    {
        FileManagerHandle filer;
        ImageViewerHandle thumbnail;
        ImageViewerHandle background;
    } image;
    struct
    {
        FileManagerHandle filer;
        ScenarioPlayerHandle player;
        char * name;
    } scenario;
    struct
    {
        OsdHandle handle;
        unsigned layout;
    } osd;
    struct
    {
        bool done;
        struct sigaction term;
        pthread_t thread;
        UiMode mode;
    } ui;
#if DYNRNG_HAS_CAPTURE
    struct
    {
        CaptureHandle handle;
    } capture;
#endif
#if DYNRNG_HAS_DBV
    struct
    {
        DbvHandle handle;
    } dbv;
#endif
#if DYNRNG_HAS_CONSOLE
    struct
    {
        ConsoleHandle handle;
    } console;
#endif
#if DYNRNG_HAS_PLAYLIST
    struct
    {
        FileManagerHandle filer;
        PlaylistHandle handle;
        char * name;
    } playlist;
#endif
#if DYNRNG_HAS_TESTER
    struct
    {
        TesterHandle handle;
    } tester;
#endif
#if DYNRNG_HAS_PQ
    struct
    {
        PqHandle handle;
    } pq;
#endif
} App;

void app_p_hotplug_occurred(void * context, int param);
void app_p_stream_info_changed(void * context, int param);
void app_p_handle_scenario(void * context, Scenario * pScenario);
void app_p_thumbnail_changed(void * context, PlatformPictureHandle pic);
void app_p_background_changed(void * context, PlatformPictureHandle pic);

PlatformDynamicRange app_p_compute_output_dynamic_range(AppHandle app, PlatformDynamicRange requested);

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

void app_p_toggle_processing(AppHandle app);
void app_p_toggle_vid_processing(AppHandle app);
void app_p_toggle_gfx_processing(AppHandle app);
void app_p_toggle_pause(AppHandle app);
#if 0
void app_p_toggle_output_dynamic_range_lock(AppHandle app);
void app_p_cycle_output_dynamic_range(AppHandle app);
#endif
int app_p_run_scenario_by_number(AppHandle app, int scenarioNum);
int app_p_run_scenario(AppHandle app, const char * scenarioUrl);
void app_p_quit(AppHandle app);

void ui_run(AppHandle app);
int ui_start(AppHandle app);
void ui_stop(AppHandle app);
void ui_kill(AppHandle app);
void ui_wait(AppHandle app);

#endif /* APP_PRIV_H__ */
