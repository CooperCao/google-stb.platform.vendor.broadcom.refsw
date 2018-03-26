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

#ifndef XMLTAGS_H__
#define XMLTAGS_H__

#ifdef __cplusplus
extern "C" {
#endif

#define XML_MAX_LENGTH             12

/* xml tags */
#define XML_TAG_ATLAS              "atlas"
#define XML_TAG_CHANNEL            "channel"
#define XML_TAG_LABEL              "label"
#define XML_TAG_CHANNELLIST        "channellist"
#define XML_TAG_PID                "pid"
#define XML_TAG_SECURITY           "security"
#define XML_TAG_STREAM             "stream"
#define XML_TAG_UDP_STREAMER       "udpStreamer"
#define XML_TAG_INFOFILE           "infofile"
#define XML_TAG_PROGRAM            "program"

/*XML Atrributes */
#define XML_ATT_VERSION            "version"
#define XML_ATT_MAJOR              "major"
#define XML_ATT_MINOR              "minor"
#define XML_ATT_TYPE               "type"
#define XML_ATT_BANDWIDTH          "bandwidth"
#define XML_ATT_MODE               "mode"
#define XML_ATT_FREQ               "freq"
#define XML_ATT_SYMRATE            "symrate"
#define XML_ATT_DISEQC             "diseqc"
#define XML_ATT_TONE               "tone"
#define XML_ATT_ADC                "adc"
#define XML_ATT_ANNEX              "annex"
#define XML_ATT_VIDEOPID           "videopid"
#define XML_ATT_AUDIOPID           "audiopid"
#define XML_ATT_PCRPID             "pcrpid"
#define XML_ATT_VIDEOTYPE          "videotype"
#define XML_ATT_FRAMERATE          "framerate"
#define XML_ATT_AUDIOTYPE          "audiotype"
#define XML_ATT_ANCILLARYPID       "ancillarypid"
#define XML_ATT_TEXT               "text"
#define XML_ATT_FILENAME           "filename"
#define XML_ATT_INDEXNAME          "indexname"
#define XML_ATT_SERVERINDEXSTATE   "serverIndexState"
#define XML_ATT_PATH               "path"
#define XML_ATT_PATH_INDEX         "pathIndex"
#define XML_ATT_PATH_INFO          "pathInfo"
#define XML_ATT_DATE               "date"
#define XML_ATT_SIZE               "size"
#define XML_ATT_X                  "x"
#define XML_ATT_Y                  "y"
#define XML_ATT_WIDTH              "width"
#define XML_ATT_HEIGHT             "height"
#define XML_ATT_ZORDER             "zorder"
#define XML_ATT_GLOBAL             "global"
#define XML_ATT_DURATION           "duration"
#define XML_ATT_MAX_BITRATE        "maxbitrate"
#define XML_ATT_BITRATE            "bitrate"
#define XML_ATT_SAMPLE_RATE        "samplerate"
#define XML_ATT_SAMPLE_SIZE        "samplesize"
#define XML_ATT_NUM_CHANNELS       "numchannels"
#define XML_ATT_DESCRIPTION        "description"
#define XML_ATT_INDEXREQUIRED      "indexrequired"
#define XML_ATT_ENCTYPE            "enctype"
#define XML_ATT_ENCKEY             "enckey"
#define XML_ATT_TIMESTAMP          "timestamp"
#define XML_ATT_URL                "url"
#define XML_ATT_HOST               "host"
#define XML_ATT_PORT               "port"
#define XML_ATT_INTERFACE          "interface"
#define XML_ATT_TRANSPORT          "transport"
#define XML_ATT_TYPE               "type"
#define XML_ATT_NUMBER             "number"
#define XML_ATT_LIVE               "live"
#define XML_ATT_PMT                "pmt"
#define XML_ATT_CKCCHECK           "ckccheck"
#define XML_ATT_DTCPKEYFORMAT      "dtcpkeyformat"
#define XML_ATT_DTCPAKEPORT        "dtcpakeport"
#define XML_ATT_SECURITY_PROTOCOL  "security"
#define XML_ATT_CACERT             "cacert"
#define XML_ATT_PLM                "plm"
#define XML_ATT_PLM_GFX            "gfxplm"

#ifdef __cplusplus
}
#endif

#endif /* XMLTAGS_H__ */