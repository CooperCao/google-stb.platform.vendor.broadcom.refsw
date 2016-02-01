/***************************************************************************
 *     (c)2014 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 **************************************************************************/
#include "mlineedit.h"
#include "mpainter.h"

MLineEdit::MLineEdit(MWidget *parent, const MRect &rect, const char *name) :
    MWidget(parent, rect, name)
{
    init();
}

MLineEdit::MLineEdit(MWidget *parent, const char *name) :
    MWidget(parent, name)
{
    init();
}

void MLineEdit::init() {
    setTransparent(false); /* we'll paint the background though */
    setTextColor(style()->color(MStyle::LINEEDIT_TEXT));
    setFocusable(true);
    _deleteDigitsOnly = false;
    _maxLength = 256;
    _pos = 0;
    _bevel = 4;
    _textBorder = 2;
    _echoMode = Normal;
}

void MLineEdit::setText(const char *t) {
    _text = t;
    if (_text)
        _textlen = strlen(_text);
    else
        _textlen = 0;
    end(); // this does the repaint
}

void MLineEdit::setMaxLength(int ml) {
    _maxLength = ml;
    if (_text.length() > _maxLength) {
        _text.setLength(_maxLength);
        repaint();
    }
}

void MLineEdit::draw(const MRect &cliprect) {
    MPainter ptr(this, cliprect);
    ptr.setFont(currentFont());

    if (hasFocus()) {
        ptr.drawFocus(clientRect(), style()->color(MStyle::LINEEDIT_FOCUSBORDER),
            style()->prop(MStyle::BUTTON_ROUNDING));
    }
    else
        ptr.drawBevel(clientRect(), _bevel, -1,-1,-1,-1,style()->prop(MStyle::BUTTON_ROUNDING));

    // face
    MRect textRect(_bevel,_bevel,width()-_bevel*2,height()-_bevel*2);
    ptr.setBrush(style()->color(MStyle::LINEEDIT_BACKGROUND, enabled()));
    ptr.fillRect(textRect);

    int indent = _bevel + _textBorder;
    textRect = MRect(indent, indent,width()-indent*2,height()-indent*2);

    ptr.setPen(textColor());

    MString drawtext;
    switch (_echoMode) {
    case Password: drawtext.setLength(length(), '*'); break;
    case Normal: drawtext = _text; break;
    // do nothing for NoEcho
    }

    // draw before cursor
    int w = 0, h;
    if (pos() > 0 && drawtext.length()) {
        ptr.measureText(drawtext, pos(), &w, &h);
        ptr.drawText(textRect, drawtext, pos(),
            MPainter::alLeft, MPainter::valCenter);
    }

    // draw cursor
    w += indent;
    if (enabled()) {
        if (hasFocus()) {
            ptr.setBrush(textColor());
            ptr.fillRect(MRect(w,indent,2,height()-indent*2));
        }
    }
    textRect.setX(w);

    // draw after cursor
    if (pos() < length() && drawtext.length())
        ptr.drawText(textRect, ((const char *)drawtext)+pos(), length()-pos(),
            MPainter::alLeft, MPainter::valCenter);
}

bool MLineEdit::keyDown(int ascii, int modifiers, int scancode) {
    if (!enabled())
        return false;

    switch (ascii) {
    case MWKEY_BACKSPACE: backspace(); break;
    case MWKEY_DELETE: del(); break;
    case MWKEY_HOME:
        if (pos() == 0)
            return false;
        home();
        break;
    case MWKEY_END:
        if (pos() == length())
            return false;
        end();
        break;
    case MWKEY_LEFT:
        if (pos() == 0)
            return false;
        setPos(pos()-1);
        break;
    case MWKEY_RIGHT:
        if (pos() == length())
            return false;
        setPos(pos()+1);
        break;
    default:
        if (ascii >= 32 && ascii <= 255) {
            addChar(ascii);
        }
        else
            return false;
        break;
    }
    return true;
}

void MLineEdit::setPos(int pos) {
    if (!enabled())
        return;

    if (pos < 0) pos = 0;
    if (pos > length()) pos = length();
    _pos = pos;
    repaint();
}

#define DIGITS "0123456789"

void MLineEdit::backspace() {
    if (!enabled())
        return;

    if (_pos  && (!_deleteDigitsOnly || strchr(DIGITS,_text[_pos-1]))) {
        _text.removeChar(--_pos);
        repaint();
    }
}

void MLineEdit::del() {
    if (!enabled())
        return;

    if (length() && (!_deleteDigitsOnly || strchr(DIGITS,_text[_pos]))) {
        _text.removeChar(_pos);
        repaint();
    }
}

void MLineEdit::addChar(char ch) {
    if (!enabled())
        return;

    if (length() < maxLength()) {
        _text.insert(_pos++, ch);
        repaint();
    }
}
