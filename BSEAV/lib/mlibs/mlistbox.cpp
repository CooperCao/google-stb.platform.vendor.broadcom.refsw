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

#include "mlistbox.h"
#include "mapplication.h"
#include <bstd.h>
BDBG_MODULE(mlistbox);

#define NO_LINES

MListBox::MListBox(MWidget *parent, const char *name) :
    MWidget(parent, name), _defaultBar(this) //, false, true)
{
    init(15);
}

MListBox::MListBox(MWidget *parent, const MRect &rect, int scrollBarWidth, const char *name) :
    MWidget(parent, rect, NULL, name), _defaultBar(this) //, false, true)
{
    init(scrollBarWidth);
}

void MListBox::init(int scrollBarWidth) {
    _requireClickToMakeCurrent = false;
    _highlightCurrent = false;
    _captureUp = false;
    _captureDown = false;
    _currentIndex = -1;
    _topIndex = 0;
    _bar = NULL;
    _updating = false;
    _indent = 0;
    setScrollBar(&_defaultBar, scrollBarWidth);
    setFocusable(true);
}

void MListBox::beginUpdates() {
    _updating = true;
}

void MListBox::endUpdates() {
    _updating = false;
    resetData();
}

void MListBox::add(const char *text) {
    _values.add(text);
    resetData();
}

void MListBox::insert(int index, const char *text) {
    _values.insert(index, text);
    resetData();
}

void MListBox::change(int index, const char *text) {
    _values.change(index, text);
    resetData();
}

int MListBox::find(const char *text) {
    return _values.index(text);
}

const char *MListBox::item(int index) {
    return _values.at(index);
}

void MListBox::remove(int index) {
    _values.remove(index);
    resetData();
}

void MListBox::clear() {
    _values.clear();
    resetData();
}

void MListBox::setGeometry(const MRect &rect) {
    MWidget::setGeometry(rect);
    if (_bar)
        _bar->setGeometry(MRect(width()-_bar->width(),0,_bar->width(),height()));
    layout();
}

void MListBox::layout() {
    int y = 0;
#ifdef NO_LINES
    int yinc = _items.total() ? (height()-4) / _items.total() : 10;
#else
    int yinc = _items.total() ? height() / _items.total() : 10;
#endif
    int w = width();
    if (_bar)
        w -= _bar->width();
    for (MListItem *item = _items.first(); item; item = _items.next()) {
        item->setGeometry(MRect(0,y,w,yinc));
        y += yinc;
    }
}

int MListBox::focusIndex() {
    for (MListItem *item = _items.first(); item; item = _items.next())
        if (item->hasFocus())
            return item->_index + _topIndex;
    return -1;
}

bool MListBox::setFocus(int i) {
    // make sure it's a valid absolute index number
    if (i < 0)
        i = 0;
    else if (i >= total())
        i = total()-1;

    if (!_requireClickToMakeCurrent)
        _currentIndex = i;

    // bring it within view
    if (i < _topIndex) {
        _topIndex = i;
        resetData();
    }
    else if (i >= _topIndex + totalVisible()) {
        _topIndex = i - totalVisible() + 1;
        resetData();
    }

    BDBG_MSG(("setFocus %d (cur=%d,top=%d)", i, _currentIndex, _topIndex));
    MListItem *item = _items.at(i - _topIndex);
    if (item && item->focusable() && !item->hasFocus() && (!_bar || !_bar->containsFocus()))
        return item->focus();
    return false;
}

void MListBox::setCurrentIndex(int i, bool changeFocus) {
    // make sure it's a valid number
    if (i < 0)
        i = 0;
    else if (i >= total())
        i = total()-1;

    BDBG_MSG(("setCurrentIndex %d,focus=%d (cur=%d,top=%d)", i, changeFocus, _currentIndex, _topIndex));
    /* We might be changing _currentIndex and/or _topIndex, so always
    process it. */
    if (_highlightCurrent) {
        MListItem *item;
        item = _items.at(_currentIndex - _topIndex);
        if (item) item->setTransparent(true);
    }

    _currentIndex = i;

    if (changeFocus) {
        setFocus(i);
    }

    if (_highlightCurrent) {
        MListItem *item;
        item = _items.at(_currentIndex - _topIndex);
        if (item) item->setBackgroundColor(style()->color(MStyle::MESSAGEBOX_BACKGROUND));
    }
    FIRE(CurrentItemChanged);
}

void MListBox::setTopIndex(int topIndex) {
    // make sure it's a valid number
    if (topIndex < 0)
        topIndex = 0;
    else if (topIndex >= total())
        topIndex = total()-1;

    if (_topIndex != topIndex) {
        BDBG_MSG(("setTopIndex %d, %d", topIndex, _currentIndex));
        _topIndex = topIndex;
        setFocus(_currentIndex);
    }
}

void MListBox::setTotalVisible(int totalVisible) {
    while (totalVisible < _items.total()) {
        MListItem *item = _items.remove(_items.total()-1);
        delete item;
    }
    while (totalVisible > _items.total()) {
        MListItem *item = createItem();
        item->setIndex(_items.total());
        _items.add(item);
    }
    layout();
    resetData();
}

void MListBox::resetData() {
    int i;
    const char *s;
    MListItem *item;

    if (_updating)
        return;

    // make sure _currentIndex and _topIndex are coherent
    // -1 means no items
    if (!_values.total()) {
        _currentIndex = -1;
        _topIndex = 0;
    }
    else if (_currentIndex == -1) {
        _currentIndex = 0;
        _topIndex = 0;
    }
    else if (_currentIndex >= _values.total()) {
        if (_values.total()) {
            _currentIndex = _values.total()-1;
            if (_topIndex) {
                setTopIndex(_values.total() - totalVisible());
            }
            else {
                //the current item was deleted, so bump up, set focus and continue
                MListItem *item = _items.at(_currentIndex - _topIndex);
                if (item && containsFocus() && (!_bar || !_bar->containsFocus()))
                    item->focus();
            }
        }
        else {
            _currentIndex = -1;
        }
    }

    BDBG_MSG(("resetData: top=%d, cur=%d", _topIndex, _currentIndex));
    for (i=0,s = _values.at(_topIndex),item = _items.first();
        item;
        item = _items.next(), s = _values.next(),i++)
    {
        if (s) {
            item->setText(s);
            item->setFocusable(true);
        }
        else {
            item->setText(NULL);
            item->setFocusable(false);
        }
        if (_highlightCurrent) {
            if (i + _topIndex == _currentIndex)
                item->setBackgroundColor(style()->color(MStyle::MESSAGEBOX_BACKGROUND));
            else
                item->setTransparent(true);
        }
    }
    // repaint just the non-bar section
    repaint(MRect(0,0,width()-(_bar?_bar->width():0),height()));

    if (_bar) {
        _bar->setMaxValue(_values.total());
        _bar->setIndicatorWidth(totalVisible());
        _bar->setValue(_topIndex);
        _bar->setSteps(1,1);
    }

    if (_currentIndex == -1 && containsFocus())
        throwFocus();
}

MListBox::MListItem *MListBox::createItem() {
    return new MListItem(this, "listitem");
}

FIRE_EV_METHOD(MListBox, ItemClicked)
FIRE_EV_METHOD(MListBox, CurrentItemChanged)
FIRE_EV_METHOD(MListBox, InsertItem)
FIRE_EV_METHOD(MListBox, DeleteItem)

void MListBox::onScroll(MScrollBar *scrollBar) {
    setTopIndex(scrollBar->value());
}

void MListBox::setScrollBar(MScrollBar *bar, int w) {
    if (bar != _bar) {
        if (_bar) {
            _bar->removeHandler(this);
            _bar->hide();
        }
        _bar = bar;
        if (_bar) {
            _bar->addHandler(this);
            _bar->setGeometry(MRect(width()-w,0,w,height()));
        }
    }
}

void MListBox::click(int index) {
    FIRE(ItemClicked);
}

bool MListBox::focus() {
    if (!_requireClickToMakeCurrent)
        return setFocus(_currentIndex);
    else {
        MWidget *from = fb()->app()->focusedWidget();
        if (from && from->globalY() > globalY() + (int)height()/2)
            return setFocus(_topIndex + totalVisible() - 1);
        else
            return setFocus(_topIndex);
    }
}

void MListBox::setHighlightCurrent(bool highlightCurrent)
{
    _highlightCurrent = highlightCurrent;
    repaint();
}

void MListBox::setRequireClickToMakeCurrent(bool requireClickToMakeCurrent)
{
    _requireClickToMakeCurrent = requireClickToMakeCurrent;
}

////////////////////////////////////////////

#define LISTBOX ((MListBox*)parent())

MListBox::MListItem::MListItem(MListBox *listbox, const char *name) :
    MWidget(listbox, name)
{
    _index = -1;
}

void MListBox::MListItem::draw(const MRect &cliprect) {
    MPainter p(this, cliprect);
    p.setFont(currentFont());
    p.setPen(hasFocus() ? textColorFocus() : textColor());
    p.setBrush(-1);
    MRect r = MRect(LISTBOX->indent(), 0, width()-LISTBOX->indent(), height());
    p.drawText(r, _text, _textlen, MPainter::alLeft, MPainter::valCenter);

    if (hasFocus())
        p.drawFocus(clientRect(), -1, style()->prop(MStyle::BUTTON_ROUNDING));
#ifndef NO_LINES
    else if (_index > 0) {
        p.fillRect(MRect(0,0,width(),2));
    }
#endif
}

void MListBox::MListItem::click() {
    if (LISTBOX->_requireClickToMakeCurrent) {
        LISTBOX->setCurrentIndex(_index + LISTBOX->topIndex(), false);
    }
    LISTBOX->click(_index);
}

bool MListBox::MListItem::focus() {
    BDBG_MSG(("MListItem::focus %d", _index + LISTBOX->topIndex()));
    bool rc = MWidget::focus();
    if (!LISTBOX->_requireClickToMakeCurrent) {
        LISTBOX->setCurrentIndex(_index + LISTBOX->topIndex(), false);
    }
    return rc;
}

bool MListBox::MListItem::keyDown(int ascii, int modifiers, int scancode) {
    switch (ascii) {
    case 13:
    case ' ':
        click();
        break;
    case MWKEY_INSERT:
        LISTBOX->FIRE(InsertItem);
        break;
    case MWKEY_DELETE:
        if (LISTBOX->total())
            LISTBOX->FIRE(DeleteItem);
        break;
    case MWKEY_HOME:
        LISTBOX->setFocus(0);
        break;
    case MWKEY_END:
        LISTBOX->setFocus(LISTBOX->total()-1);
        break;
    case MWKEY_UP:
        if (LISTBOX->_captureUp && LISTBOX->focusIndex() > 0)
            LISTBOX->setFocus(LISTBOX->focusIndex()-1);
        else
            return false;
        break;
    case MWKEY_DOWN:
        if (LISTBOX->_captureDown && LISTBOX->focusIndex()<LISTBOX->total()-1)
            LISTBOX->setFocus(LISTBOX->focusIndex()+1);
        else
            return false;
        break;
    case MWKEY_PAGEUP:
        LISTBOX->setFocus(LISTBOX->topIndex() - LISTBOX->totalVisible());
        break;
    case MWKEY_PAGEDOWN:
        LISTBOX->setFocus(LISTBOX->topIndex() + LISTBOX->totalVisible()
            + LISTBOX->totalVisible() - 1);
        break;
    default:
        return false;
    }
    return true;
}

///////////////////////////////////////////////

MColumnListBox::MColumnListBox(MWidget *parent, const char *name) :
    MListBox(parent, name)
{
    _delim = '\t';
    _imgFocus = NULL;
}

MColumnListBox::MColumnListBox(MWidget *parent, const MRect &rect, int scrollBarWidth, const char *name) :
    MListBox(parent, rect, scrollBarWidth, name)
{
    _delim = '\t';
    _imgFocus = NULL;
}

////////////////////////////////////////////

void MColumnListBox::MColumnListItem::drawColumn(
    MPainter &ptr, int col, const char *text, int textlen, const MRect &rect,
    MPainter::Alignment al, MPainter::VAlignment val)
{
    int nDropShadow = parent()->dropShadow();
    if (nDropShadow > 0)
    {
        int c = ptr.pen();
        if (c & 0x00FFFFFF | 0xFF000000 == 0xFF000000)
        {
            //text is black so use opposite dropshadow color
            ptr.setPen(~c & 0x00FFFFFF | 0xFF000000);
        }
        else
        {
            ptr.setPen(style()->color(MStyle::LABEL_DROPSHADOW));
        }

        MRect rectDropShadow = rect;
        rectDropShadow.moveBy(nDropShadow, nDropShadow);
        ptr.drawText(rectDropShadow, text, textlen, al, val);

        ptr.setPen(c);
    }

    ptr.setPen(hasFocus() ? LISTBOX->textColorFocus() : LISTBOX->textColor());
    ptr.drawText(rect, text, textlen, al, val);
}

void MColumnListBox::MColumnListItem::draw(const MRect &cliprect) {
    MPainter p(this, cliprect);

    MColumnListBox *box = (MColumnListBox*)parent();

    p.setFont(currentFont());
    p.setPen(hasFocus() ? LISTBOX->textColorFocus() : LISTBOX->textColor());
    p.setBrush(-1);

    if (hasFocus() && box->_imgFocus)
    {
        //draw image based focus highlight first
        p.drawImage(*box->_imgFocus, cliprect.x(), cliprect.y(), cliprect.width(),
                    cliprect.height(), MPainter::eTile);
    }

    MStringList list;
    MStringList::split(_text, box->_delim, list);

    int x = LISTBOX->indent();
    Column *c;
    const char *s;
    int col = 0;
    for (c = box->_cols.first(), s = list.first(); c && s; c = box->_cols.next(), s = list.next(), col++) {
        //p.drawText(*s, -1, MRect(x,0,c->size,height()), c->al, c->val);
        drawColumn(p, col, s, -1, MRect(x,0,c->size,height()), c->al, c->val);
        x += c->size;
    }

    if (!box->_imgFocus)
    {
        if (hasFocus())
            p.drawFocus(clientRect(), -1, style()->prop(MStyle::BUTTON_ROUNDING));
#ifndef NO_LINES
        else if (_index > 0) {
            p.setPen(textColor());
            p.fillRect(MRect(0,0,width(),2));
        }
#endif
    }
}

MListBox::MListItem *MColumnListBox::createItem() {
    return new MColumnListItem(this, "columnlistitem");
}
