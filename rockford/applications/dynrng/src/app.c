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
#include "platform.h"
#include "app.h"
#include "app_priv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>

void app_p_print_remote_usage(void)
{
    unsigned i;
    const char ** instructions;
    unsigned count;

    instructions = osd_get_instructions(&count);
    assert(instructions);

    for (i = 0; i < count; i++)
    {
        printf("%s\n", instructions[i]);
    }
}

PlatformDynamicRange app_p_compute_output_dynamic_range(AppHandle app, PlatformDynamicRange input)
{
    PlatformDynamicRange output = PlatformDynamicRange_eSdr;

    assert(app);

    if (app->forcedOutputEotf == PlatformDynamicRange_eAuto)
    {
        if (platform_receiver_supports_dynamic_range(app->rx, PlatformDynamicRange_eHdr10) == PlatformCapability_eSupported)
        {
            output = PlatformDynamicRange_eHdr10;
        }
        else if (platform_receiver_supports_dynamic_range(app->rx, PlatformDynamicRange_eHlg) == PlatformCapability_eSupported)
        {
            output = PlatformDynamicRange_eHlg;
        }
    }
    else
    {
        if (app->forcedOutputEotf < PlatformDynamicRange_eInvalid)
        {
            output = app->forcedOutputEotf;
        }
    }

    printf("Computed '%s' as output dynrng\n", platform_get_dynamic_range_name(output));

    return output;
}

void app_p_update_rcv_model(AppHandle app)
{
    PlatformReceiverModel * pModel;
    assert(app);
    pModel = &app->model.rcv;
    pModel->format = platform_receiver_supports_format(app->rx, app->model.sel.format ? &app->model.out.info.format : &app->model.vid.info.format);
    pModel->dynrng = platform_receiver_supports_dynamic_range(app->rx, app->model.vid.info.dynrng);
    printf("RCV: dynrng %s\n", pModel->dynrng ? "supported" : "unsupported");
    pModel->gamut = platform_receiver_supports_colorimetry(app->rx, app->model.sel.gamut ? app->model.out.info.gamut : app->model.vid.info.gamut);
    pModel->depth = platform_receiver_supports_color_depth(app->rx, app->model.sel.depth ? app->model.out.info.depth : app->model.vid.info.depth);
    pModel->space = platform_receiver_supports_color_space(app->rx, app->model.sel.space ? app->model.out.info.space : app->model.vid.info.space);
    osd_update_rcv_model(app->osd, pModel);
}

void app_p_update_sel_model(AppHandle app)
{
    PlatformSelectorModel * pModel;
    assert(app);
    pModel = &app->model.sel;
    pModel->format = true; /* TODO */
    pModel->dynrng = app->forcedOutputEotf != PlatformDynamicRange_eAuto;
    pModel->gamut = app->model.out.info.gamut != PlatformColorimetry_eAuto;
    pModel->space = app->model.out.info.space != PlatformColorSpace_eAuto;
    pModel->depth = app->model.out.info.depth != 0;
    osd_update_sel_model(app->osd, pModel);
}

void app_p_update_out_model(AppHandle app)
{
    PlatformPictureModel * pModel;
    PlatformDynamicRange outDynrng;

    assert(app);
    pModel = &app->model.out;
    platform_display_set_hdmi_colorimetry(app->display, app->model.out.info.gamut);
    outDynrng = app_p_compute_output_dynamic_range(app, app->model.vid.info.dynrng); /* we decide based on video input eotf and TV support only */
    if (outDynrng != pModel->info.dynrng)
    {
        pModel->info.dynrng = outDynrng;
        /* Making sure this only happens on change avoids reset of the luma settings done by VDC */
        platform_display_set_hdmi_drm_dynamic_range(app->display, pModel->info.dynrng);
    }
    memcpy(&app->model.out.info, platform_display_get_picture_info(app->display), sizeof(app->model.out.info));
    app_p_update_gfx_plm(app);
    app_p_update_vid_plm(app);
    osd_update_out_model(app->osd, pModel);
}

void app_p_update_gfx_plm(AppHandle app)
{
    PlatformPictureModel * pModel;
    assert(app);
    pModel = &app->model.gfx;
    plm_update_dynamic_range(app->plm.gfx, pModel->info.dynrng, app->model.out.info.dynrng);
    pModel->plm = plm_get(app->plm.gfx);
    osd_update_gfx_model(app->osd, pModel);
}

void app_p_update_gfx_model(AppHandle app)
{
    PlatformPictureModel * pModel;
    const PlatformPictureInfo * pInfo;
    assert(app);
    pModel = &app->model.gfx;
    pInfo = image_viewer_get_picture_info(app->pig ? app->background : app->thumbnail);
    if (pInfo)
    {
        memcpy(&pModel->info, pInfo, sizeof(pModel->info));
    }
    else
    {
        platform_get_default_picture_info(&pModel->info);
    }
    app_p_update_gfx_plm(app);
}

void app_p_update_vid_plm(AppHandle app)
{
    PlatformPictureModel * pModel;
    assert(app);
    pModel = &app->model.vid;
    plm_update_dynamic_range(app->plm.vid, pModel->info.dynrng, app->model.out.info.dynrng);
    pModel->plm = plm_get(app->plm.vid);
    osd_update_vid_model(app->osd, pModel);
}

void app_p_update_vid_model(AppHandle app)
{
    PlatformPictureModel * pModel;
    const PlatformPictureInfo * pInfo;
    assert(app);
    pModel = &app->model.vid;
    pInfo = stream_player_get_picture_info(app->streamPlayer);
    if (pInfo)
    {
        memcpy(&pModel->info, pInfo, sizeof(pModel->info));
    }
    else
    {
        platform_get_default_picture_info(&pModel->info);
    }
    app_p_update_vid_plm(app);
}

void app_p_reapply_plm(AppHandle app)
{
    platform_display_wait_for_display_settings_application(app->display);
    plm_reapply(app->plm.vid);
    plm_reapply(app->plm.gfx);
    /* reapply may change plm model, so update again */
    app_p_update_vid_plm(app);
    app_p_update_gfx_plm(app);
}

void app_p_update_model(AppHandle app)
{
    assert(app);

    /* TODO: clean up updates, there are too many duplicates */
    app_p_update_vid_model(app);
    app_p_update_gfx_model(app);
    app_p_update_sel_model(app);
    app_p_update_out_model(app);
    app_p_update_rcv_model(app);
    app_p_reapply_plm(app);
}

void app_p_next_video_setting(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    scenario_player_play_scenario(app->scenarioPlayer, -1);
    plm_next(app->plm.vid);
    app->model.vid.plm = plm_get(app->plm.vid);
    osd_update_vid_model(app->osd, &app->model.vid);
}

void app_p_prev_video_setting(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    scenario_player_play_scenario(app->scenarioPlayer, -1);
    plm_prev(app->plm.vid);
    app->model.vid.plm = plm_get(app->plm.vid);
    osd_update_vid_model(app->osd, &app->model.vid);
}

void app_p_next_graphics_setting(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    scenario_player_play_scenario(app->scenarioPlayer, -1);
    plm_next(app->plm.gfx);
    app->model.gfx.plm = plm_get(app->plm.gfx);
    osd_update_gfx_model(app->osd, &app->model.gfx);
}

void app_p_prev_graphics_setting(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    scenario_player_play_scenario(app->scenarioPlayer, -1);
    plm_prev(app->plm.gfx);
    app->model.gfx.plm = plm_get(app->plm.gfx);
    osd_update_gfx_model(app->osd, &app->model.gfx);
}

void app_p_next_stream(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    scenario_player_play_scenario(app->scenarioPlayer, -1);
    stream_player_next(app->streamPlayer);
    if (app->args->demo)
    {
        if (!app->outputEotfSticky)
        {
            app->forcedOutputEotf = PlatformDynamicRange_eAuto;
        }
        app->model.out.info.gamut = PlatformColorimetry_eAuto;
    }
}

void app_p_prev_stream(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    scenario_player_play_scenario(app->scenarioPlayer, -1);
    stream_player_prev(app->streamPlayer);
    if (app->args->demo)
    {
        if (!app->outputEotfSticky)
        {
            app->forcedOutputEotf = PlatformDynamicRange_eAuto;
        }
        app->model.out.info.gamut  = PlatformColorimetry_eAuto;
    }
}

void app_p_toggle_guide(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    scenario_player_play_scenario(app->scenarioPlayer, -1);
    osd_toggle_guide_visibility(app->osd);
}

void app_p_toggle_osd(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    scenario_player_play_scenario(app->scenarioPlayer, -1);
    osd_toggle_visibility(app->osd);
}

void app_p_toggle_pause(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    stream_player_toggle_pause(app->streamPlayer);
}

void app_p_toggle_details(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    scenario_player_play_scenario(app->scenarioPlayer, -1);
    osd_toggle_details_mode(app->osd);
}

void app_p_toggle_pig(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    scenario_player_play_scenario(app->scenarioPlayer, -1);
    app->pig = !app->pig;
    osd_toggle_pig_mode(app->osd);
    app_p_update_gfx_model(app);
}

void app_p_next_thumbnail(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    scenario_player_play_scenario(app->scenarioPlayer, -1);
    image_viewer_next(app->thumbnail);
}

void app_p_prev_thumbnail(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    scenario_player_play_scenario(app->scenarioPlayer, -1);
    image_viewer_prev(app->thumbnail);
}

void app_p_toggle_forced_sdr(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    app->forceSdr = !app->forceSdr;
    app_p_update_sel_model(app);
    app_p_update_out_model(app);
    app_p_reapply_plm(app);
}

void app_p_toggle_output_eotf_stickiness(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    app->outputEotfSticky = !app->outputEotfSticky;
    /* TODO: show some text on OSD? */
}

void app_p_cycle_output_eotf(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    if (++app->forcedOutputEotf >= PlatformDynamicRange_eInvalid)
    {
        app->forcedOutputEotf = PlatformDynamicRange_eSdr; /* skip eAuto */
    }
    app_p_update_sel_model(app);
    app_p_update_out_model(app);
    app_p_reapply_plm(app);
}

void app_p_cycle_colorimetry(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    scenario_player_play_scenario(app->scenarioPlayer, -1);
    app->model.out.info.gamut = (app->model.out.info.gamut + 1) % PlatformColorimetry_eUnknown;
    app_p_update_out_model(app);
    app_p_reapply_plm(app);
}

void app_p_cycle_background(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    scenario_player_play_scenario(app->scenarioPlayer, -1);
    image_viewer_next(app->background);
}

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
    app->done = true;
}

void app_p_hotplug_occurred(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    printf("app received hotplug; updating model\n");
    platform_display_print_hdmi_status(app->display);
    app_p_update_model(app);
    osd_flip(app->osd);
}

void app_p_stream_info_changed(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    printf("app received stream info event: updating model\n");
    app_p_update_model(app);
    osd_flip(app->osd);
}

void app_p_scenario_changed(void * context, const Scenario * pScenario)
{
    AppHandle app = context;
    assert(app);
    stream_player_play_stream(app->streamPlayer, pScenario->streamIndex);
    app->model.out.info.gamut = pScenario->gamut;
    if (!app->outputEotfSticky)
    {
        app->forcedOutputEotf = pScenario->eotf;
    }
    osd_set_visibility(app->osd, pScenario->osd);
    osd_set_guide_visibility(app->osd, pScenario->guide);
    osd_set_details_mode(app->osd, pScenario->details);
    image_viewer_view_image(app->thumbnail, pScenario->imageIndex);
    image_viewer_view_image(app->background, pScenario->bgIndex);
    app->pig = pScenario->pig;
    osd_set_pig_mode(app->osd, app->pig);
    if (pScenario->streamIndex != app->prevStreamIndex)
    {
        plm_set(app->plm.vid, -1); /* clear the currentSwitcher */
    }
    plm_set(app->plm.vid, pScenario->plm.vidIndex);
    plm_set(app->plm.gfx, pScenario->plm.gfxIndex);
    app_p_update_model(app);
    app->prevStreamIndex = pScenario->streamIndex;
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

void app_run(AppHandle app)
{
    assert(app);
    platform_receiver_start(app->rx);
    scenario_player_play_scenario(app->scenarioPlayer, 0);
    osd_flip(app->osd);

    app_p_print_remote_usage();

    while (!app->done)
    {
        if (platform_input_try(app->input))
        {
            osd_flip(app->osd);
        }
    }
}

static const char * STR_THUMBNAIL = "thumbnail";
static const char * STR_BACKGROUND = "background";

AppHandle app_create(ArgsHandle args)
{
    int rc = 0;
    static const char * fontPath = "nxclient/arial_18_aa.bwin_font";
    AppHandle app;
    OsdTheme theme;
    ImageViewerCreateSettings viewerCreateSettings;

    assert(args);

    app = malloc(sizeof(*app));
    assert(app);
    memset(app, 0, sizeof(*app));
    app->args = args;
    app->platform = platform_open(args->name);
    assert(app->platform);
    platform_get_default_model(&app->model);
    app->model.out.info.gamut = PlatformColorimetry_eAuto;
    app->gfx = platform_graphics_open(app->platform, fontPath, app->args->osd.dims.width, app->args->osd.dims.height);
    assert(app->gfx);
    app->display = platform_display_open(app->platform);
    assert(app->display);
    app->input = platform_input_open(app->platform, app->args->method);
    assert(app->input);
    theme.textForegroundColor = app->args->osd.colors.textFg;
    theme.mainPanelBackgroundColor = app->args->osd.colors.mainPanelBg;
    theme.guidePanelBackgroundColor = app->args->osd.colors.guidePanelBg;
    theme.detailsPanelBackgroundColor = app->args->osd.colors.detailsPanelBg;
    theme.infoPanelBackgroundColor = app->args->osd.colors.infoPanelBg;
    app->osd = osd_create(app->platform, app->gfx, &theme);
    assert(app->osd);
    app->streamPlayer = stream_player_create(app->platform, &app_p_stream_info_changed, app);
    assert(app->streamPlayer);
    app->scenarioPlayer = scenario_player_create(app->args->scenarioPath, &app_p_scenario_changed, app);
    assert(app->scenarioPlayer);
    app->plm.vid = plm_create("VID", args->vidSdr2HdrPath, args->vidHdr2SdrPath, args->vidHlg2HdrPath, app->platform, 0, 0, &platform_plm_set_vid_point, &platform_plm_get_vid_point, app->args->demo);
    assert(app->plm.vid);
    app->plm.gfx = plm_create("GFX", args->gfxSdr2HdrPath, NULL, NULL, NULL, 0, 0, &platform_plm_set_gfx_point, &platform_plm_get_gfx_point, app->args->demo);
    assert(app->plm.gfx);
    image_viewer_get_default_create_settings(&viewerCreateSettings);
    viewerCreateSettings.platform = app->platform;
    viewerCreateSettings.name = STR_THUMBNAIL;
    viewerCreateSettings.path = args->sdrThumbnailPath;
    viewerCreateSettings.pictureChanged.callback = &app_p_thumbnail_changed;
    viewerCreateSettings.pictureChanged.context = app;
    app->thumbnail = image_viewer_create(&viewerCreateSettings);
    assert(app->thumbnail);
    image_viewer_get_default_create_settings(&viewerCreateSettings);
    viewerCreateSettings.platform = app->platform;
    viewerCreateSettings.name = STR_BACKGROUND;
    viewerCreateSettings.path = args->sdrBackgroundPath;
    viewerCreateSettings.pictureChanged.callback = &app_p_background_changed;
    viewerCreateSettings.pictureChanged.context = app;
    app->background = image_viewer_create(&viewerCreateSettings);
    assert(app->background);
    app->rx = platform_receiver_open(app->platform, &app_p_hotplug_occurred, app);
    assert(app->rx);

    stream_player_add_stream_source(app->streamPlayer, "SDR", args->sdrStreamPath);
    stream_player_add_stream_source(app->streamPlayer, "HDR", args->hdrStreamPath);
    stream_player_add_stream_source(app->streamPlayer, "HLG", args->hlgStreamPath);
    stream_player_add_stream_source(app->streamPlayer, "MIX", args->mixStreamPath);

#if DYNRNG_QUIT_SUPPORT
    platform_input_set_event_handler(app->input, PlatformInputEvent_eQuit, &app_p_quit, app, 0);
#endif
    platform_input_set_event_handler(app->input, PlatformInputEvent_eQuit, &app_p_toggle_output_eotf_stickiness, app, 0);
    platform_input_set_event_handler(app->input, PlatformInputEvent_eToggleOsd, &app_p_toggle_osd, app, 0);
    platform_input_set_event_handler(app->input, PlatformInputEvent_eToggleGuide, &app_p_toggle_guide, app, 0);
    platform_input_set_event_handler(app->input, PlatformInputEvent_eCycleOutputEotf, &app_p_cycle_output_eotf, app, 0);
    platform_input_set_event_handler(app->input, PlatformInputEvent_eTogglePause, &app_p_toggle_pause, app, 0);
    platform_input_set_event_handler(app->input, PlatformInputEvent_eToggleDetails, &app_p_toggle_details, app, 0);
    platform_input_set_event_handler(app->input, PlatformInputEvent_eTogglePig, &app_p_toggle_pig, app, 0);
    platform_input_set_event_handler(app->input, PlatformInputEvent_eCycleColorimetry, &app_p_cycle_colorimetry, app, 0);
    platform_input_set_event_handler(app->input, PlatformInputEvent_eCycleBackground, &app_p_cycle_background, app, 0);
    platform_input_set_event_handler(app->input, PlatformInputEvent_eNextThumbnail, &app_p_next_thumbnail, app, 0);
    platform_input_set_event_handler(app->input, PlatformInputEvent_ePrevThumbnail, &app_p_prev_thumbnail, app, 0);
    platform_input_set_event_handler(app->input, PlatformInputEvent_eNextVideoSetting, &app_p_next_video_setting, app, 0);
    platform_input_set_event_handler(app->input, PlatformInputEvent_ePrevVideoSetting, &app_p_prev_video_setting, app, 0);
    platform_input_set_event_handler(app->input, PlatformInputEvent_eNextGraphicsSetting, &app_p_next_graphics_setting, app, 0);
    platform_input_set_event_handler(app->input, PlatformInputEvent_ePrevGraphicsSetting, &app_p_prev_graphics_setting, app, 0);
    platform_input_set_event_handler(app->input, PlatformInputEvent_eNextStream, &app_p_next_stream, app, 0);
    platform_input_set_event_handler(app->input, PlatformInputEvent_ePrevStream, &app_p_prev_stream, app, 0);
    platform_input_set_event_handler(app->input, PlatformInputEvent_eScenario1, &app_p_run_scenario, app, 0);
    platform_input_set_event_handler(app->input, PlatformInputEvent_eScenario2, &app_p_run_scenario, app, 1);
    platform_input_set_event_handler(app->input, PlatformInputEvent_eScenario3, &app_p_run_scenario, app, 2);
    platform_input_set_event_handler(app->input, PlatformInputEvent_eScenario4, &app_p_run_scenario, app, 3);
    platform_input_set_event_handler(app->input, PlatformInputEvent_eScenario5, &app_p_run_scenario, app, 4);
    platform_input_set_event_handler(app->input, PlatformInputEvent_eScenario6, &app_p_run_scenario, app, 5);
    platform_input_set_event_handler(app->input, PlatformInputEvent_eScenario7, &app_p_run_scenario, app, 6);
    platform_input_set_event_handler(app->input, PlatformInputEvent_eScenario8, &app_p_run_scenario, app, 7);
    platform_input_set_event_handler(app->input, PlatformInputEvent_eScenario9, &app_p_run_scenario, app, 8);
    platform_input_set_event_handler(app->input, PlatformInputEvent_eScenario0, &app_p_run_scenario, app, 9);

    return app;

error:
    app_destroy(app);
    return NULL;
}

void app_destroy(AppHandle app)
{
    if (!app) return;
    osd_update_background(app->osd, NULL);
    osd_update_thumbnail(app->osd, NULL);
    platform_receiver_close(app->rx);
    app->rx = NULL;
    image_viewer_destroy(app->background);
    app->background = NULL;
    image_viewer_destroy(app->thumbnail);
    app->thumbnail = NULL;
    plm_destroy(app->plm.gfx);
    app->plm.gfx = NULL;
    plm_destroy(app->plm.vid);
    app->plm.vid = NULL;
    scenario_player_destroy(app->scenarioPlayer);
    app->scenarioPlayer = NULL;
    stream_player_destroy(app->streamPlayer);
    app->streamPlayer = NULL;
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
