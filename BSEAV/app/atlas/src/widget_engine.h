/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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
 *****************************************************************************/

#ifndef WIDGET_ENGINE_H__
#define WIDGET_ENGINE_H__

#include "bwin.h"
#include "bwidgets.h"
#include "resource.h"
#include "nexus_types.h"
#include "power.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (* CWidgetEngineCallback)(void * pObject, const char * strCallback);

class CWidgetEngine;

class CCallback
{
public:
    CCallback(
            void *                pObject,
            const char *          strName,
            CWidgetEngineCallback callback,
            CWidgetEngine *       pEngine,
            ePowerMode            maxPowerState = ePowerMode_S0
            )
    {
        _pObject       = pObject;
        _strName       = strName;
        _callback      = callback;
        _pEngine       = pEngine;
        _maxPowerState = maxPowerState;
    }

    void dump(bool bForce = false);

public:
    void *                _pObject;
    MString               _strName;
    CWidgetEngineCallback _callback;
    CWidgetEngine *       _pEngine;
    ePowerMode            _maxPowerState;
};

/* CWidgetEngine wraps the bwidget io handle operations and provides a generic mechanism
 * for creating/deleting/triggering bwin callbacks.  This is primarily used in Atlas to
 * synchonize Nexus callbacks with the bwin main loop (Atlas context).  This class will
 * also serve as a central point of clean up in the case that a callback is added and
 * not removed.  CWidgetEngine will clean up for itself automatically when closed/deleted.
 */
class CWidgetEngine : public CMvcModel
{
public:
    CWidgetEngine(
            const char *     name,
            CConfiguration * pCfg
            );
    ~CWidgetEngine(void);

    eRet             open(bwin_engine_t bwinEngine, bwidget_engine_settings * pSettings);
    void             close(void);
    void             start(void);
    void             stop(void);
    eRet             addCallback(void * pObject, const char * strCallback, CWidgetEngineCallback callback, ePowerMode maxPowerLevel = ePowerMode_S0);
    void             removeCallback(void * pObject, const char * strCallback);
    eRet             syncCallback(void * pObject, const char * strCallback);
    bwidget_engine_t getWidgetEngine(void) { return(_widgetEngine); }
    void             dump(bool bForce = false);
    CConfiguration * getCfg(void) { return(_pCfg); }

protected:
    CConfiguration *      _pCfg;
    bwidget_engine_t      _widgetEngine;
    MAutoList <CCallback> _listCallback;
};

#ifdef __cplusplus
}
#endif

#endif /* WIDGET_ENGINE_H__ */