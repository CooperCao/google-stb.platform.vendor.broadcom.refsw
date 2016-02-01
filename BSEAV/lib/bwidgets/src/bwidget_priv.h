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

#include "bwidgets.h"
#include <bkni.h>
#include "blst_list.h"

/* Internal enum use for simple RTTI when typecasting from generic
bwidget_t to specific widget. */
typedef enum {
    bwidget_type_base,
    bwidget_type_label,
    bwidget_type_button
} bwidget_type;

typedef struct bwidget_io {
    bwin_io_handle handle;
    bwidget_io_callback callback;
    BLST_D_ENTRY(bwidget_io) link; /* see bwidget_engine.io_handles */
} bwidget_io;

/* Implementation behind the handles. */
struct bwidget_engine {
    bwin_engine_t win_engine;
    bwidget_t focus;
    bwidget_t modal;
    bwidget_t modal_last_focus;
    bool terminated;
    bwidget_engine_settings settings;
    BLST_D_HEAD(widhead, bwidget) widgets; /* have to keep a list of widgets
        so we can look up bwidgets from bwin events */
    BLST_D_HEAD(iohead, bwidget_io) io_handles;
};

struct bwidget {
    bwidget_type type; /* Basic RTTI for verifying handles */
    void *settings; /* This points to the public settings structure for
        each widget. By making it a pointer, we avoid having unsused allocation
        because of nested implementation structures and nested settings structures. */
    bwidget_engine_t engine;
    BLST_D_ENTRY(bwidget) link; /* see bwidget_engine.widgets */
};

/* Each subclass must have its parent implementation as the
first member in order to typecast */

struct bwidget_label {
    struct bwidget widget;
};

struct bwidget_button {
    struct bwidget_label label;
};

#define GET_WIN(WIDGET) \
    ((WIDGET) ? ((bwidget_settings*)(WIDGET)->settings)->win : NULL)

bwidget_t bwidget_p_create(bwidget_engine_t engine, const void *settings, unsigned sizeof_widget, unsigned sizeof_settings, bwidget_type type);
void bwidget_p_verify(bwidget_t widget, bwidget_type type);
void bwidget_p_draw_label_helper(bwidget_t widget, bool down, const bwin_rect *cliprect);
void bwidget_p_set_base_defaults(bwidget_t widget, bwidget_engine_t engine);
void bwidget_p_set_label_defaults(bwidget_t widget, bwidget_engine_t engine);

int bwidget_p_apply_settings(bwidget_t widget, const bwidget_settings *settings);
int blabel_p_apply_settings(bwidget_t widget, const blabel_settings *settings);
int bbutton_p_apply_settings(bwidget_t widget, const bbutton_settings *settings);
