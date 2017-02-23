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

#ifndef VIDEO_DECODE_NX_H__
#define VIDEO_DECODE_NX_H__

#include "video_decode.h"
#include "video_window.h"
#include "nxclient.h"

#ifdef __cplusplus
extern "C" {
#endif

class CSimpleVideoDecodeNx : public CSimpleVideoDecode
{
public:
    CSimpleVideoDecodeNx(
            const char *     name,
            const uint16_t   number,
            CConfiguration * pCfg
            );
    ~CSimpleVideoDecodeNx();

    virtual eRet   open(CWidgetEngine * pWidgetEngine, CStc * pStc);
    virtual CStc * close(void);
    virtual eRet   start(CPid * pPid, CStc * pStc = NULL);
    virtual eRet   updateConnectSettings(NxClient_ConnectSettings * pSettings, int index = 0);

    NEXUS_SurfaceClientHandle getDesktopClientVideoWin(void) { return(_surfaceClientVideoWin); }
    eRet                      setPosition(MRect rect, uint16_t zorder);
    eRet                      setGeometryVideoWindow(MRect rect, uint8_t percent, eWinArea area, uint8_t border, uint16_t zorder);
    NEXUS_VideoFormat         getFormat(void);

protected:
    NEXUS_SurfaceClientHandle _surfaceClientVideoWin;
    NxClient_VideoWindowType  _videoWindowType;
};

#ifdef __cplusplus
}
#endif

#endif /* ifndef VIDEO_DECODE_NX_H__ */