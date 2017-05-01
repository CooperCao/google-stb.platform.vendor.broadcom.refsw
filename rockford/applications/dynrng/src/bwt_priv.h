/******************************************************************************
 * Broadcom Proprietary and Confidential. (c) 2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/
#ifndef BWT_PRIV_H__
#define BWT_PRIV_H__ 1

#include "platform_types.h"
#include "bwt.h"
#include "blst_queue.h"
#include <stdbool.h>

typedef struct BWT_Widget
{
    BLST_Q_ENTRY(BWT_Widget) link;
    BWT_ToolkitHandle bwt;
    char * name;
    bool visible;
    unsigned color;
    unsigned spacing;
    struct
    {
        unsigned rel;
        unsigned abs;
    } padding;
    BWT_PanelHandle parent;
    BWT_Dimensions dims;
    BWT_HorizontalAlignment halign;
    BWT_VerticalAlignment valign;
    BWT_Point pos;
    bool trans;
    struct
    {
        BWT_WidgetType type;
        BWT_Renderer render;
        BWT_Destructor destroy;
        BWT_Positioner place;
    } iface;
} BWT_Widget;

typedef struct BWT_Image
{
    BWT_Widget widget;
    PlatformPictureHandle pic;
} BWT_Image;

typedef struct BWT_Label
{
    BWT_Widget widget;
    char * text;
    BWT_HorizontalAlignment halign;
    BWT_VerticalAlignment valign;
} BWT_Label;

typedef struct BWT_Panel
{
    BWT_Widget widget;
    BWT_LayoutFlow flow;
    BWT_ImageHandle background;
    BWT_WidgetList children[BWT_VerticalAlignment_eMax][BWT_HorizontalAlignment_eMax];
} BWT_Panel;

typedef struct BWT_Table
{
    BWT_Panel panel;
    unsigned rows;
    unsigned cols;
    unsigned rowHeight;
    unsigned colWidth;
    unsigned fgColor;
} BWT_Table;

typedef struct BWT_VideoWindow
{
    BWT_Widget widget;
    unsigned scale;
    unsigned id;
    unsigned width;
    unsigned height;
} BWT_VideoWindow;

typedef struct BWT_Toolkit
{
    PlatformGraphicsHandle gfx;
    BWT_Dimensions fbDims;
} BWT_Toolkit;

typedef void (*BWT_Panel_WidgetListVisitor)(void * pContext, BWT_WidgetList * pChildren, BWT_VerticalAlignment valign, BWT_HorizontalAlignment halign);

#define BWT_WIDGET_IS_OPAQUE(W) (((W)->color & 0xff000000) == 0xff000000)

#endif /* BWT_PRIV_H__ */
