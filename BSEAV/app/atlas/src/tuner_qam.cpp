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

#define CALLBACK_TUNER_LOCK_STATUS_QAM  "CallbackTunerLockStatusQam"

#include "channel_qam.h"
#include "tuner_qam.h"
#include "ts_psi.h"
#include "ts_pat.h"

#include "nexus_frontend.h"
#include "nexus_frontend_qam.h"

#ifdef SNMP_SUPPORT
#include <sys/times.h>
#endif

BDBG_MODULE(atlas_tuner_qam);

static void statusCallback(
        void * context,
        int    param
        )
{
    CTuner * pTuner = (CTuner *)context;

    BSTD_UNUSED(param);
    BDBG_ASSERT(NULL != pTuner);

    B_Event_Set(pTuner->getStatusEvent());
}

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
        pWidgetEngine->syncCallback(pTuner, CALLBACK_TUNER_LOCK_STATUS_QAM);
    }

    /* set lock event for synchronous tune calls waiting for lock */
    B_Event_Set(pTuner->getLockEvent());
} /* nexusTunerLockCallback */

static void bwinTunerLockStatusCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CTunerQam * pTuner = (CTunerQam *)pObject;

    BDBG_ASSERT(NULL != pTuner);
    BSTD_UNUSED(strCallback);

    pTuner->handleLockCallback();
}

void CTunerQam::handleLockCallback()
{
    BDBG_MSG(("CTunerQam BWIN tuner lock callback:%d", isLocked()));
    setLockState(NEXUS_FrontendLockStatus_eLocked == isLocked() ? true : false);

    notifyObservers(eNotify_TunerLockStatus, this);

    if (NULL != _tunerLockCallback)
    {
        /* disabled for now...
         * _tunerLockCallback(_tunerLockCallbackContext, _tunerLockCallbackData);
         */
    }
}

void CTunerQamScanData::dump()
{
    MListItr <uint32_t> itr(&_freqList);
    uint32_t *          pFreq = NULL;

    BDBG_WRN(("tuner type:%d", getTunerType()));
    BDBG_WRN(("List of QAM Scan Frequency Requests:"));
    for (pFreq = (uint32_t *)itr.first(); pFreq; pFreq = (uint32_t *)itr.next())
    {
        BDBG_WRN(("freq:%u", *pFreq));
    }
    BDBG_WRN(("bandwidth:%d mode:%d annexA:%d annexB:%d annexC:%d symRateMax:%d symRateMin:%d", _bandwidth, _mode, _annex[0], _annex[1], _annex[2], _symbolRateMax, _symbolRateMin));
    BDBG_WRN(("append to channel list:%d", _appendToChannelList));
}

CTunerQam::CTunerQam(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    CTuner(name, number, eBoardResource_frontendQam, pCfg)
{
    _scanThread_name = "CTunerQam_Scan";
}

eRet CTunerQam::tune(NEXUS_FrontendQamSettings settings)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    settings.asyncStatusReadyCallback.callback = statusCallback;
    settings.asyncStatusReadyCallback.context  = this;

    settings.lockCallback.callback = nexusTunerLockCallback;
    settings.lockCallback.context  = this;

    NEXUS_StartCallbacks(_frontend);
    B_Event_Reset(_statusEvent);
    B_Event_Reset(_lockEvent);
    B_Event_Reset(_waitEvent);

    /* debug */
    {
        BDBG_MSG(("TUNE mode:            %s", modeToString(settings.mode).s()));
        BDBG_MSG(("TUNE annex:           %s", annexToString(settings.annex).s()));
        BDBG_MSG(("TUNE freq:            %u", settings.frequency));
        BDBG_MSG(("TUNE sym rate:        %u", settings.symbolRate));
        BDBG_MSG(("TUNE fastQcq:         %s", (0 == settings.fastAcquisition) ? "false" : "true"));
        BDBG_MSG(("TUNE terrestrial:     %s", (0 == settings.terrestrial) ? "false" : "true"));
        BDBG_MSG(("TUNE autoAcq:         %s", (0 == settings.autoAcquire) ? "false" : "true"));
        BDBG_MSG(("TUNE bandwidth:       %u", settings.bandwidth));
        BDBG_MSG(("TUNE pwr measurement: %s", (0 == settings.enablePowerMeasurement) ? "false" : "true"));
        BDBG_MSG(("TUNE spectrum mode:   %s", (NEXUS_FrontendQamSpectrumMode_eAuto == settings.spectrumMode) ? "auto" : "manual"));
        BDBG_MSG(("TUNE spectral inv:    %s", (NEXUS_FrontendQamSpectralInversion_eNormal == settings.spectralInversion) ? "normal" : "inverted"));
        BDBG_MSG(("TUNE freq offset:     %u", settings.frequencyOffset));
        BDBG_MSG(("TUNE enable null pkts:%s", (0 == settings.enableNullPackets) ? "false" : "true"));
        BDBG_MSG(("TUNE acq mode:        %s", acquisitionModeToString(settings.acquisitionMode).s()));
    }

    nerror = NEXUS_Frontend_TuneQam(_frontend, &settings);
    CHECK_NEXUS_ERROR_GOTO("unable to tune QAM frontend", ret, nerror, error);

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

eRet CTunerQam::open(CWidgetEngine * pWidgetEngine)
{
    eRet ret = eRet_Ok;

    /* call base class method */
    ret = CTuner::open(pWidgetEngine);
    CHECK_ERROR_GOTO("unable to open tuner", ret, error);

    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->addCallback(this, CALLBACK_TUNER_LOCK_STATUS_QAM, bwinTunerLockStatusCallback);
    }
    else
    {
        BDBG_MSG(("Opening CTunerQam num:%d without providing widgetEngine - tuner callbacks will be ignored", getNumber()));
    }

error:
    return(ret);
} /* open */

void CTunerQam::close()
{
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_TUNER_LOCK_STATUS_QAM);
    }

    /* call base class method */
    CTuner::close();
}

void CTunerQam::unTune(bool bFullUnTune)
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

ENUM_TO_MSTRING_INIT_CPP(CTunerQam, modeToString, NEXUS_FrontendQamMode)
ENUM_TO_MSTRING_START()
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamMode_e16, "16")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamMode_e32, "32")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamMode_e64, "64")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamMode_e128, "128")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamMode_e256, "256")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamMode_e512, "512")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamMode_e1024, "1024")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamMode_e2048, "2048")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamMode_e4096, "4096")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamMode_eAuto_64_256, "Auto 64/256")
ENUM_TO_MSTRING_END()

ENUM_TO_MSTRING_INIT_CPP(CTunerQam, annexToString, NEXUS_FrontendQamAnnex)
ENUM_TO_MSTRING_START()
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamAnnex_eA, "A")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamAnnex_eB, "B")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamAnnex_eC, "C")
ENUM_TO_MSTRING_END()

ENUM_TO_MSTRING_INIT_CPP(CTunerQam, acquisitionModeToString, NEXUS_FrontendQamAcquisitionMode)
ENUM_TO_MSTRING_START()
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamAcquisitionMode_eAuto, "Auto")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamAcquisitionMode_eFast, "Fast")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamAcquisitionMode_eSlow, "Slow")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamAcquisitionMode_eScan, "Scan")
ENUM_TO_MSTRING_END()

int CTunerQam::int_log10(float f)
{
    int log = 0;

    if (f <= 0)
    {
        return(1); /* special value */
    }
    else
    if (f < 1.0)
    {
        while (f < 1.0)
        {
            f *= 10.0;
            log--;
        }
    }
    else
    {
        int n = (int)f;
        while (n >= 10)
        {
            n /= 10;
            log++;
        }
    }
    return(log);
} /* int_log10 */

void CTunerQam::getProperties(
        MStringHash * pProperties,
        bool          bClear
        )
{
    NEXUS_FrontendQamStatus       nStatus;
    NEXUS_FrontendOutOfBandStatus nStatus_oob;
    NEXUS_FrontendCapabilities    caps;
    NEXUS_Error                   nerror = NEXUS_SUCCESS;
    eRet ret                             = eRet_Ok;

    BDBG_ASSERT(NULL != _frontend);

    if (true == bClear)
    {
        pProperties->clear();
    }

    NEXUS_Frontend_GetCapabilities(_frontend, &caps);
    if (caps.qam)
    {
        char str[32];

        {
            B_Error berror  = B_ERROR_SUCCESS;
            int     timeout = GET_INT(_pCfg, TUNE_QAM_STATUS_TIMEOUT);

            nerror = NEXUS_Frontend_RequestQamAsyncStatus(_frontend);
            CHECK_NEXUS_ERROR_GOTO("unable to request qam async frontend status", ret, nerror, error);

            berror = B_Event_Wait(getStatusEvent(), timeout);
            CHECK_BOS_WARN_GOTO("retrieving QAM tuner status failed", ret, berror, error);

            nerror = NEXUS_Frontend_GetQamAsyncStatus(_frontend, &nStatus);
            CHECK_NEXUS_ERROR_GOTO("unable to get qam async frontend status", ret, nerror, error);
        }

        snprintf(str, 32, "%0.2f MHz", nStatus.settings.frequency / 1000000.0);
        pProperties->add("Frequency", str);
        snprintf(str, 32, "%s", modeToString(nStatus.settings.mode).s());
        pProperties->add("Mode", str);
        snprintf(str, 32, "%s", annexToString(nStatus.settings.annex).s());
        pProperties->add("Annex", str);
        snprintf(str, 32, "%c", nStatus.fecLock ? 'Y' : 'N');
        pProperties->add("FEC Lock", str);
        snprintf(str, 32, "%c", nStatus.receiverLock ? 'Y' : 'N');
        pProperties->add("QAM Lock", str);
        snprintf(str, 32, "%f", nStatus.symbolRate / 1000000.0);
        pProperties->add("Symbol Rate", str);
        snprintf(str, 32, "%0.2f dB", nStatus.snrEstimate / 100.0);
        pProperties->add("SNR", str);
        snprintf(str, 32, "%0.1f%%", nStatus.ifAgcLevel/10.0);
        pProperties->add("IF AGC Level", str);
        snprintf(str, 32, "%0.1f%%", nStatus.rfAgcLevel/10.0);
        pProperties->add("RF AGC Level", str);
        if (0 < nStatus.postRsBer)
        {
            snprintf(str, 32, "10 ^ %d", int_log10(nStatus.postRsBer));
        }
        else
        {
            snprintf(str, 32, "0");
        }
        pProperties->add("Post Reed-Sol BER", str);
        snprintf(str, 32, "%0.2f KHz", nStatus.carrierFreqOffset / 1000000.0);
        pProperties->add("Carrier Freq Offset", str);
        snprintf(str, 32, "%0.2f KHz", nStatus.carrierPhaseOffset / 1000000.0);
        pProperties->add("Carrier Phase Offset", str);
        snprintf(str, 32, "%d", nStatus.fecCorrected);
        pProperties->add("FEC Corrected", str);
        snprintf(str, 32, "%d", nStatus.fecUncorrected);
        pProperties->add("FEC Uncorrected", str);
        snprintf(str, 32, "%0.2f dBmV", (float)(nStatus.dsChannelPower) / 10.0);
        pProperties->add("DS Channel Power", str);
    }
    else
    {
        char str[32];

        nerror = NEXUS_Frontend_GetOutOfBandStatus(_frontend, &nStatus_oob);
        CHECK_NEXUS_ERROR_GOTO("unable to get oob frontend status properties", ret, nerror, error);

        snprintf(str, 32, "%c", nStatus_oob.isFecLocked ? 'Y' : 'N');
        pProperties->add("FEC Lock", str);
        snprintf(str, 32, "%c", nStatus_oob.isQamLocked ? 'Y' : 'N');
        pProperties->add("QAM Lock", str);
        snprintf(str, 32, "%f", nStatus_oob.symbolRate / 1000000.0);
        pProperties->add("Symbol Rate", str);
        snprintf(str, 32, "%0.2f dB", nStatus_oob.snrEstimate / 100.0);
        pProperties->add("SNR", str);
        snprintf(str, 32, "%0.1f%%", nStatus_oob.agcIntLevel/10.0);
        pProperties->add("IF AGC Level", str);
        snprintf(str, 32, "%0.1f%%", nStatus_oob.agcExtLevel/10.0);
        pProperties->add("RF AGC Level", str);
        snprintf(str, 32, "%0.2f KHz", nStatus_oob.carrierFreqOffset / 1000000.0);
        pProperties->add("Carrier Freq Offset", str);
        snprintf(str, 32, "%0.2f KHz", nStatus_oob.carrierPhaseOffset / 1000000.0);
        pProperties->add("Carrier Phase Offset", str);
        snprintf(str, 32, "%d", nStatus_oob.correctedCount);
        pProperties->add("FEC Corrected", str);
        snprintf(str, 32, "%d", nStatus_oob.uncorrectedCount);
        pProperties->add("FEC Uncorrected", str);
    }

#ifdef SNMP_SUPPORT
    {
        unsigned int diff;
        clock_t      curr_time;
        double       elapsed_symbol;
        unsigned     symbolrate;
        struct tms   _tms;

        curr_time = times(&_tms);

        if (caps.qam)
        {
            BKNI_Memcpy(&_tuner_status.status.qam, &nStatus, sizeof(NEXUS_FrontendQamStatus));
        }
        else
        {
            BKNI_Memcpy(&_tuner_status.status.oob, &nStatus_oob, sizeof(NEXUS_FrontendOutOfBandStatus));
        }

        if (_tuner_status.state == lock_locked)
        {
            if (caps.qam)
            {
                _tuner_status.uncorrected += nStatus.fecUncorrected;
                _tuner_status.corrected   += nStatus.fecCorrected;
            }
            else
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
                symbolrate                 = caps.qam ? nStatus.symbolRate : nStatus_oob.symbolRate;
                elapsed_symbol             = (double)(symbolrate/100.00)*(double)diff;
                _tuner_status.total_symbol = (unsigned)elapsed_symbol;
                if (!caps.qam) /* OOB is always QPSK */
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
    }
#endif /* ifdef SNMP_SUPPORT */

error:
    return;
} /* getProperties */

NEXUS_FrontendLockStatus CTunerQam::isLocked()
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

NEXUS_FrontendQamMode CTunerQam::getDefaultMode()
{
    /* dtt todo: 7231 is incorrectly reporting NEXUS_FrontendQamMode_eAuto_64_256 is
     * is a valid qam mode, but then chokes on it if you use it while tuning.  for now
     * we'll always use e256 as the default mode */
    return(NEXUS_FrontendQamMode_e256);

#if 0
    if (true == _capabilities.qamModes[NEXUS_FrontendQamMode_eAuto_64_256])
    {
        return(NEXUS_FrontendQamMode_eAuto_64_256);
    }
    else
    {
        return(NEXUS_FrontendQamMode_e256);
    }
#endif /* if 0 */
}

uint32_t CTunerQam::getDefaultSymbolRateMax(NEXUS_FrontendQamMode mode)
{
    uint32_t symbolRate = 0;

    switch (mode)
    {
    case NEXUS_FrontendQamMode_e64:
        symbolRate = 5056900;
        break;

    case NEXUS_FrontendQamMode_e256:
        symbolRate = 5360537;
        break;

    case NEXUS_FrontendQamMode_eAuto_64_256:
        symbolRate = 5360537;
        break;

    default:
        symbolRate = 5360537; /* default to e256 symbol rate */
        break;
    } /* switch */
    BDBG_WRN(("No MAX symbol rate given - defaulting to:%d", symbolRate));

    return(symbolRate);
} /* getDefaultSymbolRateMax */

uint32_t CTunerQam::getDefaultSymbolRateMin(NEXUS_FrontendQamMode mode)
{
    uint32_t symbolRate = 0;

    switch (mode)
    {
    case NEXUS_FrontendQamMode_e64:
        symbolRate = 5056900;
        break;

    case NEXUS_FrontendQamMode_e256:
        symbolRate = 5360537;
        break;

    case NEXUS_FrontendQamMode_eAuto_64_256:
        symbolRate = 5056900;
        break;

    default:
        symbolRate = 5360537; /* default to e256 symbol rate */
        break;
    } /* switch */
    BDBG_WRN(("No MIN symbol rate given - defaulting to:%d", symbolRate));

    return(symbolRate);
} /* getDefaultSymbolRateMin */

void CTunerQam::saveScanData(CTunerScanData * pScanData)
{
    _scanData = (*((CTunerQamScanData *)pScanData));
}

#ifdef MPOD_SUPPORT
void CTunerQam::doScan()
{
    eRet                       ret   = eRet_Ok;
    uint16_t                   major = 1;
    CChannelQam                chQam(_pCfg, this); /* temp channel we'll use to do tuning during scan */
    CTunerScanNotificationData notifyData(this);   /* notification data for reporting scan start/stop/progress */
    channel_list_t *           _pChannelList;
    int index;

    BDBG_ASSERT(NULL != _scanThread_handle);

    if (!_scanData._useCableCardSiData)
    {
        /* nothing to scan so we are done */
        BDBG_WRN((" MPOD_SUPPORT =y disables the manual update of channel map"));
        BDBG_WRN((" Channel map update will be automatically triggered through by Cable Card"));
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
    major         = _scanData._majorStartNum;
    _pChannelList = _scanData._pChannelList;

    notifyObserversAsync(eNotify_ScanStarted, &notifyData);
    notifyObserversAsync(eNotify_ScanProgress, &notifyData);

    for (index = 0; index < _pChannelList->active_channels; index++)
    {
        NEXUS_FrontendQamSettings settings;
        NEXUS_Frontend_GetDefaultQamSettings(&settings);
        /* handle default settings for optional paramters */
        settings             = chQam.getSettings();
        settings.frequency   = _pChannelList->channel[index].frequency;
        settings.mode        = _pChannelList->channel[index].modulation;
        settings.annex       = _pChannelList->channel[index].annex;
        settings.symbolRate  = _pChannelList->channel[index].symbolrate;
        settings.bandwidth   = _scanData._bandwidth;
        settings.autoAcquire = false;
        chQam.setSettings(settings);
        chQam.setTransportType(NEXUS_TransportType_eTs);
        /* try to tune */
        ret = chQam.tune(_scanThread_id, _pConfig, true);

        if (eRet_Ok == ret)
        {
            chQam.setMajor(major);
            if (0 < chQam.addPsiPrograms(_scanThread_callback, _scanThread_context))
            {
                /* found channels so increment major channel number */
                major++;
                chQam.setSourceId(_pChannelList->channel[index].source_id);
            }
        }
        chQam.unTune(_pConfig, false, false);
        notifyData.setPercent(100 * index / _pChannelList->active_channels);
        notifyObserversAsync(eNotify_ScanProgress, &notifyData);
    }

error:
    if (chQam.isTuned())
    {
        chQam.unTune(_pConfig, false, false);
    }
    scanDone(&notifyData);
    _pWidgetEngine->removeCallback(this, CALLBACK_TUNER_LOCK_STATUS_QAM);
} /* doScan */

#else /* ifdef MPOD_SUPPORT */
void CTunerQam::doScan()
{
    eRet                       ret            = eRet_Ok;
    uint16_t                   major          = 1;
    uint16_t                   numFreqToScan  = 0;
    uint16_t                   numFreqScanned = 0;
    CChannelQam                chQam(_pCfg, this); /* temp channel we'll use to do tuning during scan */
    CTunerScanNotificationData notifyData(this);   /* notification data for reporting scan start/stop/progress */
    uint32_t *                 pFreq = NULL;

    MListItr <uint32_t> freqListItr(&(_scanData._freqList));

    BDBG_ASSERT(NULL != _scanThread_handle);

    numFreqToScan = _scanData._freqList.total();
    if (0 == numFreqToScan)
    {
        /* nothing to scan so we are done */
        BDBG_WRN(("No scan frequencies given!"));
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
        NEXUS_FrontendQamSettings settings;
        NEXUS_Frontend_GetDefaultQamSettings(&settings);

        /* handle default settings for optional paramters */
        settings                        = chQam.getSettings();
        settings.frequency              = *pFreq;
        settings.bandwidth              = (-1 != _scanData._bandwidth) ? _scanData._bandwidth : 6000000;
        settings.autoAcquire            = true;
        settings.enablePowerMeasurement = false;
        settings.frequencyOffset        = 250000;
        settings.acquisitionMode        = NEXUS_FrontendQamAcquisitionMode_eScan;
        settings.spectrumMode           = NEXUS_FrontendQamSpectrumMode_eAuto;
        settings.enableNullPackets      = false;
        settings.mode                   = (-1 != _scanData._mode) ? (NEXUS_FrontendQamMode)_scanData._mode : getDefaultMode();
        settings.scan.upperBaudSearch   = (-1 != _scanData._symbolRateMax) ? _scanData._symbolRateMax : getDefaultSymbolRateMax(settings.mode);
        settings.scan.lowerBaudSearch   = (-1 != _scanData._symbolRateMin) ? _scanData._symbolRateMin : getDefaultSymbolRateMin(settings.mode);

        /* set standard symbol rate setting for older platforms do not support advanced scan */
        settings.symbolRate = settings.scan.upperBaudSearch;

        if (-1 != _scanData._mode)
        {
            int annexSum = 0;

            /* sanity check: make sure at least one annex is set */
            for (int i = 0; i < NEXUS_FrontendQamAnnex_eMax; i++)
            {
                annexSum += _scanData._annex[i];
            }
            BDBG_ASSERT(0 < annexSum);

            memset(settings.scan.mode, 0, sizeof(settings.scan.mode));

            if (NEXUS_FrontendQamMode_eAuto_64_256 == (NEXUS_FrontendQamMode)_scanData._mode)
            {
                for (int i = 0; i < NEXUS_FrontendQamAnnex_eMax; i++)
                {
                    settings.scan.mode[i][NEXUS_FrontendQamMode_e64]  = _scanData._annex[i];
                    settings.scan.mode[i][NEXUS_FrontendQamMode_e256] = _scanData._annex[i];
                }
            }
            else
            {
                for (int i = 0; i < NEXUS_FrontendQamAnnex_eMax; i++)
                {
                    settings.scan.mode[i][(NEXUS_FrontendQamMode)_scanData._mode] = _scanData._annex[i];
                }
            }
        }

        for (int i = 0; i < NEXUS_FrontendQamAnnex_eMax; i++)
        {
            /* scan for all given annex types */
            if (_scanData._annex[i])
            {
                settings.annex = (NEXUS_FrontendQamAnnex)i;

                chQam.setSettings(settings);
                chQam.setTransportType(NEXUS_TransportType_eTs);

                /* try to tune */
                ret = chQam.tune(_scanThread_id, _pConfig, true);
                if (eRet_NotSupported == ret)
                {
                    /* some frontends don't accurately represent their capabilities.  for QAM it is most
                     * likely when specifying auto mode which really may not be supported. if this is the
                     * case, we will default to 256 mode and try tuning again. */
                    settings.mode   = NEXUS_FrontendQamMode_e256;
                    _scanData._mode = NEXUS_FrontendQamMode_e256;
                    chQam.setSettings(settings);

                    BDBG_WRN(("Failure tuning with unsupported QAM mode - defaulting to 256"));
                    ret = chQam.tune(_scanThread_id, _pConfig, true);
                }

                if (eRet_Ok == ret)
                {
                    NEXUS_FrontendQamScanStatus scanStatus;
                    NEXUS_Error                 nerror = NEXUS_SUCCESS;

                    /* tune successful */
                    nerror = NEXUS_Frontend_GetQamScanStatus(getFrontend(), &scanStatus);
                    if (eRet_Ok == CHECK_NEXUS_ERROR("Error retrieving QAM scan status", nerror))
                    {
                        BDBG_WRN(("Scan Status: "));
                        BDBG_WRN(("SymbolRate = %d", scanStatus.symbolRate));
                        BDBG_WRN(("Mode = %d", scanStatus.mode));
                        BDBG_WRN(("Annex = %d", scanStatus.annex));
                        BDBG_WRN(("Interleaver = %d", scanStatus.interleaver));
                        BDBG_WRN(("Spectrum inverted = %s", scanStatus.spectrumInverted ? "True" : "False"));
                        BDBG_WRN(("Acquisition status = %d", scanStatus.acquisitionStatus));

                        {
                            NEXUS_FrontendQamSettings qamSettings;
                            /* update channel object with scan status data if available */
                            qamSettings            = chQam.getSettings();
                            qamSettings.mode       = scanStatus.mode;
                            qamSettings.annex      = scanStatus.annex;
                            qamSettings.symbolRate = scanStatus.symbolRate;
                            chQam.setSettings(qamSettings);
                        }
                    }

                    chQam.setMajor(major);

                    if (0 < chQam.addPsiPrograms(_scanThread_callback, _scanThread_context))
                    {
                        /* found channels so increment major channel number */
                        major++;
                    }
                }
                else
                {
                    /* tune failed */
                    BDBG_MSG(("NO qam channel found at freq:%u", chQam.getFrequency()));
                }

                chQam.unTune(_pConfig, false, false);
            }
        }

        numFreqScanned++;
        notifyData.setPercent(100 * numFreqScanned / numFreqToScan);
        notifyObserversAsync(eNotify_ScanProgress, &notifyData);
    }

error:
    if (chQam.isTuned())
    {
        chQam.unTune(_pConfig, true);
    }
    scanDone(&notifyData);
    _pWidgetEngine->removeCallback(this, CALLBACK_TUNER_LOCK_STATUS_QAM);
} /* doScan */

#endif /* ifdef MPOD_SUPPORT */
#endif /* NEXUS_HAS_FRONTEND */