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

#ifndef MLISTBOX_H
#define MLISTBOX_H

#include "mwidget.h"
#include "mstring.h"
#include "mpainter.h"
#include "mscrollbar.h"
#include "mstringlist.h"

class MListBox;

BEGIN_EV(MListBox)
    EV_METHOD(MListBox,ItemClicked);
    EV_METHOD(MListBox,CurrentItemChanged);
    EV_METHOD(MListBox,InsertItem);
    EV_METHOD(MListBox,DeleteItem);
END_EV()

class MListBox : public MWidget, public MScrollBarEventHandler {
public:
    MListBox(MWidget *parent, const char *name = NULL);
    MListBox(MWidget *parent, const MRect &rect, int scrollBarWidth = 15, const char *name = NULL);

    // values
    void add(const char *text);
    void insert(int index, const char *text);
    void change(int index, const char *text);
    const char *item(int index);
    void remove(int index);
    void clear();
    int total() const {return _values.total();}
    int find(const char *text);

    // layout
    int indent() const {return _indent;}
    void setIndent(int indent) {_indent = indent;}

    /* currentIndex is the absolute index of the current selection.
    The relative index is currentIndex - topIndex */
    int currentIndex() const {return _currentIndex;}
    void setCurrentIndex(int i, bool changeFocus = true);

    /* focusIndex is the absolute index of the currently focused selection */
    int focusIndex();
    bool setFocus(int index);

    int topIndex() const {return _topIndex;}
    void setTopIndex(int topIndex);
    int totalVisible() const {return _items.total();}
    void setTotalVisible(int totalVisible);

    // behavior
    void setHighlightCurrent(bool highlightCurrent);
    void setRequireClickToMakeCurrent(bool requireClickToMakeCurrent);

    void beginUpdates();
    void endUpdates();

    //events
    void onScroll(MScrollBar *self);

    void setScrollBar(MScrollBar *bar, int width);
    MScrollBar *scrollBar() const {return _bar;}
    void captureKeys(bool up, bool down) {
        _captureUp = up;
        _captureDown = down;
    }

    bool focus();

protected:
    class MListItem : public MWidget {
        friend class MListBox;
    public:
        MListItem(MListBox *listbox, const char *name = NULL);

        void click();

        // overrides
        bool focus();
    protected:
        void setIndex(int index) {_index = index;}
        // overrides
        void draw(const MRect &cliprect);
        bool keyDown(int ascii, int modifiers, int scancode);

        int _index; /* relative index */
    };
    friend class MListItem;
    virtual MListItem *createItem();
    void click(int index /* relative index */);

    // overrides
    void setGeometry(const MRect &rect);
    virtual void layout();

    int _indent;
    MStringList _values;
    MList<MListItem> _items;
    int _topIndex;
    MScrollBar *_bar;
    MScrollBar _defaultBar;
    bool _captureUp, _captureDown, _highlightCurrent, _requireClickToMakeCurrent;

private:
    void resetData();
    void init(int scrollBarWidth);
    bool _updating;
    int _currentIndex;

    SUPPORT_EV(MListBox)
    SUPPORT_EV_METHOD(ItemClicked)
    SUPPORT_EV_METHOD(CurrentItemChanged)
    SUPPORT_EV_METHOD(InsertItem)
    SUPPORT_EV_METHOD(DeleteItem)
};

///////////////////////////////////////

#define LISTBOX ((MListBox*)parent())

class MColumnListBox : public MListBox {
public:
    MColumnListBox(MWidget *parent, const char *name = NULL);
    MColumnListBox(MWidget *parent, const MRect &rect, int scrollBarWidth = 15, const char *name = NULL);

    void setDelimeter(char delim) {_delim = delim;}
    void setFocusImage(MImage * pImg = NULL) {_imgFocus = pImg;}

    void addColumn(int width,
        MPainter::Alignment al = MPainter::alLeft,
        MPainter::VAlignment val = MPainter::valCenter) {_cols.add(new Column(width,al,val));}
    void removeColumn(int index) {_cols.remove(index);}
    int totalColumns() const {return _cols.total();}

protected:
    struct Column {
        int size;
        MPainter::Alignment al;
        MPainter::VAlignment val;
        Column(int s, MPainter::Alignment a, MPainter::VAlignment va) {
            size=s;al=a;val=va;
        }
    };
    MAutoList<Column> _cols;
    char _delim;

    MImage * _imgFocus;

    class MColumnListItem : public MListItem {
    public:
        MColumnListItem(MListBox *listbox, const char *name = NULL) :
            MListItem(listbox, name) {}
    protected:
        virtual void draw(const MRect &cliprect);
        virtual void drawColumn(
            MPainter &ptr, int col, const char *text, int textlen, const MRect &rect,
            MPainter::Alignment al, MPainter::VAlignment val);
    };
    friend class MColumnListItem;
    MListItem *createItem();
};

#endif //MLISTBOX_H
