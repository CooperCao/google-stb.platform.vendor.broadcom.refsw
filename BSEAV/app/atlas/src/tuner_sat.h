/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#ifndef TUNER_SAT_H__
#define TUNER_SAT_H__

#include "atlas_cfg.h"
#include "atlas_os.h"
#include "resource.h"
#include "tuner.h"
#include "channel_sat.h"

#ifdef __cplusplus
extern "C" {
#endif

class CTunerSatScanData : public CTunerScanData
{
public:
    CTunerSatScanData(
            bool     appendToChannelList = false,
            unsigned majorStartNum = 1
            ) :
        CTunerScanData(eBoardResource_frontendSds, appendToChannelList, majorStartNum),
        _stepFreq(-1),
        _mode(-1),
        _adc(-1) {}

    /* assignment operator overload */
    CTunerSatScanData & operator =(const CTunerSatScanData & rhs)
    {
        /* call base class '=' operator method */
        CTunerScanData::operator=(rhs);

        _stepFreq = rhs._stepFreq;
        _mode     = rhs._mode;

        return(*this);
    }

    void dump(void);

public:
    /* optional parameters (-1 if unspecified) */
    int _stepFreq;
    int _mode;
    int _adc;
};

class CTunerSat : public CTuner
{
public:
    CTunerSat(
            const char *     name,
            const unsigned   number,
            CConfiguration * pCfg
            );

    virtual eRet       initCapabilities(void);
    virtual void       scanDataSave(CTunerScanData * pScanData);
    virtual void       scanDataDump(void)          { _scanData.dump(); }
    virtual void       scanDataFreqListClear(void) { _scanData._freqList.clear(); }
    virtual eRet       scanDataFreqListAdd(uint32_t freq);
    virtual void       doScan(void);
    virtual eRet       open(CWidgetEngine * pWidgetEngine);
    virtual void       close(void);
    virtual void       handleLockCallback(void);
    virtual CChannel * createChannel(void);
    virtual void       destroyChannel(CChannel * pChannel);

    eRet                     tune(satSettings & settings);
    void                     unTune(bool bFullUnTune = false);
    void                     getProperties(MStringHash * pProperties, bool bClear);
    bool                     isDiseqcSupported(void) { return(_bDiseqc); }
    NEXUS_FrontendLockStatus isLocked(void);

    BKNI_EventHandle _scanThread_psCallbackEvent;
    NEXUS_FrontendSatellitePeakscanResult _scanThread_psResult;

protected:
    ENUM_TO_MSTRING_DECLARE(modeToString, NEXUS_FrontendSatelliteMode)
    ENUM_TO_MSTRING_DECLARE(spectralInvToString, NEXUS_FrontendSatelliteInversion)

protected:
    CTunerSatScanData _scanData;
    bool              _bDiseqc;
};

#ifdef __cplusplus
}
#endif

#endif /* ifndef TUNER_SAT_H__ */