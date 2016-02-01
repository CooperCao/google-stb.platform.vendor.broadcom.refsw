/***************************************************************************
 * (c) 2002-2015 Broadcom Corporation
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

#ifndef ATLAS_LUA_H__
#define ATLAS_LUA_H__

#include "view.h"
#include "bwidgets.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* make sure lua includes are inside the extern "C" */
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#ifdef __cplusplus
}
#endif

class CModel;

/* Lua events are created in response to custom Atlas Lua commands.  They are stored temporarily until
 * the Atlas main loop has a chance to service them.  Data associated with the Lua event is allocated
 * when the command is received, and freed after the associated action is completed.  CLuaEvent base
 * class allows us to reference all tuner events using the base class, while the template derived class
 * ensures that different _data types are properly deleted when no longer needed.
 */
class CLuaEvent
{
public:
    CLuaEvent(
            eNotification id,
            eNotification waitId = eNotify_Invalid,
            int           waitTimeout = 1000 /*B_WAIT_FOREVER*/
            ) :
        _id(id),
        _waitId(waitId),
        _waitTimeout(waitTimeout) {}
    virtual ~CLuaEvent() {}

    virtual void * getDataPtr(void) { return(NULL); }

    eNotification getId(void)               { return(_id); }
    eNotification getWaitNotification(void) { return(_waitId); }
    int           getWaitTimeout(void)      { return(_waitTimeout); }

protected:
    eNotification _id;
    eNotification _waitId;
    int           _waitTimeout;
};

template<class T>
class CLuaDataEvent : public CLuaEvent
{
public:
    CLuaDataEvent(
            eNotification id,
            T *           data,
            eNotification waitId = eNotify_Invalid,
            int           waitTimeout = 1000 /* B_WAIT_FOREVER */
            ) :
        CLuaEvent(id, waitId, waitTimeout),
        _data(data) {}

    virtual ~CLuaDataEvent(void) { if (NULL != _data) { delete _data; } }

    void * getDataPtr(void) { return((void *)_data); }

protected:
    T * _data;
};

class CLua : public CView
{
public:
    CLua(void);
    ~CLua(void);

    eRet             initialize(CConfig * pConfig);
    eRet             uninitialize(void);
    void             run(CWidgetEngine * pWidgetEngine);
    void             stop(void);
    CConfiguration * getCfg()                    { return(_pCfg); }
    CConfig *        getConfig()                 { return(_pConfig); }
    lua_State *      getLuaState(void)           { return(_pLua); }
    void             setStartState(bool started) { _shellStarted = started; }
    bool             getStartState(void)         { return(_shellStarted); }
    CWidgetEngine *  getWidgetEngine(void)       { return(_pWidgetEngine); }
    void             addEvent(CLuaEvent * pEvent);
    CLuaEvent *      getEvent(void);
    CLuaEvent *      removeEvent(void);
    void             trigger(CLuaEvent * pEvent);
    bool             handleInput(char * pLine);
    void             processNotification(CNotification & notification);
    void             setModel(CModel * pModel) { _pModel = pModel; }
    CModel *         getModel(void)            { return(_pModel); }

public:
    bool     _threadRun;
    uint16_t _pollInterval;

protected:
    lua_State *       _pLua;
    B_ThreadHandle    _threadShell;
    bool              _shellStarted;
    MList <CLuaEvent> _eventList;
    CLuaEvent         _busyLuaEvent;
    B_MutexHandle     _eventMutex;
    B_EventHandle     _busyEvent;
    CWidgetEngine *   _pWidgetEngine;
    CConfig *         _pConfig;
    CConfiguration *  _pCfg;
    CModel *          _pModel;
};

#endif /* ATLAS_LUA_H__ */