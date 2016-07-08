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

#include "bwin.h"
#include "bwin_priv.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <bkni.h>
BDBG_MODULE(bwin);

#define BWIN_MAGIC 0x123456
#define BWIN_EVENT_MAGIC 0x123123

#define CHECK_EV_LIST(win) \
do { \
    bwin_p_event *ev; \
    for (ev=BLST_Q_FIRST(&win->ev_list);ev;ev=BLST_Q_NEXT(ev,link)) \
        assert(ev->magic == BWIN_EVENT_MAGIC); \
} while (0)

static void bwin_p_repaint_parent(bwin_t window, bwin_rect *rect);

void bwin_engine_settings_init(bwin_engine_settings *settings)
{
    memset(settings, 0, sizeof(*settings));
}

bwin_engine_t
bwin_open_engine(const bwin_engine_settings *settings)
{
    bwin_engine_t win_engine = (bwin_engine_t)malloc(sizeof(*win_engine));
    if (!win_engine) return NULL;
    memset(win_engine, 0, sizeof(*win_engine));
    win_engine->settings = *settings;
    BLST_D_INIT(&win_engine->fb_list);
    BLST_Q_INIT(&win_engine->ev_list);
    if (BKNI_CreateEvent(&win_engine->event) !=  BERR_SUCCESS) {
        BKNI_Free(win_engine);
        return NULL;
    }
    return win_engine;
}

void
bwin_close_engine(bwin_engine_t win_engine)
{
    assert(BLST_D_EMPTY(&win_engine->fb_list));
    BKNI_DestroyEvent(win_engine->event);
    free(win_engine);
}

void bwin_add_io_handle(bwin_engine_t win, bwin_io_handle handle)
{
    int i;
    assert(handle);
    for (i=0;i<BWIN_IO_HANDLES;i++) {
        if (!win->io[i].handle) {
            win->io[i].handle = handle;
            win->io[i].pending = false;
            return;
        }
    }
    assert(false);
}

void bwin_remove_io_handle(bwin_engine_t win_engine, bwin_io_handle handle)
{
    int i;
    for (i=0;i<BWIN_IO_HANDLES;i++) {
        if (win_engine->io[i].handle == handle) {
            win_engine->io[i].handle = 0;
            break;
        }
    }
}

void bwin_trigger_io(bwin_engine_t win_engine, bwin_io_handle handle)
{
    int i;
    for (i=0;i<BWIN_IO_HANDLES;i++) {
        if (win_engine->io[i].handle == handle) {
            /* this is an atomic write, so it doesn't need to be synchronized
            with bwin_get_event. */
            win_engine->io[i].pending = true;
            BKNI_SetEvent(win_engine->event);
            break;
        }
    }
}

static bwin_drawing_operations g_default_drawops = {
    NULL, /* no such thing as a default sync */
    bwin_p_draw_point,
    bwin_p_draw_line,
    bwin_p_fill_rect,
    5 * 5, /* fill_rect_threshold - 25 pixels */
    bwin_p_copy_rect,
    bwin_p_blit,
    bwin_p_draw_ellipse,
    bwin_p_fill_ellipse,
    NULL, /* no such thing as a default device_bitmap_create */
    NULL  /* no such thing as a default device_bitmap_destroy */
};

void bwin_framebuffer_settings_init(bwin_framebuffer_settings *settings)
{
    memset(settings, 0, sizeof(*settings));
    settings->drawops = g_default_drawops;
}

int bwin_get_bpp(bwin_pixel_format pixel_format)
{
    switch (pixel_format) {
    case bwin_pixel_format_r5_g6_b5:    return 16;
    case bwin_pixel_format_a1_r5_g5_b5: return 16;
    case bwin_pixel_format_a8_r8_g8_b8: return 32;
    case bwin_pixel_format_palette8:    return 8;
    default: return -1;
    }
}

bwin_framebuffer_t
bwin_open_framebuffer(bwin_engine_t win_engine, const bwin_framebuffer_settings *settings)
{
    bwin_settings win_settings;
    bwin_framebuffer_t fb = (bwin_framebuffer_t)malloc(sizeof(*fb));
    if (!fb) return NULL;
    fb->settings = *settings;
    fb->win_engine = win_engine;
    fb->bpp = bwin_get_bpp(fb->settings.pixel_format);
    fb->is_buf_device_dependent = false;
    fb->is_buf_internal_alloc   = false;

    if (fb->settings.buffer)
    {
        /* buffer given so use it */
        /*printf("using given bwin framebuffer bitmap w:%d h:%d\n", settings->width, settings->height);*/
        fb->buffer = fb->settings.buffer;
    }
    else
    {
        /* no buffer given */
        if (settings->drawops.device_bitmap_create)
        {
            /* try creating device dependent bitmap */
            /* printf("creating device dependent bwin framebuffer bitmap w:%d h:%d\n", settings->width, settings->height);*/
            settings->drawops.device_bitmap_create(fb, &fb->settings.buffer, &fb->settings.data);

            if (fb->settings.buffer)
            {
                fb->buffer                  = fb->settings.buffer;
                fb->is_buf_device_dependent = true;
                fb->is_buf_internal_alloc   = true;
            }
        }
        else
        {
            /* try creating device independent bitmap */
            int pitch = bwin_get_bpp(settings->pixel_format/8) * settings->width;

            /*printf("creating device independent bwin framebuffer bitmap w:%d h:%d\n", settings->width, settings->height);*/
            fb->settings.buffer = BKNI_Malloc(pitch * settings->height);
            if (fb->settings.buffer)
            {
                fb->buffer                = fb->settings.buffer;
                fb->is_buf_internal_alloc = true;
            }
        }
    }
    BWIN_SET_RECT(&fb->rect, 0, 0, settings->width, settings->height);

    BDBG_MSG(("pitch %d, width %d, bpp %d", fb->settings.pitch, fb->settings.width, fb->bpp/8));
    assert(fb->settings.pitch >= fb->settings.width * fb->bpp/8);

    /* create a window which maps the entire framebuffer */
    bwin_settings_init(&win_settings);
    win_settings.visible = true;
    win_settings.rect.width = settings->width;
    win_settings.rect.height = settings->height;
    fb->settings.window = bwin_open(&win_settings);

    /* This sets the fb for this window and will be copied by all children */
    fb->settings.window->fb = fb;

    /* Now that we've hooked up the fb, we can schedule the repaint. */
    bwin_repaint(fb->settings.window, NULL);

    /* the win_engine keeps a list of framebuffers */
    BLST_D_INSERT_HEAD(&fb->win_engine->fb_list, fb, link);

    return fb;
}

void
bwin_close_framebuffer(bwin_framebuffer_t fb)
{
    BLST_D_REMOVE(&fb->win_engine->fb_list, fb, link);
    bwin_close(fb->settings.window);

    if (fb->is_buf_internal_alloc)
    {
        /* bwin_open_framebuffer() alloc'd the buffer so we must free it here */
        if (fb->is_buf_device_dependent)
        {
            /* free device dependent buffer */
            if (fb->settings.drawops.device_bitmap_destroy)
            {
                /*printf("destroying device dependent bwin framebuffer bitmap w:%d h:%d\n", fb->settings.width, fb->settings.height);*/
                fb->settings.drawops.device_bitmap_destroy(fb);
                fb->buffer = NULL;
            }
            else
            {
                BDBG_ERR(("MEMORY LEAK! A device independent bitmap was created for a bwin framebuffer, but a device_bitmap_destroy function does not exist!"));
            }
        }
        else
        {
            /* free device independent buffer */
            BKNI_Free(fb->buffer);
            fb->buffer = NULL;
        }
    }
    free(fb);
}

void
bwin_get_framebuffer_settings(bwin_framebuffer_t fb, bwin_framebuffer_settings *settings)
{
    *settings = fb->settings;
}

/* external way of requesting a sync callback */
void bwin_sync(bwin_framebuffer_t fb)
{
    fb->had_paint = false;
    if (fb->settings.drawops.sync) {
        (*fb->settings.drawops.sync)(fb);
    }

    /* flip the internal memory pointer */
    if (fb->settings.second_buffer) {
        if (fb->buffer == fb->settings.buffer)
            fb->buffer = fb->settings.second_buffer;
        else
            fb->buffer = fb->settings.buffer;
    }
}

int
bwin_get_event(bwin_engine_t win_engine, bwin_event *event, int timeout)
{
    BERR_Code rc;

    /* before we can get an event, we may have to build the event list by
    walking the window tree looking for repaint requests */
    bwin_p_process_events(win_engine);

    /* see if there's an event ready */
    if (!BLST_Q_EMPTY(&win_engine->ev_list)) {
        /* if there is, remove it from the list, copy to user memory, and
        delete the internal memory */
        bwin_p_event *ev = BLST_Q_FIRST(&win_engine->ev_list);
        *event = ev->ev;

        /* update the fb->had_paint if needed for later sync */
        if (event->type == bwin_event_type_paint) {
            event->window->fb->had_paint = true;
            /* clear the windows repaint rect now that we've painted it */
            event->window->repaint.width = event->window->repaint.height = 0;
        }

        BLST_Q_REMOVE_HEAD(&win_engine->ev_list, link);
        free(ev);
        CHECK_EV_LIST(win_engine);
        return 0;
    }
    else {
        bool wokenup = false;

        /* we have no events in the queue, therefore we know we're done painting.
        it's time to sync. call sync for every framebuffer that has at least one paint
        event since the last sync (if it has the sync callback) */
        bwin_framebuffer_t fb;
        for (fb = BLST_D_FIRST(&win_engine->fb_list); fb; fb = BLST_D_NEXT(fb, link)) {
            if (fb->had_paint)
                bwin_sync(fb);
        }

        do {
            int i;
            for (i=0;i<BWIN_IO_HANDLES;i++) {
                if (win_engine->io[i].pending) {
                    win_engine->io[i].pending = false;
                    event->type = bwin_event_type_io;
                    event->window = NULL;
                    event->io.handle = win_engine->io[i].handle;
                    BDBG_MSG(("io[%d]", i));
                    return 0;
                }
            }
            if (wokenup) {
                /* There is a normal condition where this code can be reached.
                bwin_trigger_io sets a pending handle to true while the code
                is not blocked on the following WaitForEvent. When it comes in again
                and nothing is pending, it will process that signal, but nothing
                will be waiting. It's a harmless race condition, so ignore it. */
                break;
            }
            else {
                BDBG_MSG(("calling wait %d", timeout));
                rc = BKNI_WaitForEvent(win_engine->event, timeout);
                if (!rc) {
                    wokenup = true;
                }
            }
        } while (wokenup); /* try one more time if woken up */
        BDBG_MSG(("idle"));
        return -1;
    }
}

int
bwin_peek_event(bwin_engine_t win_engine, bwin_event *event)
{
    /* see note in bwin_get_event */
    bwin_p_process_events(win_engine);

    if (!BLST_Q_EMPTY(&win_engine->ev_list)) {
        /* don't delete it, just copy it */
        *event = BLST_Q_FIRST(&win_engine->ev_list)->ev;
        CHECK_EV_LIST(win_engine);
        return 0;
    }
    else
        return -1;
}

void bwin_settings_init(bwin_settings *settings)
{
    memset(settings, 0, sizeof(*settings));
    settings->transparent = true;
}

/* link up to the parent window. this is needed when creating a new window, or
when reparenting or changing zorder. see bwin_set for those cases. */
static void
bwin_p_link(bwin_t window)
{
    bwin_t temp;

    /* if there's no parent, this must be the framebuffer internal window */
    if (!window->settings.parent)
        return;

    /* fetch the fb pointer up the tree */
    window->fb = window->settings.parent->fb;

    /* Insert into the list in zorder, lowest first */
    for (temp = BLST_Q_FIRST(&window->settings.parent->win_list); temp; temp = BLST_Q_NEXT(temp, link)) {
        if (window->settings.zorder <= temp->settings.zorder) {
            BLST_Q_INSERT_BEFORE(&window->settings.parent->win_list, temp, window, link);
            return;
        }
    }
    BLST_Q_INSERT_TAIL(&window->settings.parent->win_list, window, link);
}

/* unlink from the parent window */
static void
bwin_p_unlink(bwin_t window)
{
    if (!window->settings.parent)
        return;

    BLST_Q_REMOVE(&window->settings.parent->win_list, window, link);
}

/* this can be used in various locations to track down corrupted memory or
to enforce that certain parameters stay readonly */
void bwin_p_validate_settings(const bwin_settings *settings)
{
    if (settings->parent)
        assert(settings->parent->magic == BWIN_MAGIC);

    /* x,y must be 0,0. Lots of code assumes settings->rect is a client rect. */
    assert(settings->rect.x == 0);
    assert(settings->rect.y == 0);
}

bwin_t bwin_open(const bwin_settings *settings)
{
    bwin_t window;

    bwin_p_validate_settings(settings);

    window = (bwin_t)malloc(sizeof(*window));
    if (!window) return NULL;
    memset(window, 0, sizeof(*window));
    window->magic = BWIN_MAGIC;
    window->settings = *settings;
    BLST_Q_INIT(&window->win_list);

    /* link up with the parent */
    bwin_p_link(window);

    /* if created visible and not the initial framebuffer window,
    repaint immediately */
    if (window->settings.visible && window->fb)
        bwin_repaint(window, NULL);

    return window;
}

void
bwin_close(bwin_t window)
{
    bwin_p_unlink(window);
    if (window->settings.visible) {
        bwin_p_repaint_parent(window, NULL);
    }

    /* This forces all events to be recreated, because some might
    exist with this window linked in. */
    window->fb->win_engine->valid_ev_list = false;

    if (!BLST_Q_EMPTY(&window->win_list))
    {
        bwin_t pWin     = NULL;
        bwin_t pNextWin = NULL;
        int    numWins  = 0;

        BDBG_WRN(("bwin_close() error in win:%p at x:%d y:%d w:%d h:%d",
                  (void*)window, window->settings.x, window->settings.y,
                  window->settings.rect.width, window->settings.rect.height));
        for (pWin = BLST_Q_FIRST(&window->win_list); NULL != pWin; pWin = pNextWin)
        {
            BDBG_WRN(("unlinked child win:%p at x:%d y:%d, w:%d, h:%d",
                      (void*)pWin, pWin->settings.x, pWin->settings.y,
                      pWin->settings.rect.width, pWin->settings.rect.height));
            pNextWin = BLST_Q_NEXT(pWin, link);
            numWins++;
        }
        BDBG_WRN(("Total num of unlinked child windows:%d for win:%p", numWins, (void*)window));
        assert(0 == numWins);
    }

    free(window);
}

void
bwin_set(bwin_t window, const bwin_settings *settings)
{
    bool before = false;
    bool after = false;
    bool relink = false;

    bwin_p_validate_settings(settings);

    /* reparenting involves redrawing and relinking */
    if (settings->parent != window->settings.parent) {
        before = true;
        after = true;
        relink = true;
    }

    if (settings->visible != window->settings.visible) {
        if (!settings->visible) {
            before = true;
            /* paint events for invisible windows are removed when peeked */
        }
        else
            after = true;
    }

    if (settings->zorder != window->settings.zorder)
    {
        after = true;
        relink = true; /* this sets the zorder in the win_list */
    }
    if (settings->transparent != window->settings.transparent)
    {
        after = true;
    }
    if (!BWIN_RECT_ISEQUAL(&settings->rect, &window->settings.rect) ||
        settings->x != window->settings.x ||
        settings->y != window->settings.y) {
        /* if we're moving, we must paint in the parent where we were, and
        then move, then paint in our new location. */
        before = after = true;
    }

    /* now that's we've analyzed the changes, take the appropriate actions. */
    if (before)
        bwin_p_repaint_parent(window, NULL);
    if (relink)
        bwin_p_unlink(window);
    window->settings = *settings;
    if (relink)
        bwin_p_link(window);
    if (after)
        bwin_repaint(window, NULL);
}

static void bwin_p_repaint_parent(bwin_t window, bwin_rect *rect)
{
    bwin_rect r;

    if (!window->settings.parent)
        return;

    if (!rect) rect = &window->settings.rect;

    /* r is in the parent's coordinates */
    BWIN_SET_RECT(&r, rect->x+window->settings.x,rect->y+window->settings.y,rect->width,rect->height);

    /* repaint the parent */
    bwin_repaint(window->settings.parent, &r);
}

void
bwin_get(bwin_t window, bwin_settings *settings)
{
    *settings = window->settings;
}

/* Internal repaint which only checks for visiblity. This is called by the
external bwin_repaint, as well as when propogating
repaints to children and higher-zorder siblings during process_events time.
*/
void
bwin_repaint_internal(bwin_t window, const bwin_rect *rect)
{
    bwin_rect test;

    assert(rect);
    if (!window->settings.visible)
        return;

    /* but constrain it to this window */
    bwin_intersect_rect(&test, rect, &window->settings.rect);

    if (test.width && test.height) {
        /* grow our current repaint rect */
        bwin_union_rect(&window->repaint, &window->repaint, &test);
    }
}

/* External repaint function. It handles transparency and invalidates the current
ev_list so that it will be reprocessed when the next get/peek event call is made.
*/
void
bwin_repaint(bwin_t window, const bwin_rect *rect)
{
    assert(window->magic == BWIN_MAGIC);

    if (!window->settings.visible)
        return;
    if (!rect) rect = &window->settings.rect;
    BDBG_MSG(("repaint %s: %d,%d,%u,%u", window->settings.name,
        rect->x,rect->y,rect->width,rect->height));

    if (window->settings.transparent && window->settings.parent) {
        /* ask the parent to repaint the region */
        bwin_rect r = *rect;
        r.x += window->settings.x;
        r.y += window->settings.y;
        bwin_repaint(window->settings.parent, &r);
    }
    else {
        bwin_repaint_internal(window, rect);
        /* and invalidate the current event list */
        window->fb->win_engine->valid_ev_list = false;
    }
}

/* convert from window coordinates to global coordinates */
void bwin_p_convert_to_global(bwin_t window, int x, int y, int *globalx, int *globaly)
{
    *globalx = x;
    *globaly = y;
    while (window) {
        *globalx += window->settings.x;
        *globaly += window->settings.y;

        /* If the a parents coordinates cause us to go negative, its possible they may come back positive. This happens
        with scrollviews that use clip/view ports. In general, we shouldn't draw that portion that went negative.
        This is a hack solution which doesn't allow for partial painting, but it does remove out-of-bounds painting, which is worse.
        The correct action would be:
        1) compute the difference in src->dest so that the source can be clipped. we don't have that info in
        this function.
        2) if we can't get any of the dest in, then we need to fail. don't draw.
        */
        if (*globalx < 0 || *globaly < 0) {
            /* use outrageous coordinates which are sure to be clipped off before going to framebuffer */
            *globalx = -100000000;
            *globaly = -100000000;
            return;
        }

        window = window->settings.parent;
    }
}

/* convert from global coordinates to window coordinates */
void bwin_p_convert_from_global(bwin_t window, int globalx, int globaly, int *x, int *y)
{
    *x = globalx;
    *y = globaly;
    while (window) {
        *x -= window->settings.x;
        *y -= window->settings.y;
        window = window->settings.parent;
    }
}

/* makes a copy of the event and stores it to the end of the win_engine's ev_list */
void
bwin_p_add_event(bwin_engine_t win, const bwin_p_event *event)
{
    bwin_p_event *newev = (bwin_p_event *)malloc(sizeof(bwin_p_event));
    *newev = *event;
    BLST_Q_INSERT_TAIL(&win->ev_list, newev, link);
}

void
bwin_p_process_events2(bwin_t window, const bwin_rect *cliprect);

/**
Process_events is called whenever get/peek event is called.
If the ev_list is already valid, it does nothing.
Otherwise, it clears the ev_list and rebuilds it by walking through
the window tree, depth-first, and processing and propogating paint
events. This handles parents and zorder easily.
**/
void
bwin_p_process_events(bwin_engine_t win_engine)
{
    bwin_framebuffer_t fb;
    bwin_p_event *ev;

    if (win_engine->valid_ev_list) {
        return;
    }

    /* rebuild a new ev_list by walking the window tree */
    while ((ev = BLST_Q_FIRST(&win_engine->ev_list))) {
        BLST_Q_REMOVE_HEAD(&win_engine->ev_list, link);
        free(ev);
    }

    for (fb = BLST_D_FIRST(&win_engine->fb_list); fb; fb = BLST_D_NEXT(fb, link)) {
        bwin_p_process_events2(fb->settings.window, &fb->settings.window->settings.rect);
    }

    /* The ev_list is now valid until another bwin_repaint is called. */
    win_engine->valid_ev_list = true;
}

/**
Recursive function called by bwin_p_process_events. Processes
a single window along with its children and siblings with greater zorder.
**/
void
bwin_p_process_events2(bwin_t window, const bwin_rect *cliprect)
{
    bwin_t child, sibling;
    bwin_p_event event;
    bwin_rect mycliprect;

    if (!window->settings.visible)
        return;

    /* intersect the parent's cliprect with this window to determine this
    window's cliprect. */
    bwin_intersect_rect(&mycliprect, cliprect, &window->settings.rect);

    /* check if it's scheduled for repaint */
    if (window->repaint.width && window->repaint.height) {
        bwin_rect repaint_rect;

        /* clip the stored repaint rect */
        bwin_intersect_rect(&repaint_rect, &mycliprect, &window->repaint);

        /* create a paint event */
        memset(&event, 0, sizeof(event));
        event.magic = BWIN_EVENT_MAGIC;
        event.ev.type = bwin_event_type_paint;
        event.ev.window = window;
        event.ev.paint.rect = repaint_rect;

        BDBG_MSG(("add paint event %s: %d,%d,%d,%d", window->settings.name,
            repaint_rect.x, repaint_rect.y,
            repaint_rect.width, repaint_rect.height));
        bwin_p_add_event(window->fb->win_engine, &event);

        /* now propogate the paint rect to the children which will be unioned with
        whatever repaint they already have */
        for (child = BLST_Q_FIRST(&window->win_list); child; child = BLST_Q_NEXT(child, link)) {
            /* propogate the repaint */
            bwin_rect childrect = repaint_rect;
            childrect.x -= child->settings.x;
            childrect.y -= child->settings.y;
            bwin_repaint_internal(child, &childrect);
        }

        /* propogate the paint rect to any siblings with greater zorder */
        if (window->settings.parent) {
            bwin_rect parentrect = repaint_rect;

            /* convert window repaint rect to parent coordinates */
            parentrect.x += window->settings.x;
            parentrect.y += window->settings.y;

            /* we can start with our next sibling */
            for (sibling = BLST_Q_NEXT(window, link); sibling; sibling = BLST_Q_NEXT(sibling, link)) {
                bwin_rect childrect = parentrect;
                childrect.x -= sibling->settings.x;
                childrect.y -= sibling->settings.y;
                bwin_repaint_internal(sibling, &childrect);
            }
        }
    }

    /* recurse in order to process children */
    for (child = BLST_Q_FIRST(&window->win_list); child; child = BLST_Q_NEXT(child, link)) {
        /* translate this window's cliprect into child coordinates */
        bwin_rect childcliprect = mycliprect;
        childcliprect.x -= child->settings.x;
        childcliprect.y -= child->settings.y;
        bwin_p_process_events2(child, &childcliprect);
    }
}
