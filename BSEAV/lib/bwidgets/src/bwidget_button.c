/******************************************************************************
 * (c) 2004-2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
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

#include "bwidget_priv.h"

#define PSETTINGS(widget) ((bbutton_settings*)widget->settings)

static void
bwidget_p_draw_button(bwidget_t widget, const bwin_rect *cliprect)
{
    bwidget_p_verify(widget, bwidget_type_button);
    bwidget_p_draw_label_helper(widget, PSETTINGS(widget)->down, cliprect);
}

void
bwidget_p_set_button_defaults(bwidget_t widget, bwidget_engine_t engine)
{
    bwidget_p_verify(widget, bwidget_type_button);
    bwidget_p_set_label_defaults(widget, engine);
}

void bbutton_get_default_settings(bbutton_settings *settings)
{
    BKNI_Memset(settings, 0, sizeof(*settings));
    blabel_get_default_settings(&settings->label);

    /* like a label, but we use the down state */
    settings->label.widget.draw = bwidget_p_draw_button;
    settings->label.widget.focusable = true;
}

bwidget_t bbutton_create(bwidget_engine_t engine, const bbutton_settings *settings)
{
    bwidget_t button = bwidget_p_create(engine, settings, sizeof(struct bwidget_button), sizeof(*settings), bwidget_type_button);
    bwidget_p_set_button_defaults(button, engine);
    return button;
}

void
bbutton_get(bwidget_t widget, bbutton_settings *settings)
{
    BKNI_Memcpy(settings, widget->settings, sizeof(*settings));
}

int bbutton_p_apply_settings(bwidget_t widget, const bbutton_settings *settings)
{
    return blabel_p_apply_settings(widget, &settings->label);
}

int bbutton_set(bwidget_t widget, const bbutton_settings *settings)
{
    bbutton_p_apply_settings(widget, settings);
    BKNI_Memcpy(widget->settings, settings, sizeof(*settings));
    return 0;
}

void bbutton_click(bwidget_t widget)
{
    if (PSETTINGS(widget)->click) {
        (PSETTINGS(widget)->click)(widget);
    }
}
