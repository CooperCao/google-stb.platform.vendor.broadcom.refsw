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
#ifndef BWT_H__
#define BWT_H__ 1

#include "platform_types.h"
#include "blst_queue.h"
#include <stdbool.h>

#define BWT_RELATIVE_BASE 1000

typedef struct BWT_Point
{
    int x;
    int y;
} BWT_Point;

typedef struct BWT_Dimensions
{
    unsigned width;
    unsigned height;
} BWT_Dimensions;

typedef struct BWT_Toolkit * BWT_ToolkitHandle;
typedef struct BWT_Widget * BWT_WidgetHandle;
typedef void (*BWT_Renderer)(BWT_WidgetHandle);
typedef void (*BWT_Positioner)(BWT_WidgetHandle);
typedef void (*BWT_Destructor)(void *);
typedef struct BWT_Image * BWT_ImageHandle;
typedef struct BWT_Label * BWT_LabelHandle;
typedef struct BWT_Table * BWT_TableHandle;
typedef struct BWT_VideoWindow * BWT_VideoWindowHandle;
typedef struct BWT_Panel * BWT_PanelHandle;
typedef BLST_Q_HEAD(BWT_WidgetList, BWT_Widget) BWT_WidgetList;

typedef enum BWT_WidgetType
{
    BWT_WidgetType_eVideoWindow,
    BWT_WidgetType_eTable,
    BWT_WidgetType_ePanel,
    BWT_WidgetType_eImage,
    BWT_WidgetType_eLabel,
    BWT_WidgetType_eMax
} BWT_WidgetType;

typedef enum BWT_LayoutFlow
{
    BWT_LayoutFlow_eVertical,
    BWT_LayoutFlow_eDefault = BWT_LayoutFlow_eVertical,
    BWT_LayoutFlow_eHorizontal,
    BWT_LayoutFlow_eMax
} BWT_LayoutFlow;

typedef enum BWT_HorizontalAlignment
{
    BWT_HorizontalAlignment_eLeft,
    BWT_HorizontalAlignment_eDefault = BWT_HorizontalAlignment_eLeft,
    BWT_HorizontalAlignment_eCenter,
    BWT_HorizontalAlignment_eRight,
    BWT_HorizontalAlignment_eMax
} BWT_HorizontalAlignment;

typedef enum BWT_VerticalAlignment
{
    BWT_VerticalAlignment_eTop,
    BWT_VerticalAlignment_eDefault = BWT_VerticalAlignment_eTop,
    BWT_VerticalAlignment_eCenter,
    BWT_VerticalAlignment_eBottom,
    BWT_VerticalAlignment_eMax
} BWT_VerticalAlignment;

typedef struct BWT_WidgetInitSettings
{
    bool visible;
    const char * name;
    BWT_WidgetType type;
    BWT_Dimensions dims;
    unsigned color;
    unsigned spacing;
    unsigned padding;
    BWT_Renderer render;
    BWT_Destructor destroy;
    BWT_Positioner place;
} BWT_WidgetInitSettings;

/* base widget class */
void BWT_Widget_GetDefaultInitSettings(BWT_WidgetInitSettings * psettings);
void BWT_Widget_InitBase(BWT_WidgetHandle w, BWT_ToolkitHandle bwt, const BWT_WidgetInitSettings * psettings);
void BWT_Widget_UninitBase(BWT_WidgetHandle w);
void BWT_Widget_PlaceBase(BWT_WidgetHandle w);
void BWT_Widget_Destroy(BWT_WidgetHandle w);
void BWT_Widget_Render(BWT_WidgetHandle w);
void BWT_Widget_Place(BWT_WidgetHandle w);
void BWT_Widget_SetVisibility(BWT_WidgetHandle w, bool visible);
void BWT_Widget_SetColor(BWT_WidgetHandle w, unsigned color);
void BWT_Widget_ToggleVisibility(BWT_WidgetHandle w);
const BWT_Dimensions * BWT_Widget_GetDimensions(BWT_WidgetHandle w);
const BWT_Point * BWT_Widget_GetPos(BWT_WidgetHandle w);
unsigned BWT_Widget_GetPadding(BWT_WidgetHandle widget);
BWT_WidgetHandle BWT_Widget_GetParent(BWT_WidgetHandle widget);
bool BWT_Widget_IsVisible(BWT_WidgetHandle w);

/* subclasses */

typedef struct BWT_ImageCreateSettings
{
    bool visible;
    const char * name;
    PlatformPictureHandle pic;
} BWT_ImageCreateSettings;

void BWT_Image_GetDefaultCreateSettings(BWT_ImageCreateSettings * pSettings);
BWT_ImageHandle BWT_Image_Create(BWT_ToolkitHandle bwt, const BWT_ImageCreateSettings * pSettings);
void BWT_Image_Render(BWT_WidgetHandle w);
void BWT_Image_Destroy(void * v);

typedef struct BWT_PanelCreateSettings
{
    bool visible;
    const char * name;
    BWT_Dimensions dims;
    unsigned padding;
    unsigned spacing;
    unsigned color;
    BWT_ImageHandle background;
    BWT_LayoutFlow flow;
} BWT_PanelCreateSettings;

typedef struct BWT_PanelInitSettings
{
    BWT_WidgetInitSettings widget;
    BWT_ImageHandle background;
    BWT_LayoutFlow flow;
} BWT_PanelInitSettings;

void BWT_Panel_GetDefaultCreateSettings(BWT_PanelCreateSettings * pSettings);
BWT_PanelHandle BWT_Panel_Create(BWT_ToolkitHandle bwt, const BWT_PanelCreateSettings * pSettings);
void BWT_Panel_Destroy(void * v);
void BWT_Panel_GetDefaultInitSettings(BWT_PanelInitSettings * pSettings);
void BWT_Panel_InitBase(BWT_PanelHandle panel, BWT_ToolkitHandle bwt, const BWT_PanelInitSettings * pSettings);
void BWT_Panel_UninitBase(BWT_PanelHandle p);
void BWT_Panel_Render(BWT_WidgetHandle w);
void BWT_Panel_Place(BWT_WidgetHandle w);
void BWT_Panel_SetBackground(BWT_PanelHandle p, BWT_ImageHandle background);
void BWT_Panel_AddChild(BWT_PanelHandle panel, BWT_WidgetHandle child, BWT_VerticalAlignment valign, BWT_HorizontalAlignment halign);
void BWT_Panel_RemoveChild(BWT_PanelHandle panel, BWT_WidgetHandle child);

typedef struct BWT_LabelCreateSettings
{
    bool visible;
    const char * name;
    const char * text;
    unsigned color;
    BWT_Dimensions dims;
    BWT_HorizontalAlignment halign;
    BWT_VerticalAlignment valign;
} BWT_LabelCreateSettings;

void BWT_Label_GetDefaultCreateSettings(BWT_LabelCreateSettings * pSettings);
BWT_LabelHandle BWT_Label_Create(BWT_ToolkitHandle bwt, const BWT_LabelCreateSettings * pSettings);
void BWT_Label_Destroy(void * v);
void BWT_Label_Render(BWT_WidgetHandle w);
void BWT_Label_SetText(BWT_LabelHandle label, const char * text);
void BWT_Label_SetInteger(BWT_LabelHandle label, int number);

typedef struct BWT_TableCreateSettings
{
    const char * name;
    unsigned padding;
    unsigned spacing;
    unsigned rowHeight;
    unsigned colWidth;
    unsigned rows;
    unsigned cols;
    unsigned bgColor;
    unsigned fgColor;
} BWT_TableCreateSettings;

typedef struct BWT_TableInitSettings
{
    BWT_PanelInitSettings panel;
} BWT_TableInitSettings;

void BWT_Table_GetDefaultCreateSettings(BWT_TableCreateSettings * pSettings);
BWT_TableHandle BWT_Table_Create(BWT_ToolkitHandle bwt, const BWT_TableCreateSettings * pSettings);
void BWT_Table_Destroy(void * v);
void BWT_Table_GetDefaultInitSettings(BWT_TableInitSettings * pSettings);
void BWT_Table_InitBase(BWT_TableHandle table, BWT_ToolkitHandle bwt, const BWT_TableInitSettings * pSettings);
void BWT_Table_UninitBase(BWT_TableHandle table);
void BWT_Table_Render(BWT_WidgetHandle w);
void BWT_Table_Place(BWT_WidgetHandle w);
void BWT_Table_Print(BWT_TableHandle table);
BWT_WidgetHandle BWT_Table_GetRow(BWT_TableHandle table, unsigned row);
BWT_WidgetHandle BWT_Table_GetCell(BWT_TableHandle table, BWT_WidgetHandle row, unsigned col);

typedef struct BWT_VideoWindowCreateSettings
{
    bool visible;
    const char * name;
    unsigned scale;
    BWT_Dimensions dims;
    unsigned id;
} BWT_VideoWindowCreateSettings;

void BWT_VideoWindow_GetDefaultCreateSettings(BWT_VideoWindowCreateSettings * pSettings);
BWT_VideoWindowHandle BWT_VideoWindow_Create(BWT_ToolkitHandle bwt, const BWT_VideoWindowCreateSettings * pSettings);
void BWT_VideoWindow_Destroy(void * v);
void BWT_VideoWindow_Render(BWT_WidgetHandle w);
void BWT_VideoWindow_Place(BWT_WidgetHandle w);
void BWT_VideoWindow_SetScale(BWT_VideoWindowHandle window, unsigned scale);

/* toolkit */

typedef struct BWT_ToolkitCreateSettings
{
    PlatformGraphicsHandle gfx;
} BWT_ToolkitCreateSettings;

void BWT_Toolkit_GetDefaultCreateSettings(BWT_ToolkitCreateSettings * pSettings);
BWT_ToolkitHandle BWT_Toolkit_Create(const BWT_ToolkitCreateSettings * pSettings);
void BWT_Toolkit_Destroy(BWT_ToolkitHandle bwt);
unsigned BWT_Toolkit_GetTextHeight(BWT_ToolkitHandle bwt);
const BWT_Dimensions * BWT_Toolkit_GetFramebufferDimensions(BWT_ToolkitHandle bwt);
void BWT_Toolkit_Submit(BWT_ToolkitHandle bwt);

#endif /* BWT_H__ */
