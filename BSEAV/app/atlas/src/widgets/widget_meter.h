/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#ifndef ATLAS_WIDGET_METER_H__
#define ATLAS_WIDGET_METER_H__

#include "widget_label.h"

#ifdef __cplusplus
extern "C" {
#endif

class CWidgetMeter : public CWidgetLabel
{
public:
    CWidgetMeter(
            const char *     strName,
            bwidget_engine_t engine,
            CWidgetBase *    pParentWidget,
            MRect            geometry,
            bwin_font_t      font,
            bwin_t           parentWin = NULL
            ); /* optional */
    ~CWidgetMeter(void);

    bool                   isMeterVisible(void) { return(_showMeter); }
    void                   showMeter(bool show);
    int                    getLevel(void) { return(_level); }
    void                   setLevel(int level);
    uint32_t               getColor(void) { return(_color); }
    void                   setColor(uint32_t color);
    uint32_t               getColorTop(void) { return(_colorTop); }
    void                   setColorTop(uint32_t colorTop);
    uint32_t               getColorMiddle(void) { return(_colorMiddle); }
    void                   setColorMiddle(uint32_t colorMiddle);
    uint32_t               getColorBottom(void) { return(_colorBottom); }
    void                   setColorBottom(uint32_t colorBottom);
    eRet                   loadMeterImage(const char * filename, bwin_image_render_mode renderMode = bwin_image_render_mode_single);
    bwin_image_t           getMeterImage(void)               { return(_imageMeter); }
    bwin_image_render_mode getMeterRenderMode(void)          { return(_imageRenderMode); }
    void                   setRangeIndicator(bool showRange) { _showRange = showRange; }
    bool                   isRangeIndicatorVisible(void)     { return(_showRange); }
    void                   setRangeIndicatorSize(unsigned size);
    unsigned               getRangeIndicatorSize(void) { return(_rangeSize); }

protected:
    bool                   _showMeter;
    bwin_image_t           _imageMeter;
    bwin_image_render_mode _imageRenderMode;
    uint32_t               _color;
    uint32_t               _colorTop;
    uint32_t               _colorMiddle;
    uint32_t               _colorBottom;
    int                    _level;
    bool                   _showRange;
    unsigned               _rangeSize;
};

#ifdef __cplusplus
}
#endif

#endif /* ATLAS_WIDGET_METER_H__ */