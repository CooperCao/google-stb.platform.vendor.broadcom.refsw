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

BDBG_MODULE(bwidget_base);

bwidget_t bwidget_p_create(bwidget_engine_t engine, const void *settings, unsigned sizeof_widget, unsigned sizeof_settings, bwidget_type type)
{
    bwidget_t widget;
    BSTD_UNUSED(engine);
    widget = BKNI_Malloc(sizeof_widget);
    widget->settings = BKNI_Malloc(sizeof_settings);
    BKNI_Memcpy(widget->settings, settings, sizeof_settings);
    widget->type = type;
    return widget;
}

#define PSETTINGS(WIDGET) ((bwidget_settings *)((WIDGET)->settings))
void
bwidget_p_set_base_defaults(bwidget_t widget, bwidget_engine_t engine)
{
    bwidget_p_verify(widget, bwidget_type_base);

    PSETTINGS(widget)->win = bwin_open(&PSETTINGS(widget)->window);

    /* Make sure that the bwin_t is in the same bwin_engine_t as was used
    to create the bwidget_engine_t. */
/*
TODO: add bwin_get_engine(win), then get rid of the engine param
otherwise we can't verify this.
*/
    widget->engine = engine;
    BLST_D_INSERT_HEAD(&engine->widgets, widget, link);
}

void
bwidget_destroy(bwidget_t widget)
{
    BLST_D_REMOVE(&widget->engine->widgets, widget, link);
    BKNI_Free(widget->settings);
    BKNI_Free(widget);
}

static int
bwidget_p_keydown(bwidget_t widget, const bwidget_key key)
{
    BSTD_UNUSED(widget);
    BSTD_UNUSED(key);
    return -1;
}

void bwidget_get_default_settings(bwidget_settings *settings)
{
    BKNI_Memset(settings, 0, sizeof(*settings));
    bwin_settings_init(&settings->window);
    settings->enabled = true;
    settings->draw = NULL; /* nothing to draw */
    settings->window.visible = true;
    settings->key_down = bwidget_p_keydown;
}

bwidget_t
bwidget_create(bwidget_engine_t engine, const bwidget_settings *settings)
{
    bwidget_t widget = bwidget_p_create(engine, settings, sizeof(struct bwidget), sizeof(*settings), bwidget_type_base);
    bwidget_p_set_base_defaults(widget, engine);
    return widget;
}

void
bwidget_get_settings(bwidget_t widget, bwidget_settings *settings)
{
    BKNI_Memcpy(settings, widget->settings, sizeof(*settings));
}

int bwidget_p_apply_settings(bwidget_t widget, const bwidget_settings *settings)
{
    bwin_set(PSETTINGS(widget)->win, &settings->window);
    return 0;
}

int bwidget_set_settings(bwidget_t widget, const bwidget_settings *settings)
{
    BKNI_Memcpy(widget->settings, settings, sizeof(*settings));
    return 0;
}
