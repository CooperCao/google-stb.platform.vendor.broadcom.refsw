/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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
 *****************************************************************************/

#ifndef _STREAMER_HTTP_H__
#define _STREAMER_HTTP_H__

#include "resource.h"
#include "server.h"
#include "bip.h"

#ifdef __cplusplus
extern "C" {
#endif

class CWidgetEngine;
class CModel;
class CServerHttp;
class CVideo;
class CChannel;
class CConfig;

typedef enum
{
    eHttpStreamerInputType_Unknown,
    eHttpStreamerInputType_File,
    eHttpStreamerInputType_Tuner
} eHttpStreamerInputType;

class CStreamerHttp
{
public:
    CStreamerHttp(
            CConfiguration * pCfg,
            CServerHttp *    pServer
            );
    virtual ~CStreamerHttp(void);

    eRet initialize(CConfiguration * pCfg);
    eRet open(BIP_HttpRequestHandle hHttpRequest);
    void close(void);

    CServerHttp * getHttpServer(void) { return(_pServerHttp); }

    void                   setStreamerInputType(const char * url);
    BIP_HttpStreamerHandle getBipHttpStreamer(void) { return(_hHttpStreamer); }
    void                   setEndOfStreamer(void)   { _endOfStreamerRcvd = true; }
    bool                   checkEndOfStreamer(void) { return(_endOfStreamerRcvd); }

    void stopAndDestroyBipStreamer();

    void setProgram(const char * str);
    void setTranscodeOption(const char * str);

    void setStreamerStreamInfo(void);
    eRet addAllTracks(void);

    eRet start(BIP_HttpRequestHandle hHttpRequest);
    eRet stop(void);

protected:
    CConfiguration *       _pCfg;
    MString                _mediaFileAbsoloutePath; /* Absolute URL Path Name pointer */
    const char *           _pMethodName;            /* HTTP Request Method Name */
    const char *           _pPlaySpeed;             /* PlaySpeed string if set */
    eHttpStreamerInputType _streamerInputType;      /* Based on the requested Url we set this to file or tuner.*/
    MString                _tuneChannelName;        /* Channel name. Eg: 1.1 , 1.2, 1.3 etc.*/
    bool                   _programNumberValid;
    uint16_t               _programNumber;
    bool                   _rangeHeaderPresent;       /* Optional: if true, Request contains a Range Header. */
    uint64_t               _rangeStartOffset;         /* Optional: starting offset of the Range Header. */
    uint64_t               _rangeLength;              /* Optional: range length from the starting offset. */
    bool                   _dlnaTimeSeekRangePresent; /* Optional: if true, Request contains a dlnaTimeSeekRange Header. */
    int64_t                _startTimeInMs;            /* Optional: starting time of the dlnaTimeSeekRange Header. */
    int64_t                _endTimeInMs;              /* Optional: ending time of the dlnaTimeSeekRange Header. */
    BIP_HttpStreamerHandle _hHttpStreamer;            /* Cached Streamer handle */
    bool                   _streamerStarted;
    BIP_StreamerStreamInfo _streamerStreamInfo;
    bool                   _continuousPlay;
    bool                   _endOfStreamerRcvd;         /* End of streaming callback sets this to true.*/
    CServerHttp *          _pServerHttp;               /* Pointer to server object which has created this streamer object.*/
    CVideo *               _pVideo;                    /* This Cvideo objects conatins all file streaming information for a particular single program. */
    CChannel *             _pChannel;                  /* This CChannel objects contains all streaming information for a particular tuner input.*/
    void *                 _pAtlasId;                  /* model will provide this */
    bool                   _enableHls;
    bool                   _enableXcode;
    bool                   _enableLuaXcode;
    bool                   _enableDtcpIp;
    MString                _xcodeProfile;
    CConfig *              _pConfig;
};

#ifdef __cplusplus
}
#endif

#endif /* ifndef _STREAMER_HTTP_H__ */