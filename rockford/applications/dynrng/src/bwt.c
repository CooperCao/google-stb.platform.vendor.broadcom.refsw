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
#include "bwt.h"
#include "bwt_priv.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define DEBUG 0

static const char * BWT_WIDGET_NAME_NONE = "<unnamed widget>";
static const char * BWT_LABEL_NAME_NONE = "<unnamed label>";
static const char * BWT_PANEL_NAME_NONE = "<unnamed panel>";
static const char * BWT_TABLE_NAME_NONE = "<unnamed table>";
static const char * BWT_TABLE_ROW_NAME_NONE = "<unnamed table row>";
static const char * BWT_TABLE_COL_NAME_NONE = "<unnamed table col>";
static const char * BWT_IMAGE_NAME_NONE = "<unnamed image>";
static const char * BWT_VIDEO_WINDOW_NAME_NONE = "<unnamed video window>";

static const char * widgetTypeNames[] =
{
    "table",
    "panel",
    "image",
    "label",
    NULL
};

static const char * BWT_Widget_P_GetTypeName(BWT_WidgetType type)
{
    return widgetTypeNames[type];
}

void BWT_Widget_GetDefaultInitSettings(BWT_WidgetInitSettings * pSettings)
{
    memset(pSettings, 0, sizeof(*pSettings));
    pSettings->visible = true;
    pSettings->name = BWT_WIDGET_NAME_NONE;
    pSettings->type = BWT_WidgetType_eMax;
    pSettings->place = &BWT_Widget_PlaceBase;
}

static unsigned BWT_Widget_P_ConvertToPixels(unsigned qty, unsigned relTo, unsigned base)
{
    unsigned pxl = qty * relTo / base;
    if (!pxl && qty) pxl = 1;
    return pxl;
}

void BWT_Widget_InitBase(
    BWT_WidgetHandle w,
    BWT_ToolkitHandle bwt,
    const BWT_WidgetInitSettings * pSettings)
{
    assert(w);
    assert(bwt);
    assert(pSettings);
    assert(pSettings->name);
    assert(pSettings->type != BWT_WidgetType_eMax);
    memset(w, 0, sizeof(*w));
    w->name = malloc(strlen(pSettings->name) + 1);
    assert(w->name);
    strcpy(w->name, pSettings->name);
    w->bwt = bwt;
    w->color = pSettings->color;
    w->spacing = pSettings->spacing;
    w->padding.rel = pSettings->padding;
    w->visible = pSettings->visible;
    w->dims = pSettings->dims;
    w->padding.abs = BWT_Widget_P_ConvertToPixels(w->padding.rel, w->dims.height > w->dims.width ? w->dims.width : w->dims.height, BWT_RELATIVE_BASE);
    w->iface.type = pSettings->type;
    w->iface.destroy = pSettings->destroy;
    w->iface.render = pSettings->render;
    w->iface.place = pSettings->place;
#if DEBUG
    printf(" '%s':", w->name);
    if (w->dims.width || w->dims.height)
    {
        printf(" %ux%u", w->dims.width, w->dims.height);
    }
    printf(" color 0x%08x spacing %u px", w->color, w->spacing);
#endif
    return;
}

void BWT_Widget_UninitBase(BWT_WidgetHandle w)
{
    if (!w) return;
    if (w->name) free(w->name);
}

void BWT_Widget_Destroy(BWT_WidgetHandle w)
{
    if (!w) return;
    if (!w->iface.destroy) return;
    w->iface.destroy(w);
}

void BWT_Widget_CheckBounds(BWT_WidgetHandle w, BWT_WidgetHandle relTo, const char * msg)
{
    if (w->pos.x < relTo->pos.x)
    {
        printf("widget '%s' extends %s in -x axis: %d < %d\n", w->name, msg, w->pos.x, relTo->pos.x);
    }
    if (relTo->dims.width && (w->pos.x + w->dims.width > relTo->pos.x + relTo->dims.width))
    {
        printf("widget '%s' extends %s in +x axis: %d > %d\n", w->name, msg, w->pos.x + w->dims.width, relTo->pos.x + relTo->dims.width);
    }
    if (w->pos.y < relTo->pos.y)
    {
        printf("widget '%s' extends %s in -y ayis: %d < %d\n", w->name, msg, w->pos.y, relTo->pos.y);
    }
    if (relTo->dims.height && (w->pos.y + w->dims.height > relTo->pos.y + relTo->dims.height))
    {
        printf("widget '%s' extends %s in +y ayis: %d > %d\n", w->name, msg, w->pos.y + w->dims.height, relTo->pos.y + relTo->dims.height);
    }
}

void BWT_Widget_PlaceBase(BWT_WidgetHandle w)
{
    BWT_WidgetHandle parent;
    BWT_WidgetHandle sibling;
    BWT_WidgetHandle root;

    assert(w);

    if (w->parent)
    {
        BWT_WidgetHandle relTo;
        int xdims;
        int ydims;
        int xoffset;
        int yoffset;

        parent = (BWT_WidgetHandle)w->parent;
        sibling = BLST_Q_PREV(w, link);
        xoffset = yoffset = 0;

        if (sibling)
        {
            relTo = sibling;
            xoffset = parent->spacing;
            yoffset = parent->spacing;
            xdims = relTo->dims.width;
            ydims = relTo->dims.height;
            switch (w->valign)
            {
                case BWT_VerticalAlignment_eCenter:
                    ydims /= 2; /* TODO */
                    break;
                case BWT_VerticalAlignment_eBottom:
                    ydims = -w->dims.height;
                    break;
                case BWT_VerticalAlignment_eTop:
                default:
                    break;
            }
            switch (w->halign)
            {
                case BWT_HorizontalAlignment_eCenter:
                    xdims /= 2; /* TODO */
                    break;
                case BWT_HorizontalAlignment_eRight:
                    xdims = -w->dims.width;
                    break;
                case BWT_HorizontalAlignment_eLeft:
                default:
                    break;
            }
            switch (w->parent->flow)
            {
                case BWT_LayoutFlow_eHorizontal:
                    ydims = 0;
                    yoffset = 0;
                    break;
                case BWT_LayoutFlow_eVertical:
                    xdims = 0;
                    xoffset = 0;
                    break;
                default:
                    break;
            }
        }
        else
        {
            relTo = parent;
            /* only apply offsets if child does not fill parent */
            if (w->dims.width < relTo->dims.width)
            {
                xoffset = parent->padding.abs;
            }
            if (w->dims.height < relTo->dims.height)
            {
                yoffset = parent->padding.abs;
            }
            xdims = 0;
            ydims = 0;
            switch (w->valign)
            {
                case BWT_VerticalAlignment_eCenter:
                    ydims = (int)relTo->dims.height/2 - (int)w->dims.height/2;
                    break;
                case BWT_VerticalAlignment_eBottom:
                    ydims = (int)relTo->dims.height - (int)w->dims.height;
                    break;
                case BWT_VerticalAlignment_eTop:
                default:
                    break;
            }
            switch (w->halign)
            {
                case BWT_HorizontalAlignment_eCenter:
                    xdims = (int)relTo->dims.width/2 - (int)w->dims.width/2;
                    break;
                case BWT_HorizontalAlignment_eRight:
                    xdims = (int)relTo->dims.width - (int)w->dims.width;
                    break;
                case BWT_HorizontalAlignment_eLeft:
                default:
                    break;
            }
        }

        switch (w->valign)
        {
            case BWT_VerticalAlignment_eCenter:
                yoffset = 0;
                break;
            case BWT_VerticalAlignment_eBottom:
                yoffset = -yoffset;
                break;
            case BWT_VerticalAlignment_eTop:
            default:
                break;
        }
        switch (w->halign)
        {
            case BWT_HorizontalAlignment_eCenter:
                xoffset = 0;
                break;
            case BWT_HorizontalAlignment_eRight:
                xoffset = -xoffset;
                break;
            case BWT_HorizontalAlignment_eLeft:
            default:
                break;
        }

        w->pos.x = relTo->pos.x + xdims + xoffset;
        w->pos.y = relTo->pos.y + ydims + yoffset;

        BWT_Widget_CheckBounds(w, parent, "ex loco parentis");
    }
    else
    {
        w->pos.x = 0;
        w->pos.y = 0;
    }

#if DEBUG
    printf("widget '%s': pos (%d, %d)\n", w->name, w->pos.x, w->pos.y);
#endif

    if (w->parent)
    {
        root = (BWT_WidgetHandle)w->parent;
        while (root->parent)
        {
            root = (BWT_WidgetHandle)root->parent;
        }
    }
    else
    {
        root = w; /* i *am* at the root */
    }

    BWT_Widget_CheckBounds(w, root, "off-screen");
}

void BWT_Widget_Place(BWT_WidgetHandle w)
{
    assert(w);
    if (!w->iface.place) return;
    w->iface.place(w);
}

BWT_WidgetHandle BWT_Widget_GetParent(BWT_WidgetHandle w)
{
    assert(w);
    return (BWT_WidgetHandle)w->parent;
}

void BWT_Widget_Render(BWT_WidgetHandle w)
{
    assert(w);
    if (!w->iface.render) return;
    w->iface.render(w);
}

void BWT_Widget_SetColor(BWT_WidgetHandle w, unsigned color)
{
    assert(w);
#if DEBUG
    if (w->color != color)
    {
        printf("widget '%s' color = 0x%08x\n", w->name, w->color);
    }
#endif
    w->color = color;
}

void BWT_Widget_SetVisibility(BWT_WidgetHandle w, bool visible)
{
    assert(w);
#if DEBUG
    if ((!w->visible && visible) || (w->visible && !visible))
    {
        printf("widget '%s': %s\n", w->name, w->visible ? "visible" : "hidden");
    }
#endif
    w->visible = visible;
}

void BWT_Widget_ToggleVisibility(BWT_WidgetHandle w)
{
    assert(w);
    BWT_Widget_SetVisibility(w, !w->visible);
}

const BWT_Dimensions * BWT_Widget_GetDimensions(BWT_WidgetHandle w)
{
    assert(w);
    return &w->dims;
}

const BWT_Point * BWT_Widget_GetPos(BWT_WidgetHandle w)
{
    assert(w);
    return &w->pos;
}

unsigned BWT_Widget_GetPadding(BWT_WidgetHandle widget)
{
    assert(widget);
    return widget->padding.rel;
}

bool BWT_Widget_IsVisible(BWT_WidgetHandle w)
{
    assert(w);
    return w->visible;
}

static const char halignNames[] =
{
    'l',
    'c',
    'r',
    0
};

static const char valignNames[] =
{
    't',
    'c',
    'b',
    0
};

void BWT_Panel_P_VisitChildGroups(BWT_PanelHandle panel, BWT_Panel_WidgetListVisitor visit, void * pContext)
{
    unsigned i;
    unsigned j;

    assert(panel);

    for (i = 0; i < BWT_VerticalAlignment_eMax; i++)
    {
        for (j = 0; j < BWT_HorizontalAlignment_eMax; j++)
        {
            if (visit) visit(pContext, &panel->children[i][j], i, j);
        }
    }
}

static void BWT_Panel_P_PrintChildren(void * pContext, BWT_WidgetList * pChildren, BWT_VerticalAlignment valign, BWT_HorizontalAlignment halign)
{
    BWT_WidgetHandle w;
    char sep = *(char *)pContext;

    printf("%c%c { ", valignNames[valign], halignNames[halign]);
    for (w = BLST_Q_FIRST(pChildren); w; w = BLST_Q_NEXT(w, link))
    {
        printf("%s%c", BWT_Widget_P_GetTypeName(w->iface.type), sep);
    }
    printf("} ");
}

static void BWT_Panel_P_PrintChildGroups(BWT_PanelHandle panel)
{
    char sep;
    assert(panel);
    sep = panel->flow == BWT_LayoutFlow_eHorizontal ? ' ' : '\n';
    BWT_Panel_P_VisitChildGroups(panel, &BWT_Panel_P_PrintChildren, (void *)&sep);
}

static BWT_WidgetList * BWT_Panel_P_GetChildGroup(BWT_PanelHandle panel, BWT_VerticalAlignment valign, BWT_HorizontalAlignment halign)
{
    assert(valign < BWT_VerticalAlignment_eMax);
    assert(halign < BWT_HorizontalAlignment_eMax);
    return &panel->children[valign][halign];
}

static BWT_WidgetList * BWT_Panel_P_GetDefaultChildGroup(BWT_PanelHandle panel)
{
    return BWT_Panel_P_GetChildGroup(panel, BWT_VerticalAlignment_eDefault, BWT_HorizontalAlignment_eDefault);
}

void BWT_Panel_GetDefaultCreateSettings(BWT_PanelCreateSettings * pSettings)
{
    memset(pSettings, 0, sizeof(*pSettings));
    pSettings->visible = true;
    pSettings->padding = 1;
    pSettings->spacing = 1;
}

BWT_PanelHandle BWT_Panel_Create(BWT_ToolkitHandle bwt, const BWT_PanelCreateSettings * pSettings)
{
    BWT_PanelHandle panel;
    BWT_PanelInitSettings initSettings;

    assert(bwt);
    panel = malloc(sizeof(*panel));
    assert(panel);
    memset(panel, 0, sizeof(*panel));
    BWT_Panel_GetDefaultInitSettings(&initSettings);
    if (pSettings->name) initSettings.widget.name = pSettings->name;
    initSettings.widget.visible = pSettings->visible;
    memcpy(&initSettings.widget.dims, &pSettings->dims, sizeof(initSettings.widget.dims));
    initSettings.widget.color = pSettings->color;
    initSettings.widget.padding = pSettings->padding;
    initSettings.widget.spacing = pSettings->spacing;
    initSettings.flow = pSettings->flow;
    BWT_Panel_InitBase(panel, bwt, &initSettings);
    return panel;
}

static void BWT_Panel_P_DestroyChildren(void * pContext, BWT_WidgetList * pChildren, BWT_VerticalAlignment valign, BWT_HorizontalAlignment halign)
{
    BWT_WidgetHandle child;

    for (child = BLST_Q_FIRST(pChildren); child; child = BLST_Q_FIRST(pChildren))
    {
        BLST_Q_REMOVE(pChildren, child, link);
        BWT_Widget_Destroy(child);
    }
}

static void BWT_Panel_P_Destroy(BWT_PanelHandle panel)
{
    BWT_WidgetHandle child;
    if (!panel) return;
    BWT_Panel_P_VisitChildGroups(panel, &BWT_Panel_P_DestroyChildren, NULL);
    BWT_Panel_UninitBase(panel);
    free(panel);
}

void BWT_Panel_Destroy(void * v)
{
    BWT_Panel_P_Destroy(v);
}

void BWT_Panel_GetDefaultInitSettings(BWT_PanelInitSettings * pSettings)
{
    memset(pSettings, 0, sizeof(*pSettings));
    BWT_Widget_GetDefaultInitSettings(&pSettings->widget);
    pSettings->widget.name = BWT_PANEL_NAME_NONE;
    pSettings->widget.type = BWT_WidgetType_ePanel;
    pSettings->widget.render = &BWT_Panel_Render;
    pSettings->widget.destroy = &BWT_Panel_Destroy;
    pSettings->widget.place = &BWT_Panel_Place;
    pSettings->widget.spacing = 1;
    pSettings->widget.padding = 1;
    pSettings->flow = BWT_LayoutFlow_eDefault;
}

static void BWT_Panel_P_InitChildGroup(void * pContext, BWT_WidgetList * pChildren, BWT_VerticalAlignment valign, BWT_HorizontalAlignment halign)
{
    BLST_Q_INIT(pChildren);
}

void BWT_Panel_InitBase(
    BWT_PanelHandle panel,
    BWT_ToolkitHandle bwt,
    const BWT_PanelInitSettings * pSettings)
{
    const char * name;
    assert(panel);
    assert(bwt);
    memset(panel, 0, sizeof(*panel));
#if DEBUG
    printf("panel");
#endif
    BWT_Widget_InitBase((BWT_WidgetHandle)panel, bwt, &pSettings->widget);
    BWT_Panel_P_VisitChildGroups(panel, &BWT_Panel_P_InitChildGroup, NULL);
    panel->flow = pSettings->flow;
#if DEBUG
    printf(" padding %u px\n", panel->widget.padding);
#endif
    return;
}

void BWT_Panel_UninitBase(BWT_PanelHandle panel)
{
    if (!panel) return;
    BWT_Widget_UninitBase((BWT_WidgetHandle)panel);
}

static void BWT_Panel_P_PlaceChildren(void * pContext, BWT_WidgetList * pChildren, BWT_VerticalAlignment valign, BWT_HorizontalAlignment halign)
{
    BWT_WidgetHandle child;
    for (child = BLST_Q_FIRST(pChildren); child; child = BLST_Q_NEXT(child, link))
    {
        BWT_Widget_Place(child);
    }
}

static void BWT_Panel_P_PlaceChildGroups(BWT_PanelHandle panel)
{
    assert(panel);
    BWT_Panel_P_VisitChildGroups(panel, &BWT_Panel_P_PlaceChildren, NULL);
}

void BWT_Panel_Place(BWT_WidgetHandle w)
{
    BWT_Widget_PlaceBase(w);
    BWT_Panel_P_PlaceChildGroups((BWT_PanelHandle)w);
}

static void BWT_Panel_P_RenderChildren(void * pContext, BWT_WidgetList * pChildren, BWT_VerticalAlignment valign, BWT_HorizontalAlignment halign)
{
    BWT_WidgetHandle child;
    for (child = BLST_Q_FIRST(pChildren); child; child = BLST_Q_NEXT(child, link))
    {
        BWT_Widget_Render(child);
    }
}

void BWT_Panel_Render(BWT_WidgetHandle w)
{
    BWT_PanelHandle panel = (BWT_PanelHandle)w;
    PlatformRect r;
    unsigned fillColor = 0;

    assert(panel);

    /* clear root, even if disabled */
    if (!w->parent)
    {
        /* fill with bg color now */
        r.width = w->dims.width;
        r.height = w->dims.height;
        r.x = w->pos.x;
        r.y = w->pos.y;
        platform_graphics_clear(w->bwt->gfx, &r);
    }

    if (!BWT_Widget_IsVisible(w)) return;

    if (panel->background)
    {
        /* render bg image */
        BWT_Widget_Render((BWT_WidgetHandle)panel->background);
    }
    else
    {
        /* fill with bg color now */
        r.width = w->dims.width;
        r.height = w->dims.height;
        r.x = w->pos.x;
        r.y = w->pos.y;
        if (BWT_WIDGET_IS_OPAQUE(w))
        {
            platform_graphics_fill(w->bwt->gfx, &r, w->color);
        }
        else
        {
            platform_graphics_blend(w->bwt->gfx, &r, w->color);
        }
    }
    BWT_Panel_P_VisitChildGroups(panel, &BWT_Panel_P_RenderChildren, NULL);
}

void BWT_Panel_SetBackground(BWT_PanelHandle panel, BWT_ImageHandle background)
{
    assert(panel);
    panel->background = background;
}

void BWT_Panel_AddChild(BWT_PanelHandle panel, BWT_WidgetHandle child, BWT_VerticalAlignment valign, BWT_HorizontalAlignment halign)
{
    assert(panel);
    assert(child);

    if (child->parent && (child->parent != panel))
    {
        printf("warning: widget '%s' already child of panel '%s'. panel '%s' will adopt\n",
            child->name, child->parent->widget.name, panel->widget.name);
    }
    else if (child->parent && (child->parent == panel)) return; /* already child of this panel */
    BLST_Q_INSERT_TAIL(BWT_Panel_P_GetChildGroup(panel, valign, halign), child, link);
    child->parent = panel;
    memset(&child->pos, 0, sizeof(child->pos));
    child->valign = valign;
    child->halign = halign;
#if DEBUG
    printf("panel '%s': child '%s' added to %c%c region\n",
        panel->widget.name,
        child->name,
        valignNames[child->valign],
        halignNames[child->halign]);
#endif
    BWT_Widget_Place(child);
}

void BWT_Panel_RemoveChild(BWT_PanelHandle panel, BWT_WidgetHandle child)
{
    assert(panel);
    assert(child);
    if (child->parent && (panel == child->parent))
    {
        BLST_Q_REMOVE(BWT_Panel_P_GetChildGroup(panel, child->valign, child->halign), child, link);
        child->parent = NULL;
        memset(&child->pos, 0, sizeof(child->pos));
    }
    else
    {
        printf("warning: widget '%s' was not a child of any panel\n", child->name);
    }
}

void BWT_Label_GetDefaultCreateSettings(BWT_LabelCreateSettings * pSettings)
{
    memset(pSettings, 0, sizeof(*pSettings));
    pSettings->visible = true;
}

static void BWT_Label_P_GetDefaultWidgetInitSettings(BWT_WidgetInitSettings * pSettings)
{
    BWT_Widget_GetDefaultInitSettings(pSettings);
    pSettings->name = BWT_LABEL_NAME_NONE;
    pSettings->type = BWT_WidgetType_eLabel;
    pSettings->render = &BWT_Label_Render;
    pSettings->destroy = &BWT_Label_Destroy;
}

static unsigned BWT_Label_P_ValidateDimension(BWT_LabelHandle label, const char * dimName, unsigned textVal, unsigned labelVal)
{
    unsigned newVal = labelVal;
    if (textVal > labelVal)
    {
        if (labelVal)
        {
            printf("label '%s': warning: text %s %u > label %s %u; expanding\n",
                label->widget.name, dimName, textVal, dimName, labelVal);
        }
        newVal = textVal;
    }
    return newVal;
}

BWT_LabelHandle BWT_Label_Create(BWT_ToolkitHandle bwt, const BWT_LabelCreateSettings * pSettings)
{
    BWT_LabelHandle label;
    BWT_WidgetInitSettings widgetSettings;
    unsigned textHeight;

    assert(bwt);
    label = malloc(sizeof(*label));
    assert(label);
    memset(label, 0, sizeof(*label));
    BWT_Label_P_GetDefaultWidgetInitSettings(&widgetSettings);
    if (pSettings->name) widgetSettings.name = pSettings->name;
    widgetSettings.color = pSettings->color;
    widgetSettings.visible = pSettings->visible;
    memcpy(&widgetSettings.dims, &pSettings->dims, sizeof(widgetSettings.dims));
    textHeight = platform_graphics_get_text_height(bwt->gfx);
    widgetSettings.dims.height = BWT_Label_P_ValidateDimension(label, "height", textHeight, widgetSettings.dims.height);
#if DEBUG
    printf("label");
#endif
    BWT_Widget_InitBase((BWT_WidgetHandle)label, bwt, &widgetSettings);
    label->halign = pSettings->halign;
    label->valign = pSettings->valign;
    BWT_Label_SetText(label, pSettings->text);
#if DEBUG
    printf(" text '%s' h%u v%u\n", label->text, label->halign, label->valign);
#endif
    return label;
}

static void BWT_Label_P_Destroy(BWT_LabelHandle label)
{
    if (!label) return;
    if (label->text) free(label->text);
    BWT_Widget_UninitBase((BWT_WidgetHandle)label);
    free(label);
}

void BWT_Label_Destroy(void * v)
{
    BWT_Label_P_Destroy(v);
}

void BWT_Label_Render(BWT_WidgetHandle w)
{
    BWT_LabelHandle label = (BWT_LabelHandle)w;
    PlatformTextRenderingSettings settings;
    assert(w);
    assert(w->bwt);
    assert(w->bwt->gfx);
    if (!label->text || !BWT_Widget_IsVisible(w)) return;

    platform_graphics_get_default_text_rendering_settings(&settings);
    settings.rect.x = w->pos.x;
    settings.rect.y = w->pos.y;
    settings.rect.width = w->dims.width;
    settings.rect.height = w->dims.height;
    settings.color = w->color;
    settings.halign = (PlatformHorizontalAlignment)label->halign;
    settings.valign = (PlatformVerticalAlignment)label->valign;
#if DEBUG
    printf("Rendering '%s' in 0x%08x dims %ux%u @ (%u, %u) h%u v%u\n",
        label->text, settings.color, settings.rect.width,
        settings.rect.height, settings.rect.x, settings.rect.y,
        settings.halign, settings.valign);
#endif
    platform_graphics_render_text(w->bwt->gfx, label->text, &settings);
}

void BWT_Label_SetText(BWT_LabelHandle label, const char * text)
{
    unsigned textWidth = 0;
    assert(label);
    if (text)
    {
        textWidth = platform_graphics_get_text_width(label->widget.bwt->gfx, text);
    }
#if DEBUG
    if (label->text && text)
    {
        printf("label '%s': old text was '%s'\n", label->widget.name, label->text);
        printf("label '%s': new text is '%s' (%u)\n", label->widget.name, text, textWidth);
    }
#endif
    if (label->text)
    {
        free(label->text);
        label->text = NULL;
    }
    if (text)
    {
        label->text = malloc(strlen(text) + 1);
        assert(label->text);
        strcpy(label->text, text);
        label->widget.dims.width = BWT_Label_P_ValidateDimension(label, "width", textWidth, label->widget.dims.width);
    }
}

void BWT_Label_SetInteger(BWT_LabelHandle label, int number)
{
    char text[16];
    assert(label);
    sprintf(text, "%d", number);
    BWT_Label_SetText(label, text);
}

void BWT_Image_GetDefaultCreateSettings(BWT_ImageCreateSettings * pSettings)
{
    memset(pSettings, 0, sizeof(*pSettings));
    pSettings->visible = true;
}

static void BWT_Image_P_GetDefaultWidgetInitSettings(BWT_WidgetInitSettings * pSettings)
{
    BWT_Widget_GetDefaultInitSettings(pSettings);
    pSettings->name = BWT_IMAGE_NAME_NONE;
    pSettings->type = BWT_WidgetType_eImage;
    pSettings->render = &BWT_Image_Render;
    pSettings->destroy = &BWT_Image_Destroy;
}

BWT_ImageHandle BWT_Image_Create(BWT_ToolkitHandle bwt, const BWT_ImageCreateSettings * pSettings)
{
    BWT_WidgetInitSettings widgetSettings;
    BWT_ImageHandle image;

    assert(bwt);
    assert(pSettings);
    assert(pSettings->pic);
    image = malloc(sizeof(*image));
    assert(image);
    memset(image, 0, sizeof(*image));
    BWT_Image_P_GetDefaultWidgetInitSettings(&widgetSettings);
    if (pSettings->name) widgetSettings.name = pSettings->name;
    widgetSettings.visible = pSettings->visible;
    platform_picture_get_dimensions(pSettings->pic, &widgetSettings.dims.width, &widgetSettings.dims.height);

#if DEBUG
    printf("image");
#endif
    BWT_Widget_InitBase((BWT_WidgetHandle)image, bwt, &widgetSettings);
    image->pic = pSettings->pic;
#if DEBUG
    printf(" path '%s'\n", platform_picture_get_path(image->pic));
#endif

    return image;
}

void BWT_Image_Destroy(void * v)
{
    BWT_ImageHandle image = (BWT_ImageHandle)v;
    if (image)
    {
        BWT_Widget_UninitBase((BWT_WidgetHandle)image);
        free(image);
    }
}

void BWT_Image_Render(BWT_WidgetHandle w)
{
    BWT_ImageHandle image = (BWT_ImageHandle)w;
    PlatformRect r;

    assert(image);

    if (!image->pic || !BWT_Widget_IsVisible(w)) return;

    r.width = w->dims.width;
    r.height = w->dims.height;
    r.x = w->pos.x;
    r.y = w->pos.y;
#if DEBUG
    printf("Rendering %ux%u image @ (%u, %u)\n",
        w->dims.width,
        w->dims.height,
        w->pos.x,
        w->pos.y);
#endif
    platform_graphics_render_picture(w->bwt->gfx, image->pic, &r);
}

void BWT_Table_GetDefaultCreateSettings(BWT_TableCreateSettings * pSettings)
{
    memset(pSettings, 0, sizeof(*pSettings));
    pSettings->rows = 1;
    pSettings->cols = 2;
    pSettings->padding = 1;
    pSettings->spacing = 1;
}

static void BWT_Table_P_CreateGrid(BWT_ToolkitHandle bwt, BWT_TableHandle table, unsigned padding, unsigned spacing)
{
    BWT_PanelCreateSettings panelSettings;
    BWT_PanelHandle panel;
    BWT_PanelHandle cell;
    unsigned r;
    unsigned c;

    for (r = 0; r < table->rows; r++)
    {
        BWT_Panel_GetDefaultCreateSettings(&panelSettings);
        panelSettings.color = table->panel.widget.color;
        panelSettings.dims.width = table->cols * (table->colWidth + spacing);
        panelSettings.dims.height = table->rowHeight;
        panelSettings.padding = 0; /* padding is in cell, not row */
        panelSettings.spacing = spacing;
        panelSettings.name = BWT_TABLE_ROW_NAME_NONE;
        panelSettings.flow = BWT_LayoutFlow_eHorizontal;
        panel = BWT_Panel_Create(bwt, &panelSettings);
        assert(panel);
        for (c = 0; c < table->cols; c++)
        {
            BWT_Panel_GetDefaultCreateSettings(&panelSettings);
            panelSettings.padding = padding;
            panelSettings.spacing = 0; /* spacing is in row, not col */
            panelSettings.color = table->panel.widget.color;
            panelSettings.dims.width = table->colWidth;
            panelSettings.dims.height = table->rowHeight;
            panelSettings.name = BWT_TABLE_COL_NAME_NONE;
            panelSettings.flow = BWT_LayoutFlow_eHorizontal;
            cell = BWT_Panel_Create(bwt, &panelSettings);
            assert(cell);
            BWT_Panel_AddChild(panel, (BWT_WidgetHandle)cell, BWT_VerticalAlignment_eDefault, BWT_HorizontalAlignment_eDefault);
        }
        BWT_Panel_AddChild((BWT_PanelHandle)table, (BWT_WidgetHandle)panel, BWT_VerticalAlignment_eDefault, BWT_HorizontalAlignment_eDefault);
    }
}

void BWT_Table_Print(BWT_TableHandle table)
{
    BWT_PanelHandle panel;
    BWT_PanelHandle cell;
    unsigned r;
    unsigned c;

    for (r = 0, panel = (BWT_PanelHandle)BLST_Q_FIRST(BWT_Panel_P_GetDefaultChildGroup(&table->panel)); panel; r++, panel = (BWT_PanelHandle)BLST_Q_NEXT((BWT_WidgetHandle)panel, link))
    {
        for (c = 0, cell = (BWT_PanelHandle)BLST_Q_FIRST(BWT_Panel_P_GetDefaultChildGroup(panel)); cell; c++, cell = (BWT_PanelHandle)BLST_Q_NEXT((BWT_WidgetHandle)cell, link))
        {
            BWT_Panel_P_PrintChildGroups(cell);
        }
        printf("\n");
    }
}

BWT_TableHandle BWT_Table_Create(BWT_ToolkitHandle bwt, const BWT_TableCreateSettings * pSettings)
{
    BWT_TableHandle table;
    BWT_TableInitSettings initSettings;

    assert(bwt);
    table = malloc(sizeof(*table));
    assert(table);
    memset(table, 0, sizeof(*table));
    BWT_Table_GetDefaultInitSettings(&initSettings);
    if (pSettings->name) initSettings.panel.widget.name = pSettings->name;
    initSettings.panel.widget.spacing = 0; /* in grid, not table itself */
    initSettings.panel.widget.padding = 0; /* in grid, not table itself */
    initSettings.panel.widget.dims.height = pSettings->rows * (pSettings->rowHeight + pSettings->spacing);
    initSettings.panel.widget.dims.width = pSettings->cols * (pSettings->colWidth + pSettings->spacing);
    initSettings.panel.widget.color = pSettings->bgColor;
    initSettings.panel.flow = BWT_LayoutFlow_eVertical;
    BWT_Table_InitBase(table, bwt, &initSettings);
    table->colWidth = pSettings->colWidth;
    table->cols = pSettings->cols;
    table->rowHeight = pSettings->rowHeight;
    table->rows = pSettings->rows;
    table->fgColor = pSettings->fgColor;
    BWT_Table_P_CreateGrid(bwt, table, pSettings->padding, pSettings->spacing);

    return table;
}

void BWT_Table_Destroy(void * v)
{
    /* we are basically a panel with more panels as rows */
    BWT_Panel_P_Destroy(v);
}

void BWT_Table_GetDefaultInitSettings(BWT_TableInitSettings * pSettings)
{
    memset(pSettings, 0, sizeof(*pSettings));
    BWT_Panel_GetDefaultInitSettings(&pSettings->panel);
    pSettings->panel.widget.name = BWT_TABLE_NAME_NONE;
    pSettings->panel.widget.type = BWT_WidgetType_eTable;
    pSettings->panel.widget.render = &BWT_Table_Render;
    pSettings->panel.widget.destroy = &BWT_Table_Destroy;
    pSettings->panel.widget.place = &BWT_Table_Place;
}

void BWT_Table_InitBase(BWT_TableHandle table, BWT_ToolkitHandle bwt, const BWT_TableInitSettings * pSettings)
{
    const char * name;
    assert(table);
    assert(bwt);
    memset(table, 0, sizeof(*table));
#if DEBUG
    printf("table ");
#endif
    BWT_Panel_InitBase((BWT_PanelHandle)table, bwt, &pSettings->panel);
#if DEBUG
    printf("\n");
#endif
    return;
}

void BWT_Table_UninitBase(BWT_TableHandle table)
{
    if (!table) return;
    BWT_Panel_UninitBase((BWT_PanelHandle)table);
}

void BWT_Table_Render(BWT_WidgetHandle w)
{
    BWT_Panel_Render(w);
}

void BWT_Table_Place(BWT_WidgetHandle w)
{
    BWT_Panel_Place(w);
}

BWT_WidgetHandle BWT_Table_GetRow(BWT_TableHandle table, unsigned row)
{
    BWT_WidgetHandle widget;
    unsigned r;

    r = 0;
    for (widget = BLST_Q_FIRST(BWT_Panel_P_GetDefaultChildGroup(&table->panel)); widget; widget = BLST_Q_NEXT(widget, link))
    {
        if (r++ == row) break;
    }

    return widget;
}

BWT_WidgetHandle BWT_Table_GetCell(BWT_TableHandle table, BWT_WidgetHandle row, unsigned col)
{
    BWT_WidgetHandle cell;
    unsigned c;

    c = 0;
    for (cell = BLST_Q_FIRST(BWT_Panel_P_GetDefaultChildGroup((BWT_PanelHandle)row)); cell; cell = BLST_Q_NEXT(cell, link))
    {
        if (c++ == col) break;
    }

    return cell;
}

void BWT_VideoWindow_GetDefaultCreateSettings(BWT_VideoWindowCreateSettings * pSettings)
{
    memset(pSettings, 0, sizeof(*pSettings));
    pSettings->scale = 100;
}

static void BWT_VideoWindow_P_GetDefaultWidgetInitSettings(BWT_WidgetInitSettings * pSettings)
{
    BWT_Widget_GetDefaultInitSettings(pSettings);
    pSettings->name = BWT_VIDEO_WINDOW_NAME_NONE;
    pSettings->type = BWT_WidgetType_eVideoWindow;
    pSettings->render = &BWT_VideoWindow_Render;
    pSettings->destroy = &BWT_VideoWindow_Destroy;
    pSettings->place = &BWT_VideoWindow_Place;
}

BWT_VideoWindowHandle BWT_VideoWindow_Create(BWT_ToolkitHandle bwt, const BWT_VideoWindowCreateSettings * pSettings)
{
    BWT_WidgetInitSettings widgetSettings;
    BWT_VideoWindowHandle window;
    const BWT_Dimensions * dims;

    assert(bwt);
    assert(pSettings);
    window = malloc(sizeof(*window));
    assert(window);
    memset(window, 0, sizeof(*window));
    BWT_VideoWindow_P_GetDefaultWidgetInitSettings(&widgetSettings);
    if (pSettings->name) widgetSettings.name = pSettings->name;
    widgetSettings.visible = pSettings->visible;
    dims = BWT_Toolkit_GetFramebufferDimensions(bwt);
    widgetSettings.dims.width = dims->width * pSettings->scale / 100;
    widgetSettings.dims.height = dims->height * pSettings->scale / 100;

#if DEBUG
    printf("window");
#endif
    BWT_Widget_InitBase((BWT_WidgetHandle)window, bwt, &widgetSettings);
    window->scale = pSettings->scale;
#if DEBUG
    printf(" scale %u%%\n", window->scale);
#endif

    return window;
}

void BWT_VideoWindow_Destroy(void * v)
{
    BWT_VideoWindowHandle window = (BWT_VideoWindowHandle)v;
    if (window)
    {
        BWT_Widget_UninitBase((BWT_WidgetHandle)window);
        free(window);
    }
}

void BWT_VideoWindow_Render(BWT_WidgetHandle w)
{
    BWT_VideoWindowHandle window = (BWT_VideoWindowHandle)w;
    PlatformRect r;

    assert(window);

    if (!BWT_Widget_IsVisible(w)) return;

    r.width = w->dims.width;
    r.height = w->dims.height;
    r.x = w->pos.x;
    r.y = w->pos.y;
#if DEBUG
    printf("Rendering %ux%u video @ (%u, %u)\n",
        w->dims.width,
        w->dims.height,
        w->pos.x,
        w->pos.y);
#endif
    platform_graphics_render_video(w->bwt->gfx, &r);
}

void BWT_VideoWindow_SetScale(BWT_VideoWindowHandle window, unsigned scale)
{
    BWT_WidgetHandle w = (BWT_WidgetHandle)window;
    const BWT_Dimensions * dims;
    PlatformRect r;

    assert(window);

    if (w->parent)
    {
        dims = BWT_Widget_GetDimensions((BWT_WidgetHandle)w->parent);
    }
    else
    {
        dims = BWT_Toolkit_GetFramebufferDimensions(w->bwt);
    }

    w->dims.width = dims->width * scale / 100;
    w->dims.height = dims->height * scale / 100;
    window->scale = scale;

    r.width = w->dims.width;
    r.height = w->dims.height;
#if DEBUG
    printf("Scaling video to %ux%u\n",
        r.width,
        r.height);
#endif
    platform_graphics_scale_video(w->bwt->gfx, &r);
}

void BWT_VideoWindow_Place(BWT_WidgetHandle w)
{
    PlatformRect r;
    BWT_Widget_PlaceBase(w);
    r.x = w->pos.x;
    r.y = w->pos.y;
#if DEBUG
    printf("Moving video to (%u,%u)\n",
        r.x,
        r.y);
#endif
    platform_graphics_move_video(w->bwt->gfx, &r);
}

void BWT_Toolkit_GetDefaultCreateSettings(BWT_ToolkitCreateSettings * pSettings)
{
    memset(pSettings, 0, sizeof(*pSettings));
}

BWT_ToolkitHandle BWT_Toolkit_Create(const BWT_ToolkitCreateSettings * pSettings)
{
    BWT_ToolkitHandle bwt;
    const PlatformRect * fbRect;

    bwt = malloc(sizeof(*bwt));
    if (!bwt) goto out_no_memory;
    memset(bwt, 0, sizeof(*bwt));

    bwt->gfx = pSettings->gfx;
    fbRect = platform_graphics_get_fb_rect(bwt->gfx);
    bwt->fbDims.width = fbRect->width;
    bwt->fbDims.height = fbRect->height;
    return bwt;

out_no_memory:
    BWT_Toolkit_Destroy(bwt);
    return NULL;
}

void BWT_Toolkit_Destroy(BWT_ToolkitHandle bwt)
{
    if (!bwt) return;
    free(bwt);
}

unsigned BWT_Toolkit_GetTextHeight(BWT_ToolkitHandle bwt)
{
    if (!bwt) return 0;
    return platform_graphics_get_text_height(bwt->gfx);
}

const BWT_Dimensions * BWT_Toolkit_GetFramebufferDimensions(BWT_ToolkitHandle bwt)
{
    if (!bwt) return NULL;
    return &bwt->fbDims;
}

void BWT_Toolkit_Submit(BWT_ToolkitHandle bwt)
{
    if (!bwt) return;
    platform_graphics_submit(bwt->gfx);
}
