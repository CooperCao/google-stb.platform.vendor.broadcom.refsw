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

#ifndef VIDEO_WINDOW_H__
#define  VIDEO_WINDOW_H__

#include "video_decode.h"
#include "nexus_video_window.h"
#include "model.h"

#include "mgeom.h"

class CDisplay;

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    eWinArea_UpperRight,
    eWinArea_UpperLeft,
    eWinArea_LowerLeft,
    eWinArea_LowerRight
} eWinArea;

class CVideoWindow
{
public:
    CVideoWindow(
            const uint16_t   number,
            CDisplay *       pDisplay,
            CConfiguration * pCfg
            );
    ~CVideoWindow(void);

    eRet                         open(void);
    void                         close(void);
    eRet                         addInput(CVideoDecode * pVideoDecode);
    uint16_t                     getNumber(void) { return(_number); }
    NEXUS_VideoWindowHandle      getWindow(void) { return(_videoWindow); }
    eRet                         setContentMode(NEXUS_VideoWindowContentMode contentMode);
    NEXUS_VideoWindowContentMode getContentMode(void);
    eRet                         setDeinterlacer(bool bDeinterlacer);
    eRet                         setBoxDetect(bool bBoxDetect);
    eRet                         setPosition(MRect rect, uint16_t zorder = 0);
    void                         getPosition(MRect * pRect, uint16_t * pZorder);
    eRet                         setZorder(uint16_t zorder);
    eRet                         setVisible(bool bVisible = true);
    bool                         isVisible(void);
    eRet                         setGeometry(MRect rect, uint8_t percent = 100, eWinArea area = eWinArea_UpperLeft, uint8_t border = 0, uint16_t zorder = 0);
    void                         setType(eWindowType type)      { _type = type; }
    eWindowType                  getType(void)                  { return(_type); }
    bool                         isCheckedOut()                 { return(_checkedOut); }
    void                         setCheckedOut(bool checkedOut) { _checkedOut = checkedOut; }

protected:
    uint16_t                _number;
    NEXUS_VideoWindowHandle _videoWindow;
    CDisplay *              _pDisplay;
    CConfiguration *        _pCfg;
    bool                    _checkedOut;
    MList <CVideoDecode>    _videoDecodeList;
    eWindowType             _type;

    /* DTT these can probably be replaced with NEXUS_VideoWindowSettings variable?! */
    MRect    _position;
    uint16_t _zOrder;
    bool     _deinterlacer;
    bool     _playbackDeinterlacer;
    uint16_t _sharpness;
};

#ifdef __cplusplus
}
#endif

#endif /* VIDEO_WINDOW_H__ */