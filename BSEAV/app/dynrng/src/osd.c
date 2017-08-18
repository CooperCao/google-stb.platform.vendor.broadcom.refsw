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
#include "nxclient.h"
#include "picdecoder.h"
#include "osd.h"
#include "osd_priv.h"
#include "bwt.h"
#include "util_priv.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static unsigned osd_p_compute_external_dim(unsigned internalDim, unsigned padding)
{
    return BWT_RELATIVE_BASE * internalDim / (BWT_RELATIVE_BASE - 2 * padding);
}

#if USE_HEADERS
static const char * STR_DESC = "DESC";
static const char * STR_DYNRNG = "DYNRNG";
static const char * STR_PROC = "PROC";
#endif
static const char * STR_VID_LONG = "VIDEO IN";
static const char * STR_GFX_LONG = "GRAPHICS";
static const char * STR_OUT_LONG = "VIDEO OUT";
static const char * STR_INFO_PANEL_NAME = "info";
static const unsigned COLOR_RED_100 = 0xffff0000;
static const unsigned COLOR_GREEN_100 = 0xff00ff00;
static const char * STR_TABLE_PROC[] =
{
    "PROC OFF",
    "PROC ON",
    "PROC PASS",
    NULL
};

/*
 * Info Panel
 * |VIDEO IN |HDR10|PROC ON | # ON -> GREEN
 * |GRAPHICS |SDR  |PROC OFF| # OFF -> RED
 * |VIDEO OUT|HDR10|
 */
static BWT_LabelHandle osd_info_panel_p_populate_cell(OsdHandle osd, BWT_TableHandle table, BWT_WidgetHandle row, unsigned c, const char * text, BWT_HorizontalAlignment halign, BWT_VerticalAlignment valign)
{
    BWT_PanelHandle cell;
    BWT_LabelHandle label;
    BWT_LabelCreateSettings labelSettings;

    cell = (BWT_PanelHandle)BWT_Table_GetCell(table, row, c);
    BWT_Label_GetDefaultCreateSettings(&labelSettings);
    labelSettings.dims = *BWT_Widget_GetDimensions((BWT_WidgetHandle)cell);
    labelSettings.color = osd->createSettings.theme.textForegroundColor;
    labelSettings.text = text;
    labelSettings.halign = halign;
    labelSettings.valign = valign;
    label = BWT_Label_Create(osd->bwt, &labelSettings);
    assert(label);
    BWT_Panel_AddChild(cell, (BWT_WidgetHandle)label, BWT_VerticalAlignment_eDefault, BWT_HorizontalAlignment_eDefault);
    return label;
}

#if USE_HEADERS
static void osd_info_panel_p_populate_headers(OsdHandle osd, BWT_TableHandle table)
{
    BWT_WidgetHandle row;
    unsigned c;
    c = 0;

    row = BWT_Table_GetRow(table, 0);
    BWT_Widget_SetColor(row, COLOR_LIGHT_BLUE_20);
    osd_info_panel_p_populate_cell(osd, table, row, c, STR_DESC, BWT_HorizontalAlignment_eCenter, BWT_VerticalAlignment_eCenter);
    osd_info_panel_p_populate_cell(osd, table, row, ++c, STR_DYNRNG, BWT_HorizontalAlignment_eCenter, BWT_VerticalAlignment_eCenter);
    osd_info_panel_p_populate_cell(osd, table, row, ++c, STR_PROC, BWT_HorizontalAlignment_eCenter, BWT_VerticalAlignment_eCenter);
}
#endif

static void osd_info_panel_p_populate_row(OsdHandle osd, BWT_TableHandle table, OsdPictureInfoView * pView, unsigned r, const char * desc, bool processing)
{
    BWT_WidgetHandle row;
    unsigned c;
    c = 0;
    row = BWT_Table_GetRow(table, r);
    osd_info_panel_p_populate_cell(osd, table, row, c, desc, BWT_HorizontalAlignment_eLeft, BWT_VerticalAlignment_eCenter);
    pView->dynrng = osd_info_panel_p_populate_cell(osd, table, row, ++c, NULL, BWT_HorizontalAlignment_eCenter, BWT_VerticalAlignment_eCenter);
    assert(pView->dynrng);
    if (processing)
    {
        pView->processing = osd_info_panel_p_populate_cell(osd, table, row, ++c, NULL, BWT_HorizontalAlignment_eCenter, BWT_VerticalAlignment_eCenter);
        assert(pView->processing);
    }
}

static BWT_TableHandle osd_info_panel_p_create_table(OsdHandle osd, OsdInfoPanel *infoPanel, unsigned textHeight, unsigned numVideos)
{
    BWT_TableHandle table;
    BWT_TableCreateSettings tableSettings;
    unsigned r,i;

    BWT_Table_GetDefaultCreateSettings(&tableSettings);
    tableSettings.colWidth = 110;
    tableSettings.rowHeight = osd_p_compute_external_dim(textHeight, tableSettings.padding);
    tableSettings.cols = 3;
    tableSettings.rows = 2 + numVideos;
    tableSettings.fgColor = osd->createSettings.theme.textForegroundColor;
    table = BWT_Table_Create(osd->bwt, &tableSettings);
    assert(table);

    r = -1;
    for(i=0; i<numVideos; i++) {
        char buffer [16];
        snprintf(buffer, 16, "%s %d", STR_VID_LONG, i);
        osd_info_panel_p_populate_row(osd, table, &infoPanel->vid[i], ++r, buffer, true);
    }
    osd_info_panel_p_populate_row(osd, table, &infoPanel->gfx, ++r, STR_GFX_LONG, true);
    osd_info_panel_p_populate_row(osd, table, &infoPanel->out, ++r, STR_OUT_LONG, false);

    return table;
}

BWT_PanelHandle osd_p_create_info_panel(OsdHandle osd, OsdInfoPanel *infoPanel, unsigned textHeight, unsigned numVideos)
{
    BWT_PanelHandle info;
    BWT_PanelCreateSettings settings;
    const BWT_Dimensions * dims;
    BWT_TableHandle table;

    assert(osd);

    table = osd_info_panel_p_create_table(osd, infoPanel, textHeight, numVideos);
    dims = BWT_Widget_GetDimensions((BWT_WidgetHandle)table);

    BWT_Panel_GetDefaultCreateSettings(&settings);
    settings.name = STR_INFO_PANEL_NAME;
    settings.padding = PANEL_PADDING;
    settings.spacing = 1;
    settings.dims.width = osd_p_compute_external_dim(dims->width, settings.padding);
    settings.dims.height = osd_p_compute_external_dim(dims->height, settings.padding);
    settings.color = osd->createSettings.theme.infoPanelBackgroundColor;
    settings.visible = false;
    info = BWT_Panel_Create(osd->bwt, &settings);
    assert(info);

    BWT_Panel_AddChild(info, (BWT_WidgetHandle)table, BWT_VerticalAlignment_eDefault, BWT_HorizontalAlignment_eDefault);
    infoPanel->table = table;

    return info;
}

BWT_PanelHandle osd_p_create_main_panel(OsdHandle osd)
{
    BWT_PanelHandle panel;
    BWT_PanelCreateSettings settings;
    const BWT_Dimensions * dims;
    static const char * mainPanelName = "main";

    assert(osd);

    dims =  BWT_Toolkit_GetFramebufferDimensions(osd->bwt);
    assert(dims);
    BWT_Panel_GetDefaultCreateSettings(&settings);
    settings.name = mainPanelName;
    memcpy(&settings.dims, dims, sizeof(*dims));
    settings.padding = OSD_PADDING;
    settings.spacing = 0;
    settings.color = osd->createSettings.theme.mainPanelBackgroundColor;
    panel = BWT_Panel_Create(osd->bwt, &settings);
    assert(panel);
    BWT_Widget_Place((BWT_WidgetHandle)panel);

    return panel;
}

BWT_VideoWindowHandle osd_p_create_window(OsdHandle osd, const BWT_Dimensions * videoDims, unsigned id)
{
    BWT_VideoWindowHandle window;
    BWT_VideoWindowCreateSettings settings;
    const BWT_Dimensions * dims;
    static const char * windowName = "window";

    assert(osd);

    dims =  BWT_Toolkit_GetFramebufferDimensions(osd->bwt);
    assert(dims);
    BWT_VideoWindow_GetDefaultCreateSettings(&settings);
    settings.visible = true;
    settings.name = windowName;
    settings.dims = *videoDims;
    settings.id = id;
    window = BWT_VideoWindow_Create(osd->bwt, &settings);
    assert(window);

    return window;
}

static const char * STR_VIDEO_PANEL_NAME = "video";

BWT_PanelHandle osd_p_create_video_panel(OsdHandle osd, const BWT_Dimensions * mainDims)
{
    BWT_PanelHandle video;
    BWT_PanelCreateSettings panelSettings;
    unsigned i;

    assert(osd);

    BWT_Panel_GetDefaultCreateSettings(&panelSettings);
    panelSettings.name = STR_VIDEO_PANEL_NAME;
    panelSettings.dims.width = mainDims->width;
    panelSettings.dims.height = mainDims->height;
    panelSettings.padding = 0;
    panelSettings.spacing = 0;
    panelSettings.flow = BWT_LayoutFlow_eHorizontal;
    video = BWT_Panel_Create(osd->bwt, &panelSettings);
    assert(video);

    for(i = 0; i < platform_graphics_get_mosaic_count(osd->createSettings.gfx); i++) {
        osd->window[i] = osd_p_create_window(osd, &panelSettings.dims, i);
        assert(osd->window[i]);
    }

    return video;
}

static const char * STR_PIP_PANEL_NAME = "pip";

BWT_PanelHandle osd_p_create_pip_panel(OsdHandle osd, const BWT_Dimensions * mainDims)
{
    BWT_PanelHandle pip;
    BWT_PanelCreateSettings panelSettings;

    assert(osd);

    BWT_Panel_GetDefaultCreateSettings(&panelSettings);
    panelSettings.name = STR_PIP_PANEL_NAME;
    panelSettings.dims.width = mainDims->width / 3;
    panelSettings.dims.height = mainDims->height / 3;
    panelSettings.padding = 0;
    panelSettings.spacing = 0;
    panelSettings.flow = BWT_LayoutFlow_eHorizontal;
    pip = BWT_Panel_Create(osd->bwt, &panelSettings);
    assert(pip);

    return pip;
}

void osd_get_default_create_settings(OsdCreateSettings * pSettings)
{
    assert(pSettings);
    memset(pSettings, 0, sizeof(*pSettings));
}

OsdHandle osd_create(const OsdCreateSettings * pSettings)
{
    OsdHandle osd;
    const BWT_Dimensions * mainDims;
    BWT_ToolkitCreateSettings settings;
    unsigned mainPadding;
    unsigned textHeight;

    assert(pSettings);
    assert(pSettings->gfx);

    osd = malloc(sizeof(*osd));
    if (!osd) goto error;
    memset(osd, 0, sizeof(*osd));
    BWT_Toolkit_GetDefaultCreateSettings(&settings);
    settings.gfx = pSettings->gfx;
    osd->bwt = BWT_Toolkit_Create(&settings);
    if (!osd->bwt) goto error;

    memcpy(&osd->createSettings, pSettings, sizeof(*pSettings));

    if (pthread_mutex_init(&osd->lock, NULL)) goto error;

    platform_get_default_model(&osd->model);

    osd->mosaicCount = platform_graphics_get_mosaic_count(pSettings->gfx);

    osd->main = osd_p_create_main_panel(osd);
    if (!osd->main) goto error;
    mainDims = BWT_Widget_GetDimensions((BWT_WidgetHandle)osd->main);
    mainPadding = BWT_Widget_GetPadding((BWT_WidgetHandle)osd->main);
    textHeight = BWT_Toolkit_GetTextHeight(osd->bwt);

    osd->video = osd_p_create_video_panel(osd, mainDims);
    if (!osd->video) goto error;
    BWT_Panel_AddChild(osd->main, (BWT_WidgetHandle)osd->video, BWT_VerticalAlignment_eTop, BWT_HorizontalAlignment_eLeft);

    osd->pip = osd_p_create_pip_panel(osd, mainDims);
    if (!osd->pip) goto error;
    BWT_Panel_AddChild(osd->main, (BWT_WidgetHandle)osd->pip, BWT_VerticalAlignment_eTop, BWT_HorizontalAlignment_eRight);

    osd->info.base = osd_p_create_info_panel(osd, &osd->info, textHeight, 1);
    if (!osd->info.base) goto error;
    osd->mosaicInfo.base = osd_p_create_info_panel(osd, &osd->mosaicInfo, textHeight, osd->mosaicCount);
    if (!osd->mosaicInfo.base) goto error;
    osd->pipInfo.base = osd_p_create_info_panel(osd, &osd->pipInfo, textHeight, 2);
    if (!osd->pipInfo.base) goto error;
    BWT_Panel_AddChild(osd->main, (BWT_WidgetHandle)osd->info.base, BWT_VerticalAlignment_eBottom, BWT_HorizontalAlignment_eCenter);
    osd->current = &osd->info;

    osd->usageMode = PlatformUsageMode_eMax;

    osd->renderer = platform_scheduler_add_listener(platform_get_scheduler(osd->createSettings.platform), &osd_p_scheduler_callback, osd);
    if (!osd->renderer)
    {
        printf("OSD unavailable\n");
    }

    return osd;

error:
    osd_destroy(osd);
    return NULL;
}

void osd_destroy(OsdHandle osd)
{
    if (!osd) return;

    pthread_mutex_lock(&osd->lock);
    if (osd->renderer)
    {
        platform_scheduler_remove_listener(platform_get_scheduler(osd->createSettings.platform), osd->renderer);
    }
    BWT_Panel_Destroy(osd->main); /* will destroy all children of main */
    BWT_Toolkit_Destroy(osd->bwt);
    pthread_mutex_unlock(&osd->lock);
    pthread_mutex_destroy(&osd->lock);
    free(osd);
}

void osd_flip(OsdHandle osd)
{
    assert(osd);
    platform_scheduler_wake(platform_get_scheduler(osd->createSettings.platform));
}

void osd_p_scheduler_callback(void * pContext, int param)
{
    OsdHandle osd = pContext;
    assert(osd);
    (void)param;
    pthread_mutex_lock(&osd->lock);
    BWT_Widget_Render((BWT_WidgetHandle)osd->main);
    BWT_Toolkit_Submit(osd->bwt);
    pthread_mutex_unlock(&osd->lock);
}

void osd_toggle_visibility(OsdHandle osd)
{
    assert(osd);
    pthread_mutex_lock(&osd->lock);
    BWT_Widget_ToggleVisibility((BWT_WidgetHandle)osd->main);
    pthread_mutex_unlock(&osd->lock);
}

void osd_set_visibility(OsdHandle osd, bool visible)
{
    assert(osd);
    pthread_mutex_lock(&osd->lock);
    BWT_Widget_SetVisibility((BWT_WidgetHandle)osd->main, visible);
    pthread_mutex_unlock(&osd->lock);
}

static void osd_p_configure_info_panel_layout(OsdHandle osd, PlatformUsageMode usageMode, bool layout)
{
    BWT_VerticalAlignment vAlign;
    BWT_HorizontalAlignment hAlign;
    OsdInfoPanel *infoPanel = NULL;

    if(usageMode == PlatformUsageMode_eMosaic && layout == 1) {
        vAlign = BWT_VerticalAlignment_eCenter;
        hAlign = BWT_HorizontalAlignment_eLeft;
    } else {
        vAlign = BWT_VerticalAlignment_eBottom;
        hAlign = BWT_HorizontalAlignment_eCenter;
    }
    switch (usageMode)
    {
        case PlatformUsageMode_eMosaic:
            infoPanel = &osd->mosaicInfo;
            break;
        case PlatformUsageMode_eMainPip:
            infoPanel = &osd->pipInfo;
            break;
        default:
            infoPanel = &osd->info;
            break;
    }

    BWT_Widget_SetVisibility((BWT_WidgetHandle)osd->current->base, false);
    BWT_Panel_RemoveChild(osd->main, (BWT_WidgetHandle)osd->current->base);
    BWT_Panel_AddChild(osd->main, (BWT_WidgetHandle)infoPanel->base, vAlign, hAlign);
    BWT_Widget_SetVisibility((BWT_WidgetHandle)infoPanel->base, true);

    osd->current = infoPanel;
}

static const char * const usageModeStrings[] =
{
    "full-screen video",
    "pig",
    "mosaic",
    "pip",
    "unset",
    NULL
};

static const unsigned mosdims[10][2] =
{
    { 0, 0 },
    { 1, 1 },
    { 1, 2 },
    { 2, 2 },
    { 2, 2 },
    { 2, 3 },
    { 2, 3 },
    { 3, 3 },
    { 3, 3 },
    { 3, 3 }
};

static void get_mosaic_dims(unsigned count, unsigned * rows, unsigned * windows_per_row)
{
    if (count > 9) *rows = *windows_per_row = 0;
    else
    {
        *rows = mosdims[count][0];
        *windows_per_row = mosdims[count][1];
    }
}

BWT_VerticalAlignment quantize_row(unsigned row, unsigned rows)
{
    BWT_VerticalAlignment valign;
    switch (rows)
    {
        case 0:
            valign = BWT_VerticalAlignment_eDefault;
            break;
        case 1:
            valign = BWT_VerticalAlignment_eCenter;
            break;
        default:
        case 2:
            valign = (row == 0) ? BWT_VerticalAlignment_eTop : BWT_VerticalAlignment_eBottom;
            break;
        case 3:
            valign = (BWT_VerticalAlignment)row;
            break;
    }
    return valign;
}

void osd_set_usage_mode(OsdHandle osd, PlatformUsageMode usageMode, unsigned layout)
{
    BWT_VerticalAlignment vAlign = BWT_VerticalAlignment_eTop;
    BWT_HorizontalAlignment hAlign = BWT_HorizontalAlignment_eLeft;
    unsigned i;
    unsigned pip = platform_graphics_get_pip_window_id(osd->createSettings.gfx);
    unsigned full = platform_graphics_get_main_window_id(osd->createSettings.gfx);
    unsigned pig = full;

    printf("osd: old usage: %s; new usage: %s\n", usageModeStrings[osd->usageMode], usageModeStrings[usageMode]);

    if (osd->usageMode == usageMode && osd->layout == layout) return;

    pthread_mutex_lock(&osd->lock);

    switch (osd->usageMode)
    {
        case PlatformUsageMode_ePictureInGraphics:
            printf("osd: hiding background\n");
            if (osd->background) BWT_Widget_SetVisibility((BWT_WidgetHandle)osd->background, false);
            printf("osd: removing pig window\n");
            BWT_Panel_RemoveChild(osd->video, (BWT_WidgetHandle)osd->window[pig]);
            break;
        case PlatformUsageMode_eMosaic:
            if (osd->layout == 1) printf("osd: hiding background\n");
            if (osd->layout == 1 && osd->background) BWT_Widget_SetVisibility((BWT_WidgetHandle)osd->background, false);
            printf("osd: removing mosaic windows\n");
            for (i=0; i<osd->mosaicCount; i++) {
                if (osd->window[i]) BWT_Panel_RemoveChild(osd->video, (BWT_WidgetHandle)osd->window[i]);
            }
            break;
        case PlatformUsageMode_eMainPip:
            printf("osd: removing pip window\n");
            BWT_Widget_SetVisibility((BWT_WidgetHandle)osd->pip, false);
            BWT_Panel_RemoveChild(osd->pip, (BWT_WidgetHandle)osd->window[pip]);
            if (usageMode != PlatformUsageMode_eFullScreenVideo)
            {
                printf("osd: removing main window\n");
                BWT_Panel_RemoveChild(osd->video, (BWT_WidgetHandle)osd->window[full]);
            }
            break;
        default:
        case PlatformUsageMode_eFullScreenVideo:
            printf("osd: hiding thumbnail\n");
            if (osd->thumbnail) BWT_Widget_SetVisibility((BWT_WidgetHandle)osd->thumbnail, false);
            if (usageMode != PlatformUsageMode_eMainPip)
            {
                printf("osd: removing main window\n");
                BWT_Panel_RemoveChild(osd->video, (BWT_WidgetHandle)osd->window[full]);
            }
            break;
    }

    osd_p_configure_info_panel_layout(osd, usageMode, layout);

    switch (usageMode)
    {
        case PlatformUsageMode_ePictureInGraphics:
            printf("osd: showing background\n");
            if (osd->background) BWT_Widget_SetVisibility((BWT_WidgetHandle)osd->background, true);
            printf("osd: adding pig window @30%%\n");
            BWT_VideoWindow_SetScale(osd->window[pig], 30);
            BWT_Panel_AddChild(osd->video, (BWT_WidgetHandle)osd->window[pig], vAlign, hAlign);
            break;
        case PlatformUsageMode_eMosaic:
            if (layout == 1) printf("osd: showing background\n");
            if (osd->background) BWT_Widget_SetVisibility((BWT_WidgetHandle)osd->background, layout==0?false:true);
            printf("osd: adding %d mosaic windows in layout %u\n", osd->mosaicCount, layout);
            if (layout == 1) {
                unsigned smallItemScale = 33;
                if (osd->mosaicCount > 2)
                {
                    smallItemScale = 100 / (osd->mosaicCount - 1);
                }
                printf("osd: rendering 1@50%% + %d@%d%% mosaic windows\n", osd->mosaicCount - 1, smallItemScale);
                for (i = 0; i < osd->mosaicCount; i++) {
                    BWT_VideoWindow_SetScale(osd->window[i], i == 0 ? 50 : smallItemScale);
                    vAlign = i==0?BWT_VerticalAlignment_eTop:BWT_VerticalAlignment_eBottom;
                    hAlign = i==0?BWT_VerticalAlignment_eCenter:BWT_HorizontalAlignment_eLeft;
                    BWT_Panel_AddChild(osd->video, (BWT_WidgetHandle)osd->window[i], vAlign, hAlign);
                }
            } else {
                unsigned windows_per_row;
                unsigned scale;
                unsigned rows;
                unsigned row;

                get_mosaic_dims(osd->mosaicCount, &rows, &windows_per_row);
                if (!windows_per_row || !rows) {
                    printf("osd: error computing mosaic windows per row; usage incomplete\n");
                    break;
                }
                printf("osd: rendering %dx%d mosaic windows\n", rows, windows_per_row);

                if (osd->mosaicCount < 2) scale = 50;
                else scale = 100 / rows;

                row = 0;
                for (i = 0; i < osd->mosaicCount; i++) {
                    BWT_VideoWindow_SetScale(osd->window[i], scale);
                    BWT_Panel_AddChild(osd->video, (BWT_WidgetHandle)osd->window[i], quantize_row(row, rows), hAlign);
                    if ((i + 1) % windows_per_row == 0) row++;
                }
            }
            osd->layout = layout;
            break;
        case PlatformUsageMode_eMainPip:
            if (osd->usageMode != PlatformUsageMode_eFullScreenVideo)
            {
                printf("osd: adding main window @100%%\n");
                BWT_VideoWindow_SetScale(osd->window[full], 100);
                BWT_Panel_AddChild(osd->video, (BWT_WidgetHandle)osd->window[full], BWT_VerticalAlignment_eTop, BWT_HorizontalAlignment_eLeft);
            }
            printf("osd: adding pip window @33%%\n");
            BWT_VideoWindow_SetScale(osd->window[pip], 33);
            BWT_Panel_AddChild(osd->pip, (BWT_WidgetHandle)osd->window[pip], BWT_VerticalAlignment_eCenter, BWT_HorizontalAlignment_eCenter);
            break;
        default:
        case PlatformUsageMode_eFullScreenVideo:
            printf("osd: showing thumbnail\n");
            if (osd->thumbnail) BWT_Widget_SetVisibility((BWT_WidgetHandle)osd->thumbnail, true);
            if (osd->usageMode != PlatformUsageMode_eMainPip)
            {
                printf("osd: adding main window @100%%\n");
                BWT_VideoWindow_SetScale(osd->window[full], 100);
                BWT_Panel_AddChild(osd->video, (BWT_WidgetHandle)osd->window[full], vAlign, hAlign);
            }
            break;
    }

    osd->usageMode = usageMode;

    pthread_mutex_unlock(&osd->lock);
}

void osd_label_p_update_dynrng(BWT_LabelHandle label, PlatformDynamicRange dynrng)
{
    assert(label);
    BWT_Label_SetText(label, platform_get_dynamic_range_name(dynrng));
}

void osd_label_p_update_processing(BWT_LabelHandle label, PlatformTriState processing)
{
    unsigned color;
    assert(label);
    BWT_Label_SetText(label, STR_TABLE_PROC[processing]);
    switch (processing)
    {
        case PlatformTriState_eOn:
        case PlatformTriState_eInactive:
            color = COLOR_GREEN_100;
            break;
        default:
        case PlatformTriState_eOff:
            color = COLOR_RED_100;
            break;
    }
    BWT_Widget_SetColor(BWT_Widget_GetParent((BWT_WidgetHandle)label), color);
}

#define OSD_UPDATE_ENUM_TEMPLATE(X,V,N,C) \
    assert((V)->X); \
    if ((C)->X != (N)->X) \
    { \
        (C)->X = (N)->X; \
        osd_label_p_update_##X((V)->X, (C)->X); \
    }

static void osd_p_update_picture_info_view(OsdHandle osd, const PlatformPictureModel * pNewModel, PlatformPictureModel * pCurModel, OsdPictureInfoView * pView, bool processing)
{
    assert(pCurModel);
    assert(pNewModel);
    assert(pView);

    pthread_mutex_lock(&osd->lock);
    OSD_UPDATE_ENUM_TEMPLATE(dynrng, pView, &pNewModel->info, &pCurModel->info);
    if (processing)
    {
        OSD_UPDATE_ENUM_TEMPLATE(processing, pView, pNewModel, pCurModel);
    }
    pthread_mutex_unlock(&osd->lock);
}

void osd_update_out_model(OsdHandle osd, const PlatformPictureModel * pModel)
{
    osd_p_update_picture_info_view(osd, pModel, &osd->model.out, &osd->current->out, false);
}

void osd_update_gfx_model(OsdHandle osd, const PlatformPictureModel * pModel)
{
    osd_p_update_picture_info_view(osd, pModel, &osd->model.gfx, &osd->current->gfx, true);
}

void osd_update_vid_model(OsdHandle osd, const PlatformPictureModel * pModel, unsigned index)
{
    osd_p_update_picture_info_view(osd, pModel, &osd->model.vid[index], &osd->current->vid[index], true);
}

void osd_update_thumbnail(OsdHandle osd, PlatformPictureHandle pic)
{
    static const char * STR_THUMBNAIL_NAME = "thumbnail";
    assert(osd);
    if (pic == osd->model.thumbnail) return;

    pthread_mutex_lock(&osd->lock);
    osd->model.thumbnail = pic;
    if (osd->thumbnail)
    {
        BWT_Panel_RemoveChild(osd->main, (BWT_WidgetHandle)osd->thumbnail);
        BWT_Image_Destroy(osd->thumbnail);
        osd->thumbnail = NULL;
    }
    if (pic)
    {
        BWT_ImageCreateSettings settings;

        BWT_Image_GetDefaultCreateSettings(&settings);
        settings.name = STR_THUMBNAIL_NAME;
        settings.pic = pic;
        settings.visible = osd->usageMode == PlatformUsageMode_eFullScreenVideo;
        osd->thumbnail = BWT_Image_Create(osd->bwt, &settings);
        assert(osd->thumbnail);
        BWT_Panel_AddChild(osd->main, (BWT_WidgetHandle)osd->thumbnail, BWT_VerticalAlignment_eBottom, BWT_HorizontalAlignment_eRight);
    }
    pthread_mutex_unlock(&osd->lock);
}

void osd_update_background(OsdHandle osd, PlatformPictureHandle pic)
{
    static const char * STR_BACKGROUND_NAME = "background";
    assert(osd);
    if (pic == osd->model.background) return;

    pthread_mutex_lock(&osd->lock);
    osd->model.background = pic;
    if (osd->background)
    {
        BWT_Panel_SetBackground(osd->main, NULL);
        BWT_Image_Destroy(osd->background);
        osd->background = NULL;
    }
    if (pic)
    {
        BWT_ImageCreateSettings settings;

        BWT_Image_GetDefaultCreateSettings(&settings);
        settings.name = STR_BACKGROUND_NAME;
        settings.pic = pic;
        settings.visible = osd->usageMode == PlatformUsageMode_ePictureInGraphics || (osd->usageMode == PlatformUsageMode_eMosaic && osd->layout == 1);
        osd->background = BWT_Image_Create(osd->bwt, &settings);
        assert(osd->background);
        BWT_Panel_SetBackground(osd->main, osd->background);
    }
    pthread_mutex_unlock(&osd->lock);
}
