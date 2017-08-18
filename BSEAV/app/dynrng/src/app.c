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
#include "platform.h"
#include "app.h"
#include "app_priv.h"
#include "util_priv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

PlatformDynamicRange app_p_compute_output_dynamic_range(AppHandle app)
{
    PlatformDynamicRange output = PlatformDynamicRange_eSdr;

    assert(app);

    if (app->output.dynrng == PlatformDynamicRange_eAuto)
    {
        if (app->cfg->dbvOutputModeAutoSelection && platform_hdmi_receiver_supports_dynamic_range(app->rx, PlatformDynamicRange_eDolbyVision) == PlatformCapability_eSupported)
        {
            output = PlatformDynamicRange_eDolbyVision;
        }
        else if (platform_hdmi_receiver_supports_dynamic_range(app->rx, PlatformDynamicRange_eHdr10) == PlatformCapability_eSupported)
        {
            output = PlatformDynamicRange_eHdr10;
        }
        else if (platform_hdmi_receiver_supports_dynamic_range(app->rx, PlatformDynamicRange_eHlg) == PlatformCapability_eSupported)
        {
            output = PlatformDynamicRange_eHlg;
        }
    }
    else
    {
        if (app->output.dynrng <= PlatformDynamicRange_eInvalid)
        {
            output = app->output.dynrng;
        }
    }

    fprintf(stdout, "Computed '%s' as output dynrng\n", platform_get_dynamic_range_name(output));

    return output;
}

void app_p_update_out_model(AppHandle app)
{
    PlatformPictureModel * pModel;
    PlatformDynamicRange outDynrng;

    assert(app);
    pModel = &app->model.out;
    platform_display_set_colorimetry(app->display, app->model.out.info.gamut);
    outDynrng = app_p_compute_output_dynamic_range(app);
    if (outDynrng != pModel->info.dynrng)
    {
        pModel->info.dynrng = outDynrng;
        /* Making sure this only happens on change avoids reset of the luma settings done by VDC */
        platform_display_set_dynamic_range(app->display, pModel->info.dynrng);
    }
    memcpy(&app->model.out.info, platform_display_get_picture_info(app->display), sizeof(app->model.out.info));
    app_p_update_gfx_processing_model(app);
    app_p_update_vid_processing_model(app);
    osd_update_out_model(app->osd, pModel);
}

static PlatformTriState app_p_get_processing(PlatformDynamicRangeProcessingMode mode, PlatformDynamicRange in, PlatformDynamicRange out)
{
    PlatformTriState result = PlatformTriState_eInactive;
    if (in != out
        || in == PlatformDynamicRange_eDolbyVision
        || out == PlatformDynamicRange_eDolbyVision
        || in == PlatformDynamicRange_eTechnicolorPrime)
    {
        switch (mode)
        {
            case PlatformDynamicRangeProcessingMode_eOff:
                result = PlatformTriState_eOff;
                break;
            default:
            case PlatformDynamicRangeProcessingMode_eAuto:
                result = PlatformTriState_eOn;
                break;
        }
    }
    return result;
}

void app_p_update_gfx_processing_model(AppHandle app)
{
    PlatformPictureModel * pModel;
    assert(app);
    pModel = &app->model.gfx;

    pModel->processing = app_p_get_processing(app->processing.gfx.mode, pModel->info.dynrng, app->model.out.info.dynrng);

    osd_update_gfx_model(app->osd, pModel);
}

void app_p_update_gfx_model(AppHandle app)
{
    PlatformPictureModel * pModel;
    const PlatformPictureInfo * pInfo;
    assert(app);
    pModel = &app->model.gfx;
    pInfo = image_viewer_get_picture_info(app->usageMode == PlatformUsageMode_ePictureInGraphics ? app->background : app->thumbnail);
    if (pInfo)
    {
        memcpy(&pModel->info, pInfo, sizeof(pModel->info));
    }
    else
    {
        platform_get_default_picture_info(&pModel->info);
    }
    /* for now, always hard code graphics to SDR */
    pModel->info.dynrng = PlatformDynamicRange_eSdr;
    app_p_update_gfx_processing_model(app);
}

void app_p_update_vid_processing_model(AppHandle app)
{
    PlatformPictureModel * pModel;
    unsigned i;
    assert(app);
    for(i=0; i<app->streamCount; i++) {
        pModel = &app->model.vid[i];
        pModel->processing = app_p_get_processing(app->processing.vid.mode, pModel->info.dynrng, app->model.out.info.dynrng);
        osd_update_vid_model(app->osd, pModel, i);
    }
}

void app_p_update_vid_model(AppHandle app)
{
    PlatformPictureModel * pModel;
    const PlatformPictureInfo * pInfo;
    unsigned i;
    assert(app);
    for(i=0; i<app->streamCount; i++) {
        pModel = &app->model.vid[i];
        pInfo = stream_player_get_picture_info(app->streamPlayer[i]);
        if (pInfo)
        {
            memcpy(&pModel->info, pInfo, sizeof(pModel->info));
        }
        else
        {
            platform_get_default_picture_info(&pModel->info);
        }
    }
    app_p_update_vid_processing_model(app);
}

void app_p_update_model(AppHandle app)
{
    assert(app);
    app_p_update_vid_model(app);
    app_p_update_gfx_model(app);
    app_p_update_out_model(app);
}

void app_p_apply_vid_processing(AppHandle app)
{
    PlatformDynamicRangeProcessingSettings settings;
    unsigned windows;
    unsigned i;
    unsigned w;

    assert(app);

    switch (app->usageMode)
    {
        case PlatformUsageMode_eMosaic:
        case PlatformUsageMode_eMainPip:
            windows = 2;
            break;
        default:
        case PlatformUsageMode_ePictureInGraphics:
        case PlatformUsageMode_eFullScreenVideo:
            windows = 1;
            break;
    }

    for (w = 0; w < windows; w++)
    {
        platform_display_get_video_dynamic_range_processing_settings(app->display, w, &settings);
        /* turn off all processing, since it will affect when we switch input/output later */
        for (i = 0; i < PlatformDynamicRangeProcessingType_eMax; i++)
        {
            settings.modes[i] = app->processing.vid.mode;
        }
        platform_display_set_video_dynamic_range_processing_settings(app->display, w, &settings);
    }
    app_p_update_vid_processing_model(app);
}

void app_p_apply_gfx_processing(AppHandle app)
{
    PlatformDynamicRangeProcessingSettings settings;
    unsigned i;
    assert(app);
    platform_display_get_graphics_dynamic_range_processing_settings(app->display, &settings);
    /* turn off all processing, since it will affect when we switch input/output later */
    for (i = 0; i < PlatformDynamicRangeProcessingType_eMax; i++)
    {
        settings.modes[i] = app->processing.gfx.mode;
    }
    platform_display_set_graphics_dynamic_range_processing_settings(app->display, &settings);
    app_p_update_gfx_processing_model(app);
}

void app_p_toggle_pause(void * context, int param)
{
    AppHandle app = context;
    unsigned i;
    assert(app);
    (void)param;
    for(i=0; i<app->streamCount; i++) {
        stream_player_toggle_pause(app->streamPlayer[i]);
    }
}

void app_p_toggle_processing(void * context, int param)
{
    app_p_toggle_vid_processing(context, param);
    app_p_toggle_gfx_processing(context, param);
}

void app_p_toggle_vid_processing(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    (void)param;
    switch (app->processing.vid.mode)
    {
        default:
        case PlatformDynamicRangeProcessingMode_eAuto:
            app->processing.vid.mode = PlatformDynamicRangeProcessingMode_eOff;
            break;
        case PlatformDynamicRangeProcessingMode_eOff:
            app->processing.vid.mode = PlatformDynamicRangeProcessingMode_eAuto;
            break;
    }
    app_p_apply_vid_processing(app);
}

void app_p_toggle_gfx_processing(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    (void)param;
    switch (app->processing.gfx.mode)
    {
        default:
        case PlatformDynamicRangeProcessingMode_eAuto:
            app->processing.gfx.mode = PlatformDynamicRangeProcessingMode_eOff;
            break;
        case PlatformDynamicRangeProcessingMode_eOff:
            app->processing.gfx.mode = PlatformDynamicRangeProcessingMode_eAuto;
            break;
    }
    app_p_apply_gfx_processing(app);
}

#if 0
void app_p_toggle_output_dynamic_range_lock(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    app->output.dynrngLock = !app->output.dynrngLock;
    /* TODO: show some text on OSD? */
}

void app_p_cycle_output_dynamic_range(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    if (++app->output.dynrng >= PlatformDynamicRange_eInvalid)
    {
        app->output.dynrng = PlatformDynamicRange_eSdr; /* skip eAuto */
    }
    app_p_update_sel_model(app);
    app_p_update_out_model(app);
    app_p_reapply_plm(app);
}
#endif

void app_p_run_scenario(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    scenario_player_play_scenario(app->scenarioPlayer, param);
}

void app_p_quit(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    (void)param;
    app->done = true;
}

void app_p_hotplug_occurred(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    (void)param;
    fprintf(stdout, "app received hotplug; updating model\n");
    app_p_update_model(app);
    osd_flip(app->osd);
}

void app_p_stream_info_changed(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    fprintf(stdout, "app received stream(%d) info event: updating model\n", param);
    app_p_update_model(app);
    osd_flip(app->osd);
}

void app_p_reset_osd(AppHandle app)
{
    unsigned i;
    platform_get_default_model(&app->model);
    for(i=0; i<app->streamCount; i++)
        osd_update_vid_model(app->osd, &app->model.vid[i], i);
    osd_update_gfx_model(app->osd, &app->model.gfx);
    osd_update_out_model(app->osd, &app->model.out);
}

static const char * const usageModeStrings[] =
{
    "full-screen video",
    "pig",
    "mosaic",
    "pip",
    NULL
};

#define min(x,y) (((x) > (y)) ? (y) : (x))
void app_p_scenario_changed(void * context, const Scenario * pScenario)
{
    unsigned i;
    AppHandle app = context;

    assert(app);

    /* Switching from mosaic to non-non mosaic requires all videos to be stopped so that windows can be reconfigured. */
    if (app->usageMode != pScenario->usageMode || (app->layout != pScenario->layout && pScenario->usageMode == PlatformUsageMode_eMosaic)) {
        if (app->usageMode == PlatformUsageMode_eMosaic || pScenario->usageMode == PlatformUsageMode_eMosaic) {
            for(i=0; i<app->streamCount; i++) {
                stream_player_stop(app->streamPlayer[i]);
            }
        }
        else if (app->usageMode == PlatformUsageMode_eMainPip) {
            stream_player_stop(app->streamPlayer[1]);
        }
    }
    app_p_reset_osd(app);
    app->usageMode = pScenario->usageMode;
    printf("usage mode: %s\n", usageModeStrings[app->usageMode]);
    switch (app->usageMode)
    {
        case PlatformUsageMode_eMosaic:
            app->streamCount = min(pScenario->streamCount, app->mosaicCount);
            break;
        case PlatformUsageMode_eMainPip:
            app->streamCount = min(pScenario->streamCount, 2);
            break;
        default:
            app->streamCount = min(pScenario->streamCount, 1);
            break;
    }
    printf("stream count: %u\n", app->streamCount);
    app->layout = pScenario->layout;
    osd_set_usage_mode(app->osd, app->usageMode, app->layout);
    for(i=0; i<app->streamCount; i++) {
        stream_player_play_stream(app->streamPlayer[i], pScenario->streamPaths[i], app->usageMode, pScenario->forceRestart);
    }
    app->model.out.info.gamut = pScenario->gamut;
    if (!app->output.dynrngLock)
    {
        app->output.dynrng = pScenario->dynrng;
    }
    osd_set_visibility(app->osd, pScenario->osd);
    printf("image: %s\n", pScenario->imagePath);
    image_viewer_view_image(app->thumbnail, pScenario->imagePath);
    image_viewer_view_image(app->background, pScenario->bgPath);
    for(i=0; i<app->streamCount; i++)
    {
        app->prevStreamPaths[i] = set_string(app->prevStreamPaths[i], pScenario->streamPaths[i]);
    }
    app->processing.vid.mode = pScenario->processing.vid;
    app->processing.gfx.mode = pScenario->processing.gfx;
    app_p_apply_vid_processing(app);
    app_p_apply_gfx_processing(app);
    app_p_update_model(app);
}

void app_p_thumbnail_changed(void * context, PlatformPictureHandle pic)
{
    AppHandle app = context;
    assert(app);
    osd_update_thumbnail(app->osd, pic);
    app_p_update_gfx_model(app);
}

void app_p_background_changed(void * context, PlatformPictureHandle pic)
{
    AppHandle app = context;
    assert(app);
    osd_update_background(app->osd, pic);
    app_p_update_gfx_model(app);
}

static void * app_p_ui_thread(void * context)
{
    AppHandle app = context;
    while (!app->done)
    {
        if (platform_input_try(app->input))
        {
            osd_flip(app->osd);
        }
    }
    return NULL;
}

void app_run(AppHandle app)
{
    assert(app);

    platform_scheduler_start(platform_get_scheduler(app->platform));
    platform_hdmi_receiver_start(app->rx);
    scenario_player_play_scenario(app->scenarioPlayer, app->args->scenario);
    osd_flip(app->osd);

    /* note: weird threading is to ensure signal handler can cancel the ui thread */
    if (pthread_create(&app->uiThread, NULL, &app_p_ui_thread, app))
    {
        printf(("Unable to create app ui thread"));
        assert(0);
    }

    /* don't exit until ui thread finishes */
    pthread_join(app->uiThread, NULL);
}

static AppHandle gSigApp = NULL;

static void sighandler(int signum, siginfo_t * info, void * ctx)
{
    AppHandle app = gSigApp;
    (void)info;
    (void)ctx;
    printf("Received signal %d\n", signum);
    switch (signum)
    {
        case SIGINT:
        case SIGTERM:
            app->done = true;
            pthread_cancel(app->uiThread);
            break;
        default:
            break;
    }
}

static const char * STR_THUMBNAIL = "thumbnail";
static const char * STR_BACKGROUND = "background";
static const char * STR_STREAMS = "streams";
static const char * STR_IMAGES = "images";
static const char * STR_SCENARIOS = "scenarios";

AppHandle app_create(ArgsHandle args)
{
    static const char * fontPath = "nxclient/arial_18_aa.bwin_font";
    StringMapHandle cfgMap;
    AppHandle app;
    OsdCreateSettings osdCreateSettings;
    StreamPlayerCreateSettings streamPlayerCreateSettings;
    ScenarioPlayerCreateSettings scenarioPlayerCreateSettings;
    ImageViewerCreateSettings imageViewerCreateSettings;
    FileManagerCreateSettings fileManagerCreateSettings;
    unsigned i;

    assert(args);

    app = malloc(sizeof(*app));
    if (!app) goto error;
    memset(app, 0, sizeof(*app));
    app->args = args;

    app->term.sa_sigaction = &sighandler;
    app->term.sa_flags = SA_SIGINFO;
    gSigApp = app;
    sigaction(SIGTERM, &app->term, NULL);
    sigaction(SIGINT, &app->term, NULL);

    cfgMap = string_map_deserialize(app->args->configPath);
    if (!cfgMap) goto error;

    app->cfg = config_create(cfgMap);
    if (!app->cfg) goto error;

    app->platform = platform_open(args->name);
    if (!app->platform) goto error;

    platform_get_default_model(&app->model);
    app->model.out.info.gamut = PlatformColorimetry_eAuto;

    app->gfx = platform_graphics_open(app->platform, fontPath, app->cfg->osd.dims.width, app->cfg->osd.dims.height);
    if (!app->gfx) goto error;

    app->mosaicCount = platform_graphics_get_mosaic_count(app->gfx);
    app->display = platform_display_open(app->platform);
    if (!app->display) goto error;

    app->input = platform_input_open(app->platform, app->cfg->method);
    if (!app->input) goto error;

    osd_get_default_create_settings(&osdCreateSettings);
    osdCreateSettings.theme.textForegroundColor = app->cfg->osd.colors.textFg;
    osdCreateSettings.theme.mainPanelBackgroundColor = app->cfg->osd.colors.mainPanelBg;
    osdCreateSettings.theme.infoPanelBackgroundColor = app->cfg->osd.colors.infoPanelBg;
    osdCreateSettings.platform = app->platform;
    osdCreateSettings.gfx = app->gfx;
    app->osd = osd_create(&osdCreateSettings);
    if (!app->osd) goto error;

    file_manager_get_default_create_settings(&fileManagerCreateSettings);
    fileManagerCreateSettings.name = STR_STREAMS;
    fileManagerCreateSettings.path = app->cfg->streamsPath;
    fileManagerCreateSettings.filter = &stream_player_file_filter;
    app->streamsFiler = file_manager_create(&fileManagerCreateSettings);
    if (!app->streamsFiler) goto error;

    file_manager_get_default_create_settings(&fileManagerCreateSettings);
    fileManagerCreateSettings.name = STR_IMAGES;
    fileManagerCreateSettings.path = app->cfg->imagesPath;
    fileManagerCreateSettings.filter = &image_viewer_file_filter;
    app->imagesFiler = file_manager_create(&fileManagerCreateSettings);
    if (!app->imagesFiler) goto error;

    file_manager_get_default_create_settings(&fileManagerCreateSettings);
    fileManagerCreateSettings.name = STR_SCENARIOS;
    fileManagerCreateSettings.path = app->cfg->scenariosPath;
    fileManagerCreateSettings.filter = &scenario_player_file_filter;
    app->scenariosFiler = file_manager_create(&fileManagerCreateSettings);
    if (!app->scenariosFiler) goto error;

    stream_player_get_default_create_settings(&streamPlayerCreateSettings);
    streamPlayerCreateSettings.platform = app->platform;
    streamPlayerCreateSettings.streamInfo.callback = &app_p_stream_info_changed;
    streamPlayerCreateSettings.streamInfo.context = app;
    for(i=0; i<app->mosaicCount; i++) {
        app->streamPlayer[i] = stream_player_create(&streamPlayerCreateSettings);
        if (!app->streamPlayer[i]) goto error;
    }

    scenario_player_get_default_create_settings(&scenarioPlayerCreateSettings);
    scenarioPlayerCreateSettings.filer = app->scenariosFiler;
    scenarioPlayerCreateSettings.scenarioChanged.callback = &app_p_scenario_changed;
    scenarioPlayerCreateSettings.scenarioChanged.context = app;
    app->scenarioPlayer = scenario_player_create(&scenarioPlayerCreateSettings);
    if (!app->scenarioPlayer) goto error;

    image_viewer_get_default_create_settings(&imageViewerCreateSettings);
    imageViewerCreateSettings.platform = app->platform;
    imageViewerCreateSettings.name = STR_THUMBNAIL;
    imageViewerCreateSettings.filer = app->imagesFiler;
    imageViewerCreateSettings.pictureChanged.callback = &app_p_thumbnail_changed;
    imageViewerCreateSettings.pictureChanged.context = app;
    app->thumbnail = image_viewer_create(&imageViewerCreateSettings);
    if (!app->thumbnail) goto error;

    image_viewer_get_default_create_settings(&imageViewerCreateSettings);
    imageViewerCreateSettings.platform = app->platform;
    imageViewerCreateSettings.name = STR_BACKGROUND;
    imageViewerCreateSettings.filer = app->imagesFiler;
    imageViewerCreateSettings.pictureChanged.callback = &app_p_background_changed;
    imageViewerCreateSettings.pictureChanged.context = app;
    app->background = image_viewer_create(&imageViewerCreateSettings);
    if (!app->background) goto error;

    app->rx = platform_hdmi_receiver_open(app->platform, &app_p_hotplug_occurred, app);
    if (!app->rx) goto error;

    for(i=0; i<app->mosaicCount; i++) {
        stream_player_add_stream_source(app->streamPlayer[i], app->streamsFiler);
    }

    platform_input_set_event_handler(app->input, PlatformInputEvent_ePower, &app_p_quit, app);
    platform_input_set_event_handler(app->input, PlatformInputEvent_eSelect, &app_p_toggle_processing, app);
    platform_input_set_event_handler(app->input, PlatformInputEvent_eUp, &app_p_toggle_vid_processing, app);
    platform_input_set_event_handler(app->input, PlatformInputEvent_eDown, &app_p_toggle_vid_processing, app);
    platform_input_set_event_handler(app->input, PlatformInputEvent_eLeft, &app_p_toggle_gfx_processing, app);
    platform_input_set_event_handler(app->input, PlatformInputEvent_eRight, &app_p_toggle_gfx_processing, app);
    platform_input_set_event_handler(app->input, PlatformInputEvent_eNumber, &app_p_run_scenario, app);
    platform_input_set_event_handler(app->input, PlatformInputEvent_ePause, &app_p_toggle_pause, app);

    return app;

error:
    app_destroy(app);
    return NULL;
}

void app_destroy(AppHandle app)
{
    unsigned i;

    if (!app) return;

    scenario_player_play_scenario(app->scenarioPlayer, SCENARIO_PLAYER_EXIT);
    osd_update_background(app->osd, NULL);
    osd_update_thumbnail(app->osd, NULL);

    platform_scheduler_stop(platform_get_scheduler(app->platform));
    app->done = true;

    platform_hdmi_receiver_close(app->rx);
    app->rx = NULL;
    image_viewer_destroy(app->background);
    app->background = NULL;
    image_viewer_destroy(app->thumbnail);
    app->thumbnail = NULL;
    scenario_player_destroy(app->scenarioPlayer);
    app->scenarioPlayer = NULL;
    for(i=0; i<app->mosaicCount; i++) {
        stream_player_destroy(app->streamPlayer[i]);
        app->streamPlayer[i] = NULL;
        if (app->prevStreamPaths[i]) free(app->prevStreamPaths[i]);
    }
    osd_destroy(app->osd);
    app->osd = NULL;
    platform_input_close(app->input);
    app->input = NULL;
    platform_display_close(app->display);
    app->display = NULL;
    platform_graphics_close(app->gfx);
    app->gfx = NULL;
    platform_close(app->platform);
    app->platform = NULL;
    free(app);
}
