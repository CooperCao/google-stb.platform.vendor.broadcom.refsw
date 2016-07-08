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

#if NEXUS_HAS_FRONTEND

#include "tuner.h"
#include "ts_psi.h"
#include "ts_pat.h"
#ifdef MPOD_SUPPORT
#include "tspsimgr.h"
#else
#include "tspsimgr2.h"
#endif
#include "bwidgets.h"
#include "channel.h"
#ifdef SNMP_SUPPORT
#include <sys/times.h>
#endif

#define CALLBACK_TUNER_SCAN  "CallbackTunerScan"

BDBG_MODULE(atlas_tuner);

/* bwin io callback that is executed when tuner io is triggered */
static void bwinTunerScanCallback(
        void *       pObject,
        const char * strCallback
        )
{
    eRet          ret    = eRet_Ok;
    CTuner *      pTuner = (CTuner *)pObject;
    CTunerEvent * pEvent = NULL;

    BSTD_UNUSED(strCallback);
    BDBG_ASSERT(NULL != pTuner);

    BDBG_MSG(("in bwinTunerScanCallback()"));

    /* handle all queued Tuner events */
    while (NULL != (pEvent = pTuner->getEvent()))
    {
        BDBG_MSG(("Notify Observers for Tuner event code: %#x", pEvent->getId()));

        /* explicitly call CSubject::notifyObservers() since the CTuner::notifyObserversAsync()
         * only queues the request.  this is done since scan is accomplished in a separate
         * thread and must queue events until synchronizing with bwidgets/bwin here */
        ret = pTuner->CSubject::notifyObservers(pEvent->getId(), (void *)pEvent->getDataPtr());
        CHECK_ERROR_GOTO("error notifying observers", ret, error);

        /* delete Tuner event and any associated data */
        pTuner->removeEvent();
        delete pEvent;
    }

error:
    return;
} /* bwinTunerScanCallback */

#if NEXUS_HAS_FRONTEND
static void tunerLockHandler(void * context)
{
    CTuner *                 pTuner    = (CTuner *)context;
    NEXUS_FrontendLockStatus lockState = NEXUS_FrontendLockStatus_eUnknown;

    lockState = pTuner->isLocked();
    BDBG_MSG(("Tuner %s lock handler lockState:%d", pTuner->getName(), lockState));

    pTuner->setLockState(NEXUS_FrontendLockStatus_eLocked == lockState);

    if (NEXUS_FrontendLockStatus_eLocked == lockState)
    {
        /* set wait event if locked */
        B_Event_Set(pTuner->getWaitEvent());
    }
    else
    if ((false == pTuner->isAcquireInProgress()) && (NEXUS_FrontendLockStatus_eUnlocked != lockState))
    {
        /* set wait event if tuner is done searching for lock */
        B_Event_Set(pTuner->getWaitEvent());
    }

#ifdef SNMP_SUPPORT
    if (lockState == NEXUS_FrontendLockStatus_eLocked)
    {
        struct tms _tms;
        pTuner->setLockedTime(times(&_tms));
        pTuner->setState(lock_locked);
        pTuner->incAcquireCount();
    }
    else
    {
        if (lockState == NEXUS_FrontendLockStatus_eUnlocked)
        {
            pTuner->setState(lock_tuning);
            pTuner->incUnlockedCount();
        }
        else
        {
            pTuner->setState(lock_failed);
            pTuner->setFailureFrequency(pTuner->getLastFrequency());
            pTuner->incFailureCount();
        }
    }
#endif /* ifdef SNMP_SUPPORT */
} /* tunerLockHandler */

#endif /* if NEXUS_HAS_FRONTEND */

CTuner::CTuner(
        const char *     name,
        const uint16_t   number,
        eBoardResource   resource,
        CConfiguration * pCfg
        ) :
    CResource(name, number, resource, pCfg),
    _locked(false),
    _acquireInProgress(false),
    _eventMutex(NULL),
    _pWidgetEngine(NULL),
    _waitEvent(NULL),
    _statusEvent(NULL),
    _lockEvent(NULL),
    _lockEventId(0),
    _scanThread_handle(NULL),
    _scanThread_id(NULL),
    _scanThread_callback(NULL),
    _scanThread_context(NULL),
    _tunerLockCallback(NULL),
    _tunerLockCallbackContext(NULL),
    _tunerLockCallbackData(0)
{
    NEXUS_FrontendHandle hFrontend       = NULL;
    CPlatform *          pPlatformConfig = pCfg->getPlatformConfig();
    eRet                 ret             = eRet_Ok;

    BDBG_ASSERT(NEXUS_MAX_FRONTENDS > _number);
    BKNI_Memset(&_capabilities, 0, sizeof(NEXUS_FrontendCapabilities));

#ifdef SNMP_SUPPORT
    BKNI_Memset(&_tuner_status, 0, sizeof(tuner_status));
#endif

    hFrontend = pPlatformConfig->getFrontend(_number);
    if (NULL != hFrontend)
    {
        /* save tuner capabilities - retrieve from nexus */
        NEXUS_Frontend_GetCapabilities(hFrontend, &_capabilities);
        _frontend = hFrontend;
    }

    _statusEvent = B_Event_Create(NULL);
    CHECK_PTR_ERROR_GOTO("tuner async status event create failed", _statusEvent, ret, eRet_ExternalError, error);

    _lockEvent = B_Event_Create(NULL);
    CHECK_PTR_ERROR_GOTO("tuner lock event create failed", _lockEvent, ret, eRet_ExternalError, error);

    _waitEvent = B_Event_Create(NULL);
    CHECK_PTR_ERROR_GOTO("tuner wait event create failed!", _waitEvent, ret, eRet_ExternalError, error);

    _lockEventId = B_Scheduler_RegisterEvent(gScheduler, gLock, _lockEvent, tunerLockHandler, this);
    CHECK_PTR_ERROR_GOTO("tuner lock event registration failed!", _lockEventId, ret, eRet_ExternalError, error);

    _eventMutex = B_Mutex_Create(NULL);
    CHECK_PTR_ERROR_GOTO("unable to create mutex", _eventMutex, ret, eRet_ExternalError, error);

error:
    BDBG_ASSERT(eRet_Ok == ret);
}

CTuner::~CTuner()
{
    if (NULL != _scanThread_handle)
    {
        destroyThreadScan();
    }

    if (NULL != _eventMutex)
    {
        B_Mutex_Destroy(_eventMutex);
        _eventMutex = NULL;
    }

    if (NULL != _lockEventId)
    {
        B_Scheduler_UnregisterEvent(gScheduler, _lockEventId);
        _lockEventId = NULL;
    }

    if (NULL != _waitEvent)
    {
        B_Event_Destroy(_waitEvent);
        _waitEvent = NULL;
    }

    if (NULL != _lockEvent)
    {
        B_Event_Destroy(_lockEvent);
        _lockEvent = NULL;
    }

    if (NULL != _statusEvent)
    {
        B_Event_Destroy(_statusEvent);
        _statusEvent = NULL;
    }
}

eRet CTuner::open(CWidgetEngine * pWidgetEngine)
{
    eRet ret = eRet_Ok;

    _pWidgetEngine = pWidgetEngine;

    return(ret);
}

void CTuner::close()
{
    _pWidgetEngine = NULL;
}

eRet CTuner::acquire()
{
    return(eRet_Ok);
}

void CTuner::release()
{
}

static void scanThread(void * pParam)
{
    CTuner * pTuner = (CTuner *)pParam;

    BDBG_ASSERT(NULL != pParam);

    pTuner->doScan();
}

void CTuner::destroyThreadScan()
{
    if (NULL != _scanThread_handle)
    {
        B_Thread_Destroy(_scanThread_handle);
        _scanThread_handle = NULL;
    }
}

/* Scans the given list of frequencies for valid QAM channels.  Newly found channels are returned
 * using the given callback.  The caller is responsible for proper deletion of the found
 * channel objects when appropriate.
 */
eRet CTuner::scan(
        void *             id,
        CTunerScanData *   pScanData,
        CTunerScanCallback callback,
        void *             context
        )
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NULL != id);
    BDBG_ASSERT(NULL != pScanData);
    BDBG_ASSERT(NULL != callback);
    BDBG_ASSERT(NULL != _pWidgetEngine);
    BDBG_ASSERT(NULL != _pConfig); /* tuner must have access to model config prior to scan */

    /* save scan data thread will need */
    _scanThread_id       = id;
    _scanThread_callback = callback;
    _scanThread_context  = context;
    saveScanData(pScanData);

    if (NULL != _scanThread_handle)
    {
        destroyThreadScan();
    }

    _scanThread_handle = B_Thread_Create(_scanThread_name, scanThread, this, NULL);
    CHECK_PTR_ERROR_GOTO("thread creation failed!", _scanThread_handle, ret, eRet_ExternalError, error);

    _pWidgetEngine->addCallback(this, CALLBACK_TUNER_SCAN, bwinTunerScanCallback);
error:
    return(ret);
} /* scan */

void CTuner::scanDone(CTunerScanNotificationData * pNotifyData)
{
    BDBG_ASSERT(NULL != _pWidgetEngine);

    /* scan thread is done, but all notifications might not have been
     * sent yet (might still be waiting for bwin main loop
     * synchronization).  so we'll wait here until all queued
     * notifications have been sent. should be fast as there typically
     * are only 1 or 2 waiting. */
    flushEvents();

    /* notify scan is stopped and wait until complete */
    notifyObserversAsync(eNotify_ScanStopped, pNotifyData);
    flushEvents();

    _pWidgetEngine->removeCallback(this, CALLBACK_TUNER_SCAN);
}

void CTuner::dump()
{
    BDBG_MSG(("<%d>%s:%d %s%s%s%s%s%s",
              _type,
              _name.s(),
              _number,
              _capabilities.analog ? "(analog)" : "",
              _capabilities.qam ? "(qam)" : "",
              _capabilities.ofdm ? "(ofdm)" : "",
              _capabilities.satellite ? "(sds)" : "",
              _capabilities.outOfBand ? "(oob)" : "",
              _capabilities.vsb ? "(vsb)" : ""));
}

void CTuner::trigger(CTunerEvent * pTunerEvent)
{
    CWidgetEngine * pWidgetEngine = getWidgetEngine();

    BDBG_ASSERT(NULL != pTunerEvent);
    BDBG_ASSERT(NULL != pWidgetEngine);

    if (NULL != pWidgetEngine)
    {
        pWidgetEngine->syncCallback(this, CALLBACK_TUNER_SCAN);
    }
}

/* since scan runs in a separate thread, we will use this method to
 * synchronize with the bwin main loop when notifying observers from
 * that thread.  We will queue the notification and any associated
 * data which will eventually be sent after synchronizing with bwin.
 * This allows scan to send notifications from it's own thread, while
 * maintaining synchonization with the bwin main loop for all
 * notifications (important since they often result in drawing to
 * the screen) */
eRet CTuner::notifyObserversAsync(
        eNotification                id,
        CTunerScanNotificationData * data
        )
{
    CTunerScanNotificationData * dataClone = NULL;
    eRet ret                               = eRet_Ok;

    if (NULL != data)
    {
        /* make copy of given data - since we are queueing scan notifications, we will need to
         * save off the given data */
        dataClone = new CTunerScanNotificationData();
        CHECK_PTR_ERROR_GOTO("unable to copy tuner scan notification data", dataClone, ret, eRet_OutOfMemory, error);
        *dataClone = *data; /* object copy */
    }

    /*
     * create tuner event and give it playback info
     * note that we are not setting a wait notification so we will not wait for this command to complete
     */
    {
        CTunerDataEvent <CTunerScanNotificationData> * pTunerEvent =
            new CTunerDataEvent <CTunerScanNotificationData>(id, dataClone);
        CHECK_PTR_ERROR_GOTO("Unable to malloc CTunerDataEvent", pTunerEvent, ret, eRet_OutOfMemory, error);

        /* save Tuner event to queue - this event will be serviced when we get the bwin io callback:
         * bwinTunerScanCallback() */
        addEvent(pTunerEvent);

        /* trigger bwin io event here */
        trigger(pTunerEvent);
    }

error:
    return(ret);
} /* notifyObserversAsync */

void CTuner::addEvent(CTunerEvent * pEvent)
{
    CScopedMutex scopedMutex(_eventMutex);

    _eventList.add(pEvent);
}

CTunerEvent * CTuner::getEvent()
{
    CTunerEvent * pTunerEvent = NULL;
    CScopedMutex  scopedMutex(_eventMutex);

    if (0 < _eventList.total())
    {
        pTunerEvent = (CTunerEvent *)_eventList.first();
    }

    return(pTunerEvent);
}

void CTuner::flushEvents()
{
    /* wait until all events have been serviced */
    while (NULL != getEvent())
    {
        B_Thread_Sleep(100);
    }
}

CTunerEvent * CTuner::removeEvent()
{
    CTunerEvent * pTunerEvent = NULL;
    CScopedMutex  scopedMutex(_eventMutex);

    if (0 < _eventList.total())
    {
        pTunerEvent = (CTunerEvent *)_eventList.first();
        _eventList.remove();
    }

    return(pTunerEvent);
}

void CTuner::setLockState(bool locked)
{
    BDBG_MSG(("tuner locked:%d", locked));
    _locked = locked;
}

#define MAX_FRONTEND_ID  64
MString CTuner::getFrontendId()
{
    NEXUS_FrontendType frontendType;
    char               strId[MAX_FRONTEND_ID] = "";

    NEXUS_Frontend_GetType(_frontend, &frontendType);
    snprintf(strId, (MAX_FRONTEND_ID - 1), "%x v%u.%u", frontendType.chip.id, frontendType.chip.version.major, frontendType.chip.version.minor);

    return(MString(strId));
}

/* Get Soft Decisions to create Constellation */
eRet CTuner::getSoftDecisions(
        NEXUS_FrontendSoftDecision * pSoftDecisions,
        int                          length
        )
{
    eRet        ret = eRet_Ok;
    NEXUS_Error rc;

    if (NULL == pSoftDecisions)
    {
        ret = eRet_InvalidParameter;
        goto error;
    }

    BDBG_MSG(("Reading %d soft decisions", length));

    rc = NEXUS_Frontend_GetSoftDecisions(getFrontend(), pSoftDecisions, length);
    if (rc != NEXUS_SUCCESS)
    {
        ret = eRet_ExternalError;
    }

error:
    return(ret);
} /* getSoftDecisions */

#endif /* NEXUS_HAS_FRONTEND */