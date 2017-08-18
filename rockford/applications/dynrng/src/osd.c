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

static const char * STR_OSD_INSTRUCTIONS[] =
{
    "REMOTE USAGE:",
    "  UP/DOWN:  ADJUST VIDEO SETTING",
    "  LEFT/RIGHT:  ADJUST GRAPHICS SETTING",
    "  CHANUP/CHANDOWN:  CHANGE STREAMS",
    "  FFWD/REW:  CHANGE IMAGES",
    "  SELECT:  TOGGLE THESE ON-SCREEN INSTRUCTIONS",
    "  PLAY/PAUSE:  TOGGLE PLAY/PAUSE",
    "  BACK(LAST):  CYCLE OUTPUT DYNRNG (RESETS ON STREAM CHANGE UNLESS MADE STICKY)",
    "  POWER:  TOGGLE OUTPUT DYNRNG LOCK (DEFAULT OFF)",
    "  STOP:  CYCLE OUTPUT COLORIMETRY (AUTO, SD/601, HD/709, UHD/2020)"
};

const char ** osd_get_instructions(unsigned * pCount)
{
    assert(pCount);
    *pCount = sizeof(STR_OSD_INSTRUCTIONS)/sizeof(STR_OSD_INSTRUCTIONS[0]);
    return STR_OSD_INSTRUCTIONS;
}

static unsigned osd_p_compute_external_dim(unsigned internalDim, unsigned padding)
{
    return BWT_RELATIVE_BASE * internalDim / (BWT_RELATIVE_BASE - 2 * padding);
}

static unsigned osd_p_compute_internal_dim(unsigned externalDim, unsigned padding)
{
    return externalDim - externalDim * 2 * padding / BWT_RELATIVE_BASE;
}

#if ENABLE_GUIDE
static const char * STR_GUIDE_PANEL_NAME = "guide";

BWT_PanelHandle osd_p_create_guide_panel(OsdHandle osd, const BWT_Dimensions * mainDims, unsigned mainPadding, unsigned textHeight)
{
    BWT_PanelHandle guide;
    BWT_LabelHandle label;
    unsigned i;
    unsigned count;
    const char ** instructions;
    BWT_PanelCreateSettings panelSettings;
    BWT_LabelCreateSettings labelSettings;

    assert(osd);

    BWT_Panel_GetDefaultCreateSettings(&panelSettings);
    panelSettings.name = STR_GUIDE_PANEL_NAME;
    panelSettings.dims.width = osd_p_compute_internal_dim(mainDims->width, mainPadding);
    panelSettings.dims.height = osd_p_compute_internal_dim(mainDims->height, mainPadding);
    panelSettings.padding = PANEL_PADDING;
    panelSettings.visible = false;
    panelSettings.color = osd->theme.guidePanelBackgroundColor;
    guide = BWT_Panel_Create(osd->bwt, &panelSettings);
    assert(guide);

    BWT_Label_GetDefaultCreateSettings(&labelSettings);
    labelSettings.color = osd->theme.textForegroundColor;

    instructions = osd_get_instructions(&count);
    for (i = 0; i < count; i++)
    {
        labelSettings.text = instructions[i];
        label = BWT_Label_Create(osd->bwt, &labelSettings);
        assert(label);
        BWT_Panel_AddChild(guide, (BWT_WidgetHandle)label, BWT_VerticalAlignment_eDefault, BWT_HorizontalAlignment_eDefault);
    }

    return guide;
}
#endif

#if 0
BWT_LabelHandle osd_p_add_name_value_pair(
    OsdHandle osd,
    BWT_PanelHandle panel,
    const char * name,
    const char * value,
    unsigned nameX,
    unsigned valueX,
    unsigned y)
{
    BWT_LabelHandle label;
    BWT_LabelCreateSettings settings;

    assert(osd);
    assert(panel);

    BWT_Label_GetDefaultCreateSettings(&settings);
    settings.color = osd->theme.textForegroundColor;

    settings.text = name;
    label = BWT_Label_Create(osd->bwt, &settings);
    assert(label);
    BWT_Panel_AddChild(panel, (BWT_WidgetHandle)label, nameX, y);

    settings.name = name;
    settings.text = value;
    label = BWT_Label_Create(osd->bwt, &settings);
    assert(label);
    BWT_Panel_AddChild(panel, (BWT_WidgetHandle)label, valueX, y);

    return label;
}
#endif

static const char * STR_GUIDE_INSTRUCTION = "Press <select> or <enter> to view instructions";
static const char * STR_DESC = "DESC";
/*static const char * STR_VID = "VID";*/
static const char * STR_VID_LONG = "VIDEO IN";
/*static const char * STR_GFX = "GFX";*/
static const char * STR_GFX_LONG = "GRAPHICS";
static const char * STR_SEL = "SELECTOR";
/*static const char * STR_OUT = "OUT";*/
static const char * STR_OUT_LONG = "VIDEO OUT";
static const char * STR_RCV = "EDID SUPPORT";
static const char * STR_FORMAT = "FORMAT";
/*static const char * STR_SPACE = "CSPC";*/
static const char * STR_DYNRNG = "DYNRNG";
static const char * STR_GAMUT = "GAMUT";
static const char * STR_DEPTH = "BITDEPTH";
static const char * STR_PLM = "PLM";
static const char * STR_SEL_USER = "USER";
static const char * STR_SEL_AUTO = "AUTO";
static const char * STR_DETAILS_PANEL_NAME = "details";
static const char * STR_INFO_PANEL_NAME = "info";
static const unsigned COLOR_RED_100 = 0xffff0000;
static const unsigned COLOR_RED_20 = 0x33ff0000;
static const unsigned COLOR_GREEN_100 = 0xff00ff00;
static const unsigned COLOR_GREEN_20 = 0x3300ff00;
static const unsigned COLOR_BLUE_20 = 0x330000ff;
static const unsigned COLOR_LIGHT_BLUE_20 = 0x33000080;
/*static const unsigned COLOR_LIGHT_GREY_20 = 0x333f3f3f;*/
static const char * STR_TABLE_PLM[] =
{
    "PLM OFF",
    "PLM ON",
    "PLM PASS",
    NULL
};

/*
 * |DESC        |FORMAT      |DYNRNG      |GAMUT       |BITDEPTH    |PLM         |
 * |VID INPUT   |3840x2160p60|HDR10       |UHD/BT2020  |10          |0           |
 * |GFX INPUT   |180x320     |SDR         |HD/BT701    |8           |1           |
 * |SELECTOR    |AUTO        |AUTO        |AUTO        |AUTO        |            |
 * |OUTPUT      |3840x2160p60|SDR         |SD/BT601    |12          |            |
 * |EDID SUPPORT|Y           |Y           |Y           |Y           |            |
 */
static BWT_LabelHandle osd_details_panel_p_populate_cell(OsdHandle osd, BWT_TableHandle table, BWT_WidgetHandle row, unsigned c, const char * text)
{
    BWT_PanelHandle cell;
    BWT_LabelHandle label;
    BWT_LabelCreateSettings labelSettings;

    cell = (BWT_PanelHandle)BWT_Table_GetCell(table, row, c);
    BWT_Label_GetDefaultCreateSettings(&labelSettings);
    labelSettings.color = osd->theme.textForegroundColor;
    labelSettings.text = text;
    label = BWT_Label_Create(osd->bwt, &labelSettings);
    assert(label);
    BWT_Panel_AddChild(cell, (BWT_WidgetHandle)label, BWT_VerticalAlignment_eDefault, BWT_HorizontalAlignment_eDefault);
    return label;
}

void osd_details_panel_p_populate_headers(OsdHandle osd, BWT_TableHandle table)
{
    BWT_WidgetHandle row;
    unsigned c;
    c = 0;

    row = BWT_Table_GetRow(table, 0);
    BWT_Widget_SetColor(row, COLOR_LIGHT_BLUE_20);
    osd_details_panel_p_populate_cell(osd, table, row, c, STR_DESC);
    osd_details_panel_p_populate_cell(osd, table, row, ++c, STR_FORMAT);
#if 0
    osd_details_panel_p_populate_cell(osd, table, row, ++c, STR_SPACE);
#endif
    osd_details_panel_p_populate_cell(osd, table, row, ++c, STR_DYNRNG);
    osd_details_panel_p_populate_cell(osd, table, row, ++c, STR_GAMUT);
    osd_details_panel_p_populate_cell(osd, table, row, ++c, STR_DEPTH);
    osd_details_panel_p_populate_cell(osd, table, row, ++c, STR_PLM);
}

void osd_details_panel_p_populate_row(OsdHandle osd, BWT_TableHandle table, OsdPictureInfoView * pView, unsigned r, const char * desc, bool plm)
{
    BWT_WidgetHandle row;
    unsigned c;
    c = 0;
    row = BWT_Table_GetRow(table, r);
    osd_details_panel_p_populate_cell(osd, table, row, c, desc);
    pView->format = osd_details_panel_p_populate_cell(osd, table, row, ++c, NULL);
    assert(pView->format);
#if 0
    pView->space = osd_details_panel_p_populate_cell(osd, table, row, ++c, NULL);
    assert(pView->space);
#endif
    pView->dynrng = osd_details_panel_p_populate_cell(osd, table, row, ++c, NULL);
    assert(pView->dynrng);
    pView->gamut = osd_details_panel_p_populate_cell(osd, table, row, ++c, NULL);
    assert(pView->gamut);
    pView->depth = osd_details_panel_p_populate_cell(osd, table, row, ++c, NULL);
    assert(pView->depth);
    if (plm)
    {
        pView->plm = osd_details_panel_p_populate_cell(osd, table, row, ++c, NULL);
        assert(pView->plm);
    }
}

static BWT_TableHandle osd_details_panel_p_create_table(OsdHandle osd, OsdInfoPanel *detailsPanel, unsigned textHeight, unsigned numVideos)
{
    BWT_TableHandle table;
    BWT_TableCreateSettings tableSettings;
    unsigned r, i;

    BWT_Table_GetDefaultCreateSettings(&tableSettings);
    tableSettings.colWidth = 135;
    tableSettings.rowHeight = osd_p_compute_external_dim(textHeight, tableSettings.padding);
    tableSettings.cols = 6;
    tableSettings.rows = 5 + numVideos;
    tableSettings.fgColor = osd->theme.textForegroundColor;
    table = BWT_Table_Create(osd->bwt, &tableSettings);
    assert(table);

    r = 0;
    osd_details_panel_p_populate_headers(osd, table);
    for(i=0; i<numVideos; i++) {
        char buffer [16];
        snprintf(buffer, 16, "%s %d", STR_VID_LONG, i);
        osd_details_panel_p_populate_row(osd, table, &detailsPanel->vid[i], ++r, buffer, true);
    }
    osd_details_panel_p_populate_row(osd, table, &detailsPanel->gfx, ++r, STR_GFX_LONG, true);
    osd_details_panel_p_populate_row(osd, table, &detailsPanel->sel, ++r, STR_SEL, false);
    osd_details_panel_p_populate_row(osd, table, &detailsPanel->out, ++r, STR_OUT_LONG, false);
    osd_details_panel_p_populate_row(osd, table, &detailsPanel->rcv, ++r, STR_RCV, false);

    return table;
}

BWT_PanelHandle osd_p_create_details_panel(OsdHandle osd, OsdInfoPanel *detailsPanel, const BWT_Dimensions * mainDims, unsigned textHeight, unsigned numVideos)
{
    BWT_PanelHandle details;
    BWT_PanelCreateSettings settings;
    const BWT_Dimensions * dims;
    BWT_TableHandle table;

    assert(mainDims);
    assert(osd);

    table = osd_details_panel_p_create_table(osd, detailsPanel, textHeight, numVideos);
    dims = BWT_Widget_GetDimensions((BWT_WidgetHandle)table);

    BWT_Panel_GetDefaultCreateSettings(&settings);
    settings.name = STR_DETAILS_PANEL_NAME;
    settings.padding = PANEL_PADDING;
    settings.spacing = 1;
    settings.dims.width = osd_p_compute_external_dim(dims->width, settings.padding);
    settings.dims.height =  osd_p_compute_external_dim(BWT_Toolkit_GetTextHeight(osd->bwt) + settings.spacing + dims->height, settings.padding);
    settings.visible = false;
    settings.color = osd->theme.detailsPanelBackgroundColor;
    details = BWT_Panel_Create(osd->bwt, &settings);
    assert(details);

    {
        BWT_LabelHandle label;
        BWT_LabelCreateSettings labelSettings;
        BWT_Label_GetDefaultCreateSettings(&labelSettings);
        labelSettings.color = osd->theme.textForegroundColor;
        labelSettings.text = STR_GUIDE_INSTRUCTION;
        label = BWT_Label_Create(osd->bwt, &labelSettings);
        assert(label);
        BWT_Panel_AddChild(details, (BWT_WidgetHandle)label, BWT_VerticalAlignment_eBottom, BWT_HorizontalAlignment_eDefault);
    }

    BWT_Panel_AddChild(details, (BWT_WidgetHandle)table, BWT_VerticalAlignment_eBottom, BWT_HorizontalAlignment_eDefault);
    detailsPanel->table = table;

    return details;
}

/*
 * Smaller Info Panel
 * |VIDEO IN |HDR10|PLM ON | # ON -> GREEN
 * |GRAPHICS |SDR  |PLM OFF| # OFF -> RED
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
    labelSettings.color = osd->theme.textForegroundColor;
    labelSettings.text = text;
    labelSettings.halign = halign;
    labelSettings.valign = valign;
    label = BWT_Label_Create(osd->bwt, &labelSettings);
    assert(label);
    BWT_Panel_AddChild(cell, (BWT_WidgetHandle)label, BWT_VerticalAlignment_eDefault, BWT_HorizontalAlignment_eDefault);
    return label;
}

void osd_info_panel_p_populate_headers(OsdHandle osd, BWT_TableHandle table)
{
    BWT_WidgetHandle row;
    unsigned c;
    c = 0;

    row = BWT_Table_GetRow(table, 0);
    BWT_Widget_SetColor(row, COLOR_LIGHT_BLUE_20);
    osd_info_panel_p_populate_cell(osd, table, row, c, STR_DESC, BWT_HorizontalAlignment_eCenter, BWT_VerticalAlignment_eCenter);
    osd_info_panel_p_populate_cell(osd, table, row, ++c, STR_DYNRNG, BWT_HorizontalAlignment_eCenter, BWT_VerticalAlignment_eCenter);
    osd_info_panel_p_populate_cell(osd, table, row, ++c, STR_PLM, BWT_HorizontalAlignment_eCenter, BWT_VerticalAlignment_eCenter);
}

void osd_info_panel_p_populate_row(OsdHandle osd, BWT_TableHandle table, OsdPictureInfoView * pView, unsigned r, const char * desc, bool plm)
{
    BWT_WidgetHandle row;
    unsigned c;
    c = 0;
    row = BWT_Table_GetRow(table, r);
    osd_info_panel_p_populate_cell(osd, table, row, c, desc, BWT_HorizontalAlignment_eLeft, BWT_VerticalAlignment_eCenter);
    pView->dynrng = osd_info_panel_p_populate_cell(osd, table, row, ++c, NULL, BWT_HorizontalAlignment_eCenter, BWT_VerticalAlignment_eCenter);
    assert(pView->dynrng);
    if (plm)
    {
        pView->plm = osd_info_panel_p_populate_cell(osd, table, row, ++c, NULL, BWT_HorizontalAlignment_eCenter, BWT_VerticalAlignment_eCenter);
        assert(pView->plm);
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
    tableSettings.fgColor = osd->theme.textForegroundColor;
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
    settings.color = osd->theme.infoPanelBackgroundColor;
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
    settings.color = osd->theme.mainPanelBackgroundColor;
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

    for(i=0; i<platform_graphics_get_mosaic_count(osd->gfx); i++) {
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

OsdHandle osd_create(PlatformHandle platform, PlatformGraphicsHandle gfx, const OsdTheme * pTheme)
{
    OsdHandle osd;
    const BWT_Dimensions * mainDims;
    BWT_ToolkitCreateSettings settings;
    unsigned mainPadding;
    unsigned textHeight;

    assert(gfx);
    assert(pTheme);

    osd = malloc(sizeof(*osd));
    if (!osd) goto error;
    memset(osd, 0, sizeof(*osd));
    BWT_Toolkit_GetDefaultCreateSettings(&settings);
    settings.gfx = gfx;
    osd->bwt = BWT_Toolkit_Create(&settings);
    if (!osd->bwt) goto error;

    osd->platform = platform;
    osd->gfx = gfx;

    platform_get_default_model(&osd->model);

    memcpy(&osd->theme, pTheme, sizeof(*pTheme));

    osd->mosaicCount = platform_graphics_get_mosaic_count(gfx);

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

#if ENABLE_GUIDE
    osd->guide = osd_p_create_guide_panel(osd, mainDims, mainPadding, textHeight);
    if (!osd->guide) goto error;
    BWT_Panel_AddChild(osd->main, (BWT_WidgetHandle)osd->guide, BWT_VerticalAlignment_eTop, BWT_HorizontalAlignment_eRight);
#endif

    osd->details.base = osd_p_create_details_panel(osd, &osd->details, mainDims, textHeight, 1);
    if (!osd->details.base) goto error;
    osd->mosaicDetails.base = osd_p_create_details_panel(osd, &osd->mosaicDetails, mainDims, textHeight, osd->mosaicCount);
    if (!osd->mosaicDetails.base) goto error;
    osd->pipDetails.base = osd_p_create_details_panel(osd, &osd->pipDetails, mainDims, textHeight, 2);
    if (!osd->pipDetails.base) goto error;
    osd->info.base = osd_p_create_info_panel(osd, &osd->info, textHeight, 1);
    if (!osd->info.base) goto error;
    osd->mosaicInfo.base = osd_p_create_info_panel(osd, &osd->mosaicInfo, textHeight, osd->mosaicCount);
    if (!osd->mosaicInfo.base) goto error;
    osd->pipInfo.base = osd_p_create_info_panel(osd, &osd->pipInfo, textHeight, 2);
    if (!osd->pipInfo.base) goto error;
    BWT_Panel_AddChild(osd->main, (BWT_WidgetHandle)osd->info.base, BWT_VerticalAlignment_eBottom, BWT_HorizontalAlignment_eCenter);
    osd->current = &osd->info;

    osd->renderer = platform_scheduler_add_listener(platform_get_scheduler(platform), &osd_p_scheduler_callback, osd);
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
    if (osd->renderer)
    {
        platform_scheduler_remove_listener(platform_get_scheduler(osd->platform), osd->renderer);
    }
    BWT_Panel_Destroy(osd->main);
    if (osd->detailed)
    {
        BWT_Panel_Destroy(osd->info.base);
    }
    BWT_Toolkit_Destroy(osd->bwt);
    free(osd);
}

void osd_flip(OsdHandle osd)
{
    assert(osd);
    platform_scheduler_wake(platform_get_scheduler(osd->platform));
}

void osd_p_scheduler_callback(void * pContext, int param)
{
    OsdHandle osd = pContext;
    assert(osd);
    (void)param;
    BWT_Widget_Render((BWT_WidgetHandle)osd->main);
    BWT_Toolkit_Submit(osd->bwt);
}

void osd_toggle_visibility(OsdHandle osd)
{
    assert(osd);
    BWT_Widget_ToggleVisibility((BWT_WidgetHandle)osd->main);
}

void osd_set_visibility(OsdHandle osd, bool visible)
{
    assert(osd);
    BWT_Widget_SetVisibility((BWT_WidgetHandle)osd->main, visible);
}

void osd_toggle_details_mode(OsdHandle osd)
{
    assert(osd);
    osd_set_details_mode(osd, !osd->detailed);
}

static void osd_p_configure_info_panel_layout(OsdHandle osd, PlatformUsageMode usageMode, bool layout, bool detailed)
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
    if (detailed) {
        switch (usageMode)
        {
            case PlatformUsageMode_eMosaic:
                infoPanel = &osd->mosaicDetails;
                break;
            case PlatformUsageMode_eMainPip:
                infoPanel = &osd->pipDetails;
                break;
            default:
                infoPanel = &osd->details;
                break;
        }
    } else {
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
    }

    BWT_Widget_SetVisibility((BWT_WidgetHandle)osd->current->base, false);
    BWT_Panel_RemoveChild(osd->main, (BWT_WidgetHandle)osd->current->base);
    BWT_Panel_AddChild(osd->main, (BWT_WidgetHandle)infoPanel->base, vAlign, hAlign);
    BWT_Widget_SetVisibility((BWT_WidgetHandle)infoPanel->base, true);

    osd->current = infoPanel;
}

void osd_set_details_mode(OsdHandle osd, bool enable)
{
    osd_p_configure_info_panel_layout(osd, osd->usageMode, osd->layout, enable);
    osd->detailed = enable;
}

void osd_toggle_guide_visibility(OsdHandle osd)
{
    assert(osd);
#if ENABLE_GUIDE
    BWT_Widget_ToggleVisibility((BWT_WidgetHandle)osd->guide);
#endif
}

void osd_set_guide_visibility(OsdHandle osd, bool visible)
{
    assert(osd);
#if ENABLE_GUIDE
    BWT_Widget_SetVisibility((BWT_WidgetHandle)osd->guide, visible);
#else
    (void)visible;
#endif
}

void osd_toggle_pig_mode(OsdHandle osd)
{
}

void osd_toggle_mosaic_layout(OsdHandle osd)
{
    osd_set_usage_mode(osd, osd->usageMode, osd->layout^1);
}

static const char * const usageModeStrings[] =
{
    "full-screen video",
    "pig",
    "mosaic",
    "pip",
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
    unsigned pip = platform_graphics_get_pip_window_id(osd->gfx);
    unsigned full = platform_graphics_get_main_window_id(osd->gfx);
    unsigned pig = full;

    printf("osd: old usage: %s; new usage: %s\n", usageModeStrings[osd->usageMode], usageModeStrings[usageMode]);

    if (osd->usageMode == usageMode && osd->layout == layout) return;

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

    osd_p_configure_info_panel_layout(osd, usageMode, layout, osd->detailed);

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
}

void osd_label_p_update_dynrng(BWT_LabelHandle label, PlatformDynamicRange dynrng)
{
    assert(label);
    BWT_Label_SetText(label, platform_get_dynamic_range_name(dynrng));
}

void osd_label_p_update_gamut(BWT_LabelHandle label, PlatformColorimetry gamut)
{
    assert(label);
    BWT_Label_SetText(label, platform_get_colorimetry_name(gamut));
}

void osd_label_p_update_space(BWT_LabelHandle label, PlatformColorSpace space)
{
    assert(label);
    BWT_Label_SetText(label, platform_get_color_space_name(space));
}

void osd_label_p_update_plm(BWT_LabelHandle label, PlatformTriState plm)
{
    unsigned color;
    assert(label);
    BWT_Label_SetText(label, STR_TABLE_PLM[plm]);
    switch (plm)
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

#define OSD_UPDATE_INT_TEMPLATE(X,V,N,C,OP) \
    assert((V)->X); \
    if ((N)->X != (C)->X) \
    { \
        (C)->X = (N)->X; \
        if ((C)->X OP 0) \
        { \
            BWT_Label_SetInteger((V)->X, (C)->X); \
        } \
        else \
        { \
            BWT_Label_SetText((V)->X, UTIL_STR_NONE); \
        } \
    }

#define OSD_UPDATE_UNSIGNED_TEMPLATE(X,V,N,C) OSD_UPDATE_INT_TEMPLATE(X,V,N,C,>)

#define OSD_UPDATE_SIGNED_TEMPLATE(X,V,N,C) OSD_UPDATE_INT_TEMPLATE(X,V,N,C,>=)

#define OSD_UPDATE_ENUM_TEMPLATE(X,V,N,C) \
    assert((V)->X); \
    if ((C)->X != (N)->X) \
    { \
        (C)->X = (N)->X; \
        osd_label_p_update_##X((V)->X, (C)->X); \
    }

#define OSD_UPDATE_RCV_TEMPLATE(X,V,N,C) \
    assert((V)->X); \
    if ((C)->X != (N)->X) \
    { \
        (C)->X = (N)->X; \
        BWT_Label_SetText((V)->X, platform_get_capability_name((C)->X)); \
        switch ((C)->X) \
        { \
            case PlatformCapability_eSupported: \
                BWT_Widget_SetColor(BWT_Widget_GetParent((BWT_WidgetHandle)(V)->X), COLOR_GREEN_20); \
                break; \
            case PlatformCapability_eUnsupported: \
                BWT_Widget_SetColor(BWT_Widget_GetParent((BWT_WidgetHandle)(V)->X), COLOR_RED_20); \
                break; \
            default: \
            case PlatformCapability_eUnknown: \
                BWT_Widget_SetColor(BWT_Widget_GetParent((BWT_WidgetHandle)(V)->X), COLOR_BLUE_20); \
                break; \
        } \
    }

#define OSD_UPDATE_SEL_TEMPLATE(X,V,N,C) \
    assert((V)->X); \
    if ((C)->X != (N)->X) \
    { \
        (C)->X = (N)->X; \
        BWT_Label_SetText((V)->X, (C)->X ? STR_SEL_USER : STR_SEL_AUTO); \
    }

#define OSD_UPDATE_BOOL_TEMPLATE(X,V,N,C,OP) \
    assert((V)->X); \
    if ((C)->X != (N)->X) \
    { \
        (C)->X = (N)->X; \
        BWT_Label_SetText((V)->X, ((C)->X OP 0) ? STR_##X##_ON : STR_##X##_OFF); \
        BWT_Widget_SetColor(BWT_Widget_GetParent((BWT_WidgetHandle)(V)->X), ((C)->X OP 0) ? COLOR_GREEN_100 : COLOR_RED_100); \
    }

static unsigned osd_p_pow(unsigned x, unsigned shift)
{
    if (shift)
    {
        return x * osd_p_pow(x, shift - 1);
    }
    else
    {
        return 1;
    }
}

static unsigned osd_p_round(unsigned x, unsigned shift)
{
    unsigned q;
    unsigned r;
    unsigned s_q;

    assert(shift < 10);

    s_q = osd_p_pow(10, shift - 1);
    q = x / s_q;
    r = q % 10;
    q /= 10;
    if (r >= 5)
    {
        q++;
    }
    return q;
}

static void osd_p_update_format_view(const PlatformPictureFormat * pNewFormat, PlatformPictureFormat * pCurFormat, BWT_LabelHandle view)
{
    char format[16];
    assert(pNewFormat);
    assert(pCurFormat);
    assert(view);

    if (memcmp(pCurFormat, pNewFormat, sizeof(*pCurFormat)))
    {
        memcpy(pCurFormat, pNewFormat, sizeof(*pCurFormat));
        sprintf(format, "%ux%u", pCurFormat->width, pCurFormat->height);
        if (pCurFormat->rate)
        {
            strcat(format, pCurFormat->interlaced ? "i" : "p");
            sprintf(format + strlen(format), "%u", osd_p_round(pCurFormat->rate, 3));
            if (osd_p_round(pCurFormat->rate, 3) * osd_p_pow(10, 3) != pCurFormat->rate)
            {
                sprintf(format + strlen(format), "d");
            }
        }
        BWT_Label_SetText(view, format);
    }
}

static void osd_p_update_picture_details_view(const PlatformPictureModel * pNewModel, PlatformPictureModel * pCurModel, OsdPictureInfoView * pView, bool plm)
{
    assert(pCurModel);
    assert(pNewModel);
    assert(pView);

    osd_p_update_format_view(&pNewModel->info.format, &pCurModel->info.format, pView->format);
    OSD_UPDATE_ENUM_TEMPLATE(dynrng, pView, &pNewModel->info, &pCurModel->info);
    OSD_UPDATE_ENUM_TEMPLATE(gamut, pView, &pNewModel->info, &pCurModel->info);
#if 0
    OSD_UPDATE_ENUM_TEMPLATE(space, pView, &pNewModel->info, &pCurModel->info);
#endif
    OSD_UPDATE_UNSIGNED_TEMPLATE(depth, pView, &pNewModel->info, &pCurModel->info);
    if (plm)
    {
        OSD_UPDATE_ENUM_TEMPLATE(plm, pView, pNewModel, pCurModel);
    }
}

static void osd_p_update_picture_info_view(const PlatformPictureModel * pNewModel, PlatformPictureModel * pCurModel, OsdPictureInfoView * pView, bool plm)
{
    assert(pCurModel);
    assert(pNewModel);
    assert(pView);

    OSD_UPDATE_ENUM_TEMPLATE(dynrng, pView, &pNewModel->info, &pCurModel->info);
    if (plm)
    {
        OSD_UPDATE_ENUM_TEMPLATE(plm, pView, pNewModel, pCurModel);
    }
}

void osd_update_out_model(OsdHandle osd, const PlatformPictureModel * pModel)
{
    if (osd->detailed)
    {
        osd_p_update_picture_details_view(pModel, &osd->model.out, &osd->current->out, false);
    }
    else
    {
        osd_p_update_picture_info_view(pModel, &osd->model.out, &osd->current->out, false);
    }
}

void osd_update_gfx_model(OsdHandle osd, const PlatformPictureModel * pModel)
{
    if (osd->detailed)
    {
        osd_p_update_picture_details_view(pModel, &osd->model.gfx, &osd->current->gfx, true);
    }
    else
    {
        osd_p_update_picture_info_view(pModel, &osd->model.gfx, &osd->current->gfx, true);
    }
}

void osd_update_vid_model(OsdHandle osd, const PlatformPictureModel * pModel, unsigned index)
{
    if (osd->detailed)
    {
        osd_p_update_picture_details_view(pModel, &osd->model.vid[index], &osd->current->vid[index], true);
    }
    else
    {
        osd_p_update_picture_info_view(pModel, &osd->model.vid[index], &osd->current->vid[index], true);
    }
}

void osd_update_sel_model(OsdHandle osd, const PlatformSelectorModel * pModel)
{
    assert(osd);
    assert(pModel);
    if (osd->detailed) {
        OSD_UPDATE_SEL_TEMPLATE(format, &osd->details.sel, pModel, &osd->model.sel);
        OSD_UPDATE_SEL_TEMPLATE(dynrng, &osd->details.sel, pModel, &osd->model.sel);
        OSD_UPDATE_SEL_TEMPLATE(gamut, &osd->details.sel, pModel, &osd->model.sel);
#if 0
        OSD_UPDATE_SEL_TEMPLATE(space, &osd->details.sel, pModel, &osd->model.sel);
#endif
        OSD_UPDATE_SEL_TEMPLATE(depth, &osd->details.sel, pModel, &osd->model.sel);
    }
}

void osd_update_rcv_model(OsdHandle osd, const PlatformReceiverModel * pModel)
{
    assert(osd);
    assert(pModel);
    if (osd->detailed) {
        OSD_UPDATE_RCV_TEMPLATE(format, &osd->details.rcv, pModel, &osd->model.rcv);
        OSD_UPDATE_RCV_TEMPLATE(dynrng, &osd->details.rcv, pModel, &osd->model.rcv);
        OSD_UPDATE_RCV_TEMPLATE(gamut, &osd->details.rcv, pModel, &osd->model.rcv);
#if 0
        OSD_UPDATE_RCV_TEMPLATE(space, &osd->details.rcv, pModel, &osd->model.rcv);
#endif
        OSD_UPDATE_RCV_TEMPLATE(depth, &osd->details.rcv, pModel, &osd->model.rcv);
    }
}

void osd_update_thumbnail(OsdHandle osd, PlatformPictureHandle pic)
{
    static const char * STR_THUMBNAIL_NAME = "thumbnail";
    assert(osd);
    if (pic == osd->model.thumbnail) return;

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
}

void osd_update_background(OsdHandle osd, PlatformPictureHandle pic)
{
    static const char * STR_BACKGROUND_NAME = "background";
    assert(osd);
    if (pic == osd->model.background) return;

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
}
