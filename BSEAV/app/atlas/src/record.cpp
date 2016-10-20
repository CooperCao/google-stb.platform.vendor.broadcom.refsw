/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "band.h"
#include "convert.h"
#include "xmltags.h"
#include "record.h"
#include "mxmlparser.h"
#include "board.h"
#include <unistd.h> /* file i/o */
#include <fcntl.h>
/* #define DEFAULT_ENCRYPTION_KEY {12345678, 87654312, 0, 0} //NOTE: 12, not 21! */

BDBG_MODULE(atlas_record);
/* CRecpump Class*/
CRecpump::CRecpump(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    CResource(name, number, eBoardResource_recpump, pCfg),
    _streamOut(NULL),
    _indexOut(NULL),
    _navOut(NULL),
    _recpump(NULL),
    _allocated(false)
{
    BDBG_MSG((" CRecpump Constructor"));
    memset(&_recpumpSettings, 0, sizeof(_recpumpSettings));
    NEXUS_Recpump_GetDefaultSettings(&_recpumpSettings);
}

CRecpump::~CRecpump()
{
    close();
}

void CRecpump::close()
{
    stop();
    if (_recpump != NULL)
    {
        NEXUS_Recpump_Close(_recpump);
        _recpump = NULL;
    }
    _allocated = false;
}

eRet CRecpump::initialize(void)
{
    NEXUS_Recpump_GetDefaultSettings(&_recpumpSettings);
    return(eRet_Ok);
}

eRet CRecpump::open()
{
    eRet ret = eRet_Ok;

    if (_recpump != NULL)
    {
        BDBG_WRN((" you must close this recpump resource before opening again"));
        ret = eRet_ExternalError;
        goto error;
    }

    ret = CResource::initialize();
    CHECK_ERROR_GOTO("resource initialization failed", ret, error);

    /* take any id but remember the atlas resource is _number only, not NEXUS */
    _recpump = NEXUS_Recpump_Open(NEXUS_ANY_ID, NULL);
    CHECK_PTR_ERROR_GOTO("Cannot Open Recpump", _recpump, ret, eRet_ExternalError, error);

    BDBG_MSG(("(*&*&*&**&*&*& recpump %p", (void *)_recpump));
    _allocated = true;

error:
    return(ret);
} /* open */

eRet CRecpump::setTpitFilter(NEXUS_PidChannelHandle pidChannel)
{
    eRet                    ret = eRet_Ok;
    NEXUS_Error             rc  = NEXUS_SUCCESS;
    NEXUS_RecpumpTpitFilter filter;

    BDBG_MSG(("H264, Setting Tpit Filter "));
    NEXUS_Recpump_GetDefaultTpitFilter(&filter);
    filter.config.mpeg.randomAccessIndicatorEnable    = true;
    filter.config.mpeg.randomAccessIndicatorCompValue = true;
    rc = NEXUS_Recpump_SetTpitFilter(_recpump, pidChannel, &filter);
    CHECK_NEXUS_ERROR_GOTO("Cannot set Tpit Filter", ret, rc, error);

error:
    return(ret);
}

eRet CRecpump::start(void)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    CHECK_PTR_ERROR_GOTO("Null _recpump", _recpump, ret, eRet_NotAvailable, error);

    nerror = NEXUS_Recpump_Start(_recpump);
    CHECK_NEXUS_ERROR_GOTO("unable to start recpump", ret, nerror, error);

error:
    return(ret);
}

eRet CRecpump::stop(void)
{
    eRet ret = eRet_Ok;

    if (_recpump)
    {
        NEXUS_Recpump_Stop(_recpump);
    }

    return(ret);
}

eRet CRecpump::setSettings(NEXUS_RecpumpSettings * pRecpumpSettings)
{
    NEXUS_Error rc  = NEXUS_SUCCESS;
    eRet        ret = eRet_Ok;

    BDBG_ASSERT(NULL != pRecpumpSettings);

    _recpumpSettings = *pRecpumpSettings;
    NEXUS_Recpump_GetSettings(_recpump, &_recpumpSettings);
    rc = NEXUS_Recpump_SetSettings(_recpump, &_recpumpSettings);
    CHECK_NEXUS_ERROR_GOTO("Cannot set Recpump Settings", ret, rc, error);

    BDBG_MSG(("Success in setting Recpump Settings"));
    goto done;

error:
    BDBG_ERR(("Failed to Set Recpump settings. Recpump: %d", _number));
done:
    return(ret);
} /* setSettings */

/* CRecord Class*/
CRecord::CRecord(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    CResource(name, number, eBoardResource_record, pCfg),
    _file(NULL),
    _customFile(NULL),
    _stickyFile(NULL),
    _pRecpump(NULL),
    _record(NULL),
    _pParserBand(NULL),
    _allocated(false),
    _currentVideo(NULL),
    _pBoardResources(NULL),
    _pidMgr(NULL)
#if NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA
    , _pDma(NULL)
#endif
{
    memset(&_recordSettings, 0, sizeof(_recordSettings));
    NEXUS_Record_GetDefaultSettings(&_recordSettings);
}

CRecord::~CRecord()
{
    if (_record != NULL)
    {
        stop();
    }

    close();
}

void CRecord::close()
{
    if (_record != NULL)
    {
        /* stop(); */
        NEXUS_Record_Destroy(_record);
        _record = NULL;
    }

    if (_pRecpump != NULL)
    {
        _pRecpump->stop();
        _pRecpump->close();
        _pBoardResources->checkinResource(_pRecpump);
        _pRecpump = NULL;
    }

    _allocated = false;
} /* close */

void CRecord::dump(void)
{
    BDBG_MSG(("record number %d", _number));
    if (NULL != _currentVideo)
    {
        BDBG_MSG(("current video:"));
        _currentVideo->dump();
    }
}

eRet CRecord::open()
{
    eRet ret = eRet_Ok;

    ret = CResource::initialize();
    CHECK_ERROR_GOTO("resource initialization failed", ret, error);

    if ((_pRecpump != NULL) && (_record != NULL))
    {
        BDBG_WRN((" you must close this record resource before opening again"));
        ret = eRet_ExternalError;
        goto error;
    }

    BDBG_MSG(("Init Record! %d", _number));
    _pRecpump = (CRecpump *)_pBoardResources->checkoutResource(getCheckedOutId(), eBoardResource_recpump);
    CHECK_PTR_ERROR_GOTO("Unable to checkout recpump", _pRecpump, ret, eRet_ExternalError, error);

    ret = _pRecpump->open();
    CHECK_ERROR_GOTO("Unable to open recpump", ret, error);

    _record = NEXUS_Record_Create();
    CHECK_PTR_ERROR_GOTO("Unable to open record", _record, ret, eRet_ExternalError, error);

    NEXUS_Record_GetSettings(_record, &_recordSettings);
    _recordSettings.recpump = _pRecpump->getRecpump();
    NEXUS_Record_SetSettings(_record, &_recordSettings);

    BDBG_MSG(("added record/recpump #%d", _number));
    BDBG_MSG(("(*&*&*&**&*&*& record %p", (void *)_record));
    _allocated = true;
    goto done;
error:

    close();
    BDBG_ERR((" Cannot open Record"));

done:
    return(ret);
} /* initialize */

/* LUA record */
eRet CRecord::start()
{
    eRet        ret = eRet_ExternalError;
    NEXUS_Error rc  = NEXUS_SUCCESS;
    MString     fullFilePath;
    MString     fullIndexPath;

    if ((_currentVideo == NULL) || _currentVideo->getVideosPath().isEmpty())
    {
        BDBG_ERR(("Please setVideo(CVideo *Video) before calling %s ", __FUNCTION__));
        return(eRet_ExternalError);
    }

    if (_currentVideo->isRecordActive() == true)
    {
        BDBG_WRN(("Record is Active already continue to Record"));
        return(eRet_Ok);
    }

    BDBG_MSG(("Record START"));
    /* IndexName will always be the same as the filename except with .nav extension.*/
    fullFilePath  = _currentVideo->getVideoNamePath();
    fullIndexPath = _currentVideo->getIndexNamePath();

    BDBG_MSG(("Opening files: '%s' '%s'", fullFilePath.s(), fullIndexPath.s()));
    _file = NEXUS_FileRecord_OpenPosix(fullFilePath.s(), fullIndexPath.s());
    if (!_file)
    {
        BDBG_ERR(("can't open files: '%s' '%s'", fullFilePath.s(), fullIndexPath.s()));
        goto error;
    }

    /* Start record */
    rc = NEXUS_Record_Start(_record, _file);
    CHECK_NEXUS_ERROR_GOTO("record start failed", ret, rc, error);

    _currentVideo->setRecordState(true);

    ret = notifyObservers(eNotify_RecordStarted, this);
    CHECK_ERROR_GOTO("error notifying observers", ret, error);

    BDBG_MSG(("Record Started"));

    return(eRet_Ok);

error:
    return(ret);
} /* start */

/* Is Active */
bool CRecord::isActive(void)
{
    if ((_currentVideo != NULL))
    {
        return(true);
    }
    else
    {
        return(false);
    }
}

eRet CRecord::stop()
{
    eRet ret = eRet_Ok;

    BDBG_MSG((" Record stop"));

    if ((_currentVideo == NULL))
    {
        BDBG_MSG((" Record is not active"));
        ret = eRet_ExternalError;
        goto error;
    }

    /* Close all Pids associate with this plyaback and close the PID channels */
    if (NULL != _record)
    {
        NEXUS_Record_Stop(_record);
    }

    if (NULL != _file)
    {
        NEXUS_FileRecord_Close(_file);
        _file = NULL;
    }

    _currentVideo->closeVideo();
    _currentVideo->setRecordState(false);

#if NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA
    if (_pDma)
    {
        _pDma->close();
        _pBoardResources->checkinResource(_pDma);
        _pDma = NULL;
    }
#endif /* if NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA */

    ret = notifyObservers(eNotify_RecordStopped, this);
    CHECK_ERROR_GOTO("error notifying observers", ret, error);

    _currentVideo = NULL;

error:
    return(ret);
} /* stop */

void CRecord::dupPidMgr(CPidMgr * pPidMgr)
{
    BDBG_ASSERT(NULL != pPidMgr);
    /* Just copy the *PTR */
    _pidMgr = pPidMgr;
}

void CRecord::closePids(void)
{
    /* Make sure you call this function before Record Close */
    _pidMgr->closeRecordPidChannels(this);
    _pidMgr = NULL;
}

NEXUS_RecordSettings CRecord::getSettings(void)
{
    NEXUS_Record_GetSettings(_record, &_recordSettings);
    return(_recordSettings);
}

void CRecord::setSettings(NEXUS_RecordSettings * pRecordSettings)
{
    BDBG_ASSERT(NULL != pRecordSettings);

    _recordSettings = *pRecordSettings;
    if (NEXUS_Record_SetSettings(_record, &_recordSettings) != NEXUS_SUCCESS)
    {
        BDBG_ERR(("Cannot SetSettings"));
    }
}

void CRecord::setVideo(CVideo * pVideo)
{
    pVideo->inUse();
    _currentVideo = pVideo;
}

CPid * CRecord::getPid(
        uint16_t index,
        ePidType type
        )
{
    return(_pidMgr->getPid(index, type));
}

CPid * CRecord::findPid(
        uint16_t index,
        ePidType type
        )
{
    return(_pidMgr->findPid(index, type));
}