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

#ifndef CHANNELMGR_H__
#define CHANNELMGR_H__

#include "atlas_os.h"
#include "config.h"
#include "mvc.h"
#include "pidmgr.h"
#include "mlist.h"
#include "mxmlparser.h"
#include "mstring.h"
#include "xmltags.h"
#ifdef MPOD_SUPPORT
#include "cablecard.h"
#include "channel_oob.h"
#include "si.h"
#include "si_main.h"
#include "si_api_chanmap.h"
#include "si_api_ea.h"
#include "si_os.h"
#include "tuner_qam.h"
#endif /* ifdef MPOD_SUPPORT */

#ifdef __cplusplus
extern "C" {
#endif

#define CHMGR_FILTER_PASS            0
#define CHMGR_FILTER_FAIL            1
#define CHMGR_FILTER_FAIL_ENCRYPTED  2
#define CHMGR_FILTER_FAIL_PCR        4
#define CHMGR_FILTER_FAIL_AUDIOONLY  8
#define CHMGR_FILTER_FAIL_DUPLICATE  16
#define CHMGR_FILTER_FAIL_ANCILONLY  32

#ifdef MPOD_SUPPORT
#define MAX_MPEG_SECTION_LENGTH      4096
#endif
class CChannel;

class CChannelMgrLoadSaveData
{
public:
    CChannelMgrLoadSaveData(
            const char * strFileName = "channels.xml",
            const char * strListName = "default",
            bool         append = false
            ) :
        _strFileName(strFileName),
        _strListName(strListName),
        _append(append) {}

public:
    MString _strFileName;
    MString _strListName;
    bool    _append;
};

class CChannelMgrListChangedData
{
public:
    CChannelMgrListChangedData(
            CChannel * pChannel,
            bool       bAdd
            ) :
        _pChannel(pChannel),
        _bAdd(bAdd) {}

    CChannel * getChannel(void) { return(_pChannel); }
    bool       isAdd(void)      { return(_bAdd); }

public:
    CChannel * _pChannel;
    bool       _bAdd;
};

class CChannelMgr : public CMvcModel
{
public:
    CChannelMgr(void);
    ~CChannelMgr(void);

    virtual eRet registerObserver(CObserver * observer, eNotification notification = eNotify_All);

    void       setCfg(CConfiguration * pCfg) { _pCfg = pCfg; }
    void       setModel(CModel * pModel)     { _pModel = pModel; }
    eRet       loadChannelList(const char * listName, const bool append = false);
    eRet       saveChannelList(const char * fileName, const bool append = false);
    eRet       verifyChannelListFile(const char * fileName);
    eRet       clearChannelList(void);
    void       sortChannelList(void);
    eRet       addChannel(CChannel * pChannel);
    eRet       removeOtherMajorChannels(CChannel * pChannel);
    CChannel * findChannel(const char * strChannelNum, eWindowType windowType = eWindowType_Max);
    uint32_t   filterChannel(CChannel * pChannel);
    eRet       addChannelList(MList<CChannel> * pList, bool append = true);
    CChannel * findTunedChannel(void);
    CChannel * findTunedChannel(eBoardResource tunerType);
    void       dumpChannelList(bool bForce = false);
    CChannel * getNextChannel(CChannel * pChannel = NULL, bool bWrap = true);
    CChannel * getPrevChannel(CChannel * pChannel = NULL, bool bWrap = true);
    CChannel * getFirstChannel(eWindowType windowType = eWindowType_Max);
    CChannel * getLastChannel(eWindowType windowType = eWindowType_Max);
    bool       isDuplicate(CChannel * pChannel);
#if NEXUS_HAS_FRONTEND
    CTuner * checkoutTuner(void);
#endif
#if B_HAS_DTCP_IP
    void enableDtcp(void);
    void disableDtcp(void);
#endif
#ifdef MPOD_SUPPORT
    void                initialize(void);
    void                unInitialize(void);
    static void *       mpegSectionHandler(void * ctx);
    int                 updateScteMap();
    void                printChannelMap(channel_list_t * list);
    static void         sttCallback(unsigned long t, bool dst);
    static void         easCallback(EA_MSG_INFO * p_ea_msg);
    static void         scteEventCallback(void * context, int param);
    CWidgetEngine *     getWidgetEngine(void)                          { return(_pWidgetEngine); }
    void                setWidgetEngine(CWidgetEngine * pWidgetEngine) { _pWidgetEngine = pWidgetEngine; }
    CTunerQamScanData * getTunerScanData()                             { return(&_scanData);    }
#endif /* ifdef MPOD_SUPPORT */

protected:
    MAutoList <CChannel> * getChannelList(eWindowType windowType = eWindowType_Main)
    {
        eWindowType winType = windowType;

        if (eWindowType_Max == winType)
        {
            if (NULL == _pModel)
            {
                /* no model set so default to main */
                winType = eWindowType_Main;
            }
            else
            {
                /* model set so determine which window is fullscreen (main or pip) */
                winType = _pModel->getFullScreenWindowType();
            }
        }

        return(&_channelList[winType]);
    } /* getChannelList */

protected:
    CConfiguration *     _pCfg;
    CModel *             _pModel;
    MAutoList <CChannel> _channelList[eWindowType_Max];
    B_MutexHandle        _mutex;

#ifdef MPOD_SUPPORT
    CWidgetEngine *   _pWidgetEngine;
    pthread_t         mpegSectionThread;
    bool              mpegSectionThreadExit;
    unsigned char *   siBuffer;
    unsigned int      siLen;
    channel_list_t    channelList;
    bool              clearChannelMap;
    CTunerQamScanData _scanData;
#endif /* ifdef MPOD_SUPPORT */
};

#ifdef __cplusplus
}
#endif

#endif /* CHANNELMGR_H__ */