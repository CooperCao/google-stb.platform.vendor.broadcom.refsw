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

#ifndef __CPUTEST_H
#define __CPUTEST_H

#include "bwidgets.h"
#include "widget_engine.h"
#include "mvc.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * CCpuTest is a model class used to exercise the STB cpu by repeatedly
 * decompressing a jpeg.  It relies on the jpeg library and allows level
 * adjustments.
 */
class CCpuTest : public CMvcModel
{
public:
    CCpuTest(
            CWidgetEngine *  pWidgetEngine,
            CConfiguration * pCfg
            );
    ~CCpuTest(void);

    eRet start(uint32_t nLevel);
    eRet stop(void);

    CWidgetEngine * getWidgetEngine(void) { return(_pWidgetEngine); }
    void            setWidgetEngine(CWidgetEngine * pWidgetEngine);

    bool      getThreadRun(void)      { return(_bThreadRun); }
    void      setThreadRun(bool bRun) { _bThreadRun = bRun; }
    double    getCpuUtilization(void);
    uint32_t  getLevel(void)       { return(_nLevel); }
    bool      isStarted(void)      { return((0 == _threadList.total()) ? false : true); }
    uint32_t  getDelay(void)       { return(_nDelay); }
    uint8_t * getImageBuffer(void) { return(_imgBuffer); }
    int       getImageSize(void)   { return(_imgSize); }

protected:
    CWidgetEngine *        _pWidgetEngine;
    CConfiguration *       _pCfg;
    MList <B_ThreadHandle> _threadList;
    uint32_t               _nLevel;
    uint32_t               _nDelay;
    bool                   _bThreadRun;
    uint8_t *              _imgBuffer;
    int                    _imgSize;
};

#ifdef __cplusplus
}
#endif

#endif /* __CPUTEST_H */