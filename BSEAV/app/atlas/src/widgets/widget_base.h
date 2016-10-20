/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/

#ifndef ATLAS_WIDGET_BASE_H__
#define ATLAS_WIDGET_BASE_H__

#include "atlas.h"
#include "mgeom.h"
#include "bwidgets.h"
#include "view.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GRADIENT_TOP     COLOR_EGGSHELL + COLOR_STEP
#define GRADIENT_MIDDLE  COLOR_EGGSHELL
#define GRADIENT_BOTTOM  COLOR_EGGSHELL - COLOR_STEP

typedef enum bevelParts_t
{
    bevel_eTop,
    bevel_eRight,
    bevel_eBottom,
    bevel_eLeft,
    bevel_eMax /* sentinal */
} bevelParts_t;

typedef enum backgroundFillMode_t
{
    fill_eSolid,
    fill_eGradient,
    fill_eNone
} backgroundFillMode_t;

class CPoint
{
public:
    CPoint(
            float x = 0,
            float y = 0
            ) : _x(x), _y(y) {}

    bool operator   ==(CPoint v) { return(((_x == v._x) && (_y == v._y)) ? true : false); }
    CPoint operator +(CPoint v)  { return(CPoint(_x + v._x, _y + v._y)); }
    CPoint operator -(CPoint v)  { return(CPoint(_x - v._x, _y - v._y)); }
    CPoint operator *(float s)   { return(CPoint(s * _x, s * _y)); }

public:
    float _x;
    float _y;
};

class CWidgetBase : public CView
{
public:
    CWidgetBase(
            const char *     strName,
            bwidget_engine_t engine,
            CWidgetBase *    pParent
            );

    virtual bool                 setFocus(void);
    virtual void                 show(bool bShow);
    virtual bwidget_t            getWidget(void)    { return(_widget); }
    virtual bwin_t               getWin(void)       { return(bwidget_win(getWidget())); }
    virtual bwin_engine_t        getWinEngine(void) { return(bwidget_win_engine(getEngine())); }
    virtual MRect                getGeometry(void)           = 0;
    virtual void                 setGeometry(MRect geometry) = 0;
    virtual eRet                 onKeyDown(bwidget_t widget, bwidget_key key);
    virtual void                 onClick(bwidget_t widget);
    virtual void                 onFocus(bwidget_t widget);
    virtual void                 onBlur(bwidget_t widget);
    virtual bool                 isVisible(void);
    virtual backgroundFillMode_t getBackgroundFillMode(void)                      { return(_backgroundFillMode); }
    virtual void                 setBackgroundFillMode(backgroundFillMode_t mode) { _backgroundFillMode = mode; }
    virtual uint32_t             getBackgroundColor(void)           = 0;
    virtual void                 setBackgroundColor(uint32_t color) = 0;
    virtual void                 getBackgroundGradient(
            uint32_t * pColorTop,
            uint32_t * pColorMiddle,
            uint32_t * pColorBottom
            ) { *pColorTop = _gradientTop; *pColorMiddle = _gradientMiddle; *pColorBottom = _gradientBottom; }
    virtual void setBackgroundGradient(
            uint32_t colorTop,
            uint32_t colorMiddle,
            uint32_t colorBottom
            ) { _gradientTop = colorTop; _gradientMiddle = colorMiddle; _gradientBottom = colorBottom; }
    virtual void getRoundedCorners(
            uint32_t * pCornerUpperLeft,
            uint32_t * pCornerUpperRight,
            uint32_t * pCornerLowerLeft,
            uint32_t * pCornerLowerRight
            ) { *pCornerUpperLeft = _cornerUpperLeft; *pCornerUpperRight = _cornerUpperRight; *pCornerLowerLeft = _cornerLowerLeft; *pCornerLowerRight = _cornerLowerRight; }
    virtual void setRoundedCorners(
            uint32_t cornerUpperLeft,
            uint32_t cornerUpperRight,
            uint32_t cornerLowerLeft,
            uint32_t cornerLowerRight
            ) { _cornerUpperLeft = cornerUpperLeft; _cornerUpperRight = cornerUpperRight; _cornerLowerLeft = cornerLowerLeft; _cornerLowerRight = cornerLowerRight; }
    virtual void drawBackground(bwidget_t widget, blabel_settings * pLabelSettings, const bwin_rect rect, const bwin_rect * pRectClip);
    virtual void drawBevel(bwidget_t widget, blabel_settings * pLabelSettings, const bwin_rect rect, const bwin_rect * pRectClip);
    virtual void drawFocus(bwidget_t widget, blabel_settings * pLabelSettings, const bwin_rect rect, const bwin_rect * pRectClip);
    virtual void drawText(const char * strText, bwin_font_t font, bwidget_t widget, blabel_settings * pLabelSettings, const bwin_rect rect, const bwin_rect * pRectClip);
    virtual void drawText(bwidget_t widget, blabel_settings * pLabelSettings, const bwin_rect rect, const bwin_rect * pRectClip);
    virtual void drawCornerPoints(bwidget_t widget, CPoint * points, uint32_t numPoints, const bwin_rect * pRectClip);

    bwidget_engine_t getEngine(void) { return(_engine); }
    void             setFocusable(bool bFocusable);
    int              getZOrder(void);
    void             setZOrder(int zorder);
    CWidgetBase *    getParent(void)                 { return(_pParent); }
    void             setParent(CWidgetBase * parent) { _pParent = parent; }  /* note: does not change bwidget parent! */
    bwin_t           getParentWin();
    void             setParentWin(bwin_t parentWin); /* note: does not change the CWidgetBase parent! */
    long             getValue(void)       { return(_value); }
    void             setValue(long value) { _value = value; }
    uint32_t         colorConvert(uint32_t color1, uint32_t color2, uint8_t percent);
    CPoint           getBezierPoint(CPoint * points, uint8_t numPoints, float t);
    void             generateCornerPoints(CPoint * pointsCurve, uint32_t numPointsCurve, CPoint * pointsBezier, uint32_t numPointsBezier);

protected:
    bwidget_engine_t     _engine;
    CWidgetBase *        _pParent;
    bwidget_t            _widget;
    unsigned long        _value;
    backgroundFillMode_t _backgroundFillMode;
    uint32_t             _gradientTop;
    uint32_t             _gradientMiddle;
    uint32_t             _gradientBottom;
    uint32_t             _cornerUpperLeft;
    uint32_t             _cornerUpperRight;
    uint32_t             _cornerLowerLeft;
    uint32_t             _cornerLowerRight;
};

#ifdef __cplusplus
}
#endif

#endif /* ATLAS_WIDGET_BASE_H__ */