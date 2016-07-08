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

#define CALLBACK_TUNER_LOCK_STATUS_VSB  "CallbackTunerLockStatusVsb"

#include "channel_vsb.h"
#include "tuner_vsb.h"
#include "ts_psi.h"
#include "ts_pat.h"

#include "nexus_frontend.h"
#include "nexus_frontend_vsb.h"

BDBG_MODULE(atlas_tuner_vsb);

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
        pWidgetEngine->syncCallback(pTuner, CALLBACK_TUNER_LOCK_STATUS_VSB);
    }

    /* set lock event for synchronous tune calls waiting for lock */
    B_Event_Set(pTuner->getLockEvent());
} /* nexusTunerLockCallback */

static void bwinTunerLockStatusCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CTunerVsb * pTuner = (CTunerVsb *)pObject;

    BDBG_ASSERT(NULL != pTuner);
    BSTD_UNUSED(strCallback);

    pTuner->handleLockCallback();
}

void CTunerVsb::handleLockCallback()
{
    BDBG_MSG(("CTunerVsb BWIN tuner lock callback:%d", isLocked()));
    setLockState(NEXUS_FrontendLockStatus_eLocked == isLocked() ? true : false);

    notifyObservers(eNotify_TunerLockStatus, this);

    if (NULL != _tunerLockCallback)
    {
        /* disabled for now...
         * _tunerLockCallback(_tunerLockCallbackContext, _tunerLockCallbackData);
         */
    }
}

void CTunerVsbScanData::dump()
{
    MListItr <uint32_t> itr(&_freqList);
    uint32_t *          pFreq = NULL;

    BDBG_WRN(("tuner type:%d", getTunerType()));
    BDBG_WRN(("List of VSB Scan Frequency Requests:"));
    for (pFreq = (uint32_t *)itr.first(); pFreq; pFreq = (uint32_t *)itr.next())
    {
        BDBG_WRN(("freq:%u", *pFreq));
    }

    BDBG_WRN(("append to channel list:%d", _appendToChannelList));
}

CTunerVsb::CTunerVsb(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    CTuner(name, number, eBoardResource_frontendVsb, pCfg)
{
    _scanThread_name = "CTunerVsb_Scan";
}

eRet CTunerVsb::tune(NEXUS_FrontendVsbSettings settings)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    settings.lockCallback.callback = nexusTunerLockCallback;
    settings.lockCallback.context  = this;

    NEXUS_StartCallbacks(_frontend);
    B_Event_Reset(_lockEvent);
    B_Event_Reset(_waitEvent);

    nerror = NEXUS_Frontend_TuneVsb(_frontend, &settings);
    CHECK_NEXUS_ERROR_GOTO("unable to tune Vsb frontend", ret, nerror, error);

error:
    return(ret);
} /* tune */

eRet CTunerVsb::open(CWidgetEngine * pWidgetEngine)
{
    eRet ret = eRet_Ok;

    /* call base class method */
    ret = CTuner::open(pWidgetEngine);
    CHECK_ERROR_GOTO("unable to open tuner", ret, error);

    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->addCallback(this, CALLBACK_TUNER_LOCK_STATUS_VSB, bwinTunerLockStatusCallback);
    }
    else
    {
        BDBG_MSG(("Opening CTunerVsb num:%d without providing widgetEngine - tuner callbacks will be ignored", getNumber()));
    }

error:
    return(ret);
} /* open */

void CTunerVsb::close()
{
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_TUNER_LOCK_STATUS_VSB);
    }

    /* call base class method */
    CTuner::close();
}

void CTunerVsb::unTune(bool bFullUnTune)
{
    NEXUS_StopCallbacks(_frontend);
    if (true == bFullUnTune)
    {
        NEXUS_Frontend_Untune(_frontend);
    }
}

void CTunerVsb::getProperties(
        MStringHash * pProperties,
        bool          bClear
        )
{
    NEXUS_FrontendVsbStatus nStatus;
    NEXUS_Error             nerror = NEXUS_SUCCESS;
    eRet                    ret    = eRet_Ok;

    BDBG_ASSERT(NULL != _frontend);

    if (true == bClear)
    {
        pProperties->clear();
    }

    {
        char str[32];

        nerror = NEXUS_Frontend_GetVsbStatus(_frontend, &nStatus);
        CHECK_NEXUS_ERROR_GOTO("unable to get qam frontend status properties", ret, nerror, error);

        snprintf(str, 32, "%0.2f MHz", nStatus.settings.frequency / 1000000.0);
        pProperties->add("Frequency", str);
        snprintf(str, 32, "%c", nStatus.fecLock ? 'Y' : 'N');
        pProperties->add("FEC Lock", str);
        snprintf(str, 32, "%c", nStatus.receiverLock ? 'Y' : 'N');
        pProperties->add("QAM Lock", str);
        snprintf(str, 32, "%c", nStatus.opllLock ? 'Y' : 'N');
        pProperties->add("Output PLL Lock", str);
        snprintf(str, 32, "%f", nStatus.symbolRate / 1000000.0);
        pProperties->add("Symbol Rate", str);
        snprintf(str, 32, "%0.2f dB", nStatus.snrEstimate / 100.0);
        pProperties->add("SNR", str);
        snprintf(str, 32, "%0.1f%%", nStatus.ifAgcLevel/10.0);
        pProperties->add("IF AGC Level", str);
        snprintf(str, 32, "%0.1f%%", nStatus.rfAgcLevel/10.0);
        pProperties->add("RF AGC Level", str);
        snprintf(str, 32, "%0.2f KHz", nStatus.ifFrequencyError / 1000.0);
        pProperties->add("IF Freq Error", str);
        snprintf(str, 32, "%d Hz", nStatus.symbolRateError);
        pProperties->add("Symbol Rate Error", str);
        snprintf(str, 32, "%d", nStatus.fecCorrected);
        pProperties->add("Block Corrected", str);
        snprintf(str, 32, "%d", nStatus.fecUncorrected);
        pProperties->add("Block Uncorrected", str);
    }

error:
    return;
} /* getProperties */

NEXUS_FrontendLockStatus CTunerVsb::isLocked()
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

void CTunerVsb::saveScanData(CTunerScanData * pScanData)
{
    _scanData = *((CTunerVsbScanData *)pScanData);
}

void CTunerVsb::doScan()
{
    eRet                       ret            = eRet_Ok;
    uint16_t                   major          = 1;
    uint16_t                   numFreqToScan  = 0;
    uint16_t                   numFreqScanned = 0;
    CChannelVsb                chVsb(_pCfg, this); /* temp channel we'll use to do tuning during scan */
    CTunerScanNotificationData notifyData(this);   /* notification data for reporting scan start/stop/progress */
    uint32_t *                 pFreq = NULL;

    MListItr <uint32_t> freqListItr(&(_scanData._freqList));

    BDBG_ASSERT(NULL != _scanThread_handle);

    numFreqToScan = _scanData._freqList.total();
    if (0 == numFreqToScan)
    {
        /* nothing to scan so we are done */
        goto error;
    }

    /* check to see if fast status is supported */
    {
        NEXUS_FrontendFastStatus fastStatus;
        NEXUS_Error              nerror = NEXUS_SUCCESS;

        nerror = NEXUS_Frontend_GetFastStatus(getFrontend(), &fastStatus);
        if (NEXUS_NOT_SUPPORTED != nerror)
        {
            nerror = NEXUS_SUCCESS;
        }
        CHECK_NEXUS_ERROR_GOTO("fast status is not supported - aborting scan", ret, nerror, error);
    }

    /* set the starting major channel number for newly found channels */
    major = _scanData._majorStartNum;

    notifyObserversAsync(eNotify_ScanStarted, &notifyData);
    notifyObserversAsync(eNotify_ScanProgress, &notifyData);

    /* loop through all frequencies in given freq list */
    for (pFreq = freqListItr.first(); pFreq; pFreq = freqListItr.next())
    {
        NEXUS_FrontendVsbSettings settings;
        NEXUS_Frontend_GetDefaultVsbSettings(&settings);

        /* handle default settings for optional paramters */
        settings           = chVsb.getSettings();
        settings.frequency = *pFreq;
        settings.mode      = (-1 != _scanData._mode) ? (NEXUS_FrontendVsbMode)_scanData._mode :
                             NEXUS_FrontendVsbMode_e8;
        settings.autoAcquire = true;

        chVsb.setSettings(settings);
        chVsb.setTransportType(NEXUS_TransportType_eTs);

        /* try to tune */
        ret = chVsb.tune(_scanThread_id, _pConfig, true);
        if (eRet_Ok == ret)
        {
            /* tune successful */

            chVsb.setMajor(major);

            if (0 < chVsb.addPsiPrograms(_scanThread_callback, _scanThread_context))
            {
                /* found channels so increment major channel number */
                major++;
            }
        }
        else
        {
            /* tune failed */
            BDBG_MSG(("NO Vsb channel found at freq:%u", chVsb.getFrequency()));
        }

        chVsb.unTune(_pConfig, false, false);

        numFreqScanned++;
        notifyData.setPercent(100 * numFreqScanned / numFreqToScan);
        notifyObserversAsync(eNotify_ScanProgress, &notifyData);
    }

error:
    if (chVsb.isTuned())
    {
        chVsb.unTune(_pConfig, true);
    }
    scanDone(&notifyData);
    _pWidgetEngine->removeCallback(this, CALLBACK_TUNER_LOCK_STATUS_VSB);
} /* doScan */

#endif /* NEXUS_HAS_FRONTEND */