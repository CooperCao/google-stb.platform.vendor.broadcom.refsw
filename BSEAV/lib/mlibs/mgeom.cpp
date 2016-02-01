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


#include "mgeom.h"

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

bool MRect::intersects(const MRect &rect) const {
    return
        rect.right() >= x() &&
        rect.bottom() >= y() &&
        rect.x() <= right() &&
        rect.y() <= bottom();
}

MRect MRect::intersection(const MRect &rect) const {
    int xx = max(x(), rect.x());
    int yy = max(y(), rect.y());
    int r = min(right(), rect.right());
    int b = min(bottom(), rect.bottom());
    if (r <= xx || b <= yy)
        return MRect();
    else
        return MRect(xx,yy,r - xx,b - yy);
}

MRect MRect::combine(const MRect &rect) const {
    int xx = min(x(), rect.x());
    int yy = min(y(), rect.y());
    return MRect(xx,yy,
        max(right(), rect.right()) - xx,
        max(bottom(), rect.bottom()) - yy);
}

MRect MRect::minus(const MRect &rect) const {
    MRect r = intersection(rect);
    if (r.isNull())
        return *this;

    // test where rect is "bigger" than this
    int tx = (rect.x()<=x())?1:0;
    int ty = (rect.y()<=y())?1:0;
    int tr = (rect.right()>=right())?1:0;
    int tb = (rect.bottom()>=bottom())?1:0;
    int sum = tx+ty+tr+tb;

    if (sum == 3) {
        int xx = tr?x():rect.right();
        int yy = tb?y():rect.bottom();
        return MRect(
            xx,
            yy,
            (tx?right():rect.x()) - xx,
            (ty?bottom():rect.y()) - yy);
    }
    else if (sum == 4) {
        return MRect();
    }
    else
        return *this;
}

void MRect::grow(int amount) {
    _width += amount;
    _height += amount;
}

bool MRect::contains(const MRect &rect) const {
    return
        x() <= rect.x() &&
        y() <= rect.y() &&
        right() >= rect.right() &&
        bottom() >= rect.bottom();

}
