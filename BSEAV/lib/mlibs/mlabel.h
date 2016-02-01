/******************************************************************************
 * (c) 2001-2014 Broadcom Corporation
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


#ifndef MLABEL_H
#define MLABEL_H

#include "mwidget.h"
#include "mpainter.h"

class MLabel : public MWidget {
public:
    MLabel(MFrameBuffer *fb, const MRect &rect, const char *text = NULL, const char *name = NULL);

    MLabel(MWidget *parent, const char *name = NULL);
    MLabel(MWidget *parent, const MRect &rect, const char *text = NULL, const char *name = NULL);
    MLabel(MWidget *parent, const MRect &rect, const MImage *image, const char *name = NULL);
    MLabel(MWidget *parent, const MImage *image, int bevelWidth = 0, const char *name = NULL);
    MLabel(MWidget *parent, const MRect &rect,
        const MImage *leftimage, const MImage *centerimage, const MImage *rightimage,
        const char *text = NULL, const char *name = NULL);

    const MImage *image() const {return _image;}
    void setImage(const MImage *image,
        MPainter::DrawImageMode drawImageMode = MPainter::eSingle);

    void setBevel(int width, MPainter::BevelStyle style = MPainter::bsRaised);
    // The border is the space between the inside of the bevel and any text
    void setBorder(int width) {_border = width;}

    void setAlignment(MPainter::Alignment al) {_al = al;}
    void setVAlignment(MPainter::VAlignment val) {_val = val;}

    enum WrapMode {WordWrap, NewLineWrap, NoWrap};
    void setWrapMode(WrapMode mode) {_wrapMode = mode;}

    // overrides
    void focusRepaint() {}

protected:
    virtual void draw(const MRect &cliprect);

    MPainter::Alignment _al;
    MPainter::VAlignment _val;
    MPainter::DrawImageMode _drawImageMode;
    WrapMode _wrapMode;
    const MImage *_image, *_leftimage, *_rightimage;
    int _bevelWidth, _border;
    MPainter::BevelStyle _bevelStyle;

private:
    void init();

    struct TextLine {
        const char *start;
        int len, y, h;
        TextLine(const char *s, int l, int ay, int ah) {
            start = s;
            len = l;
            y = ay;
            h = ah;
        }
    };
    void measureText(MPainter &ptr, MList<TextLine> &list);
    void parseNewLineText(MPainter &ptr, MList<TextLine> &list);
    void drawMeasuredText(MPainter &ptr, MList<TextLine> &list);
};

#endif //MLABEL_H
