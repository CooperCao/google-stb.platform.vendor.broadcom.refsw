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
#include "app_shell_priv.h"
#include "util_priv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

void app_p_print_remote_usage(AppHandle app)
{
    if (app->args->advanced)
    {
        unsigned i;
        const char ** instructions;
        unsigned count;

        instructions = osd_get_instructions(&count);
        assert(instructions);

        for (i = 0; i < count; i++)
        {
            fprintf(stdout, "%s\n", instructions[i]);
        }
    }
}

PlatformDynamicRange app_p_compute_output_dynamic_range(AppHandle app, PlatformDynamicRange input)
{
    PlatformDynamicRange output = PlatformDynamicRange_eSdr;

    assert(app);

    if (app->output.dynrng == PlatformDynamicRange_eAuto)
    {
        if (app->args->dbvOutputModeAutoSelection && platform_receiver_supports_dynamic_range(app->rx, PlatformDynamicRange_eDolbyVision) == PlatformCapability_eSupported)
        {
            output = PlatformDynamicRange_eDolbyVision;
        }
        else if (platform_receiver_supports_dynamic_range(app->rx, PlatformDynamicRange_eHdr10) == PlatformCapability_eSupported)
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
        if (app->output.dynrng <= PlatformDynamicRange_eInvalid)
        {
            output = app->output.dynrng;
        }
    }

    fprintf(stdout, "Computed '%s' as output dynrng\n", platform_get_dynamic_range_name(output));

    return output;
}

void app_p_update_rcv_model(AppHandle app)
{
    PlatformReceiverModel * pModel;
    assert(app);
    pModel = &app->model.rcv;
    /* TODO : Handle mosaic mode */
    pModel->format = platform_receiver_supports_format(app->rx, app->model.sel.format ? &app->model.out.info.format : &app->model.vid[0].info.format);
    pModel->dynrng = platform_receiver_supports_dynamic_range(app->rx, app->model.vid[0].info.dynrng);
    fprintf(stdout, "RCV: dynrng %s\n", pModel->dynrng ? "supported" : "unsupported");
    pModel->gamut = platform_receiver_supports_colorimetry(app->rx, app->model.sel.gamut ? app->model.out.info.gamut : app->model.vid[0].info.gamut);
    pModel->depth = platform_receiver_supports_color_depth(app->rx, app->model.sel.depth ? app->model.out.info.depth : app->model.vid[0].info.depth);
    pModel->space = platform_receiver_supports_color_space(app->rx, app->model.sel.space ? app->model.out.info.space : app->model.vid[0].info.space);
    osd_update_rcv_model(app->osd, pModel);
}

void app_p_update_sel_model(AppHandle app)
{
    PlatformSelectorModel * pModel;
    assert(app);
    pModel = &app->model.sel;
    pModel->format = true; /* TODO */
    pModel->dynrng = app->output.dynrng != PlatformDynamicRange_eAuto;
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
    /* TODO : Currently input dynamic range is not used to compute output dynamic range,
     * so we set it to unknown. But if it is used in the future, then figure out which video stream to use */
    outDynrng = app_p_compute_output_dynamic_range(app, PlatformDynamicRange_eUnknown);
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

static PlatformTriState app_p_get_plm(PlmHandle plm)
{
    PlatformTriState result;
    int setting;
    setting = plm_get(plm);
    result = setting > 0 ? PlatformTriState_eOn : (setting == 0 ? PlatformTriState_eOff : PlatformTriState_eInactive);
    return result;
}

void app_p_update_gfx_plm(AppHandle app)
{
    PlatformPictureModel * pModel;
    assert(app);
    pModel = &app->model.gfx;
    plm_update_dynamic_range(app->plm.gfx, pModel->info.dynrng, app->model.out.info.dynrng);
    pModel->plm = app_p_get_plm(app->plm.gfx);
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
    /* for now, always hard code graphics to SDR */
    pModel->info.dynrng = PlatformDynamicRange_eSdr;
    app_p_update_gfx_plm(app);
}

void app_p_update_vid_plm(AppHandle app)
{
    PlatformPictureModel * pModel;
    unsigned i;
    assert(app);
    for(i=0; i<app->streamCount; i++) {
        pModel = &app->model.vid[i];
        plm_update_dynamic_range(app->plm.vid[i], pModel->info.dynrng, app->model.out.info.dynrng);
        pModel->plm = app_p_get_plm(app->plm.vid[i]);
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
    app_p_update_vid_plm(app);
}

void app_p_reapply_plm(AppHandle app)
{
    unsigned i;
    platform_display_wait_for_display_settings_application(app->display);
    for(i=0; i<app->streamCount; i++) {
        plm_reapply(app->plm.vid[i]);
    }
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
    unsigned i;
    assert(app);
    for(i=0; i<app->streamCount; i++) {
        plm_next(app->plm.vid[i]);
        app->model.vid[i].plm = plm_get(app->plm.vid[i]);
        osd_update_vid_model(app->osd, &app->model.vid[i], i);
    }
}

void app_p_prev_video_setting(void * context, int param)
{
    AppHandle app = context;
    unsigned i;
    assert(app);
    for(i=0; i<app->streamCount; i++) {
        plm_prev(app->plm.vid[i]);
        app->model.vid[i].plm = plm_get(app->plm.vid[i]);
        osd_update_vid_model(app->osd, &app->model.vid[i], i);
    }
}

void app_p_next_graphics_setting(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    plm_next(app->plm.gfx);
    app->model.gfx.plm = plm_get(app->plm.gfx);
    osd_update_gfx_model(app->osd, &app->model.gfx);
}

void app_p_prev_graphics_setting(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    plm_prev(app->plm.gfx);
    app->model.gfx.plm = plm_get(app->plm.gfx);
    osd_update_gfx_model(app->osd, &app->model.gfx);
}

void app_p_next_stream(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    if(app->streamCount > 1) {
        printf("Function not available in mosaic mode\n");
        return;
    }
    stream_player_next(app->streamPlayer[0], false);
}

void app_p_prev_stream(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    if(app->streamCount > 1) {
        printf("Function not available in mosaic mode\n");
        return;
    }
    stream_player_prev(app->streamPlayer[0], false);
}

void app_p_toggle_guide(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    osd_toggle_guide_visibility(app->osd);
}

void app_p_toggle_osd(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    osd_toggle_visibility(app->osd);
}

void app_p_toggle_pause(void * context, int param)
{
    AppHandle app = context;
    unsigned i;
    assert(app);
    for(i=0; i<app->streamCount; i++) {
        stream_player_toggle_pause(app->streamPlayer[i]);
    }
}

void app_p_toggle_details(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    osd_toggle_details_mode(app->osd);
}

void app_p_toggle_pig(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    if(app->streamCount > 1) {
        printf("Cannot toggle pig in mosaic mode\n");
        return;
    }
    app_p_set_pig_mode(app, !app->pig);
    osd_toggle_pig_mode(app->osd);
    app_p_update_gfx_model(app);
}

void app_p_toggle_mosaic_layout(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    if(app->streamCount == 1) {
        printf("Cannot toggle mosaic layout in non-mosaic mode\n");
        return;
    }
    app->layout ^= 1;
    osd_toggle_mosaic_layout(app->osd);
    app_p_update_gfx_model(app);
}

void app_p_next_thumbnail(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    image_viewer_next(app->thumbnail);
}

void app_p_prev_thumbnail(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    image_viewer_prev(app->thumbnail);
}

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

void app_p_cycle_colorimetry(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    app->model.out.info.gamut = (app->model.out.info.gamut + 1) % PlatformColorimetry_eUnknown;
    app_p_update_out_model(app);
    app_p_reapply_plm(app);
}

void app_p_cycle_background(void * context, int param)
{
    AppHandle app = context;
    assert(app);
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

void app_p_run_command_shell(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    shell_run(app->shell);
}

void app_p_hotplug_occurred(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    fprintf(stdout, "app received hotplug; updating model\n");
    /*platform_display_print_hdmi_status(app->display);*/
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

void app_p_set_pig_mode(AppHandle app, bool pig)
{
    app->pig = pig;
}

void app_p_reset_osd(AppHandle app)
{
    unsigned i;
    platform_get_default_model(&app->model);
    for(i=0; i<app->streamCount; i++)
        osd_update_vid_model(app->osd, &app->model.vid[i], i);
    osd_update_gfx_model(app->osd, &app->model.gfx);
    osd_update_out_model(app->osd, &app->model.out);
    osd_update_rcv_model(app->osd, &app->model.rcv);
    osd_update_sel_model(app->osd, &app->model.sel);
}

void app_p_scenario_changed(void * context, const Scenario * pScenario)
{
    unsigned i;

    AppHandle app = context;
    assert(app);

    /* Switching from mosaic to non-non mosaic requires all videos to be stopped so that windows can be reconfigured. */
    if(app->streamCount != pScenario->streamCount) {
        for(i=0; i<app->streamCount; i++) {
            stream_player_stop(app->streamPlayer[i]);
        }
    }
    app_p_reset_osd(app);
    app->streamCount = pScenario->streamCount;
    app->layout = pScenario->layout;
    osd_set_mosaic_mode(app->osd, app->streamCount>1, app->layout);
    for(i=0; i<app->streamCount; i++) {
        stream_player_play_stream_by_url(app->streamPlayer[i], pScenario->streamPaths[i], app->streamCount>1, pScenario->forceRestart);
    }
    app->model.out.info.gamut = pScenario->gamut;
    if (!app->output.dynrngLock)
    {
        app->output.dynrng = pScenario->dynrng;
    }
    osd_set_visibility(app->osd, pScenario->osd);
    osd_set_guide_visibility(app->osd, pScenario->guide);
    osd_set_details_mode(app->osd, pScenario->details);
    printf("image: %s\n", pScenario->imagePath);
    image_viewer_view_image_by_path(app->thumbnail, pScenario->imagePath);
    image_viewer_view_image_by_path(app->background, pScenario->bgPath);
    if (app->streamCount == 1) app_p_set_pig_mode(app, pScenario->pig);
    if (app->streamCount == 1) osd_set_pig_mode(app->osd, app->pig);
    for(i=0; i<app->streamCount; i++)
    {
        if ((pScenario->streamPaths[i] && app->prevStreamPaths[i] && strcmp(pScenario->streamPaths[i], app->prevStreamPaths[i]))
            ||
            (!pScenario->streamPaths[i] && app->prevStreamPaths[i])
            ||
            (pScenario->streamPaths[i] && !app->prevStreamPaths[i]))
        {
            plm_set(app->plm.vid[i], -1); /* clear the currentSwitcher */
        }
        plm_set(app->plm.vid[i], pScenario->plm.vidIndex);
        app->prevStreamPaths[i] = set_string(app->prevStreamPaths[i], pScenario->streamPaths[i]);
    }
    plm_set(app->plm.gfx, pScenario->plm.gfxIndex);
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

void app_run(AppHandle app)
{
    assert(app);
    platform_receiver_start(app->rx);
    if (app->args->runMode == ARGS_RunMode_eTest)
    {
        /* test means SQA invocation and will load reset scenario by default */
        scenario_player_play_scenario(app->scenarioPlayer, SCENARIO_PLAYER_RESET);
    }
    else
    {
        /* otherwise demo mode and loads first scenario by default */
        scenario_player_play_scenario(app->scenarioPlayer, 1);
    }
    osd_flip(app->osd);

    app_p_print_remote_usage(app);

    while (!app->done)
    {
        if (platform_input_try(app->input))
        {
            osd_flip(app->osd);
        }
    }
}

static AppHandle gSigApp = NULL;

static void sighandler(int signum, siginfo_t * info, void * ctx)
{
    AppHandle app = gSigApp;
    printf("Received signal %d\n", signum);
    switch (signum)
    {
        case SIGINT:
        case SIGTERM:
            app_destroy(app);
            break;
        default:
            break;
    }
}

static const char * STR_THUMBNAIL = "thumbnail";
static const char * STR_BACKGROUND = "background";

AppHandle app_create(ArgsHandle args)
{
    static const char * fontPath = "nxclient/arial_18_aa.bwin_font";
    AppHandle app;
    OsdTheme theme;
    ImageViewerCreateSettings viewerCreateSettings;
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
    app->platform = platform_open(args->name);
    if (!app->platform) goto error;
    platform_get_default_model(&app->model);
    app->model.out.info.gamut = PlatformColorimetry_eAuto;
    app->gfx = platform_graphics_open(app->platform, fontPath, app->args->osd.dims.width, app->args->osd.dims.height);
    if (!app->gfx) goto error;
    app->display = platform_display_open(app->platform);
    if (!app->display) goto error;
    app->input = platform_input_open(app->platform, app->args->method);
    if (!app->input) goto error;
    app->shell = shell_create();
    if (!app->shell) goto error;
    app_p_init_shell(app);
    theme.textForegroundColor = app->args->osd.colors.textFg;
    theme.mainPanelBackgroundColor = app->args->osd.colors.mainPanelBg;
    theme.guidePanelBackgroundColor = app->args->osd.colors.guidePanelBg;
    theme.detailsPanelBackgroundColor = app->args->osd.colors.detailsPanelBg;
    theme.infoPanelBackgroundColor = app->args->osd.colors.infoPanelBg;
    app->osd = osd_create(app->platform, app->gfx, &theme);
    if (!app->osd) goto error;
    for(i=0; i<MAX_MOSAICS; i++) {
        app->streamPlayer[i] = stream_player_create(app->platform, &app_p_stream_info_changed, app);
        if (!app->streamPlayer[i]) goto error;
    }
    app->scenarioPlayer = scenario_player_create(app->args->scenarioPath, &app_p_scenario_changed, app);
    if (!app->scenarioPlayer) goto error;
    for(i=0; i<MAX_MOSAICS; i++) {
        char buffer [16];
        snprintf(buffer, 16, "%s %d", "VID", i);
        app->plm.vid[i] = plm_create(buffer,
                args->vidSdr2HdrPath,
                args->vidHdr2SdrPath,
                args->vidHlg2HdrPath, app->platform,
                0,
                i,
                &platform_plm_set_vid_point,
                &platform_plm_get_vid_point,
                &platform_plm_set_vid_lra,
                &platform_plm_get_vid_lra);
        if (!app->plm.vid[i]) goto error;
    }
    app->plm.gfx = plm_create("GFX",
            args->gfxSdr2HdrPath,
            NULL,
            NULL,
            NULL,
            0,
            0,
            &platform_plm_set_gfx_point,
            &platform_plm_get_gfx_point,
            &platform_plm_set_gfx_lra,
            &platform_plm_get_gfx_lra);
    if (!app->plm.gfx) goto error;
    image_viewer_get_default_create_settings(&viewerCreateSettings);
    viewerCreateSettings.platform = app->platform;
    viewerCreateSettings.name = STR_THUMBNAIL;
    viewerCreateSettings.path = args->sdrThumbnailPath;
    viewerCreateSettings.pictureChanged.callback = &app_p_thumbnail_changed;
    viewerCreateSettings.pictureChanged.context = app;
    app->thumbnail = image_viewer_create(&viewerCreateSettings);
    if (!app->thumbnail) goto error;
    image_viewer_get_default_create_settings(&viewerCreateSettings);
    viewerCreateSettings.platform = app->platform;
    viewerCreateSettings.name = STR_BACKGROUND;
    viewerCreateSettings.path = args->sdrBackgroundPath;
    viewerCreateSettings.pictureChanged.callback = &app_p_background_changed;
    viewerCreateSettings.pictureChanged.context = app;
    app->background = image_viewer_create(&viewerCreateSettings);
    if (!app->background) goto error;
    app->rx = platform_receiver_open(app->platform, &app_p_hotplug_occurred, app);
    if (!app->rx) goto error;

    for(i=0; i<MAX_MOSAICS; i++) {
        stream_player_add_stream_source(app->streamPlayer[i], "SDR", args->sdrStreamPath);
        stream_player_add_stream_source(app->streamPlayer[i], "HDR", args->hdrStreamPath);
        stream_player_add_stream_source(app->streamPlayer[i], "HLG", args->hlgStreamPath);
        stream_player_add_stream_source(app->streamPlayer[i], "DVS", args->dvsStreamPath);
        stream_player_add_stream_source(app->streamPlayer[i], "MIX", args->mixStreamPath);
    }

    platform_input_set_event_handler(app->input, PlatformInputEvent_eToggleOutputDynamicRangeLock, &app_p_toggle_output_dynamic_range_lock, app);
    platform_input_set_event_handler(app->input, PlatformInputEvent_eCycleOutputDynamicRange, &app_p_cycle_output_dynamic_range, app);
    platform_input_set_event_handler(app->input, PlatformInputEvent_eScenario, &app_p_run_scenario, app);

    if (app->args->advanced)
    {
        platform_input_set_event_handler(app->input, PlatformInputEvent_eQuit, &app_p_quit, app);
        platform_input_set_event_handler(app->input, PlatformInputEvent_eToggleOsd, &app_p_toggle_osd, app);
        platform_input_set_event_handler(app->input, PlatformInputEvent_eToggleGuide, &app_p_toggle_guide, app);
        platform_input_set_event_handler(app->input, PlatformInputEvent_eTogglePause, &app_p_toggle_pause, app);
        platform_input_set_event_handler(app->input, PlatformInputEvent_eToggleDetails, &app_p_toggle_details, app);
        platform_input_set_event_handler(app->input, PlatformInputEvent_eTogglePig, &app_p_toggle_pig, app);
        platform_input_set_event_handler(app->input, PlatformInputEvent_eToggleMosaicLayout, &app_p_toggle_mosaic_layout, app);
        platform_input_set_event_handler(app->input, PlatformInputEvent_eCycleColorimetry, &app_p_cycle_colorimetry, app);
        platform_input_set_event_handler(app->input, PlatformInputEvent_eCycleBackground, &app_p_cycle_background, app);
        platform_input_set_event_handler(app->input, PlatformInputEvent_eNextThumbnail, &app_p_next_thumbnail, app);
        platform_input_set_event_handler(app->input, PlatformInputEvent_ePrevThumbnail, &app_p_prev_thumbnail, app);
        platform_input_set_event_handler(app->input, PlatformInputEvent_eNextVideoSetting, &app_p_next_video_setting, app);
        platform_input_set_event_handler(app->input, PlatformInputEvent_ePrevVideoSetting, &app_p_prev_video_setting, app);
        platform_input_set_event_handler(app->input, PlatformInputEvent_eNextGraphicsSetting, &app_p_next_graphics_setting, app);
        platform_input_set_event_handler(app->input, PlatformInputEvent_ePrevGraphicsSetting, &app_p_prev_graphics_setting, app);
        platform_input_set_event_handler(app->input, PlatformInputEvent_eNextStream, &app_p_next_stream, app);
        platform_input_set_event_handler(app->input, PlatformInputEvent_ePrevStream, &app_p_prev_stream, app);
        platform_input_set_event_handler(app->input, PlatformInputEvent_eStartCommandShell, &app_p_run_command_shell, app);
    }

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
    platform_receiver_close(app->rx);
    app->rx = NULL;
    image_viewer_destroy(app->background);
    app->background = NULL;
    image_viewer_destroy(app->thumbnail);
    app->thumbnail = NULL;
    plm_destroy(app->plm.gfx);
    app->plm.gfx = NULL;
    for(i=0; i<MAX_MOSAICS; i++) {
        plm_destroy(app->plm.vid[i]);
        app->plm.vid[i] = NULL;
    }
    scenario_player_destroy(app->scenarioPlayer);
    app->scenarioPlayer = NULL;
    for(i=0; i<MAX_MOSAICS; i++) {
        stream_player_destroy(app->streamPlayer[i]);
        app->streamPlayer[i] = NULL;
        if (app->prevStreamPaths[i]) free(app->prevStreamPaths[i]);
    }
    osd_destroy(app->osd);
    app->osd = NULL;
    shell_destroy(app->shell);
    app->shell = NULL;
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
