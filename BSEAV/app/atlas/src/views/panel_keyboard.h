/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#ifndef ATLAS_PANEL_KEYBOARD_H__
#define ATLAS_PANEL_KEYBOARD_H__

#include "panel.h"
#include "timer.h"

class CWidgetMenu;
class CWidgetButton;
class CWidgetLabel;

#ifdef __cplusplus
extern "C" {
#endif

typedef enum eKeyMapping
{
    eKeyMapping_qwerty,
    eKeyMapping_qwertyShift,
    eKeyMapping_qwertySymbol,
    eKeyMapping_none
} eKeyMapping;

class CPanelKeyboard : public CPanel
{
public:
    CPanelKeyboard(
            CWidgetEngine * pWidgetEngine,
            CScreenMain *   pScreenMain,
            CWidgetBase *   pParentWidget,
            MRect           geometry,
            bwin_font_t     font,
            bwin_t          parentWin = NULL
            ); /* optional */
    ~CPanelKeyboard(void);

    virtual eRet initialize(CModel * pModel, CConfig * pConfig);
    virtual void uninitialize(void);
    virtual void onClick(bwidget_t widget);
    virtual void show(bool bShow);
    virtual void processNotification(CNotification & notification);
    virtual void layout(eKeyMapping keyMap = eKeyMapping_none);
    virtual void clear(void);

    MString showModal(void);
    void    cancelModal(void) { _done = true; }

    void setShift(bool bShift)   { _bShift = bShift; }
    void setSymbol(bool bSymbol) { _bSymbol = bSymbol; }
    void setTitle(const char * strTitle, const char * strEntryTitle);
    void addKey(const char ch);
    void delKey(void);

protected:
    bool                      _bShift;
    bool                      _bSymbol;
    CWidgetMenu *             _pKeyboardMenu;
    CWidgetLabel *            _pHeading;
    MString                   _strEntry;
    MString                   _strEntryTitle;
    uint32_t                  _numDigits;
    CWidgetLabel *            _pBackground;
    bool                      _done;
    CWidgetButton *           _Back;
    MAutoList <CWidgetButton> _keyList;
};

#ifdef __cplusplus
}
#endif

#endif /* ATLAS_PANEL_KEYBOARD_H__ */