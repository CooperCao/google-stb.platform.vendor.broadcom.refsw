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

#ifndef RECORD_H__
#define RECORD_H__

#include "nexus_types.h"
#include "nexus_record.h"
#include "nexus_recpump.h"
#include "nexus_file.h"

#include "resource.h"
#include "pid.h"
#include "mvc.h"
#include "pidmgr.h"
#include "playback.h"
#include "videolist.h"
#include "mlist.h"
#include "tspsimgr2.h"
#include "mstring.h"
#include "mstringlist.h"
#include "mxmlelement.h"
#include "bmedia_probe.h"

#ifdef __cplusplus
extern "C" {
#endif

class CControl;
class CVideo;
class CParserBand;
class CBoardResources;

class CRecpump : public CResource
{
public:

    CRecpump(
            const char *     name,
            const unsigned   number,
            CConfiguration * pCfg
            );
    ~CRecpump(void);

    eRet                  initialize();
    eRet                  open();
    eRet                  start();
    eRet                  stop();
    void                  close();
    NEXUS_RecpumpHandle   getRecpump(void) { return(_recpump); }
    eRet                  setTpitFilter(NEXUS_PidChannelHandle pidChannel);
    NEXUS_RecpumpSettings getSettings(void) { return(_recpumpSettings); }
    eRet                  setSettings(NEXUS_RecpumpSettings * pRecpumpSettings);
    bool                  isAllocated(void) { return(_allocated); }

protected:
    FILE *                _streamOut;
    FILE *                _indexOut;
    FILE *                _navOut;
    NEXUS_RecpumpHandle   _recpump;
    NEXUS_RecpumpSettings _recpumpSettings;
    bool                  _allocated;
};

class CRecordData
{
public:
    CRecordData(
            const char * strFileName = NULL,
            const char * strPath = NULL,
            const char * strIndexName = NULL,
            const char * strIndexPath = NULL,
            CVideo *     video = NULL
            ) :
        _strFileName(strFileName),
        _strPath(strPath),
        _strIndexName(strIndexName),
        _strIndexPath(strIndexPath),
        _video(video),
        _security("none")
    {}

public:
    MString  _strFileName;
    MString  _strPath;
    MString  _strIndexName;
    MString  _strIndexPath;
    CVideo * _video;
    MString  _security;
};

class CRecord : public CResource
{
public:

    CRecord(
            const char *     name,
            const unsigned   number,
            CConfiguration * pCfg
            );
    ~CRecord(void);

    eRet                 open();
    eRet                 start();
    eRet                 stop();
    void                 close();
    void                 dupPidMgr(CPidMgr * pPidMgr);
    CPidMgr *            getPidMgr(void) { return(_pidMgr); }
    eRet                 parseInfo(const char * filename);
    void                 closePids(void);
    NEXUS_RecordHandle   getRecord(void) { return(_record); }
    NEXUS_RecordSettings getSettings(void);
    void                 setSettings(NEXUS_RecordSettings * pRecordSettings);
    void                 setVideo(CVideo * video);
    CVideo *             getVideo(void) { return(_currentVideo); }
    CPid *               getPid(unsigned index, ePidType type);
    CPid *               findPid(unsigned pidNum, ePidType type);
    CRecpump *           getRecpump(void)                   { return(_pRecpump); }
    CParserBand *        getBand(void)                      { return(_pParserBand); }
    void                 setBand(CParserBand * pParserBand) { _pParserBand = pParserBand; }
    eRet                 createVideo(MString fileName, MString path);
    bool                 hasIndex() { return(_currentVideo->hasIndexName()); }
    bool                 isActive(void);
    void                 dump(void);
    bool                 isAllocated(void) { return(_allocated); }
    void                 setResources(
            void *            id,
            CBoardResources * pResources
            )
    { BSTD_UNUSED(id); _pBoardResources = pResources; }
    void setEncryptionKey(unsigned long key[4]);

protected:
    NEXUS_FileRecordHandle _file;
    NEXUS_FileRecordHandle _customFile;
    NEXUS_FileRecordHandle _stickyFile;
    CRecpump *             _pRecpump;
    NEXUS_RecordHandle     _record;
    NEXUS_RecordSettings   _recordSettings;
    CParserBand *          _pParserBand;
    bool                   _allocated;
    CVideo *               _currentVideo;
    CBoardResources *      _pBoardResources;
    CPidMgr *              _pidMgr;
#if (defined NEXUS_HAS_DMA || defined NEXUS_HAS_XPT_DMA)
    CDma * _pDma;
#endif
};

#ifdef __cplusplus
}
#endif

#endif /* RECORD_H__ */