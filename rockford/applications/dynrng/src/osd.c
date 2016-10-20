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
    "  BACK(LAST):  TOGGLE FORCED SDR OUTPUT (RESETS ON STREAM CHANGE)",
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

static const char * STR_GUIDE_PANEL_NAME = "guide";

BWT_PanelHandle osd_p_create_guide_panel(OsdHandle osd, const BWT_Dimensions * mainDims, unsigned mainPadding, unsigned textHeight)
{
    BWT_PanelHandle guide;
    BWT_LabelHandle label;
    unsigned textColor;
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
static const char * STR_VID = "VID";
static const char * STR_VID_LONG = "VIDEO IN";
static const char * STR_GFX = "GFX";
static const char * STR_GFX_LONG = "GRAPHICS";
static const char * STR_SEL = "SELECTOR";
static const char * STR_OUT = "OUT";
static const char * STR_OUT_LONG = "VIDEO OUT";
static const char * STR_RCV = "EDID SUPPORT";
static const char * STR_FORMAT = "FORMAT";
static const char * STR_SPACE = "CSPC";
static const char * STR_DYNRNG = "DYNRNG";
static const char * STR_GAMUT = "GAMUT";
static const char * STR_DEPTH = "BITDEPTH";
static const char * STR_PLM = "PLM";
static const char * STR_plm_ON = "PLM ON";
static const char * STR_plm_OFF = "PLM OFF";
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
static const unsigned COLOR_LIGHT_GREY_20 = 0x333f3f3f;

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

void osd_details_panel_p_populate_row(OsdHandle osd, BWT_TableHandle table, OsdPictureDetailsView * pView, unsigned r, const char * desc, bool plm)
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

static BWT_TableHandle osd_details_panel_p_create_table(OsdHandle osd, unsigned textHeight)
{
    BWT_TableHandle table;
    BWT_TableCreateSettings tableSettings;
    BWT_WidgetHandle row;
    unsigned r;

    BWT_Table_GetDefaultCreateSettings(&tableSettings);
    tableSettings.colWidth = 135;
    tableSettings.rowHeight = osd_p_compute_external_dim(textHeight, tableSettings.padding);
    tableSettings.cols = 6;
    tableSettings.rows = 6;
    tableSettings.fgColor = osd->theme.textForegroundColor;
    table = BWT_Table_Create(osd->bwt, &tableSettings);
    assert(table);

    r = 0;
    osd_details_panel_p_populate_headers(osd, table);
    osd_details_panel_p_populate_row(osd, table, &osd->details.vid, ++r, STR_VID_LONG, true);
    osd_details_panel_p_populate_row(osd, table, &osd->details.gfx, ++r, STR_GFX_LONG, true);
    osd_details_panel_p_populate_row(osd, table, &osd->details.sel, ++r, STR_SEL, false);
    osd_details_panel_p_populate_row(osd, table, &osd->details.out, ++r, STR_OUT_LONG, false);
    osd_details_panel_p_populate_row(osd, table, &osd->details.rcv, ++r, STR_RCV, false);

    return table;
}

BWT_PanelHandle osd_p_create_details_panel(OsdHandle osd, const BWT_Dimensions * mainDims, unsigned mainPadding, unsigned textHeight)
{
    BWT_PanelHandle details;
    unsigned col1x;
    unsigned col2x;
    unsigned y;
    BWT_PanelCreateSettings settings;
    const BWT_Dimensions * dims;
    BWT_TableHandle table;

    assert(mainDims);
    assert(osd);

    table = osd_details_panel_p_create_table(osd, textHeight);
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
    osd->details.table = table;

    return details;
}

/*
 * Smaller Info Panel
 * |VIDEO IN |HDR10|PLM ON | ON -> GREEN
 * |GRAPHICS |SDR  |PLM OFF| OFF -> RED
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

static BWT_TableHandle osd_info_panel_p_create_table(OsdHandle osd, unsigned textHeight)
{
    BWT_TableHandle table;
    BWT_TableCreateSettings tableSettings;
    BWT_WidgetHandle row;
    unsigned r;

    BWT_Table_GetDefaultCreateSettings(&tableSettings);
    tableSettings.colWidth = 110;
    tableSettings.rowHeight = osd_p_compute_external_dim(textHeight, tableSettings.padding);
    tableSettings.cols = 3;
    tableSettings.rows = 3;
    tableSettings.fgColor = osd->theme.textForegroundColor;
    table = BWT_Table_Create(osd->bwt, &tableSettings);
    assert(table);

    r = 0;
    osd_info_panel_p_populate_row(osd, table, &osd->info.vid,   r, STR_VID_LONG, true);
    osd_info_panel_p_populate_row(osd, table, &osd->info.gfx, ++r, STR_GFX_LONG, true);
    osd_info_panel_p_populate_row(osd, table, &osd->info.out, ++r, STR_OUT_LONG, false);

    return table;
}

BWT_PanelHandle osd_p_create_info_panel(OsdHandle osd, unsigned textHeight)
{
    BWT_PanelHandle info;
    unsigned col1x;
    unsigned col2x;
    unsigned y;
    BWT_PanelCreateSettings settings;
    const BWT_Dimensions * dims;
    BWT_TableHandle table;

    assert(osd);

    table = osd_info_panel_p_create_table(osd, textHeight);
    dims = BWT_Widget_GetDimensions((BWT_WidgetHandle)table);

    BWT_Panel_GetDefaultCreateSettings(&settings);
    settings.name = STR_INFO_PANEL_NAME;
    settings.padding = PANEL_PADDING;
    settings.spacing = 1;
    settings.dims.width = osd_p_compute_external_dim(dims->width, settings.padding);
    settings.dims.height = osd_p_compute_external_dim(dims->height, settings.padding);
    settings.color = osd->theme.infoPanelBackgroundColor;
    settings.visible = true;
    info = BWT_Panel_Create(osd->bwt, &settings);
    assert(info);

    BWT_Panel_AddChild(info, (BWT_WidgetHandle)table, BWT_VerticalAlignment_eDefault, BWT_HorizontalAlignment_eDefault);
    osd->info.table = table;

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

BWT_VideoWindowHandle osd_p_create_window(OsdHandle osd)
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
    window = BWT_VideoWindow_Create(osd->bwt, &settings);
    assert(window);

    return window;
}

OsdHandle osd_create(PlatformHandle platform, PlatformGraphicsHandle gfx, const OsdTheme * pTheme)
{
    OsdHandle osd;
    const BWT_Dimensions * mainDims;
    const BWT_Dimensions * infoDims;
    BWT_ToolkitCreateSettings settings;
    unsigned mainPadding;
    unsigned textHeight;
    static const char * bgPanelName = "bg";

    assert(gfx);
    assert(pTheme);

    osd = malloc(sizeof(*osd));
    assert(osd);
    memset(osd, 0, sizeof(*osd));
    BWT_Toolkit_GetDefaultCreateSettings(&settings);
    settings.gfx = gfx;
    osd->bwt = BWT_Toolkit_Create(&settings);
    assert(osd->bwt);

    osd->platform = platform;
    platform_get_default_model(&osd->model);

    memcpy(&osd->theme, pTheme, sizeof(*pTheme));

    osd->main = osd_p_create_main_panel(osd);
    assert(osd->main);
    mainDims = BWT_Widget_GetDimensions((BWT_WidgetHandle)osd->main);
    mainPadding = BWT_Widget_GetPadding((BWT_WidgetHandle)osd->main);
    textHeight = BWT_Toolkit_GetTextHeight(osd->bwt);

    osd->window = osd_p_create_window(osd);
    assert(osd->window);
    BWT_Panel_AddChild(osd->main, (BWT_WidgetHandle)osd->window, BWT_VerticalAlignment_eTop, BWT_HorizontalAlignment_eLeft);

    osd->guide = osd_p_create_guide_panel(osd, mainDims, mainPadding, textHeight);
    assert(osd->guide);
    BWT_Panel_AddChild(osd->main, (BWT_WidgetHandle)osd->guide, BWT_VerticalAlignment_eTop, BWT_HorizontalAlignment_eRight);

    osd->details.base = osd_p_create_details_panel(osd, mainDims, mainPadding, textHeight);
    assert(osd->details.base);
    /* start with info panel up, not details */

    osd->info.base = osd_p_create_info_panel(osd, textHeight);
    assert(osd->info.base);
    BWT_Panel_AddChild(osd->main, (BWT_WidgetHandle)osd->info.base, BWT_VerticalAlignment_eBottom, BWT_HorizontalAlignment_eCenter);

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

void osd_set_details_mode(OsdHandle osd, bool enable)
{
    if (enable)
    {
        BWT_Widget_SetVisibility((BWT_WidgetHandle)osd->info.base, false);
        BWT_Panel_RemoveChild(osd->main, (BWT_WidgetHandle)osd->info.base);
        BWT_Panel_AddChild(osd->main, (BWT_WidgetHandle)osd->details.base, BWT_VerticalAlignment_eBottom, BWT_HorizontalAlignment_eLeft);
        BWT_Widget_SetVisibility((BWT_WidgetHandle)osd->details.base, true);
    }
    else
    {
        BWT_Widget_SetVisibility((BWT_WidgetHandle)osd->details.base, false);
        BWT_Panel_RemoveChild(osd->main, (BWT_WidgetHandle)osd->details.base);
        BWT_Panel_AddChild(osd->main, (BWT_WidgetHandle)osd->info.base, BWT_VerticalAlignment_eBottom, BWT_HorizontalAlignment_eCenter);
        BWT_Widget_SetVisibility((BWT_WidgetHandle)osd->info.base, true);
    }
    osd->detailed = enable;
}

void osd_toggle_guide_visibility(OsdHandle osd)
{
    assert(osd);
    BWT_Widget_ToggleVisibility((BWT_WidgetHandle)osd->guide);
}

void osd_set_guide_visibility(OsdHandle osd, bool visible)
{
    assert(osd);
    BWT_Widget_SetVisibility((BWT_WidgetHandle)osd->guide, visible);
}

void osd_toggle_pig_mode(OsdHandle osd)
{
    osd_set_pig_mode(osd, !osd->pig);
}

void osd_set_pig_mode(OsdHandle osd, bool enable)
{
    if (enable)
    {
        if (osd->thumbnail) BWT_Widget_SetVisibility((BWT_WidgetHandle)osd->thumbnail, false);
        if (osd->background) BWT_Widget_SetVisibility((BWT_WidgetHandle)osd->background, true);
        BWT_Panel_RemoveChild(osd->main, (BWT_WidgetHandle)osd->window);
        BWT_VideoWindow_SetScale(osd->window, 30);
        BWT_Panel_AddChild(osd->main, (BWT_WidgetHandle)osd->window, BWT_VerticalAlignment_eTop, BWT_HorizontalAlignment_eLeft);
    }
    else
    {
        if (osd->background) BWT_Widget_SetVisibility((BWT_WidgetHandle)osd->background, false);
        if (osd->thumbnail) BWT_Widget_SetVisibility((BWT_WidgetHandle)osd->thumbnail, true);
        BWT_Panel_RemoveChild(osd->main, (BWT_WidgetHandle)osd->window);
        BWT_VideoWindow_SetScale(osd->window, 100);
        BWT_Panel_AddChild(osd->main, (BWT_WidgetHandle)osd->window, BWT_VerticalAlignment_eTop, BWT_HorizontalAlignment_eLeft);
    }
    osd->pig = enable;
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

static void osd_p_update_picture_details_view(const PlatformPictureModel * pNewModel, PlatformPictureModel * pCurModel, OsdPictureDetailsView * pView, bool plm)
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
        OSD_UPDATE_SIGNED_TEMPLATE(plm, pView, pNewModel, pCurModel);
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
        OSD_UPDATE_BOOL_TEMPLATE(plm, pView, pNewModel, pCurModel, >);
    }
}

void osd_update_out_model(OsdHandle osd, const PlatformPictureModel * pModel)
{
    if (osd->detailed)
    {
        osd_p_update_picture_details_view(pModel, &osd->model.out, &osd->details.out, false);
    }
    else
    {
        osd_p_update_picture_info_view(pModel, &osd->model.out, &osd->info.out, false);
    }
}

void osd_update_gfx_model(OsdHandle osd, const PlatformPictureModel * pModel)
{
    if (osd->detailed)
    {
        osd_p_update_picture_details_view(pModel, &osd->model.gfx, &osd->details.gfx, true);
    }
    else
    {
        osd_p_update_picture_info_view(pModel, &osd->model.gfx, &osd->info.gfx, true);
    }
}

void osd_update_vid_model(OsdHandle osd, const PlatformPictureModel * pModel)
{
    if (osd->detailed)
    {
        osd_p_update_picture_details_view(pModel, &osd->model.vid, &osd->details.vid, true);
    }
    else
    {
        osd_p_update_picture_info_view(pModel, &osd->model.vid, &osd->info.vid, true);
    }
}

void osd_update_sel_model(OsdHandle osd, const PlatformSelectorModel * pModel)
{
    assert(osd);
    assert(pModel);
    OSD_UPDATE_SEL_TEMPLATE(format, &osd->details.sel, pModel, &osd->model.sel);
    OSD_UPDATE_SEL_TEMPLATE(dynrng, &osd->details.sel, pModel, &osd->model.sel);
    OSD_UPDATE_SEL_TEMPLATE(gamut, &osd->details.sel, pModel, &osd->model.sel);
#if 0
    OSD_UPDATE_SEL_TEMPLATE(space, &osd->details.sel, pModel, &osd->model.sel);
#endif
    OSD_UPDATE_SEL_TEMPLATE(depth, &osd->details.sel, pModel, &osd->model.sel);
}

void osd_update_rcv_model(OsdHandle osd, const PlatformReceiverModel * pModel)
{
    assert(osd);
    assert(pModel);
    OSD_UPDATE_RCV_TEMPLATE(format, &osd->details.rcv, pModel, &osd->model.rcv);
    OSD_UPDATE_RCV_TEMPLATE(dynrng, &osd->details.rcv, pModel, &osd->model.rcv);
    OSD_UPDATE_RCV_TEMPLATE(gamut, &osd->details.rcv, pModel, &osd->model.rcv);
#if 0
    OSD_UPDATE_RCV_TEMPLATE(space, &osd->details.rcv, pModel, &osd->model.rcv);
#endif
    OSD_UPDATE_RCV_TEMPLATE(depth, &osd->details.rcv, pModel, &osd->model.rcv);
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
        settings.visible = !osd->pig;
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
        settings.visible = osd->pig;
        osd->background = BWT_Image_Create(osd->bwt, &settings);
        assert(osd->background);
        BWT_Panel_SetBackground(osd->main, osd->background);
    }
}
