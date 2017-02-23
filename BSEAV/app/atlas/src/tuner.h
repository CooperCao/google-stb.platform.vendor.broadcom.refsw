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

#ifndef TUNER_H__
#define TUNER_H__

#include "atlas_cfg.h"
#include "atlas_os.h"
#include "resource.h"
#include "bwidgets.h"
#include "widget_engine.h"
#ifdef MPOD_SUPPORT
#include "si.h"
#include "si_main.h"
#include "si_api_chanmap.h"
#endif /* ifdef MPOD_SUPPORT */

#include "tspsimgr2.h"

#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#endif

#ifdef SNMP_SUPPORT
#include "snmp_tuner.h"
#endif

class CChannel;

/* returns true if channel was added, false if channel was ignored */
typedef bool (* CTunerScanCallback)(CChannel * pChannel, void * context);

typedef void (* CTunerLockCallback)(void * context, int data);

#if NEXUS_HAS_FRONTEND

class CConfig;
class CTuner;
class CParserBand;

/* notification data sent during CTuner based notifyObservers() calls */
class CTunerScanNotificationData
{
public:
    CTunerScanNotificationData(
            CTuner * pTuner = NULL,
            uint8_t  percent = 0
            ) :
        _percent(percent),
        _pTuner(pTuner),
        _appendToChannelList(false) {}

    uint8_t  getPercent(void)                    { return(_percent); }
    void     setPercent(uint8_t percent)         { _percent = percent; }
    CTuner * getTuner(void)                      { return(_pTuner); }
    void     setTuner(CTuner * pTuner)           { _pTuner = pTuner; }
    bool     getAppendToChannelList(void)        { return(_appendToChannelList); }
    void     setAppendToChannelList(bool append) { _appendToChannelList = append; }

protected:
    uint8_t  _percent;
    CTuner * _pTuner;
    bool     _appendToChannelList;
};

/* Tuner scan events are stored temporarily until the Atlas main loop has a chance to service them.
 * This is done because the scan operates in a thread.  Data associated with the Tuner event is allocated
 * when the scan notification is sent, and freed after the associated action is completed.  CTunerEvent base
 * class allows us to reference all tuner events using the base class, while the template derived class
 * ensures that different _data types are properly deleted when no longer needed.
 */
class CTunerEvent
{
public:
    CTunerEvent(eNotification id) :
        _id(id) {}

    virtual ~CTunerEvent() {}

    eNotification  getId(void)      { return(_id); }
    virtual void * getDataPtr(void) { return(NULL); }

protected:
    eNotification _id;
};

template<class T>
class CTunerDataEvent : public CTunerEvent
{
public:
    CTunerDataEvent(
            eNotification id,
            T *           data
            ) :
        CTunerEvent(id),
        _data(data) {}

    virtual ~CTunerDataEvent(void) { if (NULL != _data) { delete _data; } }

    void * getDataPtr(void) { return((void *)_data); }

protected:
    T * _data;
};

class CTunerScanData
{
public:
    CTunerScanData(
            eBoardResource tunerType,
            bool           appendToChannelList = false,
            uint16_t       majorStartNum = 1
            ) :
        _tunerType(tunerType),
        _appendToChannelList(appendToChannelList),
        _majorStartNum(majorStartNum) {}

    void           setTunerType(eBoardResource tunerType) { _tunerType = tunerType; }
    eBoardResource getTunerType(void)                     { return(_tunerType); }

    /* assignment operator overload to manually copy internal MAutoList */
    CTunerScanData & operator =(const CTunerScanData & rhs)
    {
        MListItr <uint32_t> itr(&rhs._freqList);
        uint32_t *          pFreq = NULL;

        _tunerType           = rhs._tunerType;
        _appendToChannelList = rhs._appendToChannelList;
        _majorStartNum       = rhs._majorStartNum;
#ifdef MPOD_SUPPORT
        _pChannelList       = rhs._pChannelList;
        _useCableCardSiData = rhs._useCableCardSiData;
#endif
        /* copy frequency autolist manually */
        _freqList.clear();
        for (pFreq = (uint32_t *)itr.first(); pFreq; pFreq = (uint32_t *)itr.next())
        {
            _freqList.add(new uint32_t(*pFreq));
        }

        return(*this);
    } /* = */

protected:
    eBoardResource _tunerType;
public:
    MAutoList <uint32_t> _freqList;
    bool                 _appendToChannelList;
    uint16_t             _majorStartNum;
#ifdef MPOD_SUPPORT
    channel_list_t * _pChannelList;
    bool             _useCableCardSiData;
#endif
};

class CTuner : public CResource
{
public:
    CTuner(
            const char *     name,
            const uint16_t   number,
            eBoardResource   resource,
            CConfiguration * pCfg
            );
    virtual ~CTuner(void);

    virtual void                     saveScanData(CTunerScanData * pScanData)                     = 0; /* requires implementation by sub class to save tuner specific scan data */
    virtual void                     doScan(void)                                                 = 0; /* requires implementation by sub class to do actual scanning */
    virtual NEXUS_FrontendLockStatus isLocked(void)                                               = 0; /* requires implementation by sub class to determine lock status */
    virtual void                     unTune(bool bFullUnTune = false)                             = 0; /* requires implementation by sub class to untune */
    virtual void                     getProperties(MStringHash * pProperties, bool bClear = true) = 0; /* requires implementation by sub class to get tuner properites */
    virtual eRet                     open(CWidgetEngine * pWidgetEngine);
    virtual void                     close(void);
    virtual eRet                     initCapabilities(void) { return(eRet_Ok); }
    virtual eRet                     acquire(void);
    virtual void                     release(void);
    virtual eRet                     scan(void * id, CTunerScanData * pScanData, CTunerScanCallback callback, void * context);
    virtual void                     scanDone(CTunerScanNotificationData * pNotifyData);
    virtual void                     handleLockCallback(void) {}
    virtual void                     dump(void);

    void setLockCallback(
            CTunerLockCallback tunerLockCallback,
            void *             context,
            int                data
            ) { _tunerLockCallback = tunerLockCallback; _tunerLockCallbackContext = context; _tunerLockCallbackData = data; }
    void                          setConfig(CConfig * pConfig) { _pConfig = pConfig; }
    CConfig *                     getConfig(void)              { return(_pConfig); }
    eRet                          getSoftDecisions(NEXUS_FrontendSoftDecision * pSoftDecisions, int length);
    B_EventHandle                 getStatusEvent(void) { return(_statusEvent); }
    B_EventHandle                 getLockEvent(void)   { return(_lockEvent); }
    B_EventHandle                 getWaitEvent(void)   { return(_waitEvent); }
    void                          setLockState(bool locked);
    bool                          getLockState(void)         { return(_locked); }
    void                          resetStatus(void)          { if (NULL != _frontend) { NEXUS_Frontend_ResetStatus(_frontend); } }
    NEXUS_FrontendHandle          getFrontend(void)          { return(_frontend); }
    NEXUS_FrontendConnectorHandle getFrontendConnector(void) { return(NEXUS_Frontend_GetConnector(_frontend)); }
    MString                       getFrontendId(void);
    void                          addEvent(CTunerEvent * pEvent);
    CTunerEvent *                 getEvent(void);
    void                          flushEvents();
    CTunerEvent *                 removeEvent(void);
    bool                          isAcquireInProgress(void) { return(_acquireInProgress); }
    void                          destroyThreadScan(void);

    /* replace CSubject::notifyObservers() with one specifically for scan that will
     * synchronize with the bwin main loop (since scan runs in a thread). */
    eRet notifyObserversAsync(eNotification id, CTunerScanNotificationData * data);

    CWidgetEngine * getWidgetEngine(void) { return(_pWidgetEngine); }

#ifdef SNMP_SUPPORT
    void         getStatus(tuner_status * status)            { BKNI_Memcpy(status, &_tuner_status, sizeof(tuner_status)); }
    lock_state   getState(void)                              { return(_tuner_status.state); }
    void         setState(lock_state state)                  { _tuner_status.state = state; }
    clock_t      getLockedTime(void)                         { return(_tuner_status.locked_time); }
    void         setLockedTime(clock_t locked_time)          { _tuner_status.locked_time = locked_time; }
    unsigned int getLastFrequency(void)                      { return(_tuner_status.last_freq); }
    void         setLastFrequency(unsigned int frequency)    { _tuner_status.last_freq = frequency; }
    void         setFailureFrequency(unsigned int frequency) { _tuner_status.failure_freq = frequency; }
    void         incAcquireCount(void)                       { _tuner_status.acquire_count++; }
    void         incUnlockedCount(void)                      { _tuner_status.unlock_count++; }
    void         incFailureCount(void)                       { _tuner_status.failure_count++; }
#endif /* ifdef SNMP_SUPPORT */

protected:
    void trigger(CTunerEvent * pTunerEvent);

protected:
    CConfig *                  _pConfig;
    bool                       _locked;
    bool                       _acquireInProgress;
    MList <CTunerEvent>        _eventList;
    B_MutexHandle              _eventMutex;
    CWidgetEngine *            _pWidgetEngine;
    NEXUS_FrontendCapabilities _capabilities;
    NEXUS_FrontendHandle       _frontend;
    B_EventHandle              _waitEvent;
    B_EventHandle              _statusEvent;
    B_EventHandle              _lockEvent;
    B_SchedulerEventId         _lockEventId;
    B_ThreadHandle             _scanThread_handle;
    MString                    _scanThread_name;
    void *                     _scanThread_id;
    CTunerScanCallback         _scanThread_callback;
    void *                     _scanThread_context;
    CTunerLockCallback         _tunerLockCallback;
    void *                     _tunerLockCallbackContext;
    int _tunerLockCallbackData;
#ifdef SNMP_SUPPORT
    tuner_status _tuner_status;
#endif
};

#endif /* NEXUS_HAS_FRONTEND */
#endif /* TUNER_H__ */
