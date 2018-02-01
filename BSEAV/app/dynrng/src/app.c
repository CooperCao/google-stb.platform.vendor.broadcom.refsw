/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "util.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#define PRINT_PICTURE_INFO 0

static int app_p_osd_resized(AppHandle app, unsigned width, unsigned height);
static void app_p_usage_protection(AppHandle app);

PlatformDynamicRange app_p_compute_output_dynamic_range(AppHandle app, PlatformDynamicRange requested)
{
    PlatformDynamicRange output = PlatformDynamicRange_eSdr;

    assert(app);

    if (requested == PlatformDynamicRange_eAuto)
    {
        if (app->cfg->dbvOutputModeAutoSelection && platform_hdmi_receiver_supports_dynamic_range(app->platform.rx, PlatformDynamicRange_eDolbyVision) == PlatformCapability_eSupported)
        {
            output = PlatformDynamicRange_eDolbyVision;
        }
        else if (platform_hdmi_receiver_supports_dynamic_range(app->platform.rx, PlatformDynamicRange_eHdr10) == PlatformCapability_eSupported)
        {
            output = PlatformDynamicRange_eHdr10;
        }
        else if (platform_hdmi_receiver_supports_dynamic_range(app->platform.rx, PlatformDynamicRange_eHlg) == PlatformCapability_eSupported)
        {
            output = PlatformDynamicRange_eHlg;
        }
    }
    else if (requested <= PlatformDynamicRange_eInvalid)
    {
        output = requested;
    }

    fprintf(stdout, "Computed '%s' as output dynrng\n", platform_get_dynamic_range_name(output));

    return output;
}

static void app_p_compute_osd_dims(unsigned formatHeight, unsigned * osdWidth, unsigned * osdHeight)
{
    if (formatHeight >= 1080)
    {
        *osdWidth = 1920;
        *osdHeight = 1080;
    }
    else
    {
        switch (formatHeight)
        {
            case 720:
                *osdWidth = 1280;
                *osdHeight = 720;
                break;
            default:
            case 480:
                *osdWidth = 720;
                *osdHeight = 480;
                break;
        }
    }
}

#if PRINT_PICTURE_INFO
#define BUF_LEN 128
static void app_p_print_picture_info(const char * tag, const PlatformPictureInfo * pInfo)
{
    char buf[BUF_LEN];
    assert(pInfo);
    platform_print_picture_info(tag, pInfo, buf, BUF_LEN);
    printf("%s\n", buf);
}

static void app_p_print_out_picture_info(AppHandle app)
{
    PlatformPictureInfo info;

    assert(app);

    memcpy(&info, &app->platform.model.out.info, sizeof(info));

    if (info.gamut == PlatformColorimetry_eAuto)
    {
        info.gamut = app->platform.model.vid[0].info.gamut;
    }

    if (info.depth == 0)
    {
        info.depth = -1; /* get from hdmi status */
    }

    if (info.sampling == 0)
    {
        info.sampling = -1; /* get from hdmi status */
    }

    if (info.space == PlatformColorSpace_eAuto)
    {
        info.space = PlatformColorSpace_eMax; /* get from hdmi status */
    }

    app_p_print_picture_info("out", &info);
}
#endif

static void app_p_format_change_osd_resize_check(AppHandle app)
{
    PlatformPictureInfo * pRequestedInfo;
    PlatformPictureInfo * pModelInfo;

    pRequestedInfo = &app->platform.output.pictureInfo;
    pModelInfo = &app->platform.model.out.info;

    /* determine if we need to resize the OSD based on the requested format */
    if (pRequestedInfo->format.height != pModelInfo->format.height)
    {
        unsigned oldOsdHeight;
        unsigned oldOsdWidth;
        unsigned newOsdHeight;
        unsigned newOsdWidth;
        app_p_compute_osd_dims(pModelInfo->format.height, &oldOsdWidth, &oldOsdHeight);
        app_p_compute_osd_dims(pRequestedInfo->format.height, &newOsdWidth, &newOsdHeight);
        if (newOsdHeight != oldOsdHeight)
        {
            app_p_osd_resized(app, newOsdWidth, newOsdHeight);
        }
    }
}

void app_p_update_out_model(AppHandle app)
{
    PlatformPictureModel * pModel;
    PlatformPictureInfo localInfo;
    PlatformPictureInfo * pRequestedInfo;
    PlatformPictureInfo * pModelInfo;

    assert(app);
    pModel = &app->platform.model.out;
    pRequestedInfo = &app->platform.output.pictureInfo;
    pModelInfo = &pModel->info;

    /* copy user requested info to local */
    memcpy(&localInfo, pRequestedInfo, sizeof(localInfo));

    /* override user requested dynrng if auto */
    localInfo.dynrng = app_p_compute_output_dynamic_range(app, pRequestedInfo->dynrng);

    if (memcmp(&localInfo, pModelInfo, sizeof(*pModelInfo))) /* compare local info with model */
    {
        /* if local override info does not match model info, try to apply local override to hardware */
        /* Making sure this only happens on change avoids reset of the luma settings done by VDC */
        platform_display_set_picture_info(app->platform.display, &localInfo);
    }
    /* copy from hardware to model */
    platform_display_get_picture_info(app->platform.display, pModelInfo);
#if PRINT_PICTURE_INFO
    app_p_print_out_picture_info(app);
#endif
    app_p_update_gfx_processing_model(app);
    app_p_update_vid_processing_model(app);
    osd_update_out_model(app->osd.handle, pModel);
}

static void app_p_resolve_picture_info(AppHandle app, const PlatformPictureInfo * pNewInfo)
{
    PlatformPictureInfo * pCurrentInfo;

    assert(app);
    assert(pNewInfo);

    pCurrentInfo = &app->platform.output.pictureInfo;
    if (pNewInfo->dynrng != PlatformDynamicRange_eMax)
    {
        pCurrentInfo->dynrng = pNewInfo->dynrng;
    }
    if (pNewInfo->gamut != PlatformColorimetry_eMax)
    {
        pCurrentInfo->gamut = pNewInfo->gamut;
    }
    if (pNewInfo->space != PlatformColorSpace_eMax)
    {
        pCurrentInfo->space = pNewInfo->space;
    }
    if (pNewInfo->sampling != -1)
    {
        pCurrentInfo->sampling = pNewInfo->sampling;
    }
    if (pNewInfo->depth != -1)
    {
        pCurrentInfo->depth = pNewInfo->depth;
    }
    if (pNewInfo->format.height)
    {
        pCurrentInfo->format.height = pNewInfo->format.height;
        pCurrentInfo->format.interlaced = pNewInfo->format.interlaced;
    }
    if (pNewInfo->format.width)
    {
        pCurrentInfo->format.width = pNewInfo->format.width;
    }
    if (pNewInfo->format.rate)
    {
        pCurrentInfo->format.rate = pNewInfo->format.rate;
        pCurrentInfo->format.dropFrame = pNewInfo->format.dropFrame;
    }
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
    pModel = &app->platform.model.gfx;

    pModel->processing = app_p_get_processing(app->platform.processing.gfx.mode,
            pModel->info.dynrng, app->platform.model.out.info.dynrng);

    osd_update_gfx_model(app->osd.handle, pModel);
}

void app_p_update_gfx_model(AppHandle app)
{
    PlatformPictureModel * pModel;
    const PlatformPictureInfo * pInfo;
    assert(app);
    pModel = &app->platform.model.gfx;
    pInfo = image_viewer_get_picture_info(app->platform.usageMode
            == PlatformUsageMode_ePictureInGraphics
            ?
            app->image.background
            :
            app->image.thumbnail);
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

    platform_display_set_picture_quality(app->platform.display, &app->platform.processing.gfx.picCtrl);
}

void app_p_update_vid_processing_model(AppHandle app)
{
    PlatformPictureModel * pModel;
    unsigned i;
    assert(app);
    for(i=0; i<app->stream.count; i++) {
        pModel = &app->platform.model.vid[i];
        pModel->processing = app_p_get_processing(app->platform.processing.vid.mode, pModel->info.dynrng, app->platform.model.out.info.dynrng);
        osd_update_vid_model(app->osd.handle, pModel, i);
    }
    app_p_usage_protection(app);
}

void app_p_update_vid_model(AppHandle app)
{
    PlatformPictureModel * pModel;
    const PlatformPictureInfo * pInfo;
    unsigned i;
    assert(app);
    for(i=0; i<app->stream.count; i++) {
        pModel = &app->platform.model.vid[i];
        pInfo = stream_player_get_picture_info(app->stream.players[i]);
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

    switch (app->platform.usageMode)
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
        platform_display_get_video_dynamic_range_processing_settings(app->platform.display, w, &settings);
        /* turn off all processing, since it will affect when we switch input/output later */
        for (i = 0; i < PlatformDynamicRangeProcessingType_eMax; i++)
        {
            settings.modes[i] = app->platform.processing.vid.mode;
        }
        platform_display_set_video_dynamic_range_processing_settings(app->platform.display, w, &settings);
        platform_display_set_video_target_peak_brightness(app->platform.display, w, app->platform.processing.vid.hdrPeakBrightness, app->platform.processing.vid.sdrPeakBrightness);
    }
    app_p_update_vid_processing_model(app);
}

void app_p_apply_gfx_processing(AppHandle app)
{
    PlatformDynamicRangeProcessingSettings settings;
    unsigned i;
    assert(app);
    platform_display_get_graphics_dynamic_range_processing_settings(app->platform.display, &settings);
    /* turn off all processing, since it will affect when we switch input/output later */
    for (i = 0; i < PlatformDynamicRangeProcessingType_eMax; i++)
    {
        settings.modes[i] = app->platform.processing.gfx.mode;
    }
    platform_display_set_graphics_dynamic_range_processing_settings(app->platform.display, &settings);
    app_p_update_gfx_processing_model(app);
}

void app_p_toggle_pause(AppHandle app)
{
    unsigned i;
    assert(app);
    for(i=0; i<app->stream.count; i++) {
        stream_player_toggle_pause(app->stream.players[i]);
    }
}

void app_p_toggle_processing(AppHandle app)
{
    app_p_toggle_vid_processing(app);
    app_p_toggle_gfx_processing(app);
}

void app_p_toggle_vid_processing(AppHandle app)
{
    assert(app);
    switch (app->platform.processing.vid.mode)
    {
        default:
        case PlatformDynamicRangeProcessingMode_eAuto:
            app->platform.processing.vid.mode = PlatformDynamicRangeProcessingMode_eOff;
            break;
        case PlatformDynamicRangeProcessingMode_eOff:
            app->platform.processing.vid.mode = PlatformDynamicRangeProcessingMode_eAuto;
            break;
    }
    app_p_apply_vid_processing(app);
}

void app_p_toggle_gfx_processing(AppHandle app)
{
    assert(app);
    switch (app->platform.processing.gfx.mode)
    {
        default:
        case PlatformDynamicRangeProcessingMode_eAuto:
            app->platform.processing.gfx.mode = PlatformDynamicRangeProcessingMode_eOff;
            break;
        case PlatformDynamicRangeProcessingMode_eOff:
            app->platform.processing.gfx.mode = PlatformDynamicRangeProcessingMode_eAuto;
            break;
    }
    app_p_apply_gfx_processing(app);
}

#if 0
void app_p_toggle_output_dynamic_range_lock(AppHandle app)
{
    assert(app);
    app->platform.output.dynrngLock = !app->platform.output.dynrngLock;
    /* TODO: show some text on OSD? */
}

void app_p_cycle_output_dynamic_range(AppHandle app)
{
    assert(app);
    if (++app->platform.output.dynrng >= PlatformDynamicRange_eInvalid)
    {
        app->platform.output.dynrng = PlatformDynamicRange_eSdr; /* skip eAuto */
    }
    app_p_update_sel_model(app);
    app_p_update_out_model(app);
    app_p_reapply_plm(app);
}
#endif

#define MAX_SCENARIO_NUMBER_LEN 11
int app_p_run_scenario_by_number(AppHandle app, int scenarioNum)
{
    char name[MAX_SCENARIO_NUMBER_LEN];
    assert(app);
    snprintf(name, MAX_SCENARIO_NUMBER_LEN, "%d", scenarioNum);
    return app_p_run_scenario(app, name);
}

int app_p_run_scenario(AppHandle app, const char * scenarioName)
{
    assert(app);
    app->scenario.name = set_string(app->scenario.name, scenarioName);
    return scenario_player_play_scenario(app->scenario.player, app->scenario.name);
}

void app_p_quit(AppHandle app)
{
    assert(app);
    ui_stop(app);
#if DYNRNG_HAS_CONSOLE
    if (app->console.handle)
    {
        console_stop(app->console.handle);
    }
#endif
}

#if DYNRNG_DBV_CONFORMANCE_MODE
static void app_p_abort_scenario(AppHandle app)
{
    unsigned i;
    scenario_player_abort(app->scenario.player);
#if DYNRNG_HAS_CAPTURE
    capture_abort(app->capture.handle);
#endif
    for(i=0; i<app->stream.count; i++) {
        stream_player_stop(app->stream.players[i]);
    }
    osd_update_background(app->osd.handle, NULL);
    osd_update_thumbnail(app->osd.handle, NULL);
}
#endif

static void app_p_usage_protection(AppHandle app)
{
#if DYNRNG_DBV_CONFORMANCE_MODE
    if
    (
        (
            (
                app->platform.model.vid[0].info.dynrng != PlatformDynamicRange_eDolbyVision
                &&
                app->platform.model.vid[0].info.dynrng != PlatformDynamicRange_eUnknown
            )
            ||
            app->platform.model.vid[0].processing == PlatformTriState_eOff
        )
        &&
        (
            app->platform.model.out.info.dynrng != PlatformDynamicRange_eDolbyVision
            &&
            app->platform.model.out.info.dynrng != PlatformDynamicRange_eUnknown
        )
    )
    {
        fprintf(stdout, "blocked usage mode\n");
        app_p_abort_scenario(app);
    }
#else
    (void)app;
#endif
}

void app_p_hotplug_occurred(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    (void)param;
    fprintf(stdout, "app received hotplug; updating model\n");
    app_p_update_model(app);
    osd_flip(app->osd.handle);
}

void app_p_stream_info_changed(void * context, int param)
{
    AppHandle app = context;
    assert(app);
    fprintf(stdout, "app received stream(%d) info event: updating model\n", param);
    app_p_update_model(app);
    osd_flip(app->osd.handle);
}

void app_p_reset_osd(AppHandle app)
{
    PlatformModel model;
    unsigned i;
    platform_get_default_model(&model);
    /* only reset video and gfx models */
    memcpy(&app->platform.model.gfx, &model.gfx, sizeof(model.gfx));
    memcpy(&app->platform.model.vid, &model.vid, sizeof(model.vid));
    for(i=0; i<app->stream.count; i++)
        osd_update_vid_model(app->osd.handle, &model.vid[i], i);
    osd_update_gfx_model(app->osd.handle, &model.gfx);
    osd_update_out_model(app->osd.handle, &model.out);
}

static const char * const usageModeStrings[] =
{
    "full-screen video",
    "pig",
    "mosaic",
    "pip",
    NULL
};

static int app_p_unrecognized_scenario_syntax(void * context, const char * name, const char * value)
{
    AppHandle app = context;
#if DYNRNG_HAS_CAPTURE
    if (app->capture.handle)
    {
        capture_handle_scenario_nvp(app->capture.handle, name, value);
    }
#else
    (void)app;
#endif
#if DYNRNG_HAS_TESTER
    if (app->tester.handle)
    {
        tester_handle_scenario_nvp(app->tester.handle, name, value);
    }
#endif
#if DYNRNG_HAS_PQ
    if (app->pq.handle)
    {
        pq_handle_scenario_nvp(app->pq.handle, name, value);
    }
#else
    (void)app;
#endif
    return 0; /* don't care if unrecognized syntax, try running anyway */
}

#define min(x,y) (((x) > (y)) ? (y) : (x))
void app_p_scenario_changed(void * context, const Scenario * pScenario)
{
    unsigned i;
    AppHandle app = context;
    StreamPlayerPlaySettings playSettings;
    PlatformMediaPlayerSettings mpSettings;
    Scenario tmp;

    assert(app);

#if DYNRNG_DBV_CONFORMANCE_MODE
    if (pScenario->usageMode == PlatformUsageMode_eMosaic || pScenario->usageMode == PlatformUsageMode_eMainPip)
    {
        memcpy(&tmp, pScenario, sizeof(tmp));
        tmp.usageMode = app->platform.usageMode;
        pScenario = &tmp;
    }
#else
    (void)tmp;
#endif
    /* Switching from mosaic to non-non mosaic requires all videos to be stopped so that windows can be reconfigured. */
    if (app->platform.usageMode != pScenario->usageMode || (app->osd.layout != pScenario->layout && pScenario->usageMode == PlatformUsageMode_eMosaic)) {
        if (app->platform.usageMode == PlatformUsageMode_eMosaic || pScenario->usageMode == PlatformUsageMode_eMosaic) {
            for(i=0; i<app->stream.count; i++) {
                stream_player_stop(app->stream.players[i]);
            }
        }
        else if (app->platform.usageMode == PlatformUsageMode_eMainPip) {
            stream_player_stop(app->stream.players[1]);
        }
    }
    /* copy user requested pic info from scenario if specified, application to hardware happens in update model call at end */
    app_p_resolve_picture_info(app, &pScenario->pictureInfo);

    /* format change osd resize check must happen here in case it requires complete recreate of osd */
    app_p_format_change_osd_resize_check(app);

    app_p_reset_osd(app);

    app->platform.usageMode = pScenario->usageMode;
    printf("usage mode: %s\n", usageModeStrings[app->platform.usageMode]);
    switch (app->platform.usageMode)
    {
        case PlatformUsageMode_eMosaic:
            app->stream.count = min(pScenario->streamCount, app->platform.mosaicCount);
            break;
        case PlatformUsageMode_eMainPip:
            app->stream.count = min(pScenario->streamCount, 2);
            break;
        default:
            app->stream.count = min(pScenario->streamCount, 1);
            break;
    }
    printf("stream count: %u\n", app->stream.count);
    app->osd.layout = pScenario->layout;
    osd_set_usage_mode(app->osd.handle, app->platform.usageMode, app->osd.layout);
    for(i=0; i<app->stream.count; i++) {
        stream_player_get_platform_settings(app->stream.players[i], &mpSettings);
        mpSettings.usageMode = app->platform.usageMode;
        (void)stream_player_set_platform_settings(app->stream.players[i], &mpSettings);
        stream_player_get_default_play_settings(&playSettings);
        playSettings.streamUrl = pScenario->streamPaths[i];
        playSettings.playMode = pScenario->playMode;
        playSettings.forceRestart = pScenario->forceRestart;
        playSettings.startPaused = pScenario->startPaused;
        playSettings.stcTrick = pScenario->stcTrick;
        stream_player_play_stream(app->stream.players[i], &playSettings);
    }
    osd_set_visibility(app->osd.handle, pScenario->osd);
    osd_set_info_visibility(app->osd.handle, pScenario->info);
    printf("image: %s\n", pScenario->imagePath);
    image_viewer_view_image(app->image.thumbnail, pScenario->imagePath);
    printf("bg: %s\n", pScenario->bgPath);
    image_viewer_view_image(app->image.background, pScenario->bgPath);
    for(i=0; i<app->stream.count; i++)
    {
        app->stream.prevPaths[i] = set_string(app->stream.prevPaths[i], pScenario->streamPaths[i]);
    }
    app->platform.processing.vid.mode = pScenario->processing.vid;
    app->platform.processing.gfx.mode = pScenario->processing.gfx;
    platform_display_set_gfx_luminance(app->platform.display, pScenario->gfxLuminance.min, pScenario->gfxLuminance.max);
    app_p_apply_vid_processing(app);
    app_p_apply_gfx_processing(app);
    app_p_update_model(app);
}

void app_p_thumbnail_changed(void * context, PlatformPictureHandle pic)
{
    AppHandle app = context;
    assert(app);
    osd_update_thumbnail(app->osd.handle, pic);
    app_p_update_gfx_model(app);
}

void app_p_background_changed(void * context, PlatformPictureHandle pic)
{
    AppHandle app = context;
    assert(app);
    osd_update_background(app->osd.handle, pic);
    app_p_update_gfx_model(app);
}

void ui_run(AppHandle app)
{
    assert(app);
    while (!app->ui.done)
    {
        if (platform_input_try(app->platform.input))
        {
            osd_flip(app->osd.handle);
        }
    }
}

static void * ui_thread(void * context)
{
    AppHandle app = context;
    assert(app);
    ui_run(app);
    return NULL;
}

int ui_start(AppHandle app)
{
    assert(app);
    return pthread_create(&app->ui.thread, NULL, &ui_thread, app);
}

void ui_stop(AppHandle app)
{
    assert(app);
    app->ui.done = true;
}

void ui_wait(AppHandle app)
{
    assert(app);
    if (app->ui.thread)
    {
        pthread_join(app->ui.thread, NULL);
    }
}

void app_run(AppHandle app)
{
    assert(app);

    platform_scheduler_start(platform_get_scheduler(app->platform.handle));
    platform_hdmi_receiver_start(app->platform.rx);
    app_p_run_scenario(app, app->args->scenario);
    osd_flip(app->osd.handle);

#if DYNRNG_HAS_CONSOLE
    if (app->console.handle)
    {
        if (console_start(app->console.handle))
        {
            printf("unable to start console. Continuing without.\n");
        }
    }
#endif

    /* note: weird threading is to ensure signal handler can cancel the ui thread */
    if (ui_start(app))
    {
        printf(("Unable to start ui"));
        assert(0);
    }

    ui_wait(app);

#if DYNRNG_HAS_CONSOLE
    if (app->console.handle)
    {
        console_stop(app->console.handle);
    }
#endif
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
            ui_stop(app);
            ui_wait(app);
#if DYNRNG_HAS_CONSOLE
            if (app->console.handle)
            {
                console_stop(app->console.handle);
            }
#endif
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

static int app_p_create_graphics(AppHandle app, unsigned width, unsigned height)
{
    int rc = 0;
    unsigned i;
    static const char * fontPath = "nxclient/arial_18_aa.bwin_font";
    OsdCreateSettings osdCreateSettings;

    app->platform.gfx = platform_graphics_open(app->platform.handle, fontPath, width, height);
    if (!app->platform.gfx) { rc = -1; goto error; }

    app->platform.mosaicCount = platform_graphics_get_max_mosaic_count(app->platform.gfx);

    osd_get_default_create_settings(&osdCreateSettings);
    osdCreateSettings.theme.textForegroundColor = app->cfg->osd.colors.textFg;
    osdCreateSettings.theme.mainPanelBackgroundColor = app->cfg->osd.colors.mainPanelBg;
    osdCreateSettings.theme.infoPanelBackgroundColor = app->cfg->osd.colors.infoPanelBg;
    osdCreateSettings.platform = app->platform.handle;
    osdCreateSettings.gfx = app->platform.gfx;
    app->osd.handle = osd_create(&osdCreateSettings);
    if (!app->osd.handle) { rc = -1; goto error; }

    for(i=0; i < app->platform.mosaicCount; i++) {
        app->platform.mediaPlayers[i] = platform_media_player_create(app->platform.handle, &app_p_stream_info_changed, app);
        if (!app->platform.mediaPlayers[i]) { rc = -1; goto error; }
        app->stream.players[i] = stream_player_create(app->platform.mediaPlayers[i]);
        if (!app->stream.players[i]) { rc = -1; goto error; }
        stream_player_add_stream_source(app->stream.players[i], app->stream.filer);
    }

error:
    return rc;
}

static void app_p_destroy_graphics(AppHandle app)
{
    unsigned i;
    for(i=0; i < app->platform.mosaicCount; i++) {
        if (app->stream.players[i]) stream_player_destroy(app->stream.players[i]);
        app->stream.players[i] = NULL;
        if (app->stream.prevPaths[i]) free(app->stream.prevPaths[i]);
        app->stream.prevPaths[i] = NULL;
        if (app->platform.mediaPlayers[i]) platform_media_player_destroy(app->platform.mediaPlayers[i]);
        app->platform.mediaPlayers[i] = NULL;
    }

    osd_destroy(app->osd.handle);
    app->osd.handle = NULL;
    platform_graphics_close(app->platform.gfx);
    app->platform.gfx = NULL;
}

static int app_p_osd_resized(AppHandle app, unsigned width, unsigned height)
{
    int rc = 0;
    printf("osd resize %ux%u\n", width, height);
    platform_scheduler_stop(platform_get_scheduler(app->platform.handle));
    app_p_destroy_graphics(app);
    rc = app_p_create_graphics(app, width, height);
    platform_scheduler_start(platform_get_scheduler(app->platform.handle));
    return rc;
}

#if DYNRNG_HAS_CAPTURE
static void app_p_frame_advance_request_handler(void * context)
{
    AppHandle app = context;
    unsigned i;

    for(i=0; i<app->stream.count; i++) {
        stream_player_frame_advance(app->stream.players[i]);
    }
}
static void app_p_test_id_request_handler(void * context, char * pTestId, size_t testIdLen)
{
    AppHandle app = context;
    assert(pTestId);
    assert(app);
    if (strlen(app->scenario.name) < testIdLen)
    {
        strcpy(pTestId, app->scenario.name);
    }
}
static void app_p_capture_platform_settings_request_handler(void * context, const CapturePlatformSettings * pSettings)
{
    AppHandle app = context;
    PlatformMediaPlayerSettings mpSettings;
    PlatformPqSettings pqOffSettings;
    unsigned i;

    assert(pSettings);
    assert(app);

    /* todo: move all this to after a commit is signaled */
    memset(&pqOffSettings, 0, sizeof(pqOffSettings));
    for(i=0; i<app->stream.count; i++) {
        stream_player_get_platform_settings(app->stream.players[i], &mpSettings);
        if (pSettings->pqEnabled && memcmp(&app->platform.pq[i], &pqOffSettings, sizeof(app->platform.pq[i])))
        {
            memcpy(&mpSettings.pqSettings, &app->platform.pq[i], sizeof(mpSettings.pqSettings));
        }
        else if (!pSettings->pqEnabled)
        {
            memcpy(&app->platform.pq[i], &mpSettings.pqSettings, sizeof(app->platform.pq[i]));
            memcpy(&mpSettings.pqSettings, &pqOffSettings, sizeof(mpSettings.pqSettings));
        }
        (void)stream_player_set_platform_settings(app->stream.players[i], &mpSettings);
    }

    platform_display_set_rendering_priority(app->platform.display, pSettings->renderingPriority);
    app->platform.model.renderingPriority = pSettings->renderingPriority;

    memcpy(&app->platform.output.pictureInfo.ar, &pSettings->ar, sizeof(app->platform.output.pictureInfo.ar));
    app_p_update_out_model(app);
}
static int app_p_create_capture(AppHandle app, StringMapHandle cfgMap)
{
    CaptureCreateSettings captureCreateSettings;
    capture_get_default_create_settings(&captureCreateSettings);
    captureCreateSettings.cfgMap = cfgMap;
    captureCreateSettings.frameAdvanceRequest.callback = &app_p_frame_advance_request_handler;
    captureCreateSettings.frameAdvanceRequest.context = app;
    captureCreateSettings.testIdRequest.callback = &app_p_test_id_request_handler;
    captureCreateSettings.testIdRequest.context = app;
    captureCreateSettings.platformSettingsRequest.callback = &app_p_capture_platform_settings_request_handler;
    captureCreateSettings.platformSettingsRequest.context = app;
    app->capture.handle = capture_create(&captureCreateSettings);
    if (!app->capture.handle) return -ERR_DEPENDENCY;
    return 0;
}
#endif

void app_p_input_event_dispatcher(void * context, PlatformInputEvent event, int param)
{
    AppHandle app = context;
    assert(app);
    switch (event)
    {
        case PlatformInputEvent_ePower:
            app_p_quit(app);
            break;
        case PlatformInputEvent_eSelect:
            app_p_toggle_processing(app);
            break;
        case PlatformInputEvent_eUp:
            app_p_toggle_vid_processing(app);
            break;
        case PlatformInputEvent_eDown:
            app_p_toggle_vid_processing(app);
            break;
        case PlatformInputEvent_eLeft:
            app_p_toggle_gfx_processing(app);
            break;
        case PlatformInputEvent_eRight:
            app_p_toggle_gfx_processing(app);
            break;
        case PlatformInputEvent_eNumber:
            app_p_run_scenario_by_number(app, param);
            break;
        case PlatformInputEvent_ePause:
            app_p_toggle_pause(app);
            break;
        default:
            break;
    }
}

#if DYNRNG_HAS_CONSOLE
static int app_p_unrecognized_console_syntax(void * ctx, const char * line)
{
    AppHandle app = ctx;
    int result = 0;

    assert(app);
    printf("Attempting to run playlist '%s'\n", line);
    result = playlist_play_list(app->playlist.handle, line);
    if (result == -ERR_NOT_FOUND)
    {
        printf("Attempting to run scenario '%s'\n", line);
        result = app_p_run_scenario(app, line);
    }
    return result;
}
static int app_p_create_console(AppHandle app)
{
    ConsoleCreateSettings consoleCreateSettings;
    console_get_default_create_settings(&consoleCreateSettings);
    consoleCreateSettings.inputEvent.callback = &app_p_input_event_dispatcher;
    consoleCreateSettings.inputEvent.context = app;
    consoleCreateSettings.unrecognizedSyntax.callback = &app_p_unrecognized_console_syntax;
    consoleCreateSettings.unrecognizedSyntax.context = app;
    app->console.handle = console_create(&consoleCreateSettings);
    if (!app->console.handle) return -ERR_DEPENDENCY;
    return 0;
}
#endif

#if DYNRNG_HAS_PLAYLIST
static int app_p_play_scenario(void * ctx, const char * scenarioName)
{
    AppHandle app = ctx;
    assert(app);
    return app_p_run_scenario(app, scenarioName);
}
static void app_p_unrecognized_playlist_syntax(void * ctx, const char * name, const char * value)
{
    (void)ctx;
    printf("Unrecognized playlist syntax '%s = %s'\n", name, value);
}
static int app_p_create_playlist(AppHandle app, StringMapHandle cfgMap)
{
    PlaylistCreateSettings playlistCreateSettings;
    playlist_get_default_create_settings(&playlistCreateSettings);
    playlistCreateSettings.cfgMap = cfgMap;
    playlistCreateSettings.playScenarioRequest.callback = &app_p_play_scenario;
    playlistCreateSettings.playScenarioRequest.context = app;
    playlistCreateSettings.unrecognizedSyntax.callback = &app_p_unrecognized_playlist_syntax;
    playlistCreateSettings.unrecognizedSyntax.context = app;
    app->playlist.handle = playlist_create(&playlistCreateSettings);
    if (!app->playlist.handle) return -ERR_DEPENDENCY;
    return 0;
}
#endif

#if DYNRNG_HAS_TESTER
static const char * app_p_scenario_name_request_handler(void * ctx)
{
    AppHandle app = ctx;
    assert(app);
    return app->scenario.name;
}

static int app_p_create_tester(AppHandle app)
{
    int result = 0;
    TesterCreateSettings testerCreateSettings;
    assert(app);
    tester_get_default_create_settings(&testerCreateSettings);
    testerCreateSettings.scenarioNameRequest.callback = &app_p_scenario_name_request_handler;
    testerCreateSettings.scenarioNameRequest.context = app;
    app->tester.handle = tester_create(&testerCreateSettings);
    if (!app->tester.handle) { result = -ERR_DEPENDENCY; goto end; }
end:
    return result;
}
#endif

#if DYNRNG_HAS_PQ
static void app_p_pq_platform_settings_request_handler(void * context, const PqSettings * pSettings)
{
    AppHandle app = context;
    PlatformMediaPlayerSettings mpSettings;
    PlatformPqSettings pqOffSettings;
    unsigned i;

    assert(pSettings);
    assert(app);

    /* todo: move all this to after a commit is signaled */
    memset(&pqOffSettings, 0, sizeof(pqOffSettings));
    for(i=0; i<app->stream.count; i++) {
        stream_player_get_platform_settings(app->stream.players[i], &mpSettings);
        memcpy(&mpSettings.pqSettings.pictureCtrlSettings, &pSettings->vidPicCtrl, sizeof(mpSettings.pqSettings.pictureCtrlSettings));
        (void)stream_player_set_platform_settings(app->stream.players[i], &mpSettings);
    }

    app->platform.processing.vid.hdrPeakBrightness = pSettings->hdrPeakBrightness;
    app->platform.processing.vid.sdrPeakBrightness = pSettings->sdrPeakBrightness;
    app_p_apply_vid_processing(app);

    memcpy(&app->platform.processing.gfx.picCtrl, &pSettings->gfxPicCtrl, sizeof(app->platform.processing.gfx.picCtrl));
    app_p_update_gfx_model(app);
}

static int app_p_create_pq(AppHandle app)
{
    int result = 0;
    PqCreateSettings pqCreateSettings;
    assert(app);
    pq_get_default_create_settings(&pqCreateSettings);
    pqCreateSettings.pqSettingsRequest.callback = &app_p_pq_platform_settings_request_handler;
    pqCreateSettings.pqSettingsRequest.context = app;
    app->pq.handle = pq_create(&pqCreateSettings);
    if (!app->pq.handle) { result = -ERR_DEPENDENCY; goto end; }
end:
    return result;
}
#endif


AppHandle app_create(ArgsHandle args)
{
    StringMapHandle cfgMap;
    AppHandle app;
    ScenarioPlayerCreateSettings scenarioPlayerCreateSettings;
    ImageViewerCreateSettings imageViewerCreateSettings;
    FileManagerCreateSettings fileManagerCreateSettings;

    assert(args);

    app = malloc(sizeof(*app));
    if (!app) goto error;
    memset(app, 0, sizeof(*app));
    app->args = args;

    app->ui.term.sa_sigaction = &sighandler;
    app->ui.term.sa_flags = SA_SIGINFO;
    gSigApp = app;
    sigaction(SIGTERM, &app->ui.term, NULL);
    sigaction(SIGINT, &app->ui.term, NULL);

    cfgMap = string_map_deserialize(app->args->configPath);
    if (!cfgMap) goto error;

    app->cfg = config_create(cfgMap);
    if (!app->cfg) goto error;

    app->platform.handle = platform_open(args->name);
    if (!app->platform.handle) goto error;

    platform_get_default_model(&app->platform.model);
    app->platform.model.out.info.gamut = PlatformColorimetry_eAuto;

    app->platform.input = platform_input_open(app->platform.handle);
    if (!app->platform.input) goto error;

    app->platform.display = platform_display_open(app->platform.handle);
    if (!app->platform.display) goto error;

    platform_display_get_picture_info(app->platform.display, &app->platform.model.out.info);
    memcpy(&app->platform.output.pictureInfo, &app->platform.model.out.info, sizeof(app->platform.output.pictureInfo));

    file_manager_get_default_create_settings(&fileManagerCreateSettings);
    fileManagerCreateSettings.name = STR_STREAMS;
    fileManagerCreateSettings.path = app->cfg->streamsPath;
    fileManagerCreateSettings.filter = &stream_player_file_filter;
    app->stream.filer = file_manager_create(&fileManagerCreateSettings);
    if (!app->stream.filer) goto error;

    file_manager_get_default_create_settings(&fileManagerCreateSettings);
    fileManagerCreateSettings.name = STR_IMAGES;
    fileManagerCreateSettings.path = app->cfg->imagesPath;
    fileManagerCreateSettings.filter = &image_viewer_file_filter;
    app->image.filer = file_manager_create(&fileManagerCreateSettings);
    if (!app->image.filer) goto error;

    file_manager_get_default_create_settings(&fileManagerCreateSettings);
    fileManagerCreateSettings.name = STR_SCENARIOS;
    fileManagerCreateSettings.path = app->cfg->scenariosPath;
    fileManagerCreateSettings.filter = &scenario_player_file_filter;
    app->scenario.filer = file_manager_create(&fileManagerCreateSettings);
    if (!app->scenario.filer) goto error;

    scenario_player_get_default_create_settings(&scenarioPlayerCreateSettings);
    scenarioPlayerCreateSettings.filer = app->scenario.filer;
    scenarioPlayerCreateSettings.scenarioChanged.callback = &app_p_scenario_changed;
    scenarioPlayerCreateSettings.scenarioChanged.context = app;
    scenarioPlayerCreateSettings.unrecognizedSyntax.callback = &app_p_unrecognized_scenario_syntax;
    scenarioPlayerCreateSettings.unrecognizedSyntax.context = app;
    app->scenario.player = scenario_player_create(&scenarioPlayerCreateSettings);
    if (!app->scenario.player) goto error;

    if (app_p_create_graphics(app, app->cfg->osd.dims.width, app->cfg->osd.dims.height)) goto error;

    image_viewer_get_default_create_settings(&imageViewerCreateSettings);
    imageViewerCreateSettings.platform = app->platform.handle;
    imageViewerCreateSettings.name = STR_THUMBNAIL;
    imageViewerCreateSettings.filer = app->image.filer;
    imageViewerCreateSettings.pictureChanged.callback = &app_p_thumbnail_changed;
    imageViewerCreateSettings.pictureChanged.context = app;
    app->image.thumbnail = image_viewer_create(&imageViewerCreateSettings);
    if (!app->image.thumbnail) goto error;

    image_viewer_get_default_create_settings(&imageViewerCreateSettings);
    imageViewerCreateSettings.platform = app->platform.handle;
    imageViewerCreateSettings.name = STR_BACKGROUND;
    imageViewerCreateSettings.filer = app->image.filer;
    imageViewerCreateSettings.pictureChanged.callback = &app_p_background_changed;
    imageViewerCreateSettings.pictureChanged.context = app;
    app->image.background = image_viewer_create(&imageViewerCreateSettings);
    if (!app->image.background) goto error;

    app->platform.rx = platform_hdmi_receiver_open(app->platform.handle, &app_p_hotplug_occurred, app);
    if (!app->platform.rx) goto error;

#if !DYNRNG_DBV_CONFORMANCE_MODE
    platform_input_set_event_handler(app->platform.input, &app_p_input_event_dispatcher, app);
#endif

#if DYNRNG_HAS_CAPTURE
    if (app_p_create_capture(app, cfgMap)) printf("capture component creation failed; capture disabled\n");
#endif

#if DYNRNG_HAS_PLAYLIST
    if (app_p_create_playlist(app, cfgMap)) printf("playlist component creation failed; playlists disabled\n");
#endif

#if DYNRNG_HAS_TESTER
    if (app_p_create_tester(app)) printf("tester component creation failed; testing features disabled\n");
#endif

#if DYNRNG_HAS_CONSOLE
    if (app_p_create_console(app)) printf("console component creation failed; console disabled\n");
#endif

#if DYNRNG_HAS_PQ
    if (app_p_create_pq(app)) printf("pq component creation failed; pq disabled\n");
#endif

    return app;

error:
    app_destroy(app);
    return NULL;
}

void app_destroy(AppHandle app)
{
    if (!app) return;

    app_p_run_scenario(app, SCENARIO_PLAYER_EXIT_SCENARIO_NAME);
    osd_update_background(app->osd.handle, NULL);
    osd_update_thumbnail(app->osd.handle, NULL);

    platform_scheduler_stop(platform_get_scheduler(app->platform.handle));

#if DYNRNG_HAS_TESTER
    if (app->tester.handle)
    {
        tester_destroy(app->tester.handle);
        app->tester.handle = NULL;
    }
#endif

#if DYNRNG_HAS_CONSOLE
    if (app->console.handle)
    {
        console_destroy(app->console.handle);
        app->console.handle = NULL;
    }
#endif

#if DYNRNG_HAS_CAPTURE
    if (app->capture.handle)
    {
        capture_destroy(app->capture.handle);
        app->capture.handle = NULL;
    }
#endif

#if DYNRNG_HAS_PLAYLIST
    if (app->playlist.handle)
    {
        playlist_destroy(app->playlist.handle);
        app->playlist.handle = NULL;
    }
#endif

#if DYNRNG_HAS_PQ
    if (app->pq.handle)
    {
        pq_destroy(app->pq.handle);
        app->pq.handle = NULL;
    }
#endif

    platform_hdmi_receiver_close(app->platform.rx);
    app->platform.rx = NULL;
    image_viewer_destroy(app->image.background);
    app->image.background = NULL;
    image_viewer_destroy(app->image.thumbnail);
    app->image.thumbnail = NULL;
    app_p_destroy_graphics(app);
    scenario_player_destroy(app->scenario.player);
    app->scenario.player = NULL;
    platform_input_close(app->platform.input);
    app->platform.input = NULL;
    platform_display_close(app->platform.display);
    app->platform.display = NULL;
    platform_close(app->platform.handle);
    app->platform.handle = NULL;
    free(app);
}
