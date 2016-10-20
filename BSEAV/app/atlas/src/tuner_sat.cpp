/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#if NEXUS_HAS_FRONTEND

#define CALLBACK_TUNER_LOCK_STATUS_SAT  "CallbackTunerLockStatusSat"

#include "channel_sat.h"
#include "tuner_sat.h"
#include "ts_psi.h"
#include "ts_pat.h"
#ifdef MPOD_SUPPORT
#include "tspsimgr.h"
#else
#include "tspsimgr2.h"
#endif

BDBG_MODULE(atlas_tuner_sat);

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
        pWidgetEngine->syncCallback(pTuner, CALLBACK_TUNER_LOCK_STATUS_SAT);
    }

    /* set lock event for synchronous tune calls waiting for lock */
    B_Event_Set(pTuner->getLockEvent());
} /* nexusTunerLockCallback */

static void bwinTunerLockStatusCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CTunerSat * pTuner = (CTunerSat *)pObject;

    BDBG_ASSERT(NULL != pTuner);
    BSTD_UNUSED(strCallback);

    pTuner->handleLockCallback();
}

void CTunerSat::handleLockCallback()
{
    BDBG_MSG(("CTunerSat BWIN tuner lock callback:%d", isLocked()));
    setLockState(NEXUS_FrontendLockStatus_eLocked == isLocked() ? true : false);

    notifyObservers(eNotify_TunerLockStatus, this);

    if (NULL != _tunerLockCallback)
    {
        /* disabled for now...
         * _tunerLockCallback(_tunerLockCallbackContext, _tunerLockCallbackData);
         */
    }
}

void CTunerSatScanData::dump()
{
    MListItr <uint32_t> itr(&_freqList);
    uint32_t *          pFreq = NULL;

    BDBG_WRN(("tuner type:%d", getTunerType()));
    BDBG_WRN(("List of Sat Scan Frequency Requests:"));
    for (pFreq = (uint32_t *)itr.first(); pFreq; pFreq = (uint32_t *)itr.next())
    {
        BDBG_WRN(("freq:%u", *pFreq));
    }

    BDBG_WRN(("append to channel list:%d", _appendToChannelList));
}

CTunerSat::CTunerSat(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    CTuner(name, number, eBoardResource_frontendSds, pCfg),
    _scanThread_psCallbackEvent(NULL),
    _bDiseqc(false)
{
    memset(&_scanThread_psResult, 0, sizeof(_scanThread_psResult));
    _scanThread_name = "CTunerSat_Scan";
}

/* send tuner capabilities using notifications */
eRet CTunerSat::initCapabilities()
{
    NEXUS_FrontendCapabilities          capabilities;
    NEXUS_FrontendSatelliteCapabilities capabilitiesSat;
    NEXUS_FrontendDeviceHandle          deviceHandle = NULL;
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    BDBG_ASSERT(NULL != _frontend);

    deviceHandle = NEXUS_Frontend_GetDevice(_frontend);
    CHECK_PTR_ERROR_GOTO("unable to get sat frontend device", deviceHandle, ret, eRet_NotAvailable, error);

    nerror = NEXUS_FrontendDevice_GetSatelliteCapabilities(deviceHandle, &capabilitiesSat);
    CHECK_NEXUS_WARN_GOTO("Unable to retrieve sat tuner capabilities", ret, nerror, error);

    NEXUS_Frontend_GetCapabilities(_frontend, &capabilities);
    _bDiseqc = capabilities.diseqc;

    goto done;
error:
    /* error getting capabilities so default to 1 */
    BDBG_WRN(("Default numAdc to 1"));
    capabilitiesSat.numAdc      = 1;
    capabilitiesSat.numChannels = 1;
done:
    notifyObservers(eNotify_Capabilities, &capabilitiesSat);
    return(ret);
} /* initCapabilities */

eRet CTunerSat::tune(satSettings & settings)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    settings.sat.lockCallback.callback = nexusTunerLockCallback;
    settings.sat.lockCallback.context  = this;

    NEXUS_StartCallbacks(_frontend);
    B_Event_Reset(_lockEvent);
    B_Event_Reset(_waitEvent);

    if (settings.satCapabilities.numAdc > 0)
    {
        BDBG_MSG(("Selected satellite ADC setting:%d", settings.runSettings.selectedAdc));
        nerror = NEXUS_Frontend_SetSatelliteRuntimeSettings(_frontend, &settings.runSettings);
        CHECK_NEXUS_WARN("unable to set satellite runtime settings - may not be supported for this tuner", nerror);
    }

    nerror = NEXUS_Frontend_SetDiseqcSettings(_frontend, &settings.diseqc);
    CHECK_NEXUS_ERROR_GOTO("unable to set sat diseqc settings", ret, nerror, error);

    nerror = NEXUS_Frontend_TuneSatellite(_frontend, &settings.sat);
    CHECK_NEXUS_ERROR_GOTO("unable to tune Sat frontend", ret, nerror, error);

error:
    return(ret);
} /* tune */

eRet CTunerSat::open(CWidgetEngine * pWidgetEngine)
{
    eRet ret = eRet_Ok;

    /* call base class method */
    ret = CTuner::open(pWidgetEngine);
    CHECK_ERROR_GOTO("unable to open tuner", ret, error);

    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->addCallback(this, CALLBACK_TUNER_LOCK_STATUS_SAT, bwinTunerLockStatusCallback);
    }
    else
    {
        BDBG_MSG(("Opening CTunerSat num:%d without providing widgetEngine - tuner callbacks will be ignored", getNumber()));
    }

error:
    return(ret);
} /* open */

void CTunerSat::close()
{
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_TUNER_LOCK_STATUS_SAT);
    }

    /* call base class method */
    CTuner::close();
}

void CTunerSat::unTune(bool bFullUnTune)
{
    NEXUS_StopCallbacks(_frontend);
    if (true == bFullUnTune)
    {
        NEXUS_Frontend_Untune(_frontend);
    }
}

ENUM_TO_MSTRING_INIT_CPP(CTunerSat, modeToString, NEXUS_FrontendSatelliteMode)
ENUM_TO_MSTRING_START()
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendSatelliteMode_eDvb, "DVB")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendSatelliteMode_eDss, "DSS")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendSatelliteMode_eDcii, "DCII")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendSatelliteMode_eQpskTurbo, "QPSK Turbo")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendSatelliteMode_e8pskTurbo, "8PSK Turbo")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendSatelliteMode_eTurbo, "Turbo")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendSatelliteMode_eQpskLdpc, "QPSK LDPC")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendSatelliteMode_e8pskLdpc, "8PSK LDPC")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendSatelliteMode_eLdpc, "LDPC")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendSatelliteMode_eBlindAcquisition, "Blind Acq")
ENUM_TO_MSTRING_END()

ENUM_TO_MSTRING_INIT_CPP(CTunerSat, spectralInvToString, NEXUS_FrontendSatelliteInversion)
ENUM_TO_MSTRING_START()
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendSatelliteInversion_eScan, "Scan")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendSatelliteInversion_eNormal, "Normal")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendSatelliteInversion_eI, "I")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendSatelliteInversion_eQ, "Q")
ENUM_TO_MSTRING_END()

void CTunerSat::getProperties(
        MStringHash * pProperties,
        bool          bClear
        )
{
    NEXUS_FrontendSatelliteStatus nStatus;
    NEXUS_Error                   nerror = NEXUS_SUCCESS;
    eRet ret                             = eRet_Ok;

    BDBG_ASSERT(NULL != _frontend);

    if (true == bClear)
    {
        pProperties->clear();
    }

    {
        char str[32];

        nerror = NEXUS_Frontend_GetSatelliteStatus(_frontend, &nStatus);
        CHECK_NEXUS_ERROR_GOTO("unable to get sat frontend status properties", ret, nerror, error);

        snprintf(str, 32, "%0.2f MHz", nStatus.settings.frequency / 1000000.0);
        pProperties->add("Frequency", str);
        snprintf(str, 32, "%s", modeToString(nStatus.settings.mode).s());
        pProperties->add("Mode", str);
        snprintf(str, 32, "%0.2f Mbps", nStatus.settings.symbolRate / 1000000.0);
        pProperties->add("Mode", str);

        snprintf(str, 32, "%c", nStatus.demodLocked ? 'Y' : 'N');
        pProperties->add("Demod Lock", str);
        snprintf(str, 32, "%c", nStatus.tunerLocked ? 'Y' : 'N');
        pProperties->add("Satellite Lock", str);
        snprintf(str, 32, "%c", nStatus.bertLocked ? 'Y' : 'N');
        pProperties->add("BER Tester Lock", str);

        snprintf(str, 32, "%d/%d", nStatus.codeRate.numerator, nStatus.codeRate.denominator);
        pProperties->add("Code Rate", str);
        snprintf(str, 32, "%s", spectralInvToString(nStatus.spectralInversion).s());
        pProperties->add("Spectral Inversion", str);
        snprintf(str, 32, "%d Hz", nStatus.carrierError);
        pProperties->add("Carrier Error", str);
        snprintf(str, 32, "%0.2f dB", nStatus.snrEstimate / 100.0);
        pProperties->add("S/N Ratio Estimate", str);
        if (1 > nStatus.berEstimate)
        {
            snprintf(str, 32, "10 ^ %d", nStatus.berEstimate);
        }
        else
        {
            snprintf(str, 32, "N/A");
        }
        pProperties->add("BER Estimate", str);
        snprintf(str, 32, "SDS%d", nStatus.channel);
        pProperties->add("Channel Num", str);

        snprintf(str, 32, "%d", nStatus.fecCorrected);
        pProperties->add("FEC Corrected", str);
        snprintf(str, 32, "%d", nStatus.fecUncorrected);
        pProperties->add("FEC Uncorrected", str);
        snprintf(str, 32, "%d", nStatus.fecClean);
        pProperties->add("FEC Clean", str);
    }

error:
    return;
} /* getProperties */

NEXUS_FrontendLockStatus CTunerSat::isLocked()
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

#if 0
    eRet                          ret    = eRet_Ok;
    NEXUS_Error                   nerror = NEXUS_SUCCESS;
    NEXUS_FrontendSatelliteStatus status;
    bool                          locked = false;

    /* DTT - possible problem with NEXUS_Frontend_GetSatelliteStatus() so for now we'll rely on the
     * status received during the last lock changed event */
    return(_locked);

    nerror = NEXUS_Frontend_GetSatelliteStatus(getFrontend(), &status);
    if (NEXUS_SUCCESS == nerror)
    {
        locked = status.demodLocked;
    }

    return(locked);

#endif /* if 0 */
} /* isLocked */

/* Do a Blind Scan */

#include "nexus_frontend.h"
#include "nexus_frontend_satellite.h"

#if 0 /* peak scan */
static void peakscan_callback(
        void * context,
        int    param
        )
{
    CTunerSat * pTuner = (CTunerSat *)context;
    NEXUS_FrontendSatellitePeakscanResult result;
    NEXUS_Error rc;

    BSTD_UNUSED(param);

    rc = NEXUS_Frontend_GetSatellitePeakscanResult(pTuner->getFrontend(), &result);
    BDBG_ASSERT(!rc);
    BDBG_WRN(("Peak scan callback. freq=%u, symRate=%u, power=%#x\n",
              result.peakFrequency, result.symbolRate, result.peakPower));
    pTuner->_scanThread_psResult = result;
    BKNI_SetEvent(pTuner->_scanThread_psCallbackEvent);
}

#endif /* if 0 */

void CTunerSat::saveScanData(CTunerScanData * pScanData)
{
    _scanData = *((CTunerSatScanData *)pScanData);
}

void CTunerSat::doScan()
{
    eRet                       ret            = eRet_Ok;
    uint16_t                   major          = 1;
    uint16_t                   numFreqToScan  = 0;
    uint16_t                   numFreqScanned = 0;
    CChannelSat                chSat(_pCfg, this); /* temp channel we'll use to do tuning during scan */
    CTunerScanNotificationData notifyData(this);   /* notification data for reporting scan start/stop/progress */

    MListItr <uint32_t> freqListItr(&(_scanData._freqList));
    uint32_t *          pFreq = NULL;

    BDBG_ASSERT(NULL != _scanThread_handle);

    numFreqToScan = _scanData._freqList.total();
    if (0 == numFreqToScan)
    {
        /* nothing to scan so we are  */
        goto error;
    }

    /* set the starting major channel number for newly found channels */
    major = _scanData._majorStartNum;

    notifyObserversAsync(eNotify_ScanStarted, &notifyData);
    notifyObserversAsync(eNotify_ScanProgress, &notifyData);

    /* loop through all frequencies in given freq list */
#if 1
    for (pFreq = freqListItr.first(); pFreq; pFreq = freqListItr.next())
    {
        satSettings settings;

        /* handle default settings for optional paramters */
        settings               = *(chSat.getSettings());
        settings.sat.frequency = *pFreq*1000000;
        settings.sat.mode      = (-1 != _scanData._mode) ? (NEXUS_FrontendSatelliteMode)_scanData._mode :
                                 NEXUS_FrontendSatelliteMode_eDvb;

        settings.diseqc.voltage          = NEXUS_FrontendDiseqcVoltage_e13v;
        settings.diseqc.toneEnabled      = true;
        settings.sat.symbolRate          = 20000000;
        settings.runSettings.selectedAdc = (-1 != _scanData._adc) ? _scanData._adc : 0;

#else /* Peak Scan */
    while (1)
    {
        satSettings settings;
        NEXUS_FrontendSatellitePeakscanSettings psSettings;

        /* Set Diseqc Settings */
        settings.diseqc.enabled     = true;
        settings.diseqc.voltage     = NEXUS_FrontendDiseqcVoltage_e13v;
        settings.diseqc.toneEnabled = true;

        BKNI_CreateEvent(&_scanThread_psCallbackEvent);
        NEXUS_Frontend_GetDefaultSatellitePeakscanSettings(&psSettings);
        psSettings.frequency      = (*freqListItr.first())*10000000;
        psSettings.frequencyRange = ((*freqListItr.last())*10000000) - (*freqListItr.first())*10000000;
        /* Step in 1 Mhz */
        psSettings.peakscanCallback.callback = peakscan_callback;
        psSettings.peakscanCallback.context  = this;

        rc = NEXUS_Frontend_SatellitePeakscan(getFrontend(), &psSettings);
        BDBG_ASSERT(!rc);
        rc = BKNI_WaitForEvent(_scanThread_psCallbackEvent, 3000);
        if (rc)
        {
            BDBG_ERR(("Callback not fired. Aborting\n"));
            goto errorEvent;
        }

        settings.sat.mode          = NEXUS_FrontendSatelliteMode_eBlindAcquisition;
        settings.sat.frequency     = _scanThread_psResult.peakFrequency;
        settings.sat.symbolRate    = _scanThread_psResult.symbolRate;
        settings.sat.carrierOffset = 0;

        if (!_scanThread_psResult.peakFrequency)
        {
            BDBG_WRN(("No signal found. Continuing peak scan...\n"));
            psSettings.frequency = _scanThread_psResult.lastFrequency + psSettings.frequencyRange + psSettings.frequencyStep;
            continue;
        }

#endif /* if 1 */
        BDBG_MSG(("Frequency %d", settings.sat.frequency));
        chSat.setSettings(&settings);
        chSat.setTransportType(NEXUS_TransportType_eTs);

        /* try to tune */
        ret = chSat.tune(_scanThread_id, _pConfig, true);
        if (eRet_Ok == ret)
        {
            /* tune successful */

            chSat.setMajor(major);
            if (0 < chSat.addPsiPrograms(_scanThread_callback, _scanThread_context))
            {
                /* found channels so increment major channel number */
                major++;
            }
        }
        else
        {
            /* tune failed */

            BDBG_MSG(("NO sat channel found at freq:%u", chSat.getFrequency()));
        }

        chSat.unTune(_pConfig, false, false);
#if 1
        numFreqScanned++;
        notifyData.setPercent(100 * numFreqScanned / numFreqToScan);
#else
        psSettings.frequency = _scanThread_psResult.lastFrequency + psSettings.frequencyRange + psSettings.frequencyStep;
#endif
        notifyObserversAsync(eNotify_ScanProgress, &notifyData);
    }

#if 0 /* peak scan */
errorEvent:
#endif

#if 0
    BKNI_DestroyEvent(_scanThread_psCallbackEvent);
#endif
error:
    if (chSat.isTuned())
    {
        chSat.unTune(_pConfig, true);
    }
    scanDone(&notifyData);
    _pWidgetEngine->removeCallback(this, CALLBACK_TUNER_LOCK_STATUS_SAT);
} /* doScan */

CChannel * CTunerSat::createChannel()
{
    CChannelSat * pChannel = NULL;

    pChannel = new CChannelSat(_pCfg, this);
    return(pChannel);
}

void CTunerSat::destroyChannel(CChannel * pChannel)
{
    DEL(pChannel);
}

#endif /* NEXUS_HAS_FRONTEND */