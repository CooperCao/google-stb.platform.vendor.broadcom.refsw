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

#include "mlistview.h"
#include "mpainter.h"
#include "mpixmap.h"
#include "mapplication.h"

BDBG_MODULE(mlistview);
#define LIST_HBORDER 5
#define MAX_FULL_SIZE_THUMB_RESOLUTION (640 * 480)

MListViewItem::MListViewItem(MListView *v, MWidget *parent, const char *name) :
    MPushButton(parent, NULL, name)
{
    _view = v;
    _image = NULL;
    setAlignment(MPainter::alLeft);
    setVAlignment(MPainter::valCenter);
    setTransparent(true);
}

MListViewItem::~MListViewItem()
{
}

int MListViewItem::measureWidth() const
{
    return MFontMetrics(currentFont()).width(_label, _label.length());
}

void MListViewItem::focusRepaint()
{
    if (hasFocus())
    {
        // much more efficient
        MPainter ptr(this);
        ptr.drawFocus(clientRect());
    }
    else
    {
        repaint();
    }
}

void MListViewItem::draw(const MRect &cliprect)
{
    /* This cliprect should be the intersect of the cliprect passed in and
    the viewable area. rectVisible is the MListView viewable area. */
    MRect rectVisible = view()->clipper()->rect();
    rectVisible = rectVisible.intersection(cliprect);

    /* use the modified rectVisible for the cliprect */
    MPainter ptr(this, rectVisible);
    ptr.setFont(currentFont());
    ptr.setPen(textColor());

    BDBG_MSG(("draw %d,%d,%d,%d%s", x(),y(),width(),height(),hasFocus()?" (focus)":""));
    switch (_view->view())
    {
    case MListView::ListView:
    case MListView::DetailedListView:
        {
        MRect r = clientRect();
        r.moveBy(3,0);

        int nDropShadow = parent()->dropShadow();
        if (nDropShadow > 0)
        {
            int c = ptr.pen();
                if ((c & 0x00FFFFFF | 0xFF000000) == 0xFF000000)
            {
                //text is black so use opposite dropshadow color
                ptr.setPen(~c & 0x00FFFFFF | 0xFF000000);
            }
            else
            {
                ptr.setPen(style()->color(MStyle::LABEL_DROPSHADOW));
            }

            MRect rectDropShadow = r;
            rectDropShadow.moveBy(nDropShadow, nDropShadow);
            ptr.drawText(rectDropShadow, _label, _label.length(), _al, _val);

            ptr.setPen(c);
        }

        ptr.setPen(hasFocus() ? parent()->textColorFocus() : parent()->textColor());
        ptr.drawText(r, _label, _label.length(), _al, _val);

        }
        break;

    case MListView::IconView:
        int fh = fontMetrics().height();
        int h = height()-fh;
        int pw = width()-_view->_iconborder*2;
        int ph = h-_view->_iconborder*2;

        BDBG_MSG(("drawing item = x:%d y:%d w:%d h:%d\n", x(), y(), width(), height()));
/* cache for icons */
#if 1
        MPixmap * pixmapNew = NULL;

        if ((pixmapNew = view()->cache()->reserve(label().s())) != NULL)
        {
            //cache hit
            BDBG_MSG(("+++++++++++++++++cache hit:%s\n", label().s()));
        }
        else
        {
            BDBG_MSG(("-----------------cache miss:%s\n", label().s()));
            //convert image to dynamically allocated pixmap for cache
            MRect result = MPainter::fitRect(icon()->widthThumb(),
                                             icon()->heightThumb(),
                                             MRect(0, 0, pw, ph),
                                             MPainter::eMaximize,
                                             MPainter::alCenter,
                                             MPainter::valCenter);

            pixmapNew = new MPixmap(fb(), result.width(), result.height());

            MPainter p2(pixmapNew);

            /* If the image as alpha channel, then it's going to blend with whatever
            is in the pixmap, so set this. */
            p2.setPen(0xFF000000);
            p2.fillRect(MRect(0, 0, result.width(), result.height()));

            bool bForceFullSizeThumb = false;
            int resIcon      = icon()->width() * icon()->height();
            int resIconThumb = icon()->widthThumb() * icon()->heightThumb();
            int resDest      = pw * ph;

            //if 1. full sized image is relatively small
            //   2. thumb is over 50% smaller than destination
            // then use full sized image instead
            if ((resIcon <= MAX_FULL_SIZE_THUMB_RESOLUTION) &&
                (resIconThumb < (resDest / 2)))
            {
                BDBG_MSG(("embedded thumbnail too small using full size image for thumb:%s", label().s()));
                bForceFullSizeThumb = true;
            }

            if (bForceFullSizeThumb ||
                !(p2.drawImageThumb(*icon(), 0, 0, result.width(), result.height(),
                                    MPainter::eStretch, MPainter::alCenter, MPainter::valCenter)))
            {
                //embedded thumb too small or error drawing embedded thumbnail
                //(may not exist)

                result = MPainter::fitRect(icon()->width(),
                                           icon()->height(),
                                           MRect(0, 0, pw, ph),
                                           MPainter::eMaximize,
                                           MPainter::alCenter,
                                           MPainter::valCenter);

                if (pixmapNew)
                {
                    p2.end();
                    delete pixmapNew;
                    pixmapNew = NULL;
                }
                pixmapNew = new MPixmap(fb(), result.width(), result.height());
                p2.begin(pixmapNew);

                if (resIcon <= MAX_FULL_SIZE_THUMB_RESOLUTION)
                {
                    //draw full sized image if not too big
                    p2.drawImage(*icon(), 0, 0, result.width(), result.height(),
                                 MPainter::eStretch, MPainter::alCenter, MPainter::valCenter);
                }
                else
                if (view()->noThumbImage())
                {
                    //draw placeholder icon (if it exists)

                    result = MPainter::fitRect(view()->noThumbImage()->width(),
                                               view()->noThumbImage()->height(),
                                               MRect(_view->_iconborder, _view->_iconborder, pw, ph),
                                               MPainter::eMaximize,
                                               MPainter::alCenter,
                                               MPainter::valCenter);

                    if (pixmapNew)
                    {
                        p2.end();
                        delete pixmapNew;
                        pixmapNew = NULL;
                    }

                    ptr.drawImage(*view()->noThumbImage(),
                                  result.x(), result.y(), result.width(), result.height(),
                                  MPainter::eMaximize, MPainter::alCenter, MPainter::valCenter);
                }
                else
                {
                    //as a last resort we will just use the full image as the thumb.
                    p2.drawImage(*icon(), 0, 0, result.width(), result.height(),
                                 MPainter::eStretch, MPainter::alCenter, MPainter::valCenter);
                }
            }


            if (pixmapNew)
            {
                //add to thumbnail image cache
                BDBG_MSG(("cache %d,%d", result.width(), result.height()));
                view()->cache()->add(label().s(), pixmapNew);
            }
        }

        if (pixmapNew)
        {
            ptr.drawPixmap(*pixmapNew, _view->_iconborder, _view->_iconborder, pw, ph,
                           MPainter::eMaximize, MPainter::alCenter, MPainter::valCenter);
        }

        view()->cache()->unreserve(label().s());
        //view()->cache()->print();

#else
    // do not use icon cache
        ptr.drawImageThumb(*icon(), _view->_iconborder, _view->_iconborder, pw, ph,
            MPainter::eMaximize, MPainter::alCenter, MPainter::valCenter);
#endif

        int textWidth = fontMetrics().width(_label);
        int textX     = 0;
        if ((uint32_t)textWidth <= width())
        {
            //center text label
            textX = width() / 2 - textWidth / 2;
        }

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

            MRect rectDropShadow(textX, h, width(), fh);
            rectDropShadow.moveBy(nDropShadow, nDropShadow);
            ptr.drawText(rectDropShadow, _label, _label.length(), _al, _val);

            ptr.setPen(c);
        }

        ptr.setPen(hasFocus() ? parent()->textColorFocus() : parent()->textColor());
        BDBG_MSG(("drawicontext %d,%d,%d,%d", 0,h,width(),fh));
        ptr.drawText(MRect(textX,h,width(),fh), _label, _label.length(), _al, _val);
        break;
    }
    if (hasFocus())
        ptr.drawFocus(clientRect());

    //force sync so each thumbnail appears immediately as it renders -
    //otherwise user will have to wait until all of them are rendered
    //before seeing anything on screen.
    ptr.sync();
}

void MListViewItem::setLabel(const char *label)
{
    _label = label;
    view()->layout();
}

void MListViewItem::setDescription(const char *description)
{
    _description = description;
}

void MListViewItem::setIcon(const MImage *image)
{
    _image = image;
    if (view()->view() == MListView::IconView)
        view()->layout();
}

//////////////////////////////////////////

MListView::MListView(MWidget *parent, const MRect &rect, const char *name) :
    MScrollView(parent, rect, name)
{
    _view = IconView;
    _updating = false;
    _iconwidth = -1;
    _iconborder = 15;
    _selected = NULL;
    _iconNoThumbnail = NULL;
    _cacheThumbs.clear();
    _cacheThumbs.setMaxElements(30);
}

MListView::~MListView()
{
    _cacheThumbs.clear();
}

void MListView::setSelected(MListViewItem *selected)
{
    if (selected != _selected)
    {
        //TODO: make customizable
        if (_selected)
        {
            _selected->setTransparent(true);
            _selected->setTextColor(-1);
        }
        _selected = selected;
        if (selected)
        {
            selected->setBackgroundColor(0xff117711);
        }
    }
}

MListViewItem *MListView::insert(int index)
{
    MListViewItem *item = createItem();
    item->addHandler(this);
    item->show();
    _items.insert(index, item);
    layout();
    return item;
}

MListViewItem *MListView::add()
{
    return insert(_items.total());
}

void MListView::remove(MListViewItem *item)
{
    if (item == _selected)
        _selected = NULL;
    _items.remove(item);
    layout();
    delete item;
}

void MListView::remove(int index)
{
    MListViewItem *item = _items.remove(index);
    if (item)
    {
        if (item == _selected)
            _selected = NULL;
        layout();
        delete item;
    }
}

void MListView::setView(View view)
{
    _view = view;
#if 0
    beginUpdates();
    setVScrollBarMode(AlwaysOff);
    setHScrollBarMode(AlwaysOn);
    endUpdates();
#endif
}

void MListView::setNoThumbImage(MImage * iconNoThumbnail)
{
    _iconNoThumbnail = iconNoThumbnail;
}

MListViewItem *MListView::createItem()
{
    return new MListViewItem(this, viewport());
}

void MListView::clear()
{
    _items.clear();
    _selected = NULL;
}

void MListView::layout()
{
    int rowsVisible     =  0;
    int rowsTotal       =  0;
    int lastY           = -1;

    if (_updating)
        return;
    BDBG_MSG(("\n##################### MListView::layout\n"));

    int x = 0;
    int y = 0;
    int itemwidth = 0, rowheight = 0;
    int maxcolwidth = -1;
    int maxrows = -1;

    switch (_view)
    {
    case ListView:
    case DetailedListView:
        rowheight = MFontMetrics(currentFont()).height();
        break;
    case IconView:
        if (_iconwidth == -1)
            itemwidth = clipper()->width() / 4;
        else
            itemwidth = _iconwidth + _iconborder*2;
        rowheight = itemwidth;
        break;
    }

    if (hScrollBarMode() == AlwaysOff)
    {
        // precalc maxwidth to determine how many columns
        for (MListViewItem *item = _items.first(); item; item = _items.next())
        {
            int w = item->measureWidth();
            if (w > maxcolwidth)
                maxcolwidth = w;
        }
        maxcolwidth += LIST_HBORDER*2;
        int cols = width() / maxcolwidth;
        if (cols == 0) cols++;
        maxrows = (_items.total() + cols - 1) / cols;
        //printf("%d x %d\n", cols, maxrows);
    }

    for (MListViewItem *item = _items.first(); item; item = _items.next())
    {

        switch (_view)
        {
        case ListView:
        case DetailedListView:
            {
                rowheight = 27;
                int w = item->measureWidth() + LIST_HBORDER*2;
                if (maxcolwidth == -1)
                {
                    if (w < 20)
                        w = 20;
                    if (w > maxcolwidth)
                        maxcolwidth = w;
                }

                //printf("ListView::layout %s %d,%d,%d,%d (%s)\n",name(),x,y,w,rowheight, (const char *)item->label());
                item->setGeometry(x,y,w,rowheight);
                y += rowheight;

                if (hScrollBarMode() != AlwaysOff)
                {
                    if (y+rowheight >= (int)clipper()->height())
                    {
                        y = 0;
                        x += maxcolwidth + 5;
                        maxcolwidth = 0;
                    }
                }
                else
                {
                    if (y / rowheight == maxrows)
                    {
                        y = 0;
                        x += maxcolwidth + 5;
                    }
                }
            }
            break;
        case IconView:
            item->setGeometry(x,y,itemwidth,rowheight);
            x += itemwidth;
            if (x + itemwidth > (int)clipper()->width())
            {
                y += rowheight;
                x = 0;
            }
            break;
        }

        if (item->y() != lastY)
        {
            //count total rows
            rowsTotal++;
        }

        if ((item->y() + rowheight) < (int)clipper()->height())
        {
        item->show();
        item->drawNow();

            if (item->y() != lastY)
            {
                //count visible rows
                rowsVisible++;
            }
        }

        lastY = item->y();
    }

    // sets scrollbars
    MScrollView::layout();

    if (rowsTotal > 0)
    {
        _vsb->setIndicatorWidth(rowsVisible * 100 / rowsTotal);
    }

    //TODO: set indicator width for _hsb

    FIRE(Layout);
}

void MListView::onClick(MButton *sender)
{
    FIRE(Click);
}

FIRE_EV_METHOD(MListView, Click)
FIRE_EV_METHOD(MListView, Layout)

MListViewItem *MListView::current()
{
    MWidget *fw = app()->focusedWidget();
    if (!fw || fw->parent() != viewport() || strcmp(fw->className(), "MListViewItem"))
        return NULL;
    else
        return(MListViewItem *)fw;
}

int MListView::currentIndex()
{
    return _items.index(current());
}

int MListView::index(MListViewItem *item)
{
    return _items.index(item);
}

MListViewItem *MListView::item(int index)
{
    return _items.at(index);
}

void MListView::setIconWidth(int iconwidth, int iconborder)
{
    _iconwidth = iconwidth;
    _iconborder = iconborder;
}
