/******************************************************************************
 * (c) 2002-2014 Broadcom Corporation
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

#ifndef MLISTVIEW_H
#define MLISTVIEW_H

#include "mscrollview.h"
#include "mpixmap.h"
#include "mcache.h"

class MListView;

BEGIN_EV(MListView)
    EV_METHOD(MListView, Click)
    EV_METHOD(MListView, Layout) // called after MListViewItem's are arranged
END_EV()

class MListViewItem : public MPushButton {
public:
    MListViewItem(MListView *v, MWidget *parent, const char *name = NULL);
    ~MListViewItem();
    const char *className() const {return "MListViewItem";}

    MString &label() {return _label;}
    MString &description() {return _description;}
    const MImage *icon() {return _image;}

    void setLabel(const char *label);
    void setDescription(const char *description);
    void setIcon(const MImage *image);

    MListView *view() const {return _view;}

    int measureWidth() const;
    virtual void focusRepaint();

protected:
    MString _label;
    MString _description;
    const MImage *_image;
    MListView *_view;

    void draw(const MRect &cliprect);
};

class MListView : public MScrollView, public MButtonEventHandler {
    friend class MListViewItem;

public:
    MListView(MWidget *parent, const MRect &rect, const char *name = NULL);
    ~MListView();

    MListViewItem *add();
    MListViewItem *insert(int index);
    void remove(MListViewItem *item);
    void remove(int index);
    void clear();
    int total() const {return _items.total();}
    MListViewItem *first() {return _items.first();}
    MListViewItem *next() {return _items.next();}

    enum View {ListView, DetailedListView, IconView};
    void setView(View view);
    View view() const {return _view;}
    void setIconWidth(int iconwidth, int iconborder);
    void setNoThumbImage(MImage * iconNoThumbnail);
    MImage * noThumbImage() { return _iconNoThumbnail; };

    MListViewItem *item(int index);
    int index(MListViewItem *item);
    MListViewItem *current();
    int currentIndex();

    void setSelected(MListViewItem *child);
    MListViewItem *selected() const {return _selected;}

    MCache<MPixmap> * cache() { return &_cacheThumbs; };

    // events
    void onClick(MButton *sender);

protected:
    MAutoList<MListViewItem> _items;
    View _view;
    int _iconwidth, _iconborder;
    MListViewItem *_selected;
    MCache<MPixmap> _cacheThumbs;
    MImage * _iconNoThumbnail;

    virtual MListViewItem *createItem();

    // overrides
    void layout();

    SUPPORT_EV(MListView)
    SUPPORT_EV_METHOD(Click)
    SUPPORT_EV_METHOD(Layout)
};

#endif
