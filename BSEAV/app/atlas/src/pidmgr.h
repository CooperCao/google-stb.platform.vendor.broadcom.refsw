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

#ifndef PIDMGR_H__
#define PIDMGR_H__

#include "nexus_types.h"

#include "atlas_cfg.h"
#include "pid.h"
#include "mvc.h"
#include "mlist.h"
#include "tspsimgr2.h"

class CInputBand;
class CParserBand;

#ifdef __cplusplus
extern "C" {
#endif

class CPidMgr : public CMvcModel
{
public:
    CPidMgr(void);
    CPidMgr(const CPidMgr & src); /* copy constructor */
    ~CPidMgr(void);

    virtual bool operator ==(CPidMgr &other);
    virtual void operator =(CPidMgr &other);

    eRet                initialize(CConfiguration * pCfg);
    eRet                mapInputBand(CInputBand * pInputBand); /* move */
    eRet                addPid(CPid * pPid);
    eRet                removePid(CPid * pPid);
    CPid *              getPid(unsigned index, ePidType type);
    CPid *              findPid(unsigned pidNum, ePidType type);
    eRet                createPids(PROGRAM_INFO_T * pProgramInfo);
    void                clearPids(void);
    void                clearPcrPid(void);
    void                setPcrPid(CPid * pid)                      { _pPcrPid = pid; }
    void                setTransportType(NEXUS_TransportType type) { _transportType = type; }      /* move */
    NEXUS_TransportType getTransportType(void)                     { return(_transportType); }     /* move */
    void                setParserBand(CParserBand * pParserBand)   { _pParserBand = pParserBand; } /* move */
    CParserBand *       getParserBand(void)                        { return(_pParserBand); }       /* move */
    void                readXML(MXmlElement * xmlElem);
    MXmlElement *       writeXML(MXmlElement * xmlElem);
    unsigned            getCaPid(void)             { return(_caPid); }
    void                setCaPid(unsigned caPid)   { _caPid = caPid; }
    unsigned            getPmtPid(void)            { return(_pmtPid); }
    void                setPmtPid(unsigned pmtPid) { _pmtPid = pmtPid; }
    void                closePidChannels(CPlaypump * pPlaypump);
    void                closePidChannels(void);
    void                closePlaybackPids(CPlayback * pPlayback);
    void                closePlaybackPidChannels(CPlayback * pPlayback);
    void                closeRecordPids(CRecord * pRecord);
    void                closeRecordPidChannels(CRecord * pRecord);
    bool                isEncrypted(void);
    bool                isEmpty(void);
    unsigned            getProgram(void)             { return(_program); }
    void                setProgram(unsigned program) { _program = program; }
    bool                isImmutable(void)            { return(_bImmutable); }
    void                print(bool bForce = false);
    void                dump(bool bForce = false) { print(bForce); }

protected:
    CParserBand *       _pParserBand;
    MAutoList <CPid>    _videoPidList;
    MAutoList <CPid>    _audioPidList;
    MAutoList <CPid>    _ancillaryPidList;
    CPid *              _pPcrPid;
    NEXUS_TransportType _transportType;
    unsigned            _caPid;
    unsigned            _pmtPid;
    unsigned            _program;
    CConfiguration *    _pCfg;
    bool                _bImmutable;
};

#ifdef __cplusplus
}
#endif

#endif /*PIDMGR_H__ */