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

#include "nexus_types.h"
#include "thumb.h"
#include "board.h"
#include "videolist.h"
#include "still_decode.h"
#include "bthumbnail_extractor.h"

BDBG_MODULE(atlas_thumb);

void stillReadyCallback(void * param)
{
    CThumb * pThumb = (CThumb *)param;

    BDBG_ASSERT(NULL != pThumb);

    BDBG_MSG(("stillReadyCallback"));
    B_Event_Set(pThumb->getEvent());
}

CThumb::CThumb(CConfiguration * pCfg) :
    _pPlaypump(NULL),
    _pBoardResources(NULL),
    _pId(NULL),
    _pConfig(NULL),
    _pCfg(pCfg),
    _pModel(NULL),
    _extractor(NULL),
    _pVideo(NULL),
    _fpData(NULL),
    _fpIndex(NULL),
    _bfpData(NULL),
    _bfpIndex(NULL),
    _eventStillReady(NULL),
    _pStillDecode(NULL),
    _pGraphics(NULL)
{
} /* CThumb */

CThumb::~CThumb()
{
    destroy();
} /* ~CThumb */

eRet CThumb::create()
{
    eRet ret = eRet_Ok;
    bthumbnail_extractor_create_settings create_settings;
    B_EventSettings                      eventSettings;

    bthumbnail_extractor_get_default_create_settings(&create_settings);
    _extractor = bthumbnail_extractor_create(&create_settings);
    CHECK_PTR_ERROR_GOTO("unable to create thumbnail extractor", _extractor, ret, eRet_OutOfMemory, error);

    B_Event_GetDefaultSettings(&eventSettings);
    _eventStillReady = B_Event_Create(&eventSettings);
    CHECK_PTR_ERROR_GOTO("unable to create thumbnail extractor event", _eventStillReady, ret, eRet_OutOfMemory, error);

    B_Event_Reset(_eventStillReady);

error:
    return(ret);
} /* create */

void CThumb::destroy()
{
    if (NULL != _eventStillReady)
    {
        B_Event_Destroy(_eventStillReady);
        _eventStillReady = NULL;
    }

    if (NULL != _extractor)
    {
        bthumbnail_extractor_destroy(_extractor);
        _extractor = NULL;
    }
}

eRet CThumb::open(
        CVideo *       pVideo,
        CStillDecode * pStillDecode,
        CGraphics *    pGraphics
        )
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;
    MString     strVideoPath;
    MString     strIndexPath;

    BDBG_ASSERT(NULL != pVideo);
    BDBG_ASSERT(NULL != pStillDecode);
    BDBG_ASSERT(NULL != pGraphics);
    BDBG_ASSERT(NULL != _extractor);
    BDBG_ASSERT(NULL != _pBoardResources);
    BDBG_ASSERT(NULL != _pId);

    _pVideo       = pVideo;
    _pStillDecode = pStillDecode;
    _pGraphics    = pGraphics;

    strVideoPath = pVideo->getVideoNamePath();
    strIndexPath = pVideo->getIndexNamePath();

    if (strVideoPath == strIndexPath)
    {
        /* pVideo will have the video name stored as index name if there is no index.
         * for the bthumbnail_extractor we need to specify the index as NULL in this
         * case. */
        strIndexPath = "";
    }

    BDBG_MSG(("CThumb::open() videoName:%s", strVideoPath.s()));
    BDBG_MSG(("CThumb::open() indexName:%s", strIndexPath.s()));
    _fpData = fopen(strVideoPath, "rb");
    CHECK_PTR_ERROR_GOTO("unable to open video data file", _fpData, ret, eRet_ExternalError, error);

    _bfpData = bfile_stdio_read_attach(_fpData);
    CHECK_PTR_ERROR_GOTO("unable to open bfile video data file", _bfpData, ret, eRet_ExternalError, error);

    if (false == strIndexPath.isEmpty())
    {
        _fpIndex = fopen(strIndexPath, "rb");
        CHECK_PTR_ERROR_GOTO("unable to open video index file", _fpIndex, ret, eRet_ExternalError, error);

        _bfpIndex = bfile_stdio_read_attach(_fpIndex);
        CHECK_PTR_ERROR_GOTO("unable to open bfile video index file", _bfpIndex, ret, eRet_ExternalError, error);
    }
    else
    {
        /* no index file given */
        _fpIndex  = NULL;
        _bfpIndex = NULL;
    }

    _pPlaypump = (CPlaypump *)_pBoardResources->checkoutResource(getId(), eBoardResource_playpump);
    CHECK_PTR_ERROR_GOTO("Unable to checkout playpump for thumbnail extractor", _pPlaypump, ret, eRet_OutOfMemory, error);

    _pPlaypump->initialize();
    ret = _pPlaypump->open();
    CHECK_ERROR_GOTO("unable to open playpump for thumbnail extractor", ret, error);

    {
        bthumbnail_extractor_settings settings;
        CPid * pPidVideo = pVideo->getPid(0, ePidType_Video);

        if (NULL == pPidVideo)
        {
            /* no video pid so no thumbnail available */
            ret = eRet_NotAvailable;
            goto error;
        }

        bthumbnail_extractor_get_settings(_extractor, &settings);
        settings.videoCodec    = pPidVideo->getVideoCodec();
        settings.transportType = pVideo->getPidMgr()->getTransportType();
        settings.timestampType = NEXUS_TransportTimestampType_eNone;
        settings.videoPid      = pPidVideo->getPid();
        settings.playpump      = _pPlaypump->getPlaypump();
        settings.datafile      = _bfpData;
        settings.indexfile     = _bfpIndex;
        BDBG_MSG(("videoCodec:%d, transportType:%d, timestampType:%d videoPid:%d playpump:%p datafile:%p indexfile:%p",
                  settings.videoCodec,
                  settings.transportType,
                  settings.timestampType,
                  settings.videoPid,
                  (void *)settings.playpump,
                  (void *)settings.datafile,
                  (void *)settings.indexfile
                  ));
        nerror = bthumbnail_extractor_set_settings(_extractor, &settings);
        CHECK_NEXUS_ERROR_GOTO("unable to set thumbnail extractor settings", ret, nerror, error);
    }

    goto done;
error:
    close();
done:
    return(ret);
} /* open */

void CThumb::close()
{
    if (NULL != _extractor)
    {
        bthumbnail_extractor_settings settings;
        bthumbnail_extractor_get_settings(_extractor, &settings);
        settings.datafile  = NULL;
        settings.indexfile = NULL;
        bthumbnail_extractor_set_settings(_extractor, &settings);
    }

    if (NULL != _pPlaypump)
    {
        _pPlaypump->close();
        if (true == _pPlaypump->isCheckedOut())
        {
            _pBoardResources->checkinResource(_pPlaypump);
            _pPlaypump = NULL;
        }
    }

    if (NULL != _bfpIndex)
    {
        bfile_stdio_read_detach(_bfpIndex);
        _bfpIndex = NULL;
    }

    if (NULL != _fpIndex)
    {
        fclose(_fpIndex);
        _fpIndex = NULL;
    }

    if (NULL != _bfpData)
    {
        bfile_stdio_read_detach(_bfpData);
        _bfpData = NULL;
    }

    if (NULL != _fpData)
    {
        fclose(_fpData);
        _fpData = NULL;
    }
} /* close */

/* calling code is responsible for destroying *pStillSurface */
eRet CThumb::extract(
        uint32_t            timeStamp,
        NEXUS_SurfaceHandle stillSurface,
        unsigned            width,
        unsigned            height
        )
{
    eRet        ret       = eRet_Ok;
    NEXUS_Error nerror    = NEXUS_SUCCESS;
    B_Error     berror    = B_ERROR_SUCCESS;
    CPid *      pPidVideo = NULL;

    BDBG_ASSERT(NULL != _pModel);
    BDBG_ASSERT(NULL != _pVideo);
    BDBG_ASSERT(NULL != _pStillDecode);
    BDBG_ASSERT(NULL != stillSurface);

    BDBG_ASSERT(true == _pStillDecode->isOpened());

    B_Event_Reset(_eventStillReady);

    nerror = bthumbnail_extractor_start_playpump(_extractor);
    CHECK_NEXUS_ERROR_GOTO("unable to start thumbnail extractor playpump", ret, nerror, error);

    pPidVideo = _pVideo->getPid(0, ePidType_Video);
    CHECK_PTR_MSG_GOTO("Unable to start still decoder without valid video pid, This maybe an Audio only stream", pPidVideo, ret, eRet_NotAvailable, done);

    if (pPidVideo->isDVREncryption())
    {
        pPidVideo->encrypt();
    }

    {
        NEXUS_PlaypumpOpenPidChannelSettings pidSettings;
        NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidSettings);
        pPidVideo->setSettings(&pidSettings);

        ret = pPidVideo->open(_pPlaypump);
        CHECK_ERROR_GOTO("unable to open video pid", ret, error);
    }

    ret = _pStillDecode->start(pPidVideo, stillReadyCallback, this);
    CHECK_ERROR_GOTO("unable to start still decoder", ret, error);

    nerror = bthumbnail_extractor_feed_picture(_extractor, timeStamp);
    CHECK_NEXUS_ERROR_GOTO("unable to feed picture to thumbnail extractor", ret, nerror, error);

    /* wait for still decoder to finish extracting thumbnail */
    berror = B_Event_Wait(_eventStillReady, 250);
    CHECK_BOS_MSG_GOTO("event wait error", ret, berror, error);

    /* destripe extracted thumbnail surface */
    ret = getStillSurface(_pStillDecode, _pGraphics, stillSurface, width, height);
    CHECK_ERROR_GOTO("unable to create destriped still surface", ret, error);

error:
    {
        NEXUS_StillDecoderStatus status;
        nerror = NEXUS_StillDecoder_GetStatus(_pStillDecode->getStillVideoDecode(), &status);
        if (NEXUS_SUCCESS != nerror)
        {
            if (false == status.endCodeFound)
            {
                ATLAS_ERROR("Still decode timed out because no end code was found in the ITB. Is this a valid still?", eRet_Timeout);
            }
            else
            if (false == status.stillPictureReady)
            {
                ATLAS_ERROR("Still decode timed out because the decoder did not respond. Is this a valid still?", eRet_Timeout);
            }
            else
            {
                ATLAS_ERROR("Still decode timed out for unknown reasons.", eRet_Timeout);
            }
            ret = eRet_NotAvailable;
        }
    }
done:
    _pStillDecode->stop();

    if (NULL != pPidVideo)
    {
        pPidVideo->close(_pPlaypump);
    }

    bthumbnail_extractor_stop_playpump(_extractor);
    return(ret);
} /* extract */

/* calling code MUST destroy returned surface when done with it. */
eRet CThumb::getStillSurface(
        CStillDecode *      pStillDecode,
        CGraphics *         pGraphics,
        NEXUS_SurfaceHandle stillSurface,
        unsigned            width,
        unsigned            height
        )
{
    eRet                       ret            = eRet_Ok;
    NEXUS_Error                nerror         = NEXUS_SUCCESS;
    NEXUS_StripedSurfaceHandle stripedSurface = NULL;

    nerror = NEXUS_StillDecoder_GetStripedSurface(pStillDecode->getStillVideoDecode(), &stripedSurface);
    CHECK_NEXUS_ERROR_GOTO("unable to get striped surface", ret, nerror, error);

    ret = pGraphics->destripeToSurface(stripedSurface, stillSurface, MRect(0, 0, width, height));
    CHECK_ERROR_GOTO("unable to destripe surface", ret, error);

error:
    return(ret);
} /* createDestripedSurface */

void CThumb::dump()
{
    BDBG_WRN(("CThumb::dump()"));
} /* dump */