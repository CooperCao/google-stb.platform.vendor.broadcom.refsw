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

#if NEXUS_HAS_FRONTEND

#define CALLBACK_TUNER_LOCK_STATUS_OFDM  "CallbackTunerLockStatusOfdm"

#include "channel_ofdm.h"
#include "tuner_ofdm.h"
#include "ts_psi.h"
#include "ts_pat.h"

#include "nexus_frontend.h"
#include "nexus_frontend_ofdm.h"

BDBG_MODULE(atlas_tuner_ofdm);

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
        pWidgetEngine->syncCallback(pTuner, CALLBACK_TUNER_LOCK_STATUS_OFDM);
    }

    /* set lock event for synchronous tune calls waiting for lock */
    B_Event_Set(pTuner->getLockEvent());
} /* nexusTunerLockCallback */

static void bwinTunerLockStatusCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CTunerOfdm * pTuner = (CTunerOfdm *)pObject;

    BDBG_ASSERT(NULL != pTuner);
    BSTD_UNUSED(strCallback);

    pTuner->handleLockCallback();
}

void CTunerOfdm::handleLockCallback()
{
    BDBG_MSG(("CTunerOfdm BWIN tuner lock callback:%d", isLocked()));
    setLockState(NEXUS_FrontendLockStatus_eLocked == isLocked() ? true : false);

    notifyObservers(eNotify_TunerLockStatus, this);

    if (NULL != _tunerLockCallback)
    {
        /* disabled for now...
         * _tunerLockCallback(_tunerLockCallbackContext, _tunerLockCallbackData);
         */
    }
}

void CTunerOfdmScanData::dump()
{
    MListItr <uint32_t> itr(&_freqList);
    uint32_t *          pFreq = NULL;

    BDBG_WRN(("tuner type:%d", getTunerType()));
    BDBG_WRN(("List of Ofdm Scan Frequency Requests:"));
    for (pFreq = (uint32_t *)itr.first(); pFreq; pFreq = (uint32_t *)itr.next())
    {
        BDBG_WRN(("freq:%u", *pFreq));
    }

    BDBG_WRN(("append to channel list:%d", _appendToChannelList));
}

CTunerOfdm::CTunerOfdm(
        const char *     name,
        const unsigned   number,
        CConfiguration * pCfg
        ) :
    CTuner(name, number, eBoardResource_frontendOfdm, pCfg)
{
    eRet ret = eRet_Ok;

    _scanThread_name = "CTunerOfdm_Scan";

    BDBG_ASSERT(eRet_Ok == ret);
}

eRet CTunerOfdm::tune(NEXUS_FrontendOfdmSettings * pSettings)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    BDBG_ASSERT(NULL != pSettings);

    pSettings->lockCallback.callback             = nexusTunerLockCallback;
    pSettings->lockCallback.context              = this;
    pSettings->asyncStatusReadyCallback.callback = statusCallback;
    pSettings->asyncStatusReadyCallback.context  = this;

    NEXUS_StartCallbacks(_frontend);
    B_Event_Reset(_lockEvent);
    B_Event_Reset(_waitEvent);
    B_Event_Reset(_statusEvent);

    nerror = NEXUS_Frontend_TuneOfdm(_frontend, pSettings);
    CHECK_NEXUS_ERROR_GOTO("unable to tune Ofdm frontend", ret, nerror, error);

error:
    return(ret);
} /* tune */

eRet CTunerOfdm::open(CWidgetEngine * pWidgetEngine)
{
    eRet ret = eRet_Ok;

    /* call base class method */
    ret = CTuner::open(pWidgetEngine);
    CHECK_ERROR_GOTO("unable to open tuner", ret, error);

    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->addCallback(this, CALLBACK_TUNER_LOCK_STATUS_OFDM, bwinTunerLockStatusCallback);
    }
    else
    {
        BDBG_MSG(("Opening CTunerOfdm num:%d without providing widgetEngine - tuner callbacks will be ignored", getNumber()));
    }

error:
    return(ret);
} /* open */

void CTunerOfdm::close()
{
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_TUNER_LOCK_STATUS_OFDM);
    }

    /* call base class method */
    CTuner::close();
}

void CTunerOfdm::unTune(bool bFullUnTune)
{
    NEXUS_StopCallbacks(_frontend);

    if (true == bFullUnTune)
    {
        NEXUS_Frontend_Untune(_frontend);
    }
}

ENUM_TO_MSTRING_INIT_CPP(CTunerOfdm, modeToString, NEXUS_FrontendOfdmMode)
ENUM_TO_MSTRING_START()
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendOfdmMode_eDvbt, "DVB-T")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendOfdmMode_eDvbt2, "DVB-T2")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendOfdmMode_eDvbc2, "DVB-C2")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendOfdmMode_eIsdbt, "ISDB-T")
ENUM_TO_MSTRING_END()

void CTunerOfdm::getProperties(
        MStringHash * pProperties,
        bool          bClear
        )
{
    /* coverity[stack_use_local_overflow] */
    NEXUS_FrontendOfdmStatus nStatus;
    NEXUS_Error              nerror = NEXUS_SUCCESS;
    eRet                     ret    = eRet_Ok;

    BDBG_ASSERT(NULL != _frontend);

    if (true == bClear)
    {
        pProperties->clear();
    }

    {
        char    str[32];
        B_Error berror = B_ERROR_SUCCESS;

        nerror = NEXUS_Frontend_RequestOfdmAsyncStatus(getFrontend());
        CHECK_NEXUS_ERROR_GOTO("unable to request ofdm async status", ret, nerror, error);

        /* TTTTTTTTTTTTTTTTTTTTTTTT fine tune this timeout! */
        berror = B_Event_Wait(getStatusEvent(), GET_INT(_pCfg, TUNE_OFDM_STATUS_TIMEOUT));
        CHECK_BOS_WARN_GOTO("unable to request OFDM tuner status", ret, berror, error);

        nerror = NEXUS_Frontend_GetOfdmAsyncStatus(getFrontend(), &nStatus);
        CHECK_NEXUS_ERROR_GOTO("unable to retrieve OFDM tuner status", ret, nerror, error);

        snprintf(str, 32, "%0.2f MHz", nStatus.settings.frequency / 1000000.0);
        pProperties->add("Frequency", str);
        snprintf(str, 32, "%s", modeToString(nStatus.settings.mode).s());
        pProperties->add("Mode", str);
        snprintf(str, 32, "%d MHz", nStatus.settings.bandwidth / 1000000);
        pProperties->add("Bandwidth", str);
        snprintf(str, 32, "%c", nStatus.fecLock ? 'Y' : 'N');
        pProperties->add("FEC Lock", str);
        snprintf(str, 32, "%c", nStatus.receiverLock ? 'Y' : 'N');
        pProperties->add("OFDM Lock", str);
        snprintf(str, 32, "%s", (nStatus.settings.priority == NEXUS_FrontendOfdmPriority_eHigh) ? "High" : "Low");
        pProperties->add("Priority", str);
        snprintf(str, 32, "%s", (nStatus.settings.cciMode == NEXUS_FrontendOfdmCciMode_eAuto) ? "Auto" : "None");
        pProperties->add("CCI Mode", str);
        snprintf(str, 32, "%0.2f dB", nStatus.snr / 100.0);
        pProperties->add("S/N Ratio", str);
        snprintf(str, 32, "%d", nStatus.fecCorrectedBlocks);
        pProperties->add("FEC Corrected", str);
        snprintf(str, 32, "%d", nStatus.fecUncorrectedBlocks);
        pProperties->add("FEC Uncorrected", str);
        snprintf(str, 32, "%d", nStatus.reacquireCount);
        pProperties->add("Reacquisitions", str);
        snprintf(str, 32, "%d dBm", nStatus.signalStrength/10);
        pProperties->add("Signal Strength", str);
    }

error:
    return;
} /* getProperties */

NEXUS_FrontendLockStatus CTunerOfdm::isLocked()
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

NEXUS_FrontendOfdmMode CTunerOfdm::getDefaultMode()
{
    if (true == _capabilities.ofdmModes[NEXUS_FrontendOfdmMode_eIsdbt])
    {
        return(NEXUS_FrontendOfdmMode_eIsdbt);
    }
    else
    if (true == _capabilities.ofdmModes[NEXUS_FrontendOfdmMode_eDvbt])
    {
        return(NEXUS_FrontendOfdmMode_eDvbt);
    }
    else
    if (true == _capabilities.ofdmModes[NEXUS_FrontendOfdmMode_eDvbt2])
    {
        return(NEXUS_FrontendOfdmMode_eDvbt2);
    }
    else
    if (true == _capabilities.ofdmModes[NEXUS_FrontendOfdmMode_eDvbc2])
    {
        return(NEXUS_FrontendOfdmMode_eDvbc2);
    }
    else
    {
        return(NEXUS_FrontendOfdmMode_eMax);
    }
} /* getDefaultMode */

void CTunerOfdm::saveScanData(CTunerScanData * pScanData)
{
    _scanData = *((CTunerOfdmScanData *)pScanData);
}

void CTunerOfdm::doScan()
{
    eRet                       ret            = eRet_Ok;
    unsigned                   major          = 1;
    unsigned                   numFreqToScan  = 0;
    unsigned                   numFreqScanned = 0;
    CChannelOfdm               chOfdm(_pCfg, this); /* temp channel we'll use to do tuning during scan */
    CTunerScanNotificationData notifyData(this);    /* notification data for reporting scan start/stop/progress */
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
        NEXUS_FrontendOfdmSettings settings;

        /* handle default settings for optional paramters */
        settings                 = chOfdm.getSettings();
        settings.frequency       = *pFreq;
        settings.acquisitionMode = NEXUS_FrontendOfdmAcquisitionMode_eAuto;
        settings.terrestrial     = true;
        settings.spectrum        = NEXUS_FrontendOfdmSpectrum_eAuto;
        settings.mode            = (-1 != _scanData._mode) ? (NEXUS_FrontendOfdmMode)_scanData._mode :
                                   getDefaultMode();
        settings.bandwidth = (-1 != _scanData._bandwidth) ? _scanData._bandwidth :
                             6000000;
        chOfdm.setSettings(&settings);
        chOfdm.setTransportType(NEXUS_TransportType_eTs);

        /* try to tune */
        ret = chOfdm.tune(_scanThread_id, _pConfig, true);
        if (eRet_Ok == ret)
        {
            /* tune successful */
#if 0
            THIS CAUSES THE 3461 TUNER TO GET INTO A BAD STATE ... REMOVED FOR NOW
            SINCE IT IS MAINLY DEBUG CODE ANYWAYS

                        nerror = NEXUS_Frontend_GetOfdmStatus(getFrontend(), &status);
            if (eRet_Ok == CHECK_NEXUS_ERROR("Error retrieving OFDM scan status", nerror))
            {
                BDBG_WRN(("Scan Status: "));
                BDBG_WRN(("Mode = %d", status.settings.mode));
                BDBG_WRN(("Spectrum inverted = %s", status.spectrumInverted ? "True" : "False"));
                BDBG_WRN(("Modulation type:%d", status.modulation));
                if (NEXUS_FrontendOfdmMode_eIsdbt == status.settings.mode)
                {
                    BDBG_WRN(("ISDB-T Layer A modulation type:%d", status.modulationA));
                    BDBG_WRN(("ISDB-T Layer B modulation type:%d", status.modulationB));
                    BDBG_WRN(("ISDB-T Layer C modulation type:%d", status.modulationC));
                }
            }
#endif /* if 0 */

            chOfdm.setMajor(major);

            if (0 < chOfdm.addPsiPrograms(_scanThread_callback, _scanThread_context))
            {
                /* found channels so increment major channel number */
                major++;
            }
        }
        else
        {
            /* tune failed */
            BDBG_MSG(("NO Ofdm channel found at freq:%u", chOfdm.getFrequency()));
        }

        chOfdm.unTune(_pConfig, false, false);

        numFreqScanned++;
        notifyData.setPercent(100 * numFreqScanned / numFreqToScan);
        notifyObserversAsync(eNotify_ScanProgress, &notifyData);
    }

error:
    if (chOfdm.isTuned())
    {
        chOfdm.unTune(_pConfig, true);
    }
    scanDone(&notifyData);
    _pWidgetEngine->removeCallback(this, CALLBACK_TUNER_LOCK_STATUS_OFDM);
} /* doScan */

#endif /* NEXUS_HAS_FRONTEND */