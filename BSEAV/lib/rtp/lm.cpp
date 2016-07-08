/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

#ifdef SPF_SUPPORT
#include "bstd.h"

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "GroupsockHelper.hh"
#ifdef SPF_SUPPORT
#include "RTPInterface.hh"
#endif /* SPF_SUPPORT */


#include "brtp.h"
#include "brtp_packet.h"
#include "brtp_parser_h264.h"
#include "brtp_parser_mpeg4.h"
#include "brtp_parser_aacloas.h"
#include "brtp_parser_amr.h"
#include "brtp_util.h"

#include "lm.h"

#include "bkni.h"

#include <math.h>

BDBG_MODULE(lm);




class BRTPSource: public RTPSource {

protected:
  virtual ~BRTPSource() {
      fRTPInterface.stopNetworkReading();
  }

private:
    BRTPSource(UsageEnvironment& env, Groupsock* RTPgs,
        unsigned char rtpPayloadFormat,
        unsigned rtpTimestampFrequency,
        const lm_stream_connect_cfg *stream_cfg
        ) :
    RTPSource(env, RTPgs, rtpPayloadFormat, rtpTimestampFrequency),
    stream_cfg(*stream_cfg),
    synced(false),
    fAreDoingNetworkReads(False)
    {
        return;
    }

private:
    lm_stream_connect_cfg stream_cfg;
    bool synced;
    Boolean fAreDoingNetworkReads;
private:
    friend void networkReadHandler(BRTPSource*, int);
    static void networkReadHandler(BRTPSource* source, int /*mask*/) {
        unsigned numBytesRead;
        struct sockaddr_in fromAddress;
        uint8_t buf[2048];
        void *pkt;
                Boolean packetReadWasIncomplete = false;

        BDBG_ASSERT(source);
        BDBG_ASSERT(source->stream_cfg.rtp);
        if (!source->fRTPInterface.handleRead(buf, sizeof(buf), numBytesRead, fromAddress, packetReadWasIncomplete )) {
            return ;
        }
        BDBG_MSG(("received %#lx: %u", (unsigned long)source, numBytesRead));
        pkt = BKNI_Malloc(numBytesRead);
        if (pkt) {
            memcpy(pkt, buf, numBytesRead);
            brtp_enqueue status = source->stream_cfg.rtp_data(source->stream_cfg.rtp, pkt, numBytesRead, pkt);
            if (status!=brtp_enqueue_queued) {
                BKNI_Free(pkt);
            }
        }
        return;
    }
public:
    void pkt_free(void *pkt)
    {
        BKNI_Free(pkt);
        return;
    }

public:
    friend void SRHandler(void *);
    static void SRHandler(void* source_) {
        BRTPSource *source =  (BRTPSource *)source_;
        BDBG_ASSERT(source);


        BDBG_MSG(("SRHandler: %#lx", (unsigned long)source));
        RTPReceptionStatsDB::Iterator iter(source->receptionStatsDB());
        RTPReceptionStats *stats;

        while ((stats = iter.next(True)) != NULL) {

            BDBG_MSG(("stats: %p", (void *)stats));

            const struct timeval &fTotalInterPacketGaps =  stats->totalInterPacketGaps();
            /* hack to access private fSyncTimestamp */
            u_int32_t *fSyncTimestamp = (u_int32_t *)((uint8_t *)&fTotalInterPacketGaps + sizeof(struct timeval) + sizeof(Boolean));
            uint32_t ntp_msw = stats->lastReceivedSR_NTPmsw();
            uint32_t ntp_lsw = stats->lastReceivedSR_NTPlsw();

            if (ntp_msw==0 && ntp_lsw==0) {
                continue;
            }

            BDBG_MSG(("NTP:%p %08x:%08x TS: %08x (%#lx)", source->stream_cfg.stream_cntx, ntp_msw, ntp_lsw, *fSyncTimestamp, (unsigned long)fSyncTimestamp));

            source->stream_cfg.rtcp_sr(source->stream_cfg.stream_cntx, ntp_msw, ntp_lsw, *fSyncTimestamp);
        }
    }

public:
    // redefined virtual functions:
    virtual void doGetNextFrame() {
        if (!fAreDoingNetworkReads) {
            // Turn on background read handling of incoming packets:
            fAreDoingNetworkReads = True;
            TaskScheduler::BackgroundHandlerProc* handler
            = (TaskScheduler::BackgroundHandlerProc*)&networkReadHandler;
                fRTPInterface.startNetworkReading(handler);
        }
        return;
    }
    virtual void setPacketReorderingThresholdTime(unsigned /* uSeconds */)
    {
    }
public:
    static BRTPSource* createNew(UsageEnvironment& env, Groupsock* RTPgs,
            unsigned char rtpPayloadFormat,
            unsigned rtpTimestampFrequency,
            const lm_stream_connect_cfg *stream_cfg
    ) {
        BDBG_ASSERT(stream_cfg);
        return new BRTPSource(env, RTPgs, rtpPayloadFormat, rtpTimestampFrequency, stream_cfg);
    }
};

class BRTPFactory : public LM_RTPFactory  {
private:
    const lm_stream_connect_cfg *stream_cfg;
public:
    BRTPFactory(const lm_stream_connect_cfg *cfg) : stream_cfg(cfg) { }
    virtual RTPSource *createSource(
        UsageEnvironment& env, Groupsock* RTPgs,
        unsigned char rtpPayloadFormat,
        unsigned rtpTimestampFrequency) {
        return BRTPSource::createNew(env, RTPgs, rtpPayloadFormat, rtpTimestampFrequency, stream_cfg);
    }
};

static void sendRequestCallback(RTSPClient* context, int resultCode, char* resultString) {
    BDBG_MSG(("%s: request returned: code %d; string (%s)", __FUNCTION__, resultCode, resultString ));
    if (context) {
        if(resultString) context->fResultString = strdup(resultString); else context->fResultString = NULL;
        context->fWatchVariableForSyncInterface=1;
        context->fResultCode = resultCode;
    }

    if(resultString) delete[] resultString;
}

static char * sendDescribeCmd ( RTSPClient* rtspClient, const char * url )
{
    rtspClient->fDescribeStatusCode = 0; // BRCM: CAD 2013-10-21

    rtspClient->setBaseURL( url );
    BDBG_MSG(("%s: calling sendDescribeCommand(); RTSPClient (%p); url (%s) ", __FUNCTION__, (void *)rtspClient, rtspClient->url() ));
    rtspClient->sendDescribeCommand(sendRequestCallback, NULL );

    UsageEnvironment& env = rtspClient->envir();

    rtspClient->fWatchVariableForSyncInterface=0;
    env.taskScheduler().doEventLoop(&rtspClient->fWatchVariableForSyncInterface );

    if ( rtspClient->fResultCode == 0) return rtspClient->fResultString; // success
    if ( rtspClient->fDescribeStatusCode == 0) rtspClient->fDescribeStatusCode = 2 /* stream unavailable*/; // BRCM: CAD 2013-10-21
    BDBG_MSG(("%s: Ctx (%p), returning (%s) ", __FUNCTION__, (void *)rtspClient, rtspClient->fResultString ));
    delete[] rtspClient->fResultString;
    return NULL;
}

static int sendSetupCmd ( RTSPClient* rtspClient, MediaSubsession& subsession,
                  Boolean streamOutgoing, Boolean streamUsingTCP, Boolean forceMulticastOnUnspecified,
                  Authenticator* authenticator)
{
    BDBG_MSG(("%s: calling sendSetupCommand(session %p, Outgoing %u, UsingTCP %u, forceMulti %u, authenticator %p); ",
               __FUNCTION__, (void *)&subsession, streamOutgoing, streamUsingTCP, forceMulticastOnUnspecified, (void *)authenticator ));
    rtspClient->sendSetupCommand(subsession, sendRequestCallback,  streamOutgoing, streamUsingTCP, forceMulticastOnUnspecified, authenticator);

    UsageEnvironment& env = rtspClient->envir();

    rtspClient->fWatchVariableForSyncInterface=0;
    env.taskScheduler().doEventLoop(&rtspClient->fWatchVariableForSyncInterface );

    BDBG_MSG(("%s: Ctx (%p), returning (%s) ", __FUNCTION__, (void *)rtspClient, rtspClient->fResultString ));
    delete[] rtspClient->fResultString;
    return rtspClient->fResultCode == 0;
}

static int sendPlayCmd ( RTSPClient* rtspClient, MediaSession& session,
        double start, double end, float scale, Authenticator* authenticator )
{
    BDBG_MSG(("%s: calling sendPlayCommand(); RTSPClient (%p) ", __FUNCTION__, (void *)rtspClient ));
    rtspClient->sendPlayCommand(session, sendRequestCallback, start, end, scale, authenticator );

    UsageEnvironment& env = rtspClient->envir();

    rtspClient->fWatchVariableForSyncInterface=0;
    env.taskScheduler().doEventLoop(&rtspClient->fWatchVariableForSyncInterface );
    delete[] rtspClient->fResultString;

    BDBG_MSG(("%s: Ctx (%p), returning (%s) ", __FUNCTION__, (void *)rtspClient, rtspClient->fResponseBuffer ));
    return rtspClient->fResultCode==0;
}

static bool sendTeardownCmd ( RTSPClient* rtspClient, MediaSession& session )
{
    BDBG_MSG(("%s: calling sendTeardownCommand(); RTSPClient (%p) ", __FUNCTION__, (void *)rtspClient ));
    rtspClient->sendTeardownCommand(session, sendRequestCallback );

    UsageEnvironment& env = rtspClient->envir();

    rtspClient->fWatchVariableForSyncInterface=0;
    env.taskScheduler().doEventLoop(&rtspClient->fWatchVariableForSyncInterface );

    BDBG_MSG(("%s: Ctx (%p), returning (%s) ", __FUNCTION__, (void *)rtspClient, rtspClient->fResponseBuffer ));
    return true;
}

#if 0
static char * sendOptionsCmd ( RTSPClient* rtspClient )
{
    BDBG_MSG(("%s: calling sendOptionsCommand(); RTSPClient (%p) ", __FUNCTION__, rtspClient ));
    rtspClient->sendOptionsCommand(sendRequestCallback, NULL );

    UsageEnvironment& env = rtspClient->envir();

    rtspClient->fWatchVariableForSyncInterface=0;
    env.taskScheduler().doEventLoop(&rtspClient->fWatchVariableForSyncInterface );

    if ( rtspClient->fResultCode == 0)  return rtspClient->fResultString; // success
    BDBG_MSG(("%s: Ctx (%p), returning (%s) ", __FUNCTION__, rtspClient, rtspClient->fResultString ));
    return NULL;
}

static bool sendSetParameterCmd ( RTSPClient* rtspClient, MediaSession* rtspMediaSession,
                           const char * parameter,  const char * value, Authenticator* authenticator )
{
    bool rc = 0;
    BDBG_MSG(( "%s: calling sendSetParameterCommand()\n", __FUNCTION__ ));
    rc = rtspClient->sendSetParameterCommand( *rtspMediaSession, sendRequestCallback, parameter, value, authenticator);

    UsageEnvironment& env = rtspClient->envir();

    rtspClient->fWatchVariableForSyncInterface=0;
    env.taskScheduler().doEventLoop(&rtspClient->fWatchVariableForSyncInterface );

    rc = rtspClient->fResultCode == 0;
    BDBG_MSG(("%s: Ctx (%p), returning (%s) ", __FUNCTION__, rtspClient, rc ));
    return rtspClient->fResultCode==0;
}
#endif

static bool sendGetParameterCmd ( RTSPClient* rtspClient, MediaSession* rtspMediaSession, const char * parameterName,  char*& parameterValue )
{
    bool rc = 0;
    BDBG_MSG(( "%s: calling sendGetParameterCommand()\n", __FUNCTION__ ));
    rc = rtspClient->sendGetParameterCommand( *rtspMediaSession, sendRequestCallback, parameterName );

    UsageEnvironment& env = rtspClient->envir();

    rtspClient->fWatchVariableForSyncInterface=0;
    env.taskScheduler().doEventLoop(&rtspClient->fWatchVariableForSyncInterface );

    if( rtspClient->fResultString ) parameterValue = strdup( rtspClient->fResultString );

    rc = rtspClient->fResultCode == 0;
    BDBG_MSG(("%s: Ctx (%p), returning (%d) ", __FUNCTION__, (void *)rtspClient, rc ));
    return rtspClient->fResultCode==0;
}

void
lm_pkt_free(void *source_cnxt, void *pkt)
{
    BRTPSource *source = (BRTPSource *)source_cnxt;

    source->pkt_free(pkt);
    return;
}


lm_state_t
lm_init(void)
{
  lm_state_t lm = new lm_state;
  BDBG_ASSERT(lm);

  lm->scheduler = BasicTaskScheduler::createNew();
  lm->env = BasicUsageEnvironment::createNew(*lm->scheduler);
  return lm;
}

void
lm_run(lm_state_t lm, char *stop)
{
    BDBG_ASSERT(lm);
    lm->scheduler->doEventLoop(stop);
    return;
}

void
lm_shutdown(lm_state_t lm)
{
    delete lm->scheduler;
    lm->env->reclaim();
    delete lm;
    return;
}

#if 0
class MediaSubsession_: public MediaSubsession {
public:
    /* MediaSubsession_() {} */
    void setRTPSource(RTPSource *source) {
        fRTPSource = source;
    }
};
#endif


static int
lm_match_stream(lm_session_t session,   MediaSubsession *subsession)
{
    const char *codec = subsession->codecName();
    lm_stream_desc *desc=NULL;
    lm_stream_t stream=NULL;

    if (!codec) {
        return 0;
    }

    if ((!strcmp(codec, "H264"))) {
        const char *sprop = subsession->fmtp_spropparametersets();

        BDBG_WRN(("lm_match_stream: %#lx found H264 video stream", (unsigned long)session));
        stream =&session->stream[blm_media_video];
        desc = &stream->desc;
        if (sprop) {
            desc->cfg_len = strlen(sprop)+1;
            BDBG_ASSERT(desc->cfg_len <= sizeof(desc->cfg));
            memcpy(desc->cfg, sprop, desc->cfg_len);
        } else {
            desc->cfg_len = 0;
            *desc->cfg = '\0';
        }
        desc->stream_type = blm_stream_h264;
    } else if (!strcmp(codec, "MPEG4-GENERIC") && subsession->fmtp_mode() && !strcmp(subsession->fmtp_mode(), "aac-hbr")) {
        BDBG_WRN(("lm_match_stream: %#lx found AAC HBR audio stream", (unsigned long)session));
        stream =&session->stream[blm_media_audio];
        desc = &stream->desc;
        brtp_parser_mpeg4_stream_cfg *cfg = (brtp_parser_mpeg4_stream_cfg *)desc->cfg;
        desc->cfg_len = sizeof(*cfg);
        BDBG_ASSERT(desc->cfg_len <= sizeof(desc->cfg));
        brtp_parser_mpeg4_default_stream_cfg(brtp_parser_mpeg4_aac_hbr, cfg);
        if (subsession->fmtp_config()) {
            cfg->config_len = bhex_decode(subsession->fmtp_config(), strlen(subsession->fmtp_config()), cfg->config, sizeof(cfg->config));
        }
        cfg->sizelength = subsession->fmtp_sizelength();
        cfg->indexlength = subsession->fmtp_indexlength();
        cfg->profile = subsession->fmtp_profile_level_id();
        desc->stream_type = blm_stream_aac_hbr;
    } else if (!strcmp(codec, "MP4A-LATM") ) {
        BDBG_WRN(("lm_match_stream: %#lx found MPEG4-LATM audio stream", (unsigned long)session));
        stream =&session->stream[blm_media_audio];
        desc = &stream->desc;
        brtp_parser_aacloas_stream_cfg *cfg = (brtp_parser_aacloas_stream_cfg *)desc->cfg;
        desc->cfg_len = sizeof(*cfg);
        BDBG_ASSERT(desc->cfg_len <= sizeof(desc->cfg));
        brtp_parser_aacloas_default_stream_cfg(brtp_parser_aacloas_aac_loas, cfg);
        if (subsession->fmtp_config()) {
            cfg->config_len = bhex_decode(subsession->fmtp_config(), strlen(subsession->fmtp_config()), cfg->config, sizeof(cfg->config));
        }
        cfg->sizelength = subsession->fmtp_sizelength();
        cfg->indexlength = subsession->fmtp_indexlength();
        cfg->profile = subsession->fmtp_profile_level_id();
        desc->stream_type = blm_stream_aac_loas;
    } else if ( (!strcmp(codec, "H263-2000"))) {
        const char *sprop = subsession->fmtp_spropparametersets();

        BDBG_WRN(("lm_match_stream: %#lx found H263 video stream", (unsigned long)session));
        stream =&session->stream[blm_media_video];
        desc = &stream->desc;
        if (sprop) {
            desc->cfg_len = strlen(sprop)+1;
            BDBG_ASSERT(desc->cfg_len <= sizeof(desc->cfg));
            memcpy(desc->cfg, sprop, desc->cfg_len);
        } else {
            desc->cfg_len = 0;
            *desc->cfg = '\0';
        }
        desc->stream_type = blm_stream_h263;
        //return 0;

    } else if ( (!strcmp(codec, "MP4V-ES"))) {
        const char *sprop = subsession->fmtp_config();

        BDBG_WRN(("lm_match_stream: %#lx found MPEG-4 Part 2 video stream", (unsigned long)session));

        BDBG_WRN(("Sequence Header %s", sprop));
        stream =&session->stream[blm_media_video];
        desc = &stream->desc;
        if (sprop) {
            desc->cfg_len = strlen(sprop)+1;
            BDBG_ASSERT(desc->cfg_len <= sizeof(desc->cfg));
            memcpy(desc->cfg, sprop, desc->cfg_len);
        } else {
            desc->cfg_len = 0;
            *desc->cfg = '\0';
        }
        desc->stream_type = blm_stream_mpeg4part2;

    }else if ( (!strcmp(codec, "AMR"))) {
        BDBG_WRN(("lm_match_stream: %#lx found AMR audio stream", (unsigned long)session));
        stream =&session->stream[blm_media_audio];
        desc = &stream->desc;
        brtp_parser_amr_stream_cfg *cfg = (brtp_parser_amr_stream_cfg *)desc->cfg;
        desc->cfg_len = sizeof(*cfg);
        BDBG_ASSERT(desc->cfg_len <= sizeof(desc->cfg));
        //brtp_parser_amr_default_stream_cfg(brtp_parser_amr_m, cfg);
        cfg->sizelength = subsession->fmtp_sizelength();
        cfg->indexlength = subsession->fmtp_indexlength();
        cfg->profile = subsession->fmtp_profile_level_id();
        desc->stream_type = blm_stream_amr;
    }else if ( (!strcmp(codec, "PCMU"))) {
        BDBG_WRN(("lm_match_stream: %#lx found PCMU (G711) audio stream", (unsigned long)session));
        stream =&session->stream[blm_media_audio];
        desc = &stream->desc;
        brtp_parser_mpeg4_stream_cfg *cfg = (brtp_parser_mpeg4_stream_cfg *)desc->cfg;
        desc->cfg_len = sizeof(*cfg);
        BDBG_ASSERT(desc->cfg_len <= sizeof(desc->cfg));
        brtp_parser_mpeg4_default_stream_cfg(brtp_parser_mpeg4_g711, cfg);
        cfg->sizelength = subsession->fmtp_sizelength();
        cfg->indexlength = subsession->fmtp_indexlength();
        cfg->profile = subsession->fmtp_profile_level_id();
        desc->stream_type = blm_stream_g711;
    }else{
        BDBG_WRN(("lm_match_stream: %#lx Unknown codec=%s", (unsigned long)session, codec));
        return 0;
    }
    BDBG_ASSERT(desc);
    BDBG_ASSERT(stream);
    desc->rtp_format = subsession->rtpPayloadFormat();
    desc->clock_rate = subsession->rtpTimestampFrequency();
    desc->videoWidth = subsession->videoWidth();
    desc->videoHeight = subsession->videoHeight();
    desc->duration = subsession->playEndTime();
    stream->subsession = subsession;
    return 0;
}

lm_session_t
lm_connect(lm_state_t lm, const char *url)
{
    int verbosityLevel = 10;
    const char *applicationName = "rtp_test";
    portNumBits tunnelOverHTTPPortNum = 0;
    lm_session_t  session = new lm_session;
    char* sdpDescription;
    /* int simpleRTPoffsetArg = -1; */
    unsigned short desiredPortNum = 0;
    unsigned i;

    BDBG_ASSERT(session);

    for(i=0;i<sizeof(session->stream)/sizeof(*session->stream);i++) {
        session->stream[i].desc.stream_type = blm_stream_invalid;
        session->stream[i].subsession = NULL;
        session->stream[i].rtcpInstance = NULL;
        session->stream[i].rtpSource = NULL;
        session->stream[i].rtpSocket = NULL;
        session->stream[i].rtcpSocket = NULL;
    }
    session->lm = lm;

    session->client = RTSPClient::createNew(*lm->env, NULL, verbosityLevel, applicationName, 0, tunnelOverHTTPPortNum);
    BDBG_ASSERT(session->client);

    sdpDescription = sendDescribeCmd( session->client, url );
    session->statusCode = session->client->describeStatus();
    if (!sdpDescription) {
        BDBG_ERR(("lm_start:%#lx can't get sdp for %s",  (unsigned long)lm, url));
        goto err_sdp;
    }
    /* BDBG_WRN(("lm_start:%s -> %s", url, sdpDescription)); */

    session->session = MediaSession::createNew(*lm->env, sdpDescription);
    delete [] sdpDescription;
    if (session == NULL) {
        BDBG_ERR(("lm_start:%#lx can't start session for %s",  (unsigned long)lm, url));
        goto err_session;
    }
    {
        MediaSubsessionIterator iter(*session->session);
        MediaSubsession *subsession;
        while ((subsession = iter.next()) != NULL) {
            if (desiredPortNum != 0) {
              subsession->setClientPortNum(desiredPortNum);
              desiredPortNum += 2;
            }
            BDBG_WRN(("codec %s payloadFormat %u timestamp %u sprop '%s' config '%s' mode '%s'", subsession->codecName(), (unsigned)subsession->rtpPayloadFormat(), subsession->rtpTimestampFrequency(), subsession->fmtp_spropparametersets(), subsession->fmtp_config(), subsession->fmtp_mode() /* subsession->savedSDPLines() */));
            lm_match_stream(session, subsession);
#if 0
            if (!subsession->initiate(simpleRTPoffsetArg)) {
                BDBG_WRN(("lm_start:%#lx %s unable to create receiver for %s/%s subsession %s", (unsigned long)lm, url, subsession->mediumName(), subsession->codecName(), lm->env->getResultMsg()));
                continue;
            }
            BDBG_WRN(("lm_start:%#lx %s created receiver for %s/%s subsession (client ports %d-%d)", (unsigned long)lm, url, subsession->mediumName(), subsession->codecName(), subsession->clientPortNum(),subsession->clientPortNum()+1));
#endif
        }
    }

    return session;

err_session:
err_sdp:
    delete session;
    return NULL;
}

void
lm_session_close(lm_session_t session)
{
    unsigned i;
    BDBG_ASSERT(session);

    for(i=0;i<sizeof(session->stream)/sizeof(*session->stream);i++) {
        lm_stream_t stream = &session->stream[i];
        if(stream->desc.stream_type == blm_stream_invalid) {
            continue;
        }
        Medium::close(stream->rtcpInstance);
        stream->rtcpInstance = NULL;
        Medium::close(stream->rtpSource);
        stream->rtpSource = NULL;
        if(stream->rtpSocket) {
            delete stream->rtpSocket;
            stream->rtpSocket = NULL;
        }
        if(stream->rtcpSocket) {
            delete stream->rtcpSocket;
            stream->rtcpSocket = NULL;
        }
        stream->desc.stream_type = blm_stream_invalid;
    }

    if (session->client && session->session) {
        sendTeardownCmd( session->client, *session->session );
    }
    if (session->session) {
        Medium::close(session->session);
        session->session = NULL;
    }
    if (session->client) {
        Medium::close(session->client);
        session->client = NULL;
    }
    delete session;
}

const lm_stream_desc *
lm_get_stream_desc(lm_session_t session, blm_media media)
{
    BDBG_ASSERT(session);
    BDBG_ASSERT((unsigned)media < sizeof(session->stream)/sizeof(*session->stream));
    if(session->stream[media].desc.stream_type == blm_stream_invalid) {
        BDBG_ERR(("lm_get_stream_desc desc.stream_type is blm_stream_invalid"));
        return NULL;
    }
    return &session->stream[media].desc;
}

static brtp_enqueue
b_lm_ignore_data(void *rtp, void *pkt, size_t len, void *cookie)
{
    BSTD_UNUSED(rtp);
    BSTD_UNUSED(pkt);
    BSTD_UNUSED(len);
    BSTD_UNUSED(cookie);
    return brtp_enqueue_discard;
}

static void
b_lm_ignore_sync(void *cntx, uint32_t ntp_msw, uint32_t ntp_lsw, uint32_t timestamp_offset)
{
    BSTD_UNUSED(cntx);
    BSTD_UNUSED(ntp_msw);
    BSTD_UNUSED(ntp_lsw);
    BSTD_UNUSED(timestamp_offset);
    return;
}

static void
b_lm_ignore_bye(void *cntx)
{
    BSTD_UNUSED(cntx);
    return;
}

static const lm_stream_connect_cfg b_lm_stream_default_cfg = {
    NULL, /* rtp */
    NULL, /* stream_cntx */
    b_lm_ignore_data,
    b_lm_ignore_sync,
    b_lm_ignore_bye,
    false, /* use_tcp */
    128*1024, /* buffer_size */
};

void
lm_stream_default_cfg(lm_stream_connect_cfg *cfg)
{
    *cfg = b_lm_stream_default_cfg;
    return;
}

lm_stream_t
lm_session_stream(lm_session_t session, blm_media media, const lm_stream_connect_cfg *cfg)
{
    BDBG_ASSERT(session);
    BDBG_ASSERT(cfg);
    BRTPFactory factory(cfg);
    Boolean streamOutgoing=0, forceMulticastOnUnspecified=0;


    BDBG_ASSERT((unsigned)media < sizeof(session->stream)/sizeof(*session->stream));
    if(session->stream[media].desc.stream_type == blm_stream_invalid) {
        return NULL;
    }
    lm_stream_t stream = &session->stream[media];
    lm_stream_initiate(session, stream, factory);
    Boolean streamUsingTCP = cfg->use_tcp;

    sendSetupCmd( session->client, *stream->subsession, streamOutgoing, streamUsingTCP, forceMulticastOnUnspecified, NULL );
    BDBG_WRN(("lm_stream_connect: '%s'", session->lm->env->getResultMsg()));
    lm_stream_set_session(stream, session, streamUsingTCP, cfg->buffer_size);

    if (stream->rtcpInstance) {
        stream->rtcpInstance->setByeHandler(cfg->rtcp_bye, cfg->stream_cntx);
    }

    if (stream->rtcpInstance && stream->rtpSource) {
        stream->rtcpInstance->setSRHandler(BRTPSource::SRHandler, stream->rtpSource);
    }

    stream->rtpSource->doGetNextFrame();
    return stream;
}

TaskToken keepAliveTaskId;
void
lm_session_sendKeepAlive(void *ctx)
{
    lm_session_t session = (lm_session_t )ctx;
    Boolean success=0;
    char *paramValue = NULL;

    success = sendGetParameterCmd( session->client,  session->session, NULL, paramValue);
    if (!success) {
        BDBG_WRN(("lm_session %p: Failed to send the sendGetParameterCmd", (void *)session));
    }
    {
        int64_t timeoutMicroSeconds = 20 * 1000000;
        session->lm->env->taskScheduler().rescheduleDelayedTask( keepAliveTaskId, timeoutMicroSeconds, (TaskFunc *)&lm_session_sendKeepAlive, session);
    }
    BDBG_WRN(("%s: resscheduled keepalive timer & sent keepalive", __FUNCTION__));
}

int
lm_session_play(lm_session_t session)
{
    Boolean success=0;
    double start=0.0, end=-1.0;
    float scale=1.0;

    success = sendPlayCmd( session->client,  *session->session, start, end, scale, NULL );
#if 0
    unsigned i;
    for(i=0;i<sizeof(session->stream)/sizeof(*session->stream);i++) {
        lm_stream_t stream = &session->stream[i];
        if(stream->desc.stream_type == blm_stream_invalid) {
            continue;
        }
        /*
        session->client->playMediaSubsession(*stream->subsession);
        BDBG_WRN(("%u: rtp_info: track:%u seq:%u ts:%u", i, session->stream[i].subsession->rtpInfo.trackId, session->stream[i].subsession->rtpInfo.seqNum, session->stream[i].subsession->rtpInfo.timestamp));
        */
    }
#endif
    if (!success) {
        return -1;
    }

    {
        int64_t timeoutMicroSeconds = 20 * 1000000;
        keepAliveTaskId = session->lm->env->taskScheduler().scheduleDelayedTask( timeoutMicroSeconds, (TaskFunc *)&lm_session_sendKeepAlive, session);
    }
    return 0;
}

void
lm_stream_update_pkt_info(lm_stream_t stream, brtp_pkt_info *info, size_t nentries)
{
    BDBG_ASSERT(stream);
    BDBG_ASSERT(info);
    unsigned i;
    struct timeval resultPresentationTime;
    Boolean resultHasBeenSyncedUsingRTCP;
    Boolean useForJitterCalculation = true;


    RTPReceptionStatsDB& db = stream->rtpSource->receptionStatsDB();
    for(i=0;i<nentries;i++) {
        db.noteIncomingPacket(info[i].ssrc, info[i].sequence, info[i].timestamp,
                stream->subsession->rtpTimestampFrequency(),
                useForJitterCalculation, resultPresentationTime, resultHasBeenSyncedUsingRTCP,
                info[i].packet_size
                );
    }
    return;
}

lm_timer_t
lm_timer_schedulle(lm_state_t lm, unsigned delay /* ms */, void (*timer)(void *cntx), void *cntx)
{
    BDBG_ASSERT(lm);
    return (lm_timer_t) lm->env->taskScheduler().scheduleDelayedTask(delay*1000,  timer, cntx);
}

void
lm_timer_cancel(lm_state_t lm, lm_timer_t timer)
{
    TaskToken task = (TaskToken)timer;
    BDBG_ASSERT(lm);
    lm->env->taskScheduler().unscheduleDelayedTask(task);
}


void
lm_session_get_status(lm_session_t session, lm_session_status *status)
{
    BDBG_ASSERT(session);
    double end_time = session->session->playEndTime();
    if (end_time>0) {
        status->end_time = (unsigned)floor(end_time+0.5);
        if (status->end_time<1) {
            status->end_time = 1;
        }
    } else {
        status->end_time = 0;
    }
    return;
}
#endif /* SPF_SUPPORT */
