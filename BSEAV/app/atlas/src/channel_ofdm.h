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

#ifndef CHANNEL_OFDM_H__
#define CHANNEL_OFDM_H__

#if NEXUS_HAS_FRONTEND

#include "config.h"
#include "channel.h"

class CTunerOfdm;

/*
 *  CChannelOfdm encapulates all the functionality specific to a Ofdm channel.
 *  This includes tune/untune and serializing to/from disk.
 */

class CChannelOfdm : public CChannel
{
public:
    CChannelOfdm(
            CConfiguration * pCfg,
            CTunerOfdm *     pTuner = NULL
            );
    ~CChannelOfdm(void);
    CChannelOfdm(const CChannelOfdm & chOfdm); /* copy constructor */

    virtual eRet          tune(void * id, CConfig * pConfig, bool bWaitForLock, uint16_t index = ANY_INDEX);
    virtual eRet          unTune(CConfig * pResourceLibrary, bool bFullUnTune = false, bool bCheckInTuner = true);
    virtual eRet          readXML(MXmlElement * xmlElemChannel);
    virtual void          writeXML(MXmlElement * xmlElemChannel);
    virtual bool operator ==(CChannel &other);

    eRet                       initialize(PROGRAM_INFO_T * pProgramInfo);
    void                       handleLockCallback(void);
    CChannel *                 createCopy(CChannel * pChannel);
    void                       updateDescription(void);
    eRet                       getChannelInfo(CHANNEL_INFO_T * pChanInfo, bool bScanning);
    uint32_t                   getFrequency(void) { return(_settings.frequency); }
    void                       setFrequency(uint32_t frequency);
    NEXUS_FrontendOfdmMode     getMode(void)                                       { return(_settings.mode); }
    void                       setMode(NEXUS_FrontendOfdmMode mode)                { _settings.mode = mode; }
    NEXUS_FrontendOfdmSettings getSettings(void)                                   { return(_settings); }
    void                       setSettings(NEXUS_FrontendOfdmSettings * pSettings) { if (NULL != pSettings) { _settings = *pSettings; } }
    /* eRet mapInputBand(CInputBand * pInputBand); */

    STRING_TO_ENUM_DECLARE(stringToNexusOfdmMode, NEXUS_FrontendOfdmMode)
    ENUM_TO_MSTRING_DECLARE(nexusOfdmModeToString, NEXUS_FrontendOfdmMode)

protected:
    NEXUS_FrontendOfdmSettings _settings;
};

#endif /* NEXUS_HAS_FRONTEND */
#endif /* CHANNEL_OFDM_H__ */