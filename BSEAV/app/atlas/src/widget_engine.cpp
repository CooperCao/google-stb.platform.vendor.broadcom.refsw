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

#include "widget_engine.h"

BDBG_MODULE(atlas_widget_engine);

static void bwinCallback(
        bwidget_engine_t engine,
        bwin_io_handle   handle
        )
{
    CCallback * pCallback = (CCallback *)handle;

    BSTD_UNUSED(engine);
    BDBG_ASSERT(NULL != pCallback);

    /* we are now synchonized with the bwin main loop so call the given callback */
    {
        CWidgetEngine * pWidgetEngine = pCallback->_pEngine;
        BDBG_ASSERT(NULL != pWidgetEngine);

        CConfiguration * pCfg = pWidgetEngine->getCfg();
        BDBG_ASSERT(NULL != pCfg);

        if (GET_INT(pCfg, POWER_STATE) <= pCallback->_maxPowerState)
        {
            /* callback valid during current power state */
            pCallback->dump();
            pCallback->_callback(pCallback->_pObject, pCallback->_strName);
        }
        else
        {
            /* callback not valid during current power state */
            BDBG_WRN(("Filtered bwin callback:%s", pCallback->_strName.s()));
            goto done;
        }
    }

done:
    return;
} /* bwinCallback */

void CCallback::dump(bool bForce)
{
    BDBG_Level level;

    if (true == bForce)
    {
        BDBG_GetModuleLevel("atlas_widget_engine", &level);
        BDBG_SetModuleLevel("atlas_widget_engine", BDBG_eMsg);
    }

    BDBG_MSG(("calling CCallback name:%s object:%p callback:%p", _strName.s(), _pObject, _callback));

    if (true == bForce)
    {
        BDBG_SetModuleLevel("atlas_widget_engine", level);
    }
} /* dump */

CWidgetEngine::CWidgetEngine(
        const char *     name,
        CConfiguration * pCfg
        ) :
    CMvcModel(name),
    _pCfg(pCfg),
    _widgetEngine(NULL)
{
}

CWidgetEngine::~CWidgetEngine()
{
    close();
}

eRet CWidgetEngine::open(
        bwin_engine_t             bwinEngine,
        bwidget_engine_settings * pSettings
        )
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NULL != bwinEngine);

    _widgetEngine = bwidget_engine_open(bwinEngine, pSettings);
    CHECK_PTR_ERROR_GOTO("Unable to open widget engine", _widgetEngine, ret, eRet_ExternalError, error);

error:
    return(ret);
}

void CWidgetEngine::close()
{
    CCallback * pCallback = NULL;

    /* remove any leftover callbacks in list */
    for (pCallback = _listCallback.first(); pCallback; pCallback = _listCallback.next())
    {
        /* found callback in list - remove it */
        BDBG_WRN(("removing leftover bwidget io callback:%s", pCallback->_strName.s()));
        _listCallback.remove(pCallback);
        DEL(pCallback);
    }

    if (NULL != _widgetEngine)
    {
        bwidget_engine_close(_widgetEngine);
        _widgetEngine = NULL;
    }
} /* close */

void CWidgetEngine::start()
{
    BDBG_ASSERT(NULL != _widgetEngine);

    bwidget_run(_widgetEngine);
}

void CWidgetEngine::stop()
{
    bwidget_terminate(_widgetEngine);
}

eRet CWidgetEngine::addCallback(
        void *                pObject,
        const char *          strCallback,
        CWidgetEngineCallback callback,
        ePowerMode            maxPowerLevel
        )
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NULL != pObject);
    BDBG_ASSERT(NULL != strCallback);
    BDBG_ASSERT(NULL != callback);

    /* check for existing object/callback name in list - remove it if found */
    removeCallback(pObject, strCallback);

    CCallback * pCallback = new CCallback(pObject, strCallback, callback, this, maxPowerLevel);
    CHECK_PTR_ERROR_GOTO("unable to allocate ccallback object", pCallback, ret, eRet_OutOfMemory, error);

    bwidget_add_io_handle(_widgetEngine, (bwin_io_handle)pCallback, bwinCallback);
    _listCallback.add(pCallback);

    /* sanity check */
    if (499 <= _listCallback.total())
    {
        /* we are using too many io handles.  either there is a problem, or we need to
         * increase the number allowed in bwin */
        ATLAS_ERROR("Using too many bwin io handles!", eRet_NotAvailable);
        dump(true);
    }

error:
    return(ret);
} /* addCallback */

void CWidgetEngine::removeCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CCallback * pCallback = NULL;

    BDBG_ASSERT(NULL != pObject);
    BDBG_ASSERT(NULL != strCallback);

    /* find given pObject/strCallback pair in CCallback list */
    for (pCallback = _listCallback.first(); pCallback; pCallback = _listCallback.next())
    {
        if ((pCallback->_pObject == pObject) &&
            (pCallback->_strName == strCallback))
        {
            /* found matching callback in list - remove it */
            bwidget_remove_io_handle(_widgetEngine, (bwin_io_handle)pCallback);
            _listCallback.remove(pCallback);
            DEL(pCallback);
        }
    }
} /* removeCallback */

/* returns eRet_NotAvailable if given callback was not previously added using
 * addCallback(). returns eRet_Ok otherwise.
 */
eRet CWidgetEngine::syncCallback(
        void *       pObject,
        const char * strCallback
        )
{
    eRet        ret       = eRet_NotAvailable; /* assume callback not found */
    CCallback * pCallback = NULL;

    MListItr <CCallback> itr(&_listCallback);

    BDBG_ASSERT(NULL != pObject);
    BDBG_ASSERT(NULL != strCallback);

    /* find given pObject/strCallback pair in CCallback list */
    for (pCallback = itr.first(); pCallback; pCallback = itr.next())
    {
        if ((pCallback->_pObject == pObject) &&
            (pCallback->_strName == strCallback))
        {
            /* found matching callback in list - trigger bwin synchronized callback */
            bwidget_trigger_io(_widgetEngine, (bwin_io_handle)pCallback);
            ret = eRet_Ok;
            break;
        }
    }

    return(ret);
} /* syncCallback */

void CWidgetEngine::dump(bool bForce)
{
    BDBG_Level level;

    if (true == bForce)
    {
        BDBG_GetModuleLevel("atlas_widget_engine", &level);
        BDBG_SetModuleLevel("atlas_widget_engine", BDBG_eMsg);
    }

    CCallback *          pCallback = NULL;
    int                  i         = 1;
    MListItr <CCallback> itr(&_listCallback);

    /* printout all added callbacks */
    for (pCallback = itr.first(); pCallback; pCallback = itr.next())
    {
        BDBG_MSG(("%d. CWidgetEngine callback name:%s object:%p callback function:%p",
                  i, pCallback->_strName.s(), pCallback->_pObject, pCallback->_callback));
        i++;
    }

    if (true == bForce)
    {
        BDBG_SetModuleLevel("atlas_widget_engine", level);
    }
} /* dump */