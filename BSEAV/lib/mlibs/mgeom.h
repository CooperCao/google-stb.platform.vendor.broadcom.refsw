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


#ifndef MGEOM_H
#define MGEOM_H

class MRect {
public:
    MRect() {set(0,0,0,0);}
    MRect(int x, int y, unsigned w, unsigned h) {set(x,y,w,h);}

    void set(int x, int y, unsigned w, unsigned h) {
        _x = x;
        _y = y;
        _width = w;
        _height = h;
    }
    int x() const {return _x;}
    int y() const {return _y;}
    unsigned width() const {return _width;}
    unsigned height() const {return _height;}
    // derived
    int right() const {return x()+(int)width();}
    int bottom() const {return y()+(int)height();}
    int midX() const {return x()+(int)width()/2;}
    int midY() const {return y()+(int)height()/2;}

    void setX(int x) {_x = x;}
    void setY(int y) {_y = y;}
    void setRight(int r) {_width = r-_x;}
    void setBottom(int b) {_height = b-_y;}
    void setWidth(unsigned w) {_width = w;}
    void setHeight(unsigned h) {_height = h;}

    void setSize(unsigned w, unsigned h) {_width = w; _height = h;}
    void moveTo(int x, int y) {_x = x; _y = y;}
    void moveBy(int dx, int dy) {_x += dx; _y += dy;}

    bool isNull() {return _width <= 0 || _height <= 0;}

    /**
    * Returns true if the rectangles intersect
    */
    bool intersects(const MRect &rect) const;

    /**
    * Returns the rectangle of intersection
    */
    MRect intersection(const MRect &rect) const;

    bool contains(const MRect &rect) const;

    /**
    * Returns the small rectangle that contains both rectangles.
    */
    MRect combine(const MRect &rect) const;

    /**
    * Returns the smallest rectangle that contains what's left
    * after substracting rect.
    */
    MRect minus(const MRect &rect) const;

    /**
    * Increase both width and height. If amount<0, then it shrinks the MRect.
    */
    void grow(int amount);

protected:
    int _x, _y;
    unsigned _width, _height;
};

inline bool operator == (const MRect &r1, const MRect &r2) {
    return
        r1.x() == r2.x() &&
        r1.y() == r2.y() &&
        r1.width() == r2.width() &&
        r1.height() == r2.height();
}

inline bool operator != (const MRect &r1, const MRect &r2) {
    return !(r1 == r2);
}

class MPoint {
public:
    MPoint() {_x = _y = 0;}
    MPoint(int x, int y) {_x = x; _y = y;}
    int x() const {return _x;}
    int y() const {return _y;}
    void setX(int x) {_x = x;}
    void setY(int y) {_y = y;}

protected:
    int _x, _y;
};

inline MPoint operator +(const MPoint &p1, const MPoint &p2) {
    return MPoint(p1.x()+p2.x(),p1.y()+p2.y());
}
inline MPoint operator -(const MPoint &p1, const MPoint &p2) {
    return MPoint(p1.x()-p2.x(),p1.y()-p2.y());
}

class MSize {
public:
    MSize() {_width = _height = 0;}
    MSize(unsigned w, unsigned h) {_width = w; _height = h;}
    unsigned width() const {return _width;}
    unsigned height() const {return _height;}
    void setWidth(unsigned w) {_width = w;}
    void setHeight(unsigned h) {_height = h;}

protected:
    unsigned _width, _height;
};

#endif //MGEOM_H
