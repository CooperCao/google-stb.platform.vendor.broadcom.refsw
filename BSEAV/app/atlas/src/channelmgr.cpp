/***************************************************************************
 * (c) 2002-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
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
 *
 *****************************************************************************/

#include "channelmgr.h"

#include "mxmlparser.h"
#include "mxmlelement.h"
#include "channel_sat.h"
#include "channel_qam.h"
#include "channel_vsb.h"
#include "channel_ofdm.h"
#include "channel_streamer.h"
#include "channel_bip.h"
#include "channel_playback.h"

#include <unistd.h> /* file i/o */
#include <fcntl.h>

BDBG_MODULE(atlas_channelmgr);

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

    MListItr <CChannel> itrMain(&_channelListMain);
    MListItr <CChannel> itrPip(&_channelListPip);
    CChannel *          ptr = NULL;

    CMvcModel::registerObserver(observer, notification);

    /* serialized access to _channelList */
    CScopedMutex channelListMutex(_mutex);

    for (ptr = itrMain.first(); ptr; ptr = itrMain.next())
    {
        /* propogate observer registration to all MAIN channels */
        ptr->registerObserver(observer, notification);
    }

    for (ptr = itrPip.first(); ptr; ptr = itrPip.next())
    {
        /* propogate observer registration to all PIP channels */
        ptr->registerObserver(observer, notification);
    }

    return(ret);
} /* registerObserver */

eRet CChannelMgr::clearChannelList()
{
    eRet ret = eRet_Ok;

    /* serialized access to _channelListMain/Pip */
    CScopedMutex channelListMutex(_mutex);

    /* clear all channels from list (auto deletes data) */
    _channelListMain.clear();
    _channelListPip.clear();

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

    return(ret);
} /* filterChannel */

/* Add given channel to channel list.  Because the channel manager does not know what type
 * of channel is being added, the caller must allocate the proper subclass of CChannel and
 * pass it in for storage.  The channel manager will then own the channel and will handle its
 * destruction when necessary.
 */
eRet CChannelMgr::addChannel(CChannel * pChannel)
{
    CChannel * pChannelMain = pChannel;
    CChannel * pChannelPip  = NULL;

    {
        /* serialized access to _channelListMain/Pip */
        CScopedMutex channelListMutex(_mutex);

        BDBG_MSG(("--- ADD NEW CHANNEL TO CH LIST ---"));
        pChannelMain->dump();
        BDBG_MSG(("----------------------------------"));
        _channelListMain.add(pChannelMain);

        pChannelPip = pChannelMain->createCopy(pChannelMain);
        _channelListPip.add(pChannelPip);
    }

    /* copy channelmgr registered observers to new channel */
    registerObserverList(pChannelMain);
    registerObserverList(pChannelPip);

    CChannelMgrListChangedData chListChangedData(pChannel, true);
    notifyObservers(eNotify_ChannelListChanged, &chListChangedData);

    return(eRet_Ok);
} /* addChannel */

/* find channel based on channel number */
CChannel * CChannelMgr::findChannel(
        const char * strChannelNum,
        eWindowType  windowType
        )
{
    uint16_t   major         = 0;
    uint16_t   minor         = 1;
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
        major = (uint16_t)strChNum.toInt();
        /* mark minor channel as missing so we will ignore it during channel search */
        minorMissing = true;
    }
    else
    {
        /* parse major/minor numbers */
        major = (uint16_t)strChNum.left(dotIndex).toInt();
        minor = (uint16_t)MString(strChNum.mid(dotIndex + 1)).toInt();
    }

    {
        /* serialized access to _channelListMain/Pip */
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

static int compare(
        CChannel * ch1,
        CChannel * ch2
        )
{
    uint16_t majorCh1 = 0;
    uint16_t majorCh2 = 0;
    uint8_t  minorCh1 = 0;
    uint8_t  minorCh2 = 0;

    BDBG_ASSERT(NULL != ch1);
    BDBG_ASSERT(NULL != ch2);

    majorCh1 = ch1->getMajor();
    minorCh1 = ch1->getMinor();
    majorCh2 = ch2->getMajor();
    minorCh2 = ch2->getMinor();
    if (majorCh1 == majorCh2)
    {
        return(minorCh1 - minorCh2);
    }

    return(majorCh1 - majorCh2);
} /* compare */

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
        uint16_t major = 1;
        uint16_t minor = 0;

        xmlElemAtlas = xmlElemTop->findChild(XML_TAG_ATLAS);
        if (xmlElemAtlas)
        {
            strVersion = xmlElemAtlas->attrValue(XML_ATT_VERSION);
            if (false == strVersion.isEmpty())
            {
                uint16_t dotIndex = strVersion.find('.');

                major = (uint16_t)strVersion.left(dotIndex).toInt();
                minor = (uint16_t)MString(strVersion.mid(dotIndex + 1)).toInt();
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
                if (xmlElemChannel->tag() != XML_TAG_CHANNEL)
                {
                    BDBG_WRN((" Tag is not Channel its %s", xmlElemChannel->tag().s()));
                    continue;
                }

                MString strName = xmlElemChannel->attrValue(XML_ATT_TYPE);
                BDBG_MSG(("XML_ATT_TYPE : %s", strName.s()));

#if NEXUS_HAS_FRONTEND
                if (strName == "sds")
                {
                    pCh = new CChannelSat(_pCfg);
                    CHECK_PTR_ERROR_GOTO("Error allocating Sat channel", pCh, ret, eRet_OutOfMemory, channel_error);
                }
                else
                if (strName == "qam")
                {
                    pCh = new CChannelQam(_pCfg);
                    CHECK_PTR_ERROR_GOTO("Error allocating QAM channel", pCh, ret, eRet_OutOfMemory, channel_error);
                }
                else
                if (strName == "vsb")
                {
                    pCh = new CChannelVsb(_pCfg);
                    CHECK_PTR_ERROR_GOTO("Error allocating VSB channel", pCh, ret, eRet_OutOfMemory, channel_error);
                }
                else
                if (strName == "ofdm")
                {
                    pCh = new CChannelOfdm(_pCfg);
                    CHECK_PTR_ERROR_GOTO("Error allocating Ofdm channel", pCh, ret, eRet_OutOfMemory, channel_error);
                }
                else
#endif /* NEXUS_HAS_FRONTEND */
#ifdef PLAYBACK_IP_SUPPORT
                if (strName == "ip")
                {
                    pCh = new CChannelBip(_pCfg);
                    CHECK_PTR_ERROR_GOTO("Error allocating BIP channel", pCh, ret, eRet_OutOfMemory, channel_error);
                }
                else
#endif /* ifdef PLAYBACK_IP_SUPPORT */
                if (strName == "playback")
                {
                    pCh = new CChannelPlayback(_pCfg);
                    CHECK_PTR_ERROR_GOTO("Error allocating Playback channel", pCh, ret, eRet_OutOfMemory, channel_error);
                }
                else
                if (strName == "streamer")
                {
                    pCh = new CChannelStreamer(_pCfg);
                    CHECK_PTR_ERROR_GOTO("Error allocating Streamer channel", pCh, ret, eRet_OutOfMemory, channel_error);
                }
                else
                {
                    BDBG_WRN(("Unhandled channel type:%s - skipping", strName.s()));
                    /*
                     * Need to fix this all channel false in this else statement
                     * goto channel_error;
                     */
                }

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
                /* serialized access to _channelListMain/Pip */
                CScopedMutex        channelListMutex(_mutex);
                MListItr <CChannel> itrMain(&_channelListMain);
                MListItr <CChannel> itrPip(&_channelListPip);
                CChannel *          ptrMain = NULL;
                CChannel *          ptrPip  = NULL;

                for (ptrMain = itrMain.first(), ptrPip = itrPip.first();
                     ptrMain;
                     ptrMain = itrMain.next(), ptrPip = itrPip.next())
                {
                    if (0 == ptrMain->getMajor())
                    {
                        ptrMain->setMajor(major);
                        ptrPip->setMajor(major);
                        ptrMain->setMinor(1);
                        ptrPip->setMinor(1);
                        major++;
                    }
                }

                /* keep lists sorted by channel number */
                _channelListMain.sort(compare);
                _channelListPip.sort(compare);
            }
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

    MListItr <CChannel> itr(&_channelListMain);
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
        uint16_t major = 1;
        uint16_t minor = 0;

        xmlElemAtlas = xmlElemTop->findChild(XML_TAG_ATLAS);
        if (xmlElemAtlas)
        {
            strVersion = xmlElemAtlas->attrValue(XML_ATT_VERSION);
            if (false == strVersion.isEmpty())
            {
                uint16_t dotIndex = strVersion.find('.');

                major = (uint16_t)strVersion.left(dotIndex).toInt();
                minor = (uint16_t)MString(strVersion.mid(dotIndex + 1)).toInt();
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
    MListItr <CChannel> itr(&_channelListMain);
    CChannel *          ptr = NULL;
    BDBG_Level          level;

    /* serialized access to _channelList */
    CScopedMutex channelListMutex(_mutex);

    if (true == bForce)
    {
        BDBG_GetModuleLevel("atlas_channel", &level);
        BDBG_SetModuleLevel("atlas_channel", BDBG_eMsg);
    }

    for (ptr = itr.first(); ptr; ptr = itr.next())
    {
        ptr->dump();
    }

    if (true == bForce)
    {
        BDBG_SetModuleLevel("atlas_channel", level);
    }
} /* dumpChannelList */

/* find any tuned channel */
CChannel * CChannelMgr::findTunedChannel()
{
    MListItr <CChannel> itrMain(&_channelListMain);
    MListItr <CChannel> itrPip(&_channelListPip);
    CChannel *          ptrMain = NULL;
    CChannel *          ptrPip  = NULL;
    CChannel *          ptr     = NULL;

    /* serialized access to _channelListMain/Pip */
    CScopedMutex channelListMutex(_mutex);

    for (ptrMain = itrMain.first(), ptrPip = itrPip.first();
         ptrMain;
         ptrMain = itrMain.next(), ptrPip = itrPip.next())
    {
        if (true == ptrMain->isTuned())
        {
            ptr = ptrMain;
            break;
        }
        else
        if (true == ptrPip->isTuned())
        {
            ptr = ptrPip;
            break;
        }
    }

    return(ptr);
} /* findTunedChannel */

/* find any tuned channel that matches the given tuner type */
/* dtt todo: extend this to check only a particular given tuner (so if there are 2 qam tuners,
 *           we could find the tuned channel for say the 2nd tuner only */
CChannel * CChannelMgr::findTunedChannel(eBoardResource tunerType)
{
    MListItr <CChannel> itrMain(&_channelListMain);
    MListItr <CChannel> itrPip(&_channelListPip);
    CChannel *          ptrMain = NULL;
    CChannel *          ptrPip  = NULL;
    CChannel *          ptr     = NULL;

    /* serialized access to _channelListMain/Pip */
    CScopedMutex channelListMutex(_mutex);

    for (ptrMain = itrMain.first(), ptrPip = itrPip.first();
         ptr;
         ptrMain = itrMain.next(), ptrPip = itrPip.next())
    {
        if ((true == ptrMain->isTuned()) && (tunerType == ptrMain->getType()))
        {
            ptr = ptrMain;
            break;
        }
        else
        if ((true == ptrPip->isTuned()) && (tunerType == ptrPip->getType()))
        {
            ptr = ptrPip;
            break;
        }
    }

    return(ptr);
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

    /* serialized access to _channelListMain/Pip */
    CScopedMutex channelListMutex(_mutex);

    /* check for wrap in main list */
    if (_channelListMain.last() == pCh)
    {
        if (true == bWrap)
        {
            /* we are at the end of the channel list, wrap to beginning */
            pChFound = _channelListMain.first();
        }
    }
    else
    if (_channelListPip.last() == pCh)
    {
        if (true == bWrap)
        {
            /* we are at the end of the channel list, wrap to beginning */
            pChFound = _channelListPip.first();
        }
    }
    else
    if (0 <= _channelListMain.index(pCh))
    {
        pChFound = _channelListMain.next();
    }
    else
    if (0 <= _channelListPip.index(pCh))
    {
        pChFound = _channelListPip.next();
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

    if (_channelListMain.first() == pCh)
    {
        if (true == bWrap)
        {
            /* we are at the beginning of the channel list, wrap to end */
            pChFound = _channelListMain.last();
        }
    }
    else
    if (_channelListPip.first() == pCh)
    {
        if (true == bWrap)
        {
            /* we are at the beginning of the channel list, wrap to end */
            pChFound = _channelListPip.last();
        }
    }
    else
    if (0 <= _channelListMain.index(pCh))
    {
        pChFound = _channelListMain.prev();
    }
    else
    if (0 <= _channelListPip.index(pCh))
    {
        pChFound = _channelListPip.prev();
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
    MListItr <CChannel> itrMain(&_channelListMain);
    MListItr <CChannel> itrPip(&_channelListPip);
    CChannel *          ptrMain = NULL;
    CChannel *          ptrPip  = NULL;
    bool                bDup    = false;

    /* serialized access to _channelListMain/Pip */
    CScopedMutex channelListMutex(_mutex);

    for (ptrMain = itrMain.first(), ptrPip = itrPip.first();
         ptrMain;
         ptrMain = itrMain.next(), ptrPip = itrPip.next())
    {
        /* compare channel contents */
        if ((*ptrMain == *pChannel) || (*ptrPip == *pChannel))
        {
            bDup = true;
            break;
        }
    }

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
    /* serialized access to _channelListMain/Pip */
    CScopedMutex channelListMutex(_mutex);

    _channelListMain.sort(compareChannels);
    _channelListPip.sort(compareChannels);
}

CChannelMgr::~CChannelMgr()
{
    if (_mutex)
    {
        B_Mutex_Destroy(_mutex);
        _mutex = NULL;
    }
}
