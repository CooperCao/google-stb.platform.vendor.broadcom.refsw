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

#include "band.h"
#include "board.h"
#include "playback.h"
#include "videolist.h"
#include "convert.h"
#include "xmltags.h"
#include "mxmlparser.h"
#include "bmedia_probe.h"
#include "bmpeg2ts_probe.h"
#include "bfile_stdio.h"
#include <unistd.h> /* file i/o */
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>
#if B_HAS_ASF
#include "basf_probe.h"
#endif
#if B_HAS_AVI
#include "bavi_probe.h"
#endif
#include "nexus_core_utils.h"

#ifdef PLAYBACK_IP_SUPPORT
#include "bip.h"
#include "bip_media_info.h"
#endif /* PLAYBACK_IP_SUPPORT */

#define MAXBUF  1024*20

BDBG_MODULE(atlas_videolist);

CVideo::CVideo(
        CConfiguration * pCfg,
        const char *     strMedia,
        const char *     strIndex,
        const char *     strPathVideos,
        const char *     strPathInfo,
        const char *     strPathIndex
        ) :
    _date(0),
    _size(0.0),
    _duration(0),
    _maxBitrate(0),
    _mediaName(strMedia),
    _indexName(strIndex),
    _videosPath(strPathVideos),
    _infoPath(strPathInfo),
    _indexPath(strPathIndex),
    _isForPurchase(false),
    _timestamp_enabled(false),
    _encrypted(false),
    _isRecordActive(false),
    _isIndexRequired(false),
    _timeShifting(false),
    _position(0),
    _usageCounter(0),
    _serverIndexState(eServerIndexState_Missing),
    _totalStreams(0),
    _isIndexGenerationNeeded(false),
    _thumbTimestamp(0)
{
    BDBG_ASSERT(NULL != pCfg);
    _pidMgr.initialize(pCfg);

    if (_videosPath.isEmpty()) { _videosPath = GET_STR(pCfg, VIDEOS_PATH); }
    if (_infoPath.isEmpty()) { _infoPath = GET_STR(pCfg, INFO_PATH); }
    if (_indexPath.isEmpty()) { _indexPath = GET_STR(pCfg, INDEX_PATH); }
}

CVideo::CVideo(
        const CConfiguration * pCfg,
        const CVideo           & video
        ) : /* copy constructor */
    _description(video._description),
    _date(video._date),
    _size(video._size),
    _duration(video._duration),
    _maxBitrate(video._maxBitrate),
    _infoName(video._infoName),
    _mediaName(video._mediaName),
    _indexName(video._indexName),
    _videosPath(video._videosPath),
    _infoPath(video._infoPath),
    _indexPath(video._indexPath),
    _isForPurchase(video._isForPurchase),
    _timestamp_enabled(video._timestamp_enabled),
    _encrypted(video._encrypted),
    _isRecordActive(video._isRecordActive),
    _isIndexRequired(video._isIndexRequired),
    _timeShifting(video._timeShifting),
    _position(video._position),
    _usageCounter(video._usageCounter),
    _serverIndexState(video._serverIndexState),
    _totalStreams(video._totalStreams),
    _isIndexGenerationNeeded(video._isIndexGenerationNeeded),
    _thumbTimestamp(video._thumbTimestamp)
{
    /* do NOT copy pids */
    BDBG_ASSERT(NULL != pCfg);
    _pidMgr.initialize((CConfiguration *)pCfg);
}

CVideo::~CVideo()
{
    /* Deleting the Video object */
    BDBG_MSG(("Video Name %s ", _mediaName.s()));
}

/* gets width of first video pid */
unsigned CVideo::getWidth()
{
    CPid *   pPid   = NULL;
    unsigned nWidth = 0;

    /* get first video pid */
    pPid = _pidMgr.getPid(0, ePidType_Video);
    if (NULL != pPid)
    {
        nWidth = pPid->getWidth();
    }

    return(nWidth);
}

/* sets width of first video pid */
void CVideo::setWidth(unsigned width)
{
    CPid * pPid = NULL;

    /* get first video pid */
    pPid = _pidMgr.getPid(0, ePidType_Video);
    if (NULL != pPid)
    {
        pPid->setWidth(width);
    }
}

/* gets height of first video pid */
unsigned CVideo::getHeight()
{
    CPid *   pPid    = NULL;
    unsigned nHeight = 0;

    /* get first video pid */
    pPid = _pidMgr.getPid(0, ePidType_Video);
    if (NULL != pPid)
    {
        nHeight = pPid->getHeight();
    }

    return(nHeight);
}

/* sets height of first video pid */
void CVideo::setHeight(unsigned height)
{
    CPid * pPid = NULL;

    /* get first video pid */
    pPid = _pidMgr.getPid(0, ePidType_Video);
    if (NULL != pPid)
    {
        pPid->setHeight(height);
    }
}

void CVideo::dump(bool bForce)
{
    BDBG_Level levelVideo;

    if (true == bForce)
    {
        BDBG_GetModuleLevel("atlas_videolist", &levelVideo);
        BDBG_SetModuleLevel("atlas_videolist", BDBG_eMsg);
    }

    BDBG_MSG(("Video Name %s ", _mediaName.s()));
    BDBG_MSG(("Video Size %s ", MString(_size).s()));
    BDBG_MSG(("Info Name %s ", _infoName.s()));
    BDBG_MSG(("Index Name %s ", _indexName.s()));
    BDBG_MSG(("Videos Path name %s", _videosPath.s()));
    BDBG_MSG(("Info Path name %s", _infoPath.s()));
    BDBG_MSG(("Index Path name %s", _indexPath.s()));
    BDBG_MSG(("Duration %s", MString(_duration).s()));
    BDBG_MSG(("Max Bitrate %s", MString(_maxBitrate).s()));
    _pidMgr.dump(bForce);

    if (true == bForce)
    {
        BDBG_SetModuleLevel("atlas_videolist", levelVideo);
    }
} /* dump */

bool CVideo::isEncrypted(void)
{
    CPid * pVideoPid = NULL;
    CPid * pAudioPid = NULL;
    bool   encrypted = false;

    pVideoPid = _pidMgr.getPid(0, ePidType_Video);
    pAudioPid = _pidMgr.getPid(0, ePidType_Audio);

    if (pVideoPid)
    {
        encrypted = pVideoPid->isDVREncryption();
    }
    else
    if (pAudioPid)
    {
        encrypted = pAudioPid->isDVREncryption();
    }

    return(encrypted);
} /* isEncrypted */

void CVideo::closeVideo(void)
{
    if (--_usageCounter > 0)
    {
        return;
    }

    _timeShifting = false;
    _encrypted    = false;
    _position     = 0;
}

bool CVideo::isPlaybackActive(void)
{
    if (_usageCounter > 0)
    {
        return(true);
    }
    else
    {
        return(false);
    }
}

bool CVideo::hasIndexName()
{
    return(false == _indexName.isEmpty());
}

bool CVideo::hasIndex()
{
    struct stat stIndex;
    MString     strIndexNamePath = getIndexNamePath();

    B_Os_Memset(&stIndex, 0, sizeof(stIndex));

    if (false == strIndexNamePath.isEmpty())
    {
        if (stat(strIndexNamePath, &stIndex) < 0)
        {
            BDBG_ERR((" Stat returned an error"));
        }
    }

    return((false == _indexName.isEmpty()) && (0 < stIndex.st_size));
} /* hasIndex */

eRet CVideo::writeXML(MXmlElement * xmlElem)
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NULL != xmlElem);

    if (xmlElem->tag() != XML_TAG_STREAM)
    {
        return(ret);
    }

    xmlElem->addAttr(XML_ATT_DURATION, MString(getDuration()));
    xmlElem->addAttr(XML_ATT_MAX_BITRATE, MString(getMaxBitrate()));

    return(ret);
} /* write */

eRet CVideo::generateInfoName(const char * strName)
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NULL != strName);

    _infoName = strName;
    _infoName.truncate(_infoName.findRev("."));
    _infoName += ".nfo";

    return(ret);
}

eRet CVideo::readXML(MXmlElement * xmlElem)
{
    eRet    ret = eRet_Ok;
    MString strDuration;
    MString strMaxBitrate;
    MString strType;
    MString strPmt;

    BDBG_ASSERT(NULL != xmlElem);

    if (xmlElem->tag() != XML_TAG_STREAM)
    {
        return(ret);
    }

    strDuration   = xmlElem->attrValue(XML_ATT_DURATION);
    strMaxBitrate = xmlElem->attrValue(XML_ATT_MAX_BITRATE);

    if (false == strDuration.isEmpty())
    {
        setDuration(strDuration.toInt());
    }

    if (false == strMaxBitrate.isEmpty())
    {
        setMaxBitrate(strMaxBitrate.toInt());
    }

    _pidMgr.readXML(xmlElem);

    return(ret);
} /* readXML */

/* in the playback case we will never call this function. Playback will copy the video
 * objects pidMgr, so it can independantly open/close the pid Channels. This is primarily used
 * to handle the case where we are recording a given channel.  in that case,
 * we will reuse the possibly already opened pid channels in the channel object.
 * since we still need to use a video object, we will simply duplicated the
 * channel's pidmgr with this video object. */
void CVideo::dupPidMgr(CPidMgr * pPidMgr)
{
    BDBG_ASSERT(NULL != pPidMgr);
    _pidMgr = *pPidMgr;
}

eRet CVideo::updateFileSysInfo()
{
    eRet        ret  = eRet_Ok;
    time_t      date = 0;
    int64_t     size = 0;
    struct stat buf;
    FILE *      fin = fopen64((getVideoNamePath()).s(), "rb");

    CHECK_PTR_ERROR_GOTO("unable to open file", fin, ret, eRet_ExternalError, error);

    if (0 == fstat(fileno(fin), &buf))
    {
        date = buf.st_mtime;
        size = (int64_t)buf.st_size;
    }

error:
    if (NULL != fin)
    {
        fclose(fin);
        fin = NULL;
    }

    _date = date;
    _size = size;

    return(ret);
} /* updateFileSysInfo */

CPlaybackList::CPlaybackList(CConfiguration * pCfg) :
    CMvcModel("CPlaybackList"),
    _pCfg(pCfg),
    _lastVideo(NULL),
    _index(0),
    _nextVideoNumber(0),
    _generateIndexesThread_handle(NULL),
    _stopGenerateIndexes(true)
{
    BDBG_ASSERT(NULL != pCfg);

    _mutexPlaylist = B_Mutex_Create(NULL);
    BDBG_ASSERT(NULL != _mutexPlaylist);
}

CPlaybackList::~CPlaybackList()
{
    MListItr <CVideo> itrVideo(&_videos);
    CVideo *          pVideo = NULL;

    if (NULL != _generateIndexesThread_handle)
    {
        destroyThreadGenerateIndexes();
    }

    {
        CScopedMutex observerListMutex(_mutexPlaylist);

        for (pVideo = itrVideo.first(); pVideo; pVideo = itrVideo.next())
        {
            DEL(pVideo);
        }
        _videos.clear();
    }

    if (_mutexPlaylist)
    {
        B_Mutex_Destroy(_mutexPlaylist);
        _mutexPlaylist = NULL;
    }
}

eRet CPlaybackList::readInfo(
        const char * infoName,
        const char * path
        )
{
    MXmlParser    xmlParser;
    MXmlElement * xmlElemInfo  = NULL;
    MXmlElement * xmlElemTop   = NULL;
    MXmlElement * xmlElemAtlas = NULL;
    CVideo *      pVideo       = NULL;
    int           fd           = -1;
    char *        buf          = NULL;
    int           nBufSize     = 0;
    eRet          ret          = eRet_Ok;
    const int     bufMax       = 2048;
    MString       strPath(path);
    MString       strPathInfo;
    MString       strPathIndex;
    MString       strInfoName(infoName);
    MString       strMediaName;
    MString       strSize;
    MString       strIndexName;
    MString       strServerIndexState;
    MString       strDescription;
    MString       strIndexRequired;
    MString       strTimestamp;
    MString       strVersion;

    fd = open(strPath + MString("/") + strInfoName, O_RDONLY, 0);
    if (!fd)
    {
        BDBG_ERR(("Failed to open file '%s' (path:%s)!", strInfoName.s(), strPath.s()));
        return(eRet_ExternalError);
    }

    buf = (char *)malloc(sizeof(char) * bufMax);
    CHECK_PTR_ERROR_GOTO("Error malloc parse buffer!", buf, ret, eRet_OutOfMemory, error);

    if (0 <= fd)
    {
        memset(buf, 0, sizeof(char) * bufMax);
        nBufSize        = read(fd, buf, (size_t)bufMax - 1);
        buf[bufMax - 1] = '\0';
        close(fd);
        fd = -1;
    }

    if (nBufSize <= 0)
    {
        BDBG_WRN(("Cannot open XML nfo file: %s", strInfoName.s()));
        ret = eRet_ExternalError;
        goto error;
    }

    xmlElemTop = xmlParser.parse(buf);
    CHECK_PTR_ERROR_GOTO("Syntax error parsing channel list XML", xmlElemTop, ret, eRet_ExternalError, error);

#if 0
    {
        BDBG_Level dbgLevel = (BDBG_Level)0;
        BDBG_GetModuleLevel("atlas_videolist", &dbgLevel);

        if (BDBG_eMsg == dbgLevel)
        {
            BDBG_MSG(("----------------------------------------------------------"));
            BDBG_MSG(("xmlElemTop:"));
            xmlElemTop->print();
        }
    }
#endif /* if 0 */

    /* check xml version number */
    {
        int major = 1;
        int minor = 0;

        xmlElemAtlas = xmlElemTop->findChild(XML_TAG_ATLAS);
        if (xmlElemAtlas)
        {
            strVersion = xmlElemAtlas->attrValue(XML_ATT_VERSION);
            if (false == strVersion.isEmpty())
            {
                unsigned dotIndex = strVersion.find('.');

                major = (unsigned)strVersion.left(dotIndex).toInt();
                minor = (unsigned)MString(strVersion.mid(dotIndex + 1)).toInt();
            }
        }

        /* compare XML version number */
        if (GET_INT(_pCfg, XML_VERSION_MAJOR_NFO) != major)
        {
            /* XML version mismatch! */
            ret = eRet_ExternalError;
            goto error;
        }
    }

    xmlElemInfo = xmlElemAtlas->findChild(XML_TAG_INFOFILE);
    if (xmlElemInfo)
    {
        strMediaName = xmlElemInfo->attrValue(XML_ATT_FILENAME);
        if (false == strMediaName.isNull())
        {
            strSize      = xmlElemInfo->attrValue(XML_ATT_SIZE);
            strIndexName = xmlElemInfo->attrValue(XML_ATT_INDEXNAME);

            strServerIndexState = xmlElemInfo->attrValue(XML_ATT_SERVERINDEXSTATE);
            if (strServerIndexState.isNull())
            {
                strServerIndexState = "Missing";
            }

            strDescription = xmlElemInfo->attrValue(XML_ATT_DESCRIPTION);
            if (strDescription.isNull())
            {
                BDBG_MSG(("Default Descrition "));
                strDescription = strMediaName;
            }

            strIndexRequired = xmlElemInfo->attrValue(XML_ATT_INDEXREQUIRED);
            if (strIndexRequired.isNull())
            {
                strIndexRequired = "false";
            }

            strTimestamp = xmlElemInfo->attrValue(XML_ATT_TIMESTAMP);
            if (strTimestamp.isNull())
            {
                strTimestamp = "false";
            }

            {
                MString strPathNew = xmlElemInfo->attrValue(XML_ATT_PATH);
                if (0 < strPathNew.length())
                {
                    /* Overwrite Path with path from .nfo file */
                    BDBG_MSG(("New Videos Path %s", strPathNew.s()));
                    strPath = strPathNew;
                }
                strPathNew = xmlElemInfo->attrValue(XML_ATT_PATH_INFO);
                if (0 < strPathNew.length())
                {
                    /* Overwrite Path with path from .nfo file */
                    BDBG_MSG(("New Info Path %s", strPathNew.s()));
                    strPathInfo = strPathNew;
                }
                strPathNew = xmlElemInfo->attrValue(XML_ATT_PATH_INDEX);
                if (0 < strPathNew.length())
                {
                    /* Overwrite Path with path from .nfo file */
                    BDBG_MSG(("New Index Path %s", strPathNew.s()));
                    strPathIndex = strPathNew;
                }
            }

            MXmlElement * xmlElemStream = NULL;
            uint32_t      nStreams      = 0;

            for (xmlElemStream = xmlElemInfo->firstChild();
                 xmlElemStream;
                 xmlElemStream = xmlElemInfo->nextChild())
            {
                /* count total number of streams */
                nStreams++;
            }

            /* look for multiple streams */
            for (xmlElemStream = xmlElemInfo->firstChild();
                 xmlElemStream;
                 xmlElemStream = xmlElemInfo->nextChild())
            {
                if (xmlElemStream->tag() == XML_TAG_STREAM)
                {
                    /* create a new video object for each stream listed in nfo file */
                    pVideo = new CVideo(_pCfg, strMediaName, strIndexName, strPath, strPathInfo, strPathIndex);
                    CHECK_PTR_ERROR_GOTO("unable to allocate CVideo", pVideo, ret, eRet_OutOfMemory, error);
                    pVideo->setSize(strSize.toLongLong());
                    pVideo->setInfoName(strInfoName);
                    pVideo->setDescription(strDescription);
                    pVideo->setIndexRequired("true" == strIndexRequired);
                    pVideo->enableTimestamp("true" == strTimestamp);
                    pVideo->setServerIndexState(stringToServerIndexState(strServerIndexState));
                    pVideo->setTotalStreams(nStreams);
                    pVideo->readXML(xmlElemStream);
                    {
                        CScopedMutex observerListMutex(_mutexPlaylist);
                        _videos.add(pVideo);
                    }
                }
            }
        }
        else
        {
            BDBG_MSG((" No File Name Found, ingnore entry"));
        }
    }

    goto done;
error:
    DEL(pVideo);
done:
    DEL(xmlElemTop);

    if (buf)
    {
        free(buf);
        buf = NULL;
    }

    if (0 <= fd)
    {
        close(fd);
        fd = -1;
    }
    return(ret);
} /* readInfo */

eRet CVideo::generateIndex()
{
    eRet ret = eRet_Ok;

#ifdef PLAYBACK_IP_SUPPORT
    BIP_Status errorBip     = BIP_SUCCESS;
    int        retOS        = -1;
    CPid *     pPid         = _pidMgr.getPid(0, ePidType_Video);
    FILE *     file         = NULL;
    MString    strIndexName = getIndexName();
    MString    strIndexNamePath;
    MString    strIndexNamePathTmp;

    if (NULL == pPid)
    {
        return(ret);
    }

    /* change/add .nav file extension */
    {
        int indexDot = 0;

        indexDot = strIndexName.findRev('.');
        if (-1 < indexDot)
        {
            strIndexName.truncate(indexDot);
        }

        strIndexName    += ".nav";
        strIndexNamePath = getIndexPath() + MString("/") + strIndexName;
        BDBG_MSG(("new index filename:%s", strIndexNamePath.s()));
    }

    file = fopen(strIndexNamePath, "r");
    if (NULL != file)
    {
        /* nav index file already exists */
        BDBG_MSG(("Nav index exists for %s",strIndexName.s()));
        goto close_file;
    }

    /* nav index file doesn't exist so create it */
    BDBG_WRN(("creating index for:%s pid:%d nav:%s", getVideoNamePath().s(), pPid->getPid(), getIndexNamePath().s()));

    /* generate nav into a tmp file in case something goes wrong before we are finished.
     * we will rename the tmp file after generation is completed */
    strIndexNamePathTmp = strIndexNamePath + ".tmp";

    errorBip = BIP_MediaInfo_MakeNavForTsFile(
            getVideoNamePath(),
            strIndexNamePathTmp,
            pPid->getPid(),
            NULL);
    CHECK_BIP_ERROR_GOTO((MString("Unable to make index file for:") + getVideoNamePath()).s(), ret, errorBip, error);

    retOS = rename(strIndexNamePathTmp, strIndexNamePath);
    CHECK_BOS_ERROR_GOTO("unable to rename TEMP index nav file", ret, retOS, error);

    /* set new index name */
    setIndexName(strIndexName);

    goto done;
error:
    retOS = remove(strIndexNamePathTmp);
    CHECK_BOS_WARN("unable to remove TEMP index nav file", ret, retOS);

    /* create empty nav file so atlas won't repeatedly try failing nav generation */
    file = fopen(strIndexNamePath,"w");
close_file:
    if(file != NULL )
    {
        fclose(file);
        file = NULL;
    }

done:
    setIndexGenerationNeeded(false);
#endif /* PLAYBACK_IP_SUPPORT */
    return(ret);
} /* generateIndex() */

uint32_t CVideo::incrementThumbTimestamp(uint32_t increment)
{
    uint32_t nDuration = getDuration() * 90 / 100;

    if (nDuration > 0)
    {
        /* increment only if video has a valid duration */
        _thumbTimestamp = _thumbTimestamp + increment;
        _thumbTimestamp = _thumbTimestamp % nDuration;
    }
    else
    {
        _thumbTimestamp = increment;
    }

    return(_thumbTimestamp);
} /* incrementThumbTimestamp */

MString CVideo::getVideoNamePathAbsolute(void)
{
    MString strPathAbsolute;
    char *  pPathAbsolute = NULL;

    pPathAbsolute   = realpath(getVideoNamePath(), NULL);
    strPathAbsolute = pPathAbsolute;
    FRE(pPathAbsolute);

    return(strPathAbsolute);
}

/* if not given program number, find() will return the first matching video name
 * if given program number, find() will return videoname and program num match. */
CVideo * CPlaybackList::find(
        const char * name,
        int          nProgram
        )
{
    MListItr <CVideo> itrVideo(&_videos);
    MString           compareName(name);
    CVideo *          video = NULL;
    int               index = 0;

    /* convert possible given path to filename only for comparison */
    index = compareName.findRev('/');
    if (0 < index)
    {
        compareName = compareName.mid(index + 1);
    }

    {
        CScopedMutex observerListMutex(_mutexPlaylist);

        for (video = itrVideo.first(); video; video = itrVideo.next())
        {
            if (video->getVideoName() == compareName)
            {
                /* found video name match */
                if (-1 == nProgram)
                {
                    /* match */
                    break;
                }
                else
                {
                    /* program number given so compare it */
                    if (nProgram == (int)video->getProgram())
                    {
                        /* found video name match that also matches program number */
                        break;
                    }
                }
            }
        }
    }

    return(video);
} /* find */

eRet CPlaybackList::writeInfo(
        MList<CVideo> * probedVideos,
        CPidMgr *       pPidMgr
        )
{
    MXmlElement * xmlElemAtlas = NULL;
    MXmlElement * xmlElemInfo  = NULL;
    FILE *        file         = NULL;
    CVideo *      pVideo       = NULL;
    MString       strInfoName;
    eRet          ret = eRet_Ok;

    BDBG_ASSERT(NULL != probedVideos);

    if (0 == probedVideos->total())
    {
        /* no videos to save */
        goto error;
    }

    pVideo      = probedVideos->first();
    strInfoName = pVideo->getInfoName();

    file = fopen(pVideo->getInfoNamePath(), "w");
    if (!file)
    {
        BDBG_ERR(("Cannot open file for writing %s", pVideo->getInfoNamePath().s()));
        ret = eRet_ExternalError;
        goto error;
    }

    xmlElemAtlas = new MXmlElement(NULL, XML_TAG_ATLAS);
    CHECK_PTR_ERROR_GOTO("unable to allocate tag", xmlElemAtlas, ret, eRet_OutOfMemory, error);

    /* add version attribute */
    {
        MString strVersion = GET_STR(_pCfg, XML_VERSION_MAJOR_NFO);
        strVersion += ".";
        strVersion += GET_STR(_pCfg, XML_VERSION_MINOR_NFO);
        xmlElemAtlas->addAttr(XML_ATT_VERSION, strVersion.s());
    }

    xmlElemInfo = new MXmlElement(xmlElemAtlas, XML_TAG_INFOFILE);
    CHECK_PTR_ERROR_GOTO("unable to allocate tag", xmlElemInfo, ret, eRet_OutOfMemory, error);

    xmlElemInfo->addAttr(XML_ATT_FILENAME, pVideo->getVideoName());
    xmlElemInfo->addAttr(XML_ATT_SIZE, MString(pVideo->getSize()));
    if(pVideo->getIndexName().isEmpty() == false)
    {
        xmlElemInfo->addAttr(XML_ATT_INDEXNAME, pVideo->getIndexName());
    }

    xmlElemInfo->addAttr(XML_ATT_SERVERINDEXSTATE, serverIndexStateToString(pVideo->getServerIndexState()));

    if (false == pVideo->getDescription().isEmpty())
    {
        xmlElemInfo->addAttr(XML_ATT_DESCRIPTION, pVideo->getDescription());
    }
    xmlElemInfo->addAttr(XML_ATT_INDEXREQUIRED, pVideo->isIndexRequired() ? "true" : "false");
    xmlElemInfo->addAttr(XML_ATT_TIMESTAMP, pVideo->isTimestampEnabled() ? "true" : "false");
    xmlElemInfo->addAttr(XML_ATT_PATH, pVideo->getVideosPath());
    if (false == pVideo->getInfoPath().isEmpty())
    {
        xmlElemInfo->addAttr(XML_ATT_PATH_INFO, pVideo->getInfoPath());
    }
    if (false == pVideo->getIndexPath().isEmpty())
    {
        xmlElemInfo->addAttr(XML_ATT_PATH_INDEX, pVideo->getIndexPath());
    }

    {
        MListItr<CVideo> itr(probedVideos);
        for (pVideo = itr.first(); pVideo; pVideo = itr.next())
        {
            MXmlElement * xmlElemStream = NULL;

            if (NULL == pPidMgr)
            {
                xmlElemStream = pVideo->getPidMgr()->writeXML(xmlElemInfo);
            }
            else
            {
                BDBG_MSG(("Writing pidMgr that was passed in"));
                xmlElemStream = pPidMgr->writeXML(xmlElemInfo);
            }

            if (NULL != xmlElemStream)
            {
                pVideo->writeXML(xmlElemStream);
            }
        }
    }

    xmlElemAtlas->print(file);

    BDBG_MSG(("Saved nfo file: %s", strInfoName.s()));

error:
    DEL(xmlElemAtlas);

    if (file)
    {
        fclose(file);
        file = NULL;
    }

    return(ret);
} /* writeInfo */

eRet CPlaybackList::createInfo(
        const char * mediafile,
        const char * indexfile,
        const char * path,
        bool         bAddToVideoList
        )
{
    bmedia_probe_config         probe_config;
    const bmedia_probe_stream * stream;
    const bmedia_probe_track *  track;
    bfile_io_read_t             fd = NULL;
    MString                     strMediaFile(mediafile);
    MString                     strIndexFile(indexfile);
    MString                     strPath(path);
    bmedia_probe_t              probe = NULL;

    MList <CVideo> probedVideos;
    CVideo *       pVideo = NULL;
    eRet           retVal = eRet_Ok;

    BDBG_MSG(("createInfo(%s, %s, %s)", mediafile, indexfile, path));
    FILE * fin = fopen64(strPath + MString("/") + strMediaFile, "rb");
    if (!fin)
    {
        BDBG_ERR(("can't open media file '%s' (path:%s)", strMediaFile.s(), strPath.s()));
        return(eRet_ExternalError);
    }

    probe = bmedia_probe_create();
    BDBG_ASSERT(probe);

    /* FUNCTION BELOW is from SETTOP API */
    fd = bfile_stdio_read_attach(fin);

    bmedia_probe_default_cfg(&probe_config);
    probe_config.file_name   = strMediaFile.s();
    probe_config.type        = bstream_mpeg_type_unknown;
    probe_config.probe_index = true;
    stream                   = bmedia_probe_parse(probe, fd, &probe_config);

    /* now stream is either NULL, or stream descriptor with linked list of audio/video tracks */
    bfile_stdio_read_detach(fd);

    if (NULL == stream)
    {
        BDBG_WRN(("can't parse stream '%s' (path:%s)", strMediaFile.s(), strPath.s()));
        retVal = eRet_ExternalError;
        goto error;
    }

    /* media file probed */

    pVideo = new CVideo(_pCfg, strMediaFile.s(), strIndexFile.s(), strPath.s());
    CHECK_PTR_ERROR_GOTO("unable to allocate CVideo object", pVideo, retVal, eRet_OutOfMemory, error);

    if (bstream_mpeg_type_ts == stream->type)
    {
        if (192 == (((bmpeg2ts_probe_stream *)stream)->pkt_len))
        {
            pVideo->enableTimestamp(true);
        }
    }

    BDBG_MSG(("media file %s, index file %s index=%d", strMediaFile.s(), strIndexFile.s(), stream->index));
    if ((bmedia_probe_index_required == stream->index) ||
        (bmedia_probe_index_self == stream->index))
    {
        pVideo->setIndexRequired(true);
    }
    BDBG_MSG(("Index required: %s", pVideo->isIndexRequired() ? "true" : "false"));
    if ((bmedia_probe_index_unknown == stream->index) ||
        (bmedia_probe_index_missing == stream->index))
    {
        pVideo->setIndexName(NULL);
    }
    BDBG_MSG(("Has index: %s", pVideo->hasIndex() ? "true" : "false"));

    pVideo->setServerIndexState(eServerIndexState_Missing);
    pVideo->setDuration(stream->duration);
    pVideo->setMaxBitrate(stream->max_bitrate);

    /* populate other structure with first audio and video tracks */
    {
        unsigned prev_program  = 0;
        unsigned audio_program = 0;
        unsigned video_program = 0;

        for (track = BLST_SQ_FIRST(&stream->tracks); track; track = BLST_SQ_NEXT(track, link))
        {
            if ((track != BLST_SQ_FIRST(&stream->tracks)) && (prev_program != track->program))
            {
                if (false == pVideo->getPidMgr()->isEmpty())
                {
                    pVideo->generateInfoName(mediafile);
                    pVideo->setProgram(prev_program);
                    pVideo->updateFileSysInfo();
                    probedVideos.add(pVideo);
                    addVideo(pVideo);

                    pVideo = new CVideo(_pCfg, *pVideo); /* copy contructor */
                    BDBG_MSG(("New Video Track"));
                }

                audio_program = 0;
                video_program = 0;
            }

            prev_program = track->program;

#if B_HAS_ASF
            if (stream->type == bstream_mpeg_type_asf)
            {
                pVideo->setForPurchase(pVideo->isForPurchase() || ((basf_probe_track *)track)->encryptedContentFlag);
            }
#endif /* if B_HAS_ASF */
#if B_HAS_AVI
            if (stream->type == bstream_mpeg_type_avi)
            {
                pVideo->setForPurchase(pVideo->isForPurchase() || ((bavi_probe_track *)track)->encryptedContentFlag);
            }
#endif /* if B_HAS_AVI */
            pVideo->getPidMgr()->setTransportType(b_mpegtype2nexus(stream->type));

            switch (track->type)
            {
            case bmedia_track_type_audio:
                if (track->info.audio.codec != baudio_format_unknown /*&& audio_program < 1*/)
                {
                    /* Set the pid to audio here */
                    CPid * pid = new CPid(track->number, ePidType_Audio);
                    CHECK_PTR_ERROR_GOTO("unable to allocate CPid", pid, retVal, eRet_OutOfMemory, error);
                    BDBG_MSG(("adding audio codec:%d", track->info.audio.codec));
                    pid->setAudioCodec(bmediaToAudioCodec(track->info.audio.codec));
                    pid->setBitrate(track->info.audio.bitrate);
                    pid->setSampleRate(track->info.audio.sample_rate);
                    pid->setSampleSize(track->info.audio.sample_size);
                    pid->setAudioChannels(track->info.audio.channel_count);
                    pid->setPlayback(true);
                    pVideo->getPidMgr()->addPid(pid);
                    audio_program++;

                    BDBG_MSG(("Audio Track added pid 0x%x", pid->getPid()));
                }
                break;
            case bmedia_track_type_video:
                if (track->info.video.codec != bvideo_codec_unknown)
                {
                    CPid * pid = new CPid(track->number, ePidType_Video);
                    CHECK_PTR_ERROR_GOTO("unable to allocate CPid", pid, retVal, eRet_OutOfMemory, error);
                    pid->setVideoCodec(bmediaToVideoCodec(track->info.video.codec));
                    pid->setBitrate(track->info.video.bitrate);
                    pid->setWidth(track->info.video.width);
                    pid->setHeight(track->info.video.height);
#if B_HAS_ASF
                    if (stream->type == bstream_mpeg_type_asf)
                    {
                        NEXUS_VideoFrameRate videoFrameRate;
                        basf_probe_track *   asf_track = (basf_probe_track *)track;
                        if (asf_track->averageTimePerFrame)
                        {
                            uint64_t framerate = 10*(1000000000/asf_track->averageTimePerFrame);
                            NEXUS_LookupFrameRate((unsigned)framerate, &videoFrameRate);
                            pid->setVideoFrameRate(videoFrameRate);
                        }
                    }
#endif /* if B_HAS_ASF */
#if B_HAS_AVI
                    if (stream->type == bstream_mpeg_type_avi)
                    {
                        NEXUS_VideoFrameRate videoFrameRate;
                        NEXUS_LookupFrameRate((unsigned)((bavi_probe_stream *)stream)->video_framerate, &videoFrameRate);
                        pid->setVideoFrameRate(videoFrameRate);
                    }
#endif /* if B_HAS_AVI */
                    pid->setPlayback(true);
                    pVideo->getPidMgr()->addPid(pid);
                    video_program++;

                    pVideo->setWidth(track->info.video.width);
                    pVideo->setHeight(track->info.video.height);
                    BDBG_MSG(("Video Track added pid 0x%x", pid->getPid()));
                }
                break;
            case bmedia_track_type_pcr:
            {
                CPid * pid = new CPid(track->number, ePidType_Pcr);
                CHECK_PTR_ERROR_GOTO("unable to allocate CPid", pid, retVal, eRet_OutOfMemory, error);
                pid->setPlayback(true);
                pVideo->getPidMgr()->addPid(pid);

                BDBG_MSG(("PCR Track added pid 0x%x", pid->getPid()));
            }
            break;
            default:
                break;
            } /* switch */
        }

        if (false == pVideo->getPidMgr()->isEmpty())
        {
            pVideo->generateInfoName(strMediaFile.s());
            pVideo->setProgram(prev_program);
            pVideo->updateFileSysInfo();

            probedVideos.add(pVideo);

            if (true == bAddToVideoList)
            {
                addVideo(pVideo);
            }
        }

        if ((false == bAddToVideoList) || (true == pVideo->getPidMgr()->isEmpty()))
        {
            DEL(pVideo);
        }
    }

    if (0 < probedVideos.total())
    {
        retVal = writeInfo(&probedVideos);
    }

error:
    if (NULL != stream)
    {
        bmedia_probe_stream_free(probe, stream);
    }

    if (NULL != probe)
    {
        bmedia_probe_destroy(probe);
    }

    if (NULL != fin)
    {
        fclose(fin);
        fin = NULL;
    }
    /* CVideo is put in the list then freed when list is destroyed */
    /* coverity[resource_leak] */
    return(retVal);
} /* createInfo */

/* This function just creates the nfo file based on a video that is complete */
void CPlaybackList::createInfo(
        CVideo *  pVideo,
        CPidMgr * pPidMgr,
        bool      bAddToVideoList
        )
{
    MList<CVideo> videoList;
    FILE *        fin = NULL;

    BDBG_ASSERT(NULL != pVideo);
    fin = fopen64(pVideo->getVideoNamePath(), "rb");
    if (!fin)
    {
        BDBG_ERR(("can't open media file '%s'", (pVideo->getVideoNamePath()).s()));
        goto error;
    }

    if (true == bAddToVideoList)
    {
        videoList.add(pVideo);
    }

    if (pVideo->getVideoName().isEmpty())
    {
        goto error;
    }

    writeInfo(&videoList, pPidMgr);
    goto done;

error:
    BDBG_ERR((" Incomplete Video object , cannot create a nfo file!"));
done:
    if (NULL != fin)
    {
        fclose(fin);
        fin = NULL;
    }
} /* createInfo */

void CPlaybackList::addVideo(
        CVideo * video,
        int      index
        )
{
    video->dump();
    {
        CScopedMutex observerListMutex(_mutexPlaylist);

        if (index > -1)
        {
            _videos.insert(index, video);
        }
        else
        {
            BDBG_MSG(("Adding Video"));
            _videos.add(video);
        }
    }
} /* addVideo */

void CPlaybackList::removeVideo(CVideo * video)
{
    BDBG_MSG(("Removing Video"));
    video->dump();
    {
        CScopedMutex observerListMutex(_mutexPlaylist);
        _videos.remove(video);
    }
}

void CPlaybackList::removeVideo(int index)
{
    MListItr <CVideo> itrVideo(&_videos);
    CVideo *          video;

    BDBG_MSG(("Removing Video"));
    {
        CScopedMutex observerListMutex(_mutexPlaylist);

        video = itrVideo.at(index);
        if (video != NULL)
        {
            BDBG_MSG(("Removing Video"));
            _videos.remove(video);
            return;
        }
    }

    BDBG_MSG(("Video at index %d not found", index));
} /* removeVideo */

void CPlaybackList::dump(bool bForce)
{
    CVideo *   video;
    BDBG_Level level;

    MListItr <CVideo> itrVideo(&_videos);

    if (true == bForce)
    {
        BDBG_GetModuleLevel("atlas_videolist", &level);
        BDBG_SetModuleLevel("atlas_videolist", BDBG_eMsg);
    }

    for (video = itrVideo.first(); video; video = itrVideo.next())
    {
        video->dump(bForce);
    }

    if (true == bForce)
    {
        BDBG_SetModuleLevel("atlas_videolist", level);
    }
} /* dump */

void CPlaybackList::sync(void)
{
    notifyObservers(eNotify_PlaybackListChanged, this);
}

void CPlaybackList::generateVideoName(MString path)
{
    MString      atlas;
    MString      name;
    MStringList  atlasList;
    int          number               = 1;
    const char * cstrDefaultVideoName = "Atlas";
    const int    nDefaultVideoNameLen = 5;

    const char * dirstr = path.s();
    DIR *        dir    = opendir((char *) dirstr);

    if (!dir)
    {
        BDBG_WRN(("Directory '%s' not found", dirstr));
        return;
    }

    struct dirent * d;

    while ((d = readdir(dir)))
    {
        if ((strcmp(d->d_name, ".") == 0) || (strcmp(d->d_name, "..") == 0))
        {
            continue; /* skip "." and ".." */
        }
        name = d->d_name;
        BDBG_MSG(("name file %s", name.s()));

        /* Atlas must be at the beginning of the FileName*/
        if (name.strncmp(cstrDefaultVideoName, nDefaultVideoNameLen) != 0)
        {
            continue;
        }
        atlas = name;
        BDBG_MSG(("name file after compare %s", atlas.s()));
        atlas.truncate(atlas.findRev("."));
        BDBG_MSG(("filename after cut of .mpg/nav/etc %s", atlas.s()));

        atlas = atlas.mid(nDefaultVideoNameLen);
        BDBG_MSG(("Atlas now should be a number %s, number in int %d", atlas.s(), atlas.toInt()));

        if (number <= atlas.toInt())
        {
            number = atlas.toInt()+1;
        }
    }

    BDBG_MSG(("new Number is %d", number));
    _nextVideoNumber = number;
    closedir(dir);
} /* generateVideoName */

CVideo * CPlaybackList::getVideo(uint32_t index)
{
    CScopedMutex observerListMutex(_mutexPlaylist);

    return(_videos.at(index));
}

MString CPlaybackList::getNextAtlasName()
{
    MString atlasName;

    atlasName = "Atlas";
    atlasName = atlasName+MString(_nextVideoNumber)+MString(".mpg");
    _nextVideoNumber++;
    return(atlasName);
}

static int compare(
        CVideo * vid1,
        CVideo * vid2
        )
{
    MString strVid1;
    MString strVid2;

    BDBG_ASSERT(NULL != vid1);
    BDBG_ASSERT(NULL != vid2);

    strVid1 = (vid1->getVideoName() + MString("_") + MString(vid1->getProgram())).s();
    strVid2 = (vid2->getVideoName() + MString("_") + MString(vid2->getProgram())).s();

    return(strVid1.strncasecmp(strVid2));
}

/* copy given directory entry names matching strExtension into pStrList.
 * strExtension can be NULL if no extension matching is required. */
void CPlaybackList::directoryToStringList(
        const char *  strDir,
        const char *  strExtension,
        MStringList * pStrList
        )
{
    DIR *           dir = opendir((char *) strDir);
    struct dirent * d   = NULL;

    BDBG_ASSERT(NULL != strDir);
    BDBG_ASSERT(NULL != pStrList);

    if (!dir)
    {
        BDBG_WRN(("Directory '%s' not found", strDir));
        return;
    }

    while ((d = readdir(dir)))
    {
        MString strDirEntryName;
        int     indexDot = 0;

        strDirEntryName = d->d_name;

        if ((strDirEntryName == ".") || (strDirEntryName == ".."))
        {
            continue; /* skip "." and ".." */
        }

        indexDot = strDirEntryName.findRev('.');

        if ((-1 == indexDot) || (NULL == strExtension) || (MString(strExtension) == strDirEntryName.mid(indexDot)))
        {
            /* add directory entry name to string list if it does not have an extension,
             * no extension match is specified, or if extension equals given extension match */
            pStrList->add(strDirEntryName.s());
        }
    }
    closedir(dir);
    dir = NULL;
} /* directoryToStringList */

void CPlaybackList::refreshFromDisk()
{
    MListItr <CVideo> itrVideo(&_videos);
    MString           name;
    CVideo *          pVideo = NULL;

    MStringList infoList;
    MStringList mediaList;
    MStringList indexList;

    MString pathMedia = GET_STR(_pCfg, VIDEOS_PATH);
    MString pathInfo  = GET_STR(_pCfg, INFO_PATH);
    MString pathIndex = GET_STR(_pCfg, INDEX_PATH);

    MString infoName;
    MString mediaName;
    MString indexName;

    ATLAS_MEMLEAK_TRACE("BEGIN");

    if (NULL != _generateIndexesThread_handle)
    {
        destroyThreadGenerateIndexes();
    }

    /* skip the following extensions when searching for media files */
    MStringList listIgnoreExtensions;
    listIgnoreExtensions.add("nfo");
    listIgnoreExtensions.add("info");
    listIgnoreExtensions.add("log");
    listIgnoreExtensions.add("txt");
    listIgnoreExtensions.add("xml");
    listIgnoreExtensions.add("nav");
    listIgnoreExtensions.add("tmp");
    listIgnoreExtensions.add("m3u8");

    {
        CScopedMutex observerListMutex(_mutexPlaylist);

        if (!pathMedia && (_videos.total() == 0))
        {
            BDBG_MSG((" First Entry , List is empty"));
        }
        else
        {
            BDBG_MSG((" Empty List"));
            for (pVideo = _videos.first(); pVideo != NULL; pVideo = _videos.next())
            {
                pVideo->closeVideo();
                DEL(pVideo);
            }
            _videos.clear();
        }
    }

    generateVideoName(pathMedia);

    directoryToStringList(pathMedia, NULL, &mediaList);
    directoryToStringList(pathInfo, ".nfo", &infoList);
    directoryToStringList(pathIndex, ".nav", &indexList);

    /* Filter out ALL  nonrecognized media File  extensions */
    {
        MString strExtension;

        mediaName = mediaList.first();
        while (mediaName)
        {
            for (strExtension = listIgnoreExtensions.first(); strExtension; strExtension = listIgnoreExtensions.next())
            {
                MString temp = mediaName.mid(mediaName.findRev(".")+1, strExtension.length());
                BDBG_MSG(("Comparing %s, %s, media . @%d", strExtension.s(), mediaName.s(), mediaName.findRev(".")));
                BDBG_MSG(("mediaName extension %s ", temp.lower().s()));
                if (temp.lower() == strExtension)
                {
                    BDBG_MSG(("Match %s, %s, media . @%d", strExtension.s(), mediaName.s(), mediaName.findRev(".")));
                    break;
                }
                /* TODO create a function that checks to see what atlas# there is so we can start recording after that number example: atlas1.mpg,atlas.mpg next
                 * update the counter to atlas3.mpg */
            }

            if (NULL != strExtension)
            {
                BDBG_MSG(("Remove %s", mediaName.s()));
                mediaList.remove(mediaName);
                mediaName = mediaList.current();
            }
            else
            {
                mediaName = mediaList.next();
            }
        }
    }

    /*
     * All files are now in 3 lists,Valid media Files, info Files, index Files.
     * create video object for each program
     */
    for (mediaName = mediaList.first(); (false == mediaName.isEmpty()); mediaName = mediaList.next())
    {
        for (infoName = infoList.first(); (false == infoName.isEmpty()); infoName = infoList.next())
        {
            /* compare mediaName and infoName sans extension */
            if (mediaName.left(mediaName.findRev('.')) == infoName.left(infoName.findRev('.')))
            {
                /* found matching info file */
                if (eRet_Ok == readInfo(infoName.s(), pathInfo.s()))
                {
                    break;
                }
                else
                {
                    BDBG_WRN(("XML version mismatch for file:%s", infoName.s()));
                }
            }
        }

        if (infoName.isEmpty())
        {
            /* no matching info file found so create one */

            for (indexName = indexList.first(); (false == indexName.isEmpty()); indexName = indexList.next())
            {
                /* compare mediaName and indexName sans extension */
                if (mediaName.left(mediaName.findRev('.')) == indexName.left(indexName.findRev('.')))
                {
                    struct stat st;

                    /* found matching index file */

                    /* if index file isn't empty then it is valid */
                    stat(indexName.s(), &st);
                    if (0 < st.st_size)
                    {
                        break;
                    }
                }
            }

            if (indexName.isEmpty())
            {
                /* no matching index file */
                indexName = mediaName;
            }

            createInfo(mediaName.s(), indexName.s(), pathMedia.s());
        }
    }

    {
        CScopedMutex observerListMutex(_mutexPlaylist);

        if (0 < _videos.total())
        {
            _videos.sort(compare);
        }
    }

    /* determine if index nav files need to be generated */
    {
        MString strIndexName;
        int     indexDot = 0;

        CScopedMutex observerListMutex(_mutexPlaylist);

        for (CVideo * pVideo = _videos.first(); pVideo; pVideo = _videos.next())
        {
            if ((NEXUS_TransportType_eTs != pVideo->getPidMgr()->getTransportType()) ||
                (false == pVideo->isIndexRequired()))
            {
                /* no index needed */
                continue;
            }

            strIndexName = pVideo->getIndexName();
            if (true == strIndexName.isEmpty())
            {
                /* no index needed */
                continue;
            }

            indexDot = strIndexName.findRev('.');
            if (-1 < indexDot)
            {
                if (MString(".nav") == strIndexName.mid(indexDot))
                {
                    MString indexName;

                    /* verify index file exists on disk */
                    for (indexName = indexList.first(); (false == indexName.isEmpty()); indexName = indexList.next())
                    {
                        if (strIndexName == indexName)
                        {
                            /* nav already exists */
                            break;
                        }
                    }

                    if (false == indexName.isEmpty())
                    {
                        /* nav found */
                        continue;
                    }
                }
            }

            pVideo->setIndexGenerationNeeded(true);

            BDBG_WRN(("nav needed %s transportType:%s isIndexRequired():%s (%s)",
                      pVideo->getVideoName().s(),
                      transportTypeToString(pVideo->getPidMgr()->getTransportType()).s(),
                      pVideo->isIndexRequired() ? "true" : "false",
                      pVideo->getIndexName().s()));
        }
    }

    /* spawn thread to generate indexes for new video list if necessary */
    {
        eRet ret = generateIndexes();
        CHECK_ERROR("unable to generate nav indexes", ret);
    }

    ATLAS_MEMLEAK_TRACE("END");

    notifyObservers(eNotify_PlaybackListChanged, this);
    dump();
} /* refreshFromDisk */

void CPlaybackList::destroyThreadGenerateIndexes()
{
    if (NULL != _generateIndexesThread_handle)
    {
        _stopGenerateIndexes = true;

        B_Thread_Destroy(_generateIndexesThread_handle);
        _generateIndexesThread_handle = NULL;
    }

    return;
} /* destroyThreadGenerateIndexes */

void CPlaybackList::doGenerateIndexes()
{
    CVideo * pVideo = NULL;

    MListItr<CVideo> itr(&_videos);

    BDBG_MSG(("start generating indexes number:%d", _videos.total()));
    {
        uint32_t index = 0;
        while (NULL != (pVideo = getVideo(index++)))
        {
            if (true == _stopGenerateIndexes)
            {
                break;
            }

            if (true == pVideo->isIndexGenerationNeeded())
            {
                pVideo->generateIndex();

                /* after generating a new index we will remove the nfo file.
                 * the nfo file will be regenerated the next time atlas runs.
                 * since the video list in memory has the correct information,
                 * atlas can run just fine until restarted. */
                remove(pVideo->getInfoNamePath());
            }
        }
    }
    BDBG_MSG(("done generating indexes"));
} /* doGenerateIndexes */

static void generateIndexesThread(void * pParam)
{
    CPlaybackList * pPlaybackList = (CPlaybackList *)pParam;

    BDBG_ASSERT(NULL != pPlaybackList);

    pPlaybackList->doGenerateIndexes();
}

eRet CPlaybackList::generateIndexes()
{
    eRet ret = eRet_Ok;

    if (false == GET_BOOL(_pCfg, GENERATE_INDEXES))
    {
        return(ret);
    }

    if (NULL != _generateIndexesThread_handle)
    {
        destroyThreadGenerateIndexes();
    }

    _stopGenerateIndexes = false;

    _generateIndexesThread_handle = B_Thread_Create("CPlaybackListGenerateIndexesThread", generateIndexesThread, this, NULL);
    CHECK_PTR_ERROR_GOTO("thread creation failed!", _generateIndexesThread_handle, ret, eRet_ExternalError, error);

error:
    return(ret);
} /* generateIndexes */
