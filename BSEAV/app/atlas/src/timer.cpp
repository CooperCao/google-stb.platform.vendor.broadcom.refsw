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

#include "atlas.h"
#include "atlas_os.h"
#include "timer.h"
#include "power.h"

#define CALLBACK_TIMER  "CallbackTimer"

BDBG_MODULE(atlas_timer);

/*
 *  Callback from scheduler when timer expires.  Triggering the bwidget
 *  main loop will ensure that the timer observer is not called until
 *  it can safely run.
 */
static void nexusTimerCallback(void * context)
{
    CTimer * pTimer = (CTimer *)context;

    BDBG_ASSERT(NULL != context);

    pTimer->clearTimerId();

    CWidgetEngine * pWidgetEngine = pTimer->getWidgetEngine();

    if (NULL != pWidgetEngine)
    {
        pWidgetEngine->syncCallback(pTimer, CALLBACK_TIMER);
    }
}

/*
 *  Callback from the bwidgets main loop - io trigger is complete and we can
 *  safely notify the observer of the timer expiration.
 */
static void bwinTimerCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CTimer * pTimer = (CTimer *)pObject;

    BDBG_ASSERT(NULL != pTimer);
    BSTD_UNUSED(strCallback);

    {
        CObserver * pObserver = pTimer->getObserver();
        CSubject *  pSubject  = pTimer->getSubject();

        if (NULL != pObserver)
        {
            pTimer->notifyObservers(eNotify_Timeout, pTimer);
        }

        if (NULL != pSubject)
        {
            pSubject->timerCallback(pTimer);
        }
    }
} /* bwinTimerCallback */

/*
 * Constructor
 * widgetEngine - the bwidget engine that will be synchronized with when the timer
 *                expires.
 * pObserver - the observer who will be notified when the timer expires.
 * timeout - the timeout period in millisecs.  This value can be overridden when the
 *           timer is started
 */
CTimer::CTimer(
        CWidgetEngine * pWidgetEngine,
        CObserver *     pObserver,
        int             timeout
        ) :
    CMvcModel("CTimer"),
    _pObserver(pObserver),
    _pSubject(NULL),
    _timerId(NULL),
    _timeout(timeout),
    _pWidgetEngine(pWidgetEngine)
{
    BDBG_ASSERT(_pWidgetEngine);
    BDBG_ASSERT(_pObserver);

    _pWidgetEngine->addCallback(this, CALLBACK_TIMER, bwinTimerCallback, ePowerMode_S3);
    registerObserver(_pObserver, eNotify_Timeout);
}

/* setWidgetEngine() must be called before starting a timer in this case. */
CTimer::CTimer(CObserver * pObserver) :
    CMvcModel("CTimer"),
    _pObserver(pObserver),
    _pSubject(NULL),
    _timerId(NULL),
    _timeout(0),
    _pWidgetEngine(NULL)
{
    BDBG_ASSERT(_pObserver);

    registerObserver(_pObserver, eNotify_Timeout);
}

/* add timer support for CSubject objects (who can't receive notifications).  in this case
 * we will use a direct callback mechanism which will be called within the bwin context so
 * it will already be synchronized with the rest of Atlas. Note: setWidgetEngine() must be
 * called before starting a timer in this case.
 */
CTimer::CTimer(CSubject * pSubject) :
    CMvcModel("CTimer"),
    _pObserver(NULL),
    _pSubject(pSubject),
    _timerId(NULL),
    _timeout(0),
    _pWidgetEngine(NULL)
{
    BDBG_ASSERT(_pSubject);
}

CTimer::~CTimer()
{
    stop();
    clearAllObservers();

    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_TIMER);
        _pWidgetEngine = NULL;
    }
}

void CTimer::setWidgetEngine(CWidgetEngine * pWidgetEngine)
{
    if (NULL != pWidgetEngine)
    {
        pWidgetEngine->addCallback(this, CALLBACK_TIMER, bwinTimerCallback, ePowerMode_S3);
    }
    else
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_TIMER);
    }

    _pWidgetEngine = pWidgetEngine;
}

/*
 * Starts the timer.
 *  timeout - optional parameter that if given will specify the timeout period in millisecs.
 */
void CTimer::start(int timeout)
{
    if (NULL != _timerId)
    {
        BDBG_MSG(("Restarting timer: %d ms", _timeout));
        stop();
    }

    if (0 < timeout)
    {
        _timeout = timeout;
    }

    _timerId = B_Scheduler_StartTimer(gScheduler, gLock, _timeout, nexusTimerCallback, this);
}

void CTimer::stop()
{
    if (NULL != _timerId)
    {
        B_Scheduler_CancelTimer(gScheduler, _timerId);
        _timerId = NULL;
    }
}