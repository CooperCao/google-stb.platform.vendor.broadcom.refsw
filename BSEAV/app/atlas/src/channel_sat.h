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

#ifndef CHANNEL_SAT_H__
#define CHANNEL_SAT_H__

#if NEXUS_HAS_FRONTEND

#include "config.h"
#include "channel.h"

class CTunerSat;

/*
 *  CChannelSat encapulates all the functionality specific to a Sat channel.
 *  This includes tune/untune and serializing to/from disk.
 */

typedef  struct satSettings
{
    NEXUS_FrontendSatelliteSettings          sat;
    NEXUS_FrontendDiseqcSettings             diseqc;
    uint8_t                                  diseqcChannel;
    NEXUS_FrontendSatelliteCapabilities      satCapabilities;
    NEXUS_FrontendSatelliteRuntimeSettings   runSettings;
} satSettings;

extern const char * g_nexusSatModes[NEXUS_FrontendSatelliteMode_eMax];

class CChannelSat : public CChannel
{
public:
    CChannelSat(
            CConfiguration * pCfg,
            CTunerSat *      pTuner = NULL
            );
    CChannelSat(const CChannelSat & chSat); /* copy constructor */
    ~CChannelSat(void);
    CChannel * createCopy(CChannel * pChannel);

    virtual eRet tune(void * id, CConfig * pConfig, bool bWaitForLock, uint16_t index = ANY_INDEX);
    virtual eRet unTune(CConfig * pResourceLibrary, bool bFullUnTune = false, bool bCheckInTuner = true);
    virtual eRet readXML(MXmlElement * xmlElemChannel);
    virtual void writeXML(MXmlElement * xmlElemChannel);

    virtual bool operator ==(CChannel &other);

    eRet                        initialize(PROGRAM_INFO_T * pProgramInfo);
    void                        updateDescription(void);
    NEXUS_FrontendSatelliteMode stringToNexusSatMode(const char * mode);
    const char *                nexusSatModeToString(NEXUS_FrontendSatelliteMode mode);
    uint32_t                    getFrequency(void) { return(_settings.sat.frequency); }
    void                        setFrequency(uint32_t frequency);
    NEXUS_FrontendSatelliteMode getMode(void)                             { return(_settings.sat.mode); }
    void                        setMode(NEXUS_FrontendSatelliteMode mode) { _settings.sat.mode = mode; }
    satSettings *               getSettings(void)                         { return(&_settings); }
    void                        setSettings(satSettings * pSettings)      { BDBG_ASSERT(NULL != pSettings); _settings = *pSettings; }

protected:
    satSettings _settings;
};

#endif /* NEXUS_HAS_FRONTEND */
#endif /* CHANNEL_SAT_H__ */