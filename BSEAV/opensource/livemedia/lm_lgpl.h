/***************************************************************************
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 ***************************************************************************/
/* interface exposed for the lm_lgpl.cpp file */
#ifndef _LM_LGPL_H_
#define _LM_LGPL_H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum blm_stream {blm_stream_invalid=0, blm_stream_h264, blm_stream_h263, blm_stream_mpeg4part2, blm_stream_aac_hbr, blm_stream_aac_loas, blm_stream_amr, blm_stream_g711} blm_stream;
typedef enum blm_media {blm_media_video=0, blm_media_audio=1, blm_media_last=2} blm_media;

typedef struct lm_state *lm_state_t;
typedef struct lm_session *lm_session_t;
typedef struct lm_stream  *lm_stream_t;
typedef struct lm_stream_desc {
    blm_stream stream_type;
    unsigned char rtp_format;
    uint32_t ssrc;
    unsigned clock_rate;
    unsigned cfg_len;
    unsigned char cfg[256];
    unsigned int duration;
    unsigned short videoWidth;
    unsigned short videoHeight;
} lm_stream_desc;

#ifdef __cplusplus
}

struct lm_state {
    UsageEnvironment* env;
    TaskScheduler* scheduler;
};

struct lm_stream {
    lm_stream_desc desc;
    MediaSubsession* subsession;
    RTCPInstance* rtcpInstance;
    RTPSource* rtpSource;
    Groupsock* rtpSocket;
    Groupsock* rtcpSocket;
};

struct lm_session {
    RTSPClient *client;
    MediaSession *session;
    unsigned statusCode;
    lm_state_t lm;
    lm_stream stream[blm_media_last];
};

class LM_RTPFactory {
public:
    LM_RTPFactory() { }
    virtual RTPSource *createSource(
        UsageEnvironment& env, Groupsock* RTPgs,
        unsigned char rtpPayloadFormat,
        unsigned rtpTimestampFrequency) = 0;
};
int lm_stream_initiate(lm_session_t lm_session, lm_stream_t stream, LM_RTPFactory &rtpFactory);
int lm_stream_set_session(lm_stream_t stream, lm_session_t session, bool usingTcp, unsigned bufferSize);

#endif

#endif /* _LM_LGPL_H_ */
