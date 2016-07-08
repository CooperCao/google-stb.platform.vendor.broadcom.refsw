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

#include "channel_upstream.h"
#include "tuner_upstream.h"
#include "ts_psi.h"
#include "ts_pat.h"

#include "nexus_frontend.h"
#include "nexus_frontend_upstream.h"

BDBG_MODULE(atlas_tuner_upstream);

#ifdef USED /* Commented out to fix warning, enable it again if needed in future. */
static void tunerLockCallback(
        void * context,
        int    param
        )
{
    CTuner * pTuner = (CTuner *)context;

    BSTD_UNUSED(param);
    BDBG_ASSERT(NULL != pTuner);

    B_Event_Set(pTuner->getLockEvent());
}

#endif /* ifdef USED */
void CTunerUpstreamScanData::dump()
{
    MListItr <uint32_t> itr(&_freqList);
    uint32_t *          pFreq = NULL;

    BDBG_WRN(("tuner type:%d", getTunerType()));
    BDBG_WRN(("List of UPSTREAM Scan Frequency Requests:"));
    for (pFreq = (uint32_t *)itr.first(); pFreq; pFreq = (uint32_t *)itr.next())
    {
        BDBG_WRN(("freq:%u", *pFreq));
    }

    BDBG_WRN(("append to channel list:%d", _appendToChannelList));
}

CTunerUpstream::CTunerUpstream(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    CTuner(name, number, eBoardResource_frontendUpstream, pCfg)
{
    _scanThread_name = "CTunerUpstream_Scan";
}

eRet CTunerUpstream::tune(NEXUS_FrontendUpstreamSettings settings)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

#if 0
    settings.lockCallback.callback = tunerLockCallback;
    settings.lockCallback.context  = this;

    NEXUS_StartCallbacks(_frontend);
    B_Event_Reset(_lockEvent);
    B_Event_Reset(_waitEvent);
#endif /* if 0 */

    nerror = NEXUS_Frontend_TuneUpstream(_frontend, &settings);
    CHECK_NEXUS_ERROR_GOTO("unable to tune Upstream frontend", ret, nerror, error);

error:
    if (NEXUS_NOT_SUPPORTED == nerror)
    {
        ret = eRet_NotSupported;
    }
    return(ret);
} /* tune */

void CTunerUpstream::unTune(bool bFullUnTune)
{
    BSTD_UNUSED(bFullUnTune);
    NEXUS_Frontend_Untune(_frontend);
}

void CTunerUpstream::getProperties(
        MStringHash * pProperties,
        bool          bClear
        )
{
    BSTD_UNUSED(pProperties);
    BSTD_UNUSED(bClear);
}

NEXUS_FrontendLockStatus CTunerUpstream::isLocked()
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

NEXUS_FrontendUpstreamMode CTunerUpstream::getDefaultMode()
{
    if (true == _capabilities.upstreamModes[NEXUS_FrontendUpstreamMode_ePodDvs178])
    {
        return(NEXUS_FrontendUpstreamMode_ePodDvs178);
    }
    else
    {
        return(NEXUS_FrontendUpstreamMode_ePodDvs167);
    }
}

void CTunerUpstream::saveScanData(CTunerScanData * pScanData)
{
    _scanData = (*((CTunerUpstreamScanData *)pScanData));
}

void CTunerUpstream::doScan()
{
    BDBG_WRN(("CTunerUpstream::doScan not supported!"));
}

#endif /* NEXUS_HAS_FRONTEND */