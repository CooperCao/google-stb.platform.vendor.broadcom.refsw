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

#include "channelmgr.h"

#include "mxmlparser.h"
#include "mxmlelement.h"
#include "channel.h"
#include "channel_sat.h"
#include "channel_qam.h"
#include "channel_vsb.h"
#include "channel_ofdm.h"
#include "channel_streamer.h"
#include "channel_bip.h"
#include "channel_playback.h"
#include "channel_mosaic.h"

#include <unistd.h> /* file i/o */
#include <fcntl.h>

BDBG_MODULE(atlas_channelmgr);

CChannel * createChannel(
        CConfiguration * pCfg,
        MXmlElement *    xmlElemChannel
        )
{
    eRet       ret = eRet_Ok;
    CChannel * pCh = NULL;

    BDBG_ASSERT(NULL != pCfg);
    BDBG_ASSERT(NULL != xmlElemChannel);

    if (xmlElemChannel->tag() != XML_TAG_CHANNEL)
    {
        BDBG_WRN((" Tag is not Channel it is %s", xmlElemChannel->tag().s()));
        return(pCh);
    }

    MString strName = xmlElemChannel->attrValue(XML_ATT_TYPE);
    BDBG_MSG(("XML_ATT_TYPE : %s", strName.s()));

#if NEXUS_HAS_FRONTEND
    if (strName == "sds")
    {
        pCh = new CChannelSat(pCfg);
        CHECK_PTR_ERROR_GOTO("Error allocating Sat channel", pCh, ret, eRet_OutOfMemory, error);
    }
    else
    if (strName == "qam")
    {
        pCh = new CChannelQam(pCfg);
        CHECK_PTR_ERROR_GOTO("Error allocating QAM channel", pCh, ret, eRet_OutOfMemory, error);
    }
    else
    if (strName == "vsb")
    {
        pCh = new CChannelVsb(pCfg);
        CHECK_PTR_ERROR_GOTO("Error allocating VSB channel", pCh, ret, eRet_OutOfMemory, error);
    }
    else
    if (strName == "ofdm")
    {
        pCh = new CChannelOfdm(pCfg);
        CHECK_PTR_ERROR_GOTO("Error allocating Ofdm channel", pCh, ret, eRet_OutOfMemory, error);
    }
    else
#endif /* NEXUS_HAS_FRONTEND */
#ifdef PLAYBACK_IP_SUPPORT
    if (strName == "ip")
    {
        pCh = new CChannelBip(pCfg);
        CHECK_PTR_ERROR_GOTO("Error allocating BIP channel", pCh, ret, eRet_OutOfMemory, error);
    }
    else
#endif /* ifdef PLAYBACK_IP_SUPPORT */
    if (strName == "playback")
    {
        pCh = new CChannelPlayback(pCfg);
        CHECK_PTR_ERROR_GOTO("Error allocating Playback channel", pCh, ret, eRet_OutOfMemory, error);
    }
    else
    if (strName == "streamer")
    {
        pCh = new CChannelStreamer(pCfg);
        CHECK_PTR_ERROR_GOTO("Error allocating Streamer channel", pCh, ret, eRet_OutOfMemory, error);
    }
    else
    if (strName == "mosaic")
    {
        pCh = new CChannelMosaic(pCfg);
        CHECK_PTR_ERROR_GOTO("Error allocating Mosaic channel", pCh, ret, eRet_OutOfMemory, error);
    }
    else
    {
        BDBG_WRN(("Unhandled channel type:%s - skipping", strName.s()));
        /*
         * Need to fix this all channel false in this else statement
         * goto error;
         */
    }

error:
    return(pCh);
} /* createChannel */

#ifdef MPOD_SUPPORT
static BKNI_EventHandle scteEvent;
/* #define CHANNEL_MGR_DEBUG */
#define EPOCH_DIFF  315964819 /*difference between UNIX and GPS epoch*/
void CChannelMgr::sttCallback(
        unsigned long t,
        bool          dst
        )
{
    int         ret;
    time_t      sys_time, curr_time;
    struct tm * timeinfo;
    int         diff;

    BSTD_UNUSED(dst);
    time(&curr_time);
    sys_time = t + EPOCH_DIFF;
    diff     = curr_time-sys_time;
    if ((diff <= 1) && (diff >= -1)) { return; }
    ret = stime(&sys_time);
    if (!ret)
    {
        BDBG_WRN((" Linux System Time Updated from Cablecard  diff is %d", diff));
        time(&sys_time);
        timeinfo = localtime(&sys_time);
        BDBG_MSG((" year %d month %d day %d hour %d  min %d sec %d zone %s isdst %d",
                  timeinfo->tm_year, timeinfo->tm_mon, timeinfo->tm_mday,
                  timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec,
                  timeinfo->tm_zone, timeinfo->tm_isdst));
    }
    return;
} /* sttCallback */

void CChannelMgr::easCallback(EA_MSG_INFO * p_ea_msg)
{
    BSTD_UNUSED(p_ea_msg);
    BDBG_WRN(("EAS message received"));
}

void CChannelMgr::printChannelMap(channel_list_t * list)
{
    int              i;
    channel_info_t * channelInfo;

    for (i = 0; i < list->active_channels; i++)
    {
        channelInfo = &list->channel[i];
        BDBG_MSG(("Broadcast Channel #%d:\n", i));
        BDBG_MSG(("\tservice_media  = 0x%08x\n", channelInfo->service_media));
        BDBG_MSG(("\tannex          = %d\n", channelInfo->annex));
        BDBG_MSG(("\tmodulation     = %d\n", channelInfo->modulation));
        BDBG_MSG(("\tfrequency      = %d\n", channelInfo->frequency));
        BDBG_MSG(("\tsymbolrate     = %d\n", channelInfo->symbolrate));
    }
}

int CChannelMgr::updateScteMap()
{
    unsigned       numPrograms;
    channel_list_t tmpChannelList;

    if (SCTE_Api_Check_Chan_Map())
    {
        return(-1);
    }
    channel_list_init(&tmpChannelList, MAX_CHANNEL_MAP);
    SCTE_Api_Get_Chan_Map(&tmpChannelList, &numPrograms);
    if (channel_list_compare(&channelList, &tmpChannelList) == 0)
    {
        channel_list_destroy(&tmpChannelList);
        BDBG_MSG(("##### discard SCTE65 channle map %d entries", numPrograms));
        return(2);
    }

    BDBG_MSG(("##### updating SCTE65 channel map Total %d entries... ", numPrograms));
    if (numPrograms)
    {
        channel_list_copy(&channelList, &tmpChannelList);
        channel_list_destroy(&tmpChannelList);
    }
    return(1);
} /* updateScteMap */

void CChannelMgr::scteEventCallback(
        void * context,
        int    param
        )
{
    BSTD_UNUSED(param);
    BSTD_UNUSED(context);
    BKNI_SetEvent(scteEvent);
}

void * CChannelMgr:: mpegSectionHandler(void * ctx)
{
    int           ret;
    CChannelMgr * pChannelMgr = (CChannelMgr *) ctx;

    while (1)
    {
        BKNI_WaitForEvent(scteEvent, BKNI_INFINITE);
        if (pChannelMgr->mpegSectionThreadExit)
        {
            BDBG_ERR(("exit mpegSectionHandler thread"));
            break;
        }
        BDBG_MSG(("##### Process SCTE65 data ...\n"));
        pChannelMgr->siLen = cablecard_get_mpeg_section(cablecard_get_instance(), pChannelMgr->siBuffer, MAX_MPEG_SECTION_LEN);
        SCTE_Process_Data(pChannelMgr->siBuffer, pChannelMgr->siLen);
        ret = pChannelMgr->updateScteMap();
        if (ret == 2)
        {
            CWidgetEngine * pWidgetEngine = pChannelMgr->getWidgetEngine();
            /* sync with bwin loop */
            if (NULL != pWidgetEngine)
            {
                pWidgetEngine->syncCallback(pChannelMgr, CALLBACK_CHANNEL_MAP_UPDATE);
            }
            pChannelMgr->_scanData._useCableCardSiData  = true;
            pChannelMgr->_scanData._pChannelList        = &pChannelMgr->channelList;
            pChannelMgr->_scanData._appendToChannelList = false;
            /* sync with bwin loop */
            if (NULL != pWidgetEngine)
            {
                pWidgetEngine->syncCallback(pChannelMgr, CALLBACK_CHANNEL_SCAN_START);
            }
            break;
        }
    }
    return(NULL);
} /* mpegSectionHandler */

#endif /* ifdef MPOD_SUPPORT */

CChannelMgr::CChannelMgr() :
    CMvcModel("CChannelMgr"),
    _pCfg(NULL),
    _pModel(NULL)
#ifdef MPOD_SUPPORT
    , mpegSectionThread(0),
    mpegSectionThreadExit(false),
    siBuffer(NULL),
    siLen(0),
    clearChannelMap(false)
#endif /* ifdef MPOD_SUPPORT */
{
    _mutex = B_Mutex_Create(NULL);
    BDBG_ASSERT(_mutex);

#ifdef MPOD_SUPPORT
    memset(&channelList, 0, sizeof(channel_list_t));
#endif /* ifdef MPOD_SUPPORT */
}

#ifdef MPOD_SUPPORT
void CChannelMgr::initialize()
{
    CChannelMgr * pChannelMgr = this;
    SI_Callback   siCallback  = { &CChannelMgr::sttCallback, &CChannelMgr::easCallback };

    SI_Init(&siCallback);
    channel_list_init(&channelList, MAX_CHANNEL_MAP);
    BKNI_CreateEvent(&scteEvent);
    siBuffer = (unsigned char *)malloc(MAX_MPEG_SECTION_LENGTH);
    pthread_create(&mpegSectionThread, NULL, &CChannelMgr::mpegSectionHandler, (void *)pChannelMgr);
    mpegSectionThreadExit = false;
    return;
}

void CChannelMgr::unInitialize()
{
    mpegSectionThreadExit = true;
    BKNI_SetEvent(scteEvent);
    pthread_join(mpegSectionThread, NULL);
    channel_list_destroy(&channelList);
    SI_UnInit();
    BKNI_DestroyEvent(scteEvent);
    free(siBuffer);
    return;
}

#endif /* ifdef MPOD_SUPPORT */

eRet CChannelMgr::registerObserver(
        CObserver *   observer,
        eNotification notification
        )
{
    eRet ret = eRet_Ok;

    CChannel * pChannel = NULL;

    CMvcModel::registerObserver(observer, notification);

    /* serialized access to _channelList */
    CScopedMutex channelListMutex(_mutex);

    for (int winType = 0; winType < eWindowType_Max; winType++)
    {
        for (pChannel = _channelList[winType].first(); pChannel; pChannel = _channelList[winType].next())
        {
            /* propogate observer registration to all channels */
            pChannel->registerObserver(observer, notification);
        }
    }

    return(ret);
} /* registerObserver */

eRet CChannelMgr::clearChannelList()
{
    eRet ret = eRet_Ok;

    /* serialized access to _channelListMain/Pip */
    CScopedMutex channelListMutex(_mutex);

    /* clear all channels from list (auto deletes data) */
    for (int winType = 0; winType < eWindowType_Max; winType++)
    {
        _channelList[winType].clear();
    }

    return(ret);
}

uint32_t CChannelMgr::filterChannel(CChannel * pChannel)
{
    uint32_t ret = CHMGR_FILTER_PASS;

    if ((false == GET_BOOL(_pCfg, SCAN_ENCRYPTED_CHANNELS)) &&
        (true == pChannel->isEncrypted()))
    {
        BDBG_WRN(("ignore encrypted channel"));
        ret |= CHMGR_FILTER_FAIL_ENCRYPTED;
    }

    if (NULL == pChannel->getPid(0, ePidType_Pcr))
    {
        BDBG_WRN(("ignore channel with missing pcr pid"));
        ret |= CHMGR_FILTER_FAIL_PCR;
    }

    if ((false == GET_BOOL(_pCfg, SCAN_AUDIO_ONLY_CHANNELS)) &&
        (NULL == pChannel->getPid(0, ePidType_Video)))
    {
        BDBG_WRN(("ignore audio only channel"));
        ret |= CHMGR_FILTER_FAIL_AUDIOONLY;
    }

    if ((false == GET_BOOL(_pCfg, SCAN_DUPLICATE_CHANNELS)) &&
        (true == isDuplicate(pChannel)))
    {
        BDBG_WRN(("ignore duplicate channel"));
        pChannel->dump();
        ret |= CHMGR_FILTER_FAIL_DUPLICATE;
    }

    if ((NULL == pChannel->getPid(0, ePidType_Video)) &&
        (NULL == pChannel->getPid(0, ePidType_Audio)) &&
        (NULL != pChannel->getPid(0, ePidType_Ancillary)))
    {
        BDBG_WRN(("ignore channel with only ancillary pid"));
        ret |= CHMGR_FILTER_FAIL_ANCILONLY;
    }

    return(ret);
} /* filterChannel */

/* Add given channel to channel list.  Because the channel manager does not know what type
 * of channel is being added, the caller must allocate the proper subclass of CChannel and
 * pass it in for storage.  The channel manager will then own the channel and will handle its
 * destruction when necessary.
 */
eRet CChannelMgr::addChannel(CChannel * pChannel)
{
    CChannel * pChannelNew = NULL;

    for (int winType = 0; winType < eWindowType_Max; winType++)
    {
        if (0 == winType)
        {
            BDBG_MSG(("--- ADD NEW CHANNEL TO CH LIST ---"));
            pChannel->dump();
            BDBG_MSG(("----------------------------------"));

            pChannelNew = pChannel;
        }
        else
        {
            pChannelNew = pChannel->createCopy(pChannel);
        }

        {
            /* serialized access to _channelList */
            CScopedMutex channelListMutex(_mutex);

            _channelList[winType].add(pChannelNew);
        }

        /* copy channelmgr registered observers to new channel */
        registerObserverList(pChannelNew);
    }

    CChannelMgrListChangedData chListChangedData(pChannel, true);
    notifyObservers(eNotify_ChannelListChanged, &chListChangedData);

    return(eRet_Ok);
} /* addChannel */

/* remove all channels matching the major channel number of the given channel.
 * the given channel is NOT removed from the list. */
eRet CChannelMgr::removeOtherMajorChannels(CChannel * pChannel)
{
    CChannel * pCh = NULL;
    eRet       ret = eRet_Ok;

    /* search for matching channel in channel list */
    for (int i = 0; i < eWindowType_Max; i++)
    {
        pCh = getFirstChannel((eWindowType)i);

        while (NULL != pCh)
        {
            CChannel * pChDel = NULL;

            pChDel = pCh;
            pCh    = getNextChannel(pChDel, false);

            /* compare major channel number and make sure we don't remove given channel */
            if ((pChDel->getMajor() == pChannel->getMajor()) && (pChDel != pChannel))
            {
                /* serialized access to _channelList */
                CScopedMutex channelListMutex(_mutex);

                /* found matching major channel number - remove/free  it */
                _channelList[(eWindowType)i].remove(pChDel);
                DEL(pChDel);
            }
        }
    }

    return(ret);
} /* removeOtherMajorChannels */

/* find channel based on channel number */
CChannel * CChannelMgr::findChannel(
        const char * strChannelNum,
        eWindowType  windowType
        )
{
    unsigned   major         = 0;
    unsigned   minor         = 1;
    int        dotIndex      = 0;
    CChannel * ptr           = NULL;
    CChannel * pChannelFound = NULL;
    bool       minorMissing  = false;
    MString    strChNum(strChannelNum);

    /* set iterator to list associated with given channel list type (main/pip) */
    MListItr <CChannel> itr(getChannelList(windowType));

    BDBG_ASSERT(NULL != strChannelNum);

    dotIndex = strChNum.find('.');
    if ((0 > dotIndex) ||                      /* no dot in channel num */
        ((dotIndex + 1) == strChNum.length())) /* dot exists but no minor channel num */
    {
        major = (unsigned)strChNum.toInt();
        /* mark minor channel as missing so we will ignore it during channel search */
        minorMissing = true;
    }
    else
    {
        /* parse major/minor numbers */
        major = (unsigned)strChNum.left(dotIndex).toInt();
        minor = (unsigned)MString(strChNum.mid(dotIndex + 1)).toInt();
    }

    {
        /* serialized access to _channelList */
        CScopedMutex channelListMutex(_mutex);

        /* search for matching channel in channel list */
        for (ptr = itr.first(); ptr; ptr = itr.next())
        {
            /* compare major channel number */
            if (ptr->getMajor() == major)
            {
                /* compare minor channel number */
                if ((true == minorMissing) || (ptr->getMinor() == minor))
                {
                    break;
                }
            }
        }
    }

    if (NULL != ptr)
    {
        /* given channel found in channel list - copy data */
        pChannelFound = ptr;
    }

    return(pChannelFound);
} /* findChannel */

/* Moves channels from the given list to the actual channel list.  The actual channel
 * list will own the new channels and will delete them when necessary.  if append is true,
 * the new channels are appended to the channel list.  otherwise, the new channels replace
 * the channel list. */
eRet CChannelMgr::addChannelList(
        MList<CChannel> * pList,
        bool              append
        )
{
    eRet ret = eRet_Ok;

    if (false == append)
    {
        /* we are replacing the existing channel list - clear current list */
        clearChannelList();
    }

    /* add new channels to existing channel list (note that the allocated channels in
     * the new list are simply removed and placed in the actual channel list.
     * the actual channel list will own them and delete them when necessary.) */
    CChannel * pChannel = NULL;
    while (NULL != (pChannel = pList->remove(0)))
    {
        ret = addChannel(pChannel);
        CHECK_ERROR_GOTO("unable to add to channel list", ret, error);
    }

    if (false == append)
    {
        /* we've replaced the channel list so set the current channel to the beginning */
        notifyObservers(eNotify_ChannelListChanged);
    }

error:
    return(ret);
} /* addChannelList */

eRet CChannelMgr::loadChannelList(
        const char * listName,
        const bool   append
        )
{
    MXmlElement * xmlElemTop         = NULL;
    MXmlElement * xmlElemAtlas       = NULL;
    int           fd                 = 0;
    char *        buf                = NULL;
    int           nBufSize           = 0;
    eRet          ret                = eRet_Ok;
    int           channelNum         = 1;
    MXmlElement * xmlElemChannelList = NULL;
    MXmlParser    xmlParser;
    const int     maxBufSize = 1024 * 30;
    MString       strVersion;

    /* if we are replacing the channel list, make sure we are not tuned to any channel */
    BDBG_ASSERT((false == append) ? NULL == findTunedChannel() : true);

    fd = open(listName, O_RDONLY, 0);
    if (0 > fd)
    {
        BDBG_ERR(("Failed to open file '%s'!", listName));
        return(eRet_ExternalError);
    }

    buf = (char *)malloc(sizeof(char) * maxBufSize);
    CHECK_PTR_ERROR_GOTO("Error malloc parse buffer!", buf, ret, eRet_OutOfMemory, error);

    /* read into buffer */
    memset(buf, 0, sizeof(char) * maxBufSize);
    nBufSize            = read(fd, buf, maxBufSize - 1);
    buf[maxBufSize - 1] = '\0';
    close(fd);
    fd = 0;

    /*
     * BDBG_MSG(("----------------------------------------------------------"));
     * BDBG_MSG(("BUF:\n%s", buf));
     */

    if (nBufSize <= 0)
    {
        BDBG_WRN(("Cannot open XML channel list file: %s", listName));
        ret = eRet_ExternalError;
        goto error;
    }

    xmlElemTop = xmlParser.parse(buf);
    CHECK_PTR_ERROR_GOTO("Syntax error parsing channel list XML", xmlElemTop, ret, eRet_ExternalError, error);

    if (false == append)
    {
        clearChannelList();
    }

#if 0
    BDBG_MSG(("----------------------------------------------------------"));
    BDBG_MSG(("xmlElemTop:"));
    xmlElemTop->print();
#endif

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

                major = strVersion.left(dotIndex).toInt();
                minor = MString(strVersion.mid(dotIndex + 1)).toInt();
            }
        }

        /* compare XML version number */
        if (GET_INT(_pCfg, XML_VERSION_MAJOR_CHLIST) != major)
        {
            /* XML version mismatch! */
            ret = eRet_ExternalError;
            goto error;
        }
    }

    xmlElemChannelList = xmlElemAtlas->findChild(XML_TAG_CHANNELLIST);
    if (xmlElemChannelList)
    {
        MXmlElement * xmlElemChannel = NULL;

        /* save Channel Here */
        for (xmlElemChannel = xmlElemChannelList->firstChild();
             xmlElemChannel;
             xmlElemChannel = xmlElemChannelList->nextChild())
        {
            CChannel * pCh = NULL;
            BDBG_MSG(("XML CHANNEL channelNum %d", channelNum));

            if (xmlElemChannel)
            {
                pCh = createChannel(_pCfg, xmlElemChannel);

                if (NULL != pCh)
                {
                    pCh->readXML(xmlElemChannel);
                    addChannel(pCh);
                }
            }

            channelNum++;
            continue;
        }

channel_error:

        /* if the added channels did not contain valid major/minor channel numbers then we will
         * re-number them here*/
        {
            CChannel * pChLast = NULL;
            uint8_t    major   = 1;

            /* get the last major channel number and make sure it is non-zero */
            pChLast = getLastChannel();
            if (NULL != pChLast)
            {
                major = pChLast->getMajor();
                if (0 == major)
                {
                    major = 1;
                }
            }

            /* newly added channels may have a major channel number of 0
             * so we will update it here */
            {
                /* serialized access to _channelList */
                CScopedMutex channelListMutex(_mutex);
                CChannel *   pChannel = NULL;

                for (int winType = 0; winType < eWindowType_Max; winType++)
                {
                    for (pChannel = _channelList[winType].first(); pChannel; pChannel = _channelList[winType].next())
                    {
                        if (0 == pChannel->getMajor())
                        {
                            /* found newly added channel */
                            pChannel->setMajor(major);
                            pChannel->setMinor(1);

                            major++;
                        }
                    }
                }
            }

            sortChannelList();
        }
    }

error:
    if (fd)
    {
        close(fd);
        fd = 0;
    }

    if (xmlElemTop)
    {
        delete xmlElemTop;
        xmlElemTop = NULL;
    }

    if (buf)
    {
        free(buf);
        buf = NULL;
    }

    notifyObservers(eNotify_ChannelListChanged);
    dumpChannelList();

    return(ret);
} /* loadChannelList */

/*
 * Print the list of channels to XML, traverse the channel list
 */
eRet CChannelMgr::saveChannelList(
        const char * fileName,
        bool         append
        )
{
    CChannel *    ch                 = NULL;
    MXmlElement * xmlElemAtlas       = NULL;
    MXmlElement * xmlElemChannelList = NULL;
    MXmlElement * xmlElemChannel     = NULL;
    FILE *        file               = NULL;

    MListItr <CChannel> itr(&_channelList[eWindowType_Main]);
    eRet                ret = eRet_Ok;

    file = fopen(fileName, (append) ? "a" : "w");
    if (!file)
    {
        BDBG_ERR(("Cannot open file for writing %s,", fileName));
        return(eRet_ExternalError);
    }

    /* serialized access to _channelList */
    CScopedMutex channelListMutex(_mutex);

    xmlElemAtlas = new MXmlElement(NULL, XML_TAG_ATLAS);
    CHECK_PTR_ERROR_GOTO("unable to allocate xml element", xmlElemAtlas, ret, eRet_OutOfMemory, error);

    /* add version attribute */
    {
        MString strVersion = GET_STR(_pCfg, XML_VERSION_MAJOR_CHLIST);
        strVersion += ".";
        strVersion += GET_STR(_pCfg, XML_VERSION_MINOR_CHLIST);
        xmlElemAtlas->addAttr(XML_ATT_VERSION, strVersion.s());
    }

    xmlElemChannelList = new MXmlElement(xmlElemAtlas, XML_TAG_CHANNELLIST);
    CHECK_PTR_ERROR_GOTO("unable to allocate xml element", xmlElemChannelList, ret, eRet_OutOfMemory, error);

    for (ch = itr.first(); ch; ch = itr.next())
    {
        /* Start with first Element */
        xmlElemChannel = new MXmlElement(xmlElemChannelList, XML_TAG_CHANNEL);
        CHECK_PTR_ERROR_GOTO("unable to allocate xml element", xmlElemChannel, ret, eRet_OutOfMemory, error);
        ch->writeXML(xmlElemChannel);
    }

    xmlElemAtlas->print(file);

    if (file)
    {
        fclose(file);
    }

    BDBG_MSG(("----------------------------------------------------------"));
    BDBG_MSG(("Saved channel list to file: %s", fileName));

error:
    DEL(xmlElemAtlas);
    return(eRet_Ok);
} /* saveChannelList */

eRet CChannelMgr::verifyChannelListFile(const char * fileName)
{
    MXmlElement * xmlElemTop   = NULL;
    MXmlElement * xmlElemAtlas = NULL;
    int           fd           = 0;
    char *        buf          = NULL;
    int           nBufSize     = 0;
    eRet          ret          = eRet_Ok;
    MXmlParser    xmlParser;
    const int     maxBufSize = 1024 * 30;
    MString       strVersion;

    fd = open(fileName, O_RDONLY, 0);
    if (0 > fd)
    {
        BDBG_ERR(("Failed to open file '%s'!", fileName));
        return(eRet_ExternalError);
    }

    buf = (char *)malloc(sizeof(char) * maxBufSize);
    CHECK_PTR_ERROR_GOTO("Error malloc parse buffer!", buf, ret, eRet_OutOfMemory, error);

    /* read file into buffer */
    memset(buf, 0, sizeof(char) * maxBufSize);
    nBufSize            = read(fd, buf, maxBufSize - 1);
    buf[maxBufSize - 1] = '\0';
    close(fd);
    fd = 0;

    /*
     * BDBG_MSG(("----------------------------------------------------------"));
     * BDBG_MSG(("BUF:\n%s", buf));
     */

    if (nBufSize <= 0)
    {
        BDBG_WRN(("Cannot open XML channel list file: %s", fileName));
        ret = eRet_NotAvailable;
        goto error;
    }

    xmlElemTop = xmlParser.parse(buf);
    CHECK_PTR_ERROR_GOTO("Syntax error parsing channel list XML", xmlElemTop, ret, eRet_ExternalError, error);

#if 0
    BDBG_MSG(("----------------------------------------------------------"));
    BDBG_MSG(("xmlElemTop:"));
    xmlElemTop->print();
#endif

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

                major = strVersion.left(dotIndex).toInt();
                minor = MString(strVersion.mid(dotIndex + 1)).toInt();
            }
        }

        /* compare XML version number */
        if (GET_INT(_pCfg, XML_VERSION_MAJOR_CHLIST) != major)
        {
            /* XML version mismatch! */
            ret = eRet_ExternalError;
            goto error;
        }
    }

error:
    if (fd)
    {
        close(fd);
        fd = 0;
    }

    if (xmlElemTop)
    {
        delete xmlElemTop;
        xmlElemTop = NULL;
    }

    if (buf)
    {
        free(buf);
        buf = NULL;
    }

    if (eRet_ExternalError == ret)
    {
        /* XML syntax is incorrect or of the wrong version */
        notifyObservers(eNotify_ChannelListVersion);
    }
    return(ret);
} /* verifyChannelListFile */

/* if bForce == true then override existing debug level and force printout */
void CChannelMgr::dumpChannelList(bool bForce)
{
    /* only dumps main channel list (not pip) */
    MListItr <CChannel> itr(&_channelList[eWindowType_Main]);
    CChannel *          ptr = NULL;
    BDBG_Level          level;

    /* serialized access to _channelList */
    CScopedMutex channelListMutex(_mutex);

    for (ptr = itr.first(); ptr; ptr = itr.next())
    {
        ptr->dump(bForce);
    }
} /* dumpChannelList */

/* find any tuned channel */
CChannel * CChannelMgr::findTunedChannel()
{
    CChannel * pChannel      = NULL;
    CChannel * pChannelFound = NULL;

    /* serialized access to _channelListMain/Pip */
    CScopedMutex channelListMutex(_mutex);

    for (int winType = 0; winType < eWindowType_Max; winType++)
    {
        for (pChannel = _channelList[winType].first(); pChannel; pChannel = _channelList[winType].next())
        {
            if (true == pChannel->isTuned())
            {
                pChannelFound = pChannel;
                break;
            }
        }
    }

    return(pChannelFound);
} /* findTunedChannel */

/* find any tuned channel that matches the given tuner type */
/* dtt todo: extend this to check only a particular given tuner (so if there are 2 qam tuners,
 *           we could find the tuned channel for say the 2nd tuner only */
CChannel * CChannelMgr::findTunedChannel(eBoardResource tunerType)
{
    CChannel * pChannel      = NULL;
    CChannel * pChannelFound = NULL;

    /* serialized access to _channelListMain/Pip */
    CScopedMutex channelListMutex(_mutex);

    for (int winType = 0; winType < eWindowType_Max; winType++)
    {
        for (pChannel = _channelList[winType].first(); pChannel; pChannel = _channelList[winType].next())
        {
            if ((true == pChannel->isTuned()) && (tunerType == pChannel->getType()))
            {
                pChannelFound = pChannel;
                break;
            }
        }
    }

    return(pChannelFound);
} /* findTunedChannel */

/* note that this method does not alter the _currentChannel */
CChannel * CChannelMgr::getNextChannel(
        CChannel * pChannel,
        bool       bWrap
        )
{
    CChannel * pCh      = NULL;
    CChannel * pChFound = NULL;

    if (NULL != pChannel)
    {
        /* use channel as starting point if given */
        pCh = pChannel;
    }
    else
    {
        /* no channel given so use first channel as starting point */
        pCh = getFirstChannel();
    }

    /* serialized access to _channelList */
    CScopedMutex channelListMutex(_mutex);

    /* check for wrap in main list */
    for (int winType = 0; winType < eWindowType_Max; winType++)
    {
        if (_channelList[winType].last() == pCh)
        {
            if (true == bWrap)
            {
                /* we are at the end of the channel list, wrap to beginning */
                pChFound = _channelList[winType].first();
                break;
            }
        }

        if (0 <= _channelList[winType].index(pCh))
        {
            pChFound = _channelList[winType].next();
            break;
        }
    }

    return(pChFound);
} /* getNextChannel */

/* note that this method does not alter the _currentChannel */
CChannel * CChannelMgr::getPrevChannel(
        CChannel * pChannel,
        bool       bWrap
        )
{
    CChannel * pCh      = NULL;
    CChannel * pChFound = NULL;

    if (NULL != pChannel)
    {
        /* use channel as starting point if given */
        pCh = pChannel;
    }
    else
    {
        /* no channel given so use last channel as starting point */
        pCh = getLastChannel();
    }

    /* serialized access to _channelListMain/Pip */
    CScopedMutex channelListMutex(_mutex);

    /* check for wrap in main list */
    for (int winType = 0; winType < eWindowType_Max; winType++)
    {
        if (_channelList[winType].first() == pCh)
        {
            if (true == bWrap)
            {
                /* we are at the beginnig of the channel list, wrap to end */
                pChFound = _channelList[winType].last();
                break;
            }
        }

        if (0 < _channelList[winType].index(pCh))
        {
            pChFound = _channelList[winType].prev();
            break;
        }
    }

    return(pChFound);
} /* getPrevChannel */

CChannel * CChannelMgr::getFirstChannel(eWindowType windowType)
{
    /* serialized access to _channelListMain/Pip */
    CScopedMutex channelListMutex(_mutex);

    return(getChannelList(windowType)->first());
}

CChannel * CChannelMgr::getLastChannel(eWindowType windowType)
{
    /* serialized access to _channelListMain/Pip */
    CScopedMutex channelListMutex(_mutex);

    return(getChannelList(windowType)->last());
}

/* two channels are duplicates if either the channel contents are identical or
 * the pointer point to the same channel object */
bool CChannelMgr::isDuplicate(CChannel * pChannel)
{
    CChannel * pCh  = NULL;
    bool       bDup = false;

    /* serialized access to _channelListMain/Pip */
    CScopedMutex channelListMutex(_mutex);

    for (int winType = 0; winType < eWindowType_Max; winType++)
    {
        for (pCh = _channelList[winType].first(); pCh; pCh = _channelList[winType].next())
        {
            if (*pCh == *pChannel)
            {
                bDup = true;
                goto done;
            }
        }
    }

done:
    return(bDup);
} /* isDuplicate */

static int compareChannels(
        CChannel * ch1,
        CChannel * ch2
        )
{
    if (ch1->getMajor() > ch2->getMajor())
    {
        return(1);
    }
    else
    if (ch1->getMajor() < ch2->getMajor())
    {
        return(-1);
    }

    /* major numbers are equal */

    if (ch1->getMinor() > ch2->getMinor())
    {
        return(1);
    }
    else
    if (ch1->getMinor() < ch2->getMinor())
    {
        return(-1);
    }

    /* minor number are equal */

    return(0);
} /* compareChannels */

void CChannelMgr::sortChannelList()
{
    /* serialized access to _channelList */
    CScopedMutex channelListMutex(_mutex);

    for (int winType = 0; winType < eWindowType_Max; winType++)
    {
        _channelList[winType].sort(compareChannels);
    }
}

CChannelMgr::~CChannelMgr()
{
    if (_mutex)
    {
        B_Mutex_Destroy(_mutex);
        _mutex = NULL;
    }
}