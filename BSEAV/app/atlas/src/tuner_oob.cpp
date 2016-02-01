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

#if NEXUS_HAS_FRONTEND

#define CALLBACK_TUNER_LOCK_STATUS_OOB  "CallbackTunerLockStatusOob"

#include "channel_oob.h"
#include "tuner_oob.h"
#include "ts_psi.h"
#include "ts_pat.h"

#include "nexus_frontend.h"
#include "nexus_frontend_oob.h"

#ifdef SNMP_SUPPORT
#include <sys/times.h>
#endif

BDBG_MODULE(atlas_tuner_oob);

static void nexusTunerLockCallback(
        void * context,
        int    param
        )
{
    CTuner * pTuner = (CTuner *)context;

    BSTD_UNUSED(param);
    BDBG_ASSERT(NULL != pTuner);

    CWidgetEngine * pWidgetEngine = pTuner->getWidgetEngine();

    /* sync with bwin loop */
    if (NULL != pWidgetEngine)
    {
        pWidgetEngine->syncCallback(pTuner, CALLBACK_TUNER_LOCK_STATUS_OOB);
    }

    /* set lock event for synchronous tune calls waiting for lock */
    B_Event_Set(pTuner->getLockEvent());
} /* nexusTunerLockCallback */

static void bwinTunerLockStatusCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CTunerOob * pTuner = (CTunerOob *)pObject;

    BDBG_ASSERT(NULL != pTuner);
    BSTD_UNUSED(strCallback);

    pTuner->handleLockCallback();
}

void CTunerOob::handleLockCallback()
{
    BDBG_MSG(("CTunerOob BWIN tuner lock callback:%d", isLocked()));
    setLockState(NEXUS_FrontendLockStatus_eLocked == isLocked() ? true : false);

    notifyObservers(eNotify_TunerLockStatus, this);

    if (NULL != _tunerLockCallback)
    {
        /* disabled for now...
         * _tunerLockCallback(_tunerLockCallbackContext, _tunerLockCallbackData);
         */
    }
}

void CTunerOobScanData::dump()
{
    MListItr <uint32_t> itr(&_freqList);
    uint32_t *          pFreq = NULL;

    BDBG_WRN(("tuner type:%d", getTunerType()));
    BDBG_WRN(("List of OOB Scan Frequency Requests:"));
    for (pFreq = (uint32_t *)itr.first(); pFreq; pFreq = (uint32_t *)itr.next())
    {
        BDBG_WRN(("freq:%u", *pFreq));
    }

    BDBG_WRN(("append to channel list:%d", _appendToChannelList));
}

CTunerOob::CTunerOob(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    CTuner(name, number, eBoardResource_frontendOob, pCfg)
{
    _scanThread_name = "CTunerOob_Scan";
}

eRet CTunerOob::tune(NEXUS_FrontendOutOfBandSettings settings)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    settings.lockCallback.callback = nexusTunerLockCallback;
    settings.lockCallback.context  = this;

    NEXUS_StartCallbacks(_frontend);
    B_Event_Reset(_lockEvent);
    B_Event_Reset(_waitEvent);

    nerror = NEXUS_Frontend_TuneOutOfBand(_frontend, &settings);
    CHECK_NEXUS_ERROR_GOTO("unable to tune OOB frontend", ret, nerror, error);

#ifdef SNMP_SUPPORT
    setState(lock_tuning);
    setLastFrequency(settings.frequency);
#endif

error:
    if (NEXUS_NOT_SUPPORTED == nerror)
    {
        ret = eRet_NotSupported;
    }
    return(ret);
} /* tune */

eRet CTunerOob::open(CWidgetEngine * pWidgetEngine)
{
    eRet ret = eRet_Ok;

    /* call base class method */
    ret = CTuner::open(pWidgetEngine);
    CHECK_ERROR_GOTO("unable to open tuner", ret, error);

    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->addCallback(this, CALLBACK_TUNER_LOCK_STATUS_OOB, bwinTunerLockStatusCallback);
    }
    else
    {
        BDBG_MSG(("Opening CTunerOob num:%d without providing widgetEngine - tuner callbacks will be ignored", getNumber()));
    }

error:
    return(ret);
} /* open */

void CTunerOob::close()
{
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_TUNER_LOCK_STATUS_OOB);
    }

    /* call base class method */
    CTuner::close();
}

void CTunerOob::unTune(bool bFullUnTune)
{
    NEXUS_StopCallbacks(_frontend);
    if (true == bFullUnTune)
    {
        NEXUS_Frontend_Untune(_frontend);
    }

#ifdef SNMP_SUPPORT
    setState(lock_idle);
#endif
}

void CTunerOob::getProperties(
        MStringHash * pProperties,
        bool          bClear
        )
{
    BSTD_UNUSED(pProperties);
    BSTD_UNUSED(bClear);

#ifdef SNMP_SUPPORT
    {
        NEXUS_FrontendOutOfBandStatus nStatus_oob;
        NEXUS_FrontendCapabilities    caps;
        NEXUS_Error                   nerror = NEXUS_SUCCESS;
        eRet ret                             = eRet_Ok;

        unsigned int diff;
        clock_t      curr_time;
        double       elapsed_symbol;
        unsigned     symbolrate;
        struct tms   _tms;

        BDBG_ASSERT(NULL != _frontend);

        NEXUS_Frontend_GetCapabilities(_frontend, &caps);

        nerror = NEXUS_Frontend_GetOutOfBandStatus(_frontend, &nStatus_oob);
        CHECK_NEXUS_ERROR_GOTO("unable to get oob frontend status properties", ret, nerror, error);

        curr_time = times(&_tms);

        if (caps.outOfBand)
        {
            BKNI_Memcpy(&_tuner_status.status.oob, &nStatus_oob, sizeof(NEXUS_FrontendOutOfBandStatus));
        }

        if (_tuner_status.state == lock_locked)
        {
            if (caps.outOfBand)
            {
                _tuner_status.uncorrected += nStatus_oob.uncorrectedCount;
                _tuner_status.corrected   += nStatus_oob.correctedCount;
            }

            if (curr_time < _tuner_status.locked_time)
            {
                diff = (0xffffffff - _tuner_status.locked_time) + curr_time;
            }
            else
            {
                diff = curr_time - _tuner_status.locked_time;
            }

            if (diff >= 100)
            {
                /* TODO:: handle wrap around case after long long time */
                _tuner_status.elapsed_time = diff/100;
                symbolrate                 = nStatus_oob.symbolRate;
                elapsed_symbol             = (double)(symbolrate/100.00)*(double)diff;
                _tuner_status.total_symbol = (unsigned)elapsed_symbol;
                if (caps.outOfBand) /* OOB is always QPSK */
                {
                    _tuner_status.total_byte = _tuner_status.total_symbol/4;
                }
                _tuner_status.unerrored = _tuner_status.total_symbol - _tuner_status.corrected - _tuner_status.uncorrected;
            }
        }
        else
        {
            _tuner_status.uncorrected = _tuner_status.unerrored = _tuner_status.corrected = _tuner_status.elapsed_time = 0;
        }

error:
        return;
    }
#endif /* ifdef SNMP_SUPPORT */
} /* getProperties */

NEXUS_FrontendLockStatus CTunerOob::isLocked()
{
    eRet                     ret    = eRet_Ok;
    NEXUS_Error              nerror = NEXUS_SUCCESS;
    NEXUS_FrontendFastStatus fastStatus;
    NEXUS_FrontendLockStatus lockStatus = NEXUS_FrontendLockStatus_eUnknown;

    nerror = NEXUS_Frontend_GetFastStatus(getFrontend(), &fastStatus);
    CHECK_NEXUS_WARN_GOTO("Error retrieving fast status", ret, nerror, error);

    BDBG_MSG(("fastStatus.lockStatus = %d, fastStatus.acquireInProgress = %d",
              fastStatus.lockStatus, fastStatus.acquireInProgress));

    lockStatus         = fastStatus.lockStatus;
    _acquireInProgress = fastStatus.acquireInProgress;

error:
    return(lockStatus);
} /* isLocked */

NEXUS_FrontendOutOfBandMode CTunerOob::getDefaultMode()
{
    if (true == _capabilities.outOfBandModes[NEXUS_FrontendOutOfBandMode_ePod_Dvs178Qpsk])
    {
        return(NEXUS_FrontendOutOfBandMode_ePod_Dvs178Qpsk);
    }
    else
    {
        return(NEXUS_FrontendOutOfBandMode_ePod_Dvs167Qpsk);
    }
}

void CTunerOob::saveScanData(CTunerScanData * pScanData)
{
    _scanData = (*((CTunerOobScanData *)pScanData));
}

void CTunerOob::doScan()
{
    BDBG_WRN(("CTunerOob::doScan not supported!"));
}

#endif /* NEXUS_HAS_FRONTEND */