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

#ifndef CHANNEL_UPSTREAM_H__
#define CHANNEL_UPSTREAM_H__

#if NEXUS_HAS_FRONTEND

#include "config.h"
#include "channel.h"

class CTunerUpstream;

/*
 *  CChannelUpstream encapulates all the functionality specific to an Upstream channel.
 *  This includes tune/untune and serializing to/from disk.
 */
extern const char * g_nexusUpstreamModes[NEXUS_FrontendUpstreamMode_eMax];

class CChannelUpstream : public CChannel
{
public:
    CChannelUpstream(
            CConfiguration * pCfg,
            CTunerUpstream * pTuner = NULL
            );
    CChannelUpstream(const CChannelUpstream & chUpstream); /* copy constructor */
    ~CChannelUpstream(void);
    CChannel * createCopy(CChannel * pChannel);

    virtual eRet tune(void * id, CConfig * pConfig, bool bWaitForLock, uint16_t index = ANY_INDEX);
    virtual eRet unTune(CConfig * pResourceLibrary, bool bFullUnTune = false, bool bCheckInTuner = true);

    virtual bool operator ==(CChannel &other);

    eRet                           initialize(PROGRAM_INFO_T * pProgramInfo);
    uint32_t                       getFrequency(void)                                   { return(_settings.frequency); }
    void                           setFrequency(uint32_t frequency)                     { _settings.frequency = frequency; }
    NEXUS_FrontendUpstreamMode     getMode(void)                                        { return(_settings.mode); }
    void                           setMode(NEXUS_FrontendUpstreamMode mode)             { _settings.mode = mode; }
    NEXUS_FrontendUpstreamSettings getSettings(void)                                    { return(_settings); }
    void                           setSettings(NEXUS_FrontendUpstreamSettings settings) { _settings = settings; }

protected:
    NEXUS_FrontendUpstreamSettings _settings;
};

#endif /* NEXUS_HAS_FRONTEND */
#endif /* CHANNEL_UPSTREAM_H__ */