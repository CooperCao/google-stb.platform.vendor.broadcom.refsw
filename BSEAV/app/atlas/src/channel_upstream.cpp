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

#include "channel_upstream.h"
#include "channelmgr.h"
#include "nexus_core_utils.h"
#include "tuner_upstream.h"

BDBG_MODULE(channel_upstream);

CChannelUpstream::CChannelUpstream(
        CConfiguration * pCfg,
        CTunerUpstream * pTuner
        ) :
    CChannel("CChannelUpstream", eBoardResource_frontendUpstream, pCfg)
{
    setTunerRequired(true);

#if NEXUS_HAS_FRONTEND
    setTuner(pTuner);
#endif
    NEXUS_Frontend_GetDefaultUpstreamSettings(&_settings);
}

CChannelUpstream::CChannelUpstream(const CChannelUpstream & chUpstream) :
    CChannel(chUpstream),
    _settings(chUpstream._settings)
{
    /* copy constructor will not copy any unique objects like tuner */
}

CChannel * CChannelUpstream::createCopy(CChannel * pChannel)
{
    CChannelUpstream * pChNew = NULL;

    pChNew = new CChannelUpstream(*(CChannelUpstream *)pChannel);
    return(pChNew);
}

CChannelUpstream::~CChannelUpstream()
{
}

eRet CChannelUpstream::initialize(PROGRAM_INFO_T * pProgramInfo)
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NULL != pProgramInfo);

    setProgramNum(pProgramInfo->program_number);
    _pidMgr.clearPids();
    ret = _pidMgr.createPids(pProgramInfo);
    CHECK_ERROR_GOTO("unable to create pids from program info", ret, error);

error:
    return(ret);
}

eRet CChannelUpstream::tune(
        void *    id,
        CConfig * pConfig,
        bool      bWaitForLock,
        uint16_t  index
        )
{
    eRet                         ret    = eRet_Ok;
    NEXUS_Error                  nerror = NEXUS_SUCCESS;
    NEXUS_FrontendUserParameters userParams;
    CInputBand *                 pInputBand          = NULL;
    CParserBand *                pParserBand         = NULL;
    CTunerUpstream *             pTuner              = NULL;
    CBoardResources *            pBoardResources     = NULL;
    CConfiguration *             pCfg                = NULL;
    bool                         bLocalTunerCheckout = false;

    BDBG_ASSERT(NULL != pConfig);
    pBoardResources = pConfig->getBoardResources();
    pCfg            = pConfig->getCfg();

    pTuner = (CTunerUpstream *)getTuner();
    if ((NULL != pTuner) && (index != pTuner->getNumber()) && (ANY_INDEX != index))
    {
        /* given tuner index does not match currently checked out tuner so check in */
        pTuner->release();
        pTuner->close();
        pBoardResources->checkinResource(pTuner);
        pTuner = NULL;
    }

    if (NULL == pTuner)
    {
        pTuner = (CTunerUpstream *)pBoardResources->checkoutResource(id, eBoardResource_frontendUpstream, index);
        CHECK_PTR_ERROR_GOTO("unable to checkout Upstream tuner", pTuner, ret, eRet_NotAvailable, done);

        ret = pTuner->open(_pWidgetEngine);
        CHECK_ERROR_GOTO("unable to open Upstream tuner", ret, done);

        ret = pTuner->acquire();
        CHECK_ERROR_GOTO("unable to acquire Upstream tuner", ret, done);

        pTuner->setConfig(pConfig);
        setTuner(pTuner);
        bLocalTunerCheckout = true;
    }

    nerror = NEXUS_Frontend_GetUserParameters(_pTuner->getFrontend(), &userParams);
    CHECK_NEXUS_ERROR_GOTO("unable to get frontend user parameters", ret, nerror, done);

    /* checkout input band based on frontend user parameters (set by nexus platform) */
    pInputBand = (CInputBand *)pBoardResources->checkoutResource(id, eBoardResource_inputBand, userParams.param1);
    CHECK_PTR_ERROR_GOTO("unable to checkout inputband", pInputBand, ret, eRet_NotAvailable, error);
    setInputBand(pInputBand);

    pParserBand = (CParserBand *)pBoardResources->checkoutResource(id, eBoardResource_parserBand);
    CHECK_PTR_ERROR_GOTO("unable to checkout parser band", pParserBand, ret, eRet_NotAvailable, error);
    pParserBand->open();
    setParserBand(pParserBand);

    ret = mapInputBand(pInputBand);
    CHECK_ERROR_GOTO("error mapping input band to parser band", ret, error);

    BDBG_MSG(("CH NUMBER: %d.%d", getMajor(), getMinor()));
    BDBG_MSG(("mode:%d freq:%u symbolRate:%u",
              _settings.mode, _settings.frequency, _settings.symbolRate));

    ret = pTuner->tune(_settings);
    CHECK_ERROR_GOTO("tuning error!", ret, error);

    _tuned = true;

    if (true == bWaitForLock)
    {
        B_Error berror  = B_ERROR_SUCCESS;
        int     timeout = GET_INT(pCfg, TUNE_QAM_TIMEOUT);

        berror = B_Event_Wait(_pTuner->getWaitEvent(), timeout);
        CHECK_BOS_WARN_GOTO("tune failed to lock", ret, berror, error);

        if ((false == pTuner->getLockState()) && (false == pTuner->isAcquireInProgress()))
        {
            /* wait event could have been set if unlocked but tuner is done searching
             * so we will indicate a failed tune in the return code */
            ret = eRet_NotAvailable;
        }

        pTuner->resetStatus();
    }

error:
    if (eRet_Ok != ret)
    {
        if ((NULL != pTuner) && (true == bLocalTunerCheckout))
        {
            pBoardResources->checkinResource(pTuner);
            setTuner(NULL);
        }

        if (NULL != pParserBand)
        {
            pBoardResources->checkinResource(pParserBand);
            pParserBand = NULL;
        }
        if (NULL != pInputBand)
        {
            pBoardResources->checkinResource(pInputBand);
            pInputBand = NULL;
        }
    }

done:
    _tuned = (eRet_Ok == ret) ? true : false;
    return(ret);
} /* tune */

eRet CChannelUpstream::unTune(
        CConfig * pConfig,
        bool      bFullUnTune,
        bool      bCheckInTuner
        )
{
    eRet              ret             = eRet_Ok;
    CBoardResources * pBoardResources = NULL;

    BDBG_ASSERT(pConfig);
    pBoardResources = pConfig->getBoardResources();

    if ((NULL != _pParserBand) && (_pParserBand->isCheckedOut()))
    {
        _pParserBand->close();
        pBoardResources->checkinResource(_pParserBand);
        setParserBand(NULL);
    }

    if ((NULL != _pInputBand) && (_pInputBand->isCheckedOut()))
    {
        pBoardResources->checkinResource(_pInputBand);
        setInputBand(NULL);
    }

    if (NULL != _pTuner)
    {
        _pTuner->unTune(bFullUnTune);

        if ((true == bCheckInTuner) && (true == _pTuner->isCheckedOut()))
        {
            _pTuner->release();
            _pTuner->close();
            pBoardResources->checkinResource(_pTuner);
            setTuner(NULL);
        }
    }

    _tuned = false;

    return(ret);
} /* unTune */

bool CChannelUpstream::operator ==(CChannel &other)
{
    CChannelUpstream * pOtherUpstream = (CChannelUpstream *)&other;

    /* check base class equivalency first */
    if (true == CChannel::operator ==(other))
    {
        if ((pOtherUpstream->getFrequency() == getFrequency()) &&
            (pOtherUpstream->getMode() == getMode()))
        {
            return(true);
        }
    }

    return(false);
}

#endif /* NEXUS_HAS_FRONTEND */