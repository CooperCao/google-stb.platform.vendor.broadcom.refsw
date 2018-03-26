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
#include "server_udp.h"
#include "streamer_udp.h"
#include "atlas.h"
#include "atlas_cfg.h"
#include "convert.h"
#include "model.h"
#include "mxmlparser.h"
#include "mxmlelement.h"

#include <unistd.h> /* file i/o */
#include <fcntl.h>

#define CALLBACK_SERVER_REQUEST_RECVD        "callbackServerRequestRecvd"
#define CALLBACK_SERVER_STREAMER_STOP        "callbackServerStreamerStop"

#define ATLAS_BIP_SERVER_MAX_TRACKED_EVENTS  2

BDBG_MODULE(atlas_server_udp);

static int openFd(const char * udpUrlFile)
{
    int fd = -1;

    fd = open(udpUrlFile, O_RDONLY, 0);
    return(fd);
}

static void closeFd(int fd)
{
    close(fd);
}

eRet CServerUdp::readXML(
        const char *     udpUrlFile,
        CConfiguration * pCfg
        )
{
    MXmlElement * xmlElemTop         = NULL;
    MXmlElement * xmlElemUdpStreamer = NULL;
    int           fd                 = 0;
    char *        buf                = NULL;
    int           nBufSize           = 0;
    eRet          ret                = eRet_Ok;
    MXmlElement * xmlElemStream      = NULL;
    MXmlParser    xmlParser;
    const int     maxBufSize = 1024 * 30;
    MString       strVersion;

    fd = openFd(udpUrlFile);
    if (fd < 0)
    {
        /* optional */
        BDBG_MSG(("Failed to open file '%s'!", udpUrlFile));
        return(eRet_ExternalError);
    }

    buf = (char *)malloc(sizeof(char) * maxBufSize);
    CHECK_PTR_ERROR_GOTO("Error malloc parse buffer!", buf, ret, eRet_OutOfMemory, error);

    /* read into buffer */
    memset(buf, 0, sizeof(char) * maxBufSize);
    nBufSize            = read(fd, buf, maxBufSize - 1);
    buf[maxBufSize - 1] = '\0';
    closeFd(fd);
    fd = -1;

    BDBG_MSG(("----------------------------------------------------------"));
    BDBG_MSG(("BUF:\n%s", buf));

    if (nBufSize <= 0)
    {
        BDBG_WRN(("Cannot open XML channel list file: %s", udpUrlFile));
        ret = eRet_ExternalError;
        goto error;
    }

    xmlElemTop = xmlParser.parse(buf);
    CHECK_PTR_ERROR_GOTO("Syntax error parsing channel list XML", xmlElemTop, ret, eRet_ExternalError, error);

    /* check xml version number */
    {
#if 0
        BDBG_MSG(("----------------------------------------------------------"));
        BDBG_MSG(("xmlElemTop:"));
        xmlElemTop->print();
#endif

        int major = 1;
        int minor = 0;

        xmlElemUdpStreamer = xmlElemTop->findChild(XML_TAG_UDP_STREAMER);
        if (xmlElemUdpStreamer)
        {
            strVersion = xmlElemUdpStreamer->attrValue(XML_ATT_VERSION);
            if (false == strVersion.isEmpty())
            {
                unsigned dotIndex = strVersion.find('.');

                major = (unsigned)strVersion.left(dotIndex).toInt();
                minor = (unsigned)MString(strVersion.mid(dotIndex + 1)).toInt();
            }
        }

        /* compare XML version number */
        if (GET_INT(_pCfg, XML_VERSION_MAJOR_UDPLIST) != major)
        {
            /* XML version mismatch! */
            ret = eRet_ExternalError;
            goto error;
        }
    }

    xmlElemStream = xmlElemUdpStreamer->findChild(XML_TAG_STREAM);
    if (xmlElemStream)
    {
        MString url;

        /* Save URL here */
        for (xmlElemStream = xmlElemUdpStreamer->firstChild();
             xmlElemStream;
             xmlElemStream = xmlElemUdpStreamer->nextChild())
        {
            url = xmlElemStream->attrValue(XML_ATT_URL);
            BDBG_MSG(("XML CHANNEL url %s", url.s()));
            if (!url.isEmpty())
            {
                MString        interface;
                CStreamerUdp * pStreamer = NULL;

                pStreamer = new CStreamerUdp(_pCfg, this);
                CHECK_PTR_ERROR_GOTO("Unable to create CStreamerUdp object", pStreamer, ret, eRet_OutOfMemory, error);

                ret = pStreamer->open(url);
                CHECK_ERROR_GOTO("unable to open CStreamerUdp", ret, error);

                interface = xmlElemStream->attrValue(XML_ATT_INTERFACE);
                BDBG_MSG(("Interface Name %s", interface.s()));
                if (!interface.isEmpty())
                {
                    pStreamer->setInterfaceName(interface);
                }
                else
                {
                    /* default interface name */
                    pStreamer->setInterfaceName(_interfaceName);
                }

                _streamerList.add(pStreamer);
            }

            continue;
        }
    }

error:
    FRE(buf);
    if (fd >= 0)
    {
        closeFd(fd);
    }
    return(ret);
} /* readXML */

static void bwinProcessRecvdRequest(
        void *       pObject,
        const char * strCallback
        )
{
    CServer * pServer = (CServer *)pObject;

    BSTD_UNUSED(strCallback);

    pServer->processRecvdRequest();
}

static void bwinStreamerStopCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CServer * pServer = (CServer *)pObject;

    BSTD_UNUSED(strCallback);

    pServer->stopStreamer();
}

void CServerUdp::stopStreamer()
{
    CStreamerUdp * pStreamer = NULL;

    BDBG_MSG((BIP_MSG_PRE_FMT "Stopping streamer called" BIP_MSG_PRE_ARG));
    for (pStreamer = _streamerList.first(); pStreamer; pStreamer = _streamerList.next())
    {
        if (true == pStreamer->checkEndOfStreamer())
        {
            /* first remove the Streamer from the list.*/
            _streamerList.remove(pStreamer);

            BDBG_MSG((BIP_MSG_PRE_FMT "INSIDE:_streamerListTotal=%d, removed and stopping streamer ====================>%p" BIP_MSG_PRE_ARG, _streamerList.total(), (void *)pStreamer));
            pStreamer->stop();
            pStreamer->close();
            DEL(pStreamer);
        }
    }
} /* stopStreamer */

static void bipProcessRecvdRequest(
        void * context,
        int    param
        )
{
    CServer *       pServer       = (CServer *)context;
    CWidgetEngine * pWidgetEngine = pServer->getWidgetEngine();

    /* sync with bwin loop */
    BDBG_ASSERT(NULL != pWidgetEngine);
    BSTD_UNUSED(param);
    BDBG_MSG(("bip request received callback"));

    pWidgetEngine->syncCallback(pServer, CALLBACK_SERVER_REQUEST_RECVD);
}

CServerUdp::CServerUdp(CConfiguration * pCfg) :
    CServer("CServerUdp", pCfg),
    _pModel(NULL),
    _port(1234),
    _url("/"),
    _interfaceName("eth0"),
    _pacingEnabled(false),
    _serverStarted(false),
    _maxConcurrentRequestsToQueue(16),
    _persistentConnectionTimeoutInMs(5000)
{
}

CServerUdp::~CServerUdp()
{
}

eRet CServerUdp::open(
        CWidgetEngine *  pWidgetEngine,
        CConfiguration * pCfg
        )
{
    eRet    ret                = eRet_Ok;
    MString mediaDirectoryPath = GET_STR(pCfg, VIDEOS_PATH); /* we get it every time so that if pCfg changes and the path changes the we get the latest one.*/
    MString udpUrlFile         = GET_STR(pCfg, UDP_URL_LIST);

    /* call base class open first */
    ret = CServer::open(pWidgetEngine);
    CHECK_ERROR_GOTO("unable to open CServer", ret, error);

    /* to be used at a different time */
    _port                            = 8090;
    _maxConcurrentRequestsToQueue    = GET_INT(pCfg, HTTP_SERVER_MAX_CONCURRENT_REQUEST);
    _persistentConnectionTimeoutInMs = GET_INT(pCfg, HTTP_SERVER_PERSISTENT_TIMEOUT_IN_MS);
    _pacingEnabled                   = GET_BOOL(pCfg, HTTP_SERVER_ENABLE_HW_PACING);

    BDBG_MSG((BIP_MSG_PRE_FMT "mediaDirectoryPath ------------------------------------> %s" BIP_MSG_PRE_ARG, mediaDirectoryPath.s()));

    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->addCallback(this, CALLBACK_SERVER_REQUEST_RECVD, bwinProcessRecvdRequest);
        _pWidgetEngine->addCallback(this, CALLBACK_SERVER_STREAMER_STOP, bwinStreamerStopCallback);
    }

    ret = readXML(udpUrlFile.s(), pCfg);
    /* not an error to not read the udpUrlFile */
    BDBG_MSG(("unable to open UDP ServerList"));

error:
    return(ret);
} /* open */

void CServerUdp::close()
{
    if (NULL != _pWidgetEngine)
    {
        /* Will be used when we pipe in our own HTTP request for UDP/RTP */
        _pWidgetEngine->removeCallback(this, CALLBACK_SERVER_REQUEST_RECVD);
        _pWidgetEngine->removeCallback(this, CALLBACK_SERVER_STREAMER_STOP);
    }

    stop();

    /* call base class close last */
    CServer::close();
} /* close */

eRet CServerUdp::start()
{
    eRet           ret       = eRet_Ok;
    CStreamerUdp * pStreamer = NULL;
    char *         udpList   = NULL;

    /*
     * read the UDP/RTP list udpUrl.xml
     * create a new streamer object for each request. Right now its one
     * use ReadXML function
     */
    if (_streamerList.total() == 0)
    {
        BDBG_MSG((" UDP/RTP server is not active please write XML entries into udpUrl.xml"));
        ret = eRet_NotAvailable;
        goto error;
    }

    for (pStreamer = _streamerList.first(); pStreamer; pStreamer = _streamerList.next())
    {
        ret = pStreamer->start();
        CHECK_ERROR_GOTO("unable to start CStreamerUdp", ret, error);
    }

    BDBG_MSG((BIP_MSG_PRE_FMT "CStreamerUdp %p: Streaming is started!" BIP_MSG_PRE_ARG, (void *)pStreamer));

    _serverStarted = true;
error:

    return(ret);
} /* start */

eRet CServerUdp::stop()
{
    eRet           ret       = eRet_Ok;
    CStreamerUdp * pStreamer = NULL;

    BDBG_MSG((BIP_MSG_PRE_FMT " CServerUdp %p" BIP_MSG_PRE_ARG, (void *)this));

    /* Set the Server to done */
    _serverStarted = false;

    for (pStreamer = _streamerList.first(); pStreamer; pStreamer = _streamerList.next())
    {
        /* first remove the Streamer from the list.*/
        _streamerList.remove(pStreamer);
        pStreamer->stop();
        pStreamer->close();

        DEL(pStreamer);
    }

    /*error:*/
    return(ret);
} /* stop */

void CServerUdp::dump(bool bForce)
{
    BDBG_Level level;
    int        i;

    if (true == bForce)
    {
        BDBG_GetModuleLevel("atlas_server_udp", &level);
        BDBG_SetModuleLevel("atlas_server_udp", BDBG_eMsg);
    }

    {
        MListItr <CStreamerUdp> itrStreamers(&_streamerList);
        CStreamerUdp *          pStreamer = NULL;
        for (pStreamer = itrStreamers.first(), i = 0; pStreamer; pStreamer = itrStreamers.next(), i++)
        {
            BDBG_MSG((BIP_MSG_PRE_FMT "_streamerListTotal=%d, streamerIndex=%d, streamer ++++++++++++++++++++>%p" BIP_MSG_PRE_ARG, _streamerList.total(), i, (void *)pStreamer));
        }
    }

    if (true == bForce)
    {
        BDBG_SetModuleLevel("atlas_server_udp", level);
    }
} /* dump */